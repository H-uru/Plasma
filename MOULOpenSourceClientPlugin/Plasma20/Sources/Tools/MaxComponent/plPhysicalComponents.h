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
#ifndef plPhysicalComponents_h_inc
#define plPhysicalComponents_h_inc

#include "max.h"								//Max Dependencies
#include "plComponent.h"

#define PHYSICS_BASE_CID			Class_ID(0x610f187a, 0x25824341)
#define PHYSICS_DEBUG_CID			Class_ID(0x24131a19, 0x3ef26119)
#define PHYSICS_TERRAIN_CID			Class_ID(0x5a526841, 0x11d00083)
#define PHYSICS_DETECTOR_CID		Class_ID(0x33b60376, 0x7e5163e0)
#define PHYSICS_INVISIBLE_CID		Class_ID(0x11e81ee4, 0x36b81450)
#define PHYSICS_PLAYER_CID			Class_ID(0x48703458, 0x4c3a4f39)
#define PHYSICS_SIMPLE_CID			Class_ID(0x1e1d2ec5, 0x37f612d6)	
#define PHYSICS_BLOCKER_CID			Class_ID(0xc9a44be, 0x33c87ffb)
#define PHYSICS_BOUND_BLOCKER_CID	Class_ID(0x1beb2bc6, 0x49cd7e64)
#define PHYS_WALKABLE_CID			Class_ID(0xcc56fb9, 0x716e0810)
#define PHYS_CLIMBABLE_CID			Class_ID(0x74602123, 0x7505f97)
#define PHYS_SWIMSURFACE_CID		Class_ID(0x1cde28dd, 0x3c792b48)
#define PHYS_SWIM3D_CID				Class_ID(0x12040f17, 0x496913c4)
#define PHYS_CAMERA_BLOCK_CID		Class_ID(0x32c729fc, 0x606e79d9)
#define PHYS_SUBWORLD_CID			Class_ID(0x157e2138, 0x272540c1)
#define SUBWORLD_REGION_CID			Class_ID(0x50d13e25, 0x5c4d139b)
#define PANIC_LINK_REGION_CID		Class_ID(0x8d1242, 0x4c283d12)

#include <map>
#include "plPhysical.h"
#include "../plAvatar/plSwimRegion.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Physical Component	CORE
//
//


//!  A Class
 
/*!
 As all physical parameters are shared and accessed by the Core Component and
 its Convert() function, this class holds all the current physical parameters.
 It is accessed by the individual components via their GetParamVals() functions.  
 \sa GetParamVals()
*/

class plPhysicCoreComponent : public plComponent
{
protected:
//	plHKPhysical* fMyHKPhysical;

public:
	//!	Enum Global for Physical Components.  \c Append \c only!

	/*!
	 Enum Global for Physical Components.  \c Append \c only!  This enum is essential
	 for backward compatability issues.  Max uses it to maintain versioning for the
	 ParamBlock2s that are extensively used.  As long as vals are never deleted,
	 full backward compatability is assured.  Just append your new vals to the enum 
	 as versions increase.
	*/
	enum
	{
		kMass,						/*!< Mass Enum value, inserted in v1.*/
		kBounce,					/*!< Bounce Enum value, inserted in v1.*/
		kFriction,					/*!< Friction Enum value, inserted in v1.*/
		kGhostChkBx,				/*!< Ghost Enum value, inserted in v1.*/
		kNotifyCollisionsChkBx_DEAD,		/*!< Collision Report Enum value, inserted in v1.*/
		kStartForceX,				/*!< Start Force X Enum value, inserted in v1.*/
		kStartForceY,				/*!< Start Force Y Enum value,inserted in v1.*/
		kStartForceZ,				/*!< Start Force Z Enum value,inserted in v1.*/
		kStartTorqueX,				/*!< Start Torque X Enum value,inserted in v1.*/
		kStartTorqueY,				/*!< Start Torque Y Enum value,inserted in v1.*/
		kStartTorqueZ,				/*!< Start Torque Z Enum value,inserted in v1.*/
		kBoundCondRadio,			/*!< Bounding State Enum value, inserted in v1.*/
		kDynamicChkBx,				/*!< Dynamic State Enum value, inserted in v1, Removed in v2.*/
		kWeight,					/*!< Weight Enum value, inserted in v1.*/
		kTurnForce,					/*!< TurnForce Enum value, inserted in v1.*/
		kInvisibleChkBx,			/*!< Invisible Enum value, inserted in v1.*/
		kCustomBoundListStuff,		/*!< Bounding Proxy Enum value, inserted in v2.*/
		kCustomBoundField,			/*!< Bounding String Enum value, inserted in v2, Removed in v2.*/
		kLOSChkBx,					/*!< LOS Enum value, inserted in v3.	*/
		kConstPinChkBx,				/*!< Constraint Pin Enum value, inserted in v4.	 Removed in v5*/
		kAcceleration,				/*!< Accel Enum value, inserted in v5.	*/
		kAlignProxyShape,			/*!< Alignment of Proxy Shape Enum value, inserted in v6.	*/
		kMemberOfEventGroupBoolTab_DEAD,	/*!< Designates which Event Groups this is a member of, inserted in v6 */
		kBounceEventGroupBoolTab_DEAD,	/*!< Designates which Event Groups this reacts to, inserted in v6 */
		kReportEventGroupBoolTab_DEAD,	/*!< Designates which Event Groups this reacts to, inserted in v6 */
		kMemEventGroupChoice_DEAD,
		kBounceEventGroupChoice_DEAD,
		kReportEventGroupChoice_DEAD,
		kCamLOSChkBx,
		kUILOSChkBx,
		kMemberGroups_DEAD,
		kBounceGroups_DEAD,
		kReportGroups,
		kCamAvoidChkBx,
		kGravityX,
		kGravityY,
		kGravityZ,
		kNoSynchronize,				// Don't do any network synchronization for this physical
		kStartInactive_DEAD,
		kStartInactive,				// Deactive this when first adding to the sim
		kSwimCurrentType,
		kSwimCurrentRotation,
		kSwimCurrentPull,			// Obsolete
		kSwimDetectorNode,
		kSwimCurrentNode,
		kSwimBuoyancyDown,
		kSwimBuoyancyUp,
		kSwimMaxUpVel,
		kSwimCurrentPullFarDist,
		kSwimCurrentPullFarVel,
		kSwimCurrentPullNearDist,
		kSwimCurrentPullNearVel,
		kSwimCurrentStraightNearDist,
		kSwimCurrentStraightNearVel,
		kSwimCurrentStraightFarDist,
		kSwimCurrentStraightFarVel,
		kAvAnimPushable,			// Is this a big object that the avatar should use push anims on?
		kGroup,
	};

	virtual void DeleteThis() { delete this; }

	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg); 
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg); 
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

	int CanConvertToType(Class_ID obtype)
	{ return (obtype == PHYSICS_BASE_CID) ? 1 : plComponent::CanConvertToType(obtype); }

protected:
	void IFixBounds();
	hsBool IGetProxy(plMaxNode *node, plErrorMsg *pErrMsg);

	UInt32 IGetEventGroup(ParamID paramID);
};

class plSwim2DComponent : public plPhysicCoreComponent
{
public:
	plSwim2DComponent();
	hsBool SetupProperties(plMaxNode* node, plErrorMsg *pErrMsg);
	hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

	std::map<plMaxNode*, plSwimRegionInterface*> fSwimRegions;
	
	enum
	{
		kCurrentNone,
		kCurrentSpiral,
		kCurrentStraight,
	};
};

#endif // plPhysicalComponents_h_inc