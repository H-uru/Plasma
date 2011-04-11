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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnGameMgr/Marker/pnGmMarker.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNGAMEMGR_MARKER_PNGMMARKER_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnGameMgr/Marker/pnGmMarker.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNGAMEMGR_MARKER_PNGMMARKER_H


/*****************************************************************************
*
*   Marker
*
***/

enum EMarkerInitResult {
	kMarkerInitSuccess,
	kMarkerInitError,
	kNumMarkerInitResults
};
enum EMarkerGameType {
	kMarkerGameQuest,
	kMarkerGameCGZ, // this is a quest game, but differentiating between the two on the client side makes some things easier
	kMarkerGameCapture,
	kMarkerGameCaptureAndHold,
	kNumMarkerGameTypes
};


//============================================================================
//	Game type id
//============================================================================

const Uuid kGameTypeId_Marker = Uuid(L"000b2c39-0319-4be1-b06c-7a105b160fcf");


//============================================================================
//	Network message ids
//============================================================================

// Cli2Srv message ids
enum {
	kCli2Srv_Marker_StartGame = kCli2Srv_NumGameMsgIds,
	kCli2Srv_Marker_PauseGame,
	kCli2Srv_Marker_ResetGame,
	kCli2Srv_Marker_ChangeGameName,
	kCli2Srv_Marker_ChangeTimeLimit,
	kCli2Srv_Marker_DeleteGame,
	kCli2Srv_Marker_AddMarker,
	kCli2Srv_Marker_DeleteMarker,
	kCli2Srv_Marker_ChangeMarkerName,
	kCli2Srv_Marker_CaptureMarker,
};

// Srv2Cli message ids
enum {
	kSrv2Cli_Marker_TemplateCreated = kSrv2Cli_NumGameMsgIds,
	kSrv2Cli_Marker_TeamAssigned,
	kSrv2Cli_Marker_GameType,
	kSrv2Cli_Marker_GameStarted,
	kSrv2Cli_Marker_GamePaused,
	kSrv2Cli_Marker_GameReset,
	kSrv2Cli_Marker_GameOver,
	kSrv2Cli_Marker_GameNameChanged,
	kSrv2Cli_Marker_TimeLimitChanged,
	kSrv2Cli_Marker_GameDeleted,
	kSrv2Cli_Marker_MarkerAdded,
	kSrv2Cli_Marker_MarkerDeleted,
	kSrv2Cli_Marker_MarkerNameChanged,
	kSrv2Cli_Marker_MarkerCaptured,
};


//============================================================================
// Begin networked data structures
#include <PshPack1.h>
//============================================================================

	//========================================================================
	// Message parameters
	//========================================================================
	struct Marker_CreateParam {
		byte		gameType;		// member of EMarkerGameType
		wchar		gameName[256];
		dword		timeLimit;
		wchar		templateID[80];	// empty if creating a new game, guid if a quest game and we need to grab the data from the state server
	};

	//========================================================================
	// Tic-Tac-Toe message structures
	//========================================================================

	// Cli2Srv
	struct Cli2Srv_Marker_StartGame : GameMsgHeader {
		// nothing
	};
	struct Cli2Srv_Marker_PauseGame : GameMsgHeader {
		// nothing
	};
	struct Cli2Srv_Marker_ResetGame : GameMsgHeader {
		// nothing
	};
	struct Cli2Srv_Marker_ChangeGameName : GameMsgHeader {
		wchar		gameName[256];
	};
	struct Cli2Srv_Marker_ChangeTimeLimit : GameMsgHeader {
		dword		timeLimit;
	};
	struct Cli2Srv_Marker_DeleteGame : GameMsgHeader {
		// nothing
	};
	struct Cli2Srv_Marker_AddMarker : GameMsgHeader {
		double		x;
		double		y;
		double		z;
		wchar		name[256];
		wchar		age[80];
	};
	struct Cli2Srv_Marker_DeleteMarker : GameMsgHeader {
		dword		markerID;
	};
	struct Cli2Srv_Marker_ChangeMarkerName : GameMsgHeader {
		dword		markerID;
		wchar		markerName[256];
	};
	struct Cli2Srv_Marker_CaptureMarker : GameMsgHeader {
		dword		markerID;
	};

	// Srv2Cli
	struct Srv2Cli_Marker_TemplateCreated : GameMsgHeader {
		wchar		templateID[80];
	};
	struct Srv2Cli_Marker_TeamAssigned : GameMsgHeader {
		byte		teamNumber;	// 1 or 2
	};
	struct Srv2Cli_Marker_GameType : GameMsgHeader {
		byte		gameType; // member of EMarkerGameType
	};
	struct Srv2Cli_Marker_GameStarted : GameMsgHeader {
		// nothing
	};
	struct Srv2Cli_Marker_GamePaused : GameMsgHeader {
		dword		timeLeft;	// 0 if quest game, since they don't have a timer
	};
	struct Srv2Cli_Marker_GameReset : GameMsgHeader {
		// nothing
	};
	struct Srv2Cli_Marker_GameOver : GameMsgHeader {
		// nothing
	};
	struct Srv2Cli_Marker_GameNameChanged : GameMsgHeader {
		wchar		newName[256];
	};
	struct Srv2Cli_Marker_TimeLimitChanged : GameMsgHeader {
		dword		newTimeLimit;
	};
	struct Srv2Cli_Marker_GameDeleted : GameMsgHeader {
		bool		failed; // did the delete fail?
	};
	struct Srv2Cli_Marker_MarkerAdded : GameMsgHeader {
		double		x;
		double		y;
		double		z;
		dword		markerID;
		wchar		name[256];
		wchar		age[80];
	};
	struct Srv2Cli_Marker_MarkerDeleted : GameMsgHeader {
		dword		markerID;
	};
	struct Srv2Cli_Marker_MarkerNameChanged : GameMsgHeader {
		dword		markerID;
		wchar		newName[256];
	};
	struct Srv2Cli_Marker_MarkerCaptured : GameMsgHeader {
		dword		markerID;
		byte		team;		// 0 for no team, or for quest games
	};


//============================================================================
// End networked data structures
#include <PopPack.h>
//============================================================================
