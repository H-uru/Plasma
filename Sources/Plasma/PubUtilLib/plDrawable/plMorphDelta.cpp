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
#include "plMorphDelta.h"

#include "hsStream.h"

#include "plAccessGeometry.h"
#include "plAccessSpan.h"
#include "plAccessVtxSpan.h"
#include "plGeometrySpan.h"

#include "plTweak.h"

static const float kMinWeight = 1.e-2f;


plMorphDelta& plMorphDelta::operator=(const plMorphDelta& src)
{
    SetNumSpans(src.GetNumSpans());
    for (size_t i = 0; i < fSpans.size(); i++)
    {
        SetDeltas(i, src.fSpans[i].fDeltas, src.fSpans[i].fNumUVWChans, src.fSpans[i].fUVWs);
    }
    return *this;
}

void plMorphDelta::Apply(std::vector<plAccessSpan>& dst, float weight /* = -1.f */) const
{
    if( weight == -1.f)
        weight = fWeight; // None passed in, use our stored value

    if( weight <= kMinWeight )
        return;

    // Easy
    // For each span
    for (size_t iSpan = 0; iSpan < fSpans.size(); iSpan++)
    {
        plAccessVtxSpan& vtxDst = dst[iSpan].AccessVtx();

        const plMorphSpan& span = fSpans[iSpan];

        // For each vertDelta
        const hsPoint3* uvwDel = span.fUVWs;
        for (const plVertDelta& delta : span.fDeltas)
        {
            // Add delPos * wgt to position
            // Add delNorm * wgt to normal
            vtxDst.Position(delta.fIdx) += delta.fPos * weight;
            vtxDst.Normal(delta.fIdx) += delta.fNorm * weight;

            // Leave skin weights and indices alone?

            // Skip color for now, since diffuse and specular are
            // ignored on the avatar?
            // // Add delDiff * wgt to diffuse
            // // Add delSpec * wgt to specular

            // For each UVW
            hsPoint3* uvws = vtxDst.UVWs(delta.fIdx);
            int iUVW;
            for( iUVW = 0; iUVW < span.fNumUVWChans; iUVW++ )
            {
                // Add delUVW * wgt to uvw
                *uvws += *uvwDel * weight;
                uvws++;
                uvwDel++;
            }
        }
    }
}

// MorphDelta - ComputeDeltas
void plMorphDelta::ComputeDeltas(const std::vector<plAccessSpan>& base, const std::vector<plAccessSpan>& moved)
{
    SetNumSpans(base.size());

    // For each span
    {
        // for( i = 0; i < numVerts; i++ )
        {
            // NOTE: we want to discard zero deltas, but a
            // delta in any channel forces us to save the whole thing.
            // But we don't want to compare to zero (because we'll end
            // up with a lot of near zero deltas), but the epsilon we
            // compare to needs to be different for comparing something
            // like a normal delta and a position delta.
            //
            // For position, normal, color and all uvws
            // Calc del and delLenSq
            // If any delLenSq big enough, set nonZero to true
            // If nonZero
            {
                // Append to deltas (i, del's)
            }
        }
    }
}

