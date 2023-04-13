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

#include "pyGmBlueSpiral.h"

#include "pyGameHandler.h"
#include "plPythonConvert.h"

#include <array>

#include "pfGameMgr/pfGmBlueSpiral.h"

// ===========================================================================

void pyGmBlueSpiral::StartGame() const
{
    GetGameCli()->StartGame();
}

void pyGmBlueSpiral::HitCloth(uint8_t cloth) const
{
    GetGameCli()->HitCloth(cloth);
}

// ===========================================================================

class pyGmBlueSpiralHandler : public pyGameHandler<pfGmBlueSpiralHandler, pyGmBlueSpiral>
{
public:
    pyGmBlueSpiralHandler(PyObject* obj)
        : pyGameHandler(obj)
    {
    }

public:
    void OnClothOrder(const std::array<uint8_t, 7>& order) override
    {
        // Hmmm... plPython::ConvertFrom() absolutely hates anything like
        // std::array<uint8_t, 7> and std::tuple<uint8_t, ...> on gcc/clang.
        // Oh well.
        ICallMethod(
            "OnClothOrder",
            std::apply(
                [](auto&&... args) -> PyObject* {
                    return plPython::ConvertFrom(plPython::ToTuple, args...);
                },
                std::tuple_cat(order)
            )
        );
    }

    void OnClothHit() override
    {
        ICallMethod("OnClothHit");
    }

    void OnGameWon() override
    {
        ICallMethod("OnGameWon");
    }

    void OnGameOver() override
    {
        ICallMethod("OnGameOver");
    }

    void OnGameStarted(bool startSpin) override
    {
        ICallMethod("OnGameStarted", startSpin);
    }
};

// ===========================================================================

void pyGmBlueSpiral::Join(PyObject* handler, uint32_t tableID)
{
    pfGmBlueSpiral::Join(
        new pyGmBlueSpiralHandler(handler),
        tableID
    );
}

bool pyGmBlueSpiral::IsSupported()
{
    return pfGmBlueSpiral::IsSupported();
}
