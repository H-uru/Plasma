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
#include "plMorphSequence.h"
#include "plMorphSequenceSDLMod.h"

#include "plAccessGeometry.h"
#include "plAccessVtxSpan.h"

#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plDrawInterface.h"
#include "pnSceneObject/plCoordinateInterface.h"

#include "plDrawableSpans.h"
#include "plInstanceDrawInterface.h"

#include "hsResMgr.h"
#include "plgDispatch.h"
#include "plMessage/plRenderMsg.h"

#include "plSharedMesh.h"

#include "plTweak.h"
#include "hsTimer.h"
#include "hsFastMath.h"

///////////////////////////////////////////////////////////////////////////

void plMorphDataSet::Read(hsStream* s, hsResMgr* mgr)
{
    hsKeyedObject::Read(s, mgr);
    
    int n = s->ReadLE32();
    fMorphs.SetCount(n);
    int i;
    for( i = 0; i < n; i++ )
        fMorphs[i].Read(s, mgr);
}

void plMorphDataSet::Write(hsStream* s, hsResMgr* mgr)
{
    hsKeyedObject::Write(s, mgr);

    s->WriteLE32(fMorphs.GetCount());
    int i;
    for( i = 0; i < fMorphs.GetCount(); i++ )
        fMorphs[i].Write(s, mgr);
}   

///////////////////////////////////////////////////////////////////////////

const uint32_t kChanMask = plAccessVtxSpan::kPositionMask
                        | plAccessVtxSpan::kNormalMask
                        | plAccessVtxSpan::kUVWMask;

// Probably want another type which is just initialize/override
// so we can set the mesh to whatever base set we want, as if
// that's what got loaded from disk.
// Use Access RW. That might already be handled in the customization stuff.

plConst(float)   kMorphTime(0.5);

class plMorphTarget
{
public:
    uint16_t      fLayer;
    uint16_t      fDelta;
    float    fWeight;
};

hsTArray<plMorphTarget> fTgtWgts;

plMorphSequence::plMorphSequence()
:   fMorphFlags(0),
    fMorphSDLMod(nil),
    fGlobalLayerRef(-1)
{
}

plMorphSequence::~plMorphSequence()
{
    DeInit();
}

bool plMorphSequence::MsgReceive(plMessage* msg)
{
    plRenderMsg* rend = plRenderMsg::ConvertNoRef(msg);
    if( rend )
    {
        // For now, I'm ignoring the target weight stuff for shared meshes.
        // Can always add it in later if desired.
        if( fTgtWgts.GetCount() )
        {
            float delWgt = hsTimer::GetDelSysSeconds() / (kMorphTime > 0 ? float(kMorphTime) : 1.e-3f);
            int i;
            for( i = 0; i < fTgtWgts.GetCount(); i++ )
            {
                float currWgt = GetWeight(fTgtWgts[i].fLayer, fTgtWgts[i].fDelta);
                if( fTgtWgts[i].fWeight < currWgt )
                {
                    if( fTgtWgts[i].fWeight >= (currWgt -= delWgt) )
                        currWgt = fTgtWgts[i].fWeight;
                }
                else if( fTgtWgts[i].fWeight > currWgt )
                {
                    if( fTgtWgts[i].fWeight <= (currWgt += delWgt) )
                        currWgt = fTgtWgts[i].fWeight;
                }
                
                fMorphs[fTgtWgts[i].fLayer].SetWeight(fTgtWgts[i].fDelta, currWgt);

                if( fTgtWgts[i].fWeight == currWgt )
                {
                    fTgtWgts.Remove(i);
                    i--;
                }
            }
            ISetDirty(true);
        }

        if( !(fMorphFlags & kDirty) )
        {
            // We went a whole frame without getting dirty, 
            // we can stop refreshing now.
            plgDispatch::Dispatch()->UnRegisterForExactType(plRenderMsg::Index(), GetKey());

            return true;
        }
        ISetDirty(false);
        if( fMorphFlags & kDirtyIndices )
            IFindIndices();

        if( fMorphFlags & kHaveShared )
        {
            IApplyShared();
        }
        else
        {
            Apply();
        }
        return true;
    }

    plSharedMeshBCMsg *smMsg = plSharedMeshBCMsg::ConvertNoRef(msg);
    if (smMsg)
    {
        if (IGetDrawInterface()->GetKey() == smMsg->GetSender() || IIsUsingDrawable(smMsg->fDraw))
            fMorphFlags |= kDirtyIndices;
    }           

    plGenRefMsg *refMsg = plGenRefMsg::ConvertNoRef(msg);
    if (refMsg)
    {
        plSharedMesh *mesh = plSharedMesh::ConvertNoRef(refMsg->GetRef());
        if (mesh)
        {
            if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest) )
            {
                AddSharedMesh(mesh);
            }
            else if( refMsg->GetContext() & plRefMsg::kOnReplace)
            {
                plSharedMesh *oldMesh = plSharedMesh::ConvertNoRef(refMsg->GetOldRef());
                if (oldMesh)
                    RemoveSharedMesh(oldMesh);
                AddSharedMesh(mesh);
            }
            else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
                RemoveSharedMesh(mesh);             
            
            return true;
        }
    }
    
    return plSingleModifier::MsgReceive(msg);
}

