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
#ifndef plCloneSpawnModifier_inc
#define plCloneSpawnModifier_inc

#include "../pnModifier/plSingleModifier.h"

class plCloneSpawnModifier : public plSingleModifier
{
protected:
	char* fTemplateName;
	bool fExportTime;

	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty) { return true; }

public:
	plCloneSpawnModifier();
	~plCloneSpawnModifier();

	CLASSNAME_REGISTER(plCloneSpawnModifier);
	GETINTERFACE_ANY(plCloneSpawnModifier, plSingleModifier);

	virtual void Read(hsStream *s, hsResMgr *mgr);
	virtual void Write(hsStream *s, hsResMgr *mgr);

	virtual void SetTarget(plSceneObject* so);

	void SetTemplateName(const char *templateName);

	// Set this to true at export time so the clone mod won't try to make a
	// clone when it's attached
	void SetExportTime() { fExportTime = true; }

	// Console backdoor
	static plKey SpawnClone(const char* cloneName, const char* cloneAge, const hsMatrix44& pos, plKey requestor);
};

#endif // plCloneSpawnModifier_inc
