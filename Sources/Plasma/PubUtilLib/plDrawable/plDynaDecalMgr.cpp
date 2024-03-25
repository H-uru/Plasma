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
#include "plDynaDecalMgr.h"
#include "plDynaDecal.h"

#include "plCutter.h"

#include "plAccessGeometry.h"
#include "plAccessSpan.h"
#include "plGBufferGroup.h"

#include "plDrawableSpans.h"
#include "plAuxSpan.h"
#include "plSpaceTree.h"

#include "plPrintShape.h"

#include <string_theory/format>

#include "plAvatar/plArmatureMod.h"

#include "plParticleSystem/plParticleSystem.h"
#include "plParticleSystem/plParticleEmitter.h"

#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plDrawInterface.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayerInterface.h"
#include "plScene/plPageTreeMgr.h"

#include "hsGDeviceRef.h"

#include "plMessage/plAgeLoadedMsg.h"
#include "plMessage/plDynaDecalEnableMsg.h"
#include "pnMessage/plRefMsg.h"
#include "pnMessage/plTimeMsg.h"
#include "plgDispatch.h"

#include "pnEncryption/plRandom.h"
#include "hsFastMath.h"

#include "hsStream.h"
#include "hsResMgr.h"
#include "hsTimer.h"

#include "pnMessage/plPipeResMakeMsg.h"

#include "plGImage/plBumpMapGen.h"

// Stuff for creating a bumpenv decal on demand.
#include "plGImage/plMipmap.h"
#include "plSurface/plLayer.h"
#include "plMessage/plLayRefMsg.h"

//### Hackage
#include "plMessage/plRenderMsg.h"
#include "plMessage/plListenerMsg.h"
#include "plPipeline.h"

#include "plTweak.h"

#include "plProfile.h"

plProfile_CreateTimerNoReset("Total", "DynaDecal", Total);
plProfile_CreateTimerNoReset("Cutter", "DynaDecal", Cutter);
plProfile_CreateTimerNoReset("Process", "DynaDecal", Process);
plProfile_CreateTimerNoReset("Callback", "DynaDecal", Callback);

static const int    kBinBlockSize = 20;
static const uint16_t kDefMaxNumVerts = 1000;
static const uint16_t kDefMaxNumIdx = kDefMaxNumVerts;

static const float kDefLifeSpan = 30.f;
static const float kDefDecayStart = kDefLifeSpan * 0.5f;
static const float kDefRampEnd = kDefLifeSpan * 0.1f;

static const float kInitAuxSpans = 5;

#define MF_NO_INIT_ALLOC
#define MF_NEVER_RUN_OUT

// If we aren't doing an initial alloc, we MUST have NEVER_RUN_OUT
// It's also useful to not have initial alloc but do have NEVER_RUN_OUT
// So:
//  MF_NO_INIT_ALLOC && MF_NEVER_RUN_OUT - Okay
//  !MF_NO_INIT_ALLOC && !MF_NEVER_RUN_OUT - Okay
//  !MF_NO_INIT_ALLOC && MF_NEVER_RUN_OUT - Okay
//  MF_NO_INIT_ALLOC && !MF_NEVER_RUN_OUT - Bad (you'll never get any decals)
#if defined(MF_NO_INIT_ALLOC) && !defined(MF_NEVER_RUN_OUT)
#define MF_NEVER_RUN_OUT
#endif // defined(MF_NO_INIT_ALLOC) && !defined(MF_NEVER_RUN_OUT)

using namespace std;

bool plDynaDecalMgr::fDisableAccumulate = false;
bool plDynaDecalMgr::fDisableUpdate = false;

plDynaDecalMgr::plDynaDecalMgr()
:   
    fMatPreShade(),
    fMatRTShade(),
    fMaxNumVerts(kDefMaxNumVerts),
    fMaxNumIdx(kDefMaxNumIdx),
    fWetLength(),
    fRampEnd(kDefRampEnd),
    fDecayStart(kDefDecayStart),
    fLifeSpan(kDefLifeSpan),
    fInitAtten(1.f),
    fIntensity(1.f),
    fWaitOnEnable(false),
    fGridSizeU(2.5f),
    fGridSizeV(2.5f),
    fScale(1.f, 1.f, 1.f),
    fPartyTime(1.f)
{
    fCutter = new plCutter;
}

plDynaDecalMgr::~plDynaDecalMgr()
{
    for (plDynaDecal* decal : fDecals)
        delete decal;

    for (plAuxSpan* aux : fAuxSpans)
    {
        if (aux->fDrawable)
        {
            plSpan* span = const_cast<plSpan*>(aux->fDrawable->GetSpan(aux->fBaseSpanIdx));

            span->RemoveAuxSpan(aux);
        }

        delete aux->fGroup;

        delete aux;
    }

    delete fCutter;
}

void plDynaDecalMgr::SetKey(plKey k)
{
    hsKeyedObject::SetKey(k);
    if( k )
    {
        plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
        plgDispatch::Dispatch()->RegisterForExactType(plPipeGeoMakeMsg::Index(), GetKey());
        plgDispatch::Dispatch()->RegisterForExactType(plAgeLoadedMsg::Index(), GetKey());
    }
}

void plDynaDecalMgr::Read(hsStream* stream, hsResMgr* mgr)
{
    plSynchedObject::Read(stream, mgr);

    mgr->ReadKeyNotifyMe(stream, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefMatPreShade), plRefFlags::kActiveRef);

    mgr->ReadKeyNotifyMe(stream, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefMatRTShade), plRefFlags::kActiveRef);

    uint32_t n = stream->ReadLE32();
    for (uint32_t i = 0; i < n; i++)
    {
        mgr->ReadKeyNotifyMe(stream, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefTarget), plRefFlags::kPassiveRef);
    }
    // Associated slave particle systems. We read in the scene objects now, and find the associated systems on loaded message.
    n = stream->ReadLE32();
    for (uint32_t i = 0; i < n; i++)
    {
        mgr->ReadKeyNotifyMe(stream, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefPartyObject), plRefFlags::kPassiveRef);
    }

    fMaxNumVerts = stream->ReadLE32();
    fMaxNumIdx = stream->ReadLE32();

    fWaitOnEnable = stream->ReadLE32();

    fIntensity = stream->ReadLEFloat();
    fInitAtten = fIntensity;

    fWetLength = stream->ReadLEFloat();
    fRampEnd = stream->ReadLEFloat();
    fDecayStart = stream->ReadLEFloat();
    fLifeSpan = stream->ReadLEFloat();

    fGridSizeU = stream->ReadLEFloat();
    fGridSizeV = stream->ReadLEFloat();

    fScale.Read(stream);

    fPartyTime = stream->ReadLEFloat();

    n = stream->ReadLE32();
    fNotifies.resize(n);
    for (uint32_t i = 0; i < n; i++)
        fNotifies[i] = mgr->ReadKey(stream);
    
    // If we need to be creating DynaDecalMgrs on the fly, this should go in the
    // constructor, or we should call it explicitly on the DynaDecalMgr we create.
    // But putting it here makes it automatic for normal scene loading, without
    // popping up during the export conversion process.
#ifndef MF_NO_INIT_ALLOC
    InitAuxSpans();
#endif // MF_NO_INIT_ALLOC

    /////////////////////////////////////////////////////
    // ###Things that should be in derived classes follow.

}

