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
#include "plInterMeshSmooth.h"

#include "plDrawableSpans.h"

class EdgeBin
{
public:
	UInt16	fVtx;
	UInt16	fCount;

	EdgeBin() : fVtx(0), fCount(0) {}
};

void plInterMeshSmooth::FindEdges(UInt32 maxVtxIdx, UInt32 nTris, UInt16* idxList, hsTArray<UInt16>& edgeVerts)
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

void plInterMeshSmooth::FindEdges(hsTArray<plSpanHandle>& sets, hsTArray<UInt16>* edgeVerts)
{
	int i;
	for( i = 0; i < sets.GetCount(); i++ )
	{
		const plSpan* span = sets[i].fDrawable->GetSpan(sets[i].fSpanIdx);
		if( !(span->fTypeMask & plSpan::kIcicleSpan) )
			continue;

		UInt32 nTris = sets[i].fDrawable->CvtGetNumTris(sets[i].fSpanIdx);
		UInt16* idxList = sets[i].fDrawable->CvtGetIndexList(sets[i].fSpanIdx);
		UInt32 maxVertIdx = sets[i].fDrawable->CvtGetNumVerts(sets[i].fSpanIdx)-1;

		FindEdges(maxVertIdx, nTris, idxList, edgeVerts[i]);
	}
}

void plInterMeshSmooth::SmoothNormals(hsTArray<plSpanHandle>& sets)
{
	hsTArray<UInt16>* shareVtx = TRACKED_NEW hsTArray<UInt16>[sets.GetCount()];
	hsTArray<UInt16>* edgeVerts = TRACKED_NEW hsTArray<UInt16>[sets.GetCount()];
	FindEdges(sets, edgeVerts);

	int i;
	for( i = 0; i < sets.GetCount()-1; i++ )
	{
		int j;
		for( j = edgeVerts[i].GetCount()-1; j >= 0; --j )
		{
			hsPoint3 pos = GetPosition(sets[i], edgeVerts[i][j]);
			hsVector3 normAccum = GetNormal(sets[i], edgeVerts[i][j]);;

			shareVtx[i].Append(edgeVerts[i][j]);

			int k;
			for( k = i+1; k < sets.GetCount(); k++ )
			{
				FindSharedVerts(pos, sets[k], edgeVerts[k], shareVtx[k], normAccum);
			}

			normAccum.Normalize();
			GetNormal(sets[i], edgeVerts[i][j]) = normAccum;

			for( k = i+1; k < sets.GetCount(); k++ )
			{
				SetNormals(sets[k], shareVtx[k], normAccum);
			}

			// Now remove all the shared verts (which we just processed)
			// from edgeVerts so we don't process them again.
			for( k = i; k < sets.GetCount(); k++ )
			{
				int m;
				for( m = 0; m < shareVtx[k].GetCount(); m++ )
				{
					int idx = edgeVerts[k].Find(shareVtx[k][m]);
					hsAssert(idx != edgeVerts[k].kMissingIndex, "Lost vertex between find and remove");
					edgeVerts[k].Remove(idx);
				}
				shareVtx[k].SetCount(0);
			}
		}
	}

	delete [] shareVtx;
	delete [] edgeVerts;
}

void plInterMeshSmooth::FindSharedVerts(hsPoint3& searchPos, plSpanHandle& set, hsTArray<UInt16>& edgeVerts, hsTArray<UInt16>& shareVtx, hsVector3& normAccum)
{
	int i;
	for( i = 0; i < edgeVerts.GetCount(); i++ )
	{
		hsPoint3 pos = GetPosition(set, edgeVerts[i]);
		hsVector3 norm = GetNormal(set, edgeVerts[i]);
		if( searchPos == pos )
		{
			if( norm.InnerProduct(normAccum) > fMinNormDot )
			{
				shareVtx.Append(edgeVerts[i]);
				normAccum += norm;
			}
		}
	}
}

void plInterMeshSmooth::SetNormals(plSpanHandle& set, hsTArray<UInt16>& shareVtx, hsVector3& norm)
{
	int i;
	for( i = 0; i < shareVtx.GetCount(); i++ )
		GetNormal(set, shareVtx[i]) = norm;
}

hsPoint3& plInterMeshSmooth::GetPosition(plSpanHandle& set, UInt16 idx)
{
	return set.fDrawable->CvtGetPosition(set.fSpanIdx, idx);
}

hsVector3& plInterMeshSmooth::GetNormal(plSpanHandle& set, UInt16 idx)
{
	return set.fDrawable->CvtGetNormal(set.fSpanIdx, idx);
}

void plInterMeshSmooth::SetAngle(hsScalar degs)
{
	fMinNormDot = hsCosine(hsScalarDegToRad(degs));
}

hsScalar plInterMeshSmooth::GetAngle() const
{
	return hsScalarRadToDeg(hsACosine(fMinNormDot));
}