int plMorphSequence::GetNumLayers(plKey meshKey /* = nil */) const
{
    int index = IFindSharedMeshIndex(meshKey);
    if (index < 0)
        return fMorphs.GetCount();
    else
        return fSharedMeshes[index].fMesh->fMorphSet->fMorphs.GetCount();
}

int plMorphSequence::GetNumDeltas(int iLay, plKey meshKey /* = nil */) const 
{ 
    int index = IFindSharedMeshIndex(meshKey);
    if (index < 0)
        return fMorphs[iLay].GetNumDeltas(); 
    else
        return fSharedMeshes[index].fMesh->fMorphSet->fMorphs[iLay].GetNumDeltas();
}

float plMorphSequence::GetWeight(int iLay, int iDel, plKey meshKey /* = nil */) const 
{ 
    int index = IFindSharedMeshIndex(meshKey);
    if (index == -1)
        return fMorphs[iLay].GetWeight(iDel); 
    else
        return fSharedMeshes[index].fArrayWeights[iLay].fDeltaWeights[iDel];
}

void plMorphSequence::SetWeight(int iLay, int iDel, float w, plKey meshKey /* = nil */)
{
    int index = IFindSharedMeshIndex(meshKey);

    // Only dirty if the weight isn't for a pending mesh
    if(meshKey == nil || index >= 0)
        ISetDirty(true);

    if (meshKey == nil)
    {
        if( iLay < fMorphs.GetCount() )
        {
            if( kMorphTime > 0 )
            {
                plMorphTarget tgt;
                tgt.fLayer = iLay;
                tgt.fDelta = iDel;
                tgt.fWeight = w;

                fTgtWgts.Append(tgt);
            }
            else
            {
                fMorphs[iLay].SetWeight(iDel, w);
            }
        }
    }
    else if (index >= 0)
    {
        fSharedMeshes[index].fArrayWeights[iLay].fDeltaWeights[iDel] = w;
        fSharedMeshes[index].fFlags |= plSharedMeshInfo::kInfoDirtyMesh;
        if (index == fGlobalLayerRef)
            ISetAllSharedToGlobal();
    }
    else
    {
        // Setting weight for a mesh we haven't added yet (loading state)
        index = IFindPendingStateIndex(meshKey);
        if (index < 0)
        {
            fPendingStates.Push();
            index = fPendingStates.GetCount() - 1;
            fPendingStates[index].fSharedMeshKey = meshKey;
        }
        if (fPendingStates[index].fArrayWeights.GetCount() <= iLay)
        {
            int had = fPendingStates[index].fArrayWeights.GetCount();
            hsTArray<plMorphArrayWeights> temp(iLay + 1);
            temp = fPendingStates[index].fArrayWeights;
            temp.SetCount(iLay + 1);

            fPendingStates[index].fArrayWeights.Swap(temp);
            int i;
            for( i = had; i < iLay+1; i++ )
                fPendingStates[index].fArrayWeights[i].fDeltaWeights.Reset();
        }
        if (fPendingStates[index].fArrayWeights[iLay].fDeltaWeights.GetCount() <= iDel)
            fPendingStates[index].fArrayWeights[iLay].fDeltaWeights.ExpandAndZero(iDel + 1);

        fPendingStates[index].fArrayWeights[iLay].fDeltaWeights[iDel] = w;
    }
}

