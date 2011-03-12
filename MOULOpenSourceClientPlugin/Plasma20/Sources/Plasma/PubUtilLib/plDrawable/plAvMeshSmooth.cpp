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
#include "plAvMeshSmooth.h"

#include "plGeometrySpan.h"
#include "plAccessGeometry.h"
#include "plAccessTriSpan.h"

#include "hsFastMath.h"

class EdgeBin
{
public:
	UInt16	fVtx;
	UInt16	fCount;

	EdgeBin() : fVtx(0), fCount(0) {}
};

void plAvMeshSmooth::FindEdges(UInt32 maxVtxIdx, UInt32 nTris, UInt16* idxList, hsTArray<UInt16>& edgeVerts)
{
	hsTArray<EdgeBin>*	bins = TRACKED_NEW hsTArray<EdgeBin>[maxVtxIdx+1];

	hsBitVector edgeVertBits;
	// For each vert pair (edge) in idxList
	int i;
	for( i = 0; i < nTris; i++ )
	{
		int j;
		for( j = 0; j < 3; j++ )
		{
			int jPlus = j < 2 ? j+1 : 0;
			int idx0 = idxList[i*3 + j];
			int idx1 = idxList[i*3 + jPlus];

			int lo, hi;
			
			// Look in the LUT for the lower index.
			if( idx0 < idx1 )
			{
				lo = idx0;
				hi = idx1;
			}
			else
			{
				lo = idx1;
				hi = idx0;
			}

			hsTArray<EdgeBin>& loBin = bins[lo];
			// In that bucket, look for the higher index.
			int k;
			for( k = 0; k < loBin.GetCount(); k++ )
			{
				if( loBin[k].fVtx == hi )
					break;
			}

			// If we find it, increment it's count,
			// else add it.
			if( k < loBin.GetCount() )
			{
				loBin[k].fCount++;
			}
			else
			{
				EdgeBin* b = loBin.Push();
				b->fVtx = hi;
				b->fCount = 1;
			}
		}
	}

	// For each bucket in the LUT,
	for( i = 0; i < maxVtxIdx+1; i++ )
	{
		hsTArray<EdgeBin>& loBin = bins[i];
		// For each higher index
		int j;
		for( j = 0; j < loBin.GetCount(); j++ )
		{
			// If the count is one, it's an edge, so set the edge bit for both indices (hi and lo)
			if( 1 == loBin[j].fCount )
			{
				edgeVertBits.SetBit(i);
				edgeVertBits.SetBit(loBin[j].fVtx);
			}
		}
	}
	
	// Now translate the bitvector to a list of indices.
	for( i = 0; i < maxVtxIdx+1; i++ )
	{
		if( edgeVertBits.IsBitSet(i) )
			edgeVerts.Append(i);
	}
	delete [] bins;
}

void plAvMeshSmooth::FindEdges(hsTArray<XfmSpan>& spans, hsTArray<UInt16>* edgeVerts)
{
	int i;
	for( i = 0; i < spans.GetCount(); i++ )
	{
		fAccGeom.AccessSpanFromGeometrySpan(spans[i].fAccSpan, spans[i].fSpan);
		if( !spans[i].fAccSpan.HasAccessTri() )
			continue;

		plAccessTriSpan& triSpan = spans[i].fAccSpan.AccessTri();

		UInt32 nTris = triSpan.TriCount();
		UInt16* idxList = triSpan.fTris;
		UInt32 maxVertIdx = triSpan.VertCount()-1;

		FindEdges(maxVertIdx, nTris, idxList, edgeVerts[i]);
	}
}

