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

#include "plAccessGeometry.h"

#include "pnSceneObject/plDrawInterface.h"

#include "plDrawableSpans.h"
#include "plGeometrySpan.h"

#include "plAccessSpan.h"
#include "plAccessPartySpan.h"
#include "plAccessTriSpan.h"
#include "plAccessVtxSpan.h"
#include "plAccessSnapShot.h"

// For dipping directly into device buffers.
#include "plGBufferGroup.h"
#include "hsGDeviceRef.h"
#include "plPipeline.h"

#include "plTweak.h"

//////////////////////////////////////////////////////////////////////////////////
// Dropping these here, because they have no place else to go except a header.
void plAccessSpan::SetSource(plSpan* s)
{ 
    fLocalToWorld = &s->fLocalToWorld; 
    fWorldToLocal = &s->fWorldToLocal;
    fLocalBounds = &s->fLocalBounds;
    fWorldBounds = &s->fWorldBounds;

    fWaterHeight = s->fProps & plSpan::kWaterHeight ? &s->fWaterHeight : nullptr;
}
void plAccessSpan::SetSource(plGeometrySpan* s)
{ 
    fLocalToWorld = &s->fLocalToWorld; 
    fWorldToLocal = &s->fWorldToLocal;
    fLocalBounds = &s->fLocalBounds;
    fWorldBounds = &s->fWorldBounds;

    fWaterHeight = s->fProps & plGeometrySpan::kWaterHeight ? &s->fWaterHeight : nullptr;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
// Simple constructor

plAccessGeometry::plAccessGeometry(plPipeline* pipe)
:   fPipe(pipe)
{
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
// Global access stuff.
plAccessGeometry*   plAccessGeometry::fInstance = nullptr;

void plAccessGeometry::Init(plPipeline* pipe)
{
    plAccessGeometry* oldAcc = fInstance;

    fInstance = new plAccessGeometry(pipe);

    hsRefCnt_SafeUnRef(oldAcc);
}

void plAccessGeometry::DeInit()
{
    if( fInstance )
        fInstance->Nilify();
    hsRefCnt_SafeUnRef(fInstance);
}

void plAccessGeometry::SetTheIntance(plAccessGeometry* i)
{
    hsRefCnt_SafeAssign(fInstance, i);
}




//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

// The first couple of these just interpret between the SceneObjects we like to
// think about and the clumps of geometry that comprise each one.

void plAccessGeometry::OpenRO(const plDrawInterface* di, std::vector<plAccessSpan>& accs, bool useSnap) const
{
    int numGot = 0;
    accs.clear();
    accs.reserve(di->GetNumDrawables());
    for (size_t j = 0; j < di->GetNumDrawables(); j++)
    {
        plDrawableSpans* dr = plDrawableSpans::ConvertNoRef(di->GetDrawable(j));
        // Nil dr - it hasn't loaded yet or something.
        if( dr )
        {
            plDISpanIndex& diIndex = dr->GetDISpans(di->GetDrawableMeshIndex(j));
            if( !diIndex.IsMatrixOnly() )
            {
                for (size_t k = 0; k < diIndex.GetCount(); k++)
                {
                    accs.resize(numGot+1);
                    OpenRO(dr, diIndex[k], accs[numGot++]);
                }
            }
        }
    }

}

void plAccessGeometry::OpenRW(const plDrawInterface* di, std::vector<plAccessSpan>& accs, bool idxToo) const
{
    int numGot = 0;
    accs.clear();
    accs.reserve(di->GetNumDrawables());
    for (size_t j = 0; j < di->GetNumDrawables(); j++)
    {
        plDrawableSpans* dr = plDrawableSpans::ConvertNoRef(di->GetDrawable(j));
        // Nil dr - it hasn't loaded yet or something.
        if( dr )
        {
            plDISpanIndex& diIndex = dr->GetDISpans(di->GetDrawableMeshIndex(j));
            if( !diIndex.IsMatrixOnly() )
            {
                for (size_t k = 0; k < diIndex.GetCount(); k++)
                {
                    accs.resize(numGot+1);
                    OpenRW(dr, diIndex[k], accs[numGot++], idxToo);
                }
            }
        }
    }
}

void plAccessGeometry::Close(std::vector<plAccessSpan>& accs) const
{
    for (plAccessSpan& span : accs)
        Close(span);
}

void plAccessGeometry::TakeSnapShot(const plDrawInterface* di, uint32_t channels) const
{
    for (size_t j = 0; j < di->GetNumDrawables(); j++)
    {
        plDrawableSpans* dr = plDrawableSpans::ConvertNoRef(di->GetDrawable(j));
        // Nil dr - it hasn't loaded yet or something.
        if( dr )
        {
            plDISpanIndex& diIndex = dr->GetDISpans(di->GetDrawableMeshIndex(j));
            if( !diIndex.IsMatrixOnly() )
            {
                for (size_t k = 0; k < diIndex.GetCount(); k++)
                {
                    TakeSnapShot(dr, diIndex[k], channels);
                }
            }
        }
    }
}

void plAccessGeometry::RestoreSnapShot(const plDrawInterface* di, uint32_t channels) const
{
    for (size_t j = 0; j < di->GetNumDrawables(); j++)
    {
        plDrawableSpans* dr = plDrawableSpans::ConvertNoRef(di->GetDrawable(j));
        // Nil dr - it hasn't loaded yet or something.
        if( dr )
        {
            plDISpanIndex& diIndex = dr->GetDISpans(di->GetDrawableMeshIndex(j));
            if( !diIndex.IsMatrixOnly() )
            {
                for (size_t k = 0; k < diIndex.GetCount(); k++ )
                {
                    RestoreSnapShot(dr, diIndex[k], channels);
                }
            }
        }
    }
}

void plAccessGeometry::ReleaseSnapShot(const plDrawInterface* di) const
{
    for (size_t j = 0; j < di->GetNumDrawables(); j++)
    {
        plDrawableSpans* dr = plDrawableSpans::ConvertNoRef(di->GetDrawable(j));
        // Nil dr - it hasn't loaded yet or something.
        if( dr )
        {
            plDISpanIndex& diIndex = dr->GetDISpans(di->GetDrawableMeshIndex(j));
            if( !diIndex.IsMatrixOnly() )
            {
                for (size_t k = 0; k < diIndex.GetCount(); k++)
                {
                    ReleaseSnapShot(dr, diIndex[k]);
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

void plAccessGeometry::Close(plAccessSpan& acc) const
{
    if( !fPipe )
        return;

    fPipe->CloseAccess(acc);
}

void plAccessGeometry::IOpen(plDrawable* d, uint32_t spanIdx, plAccessSpan& acc, bool useSnap, bool readOnly, bool idxToo) const
{
    acc.SetType(plAccessSpan::kUndefined);

    plDrawableSpans* drawable = plDrawableSpans::ConvertNoRef(d);
    if( !drawable )
        return;

    if( drawable->GetSourceSpans().GetCount() && !drawable->GetNumSpans() )
        IAccessSpanFromSourceSpan(acc, drawable->GetSourceSpans()[spanIdx]);
    else
        IAccessSpanFromSpan(acc, drawable, drawable->GetSpan(spanIdx), useSnap, readOnly);

    if( !readOnly )
    {
        // Need to mark the drawable's data as dirty.
        plDrawableSpans* ds = plDrawableSpans::ConvertNoRef(drawable);
        if( !ds )
            return;
        if( acc.HasAccessVtx() )
        {
            plVertexSpan* vtx = (plVertexSpan*)ds->GetSpan(spanIdx);
            ds->DirtyVertexBuffer(vtx->fGroupIdx, vtx->fVBufferIdx);
        }

        if( idxToo && acc.HasAccessTri() )
        {
            plIcicle* ice = (plIcicle*)ds->GetSpan(spanIdx);
            ds->DirtyIndexBuffer(ice->fGroupIdx, ice->fIBufferIdx);
        }
    }
}

void plAccessGeometry::OpenRO(plDrawable* d, uint32_t spanIdx, plAccessSpan& acc, bool useSnap) const
{
    IOpen(d, spanIdx, acc, useSnap, true);
}

void plAccessGeometry::OpenRW(plDrawable* drawable, uint32_t spanIdx, plAccessSpan& acc, bool idxToo) const
{
    IOpen(drawable, spanIdx, acc, false, false, idxToo);

}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

void plAccessGeometry::TakeSnapShot(plDrawable* drawable, uint32_t spanIdx, uint32_t channels) const
{
    plDrawableSpans* ds = plDrawableSpans::ConvertNoRef(drawable);
    if( !ds )
        return;
    const plSpan* span = ds->GetSpan(spanIdx);

    if( !span->fSnapShot )
        span->fSnapShot = new plAccessSnapShot;

    plAccessSpan tmp;
    OpenRO(drawable, spanIdx, tmp, false);

    if( tmp.HasAccessVtx() )
    {
        span->fSnapShot->IncRef();
        span->fSnapShot->CopyFrom(tmp.AccessVtx(), channels);
    }
}

void plAccessGeometry::RestoreSnapShot(plDrawable* drawable, uint32_t spanIdx, uint32_t channels) const
{
    plDrawableSpans* ds = plDrawableSpans::ConvertNoRef(drawable);
    if( !ds )
        return;
    const plSpan* span = ds->GetSpan(spanIdx);

    if( !span->fSnapShot )
        return;

    plAccessSpan tmp;
    OpenRW(drawable, spanIdx, tmp);

    if( tmp.HasAccessVtx() )
        span->fSnapShot->CopyTo(tmp.AccessVtx(), channels);
}

void plAccessGeometry::ReleaseSnapShot(plDrawable* drawable, uint32_t spanIdx) const
{
    plDrawableSpans* ds = plDrawableSpans::ConvertNoRef(drawable);
    if( !ds )
        return;
    const plSpan* span = ds->GetSpan(spanIdx);

    if( !span->fSnapShot )
        return;

    span->fSnapShot->Release();
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

void plAccessGeometry::IAccessSpanFromSourceSpan(plAccessSpan& dst, const plGeometrySpan* src) const
{
    // Get the connectivity info
    dst.SetType(plAccessSpan::kTri);
    plAccessTriSpan& tri = dst.AccessTri();
    tri.fNumTris = src->fNumIndices / 3;
    tri.fTris = src->fIndexData;
    tri.fIdxDeviceRef = nullptr;

    dst.SetSource(const_cast<plGeometrySpan*>(src));
    dst.SetMaterial(src->fMaterial);

    plAccessVtxSpan& acc = dst.AccessVtx();
    acc.fNumVerts = (uint16_t)(src->fNumVerts);
    uint32_t sz = src->GetVertexSize(src->fFormat);
    uint8_t* ptr = src->fVertexData;
    acc.PositionStream(ptr, (uint16_t)sz, 0);
    ptr += sizeof(hsPoint3);
    acc.NormalStream(ptr, (uint16_t)sz, 0);
    ptr += sizeof(hsVector3);
    acc.DiffuseStream(src->fDiffuseRGBA, sizeof(uint32_t), 0);
    acc.SpecularStream(src->fSpecularRGBA, sizeof(uint32_t), 0);
    acc.UVWStream(ptr, (uint16_t)sz, 0);
    acc.SetNumUVWs(src->CalcNumUVs(src->fFormat));

    acc.fVtxDeviceRef = nullptr;
}

void plAccessGeometry::IAccessSpanFromSpan(plAccessSpan& dst, plDrawableSpans* drawable, const plSpan* span, bool useSnap, bool readOnly) const
{
    dst.SetType(plAccessSpan::kUndefined);
    dst.SetSource(const_cast<plSpan*> (span));
    if( span->fTypeMask & plSpan::kIcicleSpan )
    {
        IAccessSpanFromIcicle(dst, drawable, (const plIcicle*)span, readOnly);
    }
    else if( span->fTypeMask & plSpan::kParticleSpan )
    {
        IAccessSpanFromParticle(dst, drawable, (const plParticleSpan*)span, readOnly);
    }
    if( useSnap )
    {
        IAccessSpanFromSnap(dst, drawable, span);
    }
}

void plAccessGeometry::IAccessSpanFromSnap(plAccessSpan& dst, plDrawableSpans* drawable, const plSpan* span) const
{
    plAccessVtxSpan& acc = dst.AccessVtx();
    if( span->fSnapShot )
    {
        span->fSnapShot->SetupChannels(acc);
    }
}

void plAccessGeometry::IAccessSpanFromVertexSpan(plAccessSpan& dst, plDrawableSpans* drawable, const plVertexSpan* span, bool readOnly) const
{
    dst.SetMaterial(drawable->GetMaterial(span->fMaterialIdx));

    plAccessVtxSpan& acc = dst.AccessVtx();
    
    plGBufferGroup* grp = drawable->GetBufferGroup(span->fGroupIdx);

//#define MF_TOSSER
#ifndef MF_TOSSER
    plConst(bool) useDev(false);
#else // MF_TOSSER
    plConst(bool) useDev(true);
#endif // MF_TOSSER
    if( useDev && !drawable->GetNativeProperty(plDrawable::kPropVolatile) && grp->GetVertexBufferRef(span->fVBufferIdx) )
    {
        fPipe->OpenAccess(dst, drawable, span, readOnly);
        return;
    }

    acc.fNumVerts = (uint16_t)(span->fVLength);

    plGBufferCell* cell = grp->GetCell(span->fVBufferIdx, span->fCellIdx);

    uint8_t* ptr = grp->GetVertBufferData(span->fVBufferIdx);

    // Interleaved
    if( cell->fColorStart == uint32_t(-1) )
    {
        uint32_t stride = grp->GetVertexSize();

        ptr += cell->fVtxStart + span->fCellOffset * stride;
        int32_t offset = (-(int32_t)(span->fVStartIdx)) * (int32_t)stride;

        acc.PositionStream(ptr, (uint16_t)stride, offset);
        ptr += sizeof(hsPoint3);

        int numWgts = grp->GetNumWeights();
        if( numWgts )
        {
            acc.SetNumWeights(numWgts);
            acc.WeightStream(ptr, (uint16_t)stride, offset);
            ptr += numWgts * sizeof(float);
            if( grp->GetVertexFormat() & plGBufferGroup::kSkinIndices )
            {
                acc.WgtIndexStream(ptr, (uint16_t)stride, offset);
                ptr += sizeof(uint32_t);
            }
            else
            {
                acc.WgtIndexStream(nullptr, 0, offset);
            }
        }
        else
        {
            acc.SetNumWeights(0);
        }

        acc.NormalStream(ptr, (uint16_t)stride, offset);
        ptr += sizeof(hsVector3);

        acc.DiffuseStream(ptr, (uint16_t)stride, offset);
        ptr += sizeof(uint32_t);

        acc.SpecularStream(ptr, (uint16_t)stride, offset);
        ptr += sizeof(uint32_t);

        acc.UVWStream(ptr, (uint16_t)stride, offset);

        acc.SetNumUVWs(grp->GetNumUVs());

    }
    else
    {
        uint32_t stride = grp->GetVertexLiteStride();

        ptr += cell->fVtxStart + span->fCellOffset * stride;
        int32_t posOffset = (-(int32_t)(span->fVStartIdx)) * (int32_t)stride;

        acc.PositionStream(ptr, (uint16_t)stride, posOffset);
        ptr += sizeof(hsPoint3);

        int numWgts = grp->GetNumWeights();
        if( numWgts )
        {
            acc.SetNumWeights(numWgts);
            acc.WeightStream(ptr, (uint16_t)stride, posOffset);
            ptr += numWgts * sizeof(float);
            if( grp->GetVertexFormat() & plGBufferGroup::kSkinIndices )
            {
                acc.WgtIndexStream(ptr, (uint16_t)stride, posOffset);
                ptr += sizeof(uint32_t);
            }
            else
            {
                acc.WgtIndexStream(nullptr, 0, 0);
            }
        }
        else
        {
            acc.SetNumWeights(0);
        }

        acc.NormalStream(ptr, (uint16_t)stride, posOffset);
        ptr += sizeof(hsVector3);

        plGBufferColor* col = grp->GetColorBufferData(span->fVBufferIdx) + cell->fColorStart;

        int16_t colOffset = (int16_t)((-(int32_t)(span->fVStartIdx)) * (int32_t)stride);
        colOffset = (int16_t)((-(int32_t)(span->fVStartIdx)) * sizeof(*col));

        acc.DiffuseStream(&col->fDiffuse, sizeof(*col), colOffset);

        acc.SpecularStream(&col->fSpecular, sizeof(*col), colOffset);

        acc.UVWStream(ptr, (uint16_t)stride, posOffset);

        acc.SetNumUVWs(grp->GetNumUVs());
    }

    acc.fVtxDeviceRef = nullptr;
}

void plAccessGeometry::IAccessConnectivity(plAccessSpan& dst, plDrawableSpans* drawable, const plSpan* src) const
{
    if( src->fTypeMask & plSpan::kIcicleSpan )
    {
        const plIcicle* span = (const plIcicle*)src;

        dst.SetType(plAccessSpan::kTri);

        plAccessTriSpan& acc = dst.AccessTri();
        
        acc.fNumTris = span->fILength / 3;
        plGBufferGroup* grp = drawable->GetBufferGroup(span->fGroupIdx);
        acc.fTris = grp->GetIndexBufferData(span->fIBufferIdx) + span->fIStartIdx;

        acc.fIdxDeviceRef = nullptr;
    }
    // Hmm, particle should probably go here...
    else 
    {
        dst.SetType(plAccessSpan::kVtx);
    }

}

void plAccessGeometry::IAccessSpanFromIcicle(plAccessSpan& dst, plDrawableSpans* drawable, const plIcicle* span, bool readOnly) const
{
    dst.SetType(plAccessSpan::kTri);

    plAccessTriSpan& acc = dst.AccessTri();
    
    IAccessSpanFromVertexSpan(dst, drawable, span, readOnly);
    
    acc.fNumTris = span->fILength / 3;
    plGBufferGroup* grp = drawable->GetBufferGroup(span->fGroupIdx);
    acc.fTris = grp->GetIndexBufferData(span->fIBufferIdx) + span->fIStartIdx;

    acc.fIdxDeviceRef = nullptr;
}

void plAccessGeometry::IAccessSpanFromParticle(plAccessSpan& dst, plDrawableSpans* drawable, const plParticleSpan* span, bool readOnly) const
{
    hsAssert(false, "Aint got to it yet");
    // dst.SetType(plAccessSpan::kParty);
}




