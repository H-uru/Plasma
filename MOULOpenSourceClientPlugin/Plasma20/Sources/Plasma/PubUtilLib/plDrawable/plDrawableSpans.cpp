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
//////////////////////////////////////////////////////////////////////////////
//																			//
//	plDrawableSpans Class Functions											//
//																			//
//// Version History /////////////////////////////////////////////////////////
//																			//
//	4.3.2001 mcn - Created.													//
//	5.3.2001 mcn - Completely revamped. Now plDrawableSpans *IS* the group	//
//				   of spans, and each span is either an icicle or patch.	//
//				   This eliminates the need entirely for separate drawables,//
//				   at the cost of having to do a bit of extra work to		//
//				   maintain different types of spans in the same drawable.	//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"

#include "plAccessSpan.h"
#include "plAccessTriSpan.h"

#include "plDrawableSpans.h"
#include "hsStream.h"
#include "hsResMgr.h"
#include "plPipeline.h"
#include "plGeometrySpan.h"
#include "plSpaceTree.h"
#include "plParticleFiller.h"
#include "plSpaceTreeMaker.h"

#include "plClusterGroup.h"
#include "plCluster.h"
#include "plSpanTemplate.h"
									
#include "../plMath/hsRadixSort.h"
#include "../plSurface/hsGMaterial.h"
#include "../plSurface/plLayerInterface.h"
#include "../plPipeline/plFogEnvironment.h"
#include "../plPipeline/hsGDeviceRef.h"
#include "../plPipeline/plPipeDebugFlags.h"
#include "../pnMessage/plRefMsg.h"
#include "../pnMessage/plNodeRefMsg.h" 
#include "../pnMessage/plDISpansMsg.h"
#include "../plMessage/plDeviceRecreateMsg.h"
#include "../plMessage/plRenderMsg.h"
#include "../plPipeline/plGBufferGroup.h"
#include "../pnSceneObject/plDrawInterface.h"
#include "../pnKeyedObject/plKey.h"
#include "../plParticleSystem/plParticleEmitter.h"
#include "../plParticleSystem/plParticle.h"
#include "../plGLight/plLightInfo.h"
#include "plgDispatch.h"
#include "plProfile.h"

#include "../plMath/plTriUtils.h"

#include "../pnMessage/plPipeResMakeMsg.h"

#include "../plScene/plVisMgr.h"
#include "../plScene/plVisRegion.h"

#include <algorithm>

//// Local Konstants /////////////////////////////////////////////////////////

const UInt32		plDrawableSpans::kSpanTypeMask			= 0xc0000000;
const UInt32		plDrawableSpans::kSpanIDMask			= ~kSpanTypeMask;
const UInt32		plDrawableSpans::kSpanTypeIcicle		= 0x00000000;
const UInt32		plDrawableSpans::kSpanTypeParticleSpan	= 0xc0000000;

//// Constructor & Destructor ////////////////////////////////////////////////

plDrawableSpans::plDrawableSpans() :
	fSceneNode(nil),
	fSpaceTree(nil)
{
	fReadyToRender = false;
	fProps = 0;
	fCriteria = 0;
	fRegisteredForRecreate = false;
	fRegisteredForRender = false;
	fNeedCleanup = false;

	fOptimized = true;
	
	fSettingMatIdxLock = false;

	fSkinTime = 0;

	fType = kNormal;
	fMaterials.Reset();

	fLocalToWorld.Reset();
	fWorldToLocal.Reset();

	fVisSet.SetBit(0);
}

plDrawableSpans::~plDrawableSpans()
{
	int			i;


	for( i = 0; i < fGroups.GetCount(); i++ )
	{
		delete fGroups[ i ];
	}
	fGroups.Reset();

	/// Loop and delete both our types of spans
	for( i = 0; i < fSpans.GetCount(); i++ )
		fSpans[ i ]->Destroy();
	fSpans.Reset();
	fIcicles.Reset();
	fParticleSpans.Reset();

	for( i = 0; i < fSourceSpans.GetCount(); i++ )
		delete fSourceSpans[ i ];
	fSourceSpans.Reset();

	/// Loop and unref our materials
	if( GetKey() != nil )
	{
		for( i = 0; i < fMaterials.GetCount(); i++ )
		{
			if( fMaterials[ i ] != nil && fMaterials[ i ]->GetKey() != nil )
				GetKey()->Release( fMaterials[ i ]->GetKey() );
		}
	}
	fMaterials.Reset();

	delete fSpaceTree;

	for( i = 0; i < fDIIndices.GetCount(); i++ )
		delete fDIIndices[ i ];
	fDIIndices.Reset();

	if( fRegisteredForRecreate )
		plgDispatch::Dispatch()->UnRegisterForExactType( plDeviceRecreateMsg::Index(), GetKey() );

	if( fRegisteredForRender )
		plgDispatch::Dispatch()->UnRegisterForExactType( plRenderMsg::Index(), GetKey() );
}

void plDrawableSpans::SetKey(plKey k)
{
	hsKeyedObject::SetKey(k);
	if( k )
	{
		fRegisteredForRecreate = true;
		plgDispatch::Dispatch()->RegisterForExactType(plDeviceRecreateMsg::Index(), GetKey());
		plgDispatch::Dispatch()->RegisterForExactType(plPipeGeoMakeMsg::Index(), GetKey());
	}
}

//// ChangeSceneNode /////////////////////////////////////////////////////////

void	plDrawableSpans::SetSceneNode( plKey newNode )
{
	plKey curNode=GetSceneNode();
	if( curNode == newNode )
		return;
	if( newNode )
	{
		plNodeRefMsg* refMsg = TRACKED_NEW plNodeRefMsg(newNode, plNodeRefMsg::kOnRequest, -1, plNodeRefMsg::kDrawable);
		hsgResMgr::ResMgr()->SendRef(GetKey(), refMsg, plRefFlags::kActiveRef);
	}
	if( curNode )
	{
		curNode->Release(GetKey());
	}
	fSceneNode = newNode;
}

//// PrepForRender ///////////////////////////////////////////////////////////

void	plDrawableSpans::PrepForRender( plPipeline *p )
{
	/// If we're not registered for this message, register for it so we know when
	/// we need to refresh the buffers
	if( !fRegisteredForRecreate && GetKey() != nil )
	{
		fRegisteredForRecreate = true;
		plgDispatch::Dispatch()->RegisterForExactType( plDeviceRecreateMsg::Index(), GetKey() );
	}

	if( !fReadyToRender )
	{
		UInt32		i;

		for( i = 0; i < fGroups.GetCount(); i++ )
		{
			// Each group will decide whether it needs to be prepped
			fGroups[ i ]->PrepForRendering( p, false );
		}

		fReadyToRender = true;
	}

	if( fParticleSpans.GetCount() )
	{
		UInt32 i;
#if 0
		for( i = 0; i < fSpans.GetCount(); i++ )
		{
			if( fSpans[ i ]->fTypeMask & plSpan::kParticleSpan )
			{
				plParticleSpan* span = (plParticleSpan*)fSpans[i];
				
				plParticleFiller::FillParticles(p, this, span);
			}
		}
#else
		for( i = 0; i < fParticleSpans.GetCount(); i++ )
		{
			plParticleFiller::FillParticles(p, this, &fParticleSpans[i]);
		}
#endif
	}
}

void plDrawableSpans::SetDISpanVisSet(UInt32 diIndex, hsKeyedObject* ref, hsBool on)
{
	// Could actually do something here to neutralize bones, but we're not.
	// Main thing is that if it's Matrix Only, then the indices are into
	// the LocalToWorlds, not into fSpans
	if( fDIIndices[diIndex]->IsMatrixOnly() )
		return;

	plVisRegion* reg = plVisRegion::ConvertNoRef(ref);
	if( !reg )
		return;
	hsBool isNot = reg->GetProperty(plVisRegion::kIsNot);
	UInt32 visRegIndex = reg->GetIndex();

	if( isNot )
	{
		fVisNot.SetBit(visRegIndex, on);
		int i;
		for( i = 0; i < fDIIndices[ diIndex ]->GetCount(); i++ )
		{
			int spanIndex = (*fDIIndices[diIndex])[i];
			fSpans[spanIndex]->SetVisNot(visRegIndex, on);
		}
	}
	else
	{
		fVisSet.SetBit(visRegIndex, on);
		int i;
		for( i = 0; i < fDIIndices[ diIndex ]->GetCount(); i++ )
		{
			int spanIndex = (*fDIIndices[diIndex])[i];
			fSpans[spanIndex]->SetVisBit(visRegIndex, on);

			// HACKAGE
			// We need to be more careful about when we set and clear this bit,
			// but not today.
			// Okay, today. mf
			if( reg->ReplaceNormal() )
				fSpans[spanIndex]->SetVisBit(plVisMgr::kNormal, false);
		}
	}
}

void plDrawableSpans::SetVisSet(plVisMgr* visMgr)
{
	plProfile_Extern(VisSelect);
	plProfile_BeginTiming(VisSelect);
	if( visMgr )
	{
		const hsBitVector&	visSet = visMgr->GetVisSet();
		const hsBitVector&	visNot = visMgr->GetVisNot();

		// Go through some contortions to not be new[]'ing and delete[]'ing.
		static hsBitVector myVis;
		static hsBitVector myNot;

		// myVis = (visSet ^ fLastVisSet) & fVisSet;
		myVis = visSet;
		myVis ^= fLastVisSet;
		myVis &= fVisSet;

		// myNot = (visNot ^ fLastVisNot) & fVisNot;
		myNot = visNot;
		myNot ^= fLastVisNot;
		myNot &= fVisNot;

		// myVis = myVis | myNot
		myVis |= myNot;

		GetSpaceTree()->SetCache(&fVisCache);
		if( !myVis.Empty() )
		{
			fVisCache.Clear();
			
			int i;
			for( i = 0; i < fSpans.GetCount(); i++ )
			{
				if( !fSpans[i]->GetVisNot().Overlap(visNot) && fSpans[i]->GetVisSet().Overlap(visSet) )
				{
					GetSpaceTree()->EnableLeaf(i, fVisCache);
				}
			}
			fLastVisSet = visSet;
			fLastVisNot = visNot;
		}
	}
	else
	{
		GetSpaceTree()->SetCache(nil);
	}
	plProfile_EndTiming(VisSelect);
}

// Here's the problem. When we update one of the matrices in the matrix palette,
// we've invalidated the current bounds of all the spans that use it. The skinning
// matrix may have moved the spans, or may just have stretched them, but either
// way, they aren't right any more. Wait, it gets worse. We're going to have a bunch
// of SetTransform calls come in, and then some time later, without our knowledge,
// our bounds (via the space tree and PageTreeMgr) will be used to evaluate whether
// to bother drawing us. Clearly, if we're going to keep our bounds up to date, we
// want to wait until all the SetTransforms have come in, and then update our bounds
// all at once, but before the visibility determination phase kicks in.
// The alternative is to make the local bounds conservative. We bloat out the local
// bounds on export so that whereever the skinning matrices push us around, we're 
// still inside the local bounds. This is certainly cheaper to compute (all computation
// is offline), but gives a less tight bounds. Whether the conservative bounds is
// tight enough will depend on how the skinning is actually being used.
//
// Therefore I'm deferring decision on which way to go, and putting in this temp hack
// which does it the dumbest way possible while still being correct.
//
// So recap:
// If we need tight up-to-date bounds, we'll probably send a system message between
// the update phase (Eval Msg) and the render phase, only marking as dirty the changed
// palette matrices.
// If conservative bounds are good enough, we'll need to compute those from the skinning
// animations offline.
void plDrawableSpans::IUpdateMatrixPaletteBoundsHack()
{
	int i;
	for( i = 0; i < fSpans.GetCount(); i++ )
	{
		if( fSpans[i]->fNumMatrices && !(fSpans[i]->fProps & plSpan::kPropNoDraw) )
		{
			hsBounds3Ext bnd = fSpans[i]->fLocalBounds;
			const hsMatrix44& xfm0 = fSpans[i]->fMaxBoneIdx ? fLocalToWorlds[fSpans[i]->fBaseMatrix + fSpans[i]->fMaxBoneIdx] : fSpans[i]->fLocalToWorld;
			bnd.Transform(&xfm0);
			fSpans[i]->fWorldBounds = bnd;

			bnd = fSpans[i]->fLocalBounds;
			const hsMatrix44& xfm1 = fSpans[i]->fPenBoneIdx ? fLocalToWorlds[fSpans[i]->fBaseMatrix + fSpans[i]->fPenBoneIdx] : fSpans[i]->fLocalToWorld;
			bnd.Transform(&xfm1);
			fSpans[i]->fWorldBounds.Union(&bnd);

			GetSpaceTree()->MoveLeaf(i, fSpans[i]->fWorldBounds);
		}
	}
}

hsBool plDrawableSpans::IBoundsInvalid(const hsBounds3Ext& bnd) const
{
	int i;
	for( i = 0; i < 3; i++ )
	{
		const hsScalar kLimit(1.e5f);

		if( bnd.GetMaxs()[i] > kLimit )
			return true;
		if( bnd.GetMins()[i] < -kLimit )
			return true;
	}
	return false;
}

//// SetTransform ////////////////////////////////////////////////////////////

static inline hsMatrix44 IMatrixMul34(const hsMatrix44& lhs, const hsMatrix44& rhs)
{
	hsMatrix44 ret;
	ret.NotIdentity();
	ret.fMap[3][0] = ret.fMap[3][1] = ret.fMap[3][2] = 0;
	ret.fMap[3][3] = 1.f;

	ret.fMap[0][0] = lhs.fMap[0][0] * rhs.fMap[0][0]
		+ lhs.fMap[0][1] * rhs.fMap[1][0]
		+ lhs.fMap[0][2] * rhs.fMap[2][0];

	ret.fMap[0][1] = lhs.fMap[0][0] * rhs.fMap[0][1]
		+ lhs.fMap[0][1] * rhs.fMap[1][1]
		+ lhs.fMap[0][2] * rhs.fMap[2][1];

	ret.fMap[0][2] = lhs.fMap[0][0] * rhs.fMap[0][2]
		+ lhs.fMap[0][1] * rhs.fMap[1][2]
		+ lhs.fMap[0][2] * rhs.fMap[2][2];

	ret.fMap[0][3] = lhs.fMap[0][0] * rhs.fMap[0][3]
		+ lhs.fMap[0][1] * rhs.fMap[1][3]
		+ lhs.fMap[0][2] * rhs.fMap[2][3]
		+ lhs.fMap[0][3];

	ret.fMap[1][0] = lhs.fMap[1][0] * rhs.fMap[0][0]
		+ lhs.fMap[1][1] * rhs.fMap[1][0]
		+ lhs.fMap[1][2] * rhs.fMap[2][0];

	ret.fMap[1][1] = lhs.fMap[1][0] * rhs.fMap[0][1]
		+ lhs.fMap[1][1] * rhs.fMap[1][1]
		+ lhs.fMap[1][2] * rhs.fMap[2][1];

	ret.fMap[1][2] = lhs.fMap[1][0] * rhs.fMap[0][2]
		+ lhs.fMap[1][1] * rhs.fMap[1][2]
		+ lhs.fMap[1][2] * rhs.fMap[2][2];

	ret.fMap[1][3] = lhs.fMap[1][0] * rhs.fMap[0][3]
		+ lhs.fMap[1][1] * rhs.fMap[1][3]
		+ lhs.fMap[1][2] * rhs.fMap[2][3]
		+ lhs.fMap[1][3];

	ret.fMap[2][0] = lhs.fMap[2][0] * rhs.fMap[0][0]
		+ lhs.fMap[2][1] * rhs.fMap[1][0]
		+ lhs.fMap[2][2] * rhs.fMap[2][0];

	ret.fMap[2][1] = lhs.fMap[2][0] * rhs.fMap[0][1]
		+ lhs.fMap[2][1] * rhs.fMap[1][1]
		+ lhs.fMap[2][2] * rhs.fMap[2][1];

	ret.fMap[2][2] = lhs.fMap[2][0] * rhs.fMap[0][2]
		+ lhs.fMap[2][1] * rhs.fMap[1][2]
		+ lhs.fMap[2][2] * rhs.fMap[2][2];

	ret.fMap[2][3] = lhs.fMap[2][0] * rhs.fMap[0][3]
		+ lhs.fMap[2][1] * rhs.fMap[1][3]
		+ lhs.fMap[2][2] * rhs.fMap[2][3]
		+ lhs.fMap[2][3];

	return ret;
}

#include "../plStatusLog/plStatusLog.h"

#ifdef MF_TEST_UPDATE
plProfile_CreateCounter("DSSetTrans", "Update", DSSetTrans);
plProfile_CreateCounter("DSMatSpans", "Update", DSMatSpans);
plProfile_CreateCounter("DSRegSpans", "Update", DSRegSpans);

plProfile_CreateTimer("DSSetTransT", "Update", DSSetTransT);
plProfile_CreateTimer("DSMatTransT", "Update", DSMatTransT);
plProfile_CreateTimer("DSRegTransT", "Update", DSRegTransT);
plProfile_CreateTimer("DSBndTransT", "Update", DSBndTransT);
#endif // MF_TEST_UPDATE