// A little note about why we need to pass in so much to do this.
// If the input geometryspans were in local space (ForceLocal), then
// all we would need to do is ignore any transforms they might have,
// and life is grand.
// But for reasons I don't pretend to understand, we can't do that, so
// here, to smooth the delta meshes, we transform both them and the base "snap-to"
// meshes into their respective local spaces, and then look for matches. This works
// because the base and delta meshes are constrained, not to be coincident in world space,
// but to be coincident in the local space relative to Max pivot.
// The funny painful thing is that later, when we go to use these smoothed delta meshes,
// again we need to coerce them into a neutral space. At that time, we'll use the
// morph target mesh's local space. Whatever.
void plAvMeshSmooth::Smooth(hsTArray<XfmSpan>& srcSpans, hsTArray<XfmSpan>& dstSpans)
{
	hsTArray<UInt16>* dstEdgeVerts = TRACKED_NEW hsTArray<UInt16>[dstSpans.GetCount()];
	FindEdges(dstSpans, dstEdgeVerts);

	hsTArray<UInt16>* srcEdgeVerts = TRACKED_NEW hsTArray<UInt16>[srcSpans.GetCount()];
	FindEdges(srcSpans, srcEdgeVerts);

	int i;
	for( i = 0; i < dstSpans.GetCount(); i++ )
	{
		plAccessTriSpan& dstTriSpan = dstSpans[i].fAccSpan.AccessTri();

		int j;
		for( j = 0; j < dstEdgeVerts[i].GetCount(); j++ )
		{

			hsPoint3 dstPos = IPositionToNeutral(dstSpans[i], dstEdgeVerts[i][j]);
			hsVector3 dstNorm = INormalToNeutral(dstSpans[i], dstEdgeVerts[i][j]);
			hsColorRGBA dstDiff;
			if( dstTriSpan.HasDiffuse() )
				dstDiff = dstTriSpan.DiffuseRGBA(dstEdgeVerts[i][j]);
			else
				dstDiff.Set(1.f, 1.f, 1.f, 1.f);

			hsScalar maxDot = fMinNormDot;

			hsPoint3 smoothPos = dstPos;
			hsVector3 smoothNorm = dstNorm;
			hsColorRGBA smoothDiff = dstDiff;

			int k;
			for( k = 0; k < srcSpans.GetCount(); k++ )
			{
				int m;
				for( m = 0; m < srcEdgeVerts[k].GetCount(); m++ )
				{
					hsPoint3 srcPos = IPositionToNeutral(srcSpans[k], srcEdgeVerts[k][m]);
					hsVector3 srcNorm = INormalToNeutral(srcSpans[k], srcEdgeVerts[k][m]);

					hsScalar dist = hsVector3(&dstPos, &srcPos).MagnitudeSquared();
					if( dist <= fDistTolSq )
					{
						smoothPos = srcPos;

						hsScalar currDot = srcNorm.InnerProduct(dstNorm);
						if( currDot > maxDot )
						{
							maxDot = currDot;
							smoothNorm = srcNorm;
							if( srcSpans[k].fAccSpan.AccessTri().HasDiffuse() )
								smoothDiff = srcSpans[k].fAccSpan.AccessTri().DiffuseRGBA(srcEdgeVerts[k][m]);
							else
								smoothDiff = dstDiff;
						}
					}
				}
			}
			if( fFlags & kSmoothPos )
				dstTriSpan.Position(dstEdgeVerts[i][j]) = IPositionToSpan(dstSpans[i], smoothPos);
			if( fFlags & kSmoothNorm )
				dstTriSpan.Normal(dstEdgeVerts[i][j]) = INormalToSpan(dstSpans[i], smoothNorm);
			if( (fFlags & kSmoothDiffuse) && dstTriSpan.HasDiffuse() )
				dstTriSpan.Diffuse32(dstEdgeVerts[i][j]) = smoothDiff.ToARGB32();
		}

	}

	delete [] srcEdgeVerts;
	delete [] dstEdgeVerts;
}

hsPoint3 plAvMeshSmooth::IPositionToNeutral(XfmSpan& span, int i) const
{
	return span.fSpanToNeutral * span.fAccSpan.AccessTri().Position(i);
}

hsVector3 plAvMeshSmooth::INormalToNeutral(XfmSpan& span, int i) const
{
	hsVector3 ret = span.fNormSpanToNeutral * span.fAccSpan.AccessTri().Normal(i);
	hsFastMath::Normalize(ret);
	return ret;
}

hsPoint3 plAvMeshSmooth::IPositionToSpan(XfmSpan& span, const hsPoint3& wPos) const
{
	return span.fNeutralToSpan * wPos;
}

hsVector3 plAvMeshSmooth::INormalToSpan(XfmSpan& span, const hsVector3& wNorm) const
{
	hsVector3 ret = span.fNormNeutralToSpan * wNorm;
	hsFastMath::Normalize(ret);
	return ret;
}

void plAvMeshSmooth::SetAngle(hsScalar degs)
{
	fMinNormDot = hsCosine(hsScalarDegToRad(degs));
}

hsScalar plAvMeshSmooth::GetAngle() const
{
	return hsScalarRadToDeg(hsACosine(fMinNormDot));
}

void plAvMeshSmooth::SetDistTol(hsScalar dist)
{
	fDistTolSq = dist * dist;
}

hsScalar plAvMeshSmooth::GetDistTol() const
{
	return hsSquareRoot(fDistTolSq);
}