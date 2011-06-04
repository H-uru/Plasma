/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011  Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#include "hsTypes.h"
#include "plRenderRequest.h"
#include "plPageTreeMgr.h"
#include "../plPipeline/plRenderTarget.h"
#include "hsFastMath.h"
#include "hsStream.h"
#include "plPipeline.h"
#include "../plMessage/plRenderRequestMsg.h"
#include "plgDispatch.h"
#include "plVisMgr.h"

plRenderRequest::plRenderRequest()
:	fRenderTarget(nil),
	fPageMgr(nil),
	fAck(nil),
	fOverrideMat(nil),
	fEraseMat(nil),
	fDrawableMask(UInt32(-1)),
	fSubDrawableMask(UInt32(-1)),
	fRenderState(0),
	fClearDepth(1.f),
	fFogStart(-1.f),
	fClearDrawable(nil),
	fPriority(-1.e6f),
	fUserData(0),
	fIgnoreOccluders(false)
{
	fClearColor.Set(0,0,0,1.f);

	fLocalToWorld.Reset();
	fWorldToLocal.Reset();
	
}

plRenderRequest::~plRenderRequest()
{
}

void plRenderRequest::SetLocalTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	fLocalToWorld = l2w;
	fWorldToLocal = w2l;
}

void plRenderRequest::Read(hsStream* s, hsResMgr* mgr)
{
	fClearDrawable = nil;
	fRenderTarget = nil;
	fPageMgr = nil;

	fDrawableMask = s->ReadSwap32();
	fSubDrawableMask = s->ReadSwap32();

	fRenderState = s->ReadSwap32();

	fLocalToWorld.Read(s);
	fWorldToLocal.Read(s);

	fPriority = s->ReadSwapScalar();
}

void plRenderRequest::Write(hsStream* s, hsResMgr* mgr)
{
	s->WriteSwap32(fDrawableMask);
	s->WriteSwap32(fSubDrawableMask);

	s->WriteSwap32(fRenderState);

	fLocalToWorld.Write(s);
	fWorldToLocal.Write(s);

	s->WriteSwapScalar(fPriority);
}

void plRenderRequest::Render(plPipeline* pipe, plPageTreeMgr* pageMgr)
{
	if( !fVisForce.Empty() )
	{
		plGlobalVisMgr::Instance()->DisableNormal();
		plGlobalVisMgr::Instance()->ForceVisSets(fVisForce, false);
	}

	pipe->PushRenderRequest(this);

	pipe->ClearRenderTarget(GetClearDrawable());

	int numDrawn = 0;
	if( GetPageTreeMgr() )
		numDrawn = GetPageTreeMgr()->Render(pipe);
	else
		numDrawn = pageMgr->Render(pipe);

	pipe->PopRenderRequest(this);
	
	if( GetAck() )
	{
		plRenderRequestAck* ack = TRACKED_NEW plRenderRequestAck( GetAck(), GetUserData() );
		ack->SetNumDrawn(numDrawn);
		plgDispatch::MsgSend( ack );
	}
}

void plRenderRequest::SetRenderTarget(plRenderTarget* t) 
{ 
	if( t != fRenderTarget )
	{
		fRenderTarget = t; 

		if( fRenderTarget )
		{
			fViewTransform.SetWidth(t->GetWidth());
			fViewTransform.SetHeight(t->GetHeight());
		}
	}
}

void plRenderRequest::SetVisForce(const hsBitVector& b)
{
	if( b.Empty() )
		fVisForce.Reset();
	else
		fVisForce = b;
}

hsBool plRenderRequest::GetRenderCharacters() const 
{ 
	return fVisForce.IsBitSet(plVisMgr::kCharacter); 
}
