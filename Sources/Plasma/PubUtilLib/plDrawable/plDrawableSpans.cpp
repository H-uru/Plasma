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
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  plDrawableSpans Class Functions                                         //
//                                                                          //
//// Version History /////////////////////////////////////////////////////////
//                                                                          //
//  4.3.2001 mcn - Created.                                                 //
//  5.3.2001 mcn - Completely revamped. Now plDrawableSpans *IS* the group  //
//                 of spans, and each span is either an icicle or patch.    //
//                 This eliminates the need entirely for separate drawables,//
//                 at the cost of having to do a bit of extra work to       //
//                 maintain different types of spans in the same drawable.  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"

#include "plgDispatch.h"
#include "plPipeDebugFlags.h"
#include "plPipeline.h"
#include "plProfile.h"
#include "hsResMgr.h"
#include "hsStream.h"


#include "plAccessSpan.h"
#include "plAccessTriSpan.h"

#include "plDrawableSpans.h"

#include "plGeometrySpan.h"
#include "plSpaceTree.h"
#include "plParticleFiller.h"
#include "plSpaceTreeMaker.h"

#include "plClusterGroup.h"
#include "plCluster.h"
#include "plSpanTemplate.h"
#include "plGBufferGroup.h"

#include "plMath/hsRadixSort.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayerInterface.h"
#include "plPipeline/plFogEnvironment.h"
#include "hsGDeviceRef.h"
#include "pnMessage/plRefMsg.h"
#include "pnMessage/plNodeRefMsg.h" 
#include "pnMessage/plDISpansMsg.h"
#include "plMessage/plDeviceRecreateMsg.h"
#include "plMessage/plRenderMsg.h"
#include "pnSceneObject/plDrawInterface.h"
#include "pnKeyedObject/plKey.h"
#include "plParticleSystem/plParticleEmitter.h"
#include "plParticleSystem/plParticle.h"
#include "plGLight/plLightInfo.h"

#include "plMath/plTriUtils.h"

#include "pnMessage/plPipeResMakeMsg.h"

#include "plScene/plVisMgr.h"
#include "plScene/plVisRegion.h"

#include "plStatusLog/plStatusLog.h"

#include <algorithm>

//// Local Konstants /////////////////////////////////////////////////////////

const uint32_t        plDrawableSpans::kSpanTypeMask          = 0xc0000000;
const uint32_t        plDrawableSpans::kSpanIDMask            = ~kSpanTypeMask;
const uint32_t        plDrawableSpans::kSpanTypeIcicle        = 0x00000000;
const uint32_t        plDrawableSpans::kSpanTypeParticleSpan  = 0xc0000000;

//// Constructor & Destructor ////////////////////////////////////////////////

plDrawableSpans::plDrawableSpans() :
    fSpaceTree()
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
    fMaterials.clear();

    fLocalToWorld.Reset();
    fWorldToLocal.Reset();

    fVisSet.SetBit(0);
}