// MorphDelta - ComputeDeltas
void plMorphDelta::ComputeDeltas(const std::vector<plGeometrySpan*>& base, const std::vector<plGeometrySpan*>& moved, const hsMatrix44& d2b, const hsMatrix44& d2bTInv)
{
    SetNumSpans(base.size());

    hsPoint3 delUVWs[8];

    // For each span
    for (size_t iSpan = 0; iSpan < base.size(); iSpan++)
    {
        plAccessSpan baseAcc;
        plAccessGeometry::Instance()->AccessSpanFromGeometrySpan(baseAcc, base[iSpan]);
        plAccessSpan movedAcc;
        plAccessGeometry::Instance()->AccessSpanFromGeometrySpan(movedAcc, moved[iSpan]);

        plAccPosNormUVWIterator baseIter(&baseAcc.AccessVtx());
        plAccPosNormUVWIterator movedIter(&movedAcc.AccessVtx());


        const uint16_t numUVWs = baseAcc.AccessVtx().NumUVWs();

        std::vector<plVertDelta> deltas;
        std::vector<hsPoint3> uvws;

        uint16_t iVert = 0;
        for( baseIter.Begin(), movedIter.Begin(); baseIter.More(); baseIter.Advance(), movedIter.Advance() )
        {
            // NOTE: we want to discard zero deltas, but a
            // delta in any channel forces us to save the whole thing.
            // But we don't want to compare to zero (because we'll end
            // up with a lot of near zero deltas), but the epsilon we
            // compare to needs to be different for comparing something
            // like a normal delta and a position delta.
            //
            // For position, normal, color and all uvws
            // Calc del and delLenSq
            // If any delLenSq big enough, set nonZero to true
            bool nonZero = false;

            // These are actually min del SQUARED.
            plConst(float) kMinDelPos(1.e-4f); // From Budtpueller's Handbook of Constants
            plConst(float) kMinDelNorm(3.e-2f); // About 10 degrees
            plConst(float) kMinDelUVW(1.e-5f); // Science'd from clothing UVW work
            hsPoint3 mPos = d2b * *movedIter.Position();
            hsVector3 delPos( &mPos, baseIter.Position());
            float delPosSq = delPos.MagnitudeSquared();
            if( delPosSq > kMinDelPos )
                nonZero = true;
            else
                delPos.Set(0,0,0);


            hsVector3 delNorm = (d2bTInv * *movedIter.Normal()) - *baseIter.Normal();
            float delNormSq = delNorm.MagnitudeSquared();
            if( delNormSq > kMinDelNorm )
                nonZero = true;
            else
                delNorm.Set(0,0,0);

            for (uint16_t i = 0; i < numUVWs; i++)
            {
                delUVWs[i] = *movedIter.UVW(i) - *baseIter.UVW(i);
                float delUVWSq = delUVWs[i].MagnitudeSquared();
                if( delUVWSq > kMinDelUVW )
                    nonZero = true;
                else
                    delUVWs[i].Set(0,0,0);
            }

            if( nonZero )
            {
                // Append to deltas (i, del's)
                deltas.emplace_back(iVert, delPos, delNorm);

                for (uint16_t i = 0; i < numUVWs; i++)
                    uvws.emplace_back(delUVWs[i]);
            }
            else
            {
                nonZero = false; // Breakpoint.
            }

            iVert++;
        }
        SetDeltas(iSpan, deltas, numUVWs, uvws.data());
    }
}

void plMorphDelta::SetNumSpans(size_t n)
{
    fSpans.clear();
    fSpans.resize(n);
}

void plMorphDelta::AllocDeltas(size_t iSpan, size_t nDel, size_t nUVW)
{
    fSpans[iSpan].fDeltas.resize(nDel);
    fSpans[iSpan].fNumUVWChans = (uint16_t)nUVW;

    delete [] fSpans[iSpan].fUVWs;

    size_t uvwCnt = nDel * nUVW;
    if( uvwCnt )
        fSpans[iSpan].fUVWs = new hsPoint3[uvwCnt];
    else
        fSpans[iSpan].fUVWs = nullptr;
}

void plMorphDelta::SetDeltas(size_t iSpan, const std::vector<plVertDelta>& deltas, size_t numUVWChans, const hsPoint3* uvws)
{
    AllocDeltas(iSpan, deltas.size(), numUVWChans);
    if (!deltas.empty())
    {
        fSpans[iSpan].fDeltas = deltas;

        if (numUVWChans)
            std::copy(uvws, uvws + (deltas.size() * numUVWChans), fSpans[iSpan].fUVWs);
    }
}

void plMorphDelta::Read(hsStream* s, hsResMgr* mgr)
{
    fWeight = s->ReadLEFloat();

    uint32_t n = s->ReadLE32();
    SetNumSpans(n);
    for (uint32_t iSpan = 0; iSpan < n; iSpan++)
    {
        uint32_t nDel = s->ReadLE32();
        uint32_t nUVW = s->ReadLE32();
        AllocDeltas(iSpan, nDel, nUVW);
        if( nDel )
        {
            s->Read(nDel * sizeof(plVertDelta), fSpans[iSpan].fDeltas.data());
            if( nUVW )
                s->Read(nDel * nUVW * sizeof(hsPoint3), fSpans[iSpan].fUVWs);
        }
    }

}

void plMorphDelta::Write(hsStream* s, hsResMgr* mgr)
{
    s->WriteLEFloat(fWeight);

    s->WriteLE32((uint32_t)fSpans.size());

    for (const plMorphSpan& span : fSpans)
    {
        size_t nDel = span.fDeltas.size();
        uint32_t nUVW = span.fNumUVWChans;
        s->WriteLE32((uint32_t)nDel);
        s->WriteLE32(nUVW);

        if( nDel )
        {
            s->Write(nDel * sizeof(plVertDelta), span.fDeltas.data());

            if( nUVW )
                s->Write(nDel * nUVW * sizeof(hsPoint3), span.fUVWs);
        }
    }
}
