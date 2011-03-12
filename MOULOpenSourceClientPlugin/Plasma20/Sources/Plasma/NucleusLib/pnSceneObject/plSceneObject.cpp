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
#include "plSceneObject.h"
#include "plDrawInterface.h"
#include "plSimulationInterface.h"
#include "plCoordinateInterface.h"
#include "plAudioInterface.h"
#include "../pnDispatch/plDispatch.h"
#include "../pnModifier/plModifier.h"
#include "../pnMessage/plMessage.h"
#include "../pnMessage/plRefMsg.h"
#include "plDrawable.h"
#include "plPhysical.h"
#include "plAudible.h"
#include "../pnMessage/plTimeMsg.h"
#include "../pnMessage/plCorrectionMsg.h"
#include "../pnMessage/plWarpMsg.h"
#include "../pnMessage/plSoundMsg.h"
#include "../pnMessage/plEnableMsg.h"
#include "../pnMessage/plAttachMsg.h"
#include "../pnMessage/plObjRefMsg.h"
#include "../pnMessage/plNodeRefMsg.h"
#include "../pnMessage/plIntRefMsg.h"
#include "../pnMessage/plSimulationSynchMsg.h"
#include "../pnMessage/plSimulationMsg.h"
#include "../pnMessage/plNodeChangeMsg.h"
#include "../pnMessage/plSelfDestructMsg.h"
#include "../pnKeyedObject/plKey.h"

#include "hsStream.h"
#include "hsResMgr.h"
#include "plCreatableIndex.h" // For plLightInfo::Index(), so we don't have to include plLightInfo.h



int dbgCurrentTest = 0;

plSceneObject::plSceneObject()
:	fDrawInterface(nil),
	fSimulationInterface(nil),
	fCoordinateInterface(nil),
	fAudioInterface(nil),
	fSceneNode(nil)
{
}

plSceneObject::~plSceneObject()
{
	SetDrawInterface(nil);
	SetSimulationInterface(nil);
	SetCoordinateInterface(nil);
	SetAudioInterface(nil);

	IRemoveAllGenerics();

	int i;
	int knt;

	knt = fModifiers.GetCount();
	for( i = 0; i < knt; i++ )
	{
		if( fModifiers[i] )
			fModifiers[i]->RemoveTarget(this);
	}
}

void plSceneObject::Read(hsStream* stream, hsResMgr* mgr)
{
	plSynchedObject::Read(stream, mgr);

	// Interfaces will attach themselves to us on read.
	// DI
	mgr->ReadKeyNotifyMe(stream, TRACKED_NEW plObjRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface), plRefFlags::kActiveRef);

	// SI
	mgr->ReadKeyNotifyMe(stream, TRACKED_NEW plObjRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface), plRefFlags::kActiveRef);

	// CI
	mgr->ReadKeyNotifyMe(stream, TRACKED_NEW plObjRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface), plRefFlags::kActiveRef);

	// AI
	mgr->ReadKeyNotifyMe(stream, TRACKED_NEW plObjRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface), plRefFlags::kActiveRef);

	int i;

	int nGen = stream->ReadSwap32();
	fGenerics.SetCount(0);
	for( i = 0; i < nGen; i++ )
	{
		mgr->ReadKeyNotifyMe(stream, TRACKED_NEW plObjRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface), plRefFlags::kActiveRef);
	}

	plObjRefMsg* refMsg;

	int nOldMods=fModifiers.GetCount();		// existng modifiers created during interface loading
	int nNewMods = stream->ReadSwap32();
	fModifiers.ExpandAndZero(nOldMods+nNewMods);	// reserve space for new modifiers+existing modifiers
	for( i = nOldMods; i < nOldMods+nNewMods; i++ )
	{
		refMsg = TRACKED_NEW plObjRefMsg(GetKey(), plRefMsg::kOnCreate, i, plObjRefMsg::kModifier);
		mgr->ReadKeyNotifyMe(stream,refMsg, plRefFlags::kActiveRef);
	}

	plKey nodeKey = mgr->ReadKey(stream);
//	SetSceneNode(nodeKey);
	fSceneNode = nodeKey;

}

