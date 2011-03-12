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
#include "plSceneNode.h"
#include "../pnDispatch/plDispatch.h"
#include "../plMessage/plNodeCleanupMsg.h"
#include "../pnMessage/plNodeRefMsg.h"

#include "hsStream.h"
#include "hsResMgr.h"

#include "../pnSceneObject/plSceneObject.h"
#include "plDrawable.h"
#include "plPhysical.h"
#include "plAudible.h"
#include "../plGLight/plLightInfo.h"
#include "../pnMessage/plRefMsg.h"
#include "plPipeline.h"
#include "../pnKeyedObject/plKey.h"
#include "../plDrawable/plSpaceTreeMaker.h"
#include "../plDrawable/plSpaceTree.h"
#include "plPageTreeMgr.h"
#include "plOccluder.h"

//MFHORSE
//BLACK
// temp hack for debugging
#include "../plDrawable/plDrawableSpans.h"
#include "../pnKeyedObject/plKeyImp.h"

plSceneNode::plSceneNode()
:	fDepth(0),
	fSpaceTree(nil),
	fFilterGenerics(false)
{
}

plSceneNode::~plSceneNode()
{
	plgDispatch::Dispatch()->UnRegisterForExactType(plNodeCleanupMsg::Index(), GetKey());

	delete fSpaceTree;
}

//// Init ////////////////////////////////////////////////////////////////////
//	Because we can't register for messages on construction. Doh.

void plSceneNode::Init()
{
	///  :P
	plgDispatch::Dispatch()->RegisterForExactType(plNodeCleanupMsg::Index(), GetKey());
}

void plSceneNode::Read(hsStream* s, hsResMgr* mgr)
{
	hsKeyedObject::Read(s, mgr);

	UInt32 n;
	int i;

	n = s->ReadSwap32();
	fSceneObjects.Reset();
	for( i = 0; i < n; i++ )
	{
		plNodeRefMsg* refMsg = TRACKED_NEW plNodeRefMsg(GetKey(), plRefMsg::kOnCreate, i, plNodeRefMsg::kObject);
		plKey key = mgr->ReadKeyNotifyMe(s, refMsg, plRefFlags::kActiveRef);
	}

	n = s->ReadSwap32();
	fGenericPool.Reset();
	for( i = 0; i < n; i++ )
	{
		plNodeRefMsg* refMsg = TRACKED_NEW plNodeRefMsg(GetKey(), plRefMsg::kOnCreate, -1, plNodeRefMsg::kGeneric);
		mgr->ReadKeyNotifyMe(s, refMsg, plRefFlags::kActiveRef);
	}
}

void plSceneNode::Write(hsStream* s, hsResMgr* mgr)
{
	hsKeyedObject::Write(s, mgr);

	int i;

	s->WriteSwap32(fSceneObjects.GetCount());
	for( i = 0; i < fSceneObjects.GetCount(); i++ )
		mgr->WriteKey(s,fSceneObjects[i]);

	s->WriteSwap32(fGenericPool.GetCount());
	for( i = 0; i < fGenericPool.GetCount(); i++ )
		mgr->WriteKey(s, fGenericPool[i]);
}

void plSceneNode::Harvest(plVolumeIsect* isect, hsTArray<plDrawVisList>& levList)
{
	static hsTArray<Int16> visList;
	visList.SetCount(0);
	GetSpaceTree()->HarvestLeaves(isect, visList);
	static hsTArray<Int16> visSpans;
	visSpans.SetCount(0);

	int i;
	for( i = 0; i < visList.GetCount(); i++ )
	{
		int idx = visList[i];
		fDrawPool[idx]->GetSpaceTree()->HarvestLeaves(isect, visSpans);
		if( visSpans.GetCount() )
		{
			plDrawVisList* drawVis = levList.Push();
			drawVis->fDrawable = fDrawPool[idx];
			drawVis->fVisList.Swap(visSpans);
		}
	}
}

void plSceneNode::CollectForRender(plPipeline* pipe, hsTArray<plDrawVisList>& levList, plVisMgr* visMgr)
{
	static hsTArray<Int16> visList;
	visList.SetCount(0);
	pipe->HarvestVisible(GetSpaceTree(), visList);
	static hsTArray<Int16> visSpans;
	visSpans.SetCount(0);

	int i;
	for( i = 0; i < visList.GetCount(); i++ )
	{
		int idx = visList[i];
		if( pipe->PreRender(fDrawPool[idx], visSpans, visMgr) )
		{
			plDrawVisList* drawVis = levList.Push();
			drawVis->fDrawable = fDrawPool[idx];
			drawVis->fVisList.Swap(visSpans);
		}
	}
}

void plSceneNode::SubmitOccluders(plPageTreeMgr* pageMgr) const
{
	pageMgr->AddOccluderList(fOccluders);
}

