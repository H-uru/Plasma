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
#include "../pnModifier/plSingleModifier.h"

class plNotifyMsg;

class plNPCSpawnMod : public plSingleModifier
{
public:
	plNPCSpawnMod();
	plNPCSpawnMod(const char * modelName, const char *accountName, bool autoSpawn);
	~plNPCSpawnMod();

	bool Trigger();
	void SetNotify(plNotifyMsg *notify);
	plNotifyMsg * GetNotify();
	
	CLASSNAME_REGISTER( plNPCSpawnMod );
	GETINTERFACE_ANY( plNPCSpawnMod, plSingleModifier );
	
	virtual void AddTarget(plSceneObject* so);
	virtual void RemoveTarget(plSceneObject *so);
//	hsBool MsgReceive(plMessage* msg);

	virtual void Read(hsStream *stream, hsResMgr *mgr);
	virtual void Write(hsStream *stream, hsResMgr *mgr);

protected:
	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty);
	void ISendNotify(plKey &avatarKey);		// send our notification message

private:
	char *fModelName;
	char *fAccountName;
	bool fAutoSpawn;			// spawn immediately on loading
	plKey fSpawnedKey;			// if we want to be able to spawn many things, we should make this a vector
	plNotifyMsg *fNotify;		// notify message that we send when we spawn.
};