void plSceneObject::Write(hsStream* stream, hsResMgr* mgr)
{
	plSynchedObject::Write(stream, mgr);

	mgr->WriteKey(stream, fDrawInterface);
	mgr->WriteKey(stream, fSimulationInterface);
	mgr->WriteKey(stream, fCoordinateInterface);
	mgr->WriteKey(stream, fAudioInterface);

	int i;

	stream->WriteSwap32(fGenerics.GetCount());
	for( i = 0; i < fGenerics.GetCount(); i++ )
		mgr->WriteKey(stream, fGenerics[i]);

	for( i = fModifiers.GetCount() - 1; i >= 0; i--)
		if (fModifiers[i]->GetKey() == nil)
			RemoveModifier(fModifiers[i]);

	stream->WriteSwap32(fModifiers.GetCount());
	for( i = 0; i < fModifiers.GetCount(); i++ )
		mgr->WriteKey(stream,fModifiers[i]);

	mgr->WriteKey(stream, fSceneNode);
}

//// ReleaseData //////////////////////////////////////////////////////////////
//	Called by SceneViewer to release the data for this sceneObject (really
//	just a switchboard).

void	plSceneObject::ReleaseData( void )
{
	if( fDrawInterface )
		fDrawInterface->ReleaseData();
	if( fSimulationInterface )
		fSimulationInterface->ReleaseData();
	if( fCoordinateInterface )
		fCoordinateInterface->ReleaseData();
	if( fAudioInterface )
		fAudioInterface->ReleaseData();

	int i;
	for( i = 0; i < fGenerics.GetCount(); i++ )
	{
		if( fGenerics[i] )
			fGenerics[i]->ReleaseData();
	}
}

void plSceneObject::FlushTransform()
{
	if( fCoordinateInterface )
		fCoordinateInterface->FlushTransform();
}

#include "plProfile.h"

plProfile_CreateTimer("SOTrans", "Object", SOTrans);
plProfile_CreateTimer("   SODITrans", "Object", SODITrans);
plProfile_CreateTimer("   SOSITrans", "Object", SOSITrans);
plProfile_CreateTimer("   SOAITrans", "Object", SOAITrans);
plProfile_CreateTimer("   SOGITrans", "Object", SOGITrans);
plProfile_CreateTimer("   SOMOTrans", "Object", SOMOTrans);


void plSceneObject::ISetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	plProfile_BeginTiming(SOTrans);
	plProfile_BeginTiming(SODITrans);
	if( fDrawInterface )
		fDrawInterface->SetTransform(l2w, w2l);
	plProfile_EndTiming(SODITrans);

	plProfile_BeginTiming(SOSITrans);
	if( fSimulationInterface )
	{
		if(fCoordinateInterface)
		{
			UInt16 whyTransformed = fCoordinateInterface->GetReasons();
			if(whyTransformed != plCoordinateInterface::kReasonPhysics)
			{
				// if we were transformed by anything but physics, let physics know
				// otherwise we're not even going to tell physics
				fSimulationInterface->SetTransform(l2w, w2l);
			} else {
				int moreSunshine = 10;
			}
			fCoordinateInterface->ClearReasons();
		} else {
			int somethingToBreakOn = 10;
			// if there's not coordinate interface, there's no reason to move the simulation interface
		}
	}
	plProfile_EndTiming(SOSITrans);

	plProfile_BeginTiming(SOAITrans);
	if( fAudioInterface )
		fAudioInterface->SetTransform(l2w, w2l);
	plProfile_EndTiming(SOAITrans);

	plProfile_BeginTiming(SOGITrans);
	int i;
	for( i = 0; i < fGenerics.GetCount(); i++ )
	{
		if( fGenerics[i] )
			fGenerics[i]->SetTransform(l2w, w2l);
	}
	plProfile_EndTiming(SOGITrans);


	plProfile_BeginTiming(SOMOTrans);
	for( i = 0; i < fModifiers.GetCount(); i++ )
	{
		if( fModifiers[i] )
			fModifiers[i]->SetTransform(l2w, w2l);
	}
	plProfile_EndTiming(SOMOTrans);
	plProfile_EndTiming(SOTrans);
}

void plSceneObject::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	if( fCoordinateInterface )
		fCoordinateInterface->SetTransform(l2w, w2l);
}

hsMatrix44 plSceneObject::GetLocalToWorld() const
{
	hsMatrix44 l2w;
	if( fCoordinateInterface )
		l2w = fCoordinateInterface->GetLocalToWorld();
	else
		l2w.Reset();
	return l2w;
}

hsMatrix44 plSceneObject::GetWorldToLocal() const
{
	hsMatrix44 w2l;
	if( fCoordinateInterface )
		w2l = fCoordinateInterface->GetWorldToLocal();
	else
		w2l.Reset();
	return w2l;
}

