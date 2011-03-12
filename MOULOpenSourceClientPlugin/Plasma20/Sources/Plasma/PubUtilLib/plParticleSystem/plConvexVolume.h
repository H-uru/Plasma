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
#ifndef plConvexVolume_inc
#define plConvexVolume_inc

#include "../pnSceneObject/plObjInterface.h"

struct hsPlane3;
struct hsPoint3;
struct hsMatrix44;
class hsResMgr;

// A convex volume defined by several boundary planes
// For now it assumes the user won't add planes that make it concave

class plConvexVolume : public plCreatable
{
public:
	plConvexVolume();
	~plConvexVolume();

	CLASSNAME_REGISTER( plConvexVolume );
	GETINTERFACE_ANY( plConvexVolume, plCreatable );

	void Update(const hsMatrix44 &l2w);

	hsBool AddPlane(const hsPlane3 &plane);
	void SetNumPlanesAndClear(const UInt32 num);
	void SetPlane(const hsPlane3 &plane, const UInt32 index);

	// If you only care about the test, call this. Otherwise call ResolvePoint.
	hsBool IsInside(const hsPoint3 &pos) const;

	// returns true if the point was inside the volume, and thus moved outward.
	hsBool ResolvePoint(hsPoint3 &pos) const; 

	// returns true if the point was inside and pos and velocity updated to bounce off offending plane.
	// input bounce==1.f for perfect bounce, bounce==0 to slide.
	hsBool BouncePoint(hsPoint3 &pos, hsVector3 &velocity, hsScalar bounce, hsScalar friction) const;

	inline hsBool TestPlane(const hsPoint3 &pos, const hsPlane3 &plane) const; // Is the point inside the plane?
	virtual void Read(hsStream* s, hsResMgr *mgr);
	virtual void Write(hsStream* s, hsResMgr *mgr);
	//virtual hsBool MsgReceive(plMessage* msg);
	
protected:
	void IClear();

	hsPlane3 *fLocalPlanes;
	hsPlane3 *fWorldPlanes;
	UInt32 fNumPlanes;
};

inline hsBool plConvexVolume::TestPlane(const hsPoint3 &pos, const hsPlane3 &plane) const
{
	hsScalar dis = plane.fN.InnerProduct(pos);
	dis += plane.fD;
	if( dis >= 0.f )	
		return false;	
						
	return true;
}

#endif
