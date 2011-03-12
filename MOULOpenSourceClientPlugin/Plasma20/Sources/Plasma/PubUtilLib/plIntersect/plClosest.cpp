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
#include "hsGeometry3.h"
#include "plClosest.h"
#include "hsFastMath.h"


static const hsScalar kRealSmall = 1.e-5f;

// Find the closest point on a line (or segment) to a point.
UInt32 plClosest::PointOnLine(const hsPoint3& p0,
				  const hsPoint3& p1, const hsVector3& v1,
				  hsPoint3& cp,
				  UInt32 clamp)
{
	hsScalar invV1Sq = v1.MagnitudeSquared();
	// v1 is also zero length. The two input points are the only options for output.
	if( invV1Sq < kRealSmall )
	{
		cp = p1;
		return kClamp;
	}
	hsScalar t = v1.InnerProduct(p0 - p1) / invV1Sq;
	cp = p1;
	// clamp to the ends of segment v1.
	if( (clamp & kClampLower1) && (t < 0) )
	{
		return kClampLower1;
	}
	if( (clamp & kClampUpper1) && (t > 1.f) )
	{
		cp += v1;
		return kClampUpper1;
	}

	cp += v1 * t;
	return 0;
}

// Find closest points to each other from two lines (or segments).
UInt32 plClosest::PointsOnLines(const hsPoint3& p0, const hsVector3& v0, 
				  const hsPoint3& p1, const hsVector3& v1,
				  hsPoint3& cp0, hsPoint3& cp1,
				  UInt32 clamp)
{
	hsScalar invV0Sq = v0.MagnitudeSquared();
	// First handle degenerate cases.
	// v0 is zero length. Resolves to finding closest point on p1+v1 to p0
	if( invV0Sq < kRealSmall )
	{
		cp0 = p0;
		return kClamp0 | PointOnLine(p0, p1, v1, cp1, clamp);
	}
	invV0Sq = 1.f / invV0Sq;

	// The real thing here, two non-zero length segments. (v1 can
	// be zero length, it doesn't affect the math like |v0|=0 does,
	// so we don't even bother to check. Only means maybe doing extra
	// work, since we're using segment-segment math when all we really
	// need is point-segment.)

	// The parameterized points for along each of the segments are
	// P(t0) = p0 + v0*t0
	// P(t1) = p1 + v1*t1
	//
	// The closest point on p0+v0 to P(t1) is:
	//	cp0 = p0 + ((P(t1) - p0) dot v0) * v0 / ||v0||	||x|| is mag squared here
	//	cp0 = p0 + v0*t0 => t0 = ((P(t1) - p0) dot v0 ) / ||v0||
	//						t0 = ((p1 + v1*t1 - p0) dot v0) / ||v0||
	//
	//	The distance squared from P(t1) to cp0 is:
	//	(cp0 - P(t1)) dot (cp0 - P(t1))
	//
	//	This expands out to:
	//
	//	CV0 dot CV0 + 2 CV0 dot DV0 * t1 + (DV0 dot DV0) * t1^2
	//
	//	where
	//
	//	CV0 = p0 - p1 + ((p1 - p0) dot v0) / ||v0||) * v0 == vector from p1 to closest point on p0+v0
	//  and
	//	DV0 = ((v1 dot v0) / ||v0||) * v0 - v1 == ortho divergence vector of v1 from v0 negated.
	//
	//	Taking the first derivative to find the local minimum of the function gives
	//
	//	t1 = - (CV0 dot DV0) / (DV0 dot DV0)
	//	and
	//	t0 = ((p1 - v1 * t1 - p0) dot v0) / ||v0|| 
	//
	// which seems kind of obvious in retrospect.

	hsVector3 p0subp1(&p0, &p1);

	hsVector3 CV0 = p0subp1;
	CV0 += v0 * p0subp1.InnerProduct(v0) * -invV0Sq;
	
	hsVector3 DV0 = v0 * (v1.InnerProduct(v0) * invV0Sq) - v1;
	
	// Check for the vectors v0 and v1 being parallel, in which case
	// following the lines won't get us to any closer point.
	hsScalar DV0dotDV0 = DV0.InnerProduct(DV0);
	if( DV0dotDV0 < kRealSmall )
	{
		// If neither is clamped, return any two corresponding points.
		// If one is clamped, return closest points in its clamp range.
		// If both are clamped, well, both are clamped. The distance between
		//		points will no longer be the distance between lines.
		// In any case, the distance between the points should be correct.
		UInt32 clamp1 = PointOnLine(p0, p1, v1, cp1, clamp);
		UInt32 clamp0 = PointOnLine(cp1, p0, v0, cp0, clamp >> 1);
		return clamp1 | (clamp0 << 1);
	}

	UInt32 retVal = 0;

	hsScalar t1 = - (CV0.InnerProduct(DV0)) / DV0dotDV0;
	if( (clamp & kClampLower1) && (t1 <= 0) )
	{
		t1 = 0;
		retVal |= kClampLower1;
	}
	else if( (clamp & kClampUpper1) && (t1 >= 1.f) )
	{
		t1 = 1.f;
		retVal |= kClampUpper1;
	}

	hsScalar t0 = v0.InnerProduct(p0subp1 - v1 * t1) * -invV0Sq;
	cp0 = p0;
	if( (clamp & kClampUpper0) && (t0 >= 1.f) )
	{
		cp0 += v0;
		retVal |= kClampUpper0;
	}
	else if( !(clamp & kClampLower0) || (t0 > 0) )
	{
		cp0 += v0 * t0;
	}
	else
	{
		retVal |= kClampLower0;
	}

	// If we clamped t0, we need to recalc t1 because the original
	// calculation of t1 was based on an infinite p0+v0.
	if( retVal & kClamp0 )
	{
		t1 = v1.InnerProduct(cp0 - p1) / v1.MagnitudeSquared();
		retVal &= ~kClamp1;
		if( (clamp & kClampLower1) && (t1 <= 0) )
		{
			t1 = 0;
			retVal |= kClampLower1;
		}
		else if( (clamp & kClampUpper1) && (t1 >= 1.f) )
		{
			t1 = 1.f;
			retVal |= kClampUpper1;
		}
	}

	cp1 = p1;
	cp1 += v1 * t1;

	return retVal;;
}