plDrawable&	plDrawableSpans::SetTransform( UInt32 index, const hsMatrix44& l2w, const hsMatrix44& w2l ) 
{ 
#ifdef MF_TEST_UPDATE
	plProfile_IncCount(DSSetTrans, 1);
	plProfile_BeginTiming(DSSetTransT);
#endif // MF_TEST_UPDATE

	if( index == (UInt32)-1 )
	{
		fLocalToWorld = l2w; 
		fWorldToLocal = w2l; 

		fWorldBounds = fLocalBounds; 
		fWorldBounds.Transform( &l2w ); 
	}
	else
	{
		int				i;
		UInt32			idx;
		plDISpanIndex	*spans = fDIIndices[ index ];		


		if( spans->IsMatrixOnly() )
		{
#ifdef MF_TEST_UPDATE
			plProfile_IncCount(DSMatSpans, spans->GetCount());
			plProfile_BeginTiming(DSMatTransT);
#endif // MF_TEST_UPDATE
			for( i = 0; i < spans->GetCount(); i++ )
			{
#if 0
				fLocalToWorlds[ (*spans)[ i ] ] = l2w * fLocalToBones[ (*spans)[ i ] ];
				fWorldToLocals[ (*spans)[ i ] ] = fBoneToLocals[ (*spans)[ i ] ] * w2l;
#else
				fLocalToWorlds[ (*spans)[ i ] ] = IMatrixMul34(l2w, fLocalToBones[ (*spans)[ i ] ]);
				fWorldToLocals[ (*spans)[ i ] ] = IMatrixMul34(fBoneToLocals[ (*spans)[ i ] ], w2l);
#endif
			}
#ifdef MF_TEST_UPDATE
			plProfile_EndTiming(DSMatTransT);
#endif // MF_TEST_UPDATE
		}
		else if( !spans->DontTransform() )
		{
#ifdef MF_TEST_UPDATE
			plProfile_IncCount(DSRegSpans, spans->GetCount());
#endif // MF_TEST_UPDATE
			for( i = 0; i < spans->GetCount(); i++ )
			{			
#ifdef MF_TEST_UPDATE
				plProfile_BeginTiming(DSRegTransT);
#endif // MF_TEST_UPDATE
	
				idx = (*spans)[ i ];
				plSpan	*mSpan = fSpans[ idx ];
				mSpan->fLocalToWorld = l2w;
				mSpan->fWorldToLocal = w2l;

				mSpan->fWorldBounds = mSpan->fLocalBounds;
				mSpan->fWorldBounds.Transform( &l2w );

				if( fSourceSpans.GetCount() > idx )
				{
					/// If we have a geoSpan for this, update its transform as well,
					/// just in case we need to use it later (<cough> SceneViewer reshade <cough>)
					if( fSourceSpans[ idx ] == nil )
					{
						plStatusLog::AddLineS( "pipeline.log", 0xffffffff, "Nil source spans found in SetTransform()" );
					}

					fSourceSpans[ idx ]->fLocalToWorld = l2w;
					fSourceSpans[ idx ]->fWorldToLocal = w2l;
				}
#ifdef MF_TEST_UPDATE
				plProfile_EndTiming(DSRegTransT);

				plProfile_BeginTiming(DSBndTransT);
#endif // MF_TEST_UPDATE
				if( IBoundsInvalid(mSpan->fWorldBounds) )
				{
					mSpan->fProps |= kPropNoDraw;
					GetSpaceTree()->SetLeafFlag((Int16)idx, plSpaceTreeNode::kDisabled, true);
				}
				else
				{
					GetSpaceTree()->MoveLeaf((Int16)((*spans)[i]), mSpan->fWorldBounds);
				}
#ifdef MF_TEST_UPDATE
				plProfile_EndTiming(DSBndTransT);
#endif // MF_TEST_UPDATE
			}
		}

#ifdef MF_TEST_UPDATE
		plProfile_BeginTiming(DSBndTransT);
#endif // MF_TEST_UPDATE
		fWorldBounds = GetSpaceTree()->GetNode(GetSpaceTree()->GetRoot()).GetWorldBounds();
#ifdef MF_TEST_UPDATE
		plProfile_EndTiming(DSBndTransT);
#endif // MF_TEST_UPDATE
	}

#ifdef MF_TEST_UPDATE
	plProfile_EndTiming(DSSetTransT);
#endif // MF_TEST_UPDATE
	// Might want to assert that MaxWorldBounds still contains WorldBounds.

	return *this;
}

void plDrawableSpans::SetNativeTransform(UInt32 idx, const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	if( idx == UInt32(-1) )
	{
		hsAssert(false, "Invalid index to SetNativeTransform");
	}
	else
	{
		plSpan* span = fSpans[idx];
		span->fLocalToWorld = l2w;
		span->fWorldToLocal = w2l;

		span->fWorldBounds = span->fLocalBounds;
		span->fWorldBounds.Transform(&l2w);

		if( fSourceSpans.GetCount() > idx )
		{
			fSourceSpans[idx]->fLocalToWorld = l2w;
			fSourceSpans[idx]->fWorldToLocal = w2l;
		}

		if( IBoundsInvalid(span->fWorldBounds) )
		{
			SetNativeProperty( idx, kPropNoDraw, true);
		}
		else
		{
			GetSpaceTree()->MoveLeaf((Int16)idx, span->fWorldBounds);
		}
	}
}

//// GetLocalToWorld & GetWorldToLocal ///////////////////////////////////////

const hsMatrix44& plDrawableSpans::GetLocalToWorld( UInt32 span ) const
{
	if( span == (UInt32)-1 )
		return fLocalToWorld;

	return fSpans[ span ]->fLocalToWorld;
}

const hsMatrix44& plDrawableSpans::GetWorldToLocal( UInt32 span ) const
{
	if( span == (UInt32)-1 )
		return fWorldToLocal;

	return fSpans[ span ]->fWorldToLocal;
}


//// Set/GetNativeProperty ///////////////////////////////////////////////////

plDrawable& plDrawableSpans::SetNativeProperty( UInt32 index, int prop, hsBool on)
{
	int		i;


	if( index == (UInt32)-1 )
	{
		hsAssert(false, "Invalid index to SetNativeProperty");
	}
	else
	{
		plDISpanIndex	*spans = fDIIndices[ index ];		

		if( !spans->IsMatrixOnly() )
		{
			if( on )
			{
				for( i = 0; i < spans->GetCount(); i++ )
					fSpans[ (*spans)[ i ] ]->fProps |= prop;
			}
			else
			{
				for( i = 0; i < spans->GetCount(); i++ )
					fSpans[ (*spans)[ i ] ]->fProps &= ~prop;
			}
			if( (prop & kPropNoDraw) ) 
			{
				for( i = 0; i < spans->GetCount(); i++ )
					GetSpaceTree()->SetLeafFlag((Int16)((*spans)[ i ]), plSpaceTreeNode::kDisabled, on);
			}
		}
	}

	return *this;
}

hsBool	plDrawableSpans::GetNativeProperty( UInt32 index, int prop ) const
{
	int		i;
	UInt32	ret = false;


	if( index == (UInt32)-1 )
	{
		for( i = 0; i < fSpans.GetCount(); i++ )
			ret |= ( fSpans[ i ]->fProps & prop );
	}
	else
	{
		plDISpanIndex& spans = *fDIIndices[ index ];		

		if( !spans.IsMatrixOnly() )
		{
			for( i = 0; i < spans.GetCount(); i++ )
				ret |= ( fSpans[ spans[ i ] ]->fProps & prop );
		}
	}

	return ret != 0;
}

plDrawable& plDrawableSpans::SetSubType(UInt32 index, plSubDrawableType t, hsBool on)
{
	if( UInt32(-1) == index )
	{
		if( on )
		{
			int i;
			for( i = 0; i < fSpans.GetCount(); i++ )
				fSpans[i]->fSubType |= t;
		}
		else
		{
			int i;
			for( i = 0; i < fSpans.GetCount(); i++ )
				fSpans[i]->fSubType &= ~t;
		}
	}
	else
	{
		plDISpanIndex& spans = *fDIIndices[ index ];

		if( on )
		{
			int i;
			for( i = 0; i < spans.GetCount(); i++ )
				fSpans[ spans[i] ]->fSubType |= t;
		}
		else
		{
			int i;
			for( i = 0; i < spans.GetCount(); i++ )
				fSpans[ spans[i] ]->fSubType &= ~t;
		}
	}
	return *this;
}

UInt32 plDrawableSpans::GetSubType(UInt32 index) const
{
	UInt32 retVal = 0;

	if( UInt32(-1) == index )
	{
		int i;
		for( i = 0; i < fSpans.GetCount(); i++ )
			retVal |= fSpans[i]->fSubType;
	}
	else
	{
		plDISpanIndex& spans = *fDIIndices[ index ];

		int i;
		for( i = 0; i < spans.GetCount(); i++ )
			retVal |= fSpans[ spans[i] ]->fSubType;
	}
	return retVal;
}

//// IXlateSpanProps /////////////////////////////////////////////////////////
//	Never used yet--just here in case we ever need it

UInt32	plDrawableSpans::IXlateSpanProps( UInt32 props, hsBool xlateToSpan )
{
	UInt32		retProps = 0;


	if( xlateToSpan )
	{
		/// Drawable props to plSpan props
		if( props & kPropNoDraw )	retProps |= plSpan::kPropNoDraw;
		if( props & kPropSortFaces )	retProps |= plSpan::kPropFacesSortable;
	}
	else
	{
		/// plSpan props to Drawable props
		if( props & plSpan::kPropNoDraw )	retProps |= kPropNoDraw;
		if( props & plSpan::kPropFacesSortable )	retProps |= kPropSortFaces;
	}

	return retProps;
}

//// Set/GetProperty /////////////////////////////////////////////////////////
//	Sets/gets a property just like the normal Set/GetNativeProperty, but the 
//	flag taken in is from plDrawInterface, not our props flags. So we have to 
//	translate...

plDrawable& plDrawableSpans::SetProperty( UInt32 index, int diProp, hsBool on )
{
	switch( diProp )
	{
		case plDrawInterface::kDisable:
			return SetNativeProperty( index, kPropNoDraw, on );
		default:
			hsAssert( false, "Bad property passed to SetProperty" );
	}

	return *this;
}

hsBool	plDrawableSpans::GetProperty( UInt32 index, int diProp ) const
{
	switch( diProp )
	{
		case plDrawInterface::kDisable:
			return GetNativeProperty( index, kPropNoDraw );
		default:
			hsAssert( false, "Bad property passed to SetProperty" );
	}

	return false;
}

plDrawable& plDrawableSpans::SetProperty( int prop, hsBool on )
{
	switch( prop )
	{
		case plDrawInterface::kDisable:
			{
				int i;
				for (i=0; i<fIcicles.Count(); i++)
					if (on)
						fIcicles[i].fProps |= kPropNoDraw;
					else
						fIcicles[i].fProps &= ~kPropNoDraw;
				IQuickSpaceTree();
			}
			return SetNativeProperty( kPropNoDraw, on );
		default:
			hsAssert( false, "Bad property passed to SetProperty" );
	}

	return *this;
}

hsBool		plDrawableSpans::GetProperty( int prop ) const
{
	switch( prop )
	{
		case plDrawInterface::kDisable:
			return GetNativeProperty( kPropNoDraw );
		default:
			hsAssert( false, "Bad property passed to SetProperty" );
	}

	return false;
}



//// Get*Bounds //////////////////////////////////////////////////////////////

const hsBounds3Ext& plDrawableSpans::GetLocalBounds( UInt32 index ) const
{
	int				i;
	static hsBounds3Ext	bnd;


	if( index == (UInt32)-1 )
		return fLocalBounds;

	plDISpanIndex	*spans = fDIIndices[ index ];		

	bnd.MakeEmpty();
	if( !spans->IsMatrixOnly() )
	{
		for( i = 0; i < spans->GetCount(); i++ )
		{
			bnd.Union( &fSpans[ (*spans)[ i ] ]->fLocalBounds );
		}
	}

	return bnd;
}

const hsBounds3Ext& plDrawableSpans::GetWorldBounds( UInt32 index ) const
{
	int				i;
	static hsBounds3Ext	bnd;


	if( index == (UInt32)-1 )
		return fWorldBounds;

	plDISpanIndex	*spans = fDIIndices[ index ];		

	bnd.MakeEmpty();
	if( !spans->IsMatrixOnly() )
	{
		for( i = 0; i < spans->GetCount(); i++ )
		{
			bnd.Union( &fSpans[ (*spans)[ i ] ]->fWorldBounds );
		}
	}

	return bnd;
}

const hsBounds3Ext& plDrawableSpans::GetMaxWorldBounds( UInt32 index ) const
{
	return GetWorldBounds( index );
}

//// Read ////////////////////////////////////////////////////////////////////
//	We read each in the array of icicles,
//	then we read in an array of indices that we translate into
//	pointers. Note: since materials and fog environments are shared,
//	we read those keys last, so we don't have to have separate messages for
//	each.

void	plDrawableSpans::Read( hsStream* s, hsResMgr* mgr )
{
	UInt32				i, j, count, count2;
	hsBool				gotSkin = false;
	plGBufferGroup		*group;
	plRefMsg			*refMsg;

	
	plDrawable::Read(s, mgr);

	fProps = s->ReadSwap32();
	fCriteria = s->ReadSwap32();
	fRenderLevel.fLevel = s->ReadSwap32();

	/// Read in the material keys
	count = s->ReadSwap32();
	fMaterials.SetCountAndZero( count );
	for( i = 0; i < count; i++ )
	{
		refMsg = TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnCreate, i, kMsgMaterial );
		mgr->ReadKeyNotifyMe( s, refMsg, plRefFlags::kActiveRef );
	}

	/// Read the icicles in
	count = s->ReadSwap32();
	fIcicles.SetCount( count );
	for( i = 0; i < count; i++ )
	{
		fIcicles[ i ].Read( s );
		if( fIcicles[ i ].fNumMatrices )
			gotSkin = true;
	}

	/// Read the patches in
	// FIXME MAJOR VERSION
	// no more patches, remove this line
	count = s->ReadSwap32();

	/// Now read the index array in and use it to create a pointer table
	fSpanSourceIndices.Reset();
	fSpans.Reset();
	count = s->ReadSwap32();
	for( i = 0; i < count; i++ )
	{
		j = s->ReadSwap32();
		switch( j & kSpanTypeMask )
		{
			case kSpanTypeIcicle:		fSpans.Append( (plSpan *)&fIcicles[ j & kSpanIDMask ] ); break;
			case kSpanTypeParticleSpan: fSpans.Append( (plSpan *)&fParticleSpans[ j & kSpanIDMask ] ); break;
		}

		fSpanSourceIndices.Append( j );

		if( fSpans[ fSpans.GetCount() - 1 ]->fTypeMask & plSpan::kParticleSpan )
		{
			plParticleSpan	*span = (plParticleSpan *)fSpans[ fSpans.GetCount() - 1 ];
			span->fSrcSpanIdx = fSpans.GetCount() - 1;
		}
	}

	// Rebuild bit vectors for various span types
	IBuildVectors();

	/// Now that we have our pointer array, read in the common keys (fog environs, etc)
	for( i = 0; i < count; i++ )
	{
		// Ref message for the fog environment
		refMsg = TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnCreate, i, kMsgFogEnviron );
		mgr->ReadKeyNotifyMe( s, refMsg, plRefFlags::kActiveRef );
	}

	/// Read in bounds and stuff
	if( count > 0 )
	{
		fLocalBounds.Read(s);
		fWorldBounds.Read(s);
		fMaxWorldBounds.Read(s);
	}
	else
	{
		fLocalBounds.MakeEmpty();
		fWorldBounds.MakeEmpty();
		fMaxWorldBounds.MakeEmpty();
	}

	for( i = 0; i < count; i++ )
	{
		if( fSpans[i]->fProps & plSpan::kPropHasPermaLights )
		{
			UInt32 lcnt = s->ReadSwap32();
			int j;
			for( j = 0; j < lcnt; j++ )
			{
				mgr->ReadKeyNotifyMe( s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, i, kMsgPermaLight), plRefFlags::kPassiveRef);
			}
		}
		if( fSpans[i]->fProps & plSpan::kPropHasPermaProjs )
		{
			UInt32 lcnt = s->ReadSwap32();
			int j;
			for( j = 0; j < lcnt; j++ )
			{
				mgr->ReadKeyNotifyMe( s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, i, kMsgPermaProj), plRefFlags::kPassiveRef);
			}
		}
	}

	/// Read in the source spans if necessary
	count = s->ReadSwap32();
	if( count > 0 )
	{
		fSourceSpans.SetCount( count );
		for( i = 0; i < count; i++ )
		{
			fSourceSpans[ i ] = TRACKED_NEW plGeometrySpan;
			fSourceSpans[ i ]->Read( s );
			fSourceSpans[ i ]->fMaterial = GetMaterial( fSpans[ i ]->fMaterialIdx );
			fSourceSpans[ i ]->fFogEnviron = fSpans[ i ]->fFogEnvironment;
			fSourceSpans[ i ]->fSpanRefIndex = i;
		}
	}
	else
		fSourceSpans.Reset();

	/// Read in the matrix palette (if any)
	count = s->ReadSwap32();
	fLocalToWorlds.SetCount(count);
	fWorldToLocals.SetCount(count);
	fLocalToBones.SetCount(count);
	fBoneToLocals.SetCount(count);
	for( i = 0; i < count; i++ )
	{
		fLocalToWorlds[i].Read(s);
		fWorldToLocals[i].Read(s);

		fLocalToBones[i].Read(s);
		fBoneToLocals[i].Read(s);
	}

	/// Read in the drawInterface index arrays
	count = s->ReadSwap32();
	fDIIndices.SetCountAndZero( count );
	for( i = 0; i < count; i++ )
	{
		fDIIndices[ i ] = TRACKED_NEW plDISpanIndex;
		
		fDIIndices[ i ]->fFlags = (UInt8)(s->ReadSwap32());
		count2 = s->ReadSwap32();
		fDIIndices[ i ]->SetCountAndZero( count2 );
		for( j = 0; j < count2; j++ )
			(*fDIIndices[ i ])[ j ] = s->ReadSwap32();
	}

	/// Read the groups in
	count = s->ReadSwap32();
	while( count-- )
	{
		group = TRACKED_NEW plGBufferGroup(0, fProps & kPropVolatile, fProps & kPropSortFaces);
		group->Read( s );

		fGroups.Append( group );

	}

	if( fProps & kPropSortFaces )
	{
		for( i = 0; i < fSpans.GetCount(); i++ )
			IMakeSpanSortable(i);
	}

	/// Other stuff now
	fSpaceTree = plSpaceTree::ConvertNoRef(mgr->ReadCreatable(s));
	
	fSceneNode = mgr->ReadKey(s);
	plNodeRefMsg* nRefMsg = TRACKED_NEW plNodeRefMsg(fSceneNode, plRefMsg::kOnCreate, -1, plNodeRefMsg::kDrawable); 
	mgr->AddViaNotify(GetKey(), nRefMsg, plRefFlags::kActiveRef);

	if( GetNativeProperty(plDrawable::kPropCharacter) )
	{
		fVisSet.SetBit(plVisMgr::kCharacter, true);
		for( i = 0; i < fSpans.GetCount(); i++ )
			fSpans[i]->SetVisBit(plVisMgr::kCharacter, true);
	}

	// Placeholder hack - see IUpdateMatrixPaletteBoundsHack() for comments
	if( gotSkin )
	{
		fRegisteredForRender = true;
		plgDispatch::Dispatch()->RegisterForExactType( plRenderMsg::Index(), GetKey() );
	}
	
	fReadyToRender = false;
}

