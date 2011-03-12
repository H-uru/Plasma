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
#include "plAnimEventModifier.h"

#include "hsResMgr.h"
#include "../pnMessage/plMessage.h"

#include "../pnMessage/plRefMsg.h"
#include "../pnMessage/plEnableMsg.h"
#include "../pnMessage/plEventCallbackMsg.h"

plAnimEventModifier::plAnimEventModifier() : fCallback(nil), fDisabled(false)
{
}

plAnimEventModifier::~plAnimEventModifier()
{
	hsRefCnt_SafeUnRef(fCallback);
}

void plAnimEventModifier::Read(hsStream* stream, hsResMgr* mgr)
{
	plSingleModifier::Read(stream, mgr);

	int numReceivers = stream->ReadSwap32();
	fReceivers.Expand(numReceivers);
	for (int i = 0; i < numReceivers; i++)
		fReceivers.Push(mgr->ReadKey(stream));

	fCallback = plMessage::ConvertNoRef(mgr->ReadCreatable(stream));

	//
	// Create a passive ref to the animation controller.  Then, when we're
	// notified that it's loaded we can send our callback setup message.
	//
	plKey animKey = fCallback->GetReceiver(0);
	hsgResMgr::ResMgr()->AddViaNotify(animKey,
									TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, 0),
									plRefFlags::kPassiveRef);
}

void plAnimEventModifier::Write(hsStream* stream, hsResMgr* mgr)
{
	plSingleModifier::Write(stream, mgr);

	int numReceivers = fReceivers.Count();
	stream->WriteSwap32(numReceivers);
	for (int i = 0; i < numReceivers; i++)
		mgr->WriteKey(stream, fReceivers[i]);

	mgr->WriteCreatable(stream, fCallback);
}

hsBool plAnimEventModifier::MsgReceive(plMessage* msg)
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

#include "../pnNetCommon/plNetApp.h"
#include "../pnMessage/plNotifyMsg.h"

void plAnimEventModifier::ISendNotify(bool triggered)
{
	if (fDisabled)
		return;
	plNotifyMsg* notify = TRACKED_NEW plNotifyMsg;
	
	// Setup the event data in case this is a OneShot responder that needs it
	plKey playerKey = plNetClientApp::GetInstance()->GetLocalPlayerKey();
	notify->AddPickEvent(playerKey, nil, true, hsPoint3(0,0,0) );

	notify->SetSender(GetKey());
	notify->AddReceivers(fReceivers);
	notify->SetState(triggered ? 1.0f : 0.0f);
	// these should never need to net propagate...
	notify->SetBCastFlag(plMessage::kNetPropagate, false);
	notify->Send();
}