void plDynaDecalMgr::Write(hsStream* stream, hsResMgr* mgr)
{
    plSynchedObject::Write(stream, mgr);

    mgr->WriteKey(stream, fMatPreShade);
    mgr->WriteKey(stream, fMatRTShade);

    stream->WriteLE32((uint32_t)fTargets.size());

    for (plSceneObject* target : fTargets)
    {
        mgr->WriteKey(stream, target);
    }

    // Particle systems (really their associated sceneobjects).
    stream->WriteLE32((uint32_t)fPartyObjects.size());
    for (plSceneObject* partyObj : fPartyObjects)
    {
        mgr->WriteKey(stream, partyObj);
    }

    stream->WriteLE32(fMaxNumVerts);
    stream->WriteLE32(fMaxNumIdx);

    stream->WriteLE32(fWaitOnEnable);

    stream->WriteLEFloat(fIntensity);

    stream->WriteLEFloat(fWetLength);
    stream->WriteLEFloat(fRampEnd);
    stream->WriteLEFloat(fDecayStart);
    stream->WriteLEFloat(fLifeSpan);

    stream->WriteLEFloat(fGridSizeU);
    stream->WriteLEFloat(fGridSizeV);

    fScale.Write(stream);

    stream->WriteLEFloat(fPartyTime);

    stream->WriteLE32((uint32_t)fNotifies.size());
    for (const plKey& notifyKey : fNotifies)
        mgr->WriteKey(stream, notifyKey);

    /////////////////////////////////////////////////////
    // ###Things that should be in derived classes follow.

}

bool plDynaDecalMgr::IMakeAuxRefs(plPipeline* pipe)
{
    for (plGBufferGroup* group : fGroups)
        group->PrepForRendering(pipe, false);

    return true;
}

const plPrintShape* plDynaDecalMgr::IGetPrintShape(const plKey& objKey) const
{
    const plPrintShape* shape = nullptr;
    plSceneObject* part = plSceneObject::ConvertNoRef(objKey->ObjectIsLoaded());
    if( part )
    {
        // This is a safe cast, because GetGenericInterface(type) will only return
        // either a valid object of that type, or nullptr.
        shape = static_cast<plPrintShape*>(part->GetGenericInterface(plPrintShape::Index()));
    }
    return shape;
}

const plPrintShape* plDynaDecalMgr::IGetPrintShape(plArmatureMod* avMod, uint32_t id) const
{
    const plPrintShape* shape = nullptr;
    const plSceneObject* part = avMod->FindBone(id);
    if( part )
    {
        // This is a safe cast, because GetGenericInterface(type) will only return
        // either a valid object of that type, or nullptr.
        shape = static_cast<plPrintShape*>(part->GetGenericInterface(plPrintShape::Index()));
    }
    return shape;
}

bool plDynaDecalMgr::IHandleEnableMsg(const plDynaDecalEnableMsg* enaMsg)
{
    IWetParts(enaMsg);

    return true;
}

bool plDynaDecalMgr::IWetParts(const plDynaDecalEnableMsg* enaMsg)
{
    if( !enaMsg->IsArmature() )
    {
        const plPrintShape* shape = IGetPrintShape(enaMsg->GetShapeKey());
        if( shape )
        {
            plDynaDecalInfo& info = IGetDecalInfo(uintptr_t(shape), shape->GetKey());
            IWetInfo(info, enaMsg);
        }
    }
    else
    if( enaMsg->GetID() == uint32_t(-1) )
    {
        plArmatureMod* avMod = plArmatureMod::ConvertNoRef(enaMsg->GetArmKey()->ObjectIsLoaded());
        for (uint32_t partID : fPartIDs)
        {
            const plPrintShape* shape = IGetPrintShape(avMod, partID);
            if( shape )
            {
                plDynaDecalInfo& info = IGetDecalInfo(uintptr_t(shape), shape->GetKey());
                IWetInfo(info, enaMsg);
            }
        }
    }
    else
    {
        IWetPart(enaMsg->GetID(), enaMsg);
    }
    return true;
}

bool plDynaDecalMgr::IWetPart(uint32_t id, const plDynaDecalEnableMsg* enaMsg)
{
    plArmatureMod* avMod = plArmatureMod::ConvertNoRef(enaMsg->GetArmKey()->ObjectIsLoaded());

    const plPrintShape* shape = IGetPrintShape(avMod, id);
    if( shape )
    {
        plDynaDecalInfo& info = IGetDecalInfo(uintptr_t(shape), shape->GetKey());
        IWetInfo(info, enaMsg);
    }
    return true;
}

void plDynaDecalMgr::IWetInfo(plDynaDecalInfo& info, const plDynaDecalEnableMsg* enaMsg) const
{
    info.fWetTime = enaMsg->GetContactTime();
    info.fWetLength = enaMsg->GetWetLength();
    if( !enaMsg->AtEnd() )
        info.fFlags |= plDynaDecalInfo::kImmersed;
    else
        info.fFlags &= ~plDynaDecalInfo::kImmersed;
}

bool plDynaDecalMgr::MsgReceive(plMessage* msg)
{
    // On eval pulse, update all our active decals, letting old ones die off.
    plEvalMsg* eval = plEvalMsg::ConvertNoRef(msg);
    if( eval )
    {
        IUpdateDecals(hsTimer::GetSysSeconds());
        return true;
    }

    plDynaDecalEnableMsg* enaMsg = plDynaDecalEnableMsg::ConvertNoRef(msg);
    if( enaMsg )
    {
        IHandleEnableMsg(enaMsg);

        return true;
    }

    plPipeGeoMakeMsg* make = plPipeGeoMakeMsg::ConvertNoRef(msg);
    if( make )
    {
        return IMakeAuxRefs(make->Pipeline());
    }

    plAgeLoadedMsg* ageLoadMsg = plAgeLoadedMsg::ConvertNoRef(msg);
    if( ageLoadMsg && ageLoadMsg->fLoaded ) 
    {
        IGetParticles();
        return true;
    }

    plGenRefMsg* refMsg = plGenRefMsg::ConvertNoRef(msg);
    if( refMsg )
    {
        switch( refMsg->fType )
        {
        case kRefMatPreShade:
            if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
                fMatPreShade = hsGMaterial::ConvertNoRef(refMsg->GetRef());
            else
                fMatPreShade = nullptr;
            return true;
        case kRefMatRTShade:
            if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
                fMatRTShade = hsGMaterial::ConvertNoRef(refMsg->GetRef());
            else
                fMatRTShade = nullptr;
            return true;
        case kRefTarget:
            if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
            {
                fTargets.emplace_back(plSceneObject::ConvertNoRef(refMsg->GetRef()));
            }
            else
            {
                auto iter = std::find(fTargets.cbegin(), fTargets.cend(), (plSceneObject*)refMsg->GetRef());
                if (iter != fTargets.cend())
                    fTargets.erase(iter);
            }
            return true;
        case kRefPartyObject:
            if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
            {
                fPartyObjects.emplace_back(plSceneObject::ConvertNoRef(refMsg->GetRef()));
            }
            else
            {
                auto iter = std::find(fPartyObjects.cbegin(), fPartyObjects.cend(), (plSceneObject*)refMsg->GetRef());
                if (iter != fPartyObjects.cend())
                    fPartyObjects.erase(iter);
            }
            return true;
        case kRefParticles:
            if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
            {
                fParticles.emplace_back(plParticleSystem::ConvertNoRef(refMsg->GetRef()));
                fParticles.back()->fMiscFlags |= plParticleSystem::kParticleSystemAlwaysUpdate;
            }
            else
            {
                auto iter = std::find(fParticles.cbegin(), fParticles.cend(), (plParticleSystem*)refMsg->GetRef());
                if (iter != fParticles.cend())
                    fParticles.erase(iter);
            }
            return true;
        case kRefAvatar:
            if( refMsg->GetContext() & (plRefMsg::kOnRemove|plRefMsg::kOnDestroy) )
                IRemoveDecalInfo(uintptr_t(refMsg->GetRef()));
            return true;
        }
    }

    return plSynchedObject::MsgReceive(msg);
}

//////////////////////////////////////////////////////////////////////////////////
//
void plDynaDecalMgr::INotifyActive(plDynaDecalInfo& info, const plKey& armKey, uint32_t id) const
{
    if( !(info.fFlags & plDynaDecalInfo::kActive) )
    {
        double secs = hsTimer::GetSysSeconds();
        for (const plKey& notifyKey : fNotifies)
        {
            plDynaDecalEnableMsg* enaMsg = new plDynaDecalEnableMsg(notifyKey, armKey, secs, fWetLength, false, id);
            enaMsg->Send();
        }
        info.fFlags |= plDynaDecalInfo::kActive;
    }
}

