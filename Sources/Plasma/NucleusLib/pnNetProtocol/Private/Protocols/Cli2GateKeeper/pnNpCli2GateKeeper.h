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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/Protocols/pnNpCli2GateKeeper.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PRIVATE_PROTOCOLS_PNNPCLI2GATEKEEPER_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/Protocols/pnNpCli2GateKeeper.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PRIVATE_PROTOCOLS_PNNPCLI2GATEKEEPER_H


// kNetProtocolCli2GateKeeper messages (must be <= (word)-1)
enum {
    // Global
    kCli2GateKeeper_PingRequest,
    kCli2GateKeeper_FileSrvIpAddressRequest,
    kCli2GateKeeper_AuthSrvIpAddressRequest,

    kNumCli2GateKeeperMessages
};
COMPILER_ASSERT_HEADER(Cli2GateKeeper, kNumCli2GateKeeperMessages <= (word)-1);

enum {
    // Global
    kGateKeeper2Cli_PingReply,
    kGateKeeper2Cli_FileSrvIpAddressReply,
    kGateKeeper2Cli_AuthSrvIpAddressReply,
   
    kNumGateKeeper2CliMessages
};
COMPILER_ASSERT_HEADER(Cli2GateKeeper, kNumGateKeeper2CliMessages <= (word)-1);


//============================================================================
// BEGIN PACKED DATA STRUCTURES
//============================================================================
#include <PshPack1.h>


/*****************************************************************************
*
*   Cli2GateKeeper connect packet
*
***/

struct Cli2GateKeeper_ConnData {
    dword       dataBytes;
    Uuid        token;
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
    dword       messageId;
    dword       transId;
    dword       pingTimeMs;
    dword       payloadBytes;
    byte        payload[1]; // [payloadBytes]
};

// FileSrvIpAddressRequest
extern const NetMsg kNetMsg_Cli2GateKeeper_FileSrvIpAddressRequest;
struct Cli2GateKeeper_FileSrvIpAddressRequest {
    dword       messageId;
    dword       transId;
    byte        isPatcher;
};

// AuthSrvIpAddressRequest
extern const NetMsg kNetMsg_Cli2GateKeeper_AuthSrvIpAddressRequest;
struct Cli2GateKeeper_AuthSrvIpAddressRequest {
    dword       messageId;
    dword       transId;
};


//============================================================================
// GateKeeper --> Cli message structures

// PingReply
extern const NetMsg kNetMsg_GateKeeper2Cli_PingReply;
struct GateKeeper2Cli_PingReply {
    dword       messageId;
    dword       transId;
    dword       pingTimeMs;
    dword       payloadBytes;
    byte        payload[1]; // [payloadBytes]
};

// FileSrvIpAddressReply
extern const NetMsg kNetMsg_GateKeeper2Cli_FileSrvIpAddressReply;
struct GateKeeper2Cli_FileSrvIpAddressReply {
    dword messageId;
    dword transId;
    wchar address[24];
};


// FileSrvIpAddressReply
extern const NetMsg kNetMsg_GateKeeper2Cli_AuthSrvIpAddressReply;
struct GateKeeper2Cli_AuthSrvIpAddressReply {
    dword messageId;
    dword transId;
    wchar address[24];
};