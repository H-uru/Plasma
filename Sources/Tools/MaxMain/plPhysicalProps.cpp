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
#include "HeadSpin.h"
#include "plPhysicalProps.h"
#include "../plPhysical/plSimDefs.h"
// For plBoundsType
#include "../pnSceneObject/plSimulationInterface.h"

#include "plMaxNode.h"
#include "../MaxExport/plErrorMsg.h"

/** These enums are used to indicate which parameters are ignorable and can
	be overridden.
	We can't use the "PhysFlags," below, for this, because those only cover boolean properties.
	*** We should use the same enum for both and just not worry that some of the enums aren't
		usable as "ignore" flags, -- the readability & simplification gain would be worth it. */
enum CanIgnore
{
	kMemberGroup				= 0x1,
	kBounceGroup				= 0x2,
	kReportGroup				= 0x4,
	kMass						= 0x8,		
	kFriction					= 0x10,
	kRestitution				= 0x20,
	kBoundsType					= 0x40,
	kProxyNode					= 0x80,
	kPinned						= 0x100,
	kAlignToOwner				= 0x200,
	kCameraAvoid				= 0x400,
	kPhysAnim					= 0x800,
	kCanIgnoreLOSBlockCamera	= 0x1000,
	kCanIgnoreLOSBlockUI		= 0x2000,
	kCanIgnoreLOSUIItem			= 0x4000,
	kCanIgnoreLOSBlockCustom	= 0x8000,
	kCanIgnoreLOSShootable		= 0x10000,
	kCanIgnoreLOSAvatarWalkable	= 0x20000,
	kCanIgnoreLOSSwimRegion		= 0x40000,

	kAll						= 0xffffffff
};

enum PhysFlags
{
	kFlagPinned				= 0x1,
	kFlagAlignToOwner		= 0x2,
	kFlagCameraAvoid		= 0x4,
	kFlagPhysAnim			= 0x8,
	kFlagStartInactive		= 0x10,
	kFlagNoSynchronize		= 0x20,
	kFlagLOSBlockCamera		= 0x40,
	kFlagLOSBlockUI			= 0x80,
	kFlagLOSUIItem			= 0x100,
	kFlagLOSBlockCustom		= 0x200,
	kFlagLOSShootable		= 0x400,
	kFlagLOSAvatarWalkable	= 0x800,
	kFlagLOSSwimRegion		= 0x1000,
};

plPhysicalProps::plPhysicalProps() :
	fUsed(false),
	fCanIgnore(kAll),
	fGroup(0),
	fReportGroup(0),
	fMass(0),
	fFriction(0),
	fRestitution(0),
	fBoundsType(plSimDefs::kHullBounds),
	fProxyNode(nil),
	fFlags(0),
	fSubworld(nil),
	fNoSynchronize(0),
	fStartInactive(0),
	fAvAnimPushable(0)
{
}

bool plPhysicalProps::SetGroup(UInt32 group, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore)
{
	return ISetParam(fGroup, group, kMemberGroup, canIgnore, node, errMsg);
}

bool plPhysicalProps::SetReportGroup(UInt32 notifyGroup, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore)
{
	return ISetParam(fReportGroup, notifyGroup, kReportGroup, canIgnore, node, errMsg);
}

bool plPhysicalProps::SetMass(float mass, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore)
{
	if(mass != 0.0f)
	{
		if (!GetPinned())
			node->SetMovable(true);
		node->SetForceLocal(true);
	}
	return ISetParam(fMass, mass, kMass, canIgnore, node, errMsg);
}

bool plPhysicalProps::SetFriction(float friction, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore)
{
	return ISetParam(fFriction, friction, kFriction, canIgnore, node, errMsg);
}

bool plPhysicalProps::SetRestitution(float restitution, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore)
{
	return ISetParam(fRestitution, restitution, kRestitution, canIgnore, node, errMsg);
}

bool plPhysicalProps::SetBoundsType(int boundsType, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore)
{
	hsAssert(boundsType >= 1 && boundsType < plSimDefs::kNumBounds, "Bad bounds type");
	return ISetParam(fBoundsType, boundsType, kBoundsType, canIgnore, node, errMsg);
}

bool plPhysicalProps::SetProxyNode(plMaxNode *proxyNode, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore)
{
	if( proxyNode )
		proxyNode->SetDrawable(false);

	return ISetParam(fProxyNode, proxyNode, kProxyNode, canIgnore, node, errMsg);
}

bool plPhysicalProps::IGetFlagParam(int flagType)
{
	return ((fFlags & flagType) != 0);
}

bool plPhysicalProps::ISetFlagParam(bool val, int flagType, int type, bool canIgnore, plMaxNode *node, plErrorMsg *errMsg)
{
	bool ourVal = IGetFlagParam(flagType);
	if (ISetParam(ourVal, val, type, canIgnore, node, errMsg))
	{
		if (ourVal)
			fFlags |= flagType;
		else
			fFlags &= ~flagType;
		return true;
	}
	
	return false;
}

