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

#include "pfGameCli.h"

#include "pnGameMgr/pnGameMgr.h"

#include "plNetClientComm/plNetClientComm.h"
#include "plNetGameLib/plNetGameLib.h"

// ===========================================================================

pfGameHandler::~pfGameHandler()
{
    if (fCli)
        fCli->fHandler = nullptr;
}

// ===========================================================================

pfGameCliCreateTrans::pfGameCliCreateTrans(pfGameCli* cli, pfGameHandler* handler)
    : fCli(cli), fResult(EGameJoinError::kGameJoinPending), fGameId()
{
    fCli->fHandler = handler;
}

pfGameCliCreateTrans::~pfGameCliCreateTrans()
{
    // If someone is holding me, they better ref me.
    fCli->UnRef();
}

// ===========================================================================

void pfGameCliCreateTrans::Recv(const GameMsgHeader* msg)
{
    const auto& reply = *(const Srv2Cli_GameMgr_GameInstance*)msg;
    fResult = reply.result;
    fGameId = reply.newGameId;
}

void pfGameCliCreateTrans::Post()
{
    if (fCli->GetHandler()) {
        fCli->GetHandler()->fCli = fCli;
        fCli->GetHandler()->OnGameCliInstance(fResult);
    }

    // Only set the GameId if we successfully got a remote game client.
    // This will cause the local game manager to hold a reference to the
    // client until we leave the game. If we do not get an instance, the game
    // client will be implicitly destroyed when this function exits because
    // the game manager unrefs us, and we own the only reference to the local
    // game client.
    if (fResult == EGameJoinError::kGameJoinSuccess)
        fCli->SetGameId(fGameId);
}

// ===========================================================================

pfGameCli::~pfGameCli()
{
    if (fHandler) {
        fHandler->fCli = nullptr;
        fHandler->OnGameCliDelete();
    }
}

// ===========================================================================

template<>
void pfGameCli::Recv(const Srv2Cli_Game_PlayerJoined& msg)
{
    if (NetCommGetPlayer() && NetCommGetPlayer()->playerInt == msg.playerId)
        fFlags |= kFlagAmJoined;
    if (fHandler)
        fHandler->OnPlayerJoined(msg.playerId);
}

template<>
void pfGameCli::Recv(const Srv2Cli_Game_PlayerLeft& msg)
{
    bool iAmLeaving = NetCommGetPlayer() && NetCommGetPlayer()->playerInt == msg.playerId;
    if (iAmLeaving)
        fFlags &= ~kFlagAmJoined;
    if (fHandler)
        fHandler->OnPlayerLeft(msg.playerId);
    if (iAmLeaving) {
        pfGameMgr* mgr = pfGameMgr::GetInstance();
        auto cliIt = mgr->fGameClis.find(GetGameID());
        if (cliIt != mgr->fGameClis.end())
            mgr->fGameClis.erase(cliIt);
        else
            hsAssert(0, "I seem to be leaving an unknown game?");
    }
}

template<>
void pfGameCli::Recv(const Srv2Cli_Game_InviteFailed& msg)
{
    hsAssert(0, "GameMgr invite facilities are assumed to be unused.");
}

template<>
void pfGameCli::Recv(const Srv2Cli_Game_OwnerChange& msg)
{
    fOwnerID = msg.ownerId;
    if (fHandler)
        fHandler->OnOwnerChanged(fOwnerID);
}

bool pfGameCli::RecvGameMgrMsg(const GameMsgHeader* msg)
{
#define DISPATCH(x)                                           \
    case kSrv2Cli_Game_##x: {                                 \
        const auto& msgFinal = *(const Srv2Cli_Game_##x*)msg; \
        Recv(msgFinal);                                       \
        return true;                                          \
    };                                                        \

    switch (msg->messageId) {
    DISPATCH(PlayerJoined)
    DISPATCH(PlayerLeft)
    DISPATCH(InviteFailed)
    DISPATCH(OwnerChange)
    default:
        return false;
    }

#undef DISPATCH
}

// ===========================================================================

#define GAMECLI_MSG(x, name)              \
    Cli2Srv_Game_##x name;                \
    name.messageId = kCli2Srv_Game_##x;   \
    name.transId = 0;                     \
    name.recvGameId = GetGameID();        \
    name.messageBytes = sizeof(name);     //

#define GAMECLI_SEND(name)                \
    NetCliGameSendGameMgrMsg(&name);      //

#define GAMESRV_MSG(x, name)              \
    Srv2Cli_Game_##x name;                \
    name.messageId = kSrv2Cli_Game_##x;   \
    name.transId = 0;                     \
    name.recvGameId = GetGameID();        \
    name.messageBytes = sizeof(name);     //

#define GAMESRV_SEND(name)                \
    Recv(name);                           //

void pfGameCli::LeaveGame()
{
    if (!(fFlags & kFlagAmJoined))
        return;

    {
        GAMECLI_MSG(LeaveGame, msg)
        GAMECLI_SEND(msg)
    }

    // The server doesn't echo this back to us, so we'll fake it.
    {
        GAMESRV_MSG(PlayerLeft, msg);
        msg.playerId = NetCommGetPlayer() ? NetCommGetPlayer()->playerInt : 0;
        GAMESRV_SEND(msg);
    }
}

#undef GAMESRV_SEND
#undef GAMESRV_MSG
#undef GAMECLI_SEND
#undef GAMECLI_MSG

// ===========================================================================

bool pfGameCli::IsLocallyOwned() const
{
    return (NetCommGetPlayer() && NetCommGetPlayer()->playerInt == fOwnerID);
}
