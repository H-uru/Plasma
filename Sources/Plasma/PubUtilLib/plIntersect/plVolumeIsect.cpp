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
#include "plVolumeIsect.h"
#include "hsBounds.h"
#include "hsFastMath.h"
#include "hsStream.h"
#include "hsResMgr.h"
#include "../plIntersect/plClosest.h"

static const hsScalar kDefLength = 5.f;

plSphereIsect::plSphereIsect()
:	fRadius(1.f)
{
	fCenter.Set(0,0,0);
	int i;
	for( i = 0; i < 3; i++ )
	{
		fMins[i] = -fRadius;
		fMaxs[i] =  fRadius;
	}
}

plSphereIsect::~plSphereIsect()
{
}

void plSphereIsect::SetCenter(const hsPoint3& c)
{
	fWorldCenter = fCenter = c;
	int i;
	for( i = 0; i < 3; i++ )
	{
		fMins[i] += c[i];
		fMaxs[i] += c[i];
	}
}

void plSphereIsect::SetRadius(hsScalar r)
{
	hsScalar del = r - fRadius;
	int i;
	for( i = 0; i < 3; i++ )
	{
		fMins[i] -= del;
		fMaxs[i] += del;
	}
	fRadius = r;
}

void plSphereIsect::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	fWorldCenter = l2w * fCenter;
	fMaxs = fMins = fWorldCenter;
	int i;
	for( i = 0; i < 3; i++ )
	{
		fMins[i] -= fRadius;
		fMaxs[i] += fRadius;
	}
}

// Could use ClosestPoint to find the closest point on the bounds
// to our center, and do a distance test on that. Would be more
// accurate than this box test approx, but whatever.
plVolumeCullResult plSphereIsect::Test(const hsBounds3Ext& bnd) const
{
	const hsPoint3& maxs = bnd.GetMaxs();
	const hsPoint3& mins = bnd.GetMins();

	if( (maxs.fX < fMins.fX)
		||
		(maxs.fY < fMins.fY)
		||
		(maxs.fZ < fMins.fZ) )
			return kVolumeCulled;

	if( (mins.fX > fMaxs.fX)
		||
		(mins.fY > fMaxs.fY)
		||
		(mins.fZ > fMaxs.fZ) )
			return kVolumeCulled;

	if( (maxs.fX > fMaxs.fX)
		||
		(maxs.fY > fMaxs.fY)
		||
		(maxs.fZ > fMaxs.fZ) )
			return kVolumeSplit;

	if( (mins.fX < fMins.fX)
		||
		(mins.fY < fMins.fY)
		||
		(mins.fZ < fMins.fZ) )
			return kVolumeSplit;

	return kVolumeClear;
}

hsScalar plSphereIsect::Test(const hsPoint3& pos) const
{
	hsScalar dist = (pos - fWorldCenter).MagnitudeSquared();
	if( dist < fRadius*fRadius )
		return 0;
	dist = hsSquareRoot(dist);
	return dist - fRadius;
}

void plSphereIsect::Read(hsStream* s, hsResMgr* mgr)
{
	fCenter.Read(s);
	fWorldCenter.Read(s);
	fRadius = s->ReadSwapScalar();
	fMins.Read(s);
	fMaxs.Read(s);
}

