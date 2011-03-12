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
#ifndef plSwimRegion_inc
#define plSwimRegion_inc

#include "../pnSceneObject/plObjInterface.h"

class plArmatureModBase;
class plPhysical;
struct hsVector3;
class plPhysicalControllerCore;
class plSwimRegionInterface : public plObjInterface
{
public:
	plSwimRegionInterface() : fDownBuoyancy(1.f), fUpBuoyancy(1.f), fMaxUpwardVel(1.f) {}
	virtual ~plSwimRegionInterface() {}
	
	CLASSNAME_REGISTER( plSwimRegionInterface );
	GETINTERFACE_ANY( plSwimRegionInterface, plObjInterface );
	
	enum {
		kDisable		= 0x0,
		kNumProps // last
	};
	
	virtual Int32 GetNumProperties() const { return kNumProps; }	
	virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l) {}
	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	virtual void GetCurrent(plPhysicalControllerCore *physical, hsVector3 &linearResult, hsScalar &angularResult, hsScalar elapsed);

	hsScalar fDownBuoyancy;
	hsScalar fUpBuoyancy;
	hsScalar fMaxUpwardVel;
};

class plSwimCircularCurrentRegion : public plSwimRegionInterface
{
public:
	plSwimCircularCurrentRegion();
	virtual ~plSwimCircularCurrentRegion() {}

	CLASSNAME_REGISTER( plSwimCircularCurrentRegion );
	GETINTERFACE_ANY( plSwimCircularCurrentRegion, plSwimRegionInterface );

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	virtual void GetCurrent(plPhysicalControllerCore *physical, hsVector3 &linearResult, hsScalar &angularResult, hsScalar elapsed);	
	virtual hsBool MsgReceive(plMessage* msg);
	
	hsScalar fRotation;
	hsScalar fPullNearDistSq;
	hsScalar fPullNearVel;
	hsScalar fPullFarDistSq;
	hsScalar fPullFarVel;

protected:
	plSceneObject *fCurrentSO;
};

class plSwimStraightCurrentRegion : public plSwimRegionInterface
{
public:
	plSwimStraightCurrentRegion();
	virtual ~plSwimStraightCurrentRegion() {}
	
	CLASSNAME_REGISTER( plSwimStraightCurrentRegion );
	GETINTERFACE_ANY( plSwimStraightCurrentRegion, plSwimRegionInterface );
	
	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);
	
	virtual void GetCurrent(plPhysicalControllerCore *physical, hsVector3 &linearResult, hsScalar &angularResult, hsScalar elapsed);	
	virtual hsBool MsgReceive(plMessage* msg);
	
	hsScalar fNearDist;
	hsScalar fNearVel;
	hsScalar fFarDist;
	hsScalar fFarVel;
	
protected:
	plSceneObject *fCurrentSO;
};

#endif // plSwimRegion_inc
