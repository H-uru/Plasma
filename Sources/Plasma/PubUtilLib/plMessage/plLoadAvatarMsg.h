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

#ifndef plLoadAvatarMsg_INC
#define plLoadAvatarMsg_INC

#include "plLoadCloneMsg.h"

class plAvTask;
class plKey;
class plUoid;

//
// A msg which is sent to the networking system 
// to cause a player to be loaded or unloaded
//
class plLoadAvatarMsg : public plLoadCloneMsg
{
public:
    plLoadAvatarMsg();
    /** Canonical constructor. If you're trying to initiate a clone, this is
        the one you want to use.
        These messages are *always* sent to the net client manager. You can't
        address them.
        After they are received on the remote machines, they are forwarded
        the remote versions of the requestor.

        \param uoidToClone - Specifies the template that will be cloned.
        \param requestorKey - The key of the object that is requesting the clone. It's
            strongly recommended that this be a local object, so that we don't get the same
            requestor creating multiple clones by starting the process on several machines.
        \param userData - Whatever you want. Will be propagated to the requestor after cloning.
        \param isPlayer - this is a player, not an NPC
        \param spawnPoint - warp to this spot after loading
        \param initialTask - queue up this task after loading (and spawning)
        \param userStr - a string that the user can set
        */
    plLoadAvatarMsg(const plUoid &uoidToClone, const plKey &requestorKey, uint32_t userData,
                    bool isPlayer, const plKey &spawnPoint, plAvTask *initialTask,
                    const ST::string &userStr = {});

    /** Use this form if you're sending a message about an existing clone -- either
        to propagate it to other machines or to tell them to unload it.
        \param existing - The key to the clone you want to unload
        \param requestorKey - The key of the object that is requesting the clone. It's
            strongly recommended that this be a local object, so that we don't get the same
            requestor creating multiple clones by starting the process on several machines.
        \param userData - Whatever you want. Will be propagated to the requestor after cloning.
        \param isPlayer - this is a player, not an NPC
        \param isLoading - Are we loading or unloading?
        \param userStr - a string that the user can set
        */
    plLoadAvatarMsg(const plKey &existing, const plKey &requestorKey, uint32_t userData,
                    bool isPlayer, bool isLoading, const ST::string &userStr = {});

    void SetIsPlayer(bool is) { fIsPlayer = is; }
    bool GetIsPlayer() const { return fIsPlayer; }

    void SetSpawnPoint(const plKey &spawnPoint) { fSpawnPoint = spawnPoint; }
    plKey GetSpawnPoint() const { return fSpawnPoint; }

    void SetInitialTask(plAvTask *task) { fInitialTask = task; }
    plAvTask* GetInitialTask() const { return fInitialTask; }

    void SetUserStr(const ST::string &userStr) { fUserStr = userStr; }
    ST::string GetUserStr() const { return fUserStr; }

    CLASSNAME_REGISTER(plLoadAvatarMsg);
    GETINTERFACE_ANY(plLoadAvatarMsg, plLoadCloneMsg);

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    void ReadVersion(hsStream* stream, hsResMgr* mgr) override;
    void WriteVersion(hsStream* stream, hsResMgr* mgr) override;
    
protected:
    bool fIsPlayer;
    plKey fSpawnPoint;
    plAvTask *fInitialTask;
    ST::string fUserStr;
};


#endif  // plLoadAvatarMsg_INC
