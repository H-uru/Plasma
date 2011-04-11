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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnGameMgr/ClimbingWall/pnGmClimbingWall.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNGAMEMGR_CLIMBINGWALL_PNGMCLIMBINGWALL_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnGameMgr/ClimbingWall/pnGmClimbingWall.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNGAMEMGR_CLIMBINGWALL_PNGMCLIMBINGWALL_H


/*****************************************************************************
*
*   Climbing Wall
*
***/

enum EClimbingWallInitResult {
	kClimbingWallInitSuccess,
	kClimbingWallInitError,
	kNumClimbingWallInitResults
};
enum EClimbingWallReadyType {
	kClimbingWallReadyNumBlockers,
	kClimbingWallReadyBlockers,
};

const unsigned kClimbingWallMaxBlockers = 20; // TODO: Adjust this to the right size
const int kClimbingWallNoBlocker = -1; // the value of a slot in the blocker array when no blocker is in that slot


//============================================================================
//	Game type id
//============================================================================

const Uuid kGameTypeId_ClimbingWall = Uuid(L"6224cdf4-3556-4740-b7cd-d637562d07be");


//============================================================================
//	Network message ids
//============================================================================

// Cli2Srv message ids
enum {
	kCli2Srv_ClimbingWall_ChangeNumBlockers = kCli2Srv_NumGameMsgIds,
	kCli2Srv_ClimbingWall_Ready,
	kCli2Srv_ClimbingWall_BlockerChanged,
	kCli2Srv_ClimbingWall_Reset,
	kCli2Srv_ClimbingWall_PlayerEntered,
	kCli2Srv_ClimbingWall_FinishedGame,
	kCli2Srv_ClimbingWall_Panic,
};

// Srv2Cli message ids
enum {
	kSrv2Cli_ClimbingWall_NumBlockersChanged = kSrv2Cli_NumGameMsgIds,
	kSrv2Cli_ClimbingWall_Ready,
	kSrv2Cli_ClimbingWall_BlockersChanged,
	kSrv2Cli_ClimbingWall_PlayerEntered,
	kSrv2Cli_ClimbingWall_SuitMachineLocked,
	kSrv2Cli_ClimbingWall_GameOver,
};


//============================================================================
// Begin networked data structures
#include <PshPack1.h>
//============================================================================

	//========================================================================
	// Message parameters
	//========================================================================
	struct ClimbingWall_CreateParam {
		// no params
	};

	//========================================================================
	// Climbing Wall message structures
	//========================================================================

	// Cli2Srv
	struct Cli2Srv_ClimbingWall_ChangeNumBlockers : GameMsgHeader {
		int			amountToAdjust;		// + or - value to adjust the number of blockers by
	};
	struct Cli2Srv_ClimbingWall_Ready : GameMsgHeader {
		byte		readyType;			// the type of ready this message represents (EClimbingWallReadyType)
		byte		teamNumber;			// the team that you are saying is ready (1 or 2)
	};
	struct Cli2Srv_ClimbingWall_BlockerChanged : GameMsgHeader {
		byte		teamNumber;			// the team that is adjusting their blockers
		byte		blockerNumber;		// the number of the blocker that was added/removed
		bool		added;				// was the blocker added, or removed?
	};
	struct Cli2Srv_ClimbingWall_Reset : GameMsgHeader {
		// <no data>
	};
	struct Cli2Srv_ClimbingWall_PlayerEntered : GameMsgHeader {
		byte		teamNumber;			// the team this player is playing for
	};
	struct Cli2Srv_ClimbingWall_FinishedGame : GameMsgHeader {
		// <no data>
	};
	struct Cli2Srv_ClimbingWall_Panic : GameMsgHeader {
		// <no data>
	};

	// Srv2Cli
	struct Srv2Cli_ClimbingWall_NumBlockersChanged : GameMsgHeader {
		byte		newBlockerCount;	// the new number of blocker we are playing with
		bool		localOnly;			// only adjust your local display, don't net prop
	};
	struct Srv2Cli_ClimbingWall_Ready : GameMsgHeader {
		byte		readyType;			// the type of ready this message represents (EClimbingWallReadyType)
		bool		team1Ready;
		bool		team2Ready;
		bool		localOnly;			// only adjust your local display, don't net prop
	};
	struct Srv2Cli_ClimbingWall_BlockersChanged : GameMsgHeader {
		byte		teamNumber;			// the team this set of blockers is for
		int			blockersSet[kClimbingWallMaxBlockers];	// which blockers are set
		bool		localOnly;			// only adjust your local display, don't net prop
	};
	struct Srv2Cli_ClimbingWall_PlayerEntered : GameMsgHeader {
		// <no data>
	};
	struct Srv2Cli_ClimbingWall_SuitMachineLocked : GameMsgHeader {
		bool		team1MachineLocked;
		bool		team2MachineLocked;
		bool		localOnly;			// only adjust your local display, don't net prop
	};
	struct Srv2Cli_ClimbingWall_GameOver : GameMsgHeader {
		byte		teamWon;			// which team won the game
		int			team1Blockers[kClimbingWallMaxBlockers];
		int			team2Blockers[kClimbingWallMaxBlockers];
		bool		localOnly;			// only adjust your local display, don't net prop
	};


//============================================================================
// End networked data structures
#include <PopPack.h>
//============================================================================