void plSphereIsect::Write(hsStream* s, hsResMgr* mgr)
{
	fCenter.Write(s);
	fWorldCenter.Write(s);
	s->WriteSwapScalar(fRadius);
	fMins.Write(s);
	fMaxs.Write(s);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

plConeIsect::plConeIsect()
:	fLength(kDefLength), fRadAngle(hsScalarPI*0.25f), fCapped(false)
{
	ISetup();
}

plConeIsect::~plConeIsect()
{
}

void plConeIsect::SetAngle(hsScalar rads)
{
	fRadAngle = rads;
	ISetup();
}

void plConeIsect::ISetup()
{
	hsScalar sinAng, cosAng;
	hsFastMath::SinCosInRangeAppr(fRadAngle, sinAng, cosAng);

	const hsScalar kHither = 0.1f;
	fLightToNDC.Reset();
	fLightToNDC.fMap[0][0] =   hsScalarDiv( cosAng, sinAng );
	fLightToNDC.fMap[1][1] =   hsScalarDiv( cosAng, sinAng );
	fLightToNDC.fMap[2][2] =	-hsScalarDiv( fLength, fLength - kHither );
	fLightToNDC.fMap[3][3] =	hsIntToScalar( 0 );
	fLightToNDC.fMap[3][2] =	hsIntToScalar( -1 );
	fLightToNDC.fMap[2][3] =	-hsScalarMulDiv( fLength, kHither, fLength - kHither );
	fLightToNDC.NotIdentity();
}

plVolumeCullResult plConeIsect::Test(const hsBounds3Ext& bnd) const
{
	plVolumeCullResult retVal = kVolumeClear;

	hsPoint2 depth;
	hsVector3 normDir = -fWorldNorm;
	bnd.TestPlane(normDir, depth);
	if( depth.fY < normDir.InnerProduct(fWorldTip) )
		return kVolumeCulled;

	int last = fCapped ? 5 : 4;
	int i;
	for( i = 0; i < last; i++ )
	{
		bnd.TestPlane(fNorms[i], depth);
		if( depth.fY + fDists[i] <= 0 )
			return kVolumeCulled;
		if( depth.fX + fDists[i] <= 0 )
			retVal = kVolumeSplit;
	}
	if( retVal == kVolumeSplit )
	{
		hsVector3 axis = normDir % hsVector3(&bnd.GetCenter(), &fWorldTip);
		hsFastMath::NormalizeAppr(axis);

		hsVector3 perp = axis % normDir;

		hsScalar sinAng, cosAng;
		hsFastMath::SinCosInRangeAppr(fRadAngle, sinAng, cosAng);

		hsVector3 tangent = normDir + sinAng * perp + (1-cosAng) * (axis % perp);

		hsVector3 normIn = tangent % axis;

		hsVector3 normIn2 = perp + sinAng * (perp % axis) + (1-cosAng) * (axis % (axis % perp));

		bnd.TestPlane(normIn, depth);
		hsScalar normInDotTip = normIn.InnerProduct(fWorldTip);
		if( depth.fY < normInDotTip )
			return kVolumeCulled;
	}

	return retVal;
}

hsScalar plConeIsect::Test(const hsPoint3& pos) const
{
	UInt32 clampFlags = fCapped ? plClosest::kClamp : plClosest::kClampLower;
	hsPoint3 cp;

	plClosest::PointOnLine(pos,
				  fWorldTip, fWorldNorm,
				  cp,
				  clampFlags);

	hsScalar radDist = (pos - cp).Magnitude();
	hsScalar axDist = fWorldNorm.InnerProduct(pos - fWorldTip) / fLength;
	if( axDist < 0 )
	{
		return radDist;
	}
	hsScalar sinAng, cosAng;

	hsFastMath::SinCosInRangeAppr(fRadAngle, sinAng, cosAng);

	hsScalar radius = axDist * sinAng / cosAng;

	radDist -= radius;
	axDist -= fLength;

	if( fCapped && (axDist > 0) )
	{
		return axDist > radDist ? axDist : radDist;
	}

	return radDist > 0 ? radDist : 0;
}

//#define MF_DEBUG_NORM
#ifdef MF_DEBUG_NORM
#define IDEBUG_NORMALIZE( a, b ) { hsScalar len = 1.f / a.Magnitude(); a *= len; b *= len; }
#else // MF_DEBUG_NORM
#define IDEBUG_NORMALIZE( a, b )
#endif // MF_DEBUG_NORM

void plConeIsect::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	fWorldTip = l2w.GetTranslate();
	fWorldNorm.Set(l2w.fMap[0][2], l2w.fMap[1][2], l2w.fMap[2][2]);

	fWorldToNDC = fLightToNDC * w2l;
	int i;
	for( i = 0; i < 2; i++ )
	{
		fNorms[i].Set(fWorldToNDC.fMap[3][0] - fWorldToNDC.fMap[i][0], fWorldToNDC.fMap[3][1] - fWorldToNDC.fMap[i][1], fWorldToNDC.fMap[3][2] - fWorldToNDC.fMap[i][2]);
		fDists[i] = fWorldToNDC.fMap[3][3] - fWorldToNDC.fMap[i][3];
		
		IDEBUG_NORMALIZE( fNorms[i], fDists[i] );

		fNorms[i+2].Set(fWorldToNDC.fMap[3][0] + fWorldToNDC.fMap[i][0], fWorldToNDC.fMap[3][1] + fWorldToNDC.fMap[i][1], fWorldToNDC.fMap[3][2] + fWorldToNDC.fMap[i][2]);
		fDists[i+2] = fWorldToNDC.fMap[3][3] + fWorldToNDC.fMap[i][3];

		IDEBUG_NORMALIZE( fNorms[i+2], fDists[i+2] );
	}

	if( fCapped )
	{
		fNorms[4].Set(fWorldToNDC.fMap[3][0] - fWorldToNDC.fMap[2][0], fWorldToNDC.fMap[3][1] - fWorldToNDC.fMap[2][1], fWorldToNDC.fMap[3][2] - fWorldToNDC.fMap[2][2]);
		fDists[4] = fWorldToNDC.fMap[3][3] - fWorldToNDC.fMap[2][3];

		IDEBUG_NORMALIZE( fNorms[4], fDists[4] );
	}
}

