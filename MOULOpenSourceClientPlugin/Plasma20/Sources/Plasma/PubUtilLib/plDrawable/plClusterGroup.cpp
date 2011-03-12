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
#include "plClusterGroup.h"

#include "plSpanTemplate.h"
#include "plCluster.h"

#include "../pnMessage/plTimeMsg.h"

#include "../plScene/plVisMgr.h"
#include "../plScene/plVisRegion.h"

#include "../plSurface/hsGMaterial.h"

#include "../plGLight/plLightInfo.h"

#include "plDrawableSpans.h"

#include "hsBitVector.h"
#include "hsStream.h"
#include "hsResMgr.h"

//STUB
#include "plgDispatch.h"
#include "../plMessage/plAgeLoadedMsg.h"

plClusterGroup::plClusterGroup()
:	fSceneNode(nil),
	fDrawable(nil),
	fTemplate(nil),
	fMaterial(nil),
	fUnPacked(0)
{
	fVisSet.SetBit(plVisMgr::kNormal);
}

plClusterGroup::~plClusterGroup()
{
	int i;
	for( i = 0; i < fClusters.GetCount(); i++ )
		delete fClusters[i];

	delete fTemplate;
}

plCluster* plClusterGroup::IAddCluster()
{
	plCluster* cluster = TRACKED_NEW plCluster;
	// Set the cluster's group.
	cluster->SetGroup(this);
	fClusters.Append(cluster);
	return cluster;
}

plCluster* plClusterGroup::IGetCluster(int i) const
{ 
	return fClusters[i]; 
}

const plCluster* plClusterGroup::GetCluster(int i) const
{ 
	return fClusters[i]; 
}

void plClusterGroup::Read(hsStream* stream, hsResMgr* mgr)
{
	hsKeyedObject::Read(stream, mgr);

	int i;

	fTemplate = TRACKED_NEW plSpanTemplate;
	fTemplate->Read(stream);

	mgr->ReadKeyNotifyMe(stream, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, kRefMaterial), plRefFlags::kActiveRef);

	const int numClust = stream->ReadSwap32();
	fClusters.SetCount(numClust);
	for( i = 0; i < numClust; i++ )
	{
		fClusters[i] = TRACKED_NEW plCluster;
		fClusters[i]->Read(stream, this);
	}

	const int numRegions = stream->ReadSwap32();
	for( i = 0; i < numRegions; i++ )
		mgr->ReadKeyNotifyMe(stream, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, kRefRegion), plRefFlags::kActiveRef);

	const int numLights = stream->ReadSwap32();
	for( i = 0; i < numLights; i++ )
		mgr->ReadKeyNotifyMe(stream, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, kRefLight), plRefFlags::kActiveRef);

	fLOD.Read(stream);

	fRenderLevel.Set(stream->ReadSwap32());

	fSceneNode = mgr->ReadKey(stream);

	//STUB
	plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
}

void plClusterGroup::Write(hsStream* stream, hsResMgr* mgr)
{
	hsKeyedObject::Write(stream, mgr);

	int i;

	fTemplate->Write(stream);

	mgr->WriteKey(stream, fMaterial);

	stream->WriteSwap32(fClusters.GetCount());
	for( i = 0; i < fClusters.GetCount(); i++ )
		fClusters[i]->Write(stream);

	stream->WriteSwap32(fRegions.GetCount());
	for( i = 0; i < fRegions.GetCount(); i++ )
		mgr->WriteKey(stream, fRegions[i]);

	stream->WriteSwap32(fLights.GetCount());
	for( i = 0; i < fLights.GetCount(); i++ )
		mgr->WriteKey(stream, fLights[i]);

	fLOD.Write(stream);

	stream->WriteSwap32(fRenderLevel.Level());

	mgr->WriteKey(stream, fSceneNode);
}

void plClusterGroup::ISendToSelf(RefType t, hsKeyedObject* ref)
{
	hsAssert(ref, "Sending self a nil object");
	plGenRefMsg* refMsg = TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, t);
	hsgResMgr::ResMgr()->SendRef(ref->GetKey(), refMsg, plRefFlags::kActiveRef);
}

