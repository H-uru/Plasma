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
#include "plInterMeshSmooth.h"

#include <memory>

#include "plDrawableSpans.h"

void plInterMeshSmooth::FindEdges(uint32_t maxVtxIdx, uint32_t nTris, uint16_t* idxList, std::vector<uint16_t>& edgeVerts)
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

void plInterMeshSmooth::FindEdges(std::vector<plSpanHandle>& sets, std::vector<uint16_t>* edgeVerts)
{
    for (size_t i = 0; i < sets.size(); i++)
    {
        const plSpan* span = sets[i].fDrawable->GetSpan(sets[i].fSpanIdx);
        if( !(span->fTypeMask & plSpan::kIcicleSpan) )
            continue;

        uint32_t nTris = sets[i].fDrawable->CvtGetNumTris(sets[i].fSpanIdx);
        uint16_t* idxList = sets[i].fDrawable->CvtGetIndexList(sets[i].fSpanIdx);
        uint32_t maxVertIdx = sets[i].fDrawable->CvtGetNumVerts(sets[i].fSpanIdx)-1;

        FindEdges(maxVertIdx, nTris, idxList, edgeVerts[i]);
    }
}

void plInterMeshSmooth::SmoothNormals(std::vector<plSpanHandle>& sets)
{
    auto shareVtx = std::make_unique<std::vector<uint16_t>[]>(sets.size());
    auto edgeVerts = std::make_unique<std::vector<uint16_t>[]>(sets.size());
    FindEdges(sets, edgeVerts.get());

    for (size_t i = 0; i < sets.size() - 1; i++)
    {
        for (hsSsize_t j = edgeVerts[i].size() - 1; j >= 0; --j)
        {
            hsPoint3 pos = GetPosition(sets[i], edgeVerts[i][j]);
            hsVector3 normAccum = GetNormal(sets[i], edgeVerts[i][j]);;

            shareVtx[i].emplace_back(edgeVerts[i][j]);

            for (size_t k = i + 1; k < sets.size(); k++)
            {
                FindSharedVerts(pos, sets[k], edgeVerts[k], shareVtx[k], normAccum);
            }

            normAccum.Normalize();
            GetNormal(sets[i], edgeVerts[i][j]) = normAccum;

            for (size_t k = i + 1; k < sets.size(); k++)
            {
                SetNormals(sets[k], shareVtx[k], normAccum);
            }

            // Now remove all the shared verts (which we just processed)
            // from edgeVerts so we don't process them again.
            for (size_t k = i; k < sets.size(); k++)
            {
                for (uint16_t vtx : shareVtx[k])
                {
                    auto iter = std::find(edgeVerts[k].cbegin(), edgeVerts[k].cend(), vtx);
                    hsAssert(iter != edgeVerts[k].cend(), "Lost vertex between find and remove");
                    edgeVerts[k].erase(iter);
                }
                shareVtx[k].clear();
            }
        }
    }
}

void plInterMeshSmooth::FindSharedVerts(hsPoint3& searchPos, plSpanHandle& set,
                                        std::vector<uint16_t>& edgeVerts,
                                        std::vector<uint16_t>& shareVtx, hsVector3& normAccum)
{
    for (uint16_t edgeVert : edgeVerts)
    {
        hsPoint3 pos = GetPosition(set, edgeVert);
        hsVector3 norm = GetNormal(set, edgeVert);
        if( searchPos == pos )
        {
            if( norm.InnerProduct(normAccum) > fMinNormDot )
            {
                shareVtx.emplace_back(edgeVert);
                normAccum += norm;
            }
        }
    }
}

void plInterMeshSmooth::SetNormals(plSpanHandle& set, std::vector<uint16_t>& shareVtx, hsVector3& norm)
{
    for (uint16_t vtx : shareVtx)
        GetNormal(set, vtx) = norm;
}

hsPoint3& plInterMeshSmooth::GetPosition(plSpanHandle& set, uint16_t idx)
{
    return set.fDrawable->CvtGetPosition(set.fSpanIdx, idx);
}

hsVector3& plInterMeshSmooth::GetNormal(plSpanHandle& set, uint16_t idx)
{
    return set.fDrawable->CvtGetNormal(set.fSpanIdx, idx);
}

void plInterMeshSmooth::SetAngle(float degs)
{
    fMinNormDot = cos(hsDegreesToRadians(degs));
}

float plInterMeshSmooth::GetAngle() const
{
    return hsRadiansToDegrees(acos(fMinNormDot));
}
