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

#ifndef _pnGameMgr_h_
#define _pnGameMgr_h_

#include "HeadSpin.h"

#include "pnGameMgrConst.h"

#include "pnUUID/pnUUID.h"

/*****************************************************************************
*
*   GameMgr
*
***/

constexpr uint32_t kGameMgrGlobalGameIdFlag = !((uint32_t)-1 - 1);  // 0x10000000

//============================================================================
// Game create/join options
//============================================================================
/*
    If set  :   Anyone may join; no invite necessary.
    Not set :   Only players with invites may join.
*/
constexpr uint32_t kGameCreatePublic = 1 << 0;
/*
    If set  :   Anyone may invite others to play.
    Not set :   Only the game owner may send invites.
*/
constexpr uint32_t kGameCreateOpen = 1 << 1;
/*
    If set  :   Player joins or creates the "common" instance of the game. In
                this case, the 'newGameId' field is not meaningful. If the
                common instance doesn't exist, it'll be created on-the-fly and
                the player will receive a GameCreated message as well as the
                normal GameJoined. This allows the game to be initialized once
                when first instanced.
    Not set :   A game with the specified gameId must exist on the server.
                Depending on the options set during the game's creation, the
                player may need to have been sent an invite. Also, the game may
                not be in a state where it allows new players to join. Player
                receives a GameJoined reply in any case. Inspect the 'result'
                field to see whether the join was successful.
*/
constexpr uint32_t kGameJoinCommon = 1 << 2;
/*
*/
constexpr uint32_t kGameJoinObserver = 1 << 3;


//============================================================================
// GameMgr Network message ids
//============================================================================
enum
{
    kCli2Srv_GameMgr_CreateGame,
    kCli2Srv_GameMgr_JoinGame,
};
enum
{
    kSrv2Cli_GameMgr_GameInstance,
    kSrv2Cli_GameMgr_InviteReceived,
    kSrv2Cli_GameMgr_InviteRevoked,
};

//============================================================================
// GameCli/Srv Network message ids
//============================================================================
enum
{
    kCli2Srv_Game_LeaveGame,
    kCli2Srv_Game_Invite,
    kCli2Srv_Game_Uninvite,
    // Cli2Srv msgIds for specific games must begin with this value.
    kCli2Srv_NumGameMsgIds
};
enum
{
    kSrv2Cli_Game_PlayerJoined,
    kSrv2Cli_Game_PlayerLeft,
    kSrv2Cli_Game_InviteFailed,
    kSrv2Cli_Game_OwnerChange,
    // Srv2Cli msgIds for specific games must begin with this value.
    kSrv2Cli_NumGameMsgIds
};


//============================================================================
// Begin networked data scructures
#pragma pack(push, 1)
//============================================================================

    struct GameMsgHeader
    {
        uint32_t       messageId;
        uint32_t       transId;
        uint32_t       recvGameId; // 0 --> GameMgr, non-zero --> GameSrv
        uint32_t       messageBytes;
    };

    //========================================================================
    // GameMgr message structures
    //========================================================================

    // Cli2Srv
    struct Cli2Srv_GameMgr_CreateGame : GameMsgHeader
    {
        plUUID                 gameTypeId;
        uint32_t               createOptions;
        uint32_t               createDataBytes;
        uint8_t                createData[1];  // [createDataBytes]
    };

    struct Cli2Srv_GameMgr_JoinGame : GameMsgHeader
    {
        // Field ordering here is vitally important, see pfGameMgr::JoinGame for explanation
        uint32_t               newGameId;
        uint32_t               createOptions;
        plUUID                 gameTypeId;
        uint32_t               createDataBytes;
        uint8_t                createData[1];  // [createDataBytes]
    };

    // Srv2Cli
    struct Srv2Cli_GameMgr_GameInstance : GameMsgHeader
    {
        EGameJoinError         result;
        uint32_t               ownerId;
        plUUID                 gameTypeId;
        uint32_t               newGameId;
    };

    struct Srv2Cli_GameMgr_InviteReceived : GameMsgHeader
    {
        uint32_t               inviterId;
        plUUID                 gameTypeId;
        uint32_t               newGameId;
    };

    struct Srv2Cli_GameMgr_InviteRevoked : GameMsgHeader
    {
        uint32_t               inviterId;
        plUUID                 gameTypeId;
        uint32_t               newGameId;
    };


    //========================================================================
    // GameCli/Srv message structures
    //========================================================================

    // Cli2Srv
    struct Cli2Srv_Game_LeaveGame : GameMsgHeader
    {
    };

    struct Cli2Srv_Game_Invite : GameMsgHeader
    {
        uint32_t         playerId;
    };

    struct Cli2Srv_Game_Uninvite : GameMsgHeader
    {
        uint32_t         playerId;
    };

    // Srv2Cli
    struct Srv2Cli_Game_PlayerJoined : GameMsgHeader
    {
        uint32_t         playerId;
    };

    struct Srv2Cli_Game_PlayerLeft : GameMsgHeader
    {
        uint32_t         playerId;
    };

    struct Srv2Cli_Game_InviteFailed : GameMsgHeader
    {
        uint32_t         inviteeId;
        uint32_t         operationId;
        EGameInviteError error;
    };

    struct Srv2Cli_Game_OwnerChange : GameMsgHeader
    {
        uint32_t         ownerId;
    };


//============================================================================
// End networked data structures
#pragma pack(pop)
//============================================================================

#endif
