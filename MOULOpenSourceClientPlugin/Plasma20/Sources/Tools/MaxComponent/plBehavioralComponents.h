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
#ifndef PL_BEHAVIORAL_COMPONENTS_INC
#define PL_BEHAVIORAL_COMPONENTS_INC

#include "plActivatorBaseComponent.h"

class plSittingModifier;

class plAvBehaviorSittingComponent : public plActivatorBaseComponent
{
public:
	enum
	{
		kTriggerNode_DEAD,
		kSeekTimeFloat_DEAD,
		kBoundState_DEAD,
		kDetector,
		kUseSmartSeek_DEAD,
		kApproachFront,
		kApproachLeft,
		kApproachRight,
		kDisableForward,
	};

	plAvBehaviorSittingComponent();
	hsBool SetupProperties(plMaxNode* node, plErrorMsg *pErrMsg);
	hsBool PreConvert(plMaxNode* node, plErrorMsg* plErrorMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};

#define BEHAVIORAL_SITTING_CID Class_ID(0x617e22cc, 0x31ef310d)

#endif