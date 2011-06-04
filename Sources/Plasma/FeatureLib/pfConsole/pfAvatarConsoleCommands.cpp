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
//////////////////////////////////////////////////////////////////////////////
//																			//
//	Avatar Console Commands and Groups										//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifdef PLASMA_EXTERNAL_RELEASE
#define LIMIT_CONSOLE_COMMANDS 1
#endif

#include "pfConsoleCmd.h"
#include "plgDispatch.h"
#include "pfConsole.h"
#include "hsResMgr.h"

#include "../pfMessage/plArmatureEffectMsg.h"

#include "../plMessage/plOneShotMsg.h"
#include "../plMessage/plAvatarMsg.h"
#include "../plMessage/plInputEventMsg.h"
#include "../plMessage/plSimStateMsg.h"
#include "../plMessage/plCCRMsg.h"
#include "../plMessage/plLinkToAgeMsg.h"
#include "../pnMessage/plNotifyMsg.h"

#include "../plModifier/plSpawnModifier.h"

#include "../plResMgr/plKeyFinder.h"

#include "../plAvatar/plArmatureMod.h"
#include "../plAvatar/plAvBrainCritter.h"
#include "../plAvatar/plAvBrainHuman.h"
#include "../plAvatar/plAvBrainSwim.h"
#include "../plAvatar/plAvBrainGeneric.h"
#include "../plAvatar/plAvatarMgr.h"
#include "../plAvatar/plSeekPointMod.h"
#include "../plAvatar/plOneShotMod.h"
#include "../plAvatar/plAGAnim.h"
#include "../plAvatar/plAvBrainUser.h"
#include "../plAvatar/plAvBrainHuman.h"
#include "../plAvatar/plNPCSpawnMod.h"
#include "../plAvatar/plAGAnimInstance.h"
#include "../plAvatar/plArmatureEffects.h"
#include "../plAvatar/plAvTaskSeek.h"

//#include "../plHavok1/plSimulationMgr.h"
#include "../pnNetCommon/plNetApp.h"
#include "../plNetClient/plNetClientMgr.h"
#include "../plDrawable/plInstanceDrawInterface.h"
#include "../plDrawable/plDrawableSpans.h"

#define PF_SANITY_CHECK( cond, msg ) { if( !( cond ) ) { PrintString( msg ); return; } }

//// This is here so Microsoft VC won't decide to "optimize" this file out
void	pfConsoleCmdGroup::DummyAvatar( void )
{
}

/////////////////////////////////////////////////////////////////
//
// Please see pfConsoleCommands.cpp for detailed instructions on
// how to add console commands.
//
/////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////
//
// UTILITIES - LOCAL AND OTHERWISE
//
/////////////////////////////////////////////////////////////////

plKey FindSceneObjectByName(const char* name, const char* ageName, char* statusStr, bool subString=false);
plKey FindObjectByName(const char* name, int type, const char* ageName, char* statusStr, bool subString=false);
plKey FindObjectByNameAndType(const char* name, const char* typeName, const char* ageName, 
							   char* statusStr, bool subString=false);
void PrintStringF(void pfun(const char *),const char * fmt, ...);

PF_CONSOLE_GROUP( Avatar )

#ifndef LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_SUBGROUP( Avatar, Spawn )
PF_CONSOLE_SUBGROUP( Avatar, Multistage )
PF_CONSOLE_SUBGROUP( Avatar, X )		// experimental stuff
PF_CONSOLE_SUBGROUP( Avatar, Climb )
PF_CONSOLE_SUBGROUP( Avatar, Turn )     // Turning config
PF_CONSOLE_SUBGROUP( Avatar, Physics )
PF_CONSOLE_SUBGROUP( Avatar, Warp )
PF_CONSOLE_SUBGROUP( Avatar, Anim )		// anim commands
PF_CONSOLE_SUBGROUP( Avatar, AG )		// animation graph stuff
PF_CONSOLE_SUBGROUP( Avatar, LOD )