//// ITestMatForSpecularity //////////////////////////////////////////////////

hsBool	plDrawableSpans::ITestMatForSpecularity( hsGMaterial *mat )
{
	int		i;


	for( i = 0; i < mat->GetNumLayers(); i++ )
	{
		if( mat->GetLayer( i )->GetShadeFlags() && hsGMatState::kShadeSpecular )
			return true;
	}

	return false;
}


#include "plProfile.h"
plProfile_CreateTimer("MatrixPalleteHack", "RenderSetup", PalletteHack);
//// MsgReceive //////////////////////////////////////////////////////////////

hsBool plDrawableSpans::MsgReceive( plMessage* msg )
{
	plGenRefMsg		*refMsg = plGenRefMsg::ConvertNoRef( msg );
	int				i;
	hsBool			hasSpec;


	if( refMsg )
	{
		if( refMsg->fType == kMsgMaterial )
		{
			/// Material add/remove on this drawable
			if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
			{
				hsAssert( refMsg->fWhich < fMaterials.GetCount(), "Invalid material index" );

				fMaterials[ refMsg->fWhich ] = hsGMaterial::ConvertNoRef( refMsg->GetRef() );
				
				if( !fSettingMatIdxLock )
				{
					// Now find all spans with this material and mark them as using or not using specular
					hasSpec = ITestMatForSpecularity( fMaterials[ refMsg->fWhich ] );

					for( i = 0; i < fSpans.GetCount(); i++ )
					{
						if( fSpans[ i ] != nil && fSpans[ i ]->fMaterialIdx == refMsg->fWhich )
						{
							if( hasSpec )
								fSpans[ i ]->fProps |= plSpan::kPropMatHasSpecular;
							else
								fSpans[ i ]->fProps &= ~plSpan::kPropMatHasSpecular;
						}
					}
				}
			}
			else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
			{
				hsAssert( refMsg->fWhich < fMaterials.GetCount(), "Invalid material index" );

				fMaterials[ refMsg->fWhich ] = nil;
			}
			return true;
		}
		else if( refMsg->fType == kMsgFogEnviron )
		{
			/// Fog environment add/remove on this drawable
			if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
			{
				hsAssert( refMsg->fWhich < fSpans.GetCount(), "Shesh, send us valid data, will ya??" );

				fSpans[ refMsg->fWhich ]->fFogEnvironment = plFogEnvironment::ConvertNoRef( refMsg->GetRef() );
			}
			else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
			{
				UInt32				i;
				plFogEnvironment	*fog = plFogEnvironment::ConvertNoRef( refMsg->GetRef() );

				for( i = 0; i < GetNumSpans(); i++ )
				{
					if( fSpans[ i ]->fFogEnvironment == fog )
					{
						fSpans[ i ]->fFogEnvironment = nil;
						break;
					}
				}
			}
			return true;
		}
		else if( refMsg->fType == kMsgPermaLight )
		{
			if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
			{
				hsAssert( refMsg->fWhich < fSpans.GetCount(), "Shesh, send us valid data, will ya??" );
				plLightInfo* li = plLightInfo::ConvertNoRef(refMsg->GetRef());
				fSpans[refMsg->fWhich]->AddPermaLight(li, false);
			}
			else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
			{
				hsAssert( refMsg->fWhich < fSpans.GetCount(), "Shesh, send us valid data, will ya??" );
				plLightInfo* li = (plLightInfo*)refMsg->GetRef();
				fSpans[refMsg->fWhich]->RemovePermaLight(li, false);
			}
			return true;
		}
		else if( refMsg->fType == kMsgPermaProj )
		{
			if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
			{
				hsAssert( refMsg->fWhich < fSpans.GetCount(), "Shesh, send us valid data, will ya??" );
				plLightInfo* li = plLightInfo::ConvertNoRef(refMsg->GetRef());
				fSpans[refMsg->fWhich]->AddPermaLight(li, true);
			}
			else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
			{
				hsAssert( refMsg->fWhich < fSpans.GetCount(), "Shesh, send us valid data, will ya??" );
				plLightInfo* li = (plLightInfo*)refMsg->GetRef();
				fSpans[refMsg->fWhich]->RemovePermaLight(li, true);
			}
			return true;
		}
		else if( refMsg->fType == kMsgPermaLightDI )
		{
			if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
			{
				hsAssert( refMsg->fWhich < fDIIndices.GetCount(), "Shesh, send us valid data, will ya??" );
				plLightInfo* li = plLightInfo::ConvertNoRef(refMsg->GetRef());

				int diIndex = int(refMsg->fWhich);
				if( (diIndex >= 0)
					&& !fDIIndices[ diIndex ]->IsMatrixOnly() )
				{
					for( i = 0; i < fDIIndices[ diIndex ]->GetCount(); i++ )
					{
						int spanIndex = (*fDIIndices[diIndex])[i];
						fSpans[spanIndex]->AddPermaLight(li, false);
					}
				}
			}
			else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
			{
				hsAssert( refMsg->fWhich < fDIIndices.GetCount(), "Shesh, send us valid data, will ya??" );
				plLightInfo* li = plLightInfo::ConvertNoRef(refMsg->GetRef());

				int diIndex = int(refMsg->fWhich);
				if( (diIndex >= 0)
					&& !fDIIndices[ diIndex ]->IsMatrixOnly() )
				{
					for( i = 0; i < fDIIndices[ diIndex ]->GetCount(); i++ )
					{
						int spanIndex = (*fDIIndices[diIndex])[i];
						fSpans[spanIndex]->RemovePermaLight(li, false);
					}
				}
			}
			return true;
		}
		else if( refMsg->fType == kMsgPermaProjDI )
		{
			if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
			{
				hsAssert( refMsg->fWhich < fDIIndices.GetCount(), "Shesh, send us valid data, will ya??" );
				plLightInfo* li = plLightInfo::ConvertNoRef(refMsg->GetRef());

				int diIndex = int(refMsg->fWhich);
				if( (diIndex >= 0)
					&& !fDIIndices[ diIndex ]->IsMatrixOnly() )
				{
					for( i = 0; i < fDIIndices[ diIndex ]->GetCount(); i++ )
					{
						int spanIndex = (*fDIIndices[diIndex])[i];
						fSpans[spanIndex]->AddPermaLight(li, true);
					}
				}
			}
			else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
			{
				hsAssert( refMsg->fWhich < fDIIndices.GetCount(), "Shesh, send us valid data, will ya??" );
				plLightInfo* li = plLightInfo::ConvertNoRef(refMsg->GetRef());

				int diIndex = int(refMsg->fWhich);
				if( (diIndex >= 0)
					&& !fDIIndices[ diIndex ]->IsMatrixOnly() )
				{
					for( i = 0; i < fDIIndices[ diIndex ]->GetCount(); i++ )
					{
						int spanIndex = (*fDIIndices[diIndex])[i];
						fSpans[spanIndex]->RemovePermaLight(li, true);
					}
				}
			}
			return true;
		}
	}	
	else if( plDeviceRecreateMsg::ConvertNoRef( msg ) != nil )
	{
		/// Device recreation message--just reset our flag so we refresh buffer groups
		fReadyToRender = false;
		return true;
	}
	else if( plRenderMsg::ConvertNoRef( msg ) )
	{
		plProfile_BeginLap(PalletteHack, this->GetKey()->GetUoid().GetObjectName());
	
		IUpdateMatrixPaletteBoundsHack();

		// Last thing here. We have a bit list of which of our spans are skinned and
		// thus need to be re-blended each frame. However, we don't want to blend them
		// multiple times per frame if at all possible. Since the pipeline already checks
		// our bitfield, what we *really* do is make a copy and give the pipeline our copy.
		// The pipeline will then clear out those bits as it blends them, and then we simply
		// re-set them here, since plRenderMsg is sent once before all the rendering is done :)
		fFakeBlendingSpanVector = fBlendingSpanVector;
		plProfile_EndLap(PalletteHack, this->GetKey()->GetUoid().GetObjectName());
	
		return true;
	}

	plDISpansMsg* diMsg = plDISpansMsg::ConvertNoRef(msg);
	if( diMsg )
	{
		if( diMsg->fType == plDISpansMsg::kRemovingSpan )
		{
			// If the only set of spans we've got is about to be removed, 
			// (and we're not flagged to stick around) then just
			// kill ourselves entirely.
			if( fDIIndices.GetCount() < 2 && !(diMsg->fFlags & plDISpansMsg::kLeaveEmptyDrawable) )
			{
				hsAssert(diMsg->fIndex + 1 == fDIIndices.GetCount(), "Deleting the an unknown set of indices");
				if( GetSceneNode() )
				{
					GetSceneNode()->Release(GetKey());
				}
			}
			else /// plDrawInterface telling us to remove some spans
			{
				RemoveDISpans( (Int32)diMsg->fIndex );
			}
		}
#ifdef HS_DEBUGGING
		else if( diMsg->fType == plDISpansMsg::kAddingSpan )
		{
			/// plDrawInterface telling us which spans it owns
			Int32	i, spanIndex = (Int32)diMsg->fIndex;
			
			if( spanIndex == -1 )
				return true;
			
			for( i = 0; i < fDIIndices[ spanIndex ]->GetCount(); i++ )
			{
				if( !fDIIndices[ spanIndex ]->IsMatrixOnly() )
					fSpans[ (*fDIIndices[ spanIndex])[ i ] ]->fOwnerKey = diMsg->GetSender();
			}
		}
#endif
		return true;
	}
	
	plPipeGeoMakeMsg* make = plPipeGeoMakeMsg::ConvertNoRef(msg);
	if( make )
	{
		fReadyToRender = false;
		PrepForRender(make->Pipeline());
		return true;
	}

	return plDrawable::MsgReceive(msg);
}

void plDrawableSpans::SetRenderLevel(const plRenderLevel& l)
{
	fRenderLevel = l;
}

const plRenderLevel& plDrawableSpans::GetRenderLevel() const
{
	return fRenderLevel;
}


//// DoIMatch ////////////////////////////////////////////////////////////////
//	Called by the sceneNode to determine if we match the criteria

hsBool	plDrawableSpans::DoIMatch( const plDrawableCriteria& crit )
{
	if( crit.fCriteria ^ fCriteria )
		return false;

	if( crit.fLevel != fRenderLevel )
		return false;

	if( crit.fType != fType )
		return false;

	if( crit.fLoadMask != fLoadMask )
		return false;

	return true;
}

//// SetCriteria /////////////////////////////////////////////////////////////
//	Sets the criteria that this ice matches to. Needed since some ice will
//	be static per sceneNode while others wont, etc.

void	plDrawableSpans::SetCriteria( const plDrawableCriteria& crit )
{
	// Very simple right now. May be more complicated later...
	fCriteria =  crit.fCriteria;
	fRenderLevel = crit.fLevel;
	fType = crit.fType;
	fLoadMask = crit.fLoadMask;

	if( fCriteria & kCritSortSpans )
		fProps |= kPropSortSpans;
	if( fCriteria & kCritSortFaces )
		fProps |= kPropSortFaces;
	if( fCriteria & kCritCharacter )
		fProps |= kPropCharacter;
}

//// IQuickSpaceTree ///////////////////////////////////////////////////
//	Creates a fast, space tree for use at run-time (like in the 
//	SceneViewer). Any time a space tree is requested but there isn't
//	one available, this will be supplied. This is a full featured
//	hierarchical bounds which is pretty fast to compute. A more highly
//	optimized version may be plugged into the Optimize function at a later
//	date if this one doesn't perform enough (it does so far).

void	plDrawableSpans::IQuickSpaceTree( void ) const
{
	int		i;

	// Make the space tree (hierarchical bounds).
	plSpaceTreeMaker maker;
	maker.Reset();
	for( i = 0; i < fSpans.GetCount(); i++ )
	{
		maker.AddLeaf( fSpans[ i ]->fWorldBounds, fSpans[ i ]->fProps & plSpan::kPropNoDraw );
	}
	plSpaceTree* tree = maker.MakeTree();
	SetSpaceTree(tree);
}

//// SetSpaceTree ////////////////////////////////////////////////////////////

void	plDrawableSpans::SetSpaceTree( plSpaceTree *st ) const
{
	delete fSpaceTree;
	fSpaceTree = st; 
	fLastVisSet.Clear();
	fLastVisNot.Clear();
}

//// GetDISpans //////////////////////////////////////////////////////////////

plDISpanIndex&	plDrawableSpans::GetDISpans( UInt32 index ) const
{
	return *fDIIndices[index];
}

//// GetVertex/IndexRef //////////////////////////////////////////////////////

hsGDeviceRef	*plDrawableSpans::GetVertexRef( UInt32 group, UInt32 idx )
{
	return fGroups[ group ]->GetVertexBufferRef( idx ); 
}

hsGDeviceRef	*plDrawableSpans::GetIndexRef( UInt32 group, UInt32 idx )
{
	return fGroups[ group ]->GetIndexBufferRef( idx );
}

void plDrawableSpans::DirtyVertexBuffer(UInt32 group, UInt32 idx)
{
	hsAssert(group < fGroups.GetCount(), "Dirtying vtx buffer I don't have");
	GetBufferGroup(group)->DirtyVertexBuffer(idx);

	SetNotReadyToRender();
}

void plDrawableSpans::DirtyIndexBuffer(UInt32 group, UInt32 idx)
{
	hsAssert(group < fGroups.GetCount(), "Dirtying index buffer I don't have");
	GetBufferGroup(group)->DirtyIndexBuffer(idx);

	SetNotReadyToRender();
}

hsGMaterial* plDrawableSpans::GetSubMaterial(int index) const
{
	return GetMaterial(fSpans[index]->fMaterialIdx);
}

// return true if span invisible before minDist and/or after maxDist
hsBool plDrawableSpans::GetSubVisDists(int index, hsScalar& minDist, hsScalar& maxDist) const
{
	return (minDist = fSpans[index]->GetMinDist()) < (maxDist = fSpans[index]->GetMaxDist());
}

//////////////////////////////////////////////////////////////////////////////
//// Runtime Dynamics ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// SortSpan ////////////////////////////////////////////////////////////////
//	Given the index of the span to sort, sorts the triangles of that span
//	based on the sorting data. Note: the span MUST be of type plIcicle.

plProfile_CreateTimer("Face Sort", "Draw", FaceSort);
plProfile_CreateCounter("Face Sort Calls", "Draw", FaceSortCalls);
plProfile_CreateCounter("Faces Sorted", "Draw", FacesSorted);

void	plDrawableSpans::SortSpan( UInt32 index, plPipeline *pipe )
{
	plProfile_Inc(FaceSortCalls);

	plProfile_BeginLap(FaceSort, "0");

	plIcicle			*span = (plIcicle *)fSpans[ index ];
	plGBufferTriangle	*list, temp;
	UInt32				numTris;
	int					i;
	hsMatrix44			w2cMatrix = pipe->GetWorldToCamera() * pipe->GetLocalToWorld();
	hsScalar			dist;

	ICheckSpanForSortable(index);

	static hsTArray<hsRadixSort::Elem>	sortList;
	static hsTArray<UInt16>				tempTriList;
	hsRadixSort::Elem					*elem;


	/// Get some stuff
	list = span->fSortData;
	numTris = span->fILength / 3;

	plProfile_IncCount(FacesSorted, numTris);

	hsAssert( numTris > 0, "How could we start sorting no triangles??" );

	/// Sort the triangles in "list"
	sortList.SetCount( numTris );
	tempTriList.SetCount( numTris * 3 );
	elem = sortList.AcquireArray();

	plProfile_EndLap(FaceSort, "0");
	plProfile_BeginLap(FaceSort, "1");

	hsVector3 vec(w2cMatrix.fMap[2][0], w2cMatrix.fMap[2][1], w2cMatrix.fMap[2][2]);
	hsScalar trans = w2cMatrix.fMap[2][3];

	// Fill out the radix sort elements with our data
	for( i = 0; i < numTris; i++ )
	{
		dist = vec.InnerProduct(list[ i ].fCenter) + trans;
		elem[ i ].fKey.fFloat = dist;
		elem[ i ].fBody = &list[ i ];
		elem[ i ].fNext = elem + i + 1;
	}
	elem[ i - 1 ].fNext = nil;

	plProfile_EndLap(FaceSort, "1");
	plProfile_BeginLap(FaceSort, "2");

	// Do da sort thingy
	hsRadixSort			rad;
	hsRadixSort::Elem	*sortedList = rad.Sort( elem, 0 );

	plProfile_EndLap(FaceSort, "2");
	plProfile_BeginLap(FaceSort, "3");

	UInt16* indices = tempTriList.AcquireArray();
	// Stuff into the temp array
	for( i = 0, elem = sortedList; i < numTris; i++ )
	{
		*indices++ = ((plGBufferTriangle *)elem->fBody)->fIndex1;
		*indices++ = ((plGBufferTriangle *)elem->fBody)->fIndex2;
		*indices++ = ((plGBufferTriangle *)elem->fBody)->fIndex3;
		elem = elem->fNext;
	}

	plProfile_EndLap(FaceSort, "3");
	plProfile_BeginLap(FaceSort, "4");

	/// Now send them on to the buffer group
	fGroups[ span->fGroupIdx ]->StuffFromTriList( span->fIBufferIdx, span->fIStartIdx, 
												  numTris, tempTriList.AcquireArray() );

	/// Optional step in a way: copy back our new, sorted list to our original
	/// array. This lets us do less sorting next call, since the order should
	/// remain largely unchanged from the last call. If this turns out NOT to
	/// be the case, or if the sorting step takes less time than this copy,
	///	take this out!
//	memcpy( list, tempTriList.AcquireArray(), numTris * sizeof( plGBufferTriangle ) );

	/// All done! (force buffer groups to refresh during next render call)
	fReadyToRender = false;

	plProfile_EndLap(FaceSort, "4");
}

//// SortVisibleSpans ////////////////////////////////////////////////////////
//	Sorts the visible spans's triangles in one big lump, for proper back-to-
//	front display.
//	Updated 5.14.2001 mcn - Fixed so loops don't assume spans are icicles

