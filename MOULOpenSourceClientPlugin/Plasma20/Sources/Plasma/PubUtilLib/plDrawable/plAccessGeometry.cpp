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

#include "plAccessGeometry.h"

#include "../pnSceneObject/plDrawInterface.h"

#include "plDrawableSpans.h"
#include "plGeometrySpan.h"

#include "plAccessSpan.h"
#include "plAccessPartySpan.h"
#include "plAccessTriSpan.h"
#include "plAccessVtxSpan.h"
#include "plAccessSnapShot.h"

// For dipping directly into device buffers.
#include "../plPipeline/plGBufferGroup.h"
#include "../plPipeline/hsGDeviceRef.h"
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

	fWaterHeight = s->fProps & plSpan::kWaterHeight ? &s->fWaterHeight : nil;
}
void plAccessSpan::SetSource(plGeometrySpan* s)
{ 
	fLocalToWorld = &s->fLocalToWorld; 
	fWorldToLocal = &s->fWorldToLocal;
	fLocalBounds = &s->fLocalBounds;
	fWorldBounds = &s->fWorldBounds;

	fWaterHeight = s->fProps & plGeometrySpan::kWaterHeight ? &s->fWaterHeight : nil;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
// Simple constructor

plAccessGeometry::plAccessGeometry(plPipeline* pipe)
:	fPipe(pipe)
{
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
// Global access stuff.
plAccessGeometry*	plAccessGeometry::fInstance = nil;

void plAccessGeometry::Init(plPipeline* pipe)
{
	plAccessGeometry* oldAcc = fInstance;

	fInstance = NEW(plAccessGeometry)(pipe);

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

void plAccessGeometry::OpenRO(const plDrawInterface* di, hsTArray<plAccessSpan>& accs, hsBool useSnap) const
{
	int numGot = 0;
	accs.SetCount(di->GetNumDrawables());
	accs.SetCount(0);
	int j;
	for( j = 0; j < di->GetNumDrawables(); j++ )
	{
		plDrawableSpans* dr = plDrawableSpans::ConvertNoRef(di->GetDrawable(j));
		// Nil dr - it hasn't loaded yet or something.
		if( dr )
		{
			plDISpanIndex& diIndex = dr->GetDISpans(di->GetDrawableMeshIndex(j));
			if( !diIndex.IsMatrixOnly() )
			{
				int k;
				for( k = 0; k < diIndex.GetCount(); k++ )
				{
					accs.Expand(numGot+1);
					accs.SetCount(numGot+1);
					OpenRO(dr, diIndex[k], accs[numGot++]);
				}
			}
		}
	}

}

void plAccessGeometry::OpenRW(const plDrawInterface* di, hsTArray<plAccessSpan>& accs, hsBool idxToo) const
{
	int numGot = 0;
	accs.Expand(di->GetNumDrawables());
	accs.SetCount(0);
	int j;
	for( j = 0; j < di->GetNumDrawables(); j++ )
	{
		plDrawableSpans* dr = plDrawableSpans::ConvertNoRef(di->GetDrawable(j));
		// Nil dr - it hasn't loaded yet or something.
		if( dr )
		{
			plDISpanIndex& diIndex = dr->GetDISpans(di->GetDrawableMeshIndex(j));
			if( !diIndex.IsMatrixOnly() )
			{
				int k;
				for( k = 0; k < diIndex.GetCount(); k++ )
				{
					accs.Expand(numGot+1);
					accs.SetCount(numGot+1);
					OpenRW(dr, diIndex[k], accs[numGot++], idxToo);
				}
			}
		}
	}
}

void plAccessGeometry::Close(hsTArray<plAccessSpan>& accs) const
{
	int i;
	for( i = 0; i < accs.GetCount(); i++ )
		Close(accs[i]);
}

void plAccessGeometry::TakeSnapShot(const plDrawInterface* di, UInt32 channels) const
{
	int j;
	for( j = 0; j < di->GetNumDrawables(); j++ )
	{
		plDrawableSpans* dr = plDrawableSpans::ConvertNoRef(di->GetDrawable(j));
		// Nil dr - it hasn't loaded yet or something.
		if( dr )
		{
			plDISpanIndex& diIndex = dr->GetDISpans(di->GetDrawableMeshIndex(j));
			if( !diIndex.IsMatrixOnly() )
			{
				int k;
				for( k = 0; k < diIndex.GetCount(); k++ )
				{
					TakeSnapShot(dr, diIndex[k], channels);
				}
			}
		}
	}
}

void plAccessGeometry::RestoreSnapShot(const plDrawInterface* di, UInt32 channels) const
{
	int j;
	for( j = 0; j < di->GetNumDrawables(); j++ )
	{
		plDrawableSpans* dr = plDrawableSpans::ConvertNoRef(di->GetDrawable(j));
		// Nil dr - it hasn't loaded yet or something.
		if( dr )
		{
			plDISpanIndex& diIndex = dr->GetDISpans(di->GetDrawableMeshIndex(j));
			if( !diIndex.IsMatrixOnly() )
			{
				int k;
				for( k = 0; k < diIndex.GetCount(); k++ )
				{
					RestoreSnapShot(dr, diIndex[k], channels);
				}
			}
		}
	}
}

void plAccessGeometry::ReleaseSnapShot(const plDrawInterface* di) const
{
	int j;
	for( j = 0; j < di->GetNumDrawables(); j++ )
	{
		plDrawableSpans* dr = plDrawableSpans::ConvertNoRef(di->GetDrawable(j));
		// Nil dr - it hasn't loaded yet or something.
		if( dr )
		{
			plDISpanIndex& diIndex = dr->GetDISpans(di->GetDrawableMeshIndex(j));
			if( !diIndex.IsMatrixOnly() )
			{
				int k;
				for( k = 0; k < diIndex.GetCount(); k++ )
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

void plAccessGeometry::IOpen(plDrawable* d, UInt32 spanIdx, plAccessSpan& acc, hsBool useSnap, hsBool readOnly, hsBool idxToo) const
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

void plAccessGeometry::OpenRO(plDrawable* d, UInt32 spanIdx, plAccessSpan& acc, hsBool useSnap) const
{
	IOpen(d, spanIdx, acc, useSnap, true);
}

void plAccessGeometry::OpenRW(plDrawable* drawable, UInt32 spanIdx, plAccessSpan& acc, hsBool idxToo) const
{
	IOpen(drawable, spanIdx, acc, false, false, idxToo);

}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

void plAccessGeometry::TakeSnapShot(plDrawable* drawable, UInt32 spanIdx, UInt32 channels) const
{
	plDrawableSpans* ds = plDrawableSpans::ConvertNoRef(drawable);
	if( !ds )
		return;
	const plSpan* span = ds->GetSpan(spanIdx);

	if( !span->fSnapShot )
		span->fSnapShot = NEW(plAccessSnapShot);

	plAccessSpan tmp;
	OpenRO(drawable, spanIdx, tmp, false);

	if( tmp.HasAccessVtx() )
	{
		span->fSnapShot->IncRef();
		span->fSnapShot->CopyFrom(tmp.AccessVtx(), channels);
	}
}

void plAccessGeometry::RestoreSnapShot(plDrawable* drawable, UInt32 spanIdx, UInt32 channels) const
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

void plAccessGeometry::ReleaseSnapShot(plDrawable* drawable, UInt32 spanIdx) const
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
	tri.fIdxDeviceRef = nil;

	dst.SetSource(const_cast<plGeometrySpan*>(src));
	dst.SetMaterial(src->fMaterial);

	plAccessVtxSpan& acc = dst.AccessVtx();
	acc.fNumVerts = (UInt16)(src->fNumVerts);
	UInt32 sz = src->GetVertexSize(src->fFormat);
	UInt8* ptr = src->fVertexData;
	acc.PositionStream(ptr, (UInt16)sz, 0);
	ptr += sizeof(hsPoint3);
	acc.NormalStream(ptr, (UInt16)sz, 0);
	ptr += sizeof(hsVector3);
	acc.DiffuseStream(src->fDiffuseRGBA, sizeof(UInt32), 0);
	acc.SpecularStream(src->fSpecularRGBA, sizeof(UInt32), 0);
	acc.UVWStream(ptr, (UInt16)sz, 0);
	acc.SetNumUVWs(src->CalcNumUVs(src->fFormat));

	acc.fVtxDeviceRef = nil;
}

void plAccessGeometry::IAccessSpanFromSpan(plAccessSpan& dst, plDrawableSpans* drawable, const plSpan* span, hsBool useSnap, hsBool readOnly) const
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

void plAccessGeometry::IAccessSpanFromVertexSpan(plAccessSpan& dst, plDrawableSpans* drawable, const plVertexSpan* span, hsBool readOnly) const
{
	dst.SetMaterial(drawable->GetMaterial(span->fMaterialIdx));

	plAccessVtxSpan& acc = dst.AccessVtx();
	
	plGBufferGroup* grp = drawable->GetBufferGroup(span->fGroupIdx);

//#define MF_TOSSER
#ifndef MF_TOSSER
	plConst(hsBool) useDev(false);
#else // MF_TOSSER
	plConst(hsBool) useDev(true);
#endif // MF_TOSSER
	if( useDev && !drawable->GetNativeProperty(plDrawable::kPropVolatile) && grp->GetVertexBufferRef(span->fVBufferIdx) )
	{
		fPipe->OpenAccess(dst, drawable, span, readOnly);
		return;
	}

	acc.fNumVerts = (UInt16)(span->fVLength);

	plGBufferCell* cell = grp->GetCell(span->fVBufferIdx, span->fCellIdx);

	UInt8* ptr = grp->GetVertBufferData(span->fVBufferIdx);

	// Interleaved
	if( cell->fColorStart == UInt32(-1) )
	{
		UInt32 stride = grp->GetVertexSize();

		ptr += cell->fVtxStart + span->fCellOffset * stride;
		Int32 offset = (-(Int32)(span->fVStartIdx)) * (Int32)stride;

		acc.PositionStream(ptr, (UInt16)stride, offset);
		ptr += sizeof(hsPoint3);

		int numWgts = grp->GetNumWeights();
		if( numWgts )
		{
			acc.SetNumWeights(numWgts);
			acc.WeightStream(ptr, (UInt16)stride, offset);
			ptr += numWgts * sizeof(hsScalar);
			if( grp->GetVertexFormat() & plGBufferGroup::kSkinIndices )
			{
				acc.WgtIndexStream(ptr, (UInt16)stride, offset);
				ptr += sizeof(UInt32);
			}
			else
			{
				acc.WgtIndexStream(nil, 0, offset);
			}
		}
		else
		{
			acc.SetNumWeights(0);
		}

		acc.NormalStream(ptr, (UInt16)stride, offset);
		ptr += sizeof(hsVector3);

		acc.DiffuseStream(ptr, (UInt16)stride, offset);
		ptr += sizeof(UInt32);

		acc.SpecularStream(ptr, (UInt16)stride, offset);
		ptr += sizeof(UInt32);

		acc.UVWStream(ptr, (UInt16)stride, offset);

		acc.SetNumUVWs(grp->GetNumUVs());

	}
	else
	{
		UInt32 stride = grp->GetVertexLiteStride();

		ptr += cell->fVtxStart + span->fCellOffset * stride;
		Int32 posOffset = (-(Int32)(span->fVStartIdx)) * (Int32)stride;

		acc.PositionStream(ptr, (UInt16)stride, posOffset);
		ptr += sizeof(hsPoint3);

		int numWgts = grp->GetNumWeights();
		if( numWgts )
		{
			acc.SetNumWeights(numWgts);
			acc.WeightStream(ptr, (UInt16)stride, posOffset);
			ptr += numWgts * sizeof(hsScalar);
			if( grp->GetVertexFormat() & plGBufferGroup::kSkinIndices )
			{
				acc.WgtIndexStream(ptr, (UInt16)stride, posOffset);
				ptr += sizeof(UInt32);
			}
			else
			{
				acc.WgtIndexStream(nil, 0, 0);
			}
		}
		else
		{
			acc.SetNumWeights(0);
		}

		acc.NormalStream(ptr, (UInt16)stride, posOffset);
		ptr += sizeof(hsVector3);

		plGBufferColor* col = grp->GetColorBufferData(span->fVBufferIdx) + cell->fColorStart;

		Int16 colOffset = (Int16)((-(Int32)(span->fVStartIdx)) * (Int32)stride);
		colOffset = (Int16)((-(Int32)(span->fVStartIdx)) * sizeof(*col));

		acc.DiffuseStream(&col->fDiffuse, sizeof(*col), colOffset);

		acc.SpecularStream(&col->fSpecular, sizeof(*col), colOffset);

		acc.UVWStream(ptr, (UInt16)stride, posOffset);

		acc.SetNumUVWs(grp->GetNumUVs());
	}

	acc.fVtxDeviceRef = nil;
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

		acc.fIdxDeviceRef = nil;
	}
	// Hmm, particle should probably go here...
	else 
	{
		dst.SetType(plAccessSpan::kVtx);
	}

}

void plAccessGeometry::IAccessSpanFromIcicle(plAccessSpan& dst, plDrawableSpans* drawable, const plIcicle* span, hsBool readOnly) const
{
	dst.SetType(plAccessSpan::kTri);

	plAccessTriSpan& acc = dst.AccessTri();
	
	IAccessSpanFromVertexSpan(dst, drawable, span, readOnly);
	
	acc.fNumTris = span->fILength / 3;
	plGBufferGroup* grp = drawable->GetBufferGroup(span->fGroupIdx);
	acc.fTris = grp->GetIndexBufferData(span->fIBufferIdx) + span->fIStartIdx;

	acc.fIdxDeviceRef = nil;
}

void plAccessGeometry::IAccessSpanFromParticle(plAccessSpan& dst, plDrawableSpans* drawable, const plParticleSpan* span, hsBool readOnly) const
{
	hsAssert(false, "Aint got to it yet");
	// dst.SetType(plAccessSpan::kParty);
}




