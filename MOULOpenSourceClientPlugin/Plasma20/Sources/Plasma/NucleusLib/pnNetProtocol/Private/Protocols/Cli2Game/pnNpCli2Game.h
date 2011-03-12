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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/Protocols/Cli2Game/pnNpCli2Game.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PRIVATE_PROTOCOLS_CLI2GAME_PNNPCLI2GAME_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/Protocols/Cli2Game/pnNpCli2Game.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PRIVATE_PROTOCOLS_CLI2GAME_PNNPCLI2GAME_H


// kNetProtocolCli2Game messages
enum {
    // Global
    kCli2Game_PingRequest,

    // Age
    kCli2Game_JoinAgeRequest,

    // Game
    kCli2Game_PropagateBuffer,
    kCli2Game_GameMgrMsg,
    
    kNumCli2GameMessages
};
COMPILER_ASSERT_HEADER(Cli2Game, kNumCli2GameMessages <= (word)-1);

enum {
    // Global
    kGame2Cli_PingReply,

    // Age
    kGame2Cli_JoinAgeReply,

    // Game
    kGame2Cli_PropagateBuffer,
    kGame2Cli_GameMgrMsg,
    
    kNumGame2CliMessages
};
COMPILER_ASSERT_HEADER(Cli2Game, kNumGame2CliMessages <= (word)-1);


//============================================================================
// BEGIN PACKED DATA STRUCTURES
//============================================================================
#include <PshPack1.h>


/*****************************************************************************
*
*   Cli2Game connect packet
*
***/

struct Cli2Game_ConnData {
    dword   dataBytes;
    Uuid	accountUuid;
    Uuid	ageUuid;
};
struct Cli2Game_Connect {
    AsyncSocketConnectPacket    hdr;
    Cli2Game_ConnData           data;
};


/*****************************************************************************
*
*   Cli2Game message structures
*
***/

// PingRequest
extern const NetMsg kNetMsg_Cli2Game_PingRequest;
struct Cli2Game_PingRequest {
    dword messageId;
    dword pingTimeMs;
};

// JoinAgeRequest
extern const NetMsg kNetMsg_Cli2Game_JoinAgeRequest;
struct Cli2Game_JoinAgeRequest {
    dword       messageId;
    dword       transId;
    dword		ageMcpId;
    Uuid        accountUuid;
    dword       playerInt;
};

// PropagateBuffer
extern const NetMsg kNetMsg_Cli2Game_PropagateBuffer;
struct Cli2Game_PropagateBuffer {
    dword       messageId;
    dword       type;
    dword       bytes;
    byte        buffer[1];  // actually, buffer[bytes]
    // no more fields
};

// GameMgrMsg
extern const NetMsg kNetMsg_Cli2Game_GameMgrMsg;
struct Cli2Game_GameMgrMsg {
	dword		messageId;
	dword		bytes;
	byte		buffer[1];	// actually: buffer[bytes]
};


/*****************************************************************************
*
*   Game2Cli message structures
*
***/

// PingReply
extern const NetMsg kNetMsg_Game2Cli_PingReply;
struct Game2Cli_PingReply{
    dword		messageId;
    dword		pingTimeMs;
};

// JoinAgeReply
extern const NetMsg kNetMsg_Game2Cli_JoinAgeReply;
struct Game2Cli_JoinAgeReply {
    dword       messageId;
    dword       transId;
    ENetError	result;
};

// PropagateBuffer
extern const NetMsg kNetMsg_Game2Cli_PropagateBuffer;
struct Game2Cli_PropagateBuffer {
    dword		messageId;
    dword		type;
    dword		bytes;
    byte		buffer[1];  // actually, buffer[bytes]
    // no more fields
};

// GameMgrMsg
extern const NetMsg kNetMsg_Game2Cli_GameMgrMsg;
struct Game2Cli_GameMgrMsg {
	dword		messageId;
	dword		bytes;
	byte		buffer[1];	// actually: buffer[bytes]
};


//============================================================================
// END PACKED DATA STRUCTURES
//============================================================================
#include <PopPack.h>
