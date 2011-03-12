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
#include "plDynaDecalMgr.h"
#include "plDynaDecal.h"

#include "plCutter.h"

#include "plAccessGeometry.h"
#include "plAccessSpan.h"

#include "plDrawableSpans.h"
#include "plAuxSpan.h"
#include "plSpaceTree.h"

#include "plPrintShape.h"

#include "../plAvatar/plArmatureMod.h"

#include "../plParticleSystem/plParticleSystem.h"
#include "../plParticleSystem/plParticleEmitter.h"

#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plDrawInterface.h"
#include "../plSurface/hsGMaterial.h"
#include "../plSurface/plLayerInterface.h"
#include "../plScene/plPageTreeMgr.h"

#include "../plPipeline/plGBufferGroup.h"
#include "../plPipeline/hsGDeviceRef.h"

#include "../plMessage/plAgeLoadedMsg.h"
#include "../plMessage/plDynaDecalEnableMsg.h"
#include "../pnMessage/plRefMsg.h"
#include "../pnMessage/plTimeMsg.h"
#include "plgDispatch.h"

#include "../plMath/plRandom.h"
#include "hsFastMath.h"

#include "hsStream.h"
#include "hsResMgr.h"
#include "hsTimer.h"

#include "../pnMessage/plPipeResMakeMsg.h"

// Stuff for creating a bumpenv decal on demand.
#include "../plGImage/plMipmap.h"
#include "../plSurface/plLayer.h"
#include "../plMessage/plLayRefMsg.h"

//### Hackage
#include "../plMessage/plRenderMsg.h"
#include "../plMessage/plListenerMsg.h"
#include "plPipeline.h"

#include "plTweak.h"

#include "plProfile.h"

plProfile_CreateTimerNoReset("Total", "DynaDecal", Total);
plProfile_CreateTimerNoReset("Cutter", "DynaDecal", Cutter);
plProfile_CreateTimerNoReset("Process", "DynaDecal", Process);
plProfile_CreateTimerNoReset("Callback", "DynaDecal", Callback);

static plRandom sRand;
static const int	kBinBlockSize = 20;
static const UInt16 kDefMaxNumVerts = 1000;
static const UInt16 kDefMaxNumIdx = kDefMaxNumVerts;

static const hsScalar kDefLifeSpan = 30.f;
static const hsScalar kDefDecayStart = kDefLifeSpan * 0.5f;
static const hsScalar kDefRampEnd = kDefLifeSpan * 0.1f;

static const hsScalar kInitAuxSpans = 5;

#define MF_NO_INIT_ALLOC
#define MF_NEVER_RUN_OUT

// If we aren't doing an initial alloc, we MUST have NEVER_RUN_OUT
// It's also useful to not have initial alloc but do have NEVER_RUN_OUT
// So:
//	MF_NO_INIT_ALLOC && MF_NEVER_RUN_OUT - Okay
//	!MF_NO_INIT_ALLOC && !MF_NEVER_RUN_OUT - Okay
//	!MF_NO_INIT_ALLOC && MF_NEVER_RUN_OUT - Okay
//	MF_NO_INIT_ALLOC && !MF_NEVER_RUN_OUT - Bad (you'll never get any decals)
#if defined(MF_NO_INIT_ALLOC) && !defined(MF_NEVER_RUN_OUT)
#define MF_NEVER_RUN_OUT
#endif // defined(MF_NO_INIT_ALLOC) && !defined(MF_NEVER_RUN_OUT)

using namespace std;

hsBool plDynaDecalMgr::fDisableAccumulate = false;
hsBool plDynaDecalMgr::fDisableUpdate = false;

plDynaDecalMgr::plDynaDecalMgr()
:	
	fMatPreShade(nil),
	fMatRTShade(nil),
	fMaxNumVerts(kDefMaxNumVerts),
	fMaxNumIdx(kDefMaxNumIdx),
	fWetLength(0),
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
	fCutter = TRACKED_NEW plCutter;
}

