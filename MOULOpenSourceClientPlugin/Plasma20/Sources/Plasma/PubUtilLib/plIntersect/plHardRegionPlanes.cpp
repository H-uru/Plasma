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

#include "hsTypes.h"
#include "plHardRegionPlanes.h"

#include "hsStream.h"
#include "hsGeometry3.h"
#include "hsFastMath.h"
#include "hsMatrix44.h"


plHardRegionPlanes::plHardRegionPlanes()
{
}

plHardRegionPlanes::~plHardRegionPlanes()
{
}

hsBool plHardRegionPlanes::IIsInside(const hsPoint3& pos) const
{
	int i;
	for( i = 0; i < fPlanes.GetCount(); i++ )
	{
		if( fPlanes[i].fWorldNorm.InnerProduct(pos) > fPlanes[i].fWorldDist )
			return false;
	}
	return true;
}

hsBool plHardRegionPlanes::ICameraInside() const
{
	return IIsInside(fCamPos);
}

void plHardRegionPlanes::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	int i;
	for( i = 0; i < fPlanes.GetCount(); i++ )
	{
		fPlanes[i].fWorldPos = l2w * fPlanes[i].fPos;

		// Normal gets transpose of inverse.
		fPlanes[i].fWorldNorm.fX = w2l.fMap[0][0] * fPlanes[i].fNorm.fX
									+ w2l.fMap[1][0] * fPlanes[i].fNorm.fY
									+ w2l.fMap[2][0] * fPlanes[i].fNorm.fZ;

		fPlanes[i].fWorldNorm.fY = w2l.fMap[0][1] * fPlanes[i].fNorm.fX
									+ w2l.fMap[1][1] * fPlanes[i].fNorm.fY
									+ w2l.fMap[2][1] * fPlanes[i].fNorm.fZ;

		fPlanes[i].fWorldNorm.fZ = w2l.fMap[0][2] * fPlanes[i].fNorm.fX
									+ w2l.fMap[1][2] * fPlanes[i].fNorm.fY
									+ w2l.fMap[2][2] * fPlanes[i].fNorm.fZ;

		hsFastMath::NormalizeAppr(fPlanes[i].fWorldNorm);

		fPlanes[i].fWorldDist = fPlanes[i].fWorldNorm.InnerProduct(fPlanes[i].fWorldPos);
	}
}

void plHardRegionPlanes::Read(hsStream* s, hsResMgr* mgr)
{
	plHardRegion::Read(s, mgr);

	int n = s->ReadSwap32();
	fPlanes.SetCount(n);

	int i;
	for( i = 0; i < n; i++ )
	{
		fPlanes[i].fNorm.Read(s);
		fPlanes[i].fPos.Read(s);

		fPlanes[i].fWorldNorm.Read(s);
		fPlanes[i].fWorldPos.Read(s);

		fPlanes[i].fWorldDist = fPlanes[i].fWorldNorm.InnerProduct(fPlanes[i].fWorldPos);
	}
}

void plHardRegionPlanes::Write(hsStream* s, hsResMgr* mgr)
{
	plHardRegion::Write(s, mgr);

	s->WriteSwap32(fPlanes.GetCount());

	int i;
	for( i = 0; i < fPlanes.GetCount(); i++ )
	{
		fPlanes[i].fNorm.Write(s);
		fPlanes[i].fPos.Write(s);

		fPlanes[i].fWorldNorm.Write(s);
		fPlanes[i].fWorldPos.Write(s);
	}
}

void plHardRegionPlanes::AddPlane(const hsVector3& n, const hsPoint3& p)
{
	hsVector3 nNorm = n;
	hsFastMath::Normalize(nNorm);

	// First, make sure some idiot isn't adding the same plane in twice.
	// Also, look for the degenerate case of two parallel planes. In that
	// case, take the outer.
	int i;
	for( i = 0; i < fPlanes.GetCount(); i++ )
	{
		const hsScalar kCloseToOne = 1.f - 1.e-4f;
		if( fPlanes[i].fNorm.InnerProduct(nNorm) >= kCloseToOne )
		{
			hsScalar newDist = nNorm.InnerProduct(p);
			hsScalar oldDist = fPlanes[i].fNorm.InnerProduct(fPlanes[i].fPos);
			if( newDist > oldDist )
			{
				fPlanes[i].fPos = p;
			}
			return;
		}
	}
	HardPlane plane;
	plane.fWorldNorm = plane.fNorm = nNorm;
	plane.fWorldPos = plane.fPos = p;
	plane.fWorldDist = plane.fWorldNorm.InnerProduct(plane.fWorldPos);

	fPlanes.Append(plane);
}
