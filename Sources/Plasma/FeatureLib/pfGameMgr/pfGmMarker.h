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

#ifndef _pfGmMarker_h_
#define _pfGmMarker_h_

#include "pfGameCli.h"

#include <string_theory/string>

#include "pnGameMgr/pnGmMarkerConst.h"

class pfGmMarker;

/**
 * Abstract base class declaring the event handler methods
 * for legacy marker games.
 */
class pfGmMarkerHandler : public pfGameHandler
{
public:
    /**
     * Gets a weak reference to the marker game client that posts
     * notifications to this handler.
     */
    pfGmMarker* GetGameCli() const
    {
        return (pfGmMarker*)pfGameHandler::GetGameCli();
    }

public:
    /**
     * Called when the server acknowledges that a brand new marker game has been created.
     */
    virtual void OnTemplateCreated(ST::string templateID) {}

    /**
     * Called when this client is assigned a team in the marker game.
     * \note As of the writing of this code, it is unknown whether or not multiplayer
     *       marker games actually function correctly on legacy shards.
     */
    virtual void OnTeamAssigned(uint8_t teamNumber) {}

    /**
     * Called during the initial state cascade from the game server to indicate the
     * type of marker game we have.
     */
    virtual void OnGameType(EMarkerGameType gameType) {}

    /**
     * Called when the server acknowledges that the marker game has started.
     */
    virtual void OnGameStarted() {}

    /**
     * Called when the marker game has been paused.
     * \remarks Per testing, Cyan's server does NOT notify the sender of a pause
     *          request that the game has been paused with this notification.
     */
    virtual void OnGamePaused(uint32_t timeLeft) {}

    /**
     * Called when all marker captures have been cleared out and the game
     * state has been completely reset to the never-played state.
     */
    virtual void OnGameReset() {}

    /**
     * Called when the marker game has ended.
     */
    virtual void OnGameOver() {}

    /**
     * Called when the server acknowledges a change to the name of the
     * marker game.
     * \note This is the final notification in the initial game state cascade.
     */
    virtual void OnGameNameChanged(ST::string newName) {}

    /**
     * Called when the server acknowledges a change to the time limit
     * of the marker game.
     * \note As of the writing of this code, it is unknown whether or not multiplayer
     *       marker games actually function correctly on legacy shards.
     */
    virtual void OnTimeLimitChanged(uint32_t newTimeLimit) {}

    /**
     * Called when the marker data has been deleted from the server's database.
     * \note You can detect a previously deleted marker game by observing an
     *       empty game name field on a previously created marker game.
     */
    virtual void OnGameDeleted(bool success) {}

    /**
     * Called when the server acknowledges the creation of a new (or existing) marker.
     * \note This will be called for each marker in the game during the initial
     *       state cascade from the server.
     */
    virtual void OnMarkerAdded(double x, double y, double z, uint32_t markerID,
                               ST::string name, ST::string age) {}

    /**
     * Called when the server acknowledges that a marker has been deleted from the game.
     */
    virtual void OnMarkerDeleted(uint32_t markerID) {}

    /**
     * Called when the server acknowledges that the name of a marker has changed.
     */
    virtual void OnMarkerNameChanged(uint32_t markerID, ST::string newName) {}

    /**
     * Called when the server acknowleges that a marker has been captured.
     * \note This will be called for each marker that has previously been
     *       captured during the initial state cascade from the server.
     */
    virtual void OnMarkerCaptured(uint32_t markerID, uint8_t team) {}
};

/**
 * Legacy marker game client.
 */
class pfGmMarker : public pfGameCli
{
public:
    CLASSNAME_REGISTER(pfGmMarker);
    GETINTERFACE_ANY(pfGmMarker, pfGameCli);

private:
    template<typename _MsgT>
    void Recv(const _MsgT& msg) = delete;

protected:
    bool RecvGameMgrMsg(const GameMsgHeader* msg) override;

public:
    pfGmMarkerHandler* GetHandler() const
    {
        return static_cast<pfGmMarkerHandler*>(pfGameCli::GetHandler());
    }

public:
    /**
     * Request for the server to start the marker game.
     */
    void StartGame() const;

    /**
     * Request for the server to pause the marker game.
     * \note Per testing, Cyan's server does NOT post a notification to
     *       the requester when a marker game is paused.
     */
    void PauseGame() const;

    /**
     * Request for the server to clear all markers to the uncaptured state.
     */
    void ResetGame() const;

    /**
     * Request for the server to change the internal marker game name.
     * \note There is also a marker game name field on the marker game vault
     *       node that is used by the GUI (eg the BigKI) to display the name
     *       of the marker game before it is loaded. Be sure these remain in sync.
     */
    void ChangeGameName(const ST::string& name) const;

    /**
     * Request for the server to change the marker game's time limit.
     * \note As of the writing of this code, it is unknown whether or not multiplayer
     *       marker games actually function correctly on legacy shards.
     */
    void ChangeTimeLimit(uint32_t timeLimit) const;

    /**
     * Request for the server to delete all data associated with this game, including
     * the marker definitions and game name.
     * \note This can irrevocably break marker games on the server. Use with caution.
     */
    void DeleteGame() const;

    /**
     * Request for the server to add a new marker to the game.
     * \param[in] x,y,z The coordinates of the new marker.
     * \param[in] name The name of the new marker.
     * \param[in] age The name of the Age the new marker should appear in.
     */
    void AddMarker(double x, double y, double z, const ST::string& name, const ST::string& age) const;

    /**
     * Request for the server to delete a specific marker from the game.
     */
    void DeleteMarker(uint32_t markerID) const;

    /**
     * Request for the server to change the name of a specific marker from the game.
     */
    void ChangeMarkerName(uint32_t markerID, const ST::string& markerName) const;

    /**
     * Request for the server to register a capture of the specified marker for our team.
     */
    void CaptureMarker(uint32_t markerID) const;

public:
    /**
     * Initialize a new marker game client with the server.
     * \param[in] handler
     * \parblock
     * The marker game event handler.
     *
     * This should be a weak reference to a subclass of `pfGmMarkerHandler` that implements
     * your marker gameplay logic. When the game is successfully established, the server will
     * send down an initial state cascade for the marker game.
     * \endparblock
     * \param[in] gameType The type of marker mission the game will handle.
     * \param[in] timeLimit
     * \parblock
     * The game time limit.
     *
     * \note As of the writing of this code, it is unknown whether or not multiplayer
     *       marker games actually function correctly on legacy shards.
     * \endparblock
     * \param[in] templateID
     * \parblock
     * The template UUID of the marker game.
     *
     * To create an empty marker game, do not set this parameter. To play an already existing
     * marker mission, pass in its template ID from its vault marker game node. The server
     * will initialize an instance of that game for you and will maintain knowledge of the
     * markers that you have captured even if you "recreate" a new game with the same template
     * ID.
     * \endparblock
     */
    static void Create(
        pfGmMarkerHandler* handler,
        EMarkerGameType gameType,
        uint32_t timeLimit = 0,
        ST::string templateID = {}
    );

    /**
     * Checks for the presence of a server-side marker game manager.
     */
    static bool IsSupported();
};

#endif
