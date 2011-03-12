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
#include "plSDLModifier.h"

#include "../pnNetCommon/plSynchedObject.h"
#include "../pnDispatch/plDispatch.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnMessage/plSDLModifierMsg.h"

#include "../plNetMessage/plNetMessage.h"
#include "../plSDL/plSDL.h"
#include "../plNetClient/plNetClientMgr.h"
#include "../plNetClient/plNetObjectDebugger.h"

plSDLModifier::plSDLModifier() : fStateCache(nil), fSentOrRecvdState(false)
{
}

plSDLModifier::~plSDLModifier()
{
	delete fStateCache;
}

plKey plSDLModifier::GetStateOwnerKey() const
{ 
	return GetTarget() ? GetTarget()->GetKey() : nil; 
}

void plSDLModifier::AddTarget(plSceneObject* so) 
{
	if (so)
		plSingleModifier::AddTarget(so);
	if (!fStateCache)
		fStateCache = TRACKED_NEW plStateDataRecord(GetSDLName());
}

UInt32 plSDLModifier::IApplyModFlags(UInt32 sendFlags)
{
	return sendFlags;
}

//
// write to net msg and send to server
//
void plSDLModifier::ISendNetMsg(plStateDataRecord*& state, plKey senderKey, UInt32 sendFlags)
{
	hsAssert(senderKey, "nil senderKey?");

	plSynchedObject* sobj = plSynchedObject::ConvertNoRef(senderKey->ObjectIsLoaded());
	if (sobj && (sobj->IsInSDLVolatileList(GetSDLName())))
		state->SetFlags(state->GetFlags() | plStateDataRecord::kVolatile);

	bool dirtyOnly = (sendFlags & plSynchedObject::kForceFullSend) == 0;
	bool broadcast = (sendFlags & plSynchedObject::kBCastToClients) != 0;
	int writeOptions=0;
//	if (dirtyOnly)
		writeOptions |= plSDL::kDirtyOnly;
	if (broadcast)
		writeOptions |= plSDL::kBroadcast;
		
	writeOptions |= plSDL::kTimeStampOnRead;
	
	plNetClientMgr::GetInstance()->StoreSDLState(state, senderKey->GetUoid(), sendFlags, writeOptions);
		
	fSentOrRecvdState = true;
}

//
// Process SDL msgs to send and recv state
//
hsBool plSDLModifier::MsgReceive(plMessage* msg)
{
	plSDLModifierMsg* sdlMsg = plSDLModifierMsg::ConvertNoRef(msg);
	if (sdlMsg && !stricmp(sdlMsg->GetSDLName(),GetSDLName()))
	{		
		UInt32 sendFlags = IApplyModFlags(sdlMsg->GetFlags());

		if (!fSentOrRecvdState)
			sendFlags |= plSynchedObject::kNewState;
		
		if (sdlMsg->GetAction()==plSDLModifierMsg::kSendToServer)
		{
			// local player is changing the state and sending it out
			plStateChangeNotifier::SetCurrentPlayerID(plNetClientApp::GetInstance()->GetPlayerID());	

			SendState(sendFlags);
		}
		else
		if (sdlMsg->GetAction()==plSDLModifierMsg::kSendToServerAndClients)
		{
			// local player is changing the state and sending it out
			plStateChangeNotifier::SetCurrentPlayerID(plNetClientApp::GetInstance()->GetPlayerID());

			SendState(sendFlags | plSynchedObject::kBCastToClients);
		}
		else
		if (sdlMsg->GetAction()==plSDLModifierMsg::kRecv)
		{
			plStateDataRecord* sdRec=sdlMsg->GetState();
			plStateChangeNotifier::SetCurrentPlayerID(sdlMsg->GetPlayerID());	// remote player changed the state
			ReceiveState(sdRec);
		}

		return true;	// consumed
	}

	return plSingleModifier::MsgReceive(msg);
}

//
// send a state update
//
bool gMooseDump=false;
void plSDLModifier::SendState(UInt32 sendFlags)
{
	hsAssert(fStateCache, "nil stateCache");

	bool debugObject = (plNetObjectDebugger::GetInstance() && 
			plNetObjectDebugger::GetInstance()->IsDebugObject(GetStateOwnerKey()->ObjectIsLoaded()));
	
	bool force = (sendFlags & plSynchedObject::kForceFullSend) != 0;
	bool broadcast = (sendFlags & plSynchedObject::kBCastToClients) != 0;

	// record current state
	plStateDataRecord* curState = TRACKED_NEW plStateDataRecord(GetSDLName());
	IPutCurrentStateIn(curState);	// return sdl record which reflects current state of sceneObj, dirties curState
	if (!force)
	{
		curState->FlagDifferentState(*fStateCache);	// flag items which are different from localCopy as dirty
	}

	if (curState->IsDirty())
	{
		// send current state
		bool dirtyOnly = force ? false : true;
		ISendNetMsg(curState, GetStateOwnerKey(), sendFlags);			// send the state

		if (debugObject)
		{
			gMooseDump=true;
			plNetObjectDebugger::GetInstance()->SetDebugging(true);
			curState->DumpToObjectDebugger(xtl::format("Object %s SENDS SDL state", 
				GetStateOwnerKey()->GetName(), dirtyOnly).c_str());
			gMooseDump=false;
		}

		// cache current state, send notifications if necessary
		fStateCache->UpdateFrom(*curState, dirtyOnly);	// update local copy of state
		
		ISentState(curState);
	}
	delete curState;

	if (plNetObjectDebugger::GetInstance())
		plNetObjectDebugger::GetInstance()->SetDebugging(false);
}

void plSDLModifier::ReceiveState(const plStateDataRecord* srcState)
{
	hsAssert(fStateCache, "nil stateCache");
	
	if (plNetObjectDebugger::GetInstance() && 
		plNetObjectDebugger::GetInstance()->IsDebugObject(GetStateOwnerKey()->ObjectIsLoaded()))
	{
		gMooseDump=true;
		plNetObjectDebugger::GetInstance()->SetDebugging(true);
		srcState->DumpToObjectDebugger(xtl::format("Object %s RECVS SDL state", 
			GetStateOwnerKey()->GetName()).c_str());
		gMooseDump=false;
	}

	if (srcState->IsUsed())
	{
		plSynchEnabler ps(false);	// disable dirty tracking while we are receiving/applying state

		// apply incoming state
		ISetCurrentStateFrom(srcState);			// apply incoming state to sceneObj
		
		// cache state, send notifications if necessary	
		fStateCache->UpdateFrom(*srcState, false);	// update local copy of state
		fSentOrRecvdState = true;
	}
	else
	{
		plNetClientApp::GetInstance()->DebugMsg("\tReceiving and ignoring unused SDL state msg: type %s, object %s", 
			GetSDLName(), GetStateOwnerKey()->GetName());
	}
	
	if (plNetObjectDebugger::GetInstance())
		plNetObjectDebugger::GetInstance()->SetDebugging(false);
}

void plSDLModifier::AddNotifyForVar(plKey key, const char* varName, float tolerance) const
{
	// create a SDL notifier object
	plStateChangeNotifier notifier(tolerance, key);
	// set the notification
	plStateDataRecord* rec = GetStateCache();
	if (rec)
	{
		plSimpleStateVariable* var = rec->FindVar(varName);
		// was the variable found?
		if (var)
			var->AddStateChangeNotification(notifier);
	}
}