void plDynaDecalMgr::INotifyInactive(plDynaDecalInfo& info, const plKey& armKey, uint32_t id) const
{
    if( info.fFlags & plDynaDecalInfo::kActive )
    {
        double secs = hsTimer::GetSysSeconds();
        for (const plKey& notifyKey : fNotifies)
        {
            plDynaDecalEnableMsg* enaMsg = new plDynaDecalEnableMsg(notifyKey, armKey, secs, fWetLength, true, id);
            enaMsg->Send();
        }
        info.fFlags &= ~plDynaDecalInfo::kActive;
    }
}

plDynaDecalInfo& plDynaDecalInfo::Init(const plKey& key)
{
    fKey = key;

    fLastTime = -1.e33f;
    fLastPos.Set(-1.e33f, -1.e33f, -1.e33f);
    fWetTime = -1.e33f;
    fWetLength = 0;
    fFlags = kNone;

    return *this;
}

plDynaDecalInfo& plDynaDecalMgr::IGetDecalInfo(uintptr_t id, const plKey& key)
{
    plDynaDecalMap::iterator iter = fDecalMap.find(id);
    if( iter == fDecalMap.end() )
    {
        plDynaDecalInfo decalInfo;
        decalInfo.Init(key);
        plGenRefMsg* refMsg = new plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kRefAvatar);
        hsgResMgr::ResMgr()->AddViaNotify(plKey(key), refMsg, plRefFlags::kPassiveRef);

        pair<plDynaDecalMap::iterator, bool> iterPair;
        iterPair = fDecalMap.insert(plDynaDecalMap::value_type(id, decalInfo));
        iter = iterPair.first;
    }

    return iter->second;
}

void plDynaDecalMgr::IRemoveDecalInfo(uintptr_t id)
{
    plDynaDecalMap::iterator iter = fDecalMap.find(id);
    if( iter != fDecalMap.end() )
        fDecalMap.erase(iter, iter);
}

void plDynaDecalMgr::IRemoveDecalInfos(const plKey& key)
{
    plDynaDecalMap::iterator iter = fDecalMap.begin();
    while( iter != fDecalMap.end() )
    {
        if( iter->second.fKey == key )
        {
            plDynaDecalMap::iterator nuke0 = iter;
            plDynaDecalMap::iterator nuke1 = iter;
            iter++;
            while( (iter != fDecalMap.end()) && (iter->second.fKey == key) )
            {
                nuke1 = iter;
                iter++;
            }
            fDecalMap.erase(nuke0, nuke1);
        }
    }
}

float plDynaDecalMgr::IHowWet(plDynaDecalInfo& info, double t) const
{
    // We aren't playing this wet/dry/enable/disable thing.
    if( !fWaitOnEnable )
        return fIntensity;

    // We've been notified we've entered the pool, 
    // and haven't been notified we've left it.
    if( info.fFlags & plDynaDecalInfo::kImmersed )
    {
        info.fWetTime = t;
        return fIntensity;
    }

    // We've never been enabled.
    if( info.fWetLength <= 0 )
        return 0;

    // We're wet, let's see how wet.
    float wet = (float)(1.f - (t - info.fWetTime) / info.fWetLength);
    if( wet > 1.f ) // This should never happen. It means t < info.fWetTime (we get wet in the future).
        return fIntensity;
    if( wet < 0 )
        return 0;
    return wet * fIntensity;
}
//
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

plAuxSpan* plDynaDecalMgr::IGetAuxSpan(plDrawableSpans* targ, int iSpan, hsGMaterial* mat, uint16_t numVerts, uint16_t numIdx)
{
    // Some of this code just assumes you get the number of verts you ask for.
    // Which was causing errors when you asked for more than the max and didn't
    // get it. So now if you ask for too many verts, you just lose.
    if (numVerts > fMaxNumVerts || numIdx > fMaxNumIdx)
        return nullptr;

    plSpan* span = const_cast<plSpan*>(targ->GetSpan(iSpan));

    // First, see if we've got an aux span already sitting on this span.
    // We can use an existing aux span iff
    // a) we own it
    // b) there's enough room to append our stuff.
    //
    // This is kind of overkill in both respects, because:
    // a) we don't care if we own it, it just really needs the same material
    // b) there might be room at the beginning or middle of the span
    //
    // Relaxing those brings on the additional bookkeeping with having a span
    // that requires multiple drawprimitive calls.
    // Case a) can cause it if we share with a DynaDecalMgr with a different
    // lifespan than ours. That would allow decals in the middle of the span
    // to die out.
    // Case b) won't have random bits dropping out of the middle, but will 
    // create a gap in the middle (and probably end) when we wrap around.
    //
    // The simplicity in bookkeeping should make up for any space/speed advantages
    // in packing more into a single AuxSpan.
    for (size_t i = 0; i < span->GetNumAuxSpans(); i++)
    {
        plAuxSpan* aux = span->GetAuxSpan(i);
        if( (aux->fOwner == (void*)this)
            &&(aux->fVStartIdx + aux->fVLength + numVerts < aux->fVBufferLimit)
            &&(aux->fIStartIdx + aux->fILength + numIdx < aux->fIBufferLimit) )
            return aux;
    }

    bool rtLit = span->fProps & plSpan::kLiteVtxNonPreshaded;

    // Now look to see if we've got one sitting around unused that's suitable.
    // Here the suitable criteria is a little different. We know we are the owner,
    // and we know there's enough room (because it's sitting idle).
    for (plAuxSpan* aux : fAuxSpans)
    {
        if( !aux->fDrawable
            &&(aux->fVStartIdx + aux->fVLength + numVerts < aux->fVBufferLimit)
            &&(aux->fIStartIdx + aux->fILength + numIdx < aux->fIBufferLimit) )
        {
            aux->fDrawable = targ;
            aux->fBaseSpanIdx = iSpan;

            ISetAuxMaterial(aux, mat, rtLit);

            span->AddAuxSpan(aux);

            return aux;
        }
    }

    // Ain't got one. We could allocate another one, or we can just say too bad doo-dad.
    // If we allocate a new one:
    //      A) we'll need to flush managed memory to load it in.
    //      B) we'll be stuck with that memory (video and system) used up until this age is paged out and reloaded
    //      C) we've got a whole bunch of extra faces to draw.
    // If we just return nullptr:
    //      A) opposite of above
    //      B) we stop leaving footprints/ripples for a while.
    // I'm going to try the latter for a bit.

#ifdef MF_NEVER_RUN_OUT
    // Okay, nothing there. Let's get a new one.
    plAuxSpan* aux = new plAuxSpan;
    fAuxSpans.emplace_back(aux);

    IAllocAuxSpan(aux, numVerts, numIdx);

    aux->fOwner = (void*)this;
    aux->fDrawable = targ;
    aux->fBaseSpanIdx = iSpan;

    ISetAuxMaterial(aux, mat, rtLit);

    span->AddAuxSpan(aux);

    return aux;
#else // MF_NEVER_RUN_OUT
    return nullptr;
#endif // MF_NEVER_RUN_OUT
}

void plDynaDecalMgr::InitAuxSpans()
{
    int i;
    for( i = 0; i < kInitAuxSpans; i++ )
    {
        plAuxSpan* aux = new plAuxSpan;
        fAuxSpans.emplace_back(aux);
        IAllocAuxSpan(aux, fMaxNumVerts, fMaxNumIdx);
    }
}

