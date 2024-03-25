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

#include <memory.h>

#include "HeadSpin.h"
#include "plGeoSpanDice.h"
#include "plGeometrySpan.h"

plGeoSpanDice::plGeoSpanDice()
:   fMinFaces(0),
    fMaxFaces(0)
{
    fMaxSize.Set(0,0,0);
}

plGeoSpanDice::~plGeoSpanDice()
{
}

bool plGeoSpanDice::Dice(std::vector<plGeometrySpan*>& spans) const
{
    size_t startingCount = spans.size();

    std::vector<plGeometrySpan*> out;
    std::vector<plGeometrySpan*> next;

    while (!spans.empty())
    {
        for (plGeometrySpan* span : spans)
        {
            if (!IHalf(span, next))
            {
                out.emplace_back(span);
            }
        }
        spans.swap(next);
        next.clear();
    }
    spans.swap(out);

    return spans.size() != startingCount;
}

bool plGeoSpanDice::INeedSplitting(plGeometrySpan* src) const
{
    // Do we have enough faces to bother?
    if( fMinFaces )
    {
        if( src->fNumIndices < fMinFaces * 3 )
            return false;
    }
    // Do we have enough faces to bother no matter how small we are?
    if( fMaxFaces )
    {
        if( src->fNumIndices > fMaxFaces * 3 )
            return true;
    }
    // Are we big enough to bother?
    if( (fMaxSize.fX > 0) || (fMaxSize.fY > 0) || (fMaxSize.fZ > 0) )
    {
        hsPoint3 size = src->fLocalBounds.GetMaxs() - src->fLocalBounds.GetMins();
        if( size.fX > fMaxSize.fX )
            return true;
        if( size.fY > fMaxSize.fY )
            return true;
        if( size.fZ > fMaxSize.fZ )
            return true;
    }

    return false;
}

bool plGeoSpanDice::IHalf(plGeometrySpan* src, std::vector<plGeometrySpan*>& out, int exclAxis) const
{
    if( !INeedSplitting(src) )
        return false;

    int iAxis = ISelectAxis(exclAxis, src);
    // Ran out of axes to try.
    if( iAxis < 0 )
        return false;

    float midPoint = src->fLocalBounds.GetCenter()[iAxis];

    std::vector<uint32_t>     loTris;
    std::vector<uint32_t>     hiTris;

    uint16_t* indexData = src->fIndexData;

    int numTris = src->fNumIndices / 3;

    int stride = src->GetVertexSize(src->fFormat);

    int i;
    for( i = 0; i < numTris; i++ )
    {
        hsPoint3& pos0 = *(hsPoint3*)(src->fVertexData + *indexData++ * stride);
        hsPoint3& pos1 = *(hsPoint3*)(src->fVertexData + *indexData++ * stride);
        hsPoint3& pos2 = *(hsPoint3*)(src->fVertexData + *indexData++ * stride);

        if( (pos0[iAxis] >= midPoint)
            &&(pos1[iAxis] >= midPoint)
            &&(pos2[iAxis] >= midPoint) )
        {
            hiTris.emplace_back(i);
        }
        else
        {
            loTris.emplace_back(i);
        }
    }

    // This axis isn't working out, try another.
    if (hiTris.empty() || loTris.empty())
        return IHalf(src, out, exclAxis | (1 << iAxis));


    plGeometrySpan* loDst = IExtractTris(src, loTris);
    plGeometrySpan* hiDst = IExtractTris(src, hiTris);

    delete src;

    out.emplace_back(loDst);
    out.emplace_back(hiDst);

    return true;
}

int plGeoSpanDice::ISelectAxis(int exclAxis, plGeometrySpan* src) const
{
    int iAxis = -1;
    float maxDim = 0;

    int i;
    for( i = 0; i < 3; i++ )
    {
        // Check to see if we've already tried this one.
        if( exclAxis & (1 << i) )
            continue;

        float dim = src->fLocalBounds.GetMaxs()[i] - src->fLocalBounds.GetMins()[i];
        if( dim > maxDim )
        {
            maxDim = dim;
            iAxis = i;
        }
    }
    return iAxis;
}