bool plPhysicalProps::SetPinned(bool status, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore)
{
	return ISetFlagParam(status, kFlagPinned, kPinned, canIgnore, node, errMsg);
}

bool plPhysicalProps::SetLOSBlockCamera(bool status, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore)
{
	return ISetFlagParam(status, kFlagLOSBlockCamera, kCanIgnoreLOSBlockCamera, canIgnore, node, errMsg);
}

bool plPhysicalProps::GetLOSBlockCamera()
{
	return IGetFlagParam(kFlagLOSBlockCamera);
}

bool plPhysicalProps::SetLOSBlockUI(bool status, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore)
{
	return ISetFlagParam(status, kFlagLOSBlockUI, kCanIgnoreLOSBlockUI, canIgnore, node, errMsg);
}

bool plPhysicalProps::GetLOSBlockUI()
{
	return IGetFlagParam(kFlagLOSBlockUI);
}

bool plPhysicalProps::SetLOSUIItem(bool status, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore)
{
	return ISetFlagParam(status, kFlagLOSUIItem, kCanIgnoreLOSUIItem, canIgnore, node, errMsg); 
}

bool plPhysicalProps::GetLOSUIItem()
{
	return IGetFlagParam(kFlagLOSUIItem);
}

bool plPhysicalProps::SetLOSBlockCustom(bool status, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore)
{
	return ISetFlagParam(status, kFlagLOSBlockCustom, kCanIgnoreLOSBlockCustom, canIgnore, node, errMsg);
}

bool plPhysicalProps::GetLOSBlockCustom()
{
	return IGetFlagParam(kFlagLOSBlockCustom);
}

bool plPhysicalProps::SetCameraAvoidFlag(bool allowLOS, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore)
{
	return ISetFlagParam(allowLOS, kFlagCameraAvoid, kCameraAvoid, canIgnore, node, errMsg);
}

bool plPhysicalProps::SetLOSShootable(bool status, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore)
{
	return ISetFlagParam(status, kFlagLOSShootable, kCanIgnoreLOSShootable, canIgnore, node, errMsg);
}
bool plPhysicalProps::GetLOSShootable()
{
	return IGetFlagParam(kFlagLOSShootable);
}

bool plPhysicalProps::SetLOSAvatarWalkable(bool status, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore)
{
	return ISetFlagParam(status, kFlagLOSAvatarWalkable, kCanIgnoreLOSAvatarWalkable, canIgnore, node, errMsg);
}

bool plPhysicalProps::GetLOSAvatarWalkable()
{
	return IGetFlagParam(kFlagLOSAvatarWalkable);
}

bool plPhysicalProps::SetLOSSwimRegion(bool status, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore)
{
	return ISetFlagParam(status, kFlagLOSSwimRegion, kCanIgnoreLOSSwimRegion, canIgnore, node, errMsg);
}

bool plPhysicalProps::GetLOSSwimRegion()
{
	return IGetFlagParam(kFlagLOSSwimRegion);
}

//bool plPhysicalProps::SetUILOSFlag(bool allowLOS, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore)
//{
//	return ISetFlagParam(allowLOS, kFlagUILOS, kUILOS, canIgnore, node, errMsg);
//}

bool plPhysicalProps::SetAlignToOwner(bool alignToOwner, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore)
{
	return ISetFlagParam(alignToOwner, kFlagAlignToOwner, kAlignToOwner, canIgnore, node, errMsg);
}

bool plPhysicalProps::SetSubworld(plMaxNode* subworld)
{
	// Note that we do *not* set fUsed, because we don't (necessarily) want a physical for this node.
	// We just want to remember that it is a subworld.
	fSubworld = subworld;
	return true;
}

bool plPhysicalProps::SetPhysAnim(bool anim, plMaxNode *node, plErrorMsg *errMsg, bool canIgnore)
{
	return ISetFlagParam(anim, kFlagPhysAnim, kPhysAnim, canIgnore, node, errMsg);
}

bool plPhysicalProps::GetPinned()
{
	return IGetFlagParam(kFlagPinned);
}

bool plPhysicalProps::GetCameraAvoid()
{
	return IGetFlagParam(kFlagCameraAvoid);
}

bool plPhysicalProps::GetAlignToOwner()
{
	return IGetFlagParam(kFlagAlignToOwner);
}

bool plPhysicalProps::GetPhysAnim()
{
	return IGetFlagParam(kFlagPhysAnim);
}

void plPhysicalProps::SetCanIgnore(UInt32 type, bool canIgnore)
{
	if (canIgnore)
		fCanIgnore |= type;
	else
		fCanIgnore &= ~type;
}

bool plPhysicalProps::CanIgnore(UInt32 type)
{
	return ((fCanIgnore & type) != 0);
}

void plPhysicalProps::IDisplayErrorMsg(plMaxNode *node, plErrorMsg *errMsg)
{
	if (!errMsg->IsBogus())
	{
		errMsg->Set(true,
			"Physics Conflict",
			"The node \"%s\" has a conflict in its physical settings.\nMake sure the physical components on it are compatible.",
			node->GetName()).Show();
	}
}
