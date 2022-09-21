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

#include "pfGmBlueSpiral.h"
#include "pfGameMgrTrans.h"

#include <algorithm>

#include "pnGameMgr/pnGmBlueSpiral.h"

#include "plNetGameLib/plNetGameLib.h"

// ===========================================================================

template<>
void pfGmBlueSpiral::Recv(const Srv2Cli_BlueSpiral_ClothOrder& msg)
{
    if (GetHandler()) {
        std::array<uint8_t, 7> order;
        std::copy_n(std::cbegin(msg.order), std::size(order), order.begin());
        GetHandler()->OnClothOrder(order);
    }
}

template<>
void pfGmBlueSpiral::Recv(const Srv2Cli_BlueSpiral_SuccessfulHit& msg)
{
    if (GetHandler())
        GetHandler()->OnClothHit();
}

template<>
void pfGmBlueSpiral::Recv(const Srv2Cli_BlueSpiral_GameWon& msg)
{
    if (GetHandler())
        GetHandler()->OnGameWon();
}

template<>
void pfGmBlueSpiral::Recv(const Srv2Cli_BlueSpiral_GameOver& msg)
{
    if (GetHandler())
        GetHandler()->OnGameOver();
}

template<>
void pfGmBlueSpiral::Recv(const Srv2Cli_BlueSpiral_GameStarted& msg)
{
    if (GetHandler())
        GetHandler()->OnGameStarted(msg.startSpin);
}

bool pfGmBlueSpiral::RecvGameMgrMsg(const GameMsgHeader* msg)
{
    if (pfGameCli::RecvGameMgrMsg(msg))
        return true;

#define DISPATCH(x)                                                 \
    case kSrv2Cli_BlueSpiral_##x: {                                 \
        const auto& msgFinal = *(const Srv2Cli_BlueSpiral_##x*)msg; \
        Recv(msgFinal);                                             \
        return true;                                                \
    };

    switch (msg->messageId) {
    DISPATCH(ClothOrder);
    DISPATCH(SuccessfulHit);
    DISPATCH(GameWon);
    DISPATCH(GameOver);
    DISPATCH(GameStarted);
    default:
        return false;
    }

#undef DISPATCH
}

// ===========================================================================

#define BLUESPIRAL_MSG(x, name)               \
    Cli2Srv_BlueSpiral_##x name;              \
    name.messageId = kCli2Srv_BlueSpiral_##x; \
    name.transId = 0;                         \
    name.recvGameId = GetGameID();            \
    name.messageBytes = sizeof(name);         //

#define BLUESPIRAL_SEND(name)                 \
    NetCliGameSendGameMgrMsg(&name);          //

void pfGmBlueSpiral::StartGame() const
{
    BLUESPIRAL_MSG(StartGame, msg);
    BLUESPIRAL_SEND(msg);
}

void pfGmBlueSpiral::HitCloth(uint8_t cloth) const
{
    hsAssert(cloth < 7, "Cloth out of range.");
    BLUESPIRAL_MSG(HitCloth, msg);
    msg.clothNum = cloth;
    BLUESPIRAL_SEND(msg);
}

#undef BLUESPIRAL_SEND
#undef BLUESPIRAL_MSG

// ===========================================================================

class pfGmBlueSpiralCreateTrans : public pfGameCliCreateTrans
{
    uint32_t fTableID;

public:
    pfGmBlueSpiralCreateTrans(pfGmBlueSpiral* self, pfGmBlueSpiralHandler* handler, uint32_t tableID)
        : pfGameCliCreateTrans(self, handler), fTableID(tableID)
    {
    }

    void Send() override
    {
        BlueSpiral_CreateParam param{};
        constexpr size_t bufsz = sizeof(Cli2Srv_GameMgr_JoinGame) + sizeof(param) - sizeof(Cli2Srv_GameMgr_CreateGame::createData);
        uint8_t msgbuf[bufsz];

        Cli2Srv_GameMgr_JoinGame& msg = *reinterpret_cast<Cli2Srv_GameMgr_JoinGame*>(msgbuf);
        msg.messageId = kCli2Srv_GameMgr_JoinGame;
        msg.newGameId = fTableID;
        msg.createOptions = kGameJoinCommon;
        msg.gameTypeId = kGameTypeId_BlueSpiral;
        msg.createDataBytes = sizeof(param);
        memcpy(msg.createData, &param, sizeof(param));
        ISend(&msg, bufsz);
    }
};

void pfGmBlueSpiral::Join(pfGmBlueSpiralHandler* handler, uint32_t tableID)
{
    pfGmBlueSpiral* game = new pfGmBlueSpiral();
    game->SendTransaction<pfGmBlueSpiralCreateTrans>(
        game,
        handler,
        tableID
    );
}

// ===========================================================================

bool pfGmBlueSpiral::IsSupported()
{
    return NetCliAuthCheckCap(kCapsGameMgrBlueSpiral);
}
