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
#include "plAnimEventModifier.h"

#include "hsResMgr.h"
#include "hsStream.h"

#include "pnMessage/plEnableMsg.h"
#include "pnMessage/plEventCallbackMsg.h"
#include "pnMessage/plNotifyMsg.h"
#include "pnMessage/plRefMsg.h"
#include "pnNetCommon/plNetApp.h"

plAnimEventModifier::plAnimEventModifier() : fCallback(), fDisabled()
{
}

plAnimEventModifier::~plAnimEventModifier()
{
    hsRefCnt_SafeUnRef(fCallback);
}

void plAnimEventModifier::Read(hsStream* stream, hsResMgr* mgr)
{
    plSingleModifier::Read(stream, mgr);

    fReceivers.clear();
    uint32_t numReceivers = stream->ReadLE32();
    fReceivers.reserve(numReceivers);
    for (uint32_t i = 0; i < numReceivers; i++)
        fReceivers.emplace_back(mgr->ReadKey(stream));

    fCallback = plMessage::ConvertNoRef(mgr->ReadCreatable(stream));

    //
    // Create a passive ref to the animation controller.  Then, when we're
    // notified that it's loaded we can send our callback setup message.
    //
    plKey animKey = fCallback->GetReceiver(0);
    hsgResMgr::ResMgr()->AddViaNotify(animKey,
                                    new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, 0),
                                    plRefFlags::kPassiveRef);
}

void plAnimEventModifier::Write(hsStream* stream, hsResMgr* mgr)
{
    plSingleModifier::Write(stream, mgr);

    stream->WriteLE32((uint32_t)fReceivers.size());
    for (const plKey& key : fReceivers)
        mgr->WriteKey(stream, key);

    mgr->WriteCreatable(stream, fCallback);
}

bool plAnimEventModifier::MsgReceive(plMessage* msg)
{
    // Assuming we only have one ref, the anim time convert.  When it loads, we
    // send our callback setup message.
    plGenRefMsg* genRefMsg = plGenRefMsg::ConvertNoRef(msg);
    if (genRefMsg && (genRefMsg->GetContext() & plRefMsg::kOnCreate) && fCallback)
    {
        hsRefCnt_SafeRef(fCallback);
        fCallback->Send();
    }

    plEventCallbackMsg* callbackMsg = plEventCallbackMsg::ConvertNoRef(msg);
    if (callbackMsg)
    {
        ISendNotify(true);
        ISendNotify(false);
    }

    plEnableMsg* pEnable = plEnableMsg::ConvertNoRef(msg);
    if (pEnable)
    {
        if (pEnable->Cmd(plEnableMsg::kDisable))
            fDisabled = true;
        else
        if (pEnable->Cmd(plEnableMsg::kEnable))
            fDisabled = false;
        return true;
    }
    return plSingleModifier::MsgReceive(msg);
}

void plAnimEventModifier::ISendNotify(bool triggered)
{
    if (fDisabled)
        return;
    plNotifyMsg* notify = new plNotifyMsg;
    
    // Setup the event data in case this is a OneShot responder that needs it
    plKey playerKey = plNetClientApp::GetInstance()->GetLocalPlayerKey();
    notify->AddPickEvent(playerKey, nullptr, true, {});

    notify->SetSender(GetKey());
    notify->AddReceivers(fReceivers);
    notify->SetState(triggered ? 1.0f : 0.0f);
    // these should never need to net propagate...
    notify->SetBCastFlag(plMessage::kNetPropagate, false);
    notify->Send();
}