void plDrawableSpans::SortVisibleSpans(const hsTArray<Int16>& visList, plPipeline* pipe)
{
#define MF_CHUNKSORT
#ifndef MF_CHUNKSORT

	plProfile_Inc(FaceSortCalls);

	if( !visList.GetCount() )
		return;


	static hsLargeArray<hsRadixSort::Elem>	sortScratch;
	static hsLargeArray<UInt16>			triList;
	static hsTArray<Int32>				counters;
	static hsTArray<UInt32>				startIndex;
	
	int i;
	
	plProfile_BeginTiming(FaceSort);
	if( pipe->IsDebugFlagSet( plPipeDbg::kFlagDontSortFaces ) )
	{
		/// Don't sort, just send unchanged
		int		j, idx;

		for( i = 0; i < visList.GetCount(); i++ )
		{
			plIcicle* span = (plIcicle*)fSpans[visList[i]];
			ICheckSpanForSortable(visList[i]);

			/// Build a fake list of indices....
			plGBufferTriangle*		list = span->fSortData;
			triList.SetCount( span->fILength );
			for( j = 0, idx = 0; j < span->fILength / 3; j++, idx += 3 )
			{
				triList[ idx ] = list[ j ].fIndex1;
				triList[ idx + 1 ] = list[ j ].fIndex2;
				triList[ idx + 2 ] = list[ j ].fIndex3;
			}

			/// Now send them on to the buffer group
			fGroups[ span->fGroupIdx ]->StuffFromTriList( span->fIBufferIdx, span->fIStartIdx, 
														  span->fILength / 3, triList.AcquireArray() );
		}
		fReadyToRender = false;
		return;
	}
	plProfile_EndTiming(FaceSort);

	plProfile_BeginLap(FaceSort, "0");

	startIndex.SetCount(fSpans.GetCount());

	// First figure out the total number of tris to deal with.
	int totTris = 0;
	for( i = 0; i < visList.GetCount(); i++ )
	{
		plIcicle* span = (plIcicle*)fSpans[visList[i]];

		startIndex[visList[i]] = totTris * 3;
		if( span->fProps & plSpan::kPropReverseSort )
			startIndex[visList[i]] += span->fILength - 3;

		totTris += span->fILength / 3;
	}
	if( totTris == 0 )
	{
		plProfile_EndLap(FaceSort, "0");
		return;
	}

	plProfile_IncCount(FacesSorted, totTris);

	sortScratch.SetCount(totTris);
	triList.SetCount(3 * totTris);

	hsRadixSort::Elem* elem = sortScratch.AcquireArray();

	plProfile_EndLap(FaceSort, "0");
	plProfile_BeginLap(FaceSort, "1");

	// Pack them into the sort structure. We probably want to make the 
	// plGBufferTriangle look like plTriSortData (just add span index) 
	// which would get rid of this copy and help the data alignment.
	// Oops, I already did.
	int cnt = 0;
	for( i = 0; i < visList.GetCount(); i++ )
	{
		plIcicle* span = (plIcicle*)fSpans[visList[i]];
		
		int nTris = span->fILength / 3;

		hsPoint3 viewPos = span->fWorldToLocal * pipe->GetViewPositionWorld();

		plGBufferTriangle*		list = span->fSortData;
		int j;
		for( j = 0; j < nTris; j++ )
		{
			hsScalar dist = -(viewPos - list[j].fCenter).MagnitudeSquared();
			elem[cnt].fKey.fFloat = dist;
			elem[cnt].fBody = &list[j];
			elem[cnt].fNext = elem + cnt + 1;

			cnt++;
		}
	}
	elem[cnt-1].fNext = nil;

	plProfile_EndLap(FaceSort, "1");
	plProfile_BeginLap(FaceSort, "2");

	// Actual sort
	hsRadixSort			rad;
	hsRadixSort::Elem* sortedList = rad.Sort( elem, 0 );

	plProfile_EndLap(FaceSort, "2");
	plProfile_BeginLap(FaceSort, "3");

	counters.SetCountAndZero(fSpans.GetCount());

	while( sortedList )
	{
		plGBufferTriangle* data = (plGBufferTriangle*)sortedList->fBody;
		plIcicle* span = (plIcicle*)fSpans[data->fSpanIndex];

		UInt16* idx = &triList[startIndex[data->fSpanIndex] + counters[data->fSpanIndex]];
		*idx++ = data->fIndex1;
		*idx++ = data->fIndex2;
		*idx++ = data->fIndex3;
		if( span->fProps & plSpan::kPropReverseSort )
			counters[data->fSpanIndex] -= 3;
		else
			counters[data->fSpanIndex] += 3;

		sortedList = sortedList->fNext;
	}

	plProfile_EndLap(FaceSort, "3");
	plProfile_BeginLap(FaceSort, "4");

	const int kMaxBufferGroups = 20;
	const int kMaxIndexBuffers = 20;
	static Int16	newStarts[kMaxBufferGroups][kMaxIndexBuffers];

	// Temp hack stuff so we can switch back and forth (to see if it really does any good).
	static int oldWay = false;
	static int lastOldWay = oldWay;
	if( oldWay != lastOldWay )
	{
		memset(newStarts, 0, kMaxBufferGroups * kMaxIndexBuffers * sizeof(Int16));

		for( i = 0; i < fSpans.GetCount(); i++ )
		{
			plIcicle* span = (plIcicle*)fSpans[i];

			span->fPackedIdx = span->fIStartIdx = newStarts[span->fGroupIdx][span->fIBufferIdx];
			newStarts[span->fGroupIdx][span->fIBufferIdx] += span->fILength;
		}
		lastOldWay = oldWay;
	}

	if( oldWay )
	{
		for( i = 0; i < visList.GetCount(); i++ )
		{
			plIcicle* span = (plIcicle*)fSpans[visList[i]];

			/// Now send them on to the buffer group
			fGroups[ span->fGroupIdx ]->StuffFromTriList( span->fIBufferIdx, span->fIStartIdx, 
														  span->fILength / 3, triList.AcquireArray() + startIndex[visList[i]]);
		}
	}
	else
	{
		memset(newStarts, 0, kMaxBufferGroups * kMaxIndexBuffers * sizeof(Int16));

		UInt32 start = 0;
		for( i = 0; i < visList.GetCount(); i++ )
		{
			plIcicle* span = (plIcicle*)fSpans[visList[i]];

			/// Now send them on to the buffer group
			span->fPackedIdx = span->fIStartIdx = newStarts[span->fGroupIdx][span->fIBufferIdx];
			newStarts[span->fGroupIdx][span->fIBufferIdx] += span->fILength;
			fGroups[ span->fGroupIdx ]->StuffFromTriList( span->fIBufferIdx, span->fIStartIdx, 
														  span->fILength / 3, triList.AcquireArray() + startIndex[visList[i]]);
		}
	}

	plProfile_EndLap(FaceSort, "4");

	fReadyToRender = false;

	return;

#else // MF_CHUNKSORT

	plProfile_Inc(FaceSortCalls);

	if( !visList.GetCount() )
		return;

	plProfile_BeginTiming(FaceSort);

	static hsLargeArray<hsRadixSort::Elem>	sortScratch;
	static hsLargeArray<UInt16>			triList;
	static hsTArray<Int32>				counters;
	static hsTArray<UInt32>				startIndex;
	
	int i;
	
	if( pipe->IsDebugFlagSet( plPipeDbg::kFlagDontSortFaces ) )
	{
		/// Don't sort, just send unchanged
		int		j, idx;

		for( i = 0; i < visList.GetCount(); i++ )
		{
			plIcicle* span = (plIcicle*)fSpans[visList[i]];
			ICheckSpanForSortable(visList[i]);

			/// Build a fake list of indices....
			plGBufferTriangle*		list = span->fSortData;
			triList.SetCount( span->fILength );
			for( j = 0, idx = 0; j < span->fILength / 3; j++, idx += 3 )
			{
				triList[ idx ] = list[ j ].fIndex1;
				triList[ idx + 1 ] = list[ j ].fIndex2;
				triList[ idx + 2 ] = list[ j ].fIndex3;
			}

			/// Now send them on to the buffer group
			fGroups[ span->fGroupIdx ]->StuffFromTriList( span->fIBufferIdx, span->fIStartIdx, 
														  span->fILength / 3, triList.AcquireArray() );
		}
		fReadyToRender = false;
		return;
	}

	plProfile_EndTiming(FaceSort);

	plProfile_BeginLap(FaceSort, "0");

	startIndex.SetCount(fSpans.GetCount());

	// First figure out the total number of tris to deal with.
	int totTris = 0;
	for( i = 0; i < visList.GetCount(); i++ )
	{
		plIcicle* span = (plIcicle*)fSpans[visList[i]];
		ICheckSpanForSortable(visList[i]);
		
		startIndex[visList[i]] = totTris * 3;
		if( span->fProps & plSpan::kPropReverseSort )
			startIndex[visList[i]] += span->fILength - 3;


		totTris += span->fILength / 3;
	}
	if( totTris == 0 )
	{
		plProfile_EndLap(FaceSort, "0");
		return;
	}

	plProfile_IncCount(FacesSorted, totTris);

	sortScratch.SetCount(totTris);
	triList.SetCount(3 * totTris);

	hsRadixSort::Elem* elem = sortScratch.AcquireArray();

	plProfile_EndLap(FaceSort, "0");

	int iVis = 0;
	while( iVis < visList.GetCount() )
	{
		plProfile_BeginLap(FaceSort, "1");

		// Pack them into the sort structure. We probably want to make the 
		// plGBufferTriangle look like plTriSortData (just add span index) 
		// which would get rid of this copy and help the data alignment.
		// Oops, I already did.
		const int kTriCutoff = 4000;
		int cnt = 0;
		while( (iVis < visList.GetCount()) && (cnt < kTriCutoff) )
		{
			plIcicle* span = (plIcicle*)fSpans[visList[iVis]];
			
			int nTris = span->fILength / 3;

			hsPoint3 viewPos = span->fWorldToLocal * pipe->GetViewPositionWorld();

			plGBufferTriangle*		list = span->fSortData;
			int j;
			for( j = 0; j < nTris; j++ )
			{
				hsScalar dist = -(viewPos - list[j].fCenter).MagnitudeSquared();
				elem[cnt].fKey.fFloat = dist;
				elem[cnt].fBody = &list[j];
				elem[cnt].fNext = elem + cnt + 1;

				cnt++;
			}
			iVis++;
		}
		elem[cnt-1].fNext = nil;

		plProfile_EndLap(FaceSort, "1");
		plProfile_BeginLap(FaceSort, "2");

		// Actual sort
		hsRadixSort			rad;
		hsRadixSort::Elem* sortedList = rad.Sort( elem, 0 );

		plProfile_EndLap(FaceSort, "2");
		plProfile_BeginLap(FaceSort, "3");

		counters.SetCountAndZero(fSpans.GetCount());

		while( sortedList )
		{
			plGBufferTriangle* data = (plGBufferTriangle*)sortedList->fBody;
			plIcicle* span = (plIcicle*)fSpans[data->fSpanIndex];

			UInt16* idx = &triList[startIndex[data->fSpanIndex] + counters[data->fSpanIndex]];
			*idx++ = data->fIndex1;
			*idx++ = data->fIndex2;
			*idx++ = data->fIndex3;
			if( span->fProps & plSpan::kPropReverseSort )
				counters[data->fSpanIndex] -= 3;
			else
				counters[data->fSpanIndex] += 3;
			
			sortedList = sortedList->fNext;
		}

		plProfile_EndLap(FaceSort, "3");
	}

	plProfile_BeginLap(FaceSort, "4");

	const int kMaxBufferGroups = 20;
	const int kMaxIndexBuffers = 20;
	static Int16	newStarts[kMaxBufferGroups][kMaxIndexBuffers];

	hsAssert(kMaxBufferGroups >= GetNumBufferGroups(), "Bigger than we counted on num groups sort.");

	memset(newStarts, 0, kMaxBufferGroups * kMaxIndexBuffers * sizeof(Int16));

	UInt32 start = 0;
	for( i = 0; i < visList.GetCount(); i++ )
	{
		plIcicle* span = (plIcicle*)fSpans[visList[i]];

		hsAssert(kMaxIndexBuffers > span->fIBufferIdx, "Bigger than we counted on num buffers sort.");

		if( span->fProps & plSpan::kPropReverseSort )
			startIndex[visList[i]] -= span->fILength - 3;

		/// Now send them on to the buffer group
		span->fIPackedIdx = span->fIStartIdx = newStarts[span->fGroupIdx][span->fIBufferIdx];
		newStarts[span->fGroupIdx][span->fIBufferIdx] += (Int16)(span->fILength);
		fGroups[ span->fGroupIdx ]->StuffFromTriList( span->fIBufferIdx, span->fIStartIdx, 
													  span->fILength / 3, triList.AcquireArray() + startIndex[visList[i]]);
	}

	plProfile_EndLap(FaceSort, "4");

	fReadyToRender = false;

#endif // MF_CHUNKSORT
}

struct buffTriCmpBackToFront : public std::binary_function<plGBufferTriangle, plGBufferTriangle, bool>
{
	hsPoint3 fViewPos;
	buffTriCmpBackToFront(const hsPoint3& p) : fViewPos(p) {}

	bool operator()( const plGBufferTriangle& lhs, const plGBufferTriangle& rhs) const
	{
		return hsVector3(&fViewPos, &lhs.fCenter).MagnitudeSquared() > hsVector3(&fViewPos, &rhs.fCenter).MagnitudeSquared();
	}
};

struct buffTriCmpFrontToBack : public std::binary_function<plGBufferTriangle, plGBufferTriangle, bool>
{
	hsPoint3 fViewPos;
	buffTriCmpFrontToBack(const hsPoint3& p) : fViewPos(p) {}

	bool operator()( const plGBufferTriangle& lhs, const plGBufferTriangle& rhs) const
	{
		return hsVector3(&fViewPos, &lhs.fCenter).MagnitudeSquared() < hsVector3(&fViewPos, &rhs.fCenter).MagnitudeSquared();
	}
};

void plDrawableSpans::SortVisibleSpansPartial(const hsTArray<Int16>& visList, plPipeline* pipe)
{
	plProfile_Inc(FaceSortCalls);

	plProfile_BeginTiming(FaceSort);

	int i;
	for( i = 0; i < visList.GetCount(); i++ )
	{
		hsAssert(fSpans[visList[i]]->fTypeMask & plSpan::kIcicleSpan, "Unknown type for sorting faces");

		plIcicle* span = (plIcicle*)fSpans[visList[i]];

		if( span->fProps & plSpan::kPartialSort )
		{
			hsAssert(fGroups[span->fGroupIdx]->AreIdxVolatile(), "Badly setup buffer group - set PartialSort too late?");

			ICheckSpanForSortable(visList[i]);

			const hsPoint3 viewPos = span->fWorldToLocal * pipe->GetViewPositionWorld();

			const int numTris = span->fILength/3;
			std::sort(span->fSortData, span->fSortData+numTris, buffTriCmpBackToFront(viewPos));

			UInt16* idx = fGroups[span->fGroupIdx]->GetIndexBufferData(span->fIBufferIdx) + span->fIStartIdx;
			plGBufferTriangle* iter = span->fSortData;
			int j;
			for( j = 0; j < numTris; j++ )
			{
				*idx++ = iter->fIndex1;
				*idx++ = iter->fIndex2;
				*idx++ = iter->fIndex3;
				iter++;
			}

			fGroups[span->fGroupIdx]->DirtyIndexBuffer(span->fIBufferIdx);
			fReadyToRender = false;
		}
	}

	plProfile_EndTiming(FaceSort);
}

#if 0

void plDrawableSpans::SortVisibleSpansUnit(const hsTArray<Int16>& visList, plPipeline* pipe)
{
	plProfile_Inc(FaceSortCalls);

	if( !visList.GetCount() )
		return;

	plProfile_BeginTiming(FaceSort);

	static hsLargeArray<UInt16>			triList;
	
	int i;
	
	if( pipe->IsDebugFlagSet( plPipeDbg::kFlagDontSortFaces ) )
	{
		/// Don't sort, just send unchanged
		int		j, idx;

		for( i = 0; i < visList.GetCount(); i++ )
		{
			plIcicle* span = (plIcicle*)fSpans[visList[i]];
			ICheckSpanForSortable(visList[i]);

			/// Build a fake list of indices....
			plGBufferTriangle*		list = span->fSortData;
			triList.SetCount( span->fILength );
			for( j = 0, idx = 0; j < span->fILength / 3; j++, idx += 3 )
			{
				triList[ idx ] = list[ j ].fIndex1;
				triList[ idx + 1 ] = list[ j ].fIndex2;
				triList[ idx + 2 ] = list[ j ].fIndex3;
			}

			/// Now send them on to the buffer group
			fGroups[ span->fGroupIdx ]->StuffFromTriList( span->fIBufferIdx, span->fIStartIdx, 
														  span->fILength / 3, triList.AcquireArray() );
		}
		fReadyToRender = false;
		return;
	}

	plProfile_EndTiming(FaceSort);

	plProfile_BeginLap(FaceSort, "0");

	struct sortFace
	{
		UInt16		fIndex0;
		UInt16		fIndex1;
		UInt16		fIndex2;
		hsScalar	fDist;
	};
	static hsLargeArray<sortFace>	sortList;

	struct SelectCloserFace
	{
		bool operator()(const sortFace* face0, const sortFace* face1) const
		{
			return face0->fDist < face1->fDist;
		}
	};
	hsPoint3 viewPos = fSpans[visList[0]]->fWorldToLocal * pipe->GetViewPositionWorld();

			hsScalar dist1 = (fViewPos - face1->fCenter).MagnitudeSquared();

	// First figure out the total number of tris to deal with.
	sortList.SetCount(0);
	int totTris = 0;
	for( i = 0; i < visList.GetCount(); i++ )
	{
		plIcicle* span = (plIcicle*)fSpans[visList[i]];

		int nTris = span->fILength / 3;
		sortList.Expand(sortList.GetCount() + nTris);

		plGBufferTriangle* sortData = span->fSortData;
		for( j = 0; j < nTris; j++ )
		{
			sortList.Append(*sortData++);	
		}

		totTris += nTris;
	}
	if( totTris == 0 )
	{
		plProfile_EndLap(FaceSort, "0");
		return;
	}

	plProfile_IncCount(FacesSorted, totTris);

	sortFace* pBegin = sortList.AcquireArray();
	sortFace* pEnd = pBegin + totTris;

	stl::sort(pBegin, pEnd, SelectCloserFace);

	triList.SetCount(sortList.GetCount());

	UInt16* pTri = triList.AcquireArray();

	while( pBegin < pEnd )
	{
		*pTri = pBegin->fIndex1;
		pTri++;
		*pTri = pBegin->fIndex2;
		pTri++;
		*pTri = pBegin->fIndex3;
		pTri++;
	}


	plProfile_EndLap(FaceSort, "0");

	UInt32 start = 0;
	for( i = 0; i < visList.GetCount(); i++ )
	{
		plIcicle* span = (plIcicle*)fSpans[visList[i]];

		/// Now send them on to the buffer group
		span->fIPackedIdx = span->fIStartIdx = newStarts[span->fGroupIdx][span->fIBufferIdx];
		newStarts[span->fGroupIdx][span->fIBufferIdx] += span->fILength;
		fGroups[ span->fGroupIdx ]->StuffFromTriList( span->fIBufferIdx, span->fIStartIdx, 
													  span->fILength / 3, triList.AcquireArray() + startIndex[visList[i]]);
	}

	plProfile_EndLap(FaceSort, "4");

	fReadyToRender = false;
}
#endif

