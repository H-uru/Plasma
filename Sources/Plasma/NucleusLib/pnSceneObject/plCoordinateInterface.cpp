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
#include "plCoordinateInterface.h"
#include "plDrawInterface.h"
#include "plSimulationInterface.h"
#include "plAudioInterface.h"
#include "../pnMessage/plWarpMsg.h"
#include "../pnMessage/plTimeMsg.h"
#include "../pnMessage/plCorrectionMsg.h"
#include "../pnMessage/plIntRefMsg.h"
#include "../pnNetCommon/plSDLTypes.h"
#include "plSceneObject.h"
#include "hsResMgr.h"
#include "plgDispatch.h"
#include "../pnKeyedObject/plKey.h"
#include "hsStream.h"

#include "plProfile.h"

UInt8 plCoordinateInterface::fTransformPhase = plCoordinateInterface::kTransformPhaseNormal;
hsBool plCoordinateInterface::fDelayedTransformsEnabled = true;

plCoordinateInterface::plCoordinateInterface()
: fParent(nil),
  fReason(kReasonUnknown)
{
	fLocalToParent.Reset();
	fParentToLocal.Reset();

	fLocalToWorld.Reset();
	fWorldToLocal.Reset();

	fState = 0;
}

plCoordinateInterface::~plCoordinateInterface()
{
	if( fParent )
		fParent->IRemoveChild(IGetOwner());
	int i;
	for( i = fChildren.GetCount()-1; i >= 0; i-- )
		IRemoveChild(i);
}

void plCoordinateInterface::ISetSceneNode(plKey newNode)
{
	int i;
	for( i = 0; i < fChildren.GetCount(); i++ )
	{
		if( fChildren[i] )
			fChildren[i]->SetSceneNode(newNode);
	}
}

void plCoordinateInterface::ISetOwner(plSceneObject* so)
{
	plObjInterface::ISetOwner(so);
	
	IDirtyTransform();
	fReason |= kReasonUnknown;
}

void plCoordinateInterface::ISetParent(plCoordinateInterface* par)
{
	fParent = par;

	// This won't have any effect if my owner is NetGroupConstant
	if( fParent )
		ISetNetGroupRecur(fParent->GetNetGroup());

	IDirtyTransform();
	fReason |= kReasonUnknown;
}

plCoordinateInterface* plCoordinateInterface::GetChild(int i) const 
{ 
	return fChildren[i] ? fChildren[i]->GetVolatileCoordinateInterface() : nil; 
}

void plCoordinateInterface::IRemoveChild(int i)
{
	if( fChildren[i] )
	{
		plCoordinateInterface* childCI = fChildren[i]->GetVolatileCoordinateInterface();
		if( childCI )
			childCI->ISetParent(nil);
	}
	fChildren.Remove(i);
}

void plCoordinateInterface::IRemoveChild(plSceneObject* child)
{
	int idx = fChildren.Find(child);
	if( idx != fChildren.kMissingIndex )
		IRemoveChild(idx);
}

void plCoordinateInterface::ISetChild(plSceneObject* child, int which)
{
	hsAssert(child, "Setting a nil child");
	plCoordinateInterface* childCI = child->GetVolatileCoordinateInterface();
	hsAssert(childCI, "Child with no coordinate interface");
	childCI->ISetParent(this);

	if( which < 0 )
		which = fChildren.GetCount();
	fChildren.ExpandAndZero(which+1);
	fChildren[which] = child;

	// If we can't delay our transform update, neither can any of our parents.
	if (!childCI->GetProperty(kDelayedTransformEval))
	{
		plCoordinateInterface *current = childCI->GetParent();
		while (current)
		{
			current->SetProperty(kDelayedTransformEval, false);
			current = current->GetParent();
		}
	}
}

void plCoordinateInterface::IAddChild(plSceneObject* child)
{
	ISetChild(child, -1);
}

void plCoordinateInterface::IAttachChild(plSceneObject* child, UInt8 flags)
{
	hsAssert(child, "Attaching a nil child");
	plCoordinateInterface* childCI = child->GetVolatileCoordinateInterface();
	hsAssert(childCI, "Owner without CoordinateInterface being attached");

	if (childCI->GetParent() == this)
		return; // We're already attached! Who told us to do this?
	
	hsMatrix44 l2w = childCI->GetLocalToWorld();
	hsMatrix44 w2l = childCI->GetWorldToLocal();

	if( childCI->GetParent() )
		childCI->GetParent()->IDetachChild(child, flags | kAboutToAttach);

	childCI->IUnRegisterForTransformMessage();

	IAddChild(child);

	if( flags & kMaintainWorldPosition )
		childCI->WarpToWorld(l2w,w2l);
}

