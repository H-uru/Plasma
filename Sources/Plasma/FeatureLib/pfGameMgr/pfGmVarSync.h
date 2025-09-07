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

#ifndef _pfGmVarSync_h_
#define _pfGmVarSync_h_

#include "pfGameCli.h"

#include <string_theory/string>
#include <variant>

class pfGmVarSync;

using pfGmVarSyncValue = std::variant<ST::string, double>;

/**
 * Abstract base class declaring the event handler methods
 * for legacy var sync.
 */
class pfGmVarSyncHandler : public pfGameHandler
{
public:
    /**
     * Gets a weak reference to the var sync game client that posts
     * notifications to this handler.
     */
    pfGmVarSync* GetGameCli() const
    {
        return (pfGmVarSync*)pfGameHandler::GetGameCli();
    }

public:
    /**
     * Called when the server sends us an updated value for a synchronized variable.
     */
    virtual void OnVarChanged(uint32_t varID, pfGmVarSyncValue varValue) {}

    /**
     * Called when the server has sent us all of the variables that have been
     * syncrhonized before we joined the game.
     */
    virtual void OnAllVarsSent() {}

    /**
     * Called when a new variable has been synchronized or as part of the initial
     * state broadcast.
     */
    virtual void OnVarCreated(
        ST::string varName,
        uint32_t varID,
        pfGmVarSyncValue value
    ) {}
};

class pfGmVarSync : public pfGameCli
{
public:
    CLASSNAME_REGISTER(pfGmVarSync);
    GETINTERFACE_ANY(pfGmVarSync, pfGameCli);

private:
    template <typename _MsgT>
    void Recv(const _MsgT& msg) = delete;

protected:
    bool RecvGameMgrMsg(const struct GameMsgHeader* msg) override;

public:
    pfGmVarSyncHandler* GetHandler() const
    {
        return static_cast<pfGmVarSyncHandler*>(pfGameCli::GetHandler());
    }

public:
    /**
     * Change the value of a variable on the server.
     */
    void SetVariable(uint32_t varID, const pfGmVarSyncValue& varValue) const;

    /**
     * Create a new variable on the server.
     */
    void CreateVariable(const ST::string& varName, const pfGmVarSyncValue& varValue) const;

public:
    /**
     * Join a common var sync game in the current Age.
     * \param[in] handler
     * \parblock
     * The var sync game event handler.
     *
     * This should be a weak reference to a subclass of `pfGmVarSyncHandler` that implements
     * your blue spiral gameplay logic.
     * \endparblock
     */
    static void Join(pfGmVarSyncHandler* handler);

    /**
     * Checks for the presence of a server-side var sync game manager.
     */
    static bool IsSupported();
};

#endif