void plDrawableSpans::SetInitialBone(int i, const hsMatrix44& l2b, const hsMatrix44& b2l)
{
	fLocalToBones[ i ] = l2b;
	fBoneToLocals[ i ] = b2l;
}

//// AppendDIMatrixSpans ///////////////////////////////////////////////////////////
//	Adds a di span which will only reference into the matrix palette. That is,
//	these indices won't index into any drawable data, they will only index into
//	the same matrix list that the drawable data itself can index into. When
//	an object (through its DrawInterface) sets a transform, it doesn't know
//	whether it's setting the transform for some drawable data it owns, or just
//	setting one of the matrices which influence the drawable data someone else
//	owns. 
UInt32	plDrawableSpans::AppendDIMatrixSpans(int n)
{
	/// Do garbage cleanup first
	if( fNeedCleanup )
		IRemoveGarbage();

	UInt32 baseIdx = fLocalToWorlds.GetCount();
	fLocalToWorlds.Expand(baseIdx + n);
	fLocalToWorlds.SetCount(baseIdx + n);
	fWorldToLocals.Expand(baseIdx + n);
	fWorldToLocals.SetCount(baseIdx + n);

	fLocalToBones.Expand(baseIdx + n);
	fLocalToBones.SetCount(baseIdx + n);
	fBoneToLocals.Expand(baseIdx + n);
	fBoneToLocals.SetCount(baseIdx + n);

	int i;
	for( i = baseIdx; i < baseIdx + n; i++ )
	{
		fLocalToWorlds[i].Reset();
		fWorldToLocals[i].Reset();

		fLocalToBones[i].Reset();
		fBoneToLocals[i].Reset();
	}

	return baseIdx;
}

// Look for a compatible set of palette matrices. Compatible means:
// a) Same number bones
// b) Same LocalToBones matrices (which implies same BoneToLocal matrices.
// Note that the LocalToBone transform is dependent on both the bone's transform
// and the transform of the object being skinned. In general, objects can only
// share a palette set if they have been flattened into world space (the object's
// transform is identity). Fortunately, this is a common case.
UInt32 plDrawableSpans::FindBoneBaseMatrix(const hsTArray<hsMatrix44>& initL2B, hsBool searchAll) const
{
	if (!searchAll)
	{
		// Just look amongst the added spans for a matching set
		int i;
		for( i = 0; i < fSpans.GetCount(); i++ )
		{
			if( fSpans[i] && (initL2B.GetCount() == fSpans[i]->fNumMatrices) )
			{
				int j;
				for( j = 0; j < initL2B.GetCount(); j++ )
				{
					if( initL2B[j] != fLocalToBones[j + fSpans[i]->fBaseMatrix] )
						break;
				}
				if( initL2B.GetCount() == j )
					return fSpans[i]->fBaseMatrix;
			}
		}
	}	
	else
	{
		// Since swappable geometry spans aren't added to the drawable until
		// runtime, a sharable bone pallete won't be found by scanning fSpans.
		// We have to do a larger search through all bone matrices.
		int i;
		for( i = 0; i + initL2B.GetCount() < fLocalToBones.GetCount(); i++ )
		{
			int j;
			for( j = 0; j < initL2B.GetCount(); j++ )
			{
				if( initL2B[j] != fLocalToBones[j + i] )
						break;
			}
			if( initL2B.GetCount() == j )
				return i;
		}
	}
	return UInt32(-1);
}

UInt32 plDrawableSpans::NewDIMatrixIndex()
{
	int index;
	/// Do we have a free lookup entry?
	for( index = 0; index < fDIIndices.GetCount(); index++ )
	{
		if( !fDIIndices[ index ]->GetCount() )
			break;
	}
	if( index == fDIIndices.GetCount() )
		fDIIndices.Append( TRACKED_NEW plDISpanIndex );

	fDIIndices[index]->Reset();
	fDIIndices[index]->fFlags = plDISpanIndex::kMatrixOnly;

	return index;
}

//// IFindDIIndices //////////////////////////////////////////////////////////
//	Finds the given DIIndices array and returns a pointer to it, or creates
//	a new one if requested

plDISpanIndex	*plDrawableSpans::IFindDIIndices( UInt32 &index )
{
	plDISpanIndex	*spanLookup;

	
	/// Do we have a free lookup entry?
	if( index == (UInt32)-1 ) // new index
	{
		for( index = 0; index < fDIIndices.GetCount(); index++ )
		{
			if( fDIIndices[ index ]->GetCount() == 0 )
				break;
		}
		if( index == fDIIndices.GetCount() )
			fDIIndices.Append( TRACKED_NEW plDISpanIndex );

		spanLookup = fDIIndices[ index ];
		spanLookup->fFlags = plDISpanIndex::kNone;
	}
	else
		spanLookup = fDIIndices[ index ]; // Just grab the one we requested

	return spanLookup;
}

//// AppendDISpans ///////////////////////////////////////////////////////////
//	Given a set of DI spans, appends them to the current storage buffers.
//	Note: AddDISpans() adds the spans to a list to be sorted, THEN put into
//	the buffers; this shoves them right in, bypassing the sorting altogether.

UInt32	plDrawableSpans::AppendDISpans( hsTArray<plGeometrySpan *> &spans, UInt32 index, hsBool clearSpansAfterAdd, 
									    hsBool doNotAddToSource, hsBool addToFront, int lod)
{
	hsAssert(spans.GetCount(), "Adding no spans? Blow me.");

	int				i, j;
	UInt32			spanIdx;
	plSpan			*span;
	hsBounds3Ext	bounds;
	plDISpanIndex	*spanLookup;
	UInt32			numAddedIcicle = 0;

//	hsAssert( fProps & kPropVolatile, "Trying to add spans on a non-volatile drawable" );

	/// Do garbage cleanup first
	if( fNeedCleanup )
		IRemoveGarbage();

	spanLookup = IFindDIIndices( index );

	if( GetNativeProperty(plDrawable::kPropCharacter) )
		fVisSet.SetBit(plVisMgr::kCharacter, true);

	int insertionPoint = 0;
	if( spans[0]->fProps & plGeometrySpan::kPartialSort )
	{
		insertionPoint = fSpans.GetCount();
	}
	else if( !addToFront )
	{
		int idx;
		for( idx = 0; (idx < fSpans.GetCount()) && !(fSpans[idx]->fProps & plSpan::kPartialSort); idx++ )
		{
		}
		insertionPoint = idx;
	}
	hsBool inserted = insertionPoint < fSpans.GetCount();

	/// Add the geometry spans to our list. Also add our internal span
	/// copies
	for( i = 0; i < spans.GetCount(); i++ )
	{
		spanIdx = fIcicles.GetCount();
		fIcicles.Append( plIcicle() );
		plIcicle *icicle = &fIcicles[ spanIdx ];
		IConvertGeoSpanToIcicle( spans[ i ], icicle, lod );
		span = (plSpan *)icicle;
		numAddedIcicle++;

		/// Add to our source indices
		spans[ i ]->fSpanRefIndex = insertionPoint+i;
		spanLookup->Append( spans[ i ]->fSpanRefIndex );
		fSpans.Insert( spans[ i ]->fSpanRefIndex, span );
		fSpanSourceIndices.Insert( spans[ i ]->fSpanRefIndex, spanIdx );

		if( GetNativeProperty(plDrawable::kPropCharacter) )
			span->SetVisBit(plVisMgr::kCharacter, true);

		/// Add the material to our list if necessary
		IAssignMatIdxToSpan( span, spans[ i ]->fMaterial );

		if( clearSpansAfterAdd )
		{
			delete spans[ i ];
			spans[ i ] = nil;
		}
		else if( !doNotAddToSource )
		{
			if( fSourceSpans.GetCount() < fSpans.GetCount() )
			{
				fSourceSpans.Expand( fSpans.GetCount() );
				// Since that does not change the use count, we still have to do that ourselves. ARGH!
				fSourceSpans.SetCount( fSpans.GetCount() );
			}

			fSourceSpans[ spans[ i ]->fSpanRefIndex ] = spans[ i ];
		}

		if( fProps & kPropSortFaces )
		{
			// Should add sort data too...
			IMakeSpanSortable( fSpans.GetCount() - 1 );
		}
	}

	if (inserted)
	{
		/// Go adjusting indices in the DI index list
		for( i = 0; i < fDIIndices.GetCount(); i++ )
		{
			if( !fDIIndices[ i ]->IsMatrixOnly() )
			{
				if (fDIIndices[ i ] == spanLookup)
					continue;

				for( j = 0; j < fDIIndices[ i ]->GetCount(); j++ )
				{
					if( (*fDIIndices[i])[j] >= insertionPoint )
					{
						(*fDIIndices[ i ])[ j ] += spans.GetCount();
						hsAssert((*fDIIndices[ i ])[ j ] < fSpans.GetCount(), "Span index snafu");
					}
				}
			}
		}
	}

	// Update fLocalBounds, since we were updating the world bounds
	fLocalBounds = fWorldBounds;
	fLocalBounds.Transform( &fWorldToLocal );
	fMaxWorldBounds = fWorldBounds;

	/// Rebuild the pointer array
	IRebuildSpanArray();

	fReadyToRender = false;

	return index;
}

//// IAddAMaterial ///////////////////////////////////////////////////////////

UInt32	plDrawableSpans::IAddAMaterial( hsGMaterial *material )
{
	UInt32		i;


	// Scan for if we already have it
	for( i = 0; i < fMaterials.GetCount(); i++ )
	{
		if( fMaterials[ i ] == material )
			return i;
	}

	// Scan again for a blank space to add into, if possible
	for( i = 0; i < fMaterials.GetCount(); i++ )
	{
		if( fMaterials[ i ] == nil )
		{
			fMaterials[ i ] = material;
			IRefMaterial( i );
			return i;
		}
	}

	// Add in to the end
	i = fMaterials.GetCount();
	fMaterials.Append( material );

	// Plus ref it
	IRefMaterial( i );

	return i;
}

//// IRefMaterial ////////////////////////////////////////////////////////////
//	Called if we already have a material index that we just want to use
//	again...

UInt32	plDrawableSpans::IRefMaterial( UInt32 index )
{
	hsGMaterial		*material = fMaterials[ index ];

	if( GetKey() && material != nil && material->GetKey() != nil )
		hsgResMgr::ResMgr()->AddViaNotify( material->GetKey(), TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnCreate, index, 0 ), plRefFlags::kActiveRef );

	return index;
}

//// ICheckToRemoveMaterial //////////////////////////////////////////////////
//	Runs through the span array to see if the given material index is still
//	used. If not, Release()s it and nil's it.

void	plDrawableSpans::ICheckToRemoveMaterial( UInt32 materialIdx )
{
	int				j;
	hsGMaterial		*mat;


	for( j = 0; j < fSpans.GetCount(); j++ )
	{
		if( fSpans[ j ]->fMaterialIdx == materialIdx )
			break;
	}
	if( j == fSpans.GetCount() )
	{
		/// No longer used--Release() it
		mat = fMaterials[ materialIdx ];
		if( GetKey() && mat != nil && mat->GetKey() != nil )
			GetKey()->Release( mat->GetKey() );
		fMaterials[ materialIdx ] = nil;
	}
}

//// IConvertGeoSpanToVertexSpan /////////////////////////////////////////////
//	Helper function for the two vertex-based convert functions.

hsBool	plDrawableSpans::IConvertGeoSpanToVertexSpan( plGeometrySpan *geoSpan, plVertexSpan *span, int lod, plVertexSpan *instancedParent)
{
	hsBounds3Ext	bounds;
	UInt8			groupIdx;
	UInt32			vbIndex, cellIdx, cellOffset;


	span->fLocalToWorld = geoSpan->fLocalToWorld;
	span->fWorldToLocal = geoSpan->fWorldToLocal;
	span->fProps |= ( geoSpan->fProps & plGeometrySpan::kPropRunTimeLight ) ? plSpan::kPropRunTimeLight : 0;
	if( geoSpan->fProps & plGeometrySpan::kPropNoShadowCast )
		span->fProps |= plSpan::kPropNoShadowCast;
	if( geoSpan->fProps & plGeometrySpan::kPropNoShadow )
		span->fProps |= plSpan::kPropNoShadow;
	if( geoSpan->fProps & plGeometrySpan::kPropForceShadow )
		span->fProps |= plSpan::kPropForceShadow;
	if( geoSpan->fProps & plGeometrySpan::kPropReverseSort )
		span->fProps |= plSpan::kPropReverseSort;
	if( geoSpan->fProps & plGeometrySpan::kPartialSort )
		span->fProps |= plSpan::kPartialSort;
	switch( geoSpan->fProps & plGeometrySpan::kLiteMask )
	{
		case plGeometrySpan::kLiteMaterial:			span->fProps |= plSpan::kLiteMaterial; break;
		case plGeometrySpan::kLiteVtxPreshaded:		span->fProps |= plSpan::kLiteVtxPreshaded; break;
		case plGeometrySpan::kLiteVtxNonPreshaded:	span->fProps |= plSpan::kLiteVtxNonPreshaded; break;
	}
	if( geoSpan->fProps & plGeometrySpan::kWaterHeight )
	{
		span->fProps |= plSpan::kWaterHeight;
		span->fWaterHeight = geoSpan->fWaterHeight;
	}
	if( geoSpan->fProps & plGeometrySpan::kVisLOS )
	{
		span->fProps |= plSpan::kVisLOS;
		fProps |= plDrawable::kPropHasVisLOS;
	}

	span->fNumMatrices = geoSpan->fNumMatrices;
	span->fBaseMatrix = geoSpan->fBaseMatrix;
	span->fLocalUVWChans = geoSpan->fLocalUVWChans;
	span->fMaxBoneIdx = geoSpan->fMaxBoneIdx;
	span->fPenBoneIdx = (UInt16)(geoSpan->fPenBoneIdx);
	span->fMinDist = geoSpan->fMinDist;
	span->fMaxDist = geoSpan->fMaxDist;

	bounds = geoSpan->fLocalBounds;
	span->fLocalBounds = bounds;
	bounds.Transform( &span->fLocalToWorld );
	span->fWorldBounds = bounds;
	fWorldBounds.Union( &bounds );
	span->fFogEnvironment = geoSpan->fFogEnviron;

	hsBool vertsVol = false;
	if( fProps & kPropVolatile )
		vertsVol = true;

	hsBool idxVol = false;
	if( fProps & kPropSortFaces )
		idxVol = true;
	if( geoSpan->fProps & plGeometrySpan::kPartialSort )
		idxVol = true;

	// Are we instanced?
	if( instancedParent != nil /*&& !( fProps & kPropVolatile )*/ )
	{
		/// We can instance w/o vert data IF 1) we're not the first span, 2) we can fit into the same buffer group, and
		/// 3) we can fit into the same vertex buffer in the buffer group
		if( instancedParent != span && ( geoSpan->fProps & plGeometrySpan::kPropNoPreShade ) )
		{
			/// WOW! We can actually share the *exact* same data, since we don't do any preshading. Coooooool!
			span->fGroupIdx = instancedParent->fGroupIdx;
			span->fVBufferIdx = instancedParent->fVBufferIdx;
			span->fVStartIdx = instancedParent->fVStartIdx;
			cellIdx = instancedParent->fCellIdx;
			cellOffset = instancedParent->fCellOffset;
		}
		else if( instancedParent != span && 
			( IFindBufferGroup( geoSpan->fFormat, geoSpan->fNumVerts, lod, vertsVol, idxVol ) == instancedParent->fGroupIdx ) &&
			fGroups[ instancedParent->fGroupIdx ]->GetNumVertsLeft( instancedParent->fVBufferIdx ) >= geoSpan->fNumVerts )
		{
			/// Boooring....
			/// Append the colors, but the vertices themselves we don't use, rather we point
			/// to our parent's verts (or rather, the cell will take care of that for us)
			groupIdx = (UInt8)(instancedParent->fGroupIdx);
			fGroups[ groupIdx ]->AppendToColorStorage( geoSpan, &vbIndex, &cellIdx, &cellOffset, instancedParent->fCellIdx );

			span->fGroupIdx = groupIdx;
			span->fVBufferIdx = instancedParent->fVBufferIdx;
			// Do a new vStart here, since it's really the colorStart, but oh well
			span->fVStartIdx = fGroups[ groupIdx ]->GetVertStartFromCell( vbIndex, cellIdx, cellOffset );
		}
		else
		{
			/// WE'RE the parent?? This means we're the first to get converted
			groupIdx = IFindBufferGroup( geoSpan->fFormat, geoSpan->fNumVerts, lod, vertsVol, idxVol );
			fGroups[ groupIdx ]->AppendToVertAndColorStorage( geoSpan, &vbIndex, &cellIdx, &cellOffset );

			span->fGroupIdx = groupIdx;
			span->fVBufferIdx = vbIndex;
			span->fVStartIdx = fGroups[ groupIdx ]->GetVertStartFromCell( vbIndex, cellIdx, cellOffset );
			geoSpan->fProps |= plGeometrySpan::kFirstInstance;
		}
	}
	else
	{
		// Pack the vertices in
		groupIdx = IFindBufferGroup( geoSpan->fFormat, geoSpan->fNumVerts, lod, vertsVol, idxVol );
		fGroups[ groupIdx ]->AppendToVertStorage( geoSpan, &vbIndex, &cellIdx, &cellOffset );

		span->fGroupIdx = groupIdx;
		span->fVBufferIdx = vbIndex;
		span->fVStartIdx = fGroups[ groupIdx ]->GetVertStartFromCell( vbIndex, cellIdx, cellOffset );
		geoSpan->fProps |= plGeometrySpan::kFirstInstance;
	}

	span->fCellIdx = cellIdx;
	span->fCellOffset = cellOffset;
	span->fVLength = geoSpan->fNumVerts;

	/// Add the material to our list if necessary
	span->fMaterialIdx = IAddAMaterial( geoSpan->fMaterial );

	return true;
}

