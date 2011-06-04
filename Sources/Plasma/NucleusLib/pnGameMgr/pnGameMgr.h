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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnGameMgr/pnGameMgr.h
*   
***/

#ifndef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNGAMEMGR_PNGAMEMGR_H
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNGAMEMGR_PNGAMEMGR_H


#include "../pnUtils/pnUtils.h"
#include "../pnNetBase/pnNetBase.h"
#include "../pnAsyncCore/pnAsyncCore.h"
#include "../pnNetCli/pnNetCli.h"
#include "../pnProduct/pnProduct.h"
#include "../pnKeyedObject/plKey.h"

#include "hsGeometry3.h"


/*****************************************************************************
*
*   GameMgr
*
***/

const unsigned	kGameMgrGlobalGameIdFlag = !((unsigned)-1 - 1);  // 0x10000000

//============================================================================
// EGameJoinError
//============================================================================
enum EGameJoinError {
	kGameJoinSuccess,
	kGameJoinErrNotExist,
	kGameJoinErrInitFailed,
	kGameJoinErrGameStarted,
	kGameJoinErrGameOver,
	kGameJoinErrMaxPlayers,
	kGameJoinErrAlreadyJoined,
	kGameJoinErrNoInvite,
	kNumGameJoinErrors
};

//============================================================================
// EGameInviteError
//============================================================================
enum EGameInviteError {
	kGameInviteSuccess,
	kGameInviteErrNotOwner,
	kGameInviteErrAlreadyInvited,
	kGameInviteErrAlreadyJoined,
	kGameInviteErrGameStarted,
	kGameInviteErrGameOver,
	kGameInviteErrGameFull,
	kGameInviteErrNoJoin,	// GameSrv reports the player may not join right now
	kNumGameInviteErrors
};

//============================================================================
// Game create/join options
//============================================================================
/*
	If set	:	Anyone may join; no invite necessary.
	Not set	:	Only players with invites may join.
*/
const unsigned kGameCreatePublic	= 1<<0;
/*
	If set	:	Anyone may invite others to play.
	Not set	:	Only the game owner may send invites.
*/
const unsigned kGameCreateOpen		= 1<<1;
/*
	If set	:   Player joins or creates the "common" instance of the game. In
                this case, the 'newGameId' field is not meaningful. If the
                common instance doesn't exist, it'll be created on-the-fly and
                the player will receive a GameCreated message as well as the
                normal GameJoined. This allows the game to be initialized once
                when first instanced.
	Not set	:   A game with the specified gameId must exist on the server.
                Depending on the options set during the game's creation, the
                player may need to have been sent an invite. Also, the game may
                not be in a state where it allows new players to join. Player
                receives a GameJoined reply in any case. Inspect the 'result'
                field to see whether the join was successful.
*/
const unsigned kGameJoinCommon		= 1<<2;
/*
*/
const unsigned kGameJoinObserver	= 1<<3;


//============================================================================
// GameMgr Network message ids
//============================================================================
enum {
	kCli2Srv_GameMgr_CreateGame,
	kCli2Srv_GameMgr_JoinGame,
};
enum {
	kSrv2Cli_GameMgr_GameInstance,		// Internal, not sent out in pfGameMgrMsg
	kSrv2Cli_GameMgr_InviteReceived,
	kSrv2Cli_GameMgr_InviteRevoked,
};

//============================================================================
// GameCli/Srv Network message ids
//============================================================================
enum {
	kCli2Srv_Game_LeaveGame,
	kCli2Srv_Game_Invite,
	kCli2Srv_Game_Uninvite,
	// Cli2Srv msgIds for specific games must begin with this value. See TicTacToe for example
	kCli2Srv_NumGameMsgIds
};
enum {
	kSrv2Cli_Game_PlayerJoined,
	kSrv2Cli_Game_PlayerLeft,
	kSrv2Cli_Game_InviteFailed,
	kSrv2Cli_Game_OwnerChange,
	// Srv2Cli msgIds for specific games must begin with this value. See TicTacToe for example
	kSrv2Cli_NumGameMsgIds
};


//============================================================================
// Begin networked data scructures
#include <PshPack1.h>
//============================================================================

	struct GameMsgHeader {
		dword		messageId;
		dword		transId;
		dword		recvGameId;	// 0 --> GameMgr, non-zero --> GameSrv
		dword		messageBytes;
	};

	//========================================================================
	// GameMgr message structures
	//========================================================================

	// Cli2Srv
	struct Cli2Srv_GameMgr_CreateGame : GameMsgHeader {
		Uuid				gameTypeId;
		dword				createOptions;
		dword				createDataBytes;
		byte				createData[1];	// [createDataBytes]
	};
	struct Cli2Srv_GameMgr_JoinGame : GameMsgHeader {
		// Field ordering here is vitally important, see pfGameMgr::JoinGame for explanation
		dword				newGameId;
		dword				createOptions;
		Uuid				gameTypeId;
		dword				createDataBytes;
		byte				createData[1];	// [createDataBytes]
	};

	// Srv2Cli
	struct Srv2Cli_GameMgr_GameInstance : GameMsgHeader {
		EGameJoinError		result;
		dword				ownerId;
		Uuid				gameTypeId;
		dword				newGameId;
	};
	struct Srv2Cli_GameMgr_InviteReceived : GameMsgHeader {
		dword				inviterId;
		Uuid				gameTypeId;
		dword				newGameId;
	};
	struct Srv2Cli_GameMgr_InviteRevoked : GameMsgHeader {
		dword				inviterId;
		Uuid				gameTypeId;
		dword				newGameId;
	};


	//========================================================================
	// GameCli/Srv message structures
	//========================================================================

	// Cli2Srv
	struct Cli2Srv_Game_LeaveGame : GameMsgHeader {
	};
	struct Cli2Srv_Game_Invite : GameMsgHeader {
		dword		playerId;
	};
	struct Cli2Srv_Game_Uninvite : GameMsgHeader {
		dword		playerId;
	};

	// Srv2Cli
	struct Srv2Cli_Game_PlayerJoined : GameMsgHeader {
		dword		playerId;
	};
	struct Srv2Cli_Game_PlayerLeft : GameMsgHeader {
		dword		playerId;
	};
	struct Srv2Cli_Game_InviteFailed : GameMsgHeader {
		dword				inviteeId;
		dword				operationId;
		EGameInviteError	error;
	};
	struct Srv2Cli_Game_OwnerChange : GameMsgHeader {
		dword		ownerId;
	};
	

//============================================================================
// End networked data structures
#include <PopPack.h>
//============================================================================


/*****************************************************************************
*
*   Games
*
***/

#include "TicTacToe/pnGmTicTacToe.h"
#include "Heek/pnGmHeek.h"
#include "Marker/pnGmMarker.h"
#include "BlueSpiral/pnGmBlueSpiral.h"
#include "ClimbingWall/pnGmClimbingWall.h"
#include "VarSync/pnGmVarSync.h"


#endif // PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNGAMEMGR_PNGAMEMGR_H