void plDynaDecalMgr::IAllocAuxSpan(plAuxSpan* aux, uint32_t maxNumVerts, uint32_t maxNumIdx)
{
    plGBufferGroup* grp = new plGBufferGroup(kDecalVtxFormat, true, false);
    fGroups.emplace_back(grp);

    grp->ReserveVertStorage(maxNumVerts, 
        &aux->fVBufferIdx, 
        &aux->fCellIdx, 
        &aux->fCellOffset, 
        plGBufferGroup::kReserveInterleaved);

    aux->fFlags = 0;

    aux->fVStartIdx = grp->GetVertStartFromCell(aux->fVBufferIdx, aux->fCellIdx, aux->fCellOffset);
    aux->fVLength = 0;

    uint16_t* dataPtr = nullptr;
    grp->ReserveIndexStorage(maxNumIdx, &aux->fIBufferIdx, &aux->fIStartIdx, &dataPtr);
    //aux->fIStartIdx /* should be assigning something? */;

    aux->fILength = 0;

    aux->fGroup = grp;

    aux->fVBufferInit = aux->fVStartIdx;
    aux->fVBufferLimit = aux->fVBufferInit + maxNumVerts;

    aux->fIBufferInit = aux->fIStartIdx;
    aux->fIBufferLimit = aux->fIBufferInit + maxNumIdx;

    aux->fOrigPos.resize(maxNumVerts);
    aux->fOrigUVW.resize(maxNumVerts);

    aux->fOwner = (void*)this;
    aux->fDrawable = nullptr;
    aux->fBaseSpanIdx = 0;

    grp->SetVertBufferStart(aux->fVBufferIdx, aux->fVStartIdx);
    grp->SetIndexBufferStart(aux->fIBufferIdx, aux->fIStartIdx);
    grp->SetVertBufferEnd(aux->fVBufferIdx, aux->fVStartIdx);
    grp->SetIndexBufferEnd(aux->fIBufferIdx, aux->fIStartIdx);
}

hsGMaterial* plDynaDecalMgr::ISetAuxMaterial(plAuxSpan* aux, hsGMaterial* mat, bool rtLit)
{
    if( !mat )
        mat = fMatRTShade;

    bool attenColor =  0 != (mat->GetLayer(0)->GetBlendFlags() 
                                & (hsGMatState::kBlendAdd
                                    | hsGMatState::kBlendMult
                                    | hsGMatState::kBlendMADD));
    bool bump = 0 != (mat->GetLayer(0)->GetMiscFlags() & hsGMatState::kMiscBumpChans);
    bool hasVS = nullptr != mat->GetLayer(0)->GetVertexShader();

    if( hasVS )
    {
        aux->fFlags |= plAuxSpan::kVertexShader;
        aux->fFlags &= ~plAuxSpan::kAttenColor;
        aux->fFlags &= ~(plAuxSpan::kOverrideLiteModel | plAuxSpan::kRTLit);
        aux->fMaterial = mat;
    }
    else
    if( bump )
    {
        aux->fFlags &= ~plAuxSpan::kVertexShader;
        aux->fFlags |= plAuxSpan::kAttenColor;
        aux->fFlags &= ~(plAuxSpan::kOverrideLiteModel | plAuxSpan::kRTLit);
        aux->fMaterial = mat;
    }
    else
    if( attenColor )
    {
        aux->fFlags &= ~plAuxSpan::kVertexShader;
        aux->fFlags |= plAuxSpan::kOverrideLiteModel | plAuxSpan::kAttenColor;
        aux->fFlags &= ~plAuxSpan::kRTLit;
        aux->fMaterial = mat;
    }
    else
    if( rtLit )
    {
        aux->fFlags &= ~plAuxSpan::kVertexShader;
        aux->fFlags &= ~(plAuxSpan::kOverrideLiteModel | plAuxSpan::kAttenColor);
        aux->fFlags |= plAuxSpan::kRTLit;
        aux->fMaterial = mat;
    }
    else
    {
        aux->fFlags &= ~(plAuxSpan::kOverrideLiteModel | plAuxSpan::kAttenColor);
        aux->fFlags &= ~plAuxSpan::kRTLit;
        aux->fMaterial = fMatPreShade;
    }
    return aux->fMaterial;
}

//////////////////////////////////////////////////////////////////////////////////

plDynaDecal* plDynaDecalMgr::IInitDecal(plAuxSpan* aux, double t, uint16_t numVerts, uint16_t numIdx)
{
    size_t idx = INewDecal();

    fDecals[idx]->fStartVtx = (uint16_t)(aux->fVStartIdx + aux->fVLength);
    fDecals[idx]->fNumVerts = numVerts;
    fDecals[idx]->fStartIdx = (uint16_t)(aux->fIStartIdx + aux->fILength);
    fDecals[idx]->fNumIdx = numIdx;

    fDecals[idx]->fBirth = t;
    fDecals[idx]->fFlags = plDynaDecal::kFresh;
    fDecals[idx]->fInitAtten = fInitAtten;

    fDecals[idx]->fAuxSpan = aux;

    aux->fVLength += numVerts;
    aux->fGroup->SetVertBufferEnd(aux->fVBufferIdx, aux->fVStartIdx + aux->fVLength);
    aux->fGroup->DirtyVertexBuffer(aux->fVBufferIdx);

    aux->fILength += numIdx;
    aux->fGroup->SetIndexBufferEnd(aux->fIBufferIdx, aux->fIStartIdx + aux->fILength);
    aux->fGroup->DirtyIndexBuffer(aux->fIBufferIdx);

    if( aux->fFlags & plAuxSpan::kVertexShader )
        fDecals[idx]->fFlags |= plDynaDecal::kVertexShader;
    else
    if( aux->fFlags & plAuxSpan::kAttenColor )
        fDecals[idx]->fFlags |= plDynaDecal::kAttenColor;

    // We should probably assert here that our span hasn't just overrun buffergroup storage.
    hsAssert(aux->fVStartIdx + aux->fVLength <= aux->fVBufferLimit, "Overrunning allocated storage");
    hsAssert(aux->fIStartIdx + aux->fILength <= aux->fIBufferLimit, "Overrunning allocated storage");

    hsAssert(aux->fGroup->GetVertBufferEnd(aux->fVBufferIdx) >= aux->fGroup->GetVertBufferStart(aux->fVBufferIdx), "Going out of range on verts");
    hsAssert(aux->fGroup->GetIndexBufferEnd(aux->fIBufferIdx) >= aux->fGroup->GetIndexBufferStart(aux->fIBufferIdx), "Going out of range on verts");

    return fDecals[idx];
}

void plDynaDecalMgr::IKillDecal(size_t i)
{
    // Update this decal's span.
    // Since decals die off in the same order they are created, and we always 
    // append a decal to a span, we only need to advance the span's start indices,
    // and decrement the lengths.
    plAuxSpan* aux = fDecals[i]->fAuxSpan;
    aux->fVStartIdx += fDecals[i]->fNumVerts;
    aux->fGroup->SetVertBufferStart(aux->fVBufferIdx, aux->fVStartIdx);
    aux->fVLength -= fDecals[i]->fNumVerts;

    aux->fIStartIdx += fDecals[i]->fNumIdx;
    aux->fGroup->SetIndexBufferStart(aux->fIBufferIdx, aux->fIStartIdx);
    aux->fILength -= fDecals[i]->fNumIdx;

    hsAssert(aux->fGroup->GetVertBufferEnd(aux->fVBufferIdx) >= aux->fGroup->GetVertBufferStart(aux->fVBufferIdx), "Going out of range on verts");
    hsAssert(aux->fGroup->GetIndexBufferEnd(aux->fIBufferIdx) >= aux->fGroup->GetIndexBufferStart(aux->fIBufferIdx), "Going out of range on verts");

    if( !aux->fVLength )
    {
        hsAssert(!aux->fILength, "Ran out of verts before indices");
        aux->fVStartIdx = aux->fVBufferInit;
        aux->fIStartIdx = aux->fIBufferInit;

        aux->fGroup->SetVertBufferStart(aux->fVBufferIdx, aux->fVStartIdx);
        aux->fGroup->SetIndexBufferStart(aux->fIBufferIdx, aux->fIStartIdx);
        aux->fGroup->SetVertBufferEnd(aux->fVBufferIdx, aux->fVStartIdx);
        aux->fGroup->SetIndexBufferEnd(aux->fIBufferIdx, aux->fIStartIdx);

        hsAssert(aux->fGroup->GetVertBufferEnd(aux->fVBufferIdx) >= aux->fGroup->GetVertBufferStart(aux->fVBufferIdx), "Going out of range on verts");
        hsAssert(aux->fGroup->GetIndexBufferEnd(aux->fIBufferIdx) >= aux->fGroup->GetIndexBufferStart(aux->fIBufferIdx), "Going out of range on verts");

        if( aux->fDrawable )
            const_cast<plSpan*>(aux->fDrawable->GetSpan(aux->fBaseSpanIdx))->RemoveAuxSpan(aux);

        aux->fDrawable = nullptr;
        aux->fBaseSpanIdx = 0;
    }

    delete fDecals[i];
    fDecals.erase(fDecals.begin() + i);
}