void plMorphSequence::ISetDirty(bool on) 
{ 
    if( on )
    {
        if( !(fMorphFlags & kDirty) )
            plgDispatch::Dispatch()->RegisterForExactType(plRenderMsg::Index(), GetKey());
        fMorphFlags |= kDirty; 

        // Even if we know we're already dirty, there could be new info this frame.
        // Need to tell our scene object
        //
        // Actually, in the case of the avatar, this sends an insane flurry of messages to the server 
        // when we drag (or even just hold the mouse button down on) the morph scrollbars. 
        // These messages are completely unneccessary. We broadcast our state when we join a new age, and that's enough.
        // Non-avatar related morphs (if they ever exist) will need to call DirtySynchState some special
        // way. Not here.
        //
        //if (GetTarget(0))
        //  GetTarget(0)->DirtySynchState(kSDLMorphSequence, 0);
    }
    else
    {
        fMorphFlags &= ~kDirty; 
    }
}

// MorphSequence - Init
void plMorphSequence::Init()
{
    if( fMorphFlags & kHaveShared )
    {
        IFindIndices();
    }
    else
    {
        if( !GetHaveSnap() )
        {
            // Take an access snapshot of all our spans
            const plDrawInterface* di = IGetDrawInterface();

            plAccessGeometry::Instance()->TakeSnapShot(di, kChanMask);

            ISetHaveSnap(true);
        }
    }
}

void plMorphSequence::Activate()
{
    Init();
    ISetDirty(true);
}

void plMorphSequence::DeActivate()
{
    if( fMorphFlags & kHaveShared )
    {
        IReleaseIndices();
    }
    plgDispatch::Dispatch()->UnRegisterForExactType(plRenderMsg::Index(), GetKey());
}

// MorphSequence - DeInit
void plMorphSequence::DeInit()
{
    if( fMorphFlags & kHaveShared )
    {
        IReleaseIndices();
    }
    else
    {
        if( GetHaveSnap() )
        {
            // Release all our snapshot data
            const plDrawInterface* di = IGetDrawInterface();

            if( di ) 
                plAccessGeometry::Instance()->ReleaseSnapShot(di);

            ISetHaveSnap(false);
        }
    }
}

void plMorphSequence::IRenormalize(hsTArray<plAccessSpan>& dst) const
{
    int i;
    for( i = 0; i < dst.GetCount(); i++ )
    {
        hsAssert(dst[i].HasAccessVtx(), "Come on, everyone has vertices");
        plAccessVtxSpan& accVtx = dst[i].AccessVtx();
        int j;
        for( j = 0; j < accVtx.VertCount(); j++ )
        {
            hsFastMath::Normalize(accVtx.Normal(j));
        }
    }
}

// MorphSequence - Apply
void plMorphSequence::Apply() const
{
    const plDrawInterface* di = IGetDrawInterface();
    if( !di )
        return;

    // Reset to initial.
    Reset(di);

    // We'll be accumulating into the buffer, so open RW
    hsTArray<plAccessSpan> dst;
    plAccessGeometry::Instance()->OpenRW(di, dst);

    // For each MorphArray
    int i;
    for( i = 0; i < fMorphs.GetCount(); i++ )
    {
        // Apply Delta
        fMorphs[i].Apply(dst);
    }

    IRenormalize(dst);

    // Close up the access spans
    plAccessGeometry::Instance()->Close(dst);
}

// MorphSequence - Reset to initial
void plMorphSequence::Reset(const plDrawInterface* di) const
{
    if( !di )
    {
        di = IGetDrawInterface();
    }

    // Use access span RestoreSnapshot
    if( di )
        plAccessGeometry::Instance()->RestoreSnapShot(di, kChanMask);
}

const plDrawInterface* plMorphSequence::IGetDrawInterface() const
{
    plSceneObject* so = GetTarget();
    if( !so )
        return nil;

    const plDrawInterface* di = so->GetDrawInterface();

    return di;
}

void plMorphSequence::AddTarget(plSceneObject* so)
{
    plSingleModifier::AddTarget(so);

    if (!fMorphSDLMod)
    {
        fMorphSDLMod = new plMorphSequenceSDLMod;
        so->AddModifier(fMorphSDLMod);
    }
}

