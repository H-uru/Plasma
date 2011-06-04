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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/Protocols/Srv2Mcp/pnNpSrv2Mcp.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PRIVATE_PROTOCOLS_SRV2MCP_PNNPSRV2MCP_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/Protocols/Srv2Mcp/pnNpSrv2Mcp.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PRIVATE_PROTOCOLS_SRV2MCP_PNNPSRV2MCP_H


// kNetProtocolSrv2Mcp messages
// Because SrvMcp must remain compatible with older auth builds, these message ids
// must not change unless all front-end servers are synchronously replaced.
enum {
    // Age
    kSrv2Mcp_AgeJoinRequest             = 0,
    kSrv2Mcp_PlayerLoggedOut            = 1,
    kSrv2Mcp_PlayerLoggedIn				= 2,
    kSrv2Mcp_AgeSpawned					= 3,
    kSrv2Mcp_AgeDied					= 4,
	kSrv2Mcp_KickPlayer					= 5,

    // Account
    kSrv2Mcp_AccountLoginRequest		= 20,
    kSrv2Mcp_AccountLogout              = 21,
    kSrv2Mcp_AccountSetPlayer           = 22,

	// Messaging
	kSrv2Mcp_PropagateBuffer			= 30,
};

enum {
    // Age
    kMcp2Srv_AgeJoinReply               = 0,
    kMcp2Srv_AgeSpawnRequest            = 1,
    kMcp2Srv_AgeAddPlayerRequest        = 2,
    kMcp2Srv_AgeRemovePlayerRequest     = 3,
    kMcp2Srv_AgeUnspawn                 = 4,
    kMcp2Srv_AgeSpawnedReply			= 5,

	// Account
	kMcp2Srv_AccountLoginReply			= 20,
    kMcp2Srv_AccountNotifyKicked		= 21,

	// Messaging
	kMcp2Srv_PropagateBuffer			= 30,

    // AgeSDL
    kMcp2Srv_NotifyAgeSDLChanged		= 40,
};


//============================================================================
// BEGIN PACKED DATA STRUCTURES
//============================================================================
#include <PshPack1.h>


/*****************************************************************************
*
*   Srv2Mcp connect packet
*
***/

struct Srv2Mcp_ConnData {
    dword   dataBytes;
    dword   buildId;
    dword   srvType;
    dword	publicAddr;
};
struct Srv2Mcp_Connect {
    AsyncSocketConnectPacket    hdr;
    Srv2Mcp_ConnData            data;
};


/*****************************************************************************
*
*   Srv2Mcp message structures
*
***/

struct Srv2Mcp_AgeJoinRequest : SrvMsgHeader {
	wchar		ageName[kMaxAgeNameLength];
    Uuid        ageUuid;
    Uuid        accountUuid;
    dword       playerInt;
	byte		ccrLevel;
	wchar		playerName[kMaxPlayerNameLength];
    dword		buildId;
};

struct Srv2Mcp_PlayerLoggedIn : SrvMsgHeader {
	dword		ageMcpId;
	Uuid		ageUuid;
    Uuid        accountUuid;
    wchar		playerName[kMaxPlayerNameLength];
    dword       playerInt;
};

struct Srv2Mcp_PlayerLoggedOut : SrvMsgHeader {
	dword		ageMcpId;
    Uuid        accountUuid;
    dword       playerInt;
};

struct Srv2Mcp_AgeSpawned : SrvMsgHeader {
	wchar		ageName[kMaxAgeNameLength];
	Uuid		ageUuid;
	dword		buildId;
};

struct Srv2Mcp_AgeDied : SrvMsgHeader {
	dword		ageMcpId;
};

struct Srv2Mcp_AccountLoginRequest : SrvMsgHeader {
    wchar       accountName[kMaxAccountNameLength];
    Uuid        accountUuid;
};

struct Srv2Mcp_AccountLogout : SrvMsgHeader {
    Uuid        accountUuid;
};

struct Srv2Mcp_AccountSetPlayer : SrvMsgHeader {
    Uuid        accountUuid;
    dword       playerInt;
};

struct Srv2Mcp_PropagateBuffer : SrvMsgHeader {
    dword		type;
	dword		bufferLength;
	dword		numRecvrs;
    // packed data:
	//		byte	netMessage[];
	//		dword	playerlist[];
};

struct Srv2Mcp_KickPlayer : SrvMsgHeader {
    dword       playerInt;
};

/*****************************************************************************
*
*   Mcp2Srv message structures
*
***/

struct Mcp2Srv_AgeJoinReply : SrvMsgHeader {
	dword			ageMcpId;
	Uuid			ageUuid;
    NetAddressNode  gameSrvNode;
};

struct Mcp2Srv_AgeSpawnRequest : SrvMsgHeader {
	wchar			ageName[kMaxAgeNameLength];
	dword			ageMcpId;
    Uuid            ageUuid;
    dword           buildId;
};

struct Mcp2Srv_AgeUnspawn : SrvMsgHeader {
    dword			ageMcpId;
};

struct Mcp2Srv_AgeAddPlayerRequest : SrvMsgHeader {
	dword			ageMcpId;
    Uuid            accountUuid;
    dword           playerInt;
	byte			ccrLevel;
	wchar			playerName[kMaxPlayerNameLength];
};

struct Mcp2Srv_AgeRemovePlayerRequest : SrvMsgHeader {
    Uuid            ageMcpId;
    Uuid            accountUuid;
    dword           playerInt;
};

struct Mcp2Srv_AgeSpawnedReply : SrvMsgHeader {
	dword			ageMcpId;	// assigns a new mcpId if age wasn't found (in case of a reconnect)
};

struct Mcp2Srv_AccountLoginReply : SrvMsgHeader {
	dword			acctMcpId;
};

struct Mcp2Srv_AccountNotifyKicked : SrvMsgHeader {
	Uuid			accountUuid;
	dword			acctMcpId;
	ENetError		reason;
};

struct Mcp2Srv_NotifyAgeSDLChanged : SrvMsgHeader {
	dword		ageMcpId;
	byte		global;
};

struct Mcp2Srv_PropagateBuffer : SrvMsgHeader {
	dword		ageMcpId;
	dword		type;
    dword		bytes;
    byte		buffer[1];  // actually, buffer[bytes]
    // no more fields
};


//============================================================================
// END PACKED DATA STRUCTURES
//============================================================================
#include <PopPack.h>


/*****************************************************************************
*
*   Srv2Mcp functions
*
***/

bool Srv2McpValidateConnect (
    AsyncNotifySocketListen *   listen,
    Srv2Mcp_ConnData *          connectPtr
);