hsBool plClosest::PointOnSphere(const hsPoint3& p0,
								const hsPoint3& center, hsScalar rad,
								hsPoint3& cp)
{
	hsVector3 del(&p0, &center);
	hsScalar dist = hsFastMath::InvSqrtAppr(del.MagnitudeSquared());
	dist *= rad;
	del *= dist;
	cp = center;
	cp += del;
	return dist <= 1.f;
}

hsBool plClosest::PointOnBox(const hsPoint3& p0,
							 const hsPoint3& corner,
							 const hsVector3& axis0,
							 const hsVector3& axis1,
							 const hsVector3& axis2,
							 hsPoint3& cp)
{
	UInt32 clamps = 0;
	hsPoint3 currPt = corner;
	clamps |= PointOnLine(p0, currPt, axis0, cp, kClamp);
	currPt = cp;
	clamps |= PointOnLine(p0, currPt, axis1, cp, kClamp);
	currPt = cp;
	clamps |= PointOnLine(p0, currPt, axis2, cp, kClamp);

	return !clamps;
}

hsBool plClosest::PointOnSphere(const hsPoint3& p0, const hsVector3& v0,
							const hsPoint3& center, hsScalar rad,
							hsPoint3& cp,
							UInt32 clamp)
{
	// Does the line hit the sphere? If it does, we return the entry point in cp,
	// otherwise we find the closest point on the sphere to the line.
	/*
	((p0 + v0*t) - center)^2 = rad
	v0*v0 * t*t + 2 * v0*t * (p0-c) + (p0-c)^2 - rad = 0

	t = (-2 * v0*(p0-c) +- sqrt(4 * (v0*(p0-c))^2 - 4 * v0*v0 * ((p0-c)^2 - rad) / 2 * v0 * v0

	t = (-v0*(p0-c) +- sqrt((v0*(p0-c))^2 - v0*v0 * ((p0-c)^2 - rad) / v0 * v0

	So, line hits the sphere if
	(v0*(p0-c))^2 > v0*v0 * ((p0-c)^2 - rad)

		If clamped, need additional checks on t before returning true

	If line doesn't hit the sphere, we find the closest point on the line
	to the center of the sphere, and return the intersection of the segment
	connecting that point and the center with the sphere.
	*/
	hsScalar termA = v0.InnerProduct(v0);
	if( termA < kRealSmall )
	{
		return PointOnSphere(p0, center, rad, cp);
	}
	hsVector3 p0Subc(&p0, &center);
	hsScalar termB = v0.InnerProduct(p0Subc);
	hsScalar termC = p0Subc.InnerProduct(p0Subc) - rad;
	hsScalar disc = termB * termB - 4 * termA * termC;
	if( disc >= 0 )
	{
		disc = hsSquareRoot(disc);
		hsScalar t = (-termB - disc) / (2.f * termA);
		if( (t < 0) && (clamp & kClampLower0) )
		{
			hsScalar tOut = (-termB + disc) / (2.f * termA);
			if( tOut < 0 )
			{
				// Both isects are before beginning of clamped line.
				cp = p0;
				cp += v0 * tOut;
				return false;
			}
			if( (tOut > 1.f) && (clamp & kClampUpper0) )
			{
				// The segment is entirely within the sphere. Take the closer end.
				if( -t < tOut - 1.f )
				{
					cp = p0;
					cp += v0 * t;
				}
				else
				{
					cp = p0;
					cp += v0 * tOut;
				}
				return true;
			}
			// We pierce the sphere from inside.
			cp = p0;
			cp += v0 * tOut;
			return true;
		}
		cp = p0;
		cp += v0 * t;
		if( (t > 1.f) && (clamp & kClampUpper0) )
		{
			return false;
		}
		return true;
	}

	// Okay, missed the sphere, find closest point.
	hsPoint3 lp;
	PointOnLine(center, p0, v0, lp, clamp);
	PointOnSphere(lp, center, rad, cp);

	return false;
}