void plDynaDecalMgr::IUpdateDecals(double t)
{
    if( fDisableUpdate )
        return;

    for (size_t i = 0; i < fDecals.size(); )
    {
        if (fDecals[i]->Age(t, fRampEnd, fDecayStart, fLifeSpan))
            IKillDecal(i);
        else
            ++i;
    }

    for (plAuxSpan* aux : fAuxSpans)
    {
        if (aux->fVLength)
            aux->fGroup->DirtyVertexBuffer(aux->fVBufferIdx);
    }
}

//////////////////////////////////////////////////////////////////////////////////

void plDynaDecalMgr::ICountIncoming(std::vector<plCutoutPoly>& src, uint16_t& numVerts, uint16_t& numIdx) const
{
    numVerts = 0;
    numIdx = 0;
    for (const plCutoutPoly& poly : src)
    {
        if (!poly.fVerts.empty())
        {
            numVerts += uint16_t(poly.fVerts.size());
            numIdx += uint16_t(poly.fVerts.size() - 2);
        }
    }
    numIdx *= 3;
}

plDecalVtxFormat* plDynaDecalMgr::IGetBaseVtxPtr(const plAuxSpan* auxSpan) const
{
    plGBufferGroup* grp = auxSpan->fGroup;
    plGBufferCell* cell = grp->GetCell(auxSpan->fVBufferIdx, auxSpan->fCellIdx);

    uint8_t* ptr = grp->GetVertBufferData(auxSpan->fVBufferIdx);
    
    ptr += cell->fVtxStart + auxSpan->fCellOffset;

    return (plDecalVtxFormat*)ptr;

}

uint16_t* plDynaDecalMgr::IGetBaseIdxPtr(const plAuxSpan* auxSpan) const
{
    plGBufferGroup* grp = auxSpan->fGroup;

    return grp->GetIndexBufferData(auxSpan->fIBufferIdx) + auxSpan->fIBufferInit;
}

bool plDynaDecalMgr::IConvertFlatGrid(plAuxSpan* auxSpan,
                                        plDynaDecal* decal,
                                        const plFlatGridMesh& grid) const
{
    plDecalVtxFormat* vtx = IGetBaseVtxPtr(auxSpan);
    vtx += decal->fStartVtx;
    decal->fVtxBase = vtx;

    hsPoint3* origPos = &auxSpan->fOrigPos[decal->fStartVtx];
    hsPoint3* origUVW = &auxSpan->fOrigUVW[decal->fStartVtx];

    uint32_t initColor = decal->fFlags & plDynaDecal::kAttenColor
        ? 0xff000000
        : 0x00ffffff;
    int iv;
    for( iv = 0; iv < decal->fNumVerts; iv++ )
    {
        *origPos = vtx->fPos = grid.fVerts[iv].fPos;

        vtx->fNorm.Set(0.f, 0.f, 1.f);

        vtx->fDiffuse = initColor;
        vtx->fSpecular = 0;

        vtx->fUVW[0] = grid.fVerts[iv].fUVW;
        vtx->fUVW[1].Set(0.f, 0.f, 0.f);
        *origUVW = grid.fVerts[iv].fUVW;
        origUVW->fZ = 1.f;

        vtx++;
        origPos++;
        origUVW++;
    }

    uint16_t* idx = IGetBaseIdxPtr(auxSpan);
    idx += decal->fStartIdx;

    hsAssert(grid.fIdx.size() == decal->fNumIdx, "Mismatch on dynamic indices");

    uint16_t base = decal->fStartVtx;
    for (uint16_t ii : grid.fIdx)
    {
        hsAssert(ii + base - decal->fStartVtx < decal->fNumVerts, "Index going out of range");
        hsAssert(ii + base < auxSpan->fIStartIdx + auxSpan->fILength, "Index going out of range.");

        *idx++ = ii + base;
    }

    auxSpan->fGroup->DirtyVertexBuffer(auxSpan->fVBufferIdx);
    auxSpan->fGroup->DirtyIndexBuffer(auxSpan->fIBufferIdx);

    return true;
}

void plDynaDecalMgr::ISetDepthFalloff()
{
    const float totalDepth = fCutter->GetLengthW();

    // Currently all constants, but these could be set per DecalMgr.
    plConst(float) kMinFeet(3.f);
    plConst(float) kMaxFeet(10.f);

    plConst(float) kMinDepth(0.25f);
    plConst(float) kMaxDepth(0.75f);

    fMinDepth = kMinFeet / totalDepth;
    if( fMinDepth > kMinDepth )
        fMinDepth = kMinDepth;

    fMinDepthRange = 1.f / fMinDepth;

    fMaxDepth = 1.f - (kMaxFeet / totalDepth);
    if( fMaxDepth < kMaxDepth )
        fMaxDepth = kMaxDepth;

    fMaxDepthRange = 1.f / (1.f - fMaxDepth);
}

bool plDynaDecalMgr::IConvertPolys(plAuxSpan* auxSpan,
                                   plDynaDecal* decal,
                                   std::vector<plCutoutPoly>& src)
{
    ISetDepthFalloff();

    if( decal->fFlags & plDynaDecal::kVertexShader )
        return IConvertPolysVS(auxSpan, decal, src);

    if( decal->fFlags & plDynaDecal::kAttenColor )
        return IConvertPolysColor(auxSpan, decal, src);

    return IConvertPolysAlpha(auxSpan, decal, src);
}

bool plDynaDecalMgr::IConvertPolysAlpha(plAuxSpan* auxSpan,
                                        plDynaDecal* decal,
                                        std::vector<plCutoutPoly>& src)
{
    bool loU = false;
    bool hiU = false;
    bool loV = false;
    bool hiV = false;
    plDecalVtxFormat* vtx = IGetBaseVtxPtr(auxSpan);
    vtx += decal->fStartVtx;
    decal->fVtxBase = vtx;

    hsPoint3* origPos = &auxSpan->fOrigPos[decal->fStartVtx];
    hsPoint3* origUVW = &auxSpan->fOrigUVW[decal->fStartVtx];

    const hsVector3 backDir = fCutter->GetBackDir();

    size_t iPoly = 0;
    size_t iVert = 0;
    for (uint16_t iv = 0; iv < decal->fNumVerts; iv++)
    {
        *origPos = vtx->fPos = src[iPoly].fVerts[iVert].fPos;

        vtx->fNorm = src[iPoly].fVerts[iVert].fNorm;
        vtx->fUVW[0] = src[iPoly].fVerts[iVert].fUVW;

        if( vtx->fUVW[0].fX < 0.5f )
            loU = true;
        else 
            hiU = true;
        if( vtx->fUVW[0].fY < 0.5f )
            loV = true;
        else
            hiV = true;

        hsColorRGBA col = src[iPoly].fVerts[iVert].fColor;

        float depth = vtx->fUVW[0].fZ;

        float opac = depth < fMinDepth
            ? depth * fMinDepthRange
            : depth > fMaxDepth
                ? (1.f - depth) * fMaxDepthRange
                : 1.f;

        float normOpac = 1.f - vtx->fNorm.InnerProduct(backDir);
        opac *= 1.f - normOpac * normOpac;
        if( opac < 0 )
            opac = 0;

        if( src[iPoly].fBaseHasAlpha )
            opac *= col.a;
        col.a = 0;

        origUVW->fX = vtx->fUVW[0].fX;
        origUVW->fY = vtx->fUVW[0].fY;

        origUVW->fZ = opac;
        vtx->fUVW[1].Set(0, 0, 0);

        vtx->fDiffuse = col.ToARGB32();
        vtx->fSpecular = 0;


        if (++iVert >= src[iPoly].fVerts.size())
        {
            iVert = 0;
            iPoly++;
        }
        vtx++;
        origPos++;
        origUVW++;
    }
    hsAssert(vtx <= IGetBaseVtxPtr(auxSpan) + auxSpan->fVBufferLimit, "Vtx pointer gone wild");

    uint16_t* idx = IGetBaseIdxPtr(auxSpan);
    idx += decal->fStartIdx;

    uint16_t base = decal->fStartVtx;
    for (const plCutoutPoly& poly : src)
    {
        uint16_t next = base+1;
        for (size_t k = 2; k < poly.fVerts.size(); k++)
        {
            *idx++ = base;
            *idx++ = next++;
            *idx++ = next;
        }
        base = ++next;
    }
    hsAssert(idx <= auxSpan->fGroup->GetIndexBufferData(auxSpan->fIBufferIdx) + auxSpan->fIBufferLimit, "Index ptr gone wild");

    auxSpan->fGroup->DirtyVertexBuffer(auxSpan->fVBufferIdx);
    auxSpan->fGroup->DirtyIndexBuffer(auxSpan->fIBufferIdx);

    return loU & hiU & loV & hiV;
}