void plCoordinateInterface::IDetachChild(plSceneObject* child, UInt8 flags)
{
	hsAssert(child, "Detaching a nil child");
	plCoordinateInterface* childCI = child->GetVolatileCoordinateInterface();
	hsAssert(childCI, "Owner without CoordinateInterface being attached");

	hsMatrix44 l2w = childCI->GetLocalToWorld();
	hsMatrix44 w2l = childCI->GetWorldToLocal();

	GetKey()->Release(child->GetKey());
	if( IGetOwner() && IGetOwner()->GetKey() )
		IGetOwner()->GetKey()->Release(child->GetKey());
	IRemoveChild(child);

	if( flags & kMaintainWorldPosition )
		childCI->WarpToWorld(l2w,w2l);

	// If the child was keeping us from delaying our transform, 
	// maybe we can, now that it's gone.
	if (!childCI->GetProperty(kDelayedTransformEval))
		IUpdateDelayProp();
}

/*
 *	A few notes on the delay transform properties...
 *
 *		The kCanEverDelayTransform prop is independent of any parents/children.
 *	It means this particular node must always update its transform in response
 *	to a plTransformMsg. It is intended for objects with physics, because they
 *	need to be up-to-date before the simulationMgr updates the physical world.
 *
 *		The kDelayedTransformEval prop is for nodes that are free of physics. (So no
 *	physical descendants either). If the property is set, we won't update our
 *	transform until AFTER the simulationMgr does its work.
 *
 *		When we attach a child that can't delay its eval (at the moment), we recurse
 *	up to the root, turning off the kDelayedTransformEval prop as we go. When we
 *	remove such a child, we check if that child was the only reason we weren't
 *	delaying our transform. If so, we update ourself and tell our parent to check.
 *
 *	BTW: The POINT of all this is that when we update our l2w transforms because
 *	we're animated, and then we update AGAIN after a parent node of ours involved
 *	in physics gets a slight nudge, the first update becomes pointless. The
 *	delay prop bookkeeping keeps us from doing the wasted calculations. And since 
 *	nearly all bones on the avatar are in this exact situation, it's worth doing.
 */
void plCoordinateInterface::IUpdateDelayProp()
{
	int i;
	if (!GetProperty(kCanEverDelayTransform))
		return;

	for (i = 0; i < GetNumChildren(); i++)
	{
		// If we still have a child that needs the delay...		
		if (!GetChild(i)->GetProperty(kDelayedTransformEval))
			return;
	}

	// Cool, we can delay now, which means maybe our parent can too.
	SetProperty(kDelayedTransformEval, true);
	if (GetParent())
		GetParent()->IUpdateDelayProp();
}

plCoordinateInterface* plCoordinateInterface::IGetRoot()
{
	return fParent ? fParent->IGetRoot() : this;
}

void plCoordinateInterface::IRegisterForTransformMessage(hsBool delayed)
{
	if( IGetOwner() )
	{
		if ((delayed || fTransformPhase == kTransformPhaseDelayed) && fDelayedTransformsEnabled)
			plgDispatch::Dispatch()->RegisterForExactType(plDelayedTransformMsg::Index(), IGetOwner()->GetKey());
		else
			plgDispatch::Dispatch()->RegisterForExactType(plTransformMsg::Index(), IGetOwner()->GetKey());
	}
}

void plCoordinateInterface::IUnRegisterForTransformMessage()
{
	if( IGetOwner() )
		plgDispatch::Dispatch()->UnRegisterForExactType(plTransformMsg::Index(), IGetOwner()->GetKey());
}


void plCoordinateInterface::IDirtyTransform()
{
	fState |= kTransformDirty;

	IGetRoot()->IRegisterForTransformMessage(GetProperty(kDelayedTransformEval));
}

void plCoordinateInterface::MultTransformLocal(const hsMatrix44& move, const hsMatrix44& invMove)
{
	fReason |= kReasonUnknown;
	fLocalToParent = move * fLocalToParent;
	fParentToLocal = fParentToLocal * invMove;

	IDirtyTransform();
}

void plCoordinateInterface::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	fReason |= kReasonUnknown;
	
	if( fParent )
	{
		SetLocalToParent(fParent->GetWorldToLocal() * l2w, w2l * fParent->GetLocalToWorld());
	}
	else
	{
		SetLocalToParent(l2w, w2l);
	}
}

