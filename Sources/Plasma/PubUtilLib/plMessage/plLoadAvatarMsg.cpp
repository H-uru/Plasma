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
#ifndef NO_AV_MSGS
#ifndef SERVER

#include "hsResMgr.h"
#include "hsStream.h"

#include "pnNetCommon/plNetApp.h"
#pragma hdrstop

#include "plLoadAvatarMsg.h"
#include "plAvatar/plAvatarTasks.h"


//////////////////
// PLLOADAVATARMSG
//////////////////

// CTOR (default)
plLoadAvatarMsg::plLoadAvatarMsg()
: fIsPlayer(false),
  fSpawnPoint(nullptr),
  fInitialTask(nullptr)
{
}

// CTOR uoidToClone, requestorKey, userData, isPlayer, spawnPOint, initialTask
plLoadAvatarMsg::plLoadAvatarMsg(const plUoid &uoidToClone, const plKey &requestorKey, uint32_t userData,
                                 bool isPlayer, const plKey &spawnPoint, plAvTask *initialTask, const ST::string &userStr)
: plLoadCloneMsg(uoidToClone, requestorKey, userData),
  fIsPlayer(isPlayer),
  fSpawnPoint(spawnPoint),
  fInitialTask(initialTask),
  fUserStr(userStr)
{
}

plLoadAvatarMsg::plLoadAvatarMsg(const plKey &existing, const plKey &requestor, uint32_t userData,
                                 bool isPlayer, bool isLoading, const ST::string &userStr)
: plLoadCloneMsg(existing, requestor, userData, isLoading),
  fIsPlayer(isPlayer),
  fSpawnPoint(nullptr),
  fInitialTask(nullptr),
  fUserStr(userStr)
{
}

void plLoadAvatarMsg::Read(hsStream* stream, hsResMgr* mgr)
{
    plLoadCloneMsg::Read(stream, mgr);
    fIsPlayer = stream->ReadBool();
    fSpawnPoint = mgr->ReadKey(stream);
    if(stream->ReadBool())
    {
        fInitialTask = plAvTask::ConvertNoRef(mgr->ReadCreatable(stream));
    }
    fUserStr = stream->ReadSafeString();
}

void plLoadAvatarMsg::Write(hsStream *stream, hsResMgr *mgr)
{
    plLoadCloneMsg::Write(stream, mgr);
    stream->WriteBool(fIsPlayer);
    mgr->WriteKey(stream, fSpawnPoint);
    if(fInitialTask)
    {
        stream->WriteBool(true);
        mgr->WriteCreatable(stream, fInitialTask);
    } else {
        stream->WriteBool(false);
    }
    stream->WriteSafeString(fUserStr);
}

enum LoadAvatarMsgFlags
{
    kLoadAvatarMsgIsPlayer,
    kLoadAvatarMsgSpawnPoint,
    kLoadAvatarMsgUserStr,
};

void plLoadAvatarMsg::ReadVersion(hsStream* stream, hsResMgr* mgr)
{
    plLoadCloneMsg::ReadVersion(stream, mgr);

    hsBitVector contentFlags;
    contentFlags.Read(stream);

    if (contentFlags.IsBitSet(kLoadAvatarMsgIsPlayer))
        fIsPlayer = stream->ReadBool();

    if (contentFlags.IsBitSet(kLoadAvatarMsgSpawnPoint))
        fSpawnPoint = mgr->ReadKey(stream);

    if (contentFlags.IsBitSet(kLoadAvatarMsgUserStr))
        fUserStr = stream->ReadSafeString();
}

void plLoadAvatarMsg::WriteVersion(hsStream* stream, hsResMgr* mgr)
{
    plLoadCloneMsg::WriteVersion(stream, mgr);

    hsBitVector contentFlags;
    contentFlags.SetBit(kLoadAvatarMsgIsPlayer);
    contentFlags.SetBit(kLoadAvatarMsgSpawnPoint);
    contentFlags.SetBit(kLoadAvatarMsgUserStr);
    contentFlags.Write(stream);

    // kLoadAvatarMsgIsPlayer
    stream->WriteBool(fIsPlayer);

    // kLoadAvatarMsgSpawnPoint
    mgr->WriteKey(stream, fSpawnPoint);

    // kLoadAvatarMsgUserStr
    stream->WriteSafeString(fUserStr);
}

#endif // ndef SERVER
#endif // ndef NO_AV_MSGS