//// IConvertGeoSpanToIcicle /////////////////////////////////////////////////

hsBool	plDrawableSpans::IConvertGeoSpanToIcicle(plGeometrySpan *geoSpan, plIcicle *icicle, int lod, plIcicle *instancedParent)
{
	UInt32		ibIndex, ibStart;


	IConvertGeoSpanToVertexSpan(geoSpan, icicle, lod, instancedParent);

/*
	Disabling this 8.16.2001. Works great until you have to SORT the faces, then things blow up. We could
	do this only when we won't sort faces, but it's easier right now to just never do this ever.

	if( instancedParent != nil && instancedParent != icicle && ( icicle->fProps & plSpan::kPropRunTimeLight ) )
	{
		/// WOW! We can actually share the *exact* same data, since we don't do any preshading. Coooooool!
		icicle->fIBufferIdx = instancedParent->fIBufferIdx;
		icicle->fIStartIdx = instancedParent->fIStartIdx;
		icicle->fILength = instancedParent->fILength;
		icicle->fSortData = nil;
	}
	else
*/
	{
		// Pack the indices in (automagically offsets by the start we give it)
		fGroups[ icicle->fGroupIdx ]->AppendToIndexStorage( geoSpan->fNumIndices, geoSpan->fIndexData,
														icicle->fVStartIdx, &ibIndex, &ibStart );

		icicle->fIBufferIdx = ibIndex;
		icicle->fIPackedIdx = icicle->fIStartIdx = ibStart;
		icicle->fILength = geoSpan->fNumIndices;
		icicle->fSortData = nil;
	}

	return true;
}

//// RemoveDIMatrixSpans
//	Nuke out a matrix only DI index set someone doesn't need anymore (cuz they're dead).
//	Since there's no data associated, there's not much to do, except mark the slot as
//	free and that some cleanup is in order some time. 
void	plDrawableSpans::RemoveDIMatrixSpans( UInt32 index )
{
	plDISpanIndex* spanIndices = fDIIndices[ index ];
	if( !spanIndices->IsMatrixOnly() )
		return;

	/// Flag that we need garbage cleanup now
	fNeedCleanup = true;
	fReadyToRender = false;

	spanIndices->Reset();
}

//// RemoveDISpans ///////////////////////////////////////////////////////////
//	Given a drawInterface index (i.e. the index into fDIIndices), deletes
//	the spans associated with that entry and clears the entry. Also packs
//	the spans, deletes the verts/indices associated with the deleted spans
//	and packs the vertex/index buffers as well.

void	plDrawableSpans::RemoveDISpans( UInt32 index ) 
{
	plDISpanIndex		*spanIndices = fDIIndices[ index ];

	if( spanIndices->IsMatrixOnly() )
	{
		RemoveDIMatrixSpans( index );
	}

	UInt32	i, j, k, idxRemoving, materialIdx;


// #define MF_RENDDEP
#ifdef MF_RENDDEP
	hsTArray<UInt32> spanInverseTable;
	spanInverseTable.SetCount(fSpans.GetCount());
	for( i = 0; i < fSpans.GetCount(); i++ )
		spanInverseTable[i] = i;
#endif // MF_RENDDEP

//	hsAssert( fProps & kPropVolatile, "Trying to remove spans on a non-volatile drawable" );
	hsAssert( spanIndices->GetCount() > 0, "If there are no DI spans, why were we called?" );

	/// Delete the actual spans themselves
	for( i = 0; i < spanIndices->GetCount(); i++ )
	{
		/// If this is the last use of this material, Release() it
		materialIdx = fSpans[ (*spanIndices)[ i ] ]->fMaterialIdx;

		/// Remove the source index and the object itself
		if( fSourceSpans.GetCount() && !( fSpans[ (*spanIndices)[ i ] ]->fTypeMask & plSpan::kParticleSpan ) )
		{
			delete fSourceSpans[ (*spanIndices)[i] ];
			fSourceSpans.Remove( (*spanIndices)[i] );
			for( j = (*spanIndices)[ i ]; j < fSourceSpans.GetCount(); j++ )
				fSourceSpans[ j ]->fSpanRefIndex = j;
		}
		fSpans[ (*spanIndices)[ i ] ]->Destroy();
		fSpans.Remove( (*spanIndices)[ i ] );
		idxRemoving = fSpanSourceIndices[ (*spanIndices)[ i ] ];
		fSpanSourceIndices.Remove( (*spanIndices)[ i ] );

		switch( idxRemoving & kSpanTypeMask )
		{
			case kSpanTypeIcicle:		fIcicles.Remove( idxRemoving & kSpanIDMask ); break;
			case kSpanTypeParticleSpan: fParticleSpans.Remove( idxRemoving & kSpanIDMask ); break;
		}

		// Do this AFTER the span array has been updated
		ICheckToRemoveMaterial( materialIdx );

		/// Go adjusting the source indices
		for( j = 0; j < fSpanSourceIndices.GetCount(); j++ )
		{
			if( ( ( fSpanSourceIndices[ j ] ^ idxRemoving ) & kSpanTypeMask ) == 0 )
			{
				/// Same type
				k = fSpanSourceIndices[ j ] & kSpanIDMask;
				if( k > ( idxRemoving & kSpanIDMask ) )
				{
					k--;
					fSpanSourceIndices[ j ] &= kSpanTypeMask;
					fSpanSourceIndices[ j ] |= k;
				}
			}
		}

#ifndef MF_RENDDEP
		/// Go adjusting indices in the DI index list
		for( j = 0; j < fDIIndices.GetCount(); j++ )
		{
			if( !fDIIndices[ j ]->IsMatrixOnly() )
			{
				for( k = 0; k < fDIIndices[ j ]->GetCount(); k++ )
				{
					if( (*fDIIndices[ j ])[ k ] > (*spanIndices)[ i ] )
						(*fDIIndices[ j ])[ k ]--;
				}
			}
		}
#else MF_RENDDEP
		spanInverseTable[(*spanIndices)[i]] = -1;
		for( j = (*spanIndices)[i]; j < fSpans.GetCount(); j++ )
			spanInverseTable[j]--;
#endif // MF_RENDDEP

	}
#ifdef MF_RENDDEP
	for( j = 0; j < fDIIndices.GetCount(); j++ )
	{
		if( !fDIIndices[j]->IsMatrixOnly() )
		{
			for( k = 0; k < fDIIndices[j]->GetCount(); k++ )
			{
				int idx = (*fDIIndices[j])[k];
				hsAssert(idx >= 0, "Just deleted a span another DI was pointing at");
				(*fDIIndices[j])[k] = spanInverseTable[idx];
			}
		}
	}
#endif // MF_RENDDEP
	
	/// Rebuild the pointer array, since it's now invalid
	IRebuildSpanArray();

	/// Flag that we need garbage cleanup now
	fNeedCleanup = true;
	fReadyToRender = false;
	
	/// Clear this entry. Note: DON'T DELETE IT! Otherwise they'll be hell
	/// to pay. Just clear it, so if we go looking for a free index, we just
	/// look for one with zero spans.
	spanIndices->Reset();

	/// All done!
}

//// IRebuildSpanArray ///////////////////////////////////////////////////////

void	plDrawableSpans::IRebuildSpanArray( void )
{
	UInt32		j, i;
	plIcicle	*icicle = nil;


	for( j = 0; j < fSpans.GetCount(); j++ )
	{
		switch( fSpanSourceIndices[ j ] & kSpanTypeMask )
		{
			case kSpanTypeIcicle: 
				icicle = &fIcicles[ fSpanSourceIndices[ j ] & kSpanIDMask ];
				fSpans[ j ] = (plSpan *)icicle; 
				if( icicle->fSortData != nil )
				{
					// GOTTA RESET THE SPAN INDICES TOO!!!
					for( i = 0; i < icicle->fILength / 3; i++ )
						icicle->fSortData[ i ].fSpanIndex = (UInt16)j;
				}
				break;
			case kSpanTypeParticleSpan: 
				fSpans[ j ] = (plSpan *)&fParticleSpans[ fSpanSourceIndices[ j ] & kSpanIDMask ]; 
				break;
		}
	}

	// Redid the span array, so cause the space tree to rebuild to match
	SetSpaceTree( nil );

	// Rebuild bit vectors for various span types
	IBuildVectors();
}


//// ICleanupMatrices
//	Find all the matrix slots in the palette that aren't being used, collapse
//	them out and adjust the indices into the matrix palette (the MatrixOnly DIIndices)
void	plDrawableSpans::ICleanupMatrices()
{
	// We really don't want to do this at runtime. The spans as read in have their bone
	// matrices fixed (via plSpan::fBaseMatrix && plSpan::fNumMatrices). We can't just
	// change where in our palette (fLocalToWorlds) bones will point, because the skinned spans
	// will still be looking in the same place. I'm not sure why we'd want to either.
	// Commenting out for the moment (so things will work), will look into why this was
	// ever here, and more mysterious, how it ever worked. mf
#if 0
	hsBitVector usedMatrices;

	int i, j, k;
	for( i = 0; i < fDIIndices.GetCount(); i++ )
	{
		plDISpanIndex& indices = *fDIIndices[i];
		if( indices.IsMatrixOnly() )
		{
			for( j = 0; j < indices.GetCount(); j++ )
				usedMatrices.SetBit(indices[j]);
		}
	}

	for( j = 0; j < fLocalToWorlds.GetCount(); j++ )
	{
		if( !usedMatrices.IsBitSet(j) )
		{
			for( i = 0; i < fDIIndices.GetCount(); i++ )
			{
				plDISpanIndex& indices = *fDIIndices[i];
				if( indices.IsMatrixOnly() )
				{
					for( k = 0; k < indices.GetCount(); k++ )
					{
						if( indices[k] > j )
							indices[k]--;
					}
				}
			}
			for( i = j+1; i < fLocalToWorlds.GetCount(); i++ )
			{
				fLocalToWorlds[i] = fLocalToWorlds[i-1];
				fWorldToLocals[i] = fWorldToLocals[i-1];
				fLocalToBones[i] = fLocalToBones[i-1];
				fBoneToLocals[i] = fBoneToLocals[i-1];
			}
		}
	}
#endif
}

//// IRemoveGarbage //////////////////////////////////////////////////////////
//	Cleans out all the unused spans. Oh, joy.

void	plDrawableSpans::IRemoveGarbage( void )
{
	int		groupIdx, ibIdx, i, j, k, count, offset;

	hsTArray<hsTArray<bool> *>				usedFlags;
	hsTArray<hsTArray<hsTArray<bool> *> *>	usedIdxFlags;
	plGBufferGroup							*group;


	ICleanupMatrices();

	/// Getting rid of verts
	usedFlags.SetCount( fGroups.GetCount() );
	usedIdxFlags.SetCount( fGroups.GetCount() );
	for( i = 0; i < fGroups.GetCount(); i++ )
	{
		hsAssert( fGroups[ i ]->GetNumVertexBuffers() == 1, "Cannot clean garbage on a non-volatile buffer group!" );
		usedFlags[ i ] = TRACKED_NEW hsTArray<bool>;
		usedFlags[ i ]->SetCountAndZero( fGroups[ i ]->GetVertBufferCount( 0 ) );

		usedIdxFlags[ i ] = TRACKED_NEW hsTArray<hsTArray<bool> *>;
		usedIdxFlags[ i ]->SetCount( fGroups[ i ]->GetNumIndexBuffers() );
		for( j = 0; j < fGroups[ i ]->GetNumIndexBuffers(); j++ )
		{
			(*usedIdxFlags[ i ])[ j ] = TRACKED_NEW hsTArray<bool>;
			(*usedIdxFlags[ i ])[ j ]->SetCountAndZero( fGroups[ i ]->GetIndexBufferCount( j ) );
		}
	}

	for( i = 0; i < fIcicles.GetCount(); i++ )
	{
		// Set the flags so we know these verts are used
		groupIdx = fIcicles[ i ].fGroupIdx;

		for( j = fIcicles[ i ].fCellOffset; j < fIcicles[ i ].fCellOffset + fIcicles[ i ].fVLength; j++ )
			(*usedFlags[ groupIdx ])[ j ] = true;

		for( j = fIcicles[ i ].fIStartIdx; j < fIcicles[ i ].fIStartIdx + fIcicles[ i ].fILength; j++ )
			(*((*usedIdxFlags[ groupIdx ])[ fIcicles[ i ].fIBufferIdx ]))[ j ] = true;
	}
	for( i = 0; i < fParticleSpans.GetCount(); i++ )
	{
		// Set the flags so we know these verts are used
		groupIdx = fParticleSpans[ i ].fGroupIdx;

		for( j = fParticleSpans[ i ].fCellOffset; j < fParticleSpans[ i ].fCellOffset + fParticleSpans[ i ].fVLength; j++ )
			(*usedFlags[ groupIdx ])[ j ] = true;

		for( j = fParticleSpans[ i ].fIStartIdx; j < fParticleSpans[ i ].fIStartIdx + fParticleSpans[ i ].fILength; j++ )
			(*((*usedIdxFlags[ groupIdx ])[ fParticleSpans[ i ].fIBufferIdx ]))[ j ] = true;
	}

	/// Now loop through and delete any unused
	for( groupIdx = 0; groupIdx < fGroups.GetCount(); groupIdx++ )
	{
		group = fGroups[ groupIdx ];
		hsTArray<bool>		&uFlags = *usedFlags[ groupIdx ];

		// Find groups of verts to delete!
		count = uFlags.GetCount();
		for( i = 0; i < count; )
		{
			// Skip through used verts
			for( ; i < count && uFlags[ i ] == true; i++ );

			// Find span of unused verts
			for( j = i; j < count && uFlags[ j ] == false; j++ );

			if( j > i )
			{
				// Delete this span of vertices
				group->DeleteVertsFromStorage( 0, i, j - i );

				// Adjust indices in this group
				for( ibIdx = 0; ibIdx < group->GetNumIndexBuffers(); ibIdx++ )
					group->AdjustIndicesInStorage( ibIdx, i, -(Int16)( j - i ) );

				// Adjust spans that use this vertex buffer
				for( k = 0; k < fIcicles.GetCount(); k++ )
				{
					if( fIcicles[ k ].fGroupIdx == groupIdx && fIcicles[ k ].fVBufferIdx == 0 &&
						fIcicles[ k ].fCellOffset >= i )
						fIcicles[ k ].fCellOffset -= j - i;

					// Adjust sorting data, if necessary
					if( fIcicles[ k ].fSortData != nil )
						IAdjustSortData( fIcicles[ k ].fSortData, fIcicles[ k ].fILength / 3, i, -(Int16)( j - i ) );
				}
				for( k = 0; k < fParticleSpans.GetCount(); k++ )
				{
					if( fParticleSpans[ k ].fGroupIdx == groupIdx && fParticleSpans[ k ].fVBufferIdx == 0 &&
						fParticleSpans[ k ].fCellOffset >= i )
						fParticleSpans[ k ].fCellOffset -= j - i;

					// Adjust sorting data, if necessary
					if( fParticleSpans[ k ].fSortData != nil )
						IAdjustSortData( fParticleSpans[ k ].fSortData, fParticleSpans[ k ].fILength / 3, i, -(Int16)( j - i ) );
				}

				// Move the flags too, lest we start deleting the wrong vertices
				count -= j - i;
				for( offset = j - i; i < count; i++ )
					uFlags[ i ] = uFlags[ i + offset ];
			}

			// Keep going
			i = j;
		}

		/// Getting rid of indices now
		/// IF WE ARE SORTING, then the fIStartIdx positions of the spans are not necessarily
		/// valid (since they're refreshed every frame, and not all are guaranteed to be refreshed).
		/// However, since we will be refilling the buffer next frame anyway, it doesn't really
		/// matter <crosses fingers> WHAT indices we delete, so long as the total length of the
		/// buffer is correct.
		if( fProps & kPropSortFaces )
		{
			for( ibIdx = 0; ibIdx < group->GetNumIndexBuffers(); ibIdx++ )
			{
				// For this index buffer, find the total length we want
				i = 0;
				for( j = 0; j < fIcicles.GetCount(); j++ )
				{
					if( fIcicles[ j ].fGroupIdx == groupIdx && fIcicles[ j ].fIBufferIdx == ibIdx )
						i += fIcicles[ j ].fILength;
				}
				for( j = 0; j < fParticleSpans.GetCount(); j++ )
				{
					if( fParticleSpans[ j ].fGroupIdx == groupIdx && fParticleSpans[ j ].fIBufferIdx == ibIdx )
						i += fParticleSpans[ j ].fILength;
				}

				/// i is the total length
				group->DeleteIndicesFromStorage( ibIdx, i, group->GetIndexBufferCount( ibIdx ) - i );
			}
		}
		else
		{
			/// Non-sorting, we have to figure out exactly which indices we aren't using

			for( ibIdx = 0; ibIdx < group->GetNumIndexBuffers(); ibIdx++ )
			{
				hsTArray<bool>	&uiFlags = *((*usedIdxFlags[ groupIdx ])[ ibIdx ]);

				// Find groups of indices to delete!
				count = uiFlags.GetCount();
				for( i = 0; i < count; )
				{
					// Skip through used verts
					for( ; i < count && uiFlags[ i ] == true; i++ );

					// Find span of unused verts
					for( j = i; j < count && uiFlags[ j ] == false; j++ );

					if( j > i )
					{
						// Delete this span of indices
						group->DeleteIndicesFromStorage( ibIdx, i, j - i );

						// Adjust spans that use this index buffer (only icicles use them)
						for( k = 0; k < fIcicles.GetCount(); k++ )
						{
							if( fIcicles[ k ].fGroupIdx == groupIdx && fIcicles[ k ].fIBufferIdx == ibIdx &&
								fIcicles[ k ].fIStartIdx >= i )
							{
								fIcicles[k].fIPackedIdx = (fIcicles[ k ].fIStartIdx -= j - i);
							}
						}
						// and particle spans :)
						for( k = 0; k < fParticleSpans.GetCount(); k++ )
						{
							if( fParticleSpans[ k ].fGroupIdx == groupIdx && fParticleSpans[ k ].fIBufferIdx == ibIdx &&
								fParticleSpans[ k ].fIStartIdx >= i )
							{
								fParticleSpans[k].fIPackedIdx = (fParticleSpans[ k ].fIStartIdx -= j - i);
							}
						}

						// Move the flags too, lest we start deleting the wrong vertices
						count -= j - i;
						for( offset = j - i; i < count; i++ )
							uiFlags[ i ] = uiFlags[ i + offset ];
					}

					// Keep going
					i = j;
				}
			}
		}
	}

	/// Refresh all vStartIdx entries
	for( i = 0; i < fSpans.GetCount(); i++ )
	{
		if( fSpans[ i ]->fTypeMask & plSpan::kVertexSpan )
		{
			plVertexSpan	*vtx = (plVertexSpan *)fSpans[ i ];
			vtx->fVStartIdx = fGroups[ vtx->fGroupIdx ]->GetVertStartFromCell( vtx->fVBufferIdx, vtx->fCellIdx, vtx->fCellOffset );
		}
	}

	/// Destroy!!!
	for( i = 0; i < usedFlags.GetCount(); i++ )
		delete usedFlags[ i ];
	usedFlags.Reset();

	for( i = 0; i < usedIdxFlags.GetCount(); i++ )
	{
		for( j = 0; j < usedIdxFlags[ i ]->GetCount(); j++ )
			delete ((*usedIdxFlags[ i ])[ j ]);
		delete usedIdxFlags[ i ];
	}
	usedIdxFlags.Reset();

	/// This will let us query the buffer group as to whether its ready to render or not.
	/// If not, we'll force it then to reload from its storage
	fReadyToRender = false;	

	// Whew!
	fNeedCleanup = false;
}

