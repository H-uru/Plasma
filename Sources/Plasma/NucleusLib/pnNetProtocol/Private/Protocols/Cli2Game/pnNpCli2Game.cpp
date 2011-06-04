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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/Protocols/Cli2Game/pnNpCli2Game.cpp
*   
***/

#define USES_PROTOCOL_CLI2GAME
#include "../../../Pch.h"
#pragma hdrstop


namespace Cli2Game {
/*****************************************************************************
*
*   Cli2Game message definitions
*
***/

static const NetMsgField kPingRequestFields[] = {
    kNetMsgFieldTimeMs,         // pingTimeMs
};

static const NetMsgField kJoinAgeRequestFields[] = {
    kNetMsgFieldTransId,						// transId
    NET_MSG_FIELD_DWORD(),						// ageMcpId
    kNetMsgFieldUuid,							// accountUuid
    NET_MSG_FIELD_DWORD(),						// playerInt
};

static const NetMsgField kPropagateBufferFields[] = {
    NET_MSG_FIELD_DWORD(),						// type
    NET_MSG_FIELD_VAR_COUNT(1, 1024 * 1024),	// bytes
    NET_MSG_FIELD_VAR_PTR(),					// buffer
};

static const NetMsgField kGameMgrMsgFields[] = {
    NET_MSG_FIELD_VAR_COUNT(1, 1024 * 1024),	// bytes
    NET_MSG_FIELD_VAR_PTR(),					// buffer
};


/*****************************************************************************
*
*   Game2Cli message fields
*
***/

static const NetMsgField kPingReplyFields[] = {
    kNetMsgFieldTimeMs,         // pingTimeMs
};

static const NetMsgField kJoinAgeReplyFields[] = {
    kNetMsgFieldTransId,        // transId
    kNetMsgFieldENetError,      // result
};

} using namespace Cli2Game;


/*****************************************************************************
*
*   Exported data
*
***/

// Cli2Game
const NetMsg kNetMsg_Cli2Game_PingRequest           = NET_MSG(kCli2Game_PingRequest,            kPingRequestFields);
const NetMsg kNetMsg_Cli2Game_JoinAgeRequest        = NET_MSG(kCli2Game_JoinAgeRequest,         kJoinAgeRequestFields);
const NetMsg kNetMsg_Cli2Game_PropagateBuffer       = NET_MSG(kCli2Game_PropagateBuffer,        kPropagateBufferFields);
const NetMsg kNetMsg_Cli2Game_GameMgrMsg			= NET_MSG(kCli2Game_GameMgrMsg,				kGameMgrMsgFields);

// Game2Cli
const NetMsg kNetMsg_Game2Cli_PingReply             = NET_MSG(kGame2Cli_PingReply,              kPingReplyFields);
const NetMsg kNetMsg_Game2Cli_JoinAgeReply          = NET_MSG(kGame2Cli_JoinAgeReply,           kJoinAgeReplyFields);
const NetMsg kNetMsg_Game2Cli_PropagateBuffer       = NET_MSG(kGame2Cli_PropagateBuffer,        kPropagateBufferFields);
const NetMsg kNetMsg_Game2Cli_GameMgrMsg			= NET_MSG(kGame2Cli_GameMgrMsg,				kGameMgrMsgFields);
