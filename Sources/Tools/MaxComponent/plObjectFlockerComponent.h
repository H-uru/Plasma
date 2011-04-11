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
#ifndef OBJ_FLOCKER_COMPONENT
#define OBJ_FLOCKER_COMPONENT

class pfObjectFlocker;

class plObjectFlockerComponent : public plComponent
{
public:
	enum
	{
		kBoidObject,
		kNumBoids,
		kGoalStrength,
		kWanderStrength,
		kSepStrength,
		kSepRadius,
		kCohStrength,
		kCohRadius,
		kMaxForce,
		kMaxSpeed,
		kMinSpeed,
		kUseTargetRotation,
		kRandomAnimStart,
		kHideTarget,
	};

	plObjectFlockerComponent();
	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool DeInit(plMaxNode* node, plErrorMsg* pErrMsg);

protected:
	pfObjectFlocker* fFlocker;

};

const Class_ID OBJECT_FLOCKER_COMPONENT_CLASS_ID(0x79013d96, 0x7dca7842);

#endif