plDynaDecalMgr::~plDynaDecalMgr()
{
	int i;
	for( i = 0; i < fDecals.GetCount(); i++ )
		delete fDecals[i];

	for( i = 0; i < fAuxSpans.GetCount(); i++ )
	{
		if( fAuxSpans[i]->fDrawable )
		{
			plSpan* span = const_cast<plSpan*>(fAuxSpans[i]->fDrawable->GetSpan(fAuxSpans[i]->fBaseSpanIdx));

			span->RemoveAuxSpan(fAuxSpans[i]);
		}

		delete fAuxSpans[i]->fGroup;

		delete fAuxSpans[i];
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

	mgr->ReadKeyNotifyMe(stream, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefMatPreShade), plRefFlags::kActiveRef);

	mgr->ReadKeyNotifyMe(stream, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefMatRTShade), plRefFlags::kActiveRef);

	int n = stream->ReadSwap32();
	int i;
	for( i = 0; i < n; i++ )
	{
		mgr->ReadKeyNotifyMe(stream, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefTarget), plRefFlags::kPassiveRef);
	}
	// Associated slave particle systems. We read in the scene objects now, and find the associated systems on loaded message.
	n = stream->ReadSwap32();
	for( i = 0; i < n; i++ )
	{
		mgr->ReadKeyNotifyMe(stream, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefPartyObject), plRefFlags::kPassiveRef);
	}

	fMaxNumVerts = (UInt16)(stream->ReadSwap32());
	fMaxNumIdx = (UInt16)(stream->ReadSwap32());

	fWaitOnEnable = stream->ReadSwap32();

	fIntensity = stream->ReadSwapScalar();
	fInitAtten = fIntensity;

	fWetLength = stream->ReadSwapScalar();
	fRampEnd = stream->ReadSwapScalar();
	fDecayStart = stream->ReadSwapScalar();
	fLifeSpan = stream->ReadSwapScalar();

	fGridSizeU = stream->ReadSwapScalar();
	fGridSizeV = stream->ReadSwapScalar();

	fScale.Read(stream);

	fPartyTime = stream->ReadSwapScalar();

	n = stream->ReadSwap32();
	fNotifies.SetCount(n);
	for( i = 0; i < n; i++ )
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

	stream->WriteSwap32(fTargets.GetCount());

	int i;
	for( i = 0; i < fTargets.GetCount(); i++ )
	{
		mgr->WriteKey(stream, fTargets[i]);
	}

	// Particle systems (really their associated sceneobjects).
	stream->WriteSwap32(fPartyObjects.GetCount());
	for( i = 0; i < fPartyObjects.GetCount(); i++ )
	{
		mgr->WriteKey(stream, fPartyObjects[i]);
	}

	stream->WriteSwap32(fMaxNumVerts);
	stream->WriteSwap32(fMaxNumIdx);

	stream->WriteSwap32(fWaitOnEnable);

	stream->WriteSwapScalar(fIntensity);

	stream->WriteSwapScalar(fWetLength);
	stream->WriteSwapScalar(fRampEnd);
	stream->WriteSwapScalar(fDecayStart);
	stream->WriteSwapScalar(fLifeSpan);

	stream->WriteSwapScalar(fGridSizeU);
	stream->WriteSwapScalar(fGridSizeV);

	fScale.Write(stream);

	stream->WriteSwapScalar(fPartyTime);

	stream->WriteSwap32(fNotifies.GetCount());
	for( i = 0; i < fNotifies.GetCount(); i++ )
		mgr->WriteKey(stream, fNotifies[i]);

	/////////////////////////////////////////////////////
	// ###Things that should be in derived classes follow.

}

hsBool plDynaDecalMgr::IMakeAuxRefs(plPipeline* pipe)
{
	int i;
	for( i = 0; i < fGroups.GetCount(); i++ )
		fGroups[i]->PrepForRendering(pipe, false);

	return true;
}

const plPrintShape* plDynaDecalMgr::IGetPrintShape(const plKey& objKey) const
{
	const plPrintShape* shape = nil;
	plSceneObject* part = plSceneObject::ConvertNoRef(objKey->ObjectIsLoaded());
	if( part )
	{
		// This is a safe cast, because GetGenericInterface(type) will only return
		// either a valid object of that type, or nil.
		shape = static_cast<plPrintShape*>(part->GetGenericInterface(plPrintShape::Index()));
	}
	return shape;
}

const plPrintShape* plDynaDecalMgr::IGetPrintShape(plArmatureMod* avMod, UInt32 id) const
{
	const plPrintShape* shape = nil;
	const plSceneObject* part = avMod->FindBone(id);
	if( part )
	{
		// This is a safe cast, because GetGenericInterface(type) will only return
		// either a valid object of that type, or nil.
		shape = static_cast<plPrintShape*>(part->GetGenericInterface(plPrintShape::Index()));
	}
	return shape;
}

hsBool plDynaDecalMgr::IHandleEnableMsg(const plDynaDecalEnableMsg* enaMsg)
{
	IWetParts(enaMsg);

	return true;
}

