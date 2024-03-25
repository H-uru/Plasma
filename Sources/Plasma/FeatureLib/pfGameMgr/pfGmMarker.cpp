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

#include "pfGmMarker.h"

#include "pnGameMgr/pnGmMarker.h"
#include "pnUtils/pnUtStr.h"
#include "pnUUID/pnUUID.h"

#include "plNetGameLib/plNetGameLib.h"

// ===========================================================================

template<>
void pfGmMarker::Recv(const Srv2Cli_Marker_TemplateCreated& msg)
{
    if (GetHandler()) {
        GetHandler()->OnTemplateCreated(
            ST::string::from_utf16(msg.templateID)
        );
    }
}
template<>
void pfGmMarker::Recv(const Srv2Cli_Marker_TeamAssigned& msg)
{
    if (GetHandler())
        GetHandler()->OnTeamAssigned(msg.teamNumber);
}

template<>
void pfGmMarker::Recv(const Srv2Cli_Marker_GameType& msg)
{
    if (GetHandler())
        GetHandler()->OnGameType(msg.gameType);
}

template<>
void pfGmMarker::Recv(const Srv2Cli_Marker_GameStarted& msg)
{
    if (GetHandler())
        GetHandler()->OnGameStarted();
}

template<>
void pfGmMarker::Recv(const Srv2Cli_Marker_GamePaused& msg)
{
    if (GetHandler())
        GetHandler()->OnGamePaused(msg.timeLeft);
}

template<>
void pfGmMarker::Recv(const Srv2Cli_Marker_GameReset& msg)
{
    if (GetHandler())
        GetHandler()->OnGameReset();
}

template<>
void pfGmMarker::Recv(const Srv2Cli_Marker_GameOver& msg)
{
    if (GetHandler())
        GetHandler()->OnGameOver();
}

template<>
void pfGmMarker::Recv(const Srv2Cli_Marker_TimeLimitChanged& msg)
{
    if (GetHandler())
        GetHandler()->OnTimeLimitChanged(msg.newTimeLimit);
}

template<>
void pfGmMarker::Recv(const Srv2Cli_Marker_GameDeleted& msg)
{
    if (GetHandler()) {
        // Returning failure as true is an odd semantic, IMO.
        // Better to indicate success, in my view.
        GetHandler()->OnGameDeleted(!msg.failed);
    }

    // If we've deleted the game, then nothing we can do
    // at this point is helpful, so go ahead and leave.
    // This should implicitly destroy the game client.
    if (!msg.failed)
        LeaveGame();
}

template<>
void pfGmMarker::Recv(const Srv2Cli_Marker_GameNameChanged& msg)
{
    if (GetHandler()) {
        GetHandler()->OnGameNameChanged(
            ST::string::from_utf16(msg.newName)
        );
    }
}

template<>
void pfGmMarker::Recv(const Srv2Cli_Marker_MarkerAdded& msg)
{
    if (GetHandler()) {
        GetHandler()->OnMarkerAdded(
            msg.x, msg.y, msg.z, msg.markerID,
            ST::string::from_utf16(msg.name),
            ST::string::from_utf16(msg.age)
        );
    }
}

template<>
void pfGmMarker::Recv(const Srv2Cli_Marker_MarkerDeleted& msg)
{
    if (GetHandler())
        GetHandler()->OnMarkerDeleted(msg.markerID);
}

template<>
void pfGmMarker::Recv(const Srv2Cli_Marker_MarkerNameChanged& msg)
{
    if (GetHandler()) {
        GetHandler()->OnMarkerNameChanged(
            msg.markerID,
            ST::string::from_utf16(msg.newName)
        );
    }
}

template<>
void pfGmMarker::Recv(const Srv2Cli_Marker_MarkerCaptured& msg)
{
    if (GetHandler())
        GetHandler()->OnMarkerCaptured(msg.markerID, msg.team);
}

bool pfGmMarker::RecvGameMgrMsg(const GameMsgHeader* msg)
{
    if (pfGameCli::RecvGameMgrMsg(msg))
        return true;

#define DISPATCH(x)                                             \
    case kSrv2Cli_Marker_##x: {                                 \
        const auto& msgFinal = *(const Srv2Cli_Marker_##x*)msg; \
        Recv(msgFinal);                                         \
        return true;                                            \
    };

    switch (msg->messageId) {
    DISPATCH(TemplateCreated)
    DISPATCH(TeamAssigned)
    DISPATCH(GameType)
    DISPATCH(GameStarted)
    DISPATCH(GamePaused)
    DISPATCH(GameReset)
    DISPATCH(GameOver)
    DISPATCH(GameNameChanged)
    DISPATCH(TimeLimitChanged)
    DISPATCH(GameDeleted)
    DISPATCH(MarkerAdded)
    DISPATCH(MarkerDeleted)
    DISPATCH(MarkerNameChanged)
    DISPATCH(MarkerCaptured)
    default:
        return false;
    }

#undef DISPATCH
}

