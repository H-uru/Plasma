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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/Protocols/Srv2Log/pnNpSrv2Log.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PRIVATE_PROTOCOLS_SRV2LOG_PNNPSRV2LOG_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/Protocols/Srv2Log/pnNpSrv2Log.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PRIVATE_PROTOCOLS_SRV2LOG_PNNPSRV2LOG_H


//============================================================================
// BEGIN PACKED DATA STRUCTURES
//============================================================================
#include <PshPack1.h>

// Srv2Log
enum {
	kSrv2Log_LogMsg				= 0,
};

// Log2Srv
enum {
	
};


/*****************************************************************************
*
*   Srv2State connect packet
*
***/

struct Srv2Log_ConnData {
    dword   dataBytes;
    dword   buildId;
    dword   srvType;
	dword	buildType;
	dword	productId;
};

struct Srv2Log_Connect {
    AsyncSocketConnectPacket    hdr;
    Srv2Log_ConnData			data;
};


/*****************************************************************************
*
*   Srv2Log message structures
*
***/

struct Srv2Log_LogMsg : SrvMsgHeader {
	unsigned	eventType;
	qword		timestamp;
};



//============================================================================
// END PACKED DATA STRUCTURES
//============================================================================
#include <PopPack.h>


/*****************************************************************************
*
*   Srv2State functions
*
***/

bool Srv2LogValidateConnect (
    AsyncNotifySocketListen *   listen,
    Srv2Log_ConnData *        connectPtr
);


