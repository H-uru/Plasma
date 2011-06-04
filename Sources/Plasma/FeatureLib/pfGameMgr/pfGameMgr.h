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
*   $/Plasma20/Sources/Plasma/FeatureLib/pfGameMgr/pfGameMgr.h
*   
***/

#ifndef PLASMA20_SOURCES_PLASMA_FEATURELIB_PFGAMEMGR_PFGAMEMGR_H
#define PLASMA20_SOURCES_PLASMA_FEATURELIB_PFGAMEMGR_PFGAMEMGR_H


#include "../pnUtils/pnUtils.h"
#include "../pnNetBase/pnNetBase.h"
#include "../pnAsyncCore/pnAsyncCore.h"
#include "../pnNetCli/pnNetCli.h"
#include "../pnProduct/pnProduct.h"
#include "../plNetGameLib/plNetGameLib.h"

#pragma warning(push, 0)
// These includes produce lots of warnings on W4
#include "../pnMessage/plMessage.h"
#include "../pnKeyedObject/hsKeyedObject.h"
#pragma warning(pop)


//============================================================================
// Forward declarations
//============================================================================
class pfGameCli;


//============================================================================
// pfGameMgrMsg
//-------------
// Notifications received from SrvGameMgr are sent to pfGameMgr's receivers in
// the form of a pfGameMgrMsg.
//============================================================================
class pfGameMgrMsg : public plMessage {
public:
	#pragma warning(push, 0)
	// These macros produce warnings on W4
	CLASSNAME_REGISTER(pfGameMgrMsg);
	GETINTERFACE_ANY(pfGameMgrMsg, plMessage);
	#pragma warning(pop)

	void Read(hsStream *, hsResMgr *) { FATAL("not impl"); }
	void Write(hsStream *, hsResMgr *) { FATAL("not impl"); }

	pfGameMgrMsg ();
	~pfGameMgrMsg ();
	
	void Set (const GameMsgHeader & msg);
	
	GameMsgHeader *	netMsg;
};


//============================================================================
// pfGameCliMsg
//-------------
// Notifications received by a pfGameCli are sent to its receiver in the form
// of a pfGameCliMsg.
//============================================================================
class pfGameCliMsg : public plMessage {
public:
	#pragma warning(push, 0)
	// These macros produce warnings on W4
	CLASSNAME_REGISTER(pfGameCliMsg);
	GETINTERFACE_ANY(pfGameCliMsg, plMessage);
	#pragma warning(pop)
	
	pfGameCliMsg ();
	~pfGameCliMsg ();

	void Read(hsStream *, hsResMgr *) { FATAL("not impl"); }
	void Write(hsStream *, hsResMgr *) { FATAL("not impl"); }

	void Set (pfGameCli * cli, const GameMsgHeader & msg);
	
	pfGameCli *		gameCli;
	GameMsgHeader *	netMsg;	
};


//============================================================================
// pfGameMgr singleton interface
//------------------------------
// pfGameMgr is the client-side proxy of the age server's SrvGameMgr. It sends
// client requests to the SrvGameMgr for processing, and executes SrvGameMgr
// commands locally to create pfGameCli objects, notify player of received
// game invites, etc.
//============================================================================
class pfGameMgr {
	friend struct IGameMgr;
	struct IGameMgr * internal;

	pfGameMgr ();
	
public:
	static pfGameMgr * GetInstance ();

	//========================================================================
	// Receiver list
	//--------------
	// When notificatons are received from SrvGameMgr, they are dispatched
	// as pfGameMgrMsgs to the receiver list maintained by these functions.
	void AddReceiver (plKey receiver);
	void RemoveReceiver (plKey receiver);
	//========================================================================
	
	//========================================================================
	// GameMgr properties
	//-------------------
	// Get a list of ids of games to which player is joined
	void			GetGameIds (ARRAY(unsigned) * arr)				const;
	// Return interface to the specified game	
	pfGameCli *		GetGameCli (unsigned gameId)					const;
	// Get the name of a game by its typeid
	const wchar *	GetGameNameByTypeId (const Uuid & gameTypeId)	const;
	//========================================================================