hsBool plClosest::PointOnBox(const hsPoint3& p0, const hsVector3& v0,	
							const hsPoint3& corner,						
							const hsVector3& axis0,						
							const hsVector3& axis1,
							const hsVector3& axis2,
							hsPoint3& cp,
							UInt32 clamp)
{
	UInt32 clampRes = 0;

	hsPoint3 cp0, cp1;
	hsPoint3 currPt = corner;

	clampRes |= PointsOnLines(p0, v0, currPt, axis0, cp0, cp1, clamp);
	currPt = cp1;

	clampRes |= PointsOnLines(p0, v0, currPt, axis1, cp0, cp1, clamp);
	currPt = cp1;

	clampRes |= PointsOnLines(p0, v0, currPt, axis2, cp0, cp1, clamp);
	currPt = cp1;

	return !clampRes;
}

hsBool plClosest::PointOnPlane(const hsPoint3& p0,
							   const hsPoint3& pPln, const hsVector3& n,
							   hsPoint3& cp)
{
	/*
		p' = p - ((p-pPln)*n)/|n| * n/|n|
		p' = p + ((pPln-p)*n) * n / |n|^2
	*/
	hsScalar invNLen = hsFastMath::InvSqrt(n.MagnitudeSquared());

	hsScalar nDotp = n.InnerProduct(pPln - p0);
	cp = p0 + n * (nDotp * invNLen);

	return nDotp >= 0;
}

hsBool plClosest::PointOnPlane(const hsPoint3& p0, const hsVector3& v0,
							   const hsPoint3& pPln, const hsVector3& n,
							   hsPoint3& cp,
							   UInt32 clamp)
{
	/*
		p0 + v0*t is on plane, i.e.
		(p0 + v0*t) * n = pPln * n

		p0 * n + v0 * n * t = pPln * n
		v0 * n * t = (pPln - p0) * n
		t = (pPln - p0) * n / (v0 * n)

		Then clamp appropriately, garnish, and serve with wild rice.
	*/
	hsBool retVal = true;
	hsScalar pDotn = n.InnerProduct(pPln - p0);
	hsScalar v0Dotn = n.InnerProduct(v0);
	if( (v0Dotn < -kRealSmall) || (v0Dotn > kRealSmall) )
	{
		hsScalar t = pDotn / v0Dotn;

		if( (clamp & kClampLower) && (t < 0) )
		{
			t = 0;
			retVal = false;
		}
		else if( (clamp & kClampUpper) && (t > 1.f) )
		{
			t = 1.f;
			retVal = false;
		}
		cp = p0;
		cp += v0 * t;

	}
	else
	{
		cp = p0 + v0 * 0.5f;
		retVal = (pDotn > -kRealSmall) && (pDotn < kRealSmall);
	}

	return retVal;
}

