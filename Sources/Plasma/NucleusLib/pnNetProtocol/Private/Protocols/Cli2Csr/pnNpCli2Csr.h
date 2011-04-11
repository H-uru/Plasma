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
	kCli2Csr_PingRequest		= 0,
	// Encrypt
	kCli2Csr_RegisterRequest	= 1,
	// Login
	kCli2Csr_LoginRequest		= 2,
	// Patch
	kCli2Csr_PatchRequest		= 3,

	kNumCli2CsrMessages
};
COMPILER_ASSERT_HEADER(Cli2Scr, kNumCli2CsrMessages <= (word)-1);


//============================================================================
// Csr2Cli

enum {
	// Misc
	kCsr2Cli_PingReply			= 0,
	// Encrypt
	kCsr2Cli_RegisterReply		= 1,
	// Login
	kCsr2Cli_LoginReply			= 2,
	// Patch
	kCli2Csr_PatchReply			= 3,
	
	kNumCsr2CliMessages
};
COMPILER_ASSERT_HEADER(Cli2Scr, kNumCsr2CliMessages <= (word)-1);


/*****************************************************************************
*
*   Networked structures
*
***/
#include <PshPack1.h>

//============================================================================
// Connect packet

struct Cli2Csr_ConnData {
	dword	dataBytes;
};
struct Cli2Csr_Connect {
	AsyncSocketConnectPacket	hdr;
	Cli2Csr_ConnData			data;
};

//============================================================================
// Message header

struct Cli2Csr_MsgHeader {
	dword		messageId;
	dword		transId;
};

//============================================================================
// Cli --> Csr message structures

// PingRequest
extern const NetMsg kNetMsg_Cli2Csr_PingRequest;
struct Cli2Csr_PingRequest : Cli2Csr_MsgHeader {
	dword		pingTimeMs;
	dword		payloadBytes;
	byte		payload[1];	// [payloadBytes]
};

// RegisterRequest
extern const NetMsg kNetMsg_Cli2Csr_RegisterRequest;
struct Cli2Csr_RegisterRequest : Cli2Csr_MsgHeader {
};

// LoginRequest
extern const NetMsg kNetMsg_Cli2Csr_LoginRequest;
struct Cli2Csr_LoginRequest : Cli2Csr_MsgHeader {
	dword		clientChallenge;
	wchar		csrName[kMaxAccountNameLength];
	ShaDigest	challengeHash;
};


//============================================================================
// Csr --> Cli message structures

// PingReply
extern const NetMsg kNetMsg_Csr2Cli_PingReply;
struct Csr2Cli_PingReply : Cli2Csr_MsgHeader {
	dword		pingTimeMs;
	dword		payloadBytes;
	byte		payload[1];	// [payloadBytes]
};

// RegisterReply
extern const NetMsg kNetMsg_Csr2Cli_RegisterReply;
struct Csr2Cli_RegisterReply : Cli2Csr_MsgHeader {
	dword		serverChallenge;
	dword		csrBuildId;	// buildId of the latest csr client
};

// LoginReply
extern const NetMsg kNetMsg_Csr2Cli_LoginReply;
struct Csr2Cli_LoginReply : Cli2Csr_MsgHeader {
	ENetError	result;
	Uuid		csrId;
	dword		csrFlags;
};


#include <PopPack.h>