	//========================================================================
	// pfGameCli creation
	//-------------------
	// Join an existing game
	void JoinGame (
		plKey			receiver,		// Receiver of pfGameCliMsgs for this game
		unsigned		gameId			// id of the game to join
	);
	// Create a new game
	void CreateGame (
		plKey			receiver,		// Receiver of pfGameCliMsgs for this game
		const Uuid &	gameTypeId,		// typeid of game to create
		unsigned		createOptions,	// Game create options from pnGameMgr.h
		unsigned		initBytes,		// Game-specific initialization data
		const void *	initData
	);
	// Join or create the specified common game
	void JoinCommonGame (
		plKey			receiver,		// Receiver of pfGameCliMsgs for this game
		const Uuid &	gameTypeId,		// typeid of common game to create/join
		unsigned		gameNumber,		// "table number" of common game to create/join
		// In case the common game needs to
		// be created on the server, these
		// are its creation parameters:
		unsigned		initBytes,		// Game-specific initialization data
		const void *	initData
	);
	//========================================================================

	//========================================================================
	// @@@: FUTURE WORK
	//-----------------
	// Fetch the list of games registered with SrvGameMgr's matchmaking service.
	// void RequestPublishedGameList ();
	//========================================================================
};

//============================================================================
// pfGameCli interface
//============================================================================
class pfGameCli : public plCreatable {
	friend struct IGameMgr;
	friend struct IGameCli;
	struct IGameCli * internal;

	//========================================================================
	// sub-classes must implement these
	virtual void Recv			(GameMsgHeader * msg, void * param) = 0;
	virtual void OnPlayerJoined	(const Srv2Cli_Game_PlayerJoined & msg) = 0;
	virtual void OnPlayerLeft	(const Srv2Cli_Game_PlayerLeft & msg) = 0;
	virtual void OnInviteFailed	(const Srv2Cli_Game_InviteFailed & msg) = 0;
	virtual void OnOwnerChange	(const Srv2Cli_Game_OwnerChange & msg) = 0;
	//========================================================================
	
public:
	#pragma warning(push, 0)
	// These macros produce warnings on W4
	CLASSNAME_REGISTER(pfGameCli);
	GETINTERFACE_ANY(pfGameCli, plCreatable);
	#pragma warning(pop)
	
	pfGameCli (unsigned gameId, plKey receiver);
	~pfGameCli ();
	
	//========================================================================
	// Game client properties
	//-----------------------
	unsigned		GetGameId ()		const;
	const Uuid &	GetGameTypeId ()	const;
	const wchar *	GetName ()			const;
	plKey			GetReceiver ()		const;
	unsigned		GetPlayerCount ()	const;
	//========================================================================

	//========================================================================
	// Player invitation management
	//-----------------------------
	void InvitePlayer (unsigned playerId);
	void UninvitePlayer (unsigned playerId);
	//========================================================================

	//========================================================================
	// Game methods
	//-------------
	void LeaveGame ();
	//========================================================================

	//========================================================================
	// @@@: FUTURE WORK
	//-----------------
	// "Publish" this game, adding it to the age's the matchmaking service.
	// void PublishGame (const wchar desc[]);
	// void UnpublishGame ();
	//========================================================================
};


/*****************************************************************************
*
*   Games
*
***/

#include "TicTacToe/pfGmTicTacToe.h"
#include "Heek/pfGmHeek.h"
#include "Marker/pfGmMarker.h"
#include "BlueSpiral/pfGmBlueSpiral.h"
#include "ClimbingWall/pfGmClimbingWall.h"
#include "VarSync/pfGmVarSync.h"

#endif // PLASMA20_SOURCES_PLASMA_FEATURELIB_PFGAMEMGR_PFGAMEMGR_H
