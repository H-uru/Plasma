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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#include "HeadSpin.h"
#include "plAccMeshSmooth.h"

#include "plGeometrySpan.h"
#include "plAccessGeometry.h"
#include "plAccessTriSpan.h"

#include "hsFastMath.h"

class EdgeBin
{
public:
    uint16_t  fVtx;
    uint16_t  fCount;

    EdgeBin() : fVtx(0), fCount(0) {}
};

void plAccMeshSmooth::FindEdges(uint32_t maxVtxIdx, uint32_t nTris, uint16_t* idxList, hsTArray<uint16_t>& edgeVerts)
{
    hsTArray<EdgeBin>*  bins = new hsTArray<EdgeBin>[maxVtxIdx+1];

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

void plAccMeshSmooth::FindEdges(hsTArray<plGeometrySpan*>& spans, hsTArray<uint16_t>* edgeVerts)
{
    fSpans.SetCount(spans.GetCount());

    int i;
    for( i = 0; i < spans.GetCount(); i++ )
    {
        fAccGeom.AccessSpanFromGeometrySpan(fSpans[i], spans[i]);
        if( !fSpans[i].HasAccessTri() )
            continue;

        plAccessTriSpan& triSpan = fSpans[i].AccessTri();

        uint32_t nTris = triSpan.TriCount();
        uint16_t* idxList = triSpan.fTris;
        uint32_t maxVertIdx = triSpan.VertCount()-1;

        FindEdges(maxVertIdx, nTris, idxList, edgeVerts[i]);
    }
}

void plAccMeshSmooth::Smooth(hsTArray<plGeometrySpan*>& spans)
{
    hsTArray<uint16_t>* shareVtx = new hsTArray<uint16_t>[spans.GetCount()];
    hsTArray<uint16_t>* edgeVerts = new hsTArray<uint16_t>[spans.GetCount()];
    FindEdges(spans, edgeVerts);

    int i;
    for( i = 0; i < spans.GetCount(); i++ )
    {
        while( edgeVerts[i].GetCount() )
        {
            int j = edgeVerts[i].GetCount()-1;

            plAccessTriSpan& triSpan = fSpans[i].AccessTri();

            VtxAccum accum;
            accum.fPos = IPositionToWorld(fSpans[i], edgeVerts[i][j]);
            accum.fNorm = INormalToWorld(fSpans[i], edgeVerts[i][j]);
            if( triSpan.HasDiffuse() )
                accum.fDiffuse = triSpan.DiffuseRGBA(edgeVerts[i][j]);
            else
                accum.fDiffuse.Set(1.f, 1.f, 1.f, 1.f);

            shareVtx[i].Append(edgeVerts[i][j]);

            // Find shared verts on this same span
            FindSharedVerts(fSpans[i], j, edgeVerts[i], shareVtx[i], accum);

            // Now look through the rest of the spans
            int k;
            for( k = i+1; k < spans.GetCount(); k++ )
            {
                FindSharedVerts(fSpans[k], edgeVerts[k].GetCount(), edgeVerts[k], shareVtx[k], accum);
            }

            accum.fNorm.Normalize();

            if( fFlags & kSmoothNorm )
            {
                for( k = i; k < spans.GetCount(); k++ )
                {
                    SetNormals(fSpans[k], shareVtx[k], accum.fNorm);
                }
            }
            if( fFlags & kSmoothPos )
            {
                for( k = i; k < spans.GetCount(); k++ )
                {
                    SetPositions(fSpans[k], shareVtx[k], accum.fPos);
                }
            }
            if( fFlags & kSmoothDiffuse )
            {
                for( k = i; k < spans.GetCount(); k++ )
                {
                    SetDiffuse(fSpans[k], shareVtx[k], accum.fDiffuse);
                }
            }

            // Now remove all the shared verts (which we just processed)
            // from edgeVerts so we don't process them again.
            for( k = i; k < spans.GetCount(); k++ )
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

hsPoint3 plAccMeshSmooth::IPositionToWorld(plAccessSpan& span, int i) const
{
    return span.GetLocalToWorld() * span.AccessTri().Position(i);
}

hsVector3 plAccMeshSmooth::INormalToWorld(plAccessSpan& span, int i) const
{
    if( span.GetWorldToLocal().fFlags & hsMatrix44::kIsIdent )
    {
        return span.AccessTri().Normal(i);
    }

    hsMatrix44 l2wInvTransp;
    span.GetWorldToLocal().GetTranspose(&l2wInvTransp);

    hsVector3 ret = l2wInvTransp * span.AccessTri().Normal(i);
    hsFastMath::NormalizeAppr(ret);
    return ret;
}

hsPoint3 plAccMeshSmooth::IPositionToLocal(plAccessSpan& span, const hsPoint3& wPos) const
{
    return span.GetWorldToLocal() * wPos;
}

hsVector3 plAccMeshSmooth::INormalToLocal(plAccessSpan& span, const hsVector3& wNorm) const
{
    if( span.GetLocalToWorld().fFlags & hsMatrix44::kIsIdent )
    {
        return wNorm;
    }

    hsMatrix44 w2lInvTransp;
    span.GetLocalToWorld().GetTranspose(&w2lInvTransp);

    hsVector3 ret = w2lInvTransp * wNorm;
    hsFastMath::NormalizeAppr(ret);
    return ret;
}

void plAccMeshSmooth::FindSharedVerts(plAccessSpan& span, int numEdgeVerts, hsTArray<uint16_t>& edgeVerts, hsTArray<uint16_t>& shareVtx, VtxAccum& accum)
{
    plAccessTriSpan& triSpan = span.AccessTri();
    int i;
    for( i = 0; i < numEdgeVerts; i++ )
    {
        hsPoint3 pos = IPositionToWorld(span, edgeVerts[i]);
        hsVector3 diff(&accum.fPos, &pos);
        if( diff.MagnitudeSquared() < fDistTolSq )
        {
            hsVector3 norm = INormalToWorld(span, edgeVerts[i]);
            if( norm.InnerProduct(accum.fNorm) > fMinNormDot )
            {
                shareVtx.Append(edgeVerts[i]);

                accum.fPos += pos;
                accum.fPos *= 0.5f;

                accum.fNorm += norm;
                hsFastMath::NormalizeAppr(accum.fNorm);

                hsColorRGBA diff;
                if( triSpan.HasDiffuse() )
                    diff = triSpan.DiffuseRGBA(edgeVerts[i]);
                else
                    diff.Set(1.f, 1.f, 1.f, 1.f);
                accum.fDiffuse += diff;
                accum.fDiffuse *= 0.5f;
            }
        }
    }
}

void plAccMeshSmooth::SetPositions(plAccessSpan& span, hsTArray<uint16_t>& shareVtx, const hsPoint3& pos) const
{
    plAccessTriSpan& triSpan = span.AccessTri();
    int i;
    for( i = 0; i < shareVtx.GetCount(); i++ )
        triSpan.Position(shareVtx[i]) = IPositionToLocal(span, pos);
}

void plAccMeshSmooth::SetNormals(plAccessSpan& span, hsTArray<uint16_t>& shareVtx, const hsVector3& norm) const
{
    plAccessTriSpan& triSpan = span.AccessTri();
    int i;
    for( i = 0; i < shareVtx.GetCount(); i++ )
        triSpan.Normal(shareVtx[i]) = INormalToLocal(span, norm);
}

void plAccMeshSmooth::SetDiffuse(plAccessSpan& span, hsTArray<uint16_t>& shareVtx, const hsColorRGBA& diff) const
{
    plAccessTriSpan& triSpan = span.AccessTri();
    hsAssert(triSpan.HasDiffuse(), "Calling SetColors on data with no color");
    int i;
    for( i = 0; i < shareVtx.GetCount(); i++ )
        triSpan.Diffuse32(shareVtx[i]) = diff.ToARGB32();
}

void plAccMeshSmooth::SetAngle(float degs)
{
    fMinNormDot = cos(hsDegreesToRadians(degs));
}

float plAccMeshSmooth::GetAngle() const
{
    return hsRadiansToDegrees(acos(fMinNormDot));
}

void plAccMeshSmooth::SetDistTol(float dist)
{
    fDistTolSq = dist * dist;
}

float plAccMeshSmooth::GetDistTol() const
{
    return sqrt(fDistTolSq);
}