void plMorphSequence::RemoveTarget(plSceneObject *so)
{
    plSingleModifier::RemoveTarget(so);

    if (so)
        if (fMorphSDLMod)
            so->RemoveModifier(fMorphSDLMod);

    delete fMorphSDLMod;
    fMorphSDLMod = nil;     
}
    
void plMorphSequence::Read(hsStream* s, hsResMgr* mgr)
{
    plSingleModifier::Read(s, mgr);

    fMorphFlags = 0;

    int n = s->ReadLE32();
    fMorphs.SetCount(n);
    int i;
    for( i = 0; i < n; i++ )
        fMorphs[i].Read(s, mgr);
    
    n = s->ReadLE32();
    for( i = 0; i < n; i++)
        mgr->ReadKeyNotifyMe(s, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, -1), plRefFlags::kActiveRef);
}

void plMorphSequence::Write(hsStream* s, hsResMgr* mgr)
{
    plSingleModifier::Write(s, mgr);

    s->WriteLE32(fMorphs.GetCount());
    int i;
    for( i = 0; i < fMorphs.GetCount(); i++ )
        fMorphs[i].Write(s, mgr);

    s->WriteLE32(fSharedMeshes.GetCount());
    for( i = 0; i < fSharedMeshes.GetCount(); i++ )
        mgr->WriteKey(s, fSharedMeshes[i].fMesh);
}

// Normal sequence of calls:
// 1) on notification that meshes have changed (or activate)
//      IFindIndices() - Sets up indices 
//      IApplyShared() - Find which mesh is active and
//          IResetShared(iActive);
//          IApplyShared(iActive);
//      go dormant
// 2) on weight change
//      SetWeight() - Register for render message and set the new weight
// 3) on render msg:
//      if dirty
//          IApplyShared() - Find which mesh is active and
//              IResetShared(iActive);
//              IApplyShared(iActive);
//      else
//          Unregister for render message.
// 4) on deinit
//      IReleaseIndices() - Just let's us know there's nothing much to be done.
//

void plMorphSequence::IResetShared()
{
    int i;
    for (i = 0; i < fSharedMeshes.GetCount(); i++)
        IResetShared(i);
}

void plMorphSequence::IApplyShared()
{
    int i;
    for (i = 0; i < fSharedMeshes.GetCount(); i++)
    {
        IResetShared(i);
        IApplyShared(i);
    }
}

void plMorphSequence::IFindIndices()
{
    fMorphFlags &= ~kDirtyIndices;

    int i;
    for( i = 0; i < fSharedMeshes.GetCount(); i++ )
        IFindIndices(i);
}

void plMorphSequence::IReleaseIndices()
{
    int i;
    for( i = 0; i < fSharedMeshes.GetCount(); i++ )
        IReleaseIndices(i);
}


void plMorphSequence::IApplyShared(int iShare)
{
    if( iShare >= fSharedMeshes.GetCount() || fSharedMeshes[iShare].fCurrDraw == nil)
        return;

    plSharedMeshInfo& mInfo = fSharedMeshes[iShare];

    hsTArray<plAccessSpan> dst;
    // Now copy each shared mesh geometryspan into the drawable
    // to get it back to it's pristine condition.
    int i;
    for( i = 0; i < mInfo.fMesh->fSpans.GetCount(); i++ )
    {
        plAccessSpan dstAcc;
        plAccessGeometry::Instance()->OpenRW(mInfo.fCurrDraw, mInfo.fCurrIdx[i], dstAcc);

        dst.Append(dstAcc);
    }

    // For each MorphArray
    for( i = 0; i < mInfo.fMesh->fMorphSet->fMorphs.GetCount(); i++ )
    {
        // Apply Delta
        mInfo.fMesh->fMorphSet->fMorphs[i].Apply(dst, &mInfo.fArrayWeights[i].fDeltaWeights);
    }

    IRenormalize(dst);

    // Close up the access spans
    plAccessGeometry::Instance()->Close(dst);
    mInfo.fFlags &= ~plSharedMeshInfo::kInfoDirtyMesh;
}