bool plDynaDecalMgr::IConvertPolysColor(plAuxSpan* auxSpan,
                                        plDynaDecal* decal,
                                        std::vector<plCutoutPoly>& src)
{
    bool loU = false;
    bool hiU = false;
    bool loV = false;
    bool hiV = false;
    plDecalVtxFormat* vtx = IGetBaseVtxPtr(auxSpan);
    vtx += decal->fStartVtx;
    decal->fVtxBase = vtx;

    hsPoint3* origPos = &auxSpan->fOrigPos[decal->fStartVtx];
    hsPoint3* origUVW = &auxSpan->fOrigUVW[decal->fStartVtx];

    const hsVector3 backDir = fCutter->GetBackDir();
    size_t iPoly = 0;
    size_t iVert = 0;
    for (uint16_t iv = 0; iv < decal->fNumVerts; iv++)
    {
        *origPos = vtx->fPos = src[iPoly].fVerts[iVert].fPos;

        vtx->fNorm = src[iPoly].fVerts[iVert].fNorm;
        vtx->fUVW[0] = src[iPoly].fVerts[iVert].fUVW;

        if( vtx->fUVW[0].fX < 0.5f )
            loU = true;
        else 
            hiU = true;
        if( vtx->fUVW[0].fY < 0.5f )
            loV = true;
        else
            hiV = true;

        float depth = vtx->fUVW[0].fZ;
        float opac = depth < fMinDepth
            ? depth * fMinDepthRange
            : depth > fMaxDepth
                ? (1.f - depth) * fMaxDepthRange
                : 1.f;

        float normOpac = 1.f - vtx->fNorm.InnerProduct(backDir);
        opac *= 1.f - normOpac * normOpac;
        if( opac < 0 )
            opac = 0;

        origUVW->fX = vtx->fUVW[0].fX;
        origUVW->fY = vtx->fUVW[0].fY;

        origUVW->fZ = opac;
        vtx->fUVW[1].Set(0, 0, 0);

        vtx->fDiffuse = 0xff000000;
        vtx->fSpecular = 0;


        if (++iVert >= src[iPoly].fVerts.size())
        {
            iVert = 0;
            iPoly++;
        }
        vtx++;
        origPos++;
        origUVW++;
    }
    hsAssert(vtx <= IGetBaseVtxPtr(auxSpan) + auxSpan->fVBufferLimit, "Vtx pointer gone wild");

    uint16_t* idx = IGetBaseIdxPtr(auxSpan);
    idx += decal->fStartIdx;

    uint16_t base = decal->fStartVtx;
    for (const plCutoutPoly& poly : src)
    {
        uint16_t next = base+1;
        for (size_t k = 2; k < poly.fVerts.size(); k++)
        {
            *idx++ = base;
            *idx++ = next++;
            *idx++ = next;
        }
        base = ++next;
    }
    hsAssert(idx <= auxSpan->fGroup->GetIndexBufferData(auxSpan->fIBufferIdx) + auxSpan->fIBufferLimit, "Index ptr gone wild");

    auxSpan->fGroup->DirtyVertexBuffer(auxSpan->fVBufferIdx);
    auxSpan->fGroup->DirtyIndexBuffer(auxSpan->fIBufferIdx);

    return loU & hiU & loV & hiV;
}

bool plDynaDecalMgr::IConvertPolysVS(plAuxSpan* auxSpan,
                                     plDynaDecal* decal,
                                     std::vector<plCutoutPoly>& src)
{
    bool loU = false;
    bool hiU = false;
    bool loV = false;
    bool hiV = false;
    plDecalVtxFormat* vtx = IGetBaseVtxPtr(auxSpan);
    vtx += decal->fStartVtx;
    decal->fVtxBase = vtx;

    hsPoint3* origPos = &auxSpan->fOrigPos[decal->fStartVtx];
    hsPoint3* origUVW = &auxSpan->fOrigUVW[decal->fStartVtx];

    size_t iPoly = 0;
    size_t iVert = 0;
    for (uint16_t iv = 0; iv < decal->fNumVerts; iv++)
    {
        *origPos = vtx->fPos = src[iPoly].fVerts[iVert].fPos;

        vtx->fNorm = src[iPoly].fVerts[iVert].fNorm;
        vtx->fUVW[0] = src[iPoly].fVerts[iVert].fUVW;

        if( vtx->fUVW[0].fX < 0.5f )
            loU = true;
        else 
            hiU = true;
        if( vtx->fUVW[0].fY < 0.5f )
            loV = true;
        else
            hiV = true;

        origUVW->fX = vtx->fUVW[0].fX;
        origUVW->fY = vtx->fUVW[0].fY;

        origUVW->fZ = vtx->fUVW[0].fZ = (float)decal->fBirth;

        vtx->fUVW[1].Set(0, 0, 0);

        const hsColorRGBA& col = src[iPoly].fVerts[iVert].fColor;
        vtx->fDiffuse = col.ToARGB32();
        vtx->fSpecular = 0;


        if (++iVert >= src[iPoly].fVerts.size())
        {
            iVert = 0;
            iPoly++;
        }
        vtx++;
        origPos++;
        origUVW++;
    }
    hsAssert(vtx <= IGetBaseVtxPtr(auxSpan) + auxSpan->fVBufferLimit, "Vtx pointer gone wild");

    uint16_t* idx = IGetBaseIdxPtr(auxSpan);
    idx += decal->fStartIdx;

    uint16_t base = decal->fStartVtx;
    for (const plCutoutPoly& poly : src)
    {
        uint16_t next = base+1;
        for (size_t k = 2; k < poly.fVerts.size(); k++)
        {
            *idx++ = base;
            *idx++ = next++;
            *idx++ = next;
        }
        base = ++next;
    }
    hsAssert(idx <= auxSpan->fGroup->GetIndexBufferData(auxSpan->fIBufferIdx) + auxSpan->fIBufferLimit, "Index ptr gone wild");

    auxSpan->fGroup->DirtyVertexBuffer(auxSpan->fVBufferIdx);
    auxSpan->fGroup->DirtyIndexBuffer(auxSpan->fIBufferIdx);

    return loU & hiU & loV & hiV;
}

