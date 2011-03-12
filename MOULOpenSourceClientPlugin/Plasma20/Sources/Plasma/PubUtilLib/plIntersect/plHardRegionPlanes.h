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

#ifndef plHardRegionPlanes_inc
#define plHardRegionPlanes_inc

#include "plHardRegion.h"

class plHardRegionPlanes : public plHardRegion
{
protected:
	class HardPlane
	{
	public:
		hsVector3			fNorm;
		hsPoint3			fPos;

		hsVector3			fWorldNorm;
		hsPoint3			fWorldPos;
		hsScalar			fWorldDist;
	};
	hsTArray<HardPlane>			fPlanes;

protected:
public:
	plHardRegionPlanes();
	virtual ~plHardRegionPlanes();

	CLASSNAME_REGISTER( plHardRegionPlanes );
	GETINTERFACE_ANY( plHardRegionPlanes, plHardRegion );

	virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l);

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	// Now Planes specifics
	void AddPlane(const hsVector3& n, const hsPoint3& p);
	UInt32 GetNumPlanes() const { return fPlanes.GetCount(); }
	void GetPlane(int i, hsVector3& n, hsPoint3& p) const { n = fPlanes[i].fNorm; p = fPlanes[i].fPos; }
	void GetWorldPlane(int i, hsVector3& n, hsPoint3& p) const { n = fPlanes[i].fWorldNorm; p = fPlanes[i].fWorldPos; }

	virtual hsBool IIsInside(const hsPoint3& pos) const;
	virtual hsBool ICameraInside() const;

};



#endif // plHardRegionPlanes_inc

