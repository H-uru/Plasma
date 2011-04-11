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
#ifndef PLSIMDEFS_H
#define PLSIMDEFS_H
#pragma once

namespace plSimDefs
{
	// Groups are used to determine what a physical collides with, and what a
	// detector detects
	enum Group
	{
		// A physical that blocks avatars and dynamics
		kGroupStatic,
		// Blocks only avatars
		kGroupAvatarBlocker,
		// Blocks only dynamics
		kGroupDynamicBlocker,
		// No physical should actually be set to this, since the avatar is
		// controlled by a special kind of physical.  However, you can use this
		// for detecting.
		kGroupAvatar,
		// A physical that can be pushed around by the avatar or other dynamics
		kGroupDynamic,
		// A physical that doesn't block anything, but reports on any physical
		// that enters it
		kGroupDetector,
		// Blocks nothing, for los checks only
		kGroupLOSOnly,
		//kExcludeRegion setting up so only blocks avatars and only when not in seek mode
		kGroupExcludeRegion,
		// Just for error checking
		kGroupMax,
	};

	/** A taxonomy of action types. Crucial for doing things like making sure you don't
		do things like attach duplicate actions. */
	enum ActionType
	{
		kUnknownAction		=	0x01,			// don't know the type (probably forgot to override GetType())
		kUnknownZAction		=	0x02,			// unknown type of z-order action
		kAntiGravityAction	=	0x03,			// an action that counters gravity exactly
		kUprightAction		=	0x04,			// an action that keeps an object upright by apply force
		kPhysAnimAction		=	0x05,			// an action that parses keyframed animation into physical information
		kConstraint			=	0x06,			// a general constraint.
		kCallbackAction		=	0x07,			// an action that just hands us the physics "tick"
		kPseudoPhysAction	=	0x08,			// replacement for the physAnim
		kAntiGravAction		=	0x09,			// makes things float in the air
		kBasicGroundAction	=	0x0a,			// for your basic walkAroundOnGround corrections
		kHorizontalFreeze	=	0x0b,			// Let's you fall vertically, but otherwise keeps you in place (generic brains)

		kMaxAction			=	0xffff			// force 16-bit
	};

	/** Different types of line-of-sight requests. */
	enum LOSReqType
	{
		// these are MASKS -- keep powers of two
		kLOS_IgnoreCameraRequest	= 0x1,
		kLOS_IgnoreUIRequest		= 0x2,
		kLOS_CameraAvoidObject		= 0x4,

		kLOS_Max					= 0xffff	// force 16-bit
	};

	/** Different types of physics shapes. In most cases they dictate how we intepret
		the raw vertices. In some cases (sphere) they explain why there aren't any vertices */
	enum Bounds {
		kBoxBounds		=	0x01,	// bounding box
		kSphereBounds,				// bounding sphere
		kHullBounds,				// convex hull
		kProxyBounds,				// use alternate proxy geometry
		kExplicitBounds,			// use the primary geometry
		kNumBounds,					// the number of bounds types
		
		kBoundsMax		=	0xff	// force 8-bit
	};
	
	enum plLOSDB {
		kLOSDBNone				=	0x0000,

		kLOSDBUIBlockers		=	0x0001,		// things that block ui probes
		kLOSDBUIItems			=	0x0002,		// ui items -- we check these first for a hit before checking blockers
		kLOSDBCameraBlockers	=	0x0004,		// things that block camera probes
		kLOSDBCustom			=	0x0008,		// special things. only the user knows
		kLOSDBLocalAvatar		=	0x0010,		// yes, this is silly. it's transitional :)
		kLOSDBShootableItems	=	0x0020,		// shootable items, like from the VaporMiner gun

		kLOSDBAvatarWalkable	=	0x0040,		// stuff the avatar is expected to walk on. Used to prevent sliding and allow
												// jumping. All terrain automatically goes here.
		
		kLOSDBSwimRegion		=	0x0080,

		kLOSDBMax				=	0x0100,		// MOVE THIS UP if you add new classes under it. we need it to iterate through the DBs
		kLOSDBForce16			=	0xffff
	};
}


#endif PLSIMDEFS_H