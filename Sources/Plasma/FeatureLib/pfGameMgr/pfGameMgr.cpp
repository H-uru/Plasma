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

#include "pfGameMgr.h"

#include <string_theory/format>

#include "pfGameCli.h"
#include "pfGameMgrTrans.h"

#include "pnGameMgr/pnGameMgr.h"

#include "plNetClientComm/plNetClientComm.h"
#include "plNetGameLib/plNetGameLib.h"

// ===========================================================================

pfGameMgr::pfGameMgr()
    : fTransId(1)
{
    NetCliGameSetRecvGameMgrMsgHandler(RecvGameMgrMsg);
}

pfGameMgr::~pfGameMgr()
{
    NetCliGameSetRecvGameMgrMsgHandler(nullptr);
}

// ===========================================================================

template<>
void pfGameMgr::Recv(const Srv2Cli_GameMgr_GameInstance& msg)
{
    // This can happen if we accidentally send a message to
    // the wrong gameId. Cyan's server sends this back
    // with the error code set.
    hsAssert(0, "Some kind of unhandled GameMgr error.");
}

template<>
void pfGameMgr::Recv(const Srv2Cli_Game_PlayerLeft& msg)
{
    // This message will, as far as I know, never be sent down from the server
    // like this. What's actually happening is plNetGameLib is sending us
    // a message saying, "hey, the connection just got blown up," which,
    // AFAICT, is an implicit 'PlayerLeft' sent on our behalf to everyone
    // still in the Age. Cyan's old code didn't seem to handle this, and
    // my attempts to figure out the pfGameCli ownership force me to try to
    // deal with it. Sad.
    hsAssert(msg.playerId == 0, "Potentially bogus PlayerLeft broadcast?");

    uint32_t playerId = msg.playerId;
    if (playerId == 0) {
        if (const NetCommPlayer* player = NetCommGetPlayer())
            playerId = player->playerInt;
    }

    // Tell the handlers that we left, which should (hopefully) encourage them
    // to release any strong references they hold to the game clients. Ideally,
    // only Python code is holding strong references.
    for (auto [gameId, cli] : fGameClis) {
        if (!(cli->fFlags & pfGameCli::kFlagAmJoined))
            continue;
        cli->fFlags &= ~pfGameCli::kFlagAmJoined;
        if (cli->GetHandler())
            cli->GetHandler()->OnPlayerLeft(playerId);
    }

    // Now clearing out all of the clients should trigger their deletion.
    fGameClis.clear();
}

void pfGameMgr::RecvGameMgrMsg(GameMsgHeader* msg)
{
    pfGameMgr* mgr = pfGameMgr::GetInstance();

    // First, try to do a transaction thingy.
    auto transIt = mgr->fTransactions.find(msg->transId);
    if (transIt != mgr->fTransactions.end()) {
        // This is something of an anti-pattern, but I adopted the logic
        // from the Ngl::Trans stuff in plNetGameLib, for consistency.
        transIt->second->Recv(msg);
        transIt->second->Post();
        mgr->fTransactions.erase(transIt);
        return;
    }

    // Second, by gameId.
    auto gameIt = mgr->fGameClis.find(msg->recvGameId);
    if (gameIt != mgr->fGameClis.end()) {
        gameIt->second->RecvGameMgrMsg(msg);
        return;
    } else if (msg->recvGameId) {
        hsAssert(0, ST::format("Got a message for an unknown game client {}", msg->recvGameId).c_str());
        return;
    }

#define DISPATCH(x)                                             \
    case k##Srv2Cli_##x: {                                      \
        const auto& msgFinal = *(const Srv2Cli_##x*)msg;        \
        mgr->Recv(msgFinal);                                    \
        break;                                                  \
    };

    // Lastly, we receive messages directed directly to us (the GameMgr).
    // In the old code, this could be a GameInstance, InviteReceived,
    // or an InviteRevoked. GameInstances always come in response to transactions,
    // so there is no need to handle this case. Invites, AFAIK, were never used and
    // due to the closed nature of Cyan's server code, it's unlikely anyone
    // will start broadcasting them to us at this point in life. So,
    // we should really never get here. Note that if we ever do find that
    // invites are actually in use, we'll need to add in, uh, a way to
    // create game instances from the UUID. Seems pointless right now.
    // Anyway, we still want to handle a few things for our own sanity.
    switch (msg->messageId) {
    DISPATCH(GameMgr_GameInstance)
    DISPATCH(Game_PlayerLeft) // special case
    default:
        hsAssert(0, "Hmm... An unexpected GameMgr message directed to the game manager itself.");
    }

#undef DISPATCH
}

// ===========================================================================

static auto s_GameMgr = std::make_unique<pfGameMgr>();

pfGameMgr* pfGameMgr::GetInstance()
{
    return s_GameMgr.get();
}

void pfGameMgr::Shutdown()
{
    s_GameMgr.reset();
}