plGeometrySpan* plGeoSpanDice::IExtractTris(plGeometrySpan* src, std::vector<uint32_t>& tris) const
{
    // First off, find out how many and which vers we're talking here.
    // Easiest way is while we're building the LUTs we'll want later anyway.
    std::vector<int16_t> fwdLUT(src->fNumVerts, -1);

    std::vector<uint16_t> bckLUT;

    for (uint32_t tri : tris)
    {
        uint16_t* idx = src->fIndexData + tri * 3;
        
        if( fwdLUT[*idx] < 0 )
        {
            fwdLUT[*idx] = (int16_t)bckLUT.size();
            bckLUT.emplace_back(*idx);
        }

        idx++;

        if( fwdLUT[*idx] < 0 )
        {
            fwdLUT[*idx] = (int16_t)bckLUT.size();
            bckLUT.emplace_back(*idx);
        }

        idx++;

        if( fwdLUT[*idx] < 0 )
        {
            fwdLUT[*idx] = (int16_t)bckLUT.size();
            bckLUT.emplace_back(*idx);
        }
    }

    int numVerts = (int)bckLUT.size();
    int numTris = (int)tris.size();

    plGeometrySpan* dst = IAllocSpace(src, numVerts, numTris);

    // Okay, set the index data.
    uint16_t* idxTrav = dst->fIndexData;
    for (uint32_t tri : tris)
    {
        *idxTrav++ = fwdLUT[src->fIndexData[tri * 3 + 0]];
        *idxTrav++ = fwdLUT[src->fIndexData[tri * 3 + 1]];
        *idxTrav++ = fwdLUT[src->fIndexData[tri * 3 + 2]];
    }

    // Copy over the basic vertex data
    // We'll update the bounds as we go.
    int stride = src->GetVertexSize(src->fFormat);

    hsBounds3Ext localBnd;
    localBnd.MakeEmpty();
    uint8_t* vtxTrav = dst->fVertexData;
    for (int i = 0; i < numVerts; i++)
    {
        memcpy(vtxTrav, src->fVertexData + bckLUT[i] * stride, stride);     

        hsPoint3* pos = (hsPoint3*)vtxTrav;
        localBnd.Union(pos);
        vtxTrav += stride;
    }
    if( src->fProps & plGeometrySpan::kWaterHeight )
    {
        src->AdjustBounds(localBnd);
    }
    dst->fLocalBounds = localBnd;

    // Now the rest of this optional garbage.
    if( src->fMultColor )
    {
        for (int i = 0; i < numVerts; i++)
        {
            dst->fMultColor[i] = src->fMultColor[bckLUT[i]];
        }
    }
    if( src->fAddColor )
    {
        for (int i = 0; i < numVerts; i++)
        {
            dst->fAddColor[i] = src->fAddColor[bckLUT[i]];
        }
    }
    if( src->fDiffuseRGBA )
    {
        for (int i = 0; i < numVerts; i++)
        {
            dst->fDiffuseRGBA[i] = src->fDiffuseRGBA[bckLUT[i]];
        }
    }
    if( src->fSpecularRGBA )
    {
        for (int i = 0; i < numVerts; i++)
        {
            dst->fSpecularRGBA[i] = src->fSpecularRGBA[bckLUT[i]];
        }
    }
    dst->fMaxOwner = src->fMaxOwner;

    return dst;
}

plGeometrySpan* plGeoSpanDice::IAllocSpace(plGeometrySpan* src, int numVerts, int numTris) const
{
    plGeometrySpan* dst = new plGeometrySpan;
    // Do a structure copy here. That's okay, because we're going to immediately 
    // fix up the pointers and counters that shouldn't have been copied. If
    // plGeometrySpan ever gets a copy constructor, this'll blow wide open.
    *dst = *src;

    int stride = src->GetVertexSize(src->fFormat);

    dst->fNumIndices = numTris * 3;
    dst->fIndexData = new uint16_t[dst->fNumIndices];

    dst->fNumVerts = numVerts;
    dst->fVertexData = new uint8_t[numVerts * stride];

    if( src->fMultColor )
    {
        dst->fMultColor = new hsColorRGBA[numVerts];
    }
    if( src->fAddColor )
    {
        dst->fAddColor = new hsColorRGBA[numVerts];
    }
    if( src->fDiffuseRGBA )
    {
        dst->fDiffuseRGBA = new uint32_t[numVerts];
    }
    if( src->fSpecularRGBA )
    {
        dst->fSpecularRGBA = new uint32_t[numVerts];
    }

    return dst;
}
