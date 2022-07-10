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

#ifndef _pfGameCli_h_
#define _pfGameCli_h_

#include "HeadSpin.h"

#include <type_traits>

#include "pfGameMgr.h"
#include "pfGameMgrTrans.h"

#include "pnFactory/plCreatable.h"
#include "pnGameMgr/pnGameMgrConst.h"

class pfGameCli;

/**
 * Abstract base class declaring the event handler methods
 * for legacy games.
 */
class pfGameHandler
{
    friend class pfGameCli;
    friend class pfGameCliCreateTrans;

    /**
     * Weak reference to the associated game client.
     */
    pfGameCli* fCli;

public:
    pfGameHandler()
        : fCli()
    {
    }

    virtual ~pfGameHandler();

public:
    pfGameCli* GetGameCli() const { return fCli; }

public:
    /**
     * Called when the server responds to our join request.
     */
    virtual void OnGameCliInstance(EGameJoinError result) {}

    /**
     * Called when the `pfGameCli` that posts notifications to this
     * handler is deleted.
     */
    virtual void OnGameCliDelete() {}

    /**
     * Called when a new player joins this game.
     */
    virtual void OnPlayerJoined(uint32_t playerID) {}

    /**
     * Called when a player quits this game.
     */
    virtual void OnPlayerLeft(uint32_t playerID) {}

    /**
     * Called when the ownership of the game is (re)assigned by the server.
     */
    virtual void OnOwnerChanged(uint32_t newOwnerID) {}
};

class pfGameCliCreateTrans : public pfGameMgrTrans
{
    friend class pfGameCli;

protected:
    pfGameCli* fCli;
    EGameJoinError fResult;
    uint32_t fGameId;

public:
    pfGameCliCreateTrans() = delete;

    /**
     * \param cli A stolen reference to the game client.
     * \param handler The user supplied (and user managed) game logic handler.
     */
    pfGameCliCreateTrans(pfGameCli* cli, pfGameHandler* handler);
    ~pfGameCliCreateTrans();

public:
    void Recv(const struct GameMsgHeader* msg) final;
    void Post() final;
};

/**
 * Abstract bass class for legacy game clients.
 */
class pfGameCli : public plCreatable
{
    friend class pfGameMgr;
    friend class pfGameCliCreateTrans;
    friend class pfGameHandler;

    enum
    {
        kFlagAmJoined = (1<<0),
    };

    uint32_t fGameID;
    uint32_t fOwnerID;
    uint32_t fFlags;
    pfGameHandler* fHandler;

public:
    CLASSNAME_REGISTER(pfGameCli);
    GETINTERFACE_ANY(pfGameCli, plCreatable);

protected:
    template<typename _TransT, typename... _ArgsT>
    _TransT* CreateTransaction(_ArgsT&&... args)
    {
        static_assert(std::is_base_of_v<pfGameMgrTrans, _TransT>, "Transactions must be subclasses of pfGameMgrTrans");

        pfGameMgr* mgr = pfGameMgr::GetInstance();
        _TransT* trans = mgr->CreateTransaction<_TransT>(std::forward<_ArgsT>(args)...);
        trans->fGameId = fGameID;
        return trans;
    }

    template<typename _TransT, typename... _ArgsT>
    void SendTransaction(_ArgsT&&... args)
    {
        static_assert(std::is_base_of_v<pfGameMgrTrans, _TransT>, "Transactions must be subclasses of pfGameMgrTrans");

        _TransT* trans = CreateTransaction<_TransT>(std::forward<_ArgsT>(args)...);
        trans->Send();
    }

    virtual bool RecvGameMgrMsg(const struct GameMsgHeader* msg) = 0;

private:
    template<typename _MsgT>
    void Recv(const _MsgT& msg) = delete;

protected:
    pfGameCli()
        : fGameID(), fOwnerID(), fFlags(), fHandler()
    { }
    pfGameCli(const pfGameCli&) = delete;
    pfGameCli(pfGameCli&&) = delete;

    void SetGameId(uint32_t value)
    {
        hsAssert(fGameID == 0, "resetting the gameId, eh?");
        // Holds a refrence to the game client until the server (or plNetGameLib)
        // tells us that the game is going away.
        pfGameMgr::GetInstance()->fGameClis.try_emplace(
            value,
            hsWeakRef<pfGameCli>(this)
        );
        fGameID = value;
    }

public:
    virtual ~pfGameCli();

public:
    /**
     * Explicitly ask the server to allow us to leave the game.
     * \remarks We assume that games are implicitly left when we
     *          disconnect from the game server, so notifications
     *          that we left the game can be a result from disconnects,
     *          not always calls to this method.
     */
    void LeaveGame();

public:
    /**
     * Gets the ID of the game instance on the server.
     * \note This should generally not be used - it exists for compatibility with
     *       legacy clients that expect to know this datum.
     */
    uint32_t GetGameID() const { return fGameID; }

    /**
     * Gets a weak reference to the event handler.
     */
    pfGameHandler* GetHandler() const { return fHandler; }

    /**
     * Gets the ID of the player who owns this game instance.
     */
    uint32_t GetOwnerID() const { return fOwnerID; }

    /**
     * Gets whether or not we are the owner of this game instance.
     */
    bool IsLocallyOwned() const;
};

#endif