void plConeIsect::SetLength(hsScalar d)
{
	if( d > 0 )
	{
		fCapped = true;
		fLength = d;
	}
	else
	{
		fCapped = false;
		fLength = kDefLength;
	}
	ISetup();
}

void plConeIsect::Read(hsStream* s, hsResMgr* mgr)
{
	fCapped = s->ReadSwap32();

	fRadAngle = s->ReadSwapScalar();
	fLength = s->ReadSwapScalar();

	fWorldTip.Read(s);
	fWorldNorm.Read(s);

	fWorldToNDC.Read(s);
	fLightToNDC.Read(s);

	int n = fCapped ? 5 : 4;
	int i;
	for(i = 0; i < n; i++ )
	{
		fNorms[i].Read(s);
		fDists[i] = s->ReadSwapScalar();
	}
}

void plConeIsect::Write(hsStream* s, hsResMgr* mgr)
{
	s->WriteSwap32(fCapped);

	s->WriteSwapScalar(fRadAngle);
	s->WriteSwapScalar(fLength);

	fWorldTip.Write(s);
	fWorldNorm.Write(s);

	fWorldToNDC.Write(s);
	fLightToNDC.Write(s);

	int n = fCapped ? 5 : 4;
	int i;
	for(i = 0; i < n; i++ )
	{
		fNorms[i].Write(s);
		s->WriteSwapScalar(fDists[i]);
	}
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

plCylinderIsect::plCylinderIsect()
{
}

plCylinderIsect::~plCylinderIsect()
{
}

void plCylinderIsect::ISetupCyl(const hsPoint3& wTop, const hsPoint3& wBot, hsScalar radius)
{
	fWorldNorm.Set(&wTop, &wBot);
	fLength = fWorldNorm.Magnitude();
	fMin = fWorldNorm.InnerProduct(wBot);
	fMax = fWorldNorm.InnerProduct(wTop);
	if( fMin > fMax )
	{
		hsScalar t = fMin;
		fMin = fMax;
		fMax = t;
	}
	fRadius = radius;
}

void plCylinderIsect::SetCylinder(const hsPoint3& lTop, const hsPoint3& lBot, hsScalar radius)
{
	fTop = lTop;
	fBot = lBot;
	fRadius = radius;

	ISetupCyl(fTop, fBot, fRadius);
}

void plCylinderIsect::SetCylinder(const hsPoint3& lBot, const hsVector3& axis, hsScalar radius)
{
	fBot = lBot;
	fTop = fBot;
	fTop += axis;

	ISetupCyl(fTop, fBot, radius);

}

void plCylinderIsect::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	hsPoint3 wTop = l2w * fTop;
	hsPoint3 wBot = l2w * fBot;

	ISetupCyl(wTop, wBot, fRadius);
}

