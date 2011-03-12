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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnGameMgr/TicTacToe/pnGmTicTacToe.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNGAMEMGR_TICTACTOE_PNGMTICTACTOE_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnGameMgr/TicTacToe/pnGmTicTacToe.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNGAMEMGR_TICTACTOE_PNGMTICTACTOE_H


/*****************************************************************************
*
*   Tic-Tac-Toe
*
***/

enum ETTTInitResult {
	kTTTInitSuccess,
	kTTTInitError,
	kNumTTTInitResults
};
enum ETTTGameResult {
	kTTTGameResultWinner,	// there was a winning player
	kTTTGameResultTied,		// players tied (a "cat's game")
	kTTTGameResultGave,		// other player left the game
	kTTTGameResultError,	// something bad happened on the server
	kNumTTTGameResults
};

//============================================================================
//	Game type id
//============================================================================

const Uuid kGameTypeId_TicTacToe = Uuid(L"a7236529-11d8-4758-9368-59cb43445a83");


//============================================================================
//	Network message ids
//============================================================================

// Cli2Srv message ids
enum {
	kCli2Srv_TTT_MakeMove = kCli2Srv_NumGameMsgIds,
};

// Srv2Cli message ids
enum {
	kSrv2Cli_TTT_GameStarted = kSrv2Cli_NumGameMsgIds,
	kSrv2Cli_TTT_GameOver,
	kSrv2Cli_TTT_MoveMade,
};


//============================================================================
// Begin networked data scructures
#include <PshPack1.h>
//============================================================================

	//========================================================================
	// Message parameters
	//========================================================================
	struct TTT_CreateParam {
		byte		playerCount;	// 1 or 2
	};

	//========================================================================
	// Tic-Tac-Toe message structures
	//========================================================================

	// Cli2Srv
	struct Cli2Srv_TTT_MakeMove : GameMsgHeader {
		byte		row;			// 1..3
		byte		col;			// 1..3
	};

	// Srv2Cli
	struct Srv2Cli_TTT_GameStarted : GameMsgHeader {
		bool		yourTurn;		// randomly selected first player
	};
	struct Srv2Cli_TTT_GameOver : GameMsgHeader {
		ETTTGameResult	result;
		dword			winnerId;
	};
	struct Srv2Cli_TTT_MoveMade : GameMsgHeader {
		dword			playerId;
		byte			row;			// 1..3
		byte			col;			// 1..3
	};

//============================================================================
// End networked data structures
#include <PopPack.h>
//============================================================================