plSpaceTree* plSceneNode::IBuildSpaceTree()
{
	plSpaceTreeMaker maker;
	maker.Reset();
	
	hsBounds3Ext bnd;
	bnd.Reset(&hsPoint3(0,0,0));
	
	int i;
	for( i = 0; i < fDrawPool.GetCount(); i++ )
	{
		if( fDrawPool[i] )
			maker.AddLeaf(fDrawPool[i]->GetSpaceTree()->GetWorldBounds());
		else
			maker.AddLeaf(bnd, true);
	}

	fSpaceTree = maker.MakeTree();
	fSpaceTree->MakeDirty();

	return fSpaceTree;
}

plSpaceTree* plSceneNode::ITrashSpaceTree()
{
	delete fSpaceTree;
	return fSpaceTree = nil;
}

void plSceneNode::IDirtySpaceTree()
{
	int i;
	for( i = 0; i < fDrawPool.GetCount(); i++ )
	{
		if( fDrawPool[i] && fDrawPool[i]->GetSpaceTree()->IsDirty() )
		{
			fDrawPool[i]->GetSpaceTree()->Refresh();
			fSpaceTree->MoveLeaf(i, fDrawPool[i]->GetSpaceTree()->GetWorldBounds());
		}
	}
}

plSpaceTree* plSceneNode::GetSpaceTree()
{
	if( !fSpaceTree )
	{
		IBuildSpaceTree();
	}
	IDirtySpaceTree();
	return fSpaceTree;
}

void plSceneNode::ISetDrawable(plDrawable* d)
{
	if( !d )
		return;

	if (fDrawPool.Find(d) == fDrawPool.kMissingIndex)
	{
		fDrawPool.Append(d);
	}

	ITrashSpaceTree();
}

void plSceneNode::ISetAudible(plAudible* a)
{
	if( !a )
		return;

	if( fAudioPool.kMissingIndex == fAudioPool.Find(a) )
	{
		fAudioPool.Append(a);
	}
}

void plSceneNode::ISetPhysical(plPhysical* p)
{
	if( !p )
		return;

	if( fSimulationPool.kMissingIndex == fSimulationPool.Find(p) )
	{
		fSimulationPool.Append(p);
	}
}

void plSceneNode::ISetObject(plSceneObject* o)
{
	if( o && (fSceneObjects.kMissingIndex == fSceneObjects.Find(o)) )
	{
		fSceneObjects.Append(o);

	// MF_NET_GROUPS_TEST
	// This will have no effect on members of NetGroupConstants
		o->SetNetGroup(o->SelectNetGroup(GetKey()));

		o->SetSceneNode(GetKey());
	}
}

void plSceneNode::ISetLight(plLightInfo* l)
{
	if( fLightPool.kMissingIndex == fLightPool.Find(l) )
		fLightPool.Append( l );

}

void plSceneNode::ISetOccluder(plOccluder* o)
{
	if( fOccluders.kMissingIndex == fOccluders.Find(o) )
	{
		fOccluders.Append(o);
	}
}

void plSceneNode::ISetGeneric(hsKeyedObject* k)
{
	if( fGenericPool.kMissingIndex == fGenericPool.Find(k) )
		fGenericPool.Append(k);
}

void plSceneNode::IRemoveDrawable(plDrawable* d)
{
	int idx = fDrawPool.Find(d);
	if( idx != fDrawPool.kMissingIndex )
		fDrawPool.Remove(idx);

	ITrashSpaceTree();
}

void plSceneNode::IRemoveAudible(plAudible* a)
{
	int idx = fAudioPool.Find(a);
	if( idx != fAudioPool.kMissingIndex )
		fAudioPool.Remove(idx);

}

void plSceneNode::IRemovePhysical(plPhysical* p)
{
	hsAssert(p, "Removing nil physical");

#ifdef HS_DEBUGGING
	if (p)
	{
		plKey oldNodeKey = p->GetSceneNode();
		if (oldNodeKey && oldNodeKey != GetKey())
		{
			char buf[256];
			sprintf(buf, "Trying to remove physical %s from scenenode %s,\nbut it's actually in %s",
				p->GetKeyName(), GetKeyName(), oldNodeKey->GetName());
			hsAssert(0, buf);
		}
	}
#endif

	int idx = fSimulationPool.Find(p);
	if( idx != fSimulationPool.kMissingIndex )
		fSimulationPool.Remove(idx);
}

void plSceneNode::IRemoveObject(plSceneObject* o)
{
	int idx = fSceneObjects.Find(o);
	if( idx != fSceneObjects.kMissingIndex )
		fSceneObjects.Remove(idx);
}

void plSceneNode::IRemoveLight(plLightInfo* l)
{
	hsAssert(l, "Removing nil light");

	int idx = fLightPool.Find(l);
	if( idx != fLightPool.kMissingIndex )
	{
		fLightPool.Remove(idx);
	}
}

void plSceneNode::IRemoveOccluder(plOccluder* o)
{
	int idx = fOccluders.Find(o);
	if( idx != fOccluders.kMissingIndex )
		fOccluders.Remove(idx);
}

void plSceneNode::IRemoveGeneric(hsKeyedObject* k)
{
	int idx = fGenericPool.Find(k);
	if( idx != fGenericPool.kMissingIndex )
		fGenericPool.Remove(idx);
}