plVolumeCullResult plCylinderIsect::Test(const hsBounds3Ext& bnd) const
{
	plVolumeCullResult radVal = kVolumeClear;

	// Central axis test
	hsPoint2 depth;
	bnd.TestPlane(fWorldNorm, depth);
	if( depth.fX > fMax )
		return kVolumeCulled;
	if( depth.fY < fMin )
		return kVolumeCulled;

	if( (depth.fX < fMin)
		||(depth.fY > fMax) )
	{
		radVal = kVolumeSplit;
	}

	// Radial test
	plVolumeCullResult retVal = kVolumeCulled;

	// Find the closest point on/in the bounds to our central axis.
	// If that closest point is inside the cylinder, we have a hit.
	hsPoint3 corner;
	bnd.GetCorner(&corner);
	hsVector3 axes[3];
	bnd.GetAxes(axes+0, axes+1, axes+2);
	hsPoint3 cp = corner;

	hsScalar bndRadiusSq = bnd.GetRadius();
	bndRadiusSq *= bndRadiusSq;

	hsScalar radiusSq = fRadius*fRadius;

	hsScalar maxClearDistSq = fRadius - bnd.GetRadius();
	maxClearDistSq *= maxClearDistSq;

	int i;
	for( i = 0; i < 3; i++ )
	{
		hsPoint3 cp0;
		hsPoint3 currPt;
		plClosest::PointsOnLines(fWorldBot, fWorldNorm, 
					  cp, axes[i],
					  cp0, currPt,
					  plClosest::kClamp);
		hsScalar distSq = (cp0 - currPt).MagnitudeSquared();
		if( distSq < radiusSq )
		{
			if( distSq < maxClearDistSq )
			{
				return kVolumeClear == radVal ? kVolumeClear : kVolumeSplit;
			}
			retVal = kVolumeSplit;
		}
		cp = currPt;
	}

	return retVal;
}

hsScalar plCylinderIsect::Test(const hsPoint3& pos) const
{
	hsPoint3 cp;

	plClosest::PointOnLine(pos,
				  fWorldBot, fWorldNorm,
				  cp,
				  plClosest::kClamp);

	hsScalar radDist = (pos - cp).Magnitude() - fRadius;
	hsScalar axDist = fWorldNorm.InnerProduct(pos - fWorldBot) / fLength;

	if( axDist < 0 )
		axDist = -axDist;
	else
		axDist -= fLength;

	hsScalar dist = axDist > radDist ? axDist : radDist;
	
	return dist > 0 ? dist : 0;
}

void plCylinderIsect::Read(hsStream* s, hsResMgr* mgr)
{
	fTop.Read(s);
	fBot.Read(s);
	fRadius = s->ReadSwapScalar();

	fWorldBot.Read(s);
	fWorldNorm.Read(s);
	fLength = s->ReadSwapScalar();
	fMin = s->ReadSwapScalar();
	fMax = s->ReadSwapScalar();
}

