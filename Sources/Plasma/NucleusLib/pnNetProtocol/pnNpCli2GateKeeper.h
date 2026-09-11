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

#ifndef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PNNPCLI2GATEKEEPER_H
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PNNPCLI2GATEKEEPER_H

#include "pnNpCommon.h"

// kNetProtocolCli2GateKeeper messages (must be <= (uint16_t)-1)
enum {
    // Global
    kCli2GateKeeper_PingRequest,
    kCli2GateKeeper_FileSrvIpAddressRequest,
    kCli2GateKeeper_AuthSrvIpAddressRequest,

    kNumCli2GateKeeperMessages
};
static_assert(kNumCli2GateKeeperMessages <= 0xFFFF, "Cli2GateKeeper message types overflow uint16");

enum {
    // Global
    kGateKeeper2Cli_PingReply,
    kGateKeeper2Cli_FileSrvIpAddressReply,
    kGateKeeper2Cli_AuthSrvIpAddressReply,
   
    kNumGateKeeper2CliMessages
};
static_assert(kNumGateKeeper2CliMessages <= 0xFFFF, "GateKeeper2Cli message types overflow uint16");


//============================================================================
// BEGIN PACKED DATA STRUCTURES
//============================================================================
#pragma pack(push,1)


/*****************************************************************************
*
*   Cli2GateKeeper connect packet
*
***/

struct Cli2GateKeeper_ConnData {
    uint32_t       dataBytes;
    plUUID      token;
};

struct Cli2GateKeeper_Connect {
    AsyncSocketConnectPacket    hdr;
    Cli2GateKeeper_ConnData     data;
};

//============================================================================
// Cli --> GateKeeper message structures

// PingRequest
extern const NetMsg kNetMsg_Cli2GateKeeper_PingRequest;
struct Cli2GateKeeper_PingRequest {
    uint32_t       messageId;
    uint32_t       transId;
    uint32_t       pingTimeMs;
    uint32_t       payloadBytes;
    uint8_t        payload[1]; // [payloadBytes]
};

// FileSrvIpAddressRequest
extern const NetMsg kNetMsg_Cli2GateKeeper_FileSrvIpAddressRequest;
struct Cli2GateKeeper_FileSrvIpAddressRequest {
    uint32_t       messageId;
    uint32_t       transId;
    uint8_t        isPatcher;
};

// AuthSrvIpAddressRequest
extern const NetMsg kNetMsg_Cli2GateKeeper_AuthSrvIpAddressRequest;
struct Cli2GateKeeper_AuthSrvIpAddressRequest {
    uint32_t       messageId;
    uint32_t       transId;
};


//============================================================================
// GateKeeper --> Cli message structures

// PingReply
extern const NetMsg kNetMsg_GateKeeper2Cli_PingReply;
struct GateKeeper2Cli_PingReply {
    uint32_t       messageId;
    uint32_t       transId;
    uint32_t       pingTimeMs;
    uint32_t       payloadBytes;
    uint8_t        payload[1]; // [payloadBytes]
};

// FileSrvIpAddressReply
extern const NetMsg kNetMsg_GateKeeper2Cli_FileSrvIpAddressReply;
struct GateKeeper2Cli_FileSrvIpAddressReply {
    uint32_t messageId;
    uint32_t transId;
    char16_t address[24];
};


// FileSrvIpAddressReply
extern const NetMsg kNetMsg_GateKeeper2Cli_AuthSrvIpAddressReply;
struct GateKeeper2Cli_AuthSrvIpAddressReply {
    uint32_t messageId;
    uint32_t transId;
    char16_t address[24];
};
#pragma pack(pop)

#endif // PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PNNPCLI2GATEKEEPER_H
