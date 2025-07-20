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

#include "plPythonConvert.h"
#include "pyGameHandler.h"
#include "pyGmClimbingWall.h"

#include "pfGameMgr/pfGmClimbingWall.h"


// ===========================================================================

void pyGmClimbingWall::ChangeNumBlockers(int32_t amountToAdjust) const
{
    GetGameCli()->ChangeNumBlockers(amountToAdjust);
}

void pyGmClimbingWall::Ready(
    EClimbingWallReadyType readyType,
    uint8_t teamNumber
) const
{
    GetGameCli()->Ready(readyType, teamNumber);
}

void pyGmClimbingWall::ChangeBlocker(
    uint8_t teamNumber,
    uint8_t blockerNumber,
    bool added
) const
{
    GetGameCli()->ChangeBlocker(teamNumber, blockerNumber, added);
}

void pyGmClimbingWall::Reset() const
{
    GetGameCli()->Reset();
}

void pyGmClimbingWall::EnterPlayer(uint8_t teamNumber) const
{
    GetGameCli()->EnterPlayer(teamNumber);
}

void pyGmClimbingWall::FinishGame() const
{
    GetGameCli()->FinishGame();
}

void pyGmClimbingWall::Panic() const
{
    GetGameCli()->Panic();
}

// ===========================================================================

namespace plPython
{
    template <>
    inline PyObject* ConvertFrom(EClimbingWallReadyType&& type)
    {
        return PyLong_FromUnsignedLong((unsigned long)type);
    }
} // namespace plPython

// ===========================================================================

class pyGmClimbingWallHandler : public pyGameHandler<pfGmClimbingWallHandler, pyGmClimbingWall>
{
public:
    pyGmClimbingWallHandler(PyObject* obj)
        : pyGameHandler(obj)
    {
    }

public:
    void OnNumBlockersChanged(uint8_t newBlockerCount, bool localOnly) override
    {
        ICallMethod("OnNumBlockersChanged", newBlockerCount, localOnly);
    }

    void OnReady(
        EClimbingWallReadyType readyType,
        bool team1Ready,
        bool team2Ready,
        bool localOnly
    ) override
    {
        ICallMethod(
            "OnReady",
            std::forward<EClimbingWallReadyType>(readyType),
            team1Ready,
            team2Ready,
            localOnly
        );
    }

    void OnBlockersChanged(
        uint8_t teamNumber,
        pfGmClimbingWallBlockers blockers,
        bool localOnly
    ) override
    {
        ICallMethod(
            "OnBlockersChanged",
            teamNumber,
            std::forward<pfGmClimbingWallBlockers>(blockers),
            localOnly
        );
    }

    void OnPlayerEntered() override
    {
        ICallMethod("OnPlayerEntered");
    }

    void OnSuitMachineLocked(
        bool team1MachineLocked,
        bool team2MachineLocked,
        bool localOnly
    ) override
    {
        ICallMethod(
            "OnSuitMachineLocked",
            team1MachineLocked,
            team2MachineLocked,
            localOnly
        );
    }

    void OnGameOver(
        uint8_t teamWon,
        pfGmClimbingWallBlockers team1Blockers,
        pfGmClimbingWallBlockers team2Blockers,
        bool localOnly
    ) override
    {
        ICallMethod(
            "OnGameOver",
            teamWon,
            std::forward<pfGmClimbingWallBlockers>(team1Blockers),
            std::forward<pfGmClimbingWallBlockers>(team2Blockers),
            localOnly
        );
    }
};

// ===========================================================================

void pyGmClimbingWall::Join(PyObject* handler, uint32_t tableID)
{
    pfGmClimbingWall::Join(
        new pyGmClimbingWallHandler(handler),
        tableID
    );
}

bool pyGmClimbingWall::IsSupported()
{
    return pfGmClimbingWall::IsSupported();
}
