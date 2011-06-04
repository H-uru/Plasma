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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#ifndef NO_AV_MSGS
#ifndef SERVER

#include "hsStream.h"
#include "plLoadAvatarMsg.h"
#include "hsResMgr.h"
#include "../pnNetCommon/plNetApp.h"
#include "../pnNetCommon/plSynchedObject.h"

#include "../plAvatar/plAvatarTasks.h"


//////////////////
// PLLOADAVATARMSG
//////////////////

// CTOR (default)
plLoadAvatarMsg::plLoadAvatarMsg()
: fIsPlayer(false),
  fSpawnPoint(nil),
  fInitialTask(nil),
  fUserStr(nil)
{
}

// CTOR uoidToClone, requestorKey, userData, isPlayer, spawnPOint, initialTask
plLoadAvatarMsg::plLoadAvatarMsg(const plUoid &uoidToClone, const plKey &requestorKey, UInt32 userData,
								 hsBool isPlayer, const plKey &spawnPoint, plAvTask *initialTask, const char* userStr /*= nil*/)
: plLoadCloneMsg(uoidToClone, requestorKey, userData),
  fIsPlayer(isPlayer),
  fSpawnPoint(spawnPoint),
  fInitialTask(initialTask),
  fUserStr(nil) // setting to nil so SetUserStr doesn't try to nuke garbage
{
	SetUserStr(userStr);
}

plLoadAvatarMsg::plLoadAvatarMsg(const plKey &existing, const plKey &requestor, UInt32 userData,
								hsBool isPlayer, hsBool isLoading, const char* userStr /*= nil*/)
:	plLoadCloneMsg(existing, requestor, userData, isLoading),
	fIsPlayer(isPlayer),
	fSpawnPoint(nil),
	fInitialTask(nil),
	fUserStr(nil) // setting to nil so SetUserStr doesn't try to nuke garbage
{
	SetUserStr(userStr);
}


// DTOR
plLoadAvatarMsg::~plLoadAvatarMsg()
{
	if (fUserStr)
	{
		delete [] fUserStr;
		fUserStr = nil;
	}
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
	if (fUserStr)
	{
		delete [] fUserStr;
		fUserStr = nil;
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

	if (fUserStr)
	{
		delete [] fUserStr;
		fUserStr = nil;
	}
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

// SETISPLAYER
void plLoadAvatarMsg::SetIsPlayer(bool is)
{
	fIsPlayer = is;
}

// GETISPLAYER
hsBool plLoadAvatarMsg::GetIsPlayer()
{
	return fIsPlayer;
}

// SETSPAWNPOINT
void plLoadAvatarMsg::SetSpawnPoint(const plKey &spawnPoint)
{
	fSpawnPoint = spawnPoint;
}

// GETSPAWNPOINT
plKey plLoadAvatarMsg::GetSpawnPoint()
{
	return fSpawnPoint;
}

// SETINITIALTASK
void plLoadAvatarMsg::SetInitialTask(plAvTask *initialTask)
{
	fInitialTask = initialTask;
}

// GETINITIALTASK
plAvTask * plLoadAvatarMsg::GetInitialTask()
{
	return fInitialTask;
}

// SETUSERSTR
void plLoadAvatarMsg::SetUserStr(const char *userStr)
{
	if (fUserStr)
		delete [] fUserStr;
	if (!userStr)
	{
		fUserStr = nil;
		return;
	}

	fUserStr = TRACKED_NEW char[strlen(userStr) + 1];
	strcpy(fUserStr, userStr);
	fUserStr[strlen(userStr)] = '\0';
}

// GETUSERSTR
const char* plLoadAvatarMsg::GetUserStr()
{
	return fUserStr;
}

#endif // ndef SERVER
#endif // ndef NO_AV_MSGS
