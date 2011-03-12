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

#ifndef __PLCRITTERCMDS_H__
#define __PLCRITTERCMDS_H__

#include "hsStlUtils.h"


typedef std::vector<std::string> VCharArray;

class plCritterCommands
{
public:
	// The order of these is significant; setting the blend on an animation in this list
	// to maximum will prevent all animations above it from playing.
	// I.E. if "turn left" is set to maximum, it will completely hide "idle"
	enum stdAnim {
		kIdle,
		kTurnLeft,
		kTurnRight,
		kForwardMedium,
		kForwardFast,
		kReverse,
		kBankLeft,
		kBankRight,
		kImpactDown,
		kNumStdAnims
	};

	VCharArray fAnimNameTypes;

	//Vector is used because the []operator is handy for filling
	//the dialog boxes for implementation Components. -RXA
	// KEEP THIS SYNCHRONIZED with the enum above.
	plCritterCommands() 
	{
		fAnimNameTypes.push_back("Idle");
		fAnimNameTypes.push_back("TurnLeft");
		fAnimNameTypes.push_back("TurnRight");
		fAnimNameTypes.push_back("Forward Medium");
		fAnimNameTypes.push_back("Forward Fast");
		fAnimNameTypes.push_back("Reverse");
		fAnimNameTypes.push_back("Bank Left");
		fAnimNameTypes.push_back("Bank Right");
		fAnimNameTypes.push_back("Impact Down");
	}

	int GetNumElements() { return fAnimNameTypes.size(); }

};

/*
class plCritterCommands
{
public:
	enum avStdAnim{
		kTorsoStraightenOut = 0,
		kTorsoSpasticRotate,
		kTorsoShakeHead,
		kTorsoOpenMouth,
		kTorsoHover,
		kTorsoForwardFlightSpeedB,
		kTorsoForwardFlightSpeedA,
		kTorsoBankRightFlapping,
		kTorsoBankRightCoasting,
		kTorsoBankLeftFlapping,
		kTorsoBankLeftCoasting,
		kTorsoBackwardFlight,
		kRightWingSpasticRotate,
		kRightWingHover,
		kRightWingForwardFlightSpeedB,
		kRightWingForwardFlightSpeedA,
		kRightWingBankRightFlapping,
		kRightWingBankRightCoasting,
		kRightWingBankLeftFlapping,
		kRightWingBankLeftCoasting,
		kRightWingBackwardFlight,
		kLeftWingSpasticRotate,
		kLeftWingHover,
		kLeftWingForwardFlightSpeedA,
		kLeftWingForwardFlightSpeedB,
		kLeftWingBankRightFlapping,
		kLeftWingBankRightCoasting,
		kLeftWingBankLeftFlapping,
		kLeftWingBankLeftCoasting,
		kLeftWingBackwardFlight,
		kNumStdAnims
	} ;

	VCharArray fAnimNameTypes;

	//Vector is used because the []operator is handy for filling
	//the dialog boxes for implementation Components. -RXA
	// KEEP THIS SYNCHRONIZED with the enum above.
	plCritterCommands() 
	{
		fAnimNameTypes.push_back("Torso Straighten Out");
		fAnimNameTypes.push_back("Torso Spastic Rotate");
		fAnimNameTypes.push_back("Torso Shake Head");
		fAnimNameTypes.push_back("Torso Open Mouth");
		fAnimNameTypes.push_back("Torso Hover (Idle)");
		fAnimNameTypes.push_back("Torso Forward SpeedB");
		fAnimNameTypes.push_back("Torso Forward SpeedA");
		fAnimNameTypes.push_back("Torso RBank Flapping");
		fAnimNameTypes.push_back("Torso RBank Coasting");
		fAnimNameTypes.push_back("Torso LBank Flapping");
		fAnimNameTypes.push_back("Torso LBank Coasting");
		fAnimNameTypes.push_back("Torso Backward Flight");
		fAnimNameTypes.push_back("RWing Spastic Rotate");
		fAnimNameTypes.push_back("RWing Hover (Idle)");
		fAnimNameTypes.push_back("RWing Forward SpeedB");
		fAnimNameTypes.push_back("RWing Forward SpeedA");
		fAnimNameTypes.push_back("RWing RBank Flapping");
		fAnimNameTypes.push_back("RWing RBank Coasting");
		fAnimNameTypes.push_back("RWing LBank Flapping");
		fAnimNameTypes.push_back("RWing LBank Coasting");
		fAnimNameTypes.push_back("RWing Backward Flight");
		fAnimNameTypes.push_back("LWing Spastic Rotate");
		fAnimNameTypes.push_back("LWing Hover (Idle)");
		fAnimNameTypes.push_back("LWing Forward SpeedA");
		fAnimNameTypes.push_back("LWing Forward SpeedB");
		fAnimNameTypes.push_back("LWing RBank Flapping");
		fAnimNameTypes.push_back("LWing RBank Coasting");
		fAnimNameTypes.push_back("LWing LBank Flapping");
		fAnimNameTypes.push_back("LWing LBank Coasting");
		fAnimNameTypes.push_back("LWing Backward Flight");
	}

	int GetNumElements() { return fAnimNameTypes.size(); }

};

*/


#endif