hsMatrix44 plSceneObject::GetLocalToParent() const
{
	hsMatrix44 l2p;
	if( fCoordinateInterface )
		l2p = fCoordinateInterface->GetLocalToParent();
	else
		l2p.Reset();
	return l2p;
}

hsMatrix44 plSceneObject::GetParentToLocal() const
{
	hsMatrix44 p2l;
	if( fCoordinateInterface )
		p2l = fCoordinateInterface->GetParentToLocal();
	else
		p2l.Reset();
	return p2l;
}

void plSceneObject::IAddModifier(plModifier* mo, int i)
{
	if( !mo )
		return;

	if( i < 0 )
		i = fModifiers.GetCount();
	fModifiers.ExpandAndZero(i+1);
	fModifiers.Set(i, mo);
	mo->AddTarget(this);
}

void plSceneObject::IRemoveModifier(plModifier* mo)
{
	if( !mo )
		return;

	int idx = fModifiers.Find(mo);
	if( idx != fModifiers.kMissingIndex )
	{
		mo->RemoveTarget(this);
		fModifiers.Remove(idx);
	}
}

void plSceneObject::ISetInterface(plObjInterface* iface)
{
	hsAssert(iface, "Setting nil interface");
	if( plDrawInterface::ConvertNoRef(iface) )
		ISetDrawInterface(plDrawInterface::ConvertNoRef(iface));
	else if( plSimulationInterface::ConvertNoRef(iface) )
		ISetSimulationInterface(plSimulationInterface::ConvertNoRef(iface));
	else if( plCoordinateInterface::ConvertNoRef(iface) )
		ISetCoordinateInterface(plCoordinateInterface::ConvertNoRef(iface));
	else if( plAudioInterface::ConvertNoRef(iface) )
		ISetAudioInterface(plAudioInterface::ConvertNoRef(iface));
	else
		IAddGeneric(iface);

	if( iface )
		iface->ISetSceneNode(GetSceneNode());
}

void plSceneObject::IRemoveInterface(Int16 idx, plObjInterface* who)
{
	if( plFactory::DerivesFrom(plDrawInterface::Index(), idx) )
		ISetDrawInterface(nil);
	else if( plFactory::DerivesFrom(plSimulationInterface::Index(), idx) )
		ISetSimulationInterface(nil);
	else if( plFactory::DerivesFrom(plCoordinateInterface::Index(), idx) )
		ISetCoordinateInterface(nil);
	else if( plFactory::DerivesFrom(plAudioInterface::Index(), idx) )
		ISetAudioInterface(nil);
	else
		IRemoveGeneric(who);
}

hsBool plSceneObject::IPropagateToModifiers(plMessage* msg)
{
	hsBool retVal = false;
	int i;
	int nMods = fModifiers.GetCount();

	for( i = 0; i < nMods; i++ )
	{
		if( fModifiers[i] )
		{
			plModifier *mod = fModifiers[i];
			hsBool modRet = mod->MsgReceive(msg);
			retVal |= modRet;
		}
	}
	return retVal;
}

hsBool plSceneObject::Eval(double secs, hsScalar delSecs)
{
	UInt32 dirty = ~0L;
	hsBool retVal = false;
	int i;
	for( i = 0; i < fModifiers.GetCount(); i++ )
	{
		if( fModifiers[i] )
			retVal |= fModifiers[i]->IEval(secs, delSecs, dirty);
	}
	return retVal;
}

void plSceneObject::SetSceneNode(plKey newNode)
{
	plKey curNode=GetSceneNode();
	if( curNode == newNode )
		return;
	if( fDrawInterface )
		fDrawInterface->ISetSceneNode(newNode);
	if( fSimulationInterface )
		fSimulationInterface->ISetSceneNode(newNode);
	if( fAudioInterface )
		fAudioInterface->ISetSceneNode(newNode);
	if( fCoordinateInterface )
		fCoordinateInterface->ISetSceneNode(newNode);

	int i;
	for( i = 0; i < GetNumGenerics(); i++ )
	{
		if( fGenerics[i] )
			fGenerics[i]->ISetSceneNode(newNode);
	}

	if( newNode )
	{
		plNodeRefMsg* refMsg = TRACKED_NEW plNodeRefMsg(newNode, plNodeRefMsg::kOnRequest, -1, plNodeRefMsg::kObject);
		plKey key = GetKey();	// for linux build
		hsgResMgr::ResMgr()->AddViaNotify(key, refMsg, plRefFlags::kActiveRef);
	}
	if( curNode)
	{
		curNode->Release(GetKey());
	}
	fSceneNode = newNode;
}

