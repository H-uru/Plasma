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

#ifndef plFollowMod_inc
#define plFollowMod_inc

#include "hsMatrix44.h"
#include "../pnModifier/plSingleModifier.h"

class plSceneObject;
class plMessage;
class hsStream;
class hsResMgr;

class plFollowMod : public plSingleModifier
{
public:
	enum FollowRefs
	{
		kRefLeader
	};
	enum FollowLeaderType
	{
		kLocalPlayer,
		kObject,
		kCamera,
		kListener
	};
	enum FollowModMode
	{
		kPositionX	= 0x1,
		kPositionY	= 0x2,
		kPositionZ	= 0x4,
		kPosition	= (kPositionX | kPositionY | kPositionZ),
		kRotate		= 0x8,
		kFullTransform	= kPosition | kRotate
	};
protected:
	FollowLeaderType	fLeaderType;
	UInt8				fMode;
	UInt8				fLeaderSet;

	plSceneObject*		fLeader; // may be nil if Leader isn't a sceneobject

	hsMatrix44			fLeaderL2W;
	hsMatrix44			fLeaderW2L;

	hsBool ICheckLeader();
	void IMoveTarget();

	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty);

public:
	plFollowMod();
	~plFollowMod();

	CLASSNAME_REGISTER( plFollowMod );
	GETINTERFACE_ANY( plFollowMod, plSingleModifier );

	virtual hsBool MsgReceive(plMessage* msg);

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	virtual void SetTarget(plSceneObject* so);

	void SetType(FollowLeaderType t) { fLeaderType = t; }
	FollowLeaderType GetType() const { return fLeaderType; }

	void SetMode(UInt8 m) { fMode = m; }
	UInt8 GetMode() const { return fMode; }

	void Activate();
	void Deactivate();

};

#endif // plFollowMod_inc
