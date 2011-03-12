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
#ifndef plPhysicsSoundMgr_h_inc
#define plPhysicsSoundMgr_h_inc

#include "hsStlUtils.h"
#include "plPhysical.h"
#include "hsGeometry3.h"

class plPhysicsSoundMgr
{
public:
	void AddContact(plPhysical* phys1, plPhysical* phys2, const hsPoint3& hitPoint, const hsVector3& hitNormal);
	void Update();

private:
	enum EventType
	{
		kSlide,
		kContact,
		kEndSlide
	};

	enum EventStopType
	{
		kStopFromImpact = 0,
		kStopFromEndSlide = 2
	};

	class CollidePair
	{
	public:
		plKey firstPhysKey;
		plKey secondPhysKey;
		hsPoint3 point;
		hsVector3 normalForce;

		CollidePair(const plKey& firstPhys, const plKey& secondPhys, const hsPoint3& point, const hsVector3& normal);
		bool operator<(const CollidePair& rhs) const;
		bool operator==(const CollidePair& rhs) const;
		plPhysical* FirstPhysical() const;
		plPhysical* SecondPhysical() const;
	};

	void IStartCollision(CollidePair& cp);
	void IStopCollision(CollidePair& cp);
	void IUpdateCollision(CollidePair& cp);
	void IProcessSlide(plPhysicalSndGroup* sndA, plPhysicalSndGroup* sndB, hsScalar strength);
	
	typedef std::set<CollidePair> CollideSet;
	CollideSet fPrevCollisions;
	CollideSet fCurCollisions;
};

#endif // plPhysicsSoundMgr_h_inc