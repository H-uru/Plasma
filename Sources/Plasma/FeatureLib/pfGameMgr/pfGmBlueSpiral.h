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

#ifndef _pfGmBlueSpiral_h_
#define _pfGmBlueSpiral_h_

#include "pfGameCli.h"

#include <array>
#include <string_theory/string>

#include "pnGameMgr/pnGmMarkerConst.h"

class pfGmBlueSpiral;

/**
 * Abstract base class declaring the event handler methods
 * for legacy blue spiral.
 */
class pfGmBlueSpiralHandler : public pfGameHandler
{
public:
    /**
     * Gets a weak reference to the blue spiral game client that posts
     * notifications to this handler.
     */
    pfGmBlueSpiral* GetGameCli() const
    {
        return (pfGmBlueSpiral*)pfGameHandler::GetGameCli();
    }

public:
    /**
     * Called when the server sends us the correct cloth order for the current blue spiral game.
     */
    virtual void OnClothOrder(const std::array<uint8_t, 7>& order) {}

    /**
     * Called when a cloth was pressed correctly.
     */
    virtual void OnClothHit() {}


    /**
     * Called when all cloths have been pressed in the correct order.
     * \remarks In the Eder Delin and Eder Tsogal Ages, this notification should
     *          trigger the Bahro doors to open.
     */
    virtual void OnGameWon() {}

    /**
     * Called when the blue spiral game has ended.
     * \remarks This is called both in "win" and "lose" situations. The game
     *          lasts for a fixed amount of time, so on receipt of this message,
     *          Eder Delin and Eder Tsogal will close their Bahro doors (if applicable)
     *          and stop the Bahro symbol display/spinner (if applicable).
     */
    virtual void OnGameOver() {}

    /**
     * Called when the blue spiral game has begun.
     * \remarks This is called *twice* per game. The first call will receive the
     *          argument `startSpin=false`, indicating that, in the case of the
     *          Eder Delin and Eder Tsogal Ages, the cloth order should be displayed
     *          on the door. This will be called again 15 seconds later with
     *          `startSpin=true`, indicating that the Bahro symbol should start
     *          spinning.
     */
    virtual void OnGameStarted(bool startSpin) {}
};

class pfGmBlueSpiral : public pfGameCli
{
public:
    CLASSNAME_REGISTER(pfGmBlueSpiral);
    GETINTERFACE_ANY(pfGmBlueSpiral, pfGameCli);

private:
    template<typename _MsgT>
    void Recv(const _MsgT& msg) = delete;

protected:
    bool RecvGameMgrMsg(const GameMsgHeader* msg) override;

public:
    pfGmBlueSpiralHandler* GetHandler() const
    {
        return static_cast<pfGmBlueSpiralHandler*>(pfGameCli::GetHandler());
    }

public:
    /**
     * Request for the server to start the game timer.
     */
    void StartGame() const;

    /**
     * Request for the server to hit a specific cloth index and validate the
     * correct sequence of cloth inputs.
     */
    void HitCloth(uint8_t cloth) const;

public:
    /**
     * Join a common blue spiral game in the current Age.
     * \param[in] handler
     * \parblock
     * The blue spiral game event handler.
     *
     * This should be a weak reference to a subclass of `pfGmBlueSpiralHandler` that implements
     * your blue spiral gameplay logic.
     * \endparblock
     * \param[in] tableID
     * \parblock
     * The index of the game table in the current Age to join.
     */
    static void Join(pfGmBlueSpiralHandler* handler, uint32_t tableID);

    /**
     * Checks for the presence of a server-side marker game manager.
     */
    static bool IsSupported();
};

#endif
