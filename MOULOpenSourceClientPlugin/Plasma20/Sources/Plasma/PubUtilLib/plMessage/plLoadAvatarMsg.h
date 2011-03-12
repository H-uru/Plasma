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

#ifndef plLoadAvatarMsg_INC
#define plLoadAvatarMsg_INC

#include "plLoadCloneMsg.h"
#include "hsUtils.h"
#include "../pnKeyedObject/plUoid.h"

class plAvTask;

//
// A msg which is sent to the networking system 
// to cause a player to be loaded or unloaded
//
class plKey;
class hsStream;
class hsResMgr;

// not sure if we need this yet, but it's already in the index so here's just enough
// implementation to keep the compiler happy.
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
	plLoadAvatarMsg(const plUoid &uoidToClone, const plKey &requestorKey, UInt32 userData,
					hsBool isPlayer, const plKey &spawnPoint, plAvTask *initialTask, const char *userStr = nil);

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
	plLoadAvatarMsg(const plKey &existing, const plKey &requestorKey, UInt32 userData,
					hsBool isPlayer, hsBool isLoading, const char *userStr = nil);

	virtual ~plLoadAvatarMsg();

	void SetIsPlayer(bool is);
	hsBool GetIsPlayer();

	void SetSpawnPoint(const plKey &spawnSceneObjectKey);
	plKey GetSpawnPoint();

	void SetInitialTask(plAvTask *task);
	plAvTask * GetInitialTask();

	void SetUserStr(const char *userStr);
	const char* GetUserStr();

	CLASSNAME_REGISTER(plLoadAvatarMsg);
	GETINTERFACE_ANY(plLoadAvatarMsg, plLoadCloneMsg);

	void Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);

	void ReadVersion(hsStream* stream, hsResMgr* mgr);
	void WriteVersion(hsStream* stream, hsResMgr* mgr);
	
protected:
	hsBool fIsPlayer;
	plKey fSpawnPoint;
	plAvTask *fInitialTask;
	char *fUserStr;
};


#endif	// plLoadAvatarMsg_INC


#endif // ndef SERVER
#endif // ndef NO_AV_MSGS