hsBool plClosest::PointBetweenBoxes(const hsPoint3& aCorner,
									const hsVector3& aAxis0,
									const hsVector3& aAxis1,
									const hsVector3& aAxis2,
									const hsPoint3& bCorner,
									const hsVector3& bAxis0,
									const hsVector3& bAxis1,
									const hsVector3& bAxis2,
									hsPoint3& cp0, hsPoint3& cp1)
{
	const hsVector3*	aAxes[3] = { &aAxis0, &aAxis1, &aAxis2 };
	const hsVector3*	bAxes[3] = { &bAxis0, &bAxis1, &bAxis2 };

	return PointBetweenBoxes(aCorner, aAxes, bCorner, bAxes, cp0, cp1);
}

#if 0 // TRASH THIS
hsBool plClosest::PointBetweenBoxes(const hsPoint3& aCorner,
									const hsVector3* aAxes[3],
									const hsPoint3& bCorner,
									const hsVector3* bAxes[3],
									hsPoint3& cp0, hsPoint3& cp1)
{
	hsPoint3 aCurrPt = aCorner;
	hsPoint3 bCurrPt = bCorner;

	hsPoint3 bStartPt[3];
	bStartPt[0] = bStartPt[1] = bStartPt[2] = bCorner;

	hsBool retVal = true;
	int i, j;
	for( i = 0; i < 3; i++ )
	{
		hsPoint3 aBestPt;
		hsPoint3 bBestPt;

		hsScalar minDistSq = 1.e33f;
		for( j = 0; j < 3; j++ )
		{
			hsPoint3 aNextPt, bNextPt;
			PointsOnLines(aCurrPt, *aAxes[i],
										bStartPt[j], *bAxes[j],
										aNextPt, bNextPt,
										plClosest::kClamp);

			hsScalar distSq = hsVector3(&aNextPt, &bNextPt).MagnitudeSquared();
			if( distSq < minDistSq )
			{
				aBestPt = aNextPt;
				bBestPt = bNextPt;

				if( distSq < kRealSmall )
					retVal = true;

				minDistSq = distSq;
			}
			hsVector3 bMove(&bNextPt, &bStartPt[j]);
			int k;
			for( k = 0; k < 3; k++ )
			{
				if( k != j )
					bStartPt[k] += bMove;
			}
		}
		aCurrPt = aBestPt;
		bCurrPt = bBestPt;
	}
	cp0 = aCurrPt;
	cp1 = bCurrPt;

	return retVal;
}
#elif 0 // TRASH THIS

hsBool plClosest::PointBetweenBoxes(const hsPoint3& aCorner,
									const hsVector3* aAxes[3],
									const hsPoint3& bCorner,
									const hsVector3* bAxes[3],
									hsPoint3& cp0, hsPoint3& cp1)
{
	/*
		Six combinations to try to go through every possible
		combination of axes from A and B

			00 00  01 01  02 02
			11 12  12 10  10 11
			22 21  20 22  21 20
	*/

	int bIdx0 = 0;
	int bIdx1 = 1;
	int bIdx2 = 2;

	hsPoint3 aBestPt, bBestPt;
	hsScalar minDistSq = 1.e33f;

	hsBool retVal = false;

	int i;
	for( i = 0; i < 6; i++ )
	{
		hsPoint3 aCurrPt = aCorner;
		hsPoint3 bCurrPt = bCorner;

		hsPoint3 aNextPt, bNextPt;
		PointsOnLines(aCurrPt, *aAxes[0],
									bCurrPt, *bAxes[bIdx0],
									aNextPt, bNextPt,
									plClosest::kClamp);

		aCurrPt = aNextPt;
		bCurrPt = bNextPt;

		PointsOnLines(aCurrPt, *aAxes[1],
									bCurrPt, *bAxes[bIdx1],
									aNextPt, bNextPt,
									plClosest::kClamp);

		aCurrPt = aNextPt;
		bCurrPt = bNextPt;

		PointsOnLines(aCurrPt, *aAxes[2],
									bCurrPt, *bAxes[bIdx2],
									aNextPt, bNextPt,
									plClosest::kClamp);


		hsScalar distSq = hsVector3(&aNextPt, &bNextPt).MagnitudeSquared();
		if( distSq < minDistSq )
		{
			aBestPt = aNextPt;
			bBestPt = bNextPt;

			if( distSq < kRealSmall )
				retVal = true;

			minDistSq = distSq;
		}

		if( i & 0x1 )
		{
			bIdx0++;
			bIdx1 = bIdx0 < 2 ? bIdx0+1 : 0;
			bIdx2 = bIdx1 < 2 ? bIdx1+1 : 0;
		}
		else
		{
			int t = bIdx1;
			bIdx1 = bIdx2;
			bIdx2 = t;
		}
	}
	cp0 = aBestPt;
	cp1 = bBestPt;

	return retVal;
}

