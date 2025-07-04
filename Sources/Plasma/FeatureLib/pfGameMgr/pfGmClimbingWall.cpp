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

#include "pfGmClimbingWall.h"
#include "pfGameMgrTrans.h"

#include "pnGameMgr/pnGmClimbingWall.h"

#include "plNetGameLib/plNetGameLib.h"

// ===========================================================================

template<size_t _Size>
static pfGmClimbingWallBlockers ITransformBlockersFromMsg(const int32_t (&blockers)[_Size])
{
    pfGmClimbingWallBlockers ret;
    for (auto blocker : blockers) {
        if (blocker != kClimbingWallNoBlocker)
            ret.insert(blocker);
    }
    return ret;
}

// ===========================================================================

template<>
void pfGmClimbingWall::Recv(const Srv2Cli_ClimbingWall_NumBlockersChanged& msg)
{
    if (GetHandler())
        GetHandler()->OnNumBlockersChanged(msg.newBlockerCount, msg.localOnly);
}

template <>
void pfGmClimbingWall::Recv(const Srv2Cli_ClimbingWall_Ready& msg)
{
    if (GetHandler()) {
        GetHandler()->OnReady(
            (EClimbingWallReadyType)msg.readyType,
            msg.team1Ready,
            msg.team2Ready,
            msg.localOnly
        );
    }
}

template <>
void pfGmClimbingWall::Recv(const Srv2Cli_ClimbingWall_BlockersChanged& msg)
{
    if (GetHandler()) {
        GetHandler()->OnBlockersChanged(
            msg.teamNumber,
            ITransformBlockersFromMsg(msg.blockersSet),
            msg.localOnly
        );
    }
}

template <>
void pfGmClimbingWall::Recv(const Srv2Cli_ClimbingWall_PlayerEntered& msg)
{
    if (GetHandler())
        GetHandler()->OnPlayerEntered();
}

template <>
void pfGmClimbingWall::Recv(const Srv2Cli_ClimbingWall_SuitMachineLocked& msg)
{
    if (GetHandler()) {
        GetHandler()->OnSuitMachineLocked(
            msg.team1MachineLocked,
            msg.team2MachineLocked,
            msg.localOnly
        );
    }
}

template <>
void pfGmClimbingWall::Recv(const Srv2Cli_ClimbingWall_GameOver& msg)
{
    if (GetHandler()) {
        GetHandler()->OnGameOver(
            msg.teamWon,
            ITransformBlockersFromMsg(msg.team1Blockers),
            ITransformBlockersFromMsg(msg.team2Blockers),
            msg.localOnly
        );
    }
}

bool pfGmClimbingWall::RecvGameMgrMsg(const GameMsgHeader* msg)
{
    if (pfGameCli::RecvGameMgrMsg(msg))
        return true;

#define DISPATCH(x)                                                   \
    case kSrv2Cli_ClimbingWall_##x: {                                 \
        const auto& msgFinal = *(const Srv2Cli_ClimbingWall_##x*)msg; \
        Recv(msgFinal);                                               \
        return true;                                                  \
    };                                                                //

    switch (msg->messageId) {
        DISPATCH(NumBlockersChanged)
        DISPATCH(Ready)
        DISPATCH(BlockersChanged)
        DISPATCH(PlayerEntered)
        DISPATCH(SuitMachineLocked)
        DISPATCH(GameOver)
        default:
            return false;
    }

#undef DISPATCH
}

// ===========================================================================

#define CLIMBINGWALL_MSG(x, name)               \
    Cli2Srv_ClimbingWall_##x name;              \
    name.messageId = kCli2Srv_ClimbingWall_##x; \
    name.transId = 0;                           \
    name.recvGameId = GetGameID();              \
    name.messageBytes = sizeof(name);           //

#define CLIMBINGWALL_SEND(name)                 \
    NetCliGameSendGameMgrMsg(&name);            //

void pfGmClimbingWall::ChangeNumBlockers(int32_t amountToAdjust) const
{
    CLIMBINGWALL_MSG(ChangeNumBlockers, msg);
    msg.amountToAdjust = amountToAdjust;
    CLIMBINGWALL_SEND(msg);
}

void pfGmClimbingWall::Ready(
    EClimbingWallReadyType readyType,
    uint8_t teamNumber
) const
{
    CLIMBINGWALL_MSG(Ready, msg);
    msg.readyType = (uint8_t)readyType;
    msg.teamNumber = teamNumber;
    CLIMBINGWALL_SEND(msg);
}

void pfGmClimbingWall::ChangeBlocker(
    uint8_t teamNumber,
    uint8_t blockerNumber,
    bool    added
) const
{
    CLIMBINGWALL_MSG(BlockerChanged, msg);
    msg.teamNumber = teamNumber;
    msg.blockerNumber = blockerNumber;
    msg.added = added;
    CLIMBINGWALL_SEND(msg);
}

void pfGmClimbingWall::Reset() const
{
    CLIMBINGWALL_MSG(Reset, msg);
    CLIMBINGWALL_SEND(msg);
}

void pfGmClimbingWall::EnterPlayer(uint8_t teamNumber) const
{
    CLIMBINGWALL_MSG(PlayerEntered, msg);
    msg.teamNumber = teamNumber;
    CLIMBINGWALL_SEND(msg);
}

void pfGmClimbingWall::FinishGame() const
{
    CLIMBINGWALL_MSG(FinishedGame, msg);
    CLIMBINGWALL_SEND(msg);
}

void pfGmClimbingWall::Panic() const
{
    CLIMBINGWALL_MSG(Panic, msg);
    CLIMBINGWALL_SEND(msg);
}

#undef CLIMBINGWALL_SEND
#undef CLIMBINGWALL_MSG

// ===========================================================================

class pfGmClimbingWallCreateTrans : public pfGameCliCreateTrans
{
    uint32_t fTableID;

public:
    pfGmClimbingWallCreateTrans(pfGmClimbingWall* self, pfGmClimbingWallHandler* handler, uint32_t tableID)
        : pfGameCliCreateTrans(self, handler), fTableID(tableID)
    {
    }

    void Send() override
    {
        ClimbingWall_CreateParam param{};
        constexpr size_t bufsz = sizeof(Cli2Srv_GameMgr_JoinGame) + sizeof(param) - sizeof(Cli2Srv_GameMgr_CreateGame::createData);
        uint8_t msgbuf[bufsz];

        Cli2Srv_GameMgr_JoinGame& msg = *reinterpret_cast<Cli2Srv_GameMgr_JoinGame*>(msgbuf);
        msg.messageId = kCli2Srv_GameMgr_JoinGame;
        msg.newGameId = fTableID;
        msg.createOptions = kGameJoinCommon;
        msg.gameTypeId = kGameTypeId_ClimbingWall;
        msg.createDataBytes = sizeof(param);
        memcpy(msg.createData, &param, sizeof(param));
        ISend(&msg, bufsz);
    }
};

void pfGmClimbingWall::Join(pfGmClimbingWallHandler* handler, uint32_t tableID)
{
    pfGmClimbingWall* game = new pfGmClimbingWall();
    game->SendTransaction<pfGmClimbingWallCreateTrans>(
        game,
        handler,
        tableID
    );
}

// ===========================================================================

bool pfGmClimbingWall::IsSupported()
{
    return NetCliAuthCheckCap(kCapsGameMgrClimbingWall);
}