void plCoordinateInterface::SetTransformPhysical(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	// since we use public interfaces to do the details
	// AND those public interfaces could be called by anyone
	// AND those public interfaces therefore have to set their own "reason" for the transform change
	// THEREFORE: we need to preserve the "reason" flags before we call the public interfaces
	//			  so that we don't get reasonPhysics + reasonUnknown, just reasonPhysics
	UInt16 oldReason = fReason;

	if( fParent )
	{
		SetLocalToParent(fParent->GetWorldToLocal() * l2w, w2l * fParent->GetLocalToWorld());
	}
	else
	{
		SetLocalToParent(l2w, w2l);
	}
	fReason = oldReason | kReasonPhysics;
}

UInt16 plCoordinateInterface::GetReasons()
{
	return fReason;
}

void plCoordinateInterface::ClearReasons()
{
	fReason = nil;
}

void plCoordinateInterface::SetLocalToParent(const hsMatrix44& l2p, const hsMatrix44& p2l)
{
	fReason |= kReasonUnknown;
	fLocalToParent = l2p;
	fParentToLocal = p2l;

	IDirtyTransform();
}

void plCoordinateInterface::WarpToLocal(const hsMatrix44& l2p, const hsMatrix44& p2l)
{
	fReason |= kReasonUnknown;
	SetLocalToParent(l2p, p2l);

	// update physical state when an object is warped
	if (IGetOwner())
		IGetOwner()->DirtySynchState(kSDLPhysical, 0);
}

void plCoordinateInterface::WarpToWorld(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	fReason |= kReasonUnknown;
	if( fParent )
	{
		hsMatrix44 l2p = fParent->GetWorldToLocal() * l2w;
		hsMatrix44 p2l = w2l * fParent->GetLocalToWorld();
		WarpToLocal(l2p, p2l);
	}
	else
	{
		WarpToLocal(l2w, w2l);
	}
}

plProfile_CreateCounter("CITrans", "Object", CITrans);
plProfile_CreateCounter("   CIRecalc", "Object", CIRecalc);
plProfile_CreateCounter("   CIDirty", "Object", CIDirty);
plProfile_CreateCounter("   CISet", "Object", CISet);

plProfile_CreateTimer("CITransT", "Object", CITransT);
plProfile_CreateTimer("   CIRecalcT", "Object", CIRecalcT);
plProfile_CreateTimer("   CIDirtyT", "Object", CIDirtyT);
plProfile_CreateTimer("   CISetT", "Object", CISetT);

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

void plCoordinateInterface::IRecalcTransforms()
{
	plProfile_IncCount(CIRecalc, 1);
	plProfile_BeginTiming(CIRecalcT);
	if( fParent )
	{
#if 0
		fLocalToWorld = fParent->GetLocalToWorld() * fLocalToParent;
		fWorldToLocal = fParentToLocal * fParent->GetWorldToLocal();
#else
		fLocalToWorld = IMatrixMul34(fParent->GetLocalToWorld(), fLocalToParent);
		fWorldToLocal = IMatrixMul34(fParentToLocal, fParent->GetWorldToLocal());
#endif
	}
	else
	{
		fLocalToWorld = fLocalToParent;
		fWorldToLocal = fParentToLocal;
	}
	plProfile_EndTiming(CIRecalcT);
}

void plCoordinateInterface::ITransformChanged(hsBool force, UInt16 reasons, hsBool checkForDelay)
{
	plProfile_IncCount(CITrans, 1);
	plProfile_BeginTiming(CITransT);

	// inherit reasons for transform change from our parents
	fReason |= reasons;
	
	UInt16 propagateReasons = fReason;

	hsBool process = !(checkForDelay && GetProperty(kDelayedTransformEval)) || !fDelayedTransformsEnabled;

	if (process)
	{
		if( fState & kTransformDirty )
			force = true;
	}

	if( force )
	{
		IRecalcTransforms();

		plProfile_IncCount(CISet, 1);
		plProfile_BeginTiming(CISetT);
		if( IGetOwner() )
		{
			IGetOwner()->ISetTransform(fLocalToWorld, fWorldToLocal);
		}
		plProfile_EndTiming(CISetT);
		fState &= ~kTransformDirty;		
	}

	plProfile_EndTiming(CITransT);
	if (process)
	{
		int i;
		for( i = 0; i < fChildren.GetCount(); i++ )
		{
			if( fChildren[i] && fChildren[i]->GetVolatileCoordinateInterface() )
				fChildren[i]->GetVolatileCoordinateInterface()->ITransformChanged(force, propagateReasons, checkForDelay);
		}
	}
	else if (force)
	{
		plProfile_IncCount(CIDirty, 1);
		plProfile_BeginTiming(CITransT);
		// Our parent is dirty and we're bailing out on evaluating right now.
		// Need to ensure we'll be evaluated in the delay pass
		plProfile_BeginTiming(CIDirtyT);
		IDirtyTransform();
		plProfile_EndTiming(CIDirtyT);
		plProfile_EndTiming(CITransT);
	}		
}

