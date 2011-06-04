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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnGameMgr/VarSync/pnGmVarSync.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNGAMEMGR_VARSYNC_PNGMVARSYNC_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnGameMgr/VarSync/pnGmVarSync.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNGAMEMGR_VARSYNC_PNGMVARSYNC_H


/*****************************************************************************
*
*   Var Sync
*
***/

enum EVarSyncInitResult {
	kVarSyncInitSuccess,
	kVarSyncInitError,
	kNumVarSyncInitResults
};


//============================================================================
//	Game type id
//============================================================================

const Uuid kGameTypeId_VarSync = Uuid(L"475c2e9b-a245-4106-a047-9b25d41ff333");


//============================================================================
//	Network message ids
//============================================================================

// Cli2Srv message ids
enum {
	kCli2Srv_VarSync_SetStringVar = kCli2Srv_NumGameMsgIds,
	kCli2Srv_VarSync_SetNumericVar,
	kCli2Srv_VarSync_RequestAllVars,
	kCli2Srv_VarSync_CreateStringVar,
	kCli2Srv_VarSync_CreateNumericVar,
};

// Srv2Cli message ids
enum {
	kSrv2Cli_VarSync_StringVarChanged = kSrv2Cli_NumGameMsgIds,
	kSrv2Cli_VarSync_NumericVarChanged,
	kSrv2Cli_VarSync_AllVarsSent,
	kSrv2Cli_VarSync_StringVarCreated,
	kSrv2Cli_VarSync_NumericVarCreated,
};


//============================================================================
// Begin networked data structures
#include <PshPack1.h>
//============================================================================

	//========================================================================
	// Message parameters
	//========================================================================
	struct VarSync_CreateParam {
	};

	//========================================================================
	// VarSync message structures
	//========================================================================

	// Cli2Srv
	struct Cli2Srv_VarSync_SetStringVar : GameMsgHeader {
		unsigned long	varID;
		wchar			varValue[256];
	};
	struct Cli2Srv_VarSync_SetNumericVar : GameMsgHeader {
		unsigned long	varID;
		double			varValue;
	};
	struct Cli2Srv_VarSync_RequestAllVars : GameMsgHeader {
	};
	struct Cli2Srv_VarSync_CreateStringVar : GameMsgHeader {
		wchar		varName[256];
		wchar		varValue[256];
	};
	struct Cli2Srv_VarSync_CreateNumericVar : GameMsgHeader {
		wchar		varName[256];
		double		varValue;
	};

	// Srv2Cli
	struct Srv2Cli_VarSync_StringVarChanged : GameMsgHeader {
		unsigned long	varID;
		wchar			varValue[256];
	};
	struct Srv2Cli_VarSync_NumericVarChanged : GameMsgHeader {
		unsigned long	varID;
		double			varValue;
	};
	struct Srv2Cli_VarSync_AllVarsSent : GameMsgHeader {
	};
	struct Srv2Cli_VarSync_StringVarCreated : GameMsgHeader {
		wchar			varName[256];
		unsigned long	varID;
		wchar			varValue[256];
	};
	struct Srv2Cli_VarSync_NumericVarCreated : GameMsgHeader {
		wchar			varName[256];
		unsigned long	varID;
		double			varValue;
	};

//============================================================================
// End networked data structures
#include <PopPack.h>
//============================================================================
