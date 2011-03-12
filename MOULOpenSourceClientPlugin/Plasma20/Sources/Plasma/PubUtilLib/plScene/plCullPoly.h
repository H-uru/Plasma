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

#ifndef plCullPoly_inc
#define plCullPoly_inc

#include "hsTemplates.h"
#include "hsGeometry3.h"
#include "hsBitVector.h"

class hsStream;
class hsResMgr;
struct hsMatrix44;

const hsScalar			kCullPolyDegen = 1.e-4f;

class plCullPoly
{
public:
	enum {
		kNone		= 0x0,
		kHole		= 0x1,
		kTwoSided	= 0x2
	};

	UInt32					fFlags;
	mutable hsBitVector		fClipped; // fClipped[i] => edge(fVerts[i], fVerts[(i+1)%n])

	hsTArray<hsPoint3>		fVerts;
	hsVector3				fNorm;
	hsScalar				fDist;
	hsPoint3				fCenter;
	hsScalar				fRadius;

	const hsPoint3&			GetCenter() const { return fCenter; }
	hsScalar				GetRadius() const { return fRadius; }

	void					SetHole(hsBool on) { if( on )fFlags |= kHole; else fFlags &= ~kHole; }
	void					SetTwoSided(hsBool on) { if( on )fFlags |= kTwoSided; else fFlags &= ~kTwoSided; }

	hsBool					IsHole() const { return fFlags & kHole; } // Assumes kHole is 0x1
	hsBool					IsTwoSided() const { return 0 != (fFlags & kTwoSided); }

	plCullPoly&				Init(const plCullPoly& p) { fClipped.Clear(); fVerts.SetCount(0); fFlags = p.fFlags; fNorm = p.fNorm; fDist = p.fDist; fCenter = p.fCenter; return *this; }
	plCullPoly&				Flip(const plCullPoly& p);
	plCullPoly&				InitFromVerts(UInt32 f=kNone);
	hsScalar				ICalcRadius() const;

	plCullPoly&				Transform(const hsMatrix44& l2w, const hsMatrix44& w2l, plCullPoly& dst) const;

	void					Read(hsStream* s, hsResMgr* mgr);
	void					Write(hsStream* s, hsResMgr* mgr);

	hsBool					DegenerateVert(const hsPoint3& p) const { return fVerts.GetCount() && (kCullPolyDegen > hsVector3(&p, &fVerts[fVerts.GetCount()-1]).MagnitudeSquared()); }

	hsBool					Validate() const; // no-op, except for special debugging circumstances.
};

#endif // plCullPoly_inc