plKey plSceneObject::GetSceneNode() const
{
	return fSceneNode;
}

void plSceneObject::SetNetGroup(plNetGroupId netGroup)
{
	if( !fCoordinateInterface )
		plSynchedObject::SetNetGroup(netGroup);
	else
		fCoordinateInterface->ISetNetGroupRecur(netGroup);
}

const plModifier* plSceneObject::GetModifierByType(UInt16 classIdx) const
{
	int i;
	for (i = 0; i < fModifiers.GetCount(); i++)
	{
		plModifier * mod = fModifiers[i];
		if (mod && plFactory::DerivesFrom(classIdx, mod->ClassIndex()))
			return mod;
	}

	return nil;
}

hsBool plSceneObject::MsgReceive(plMessage* msg)
{

#if 0	// objects are only in the nil room when they are being paged out
	// TEMP - until we have another way to neutralize objects
	// for an object in the 'nil' room, ignore most msgs	
	if (GetSceneNode()==nil && !plNodeChangeMsg::ConvertNoRef(msg) &&
		!plRefMsg::ConvertNoRef(msg)&&
		!plSelfDestructMsg::ConvertNoRef(msg))
		return false;
#endif

	hsBool retVal = false;
	// If it's a bcast, let our own dispatcher find who's interested.
	plTransformMsg* trans;
	plEvalMsg* eval = plEvalMsg::ConvertNoRef(msg);
	plAttachMsg* att = nil;
	if( eval )
	{
		// Switched things over so that modifiers register for the eval message themselves,
		// and can be smart about not evaluating if they know it's unneccessary.

		//Eval(eval->DSeconds(), eval->DelSeconds());
		return true;
	}
	else
	if( trans = plTransformMsg::ConvertNoRef(msg) ) // also catches the derived plDelayedTransformMsg
	{
		if( fCoordinateInterface )
		{
			// flush any dirty transforms
			fCoordinateInterface->ITransformChanged(false, 0, trans->ClassIndex() == plTransformMsg::Index());
		}
		return true;
	}
	else
	if( att = plAttachMsg::ConvertNoRef(msg) )
	{
		if( fCoordinateInterface )
		{
			plSceneObject *child = plSceneObject::ConvertNoRef(att->GetRef());
			
			if( child )
			{
				if( !fCoordinateInterface )
				{
					// If we have no coordinate interface, we could make ourselves one here,
					// but for now it's an error.
					hsAssert(false, "Trying to attach a child when we have no coordinateInterface");
					return true;
				}
				if( !child->GetVolatileCoordinateInterface() )
				{
					// If the child has no coordinate interface, we could add one to it here,
					// but for now it's an error.
					hsAssert(false, "Trying to attach a child who has no coordinateInterface");
					return true;
				}
				plIntRefMsg* intRefMsg = TRACKED_NEW plIntRefMsg(fCoordinateInterface->GetKey(), att->GetContext(), -1, plIntRefMsg::kChildObject);
				intRefMsg->SetRef(child);
				hsgResMgr::ResMgr()->AddViaNotify(intRefMsg, plRefFlags::kPassiveRef);
			}
		}
	}
	else // Am I the final destination?
	{
		retVal = IMsgHandle(msg);
	}

	if( msg->HasBCastFlag(plMessage::kPropagateToModifiers) )
	{
		retVal |= IPropagateToModifiers(msg);
	}
		
	if (msg->HasBCastFlag(plMessage::kPropagateToChildren))
	{
		const plCoordinateInterface* ci = GetCoordinateInterface();
		for (int i = 0; i < ci->GetNumChildren(); i++)
		{
			plSceneObject* child = (plSceneObject*)ci->GetChild(i)->GetOwner();
			if (child)
			{
				hsBool modRet = child->MsgReceive(msg);
				retVal |= modRet;
			}
		}
	}
	
	// Might want an option to propagate messages to children here.

	return plSynchedObject::MsgReceive(msg);
}