#endif // LIMIT_CONSOLE_COMMANDS

#include "../plAvatar/plAnimStage.h"

plAvBrainHuman * GetMainAvatarBrain()
{
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
	if(avatar)
	{
		plAvBrainHuman *brain = plAvBrainHuman::ConvertNoRef(avatar->GetBrain(0));
		if(brain)
			return brain;
	}
	return nil;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// THE COMMANDS
//
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
//
// SPAWNING
//
/////////////////////////////////////////////////////////////////////////////////////////

#ifndef LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD( Avatar_Spawn, Show, "", "Print a list of spawn points.")
{
	plAvatarMgr *mgr = plAvatarMgr::GetInstance();
	int n = mgr->NumSpawnPoints();

	for (int i = 0; i < n; i++)
	{
		const plSpawnModifier * spawn = mgr->GetSpawnPoint(i);
		if(spawn)
		{
			PrintStringF(PrintString, "%d. %s", i, spawn->GetKey()->GetName());
		}
	}
}

PF_CONSOLE_CMD( Avatar_Spawn, Go, "int which", "Go a spawn point indicated by number.")
{
	plAvatarMgr *mgr = plAvatarMgr::GetInstance();
	int n = params[0];
	int max = mgr->NumSpawnPoints();
	
	if(n < max)
	{
		plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
		
		if (avatar)
		{
			double fakeTime = 0.0f;
			avatar->SpawnAt(n, fakeTime);
		}
	}
}

static int whichSpawn = 0;
PF_CONSOLE_CMD( Avatar_Spawn, next, "", "Go to the next spawn point in sequence.")
{
	plAvatarMgr *mgr = plAvatarMgr::GetInstance();
	int max = mgr->NumSpawnPoints();
	
	whichSpawn = ++whichSpawn < max ? whichSpawn : 0;
	
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
	if(avatar)
	{
		PrintStringF(PrintString, "Spawning at point %d", whichSpawn);
		double fakeTime = 0.0f;
		avatar->SpawnAt(whichSpawn, fakeTime);
	}
}

PF_CONSOLE_CMD( Avatar_Spawn, prev, "", "Go to the prev spawn point in sequence.")
{
	plAvatarMgr *mgr = plAvatarMgr::GetInstance();
	int max = mgr->NumSpawnPoints();
	
	whichSpawn= --whichSpawn >= 0 ? whichSpawn: max-1;
	
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
	if(avatar)
	{
		PrintStringF(PrintString, "Spawning at point %d", whichSpawn);
		double fakeTime = 0.0f;
		avatar->SpawnAt(whichSpawn, fakeTime);
	}
}

PF_CONSOLE_CMD( Avatar_Spawn, Respawn,"", "Moves the avatar back to the start point.")
{
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
	
	if (avatar)
	{
		avatar->Spawn(0);
	}
}

PF_CONSOLE_CMD( Avatar_Spawn, SetSpawnOverride, "string spawnPointName", "Overrides the normal spawn point choice to be the object specified.")
{
	plArmatureMod::SetSpawnPointOverride( (const char *)params[ 0 ] );
	
	char str1[ 512 ];
	sprintf( str1, "Spawn point override set to object %s", (const char *)params[ 0 ] );
	PrintString( str1 );
}

PF_CONSOLE_CMD( Avatar_Spawn, DontPanic,"", "Toggles the Don't panic link flag.")
{
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();

	if (avatar)
	{
		bool state = avatar->ToggleDontPanicLinkFlag();
		char str1[256];
		sprintf(str1, "DontPanic set to %s", state?"true":"false");
		PrintString( str1 );
	}
}


/////////////////////////////////////////////////////////////////////////////////////////
//
// TURN TUNING
//
/////////////////////////////////////////////////////////////////////////////////////////

PF_CONSOLE_CMD( Avatar_Turn, GetMaxTurn, "int walk", "Show the maximum turn speed in radians per second.")
{
	plAvBrainHuman *brain = GetMainAvatarBrain();

	float maxTurn = brain->GetMaxTurnSpeed((int)params[0] != 0);

	PrintStringF(PrintString, "Avatar max turn speed is %f radians per second.", maxTurn);
}

PF_CONSOLE_CMD( Avatar_Turn, SetMaxTurn, "float maxTurn, int walk", "Set the maximum turn speed in radians per second.")
{
	plAvBrainHuman *brain = GetMainAvatarBrain();

	float newMaxTurn = params[0];

	brain->SetMaxTurnSpeed(newMaxTurn, (int)params[1] != 0);

	PrintStringF(PrintString, "Set the avatar max turn speed to %f radians per second.", newMaxTurn);
}

// TURN TIME

PF_CONSOLE_CMD( Avatar_Turn, GetTurnTime, "int walk", "Show the amount of time required to reach max turn speed.")
{
	plAvBrainHuman *brain = GetMainAvatarBrain();

	float turnTime = brain->GetTimeToMaxTurn((int)params[0] != 0);

	PrintStringF(PrintString, "The amount of time required to reach max avatar turn speed is %f seconds.", turnTime);
}

PF_CONSOLE_CMD( Avatar_Turn, SetTurnTime, "float turnTime, int walk", "Set the amount of time required to reach max turn speed.")
{
	plAvBrainHuman *brain = GetMainAvatarBrain();

	float newTurnTime = params[0];

	brain->SetTimeToMaxTurn(newTurnTime, (int)params[1] != 0);

	PrintStringF(PrintString, "Set the amount of time required to reach max avatar turn speed to %f seconds.", newTurnTime);
}

// TURN TYPE

PF_CONSOLE_CMD( Avatar_Turn, GetTurnType, "int walk", "Show the amount of time required to reach max turn speed.")
{
	plAvBrainHuman *brain = GetMainAvatarBrain();
	
	int turnType = brain->GetTurnCurve((int)params[0] != 0);
	
	PrintStringF(PrintString, "The avatar turn curve type is  %d.", turnType);
}

PF_CONSOLE_CMD( Avatar_Turn, SetTurnType, "int turnType, int walk", "Set the turn acceleration curve type [0..2].")
{
	plAvBrainHuman *brain = GetMainAvatarBrain();
	
	int newCurveType = params[0];
	
	brain->SetTurnCurve(plAvBrainHuman::TurnCurve(newCurveType), (int)params[1] != 0);
	
	PrintStringF(PrintString, "Set turn curve to %d.", newCurveType);
}


PF_CONSOLE_CMD( Avatar_Turn, SetMouseTurnSensitivity, "float sensitivity", "Set how strong the mouse affects turning.")
{
	plArmatureMod::SetMouseTurnSensitivity(params[0]);
	
	PrintStringF(PrintString, "Set mouse sensitivity to %f", (float)params[0]);
}



/////////////////////////////////////////////////////////////////////////////////////////
//
// MULTISTAGE
//
/////////////////////////////////////////////////////////////////////////////////////////


// MULTISTAGE.TRIGGER
PF_CONSOLE_CMD( Avatar_Multistage, Trigger, "string multiComp", "Triggers the named Multistage Animation component")
{
	char str[256];
	plKey key = FindObjectByNameAndType((const char*)params[0], "plMultistageBehMod", nil, str, true);
	PrintString(str);

	if (key)
	{
		plNotifyMsg *msg = TRACKED_NEW plNotifyMsg;

		msg->fType = plNotifyMsg::kActivator;
		msg->fState = 1;	// Triggered
		
		// Setup the event data in case this is a OneShot responder that needs it
		plKey playerKey = plAvatarMgr::GetInstance()->GetLocalAvatar()->GetKey();
		proPickedEventData *ed = TRACKED_NEW proPickedEventData;
		ed->fPicker = playerKey;
		ed->fPicked = key; // ???
		msg->AddEvent(ed);
		
		// Send it to the responder modifier
		msg->AddReceiver(key);
		plgDispatch::MsgSend(msg);
	}
}

// MULTISTAGE.ADVANCE
PF_CONSOLE_CMD( Avatar_Multistage, Advance, "", "Advances the avatar's current multistage to the next stage.")
{
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
	
	if (avatar)
	{
		plKey avKey = avatar->GetKey();
		
		plAvBrainGenericMsg *msg = TRACKED_NEW plAvBrainGenericMsg(nil, avKey, plAvBrainGenericMsg::kNextStage, 0, true, 0.5f);
		msg->Send();
	}
}

// MULTISTAGE.REGRESS
PF_CONSOLE_CMD( Avatar_Multistage, Regress, "", "Regresses the avatar's current multistage to the previous stage.")
{
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
	
	if (avatar)
	{
		plKey avKey = avatar->GetKey();
		
		plAvBrainGenericMsg *msg = TRACKED_NEW plAvBrainGenericMsg(nil, avKey, plAvBrainGenericMsg::kPrevStage, 0, true, 0.5f);
		msg->Send();
	}
}

PF_CONSOLE_CMD( Avatar_Multistage, Mode, "string stage1, string stage2, string stage3", "make a simple multistage")
{
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
	
	const char *one = params[0];
	const char *two = params[1];
	const char *three = params[2];
	
	PushSimpleMultiStage(avatar, one, two, three, true, true, plAGAnim::kBodyFull);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// MISCKELANYOUS
//
/////////////////////////////////////////////////////////////////////////////////////////

PF_CONSOLE_CMD( Avatar, Debug, "", "Toggle the avatar debug display.")
{
	static int toggle = 0;
	
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
	if(avatar)
	{
		toggle = toggle ? 0 : 1;
		avatar->SetDebugState(toggle);
	}
}

PF_CONSOLE_CMD( Avatar, DebugByID, "int PlayerID", "Show debug display for a specific avatar.")
{
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->FindAvatarByPlayerID((int)params[0]);
	if (avatar)
	{
		avatar->SetDebugState(!avatar->GetDebugState());
	}
}

PF_CONSOLE_CMD( Avatar, LogSmartSeek, "int enabled", "Enable/Disable smart seek logging (avatar.log)")
{
	plAvTaskSeek::fLogProcess = ((int)params[0] == 1);
}

PF_CONSOLE_CMD( Avatar, PopBrain, "", "Remove the topmost brain from the avatar. Careful there, sport.")
{
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
	if(avatar)
	{
		avatar->PopBrain();
	}
}

#include "../plScene/plRelevanceMgr.h"

PF_CONSOLE_CMD( Avatar, 
			   MarkRelevanceRegion,
			   "string regionA, string regionB, int wantsUpdate",
			   "Mark whether avatars in regionA want updates on those on regionB" )
{
	plRelevanceMgr *mgr = plRelevanceMgr::Instance();
	char *regA = params[0];
	char *regB = params[1];
	mgr->MarkRegion(mgr->GetIndex(regA), mgr->GetIndex(regB), params[2]);
}

PF_CONSOLE_CMD( Avatar,
			   ToggleRelevanceRegions,
			   "",
			   "Enable/Disable all relevance regions" )
{
	plRelevanceMgr *mgr = plRelevanceMgr::Instance();
	mgr->SetEnabled(!mgr->GetEnabled());
	
	char buff[256];
	sprintf(buff, "All relevance regions are now %s", (mgr->GetEnabled() ? "ENABLED" : "DISABLED"));
	PrintString(buff);
}

PF_CONSOLE_CMD( Avatar, SeekPoint, "string seekpoint", "Move to the given seekpoint.")
{
	char *spName = params[0];
	
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
	
	if(avatar)
	{
		char buff[256];
		plKey seekKey = FindSceneObjectByName(spName, nil, buff);
		plSeekPointMod *mod = plAvatarMgr::GetInstance()->FindSeekPoint(spName);
		
		if(mod)
		{
			plKey seekKey = mod->GetKey();
			plSeekPointMod *seekMod = (plSeekPointMod *)seekKey->GetObjectPtr();
			plSceneObject *seekTarget = seekMod->GetTarget(0);
			plKey targetKey = seekTarget->GetKey();
			
			plKey avKey = avatar->GetKey();
			hsScalar unused = 0.0f;
			plAvSeekMsg *msg = TRACKED_NEW plAvSeekMsg(nil, avKey, targetKey, unused, false);
			
			plgDispatch::MsgSend(msg);
		}
	}
}

PF_CONSOLE_CMD( Avatar, ShowLocations, "", "Show player positions/orientations" )
{
	hsBool curVal = plNetClientApp::GetInstance()->GetFlagsBit(plNetClientApp::kShowAvatars);
	plNetClientApp::GetInstance()->SetFlagsBit(plNetClientApp::kShowAvatars, !curVal);	
}

PF_CONSOLE_CMD( Avatar,
			   SetFootEffect,
			   "int group",
			   "Force the avatar to use certain footstep effects" )
{
	const plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
	
	if (avMod)
	{
		plArmatureEffectStateMsg *msg = TRACKED_NEW plArmatureEffectStateMsg();
		msg->AddReceiver(avMod->GetArmatureEffects()->GetKey());
		msg->fSurface = (int)params[0];
		plgDispatch::MsgSend(msg);
	}
}

PF_CONSOLE_CMD( Avatar, SetStealthMode, "int mode", "Set the stealth mode of your avatar.")
{
	const plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
	if (avMod)
	{
		int mode=params[0];
		plKey avKey=avMod->GetTarget(0)->GetKey();
		int level = mode==plAvatarStealthModeMsg::kStealthVisible ? 0 : 1;
		
		// send msg to make myself invisible locally
		plAvatarStealthModeMsg *msg = TRACKED_NEW plAvatarStealthModeMsg();
		msg->SetSender(avKey);
		msg->fMode = mode;
		msg->fLevel = level;
		plgDispatch::MsgSend(msg);
		
		// send net msg to other players to synch them up
		// the msg will go to their NetClientMgr who will decide whether they see
		// our avatar as total or semi-invisible based on the invis level.
		plCCRInvisibleMsg *invisMsg = TRACKED_NEW plCCRInvisibleMsg;		// ctor sets flags and receiver
		invisMsg->fInvisLevel=level;
		invisMsg->fAvKey=avKey;
		invisMsg->Send();
	}
}

PF_CONSOLE_CMD( Avatar, SortFaces, "", "Toggle sorting of polys on the avatar" )
{
	const plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
	const plSceneObject *so = avMod->GetClothingSO(0);
	
	plInstanceDrawInterface *idi = plInstanceDrawInterface::ConvertNoRef(const_cast<plDrawInterface*>(so->GetDrawInterface()));
	plDrawableSpans *drawable = idi->GetInstanceDrawable();
	drawable->SetNativeProperty(plDrawable::kPropSortFaces, !drawable->GetNativeProperty(plDrawable::kPropSortFaces));
}

#endif // LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD( Avatar, SetMouseTurnSensitivity, "float sensitivity", "Set how strong the mouse affects turning.")
{
	plArmatureMod::SetMouseTurnSensitivity(params[0]);
	
	PrintStringF(PrintString, "Set mouse sensitivity to %f", (float)params[0]);
}


PF_CONSOLE_CMD( Avatar, ClickToTurn, "bool b", "Set click-to-turn functionality.")
{
	bool b = params[0];
	plArmatureMod::fClickToTurn = b;
}

PF_CONSOLE_CMD( Avatar, FakeLinkToObj, "string objName", "Pseudo-Link the avatar to the specified object's location")
{
	char *spName = params[0];
	char buff[256];
	plKey seekKey = FindSceneObjectByName(spName, nil, buff);
	if (!seekKey)
	{
		PrintString("Can't find object with that name, fake link failed.");
		return;
	}
	plPseudoLinkEffectMsg* msg = TRACKED_NEW plPseudoLinkEffectMsg;
	msg->fAvatarKey = plNetClientMgr::GetInstance()->GetLocalPlayerKey();
	msg->fLinkObjKey = seekKey;
	plgDispatch::MsgSend(msg);
	
}


#ifndef LIMIT_CONSOLE_COMMANDS

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// PHYSICS
//
///////////////////////////////////////////////////////////////////////////////////////////////////

PF_CONSOLE_CMD( Avatar_Physics, TogglePhysical, "", "Disable/enable physics on the avatar.")
{
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
	
	if(avatar)
	{
		plControlEventMsg* pMsg = TRACKED_NEW plControlEventMsg;
		pMsg->SetControlActivated(true);
		pMsg->SetControlCode(B_CONTROL_TOGGLE_PHYSICAL);
		
		avatar->MsgReceive(pMsg);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Animation
//
///////////////////////////////////////////////////////////////////////////////////////////////////

PF_CONSOLE_CMD( Avatar_Anim, BlendAnim, "string Animation, float blendFactor", "Blend the given animation with the current animation.")
{
	char *animationName = params[0];
	float blendFactor = params[1];
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();

	if (avatar && animationName)
	{
		plAGAnim * anim = plAGAnim::FindAnim(animationName);
		if(anim)
		{
			plAGAnimInstance * inst = avatar->AttachAnimationBlended(animationName, blendFactor);
			inst->SetLoop(true);
		} else
			PrintString("BlendAnim: Couldn't find animation.");
	}
}

PF_CONSOLE_CMD( Avatar_Anim, BlendAnimPri, "string Animation, float blendFactor, int priority", "Blend animation using priority.")
{
	char *animationName = params[0];
	float blendFactor = params[1];
	int priority = params[2];
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();

	if (avatar && animationName)
	{
		plAGAnim * anim = plAGAnim::FindAnim(animationName);
		if(anim)
		{
			plAGAnimInstance * inst = avatar->AttachAnimationBlended(animationName, blendFactor, priority);
			inst->SetLoop(true);
		} else
			PrintString("BlendAnim: Couldn't find animation.");
	}
}

PF_CONSOLE_CMD( Avatar_Anim, PlaySimpleAnim, "string AvatarName, string Animation", "Play a simple (root not animated) one time animation on the avatar")
{
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->FindAvatarByModelName(params[0]);
	if (avatar)
		avatar->PlaySimpleAnim(params[1]);
}

PF_CONSOLE_CMD( Avatar_Anim, DetachAnim, "string Animation", "Remove the given animation from the avatar.")
{
	char *animationName = params[0];
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();

	if (avatar && animationName)
	{
		plAGAnimInstance * instance = avatar->FindAnimInstance(animationName);
		if(instance)
			avatar->DetachAnimation(instance);
		else
			PrintString("DetachAnim: Couldn't find animation on avatar.");
	}
}

PF_CONSOLE_CMD( Avatar_Anim, SetBlend, "string Animation, float blend", "Set the blend of the given animation.")
{
	char *animationName = params[0];
	float blend = params[1];
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();

	if (avatar && animationName)
	{
		plAGAnimInstance *anim = avatar->FindAnimInstance(animationName);
		if(anim)
			anim->SetBlend(blend);
		else 
			PrintString("SetBlend: Couldn't find animation.");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// LOD
//
///////////////////////////////////////////////////////////////////////////////////////////////////

PF_CONSOLE_CMD( Avatar_LOD, SetLOD, "int lod", "Show only the selected LOD.")
{
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();

	if(avatar)
	{
		plArmatureLODMod *lodder = plArmatureLODMod::ConvertNoRef(avatar);

		if(lodder)
		{
			int lod = params[0];
			
			lodder->SetLOD(lod);
		} else {
			PrintString("SetLOD: found avatar, but it doesn't support LOD.");
		}
	} else {
		PrintString("SetLOD: couldn't find avatar with that name.");
	}
}


PF_CONSOLE_CMD( Avatar_LOD, LimitLOD, "int newLOD", "Zero is (always) highest detail; 2 is (currently) lowest." )
{
	int newLOD = params[0];

	if(newLOD >= 0 && newLOD <= 2)
		plArmatureLODMod::fMinLOD = newLOD;
}

PF_CONSOLE_CMD( Avatar_LOD, SetLODDistance, "float newDist", "Set Distance for switching Avatar LOD" )
{
	plArmatureLODMod::fLODDistance = float(params[0]);
}

PF_CONSOLE_CMD( Avatar_LOD,  GetLODDistance, "", "Get Distance for switching Avatar LOD" )
{
	char buffer[256];
	
	sprintf(buffer, "Lod Distance = %f", plArmatureLODMod::fLODDistance);
	PrintString(buffer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// CLIMBING
//
///////////////////////////////////////////////////////////////////////////////////////////////////



#include "../plAvatar/plAvBrainClimb.h"

PF_CONSOLE_CMD( Avatar_Climb, Start, "string direction", "Specify initial mount direction: up, down, left, right")
{
	plArmatureMod *avMod = const_cast<plArmatureMod *>(plAvatarMgr::GetInstance()->GetLocalAvatar());
	if(avMod)
	{
		const char *dirStr = params[0];
		plAvBrainClimb::Mode mode;
		if(stricmp(dirStr, "up") == 0)
			mode = plAvBrainClimb::kMountingUp;
		else if(stricmp(dirStr, "down") == 0)
			mode = plAvBrainClimb::kMountingDown;
		else if(stricmp(dirStr, "left") == 0)
			mode = plAvBrainClimb::kMountingLeft;
		else if(stricmp(dirStr, "right") == 0)
			mode = plAvBrainClimb::kMountingRight;
		plAvBrainClimb *brain = TRACKED_NEW plAvBrainClimb(mode);
		avMod->PushBrain(brain);
	}
}

PF_CONSOLE_CMD( Avatar_Climb, EnableDismount, "string direction", "Let the avatar dismount in the specified direction.")
{
	plArmatureMod *avMod = const_cast<plArmatureMod *>(plAvatarMgr::GetInstance()->GetLocalAvatar());
	if(avMod)
	{
		plKey mgr = plAvatarMgr::GetInstance()->GetKey();
		plKey avKey = avMod->GetKey();
		const char *dirStr = params[0];
		plClimbMsg::Direction dir;
		if(stricmp(dirStr, "up") == 0)
			dir = plClimbMsg::kUp;
		else if(stricmp(dirStr, "down") == 0)
			dir = plClimbMsg::kDown;
		else if(stricmp(dirStr, "left") == 0)
			dir = plClimbMsg::kLeft;
		else if(stricmp(dirStr, "right") == 0)
			dir = plClimbMsg::kRight;
		plClimbMsg *msg = TRACKED_NEW plClimbMsg(mgr, avKey, plClimbMsg::kEnableDismount, dir, true);
		msg->Send();
	}
}

PF_CONSOLE_CMD( Avatar_Climb, EnableClimb, "string direction, int onOff", "Allow or forbid climbing in the given direction.")
{
	plArmatureMod *avMod = const_cast<plArmatureMod *>(plAvatarMgr::GetInstance()->GetLocalAvatar());
	if(avMod)
	{
		plKey mgr = plAvatarMgr::GetInstance()->GetKey();
		plKey avKey = avMod->GetKey();
		const char *dirStr = params[0];
		plClimbMsg::Direction dir;
		if(stricmp(dirStr, "up") == 0)
			dir = plClimbMsg::kUp;
		else if(stricmp(dirStr, "down") == 0)
			dir = plClimbMsg::kDown;
		else if(stricmp(dirStr, "left") == 0)
			dir = plClimbMsg::kLeft;
		else if(stricmp(dirStr, "right") == 0)
			dir = plClimbMsg::kRight;
		hsBool enable = static_cast<int>(params[1]) ? true : false;
		plClimbMsg *msg = TRACKED_NEW plClimbMsg(mgr, avKey, plClimbMsg::kEnableClimb, dir, enable);
		msg->Send();
	}
}

PF_CONSOLE_CMD( Avatar_Climb, Release, "", "")
{
	plArmatureMod *avMod = const_cast<plArmatureMod *>(plAvatarMgr::GetInstance()->GetLocalAvatar());
	if(avMod)
	{
		plKey mgr = plAvatarMgr::GetInstance()->GetKey();
		plKey avKey = avMod->GetKey();
		plClimbMsg *msg = TRACKED_NEW plClimbMsg(mgr, avKey, plClimbMsg::kRelease);
		msg->Send();
	}
}

PF_CONSOLE_CMD( Avatar_Climb, FallOff, "", "")
{
	plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
	if(avMod)
	{
		plKey mgr = plAvatarMgr::GetInstance()->GetKey();
		plKey avKey = avMod->GetKey();
		plClimbMsg *msg = TRACKED_NEW plClimbMsg(mgr, avKey, plClimbMsg::kFallOff);
		msg->Send();
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// SWIMMING
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


PF_CONSOLE_SUBGROUP( Avatar, Swim )

PF_CONSOLE_CMD( Avatar_Swim, Start, "", "")
{
	plArmatureMod *avMod = const_cast<plArmatureMod*>(plAvatarMgr::GetInstance()->GetLocalAvatar());
	if(avMod)
	{
		plAvBrainSwim * brayne = TRACKED_NEW plAvBrainSwim();
		avMod->PushBrain(brayne);
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  WARP
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// void WarpPlayerToAnother(hsBool iMove, UInt32 remoteID)
PF_CONSOLE_CMD( Avatar_Warp, WarpToPlayer, "int PlayerID", "Warp our player to the same position as another player.")
{
	plAvatarMgr::WarpPlayerToAnother(true, (int)params[0]);
}

PF_CONSOLE_CMD( Avatar_Warp, WarpPlayerHere, "int PlayerID", "Warp another player to the same position as us.")
{
	plAvatarMgr::WarpPlayerToAnother(false, (int)params[0]);
}

PF_CONSOLE_CMD( Avatar_Warp, WarpToXYZ, "float x, float y, float z", "Warp our avatar to the given location.")
{
	plAvatarMgr::WarpPlayerToXYZ((float)params[0], (float)params[1], (float)params[2]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// AG (Animation Graph)
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "hsTimer.h"

PF_CONSOLE_CMD( Avatar_AG, DumpFull, "", "print out the animation graph for the avatar")
{
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();

	double time = hsTimer::GetSysSeconds();

	avatar->DumpAniGraph(nil, false, time);
}

PF_CONSOLE_CMD( Avatar_AG, DumpFullOptimized, "", "print out the optimized animation graph for the avatar")
{
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();

	double time = hsTimer::GetSysSeconds();
	avatar->DumpAniGraph(nil, true, time);
}

PF_CONSOLE_CMD( Avatar_AG, DumpSingle, "string boneName", "print out the animation graph for the given (avatar) bone")
{
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
	double time = hsTimer::GetSysSeconds();
	const char *bone = params[0];
	avatar->DumpAniGraph(bone, false, time);
}

PF_CONSOLE_CMD( Avatar_AG, DumpSingleOptimized, "string boneName", "print out the optimized animatoin graph for the given (avatar) bone")
{
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
	double time = hsTimer::GetSysSeconds();
	const char *bone = params[0];
	avatar->DumpAniGraph(bone, true, time);
}

#endif // LIMIT_CONSOLE_COMMANDS