void plCylinderIsect::Write(hsStream* s, hsResMgr* mgr)
{
	fTop.Write(s);
	fBot.Write(s);
	s->WriteSwapScalar(fRadius);

	fWorldBot.Write(s);
	fWorldNorm.Write(s);
	s->WriteSwapScalar(fLength);
	s->WriteSwapScalar(fMin);
	s->WriteSwapScalar(fMax);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

plParallelIsect::plParallelIsect()
{
}

plParallelIsect::~plParallelIsect()
{
}

void plParallelIsect::SetNumPlanes(int n)
{
	fPlanes.SetCount(n);
}

void plParallelIsect::SetPlane(int which, const hsPoint3& locPosOne, const hsPoint3& locPosTwo)
{
	fPlanes[which].fPosOne = locPosOne;
	fPlanes[which].fPosTwo = locPosTwo;
}

void plParallelIsect::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	int i;
	for( i = 0; i < fPlanes.GetCount(); i++ )
	{
		hsPoint3 wPosOne = l2w * fPlanes[i].fPosOne;
		hsPoint3 wPosTwo = l2w * fPlanes[i].fPosTwo;
		hsVector3 norm;
		norm.Set(&wPosOne, &wPosTwo);
		fPlanes[i].fNorm = norm;
		hsScalar t0 = norm.InnerProduct(wPosOne);
		hsScalar t1 = norm.InnerProduct(wPosTwo);

		if( t0 > t1 )
		{
			fPlanes[i].fMin = t1;
			fPlanes[i].fMax = t0;
		}
		else
		{
			fPlanes[i].fMin = t0;
			fPlanes[i].fMax = t1;
		}
	}
}

plVolumeCullResult plParallelIsect::Test(const hsBounds3Ext& bnd) const
{
	plVolumeCullResult retVal = kVolumeClear;
	int i;
	for( i = 0; i < fPlanes.GetCount(); i++ )
	{
		hsPoint2 depth;
		bnd.TestPlane(fPlanes[i].fNorm, depth);
		if( depth.fY < fPlanes[i].fMin )
			return kVolumeCulled;
		if( depth.fX > fPlanes[i].fMax )
			return kVolumeCulled;
		if( depth.fX < fPlanes[i].fMin )
			retVal = kVolumeSplit;
		if( depth.fY > fPlanes[i].fMax )
			retVal = kVolumeSplit;
	}
	return retVal;
}

hsScalar plParallelIsect::Test(const hsPoint3& pos) const
{
	hsScalar maxDist = 0;
	int i;
	for( i = 0; i < fPlanes.GetCount(); i++ )
	{
		hsScalar dist = fPlanes[i].fNorm.InnerProduct(pos);

		if( dist > fPlanes[i].fMax )
		{
			dist -= fPlanes[i].fMax;
			dist *= hsFastMath::InvSqrtAppr(fPlanes[i].fNorm.MagnitudeSquared());
			if( dist > maxDist )
				maxDist = dist;
		}
		else if( dist < fPlanes[i].fMin )
		{
			dist = fPlanes[i].fMin - dist;
			dist *= hsFastMath::InvSqrtAppr(fPlanes[i].fNorm.MagnitudeSquared());
			if( dist > maxDist )
				maxDist = dist;
		}

	}
	return maxDist;
}

void plParallelIsect::Read(hsStream* s, hsResMgr* mgr)
{
	int n = s->ReadSwap16();

	fPlanes.SetCount(n);
	int i;
	for( i = 0; i < n; i++ )
	{
		fPlanes[i].fNorm.Read(s);
		fPlanes[i].fMin = s->ReadSwapScalar();
		fPlanes[i].fMax = s->ReadSwapScalar();

		fPlanes[i].fPosOne.Read(s);
		fPlanes[i].fPosTwo.Read(s);
	}
}