//// IAdjustSortData /////////////////////////////////////////////////////////
//	Adjusts the indices in the given sort data, using the delta and threshhold
//	given.

void	plDrawableSpans::IAdjustSortData( plGBufferTriangle *triList, UInt32 count, UInt32 threshhold, Int32 delta )
{
	UInt32		i;


	for( i = 0; i < count; i++ )
	{
		if( triList[ i ].fIndex1 >= threshhold )
			triList[ i ].fIndex1 = (UInt16)( triList[ i ].fIndex1 + delta );
		if( triList[ i ].fIndex2 >= threshhold )
			triList[ i ].fIndex2 = (UInt16)( triList[ i ].fIndex2 + delta );
		if( triList[ i ].fIndex3 >= threshhold )
			triList[ i ].fIndex3 = (UInt16)( triList[ i ].fIndex3 + delta );
	}
}

//// IMakeSpanSortable ////////////////////////////////////////////////////////
//	Given an index of a span, flags it sortable and creates the sorting data
//	for that span.

void	plDrawableSpans::IMakeSpanSortable( UInt32 index )
{
	plIcicle			*span = (plIcicle *)fSpans[ index ];
	plGBufferTriangle	*list;


	if( span->fProps & plSpan::kPropFacesSortable )
		return;

	/// Create data for it
	list = fGroups[ span->fGroupIdx ]->ConvertToTriList( (Int16)index, span->fIBufferIdx, span->fVBufferIdx, span->fCellIdx, 
														 span->fIStartIdx, span->fILength / 3 );
	if( list == nil )
		return;

	span->fSortData = list;

	/// Mark as sortable
	span->fProps |= plSpan::kPropFacesSortable;
}

void plDrawableSpans::UnPackCluster(plClusterGroup* cluster)
{
	const UInt32 vertsPerInst = cluster->GetTemplate()->NumVerts();
	const UInt32 idxPerInst = cluster->GetTemplate()->NumIndices();

	const UInt32 numClust = cluster->GetNumClusters();

	fVisSet |= cluster->GetVisSet();
	fVisNot |= cluster->GetVisNot();

	fIcicles.SetCount(numClust);
	fSpans.SetCount(numClust);
	int iSpan;
	for( iSpan = 0; iSpan < numClust; iSpan++ )
		fSpans[iSpan] = &fIcicles[iSpan];
	iSpan = 0;

	UInt32 vtxFormat =
		cluster->GetTemplate()->NumUVWs()
		| cluster->GetTemplate()->NumWeights() << 4;
	if( cluster->GetTemplate()->NumWgtIdx() )
		vtxFormat |= plGBufferGroup::kSkinIndices;

	const hsTArray<plLightInfo*>& lights = cluster->GetLights();

	int iStart;
	for( iStart = 0; iStart < cluster->GetNumClusters(); )
	{
		int numVerts = 0;
		int numIdx = 0;
		int iEnd;
		for( iEnd = iStart; iEnd < cluster->GetNumClusters(); iEnd++ )
		{
			numVerts += vertsPerInst * cluster->GetCluster(iEnd)->NumInsts();
			numIdx += idxPerInst * cluster->GetCluster(iEnd)->NumInsts();

			if( (numVerts > plGBufferGroup::kMaxNumVertsPerBuffer)
				||(numIdx > plGBufferGroup::kMaxNumIndicesPerBuffer) )
			{
				// Oops, too much.
				numVerts -= vertsPerInst * cluster->GetCluster(iEnd)->NumInsts();
				numIdx -= idxPerInst * cluster->GetCluster(iEnd)->NumInsts();

				iEnd--;

				break;
			}
		}

		// Still in trouble here. We need to fake up that cell crap for each of 
		// our clusters to make a span for it. Whoo-hoo.
		UInt8 grpIdx = IFindBufferGroup((UInt8)vtxFormat, numVerts, 0, false, false);
		
		UInt32 vbufferIdx;
		UInt32 cellIdx;
		UInt32 cellOffset;
		fGroups[grpIdx]->ReserveVertStorage(numVerts, &vbufferIdx, &cellIdx, &cellOffset, plGBufferGroup::kReserveInterleaved | plGBufferGroup::kReserveIsolate);
		hsAssert(!cellOffset, "This should be our own personal group");
		hsAssert(fGroups[grpIdx]->GetVertexSize() == cluster->GetTemplate()->Stride(), "Mismatch on src and dst sizes");
		
		UInt32 ibufferIdx;
		UInt32 istartIdx;
		fGroups[grpIdx]->ReserveIndexStorage(numIdx, &ibufferIdx, &istartIdx);
		UInt32 iOffset = 0;

		UInt8* vData = fGroups[grpIdx]->GetVertBufferData(vbufferIdx);
		UInt16* iData = fGroups[grpIdx]->GetIndexBufferData(ibufferIdx);
		UInt8* pvData = vData;
		UInt16* piData = iData;
		int i;
		for( i = iStart; i < iEnd; i++ )
		{
			hsBounds3Ext bnd;
			cluster->GetCluster(i)->UnPack(pvData, piData, cellOffset, bnd);
			fIcicles[iSpan].fLocalBounds = bnd;
			fIcicles[iSpan].fWorldBounds = bnd;

			fIcicles[iSpan].fTypeMask = plSpan::kSpan | plSpan::kVertexSpan | plSpan::kIcicleSpan;
			// STUB - need to set whether strictly runtime lit or preshaded based on cluster.
//			fIcicles[iSpan].fProps = plSpan::kLiteMaterial;
			fIcicles[iSpan].fProps = plSpan::kLiteVtxNonPreshaded | plSpan::kPropRunTimeLight;
			if (fProps & plSpan::kPropNoDraw)
				fIcicles[iSpan].fProps |= plSpan::kPropNoDraw;
			fIcicles[iSpan].fMaterialIdx = 0;
			fIcicles[iSpan].fLocalToWorld.Reset();
			fIcicles[iSpan].fWorldToLocal.Reset();
			fIcicles[iSpan].fBaseMatrix = 0;
			fIcicles[iSpan].fNumMatrices = 0;
			fIcicles[iSpan].fLocalUVWChans = 0;
			fIcicles[iSpan].fMaxBoneIdx = 0;
			fIcicles[iSpan].fPenBoneIdx = 0;
			fIcicles[iSpan].fFogEnvironment = nil;
			fIcicles[iSpan].fMaxDist = cluster->GetLOD().fMaxDist;
			fIcicles[iSpan].fMinDist = cluster->GetLOD().fMinDist;
			fIcicles[iSpan].fWaterHeight = 0;
			fIcicles[iSpan].fVisSet = cluster->GetVisSet();
			fIcicles[iSpan].fVisNot = cluster->GetVisNot();

			if( lights.GetCount() )
			{
				int iLight;
				for( iLight = 0; iLight < lights.GetCount(); iLight++ )
					fIcicles[iSpan].AddPermaLight(lights[iLight], lights[iLight]->GetProjection() != nil);
			}

			fIcicles[iSpan].fGroupIdx = grpIdx;
			fIcicles[iSpan].fVBufferIdx = vbufferIdx;
			fIcicles[iSpan].fCellIdx = cellIdx;
			fIcicles[iSpan].fCellOffset = cellOffset;
			fIcicles[iSpan].fVStartIdx = cellOffset;
			const UInt32 numVerts = cluster->GetCluster(i)->NumInsts() * vertsPerInst;
			fIcicles[iSpan].fVLength = numVerts;
			cellOffset += numVerts;

			fIcicles[iSpan].fIBufferIdx = ibufferIdx;
			fIcicles[iSpan].fIPackedIdx = fIcicles[iSpan].fIStartIdx = iOffset;
			const UInt32 iLength = cluster->GetCluster(i)->NumInsts() * cluster->GetTemplate()->NumIndices();
			fIcicles[iSpan].fILength = iLength;
			iOffset += iLength;

			iSpan++;

			const UInt32 vSize = cluster->GetCluster(i)->NumInsts() * cluster->GetTemplate()->VertSize();
			pvData += vSize;
			piData += iLength;
		}

		iStart = iEnd;
	}
	fMaterials.SetCountAndZero(1);
	plGenRefMsg* refMsg = TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kMsgMaterial);
	hsgResMgr::ResMgr()->SendRef(cluster->GetMaterial()->GetKey(), refMsg, plRefFlags::kActiveRef);

	fRenderLevel = cluster->GetRenderLevel();
	GetSpaceTree();
}

//// IFindBufferGroup ////////////////////////////////////////////////////////

UInt8	plDrawableSpans::IFindBufferGroup(UInt8 vtxFormat, UInt32 numVertsNeeded, int lod, hsBool vertVolatile, hsBool idxVolatile)
{
	int			i;


	// Scan through the buffer groups, looking for a good format. If there isn't
	// one, add it
	if( fProps & kPropVolatile )
		vertVolatile = true;
	if( fProps & kPropSortFaces )
		idxVolatile = true;

	for( i = 0; i < fGroups.GetCount(); i++ )
	{
		if( (fGroups[ i ]->GetVertexFormat() == vtxFormat)
			&& (!vertVolatile == !fGroups[i]->AreVertsVolatile())
			&& (!idxVolatile == !fGroups[i]->AreIdxVolatile())
			&& (lod == fGroups[i]->GetLOD()) )
		{
			if( fGroups[ i ]->GetNumPrimaryVertsLeft() >= numVertsNeeded )
				return i;
		}
	}

	// Add a new one of the right format
	fGroups.Append( TRACKED_NEW plGBufferGroup(vtxFormat, vertVolatile, idxVolatile, lod) );
	return i;
}

//// GetParticleSpanVector ///////////////////////////////////////////////////
//	Get a bitVector of the spans that are particle spans

hsBitVector const	&plDrawableSpans::GetParticleSpanVector( void ) const
{
	return fParticleSpanVector;
}

//// GetBlendingSpanVector ///////////////////////////////////////////////////
//	Get a bitVector of the spans that are blending (i.e. numMatrices > 0)

hsBitVector	const	&plDrawableSpans::GetBlendingSpanVector( void ) const
{
	/// See the plRenderMsg handler for why we do this
	return fFakeBlendingSpanVector;
}

//// SetBlendingSpanVectorBit ////////////////////////////////////////////////
//	Could just make GetBlendingSpanVector() non-const, but this way it's harder
//	for the end user, thus I'm hoping to discourage them from doing it too much.

void	plDrawableSpans::SetBlendingSpanVectorBit( UInt32 bitNumber, hsBool on )
{
	fFakeBlendingSpanVector.SetBit( bitNumber, on );
}

//// IBuildVectors ///////////////////////////////////////////////////////////

void	plDrawableSpans::IBuildVectors( void )
{
	int		i;
	bool	needRenderMsg = false;


	fParticleSpanVector.Clear();
	fBlendingSpanVector.Clear();
	for( i = 0; i < fSpans.GetCount(); i++ )
	{
		if( fSpans[ i ]->fTypeMask & plSpan::kParticleSpan )
			fParticleSpanVector.SetBit( i );
		// BoneUpdate
		if( ( fSpans[ i ]->fTypeMask & plSpan::kVertexSpan ) && (fSpans[ i ]->fNumMatrices > 2) )
//		if( ( fSpans[ i ]->fTypeMask & plSpan::kVertexSpan ) && (fSpans[ i ]->fNumMatrices > 1) )
		{
			fBlendingSpanVector.SetBit( i );
			needRenderMsg = true;
		}
	}
	// Reset this sucker too
	fFakeBlendingSpanVector = fBlendingSpanVector;

	if( needRenderMsg && !fRegisteredForRender )
	{
		// Placeholder hack - see IUpdateMatrixPaletteBoundsHack() for comments
		fRegisteredForRender = true;
		plgDispatch::Dispatch()->RegisterForExactType( plRenderMsg::Index(), GetKey() );
	} 
	else if( !needRenderMsg && fRegisteredForRender )
	{
		plgDispatch::Dispatch()->UnRegisterForExactType( plRenderMsg::Index(), GetKey() );
		fRegisteredForRender = false;
	}
}


//////////////////////////////////////////////////////////////////////////////
//// Particle Sets ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// CreateParticleSystem ////////////////////////////////////////////////////

UInt32		plDrawableSpans::CreateParticleSystem( UInt32 maxNumSpans, UInt32 maxNumParticles, hsGMaterial *material )
{
	UInt32			i, numVerts, numIndices;
	plParticleSet	*set;
	plDISpanIndex	*spanLookup;


	// Make a shiny new set
	set = TRACKED_NEW plParticleSet;
	set->fRefCount = 0;

	/// Fill out info
	numVerts = maxNumParticles * 4;		// 4 verts per particle
	numIndices = maxNumParticles * 6;	// 6 indices per particle

	if( material != nil && material->GetLayer( 0 ) != nil && material->GetLayer( 0 )->GetTexture() != nil )
		set->fFormat = plGeometrySpan::UVCountToFormat( 1 );
	else
		set->fFormat = plGeometrySpan::UVCountToFormat( 0 );

	// Reserve space
	set->fGroupIdx = IFindBufferGroup( set->fFormat, numVerts, 0, true, false );		// Force them to volatile even if the drawable isn't
	fGroups[ set->fGroupIdx ]->ReserveVertStorage( numVerts, &set->fVBufferIdx, &set->fCellIdx, &set->fCellOffset, plGBufferGroup::kReserveInterleaved | plGBufferGroup::kReserveIsolate );
	fGroups[ set->fGroupIdx ]->ReserveIndexStorage( numIndices, &set->fIBufferIdx, &set->fIStartIdx );

	set->fVStartIdx = fGroups[ set->fGroupIdx ]->GetVertStartFromCell( set->fVBufferIdx, set->fCellIdx, set->fCellOffset );
	set->fVLength = numVerts;
	set->fILength = numIndices;

	UInt32 vIdx = set->fVStartIdx;
	UInt16* idx = fGroups[set->fGroupIdx]->GetIndexBufferData(set->fIBufferIdx) + set->fIStartIdx;
	for( i = 0; i < maxNumParticles; i++ )
	{
		*idx++ = (UInt16)vIdx;
		*idx++ = (UInt16)(vIdx + 1);
		*idx++ = (UInt16)(vIdx + 2);

		*idx++ = (UInt16)vIdx;
		*idx++ = (UInt16)(vIdx + 2);
		*idx++ = (UInt16)(vIdx + 3);

		vIdx += 4;
	}

	// Create us some spans
	set->fDIEntry = -1;
	spanLookup = IFindDIIndices( set->fDIEntry );
	spanLookup->fFlags |= plDISpanIndex::kDontTransformSpans;

	set->fNumSpans = maxNumSpans;
	for( i = 0; i < maxNumSpans; i++ )
		ICreateParticleIcicle( material, set );

	set->fNextIStartIdx = set->fIStartIdx;
	set->fNextVStartIdx = set->fVStartIdx;
	set->fNextCellOffset = set->fCellOffset;


	/// Rebuild the pointer array
	IRebuildSpanArray();

	fReadyToRender = false;

	return set->fDIEntry;
}

//// IAssignMatIdxToSpan /////////////////////////////////////////////////////

void	plDrawableSpans::IAssignMatIdxToSpan( plSpan *span, hsGMaterial *mtl )
{
	fSettingMatIdxLock = true;

	if( mtl != nil )
		span->fMaterialIdx = IAddAMaterial( mtl );

	if( fMaterials[ span->fMaterialIdx ] != nil )
	{
		if( ITestMatForSpecularity( fMaterials[ span->fMaterialIdx ] ) )
			span->fProps |= plSpan::kPropMatHasSpecular;
		else
			span->fProps &= ~plSpan::kPropMatHasSpecular;
	}

	fSettingMatIdxLock = false;
}

//// ICreateParticleIcicle ///////////////////////////////////////////////////
//	Creates a shiny new plIcicle for use with particle sets.