hsBool plDynaDecalMgr::IWetParts(const plDynaDecalEnableMsg* enaMsg)
{
	if( !enaMsg->IsArmature() )
	{
		const plPrintShape* shape = IGetPrintShape(enaMsg->GetShapeKey());
		if( shape )
		{
			plDynaDecalInfo& info = IGetDecalInfo(UInt32(shape), shape->GetKey());
			IWetInfo(info, enaMsg);
		}
	}
	else
	if( enaMsg->GetID() == UInt32(-1) )
	{
		plArmatureMod* avMod = plArmatureMod::ConvertNoRef(enaMsg->GetArmKey()->ObjectIsLoaded());
		int i;
		for( i = 0; i < fPartIDs.GetCount(); i++ )
		{
			const plPrintShape* shape = IGetPrintShape(avMod, fPartIDs[i]);
			if( shape )
			{
				plDynaDecalInfo& info = IGetDecalInfo(UInt32(shape), shape->GetKey());
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

hsBool plDynaDecalMgr::IWetPart(UInt32 id, const plDynaDecalEnableMsg* enaMsg)
{
	plArmatureMod* avMod = plArmatureMod::ConvertNoRef(enaMsg->GetArmKey()->ObjectIsLoaded());

	const plPrintShape* shape = IGetPrintShape(avMod, id);
	if( shape )
	{
		plDynaDecalInfo& info = IGetDecalInfo(UInt32(shape), shape->GetKey());
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

hsBool plDynaDecalMgr::MsgReceive(plMessage* msg)
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
				fMatPreShade = nil;
			return true;
		case kRefMatRTShade:
			if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
				fMatRTShade = hsGMaterial::ConvertNoRef(refMsg->GetRef());
			else
				fMatRTShade = nil;
			return true;
		case kRefTarget:
			if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
			{
				fTargets.Append(plSceneObject::ConvertNoRef(refMsg->GetRef()));
			}
			else
			{
				int idx = fTargets.Find((plSceneObject*)refMsg->GetRef());
				if( idx != fTargets.kMissingIndex )
					fTargets.Remove(idx);
			}
			return true;
		case kRefPartyObject:
			if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
			{
				fPartyObjects.Append(plSceneObject::ConvertNoRef(refMsg->GetRef()));
			}
			else
			{
				int idx = fPartyObjects.Find((plSceneObject*)refMsg->GetRef());
				if( idx != fPartyObjects.kMissingIndex )
					fPartyObjects.Remove(idx);
			}
			return true;
		case kRefParticles:
			if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
			{
				fParticles.Append(plParticleSystem::ConvertNoRef(refMsg->GetRef()));
				fParticles[fParticles.GetCount()-1]->fMiscFlags |= plParticleSystem::kParticleSystemAlwaysUpdate;
			}
			else
			{
				int idx = fParticles.Find((plParticleSystem*)refMsg->GetRef());
				if( idx != fParticles.kMissingIndex )
					fParticles.Remove(idx);
			}
			return true;
		case kRefAvatar:
			if( refMsg->GetContext() & (plRefMsg::kOnRemove|plRefMsg::kOnDestroy) )
				IRemoveDecalInfo(UInt32(refMsg->GetRef()));
			return true;
		}
	}

	return plSynchedObject::MsgReceive(msg);
}

//////////////////////////////////////////////////////////////////////////////////
//
void plDynaDecalMgr::INotifyActive(plDynaDecalInfo& info, const plKey& armKey, UInt32 id) const
{
	if( !(info.fFlags & plDynaDecalInfo::kActive) )
	{
		double secs = hsTimer::GetSysSeconds();
		int i;
		for( i = 0; i < fNotifies.GetCount(); i++ )
		{
			plDynaDecalEnableMsg* enaMsg = TRACKED_NEW plDynaDecalEnableMsg(fNotifies[i], armKey, secs, fWetLength, false, id);
			enaMsg->Send();
		}
		info.fFlags |= plDynaDecalInfo::kActive;
	}
}

void plDynaDecalMgr::INotifyInactive(plDynaDecalInfo& info, const plKey& armKey, UInt32 id) const
{
	if( info.fFlags & plDynaDecalInfo::kActive )
	{
		double secs = hsTimer::GetSysSeconds();
		int i;
		for( i = 0; i < fNotifies.GetCount(); i++ )
		{
			plDynaDecalEnableMsg* enaMsg = TRACKED_NEW plDynaDecalEnableMsg(fNotifies[i], armKey, secs, fWetLength, true, id);
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

plDynaDecalInfo& plDynaDecalMgr::IGetDecalInfo(UInt32 id, const plKey& key)
{
	plDynaDecalMap::iterator iter = fDecalMap.find(id);
	if( iter == fDecalMap.end() )
	{
		plDynaDecalInfo decalInfo;
		decalInfo.Init(key);
		plGenRefMsg* refMsg = TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kRefAvatar);
		hsgResMgr::ResMgr()->AddViaNotify(plKey(key), refMsg, plRefFlags::kPassiveRef);

		pair<plDynaDecalMap::iterator, bool> iterPair;
		iterPair = fDecalMap.insert(plDynaDecalMap::value_type(id, decalInfo));
		iter = iterPair.first;
	}

	return iter->second;
}

void plDynaDecalMgr::IRemoveDecalInfo(UInt32 id)
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

hsScalar plDynaDecalMgr::IHowWet(plDynaDecalInfo& info, double t) const
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
	hsScalar wet = (hsScalar)(1.f - (t - info.fWetTime) / info.fWetLength);
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

plAuxSpan* plDynaDecalMgr::IGetAuxSpan(plDrawableSpans* targ, int iSpan, hsGMaterial* mat, UInt16 numVerts, UInt16 numIdx)
{
	// Some of this code just assumes you get the number of verts you ask for.
	// Which was causing errors when you asked for more than the max and didn't
	// get it. So now if you ask for too many verts, you just lose.
	if (numVerts > fMaxNumVerts || numIdx > fMaxNumIdx)
		return nil;

	plSpan* span = const_cast<plSpan*>(targ->GetSpan(iSpan));

	int i;

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
	for( i = 0; i < span->GetNumAuxSpans(); i++ )
	{
		plAuxSpan* aux = span->GetAuxSpan(i);
		if( (aux->fOwner == (void*)this)
			&&(aux->fVStartIdx + aux->fVLength + numVerts < aux->fVBufferLimit)
			&&(aux->fIStartIdx + aux->fILength + numIdx < aux->fIBufferLimit) )
			return aux;
	}

	hsBool rtLit = span->fProps & plSpan::kLiteVtxNonPreshaded;

	// Now look to see if we've got one sitting around unused that's suitable.
	// Here the suitable criteria is a little different. We know we are the owner,
	// and we know there's enough room (because it's sitting idle).
	for( i = 0; i < fAuxSpans.GetCount(); i++ )
	{
		plAuxSpan* aux = fAuxSpans[i];
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
	//		A) we'll need to flush managed memory to load it in.
	//		B) we'll be stuck with that memory (video and system) used up until this age is paged out and reloaded
	//		C) we've got a whole bunch of extra faces to draw.
	// If we just return nil:
	//		A) opposite of above
	//		B) we stop leaving footprints/ripples for a while.
	// I'm going to try the latter for a bit.

#ifdef MF_NEVER_RUN_OUT
	// Okay, nothing there. Let's get a new one.
	plAuxSpan* aux = TRACKED_NEW plAuxSpan;
	fAuxSpans.Append(aux);

	IAllocAuxSpan(aux, numVerts, numIdx);

	aux->fOwner = (void*)this;
	aux->fDrawable = targ;
	aux->fBaseSpanIdx = iSpan;

	ISetAuxMaterial(aux, mat, rtLit);

	span->AddAuxSpan(aux);

	return aux;
#else // MF_NEVER_RUN_OUT
	return nil;
#endif // MF_NEVER_RUN_OUT
}

void plDynaDecalMgr::InitAuxSpans()
{
	int i;
	for( i = 0; i < kInitAuxSpans; i++ )
	{
		plAuxSpan* aux = TRACKED_NEW plAuxSpan;
		fAuxSpans.Append(aux);
		IAllocAuxSpan(aux, fMaxNumVerts, fMaxNumIdx);
	}
}

void plDynaDecalMgr::IAllocAuxSpan(plAuxSpan* aux, UInt32 maxNumVerts, UInt32 maxNumIdx)
{
	int iGrp = fGroups.GetCount();
	plGBufferGroup* grp = TRACKED_NEW plGBufferGroup(kDecalVtxFormat, true, false);
	fGroups.Append(grp);

	grp->ReserveVertStorage(maxNumVerts, 
		&aux->fVBufferIdx, 
		&aux->fCellIdx, 
		&aux->fCellOffset, 
		plGBufferGroup::kReserveInterleaved);

	aux->fFlags = 0;

	aux->fVStartIdx = grp->GetVertStartFromCell(aux->fVBufferIdx, aux->fCellIdx, aux->fCellOffset);
	aux->fVLength = 0;

	UInt16* dataPtr = nil;
	grp->ReserveIndexStorage(maxNumIdx, &aux->fIBufferIdx, &aux->fIStartIdx, &dataPtr);
	aux->fIStartIdx;

	aux->fILength = 0;

	aux->fGroup = grp;

	aux->fVBufferInit = aux->fVStartIdx;
	aux->fVBufferLimit = aux->fVBufferInit + maxNumVerts;

	aux->fIBufferInit = aux->fIStartIdx;
	aux->fIBufferLimit = aux->fIBufferInit + maxNumIdx;

	aux->fOrigPos.SetCount(maxNumVerts);
	aux->fOrigUVW.SetCount(maxNumVerts);

	aux->fOwner = (void*)this;
	aux->fDrawable = nil;
	aux->fBaseSpanIdx = 0;

	grp->SetVertBufferStart(aux->fVBufferIdx, aux->fVStartIdx);
	grp->SetIndexBufferStart(aux->fIBufferIdx, aux->fIStartIdx);
	grp->SetVertBufferEnd(aux->fVBufferIdx, aux->fVStartIdx);
	grp->SetIndexBufferEnd(aux->fIBufferIdx, aux->fIStartIdx);
}

hsGMaterial* plDynaDecalMgr::ISetAuxMaterial(plAuxSpan* aux, hsGMaterial* mat, hsBool rtLit)
{
	if( !mat )
		mat = fMatRTShade;

	hsBool attenColor =  0 != (mat->GetLayer(0)->GetBlendFlags() 
								& (hsGMatState::kBlendAdd
									| hsGMatState::kBlendMult
									| hsGMatState::kBlendMADD));
	hsBool bump = 0 != (mat->GetLayer(0)->GetMiscFlags() & hsGMatState::kMiscBumpChans);
	hsBool hasVS = nil != mat->GetLayer(0)->GetVertexShader();

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

plDynaDecal* plDynaDecalMgr::IInitDecal(plAuxSpan* aux, double t, UInt16 numVerts, UInt16 numIdx)
{
	int idx = INewDecal();

	fDecals[idx]->fStartVtx = (UInt16)(aux->fVStartIdx + aux->fVLength);
	fDecals[idx]->fNumVerts = numVerts;
	fDecals[idx]->fStartIdx = (UInt16)(aux->fIStartIdx + aux->fILength);
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

void plDynaDecalMgr::IKillDecal(int i)
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

		aux->fDrawable = nil;
		aux->fBaseSpanIdx = 0;
	}

	delete fDecals[i];
	int newCount = fDecals.GetCount()-1;
	if( i < newCount )
	{
		memmove(&fDecals[i], &fDecals[i+1], (newCount-i) * sizeof(fDecals[i]));
	}
	fDecals.SetCount(newCount);
}

void plDynaDecalMgr::IUpdateDecals(double t)
{
	if( fDisableUpdate )
		return;

	int i;

	for( i = 0; i < fDecals.GetCount(); i++ )
	{
		if( fDecals[i]->Age(t, fRampEnd, fDecayStart, fLifeSpan) )
		{
			IKillDecal(i);
			i--;
		}
	}

	for( i = 0; i < fAuxSpans.GetCount(); i++ )
	{
		if( fAuxSpans[i]->fVLength )
		{
			plAuxSpan* aux = fAuxSpans[i];
			aux->fGroup->DirtyVertexBuffer(aux->fVBufferIdx);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////

void plDynaDecalMgr::ICountIncoming(hsTArray<plCutoutPoly>& src, UInt16& numVerts, UInt16& numIdx) const
{
	numVerts = 0;
	numIdx = 0;
	int j;
	for( j = 0; j < src.GetCount(); j++ )
	{
		if( src[j].fVerts.GetCount() )
		{
			numVerts += src[j].fVerts.GetCount();
			numIdx += src[j].fVerts.GetCount()-2;
		}
	}
	numIdx *= 3;
}

plDecalVtxFormat* plDynaDecalMgr::IGetBaseVtxPtr(const plAuxSpan* auxSpan) const
{
	plGBufferGroup* grp = auxSpan->fGroup;
	plGBufferCell* cell = grp->GetCell(auxSpan->fVBufferIdx, auxSpan->fCellIdx);

	UInt8* ptr = grp->GetVertBufferData(auxSpan->fVBufferIdx);
	
	ptr += cell->fVtxStart + auxSpan->fCellOffset;

	return (plDecalVtxFormat*)ptr;

}

UInt16* plDynaDecalMgr::IGetBaseIdxPtr(const plAuxSpan* auxSpan) const
{
	plGBufferGroup* grp = auxSpan->fGroup;

	return grp->GetIndexBufferData(auxSpan->fIBufferIdx) + auxSpan->fIBufferInit;
}

hsBool plDynaDecalMgr::IConvertFlatGrid(plAuxSpan* auxSpan,
										plDynaDecal* decal,
										const plFlatGridMesh& grid) const
{
	plDecalVtxFormat* vtx = IGetBaseVtxPtr(auxSpan);
	vtx += decal->fStartVtx;
	decal->fVtxBase = vtx;

	hsPoint3* origPos = &auxSpan->fOrigPos[decal->fStartVtx];
	hsPoint3* origUVW = &auxSpan->fOrigUVW[decal->fStartVtx];

	UInt32 initColor = decal->fFlags & plDynaDecal::kAttenColor
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

	UInt16* idx = IGetBaseIdxPtr(auxSpan);
	idx += decal->fStartIdx;

	hsAssert(grid.fIdx.GetCount() == decal->fNumIdx, "Mismatch on dynamic indices");

	UInt16 base = decal->fStartVtx;
	int ii;
	for( ii = 0; ii < grid.fIdx.GetCount(); ii++ )
	{
		hsAssert(grid.fIdx[ii] + base - decal->fStartVtx < decal->fNumVerts, "Index going out of range");
		hsAssert(grid.fIdx[ii] + base < auxSpan->fIStartIdx + auxSpan->fILength, "Index going out of range.");

		*idx++ = grid.fIdx[ii] + base;
	}

	auxSpan->fGroup->DirtyVertexBuffer(auxSpan->fVBufferIdx);
	auxSpan->fGroup->DirtyIndexBuffer(auxSpan->fIBufferIdx);

	return true;
}

void plDynaDecalMgr::ISetDepthFalloff()
{
	const hsScalar totalDepth = fCutter->GetLengthW();

	// Currently all constants, but these could be set per DecalMgr.
	plConst(hsScalar) kMinFeet(3.f);
	plConst(hsScalar) kMaxFeet(10.f);

	plConst(hsScalar) kMinDepth(0.25f);
	plConst(hsScalar) kMaxDepth(0.75f);

	fMinDepth = kMinFeet / totalDepth;
	if( fMinDepth > kMinDepth )
		fMinDepth = kMinDepth;

	fMinDepthRange = 1.f / fMinDepth;

	fMaxDepth = 1.f - (kMaxFeet / totalDepth);
	if( fMaxDepth < kMaxDepth )
		fMaxDepth = kMaxDepth;

	fMaxDepthRange = 1.f / (1.f - fMaxDepth);
}

hsBool plDynaDecalMgr::IConvertPolys(plAuxSpan* auxSpan,
								   plDynaDecal* decal, 
								   hsTArray<plCutoutPoly>& src)
{
	ISetDepthFalloff();

	if( decal->fFlags & plDynaDecal::kVertexShader )
		return IConvertPolysVS(auxSpan, decal, src);

	if( decal->fFlags & plDynaDecal::kAttenColor )
		return IConvertPolysColor(auxSpan, decal, src);

	return IConvertPolysAlpha(auxSpan, decal, src);
}

hsBool plDynaDecalMgr::IConvertPolysAlpha(plAuxSpan* auxSpan,
								   plDynaDecal* decal, 
								   hsTArray<plCutoutPoly>& src)
{
	hsBool loU = false;
	hsBool hiU = false;
	hsBool loV = false;
	hsBool hiV = false;
	plDecalVtxFormat* vtx = IGetBaseVtxPtr(auxSpan);
	vtx += decal->fStartVtx;
	decal->fVtxBase = vtx;

	hsPoint3* origPos = &auxSpan->fOrigPos[decal->fStartVtx];
	hsPoint3* origUVW = &auxSpan->fOrigUVW[decal->fStartVtx];

	const hsVector3 backDir = fCutter->GetBackDir();

	int iPoly = 0;
	int iVert = 0;
	int iv;
	for( iv = 0; iv < decal->fNumVerts; iv++ )
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

		hsScalar depth = vtx->fUVW[0].fZ;

		hsScalar opac = depth < fMinDepth
			? depth * fMinDepthRange
			: depth > fMaxDepth
				? (1.f - depth) * fMaxDepthRange
				: 1.f;

		hsScalar normOpac = 1.f - vtx->fNorm.InnerProduct(backDir);
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


		if( ++iVert >= src[iPoly].fVerts.GetCount() )
		{
			iVert = 0;
			iPoly++;
		}
		vtx++;
		origPos++;
		origUVW++;
	}
	hsAssert(vtx <= IGetBaseVtxPtr(auxSpan) + auxSpan->fVBufferLimit, "Vtx pointer gone wild");

	UInt16* idx = IGetBaseIdxPtr(auxSpan);
	idx += decal->fStartIdx;

	UInt16 base = decal->fStartVtx;
	int j;
	for( j = 0; j < src.GetCount(); j++ )
	{
		UInt16 next = base+1;
		int k;
		for( k = 2; k < src[j].fVerts.GetCount(); k++ )
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

hsBool plDynaDecalMgr::IConvertPolysColor(plAuxSpan* auxSpan,
								   plDynaDecal* decal, 
								   hsTArray<plCutoutPoly>& src)
{
	hsBool loU = false;
	hsBool hiU = false;
	hsBool loV = false;
	hsBool hiV = false;
	plDecalVtxFormat* vtx = IGetBaseVtxPtr(auxSpan);
	vtx += decal->fStartVtx;
	decal->fVtxBase = vtx;

	hsPoint3* origPos = &auxSpan->fOrigPos[decal->fStartVtx];
	hsPoint3* origUVW = &auxSpan->fOrigUVW[decal->fStartVtx];

	const hsVector3 backDir = fCutter->GetBackDir();
	int iPoly = 0;
	int iVert = 0;
	int iv;
	for( iv = 0; iv < decal->fNumVerts; iv++ )
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

		hsScalar depth = vtx->fUVW[0].fZ;
		hsScalar opac = depth < fMinDepth
			? depth * fMinDepthRange
			: depth > fMaxDepth
				? (1.f - depth) * fMaxDepthRange
				: 1.f;

		hsScalar normOpac = 1.f - vtx->fNorm.InnerProduct(backDir);
		opac *= 1.f - normOpac * normOpac;
		if( opac < 0 )
			opac = 0;

		origUVW->fX = vtx->fUVW[0].fX;
		origUVW->fY = vtx->fUVW[0].fY;

		origUVW->fZ = opac;
		vtx->fUVW[1].Set(0, 0, 0);

		vtx->fDiffuse = 0xff000000;
		vtx->fSpecular = 0;


		if( ++iVert >= src[iPoly].fVerts.GetCount() )
		{
			iVert = 0;
			iPoly++;
		}
		vtx++;
		origPos++;
		origUVW++;
	}
	hsAssert(vtx <= IGetBaseVtxPtr(auxSpan) + auxSpan->fVBufferLimit, "Vtx pointer gone wild");

	UInt16* idx = IGetBaseIdxPtr(auxSpan);
	idx += decal->fStartIdx;

	UInt16 base = decal->fStartVtx;
	int j;
	for( j = 0; j < src.GetCount(); j++ )
	{
		UInt16 next = base+1;
		int k;
		for( k = 2; k < src[j].fVerts.GetCount(); k++ )
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

hsBool plDynaDecalMgr::IConvertPolysVS(plAuxSpan* auxSpan,
								   plDynaDecal* decal, 
								   hsTArray<plCutoutPoly>& src)
{
	hsBool loU = false;
	hsBool hiU = false;
	hsBool loV = false;
	hsBool hiV = false;
	plDecalVtxFormat* vtx = IGetBaseVtxPtr(auxSpan);
	vtx += decal->fStartVtx;
	decal->fVtxBase = vtx;

	hsPoint3* origPos = &auxSpan->fOrigPos[decal->fStartVtx];
	hsPoint3* origUVW = &auxSpan->fOrigUVW[decal->fStartVtx];

	int iPoly = 0;
	int iVert = 0;
	int iv;
	for( iv = 0; iv < decal->fNumVerts; iv++ )
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

		origUVW->fZ = vtx->fUVW[0].fZ = (hsScalar)decal->fBirth;

		vtx->fUVW[1].Set(0, 0, 0);

		const hsColorRGBA& col = src[iPoly].fVerts[iVert].fColor;
		vtx->fDiffuse = col.ToARGB32();
		vtx->fSpecular = 0;


		if( ++iVert >= src[iPoly].fVerts.GetCount() )
		{
			iVert = 0;
			iPoly++;
		}
		vtx++;
		origPos++;
		origUVW++;
	}
	hsAssert(vtx <= IGetBaseVtxPtr(auxSpan) + auxSpan->fVBufferLimit, "Vtx pointer gone wild");

	UInt16* idx = IGetBaseIdxPtr(auxSpan);
	idx += decal->fStartIdx;

	UInt16 base = decal->fStartVtx;
	int j;
	for( j = 0; j < src.GetCount(); j++ )
	{
		UInt16 next = base+1;
		int k;
		for( k = 2; k < src[j].fVerts.GetCount(); k++ )
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

hsBool plDynaDecalMgr::IHitTestPolys(hsTArray<plCutoutPoly>& src) const
{
	hsBool loU = false;
	hsBool hiU = false;
	hsBool loV = false;
	hsBool hiV = false;
	int iPoly = 0;
	int iVert = 0;
	while( iPoly < src.GetCount() )
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

		if( ++iVert >= src[iPoly].fVerts.GetCount() )
		{
			iVert = 0;
			iPoly++;
		}
	}

	return loU & hiU & loV & hiV;
}

hsBool plDynaDecalMgr::IProcessPolys(plDrawableSpans* targ, int iSpan, double t, hsTArray<plCutoutPoly>& src)
{
	// Figure out how many verts and idxs are coming in.
	UInt16 numVerts, numIdx;
	ICountIncoming(src, numVerts, numIdx);
	if( !numVerts )
		return false;

	// Find a span to put them in. Either the current span, or a new
	// one if it's full up.
	plAuxSpan* auxSpan = IGetAuxSpan(targ, iSpan, nil, numVerts, numIdx);

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

hsBool plDynaDecalMgr::IProcessGrid(plDrawableSpans* targ, int iSpan, hsGMaterial* mat, double t, const plFlatGridMesh& grid)
{
	// Find a span to put them in. Either the current span, or a new
	// one if it's full up.
	plAuxSpan* auxSpan = IGetAuxSpan(targ, iSpan, mat, grid.fVerts.GetCount(), grid.fIdx.GetCount());

	// If we're full up, just see if we hit anything, but don't 
	// make any more decals.
	if( !auxSpan )
		return IHitTestFlatGrid(grid);

	auxSpan->fFlags |= plAuxSpan::kWorldSpace;

	// Get a decal to manage this group's aging.
	// Update the span to point to enough room.
	plDynaDecal* decal = IInitDecal(auxSpan, t, grid.fVerts.GetCount(), grid.fIdx.GetCount());

	// Convert the grid from src into the accessor tris
	return IConvertFlatGrid(auxSpan, decal, grid);
}

hsBool plDynaDecalMgr::IHitTestFlatGrid(const plFlatGridMesh& grid) const
{
	return true;
}

//////////////////////////////////////////////////////////////////////////////////

hsBool plDynaDecalMgr::ICutoutGrid(plDrawableSpans* drawable, int iSpan, hsGMaterial* mat, double secs)
{
	static plFlatGridMesh grid;
	grid.Reset();

	int nWid = int(fCutter->GetLengthU() / fGridSizeU);
	int nLen = int(fCutter->GetLengthV() / fGridSizeV);

	fCutter->CutoutGrid(nWid, nLen, grid);

	return IProcessGrid(drawable, iSpan, mat, secs, grid);
}

hsBool plDynaDecalMgr::ICutoutObject(plSceneObject* so, double secs)
{
	if( fDisableAccumulate )
		return false;

	hsBool retVal = false;

	if( !so )
		return retVal;

	const plDrawInterface* di = so->GetDrawInterface();
	if( !di )
		return retVal;

	plProfile_BeginTiming(Total);
	int numGot = 0;
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
					const plSpan* span = dr->GetSpan(diIndex[k]);
					if( kVolumeCulled != fCutter->GetIsect().Test(span->fWorldBounds) )
					{
						plAccessSpan src;
						plAccessGeometry::Instance()->OpenRO(dr, diIndex[k], src);

						static hsTArray<plCutoutPoly> dst;
						dst.SetCount(0);

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

hsBool plDynaDecalMgr::ICutoutList(hsTArray<plDrawVisList>& drawVis, double secs)
{
	if( fDisableAccumulate )
		return false;

	hsBool retVal = false;

	if( !drawVis.GetCount() )
		return retVal;

	hsTArray<plAccessSpan> src;

	int numSpan = 0;
	int iDraw;
	for( iDraw = 0; iDraw < drawVis.GetCount(); iDraw++ )
		numSpan += drawVis[iDraw].fVisList.GetCount();

	src.SetCount(numSpan);

	int i;

	iDraw = 0;
	int iSpan = 0;
	for( i = 0; i < numSpan; i++ )
	{
		static hsTArray<plCutoutPoly> dst;
		dst.SetCount(0);

		plAccessGeometry::Instance()->OpenRO(drawVis[iDraw].fDrawable, drawVis[iDraw].fVisList[iSpan], src[i]);

		fCutter->Cutout(src[i], dst);

		if( IProcessPolys((plDrawableSpans*)drawVis[iDraw].fDrawable, drawVis[iDraw].fVisList[iSpan], secs, dst) )
			retVal = true;

		plAccessGeometry::Instance()->Close(src[i]);

		if( ++iSpan >= drawVis[iDraw].fVisList.GetCount() )
		{
			iDraw++;
			iSpan = 0;
		}
	}
	return retVal;
}

hsBool plDynaDecalMgr::ICutoutTargets(double secs)
{
	if( fDisableAccumulate )
		return false;

	hsBool retVal = false;

	int i;
	for( i = 0; i < fTargets.GetCount(); i++ )
	{
		if( fTargets[i] )
			retVal |= ICutoutObject(fTargets[i], secs);
	}
	return retVal;
}

//////////////////////////////////////////////////////////////////////////////////

#include "../plGImage/plBumpMapGen.h"

hsGMaterial* plDynaDecalMgr::IConvertToEnvMap(hsGMaterial* mat, plBitmap* envMap)
{
	if( !mat || !envMap )
		return nil;

	plLayerInterface* oldLay = mat->GetLayer(0);

	plMipmap* oldMip = plMipmap::ConvertNoRef(oldLay->GetTexture());
	if( !oldMip )
		return mat;
	oldMip->SetCurrLevel(0);

	hsGMaterial* newMat = TRACKED_NEW hsGMaterial;
	char buff[256];
	sprintf(buff, "%s_%s", GetKey()->GetName(), "EnvMat");
	hsgResMgr::ResMgr()->NewKey(buff, newMat, GetKey()->GetUoid().GetLocation());

	static plTweak<hsScalar> kSmooth(1.f);
	plMipmap* bumpMap = plBumpMapGen::QikNormalMap(nil, oldMip, 0xffffffff, plBumpMapGen::kBubbleTest, kSmooth);
//	plMipmap* bumpMap = plBumpMapGen::QikNormalMap(nil, oldMip, 0xffffffff, plBumpMapGen::kNormalize, kSmooth);
//	plMipmap* bumpMap = plBumpMapGen::QikNormalMap(nil, oldMip, 0xffffffff, 0, 0);
	sprintf(buff, "%s_%s", GetKey()->GetName(), "BumpMap");
	hsgResMgr::ResMgr()->NewKey(buff, bumpMap, GetKey()->GetUoid().GetLocation());

	bumpMap->SetFlags(bumpMap->GetFlags() | plMipmap::kBumpEnvMap | plMipmap::kForceNonCompressed);

	plLayer* bumpLay = TRACKED_NEW plLayer;
	sprintf(buff, "%s_%s_%d", GetKey()->GetName(), "BumpMap", 0);
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

	plLayRefMsg* refMsg = TRACKED_NEW plLayRefMsg(bumpLay->GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kTexture);
	hsgResMgr::ResMgr()->SendRef(bumpMap->GetKey(), refMsg, plRefFlags::kActiveRef);

	newMat->AddLayerViaNotify(bumpLay);

	plLayer* envLay = TRACKED_NEW plLayer;
	sprintf(buff, "%s_%s_%d", GetKey()->GetName(), "EnvMap", 0);
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

	refMsg = TRACKED_NEW plLayRefMsg(envLay->GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kTexture);
	hsgResMgr::ResMgr()->SendRef(envMap->GetKey(), refMsg, plRefFlags::kActiveRef);

	newMat->AddLayerViaNotify(envLay);

	return newMat;
}

void plDynaDecalMgr::ConvertToEnvMap(plBitmap* envMap)
{
	hsGMaterial* newPreShade = IConvertToEnvMap(fMatPreShade, envMap);
	if( newPreShade && (newPreShade != fMatPreShade) )
		hsgResMgr::ResMgr()->SendRef(newPreShade->GetKey(), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefMatPreShade), plRefFlags::kActiveRef);
	
	hsGMaterial* newRTShade = IConvertToEnvMap(fMatRTShade, envMap);
	if( newRTShade && (newRTShade != fMatRTShade) )
		hsgResMgr::ResMgr()->SendRef(newRTShade->GetKey(), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefMatRTShade), plRefFlags::kActiveRef);
}

const plMipmap* plDynaDecalMgr::GetMipmap() const
{
	plMipmap* mip = nil;
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
	hsScalar lenSq(-1.f);
	do {
		hsScalar ranXx = sRand.RandMinusOneToOne();
		hsScalar ranXy = sRand.RandMinusOneToOne();
		hsScalar ranXz = sRand.RandMinusOneToOne();
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

	plConst(hsScalar) parm(0.5f);

	hsVector3 b = -fCutter->GetBackDir();

	hsScalar t = dir.InnerProduct(b);
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

void plDynaDecalMgr::ICutoutCallback(const hsTArray<plCutoutPoly>& cutouts, hsBool hasWaterHeight, hsScalar waterHeight)
{
	hsTArray<plCutoutHit> hits;

	if( (fPartyTime > 0) && fParticles.GetCount() )
	{
		if( hasWaterHeight )
			fCutter->FindHitPointsConstHeight(cutouts, hits, waterHeight);
		else
			fCutter->FindHitPoints(cutouts, hits);

		int i;
		for( i = 0; i < hits.GetCount(); i++ )
		{
			int j;
			for( j = 0; j < fParticles.GetCount(); j++ )
			{
				plParticleEmitter* emit = fParticles[j]->GetAvailEmitter();
				if( emit )
				{
					hsMatrix44 l2w = IL2WFromHit(hits[i].fPos, hits[i].fNorm);

					emit->OverrideLocalToWorld(l2w);
					emit->SetTimeToLive(fPartyTime);
				}
			}
		}
	}
}

void plDynaDecalMgr::IGetParticles()
{
	if( fParticles.GetCount() != fPartyObjects.GetCount() )
	{
		int i;
		for( i = 0; i < fPartyObjects.GetCount(); i++ )
		{
			const plParticleSystem *sys = plParticleSystem::ConvertNoRef(fPartyObjects[i]->GetModifierByType(plParticleSystem::Index()));
			// const_cast here is just to see if it's in our list, make Find happy.
			if( sys && (fParticles.kMissingIndex == fParticles.Find(const_cast<plParticleSystem*>(sys))) )
			{
				hsgResMgr::ResMgr()->AddViaNotify(sys->GetKey(), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefParticles), plRefFlags::kPassiveRef);
			}
		}
	}
}
