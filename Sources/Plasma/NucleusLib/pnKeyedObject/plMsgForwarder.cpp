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

#include "plMsgForwarder.h"

#include "HeadSpin.h"
#include "hsResMgr.h"
#include "hsStream.h"

#include "pnMessage/plMessage.h"
#include "pnMessage/plMessageWithCallbacks.h"
#include "pnMessage/plSelfDestructMsg.h"
#include "pnNetCommon/plNetApp.h"
#include "pnNetCommon/plSynchedObject.h"

#include <string_theory/format>

struct plForwardCallback
{
    std::vector<plKey> fOrigReceivers;
    size_t fNumCallbacks;
    bool fNetPropogate;
};

plMsgForwarder::plMsgForwarder()
{
}

plMsgForwarder::~plMsgForwarder()
{
    CallbackMap::iterator i = fCallbacks.begin();

    for (; i != fCallbacks.end(); i++)
        delete (*i).second;
}

void plMsgForwarder::Read(hsStream* s, hsResMgr* mgr)
{
    hsKeyedObject::Read(s, mgr);

    uint32_t numKeys = s->ReadLE32();
    fForwardKeys.clear();
    fForwardKeys.reserve(numKeys);
    for (uint32_t i = 0; i < numKeys; i++)
        fForwardKeys.emplace_back(mgr->ReadKey(s));
}

void plMsgForwarder::Write(hsStream* s, hsResMgr* mgr)
{
    hsKeyedObject::Write(s, mgr);

    s->WriteLE32((uint32_t)fForwardKeys.size());
    for (const plKey& key : fForwardKeys)
        mgr->WriteKey(s, key);
}

bool plMsgForwarder::MsgReceive(plMessage* msg)
{
    // Self destruct messages are for us only
    plSelfDestructMsg *selfMsg = plSelfDestructMsg::ConvertNoRef(msg);
    if (selfMsg)
        return hsKeyedObject::MsgReceive(msg);

    // If this is a callback message, it needs to be handled differently
    if (IForwardCallbackMsg(msg))
        return true;

    // All other messages are forwarded
    IForwardMsg(msg);
    return true;
}

bool plMsgForwarder::IForwardCallbackMsg(plMessage *msg)
{
    // Only process as a callback message if it is one, AND it has callbacks
    plMessageWithCallbacks *callbackMsg = plMessageWithCallbacks::ConvertNoRef(msg);
    if (callbackMsg && callbackMsg->GetNumCallbacks() > 0)
    {
        for (size_t i = 0; i < callbackMsg->GetNumCallbacks(); i++)
        {
            plEventCallbackMsg *event = callbackMsg->GetEventCallback(i);
            hsAssert(event, "Message forwarder only supports event callback messages");
            if (event)
            {
                plForwardCallback *fc = new plForwardCallback;
                fc->fNumCallbacks = fForwardKeys.size();

                // Turn off net propagate the callbacks to us will all be local.  Only the
                // callback we send will go over the net
                fc->fNetPropogate = (event->HasBCastFlag(plMessage::kNetPropagate) != 0);
                event->SetBCastFlag(plMessage::kNetPropagate, false);

                for (size_t j = 0; j < event->GetNumReceivers(); j++)
                    fc->fOrigReceivers.emplace_back(event->GetReceiver(j));

                event->ClearReceivers();
                event->AddReceiver(GetKey());

                fCallbacks[event] = fc;

#if 0
                hsStatusMessage(ST::format("Adding CBMsg, eventSender={}, eventRemoteMsg={}",
                    event->GetSender() ? event->GetSender()->GetName() : ST_LITERAL("nil"), fc->fNetPropogate).c_str());
#endif
            }
        }
#if 0
        hsStatusMessage(ST::format("Fwding CBMsg, sender={}, remoteMsg={}",
            msg->GetSender() ? msg->GetSender()->GetName() : ST_LITERAL("nil"), msg->HasBCastFlag(plMessage::kNetNonLocal)).c_str());
#endif
        IForwardMsg(callbackMsg);
        
        return true;
    }

    // Callback from one of our forward keys.  Don't send the final callback to the original
    // requester until all forward keys have reported in.
    plEventCallbackMsg *eventMsg = plEventCallbackMsg::ConvertNoRef(msg);
    if (eventMsg)
    {
        CallbackMap::const_iterator it = fCallbacks.find(eventMsg);
        if (it != fCallbacks.end())
        {
            plForwardCallback *fc = it->second;
            if (--fc->fNumCallbacks == 0)
            {
                hsStatusMessage(ST::format("plEventCallbackMsg received, erasing, sender={}, remoteMsg={}",
                    msg->GetSender() ? msg->GetSender()->GetName() : ST_LITERAL("nil"), msg->HasBCastFlag(plMessage::kNetNonLocal)).c_str());

                fCallbacks.erase(eventMsg);

                plUoid uoid = GetKey()->GetUoid();
                bool locallyOwned = (plNetClientApp::GetInstance()->IsLocallyOwned(uoid) != plSynchedObject::kNo);

                // If the callback was originally net propagated, and we own this forwarder, net propagate the callback
                if (fc->fNetPropogate && locallyOwned)
                    eventMsg->SetBCastFlag(plMessage::kNetPropagate);

                eventMsg->ClearReceivers();
                eventMsg->AddReceivers(fc->fOrigReceivers);
                eventMsg->SetSender(GetKey());
                hsRefCnt_SafeRef(eventMsg);
                eventMsg->Send();

                delete fc;
            }
        }
        else
        {
            hsStatusMessage(ST::format("! Unknown plEventCallbackMsg received, sender={}, remoteMsg={}",
                msg->GetSender() ? msg->GetSender()->GetName() : ST_LITERAL("nil"), msg->HasBCastFlag(plMessage::kNetNonLocal)).c_str());
            hsAssert(0, "Unknown plEventCallbackMsg received");
        }
        return true;
    }

    return false;
}

void plMsgForwarder::IForwardMsg(plMessage *msg)
{
    // Back up the message's original receivers
    std::vector<plKey> oldKeys;
    oldKeys.reserve(msg->GetNumReceivers());
    for (size_t i = 0; i < msg->GetNumReceivers(); i++)
        oldKeys.emplace_back(msg->GetReceiver(i));

    // Set to our receivers and send
    hsRefCnt_SafeRef(msg);
    msg->ClearReceivers();
    msg->AddReceivers(fForwardKeys);
    msg->Send();

    // Reset back to the original receivers.  This is so we don't screw up objects
    // who reuse their messages
    msg->ClearReceivers();
    msg->AddReceivers(oldKeys);
}

void plMsgForwarder::AddForwardKey(plKey key)
{
    const auto idx = std::find(fForwardKeys.begin(), fForwardKeys.end(), key);
    if (idx == fForwardKeys.end())
        fForwardKeys.emplace_back(std::move(key));
}