plDrawableSpans::~plDrawableSpans()
{
    for (plGBufferGroup* group : fGroups)
        delete group;
    fGroups.clear();

    /// Loop and delete both our types of spans
    for (plSpan* span : fSpans)
        span->Destroy();
    fSpans.clear();
    fIcicles.clear();
    fParticleSpans.clear();

    for (plGeometrySpan* geoSpan : fSourceSpans)
        delete geoSpan;
    fSourceSpans.clear();

    /// Loop and unref our materials
    if (GetKey() != nullptr)
    {
        for (hsGMaterial* material : fMaterials)
        {
            if (material != nullptr && material->GetKey() != nullptr)
                GetKey()->Release(material->GetKey());
        }
    }
    fMaterials.clear();

    delete fSpaceTree;

    for (plDISpanIndex* di : fDIIndices)
        delete di;
    fDIIndices.clear();

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

void    plDrawableSpans::SetSceneNode( plKey newNode )
{
    plKey curNode=GetSceneNode();
    if( curNode == newNode )
        return;
    if( newNode )
    {
        plNodeRefMsg* refMsg = new plNodeRefMsg(newNode, plNodeRefMsg::kOnRequest, -1, plNodeRefMsg::kDrawable);
        hsgResMgr::ResMgr()->SendRef(GetKey(), refMsg, plRefFlags::kActiveRef);
    }
    if( curNode )
    {
        curNode->Release(GetKey());
    }
    fSceneNode = newNode;
}

//// PrepForRender ///////////////////////////////////////////////////////////

void    plDrawableSpans::PrepForRender( plPipeline *p )
{
    /// If we're not registered for this message, register for it so we know when
    /// we need to refresh the buffers
    if (!fRegisteredForRecreate && GetKey() != nullptr)
    {
        fRegisteredForRecreate = true;
        plgDispatch::Dispatch()->RegisterForExactType( plDeviceRecreateMsg::Index(), GetKey() );
    }

    if( !fReadyToRender )
    {
        for (plGBufferGroup* group : fGroups)
        {
            // Each group will decide whether it needs to be prepped
            group->PrepForRendering(p, false);
        }

        fReadyToRender = true;
    }

    if (!fParticleSpans.empty())
    {
#if 0
        for (plSpan* baseSpan : fSpans)
        {
            if (baseSpan->fTypeMask & plSpan::kParticleSpan)
            {
                plParticleSpan* span = (plParticleSpan*)baseSpan;
                
                plParticleFiller::FillParticles(p, this, span);
            }
        }
#else
        for (plParticleSpan& span : fParticleSpans)
        {
            plParticleFiller::FillParticles(p, this, &span);
        }
#endif
    }
}

void plDrawableSpans::SetDISpanVisSet(uint32_t diIndex, hsKeyedObject* ref, bool on)
{
    // Could actually do something here to neutralize bones, but we're not.
    // Main thing is that if it's Matrix Only, then the indices are into
    // the LocalToWorlds, not into fSpans
    if( fDIIndices[diIndex]->IsMatrixOnly() )
        return;

    plVisRegion* reg = plVisRegion::ConvertNoRef(ref);
    if( !reg )
        return;
    bool isNot = reg->GetProperty(plVisRegion::kIsNot);
    uint32_t visRegIndex = reg->GetIndex();

    if( isNot )
    {
        fVisNot.SetBit(visRegIndex, on);
        for (size_t i = 0; i < fDIIndices[diIndex]->GetCount(); i++)
        {
            uint32_t spanIndex = (*fDIIndices[diIndex])[i];
            fSpans[spanIndex]->SetVisNot(visRegIndex, on);
        }
    }
    else
    {
        fVisSet.SetBit(visRegIndex, on);
        for (size_t i = 0; i < fDIIndices[diIndex]->GetCount(); i++)
        {
            uint32_t spanIndex = (*fDIIndices[diIndex])[i];
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
        const hsBitVector&  visSet = visMgr->GetVisSet();
        const hsBitVector&  visNot = visMgr->GetVisNot();

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
            
            for (size_t i = 0; i < fSpans.size(); i++)
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
        GetSpaceTree()->SetCache(nullptr);
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
    for (size_t i = 0; i < fSpans.size(); i++)
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

bool plDrawableSpans::IBoundsInvalid(const hsBounds3Ext& bnd) const
{
    int i;
    for( i = 0; i < 3; i++ )
    {
        const float kLimit(1.e5f);

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

#ifdef MF_TEST_UPDATE
plProfile_CreateCounter("DSSetTrans", "Update", DSSetTrans);
plProfile_CreateCounter("DSMatSpans", "Update", DSMatSpans);
plProfile_CreateCounter("DSRegSpans", "Update", DSRegSpans);

plProfile_CreateTimer("DSSetTransT", "Update", DSSetTransT);
plProfile_CreateTimer("DSMatTransT", "Update", DSMatTransT);
plProfile_CreateTimer("DSRegTransT", "Update", DSRegTransT);
plProfile_CreateTimer("DSBndTransT", "Update", DSBndTransT);
#endif // MF_TEST_UPDATE


plDrawable& plDrawableSpans::SetTransform( uint32_t index, const hsMatrix44& l2w, const hsMatrix44& w2l ) 
{ 
#ifdef MF_TEST_UPDATE
    plProfile_IncCount(DSSetTrans, 1);
    plProfile_BeginTiming(DSSetTransT);
#endif // MF_TEST_UPDATE

    if( index == (uint32_t)-1 )
    {
        fLocalToWorld = l2w; 
        fWorldToLocal = w2l; 

        fWorldBounds = fLocalBounds; 
        fWorldBounds.Transform( &l2w ); 
    }
    else
    {
        uint32_t          idx;
        plDISpanIndex   *spans = fDIIndices[ index ];       


        if( spans->IsMatrixOnly() )
        {
#ifdef MF_TEST_UPDATE
            plProfile_IncCount(DSMatSpans, spans->GetCount());
            plProfile_BeginTiming(DSMatTransT);
#endif // MF_TEST_UPDATE
            for (size_t i = 0; i < spans->GetCount(); i++)
            {
                fLocalToWorlds[ (*spans)[ i ] ] = IMatrixMul34(l2w, fLocalToBones[ (*spans)[ i ] ]);
                fWorldToLocals[ (*spans)[ i ] ] = IMatrixMul34(fBoneToLocals[ (*spans)[ i ] ], w2l);

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
            for (size_t i = 0; i < spans->GetCount(); i++)
            {           
#ifdef MF_TEST_UPDATE
                plProfile_BeginTiming(DSRegTransT);
#endif // MF_TEST_UPDATE
    
                idx = (*spans)[ i ];
                plSpan  *mSpan = fSpans[ idx ];
                mSpan->fLocalToWorld = l2w;
                mSpan->fWorldToLocal = w2l;

                mSpan->fWorldBounds = mSpan->fLocalBounds;
                mSpan->fWorldBounds.Transform( &l2w );

                if (fSourceSpans.size() > idx)
                {
                    /// If we have a geoSpan for this, update its transform as well,
                    /// just in case we need to use it later (<cough> SceneViewer reshade <cough>)
                    if (fSourceSpans[idx] == nullptr)
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
                    GetSpaceTree()->SetLeafFlag((int16_t)idx, plSpaceTreeNode::kDisabled, true);
                }
                else
                {
                    GetSpaceTree()->MoveLeaf((int16_t)((*spans)[i]), mSpan->fWorldBounds);
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

void plDrawableSpans::SetNativeTransform(uint32_t idx, const hsMatrix44& l2w, const hsMatrix44& w2l)
{
    if( idx == uint32_t(-1) )
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

        if (fSourceSpans.size() > idx)
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
            GetSpaceTree()->MoveLeaf((int16_t)idx, span->fWorldBounds);
        }
    }
}

//// GetLocalToWorld & GetWorldToLocal ///////////////////////////////////////

const hsMatrix44& plDrawableSpans::GetLocalToWorld( uint32_t span ) const
{
    if( span == (uint32_t)-1 )
        return fLocalToWorld;

    return fSpans[ span ]->fLocalToWorld;
}

const hsMatrix44& plDrawableSpans::GetWorldToLocal( uint32_t span ) const
{
    if( span == (uint32_t)-1 )
        return fWorldToLocal;

    return fSpans[ span ]->fWorldToLocal;
}


//// Set/GetNativeProperty ///////////////////////////////////////////////////

plDrawable& plDrawableSpans::SetNativeProperty( uint32_t index, int prop, bool on)
{
    if( index == (uint32_t)-1 )
    {
        hsAssert(false, "Invalid index to SetNativeProperty");
    }
    else
    {
        plDISpanIndex   *spans = fDIIndices[ index ];       

        if( !spans->IsMatrixOnly() )
        {
            if( on )
            {
                for (size_t i = 0; i < spans->GetCount(); i++)
                    fSpans[ (*spans)[ i ] ]->fProps |= prop;
            }
            else
            {
                for (size_t i = 0; i < spans->GetCount(); i++)
                    fSpans[ (*spans)[ i ] ]->fProps &= ~prop;
            }
            if( (prop & kPropNoDraw) ) 
            {
                for (size_t i = 0; i < spans->GetCount(); i++)
                    GetSpaceTree()->SetLeafFlag((int16_t)((*spans)[ i ]), plSpaceTreeNode::kDisabled, on);
            }
        }
    }

    return *this;
}

bool    plDrawableSpans::GetNativeProperty( uint32_t index, int prop ) const
{
    uint32_t  ret = false;


    if( index == (uint32_t)-1 )
    {
        for (plSpan* span : fSpans)
            ret |= (span->fProps & prop);
    }
    else
    {
        plDISpanIndex& spans = *fDIIndices[ index ];        

        if( !spans.IsMatrixOnly() )
        {
            for (size_t i = 0; i < spans.GetCount(); i++)
                ret |= ( fSpans[ spans[ i ] ]->fProps & prop );
        }
    }

    return ret != 0;
}

plDrawable& plDrawableSpans::SetSubType(uint32_t index, plSubDrawableType t, bool on)
{
    if( uint32_t(-1) == index )
    {
        if( on )
        {
            for (plSpan* span : fSpans)
                span->fSubType |= t;
        }
        else
        {
            for (plSpan* span : fSpans)
                span->fSubType &= ~t;
        }
    }
    else
    {
        plDISpanIndex& spans = *fDIIndices[ index ];

        if( on )
        {
            for (size_t i = 0; i < spans.GetCount(); i++)
                fSpans[ spans[i] ]->fSubType |= t;
        }
        else
        {
            for (size_t i = 0; i < spans.GetCount(); i++)
                fSpans[ spans[i] ]->fSubType &= ~t;
        }
    }
    return *this;
}

uint32_t plDrawableSpans::GetSubType(uint32_t index) const
{
    uint32_t retVal = 0;

    if( uint32_t(-1) == index )
    {
        for (plSpan* span : fSpans)
            retVal |= span->fSubType;
    }
    else
    {
        plDISpanIndex& spans = *fDIIndices[ index ];

        for (size_t i = 0; i < spans.GetCount(); i++)
            retVal |= fSpans[ spans[i] ]->fSubType;
    }
    return retVal;
}

//// IXlateSpanProps /////////////////////////////////////////////////////////
//  Never used yet--just here in case we ever need it

uint32_t  plDrawableSpans::IXlateSpanProps( uint32_t props, bool xlateToSpan )
{
    uint32_t      retProps = 0;


    if( xlateToSpan )
    {
        /// Drawable props to plSpan props
        if( props & kPropNoDraw )   retProps |= plSpan::kPropNoDraw;
        if( props & kPropSortFaces )    retProps |= plSpan::kPropFacesSortable;
    }
    else
    {
        /// plSpan props to Drawable props
        if( props & plSpan::kPropNoDraw )   retProps |= kPropNoDraw;
        if( props & plSpan::kPropFacesSortable )    retProps |= kPropSortFaces;
    }

    return retProps;
}

//// Set/GetProperty /////////////////////////////////////////////////////////
//  Sets/gets a property just like the normal Set/GetNativeProperty, but the 
//  flag taken in is from plDrawInterface, not our props flags. So we have to 
//  translate...

plDrawable& plDrawableSpans::SetProperty( uint32_t index, int diProp, bool on )
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

bool    plDrawableSpans::GetProperty( uint32_t index, int diProp ) const
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

plDrawable& plDrawableSpans::SetProperty( int prop, bool on )
{
    switch( prop )
    {
        case plDrawInterface::kDisable:
            for (plIcicle& icicle : fIcicles) {
                if (on)
                    icicle.fProps |= kPropNoDraw;
                else
                    icicle.fProps &= ~kPropNoDraw;
            }
            IQuickSpaceTree();
            return SetNativeProperty( kPropNoDraw, on );
        default:
            hsAssert( false, "Bad property passed to SetProperty" );
    }

    return *this;
}

bool        plDrawableSpans::GetProperty( int prop ) const
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

const hsBounds3Ext& plDrawableSpans::GetLocalBounds( uint32_t index ) const
{
    static hsBounds3Ext bnd;


    if( index == (uint32_t)-1 )
        return fLocalBounds;

    plDISpanIndex   *spans = fDIIndices[ index ];       

    bnd.MakeEmpty();
    if( !spans->IsMatrixOnly() )
    {
        for (size_t i = 0; i < spans->GetCount(); i++)
        {
            bnd.Union( &fSpans[ (*spans)[ i ] ]->fLocalBounds );
        }
    }

    return bnd;
}

const hsBounds3Ext& plDrawableSpans::GetWorldBounds( uint32_t index ) const
{
    static hsBounds3Ext bnd;


    if( index == (uint32_t)-1 )
        return fWorldBounds;

    plDISpanIndex   *spans = fDIIndices[ index ];       

    bnd.MakeEmpty();
    if( !spans->IsMatrixOnly() )
    {
        for (size_t i = 0; i < spans->GetCount(); i++)
        {
            bnd.Union( &fSpans[ (*spans)[ i ] ]->fWorldBounds );
        }
    }

    return bnd;
}

const hsBounds3Ext& plDrawableSpans::GetMaxWorldBounds( uint32_t index ) const
{
    return GetWorldBounds( index );
}

//// Read ////////////////////////////////////////////////////////////////////
//  We read each in the array of icicles,
//  then we read in an array of indices that we translate into
//  pointers. Note: since materials and fog environments are shared,
//  we read those keys last, so we don't have to have separate messages for
//  each.

void    plDrawableSpans::Read( hsStream* s, hsResMgr* mgr )
{
    bool                gotSkin = false;
    plGBufferGroup      *group;
    plRefMsg            *refMsg;

    
    plDrawable::Read(s, mgr);

    fProps = s->ReadLE32();
    fCriteria = s->ReadLE32();
    fRenderLevel.fLevel = s->ReadLE32();

    /// Read in the material keys
    uint32_t count = s->ReadLE32();
    fMaterials.assign(count, nullptr);
    for (uint32_t i = 0; i < count; i++)
    {
        refMsg = new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, i, kMsgMaterial );
        mgr->ReadKeyNotifyMe( s, refMsg, plRefFlags::kActiveRef );
    }

    /// Read the icicles in
    count = s->ReadLE32();
    fIcicles.resize(count);
    for (uint32_t i = 0; i < count; i++)
    {
        fIcicles[ i ].Read( s );
        if( fIcicles[ i ].fNumMatrices )
            gotSkin = true;
    }

    /// Read the patches in
    // FIXME MAJOR VERSION
    // no more patches, remove this line
    count = s->ReadLE32();

    /// Now read the index array in and use it to create a pointer table
    fSpanSourceIndices.clear();
    fSpans.clear();
    count = s->ReadLE32();
    for (uint32_t i = 0; i < count; i++)
    {
        uint32_t j = s->ReadLE32();
        switch( j & kSpanTypeMask )
        {
        case kSpanTypeIcicle:
            fSpans.emplace_back((plSpan *)&fIcicles[j & kSpanIDMask]);
            break;
        case kSpanTypeParticleSpan:
            fSpans.emplace_back((plSpan *)&fParticleSpans[j & kSpanIDMask]);
            break;
        }

        fSpanSourceIndices.emplace_back(j);

        if (fSpans.back()->fTypeMask & plSpan::kParticleSpan)
        {
            plParticleSpan  *span = (plParticleSpan *)fSpans.back();
            span->fSrcSpanIdx = (uint32_t)fSpans.size() - 1;
        }
    }

    // Rebuild bit vectors for various span types
    IBuildVectors();

    /// Now that we have our pointer array, read in the common keys (fog environs, etc)
    for (uint32_t i = 0; i < count; i++)
    {
        // Ref message for the fog environment
        refMsg = new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, i, kMsgFogEnviron );
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

    for (uint32_t i = 0; i < count; i++)
    {
        if( fSpans[i]->fProps & plSpan::kPropHasPermaLights )
        {
            uint32_t lcnt = s->ReadLE32();
            int j;
            for( j = 0; j < lcnt; j++ )
            {
                mgr->ReadKeyNotifyMe( s, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, i, kMsgPermaLight), plRefFlags::kPassiveRef);
            }
        }
        if( fSpans[i]->fProps & plSpan::kPropHasPermaProjs )
        {
            uint32_t lcnt = s->ReadLE32();
            int j;
            for( j = 0; j < lcnt; j++ )
            {
                mgr->ReadKeyNotifyMe( s, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, i, kMsgPermaProj), plRefFlags::kPassiveRef);
            }
        }
    }

    /// Read in the source spans if necessary
    count = s->ReadLE32();
    fSourceSpans.resize(count);
    for (uint32_t i = 0; i < count; i++)
    {
        fSourceSpans[i] = new plGeometrySpan;
        fSourceSpans[i]->Read(s);
        fSourceSpans[i]->fMaterial = GetMaterial(fSpans[i]->fMaterialIdx);
        fSourceSpans[i]->fFogEnviron = fSpans[i]->fFogEnvironment;
        fSourceSpans[i]->fSpanRefIndex = i;
    }

    /// Read in the matrix palette (if any)
    count = s->ReadLE32();
    fLocalToWorlds.resize(count);
    fWorldToLocals.resize(count);
    fLocalToBones.resize(count);
    fBoneToLocals.resize(count);
    for (uint32_t i = 0; i < count; i++)
    {
        fLocalToWorlds[i].Read(s);
        fWorldToLocals[i].Read(s);

        fLocalToBones[i].Read(s);
        fBoneToLocals[i].Read(s);
    }

    /// Read in the drawInterface index arrays
    count = s->ReadLE32();
    fDIIndices.assign(count, nullptr);
    for (uint32_t i = 0; i < count; i++)
    {
        fDIIndices[ i ] = new plDISpanIndex;
        
        fDIIndices[i]->fFlags = s->ReadLE32();
        uint32_t count2 = s->ReadLE32();
        fDIIndices[ i ]->SetCountAndZero( count2 );
        for (uint32_t j = 0; j < count2; j++)
            (*fDIIndices[ i ])[ j ] = s->ReadLE32();
    }

    /// Read the groups in
    count = s->ReadLE32();
    while( count-- )
    {
        group = new plGBufferGroup(0, fProps & kPropVolatile, fProps & kPropSortFaces);
        group->Read( s );

        fGroups.emplace_back(group);

    }

    if( fProps & kPropSortFaces )
    {
        for (size_t i = 0; i < fSpans.size(); i++)
            IMakeSpanSortable(i);
    }

    /// Other stuff now
    fSpaceTree = plSpaceTree::ConvertNoRef(mgr->ReadCreatable(s));
    
    fSceneNode = mgr->ReadKey(s);
    plNodeRefMsg* nRefMsg = new plNodeRefMsg(fSceneNode, plRefMsg::kOnCreate, -1, plNodeRefMsg::kDrawable); 
    mgr->AddViaNotify(GetKey(), nRefMsg, plRefFlags::kActiveRef);

    if( GetNativeProperty(plDrawable::kPropCharacter) )
    {
        fVisSet.SetBit(plVisMgr::kCharacter, true);
        for (plSpan* span : fSpans)
            span->SetVisBit(plVisMgr::kCharacter, true);
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

bool    plDrawableSpans::ITestMatForSpecularity( hsGMaterial *mat )
{
    for (size_t i = 0; i < mat->GetNumLayers(); i++)
    {
        if( mat->GetLayer( i )->GetShadeFlags() & hsGMatState::kShadeSpecular )
            return true;
    }

    return false;
}

plProfile_CreateTimer("MatrixPalleteHack", "RenderSetup", PalletteHack);
//// MsgReceive //////////////////////////////////////////////////////////////

bool plDrawableSpans::MsgReceive( plMessage* msg )
{
    plGenRefMsg     *refMsg = plGenRefMsg::ConvertNoRef( msg );
    bool            hasSpec;


    if( refMsg )
    {
        if( refMsg->fType == kMsgMaterial )
        {
            /// Material add/remove on this drawable
            if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
            {
                hsAssert(refMsg->fWhich < (int32_t)fMaterials.size(), "Invalid material index");

                fMaterials[ refMsg->fWhich ] = hsGMaterial::ConvertNoRef( refMsg->GetRef() );
                
                if( !fSettingMatIdxLock )
                {
                    // Now find all spans with this material and mark them as using or not using specular
                    hasSpec = ITestMatForSpecularity( fMaterials[ refMsg->fWhich ] );

                    for (plSpan* span : fSpans)
                    {
                        if (span != nullptr && span->fMaterialIdx == refMsg->fWhich)
                        {
                            if( hasSpec )
                                span->fProps |= plSpan::kPropMatHasSpecular;
                            else
                                span->fProps &= ~plSpan::kPropMatHasSpecular;
                        }
                    }
                }
            }
            else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
            {
                hsAssert(refMsg->fWhich < (int32_t)fMaterials.size(), "Invalid material index");

                fMaterials[refMsg->fWhich] = nullptr;
            }
            return true;
        }
        else if( refMsg->fType == kMsgFogEnviron )
        {
            /// Fog environment add/remove on this drawable
            if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
            {
                hsAssert(refMsg->fWhich < (int32_t)fSpans.size(), "Shesh, send us valid data, will ya??");

                fSpans[ refMsg->fWhich ]->fFogEnvironment = plFogEnvironment::ConvertNoRef( refMsg->GetRef() );
            }
            else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
            {
                plFogEnvironment    *fog = plFogEnvironment::ConvertNoRef( refMsg->GetRef() );

                for (plSpan* span : fSpans)
                {
                    if (span->fFogEnvironment == fog)
                    {
                        span->fFogEnvironment = nullptr;
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
                hsAssert(refMsg->fWhich < (int32_t)fSpans.size(), "Shesh, send us valid data, will ya??");
                plLightInfo* li = plLightInfo::ConvertNoRef(refMsg->GetRef());
                fSpans[refMsg->fWhich]->AddPermaLight(li, false);
            }
            else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
            {
                hsAssert(refMsg->fWhich < (int32_t)fSpans.size(), "Shesh, send us valid data, will ya??");
                plLightInfo* li = (plLightInfo*)refMsg->GetRef();
                fSpans[refMsg->fWhich]->RemovePermaLight(li, false);
            }
            return true;
        }
        else if( refMsg->fType == kMsgPermaProj )
        {
            if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
            {
                hsAssert(refMsg->fWhich < (int32_t)fSpans.size(), "Shesh, send us valid data, will ya??");
                plLightInfo* li = plLightInfo::ConvertNoRef(refMsg->GetRef());
                fSpans[refMsg->fWhich]->AddPermaLight(li, true);
            }
            else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
            {
                hsAssert(refMsg->fWhich < (int32_t)fSpans.size(), "Shesh, send us valid data, will ya??");
                plLightInfo* li = (plLightInfo*)refMsg->GetRef();
                fSpans[refMsg->fWhich]->RemovePermaLight(li, true);
            }
            return true;
        }
        else if( refMsg->fType == kMsgPermaLightDI )
        {
            if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
            {
                hsAssert(refMsg->fWhich < (int32_t)fDIIndices.size(), "Shesh, send us valid data, will ya??");
                plLightInfo* li = plLightInfo::ConvertNoRef(refMsg->GetRef());

                int diIndex = int(refMsg->fWhich);
                if( (diIndex >= 0)
                    && !fDIIndices[ diIndex ]->IsMatrixOnly() )
                {
                    for (size_t i = 0; i < fDIIndices[diIndex]->GetCount(); i++)
                    {
                        int spanIndex = (*fDIIndices[diIndex])[i];
                        fSpans[spanIndex]->AddPermaLight(li, false);
                    }
                }
            }
            else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
            {
                hsAssert(refMsg->fWhich < (int32_t)fDIIndices.size(), "Shesh, send us valid data, will ya??");
                plLightInfo* li = plLightInfo::ConvertNoRef(refMsg->GetRef());

                int diIndex = int(refMsg->fWhich);
                if( (diIndex >= 0)
                    && !fDIIndices[ diIndex ]->IsMatrixOnly() )
                {
                    for (size_t i = 0; i < fDIIndices[diIndex]->GetCount(); i++)
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
                hsAssert(refMsg->fWhich < (int32_t)fDIIndices.size(), "Shesh, send us valid data, will ya??");
                plLightInfo* li = plLightInfo::ConvertNoRef(refMsg->GetRef());

                int diIndex = int(refMsg->fWhich);
                if( (diIndex >= 0)
                    && !fDIIndices[ diIndex ]->IsMatrixOnly() )
                {
                    for (size_t i = 0; i < fDIIndices[diIndex]->GetCount(); i++)
                    {
                        int spanIndex = (*fDIIndices[diIndex])[i];
                        fSpans[spanIndex]->AddPermaLight(li, true);
                    }
                }
            }
            else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
            {
                hsAssert(refMsg->fWhich < (int32_t)fDIIndices.size(), "Shesh, send us valid data, will ya??");
                plLightInfo* li = plLightInfo::ConvertNoRef(refMsg->GetRef());

                int diIndex = int(refMsg->fWhich);
                if( (diIndex >= 0)
                    && !fDIIndices[ diIndex ]->IsMatrixOnly() )
                {
                    for (size_t i = 0; i < fDIIndices[diIndex]->GetCount(); i++)
                    {
                        int spanIndex = (*fDIIndices[diIndex])[i];
                        fSpans[spanIndex]->RemovePermaLight(li, true);
                    }
                }
            }
            return true;
        }
    }   
    else if (plDeviceRecreateMsg::ConvertNoRef(msg) != nullptr)
    {
        /// Device recreation message--just reset our flag so we refresh buffer groups
        fReadyToRender = false;
        return true;
    }
    else if( plRenderMsg::ConvertNoRef( msg ) )
    {
        plProfile_BeginLap(PalletteHack, this->GetKey()->GetUoid().GetObjectName().c_str());
    
        IUpdateMatrixPaletteBoundsHack();

        // Last thing here. We have a bit list of which of our spans are skinned and
        // thus need to be re-blended each frame. However, we don't want to blend them
        // multiple times per frame if at all possible. Since the pipeline already checks
        // our bitfield, what we *really* do is make a copy and give the pipeline our copy.
        // The pipeline will then clear out those bits as it blends them, and then we simply
        // re-set them here, since plRenderMsg is sent once before all the rendering is done :)
        fFakeBlendingSpanVector = fBlendingSpanVector;
        plProfile_EndLap(PalletteHack, this->GetKey()->GetUoid().GetObjectName().c_str());
    
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
            if (fDIIndices.size() < 2 && !(diMsg->fFlags & plDISpansMsg::kLeaveEmptyDrawable))
            {
                hsAssert(diMsg->fIndex + 1 == fDIIndices.size(), "Deleting an unknown set of indices");
                if( GetSceneNode() )
                {
                    GetSceneNode()->Release(GetKey());
                }
            }
            else /// plDrawInterface telling us to remove some spans
            {
                RemoveDISpans( (int32_t)diMsg->fIndex );
            }
        }
#ifdef HS_DEBUGGING
        else if( diMsg->fType == plDISpansMsg::kAddingSpan )
        {
            /// plDrawInterface telling us which spans it owns
            int32_t spanIndex = (int32_t)diMsg->fIndex;
            
            if( spanIndex == -1 )
                return true;
            
            for (size_t i = 0; i < fDIIndices[spanIndex]->GetCount(); i++)
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
//  Called by the sceneNode to determine if we match the criteria

bool    plDrawableSpans::DoIMatch( const plDrawableCriteria& crit )
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
//  Sets the criteria that this ice matches to. Needed since some ice will
//  be static per sceneNode while others wont, etc.

void    plDrawableSpans::SetCriteria( const plDrawableCriteria& crit )
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
//  Creates a fast, space tree for use at run-time (like in the 
//  SceneViewer). Any time a space tree is requested but there isn't
//  one available, this will be supplied. This is a full featured
//  hierarchical bounds which is pretty fast to compute. A more highly
//  optimized version may be plugged into the Optimize function at a later
//  date if this one doesn't perform enough (it does so far).

void    plDrawableSpans::IQuickSpaceTree() const
{
    // Make the space tree (hierarchical bounds).
    plSpaceTreeMaker maker;
    maker.Reset();
    for (plSpan* span : fSpans)
    {
        maker.AddLeaf(span->fWorldBounds, span->fProps & plSpan::kPropNoDraw);
    }
    plSpaceTree* tree = maker.MakeTree();
    SetSpaceTree(tree);
}

//// SetSpaceTree ////////////////////////////////////////////////////////////

void    plDrawableSpans::SetSpaceTree( plSpaceTree *st ) const
{
    delete fSpaceTree;
    fSpaceTree = st; 
    fLastVisSet.Clear();
    fLastVisNot.Clear();
}

//// GetDISpans //////////////////////////////////////////////////////////////

plDISpanIndex&  plDrawableSpans::GetDISpans( uint32_t index ) const
{
    return *fDIIndices[index];
}

//// GetVertex/IndexRef //////////////////////////////////////////////////////

hsGDeviceRef    *plDrawableSpans::GetVertexRef(size_t group, uint32_t idx)
{
    return fGroups[ group ]->GetVertexBufferRef( idx ); 
}

hsGDeviceRef    *plDrawableSpans::GetIndexRef(size_t group, uint32_t idx)
{
    return fGroups[ group ]->GetIndexBufferRef( idx );
}

void plDrawableSpans::DirtyVertexBuffer(size_t group, uint32_t idx)
{
    hsAssert(group < fGroups.size(), "Dirtying vtx buffer I don't have");
    GetBufferGroup(group)->DirtyVertexBuffer(idx);

    SetNotReadyToRender();
}

void plDrawableSpans::DirtyIndexBuffer(size_t group, uint32_t idx)
{
    hsAssert(group < fGroups.size(), "Dirtying index buffer I don't have");
    GetBufferGroup(group)->DirtyIndexBuffer(idx);

    SetNotReadyToRender();
}

hsGMaterial* plDrawableSpans::GetSubMaterial(size_t index) const
{
    return GetMaterial(fSpans[index]->fMaterialIdx);
}

// return true if span invisible before minDist and/or after maxDist
bool plDrawableSpans::GetSubVisDists(size_t index, float& minDist, float& maxDist) const
{
    return (minDist = fSpans[index]->GetMinDist()) < (maxDist = fSpans[index]->GetMaxDist());
}

//////////////////////////////////////////////////////////////////////////////
//// Runtime Dynamics ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// SortSpan ////////////////////////////////////////////////////////////////
//  Given the index of the span to sort, sorts the triangles of that span
//  based on the sorting data. Note: the span MUST be of type plIcicle.

plProfile_CreateTimer("Face Sort", "Draw", FaceSort);
plProfile_CreateCounter("Face Sort Calls", "Draw", FaceSortCalls);
plProfile_CreateCounter("Faces Sorted", "Draw", FacesSorted);

void    plDrawableSpans::SortSpan( uint32_t index, plPipeline *pipe )
{
    plProfile_Inc(FaceSortCalls);

    plProfile_BeginLap(FaceSort, "0");

    plIcicle            *span = (plIcicle *)fSpans[ index ];
    plGBufferTriangle   *list, temp;
    uint32_t              numTris;
    int                 i;
    hsMatrix44          w2cMatrix = pipe->GetWorldToCamera() * pipe->GetLocalToWorld();
    float            dist;

    ICheckSpanForSortable(index);

    static std::vector<hsRadixSort::Elem>  sortList;
    static std::vector<uint16_t>           tempTriList;
    hsRadixSort::Elem                   *elem;


    /// Get some stuff
    list = span->fSortData;
    numTris = span->fILength / 3;

    plProfile_IncCount(FacesSorted, numTris);

    hsAssert( numTris > 0, "How could we start sorting no triangles??" );

    /// Sort the triangles in "list"
    sortList.resize(numTris);
    tempTriList.resize(numTris * 3);
    elem = sortList.data();

    plProfile_EndLap(FaceSort, "0");
    plProfile_BeginLap(FaceSort, "1");

    hsVector3 vec(w2cMatrix.fMap[2][0], w2cMatrix.fMap[2][1], w2cMatrix.fMap[2][2]);
    float trans = w2cMatrix.fMap[2][3];

    // Fill out the radix sort elements with our data
    for( i = 0; i < numTris; i++ )
    {
        dist = vec.InnerProduct(list[ i ].fCenter) + trans;
        elem[ i ].fKey.fFloat = dist;
        elem[ i ].fBody = (intptr_t)&list[ i ];
        elem[ i ].fNext = elem + i + 1;
    }
    elem[i - 1].fNext = nullptr;

    plProfile_EndLap(FaceSort, "1");
    plProfile_BeginLap(FaceSort, "2");

    // Do da sort thingy
    hsRadixSort         rad;
    hsRadixSort::Elem   *sortedList = rad.Sort( elem, 0 );

    plProfile_EndLap(FaceSort, "2");
    plProfile_BeginLap(FaceSort, "3");

    uint16_t* indices = tempTriList.data();
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
                                                  numTris, tempTriList.data());

    /// Optional step in a way: copy back our new, sorted list to our original
    /// array. This lets us do less sorting next call, since the order should
    /// remain largely unchanged from the last call. If this turns out NOT to
    /// be the case, or if the sorting step takes less time than this copy,
    /// take this out!
//  memcpy( list, tempTriList.AcquireArray(), numTris * sizeof( plGBufferTriangle ) );

    /// All done! (force buffer groups to refresh during next render call)
    fReadyToRender = false;

    plProfile_EndLap(FaceSort, "4");
}

//// SortVisibleSpans ////////////////////////////////////////////////////////
//  Sorts the visible spans's triangles in one big lump, for proper back-to-
//  front display.
//  Updated 5.14.2001 mcn - Fixed so loops don't assume spans are icicles

void plDrawableSpans::SortVisibleSpans(const std::vector<int16_t>& visList, plPipeline* pipe)
{
#define MF_CHUNKSORT
#ifndef MF_CHUNKSORT

    plProfile_Inc(FaceSortCalls);

    if (visList.empty())
        return;


    static std::vector<hsRadixSort::Elem>    sortScratch;
    static std::vector<uint16_t>             triList;
    static std::vector<int32_t>              counters;
    static std::vector<uint32_t>             startIndex;
    
    plProfile_BeginTiming(FaceSort);
    if( pipe->IsDebugFlagSet( plPipeDbg::kFlagDontSortFaces ) )
    {
        /// Don't sort, just send unchanged
        for (int16_t vis : visList)
        {
            plIcicle* span = (plIcicle*)fSpans[vis];
            ICheckSpanForSortable(vis);

            /// Build a fake list of indices....
            plGBufferTriangle*      list = span->fSortData;
            triList.resize(span->fILength);
            for (uint32_t j = 0, idx = 0; j < span->fILength / 3; j++, idx += 3)
            {
                triList[ idx ] = list[ j ].fIndex1;
                triList[ idx + 1 ] = list[ j ].fIndex2;
                triList[ idx + 2 ] = list[ j ].fIndex3;
            }

            /// Now send them on to the buffer group
            fGroups[span->fGroupIdx]->StuffFromTriList(span->fIBufferIdx, span->fIStartIdx,
                                                       span->fILength / 3, triList.data());
        }
        fReadyToRender = false;
        return;
    }
    plProfile_EndTiming(FaceSort);

    plProfile_BeginLap(FaceSort, "0");

    startIndex.resize(fSpans.size());

    // First figure out the total number of tris to deal with.
    int totTris = 0;
    for (int16_t idx : visList)
    {
        plIcicle* span = (plIcicle*)fSpans[idx];

        startIndex[idx] = totTris * 3;
        if( span->fProps & plSpan::kPropReverseSort )
            startIndex[idx] += span->fILength - 3;

        totTris += span->fILength / 3;
    }
    if( totTris == 0 )
    {
        plProfile_EndLap(FaceSort, "0");
        return;
    }

    plProfile_IncCount(FacesSorted, totTris);

    sortScratch.resize(totTris);
    triList.resize(3 * totTris);

    hsRadixSort::Elem* elem = sortScratch.data();

    plProfile_EndLap(FaceSort, "0");
    plProfile_BeginLap(FaceSort, "1");

    // Pack them into the sort structure. We probably want to make the 
    // plGBufferTriangle look like plTriSortData (just add span index) 
    // which would get rid of this copy and help the data alignment.
    // Oops, I already did.
    int cnt = 0;
    for (int16_t vis : visList)
    {
        plIcicle* span = (plIcicle*)fSpans[vis];
        
        int nTris = span->fILength / 3;

        hsPoint3 viewPos = span->fWorldToLocal * pipe->GetViewPositionWorld();

        plGBufferTriangle*      list = span->fSortData;
        int j;
        for( j = 0; j < nTris; j++ )
        {
            float dist = -(viewPos - list[j].fCenter).MagnitudeSquared();
            elem[cnt].fKey.fFloat = dist;
            elem[cnt].fBody = &list[j];
            elem[cnt].fNext = elem + cnt + 1;

            cnt++;
        }
    }
    elem[cnt-1].fNext = nullptr;

    plProfile_EndLap(FaceSort, "1");
    plProfile_BeginLap(FaceSort, "2");

    // Actual sort
    hsRadixSort         rad;
    hsRadixSort::Elem* sortedList = rad.Sort( elem, 0 );

    plProfile_EndLap(FaceSort, "2");
    plProfile_BeginLap(FaceSort, "3");

    counters.assign(fSpans.size(), 0);

    while( sortedList )
    {
        plGBufferTriangle* data = (plGBufferTriangle*)sortedList->fBody;
        plIcicle* span = (plIcicle*)fSpans[data->fSpanIndex];

        uint16_t* idx = &triList[startIndex[data->fSpanIndex] + counters[data->fSpanIndex]];
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
    static int16_t    newStarts[kMaxBufferGroups][kMaxIndexBuffers];

    // Temp hack stuff so we can switch back and forth (to see if it really does any good).
    static int oldWay = false;
    static int lastOldWay = oldWay;
    if( oldWay != lastOldWay )
    {
        memset(newStarts, 0, kMaxBufferGroups * kMaxIndexBuffers * sizeof(int16_t));

        for (plSpan* baseSpan : fSpans)
        {
            plIcicle* span = (plIcicle*)baseSpan;

            span->fPackedIdx = span->fIStartIdx = newStarts[span->fGroupIdx][span->fIBufferIdx];
            newStarts[span->fGroupIdx][span->fIBufferIdx] += span->fILength;
        }
        lastOldWay = oldWay;
    }

    if( oldWay )
    {
        for (int16_t idx : visList)
        {
            plIcicle* span = (plIcicle*)fSpans[idx];

            /// Now send them on to the buffer group
            fGroups[span->fGroupIdx]->StuffFromTriList(span->fIBufferIdx, span->fIStartIdx,
                                                       span->fILength / 3, triList.data() + startIndex[idx]);
        }
    }
    else
    {
        memset(newStarts, 0, kMaxBufferGroups * kMaxIndexBuffers * sizeof(int16_t));

        uint32_t start = 0;
        for (int16_t idx : visList)
        {
            plIcicle* span = (plIcicle*)fSpans[idx];

            /// Now send them on to the buffer group
            span->fPackedIdx = span->fIStartIdx = newStarts[span->fGroupIdx][span->fIBufferIdx];
            newStarts[span->fGroupIdx][span->fIBufferIdx] += span->fILength;
            fGroups[span->fGroupIdx]->StuffFromTriList(span->fIBufferIdx, span->fIStartIdx,
                                                       span->fILength / 3, triList.data() + startIndex[idx]);
        }
    }

    plProfile_EndLap(FaceSort, "4");

    fReadyToRender = false;

    return;

#else // MF_CHUNKSORT

    plProfile_Inc(FaceSortCalls);

    if (visList.empty())
        return;

    plProfile_BeginTiming(FaceSort);

    static std::vector<hsRadixSort::Elem> sortScratch;
    static std::vector<uint16_t> triList;
    static std::vector<int32_t> counters;
    static std::vector<uint32_t> startIndex;
    
    if( pipe->IsDebugFlagSet( plPipeDbg::kFlagDontSortFaces ) )
    {
        /// Don't sort, just send unchanged
        int     j, idx;

        for (int16_t visIdx : visList)
        {
            plIcicle* span = (plIcicle*)fSpans[visIdx];
            ICheckSpanForSortable(visIdx);

            /// Build a fake list of indices....
            plGBufferTriangle*      list = span->fSortData;
            triList.resize(span->fILength);
            for( j = 0, idx = 0; j < span->fILength / 3; j++, idx += 3 )
            {
                triList[ idx ] = list[ j ].fIndex1;
                triList[ idx + 1 ] = list[ j ].fIndex2;
                triList[ idx + 2 ] = list[ j ].fIndex3;
            }

            /// Now send them on to the buffer group
            fGroups[ span->fGroupIdx ]->StuffFromTriList( span->fIBufferIdx, span->fIStartIdx, 
                                                          span->fILength / 3, triList.data() );
        }
        fReadyToRender = false;
        return;
    }

    plProfile_EndTiming(FaceSort);

    plProfile_BeginLap(FaceSort, "0");

    startIndex.resize(fSpans.size());

    // First figure out the total number of tris to deal with.
    int totTris = 0;
    for (int16_t idx : visList)
    {
        plIcicle* span = (plIcicle*)fSpans[idx];
        ICheckSpanForSortable(idx);
        
        startIndex[idx] = totTris * 3;
        if( span->fProps & plSpan::kPropReverseSort )
            startIndex[idx] += span->fILength - 3;


        totTris += span->fILength / 3;
    }
    if( totTris == 0 )
    {
        plProfile_EndLap(FaceSort, "0");
        return;
    }

    plProfile_IncCount(FacesSorted, totTris);

    sortScratch.resize(totTris);
    triList.resize(3 * totTris);

    hsRadixSort::Elem* elem = sortScratch.data();

    plProfile_EndLap(FaceSort, "0");

    size_t iVis = 0;
    while (iVis < visList.size())
    {
        plProfile_BeginLap(FaceSort, "1");

        // Pack them into the sort structure. We probably want to make the 
        // plGBufferTriangle look like plTriSortData (just add span index) 
        // which would get rid of this copy and help the data alignment.
        // Oops, I already did.
        const int kTriCutoff = 4000;
        int cnt = 0;
        while ((iVis < visList.size()) && (cnt < kTriCutoff))
        {
            plIcicle* span = (plIcicle*)fSpans[visList[iVis]];
            
            int nTris = span->fILength / 3;

            hsPoint3 viewPos = span->fWorldToLocal * pipe->GetViewPositionWorld();

            plGBufferTriangle*      list = span->fSortData;
            int j;
            for( j = 0; j < nTris; j++ )
            {
                float dist = -(viewPos - list[j].fCenter).MagnitudeSquared();
                elem[cnt].fKey.fFloat = dist;
                elem[cnt].fBody = (intptr_t)&list[j];
                elem[cnt].fNext = elem + cnt + 1;

                cnt++;
            }
            iVis++;
        }
        elem[cnt-1].fNext = nullptr;

        plProfile_EndLap(FaceSort, "1");
        plProfile_BeginLap(FaceSort, "2");

        // Actual sort
        hsRadixSort         rad;
        hsRadixSort::Elem* sortedList = rad.Sort( elem, 0 );

        plProfile_EndLap(FaceSort, "2");
        plProfile_BeginLap(FaceSort, "3");

        counters.assign(fSpans.size(), 0);

        while( sortedList )
        {
            plGBufferTriangle* data = (plGBufferTriangle*)sortedList->fBody;
            plIcicle* span = (plIcicle*)fSpans[data->fSpanIndex];

            uint16_t* idx = &triList[startIndex[data->fSpanIndex] + counters[data->fSpanIndex]];
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

    constexpr size_t kMaxBufferGroups = 20;
    constexpr size_t kMaxIndexBuffers = 20;
    static int16_t    newStarts[kMaxBufferGroups][kMaxIndexBuffers];

    hsAssert(kMaxBufferGroups >= GetNumBufferGroups(), "Bigger than we counted on num groups sort.");

    memset(newStarts, 0, kMaxBufferGroups * kMaxIndexBuffers * sizeof(int16_t));

    for (int16_t idx : visList)
    {
        plIcicle* span = (plIcicle*)fSpans[idx];

        hsAssert(kMaxIndexBuffers > span->fIBufferIdx, "Bigger than we counted on num buffers sort.");

        if( span->fProps & plSpan::kPropReverseSort )
            startIndex[idx] -= span->fILength - 3;

        /// Now send them on to the buffer group
        span->fIPackedIdx = span->fIStartIdx = newStarts[span->fGroupIdx][span->fIBufferIdx];
        newStarts[span->fGroupIdx][span->fIBufferIdx] += (int16_t)(span->fILength);
        fGroups[ span->fGroupIdx ]->StuffFromTriList( span->fIBufferIdx, span->fIStartIdx, 
                                                      span->fILength / 3, triList.data() + startIndex[idx]);
    }

    plProfile_EndLap(FaceSort, "4");

    fReadyToRender = false;

#endif // MF_CHUNKSORT
}

struct buffTriCmpBackToFront
{
    hsPoint3 fViewPos;
    buffTriCmpBackToFront(const hsPoint3& p) : fViewPos(p) {}

    bool operator()( const plGBufferTriangle& lhs, const plGBufferTriangle& rhs) const
    {
        return hsVector3(&fViewPos, &lhs.fCenter).MagnitudeSquared() > hsVector3(&fViewPos, &rhs.fCenter).MagnitudeSquared();
    }
};

struct buffTriCmpFrontToBack
{
    hsPoint3 fViewPos;
    buffTriCmpFrontToBack(const hsPoint3& p) : fViewPos(p) {}

    bool operator()( const plGBufferTriangle& lhs, const plGBufferTriangle& rhs) const
    {
        return hsVector3(&fViewPos, &lhs.fCenter).MagnitudeSquared() < hsVector3(&fViewPos, &rhs.fCenter).MagnitudeSquared();
    }
};

void plDrawableSpans::SortVisibleSpansPartial(const std::vector<int16_t>& visList, plPipeline* pipe)
{
    plProfile_Inc(FaceSortCalls);

    plProfile_BeginTiming(FaceSort);

    for (int16_t vis : visList)
    {
        hsAssert(fSpans[vis]->fTypeMask & plSpan::kIcicleSpan, "Unknown type for sorting faces");

        plIcicle* span = (plIcicle*)fSpans[vis];

        if( span->fProps & plSpan::kPartialSort )
        {
            hsAssert(fGroups[span->fGroupIdx]->AreIdxVolatile(), "Badly setup buffer group - set PartialSort too late?");

            ICheckSpanForSortable(vis);

            const hsPoint3 viewPos = span->fWorldToLocal * pipe->GetViewPositionWorld();

            const int numTris = span->fILength/3;
            std::sort(span->fSortData, span->fSortData+numTris, buffTriCmpBackToFront(viewPos));

            uint16_t* idx = fGroups[span->fGroupIdx]->GetIndexBufferData(span->fIBufferIdx) + span->fIStartIdx;
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

void plDrawableSpans::SetInitialBone(size_t i, const hsMatrix44& l2b, const hsMatrix44& b2l)
{
    fLocalToBones[ i ] = l2b;
    fBoneToLocals[ i ] = b2l;
}

//// AppendDIMatrixSpans ///////////////////////////////////////////////////////////
//  Adds a di span which will only reference into the matrix palette. That is,
//  these indices won't index into any drawable data, they will only index into
//  the same matrix list that the drawable data itself can index into. When
//  an object (through its DrawInterface) sets a transform, it doesn't know
//  whether it's setting the transform for some drawable data it owns, or just
//  setting one of the matrices which influence the drawable data someone else
//  owns. 
uint32_t plDrawableSpans::AppendDIMatrixSpans(size_t n)
{
    /// Do garbage cleanup first
    if( fNeedCleanup )
        IRemoveGarbage();

    uint32_t baseIdx = fLocalToWorlds.size();
    fLocalToWorlds.resize(baseIdx + n);
    fWorldToLocals.resize(baseIdx + n);

    fLocalToBones.resize(baseIdx + n);
    fBoneToLocals.resize(baseIdx + n);

    for (size_t i = baseIdx; i < baseIdx + n; i++)
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
uint32_t plDrawableSpans::FindBoneBaseMatrix(const std::vector<hsMatrix44>& initL2B, bool searchAll) const
{
    if (!searchAll)
    {
        // Just look amongst the added spans for a matching set
        for (plSpan* span : fSpans)
        {
            if (span && (initL2B.size() == span->fNumMatrices))
            {
                size_t j;
                for (j = 0; j < initL2B.size(); j++)
                {
                    if (initL2B[j] != fLocalToBones[j + span->fBaseMatrix])
                        break;
                }
                if (initL2B.size() == j)
                    return span->fBaseMatrix;
            }
        }
    }   
    else
    {
        // Since swappable geometry spans aren't added to the drawable until
        // runtime, a sharable bone pallete won't be found by scanning fSpans.
        // We have to do a larger search through all bone matrices.
        for (size_t i = 0; i + initL2B.size() < fLocalToBones.size(); i++)
        {
            size_t j;
            for (j = 0; j < initL2B.size(); j++)
            {
                if( initL2B[j] != fLocalToBones[j + i] )
                    break;
            }
            if (initL2B.size() == j)
                return uint32_t(i);
        }
    }
    return uint32_t(-1);
}

size_t plDrawableSpans::NewDIMatrixIndex()
{
    /// Do we have a free lookup entry?
    auto iter = std::find_if(fDIIndices.begin(), fDIIndices.end(),
                             [](plDISpanIndex* di) { return di->IsEmpty(); });

    size_t index;
    if (iter == fDIIndices.end()) {
        index = fDIIndices.size();
        fDIIndices.emplace_back(new plDISpanIndex);
    } else {
        index = iter - fDIIndices.begin();
    }

    fDIIndices[index]->Reset();
    fDIIndices[index]->fFlags = plDISpanIndex::kMatrixOnly;

    return index;
}

//// IFindDIIndices //////////////////////////////////////////////////////////
//  Finds the given DIIndices array and returns a pointer to it, or creates
//  a new one if requested

plDISpanIndex   *plDrawableSpans::IFindDIIndices( uint32_t &index )
{
    plDISpanIndex   *spanLookup;

    
    /// Do we have a free lookup entry?
    if( index == (uint32_t)-1 ) // new index
    {
        // index is a reference, so persumably the caller wants it to be updated...
        for (index = 0; index < fDIIndices.size(); index++)
        {
            if( fDIIndices[ index ]->GetCount() == 0 )
                break;
        }
        if (index == fDIIndices.size())
            fDIIndices.emplace_back(new plDISpanIndex);

        spanLookup = fDIIndices[ index ];
        spanLookup->fFlags = plDISpanIndex::kNone;
    }
    else
        spanLookup = fDIIndices[ index ]; // Just grab the one we requested

    return spanLookup;
}

//// AppendDISpans ///////////////////////////////////////////////////////////
//  Given a set of DI spans, appends them to the current storage buffers.
//  Note: AddDISpans() adds the spans to a list to be sorted, THEN put into
//  the buffers; this shoves them right in, bypassing the sorting altogether.

uint32_t plDrawableSpans::AppendDISpans(std::vector<plGeometrySpan *> &spans, uint32_t index, bool clearSpansAfterAdd,
                                        bool doNotAddToSource, bool addToFront, int lod)
{
    hsAssert(!spans.empty(), "Adding no spans? Blow me.");

    hsBounds3Ext    bounds;
    plDISpanIndex   *spanLookup;
    uint32_t          numAddedIcicle = 0;

//  hsAssert( fProps & kPropVolatile, "Trying to add spans on a non-volatile drawable" );

    /// Do garbage cleanup first
    if( fNeedCleanup )
        IRemoveGarbage();

    spanLookup = IFindDIIndices( index );

    if( GetNativeProperty(plDrawable::kPropCharacter) )
        fVisSet.SetBit(plVisMgr::kCharacter, true);

    size_t insertionPoint = 0;
    if( spans[0]->fProps & plGeometrySpan::kPartialSort )
    {
        insertionPoint = fSpans.size();
    }
    else if( !addToFront )
    {
        size_t idx;
        for (idx = 0; (idx < fSpans.size()) && !(fSpans[idx]->fProps & plSpan::kPartialSort); idx++)
        {
        }
        insertionPoint = idx;
    }
    bool inserted = insertionPoint < fSpans.size();

    /// Add the geometry spans to our list. Also add our internal span
    /// copies
    for (size_t i = 0; i < spans.size(); i++)
    {
        size_t spanIdx = fIcicles.size();
        plIcicle* icicle = &fIcicles.emplace_back();
        IConvertGeoSpanToIcicle( spans[ i ], icicle, lod );
        plSpan* span = (plSpan *)icicle;
        numAddedIcicle++;

        /// Add to our source indices
        spans[ i ]->fSpanRefIndex = insertionPoint+i;
        spanLookup->Append( spans[ i ]->fSpanRefIndex );
        fSpans.insert(fSpans.begin() + spans[i]->fSpanRefIndex, span);
        fSpanSourceIndices.insert(fSpanSourceIndices.begin() + spans[i]->fSpanRefIndex, spanIdx);

        if( GetNativeProperty(plDrawable::kPropCharacter) )
            span->SetVisBit(plVisMgr::kCharacter, true);

        /// Add the material to our list if necessary
        IAssignMatIdxToSpan( span, spans[ i ]->fMaterial );

        if( clearSpansAfterAdd )
        {
            delete spans[ i ];
            spans[i] = nullptr;
        }
        else if( !doNotAddToSource )
        {
            if (fSourceSpans.size() < fSpans.size())
                fSourceSpans.resize(fSpans.size());

            fSourceSpans[ spans[ i ]->fSpanRefIndex ] = spans[ i ];
        }

        if( fProps & kPropSortFaces )
        {
            // Should add sort data too...
            IMakeSpanSortable(fSpans.size() - 1);
        }
    }

    if (inserted)
    {
        /// Go adjusting indices in the DI index list
        for (plDISpanIndex* di : fDIIndices)
        {
            if (!di->IsMatrixOnly())
            {
                if (di == spanLookup)
                    continue;

                for (size_t j = 0; j < di->GetCount(); j++)
                {
                    if ((*di)[j] >= insertionPoint)
                    {
                        (*di)[j] += spans.size();
                        hsAssert((*di)[j] < fSpans.size(), "Span index snafu");
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

size_t plDrawableSpans::IAddAMaterial(hsGMaterial *material)
{
    // Scan for if we already have it
    for (size_t i = 0; i < fMaterials.size(); i++)
    {
        if( fMaterials[ i ] == material )
            return i;
    }

    // Scan again for a blank space to add into, if possible
    for (size_t i = 0; i < fMaterials.size(); i++)
    {
        if (fMaterials[i] == nullptr)
        {
            fMaterials[i] = material;
            return IRefMaterial(i);
        }
    }

    // Add in to the end
    size_t i = fMaterials.size();
    fMaterials.emplace_back(material);

    // Plus ref it
    return IRefMaterial(i);
}

//// IRefMaterial ////////////////////////////////////////////////////////////
//  Called if we already have a material index that we just want to use
//  again...

size_t plDrawableSpans::IRefMaterial(size_t index)
{
    hsGMaterial     *material = fMaterials[ index ];

    if (GetKey() && material != nullptr && material->GetKey() != nullptr)
        hsgResMgr::ResMgr()->AddViaNotify( material->GetKey(), new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, index, 0 ), plRefFlags::kActiveRef );

    return index;
}

//// ICheckToRemoveMaterial //////////////////////////////////////////////////
//  Runs through the span array to see if the given material index is still
//  used. If not, Release()s it and nil's it.

void plDrawableSpans::ICheckToRemoveMaterial(size_t materialIdx)
{
    auto iter = std::find_if(fSpans.cbegin(), fSpans.cend(),
                             [materialIdx](plSpan* span) {
                                 return span->fMaterialIdx == materialIdx;
                             });
    if (iter == fSpans.cend())
    {
        /// No longer used--Release() it
        hsGMaterial* mat = fMaterials[materialIdx];
        if (GetKey() && mat != nullptr && mat->GetKey() != nullptr)
            GetKey()->Release( mat->GetKey() );
        fMaterials[materialIdx] = nullptr;
    }
}

//// IConvertGeoSpanToVertexSpan /////////////////////////////////////////////
//  Helper function for the two vertex-based convert functions.

bool    plDrawableSpans::IConvertGeoSpanToVertexSpan( plGeometrySpan *geoSpan, plVertexSpan *span, int lod, plVertexSpan *instancedParent)
{
    hsBounds3Ext    bounds;
    uint32_t          vbIndex, cellIdx, cellOffset;


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
        case plGeometrySpan::kLiteMaterial:         span->fProps |= plSpan::kLiteMaterial; break;
        case plGeometrySpan::kLiteVtxPreshaded:     span->fProps |= plSpan::kLiteVtxPreshaded; break;
        case plGeometrySpan::kLiteVtxNonPreshaded:  span->fProps |= plSpan::kLiteVtxNonPreshaded; break;
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
    span->fPenBoneIdx = (uint16_t)(geoSpan->fPenBoneIdx);
    span->fMinDist = geoSpan->fMinDist;
    span->fMaxDist = geoSpan->fMaxDist;

    bounds = geoSpan->fLocalBounds;
    span->fLocalBounds = bounds;
    bounds.Transform( &span->fLocalToWorld );
    span->fWorldBounds = bounds;
    fWorldBounds.Union( &bounds );
    span->fFogEnvironment = geoSpan->fFogEnviron;

    bool vertsVol = false;
    if( fProps & kPropVolatile )
        vertsVol = true;

    bool idxVol = false;
    if( fProps & kPropSortFaces )
        idxVol = true;
    if( geoSpan->fProps & plGeometrySpan::kPartialSort )
        idxVol = true;

    // Are we instanced?
    if (instancedParent != nullptr /*&& !( fProps & kPropVolatile )*/)
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
            uint32_t groupIdx = instancedParent->fGroupIdx;
            fGroups[ groupIdx ]->AppendToColorStorage( geoSpan, &vbIndex, &cellIdx, &cellOffset, instancedParent->fCellIdx );

            span->fGroupIdx = groupIdx;
            span->fVBufferIdx = instancedParent->fVBufferIdx;
            // Do a new vStart here, since it's really the colorStart, but oh well
            span->fVStartIdx = fGroups[ groupIdx ]->GetVertStartFromCell( vbIndex, cellIdx, cellOffset );
        }
        else
        {
            /// WE'RE the parent?? This means we're the first to get converted
            uint32_t groupIdx = IFindBufferGroup(geoSpan->fFormat, geoSpan->fNumVerts, lod, vertsVol, idxVol);
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
        uint32_t groupIdx = IFindBufferGroup(geoSpan->fFormat, geoSpan->fNumVerts, lod, vertsVol, idxVol);
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

bool    plDrawableSpans::IConvertGeoSpanToIcicle(plGeometrySpan *geoSpan, plIcicle *icicle, int lod, plIcicle *instancedParent)
{
    uint32_t      ibIndex, ibStart;


    IConvertGeoSpanToVertexSpan(geoSpan, icicle, lod, instancedParent);

/*
    Disabling this 8.16.2001. Works great until you have to SORT the faces, then things blow up. We could
    do this only when we won't sort faces, but it's easier right now to just never do this ever.

    if (instancedParent != nullptr && instancedParent != icicle && (icicle->fProps & plSpan::kPropRunTimeLight))
    {
        /// WOW! We can actually share the *exact* same data, since we don't do any preshading. Coooooool!
        icicle->fIBufferIdx = instancedParent->fIBufferIdx;
        icicle->fIStartIdx = instancedParent->fIStartIdx;
        icicle->fILength = instancedParent->fILength;
        icicle->fSortData = nullptr;
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
        icicle->fSortData = nullptr;
    }

    return true;
}

//// RemoveDIMatrixSpans
//  Nuke out a matrix only DI index set someone doesn't need anymore (cuz they're dead).
//  Since there's no data associated, there's not much to do, except mark the slot as
//  free and that some cleanup is in order some time. 
void    plDrawableSpans::RemoveDIMatrixSpans( uint32_t index )
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
//  Given a drawInterface index (i.e. the index into fDIIndices), deletes
//  the spans associated with that entry and clears the entry. Also packs
//  the spans, deletes the verts/indices associated with the deleted spans
//  and packs the vertex/index buffers as well.

void    plDrawableSpans::RemoveDISpans( uint32_t index ) 
{
    plDISpanIndex       *spanIndices = fDIIndices[ index ];

    if( spanIndices->IsMatrixOnly() )
    {
        RemoveDIMatrixSpans( index );
    }

    uint32_t idxRemoving;


// #define MF_RENDDEP
#ifdef MF_RENDDEP
    std::vector<size_t> spanInverseTable;
    spanInverseTable.resize(fSpans.size());
    for (size_t i = 0; i < fSpans.size(); i++)
        spanInverseTable[i] = i;
#endif // MF_RENDDEP

//  hsAssert( fProps & kPropVolatile, "Trying to remove spans on a non-volatile drawable" );
    hsAssert( spanIndices->GetCount() > 0, "If there are no DI spans, why were we called?" );

    /// Delete the actual spans themselves
    for (size_t i = 0; i < spanIndices->GetCount(); i++)
    {
        /// If this is the last use of this material, Release() it
        hsSsize_t materialIdx = fSpans[(*spanIndices)[i]]->fMaterialIdx;

        /// Remove the source index and the object itself
        if (!fSourceSpans.empty() && !(fSpans[(*spanIndices)[i]]->fTypeMask & plSpan::kParticleSpan))
        {
            delete fSourceSpans[ (*spanIndices)[i] ];
            fSourceSpans.erase(fSourceSpans.begin() + (*spanIndices)[i]);
            for (size_t j = (*spanIndices)[i]; j < fSourceSpans.size(); j++)
                fSourceSpans[ j ]->fSpanRefIndex = j;
        }
        fSpans[ (*spanIndices)[ i ] ]->Destroy();
        fSpans.erase(fSpans.begin() + (*spanIndices)[i]);
        idxRemoving = fSpanSourceIndices[ (*spanIndices)[ i ] ];
        fSpanSourceIndices.erase(fSpanSourceIndices.begin() + (*spanIndices)[i]);

        switch( idxRemoving & kSpanTypeMask )
        {
        case kSpanTypeIcicle:
            fIcicles.erase(fIcicles.begin() + (idxRemoving & kSpanIDMask));
            break;
        case kSpanTypeParticleSpan:
            fParticleSpans.erase(fParticleSpans.begin() + (idxRemoving & kSpanIDMask));
            break;
        }

        // Do this AFTER the span array has been updated
        ICheckToRemoveMaterial( materialIdx );

        /// Go adjusting the source indices
        for (uint32_t& ssidx : fSpanSourceIndices)
        {
            if (((ssidx ^ idxRemoving) & kSpanTypeMask) == 0)
            {
                /// Same type
                uint32_t k = ssidx & kSpanIDMask;
                if( k > ( idxRemoving & kSpanIDMask ) )
                {
                    k--;
                    ssidx &= kSpanTypeMask;
                    ssidx |= k;
                }
            }
        }

#ifndef MF_RENDDEP
        /// Go adjusting indices in the DI index list
        for (plDISpanIndex* di : fDIIndices)
        {
            if (!di->IsMatrixOnly())
            {
                for (size_t k = 0; k < di->GetCount(); k++)
                {
                    if ((*di)[k] > (*spanIndices)[i])
                        (*di)[k]--;
                }
            }
        }
#else // MF_RENDDEP
        spanInverseTable[(*spanIndices)[i]] = -1;
        for (size_t j = (*spanIndices)[i]; j < fSpans.size(); j++)
            spanInverseTable[j]--;
#endif // MF_RENDDEP

    }
#ifdef MF_RENDDEP
    for (int j = 0; j < fDIIndices.GetCount(); j++)
    {
        if( !fDIIndices[j]->IsMatrixOnly() )
        {
            for (size_t k = 0; k < fDIIndices[j]->GetCount(); k++)
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

void    plDrawableSpans::IRebuildSpanArray()
{
    plIcicle    *icicle = nullptr;

    for (size_t j = 0; j < fSpans.size(); j++)
    {
        switch( fSpanSourceIndices[ j ] & kSpanTypeMask )
        {
            case kSpanTypeIcicle: 
                icicle = &fIcicles[ fSpanSourceIndices[ j ] & kSpanIDMask ];
                fSpans[ j ] = (plSpan *)icicle; 
                if (icicle->fSortData != nullptr)
                {
                    // GOTTA RESET THE SPAN INDICES TOO!!!
                    for (uint32_t i = 0; i < icicle->fILength / 3; i++)
                        icicle->fSortData[ i ].fSpanIndex = (uint16_t)j;
                }
                break;
            case kSpanTypeParticleSpan: 
                fSpans[ j ] = (plSpan *)&fParticleSpans[ fSpanSourceIndices[ j ] & kSpanIDMask ]; 
                break;
        }
    }

    // Redid the span array, so cause the space tree to rebuild to match
    SetSpaceTree(nullptr);

    // Rebuild bit vectors for various span types
    IBuildVectors();
}


//// ICleanupMatrices
//  Find all the matrix slots in the palette that aren't being used, collapse
//  them out and adjust the indices into the matrix palette (the MatrixOnly DIIndices)
void    plDrawableSpans::ICleanupMatrices()
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

    for( j = 0; j < fLocalToWorlds.size(); j++ )
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
            for( i = j+1; i < fLocalToWorlds.size(); i++ )
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
//  Cleans out all the unused spans. Oh, joy.

void    plDrawableSpans::IRemoveGarbage()
{
    // Oh joy, vector<bool>s and vectors of vector<bool>s...
    std::vector<std::vector<bool>>              usedFlags;
    std::vector<std::vector<std::vector<bool>>> usedIdxFlags;
    plGBufferGroup* group;

    ICleanupMatrices();

    /// Getting rid of verts
    usedFlags.resize(fGroups.size());
    usedIdxFlags.resize(fGroups.size());
    for (size_t i = 0; i < fGroups.size(); i++)
    {
        hsAssert( fGroups[ i ]->GetNumVertexBuffers() == 1, "Cannot clean garbage on a non-volatile buffer group!" );
        usedFlags[i].resize(fGroups[i]->GetVertBufferCount(0));

        usedIdxFlags[i].resize(fGroups[i]->GetNumIndexBuffers());
        for (size_t j = 0; j < fGroups[i]->GetNumIndexBuffers(); j++)
        {
            usedIdxFlags[i][j].resize(fGroups[i]->GetIndexBufferCount(j));
        }
    }

    for (const plIcicle& icicle : fIcicles)
    {
        // Set the flags so we know these verts are used
        uint32_t groupIdx = icicle.fGroupIdx;

        for (uint32_t j = icicle.fCellOffset; j < icicle.fCellOffset + icicle.fVLength; j++)
            usedFlags[groupIdx][j] = true;

        for (uint32_t j = icicle.fIStartIdx; j < icicle.fIStartIdx + icicle.fILength; j++)
            usedIdxFlags[groupIdx][icicle.fIBufferIdx][j] = true;
    }
    for (const plParticleSpan& span : fParticleSpans)
    {
        // Set the flags so we know these verts are used
        uint32_t groupIdx = span.fGroupIdx;

        for (uint32_t j = span.fCellOffset; j < span.fCellOffset + span.fVLength; j++)
            usedFlags[groupIdx][j] = true;

        for (uint32_t j = span.fIStartIdx; j < span.fIStartIdx + span.fILength; j++)
            usedIdxFlags[groupIdx][span.fIBufferIdx][j] = true;
    }

    /// Now loop through and delete any unused
    for (size_t groupIdx = 0; groupIdx < fGroups.size(); groupIdx++)
    {
        group = fGroups[ groupIdx ];
        std::vector<bool>& uFlags = usedFlags[groupIdx];

        // Find groups of verts to delete!
        size_t count = uFlags.size();
        for (size_t i = 0; i < count; )
        {
            // Skip through used verts
            for( ; i < count && uFlags[ i ] == true; i++ );

            // Find span of unused verts
            size_t j;
            for( j = i; j < count && uFlags[ j ] == false; j++ );

            if( j > i )
            {
                // Delete this span of vertices
                group->DeleteVertsFromStorage( 0, i, j - i );

                // Adjust indices in this group
                for (uint32_t ibIdx = 0; ibIdx < group->GetNumIndexBuffers(); ibIdx++)
                    group->AdjustIndicesInStorage( ibIdx, i, -(int16_t)( j - i ) );

                // Adjust spans that use this vertex buffer
                for (plIcicle& icicle : fIcicles)
                {
                    if (icicle.fGroupIdx == groupIdx && icicle.fVBufferIdx == 0
                            && icicle.fCellOffset >= i)
                        icicle.fCellOffset -= j - i;

                    // Adjust sorting data, if necessary
                    if (icicle.fSortData != nullptr)
                        IAdjustSortData(icicle.fSortData, icicle.fILength / 3, i, -(int16_t)(j - i));
                }
                for (plParticleSpan& span : fParticleSpans)
                {
                    if (span.fGroupIdx == groupIdx && span.fVBufferIdx == 0
                            && span.fCellOffset >= i)
                        span.fCellOffset -= j - i;

                    // Adjust sorting data, if necessary
                    if (span.fSortData != nullptr)
                        IAdjustSortData(span.fSortData, span.fILength / 3, i, -(int16_t)(j - i));
                }

                // Move the flags too, lest we start deleting the wrong vertices
                size_t offset = j - i;
                count -= offset;
                for (; i < count; i++)
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
            for (uint32_t ibIdx = 0; ibIdx < group->GetNumIndexBuffers(); ibIdx++)
            {
                // For this index buffer, find the total length we want
                uint32_t i = 0;
                for (const plIcicle& icicle : fIcicles)
                {
                    if (icicle.fGroupIdx == groupIdx && icicle.fIBufferIdx == ibIdx)
                        i += icicle.fILength;
                }
                for (const plParticleSpan& span : fParticleSpans)
                {
                    if (span.fGroupIdx == groupIdx && span.fIBufferIdx == ibIdx)
                        i += span.fILength;
                }

                /// i is the total length
                group->DeleteIndicesFromStorage( ibIdx, i, group->GetIndexBufferCount( ibIdx ) - i );
            }
        }
        else
        {
            /// Non-sorting, we have to figure out exactly which indices we aren't using

            for (uint32_t ibIdx = 0; ibIdx < group->GetNumIndexBuffers(); ibIdx++)
            {
                std::vector<bool>& uiFlags = usedIdxFlags[groupIdx][ibIdx];

                // Find groups of indices to delete!
                count = uiFlags.size();
                for (size_t i = 0; i < count; )
                {
                    // Skip through used verts
                    for( ; i < count && uiFlags[ i ] == true; i++ );

                    // Find span of unused verts
                    size_t j;
                    for( j = i; j < count && uiFlags[ j ] == false; j++ );

                    if( j > i )
                    {
                        // Delete this span of indices
                        group->DeleteIndicesFromStorage( ibIdx, i, j - i );

                        // Adjust spans that use this index buffer (only icicles use them)
                        for (plIcicle& icicle : fIcicles)
                        {
                            if (icicle.fGroupIdx == groupIdx && icicle.fIBufferIdx == ibIdx &&
                                icicle.fIStartIdx >= i)
                            {
                                icicle.fIPackedIdx = (icicle.fIStartIdx -= j - i);
                            }
                        }
                        // and particle spans :)
                        for (plParticleSpan& span : fParticleSpans)
                        {
                            if (span.fGroupIdx == groupIdx && span.fIBufferIdx == ibIdx &&
                                span.fIStartIdx >= i)
                            {
                                span.fIPackedIdx = (span.fIStartIdx -= j - i);
                            }
                        }

                        // Move the flags too, lest we start deleting the wrong vertices
                        size_t offset = j - i;
                        count -= offset;
                        for (; i < count; i++)
                            uiFlags[ i ] = uiFlags[ i + offset ];
                    }

                    // Keep going
                    i = j;
                }
            }
        }
    }

    /// Refresh all vStartIdx entries
    for (plSpan* span : fSpans)
    {
        if (span->fTypeMask & plSpan::kVertexSpan)
        {
            plVertexSpan    *vtx = (plVertexSpan *)span;
            vtx->fVStartIdx = fGroups[ vtx->fGroupIdx ]->GetVertStartFromCell( vtx->fVBufferIdx, vtx->fCellIdx, vtx->fCellOffset );
        }
    }

    /// This will let us query the buffer group as to whether its ready to render or not.
    /// If not, we'll force it then to reload from its storage
    fReadyToRender = false; 

    // Whew!
    fNeedCleanup = false;
}

//// IAdjustSortData /////////////////////////////////////////////////////////
//  Adjusts the indices in the given sort data, using the delta and threshhold
//  given.

void    plDrawableSpans::IAdjustSortData( plGBufferTriangle *triList, uint32_t count, uint32_t threshhold, int32_t delta )
{
    uint32_t      i;


    for( i = 0; i < count; i++ )
    {
        if( triList[ i ].fIndex1 >= threshhold )
            triList[ i ].fIndex1 = (uint16_t)( triList[ i ].fIndex1 + delta );
        if( triList[ i ].fIndex2 >= threshhold )
            triList[ i ].fIndex2 = (uint16_t)( triList[ i ].fIndex2 + delta );
        if( triList[ i ].fIndex3 >= threshhold )
            triList[ i ].fIndex3 = (uint16_t)( triList[ i ].fIndex3 + delta );
    }
}

//// IMakeSpanSortable ////////////////////////////////////////////////////////
//  Given an index of a span, flags it sortable and creates the sorting data
//  for that span.

void plDrawableSpans::IMakeSpanSortable(size_t index)
{
    plIcicle            *span = (plIcicle *)fSpans[ index ];
    plGBufferTriangle   *list;


    if( span->fProps & plSpan::kPropFacesSortable )
        return;

    /// Create data for it
    list = fGroups[ span->fGroupIdx ]->ConvertToTriList( (int16_t)index, span->fIBufferIdx, span->fVBufferIdx, span->fCellIdx, 
                                                         span->fIStartIdx, span->fILength / 3 );
    if (list == nullptr)
        return;

    span->fSortData = list;

    /// Mark as sortable
    span->fProps |= plSpan::kPropFacesSortable;
}

void plDrawableSpans::UnPackCluster(plClusterGroup* cluster)
{
    const uint32_t vertsPerInst = cluster->GetTemplate()->NumVerts();
    const uint32_t idxPerInst = cluster->GetTemplate()->NumIndices();

    const size_t numClust = cluster->GetNumClusters();

    fVisSet |= cluster->GetVisSet();
    fVisNot |= cluster->GetVisNot();

    fIcicles.resize(numClust);
    fSpans.resize(numClust);
    for (size_t iSpan = 0; iSpan < numClust; iSpan++)
        fSpans[iSpan] = &fIcicles[iSpan];

    uint32_t vtxFormat =
        cluster->GetTemplate()->NumUVWs()
        | cluster->GetTemplate()->NumWeights() << 4;
    if( cluster->GetTemplate()->NumWgtIdx() )
        vtxFormat |= plGBufferGroup::kSkinIndices;

    const std::vector<plLightInfo*>& lights = cluster->GetLights();

    for (size_t iStart = 0; iStart < cluster->GetNumClusters(); )
    {
        int numVerts = 0;
        int numIdx = 0;
        size_t iEnd;
        for (iEnd = iStart; iEnd < cluster->GetNumClusters(); iEnd++)
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
        size_t grpIdx = IFindBufferGroup((uint8_t)vtxFormat, numVerts, 0, false, false);
        
        uint32_t vbufferIdx;
        uint32_t cellIdx;
        uint32_t cellOffset;
        fGroups[grpIdx]->ReserveVertStorage(numVerts, &vbufferIdx, &cellIdx, &cellOffset, plGBufferGroup::kReserveInterleaved | plGBufferGroup::kReserveIsolate);
        hsAssert(!cellOffset, "This should be our own personal group");
        hsAssert(fGroups[grpIdx]->GetVertexSize() == cluster->GetTemplate()->Stride(), "Mismatch on src and dst sizes");
        
        uint32_t ibufferIdx;
        uint32_t istartIdx;
        fGroups[grpIdx]->ReserveIndexStorage(numIdx, &ibufferIdx, &istartIdx);
        uint32_t iOffset = 0;

        uint8_t* vData = fGroups[grpIdx]->GetVertBufferData(vbufferIdx);
        uint16_t* iData = fGroups[grpIdx]->GetIndexBufferData(ibufferIdx);
        uint8_t* pvData = vData;
        uint16_t* piData = iData;
        size_t iSpan = 0;
        for (size_t i = iStart; i < iEnd; i++)
        {
            hsBounds3Ext bnd;
            cluster->GetCluster(i)->UnPack(pvData, piData, cellOffset, bnd);
            fIcicles[iSpan].fLocalBounds = bnd;
            fIcicles[iSpan].fWorldBounds = bnd;

            fIcicles[iSpan].fTypeMask = plSpan::kSpan | plSpan::kVertexSpan | plSpan::kIcicleSpan;
            // STUB - need to set whether strictly runtime lit or preshaded based on cluster.
//          fIcicles[iSpan].fProps = plSpan::kLiteMaterial;
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
            fIcicles[iSpan].fFogEnvironment = nullptr;
            fIcicles[iSpan].fMaxDist = cluster->GetLOD().fMaxDist;
            fIcicles[iSpan].fMinDist = cluster->GetLOD().fMinDist;
            fIcicles[iSpan].fWaterHeight = 0;
            fIcicles[iSpan].fVisSet = cluster->GetVisSet();
            fIcicles[iSpan].fVisNot = cluster->GetVisNot();

            if (!lights.empty())
            {
                for (plLightInfo* light : lights)
                    fIcicles[iSpan].AddPermaLight(light, light->GetProjection() != nullptr);
            }

            fIcicles[iSpan].fGroupIdx = grpIdx;
            fIcicles[iSpan].fVBufferIdx = vbufferIdx;
            fIcicles[iSpan].fCellIdx = cellIdx;
            fIcicles[iSpan].fCellOffset = cellOffset;
            fIcicles[iSpan].fVStartIdx = cellOffset;
            const uint32_t numVerts = cluster->GetCluster(i)->NumInsts() * vertsPerInst;
            fIcicles[iSpan].fVLength = numVerts;
            cellOffset += numVerts;

            fIcicles[iSpan].fIBufferIdx = ibufferIdx;
            fIcicles[iSpan].fIPackedIdx = fIcicles[iSpan].fIStartIdx = iOffset;
            const uint32_t iLength = cluster->GetCluster(i)->NumInsts() * cluster->GetTemplate()->NumIndices();
            fIcicles[iSpan].fILength = iLength;
            iOffset += iLength;

            iSpan++;

            const uint32_t vSize = cluster->GetCluster(i)->NumInsts() * cluster->GetTemplate()->VertSize();
            pvData += vSize;
            piData += iLength;
        }

        iStart = iEnd;
    }
    fMaterials = {nullptr};
    plGenRefMsg* refMsg = new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kMsgMaterial);
    hsgResMgr::ResMgr()->SendRef(cluster->GetMaterial()->GetKey(), refMsg, plRefFlags::kActiveRef);

    fRenderLevel = cluster->GetRenderLevel();
    GetSpaceTree();
}

//// IFindBufferGroup ////////////////////////////////////////////////////////

size_t plDrawableSpans::IFindBufferGroup(uint8_t vtxFormat, uint32_t numVertsNeeded, int lod, bool vertVolatile, bool idxVolatile)
{
    // Scan through the buffer groups, looking for a good format. If there isn't
    // one, add it
    if( fProps & kPropVolatile )
        vertVolatile = true;
    if( fProps & kPropSortFaces )
        idxVolatile = true;

    for (size_t i = 0; i < fGroups.size(); i++)
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
    fGroups.emplace_back(new plGBufferGroup(vtxFormat, vertVolatile, idxVolatile, lod));
    return fGroups.size() - 1;
}

//// GetParticleSpanVector ///////////////////////////////////////////////////
//  Get a bitVector of the spans that are particle spans

hsBitVector const   &plDrawableSpans::GetParticleSpanVector() const
{
    return fParticleSpanVector;
}

//// GetBlendingSpanVector ///////////////////////////////////////////////////
//  Get a bitVector of the spans that are blending (i.e. numMatrices > 0)

hsBitVector const   &plDrawableSpans::GetBlendingSpanVector() const
{
    /// See the plRenderMsg handler for why we do this
    return fFakeBlendingSpanVector;
}

//// SetBlendingSpanVectorBit ////////////////////////////////////////////////
//  Could just make GetBlendingSpanVector() non-const, but this way it's harder
//  for the end user, thus I'm hoping to discourage them from doing it too much.

void    plDrawableSpans::SetBlendingSpanVectorBit( uint32_t bitNumber, bool on )
{
    fFakeBlendingSpanVector.SetBit( bitNumber, on );
}

//// IBuildVectors ///////////////////////////////////////////////////////////

void    plDrawableSpans::IBuildVectors()
{
    bool    needRenderMsg = false;


    fParticleSpanVector.Clear();
    fBlendingSpanVector.Clear();
    for (size_t i = 0; i < fSpans.size(); i++)
    {
        if( fSpans[ i ]->fTypeMask & plSpan::kParticleSpan )
            fParticleSpanVector.SetBit( i );
        // BoneUpdate
        if( ( fSpans[ i ]->fTypeMask & plSpan::kVertexSpan ) && (fSpans[ i ]->fNumMatrices > 2) )
//      if( ( fSpans[ i ]->fTypeMask & plSpan::kVertexSpan ) && (fSpans[ i ]->fNumMatrices > 1) )
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

uint32_t      plDrawableSpans::CreateParticleSystem( uint32_t maxNumSpans, uint32_t maxNumParticles, hsGMaterial *material )
{
    uint32_t          i, numVerts, numIndices;
    plParticleSet   *set;
    plDISpanIndex   *spanLookup;


    // Make a shiny new set
    set = new plParticleSet;
    set->fRefCount = 0;

    /// Fill out info
    numVerts = maxNumParticles * 4;     // 4 verts per particle
    numIndices = maxNumParticles * 6;   // 6 indices per particle

    if (material != nullptr && material->GetLayer(0) != nullptr && material->GetLayer(0)->GetTexture() != nullptr)
        set->fFormat = plGeometrySpan::UVCountToFormat( 1 );
    else
        set->fFormat = plGeometrySpan::UVCountToFormat( 0 );

    // Reserve space
    set->fGroupIdx = IFindBufferGroup( set->fFormat, numVerts, 0, true, false );        // Force them to volatile even if the drawable isn't
    fGroups[ set->fGroupIdx ]->ReserveVertStorage( numVerts, &set->fVBufferIdx, &set->fCellIdx, &set->fCellOffset, plGBufferGroup::kReserveInterleaved | plGBufferGroup::kReserveIsolate );
    fGroups[ set->fGroupIdx ]->ReserveIndexStorage( numIndices, &set->fIBufferIdx, &set->fIStartIdx );

    set->fVStartIdx = fGroups[ set->fGroupIdx ]->GetVertStartFromCell( set->fVBufferIdx, set->fCellIdx, set->fCellOffset );
    set->fVLength = numVerts;
    set->fILength = numIndices;

    uint32_t vIdx = set->fVStartIdx;
    uint16_t* idx = fGroups[set->fGroupIdx]->GetIndexBufferData(set->fIBufferIdx) + set->fIStartIdx;
    for( i = 0; i < maxNumParticles; i++ )
    {
        *idx++ = (uint16_t)vIdx;
        *idx++ = (uint16_t)(vIdx + 1);
        *idx++ = (uint16_t)(vIdx + 2);

        *idx++ = (uint16_t)vIdx;
        *idx++ = (uint16_t)(vIdx + 2);
        *idx++ = (uint16_t)(vIdx + 3);

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

void    plDrawableSpans::IAssignMatIdxToSpan( plSpan *span, hsGMaterial *mtl )
{
    fSettingMatIdxLock = true;

    if (mtl != nullptr)
        span->fMaterialIdx = IAddAMaterial( mtl );

    if (fMaterials[span->fMaterialIdx] != nullptr)
    {
        if( ITestMatForSpecularity( fMaterials[ span->fMaterialIdx ] ) )
            span->fProps |= plSpan::kPropMatHasSpecular;
        else
            span->fProps &= ~plSpan::kPropMatHasSpecular;
    }

    fSettingMatIdxLock = false;
}

//// ICreateParticleIcicle ///////////////////////////////////////////////////
//  Creates a shiny new plIcicle for use with particle sets.

plParticleSpan  *plDrawableSpans::ICreateParticleIcicle( hsGMaterial *material, plParticleSet *set )
{
    hsMatrix44      ident;
    plDISpanIndex   *spanLookup;


    /// Ahh, an icicle span
    size_t spanIdx = fParticleSpans.size();
    plParticleSpan *icicle = &fParticleSpans.emplace_back();

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

    hsPoint3 zero(0, 0, 0);
    icicle->fLocalBounds.Union( &zero );
    icicle->fWorldBounds.Union( &zero );

    icicle->fGroupIdx = set->fGroupIdx;
    icicle->fVBufferIdx = set->fVBufferIdx;
    icicle->fCellIdx = set->fCellIdx;
    icicle->fCellOffset = set->fCellOffset;
    icicle->fVStartIdx = set->fVStartIdx;
    icicle->fVLength = 0;

    icicle->fIBufferIdx = set->fIBufferIdx;
    icicle->fIPackedIdx = icicle->fIStartIdx = set->fIStartIdx;
    icicle->fILength = 0;
    icicle->fFogEnvironment = nullptr;
    icicle->fSortData = nullptr;

    icicle->fSrcSpanIdx = (uint32_t)fSpans.size();

    spanLookup = IFindDIIndices( set->fDIEntry );
    spanLookup->Append(fSpans.size());
    fSpans.emplace_back((plSpan *)icicle);
    fSpanSourceIndices.emplace_back(spanIdx | kSpanTypeParticleSpan);

    /// Set our pointer to the set and inc it's refCount
    icicle->fParentSet = set;
    set->fRefCount++;

    /// Flag that we need garbage cleanup now
    fNeedCleanup = true;
    fReadyToRender = false;
    
    // Cause us to rebuild the space tree when needed
    SetSpaceTree(nullptr);

    return icicle;
}

//// ResetParticleSystem /////////////////////////////////////////////////////

void        plDrawableSpans::ResetParticleSystem( uint32_t setIndex )
{
    plDISpanIndex   *indices = IFindDIIndices( setIndex );
    plParticleSet   *set;
    plParticleSpan  *span;

    for (size_t i = 0; i < indices->GetCount(); i++)
    {
        span = (plParticleSpan *)fSpans[ (*indices)[ i ] ];
        span->fVStartIdx = 0;
        span->fCellIdx = 0;
        span->fCellOffset = 0;
        span->fVLength = 0;
        span->fIPackedIdx = span->fIStartIdx = 0;
        span->fILength = 0;
        span->fSource = nullptr;
        span->fProps |= plSpan::kPropNoDraw;
        GetSpaceTree()->SetLeafFlag( (int16_t)(span->fSrcSpanIdx), plSpaceTreeNode::kDisabled );

        set = span->fParentSet;         // To use at the end of the loop
    }

    set->fNextIStartIdx = set->fIStartIdx;
    set->fNextVStartIdx = set->fVStartIdx;
    set->fNextCellOffset = set->fCellOffset;

    fGroups[set->fGroupIdx]->SetVertBufferEnd(set->fVBufferIdx, set->fNextVStartIdx);
    if( fProps & kPropSortFaces )
        fGroups[set->fGroupIdx]->SetIndexBufferEnd(set->fIBufferIdx, set->fNextIStartIdx);
}       


//// AssignEmitterToParticleSystem ///////////////////////////////////////////

void    plDrawableSpans::AssignEmitterToParticleSystem( uint32_t setIndex, plParticleEmitter *emitter )
{
    plDISpanIndex   *indices = IFindDIIndices( setIndex );
    plParticleSet   *set;
    plParticleSpan  *icicle;
    uint32_t          i, index, numParticles = emitter->GetParticleCount();
    plParticleCore  *array = emitter->GetParticleArray();
    hsMatrix44      ident;

    plGBufferTriangle   *sortArray;

    
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
    GetSpaceTree()->ClearLeafFlag( (int16_t)(icicle->fSrcSpanIdx), plSpaceTreeNode::kDisabled );
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
        if (icicle->fSortData == nullptr || icicle->fSortCount < (numParticles << 1))
        {
            delete [] icicle->fSortData;
            icicle->fSortData = sortArray = new plGBufferTriangle[ numParticles << 1 ];
            icicle->fSortCount = numParticles << 1;
        }
        else
            sortArray = icicle->fSortData;

        // Find our bounds and fill out sorting data
        for( i = 0, index = icicle->fVStartIdx; i < numParticles; i++ )
        {
            sortArray->fCenter = array[ i ].fPos;
            sortArray->fIndex1 = (uint16_t)index;
            sortArray->fIndex2 = (uint16_t)(index + 1);
            sortArray->fIndex3 = (uint16_t)(index + 2);
            sortArray->fSpanIndex = (uint16_t)(icicle->fSrcSpanIdx);
            sortArray++;

            sortArray->fCenter = array[ i ].fPos;
            sortArray->fIndex1 = (uint16_t)index;
            sortArray->fIndex2 = (uint16_t)(index + 2);
            sortArray->fIndex3 = (uint16_t)(index + 3);
            sortArray->fSpanIndex = (uint16_t)(icicle->fSrcSpanIdx);
            sortArray++;

            index += 4;
        }
    }

    icicle->fLocalBounds = emitter->GetBoundingBox();
    icicle->fWorldBounds = icicle->fLocalBounds;

    GetSpaceTree()->MoveLeaf( (int16_t)(icicle->fSrcSpanIdx), icicle->fWorldBounds );
    // Done for now, we'll fill on the pipeline side

    return;
}

//////////////////////////////////////////////////////////////////////////////
//// Dynamic Functions for SceneViewer ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// RefreshDISpans //////////////////////////////////////////////////////////
//  Given a set of plGeometrySpans, replaces the data in the given DI span set
//  with the vertices in the plGeometrySpans. Does NOT replace the indices,
//  and the count must be EXACT, or this function gets pyst.
//MFREPACK
// don't need to pass in spans, they're already in fSourceSpans.
// clearSpansAfterRefresh is ignored anyway, nuke it.
uint32_t  plDrawableSpans::RefreshDISpans( uint32_t index )
{
    uint32_t          spanIdx;
    hsBounds3Ext    bounds;
    plDISpanIndex   *spanLookup;

//  hsAssert( fProps & kPropVolatile, "Trying to add spans on a non-volatile drawable" );
    hsAssert( index != (uint32_t)-1, "Invalid DI span index" );

    /// Do garbage cleanup first
    if( fNeedCleanup )
        IRemoveGarbage();

    spanLookup = IFindDIIndices( index );

    /// Loop through the spans and copy the vertex data over
    for (size_t i = 0; i < spanLookup->GetCount(); i++)
    {
        // Main info
        plGeometrySpan  *geoSpan = fSourceSpans[ (*spanLookup)[i] ];
        spanIdx = fSpanSourceIndices[ (*spanLookup)[ i ] ];
        hsAssert( ( spanIdx & kSpanTypeMask ) == kSpanTypeIcicle, "Mismatch in span formats" );
        plIcicle *icicle = &fIcicles[ spanIdx & kSpanIDMask ];
        IUpdateIcicleFromGeoSpan( geoSpan, icicle );
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
    SetSpaceTree(nullptr);

    fReadyToRender = false;

    return index;
}

// Same as above, but takes an actual span index (not a DI Index).
size_t plDrawableSpans::RefreshSpan(size_t index)
{
    uint32_t          spanIdx;
    hsBounds3Ext    bounds;

//  hsAssert( fProps & kPropVolatile, "Trying to add spans on a non-volatile drawable" );
    hsAssert(index < fSourceSpans.size(), "Invalid span index");

    /// Do garbage cleanup first
    if( fNeedCleanup )
        IRemoveGarbage();

    // Main info
    plGeometrySpan  *geoSpan = fSourceSpans[ index ];
    spanIdx = fSpanSourceIndices[ index ];
    hsAssert( ( spanIdx & kSpanTypeMask ) == kSpanTypeIcicle, "Mismatch in span formats" );
    plIcicle *icicle = &fIcicles[ spanIdx & kSpanIDMask ];
    IUpdateIcicleFromGeoSpan( geoSpan, icicle );
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
    SetSpaceTree(nullptr);

    fReadyToRender = false;

    return index;
}

//// IUpdateVertexSpanFromGeoSpan ////////////////////////////////////////////
//  Common function for the two functions that follow.

void    plDrawableSpans::IUpdateVertexSpanFromGeoSpan( plGeometrySpan *geoSpan, plVertexSpan *span )
{
    hsBounds3Ext    bounds;

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
        case plGeometrySpan::kLiteMaterial:         span->fProps |= plSpan::kLiteMaterial; break;
        case plGeometrySpan::kLiteVtxPreshaded:     span->fProps |= plSpan::kLiteVtxPreshaded; break;
        case plGeometrySpan::kLiteVtxNonPreshaded:  span->fProps |= plSpan::kLiteVtxNonPreshaded; break;
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

void    plDrawableSpans::IUpdateIcicleFromGeoSpan( plGeometrySpan *geoSpan, plIcicle *icicle )
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

uint32_t plDrawableSpans::GetNumTris(int spanIdx)
{
    hsAssert(fSpans[spanIdx]->fTypeMask & plSpan::kIcicleSpan, "Asking for index info on non-triangle based span");
    plIcicle* span = (plIcicle*)fSpans[spanIdx];
    return fGroups[span->fGroupIdx]->GetIndexBufferCount(span->fIBufferIdx) / 3;
}

uint16_t* plDrawableSpans::GetIndexList(int spanIdx)
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
    // Normal follows the position (+ 12Bytes)
    return *(hsVector3*)(fSourceSpans[spanIdx]->fVertexData + vtxIdx * plGeometrySpan::GetVertexSize(fSourceSpans[spanIdx]->fFormat) + 12);
}

uint32_t plDrawableSpans::CvtGetNumTris(int spanIdx)
{
    return fSourceSpans[spanIdx]->fNumIndices / 3;
}

uint16_t* plDrawableSpans::CvtGetIndexList(int spanIdx)
{
    return fSourceSpans[spanIdx]->fIndexData;
}

uint32_t plDrawableSpans::CvtGetNumVerts(int spanIdx) const
{
    return fSourceSpans[spanIdx]->fNumVerts;
}

plGeometrySpan* plDrawableSpans::GetGeometrySpan(int spanIdx) 
{ 
    return fSourceSpans[spanIdx]; 
}

void    plDrawableSpans::GetOrigGeometrySpans(size_t diIndex, std::vector<plGeometrySpan *> &arrayToFill)
{
    if (diIndex >= fDIIndices.size())
    {
        hsAssert( false, "Invalid diIndex to GetOrigGeometrySpans()" );
        return;
    }

    plDISpanIndex   *indices = fDIIndices[ diIndex ];


    arrayToFill.clear();
    for (size_t i = 0; i < indices->GetCount(); i++)
        arrayToFill.emplace_back(fSourceSpans[(*indices)[i]]);

    // HA!
    return;
}

void plDrawableSpans::ClearAndSetMaterialCount(uint32_t count)
{
    // Release the old materials
    for (hsGMaterial* material : fMaterials)
    {
        if (material != nullptr && material->GetKey() != nullptr)
            GetKey()->Release(material->GetKey());
    }

    fMaterials.assign(count, nullptr);
}
