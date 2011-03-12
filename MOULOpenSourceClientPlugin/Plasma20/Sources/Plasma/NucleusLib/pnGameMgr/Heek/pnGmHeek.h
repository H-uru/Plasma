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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnGameMgr/Heek/pnGmHeek.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNGAMEMGR_HEEK_PNGMHEEK_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnGameMgr/Heek/pnGmHeek.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNGAMEMGR_HEEK_PNGMHEEK_H


/*****************************************************************************
*
*   Heek
*
***/

enum EHeekInitResult {
	kHeekInitSuccess,
	kHeekInitError,
	kNumHeekInitResults
};
enum EHeekChoice {
	kHeekRock,
	kHeekPaper,
	kHeekScissors,
	kNumHeekChoices
};
enum EHeekSeqFinished {
	kHeekCountdownSeq,
	kHeekChoiceAnimSeq,
	kHeekGameWinAnimSeq,
	kNumHeekSeq
};
enum EHeekLightState {
	kHeekLightOn,
	kHeekLightOff,
	kHeekLightFlash,
	kNumHeekLightStates
};
enum EHeekCountdownState {
	kHeekCountdownStart,
	kHeekCountdownStop,
	kHeekCountdownIdle,
	kNumHeekCountdownStates
};

//============================================================================
//	Game type id
//============================================================================

const Uuid kGameTypeId_Heek = Uuid(L"9d83c2e2-7835-4477-9aaa-22254c59a753");


//============================================================================
//	Network message ids
//============================================================================

// Cli2Srv message ids
enum {
	kCli2Srv_Heek_PlayGame = kCli2Srv_NumGameMsgIds,	// Sent when a player wants to join in the game (instead of observing)
	kCli2Srv_Heek_LeaveGame,							// Sent when a player is done playing (and starts observing)
	kCli2Srv_Heek_Choose,								// Sent when a player choses a move
	kCli2Srv_Heek_SeqFinished,							// Sent when a client-side animation ends
};

// Srv2Cli message ids
enum {
	kSrv2Cli_Heek_PlayGame = kSrv2Cli_NumGameMsgIds,	// Sent when the server allows or disallows a player to play
	kSrv2Cli_Heek_Goodbye,								// Sent when the server confirms the player leaving
	kSrv2Cli_Heek_Welcome,								// Sent to everyone when a new player joins
	kSrv2Cli_Heek_Drop,									// Sent when the admin needs to reset a position
	kSrv2Cli_Heek_Setup,								// Sent on link-in so observers see the correct game state (fast-forwarded)
	kSrv2Cli_Heek_LightState,							// Sent to a player when a light he owns changes state (animated)
	kSrv2Cli_Heek_InterfaceState,						// Sent to a player when his buttons change state (animated)
	kSrv2Cli_Heek_CountdownState,						// Sent to the admin to adjust the countdown state
	kSrv2Cli_Heek_WinLose,								// Sent to a player when he wins or loses a hand
	kSrv2Cli_Heek_GameWin,								// Sent to the admin when a game is won
	kSrv2Cli_Heek_PointUpdate,							// Sent to a player when their points change
};


//============================================================================
// Begin networked data structures
#include <PshPack1.h>
//============================================================================

	//========================================================================
	// Message parameters
	//========================================================================
	// No creation parameters

	//========================================================================
	// Heek message structures
	//========================================================================

	// Cli2Srv
	struct Cli2Srv_Heek_PlayGame : GameMsgHeader {
		byte		position;		// 0...4
		dword		points;
		wchar		name[256];
	};
	struct Cli2Srv_Heek_LeaveGame : GameMsgHeader {
		// no extra data
	};
	struct Cli2Srv_Heek_Choose : GameMsgHeader {
		byte		choice;			// kHeekRock...kHeekScissors
	};
	struct Cli2Srv_Heek_SeqFinished : GameMsgHeader {
		byte		seqFinished;	// kHeekCountdownSeq...kHeekGameWinSeq
	};

	// Srv2Cli
	struct Srv2Cli_Heek_PlayGame : GameMsgHeader {
		bool		isPlaying;
		bool		isSinglePlayer;
		bool		enableButtons;
	};
	struct Srv2Cli_Heek_Goodbye : GameMsgHeader {
		// no extra data
	};
	struct Srv2Cli_Heek_Welcome : GameMsgHeader {
		dword		points;
		dword		rank;
		wchar		name[256];
	};
	struct Srv2Cli_Heek_Drop : GameMsgHeader {
		byte		position;		// 0...4
	};
	struct Srv2Cli_Heek_Setup : GameMsgHeader {
		byte		position;		// 0...4
		bool		buttonState;
		bool		lightOn[6];
	};
	struct Srv2Cli_Heek_LightState : GameMsgHeader {
		byte		lightNum;
		byte		state;			// kHeekLightOn...kHeekLightFlash
	};
	struct Srv2Cli_Heek_InterfaceState : GameMsgHeader {
		bool		buttonsEnabled;
	};
	struct Srv2Cli_Heek_CountdownState : GameMsgHeader {
		byte		state;			// kHeekCountdownStart...kHeekCountdownIdle
	};
	struct Srv2Cli_Heek_WinLose : GameMsgHeader {
		bool		win;
		byte		choice;			// kHeekRock...kHeekScissors
	};
	struct Srv2Cli_Heek_GameWin : GameMsgHeader {
		byte		choice;			// kHeekRock...kHeekScissors
	};
	struct Srv2Cli_Heek_PointUpdate : GameMsgHeader {
		bool		displayUpdate;
		dword		points;
		dword		rank;
	};

//============================================================================
// End networked data structures
#include <PopPack.h>
//============================================================================