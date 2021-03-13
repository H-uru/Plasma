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

#include <memory>

#include "HeadSpin.h"
#include "plAvMeshSmooth.h"

#include "plGeometrySpan.h"
#include "plAccessGeometry.h"
#include "plAccessTriSpan.h"

#include "hsFastMath.h"

void plAvMeshSmooth::FindEdges(uint32_t maxVtxIdx, uint32_t nTris, uint16_t* idxList, std::vector<uint16_t>& edgeVerts)
{
    struct EdgeBin
    {
        uint16_t  fVtx;
        uint16_t  fCount;

        EdgeBin(uint16_t vtx, uint16_t count) : fVtx(vtx), fCount(count) { }
    };

    std::vector<std::vector<EdgeBin>> bins(maxVtxIdx + 1);

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

            std::vector<EdgeBin>& loBin = bins[lo];
            // In that bucket, look for the higher index.
            auto iter = std::find_if(loBin.begin(), loBin.end(),
                                     [hi](const EdgeBin& bin) { return bin.fVtx == hi; });

            // If we find it, increment it's count,
            // else add it.
            if (iter != loBin.end())
                iter->fCount++;
            else
                loBin.emplace_back(hi, 1);
        }
    }

    // For each bucket in the LUT,
    for( i = 0; i < maxVtxIdx+1; i++ )
    {
        std::vector<EdgeBin>& loBin = bins[i];
        // For each higher index
        for (const EdgeBin& bin : loBin)
        {
            // If the count is one, it's an edge, so set the edge bit for both indices (hi and lo)
            if (bin.fCount == 1)
            {
                edgeVertBits.SetBit(i);
                edgeVertBits.SetBit(bin.fVtx);
            }
        }
    }
    
    // Now translate the bitvector to a list of indices.
    for( i = 0; i < maxVtxIdx+1; i++ )
    {
        if( edgeVertBits.IsBitSet(i) )
            edgeVerts.emplace_back(i);
    }
}

void plAvMeshSmooth::FindEdges(std::vector<XfmSpan>& spans, std::vector<uint16_t>* edgeVerts)
{
    for (size_t i = 0; i < spans.size(); i++)
    {
        fAccGeom.AccessSpanFromGeometrySpan(spans[i].fAccSpan, spans[i].fSpan);
        if( !spans[i].fAccSpan.HasAccessTri() )
            continue;

        plAccessTriSpan& triSpan = spans[i].fAccSpan.AccessTri();

        uint32_t nTris = triSpan.TriCount();
        uint16_t* idxList = triSpan.fTris;
        uint32_t maxVertIdx = triSpan.VertCount()-1;

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
void plAvMeshSmooth::Smooth(std::vector<XfmSpan>& srcSpans, std::vector<XfmSpan>& dstSpans)
{
    auto dstEdgeVerts = std::make_unique<std::vector<uint16_t>[]>(dstSpans.size());
    FindEdges(dstSpans, dstEdgeVerts.get());

    auto srcEdgeVerts = std::make_unique<std::vector<uint16_t>[]>(srcSpans.size());
    FindEdges(srcSpans, srcEdgeVerts.get());

    for (size_t i = 0; i < dstSpans.size(); i++)
    {
        plAccessTriSpan& dstTriSpan = dstSpans[i].fAccSpan.AccessTri();

        for (uint16_t dstEdgeVert : dstEdgeVerts[i])
        {

            hsPoint3 dstPos = IPositionToNeutral(dstSpans[i], dstEdgeVert);
            hsVector3 dstNorm = INormalToNeutral(dstSpans[i], dstEdgeVert);
            hsColorRGBA dstDiff;
            if( dstTriSpan.HasDiffuse() )
                dstDiff = dstTriSpan.DiffuseRGBA(dstEdgeVert);
            else
                dstDiff.Set(1.f, 1.f, 1.f, 1.f);

            float maxDot = fMinNormDot;

            hsPoint3 smoothPos = dstPos;
            hsVector3 smoothNorm = dstNorm;
            hsColorRGBA smoothDiff = dstDiff;

            for (size_t k = 0; k < srcSpans.size(); k++)
            {
                for (uint16_t srcEdgeVert : srcEdgeVerts[k])
                {
                    hsPoint3 srcPos = IPositionToNeutral(srcSpans[k], srcEdgeVert);
                    hsVector3 srcNorm = INormalToNeutral(srcSpans[k], srcEdgeVert);

                    float dist = hsVector3(&dstPos, &srcPos).MagnitudeSquared();
                    if( dist <= fDistTolSq )
                    {
                        smoothPos = srcPos;

                        float currDot = srcNorm.InnerProduct(dstNorm);
                        if( currDot > maxDot )
                        {
                            maxDot = currDot;
                            smoothNorm = srcNorm;
                            if( srcSpans[k].fAccSpan.AccessTri().HasDiffuse() )
                                smoothDiff = srcSpans[k].fAccSpan.AccessTri().DiffuseRGBA(srcEdgeVert);
                            else
                                smoothDiff = dstDiff;
                        }
                    }
                }
            }
            if( fFlags & kSmoothPos )
                dstTriSpan.Position(dstEdgeVert) = IPositionToSpan(dstSpans[i], smoothPos);
            if( fFlags & kSmoothNorm )
                dstTriSpan.Normal(dstEdgeVert) = INormalToSpan(dstSpans[i], smoothNorm);
            if( (fFlags & kSmoothDiffuse) && dstTriSpan.HasDiffuse() )
                dstTriSpan.Diffuse32(dstEdgeVert) = smoothDiff.ToARGB32();
        }
    }
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

void plAvMeshSmooth::SetAngle(float degs)
{
    fMinNormDot = cos(hsDegreesToRadians(degs));
}

float plAvMeshSmooth::GetAngle() const
{
    return hsRadiansToDegrees(acos(fMinNormDot));
}

void plAvMeshSmooth::SetDistTol(float dist)
{
    fDistTolSq = dist * dist;
}

float plAvMeshSmooth::GetDistTol() const
{
    return sqrt(fDistTolSq);
}