bool plMorphSequence::IResetShared(int iShare)
{
    if( iShare >= fSharedMeshes.GetCount() || fSharedMeshes[iShare].fCurrDraw == nil)
        return false;

    plSharedMeshInfo& mInfo = fSharedMeshes[iShare];

    // Now copy each shared mesh geometryspan into the drawable
    // to get it back to it's pristine condition.
    int i;
    for( i = 0; i < mInfo.fMesh->fSpans.GetCount(); i++ )
    {
        plAccessSpan srcAcc;
        plAccessGeometry::Instance()->AccessSpanFromGeometrySpan(srcAcc, mInfo.fMesh->fSpans[i]);
        plAccessSpan dstAcc;
        plAccessGeometry::Instance()->OpenRW(mInfo.fCurrDraw, mInfo.fCurrIdx[i], dstAcc);
        
        plAccPosNormUVWIterator srcIter(&srcAcc.AccessVtx());
        plAccPosNormUVWIterator dstIter(&dstAcc.AccessVtx());

        const int numUVWs = srcAcc.AccessVtx().NumUVWs();

        for( srcIter.Begin(), dstIter.Begin(); srcIter.More(); srcIter.Advance(), dstIter.Advance() )
        {
            *dstIter.Position() = *srcIter.Position();
            *dstIter.Normal() = *srcIter.Normal();

            int j;
            for( j = 0; j < numUVWs; j++ )
                *dstIter.UVW(j) = *srcIter.UVW(j);
        }
    }

    return true;
}

bool plMorphSequence::IFindIndices(int iShare)
{
    plSharedMeshInfo& mInfo = fSharedMeshes[iShare];
    mInfo.fCurrDraw = nil; // In case we fail.

    const plInstanceDrawInterface* di = plInstanceDrawInterface::ConvertNoRef(IGetDrawInterface());
    if( !di )
        return false;

    int32_t meshIdx = di->GetSharedMeshIndex(mInfo.fMesh);
    if( meshIdx < 0 )
        return false;

    plDrawableSpans* dr = plDrawableSpans::ConvertNoRef(di->GetDrawable((uint8_t)meshIdx));
    if( !dr )
        return false;

    mInfo.fCurrDraw = dr;

    plDISpanIndex& diIndex = dr->GetDISpans(di->GetDrawableMeshIndex((uint8_t)meshIdx));

    hsAssert(mInfo.fMesh->fSpans.GetCount() == diIndex.GetCount(), "Mismatch between geometry and indices");

    mInfo.fCurrIdx.SetCount(diIndex.GetCount());

    int i;
    for( i = 0; i < diIndex.GetCount(); i++ )
        mInfo.fCurrIdx[i] = diIndex[i];

    return true;
}

void plMorphSequence::IReleaseIndices(int iShare)
{
    plSharedMeshInfo& mInfo = fSharedMeshes[iShare];
    mInfo.fCurrDraw = nil;
}

int32_t plMorphSequence::IFindSharedMeshIndex(plKey meshKey) const
{
    int i;
    for( i = 0; i < fSharedMeshes.GetCount(); i++ )
    {
        if (fSharedMeshes[i].fMesh->GetKey() == meshKey)
            return i;
    }

    return -1;
}

int32_t plMorphSequence::IFindPendingStateIndex(plKey meshKey) const
{
    int i;
    for( i = 0; i < fPendingStates.GetCount(); i++ )
    {
        if (fPendingStates[i].fSharedMeshKey == meshKey)
            return i;
    }
    
    return -1;
}

bool plMorphSequence::IIsUsingDrawable(plDrawable *draw)
{
    int i;
    for (i = 0; i < fSharedMeshes.GetCount(); i++)
    {
        if (fSharedMeshes[i].fCurrDraw == draw)
            return true;
    }

    return false;
}

