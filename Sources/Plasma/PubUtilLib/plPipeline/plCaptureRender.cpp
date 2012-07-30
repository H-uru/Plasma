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

#include "HeadSpin.h"
#include "plCaptureRender.h"

#ifndef MF_FRONTBUFF_CAPTURE

#include "plGImage/plMipmap.h"
#include "plMessage/plCaptureRenderMsg.h"
#include "plPipeline.h"
#include "plRenderTarget.h"
#include "plScene/plPageTreeMgr.h"
#include "plScene/plPostEffectMod.h"
#include "hsResMgr.h"
#include "pnKeyedObject/plUoid.h"

#include "pfGameGUIMgr/pfGameGUIMgr.h"

#else // MF_FRONTBUFF_CAPTURE

#include "plPipeline.h"
#include "plgDispatch.h"

#include "plMessage/plCaptureRenderMsg.h"
#include "plGImage/plMipmap.h"

#endif // MF_FRONTBUFF_CAPTURE


#ifndef MF_FRONTBUFF_CAPTURE
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

hsTArray<plCaptureRenderMsg*>   plCaptureRender::fProcessed;

// plCaptureRender::Capture
bool plCaptureRender::Capture(const plKey& ack, uint16_t width, uint16_t height)
{
    // Create our render target
    const uint16_t flags = plRenderTarget::kIsOffscreen;
    const uint8_t bitDepth(32);
    const uint8_t zDepth(-1);
    const uint8_t stencilDepth(-1);
    plRenderTarget* rt = new plRenderTarget(flags, width, height, bitDepth, zDepth, stencilDepth);

    static int idx=0;
    plString buff = plString::Format("tRT%d", idx++);
    hsgResMgr::ResMgr()->NewKey(buff, rt, ack->GetUoid().GetLocation());


    // Create a render request and render request message
    plCaptureRenderRequest* req = new plCaptureRenderRequest;

    const float pri(-100.f);
    req->SetPriority(pri);

    req->SetRenderTarget(rt);

    const uint32_t renderState 
        = plPipeline::kRenderNormal
        | plPipeline::kRenderClearColor
        | plPipeline::kRenderClearDepth;
    req->SetRenderState(renderState);

    // Set the Ack to be our requestor
    req->RequestAck(ack);

    // Submit
    plRenderRequestMsg* msg = new plRenderRequestMsg(ack, req);
    hsRefCnt_SafeUnRef(req);
    msg->Send();

    return true;
}

// plCaptureRender::IProcess
bool plCaptureRender::IProcess(plPipeline* pipe, const plKey& ack, plRenderTarget* targ)
{
    // We've just had a successful render into our render target
    
    // Copy that into a plMipmap
    plMipmap* mipMap = pipe->ExtractMipMap(targ);
    if( !mipMap )
        return false;

    static int currentCapIndex = 0;

    // Mipmap isn't created with a key so let's give it one now
    plString buff = plString::Format("CaptureRender_%d", currentCapIndex++);

    hsgResMgr::ResMgr()->NewKey(buff, mipMap, plLocation::kGlobalFixedLoc);
    mipMap->Ref();

    // Stash it, and send it off during the update phase.
    plCaptureRenderMsg* msg = new plCaptureRenderMsg(ack, mipMap);
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

hsTArray<plCaptureRender::CapInfo>  plCaptureRender::fCapReqs;

void plCaptureRender::Update(plPipeline* pipe)
{
    int i;

    for( i = 0; i < fCapReqs.GetCount(); i++ )
    {
        plMipmap* mipmap = new plMipmap(fCapReqs[i].fWidth, fCapReqs[i].fHeight, plMipmap::kARGB32Config, 1);

        pipe->CaptureScreen(mipmap, false, fCapReqs[i].fWidth, fCapReqs[i].fHeight);

        plCaptureRenderMsg* msg = new plCaptureRenderMsg(fCapReqs[i].fAck, mipmap);
        msg->Send();
    }

    fCapReqs.Reset();
}

bool plCaptureRender::Capture(const plKey& ack, uint16_t width, uint16_t height)
{
    CapInfo capInfo;
    capInfo.fAck = ack;
    capInfo.fWidth = width;
    capInfo.fHeight = height;

    fCapReqs.Append(capInfo);

    return true;
}

#endif // MF_FRONTBUFF_CAPTURE