void plCoordinateInterface::FlushTransform(hsBool fromRoot)
{
	if( fromRoot )
		IGetRoot()->ITransformChanged(false, 0, false);
	else
		ITransformChanged(false, 0, false);
}

void plCoordinateInterface::ISetNetGroupRecur(plNetGroupId netGroup)
{
	if( !IGetOwner() )
		return;

	if( IGetOwner()->GetSynchFlags() & kHasConstantNetGroup )
		return;

	IGetOwner()->plSynchedObject::SetNetGroup(netGroup);

	int i;
	for( i = 0; i < GetNumChildren(); i++ )
	{
		if( GetChild(i) )
		{
			GetChild(i)->ISetNetGroupRecur(netGroup);
		}
	}
}


void plCoordinateInterface::Read(hsStream* stream, hsResMgr* mgr)
{
	plObjInterface::Read(stream, mgr);

	fLocalToParent.Read(stream);
	fParentToLocal.Read(stream);

	fLocalToWorld.Read(stream);
	fWorldToLocal.Read(stream);

	int n = stream->ReadSwap32();
	int i;
	for( i = 0; i < n; i++ )
	{
		plIntRefMsg* refMsg = TRACKED_NEW plIntRefMsg(GetKey(), plRefMsg::kOnCreate, -1, plIntRefMsg::kChildObject);
		mgr->ReadKeyNotifyMe(stream,refMsg, plRefFlags::kPassiveRef);
	}
}

void plCoordinateInterface::Write(hsStream* stream, hsResMgr* mgr)
{
	plObjInterface::Write(stream, mgr);

	fLocalToParent.Write(stream);
	fParentToLocal.Write(stream);

	fLocalToWorld.Write(stream);
	fWorldToLocal.Write(stream);

	stream->WriteSwap32(fChildren.GetCount());
	int i;
	for( i = 0; i < fChildren.GetCount(); i++ )
		mgr->WriteKey(stream, fChildren[i]);

}

hsBool plCoordinateInterface::MsgReceive(plMessage* msg)
{
	hsBool retVal = false;

	plIntRefMsg* intRefMsg;
	plCorrectionMsg* corrMsg;

	// warp message
	plWarpMsg* pWarpMsg = plWarpMsg::ConvertNoRef(msg);
	if (pWarpMsg)
	{
		hsMatrix44 l2w = pWarpMsg->GetTransform();
		hsMatrix44 inv;
		l2w.GetInverse(&inv);
		WarpToWorld(l2w,inv);
		if (pWarpMsg->GetWarpFlags() & plWarpMsg::kFlushTransform)
			ITransformChanged(false, kReasonUnknown, false);
		return true;
	}
	else if( intRefMsg = plIntRefMsg::ConvertNoRef(msg) )
	{
		switch( intRefMsg->fType )
		{
		case plIntRefMsg::kChildObject:
		case plIntRefMsg::kChild:
			{
				plSceneObject* co = nil;
				if( intRefMsg->fType == plIntRefMsg::kChildObject )
				{
					co = plSceneObject::ConvertNoRef(intRefMsg->GetRef());
				}
				else
				{
					plCoordinateInterface* ci = plCoordinateInterface::ConvertNoRef(intRefMsg->GetRef());
					co = ci ? ci->IGetOwner() : nil;
				}
				if( intRefMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnReplace) )
				{
					ISetChild(co, intRefMsg->fWhich);
				}
				else if( intRefMsg->GetContext() & plRefMsg::kOnDestroy )
				{
					IRemoveChild(co);
				}
				else if( intRefMsg->GetContext() & plRefMsg::kOnRequest )
				{
					IAttachChild(co, kMaintainWorldPosition|kMaintainSceneNode);
				}
				else if( intRefMsg->GetContext() & plRefMsg::kOnRemove )
				{
					IDetachChild(co, kMaintainWorldPosition|kMaintainSceneNode);
				}
			}
			return true;
		default:
			break;
		}
	}
	else if( corrMsg = plCorrectionMsg::ConvertNoRef(msg) )
	{
		SetTransformPhysical(corrMsg->fLocalToWorld, corrMsg->fWorldToLocal);

		if(corrMsg->fDirtySynch)
		{
			if (IGetOwner())
				IGetOwner()->DirtySynchState(kSDLPhysical, 0);
		}
		
		return true;
	}

	return plObjInterface::MsgReceive(msg);
}