bool plDynaDecalMgr::IHitTestPolys(std::vector<plCutoutPoly>& src) const
{
    bool loU = false;
    bool hiU = false;
    bool loV = false;
    bool hiV = false;
    size_t iPoly = 0;
    size_t iVert = 0;
    while (iPoly < src.size())
    {
        const hsPoint3& uvw = src[iPoly].fVerts[iVert].fUVW;

        if( uvw.fX < 0.5f )
            loU = true;
        else 
            hiU = true;
        if( uvw.fY < 0.5f )
            loV = true;
        else
            hiV = true;

        if (++iVert >= src[iPoly].fVerts.size())
        {
            iVert = 0;
            iPoly++;
        }
    }

    return loU & hiU & loV & hiV;
}

bool plDynaDecalMgr::IProcessPolys(plDrawableSpans* targ, int iSpan, double t, std::vector<plCutoutPoly>& src)
{
    // Figure out how many verts and idxs are coming in.
    uint16_t numVerts, numIdx;
    ICountIncoming(src, numVerts, numIdx);
    if( !numVerts )
        return false;

    // Find a span to put them in. Either the current span, or a new
    // one if it's full up.
    plAuxSpan* auxSpan = IGetAuxSpan(targ, iSpan, nullptr, numVerts, numIdx);

    // If we're full up, just see if we hit anything, but don't 
    // make any more decals. Might be nice to accelerate decay
    // here, since we definitely aren't keeping up.
    if( !auxSpan )
        return IHitTestPolys(src);

    // Get a decal to manage this group's aging.
    // Update the span to point to enough room.
    plDynaDecal* decal = IInitDecal(auxSpan, t, numVerts, numIdx);

    // Convert the polys from src into the accessor tris
    return IConvertPolys(auxSpan, decal, src);
}

bool plDynaDecalMgr::IProcessGrid(plDrawableSpans* targ, int iSpan, hsGMaterial* mat, double t, const plFlatGridMesh& grid)
{
    // Find a span to put them in. Either the current span, or a new
    // one if it's full up.
    plAuxSpan* auxSpan = IGetAuxSpan(targ, iSpan, mat, grid.fVerts.size(), grid.fIdx.size());

    // If we're full up, just see if we hit anything, but don't 
    // make any more decals.
    if( !auxSpan )
        return IHitTestFlatGrid(grid);

    auxSpan->fFlags |= plAuxSpan::kWorldSpace;

    // Get a decal to manage this group's aging.
    // Update the span to point to enough room.
    plDynaDecal* decal = IInitDecal(auxSpan, t, grid.fVerts.size(), grid.fIdx.size());

    // Convert the grid from src into the accessor tris
    return IConvertFlatGrid(auxSpan, decal, grid);
}

bool plDynaDecalMgr::IHitTestFlatGrid(const plFlatGridMesh& grid) const
{
    return true;
}

//////////////////////////////////////////////////////////////////////////////////

bool plDynaDecalMgr::ICutoutGrid(plDrawableSpans* drawable, int iSpan, hsGMaterial* mat, double secs)
{
    static plFlatGridMesh grid;
    grid.Reset();

    int nWid = int(fCutter->GetLengthU() / fGridSizeU);
    int nLen = int(fCutter->GetLengthV() / fGridSizeV);

    fCutter->CutoutGrid(nWid, nLen, grid);

    return IProcessGrid(drawable, iSpan, mat, secs, grid);
}

bool plDynaDecalMgr::ICutoutObject(plSceneObject* so, double secs)
{
    if( fDisableAccumulate )
        return false;

    bool retVal = false;

    if( !so )
        return retVal;

    const plDrawInterface* di = so->GetDrawInterface();
    if( !di )
        return retVal;

    plProfile_BeginTiming(Total);
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
                    const plSpan* span = dr->GetSpan(diIndex[k]);
                    if( kVolumeCulled != fCutter->GetIsect().Test(span->fWorldBounds) )
                    {
                        plAccessSpan src;
                        plAccessGeometry::Instance()->OpenRO(dr, diIndex[k], src);

                        static std::vector<plCutoutPoly> dst;
                        dst.clear();

                        plProfile_BeginTiming(Cutter);
                        fCutter->Cutout(src, dst);
                        plProfile_EndTiming(Cutter);

                        plProfile_BeginTiming(Process);
                        if( IProcessPolys(dr, diIndex[k], secs, dst) )
                        {
                            plProfile_BeginTiming(Callback);
                            if( src.HasWaterHeight() )
                                ICutoutCallback(dst, true, src.GetWaterHeight());
                            else
                                ICutoutCallback(dst);
                            plProfile_EndTiming(Callback);

                            retVal = true;
                        }
                        plProfile_EndTiming(Process);

                        plAccessGeometry::Instance()->Close(src);
                    }
                }
            }
        }
    }
    plProfile_EndTiming(Total);
    return retVal;
}

bool plDynaDecalMgr::ICutoutList(std::vector<plDrawVisList>& drawVis, double secs)
{
    if( fDisableAccumulate )
        return false;

    bool retVal = false;

    if (drawVis.empty())
        return retVal;

    std::vector<plAccessSpan> src;

    size_t numSpan = 0;
    for (const plDrawVisList& dv : drawVis)
        numSpan += dv.fVisList.size();

    src.resize(numSpan);

    size_t iDraw = 0;
    size_t iSpan = 0;
    for (size_t i = 0; i < numSpan; i++)
    {
        static std::vector<plCutoutPoly> dst;
        dst.clear();

        plAccessGeometry::Instance()->OpenRO(drawVis[iDraw].fDrawable, drawVis[iDraw].fVisList[iSpan], src[i]);

        fCutter->Cutout(src[i], dst);

        if( IProcessPolys((plDrawableSpans*)drawVis[iDraw].fDrawable, drawVis[iDraw].fVisList[iSpan], secs, dst) )
            retVal = true;

        plAccessGeometry::Instance()->Close(src[i]);

        if (++iSpan >= drawVis[iDraw].fVisList.size())
        {
            iDraw++;
            iSpan = 0;
        }
    }
    return retVal;
}

bool plDynaDecalMgr::ICutoutTargets(double secs)
{
    if( fDisableAccumulate )
        return false;

    bool retVal = false;

    for (plSceneObject* target : fTargets)
    {
        if (target)
            retVal |= ICutoutObject(target, secs);
    }
    return retVal;
}

//////////////////////////////////////////////////////////////////////////////////

