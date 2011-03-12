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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/PubUtilLib/plNetClient/plNetCliAgeLeaver.cpp
*   
***/

#include "plNetCliAgeLeaver.h"
#include "plNetClientMgr.h"
#include "plNetLinkingMgr.h"

#include "../pnMessage/plTimeMsg.h"

#include "../plNetClientComm/plNetClientComm.h"
#include "../plNetGameLib/plNetGameLib.h"
#include "../plAgeLoader/plAgeLoader.h"
#include "../plAgeLoader/plBackgroundDownloader.h"
#include "../plAvatar/plAvatarMgr.h"
#include "../plVault/plVault.h"

#include "../plMessage/plLoadAgeMsg.h"
#include "../plMessage/plAgeLoadedMsg.h"
#include "../plMessage/plInputIfaceMgrMsg.h"



/*****************************************************************************
*
*   Private
*
***/

struct plNCAgeLeaver {

	enum NextOp {
		kNoOp,
		kDisableClickables,
		kLinkOutFX,
		kUnloadAge,
		kNotifyAgeUnloaded,
	};
	
	NextOp					nextOp;
	bool					quitting;
	bool					complete;
	FNCAgeLeaverCallback	callback;
	void *					userState;
	
	plNCAgeLeaver (
		bool					quitting,
		FNCAgeLeaverCallback	callback,
		void *					userState
	);
	~plNCAgeLeaver ();

	void Start ();
	void Complete (bool success, const char msg[]);
	bool MsgReceive (plMessage * msg);
	void Update ();
	void ExecNextOp ();
};


/*****************************************************************************
*
*   plNCAgeLeaver
*
***/

//============================================================================
plNCAgeLeaver::plNCAgeLeaver (
	bool					quitting,
	FNCAgeLeaverCallback	callback,
	void *					userState
) :	nextOp(kNoOp)
,	quitting(quitting)
,	complete(false)
,	callback(callback)
,	userState(userState)
{
}

//============================================================================
plNCAgeLeaver::~plNCAgeLeaver () {
}

//============================================================================
void plNCAgeLeaver::Start () {
	if (plBackgroundDownloader::GetInstance())
		plBackgroundDownloader::GetInstance()->Pause();

	nextOp = kLinkOutFX;
}

//============================================================================
void plNCAgeLeaver::Complete (bool success, const char msg[]) {

	if (!complete) {
		complete = true;
		
		NCAgeLeaveCompleteNotify	notify;
		notify.success	= success;
		notify.msg		= msg;
		
		callback(this, kAgeLeaveComplete, &notify, userState);
		DEL(this);
	}
}

//============================================================================
bool plNCAgeLeaver::MsgReceive (plMessage * msg) {
	plNetClientMgr *	nc = plNetClientMgr::GetInstance();
	plAvatarMgr *		am = plAvatarMgr::GetInstance();
	plAgeLoader *		al = plAgeLoader::GetInstance();

	//========================================================================
	// Done with link out effects
	//========================================================================
	if (plLinkOutUnloadMsg * linkOutUnloadMsg = plLinkOutUnloadMsg::ConvertNoRef(msg))
	{
		if (!linkOutUnloadMsg->HasBCastFlag(plMessage::kNetNonLocal)
			&& linkOutUnloadMsg->GetPlayerID() == NetCommGetPlayer()->playerInt
		) {
			nextOp = kUnloadAge;
		}
		return true;
	}
	
	//========================================================================
	// Age data unloaded
	//========================================================================
	if (plAgeLoadedMsg * ageLoadedMsg = plAgeLoadedMsg::ConvertNoRef(msg)) {
		if (!ageLoadedMsg->fLoaded) {
			nextOp = kNotifyAgeUnloaded;
			return true;
		}
		return false;
	}
	

	return false;
}

//============================================================================
void plNCAgeLeaver::ExecNextOp () {
	plNetClientMgr *	nc = plNetClientMgr::GetInstance();
	plAvatarMgr *		am = plAvatarMgr::GetInstance();
	plAgeLoader *		al = plAgeLoader::GetInstance();

	NextOp next = nextOp;
	nextOp		= kNoOp;
	switch (next) {
		//====================================================================
		case kNoOp: {
		}
		break;

		//====================================================================
		case kDisableClickables: {
			(TRACKED_NEW plInputIfaceMgrMsg(plInputIfaceMgrMsg::kDisableClickables))->Send();
			nextOp = kLinkOutFX;
		}
		break;

		//====================================================================
		case kLinkOutFX: {
			nc->StartLinkOutFX();
		}
		break;

		//====================================================================
		case kUnloadAge: {
			NetCliGameDisconnect();

			// Cull nodes that were part of this age vault (but not shared by the player's vault)
			VaultCull(NetCommGetAge()->ageVaultId);

			// remove the age device inbox mappings
			VaultClearDeviceInboxMap();
			
			// Tell our local player that he's unspawning (if that is indeed the case)
			nc->IPlayerChangeAge(true /* exiting */, 0/* respawn */);
			// disconnect age vault
			
			// @@@ TODO: Unload age vault here

			plAgeLoader::GetInstance()->UnloadAge();			// unload age
			nc->ISendCameraReset(false/*leaving age*/);			// reset camera
			nc->IUnloadRemotePlayers();							// unload other players

			if (NetCommNeedToLoadAvatar())
				am->UnLoadLocalPlayer();
		}
		break;

		//====================================================================
		case kNotifyAgeUnloaded: {
			// Set "Playing Game" bit to false
			nc->SetFlagsBit(plNetClientMgr::kPlayingGame, false);
			
			// Release AgeSDL object, if any
			if (nc->fAgeSDLObjectKey)
				nc->GetKey()->Release(nc->fAgeSDLObjectKey);
				
			// All done leaving age
			Complete(true, "Age unloaded");
		}
		break;

		DEFAULT_FATAL(nextOp);
	}
}

//============================================================================
void plNCAgeLeaver::Update () {
	ExecNextOp();
}


/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
void NCAgeLeaverCreate (
	plNCAgeLeaver **		pleaver,
	bool					quitting,
	FNCAgeLeaverCallback	callback,
	void *					userState
) {
	ASSERT(pleaver);
	ASSERT(callback);
	
	plNCAgeLeaver * leaver;
	*pleaver = leaver = NEWZERO(plNCAgeLeaver)(
		quitting,
		callback,
		userState
	);
	leaver->Start();
}

//============================================================================
bool NCAgeLeaverMsgReceive (
	plNCAgeLeaver *			leaver,
	plMessage *				msg
) {
	ASSERT(leaver);
	return leaver->MsgReceive(msg);
}

//============================================================================
void NCAgeLeaverUpdate (
	plNCAgeLeaver * leaver
) {
	ASSERT(leaver);
	leaver->Update();
}
