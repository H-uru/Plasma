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

#ifndef _pfGmClimbingWall_h_
#define _pfGmClimbingWall_h_

#include <set>

#include "pfGameCli.h"

#include "pnGameMgr/pnGameMgr.h"
#include "pnGameMgr/pnGmClimbingWallConst.h"

using pfGmClimbingWallBlockers = std::set<uint8_t>;

class pfGmClimbingWall;

class pfGmClimbingWallHandler : public pfGameHandler
{
public:
    /**
     * Gets a weak reference to the climbing wall game client that posts
     * notifications to this handler.
     */
    pfGmClimbingWall* GetGameCli() const
    {
        return (pfGmClimbingWall*)pfGameHandler::GetGameCli();
    }

public:
    virtual void OnNumBlockersChanged(uint8_t newBlockerCount, bool localOnly) {}

    virtual void OnReady(
        EClimbingWallReadyType readyType,
        bool team1Ready,
        bool team2Ready,
        bool localOnly
    ) {}

    virtual void OnBlockersChanged(
        uint8_t teamNumber,
        pfGmClimbingWallBlockers blockers,
        bool localOnly
    ) {}

    virtual void OnPlayerEntered() {}

    virtual void OnSuitMachineLocked(
        bool team1MachineLocked,
        bool team2MachineLocked,
        bool localOnly
    ) {}

    virtual void OnGameOver(
        uint8_t teamWon,
        pfGmClimbingWallBlockers team1Blockers,
        pfGmClimbingWallBlockers team2Blockers,
        bool localOnly
    ) {}
};

class pfGmClimbingWall : public pfGameCli
{
public:
    CLASSNAME_REGISTER(pfGmClimbingWall);
    GETINTERFACE_ANY(pfGmClimbingWall, pfGameCli);

private:
    template<typename _MsgT>
    void Recv(const _MsgT& msg) = delete;

protected:
    bool RecvGameMgrMsg(const struct GameMsgHeader* msg) override;

public:
    pfGmClimbingWallHandler* GetHandler() const
    {
        return static_cast<pfGmClimbingWallHandler*>(pfGameCli::GetHandler());
    }

public:
    /**
     * Request for the server to change the maximum number of blockers.
     * \remarks This will have no effect if the game's ready state
     * `kClimbingWallReadyNumBlockers` is set.
     */
    void ChangeNumBlockers(int32_t amountToAdjust) const;

    /**
     * Request for the server to change the ready state for the given team.
     * \param[in] readyType The game state to mark ready, either the number
     * of blockers or blocker layout itself.
     * \param[in] teamNumber The team number [1..2] to mark ready.
     * \remarks The number of blockers must be marked ready by both teams
     * before the server will allow any blockers to be turned on or off.
     * Further, the blockers themselves must be marked ready before a player
     * can enter the wall game.
     */
    void Ready(EClimbingWallReadyType readyType, uint8_t teamNumber) const;

    /**
     * Request for the server to change the state of a blocker.
     * \remarks This will have no effect if this changes causes the number
     * of active blockers to exceed the maximum number of blockers.
     */
    void ChangeBlocker(
        uint8_t teamNumber,
        uint8_t blockerNumber,
        bool added
    ) const;

    /**
     * Request for the server to reset the game to the initial state.
     */
    void Reset() const;

    /**
     * Request for the server to enter the local player as the
     * person playing the wall game for the given team.
     * \param[in] teamNumber The team number [1..2] that the
     * player will be on.
     * \remarks This will have no effect if the game's ready state
     * is not `kClimbingWallReadyNumBlockers` and
     * kClimbingWallReadyBlockers`. Once a player enters, the
     * server will respond with pfGmClimbingWallHandler::OnPlayerEntered()
     * and pfGmClimbingWallHandler::OnSuitMachineLocked() for
     * the team the player entered.
     */
    void EnterPlayer(uint8_t teamNumber) const;

    /**
     * Request for the server to successfully complete the climbing
     * wall game for our team.
     * \remarks This will have no effect if the game's ready state
     * is not `kClimbingWallReadyNumBlockers` and
     * `kClimbingWallReadyBlockers` or if the local player
     * has not entered the climbing wall. Once a player has
     * successfully completed the game, the server will send
     * pfGmClimbingWallHandler::OnGameOver() with the winning
     * team's information and the blocker set for each team.
     * Approximately one second later, the server will reset
     * the game to the default state and notify all clients.
     * \sa pfGmClimbingWall::EnterPlayer()
     */
    void FinishGame() const;

    /**
     * Request for the server to forfeit the wall game for our team.
     * \sa pfGmClimbingWall::EnterPlayer()
     */
    void Panic() const;

public:
    /**
     * Join a common climbing wall game in the current Age.
     * \param[in] handler
     * \parblock
     * The climbing wall game event handler.
     *
     * This should be a weak reference to a subclass of `pfGmClimbingWallHandler` that implements
     * your climbing wall gameplay logic.
     * \endparblock
     * \param[in] tableID
     * \parblock
     * The index of the game table in the current Age to join.
     */
    static void Join(pfGmClimbingWallHandler* handler, uint32_t tableID);

    /**
     * Checks for the presence of a server-side climbing wall game manager.
     */
    static bool IsSupported();
};

#endif // _pfGmClimbingWall_h_