void plParallelIsect::Write(hsStream* s, hsResMgr* mgr)
{
	s->WriteSwap16(fPlanes.GetCount());

	int i;
	for( i = 0; i < fPlanes.GetCount(); i++ )
	{
		fPlanes[i].fNorm.Write(s);
		s->WriteSwapScalar(fPlanes[i].fMin);
		s->WriteSwapScalar(fPlanes[i].fMax);

		fPlanes[i].fPosOne.Write(s);
		fPlanes[i].fPosTwo.Write(s);
	}
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

plConvexIsect::plConvexIsect()
{
}

plConvexIsect::~plConvexIsect()
{
}

void plConvexIsect::AddPlaneUnchecked(const hsVector3& n, hsScalar dist)
{
	SinglePlane plane;
	plane.fNorm = n;
	plane.fPos.Set(0,0,0);
	plane.fDist = dist;

	fPlanes.Append(plane);
}

void plConvexIsect::AddPlane(const hsVector3& n, const hsPoint3& p)
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
			hsScalar dist = nNorm.InnerProduct(p);
			if( dist > fPlanes[i].fDist )
			{
				fPlanes[i].fDist = dist;
				fPlanes[i].fPos = p;
			}
			return;
		}
	}
	SinglePlane plane;
	plane.fNorm = nNorm;
	plane.fPos = p;
	plane.fDist = nNorm.InnerProduct(p);

	fPlanes.Append(plane);
}

void plConvexIsect::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	int i;
	for( i = 0; i < fPlanes.GetCount(); i++ )
	{
		hsPoint3 wPos = l2w * fPlanes[i].fPos;

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

		fPlanes[i].fWorldDist = fPlanes[i].fWorldNorm.InnerProduct(wPos);
	}
}

plVolumeCullResult plConvexIsect::Test(const hsBounds3Ext& bnd) const
{
	plVolumeCullResult retVal = kVolumeClear;
	int i;
	for( i = 0; i < fPlanes.GetCount(); i++ )
	{
		hsPoint2 depth;
		bnd.TestPlane(fPlanes[i].fWorldNorm, depth);

		if( depth.fX > fPlanes[i].fWorldDist )
			return kVolumeCulled;

		if( depth.fY > fPlanes[i].fWorldDist )
			retVal = kVolumeSplit;
	}
	return retVal;
}

hsScalar plConvexIsect::Test(const hsPoint3& pos) const
{
	hsScalar maxDist = 0;
	int i;
	for( i = 0; i < fPlanes.GetCount(); i++ )
	{
		hsScalar dist = fPlanes[i].fWorldNorm.InnerProduct(pos) - fPlanes[i].fWorldDist;

		if( dist > maxDist )
			maxDist = dist;
	}
	return maxDist;
}

void plConvexIsect::Read(hsStream* s, hsResMgr* mgr)
{
	Int16 n = s->ReadSwap16();

	fPlanes.SetCount(n);
	int i;
	for( i = 0; i < n; i++ )
	{
		fPlanes[i].fNorm.Read(s);
		fPlanes[i].fPos.Read(s);
		fPlanes[i].fDist = s->ReadSwapScalar();

		fPlanes[i].fWorldNorm.Read(s);
		fPlanes[i].fWorldDist = s->ReadSwapScalar();
	}
}

void plConvexIsect::Write(hsStream* s, hsResMgr* mgr)
{
	s->WriteSwap16(fPlanes.GetCount());

	int i;
	for( i = 0; i < fPlanes.GetCount(); i++ )
	{
		fPlanes[i].fNorm.Write(s);
		fPlanes[i].fPos.Write(s);
		s->WriteSwapScalar(fPlanes[i].fDist);

		fPlanes[i].fWorldNorm.Write(s);
		s->WriteSwapScalar(fPlanes[i].fWorldDist);
	}
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
plBoundsIsect::plBoundsIsect()
{
}

plBoundsIsect::~plBoundsIsect()
{
}

void plBoundsIsect::SetBounds(const hsBounds3Ext& bnd) 
{ 
	fLocalBounds = bnd; 
	fWorldBounds = bnd;
}

void plBoundsIsect::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	fWorldBounds = fLocalBounds;
	fWorldBounds.Transform(&l2w);
}

plVolumeCullResult plBoundsIsect::Test(const hsBounds3Ext& bnd) const
{
	int retVal = fWorldBounds.TestBound(bnd);
	if( retVal < 0 )
		return kVolumeCulled;
	if( retVal > 0 )
		return kVolumeClear;

	retVal = bnd.TestBound(fWorldBounds);

	return retVal < 0 ? kVolumeCulled : kVolumeSplit;	
}

