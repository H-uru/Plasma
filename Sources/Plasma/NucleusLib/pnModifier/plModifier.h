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

#ifndef plModifier_inc
#define plModifier_inc

#include "../pnNetCommon/plSynchedObject.h"

class hsStream;
class hsResMgr;
class plMessage;
class plSceneObject;
class plObjInterface;
class plDrawInterface;
class plSimulationInterface;
class plCoordinateInterface;
class plAudioInterface;
struct hsMatrix44;

class plModifier : public plSynchedObject
{
protected:

	plDrawInterface*			IGetTargetDrawInterface(int iTarg) const;
	plSimulationInterface*		IGetTargetSimulationInterface(int iTarg) const;
	plCoordinateInterface*		IGetTargetCoordinateInterface(int iTarg) const;
	plAudioInterface*			IGetTargetAudioInterface(int iTarg) const;
	plObjInterface*				IGetTargetGenericInterface(int iTarg, UInt32 classIdx) const;
	plModifier*					IGetTargetModifier(int iTarg, int iMod) const;

	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty) = 0; // called only by owner object's Eval()

	friend class plSceneObject;
public:

	plModifier();
	virtual ~plModifier();

	CLASSNAME_REGISTER( plModifier );
	GETINTERFACE_ANY( plModifier, plSynchedObject );

	virtual hsBool MsgReceive(plMessage* msg);

	virtual int GetNumTargets() const = 0;
	virtual plSceneObject* GetTarget(int iTarg) const = 0;
	virtual void AddTarget(plSceneObject* so) = 0;
	virtual void RemoveTarget(plSceneObject* so) = 0; 

	virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l) {}

};

#endif // plModifier_inc