void plMorphSequence::AddSharedMesh(plSharedMesh* mesh)
{
    // This happens if SDL state tells us to use a mesh without morph info
    // (Either because the object used to but no longer does, or because
    // the SDL is giving us the wrong key.)
    if (!mesh->fMorphSet)
        return;

    if (fSharedMeshes.GetCount() == 0)
        plgDispatch::Dispatch()->RegisterForExactType(plSharedMeshBCMsg::Index(), GetKey());

    if (IFindSharedMeshIndex(mesh->GetKey()) >= 0)
        return; // We already have it.

    hsAssert(fSharedMeshes.GetCount() < 127, "Too many meshes for one morph sequence.");
    SetUseSharedMesh(true); 
    int pendingIndex = IFindPendingStateIndex(mesh->GetKey());

    plSharedMeshInfo mInfo;
    mInfo.fMesh = mesh;
    mInfo.fCurrDraw = nil;

    // Intialize our weights to zero.
    mInfo.fArrayWeights.Reset();
    mInfo.fArrayWeights.SetCount(mesh->fMorphSet->fMorphs.GetCount());
    int i, j;
    for (i = 0; i < mesh->fMorphSet->fMorphs.GetCount(); i++)
        mInfo.fArrayWeights[i].fDeltaWeights.ExpandAndZero(mesh->fMorphSet->fMorphs[i].GetNumDeltas());
            
    // Aha, we have some pending weights. Copy them in!
    if (pendingIndex >= 0)
    {
        // Filter in any data that's valid
        for (i = 0; i < mInfo.fArrayWeights.GetCount() && i < fPendingStates[pendingIndex].fArrayWeights.GetCount(); i++)
        {
            for (j = 0; j < mInfo.fArrayWeights[i].fDeltaWeights.GetCount() && 
                        j < fPendingStates[pendingIndex].fArrayWeights[i].fDeltaWeights.GetCount(); j++)
            {
                mInfo.fArrayWeights[i].fDeltaWeights[j] = fPendingStates[pendingIndex].fArrayWeights[i].fDeltaWeights[j];
            }
        }
            
        fPendingStates.Remove(pendingIndex);

        mInfo.fFlags |= plSharedMeshInfo::kInfoDirtyMesh;
        ISetDirty(true);
    }

    fSharedMeshes.Append(mInfo);
    IFindIndices(fSharedMeshes.GetCount() - 1);

    if (mesh->fFlags & plSharedMesh::kLayer0GlobalToMod)
    {
        fGlobalLayerRef = fSharedMeshes.GetCount() - 1;
        ISetAllSharedToGlobal();
    }
    else 
        ISetSingleSharedToGlobal(fSharedMeshes.GetCount() - 1);
}

void plMorphSequence::RemoveSharedMesh(plSharedMesh* mesh)
{
    int idx = IFindSharedMeshIndex(mesh->GetKey());
    if (idx < 0)
        return; // Don't have it... can't remove it can we?
    
    fSharedMeshes.Remove(idx);

    fGlobalLayerRef = -1;
    int i;
    for (i = 0; i < fSharedMeshes.GetCount(); i++)
    {
        if (fSharedMeshes[i].fMesh->fFlags & plSharedMesh::kLayer0GlobalToMod)
        {
            fGlobalLayerRef = i;
            break;
        }
    }
    
    if (fSharedMeshes.GetCount() == 0)
        plgDispatch::Dispatch()->UnRegisterForExactType(plSharedMeshBCMsg::Index(), GetKey());
}

void plMorphSequence::FindMorphMods(const plSceneObject *so, hsTArray<const plMorphSequence*> &mods)
{
    const plMorphSequence *morph = plMorphSequence::ConvertNoRef(so->GetModifierByType(plMorphSequence::Index()));
    if (morph)
        mods.Append(morph);
    
    const plCoordinateInterface *ci = so->GetCoordinateInterface();
    int i;
    for (i = 0; i < ci->GetNumChildren(); i++)
        FindMorphMods(ci->GetChild(i)->GetOwner(), mods);
}

void plMorphSequence::ISetAllSharedToGlobal()
{
    int i;
    for (i = 0; i < fSharedMeshes.GetCount(); i++)
    {
        if (i != fGlobalLayerRef)
            ISetSingleSharedToGlobal(i);
    }
}

void plMorphSequence::ISetSingleSharedToGlobal(int idx)
{
    if (fGlobalLayerRef < 0)
        return;

    int i;
    for (i = 0; i < fSharedMeshes[fGlobalLayerRef].fArrayWeights[0].fDeltaWeights.GetCount(); i++)
        SetWeight(0, i, fSharedMeshes[fGlobalLayerRef].fArrayWeights[0].fDeltaWeights[i], fSharedMeshes[idx].fMesh->GetKey());
}
