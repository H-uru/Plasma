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
#include "plCullPoly.h"
#include "hsMatrix44.h"
#include "hsStream.h"
#include "hsFastMath.h"

plCullPoly& plCullPoly::InitFromVerts(UInt32 f)
{
	fFlags = f;

	hsAssert(fVerts.GetCount() > 2, "Initializing from degenerate poly");

	hsVector3 a;
	hsVector3 b;
	a.Set(&fVerts[1], &fVerts[0]);
	b.Set(&fVerts[2], &fVerts[0]);
	fNorm = a % b;
	hsFastMath::Normalize(fNorm);
	fDist = -(fNorm.InnerProduct(fVerts[0]));

	fCenter.Set(0,0,0);
	int i;
	for( i = 0; i < fVerts.GetCount(); i++ )
	{
		fCenter += fVerts[i];
	}
	fCenter *= 1.f / hsScalar(fVerts.GetCount());

	fRadius = ICalcRadius();

	return *this;
}

hsScalar plCullPoly::ICalcRadius() const
{
	hsScalar radSq = 0;
	int i;
	for( i = 0; i < fVerts.GetCount(); i++ )
	{
		hsScalar tmpSq = hsVector3(&fVerts[i], &fCenter).MagnitudeSquared();
		if( tmpSq > radSq )
			radSq = tmpSq;
	}
	return radSq * hsFastMath::InvSqrt(radSq);
}

plCullPoly& plCullPoly::Flip(const plCullPoly& p)
{
	fFlags = p.fFlags;

	fNorm = -p.fNorm;
	fDist = -p.fDist;
	fCenter = p.fCenter;
	fRadius = p.fRadius;

	int n = p.fVerts.GetCount();
	fVerts.SetCount(n);
	int i;
	for( i = 0; i < n; i++ )
		fVerts[n-i-1] = p.fVerts[i];

	return *this;
}

plCullPoly& plCullPoly::Transform(const hsMatrix44& l2w, const hsMatrix44& w2l, plCullPoly& dst) const
{
	hsMatrix44 tpose;
	w2l.GetTranspose(&tpose);

	dst.fFlags = fFlags;

	dst.fVerts.SetCount(fVerts.GetCount());

	int i;
	for( i = 0; i < fVerts.GetCount(); i++ )
	{
		dst.fVerts[i] = l2w * fVerts[i];
	}
	dst.fCenter = l2w * fCenter;

	dst.fNorm = tpose * fNorm;
	
	dst.fDist = -(dst.fNorm .InnerProduct(dst.fVerts[0]));

	ICalcRadius();

	return dst;
}

void plCullPoly::Read(hsStream* s, hsResMgr* mgr)
{
	fFlags = s->ReadSwap32();

	fNorm.Read(s);
	fDist = s->ReadSwapScalar();
	fCenter.Read(s);

	fRadius = s->ReadSwapScalar();

	int n = s->ReadSwap32();
	fVerts.SetCount(n);
	int i;
	for( i = 0; i < n; i++ )
		fVerts[i].Read(s);
}

void plCullPoly::Write(hsStream* s, hsResMgr* mgr)
{
	s->WriteSwap32(fFlags);

	fNorm.Write(s);
	s->WriteSwapScalar(fDist);
	fCenter.Write(s);

	s->WriteSwapScalar(fRadius);

	s->WriteSwap32(fVerts.GetCount());
	int i;
	for( i = 0; i < fVerts.GetCount(); i++ )
		fVerts[i].Write(s);
}

#ifdef HS_DEBUGGING
#define MF_VALIDATE_POLYS
#endif // HS_DEBUGGING

#ifdef MF_VALIDATE_POLYS
hsBool plCullPoly::Validate() const
{
	const hsScalar kMinMag = 1.e-8f;
	hsScalar magSq = fNorm.MagnitudeSquared();
	if( magSq < kMinMag )
		return false;
	if( fVerts.GetCount() < 3 )
		return false;
	hsVector3 norm = hsVector3(&fVerts[1], &fVerts[0]) % hsVector3(&fVerts[2], &fVerts[0]);
	magSq = norm.MagnitudeSquared();
	if( magSq < kMinMag )
		return false;
	norm *= hsFastMath::InvSqrtAppr(magSq);
	int i;
	for( i = 3; i < fVerts.GetCount(); i++ )
	{
		hsVector3 nextNorm = hsVector3(&fVerts[i-1], &fVerts[0]) % hsVector3(&fVerts[i], &fVerts[0]);
		magSq = nextNorm.MagnitudeSquared();
		if( magSq < kMinMag )
			return false;
		nextNorm *= hsFastMath::InvSqrtAppr(magSq);
		if( nextNorm.InnerProduct(norm) < kMinMag )
			return false;
	}
	return true;
}
#else // MF_VALIDATE_POLYS
hsBool plCullPoly::Validate() const
{
	return true;
}
#endif // MF_VALIDATE_POLYS

