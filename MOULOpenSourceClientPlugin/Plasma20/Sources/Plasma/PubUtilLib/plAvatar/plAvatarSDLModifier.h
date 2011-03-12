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
#ifndef plAvatarSDLModifier_inc
#define plAvatarSDLModifier_inc

#include "hsConfig.h"
#include "hsStlUtils.h"
#include "../plModifier/plSDLModifier.h"


//
// This modifier is responsible for sending and recving 
// an avatar's state
//
class plStateDataRecord;
class plArmatureMod;
class plAnimStage;
class plAvBrainGeneric;
class plAvBrainClimb;
class plAvBrainDrive;

class plAvatarPhysicalSDLModifier : public plSDLModifier
{
protected:
	static char kStrPosition[];
	static char kStrRotation[];
	static char kStrSubworld[];

	void ISetCurrentStateFrom(const plStateDataRecord* srcState);
	void IPutCurrentStateIn(plStateDataRecord* dstState);

	UInt32 IApplyModFlags(UInt32 sendFlags) { return (sendFlags | plSynchedObject::kDontPersistOnServer | plSynchedObject::kIsAvatarState); }

public:
	CLASSNAME_REGISTER( plAvatarPhysicalSDLModifier );
	GETINTERFACE_ANY( plAvatarPhysicalSDLModifier, plSDLModifier);
		
	const char* GetSDLName() const { return kSDLAvatarPhysical; }

};

class plAvatarSDLModifier : public plSDLModifier
{
protected:
	static char kStrBrainStack[];
	static char kStrInvisibilityLevel[];

	// var labels 
	struct StandardStageVarNames
	{
		static char	kStrName[];	
		static char	kStrNumLoops[];
		static char	kStrForward[];
		static char	kStrBackward[];
		static char	kStrStageAdvance[];
		static char	kStrStageRegress[];
		static char	kStrNotifyEnter[];
		static char	kStrNotifyLoop[];
		static char	kStrNotifyStageAdvance[];
		static char	kStrNotifyStageRegress[];
		static char	kStrUseGlobalCoords[];
		static char	kStrLocalTime[];
		static char	kStrCurrentLoop[];
		static char	kStrIsAttached[];		
	};

	struct GenericBrainVarNames
	{
		static char	kStrStages[];		
		static char	kStrCurrentStage[];		
		static char	kStrFreezePhysicalAtEnd[];
		static char kStrCallbackRcvr[];
		static char kStrMovingForward[];
		static char kStrExitFlags[];
		static char kStrType[];
		static char kStrMode[];
		static char kStrFadeIn[];
		static char kStrFadeOut[];
		static char kStrMoveMode[];
		static char kStrBodyUsage[];
	}; 


	struct BrainUnionVarNames
	{
		static char kGenericBrain[];
		static char kClimbBrain[];
		static char kDriveBrain[];
	};

	/** Top level: gets the current avatar brain state for any number of brains
		of varying type from the data record into the avatar.
		Will destroy all optional brains and recreate them from the incoming SDL. */
	void ISetCurrentStateFrom(const plStateDataRecord* srcState);
	/** Set the base state for the avatar. This primarily consists of the state of the
		human brain, which is required for all avatars. */
	void ISetBaseAvatarStateFrom(plArmatureMod *avMod, const plStateDataRecord* src);
	/** We found a generic brain in the SDL stream. Build the corresponding brain and attach it. */
	bool ISetGenericBrainFrom(plArmatureMod *avMod, const plStateDataRecord* srcState);
	/** We found a climb brain in the SDL stream. Build the corresponding brain and attach it. */
	void ISetClimbBrainFrom(plArmatureMod *avMod, const plStateDataRecord* src);
	/** We found a drive brain in the SDL stream. Build the corresponding brain and attach it. */
	void ISetDriveBrainFrom(plArmatureMod *avMod, const plStateDataRecord* src);
	/** Generic brains have multiple "stages" parse *one* out of the SDL stream and return it. */
	plAnimStage * IGetStageFrom(plArmatureMod *avMod, const plStateDataRecord* srcState);

	void IPutCurrentStateIn(plStateDataRecord* dstState);
	void IPutBaseAvatarStateIn(plArmatureMod *avMod, plStateDataRecord* dst);
	void IPutGenericBrainIn(plArmatureMod *avMod, plAvBrainGeneric *brain, plStateDataRecord* dstState);
	void IPutClimbBrainIn(plArmatureMod *avMod, plAvBrainClimb *brain, plStateDataRecord* dstState);
	void IPutDriveBrainIn(plArmatureMod *avMod, plAvBrainDrive *brain, plStateDataRecord* dstState);
	bool IPutStageIn(plArmatureMod *avMod, plAnimStage *stage, plStateDataRecord* dstState);
	
	UInt32 IApplyModFlags(UInt32 sendFlags) { return (sendFlags | plSynchedObject::kDontPersistOnServer | plSynchedObject::kIsAvatarState); }

public:
	CLASSNAME_REGISTER( plAvatarSDLModifier );
	GETINTERFACE_ANY( plAvatarSDLModifier, plSDLModifier);
		
	const char* GetSDLName() const { return kSDLAvatar; }

	// trying a new approach, which is to let the climb brain read its own SDL.
	// this allows us to synch protected data members with either creating a friend
	// relationship or punching a lot of hole's in the object's encapsulation
	struct ClimbBrainVarNames
	{
		static char kStrCurMode[];
		static char kStrNextMode[];
		static char kStrAllowedDirections[];
		static char kStrAllowedDismounts[];
		static char kStrVertProbeLength[];
		static char kStrHorizProbeLength[];
		static char kStrCurStageAttached[];
		static char kStrCurStage[];
		static char kStrCurStageTime[];
		static char kStrCurStageStrength[];
		static char kStrExitStageAttached[];
		static char kStrExitStage[];
		static char kStrExitStageTime[];
		static char kStrExitStageStrength[];
	};
};

#endif	// plAvatarSDLModifier_inc