// ===========================================================================

#define MARKER_MSG(x, name)               \
    Cli2Srv_Marker_##x name;              \
    name.messageId = kCli2Srv_Marker_##x; \
    name.transId = 0;                     \
    name.recvGameId = GetGameID();        \
    name.messageBytes = sizeof(name);     //

#define MARKER_SEND(name)                 \
    NetCliGameSendGameMgrMsg(&name);      //

void pfGmMarker::StartGame() const
{
    MARKER_MSG(StartGame, msg);
    MARKER_SEND(msg);
}

void pfGmMarker::PauseGame() const
{
    MARKER_MSG(PauseGame, msg);
    MARKER_SEND(msg);
}

void pfGmMarker::ResetGame() const
{
    MARKER_MSG(ResetGame, msg);
    MARKER_SEND(msg);
}

void pfGmMarker::ChangeGameName(const ST::string& name) const
{
    MARKER_MSG(ChangeGameName, msg);
    StrCopy(msg.gameName, name.to_utf16().data(), std::size(msg.gameName));
    MARKER_SEND(msg);
}

void pfGmMarker::ChangeTimeLimit(uint32_t timeLimit) const
{
    MARKER_MSG(ChangeTimeLimit, msg);
    msg.timeLimit = timeLimit;
    MARKER_SEND(msg);
}

void pfGmMarker::DeleteGame() const
{
    MARKER_MSG(DeleteGame, msg);
    MARKER_SEND(msg);
}

void pfGmMarker::AddMarker(
    double x, double y, double z,
    const ST::string& name, const ST::string& age
) const
{
    MARKER_MSG(AddMarker, msg);
    msg.x = x;
    msg.y = y;
    msg.z = z;
    StrCopy(msg.name, name.to_utf16().data(), std::size(msg.name));
    StrCopy(msg.age, age.to_utf16().data(), std::size(msg.age));
    MARKER_SEND(msg);
}


void pfGmMarker::DeleteMarker(uint32_t markerId) const
{
    MARKER_MSG(DeleteMarker, msg);
    msg.markerID = markerId;
    MARKER_SEND(msg);
}

void pfGmMarker::ChangeMarkerName(
    uint32_t markerId, const ST::string& markerName
) const
{
    MARKER_MSG(ChangeMarkerName, msg);
    msg.markerID = markerId;
    StrCopy(msg.markerName, markerName.to_utf16().data(), std::size(msg.markerName));
    MARKER_SEND(msg);
}

void pfGmMarker::CaptureMarker(uint32_t markerId) const
{
    MARKER_MSG(CaptureMarker, msg);
    msg.markerID = markerId;
    MARKER_SEND(msg);
}

#undef MARKER_SEND
#undef MARKER_MSG

// ===========================================================================

class pfGmMarkerCreateTrans : public pfGameCliCreateTrans
{
    EMarkerGameType fType;
    uint32_t fTimeLimit;
    ST::string fTemplateId;

public:
    pfGmMarkerCreateTrans(
        pfGmMarker* self, pfGmMarkerHandler* handler,
        EMarkerGameType type, uint32_t limit,
        ST::string templateId
    ) : pfGameCliCreateTrans(self, handler), fType(type),
        fTimeLimit(limit), fTemplateId(std::move(templateId))
    {
    }

    void Send() override
    {
        constexpr size_t bufsz =
            sizeof(Cli2Srv_GameMgr_CreateGame) +
            sizeof(Marker_CreateParam) -
            sizeof(Cli2Srv_GameMgr_CreateGame::createData);
        uint8_t msgbuf[bufsz];
        auto& msg = *(Cli2Srv_GameMgr_CreateGame*)msgbuf;
        auto& param = *(Marker_CreateParam*)msg.createData;

        msg.messageId = kCli2Srv_GameMgr_CreateGame;
        msg.gameTypeId = kGameTypeId_Marker;
        msg.createOptions = 0;
        msg.createDataBytes = sizeof(param);
        param.gameType = fType;
        *param.gameName = 0;
        param.timeLimit = fTimeLimit;
        StrCopy(param.templateID, fTemplateId.to_utf16().data(), std::size(param.templateID));
        ISend(&msg, bufsz);
    }
};

void pfGmMarker::Create(
    pfGmMarkerHandler* handler,
    EMarkerGameType gameType,
    uint32_t timeLimit,
    ST::string templateId
)
{
    pfGmMarker* game = new pfGmMarker();
    game->SendTransaction<pfGmMarkerCreateTrans>(
        game,
        handler,
        gameType,
        timeLimit,
        std::move(templateId)
    );
}

// ===========================================================================

bool pfGmMarker::IsSupported()
{
    return NetCliAuthCheckCap(kCapsGameMgrMarker);
}
