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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/Protocols/Srv2State/pnNpSrv2State.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PRIVATE_PROTOCOLS_SRV2STATE_PNNPSRV2STATE_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/Protocols/Srv2State/pnNpSrv2State.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PRIVATE_PROTOCOLS_SRV2STATE_PNNPSRV2STATE_H


// kNetProtocolSrv2State messages
// Because SrvState must remain compatible with older auth builds, these message ids
// must not change unless all front-end servers are synchronously replaced.
enum {
	kSrv2State_SaveObject				= 0,
	kSrv2State_DeleteObject				= 1,
	kSrv2State_FetchObject				= 2,
};

enum {
	kState2Srv_ObjectFetched 			= 0,
};


//============================================================================
// BEGIN PACKED DATA STRUCTURES
//============================================================================
#include <PshPack1.h>


/*****************************************************************************
*
*   Srv2State connect packet
*
***/

struct Srv2State_ConnData {
    dword   dataBytes;
    dword   buildId;
    dword   srvType;
};
struct Srv2State_Connect {
    AsyncSocketConnectPacket    hdr;
    Srv2State_ConnData          data;
};


/*****************************************************************************
*
*   Srv2State message structures
*
***/

struct Srv2State_FetchObject : SrvMsgHeader {
	Uuid	ownerId;
	wchar	objectName[kMaxStateObjectName];
};

struct Srv2State_SaveObject : SrvMsgHeader {
	Uuid		ownerId;		
	wchar		objectName[kMaxStateObjectName];
	dword		objectDataBytes;
	byte		objectData[1]; // objectData[objectDataBytes], actually
	// no more fields after var length alloc
};

struct Srv2State_DeleteObject : SrvMsgHeader {
	Uuid	ownerId;		
	wchar	objectName[kMaxStateObjectName];
};


/*****************************************************************************
*
*   State2Srv message structures
*
***/

struct State2Srv_ObjectFetched : SrvMsgHeader { 
	dword	objectDataBytes;
	byte	objectData[1];
	// no more fields after var length alloc
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

bool Srv2StateValidateConnect (
    AsyncNotifySocketListen *   listen,
    Srv2State_ConnData *        connectPtr
);



