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
#include "plMsgForwarder.h"
#include "hsResMgr.h"
#include "../pnMessage/plMessage.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnNetCommon/plNetApp.h"
#include "../pnNetCommon/plSynchedObject.h"

#include "../pnMessage/plSelfDestructMsg.h"
#include "../pnMessage/plMessageWithCallbacks.h"


class plForwardCallback
{
public:
	hsTArray<plKey> fOrigReceivers;
	int fNumCallbacks;
	hsBool fNetPropogate;
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

	int numKeys = s->ReadSwap32();
	fForwardKeys.Reset();
	fForwardKeys.Expand(numKeys);
	fForwardKeys.SetCount(numKeys);
	for (int i = 0; i < numKeys; i++)
	{
		plKey key = mgr->ReadKey(s);
		fForwardKeys[i] = key;
	}
}

void plMsgForwarder::Write(hsStream* s, hsResMgr* mgr)
{
	hsKeyedObject::Write(s, mgr);

	int numKeys = fForwardKeys.Count();
	s->WriteSwap32(numKeys);
	for (int i = 0; i < numKeys; i++)
		mgr->WriteKey(s, fForwardKeys[i]);
}

hsBool plMsgForwarder::MsgReceive(plMessage* msg)
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

hsBool plMsgForwarder::IForwardCallbackMsg(plMessage *msg)
{
	// Only process as a callback message if it is one, AND it has callbacks
	plMessageWithCallbacks *callbackMsg = plMessageWithCallbacks::ConvertNoRef(msg);
	if (callbackMsg && callbackMsg->GetNumCallbacks() > 0)
	{
		for (int i = 0; i < callbackMsg->GetNumCallbacks(); i++)
		{
			plEventCallbackMsg *event = callbackMsg->GetEventCallback(i);
			hsAssert(event, "Message forwarder only supports event callback messages");
			if (event)
			{
				plForwardCallback *fc = TRACKED_NEW plForwardCallback;
				fc->fNumCallbacks = fForwardKeys.Count();

				// Turn off net propagate the callbacks to us will all be local.  Only the
				// callback we send will go over the net
				fc->fNetPropogate = (event->HasBCastFlag(plMessage::kNetPropagate) != 0);
				event->SetBCastFlag(plMessage::kNetPropagate, false);

				for (int j = 0; j < event->GetNumReceivers(); j++)
					fc->fOrigReceivers.Append((plKey)event->GetReceiver(j));

				event->ClearReceivers();
				event->AddReceiver(GetKey());

				fCallbacks[event] = fc;

#if 0
				hsStatusMessageF("Adding CBMsg, eventSender=%s, eventRemoteMsg=%d\n",					
					event->GetSender() ? event->GetSender()->GetName() : "nil", fc->fNetPropogate);
#endif
			}
		}
#if 0
		hsStatusMessageF("Fwding CBMsg, sender=%s, remoteMsg=%d",
			msg->GetSender() ? msg->GetSender()->GetName() : "nil", msg->HasBCastFlag(plMessage::kNetNonLocal));
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
				hsStatusMessageF("plEventCallbackMsg received, erasing, sender=%s, remoteMsg=%d\n",
					msg->GetSender() ? msg->GetSender()->GetName() : "nil", msg->HasBCastFlag(plMessage::kNetNonLocal));

				fCallbacks.erase(eventMsg);

				plUoid uoid = GetKey()->GetUoid();
				hsBool locallyOwned = (plNetClientApp::GetInstance()->IsLocallyOwned(uoid) != plSynchedObject::kNo);

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
			hsStatusMessageF("! Unknown plEventCallbackMsg received, sender=%s, remoteMsg=%d\n",
				msg->GetSender() ? msg->GetSender()->GetName() : "nil", msg->HasBCastFlag(plMessage::kNetNonLocal));
			hsAssert(0, "Unknown plEventCallbackMsg received");
		}
		return true;
	}

	return false;
}

void plMsgForwarder::IForwardMsg(plMessage *msg)
{
	// Back up the message's original receivers
	hsTArray<plKey> oldKeys;
	for (int i = 0; i < msg->GetNumReceivers(); i++)
		oldKeys.Append((plKey)msg->GetReceiver(i));

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
	if (fForwardKeys.Find(key) == fForwardKeys.kMissingIndex)
		fForwardKeys.Append(key);
}