hsScalar plBoundsIsect::Test(const hsPoint3& pos) const
{
	hsAssert(false, "Unimplemented");
	return 0.f;
}

void plBoundsIsect::Read(hsStream* s, hsResMgr* mgr)
{
	fLocalBounds.Read(s);
	fWorldBounds.Read(s);
}

void plBoundsIsect::Write(hsStream* s, hsResMgr* mgr)
{
	fLocalBounds.Write(s);
	fWorldBounds.Write(s);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

plComplexIsect::plComplexIsect()
{
}

plComplexIsect::~plComplexIsect()
{
	int i;
	for( i = 0; i < fVolumes.GetCount(); i++ )
		delete fVolumes[i];
}

void plComplexIsect::AddVolume(plVolumeIsect* v)
{
	fVolumes.Append(v);
}

void plComplexIsect::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	int i;
	for( i = 0; i < fVolumes.GetCount(); i++ )
		fVolumes[i]->SetTransform(l2w, w2l);
}

void plComplexIsect::Read(hsStream* s, hsResMgr* mgr)
{
	int n = s->ReadSwap16();
	fVolumes.SetCount(n);
	int i;
	for( i = 0; i < n; i++ )
	{
		fVolumes[i] = plVolumeIsect::ConvertNoRef(mgr->ReadCreatable(s));
		hsAssert(fVolumes[i], "Failure reading in a sub-volume");
	}
}

void plComplexIsect::Write(hsStream* s, hsResMgr* mgr)
{
	s->WriteSwap16(fVolumes.GetCount());
	int i;
	for( i = 0; i < fVolumes.GetCount(); i++ )
		mgr->WriteCreatable(s, fVolumes[i]);
}



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

plUnionIsect::plUnionIsect()
{
}

plUnionIsect::~plUnionIsect()
{
}

plVolumeCullResult plUnionIsect::Test(const hsBounds3Ext& bnd) const
{
	plVolumeCullResult retVal = kVolumeCulled;
	int i;
	for( i = 0; i < fVolumes.GetCount(); i++ )
	{
		plVolumeCullResult ret = fVolumes[i]->Test(bnd);

		switch( ret )
		{
		case kVolumeCulled:
			break;
		case kVolumeClear:
			return kVolumeClear;
		case kVolumeSplit:
			retVal = kVolumeSplit;
			break;
		};
	}
	return retVal;
}

hsScalar plUnionIsect::Test(const hsPoint3& pos) const
{
	hsScalar retVal = 1.e33f;
	int i;
	for( i = 0; i < fVolumes.GetCount(); i++ )
	{
		hsScalar ret = fVolumes[i]->Test(pos);
		if( ret <= 0 )
			return 0;
		if( ret < retVal )
			retVal = ret;
	}
	return retVal;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

plIntersectionIsect::plIntersectionIsect()
{
}

plIntersectionIsect::~plIntersectionIsect()
{
}

plVolumeCullResult plIntersectionIsect::Test(const hsBounds3Ext& bnd) const
{
	plVolumeCullResult retVal = kVolumeClear;
	int i;
	for( i = 0; i < fVolumes.GetCount(); i++ )
	{
		plVolumeCullResult ret = fVolumes[i]->Test(bnd);

		switch( ret )
		{
		case kVolumeCulled:
			return kVolumeCulled;
		case kVolumeClear:
			break;
		case kVolumeSplit:
			retVal = kVolumeSplit;
			break;
		};
	}
	return retVal;
}

hsScalar plIntersectionIsect::Test(const hsPoint3& pos) const
{
	hsScalar retVal = -1.f;
	int i;
	for( i = 0; i < fVolumes.GetCount(); i++ )
	{
		hsScalar ret = fVolumes[i]->Test(pos);
		if( ret > retVal )
			retVal = ret;
	}
	return retVal;
}

