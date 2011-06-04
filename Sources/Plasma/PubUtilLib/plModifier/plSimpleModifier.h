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

#ifndef plSimpleModifier_inc
#define plSimpleModifier_inc

#include "../pnModifier/plModifier.h"
#include "../pnNetCommon/plSynchedValue.h"
#include "../plInterp/plAnimTimeConvert.h"

class plSceneObject;
class plRefMsg;
class plTimeMsg;
class plAnimCmdMsg;

class plSimpleModifier : public plModifier
{
public:
	enum {
		kStopped		= 0,
		kLoop,
		kBackwards,
		kWrapped,

		kNumFlags
	};
protected:
	plSceneObject*	fTarget;

	double					fLastTime; // in world time

	plAnimTimeConvert		fTimeConvert;

	// The following are in animation time.
	hsScalar				fCurrentTime;

	hsBool					IHandleTime(double wSecs, hsScalar del);

	hsBool					IHandleRef(plRefMsg* refMsg);
	virtual hsBool			IHandleCmd(plAnimCmdMsg* modMsg);

	virtual void			IApplyDynamic() = 0;

	virtual void			IBegin();
	virtual void			IEnd();

	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty);

public:
	plSimpleModifier();
	virtual ~plSimpleModifier();

	CLASSNAME_REGISTER( plSimpleModifier );
	GETINTERFACE_ANY( plSimpleModifier, plModifier);

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	virtual hsBool MsgReceive(plMessage* msg);

	virtual int GetNumTargets() const { return fTarget ? 1 : 0; }
	virtual plSceneObject* GetTarget(int w) const { /* hsAssert(w < GetNumTargets(), "Bad target"); */ return fTarget; }
	virtual void AddTarget(plSceneObject* so);
	virtual void RemoveTarget(plSceneObject* so);
	virtual plAnimTimeConvert& GetTimeConvert() { return fTimeConvert;	}


};


#endif // plSimpleModifier_inc
