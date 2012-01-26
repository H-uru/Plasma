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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/Protocols/Cli2Csr/pnNpCli2Csr.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PRIVATE_PROTOCOLS_CLI2CSR_PNNPCLI2CSR_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/Protocols/Cli2Csr/pnNpCli2Csr.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PRIVATE_PROTOCOLS_CLI2CSR_PNNPCLI2CSR_H


/*****************************************************************************
*
*  kNetProtocolCli2Csr message ids
*
***/

// Because SrvCsr must remain backward compatible with all client builds,
// the following enum values may never change under any circumstances.

//============================================================================
// Cli2Csr

enum {
    // Misc
    kCli2Csr_PingRequest        = 0,
    // Encrypt
    kCli2Csr_RegisterRequest    = 1,
    // Login
    kCli2Csr_LoginRequest       = 2,
    // Patch
    kCli2Csr_PatchRequest       = 3,

    kNumCli2CsrMessages
};
COMPILER_ASSERT_HEADER(Cli2Scr, kNumCli2CsrMessages <= (uint16_t)-1);


//============================================================================
// Csr2Cli

enum {
    // Misc
    kCsr2Cli_PingReply          = 0,
    // Encrypt
    kCsr2Cli_RegisterReply      = 1,
    // Login
    kCsr2Cli_LoginReply         = 2,
    // Patch
    kCli2Csr_PatchReply         = 3,
    
    kNumCsr2CliMessages
};
COMPILER_ASSERT_HEADER(Cli2Scr, kNumCsr2CliMessages <= (uint16_t)-1);


/*****************************************************************************
*
*   Networked structures
*
***/
#include <PshPack1.h>

//============================================================================
// Connect packet

struct Cli2Csr_ConnData {
    uint32_t   dataBytes;
};
struct Cli2Csr_Connect {
    AsyncSocketConnectPacket    hdr;
    Cli2Csr_ConnData            data;
};

//============================================================================
// Message header

struct Cli2Csr_MsgHeader {
    uint32_t       messageId;
    uint32_t       transId;
};

//============================================================================
// Cli --> Csr message structures

// PingRequest
extern const NetMsg kNetMsg_Cli2Csr_PingRequest;
struct Cli2Csr_PingRequest : Cli2Csr_MsgHeader {
    uint32_t       pingTimeMs;
    uint32_t       payloadBytes;
    uint8_t        payload[1]; // [payloadBytes]
};

// RegisterRequest
extern const NetMsg kNetMsg_Cli2Csr_RegisterRequest;
struct Cli2Csr_RegisterRequest : Cli2Csr_MsgHeader {
};

// LoginRequest
extern const NetMsg kNetMsg_Cli2Csr_LoginRequest;
struct Cli2Csr_LoginRequest : Cli2Csr_MsgHeader {
    uint32_t       clientChallenge;
    wchar_t       csrName[kMaxAccountNameLength];
    ShaDigest   challengeHash;
};


//============================================================================
// Csr --> Cli message structures

// PingReply
extern const NetMsg kNetMsg_Csr2Cli_PingReply;
struct Csr2Cli_PingReply : Cli2Csr_MsgHeader {
    uint32_t       pingTimeMs;
    uint32_t       payloadBytes;
    uint8_t        payload[1]; // [payloadBytes]
};

// RegisterReply
extern const NetMsg kNetMsg_Csr2Cli_RegisterReply;
struct Csr2Cli_RegisterReply : Cli2Csr_MsgHeader {
    uint32_t       serverChallenge;
    uint32_t       csrBuildId; // buildId of the latest csr client
};

// LoginReply
extern const NetMsg kNetMsg_Csr2Cli_LoginReply;
struct Csr2Cli_LoginReply : Cli2Csr_MsgHeader {
    ENetError   result;
    Uuid        csrId;
    uint32_t       csrFlags;
};


#include <PopPack.h>

