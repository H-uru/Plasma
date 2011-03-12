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

#ifndef plAvMeshSmooth_inc
#define plAvMeshSmooth_inc

#include "hsTemplates.h"
#include "plAccessGeometry.h"
#include "plAccessSpan.h"

struct hsPoint3;
struct hsVector3;
class plGeometrySpan;

class plAvMeshSmooth
{
public:
	enum {
		kNone				= 0x0,
		kSmoothNorm			= 0x1,
		kSmoothPos			= 0x2,
		kSmoothDiffuse		= 0x4
	};

	class XfmSpan
	{
	public:
		plGeometrySpan*		fSpan;
		hsMatrix44			fSpanToNeutral;
		hsMatrix44			fNormSpanToNeutral; // == Transpose(Inverse(fSpanToNeutral)) == Transpose(fNeutralToSpan)

		hsMatrix44			fNeutralToSpan;
		hsMatrix44			fNormNeutralToSpan; // == Transpose(Inverse(fNeutralToSpan)) == Transpose(fSpanToNeutral)

		plAccessSpan		fAccSpan;
	};

protected:
	UInt32			fFlags;

	hsScalar		fMinNormDot;
	hsScalar		fDistTolSq;

	plAccessGeometry			fAccGeom;

	hsPoint3		IPositionToNeutral(XfmSpan& span, int i) const;
	hsVector3		INormalToNeutral(XfmSpan& span, int i) const;
	hsPoint3		IPositionToSpan(XfmSpan& span, const hsPoint3& wPos) const;
	hsVector3		INormalToSpan(XfmSpan& span, const hsVector3& wNorm) const;

	void			FindEdges(UInt32 maxVtxIdx, UInt32 nTris, UInt16* idxList, hsTArray<UInt16>& edgeVerts);
	void			FindEdges(hsTArray<XfmSpan>& spans, hsTArray<UInt16>* edgeVerts);

public:
	plAvMeshSmooth() : fFlags(kSmoothNorm), fMinNormDot(0.25f), fDistTolSq(1.e-4f), fAccGeom() {}

	void		SetAngle(hsScalar degs);
	hsScalar	GetAngle() const; // returns degrees

	void		SetDistTol(hsScalar dist);
	hsScalar	GetDistTol() const;

	void		Smooth(hsTArray<XfmSpan>& srcSpans, hsTArray<XfmSpan>& dstSpans);

	void		SetFlags(UInt32 f) { fFlags = f; }
	UInt32		GetFlags() const { return fFlags; }
};

#endif // plAvMeshSmooth_inc