hsGMaterial* plDynaDecalMgr::IConvertToEnvMap(hsGMaterial* mat, plBitmap* envMap)
{
    if( !mat || !envMap )
        return nullptr;

    plLayerInterface* oldLay = mat->GetLayer(0);

    plMipmap* oldMip = plMipmap::ConvertNoRef(oldLay->GetTexture());
    if( !oldMip )
        return mat;
    oldMip->SetCurrLevel(0);

    hsGMaterial* newMat = new hsGMaterial;
    ST::string buff = ST::format("{}_EnvMat", GetKey()->GetName());
    hsgResMgr::ResMgr()->NewKey(buff, newMat, GetKey()->GetUoid().GetLocation());

    static plTweak<float> kSmooth(1.f);
    plMipmap* bumpMap = plBumpMapGen::QikNormalMap(nullptr, oldMip, 0xffffffff, plBumpMapGen::kBubbleTest, kSmooth);
//  plMipmap* bumpMap = plBumpMapGen::QikNormalMap(nullptr, oldMip, 0xffffffff, plBumpMapGen::kNormalize, kSmooth);
//  plMipmap* bumpMap = plBumpMapGen::QikNormalMap(nullptr, oldMip, 0xffffffff, 0, 0);
    buff = ST::format("{}_BumpMap", GetKey()->GetName());
    hsgResMgr::ResMgr()->NewKey(buff, bumpMap, GetKey()->GetUoid().GetLocation());

    bumpMap->SetFlags(bumpMap->GetFlags() | plMipmap::kBumpEnvMap | plMipmap::kForceNonCompressed);

    plLayer* bumpLay = new plLayer;
    buff = ST::format("{}_BumpMap_0", GetKey()->GetName());
    hsgResMgr::ResMgr()->NewKey(buff, bumpLay, GetKey()->GetUoid().GetLocation());

    bumpLay->SetState(oldLay->GetState());
    bumpLay->SetBlendFlags(hsGMatState::kBlendAdd | hsGMatState::kBlendEnvBumpNext);
    bumpLay->SetTransform(hsMatrix44::IdentityMatrix());
    bumpLay->SetUVWSrc(0);

    hsMatrix44 bumpEnvXfm;
    bumpEnvXfm.Reset();
    bumpLay->SetBumpEnvMatrix(bumpEnvXfm);

    bumpLay->SetAmbientColor(oldLay->GetAmbientColor());
    bumpLay->SetRuntimeColor(oldLay->GetRuntimeColor());
    bumpLay->SetOpacity(1.f);

    plLayRefMsg* refMsg = new plLayRefMsg(bumpLay->GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kTexture);
    hsgResMgr::ResMgr()->SendRef(bumpMap->GetKey(), refMsg, plRefFlags::kActiveRef);

    newMat->AddLayerViaNotify(bumpLay);

    plLayer* envLay = new plLayer;
    buff = ST::format("{}_EnvMap_0", GetKey()->GetName());
    hsgResMgr::ResMgr()->NewKey(buff, envLay, GetKey()->GetUoid().GetLocation());

    envLay->SetBlendFlags(hsGMatState::kBlendMult);
    envLay->SetClampFlags(0);
    envLay->SetShadeFlags(bumpLay->GetShadeFlags());
    envLay->SetZFlags(hsGMatState::kZNoZWrite);
    envLay->SetMiscFlags(hsGMatState::kMiscUseReflectionXform);
    envLay->SetUVWSrc(plLayer::kUVWReflect);

    envLay->SetAmbientColor(oldLay->GetAmbientColor());
    envLay->SetRuntimeColor(oldLay->GetRuntimeColor());
    envLay->SetOpacity(1.f);

    refMsg = new plLayRefMsg(envLay->GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kTexture);
    hsgResMgr::ResMgr()->SendRef(envMap->GetKey(), refMsg, plRefFlags::kActiveRef);

    newMat->AddLayerViaNotify(envLay);

    return newMat;
}

void plDynaDecalMgr::ConvertToEnvMap(plBitmap* envMap)
{
    hsGMaterial* newPreShade = IConvertToEnvMap(fMatPreShade, envMap);
    if( newPreShade && (newPreShade != fMatPreShade) )
        hsgResMgr::ResMgr()->SendRef(newPreShade->GetKey(), new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefMatPreShade), plRefFlags::kActiveRef);
    
    hsGMaterial* newRTShade = IConvertToEnvMap(fMatRTShade, envMap);
    if( newRTShade && (newRTShade != fMatRTShade) )
        hsgResMgr::ResMgr()->SendRef(newRTShade->GetKey(), new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefMatRTShade), plRefFlags::kActiveRef);
}

const plMipmap* plDynaDecalMgr::GetMipmap() const
{
    plMipmap* mip = nullptr;
    if( fMatRTShade )
        mip = plMipmap::ConvertNoRef(fMatRTShade->GetLayer(0)->GetTexture());

    if( !mip && fMatPreShade )
        mip = plMipmap::ConvertNoRef(fMatPreShade->GetLayer(0)->GetTexture());

    if( mip )
        mip->SetCurrLevel(0);

    return mip;
}

hsVector3 plDynaDecalMgr::IRandomUp(hsVector3 dir) const
{
    static plRandom sRand;

    hsVector3 retVal;

    // Okay, we want a pretty random vector perpindicular to the
    // direction of fire. So we take that direction and cross product
    // it with the 3 world axes. Now we have 3 random vectors perpindicular
    // to the dir (but not necessarily each other). Note that some (but not all)
    // of these vectors may be zero length, it the direction happens to line up
    // with an axis. We scale each by a random amount, sum them up, and since
    // they are all perpindicular to dir, the weighted sum is too.
    // Only problem here is that our random scalings might wind us up with
    // a zero vector. Unlikely, which means almost certain to happen. So
    // we keep trying till we get a non-zero vector.
    float lenSq(-1.f);
    do {
        float ranXx = sRand.RandMinusOneToOne();
        float ranXy = sRand.RandMinusOneToOne();
        float ranXz = sRand.RandMinusOneToOne();
        retVal.fX = -dir.fZ * ranXy + dir.fY * ranXz;
        retVal.fY = dir.fZ * ranXx + -dir.fX * ranXz;
        retVal.fZ = -dir.fY * ranXx + dir.fX * ranXy;

        lenSq = retVal.MagnitudeSquared();

    } while( lenSq <= 0 );

    retVal *= hsFastMath::InvSqrtAppr(lenSq);

    return retVal;
}

hsVector3 plDynaDecalMgr::IReflectDir(hsVector3 dir) const
{
    hsFastMath::NormalizeAppr(dir); // it's been interpolated.

    // parm of zero returns unaffected dir, parm of one returns cutter direction reflected about dir.
    // Here's the original math.
    // Here N is dir, B is -cutter back direction (incoming), k is parm.
    // Reflection R of B is 2*(N dot B)*N + B
    // Interpolating gives K*R + (1-K)*N
    // Simplifying gives (2*K*(N dot B) + (1-K)) * N + K*B
    // Or something.

    plConst(float) parm(0.5f);

    hsVector3 b = -fCutter->GetBackDir();

    float t = dir.InnerProduct(b);
    t *= -2.f * parm;
    t += (1.f - parm);

    hsVector3 ret = dir * t;

    ret += b * parm;

    return ret;
}

hsMatrix44 plDynaDecalMgr::IL2WFromHit(hsPoint3 pos, hsVector3 dir) const
{
    dir = IReflectDir(dir);

    // Negate the firing direction before constructing our psys l2w, because
    // particles fire in the negative Z direction.
    dir = -dir;

    hsVector3 up = IRandomUp(dir);
    hsVector3 acc = up % dir;

    hsMatrix44 l2w;
    l2w.Reset();
    l2w.fMap[0][0] = acc[0];
    l2w.fMap[1][0] = acc[1];
    l2w.fMap[2][0] = acc[2];

    l2w.fMap[0][1] = up[0];
    l2w.fMap[1][1] = up[1];
    l2w.fMap[2][1] = up[2];

    l2w.fMap[0][2] = dir[0];
    l2w.fMap[1][2] = dir[1];
    l2w.fMap[2][2] = dir[2];

    l2w.fMap[0][3] = pos[0];
    l2w.fMap[1][3] = pos[1];
    l2w.fMap[2][3] = pos[2];

    l2w.NotIdentity();

    return l2w;
}

void plDynaDecalMgr::ICutoutCallback(const std::vector<plCutoutPoly>& cutouts, bool hasWaterHeight, float waterHeight)
{
    std::vector<plCutoutHit> hits;

    if ((fPartyTime > 0.f) && !fParticles.empty())
    {
        if( hasWaterHeight )
            fCutter->FindHitPointsConstHeight(cutouts, hits, waterHeight);
        else
            fCutter->FindHitPoints(cutouts, hits);

        for (const plCutoutHit& hit : hits)
        {
            for (plParticleSystem* particleSys : fParticles)
            {
                plParticleEmitter* emit = particleSys->GetAvailEmitter();
                if( emit )
                {
                    hsMatrix44 l2w = IL2WFromHit(hit.fPos, hit.fNorm);

                    emit->OverrideLocalToWorld(l2w);
                    emit->SetTimeToLive(fPartyTime);
                }
            }
        }
    }
}

void plDynaDecalMgr::IGetParticles()
{
    if (fParticles.size() != fPartyObjects.size())
    {
        for (plSceneObject* partyObj : fPartyObjects)
        {
            const plParticleSystem *sys = plParticleSystem::ConvertNoRef(partyObj->GetModifierByType(plParticleSystem::Index()));
            if (sys && (std::find(fParticles.cbegin(), fParticles.cend(), sys) == fParticles.cend()))
            {
                hsgResMgr::ResMgr()->AddViaNotify(sys->GetKey(), new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefParticles), plRefFlags::kPassiveRef);
            }
        }
    }
}