hsBool plSceneObject::IMsgHandle(plMessage* msg)
{
	// To start with, plSceneObject only handles messages to add or remove
	// references. Current references are other plSceneObjects and plModifiers
	plObjRefMsg* refMsg = plObjRefMsg::ConvertNoRef(msg);
	if( refMsg )
	{
		switch( refMsg->fType )
		{
		case plObjRefMsg::kModifier:
			{
				if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
				{
					plModifier* mod = plModifier::ConvertNoRef(refMsg->GetRef());
					IAddModifier(mod, refMsg->fWhich);
				}
				else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
				{
					plModifier* mod = (plModifier*)refMsg->GetRef();
					IRemoveModifier(mod);
				}
			}
			return true;
		case plObjRefMsg::kInterface:
			{
				if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
				{
					plObjInterface* oi = plObjInterface::ConvertNoRef(refMsg->GetRef());
					ISetInterface(oi);
				}
				else
				if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
				{
					plObjInterface* oi = (plObjInterface*)refMsg->GetRef();
					// TODO - This will crash if oi's already been deleted
					IRemoveInterface(oi->ClassIndex(), oi);
				}
			}
			return true;
		}


		return false;
	}
	plNodeChangeMsg* nodeChange = plNodeChangeMsg::ConvertNoRef(msg);
	if( nodeChange )
	{
		SetSceneNode(nodeChange->GetNodeKey());
		return true;
	}
	
	if( plIntRefMsg::ConvertNoRef(msg) )
	{
		if (fCoordinateInterface)
			fCoordinateInterface->MsgReceive(msg);
		if (fAudioInterface)
			fAudioInterface->MsgReceive(msg);
		return true;
	}

	if (plCorrectionMsg::ConvertNoRef(msg))
	{
		hsAssert(fCoordinateInterface, "Unimplemented, also need to register this one we just made with the resource manager");
		
		if (fCoordinateInterface)
			fCoordinateInterface->MsgReceive(msg);

		return true;
	}
	
	// check for generic enable/disable message (passed to interfaces)

	plEnableMsg* pEnableMsg = plEnableMsg::ConvertNoRef( msg );
	if (pEnableMsg)
	{
		if( pEnableMsg->Cmd(plEnableMsg::kDrawable) )
		{
			plDrawInterface* di = GetVolatileDrawInterface();
			if( di )
				di->MsgReceive(msg);
			plObjInterface* li = GetVolatileGenericInterface(CLASS_INDEX_SCOPED(plLightInfo));
			if( li )
				li->MsgReceive(msg);
		}
		if ( pEnableMsg->Cmd(plEnableMsg::kAll) && GetDrawInterface() )
			GetVolatileDrawInterface()->MsgReceive(msg);
		
		if ( pEnableMsg->Cmd( plEnableMsg::kPhysical ) || pEnableMsg->Cmd( plEnableMsg::kAll) )
		{
			if ( GetSimulationInterface() )
			{
				GetVolatileSimulationInterface()->MsgReceive(msg);
			}
			else
			{
				// if someone is trying to disable the physics on a sceneobject that doesn't have a physics interface...
				// they might be trying to disable the avatar when the PhysX controller is being used
				// ...so, look to see if this is an avatar object, and tell the avatar to disable the physics
				IPropagateToModifiers(msg);
			}
		}
		
		if ( (pEnableMsg->Cmd( plEnableMsg::kAudible ) || pEnableMsg->Cmd( plEnableMsg::kAll )) && GetAudioInterface() )
			GetVolatileAudioInterface()->MsgReceive(msg);
		
		if( pEnableMsg->Cmd( plEnableMsg::kAll ) )
		{	
			IPropagateToGenerics(pEnableMsg);
		}
		else if( pEnableMsg->Cmd( plEnableMsg::kByType ) )
		{	
			IPropagateToGenerics(pEnableMsg->Types(), pEnableMsg);
		}
		return true;
	}
	
	// warp message
	if( plWarpMsg::ConvertNoRef(msg) )
	{
		// if there's a simulation interface, it needs to know about the warp
		// *** it would probably be better if it got this from the coordinate interface, as
		// *** only the coordinate interface knows to propagate it to children.
	//	if(fSimulationInterface)
	//	{
	//		fSimulationInterface->MsgReceive(msg);
	//	} 

		// the coordinate interface always gets the warp 
		if (fCoordinateInterface) 
		{
			fCoordinateInterface->MsgReceive(msg);
		}
		return true;
	}

	if ( plSimulationMsg::ConvertNoRef(msg) )
	{
		if(fSimulationInterface)
		{
			fSimulationInterface->MsgReceive(msg);
		}
		return true;
	}

	// audio message
	if (plSoundMsg::ConvertNoRef(msg) )
	{
		if( fAudioInterface )
			return(GetVolatileAudioInterface()->MsgReceive(msg));
		return true;
	}

	return false;
}