hsBool plClusterGroup::IAddVisRegion(plVisRegion* reg)
{
	if( reg )
	{
		int idx = fRegions.Find(reg);
		if( idx == fRegions.kMissingIndex )
		{
			fRegions.Append(reg);
			if( reg->GetProperty(plVisRegion::kIsNot) )
				fVisNot.SetBit(reg->GetIndex());
			else
			{
				fVisSet.SetBit(reg->GetIndex());
				if( reg->ReplaceNormal() )
					fVisSet.ClearBit(plVisMgr::kNormal);
			}
		}
	}
	return true;
}

hsBool plClusterGroup::IRemoveVisRegion(plVisRegion* reg)
{
	if( reg )
	{
		int idx = fRegions.Find(reg);
		if( fRegions.kMissingIndex != idx )
		{
			fRegions.Remove(idx);
			if( reg->GetProperty(plVisRegion::kIsNot) )
				fVisNot.ClearBit(reg->GetIndex());
			else
				fVisSet.ClearBit(reg->GetIndex());
		}
	}
	return true;
}

hsBool plClusterGroup::IAddLight(plLightInfo* li)
{
	int idx = fLights.Find(li);
	if( fLights.kMissingIndex == idx )
	{
		fLights.Append(li);
	}
	return true;
}

hsBool plClusterGroup::IRemoveLight(plLightInfo* li)
{
	int idx = fLights.Find(li);
	if( fLights.kMissingIndex != idx )
	{
		fLights.Remove(idx);
	}
	return true;
}

hsBool plClusterGroup::IOnReceive(plGenRefMsg* ref)
{
	switch( ref->fType )
	{
	case kRefMaterial:
		fMaterial = hsGMaterial::ConvertNoRef(ref->GetRef());
		return true;
	case kRefRegion:
		return IAddVisRegion(plVisRegion::ConvertNoRef(ref->GetRef()));
	case kRefLight:
		return IAddLight(plLightInfo::ConvertNoRef(ref->GetRef()));
	}
	return false;
}

hsBool plClusterGroup::IOnRemove(plGenRefMsg* ref)
{
	int idx = -1;
	switch( ref->fType )
	{
	case kRefMaterial:
		fMaterial = nil;
		return true;
	case kRefRegion:
		return IRemoveVisRegion(plVisRegion::ConvertNoRef(ref->GetRef()));
	case kRefLight:
		return IRemoveLight(plLightInfo::ConvertNoRef(ref->GetRef()));
	}
	return false;
}

hsBool plClusterGroup::IOnRef(plGenRefMsg* ref)
{
	if( ref->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
	{
		return IOnReceive(ref);
	}

	return IOnRemove(ref);
}

hsBool plClusterGroup::MsgReceive(plMessage* msg)
{
	plGenRefMsg* ref = plGenRefMsg::ConvertNoRef(msg);
	if( ref )
	{
		if( IOnRef(ref) )
			return true;
	}

	// STUB
	plEvalMsg* evalMsg = plEvalMsg::ConvertNoRef(msg);
	if (evalMsg)
	{
		UnPack();
		fUnPacked = true;
		plgDispatch::Dispatch()->UnRegisterForExactType(plEvalMsg::Index(), GetKey());
		return true;
	}

	return hsKeyedObject::MsgReceive(msg);
}

void plClusterGroup::UnPack()
{
	plDrawableSpans* drawable = TRACKED_NEW plDrawableSpans;
	fDrawable = hsgResMgr::ResMgr()->NewKey(GetKey()->GetName(), drawable, GetKey()->GetUoid().GetLocation());
	drawable->UnPackCluster(this);

	drawable->SetSceneNode(fSceneNode);
}

void plClusterGroup::SetVisible(bool visible)
{
	if (fDrawable)
	{
		plDrawableSpans *drawable = plDrawableSpans::ConvertNoRef(fDrawable->ObjectIsLoaded());
		if (drawable)
			drawable->SetProperty(0,!visible); // property 0 is the disable drawing property
	}
}

UInt32 plClusterGroup::NumInst() const
{
	UInt32 numInst = 0;
	int i;
	for( i = 0; i < fClusters.GetCount(); i++ )
		numInst += fClusters[i]->NumInsts();

	return numInst;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void plLODDist::Read(hsStream* s)
{
	fMinDist = s->ReadSwapScalar();
	fMaxDist = s->ReadSwapScalar();
}

void plLODDist::Write(hsStream* s) const
{
	s->WriteSwapScalar(fMinDist);
	s->WriteSwapScalar(fMaxDist);
}