hsBool plSceneNode::IOnRemove(plNodeRefMsg* refMsg)
{

	switch( refMsg->fType )
	{
	case plNodeRefMsg::kDrawable:
		IRemoveDrawable(plDrawable::ConvertNoRef(refMsg->GetRef()));
		break;
	case plNodeRefMsg::kPhysical:
		IRemovePhysical(plPhysical::ConvertNoRef(refMsg->GetRef()));
		break;
	case plNodeRefMsg::kAudible:
		IRemoveAudible(plAudible::ConvertNoRef(refMsg->GetRef()));
		break;
	case plNodeRefMsg::kObject:
		IRemoveObject(plSceneObject::ConvertNoRef(refMsg->GetRef()));
		break;
	case plNodeRefMsg::kLight:
		IRemoveLight(plLightInfo::ConvertNoRef(refMsg->GetRef()));
		break;
	case plNodeRefMsg::kOccluder:
		IRemoveOccluder(plOccluder::ConvertNoRef(refMsg->GetRef()));
		break;
	case plNodeRefMsg::kGeneric:
		IRemoveGeneric(refMsg->GetRef());
		break;
	}
	if( refMsg->GetRef() && (refMsg->GetContext() & plRefMsg::kOnRemove) )
		GetKey()->Release(refMsg->GetRef()->GetKey());
		
	return true;
}

hsBool plSceneNode::IOnAdd(plNodeRefMsg* refMsg)
{
	int which = refMsg->fWhich;

	switch( refMsg->fType )
	{
	case plNodeRefMsg::kDrawable:
		ISetDrawable(plDrawable::ConvertNoRef(refMsg->GetRef()));
		return true;
	case plNodeRefMsg::kPhysical:
		ISetPhysical(plPhysical::ConvertNoRef(refMsg->GetRef()));
		return true;
	case plNodeRefMsg::kAudible:
		ISetAudible(plAudible::ConvertNoRef(refMsg->GetRef()));
		return true;
	case plNodeRefMsg::kObject:
		ISetObject(plSceneObject::ConvertNoRef(refMsg->GetRef()));
		return true;
	case plNodeRefMsg::kLight:
		ISetLight(plLightInfo::ConvertNoRef(refMsg->GetRef()));
		return true;
	case plNodeRefMsg::kOccluder:
		ISetOccluder(plOccluder::ConvertNoRef(refMsg->GetRef()));
		return true;
	case plNodeRefMsg::kGeneric:
		ISetGeneric(refMsg->GetRef());
	}
	return true;
}

hsBool plSceneNode::MsgReceive(plMessage* msg)
{
	plNodeCleanupMsg	*cleanMsg = plNodeCleanupMsg::ConvertNoRef( msg );

	if( cleanMsg )
	{
		ICleanUp();
		return true;
	}

	plNodeRefMsg* refMsg = plNodeRefMsg::ConvertNoRef(msg);
	
	if( refMsg )
	{
		if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
			return IOnAdd(refMsg);
		else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
			return IOnRemove(refMsg);

	}

	return hsKeyedObject::MsgReceive(msg);
}

//// ICleanUp ////////////////////////////////////////////////////////////////
//	Export only: Clean up the scene node (i.e. make sure drawables optimize)

void	plSceneNode::ICleanUp( void )
{
	int		i;


	/// Go find drawables to delete
	for( i = 0; i < fDrawPool.GetCount(); i++ )
		fDrawPool[ i ]->Optimize();

	if (fFilterGenerics)
	{
		for( i = fSceneObjects.GetCount() - 1; i >= 0; i--)
			GetKey()->Release(fSceneObjects[i]->GetKey());
		for( i = fDrawPool.GetCount() - 1; i >= 0; i--)
			GetKey()->Release(fDrawPool[i]->GetKey());
		for( i = fSimulationPool.GetCount() - 1; i >= 0; i--)
			GetKey()->Release(fSimulationPool[i]->GetKey());
		for( i = fAudioPool.GetCount() - 1; i >= 0; i--)
			GetKey()->Release(fAudioPool[i]->GetKey());
		for( i = fOccluders.GetCount() - 1; i >= 0; i--)
			GetKey()->Release(fOccluders[i]->GetKey());
		for( i = fLightPool.GetCount() - 1; i >= 0; i--)
			GetKey()->Release(fLightPool[i]->GetKey());
	}

	ITrashSpaceTree();
}

//// GetMatchingDrawable /////////////////////////////////////////////////////
//	Export only: Query for a given drawable.

plDrawable	*plSceneNode::GetMatchingDrawable( const plDrawableCriteria& crit )
{
	int		i;


	for( i = 0; i < fDrawPool.GetCount(); i++ )
	{
		if( fDrawPool[ i ]->DoIMatch( crit ) )
			return fDrawPool[ i ];
	}

	return nil;
}

//// OptimizeDrawables ///////////////////////////////////////////////////////
//	Loops through all the drawables and calls Optimize on each one. For the
//	export side, to be called right before writing the drawables to disk.

void	plSceneNode::OptimizeDrawables( void )
{
	int		i;


	for( i = 0; i < fDrawPool.GetCount(); i++ )
		fDrawPool[ i ]->Optimize();
}

