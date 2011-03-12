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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnGameMgr/BlueSpiral/pnGmBlueSpiral.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNGAMEMGR_BLUESPIRAL_PNGMBLUESPIRAL_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnGameMgr/BlueSpiral/pnGmBlueSpiral.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNGAMEMGR_BLUESPIRAL_PNGMBLUESPIRAL_H


/*****************************************************************************
*
*   BlueSpiral
*
***/

enum EBlueSpiralInitResult {
	kBlueSpiralInitSuccess,
	kBlueSpiralInitError,
	kNumBlueSpiralInitResults
};


//============================================================================
//	Game type id
//============================================================================

const Uuid kGameTypeId_BlueSpiral = Uuid(L"5ff98165-913e-4fd1-a2c2-9c7f31be2cc8");


//============================================================================
//	Network message ids
//============================================================================

// Cli2Srv message ids
enum {
	kCli2Srv_BlueSpiral_StartGame = kCli2Srv_NumGameMsgIds,
	kCli2Srv_BlueSpiral_HitCloth,
};

// Srv2Cli message ids
enum {
	kSrv2Cli_BlueSpiral_ClothOrder = kSrv2Cli_NumGameMsgIds,
	kSrv2Cli_BlueSpiral_SuccessfulHit,
	kSrv2Cli_BlueSpiral_GameWon,
	kSrv2Cli_BlueSpiral_GameOver, // sent on time out and incorrect entry
	kSrv2Cli_BlueSpiral_GameStarted,
};


//============================================================================
// Begin networked data scructures
#include <PshPack1.h>
//============================================================================

	//========================================================================
	// Message parameters
	//========================================================================
	struct BlueSpiral_CreateParam {
		// empty
	};

	//========================================================================
	// Tic-Tac-Toe message structures
	//========================================================================

	// Cli2Srv
	struct Cli2Srv_BlueSpiral_StartGame : GameMsgHeader {
		// empty
	};
	struct Cli2Srv_BlueSpiral_HitCloth : GameMsgHeader {
		byte clothNum; // the cloth we hit, 0..6
	};

	// Srv2Cli
	struct Srv2Cli_BlueSpiral_ClothOrder : GameMsgHeader {
		byte order[7]; // each value is the cloth to hit, 0..6, the order is the order in the array
	};
	struct Srv2Cli_BlueSpiral_SuccessfulHit : GameMsgHeader {
		// empty
	};
	struct Srv2Cli_BlueSpiral_GameWon : GameMsgHeader {
		// empty
	};
	struct Srv2Cli_BlueSpiral_GameOver : GameMsgHeader {
		// empty
	};
	struct Srv2Cli_BlueSpiral_GameStarted : GameMsgHeader {
		bool startSpin; // if true, start spinning the door thingy
	};

//============================================================================
// End networked data structures
#include <PopPack.h>
//============================================================================
