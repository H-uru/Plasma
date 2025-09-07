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

#include "pfGmVarSync.h"
#include "pfGameMgrTrans.h"

#include <type_traits>

#include "pnGameMgr/pnGmVarSync.h"
#include "pnUtils/pnUtStr.h"

#include "plNetGameLib/plNetGameLib.h"

// ===========================================================================

constexpr uint32_t kVarSyncTableID = 255;

// ===========================================================================

template<>
void pfGmVarSync::Recv(const Srv2Cli_VarSync_StringVarChanged& msg)
{
    if (GetHandler()) {
        GetHandler()->OnVarChanged(
            msg.varID,
            ST::string::from_utf16(msg.varValue)
        );
    }
}

template<>
void pfGmVarSync::Recv(const Srv2Cli_VarSync_NumericVarChanged& msg)
{
    if (GetHandler())
        GetHandler()->OnVarChanged(msg.varID, msg.varValue);
}

template<>
void pfGmVarSync::Recv(const Srv2Cli_VarSync_AllVarsSent&)
{
    if (GetHandler())
        GetHandler()->OnAllVarsSent();
}

template<>
void pfGmVarSync::Recv(const Srv2Cli_VarSync_StringVarCreated& msg)
{
    if (GetHandler()) {
        GetHandler()->OnVarCreated(
            ST::string::from_utf16(msg.varName),
            msg.varID,
            ST::string::from_utf16(msg.varValue)
        );
    }
}

template<>
void pfGmVarSync::Recv(const Srv2Cli_VarSync_NumericVarCreated& msg)
{
    if (GetHandler()) {
        GetHandler()->OnVarCreated(
            ST::string::from_utf16(msg.varName),
            msg.varID,
            msg.varValue
        );
    }
}

bool pfGmVarSync::RecvGameMgrMsg(const GameMsgHeader* msg)
{
    if (pfGameCli::RecvGameMgrMsg(msg))
        return true;

#define DISPATCH(x)                                                 \
    case kSrv2Cli_VarSync_##x: {                                    \
        const auto& msgFinal = *(const Srv2Cli_VarSync_##x*)msg;    \
        Recv(msgFinal);                                             \
        return true;                                                \
    };

    switch (msg->messageId) {
        DISPATCH(StringVarChanged);
        DISPATCH(NumericVarChanged);
        DISPATCH(AllVarsSent);
        DISPATCH(StringVarCreated);
        DISPATCH(NumericVarCreated);
        default:
            return false;
    }

#undef DISPATCH
}

// ===========================================================================

#define VARSYNC_MSG(x, name)                  \
    Cli2Srv_VarSync_##x name;                 \
    name.messageId = kCli2Srv_VarSync_##x;    \
    name.transId = 0;                         \
    name.recvGameId = GetGameID();            \
    name.messageBytes = sizeof(name);         //

#define VARSYNC_SEND(name)                    \
    NetCliGameSendGameMgrMsg(&name);          //

void pfGmVarSync::SetVariable(
    uint32_t varID,
    const pfGmVarSyncValue& varValue
) const
{
    std::visit(
        [this, varID](auto&& value) {
            using _VarT = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<_VarT, double>) {
                VARSYNC_MSG(SetNumericVar, msg)
                msg.varID = varID;
                msg.varValue = value;
                VARSYNC_SEND(msg)
            } else if constexpr (std::is_same_v<_VarT, ST::string>) {
                VARSYNC_MSG(SetStringVar, msg)
                msg.varID = varID;
                StrCopy(msg.varValue, value.to_utf16().data(), std::size(msg.varValue));
                VARSYNC_SEND(msg)
            } else {
                static_assert(
                    std::is_same_v<_VarT, double> || std::is_same_v<_VarT, ST::string>,
                    "Non-exhaustive visitor"
                );
            }
        }, varValue
    );
}

void pfGmVarSync::CreateVariable(
    const ST::string& varName,
    const pfGmVarSyncValue& varValue
) const
{
    std::visit(
        [this, &varName](auto&& value) {
            using _VarT = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<_VarT, double>) {
                VARSYNC_MSG(CreateNumericVar, msg)
                StrCopy(msg.varName, varName.to_utf16().data(), std::size(msg.varName));
                msg.varValue = value;
                VARSYNC_SEND(msg)
            } else if constexpr (std::is_same_v<_VarT, ST::string>) {
                VARSYNC_MSG(CreateStringVar, msg)
                StrCopy(msg.varName, varName.to_utf16().data(), std::size(msg.varName));
                StrCopy(msg.varValue, value.to_utf16().data(), std::size(msg.varValue));
                VARSYNC_SEND(msg)
            } else {
                static_assert(
                    std::is_same_v<_VarT, double> || std::is_same_v<_VarT, ST::string>,
                    "Non-exhaustive visitor"
                );
            }
        }, varValue
    );
}

#undef VARSYNC_SEND
#undef VARSYNC_MSG

// ===========================================================================

class pfGmVarSyncCreateTrans : public pfGameCliCreateTrans
{
public:
    pfGmVarSyncCreateTrans(pfGmVarSync* self, pfGmVarSyncHandler* handler)
        : pfGameCliCreateTrans(self, handler)
    {
    }

    void Send() override
    {
        VarSync_CreateParam param{};
        constexpr size_t bufsz = sizeof(Cli2Srv_GameMgr_JoinGame) + sizeof(param) - sizeof(Cli2Srv_GameMgr_CreateGame::createData);
        uint8_t msgbuf[bufsz];

        Cli2Srv_GameMgr_JoinGame& msg = *reinterpret_cast<Cli2Srv_GameMgr_JoinGame*>(msgbuf);
        msg.messageId = kCli2Srv_GameMgr_JoinGame;
        msg.newGameId = kVarSyncTableID;
        msg.createOptions = kGameJoinCommon;
        msg.gameTypeId = kGameTypeId_VarSync;
        msg.createDataBytes = sizeof(param);
        memcpy(msg.createData, &param, sizeof(param));
        ISend(&msg, bufsz);
    }
};

void pfGmVarSync::Join(pfGmVarSyncHandler* handler)
{
    pfGmVarSync* game = new pfGmVarSync();
    game->SendTransaction<pfGmVarSyncCreateTrans>(
        game,
        handler
    );
}

// ===========================================================================

bool pfGmVarSync::IsSupported()
{
    return NetCliAuthCheckCap(kCapsGameMgrVarSync);
}