void plSceneObject::IPropagateToGenerics(const hsBitVector& types, plMessage* msg)
{
	hsBitIterator iter(types);
	int i;
	for( i = 0; i < fGenerics.GetCount(); i++ )
	{
		if( fGenerics[i] )
		{
			for( iter.Begin(); !iter.End(); iter.Advance() )
			{
				if( plFactory::DerivesFrom(iter.Current(), fGenerics[i]->ClassIndex()) )
				{
					fGenerics[i]->MsgReceive(msg);
					break;
				}
			}
		}
	}
}

void plSceneObject::IPropagateToGenerics(plMessage* msg)
{
	int i;
	for( i = 0; i < fGenerics.GetCount(); i++ )
	{
		if( fGenerics[i] )
			fGenerics[i]->MsgReceive(msg);
	}
}

plObjInterface* plSceneObject::GetVolatileGenericInterface(UInt16 classIdx) const
{
	int i;
	for( i = 0; i < fGenerics.GetCount(); i++ )
	{
		if( fGenerics[i] && plFactory::DerivesFrom(classIdx, fGenerics[i]->ClassIndex()) )
			return fGenerics[i];
	}
	return nil;
}

void plSceneObject::IAddGeneric(plObjInterface* gen)
{
	if( gen )
	{
		if( fGenerics.kMissingIndex == fGenerics.Find(gen) )
		{
			fGenerics.Append(gen);
			gen->ISetOwner(this);
		}
	}
}

void plSceneObject::IRemoveGeneric(plObjInterface* gen)
{
	if( gen )
	{
		int idx = fGenerics.Find(gen);
		if( fGenerics.kMissingIndex != idx )
		{
			gen->ISetOwner(nil);
			fGenerics.Remove(idx);
		}
	}
}

void plSceneObject::IRemoveAllGenerics()
{
	int i;
	for( i = 0; i < fGenerics.GetCount(); i++ )
	{
		if( fGenerics[i] )
			fGenerics[i]->ISetOwner(nil);
	}
	fGenerics.Reset();
}

void plSceneObject::ISetDrawInterface(plDrawInterface* di) 
{ 
	if( fDrawInterface != di )
	{
		if( fDrawInterface )
			fDrawInterface->ISetOwner(nil);

		fDrawInterface = di;
		if( di )
			di->ISetOwner(this);
	}
}

void plSceneObject::ISetSimulationInterface(plSimulationInterface* si) 
{ 
	if( fSimulationInterface != si )
	{
		if( fSimulationInterface )
			fSimulationInterface->ISetOwner(nil);

		fSimulationInterface = si;
		if( si )
			si->ISetOwner(this);
	}
}

void plSceneObject::ISetAudioInterface(plAudioInterface* ai) 
{ 
	if( fAudioInterface != ai )
	{
		if( fAudioInterface )
			fAudioInterface->ISetOwner(nil);
		fAudioInterface = ai;
		if( ai )
			ai->ISetOwner(this);
	}
}

void plSceneObject::ISetCoordinateInterface(plCoordinateInterface* ci) 
{ 
	if( fCoordinateInterface != ci )
	{
		if( fCoordinateInterface )
			fCoordinateInterface->ISetOwner(nil);

		fCoordinateInterface = ci;
		if( ci )
			ci->ISetOwner(this);
	}
}

//
// "is ready to process Loads"?  Check base class and modifiers.
//
hsBool	plSceneObject::IsFinal()
{
	if (!plSynchedObject::IsFinal())
		return false;

	int i;
	for(i=0;i<GetNumModifiers(); i++)
		if (fModifiers[i] && !fModifiers[i]->IsFinal())
			return false;

	return true;
}

// Export only. Interfaces perm on object.
void plSceneObject::SetDrawInterface(plDrawInterface* di) 
{ 
	ISetDrawInterface(di);
}

void plSceneObject::SetSimulationInterface(plSimulationInterface* si) 
{ 
	ISetSimulationInterface(si);
}

void plSceneObject::SetAudioInterface(plAudioInterface* ai) 
{ 
	ISetAudioInterface(ai);
}

void plSceneObject::SetCoordinateInterface(plCoordinateInterface* ci) 
{ 
	ISetCoordinateInterface(ci);
}

void plSceneObject::AddModifier(plModifier* mo)
{
	IAddModifier(mo, fModifiers.GetCount());
}

void plSceneObject::RemoveModifier(plModifier* mo)
{
	IRemoveModifier(mo);
}