#else // TRASH THIS

hsBool plClosest::PointBetweenBoxes(const hsPoint3& aCorner,
									const hsVector3* aAxes[3],
									const hsPoint3& bCorner,
									const hsVector3* bAxes[3],
									hsPoint3& cp0, hsPoint3& cp1)
{
	/*
		Six combinations to try to go through every possible
		combination of axes from A and B

			00 00  01 01  02 02
			11 12  12 10  10 11
			22 21  20 22  21 20
	*/

	struct trial {
		int		aIdx[3];
		int		bIdx[3];
	} trials[36];


	int tNext = 0;
	int k,l;
	for( k = 0; k < 3; k++ )
	{
		for( l = 0; l < 3; l++ )
		{
			int kPlus = k < 2 ? k+1 : 0;
			int kPlusPlus = kPlus < 2 ? kPlus+1 : 0;

			int lPlus = l < 2 ? l+1 : 0;
			int lPlusPlus = lPlus < 2 ? lPlus+1 : 0;
			
			trials[tNext].aIdx[0] = k;
			trials[tNext].bIdx[0] = l;
			
			trials[tNext].aIdx[1] = kPlus;
			trials[tNext].bIdx[1] = lPlus;

			trials[tNext].aIdx[2] = kPlusPlus;
			trials[tNext].bIdx[2] = lPlusPlus;

			tNext++;

			trials[tNext].aIdx[0] = k;
			trials[tNext].bIdx[0] = l;
			
			trials[tNext].aIdx[1] = kPlusPlus;
			trials[tNext].bIdx[1] = lPlusPlus;

			trials[tNext].aIdx[2] = kPlus;
			trials[tNext].bIdx[2] = lPlus;

			tNext++;

			trials[tNext].aIdx[0] = k;
			trials[tNext].bIdx[0] = l;
			
			trials[tNext].aIdx[1] = kPlus;
			trials[tNext].bIdx[1] = lPlusPlus;

			trials[tNext].aIdx[2] = kPlusPlus;
			trials[tNext].bIdx[2] = lPlus;

			tNext++;

			trials[tNext].aIdx[0] = k;
			trials[tNext].bIdx[0] = l;
			
			trials[tNext].aIdx[1] = kPlusPlus;
			trials[tNext].bIdx[1] = lPlus;

			trials[tNext].aIdx[2] = kPlus;
			trials[tNext].bIdx[2] = lPlusPlus;

			tNext++;
		}
	}

	hsPoint3 aBestPt, bBestPt;
	hsScalar minDistSq = 1.e33f;

	hsBool retVal = false;

	int i;
	for( i = 0; i < 36; i++ )
	{
		hsPoint3 aCurrPt = aCorner;
		hsPoint3 bCurrPt = bCorner;

		hsPoint3 aNextPt, bNextPt;
		PointsOnLines(aCurrPt, *aAxes[trials[i].aIdx[0]],
									bCurrPt, *bAxes[trials[i].bIdx[0]],
									aNextPt, bNextPt,
									plClosest::kClamp);

		aCurrPt = aNextPt;
		bCurrPt = bNextPt;

		PointsOnLines(aCurrPt, *aAxes[trials[i].aIdx[1]],
									bCurrPt, *bAxes[trials[i].bIdx[1]],
									aNextPt, bNextPt,
									plClosest::kClamp);

		aCurrPt = aNextPt;
		bCurrPt = bNextPt;

		PointsOnLines(aCurrPt, *aAxes[trials[i].aIdx[2]],
									bCurrPt, *bAxes[trials[i].bIdx[2]],
									aNextPt, bNextPt,
									plClosest::kClamp);


		hsScalar distSq = hsVector3(&aNextPt, &bNextPt).MagnitudeSquared();
		if( distSq < minDistSq )
		{
			aBestPt = aNextPt;
			bBestPt = bNextPt;

			if( distSq < kRealSmall )
				retVal = true;

			minDistSq = distSq;
		}

	}
	cp0 = aBestPt;
	cp1 = bBestPt;

	return retVal;
}
#endif // TRASH THIS