plParticleSpan	*plDrawableSpans::ICreateParticleIcicle( hsGMaterial *material, plParticleSet *set )
{
	UInt32			spanIdx;
	hsMatrix44		ident;
	plDISpanIndex	*spanLookup;


	/// Ahh, an icicle span
	spanIdx = fParticleSpans.GetCount();
	fParticleSpans.Append( plParticleSpan() );
	plParticleSpan *icicle = &fParticleSpans[ spanIdx ];

	ident.Reset();
	IAssignMatIdxToSpan( icicle, material );
	icicle->fMaterialIdx = IAddAMaterial( material );
	icicle->fLocalToWorld = ident;
	icicle->fWorldToLocal = ident;
	icicle->fProps |= plSpan::kPropRunTimeLight | plSpan::kPropNoDraw | plSpan::kLiteVtxPreshaded;
	if( fProps & kPropSortSpans )
		icicle->fProps |= plSpan::kPropFacesSortable;

	icicle->fLocalBounds.MakeEmpty();
	icicle->fWorldBounds.MakeEmpty();

	icicle->fLocalBounds.Union( &hsPoint3(0,0,0) );
	icicle->fWorldBounds.Union( &hsPoint3(0,0,0) );

	icicle->fGroupIdx = set->fGroupIdx;
	icicle->fVBufferIdx = set->fVBufferIdx;
	icicle->fCellIdx = set->fCellIdx;
	icicle->fCellOffset = set->fCellOffset;
	icicle->fVStartIdx = set->fVStartIdx;
	icicle->fVLength = 0;

	icicle->fIBufferIdx = set->fIBufferIdx;
	icicle->fIPackedIdx = icicle->fIStartIdx = set->fIStartIdx;
	icicle->fILength = 0;
	icicle->fFogEnvironment = nil;
	icicle->fSortData = nil;

	icicle->fSrcSpanIdx = fSpans.GetCount();

	spanLookup = IFindDIIndices( set->fDIEntry );
	spanLookup->Append( fSpans.GetCount() );
	fSpans.Append( (plSpan *)icicle );
	fSpanSourceIndices.Append( spanIdx | kSpanTypeParticleSpan );

	/// Set our pointer to the set and inc it's refCount
	icicle->fParentSet = set;
	set->fRefCount++;

	/// Flag that we need garbage cleanup now
	fNeedCleanup = true;
	fReadyToRender = false;
	
	// Cause us to rebuild the space tree when needed
	SetSpaceTree( nil );

	return icicle;
}

//// ResetParticleSystem /////////////////////////////////////////////////////

void		plDrawableSpans::ResetParticleSystem( UInt32 setIndex )
{
	UInt32			i;
	plDISpanIndex	*indices = IFindDIIndices( setIndex );
	plParticleSet	*set;
	plParticleSpan	*span;

	for( i = 0; i < indices->GetCount(); i++ )
	{
		span = (plParticleSpan *)fSpans[ (*indices)[ i ] ];
		span->fVStartIdx = 0;
		span->fCellIdx = 0;
		span->fCellOffset = 0;
		span->fVLength = 0;
		span->fIPackedIdx = span->fIStartIdx = 0;
		span->fILength = 0;
		span->fSource = nil;
		span->fProps |= plSpan::kPropNoDraw;
		GetSpaceTree()->SetLeafFlag( (Int16)(span->fSrcSpanIdx), plSpaceTreeNode::kDisabled );

		set = span->fParentSet;			// To use at the end of the loop
	}

	set->fNextIStartIdx = set->fIStartIdx;
	set->fNextVStartIdx = set->fVStartIdx;
	set->fNextCellOffset = set->fCellOffset;

	fGroups[set->fGroupIdx]->SetVertBufferEnd(set->fVBufferIdx, set->fNextVStartIdx);
	if( fProps & kPropSortFaces )
		fGroups[set->fGroupIdx]->SetIndexBufferEnd(set->fIBufferIdx, set->fNextIStartIdx);
}		


//// AssignEmitterToParticleSystem ///////////////////////////////////////////

void	plDrawableSpans::AssignEmitterToParticleSystem( UInt32 setIndex, plParticleEmitter *emitter )
{
	plDISpanIndex	*indices = IFindDIIndices( setIndex );
	plParticleSet	*set;
	plParticleSpan	*icicle;
	UInt32			i, index, numParticles = emitter->GetParticleCount();
	plParticleCore	*array = emitter->GetParticleArray();
	hsMatrix44		ident;

	plGBufferTriangle	*sortArray;

	
	icicle = (plParticleSpan *)fSpans[ (*indices)[ emitter->GetSpanIndex() ] ];
	set = icicle->fParentSet;

	icicle->fCellIdx = set->fCellIdx;
	icicle->fCellOffset = set->fNextCellOffset;
	icicle->fVStartIdx = set->fNextVStartIdx;
	icicle->fIPackedIdx = icicle->fIStartIdx = set->fNextIStartIdx;
	icicle->fVLength = numParticles * 4;
	icicle->fILength = numParticles * 6;
	icicle->fSource = emitter;
	icicle->fProps &= ~plSpan::kPropNoDraw;
	GetSpaceTree()->ClearLeafFlag( (Int16)(icicle->fSrcSpanIdx), plSpaceTreeNode::kDisabled );
	icicle->fNumParticles = numParticles;
	set->fNextVStartIdx += numParticles * 4;
	set->fNextIStartIdx += numParticles * 6;
	set->fNextCellOffset += numParticles * 4;

	fGroups[set->fGroupIdx]->SetVertBufferEnd(set->fVBufferIdx, set->fNextVStartIdx);
	if( fProps & kPropSortFaces )
		fGroups[set->fGroupIdx]->SetIndexBufferEnd(set->fIBufferIdx, set->fNextIStartIdx);

	hsAssert( set->fNextVStartIdx <= set->fVStartIdx + set->fVLength, "Buffer overflow in AddParticlesToSet()" );
	hsAssert( set->fNextIStartIdx <= set->fIStartIdx + set->fILength, "Buffer overflow in AddParticlesToSet()" );

	if( fProps & kPropSortFaces )
	{
		// Prep sorting data
		if( icicle->fSortData == nil || icicle->fSortCount < ( numParticles << 1 ) )
		{
			delete [] icicle->fSortData;
			icicle->fSortData = sortArray = TRACKED_NEW plGBufferTriangle[ numParticles << 1 ];
			icicle->fSortCount = numParticles << 1;
		}
		else
			sortArray = icicle->fSortData;

		// Find our bounds and fill out sorting data
		for( i = 0, index = icicle->fVStartIdx; i < numParticles; i++ )
		{
			sortArray->fCenter = array[ i ].fPos;
			sortArray->fIndex1 = (UInt16)index;
			sortArray->fIndex2 = (UInt16)(index + 1);
			sortArray->fIndex3 = (UInt16)(index + 2);
			sortArray->fSpanIndex = (UInt16)(icicle->fSrcSpanIdx);
			sortArray++;

			sortArray->fCenter = array[ i ].fPos;
			sortArray->fIndex1 = (UInt16)index;
			sortArray->fIndex2 = (UInt16)(index + 2);
			sortArray->fIndex3 = (UInt16)(index + 3);
			sortArray->fSpanIndex = (UInt16)(icicle->fSrcSpanIdx);
			sortArray++;

			index += 4;
		}
	}

	icicle->fLocalBounds = emitter->GetBoundingBox();
	icicle->fWorldBounds = icicle->fLocalBounds;

	GetSpaceTree()->MoveLeaf( (Int16)(icicle->fSrcSpanIdx), icicle->fWorldBounds );
	// Done for now, we'll fill on the pipeline side

	return;
}

//////////////////////////////////////////////////////////////////////////////
//// Dynamic Functions for SceneViewer ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// RefreshDISpans //////////////////////////////////////////////////////////
//	Given a set of plGeometrySpans, replaces the data in the given DI span set
//	with the vertices in the plGeometrySpans. Does NOT replace the indices,
//	and the count must be EXACT, or this function gets pyst.
//MFREPACK
// don't need to pass in spans, they're already in fSourceSpans.
// clearSpansAfterRefresh is ignored anyway, nuke it.
UInt32	plDrawableSpans::RefreshDISpans( UInt32 index )
{
	int				i;
	UInt32			spanIdx;
	plSpan			*span;
	hsBounds3Ext	bounds;
	plDISpanIndex	*spanLookup;

//	hsAssert( fProps & kPropVolatile, "Trying to add spans on a non-volatile drawable" );
	hsAssert( index != (UInt32)-1, "Invalid DI span index" );

	/// Do garbage cleanup first
	if( fNeedCleanup )
		IRemoveGarbage();

	spanLookup = IFindDIIndices( index );

	/// Loop through the spans and copy the vertex data over
	for( i = 0; i < spanLookup->GetCount(); i++ )
	{
		// Main info
		plGeometrySpan	*geoSpan = fSourceSpans[ (*spanLookup)[i] ];
		spanIdx = fSpanSourceIndices[ (*spanLookup)[ i ] ];
		hsAssert( ( spanIdx & kSpanTypeMask ) == kSpanTypeIcicle, "Mismatch in span formats" );
		plIcicle *icicle = &fIcicles[ spanIdx & kSpanIDMask ];
		IUpdateIcicleFromGeoSpan( geoSpan, icicle );
		span = (plSpan *)icicle;
		if( fProps & kPropSortFaces )
		{
			// Should add sort data too...
			IMakeSpanSortable( (*spanLookup)[ i ] );
		}
		
	}

	// Update fLocalBounds, since we were updating the world bounds
	fLocalBounds = fWorldBounds;
	fLocalBounds.Transform( &fWorldToLocal );
	fMaxWorldBounds = fWorldBounds;

	// Cause us to rebuild the space tree when needed
	SetSpaceTree( nil );

	fReadyToRender = false;

	return index;
}

// Same as above, but takes an actual span index (not a DI Index).
UInt32	plDrawableSpans::RefreshSpan( UInt32 index )
{
	UInt32			spanIdx;
	plSpan			*span;
	hsBounds3Ext	bounds;

//	hsAssert( fProps & kPropVolatile, "Trying to add spans on a non-volatile drawable" );
	hsAssert( index < fSourceSpans.GetCount(), "Invalid span index" );

	/// Do garbage cleanup first
	if( fNeedCleanup )
		IRemoveGarbage();

	// Main info
	plGeometrySpan	*geoSpan = fSourceSpans[ index ];
	spanIdx = fSpanSourceIndices[ index ];
	hsAssert( ( spanIdx & kSpanTypeMask ) == kSpanTypeIcicle, "Mismatch in span formats" );
	plIcicle *icicle = &fIcicles[ spanIdx & kSpanIDMask ];
	IUpdateIcicleFromGeoSpan( geoSpan, icicle );
	span = (plSpan *)icicle;
	if( fProps & kPropSortFaces )
	{
		// Should add sort data too...
		IMakeSpanSortable( index );
	}

	// Update fLocalBounds, since we were updating the world bounds
	fLocalBounds = fWorldBounds;
	fLocalBounds.Transform( &fWorldToLocal );
	fMaxWorldBounds = fWorldBounds;

	// Cause us to rebuild the space tree when needed
	SetSpaceTree( nil );

	fReadyToRender = false;

	return index;
}

//// IUpdateVertexSpanFromGeoSpan ////////////////////////////////////////////
//	Common function for the two functions that follow.

void	plDrawableSpans::IUpdateVertexSpanFromGeoSpan( plGeometrySpan *geoSpan, plVertexSpan *span )
{
	hsBounds3Ext	bounds;

	// This function looks pretty dubious to me. It or's in properties instead of setting them, and
	// it doesn't set all the available properties (like the skinning matrix indices). It clearly should
	// be doing more or doing less, but it's unclear which. I'm guessing the latter, but until there's
	// a reason to dig through this trash, here it stays. mf
	
	hsAssert( geoSpan->fNumVerts == span->fVLength, "Vertex count mismatch in IUpdateVertexSpanFromGeoSpan()" );

	span->fLocalToWorld = geoSpan->fLocalToWorld;
	span->fWorldToLocal = geoSpan->fWorldToLocal;
	span->fProps |= ( geoSpan->fProps & plGeometrySpan::kPropRunTimeLight ) ? plSpan::kPropRunTimeLight : 0;
	if( geoSpan->fProps & plGeometrySpan::kPropNoShadowCast )
		span->fProps |= plSpan::kPropNoShadowCast;
	if( geoSpan->fProps & plGeometrySpan::kPropNoShadow )
		span->fProps |= plSpan::kPropNoShadow;
	if( geoSpan->fProps & plGeometrySpan::kPropForceShadow )
		span->fProps |= plSpan::kPropForceShadow;
	if( geoSpan->fProps & plGeometrySpan::kPropReverseSort )
		span->fProps |= plSpan::kPropReverseSort;
	if( geoSpan->fProps & plGeometrySpan::kPartialSort )
		span->fProps |= plSpan::kPartialSort;
	switch( geoSpan->fProps & plGeometrySpan::kLiteMask )
	{
		case plGeometrySpan::kLiteMaterial:			span->fProps |= plSpan::kLiteMaterial; break;
		case plGeometrySpan::kLiteVtxPreshaded:		span->fProps |= plSpan::kLiteVtxPreshaded; break;
		case plGeometrySpan::kLiteVtxNonPreshaded:	span->fProps |= plSpan::kLiteVtxNonPreshaded; break;
	}
	if( geoSpan->fProps & plGeometrySpan::kWaterHeight )
	{
		span->fProps |= plSpan::kWaterHeight;
		span->fWaterHeight = geoSpan->fWaterHeight;
	}
	if( geoSpan->fProps & plGeometrySpan::kVisLOS )
	{
		span->fProps |= plSpan::kVisLOS;
		fProps |= plDrawable::kPropHasVisLOS;
	}

	bounds = geoSpan->fLocalBounds;
	span->fLocalBounds = bounds;
	bounds.Transform( &span->fLocalToWorld );
	span->fWorldBounds = bounds;
	fWorldBounds.Union( &bounds );

	// Pack the vertices in
	fGroups[ span->fGroupIdx ]->StuffToVertStorage( geoSpan, span->fVBufferIdx, span->fCellIdx, span->fCellOffset,
														plGBufferGroup::kReserveInterleaved );

	span->fFogEnvironment = geoSpan->fFogEnviron;

	/// Add the material to our list if necessary
	IAssignMatIdxToSpan( span, geoSpan->fMaterial );
}

//// IUpdateIcicleFromGeoSpan ////////////////////////////////////////////////

void	plDrawableSpans::IUpdateIcicleFromGeoSpan( plGeometrySpan *geoSpan, plIcicle *icicle )
{
	IUpdateVertexSpanFromGeoSpan( geoSpan, icicle );
}

hsPoint3& plDrawableSpans::GetPosition(int spanIdx, int vtxIdx) 
{ 
	hsAssert(fSpans[spanIdx]->fTypeMask & plSpan::kVertexSpan, "Getting vertex info on non-vertex based span");
	plVertexSpan* span = (plVertexSpan*)fSpans[spanIdx];

	return fGroups[span->fGroupIdx]->Position(span->fVBufferIdx, span->fCellIdx, vtxIdx); 
}
hsVector3& plDrawableSpans::GetNormal(int spanIdx, int vtxIdx) 
{ 
	hsAssert(fSpans[spanIdx]->fTypeMask & plSpan::kVertexSpan, "Getting vertex info on non-vertex based span");
	plVertexSpan* span = (plVertexSpan*)fSpans[spanIdx];

	return fGroups[span->fGroupIdx]->Normal(span->fVBufferIdx, span->fCellIdx, vtxIdx); 
}

UInt32 plDrawableSpans::GetNumTris(int spanIdx)
{
	hsAssert(fSpans[spanIdx]->fTypeMask & plSpan::kIcicleSpan, "Asking for index info on non-triangle based span");
	plIcicle* span = (plIcicle*)fSpans[spanIdx];
	return fGroups[span->fGroupIdx]->GetIndexBufferCount(span->fIBufferIdx) / 3;
}

UInt16* plDrawableSpans::GetIndexList(int spanIdx)
{
	hsAssert(fSpans[spanIdx]->fTypeMask & plSpan::kIcicleSpan, "Asking for index info on non-triangle based span");
	plIcicle* span = (plIcicle*)fSpans[spanIdx];
	return fGroups[span->fGroupIdx]->GetIndexBufferData(span->fIBufferIdx);
}

hsPoint3& plDrawableSpans::CvtGetPosition(int spanIdx, int vtxIdx)
{
	return *(hsPoint3*)(fSourceSpans[spanIdx]->fVertexData + vtxIdx * plGeometrySpan::GetVertexSize(fSourceSpans[spanIdx]->fFormat));
}

hsVector3& plDrawableSpans::CvtGetNormal(int spanIdx, int vtxIdx)
{
	// Normal follows the position (+ 12bytes)
	return *(hsVector3*)(fSourceSpans[spanIdx]->fVertexData + vtxIdx * plGeometrySpan::GetVertexSize(fSourceSpans[spanIdx]->fFormat) + 12);
}

UInt32 plDrawableSpans::CvtGetNumTris(int spanIdx)
{
	return fSourceSpans[spanIdx]->fNumIndices / 3;
}

UInt16* plDrawableSpans::CvtGetIndexList(int spanIdx)
{
	return fSourceSpans[spanIdx]->fIndexData;
}

UInt32 plDrawableSpans::CvtGetNumVerts(int spanIdx) const
{
	return fSourceSpans[spanIdx]->fNumVerts;
}

plGeometrySpan* plDrawableSpans::GetGeometrySpan(int spanIdx) 
{ 
	return fSourceSpans[spanIdx]; 
}

void	plDrawableSpans::GetOrigGeometrySpans( UInt32 diIndex, hsTArray<plGeometrySpan *> &arrayToFill )
{
	if( diIndex >= fDIIndices.GetCount() )
	{
		hsAssert( false, "Invalid diIndex to GetOrigGeometrySpans()" );
		return;
	}

	plDISpanIndex	*indices = fDIIndices[ diIndex ];
	UInt32		i;


	arrayToFill.Reset();
	for( i = 0; i < indices->GetCount(); i++ )
		arrayToFill.Append( fSourceSpans[ (*indices)[ i ] ] );

	// HA!
	return;
}

void plDrawableSpans::ClearAndSetMaterialCount(UInt32 count)
{
	// Release the old materials
	for (int i = 0; i < fMaterials.GetCount(); i++)
	{
		if ( fMaterials[ i ] != nil && fMaterials[ i ]->GetKey() != nil )
			GetKey()->Release( fMaterials[ i ]->GetKey() );
	}

	fMaterials.SetCountAndZero(count);
}

