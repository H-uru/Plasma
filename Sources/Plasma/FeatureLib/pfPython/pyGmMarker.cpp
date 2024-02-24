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

#include "pyGmMarker.h"

#include "pfGameMgr/pfGmMarker.h"

#include "plPythonCallable.h"
#include "pyGameHandler.h"
#include "pyObjectRef.h"

// ===========================================================================

void pyGmMarker::StartGame() const
{
    GetGameCli()->StartGame();
}

void pyGmMarker::PauseGame() const
{
    GetGameCli()->PauseGame();
}

void pyGmMarker::ResetGame() const
{
    GetGameCli()->ResetGame();
}

void pyGmMarker::ChangeGameName(const ST::string& name) const
{
    GetGameCli()->ChangeGameName(name);
}

void pyGmMarker::ChangeTimeLimit(uint32_t timeLimit) const
{
    GetGameCli()->ChangeTimeLimit(timeLimit);
}

void pyGmMarker::DeleteGame() const
{
    GetGameCli()->DeleteGame();
}

void pyGmMarker::AddMarker(double x, double y, double z, const ST::string& name, const ST::string& age) const
{
    GetGameCli()->AddMarker(x, y, z, name, age);
}

void pyGmMarker::DeleteMarker(uint32_t markerID) const
{
    GetGameCli()->DeleteMarker(markerID);
}

void pyGmMarker::ChangeMarkerName(uint32_t markerID, const ST::string& markerName) const
{
    GetGameCli()->ChangeMarkerName(markerID, markerName);
}

void pyGmMarker::CaptureMarker(uint32_t markerID) const
{
    GetGameCli()->CaptureMarker(markerID);
}

// ===========================================================================

namespace plPython
{
    template<>
    inline PyObject* ConvertFrom(EMarkerGameType&& type)
    {
        return PyLong_FromUnsignedLong((unsigned long)type);
    }
}

// ===========================================================================

class pyGmMarkerHandler : public pyGameHandler<pfGmMarkerHandler, pyGmMarker>
{
public:
    pyGmMarkerHandler(PyObject* obj)
        : pyGameHandler(obj)
    {
    }

public:
    void OnTemplateCreated(ST::string templateID) override
    {
        ICallMethod("OnTemplateCreated", std::move(templateID));
    }

    void OnTeamAssigned(uint8_t teamNumber) override
    {
        ICallMethod("OnTeamAssigned", teamNumber);
    }

    void OnGameType(EMarkerGameType gameType) override
    {
        ICallMethod("OnGameType", std::forward<EMarkerGameType>(gameType));
    }

    void OnGameStarted() override
    {
        ICallMethod("OnGameStarted");
    }

    void OnGamePaused(uint32_t timeLeft) override
    {
        ICallMethod("OnGamePaused", timeLeft);
    }

    void OnGameReset() override
    {
        ICallMethod("OnGameReset");
    }

    void OnGameOver() override
    {
        ICallMethod("OnGameOver");
    }

    void OnGameNameChanged(ST::string newName) override
    {
        ICallMethod("OnGameNameChanged", std::move(newName));
    }

    void OnTimeLimitChanged(uint32_t newTimeLimit) override
    {
        ICallMethod("OnTimeLimitChanged", newTimeLimit);
    }

    void OnGameDeleted(bool success) override
    {
        ICallMethod("OnGameDeleted", success);
    }

    void OnMarkerAdded(double x, double y, double z, uint32_t markerID,
                       ST::string name, ST::string age) override
    {
        ICallMethod(
            "OnMarkerAdded",
            x, y, z,
            markerID,
            std::move(name),
            std::move(age)
        );
    }

    void OnMarkerDeleted(uint32_t markerID) override
    {
        ICallMethod("OnMarkerDeleted", markerID);
    }

    void OnMarkerNameChanged(uint32_t markerID, ST::string newName) override
    {
        ICallMethod("OnMarkerNameChanged", markerID, std::move(newName));
    }

    void OnMarkerCaptured(uint32_t markerID, uint8_t team) override
    {
        ICallMethod("OnMarkerCaptured", markerID, team);
    }
};

// ===========================================================================

void pyGmMarker::Create(
    PyObject* handler,
    EMarkerGameType gameType,
    ST::string templateID
)
{
    pfGmMarker::Create(
        new pyGmMarkerHandler(handler),
        gameType, 0,
        std::move(templateID)
    );
}

bool pyGmMarker::IsSupported()
{
    return pfGmMarker::IsSupported();
}
