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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#include "plRenderRequest.h"

#include "HeadSpin.h"
#include "plgDispatch.h"
#include "hsFastMath.h"
#include "plPipeline.h"
#include "hsStream.h"

#include "plPageTreeMgr.h"
#include "plVisMgr.h"

#include "plPipeline/plRenderTarget.h"

plRenderRequest::plRenderRequest()
:   fRenderTarget(),
    fPageMgr(),
    fOverrideMat(),
    fEraseMat(),
    fDrawableMask(uint32_t(-1)),
    fSubDrawableMask(uint32_t(-1)),
    fRenderState(),
    fClearDepth(1.f),
    fFogStart(-1.f),
    fClearDrawable(),
    fPriority(-1.e6f),
    fUserData(),
    fIgnoreOccluders()
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
    fClearDrawable = nullptr;
    fRenderTarget = nullptr;
    fPageMgr = nullptr;

    fDrawableMask = s->ReadLE32();
    fSubDrawableMask = s->ReadLE32();

    fRenderState = s->ReadLE32();

    fLocalToWorld.Read(s);
    fWorldToLocal.Read(s);

    fPriority = s->ReadLEFloat();
}

void plRenderRequest::Write(hsStream* s, hsResMgr* mgr)
{
    s->WriteLE32(fDrawableMask);
    s->WriteLE32(fSubDrawableMask);

    s->WriteLE32(fRenderState);

    fLocalToWorld.Write(s);
    fWorldToLocal.Write(s);

    s->WriteLEFloat(fPriority);
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
        plRenderRequestAck* ack = new plRenderRequestAck( GetAck(), GetUserData() );
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

bool plRenderRequest::GetRenderCharacters() const 
{ 
    return fVisForce.IsBitSet(plVisMgr::kCharacter); 
}
