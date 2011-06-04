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

#ifndef plSingleModifier_inc
#define plSingleModifier_inc

#include "plModifier.h"
#include "hsBitVector.h"
#include "../pnNetCommon/plSynchedValue.h"

class plSceneObject;
class plSingleModMsg;

class plSingleModifier : public plModifier
{
protected:
	plSceneObject*	fTarget;
	hsBitVector		fFlags;

	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty) = 0;
	
public:
	plSingleModifier();
	virtual ~plSingleModifier();

	CLASSNAME_REGISTER( plSingleModifier );
	GETINTERFACE_ANY( plSingleModifier, plModifier );
	
	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	virtual int GetNumTargets() const { return 1; }
	virtual plSceneObject* GetTarget(int iTarg) const {return fTarget;}
	virtual void AddTarget(plSceneObject* so) {SetTarget(so);}
	virtual void RemoveTarget(plSceneObject* so) {fTarget = 0;} 


	virtual plSceneObject* GetTarget() const { return fTarget; }
	virtual void SetTarget(plSceneObject* so) { fTarget = so; }

	hsBool HasFlag(int f) const { return fFlags.IsBitSet(f); }
	plSingleModifier& SetFlag(int f) { fFlags.SetBit(f); return *this; }
	plSingleModifier& ClearFlag(int f) { fFlags.ClearBit(f); return *this; }

};

#endif // plSingleModifier_inc
