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

#if 1	// for debugging
#include "plCreatableIndex.h"	
#include "../plModifier/plResponderModifier.h"
#include "../plSurface/plLayerAnimation.h"
#endif

#include "hsStream.h"
#include "plNetClientMgr.h"
#include "plgDispatch.h"
#include "hsResMgr.h"
#include "hsTimer.h"

#include "../plNetMessage/plNetMessage.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnKeyedObject/plFixedKey.h"
#include "../pnKeyedObject/hsKeyedObject.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnModifier/plModifier.h"
#include "../pnMessage/plNodeRefMsg.h"
#include "../pnMessage/plClientMsg.h"
#include "../pnMessage/plNodeChangeMsg.h"
#include "../pnMessage/plPlayerPageMsg.h"

#include "../plScene/plSceneNode.h"
#include "../plScene/plRelevanceMgr.h"
#include "../plNetTransport/plNetTransportMember.h"
#include "../plResMgr/plKeyFinder.h"
#include "../plAgeDescription/plAgeDescription.h"
#include "../plAvatar/plArmatureMod.h"
#include "../plAvatar/plAvatarMgr.h"
#include "../plSDL/plSDL.h"

/// TEMP HACK TO LOAD CONSOLE INIT FILES ON AGE LOAD
#include "../plMessage/plConsoleMsg.h"
#include "../plMessage/plLoadAvatarMsg.h"
#include "../plMessage/plAgeLoadedMsg.h"

#include "../plAgeLoader/plResPatcher.h"
#include "../plProgressMgr/plProgressMgr.h"
#include "../plResMgr/plRegistryHelpers.h"
#include "../plResMgr/plRegistryNode.h"
#include "../plResMgr/plResManager.h"

#include "process.h"	// for getpid()

extern	hsBool	gDataServerLocal;


// Load Player object
// a clone will be created if cloneNum>0 
// returns the playerKey if successful.
//
// Don't call this directly. Send a clone message to the NetClientManager instead.
// Load an object, optionally cloning if necessary. 
plKey plNetClientMgr::ILoadClone(plLoadCloneMsg *pCloneMsg)
{
	plKey cloneKey = pCloneMsg->GetCloneKey();

	if(pCloneMsg->GetIsLoading())
	{
		if (cloneKey->ObjectIsLoaded())
		{
			char tmp[256];
			DebugMsg("ILoadClone: object %s is already loaded, ignoring", cloneKey->GetUoid().StringIze(tmp));
			return cloneKey;
		}

		// check if local or remote player before loading
		plLoadAvatarMsg* loadAvMsg=plLoadAvatarMsg::ConvertNoRef(pCloneMsg);
		if (loadAvMsg && loadAvMsg->GetIsPlayer())
		{
			bool originating = ( pCloneMsg->GetOriginatingPlayerID() == this->GetPlayerID() );
			if (originating)
				fLocalPlayerKey = cloneKey;
			else
				AddRemotePlayerKey(cloneKey);
		}

		plKey cloneNodeKey = hsgResMgr::ResMgr()->FindKey(kNetClientCloneRoom_KEY);

		// Put the clone into the room, which also forces it to load.
		plNodeRefMsg* nodeRefCloneMsg = TRACKED_NEW plNodeRefMsg(cloneNodeKey, plNodeRefMsg::kOnRequest, -1, plNodeRefMsg::kObject);
		hsgResMgr::ResMgr()->AddViaNotify(cloneKey, nodeRefCloneMsg, plRefFlags::kActiveRef);

		// Finally, pump the dispatch system so all the new refs get delivered. ?
		plgDispatch::Dispatch()->MsgQueueProcess();
	}
	else		// we're unloading a clone
	{
		if (!cloneKey->ObjectIsLoaded())
		{
			DebugMsg("ILoadClone: object %s is already unloaded, ignoring", cloneKey->GetName());
			return cloneKey;
		}

		ICheckPendingStateLoad(hsTimer::GetSysSeconds());
		plSynchEnabler p(false);	// turn off dirty tracking while in this function

		GetKey()->Release(cloneKey);		// undo the active ref we took in ILoadClone

		// send message to scene object to remove him from the room
		plNodeChangeMsg* nodeChange = TRACKED_NEW plNodeChangeMsg(GetKey(), cloneKey, nil);
		plgDispatch::MsgSend(nodeChange);
	}

	plKey requestorKey = pCloneMsg->GetRequestorKey();

	// Readdress the message to the requestor and send it again
	plKey myKey = GetKey();
	pCloneMsg->SetBCastFlag(plMessage::kNetPropagate, false);
	pCloneMsg->ClearReceivers();
	pCloneMsg->AddReceiver(requestorKey);
	pCloneMsg->Ref();					// each message send unrefs once
	pCloneMsg->Send();

	return cloneKey;
}

//
// Cause a player to respawn. This is typically called on the local player when he links to a new age.
// or for unspawn:
//
void plNetClientMgr::IPlayerChangeAge(hsBool exitAge, Int32 spawnPt)
{
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
	
	if (avatar)
	{
		plSynchEnabler ps(false);	// disable state change tracking while we change ages
		if (exitAge)
			avatar->LeaveAge();
		else
		{
			hsBool validSpawn = (spawnPt >= 0);
			avatar->EnterAge(!validSpawn);
			if (validSpawn)
				avatar->SpawnAt(spawnPt, hsTimer::GetSysSeconds());
		}
	}
	else if (fLocalPlayerKey)
	{
		ErrorMsg("Can't find avatarMod %s", fLocalPlayerKey->GetName());
	}
}

