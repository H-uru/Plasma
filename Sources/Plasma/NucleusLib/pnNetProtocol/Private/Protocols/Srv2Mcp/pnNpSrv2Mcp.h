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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

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
    kSrv2Mcp_PlayerLoggedIn             = 2,
    kSrv2Mcp_AgeSpawned                 = 3,
    kSrv2Mcp_AgeDied                    = 4,
    kSrv2Mcp_KickPlayer                 = 5,

    // Account
    kSrv2Mcp_AccountLoginRequest        = 20,
    kSrv2Mcp_AccountLogout              = 21,
    kSrv2Mcp_AccountSetPlayer           = 22,

    // Messaging
    kSrv2Mcp_PropagateBuffer            = 30,
};

enum {
    // Age
    kMcp2Srv_AgeJoinReply               = 0,
    kMcp2Srv_AgeSpawnRequest            = 1,
    kMcp2Srv_AgeAddPlayerRequest        = 2,
    kMcp2Srv_AgeRemovePlayerRequest     = 3,
    kMcp2Srv_AgeUnspawn                 = 4,
    kMcp2Srv_AgeSpawnedReply            = 5,

    // Account
    kMcp2Srv_AccountLoginReply          = 20,
    kMcp2Srv_AccountNotifyKicked        = 21,

    // Messaging
    kMcp2Srv_PropagateBuffer            = 30,

    // AgeSDL
    kMcp2Srv_NotifyAgeSDLChanged        = 40,
};


//============================================================================
// BEGIN PACKED DATA STRUCTURES
//============================================================================
#pragma pack(push,1)


/*****************************************************************************
*
*   Srv2Mcp connect packet
*
***/

struct Srv2Mcp_ConnData {
    uint32_t   dataBytes;
    uint32_t   buildId;
    uint32_t   srvType;
    uint32_t   publicAddr;
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
    wchar_t       ageName[kMaxAgeNameLength];
    Uuid        ageUuid;
    Uuid        accountUuid;
    uint32_t       playerInt;
    uint8_t        ccrLevel;
    wchar_t       playerName[kMaxPlayerNameLength];
    uint32_t       buildId;
};

struct Srv2Mcp_PlayerLoggedIn : SrvMsgHeader {
    uint32_t       ageMcpId;
    Uuid        ageUuid;
    Uuid        accountUuid;
    wchar_t       playerName[kMaxPlayerNameLength];
    uint32_t       playerInt;
};

struct Srv2Mcp_PlayerLoggedOut : SrvMsgHeader {
    uint32_t       ageMcpId;
    Uuid        accountUuid;
    uint32_t       playerInt;
};

struct Srv2Mcp_AgeSpawned : SrvMsgHeader {
    wchar_t       ageName[kMaxAgeNameLength];
    Uuid        ageUuid;
    uint32_t       buildId;
};

struct Srv2Mcp_AgeDied : SrvMsgHeader {
    uint32_t       ageMcpId;
};

struct Srv2Mcp_AccountLoginRequest : SrvMsgHeader {
    wchar_t       accountName[kMaxAccountNameLength];
    Uuid        accountUuid;
};

struct Srv2Mcp_AccountLogout : SrvMsgHeader {
    Uuid        accountUuid;
};

struct Srv2Mcp_AccountSetPlayer : SrvMsgHeader {
    Uuid        accountUuid;
    uint32_t       playerInt;
};

struct Srv2Mcp_PropagateBuffer : SrvMsgHeader {
    uint32_t       type;
    uint32_t       bufferLength;
    uint32_t       numRecvrs;
    // packed data:
    //      uint8_t    netMessage[];
    //      uint32_t   playerlist[];
};

struct Srv2Mcp_KickPlayer : SrvMsgHeader {
    uint32_t       playerInt;
};

/*****************************************************************************
*
*   Mcp2Srv message structures
*
***/

struct Mcp2Srv_AgeJoinReply : SrvMsgHeader {
    uint32_t ageMcpId;
    Uuid     ageUuid;
    uint32_t gameSrvNode;
};

struct Mcp2Srv_AgeSpawnRequest : SrvMsgHeader {
    wchar_t           ageName[kMaxAgeNameLength];
    uint32_t           ageMcpId;
    Uuid            ageUuid;
    uint32_t           buildId;
};

struct Mcp2Srv_AgeUnspawn : SrvMsgHeader {
    uint32_t           ageMcpId;
};

struct Mcp2Srv_AgeAddPlayerRequest : SrvMsgHeader {
    uint32_t           ageMcpId;
    Uuid            accountUuid;
    uint32_t           playerInt;
    uint8_t            ccrLevel;
    wchar_t           playerName[kMaxPlayerNameLength];
};

struct Mcp2Srv_AgeRemovePlayerRequest : SrvMsgHeader {
    Uuid            ageMcpId;
    Uuid            accountUuid;
    uint32_t           playerInt;
};

struct Mcp2Srv_AgeSpawnedReply : SrvMsgHeader {
    uint32_t           ageMcpId;   // assigns a new mcpId if age wasn't found (in case of a reconnect)
};

struct Mcp2Srv_AccountLoginReply : SrvMsgHeader {
    uint32_t           acctMcpId;
};

struct Mcp2Srv_AccountNotifyKicked : SrvMsgHeader {
    Uuid            accountUuid;
    uint32_t           acctMcpId;
    ENetError       reason;
};

struct Mcp2Srv_NotifyAgeSDLChanged : SrvMsgHeader {
    uint32_t       ageMcpId;
    uint8_t        global;
};

struct Mcp2Srv_PropagateBuffer : SrvMsgHeader {
    uint32_t       ageMcpId;
    uint32_t       type;
    uint32_t       bytes;
    uint8_t        buffer[1];  // actually, buffer[bytes]
    // no more fields
};


//============================================================================
// END PACKED DATA STRUCTURES
//============================================================================
#pragma pack(pop)


/*****************************************************************************
*
*   Srv2Mcp functions
*
***/

bool Srv2McpValidateConnect (
    AsyncNotifySocketListen *   listen,
    Srv2Mcp_ConnData *          connectPtr
);
