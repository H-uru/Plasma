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
#include "plCaptureRender.h"

#ifndef MF_FRONTBUFF_CAPTURE

#include "../plGImage/plMipmap.h"
#include "../plMessage/plCaptureRenderMsg.h"
#include "plPipeline.h"
#include "plRenderTarget.h"
#include "../plScene/plPageTreeMgr.h"
#include "../plScene/plPostEffectMod.h"
#include "hsResMgr.h"
#include "../pnKeyedObject/plUoid.h"

#include "../pfGameGUIMgr/pfGameGUIMgr.h"

// CaptureRenderRequest
//
void plCaptureRenderRequest::Render(plPipeline* pipe, plPageTreeMgr* pageMgr)
{
	// If we don't have a render target, something has gone horribly wrong.
	if( !GetRenderTarget() )
	{
		hsAssert(false, "CaptureRenderRequest with no render target");
		return;
	}

	// Set ourselves up like the current pipeline, except with our screen size.
	plViewTransform vt = pipe->GetViewTransform();
	vt.SetViewPort(0, 0, fRenderTarget->GetWidth(), fRenderTarget->GetHeight());
	SetViewTransform(vt);
	SetClearColor(pipe->GetClearColor());
	SetClearDepth(pipe->GetClearDepth());

	// Clear our render target
	// Render the scene
	pipe->PushRenderRequest(this);

	pipe->ClearRenderTarget();

	pageMgr->Render(pipe);

	pipe->PopRenderRequest(this);

	// set up state so we can clear the z-buffer for every gui dialog (and therefore not have it
	// be obscured by other geometry)
	SetRenderState(GetRenderState() & ~plPipeline::kRenderClearColor);
	SetRenderState(GetRenderState() | plPipeline::kRenderClearDepth);
	SetClearDepth(1);

	// render all GUI items
	std::vector<plPostEffectMod*> guiRenderMods = pfGameGUIMgr::GetInstance()->GetDlgRenderMods();
	for (int i = (int)guiRenderMods.size() - 1; i >= 0; i--) // render in reverse, so dialogs on the bottom get rendered first
	{
		plPageTreeMgr* dlgPageMgr = guiRenderMods[i]->GetPageMgr();
		if (dlgPageMgr)
		{
			SetViewTransform(guiRenderMods[i]->GetViewTransform());
			pipe->PushRenderRequest(this);
			pipe->ClearRenderTarget();
			dlgPageMgr->Render(pipe);
			pipe->PopRenderRequest(this);
		}
	}

	// Callback on plCaptureRender to process the render target into a mipmap
	// and send it back to the requester.
	plCaptureRender::IProcess(pipe, GetAck(), GetRenderTarget());

	delete fRenderTarget;
	fRenderTarget = nil;
}

hsTArray<plCaptureRenderMsg*>	plCaptureRender::fProcessed;

// plCaptureRender::Capture
hsBool plCaptureRender::Capture(const plKey& ack, UInt16 width, UInt16 height)
{
	// Create our render target
	const UInt16 flags = plRenderTarget::kIsOffscreen;
	const UInt8 bitDepth(32);
	const UInt8 zDepth(-1);
	const UInt8 stencilDepth(-1);
	plRenderTarget* rt = TRACKED_NEW plRenderTarget(flags, width, height, bitDepth, zDepth, stencilDepth);

	static int idx=0;
	char buff[32];
	sprintf(buff, "tRT%d", idx++);
	hsgResMgr::ResMgr()->NewKey(buff, rt, ack->GetUoid().GetLocation());


	// Create a render request and render request message
	plCaptureRenderRequest* req = TRACKED_NEW plCaptureRenderRequest;

	const hsScalar pri(-100.f);
	req->SetPriority(pri);

	req->SetRenderTarget(rt);

	const UInt32 renderState 
		= plPipeline::kRenderNormal
		| plPipeline::kRenderClearColor
		| plPipeline::kRenderClearDepth;
	req->SetRenderState(renderState);

	// Set the Ack to be our requestor
	req->RequestAck(ack);

	// Submit
	plRenderRequestMsg* msg = TRACKED_NEW plRenderRequestMsg(ack, req);
	hsRefCnt_SafeUnRef(req);
	msg->Send();

	return true;
}

// plCaptureRender::IProcess
hsBool plCaptureRender::IProcess(plPipeline* pipe, const plKey& ack, plRenderTarget* targ)
{
	// We've just had a successful render into our render target
	
	// Copy that into a plMipmap
	plMipmap* mipMap = pipe->ExtractMipMap(targ);
	if( !mipMap )
		return false;

	static int currentCapIndex = 0;

	// Mipmap isn't created with a key so let's give it one now
	char buff[512];
	sprintf(buff, "CaptureRender_%d", currentCapIndex++);

	hsgResMgr::ResMgr()->NewKey(buff, mipMap, plLocation::kGlobalFixedLoc);
	mipMap->Ref();

	// Stash it, and send it off during the update phase.
	plCaptureRenderMsg* msg = TRACKED_NEW plCaptureRenderMsg(ack, mipMap);
	fProcessed.Append(msg);

	return true;
}

void plCaptureRender::Update()
{
	int i;
	for( i = 0; i < fProcessed.GetCount(); i++ )
	{
		fProcessed[i]->Send();
	}
	fProcessed.SetCount(0);
}


#else // MF_FRONTBUFF_CAPTURE

#include "plPipeline.h"
#include "plgDispatch.h"

#include "../plMessage/plCaptureRenderMsg.h"
#include "../plGImage/plMipmap.h"

hsTArray<plCaptureRender::CapInfo>	plCaptureRender::fCapReqs;

void plCaptureRender::Update(plPipeline* pipe)
{
	int i;

	for( i = 0; i < fCapReqs.GetCount(); i++ )
	{
		plMipmap* mipmap = TRACKED_NEW plMipmap(fCapReqs[i].fWidth, fCapReqs[i].fHeight, plMipmap::kARGB32Config, 1);

		pipe->CaptureScreen(mipmap, false, fCapReqs[i].fWidth, fCapReqs[i].fHeight);

		plCaptureRenderMsg* msg = TRACKED_NEW plCaptureRenderMsg(fCapReqs[i].fAck, mipmap);
		msg->Send();
	}

	fCapReqs.Reset();
}

hsBool plCaptureRender::Capture(const plKey& ack, UInt16 width, UInt16 height)
{
	CapInfo capInfo;
	capInfo.fAck = ack;
	capInfo.fWidth = width;
	capInfo.fHeight = height;

	fCapReqs.Append(capInfo);

	return true;
}

#endif // MF_FRONTBUFF_CAPTURE
