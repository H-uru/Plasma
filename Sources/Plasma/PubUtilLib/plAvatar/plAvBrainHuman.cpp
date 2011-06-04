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
#include "hsConfig.h"
#include "hsWindows.h"

#include "plAvCallbackAction.h"					// subclasses a havok object; must be in first include section


#include "plAvBrainHuman.h"
#include "plAvBrainClimb.h"
#include "plAvBrainDrive.h"
#include "plAvBrainGeneric.h"
#include "plAvBrainSwim.h"
#include "plArmatureMod.h"
#include "plAGModifier.h"
#include "plMatrixChannel.h"
#include "plAvTask.h"
#include "plAvTaskBrain.h"
#include "plAvTaskSeek.h"
#include "plAGAnim.h"
#include "plAGAnimInstance.h"
#include "plAvatarMgr.h"
#include "plAnimStage.h"
#include "plAvatarClothing.h"

#include "hsTimer.h"
#include "hsGeometry3.h"
#include "float.h"
#include "plPipeline.h"
#include "plgDispatch.h"
#include "hsQuat.h"
#include "plPhysical.h"
#include "../plStatusLog/plStatusLog.h"

#include "../pnNetCommon/plNetApp.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../plInputCore/plAvatarInputInterface.h"
#include "../plInputCore/plInputDevice.h"
#include "../plMath/plRandom.h"
#include "../plPipeline/plDebugText.h"
#include "../plNetClient/plNetLinkingMgr.h"

#include "../plMessage/plAvatarMsg.h"
#include "../plMessage/plClimbMsg.h"
#include "../plMessage/plInputEventMsg.h"
#include "../plMessage/plLOSHitMsg.h"
#include "../plMessage/plLOSRequestMsg.h"
#include "../plMessage/plSimStateMsg.h"
#include "../plMessage/plSwimMsg.h"
#include "../plMessage/plAgeLoadedMsg.h"
#include "../pnMessage/plWarpMsg.h"
#include "../pnMessage/plProxyDrawMsg.h"
#include "../plMessage/plRideAnimatedPhysMsg.h"

float plAvBrainHuman::fWalkTimeToMaxTurn = .3f;
float plAvBrainHuman::fRunTimeToMaxTurn = .1f;
float plAvBrainHuman::fWalkMaxTurnSpeed = 2.0f;
float plAvBrainHuman::fRunMaxTurnSpeed = 1.7;
plAvBrainHuman::TurnCurve plAvBrainHuman::fWalkTurnCurve = plAvBrainHuman::kTurnExponential;
plAvBrainHuman::TurnCurve plAvBrainHuman::fRunTurnCurve = plAvBrainHuman::kTurnExponential;

const hsScalar plAvBrainHuman::kAirTimePanicThreshold = 10; // seconds

void plAvBrainHuman::SetTimeToMaxTurn(float time, hsBool walk)
{
	if (walk)
		fWalkTimeToMaxTurn = time;
	else
		fRunTimeToMaxTurn = time;
}

float plAvBrainHuman::GetTimeToMaxTurn(hsBool walk)
{
	return (walk ? fWalkTimeToMaxTurn : fRunTimeToMaxTurn);
}

void plAvBrainHuman::SetMaxTurnSpeed(float radsPerSec, hsBool walk)
{
	if (walk)
		fWalkMaxTurnSpeed = radsPerSec;
	else
		fRunMaxTurnSpeed = radsPerSec;
}

float plAvBrainHuman::GetMaxTurnSpeed(hsBool walk)
{
	return (walk ? fWalkMaxTurnSpeed : fRunMaxTurnSpeed);
}

void plAvBrainHuman::SetTurnCurve(TurnCurve curve, hsBool walk)
{
	if (walk)
		fWalkTurnCurve = curve;
	else
		fRunTurnCurve = curve;
}

plAvBrainHuman::TurnCurve plAvBrainHuman::GetTurnCurve(hsBool walk)
{
	return (walk ? fWalkTurnCurve : fRunTurnCurve);
}

plAvBrainHuman::plAvBrainHuman(bool isActor /* = false */) : 
	fHandleAGMod(nil),
	fStartedTurning(-1.0f),
	fCallbackAction(nil),
	fPreconditions(0),
	fIsActor(isActor)
{
}

hsBool plAvBrainHuman::Apply(double timeNow, hsScalar elapsed)
{
#ifndef _DEBUG
	try
	{
#endif
		// SetTurnStrength runs first to make sure it's set to a sane value
		// (or cleared). RunStandardBehaviors may overwrite it.
		fCallbackAction->SetTurnStrength(IGetTurnStrength(timeNow));
		RunStandardBehaviors(timeNow, elapsed);
		fCallbackAction->RecalcVelocity(timeNow, timeNow - elapsed, (fPreconditions & plHBehavior::kBehaviorTypeNeedsRecalcMask));
		
		plArmatureBrain::Apply(timeNow, elapsed);
#ifndef _DEBUG
	} catch (...)
	{
		// just catch all the crashes on exit...
		plStatusLog *log = plAvatarMgr::GetInstance()->GetLog();
		log->AddLine("plAvBrainHuman::Apply - crash caught");
	}
#endif
	
	return true;
}

void plAvBrainHuman::Activate(plArmatureModBase *avMod)
{
	plArmatureBrain::Activate(avMod);
	
	IInitBoneMap();
	IInitAnimations();
	if (!fCallbackAction)
	{
		plSceneObject* avObj = fArmature->GetTarget(0);
		plAGModifier* agMod = const_cast<plAGModifier*>(plAGModifier::ConvertNoRef(FindModifierByClass(avObj, plAGModifier::Index())));
		plPhysicalControllerCore* controller = avMod->GetController();
		fCallbackAction = TRACKED_NEW plWalkingController(avObj, agMod->GetApplicator(kAGPinTransform), controller);
		fCallbackAction->ActivateController();
	}
	
	
	plSceneObject *avSO = fAvMod->GetTarget(0);
	hsBool isLocal = avSO->IsLocallyOwned();
	
	if (fAvMod->GetClothingOutfit() && fAvMod->GetClothingOutfit()->fGroup != plClothingMgr::kClothingBaseNoOptions)
	{
		if (fAvMod->IsLocalAvatar()) 
			fAvMod->GetClothingOutfit()->ReadFromVault();
		else 
		{
			fAvMod->GetClothingOutfit()->WearDefaultClothing();
			fAvMod->GetClothingOutfit()->ForceUpdate(true);
		}
	}
	if (fAvMod == plAvatarMgr::GetInstance()->GetLocalAvatar())
		plAvatarInputInterface::GetInstance()->ForceAlwaysRun(plKeyboardDevice::GetInstance()->IsCapsLockKeyOn() != 0);
}

void plAvBrainHuman::IInitBoneMap()
{
	struct tuple {
		HumanBoneID fID;
		const char * fName;
	};

	tuple tupleMap[] = 
	{
		{ Pelvis,			"Bone_Root" },
		// left leg
		{ LThigh,			"Bone_LThigh" },
		{ LCalf,			"Bone_LCalf" },
		{ LFoot,			"Bone_LFoot" },
		{ LFootPrint,		"Print_L Foot" },
		{ LToe0,			"Bone_LToe" },

		// right leg
		{ RThigh,			"Bone_RThigh" },
		{ RCalf,			"Bone_RCalf" },
		{ RFoot,			"Bone_RFoot" },
		{ RFootPrint,		"Print_R Foot" },
		{ RToe0,			"Bone_RToe" },

		// spine and head, starting at base of spine
		{ Spine,			"Bone_Spine0" },
		{ TrunkPrint,		"Print_Trunk" },
		{ Spine1,			"Bone_Spine1" },
		{ Spine2,			"Bone_Spine2" },
		{ Neck,				"Bone_Neck" },
		{ Head,				"Bone_Head" },
		{ Jaw,				"Bone_Jaw" },

		// left face bones
		{ LMouthLower,		"Bone_LMouthLower" },
		{ RMouthLower,		"Bone_RMouthLower" },
		{ LBrowInner,		"Bone_LBrowInner" },
		{ LBrowOuter,		"Bone_LBrowOuter" },
		{ LCheek,			"Bone_LCheek" },
		{ LEye,				"Bone_LEye" },
		{ LEyeLid01,		"Bone_LEyeLid1" },
		{ LEyeLid02,		"Bone_LEyeLid2" },
		{ LMouthCorner,		"Bone_LMouthCorner" },
		{ LMouthUpper,		"Bone_LMouthUpper" },

		// right face bones
		{ RBrowInner,		"Bone_RBrowInner" },
		{ RBrowOuter,		"Bone_RBrowOuter" },
		{ RCheek,			"Bone_RCheek" },
		{ REye,				"Bone_REye" },
		{ REyeLid01,		"Bone_REyeLid1" },
		{ REyeLid02,		"Bone_REyeLid2" },
		{ RMouthCorner,		"Bone_RMouthCorner" },
		{ RMouthUpper,		"Bone_RMouthUpper" },
		
		// Left Arm
		{ LClavicle,		"Bone_LClavicle" },
		{ LUpperArm,		"Bone_LUpperArm" },
		{ LForearm,			"Bone_LForearm" },
		{ LHand,			"Bone_LHand" },
		{ LHandPrint,		"Print_L Hand" },
		{ LMiddleFinger1,	"Bone_LMiddle1" },
		{ LMiddleFinger2,	"Bone_LMiddle2" },
		{ LMiddleFinger3,	"Bone_LMiddle3" },
		{ LPinkyFinger1,	"Bone_LPinky1" },
		{ LPinkyFinger2,	"Bone_LPinky2" },
		{ LPinkyFinger3,	"Bone_LPinky3" },
		{ LPointerFinger1,	"Bone_LPointer1" },
		{ LPointerFinger2,	"Bone_LPointer2" },
		{ LPointerFinger3,	"Bone_LPointer3" },
		{ LRingFinger1,		"Bone_LRing1" },
		{ LRingFinger2,		"Bone_LRing2" },
		{ LRingFinger3,		"Bone_LRing3" },
		{ LThumb1,			"Bone_LThumb1" },
		{ LThumb2,			"Bone_LThumb2" },
		{ LThumb3,			"Bone_LThumb3" },
		
		// Right Arm
		{ RClavicle,		"Bone_RClavicle" },
		{ RUpperArm,		"Bone_RUpperArm" },
		{ RForearm,			"Bone_RForearm" },
		{ RHand,			"Bone_RHand" },
		{ RHandPrint,		"Print_R Hand" },
		{ RMiddleFinger1,	"Bone_RMiddle1" },
		{ RMiddleFinger2,	"Bone_RMiddle2" },
		{ RMiddleFinger3,	"Bone_RMiddle3" },
		{ RPinkyFinger1,	"Bone_RPinky1" },
		{ RPinkyFinger2,	"Bone_RPinky2" },
		{ RPinkyFinger3,	"Bone_RPinky3" },
		{ RPointerFinger1,	"Bone_RPointer1" },
		{ RPointerFinger2,	"Bone_RPointer2" },
		{ RPointerFinger3,	"Bone_RPointer3" },
		{ RRingFinger1,		"Bone_RRing1" },
		{ RRingFinger2,		"Bone_RRing2" },
		{ RRingFinger3,		"Bone_RRing3" },
		{ RThumb1,			"Bone_RThumb1" },
		{ RThumb2,			"Bone_RThumb2" },
		{ RThumb3,			"Bone_RThumb3" },
	};

	int numTuples = sizeof(tupleMap) / sizeof(tuple);

	for(int i = 0; i < numTuples; i++)
	{
		HumanBoneID id = tupleMap[i].fID;
		const char * name = tupleMap[i].fName;
		
		const plSceneObject * bone = this->fAvMod->FindBone(name);
		if( bone )
		{
			fAvMod->AddBoneMapping(id, bone);
		}
		else
			hsStatusMessageF("Couldn't find standard bone %s.", name);
	}
}

plAvBrainHuman::~plAvBrainHuman()
{
	int i;
	for (i = 0; i < fBehaviors.GetCount(); i++)
		delete fBehaviors[i];
	fBehaviors.Reset();

	delete fCallbackAction;
	fCallbackAction = nil;
}

void plAvBrainHuman::Deactivate()
{
	// fAvMod will be nil here when exporting.
	if (fAvMod)
		plAvatarMgr::GetInstance()->RemoveAvatar(fAvMod);		// unregister

	plArmatureBrain::Deactivate();
}

void plAvBrainHuman::Suspend()
{
	// Kind of hacky... but this is a rather rare case.
	// If the user lets up on the PushToTalk key in another brain
	// we'll miss the message to take off the animation.
	char *chatAnimName = fAvMod->MakeAnimationName("Talk");	
	plAGAnimInstance *anim = fAvMod->FindAnimInstance(chatAnimName);
	delete [] chatAnimName;
	if (anim)
		anim->FadeAndDetach(0, 1);

	IdleOnly();
	plArmatureBrain::Suspend();
}

void plAvBrainHuman::Resume()
{
	// If we were in another brain when the key was pressed, we missed it.
	if (fAvMod->GetInputFlag(S_PUSH_TO_TALK))
		IChatOn();
	
	fCallbackAction->Reset(false);
	
	plArmatureBrain::Resume();
}

hsBool plAvBrainHuman::IHandleControlMsg(plControlEventMsg* msg)
{
	ControlEventCode moveCode = msg->GetControlCode();

	if( msg->ControlActivated() )
	{
		switch(moveCode)
		{
		case B_CONTROL_TOGGLE_PHYSICAL:
			{
#ifndef PLASMA_EXTERNAL_RELEASE		// external clients can't go non-physical
				plAvBrainDrive *driver = TRACKED_NEW plAvBrainDrive(20, 1);
				fAvMod->PushBrain(driver);
#endif
				return true;
			}
			break;
		case S_PUSH_TO_TALK:
			fAvMod->SetInputFlag(S_PUSH_TO_TALK, true);
			IChatOn();
			return true;
			break;
		}
	} else {
		switch(moveCode)
		{
		case S_PUSH_TO_TALK:
			fAvMod->SetInputFlag(S_PUSH_TO_TALK, false);			
			IChatOff();
			return true;
			break;
		}
	}
	return false;
}

bool plAvBrainHuman::IsMovingForward()
{
	if ((fBehaviors.Count() <= kWalk) || (fBehaviors.Count() <= kRun))
		return false; // behaviors aren't set up yet
	return (fBehaviors[kWalk]->GetStrength() > 0 || fBehaviors[kRun]->GetStrength() > 0);
}

bool plAvBrainHuman::IsBehaviorPlaying(int behavior)
{
	if ((behavior < 0) || (behavior >= fBehaviors.Count()))
		return false;
	if (!fBehaviors[behavior])
		return false;
	return (fBehaviors[behavior]->GetStrength() > 0);
}

void plAvBrainHuman::Write(hsStream *stream, hsResMgr *mgr)
{
	plArmatureBrain::Write(stream, mgr);

	stream->WriteBool(fIsActor);
}

void plAvBrainHuman::Read(hsStream *stream, hsResMgr *mgr)
{
	plArmatureBrain::Read(stream, mgr);

	fIsActor = stream->ReadBool();
}

hsBool plAvBrainHuman::MsgReceive(plMessage * msg)
{
	plControlEventMsg *ctrlMsg = plControlEventMsg::ConvertNoRef(msg);
	if (ctrlMsg) 
	{
		return IHandleControlMsg(ctrlMsg);
	}
	
	plClimbMsg *climb = plClimbMsg::ConvertNoRef(msg);
	if (climb) {
		return IHandleClimbMsg(climb);
	}

	plSwimMsg *swim = plSwimMsg::ConvertNoRef(msg);
	if (swim) 
	{
		if (swim->GetIsEntering())
		{
			plAvBrainSwim *swimBrain = TRACKED_NEW plAvBrainSwim();
			swimBrain->MsgReceive(swim);
			fAvMod->PushBrain(swimBrain);
		} 
		else 
		{
			hsStatusMessage("Got non-entering swim message. Discarding.");
		}
		return true;
	}
	plRideAnimatedPhysMsg *ride = plRideAnimatedPhysMsg::ConvertNoRef(msg);
	if(ride)
	{
		if(ride->Entering())
		{
			//plAvBrainRideAnimatedPhysical *rideBrain = TRACKED_NEW plAvBrainRideAnimatedPhysical();
			//fAvMod->PushBrain(rideBrain);
			delete fCallbackAction;
			plSceneObject* avObj = fArmature->GetTarget(0);
			plAGModifier* agMod = const_cast<plAGModifier*>(plAGModifier::ConvertNoRef(FindModifierByClass(avObj, plAGModifier::Index())));
			plPhysicalControllerCore* controller = fAvMod->GetController();
			fCallbackAction= TRACKED_NEW plRidingAnimatedPhysicalController(avObj, agMod->GetApplicator(kAGPinTransform), controller);
			fCallbackAction->ActivateController();
		
		}
		else
		{
			delete fCallbackAction;
			plSceneObject* avObj = fArmature->GetTarget(0);
			plAGModifier* agMod = const_cast<plAGModifier*>(plAGModifier::ConvertNoRef(FindModifierByClass(avObj, plAGModifier::Index())));
			plPhysicalControllerCore* controller = fAvMod->GetController();
			fCallbackAction= TRACKED_NEW plWalkingController(avObj, agMod->GetApplicator(kAGPinTransform), controller);
			fCallbackAction->ActivateController();
			//hsStatusMessage("Got an exiting  ride animated physical message");
		}
	}

	return plArmatureBrain::MsgReceive(msg);
}

hsBool plAvBrainHuman::IHandleClimbMsg(plClimbMsg *msg)
{
	bool isStartClimb = msg->fCommand == plClimbMsg::kStartClimbing;
	if(isStartClimb)
	{
		// let's build a seek task to get us to the attach point
		plKey seekTarget = msg->fTarget;
		plAvTaskSeek *seekTask = TRACKED_NEW plAvTaskSeek(seekTarget);
		QueueTask(seekTask);

		// now a brain task to start the actual climb.
		plAvBrainClimb::Mode startMode;
		switch(msg->fDirection)
		{
		case plClimbMsg::kUp:
			startMode = plAvBrainClimb::kMountingUp;
			break;
		case plClimbMsg::kDown:
			startMode = plAvBrainClimb::kMountingDown;
			break;
		case plClimbMsg::kLeft:
			startMode = plAvBrainClimb::kMountingLeft;
			break;
		case plClimbMsg::kRight:
			startMode = plAvBrainClimb::kMountingRight;
			break;
		}
		plAvBrainClimb *brain = TRACKED_NEW plAvBrainClimb(startMode);
		plAvTaskBrain *brainTask = TRACKED_NEW plAvTaskBrain(brain);
		QueueTask(brainTask);
	}
	// ** potentially controversial:
	// It's fairly easy for a human brain to hit a climb trigger - like when falling off a wall.
	// When this happens, We should just eat the message to keep it from travelling any further.
	// The argument against is that there might be a climb brain that actually wants the message,
	// but if that were true the message would have been given to that climb brain first.
	return true;
}

hsScalar plAvBrainHuman::IGetTurnStrength(double timeNow)
{
	float result = 0.f;
	float timeToMaxTurn, maxTurnSpeed;
	plAvBrainHuman::TurnCurve turnCurve;
	if (fAvMod->FastKeyDown())
	{
		timeToMaxTurn = fRunTimeToMaxTurn;
		maxTurnSpeed = fRunMaxTurnSpeed;
		turnCurve = fRunTurnCurve;
	}
	else
	{
		timeToMaxTurn = fWalkTimeToMaxTurn;
		maxTurnSpeed = fWalkMaxTurnSpeed;
		turnCurve = fWalkTurnCurve;
	}
	
	plArmatureBehavior * turnLeft  = fBehaviors.Count() >= kMovingTurnLeft  ? fBehaviors[kMovingTurnLeft]  : nil;
	plArmatureBehavior * turnRight = fBehaviors.Count() >= kMovingTurnRight ? fBehaviors[kMovingTurnRight] : nil;
	
	hsScalar turnLeftStrength = turnLeft ? turnLeft->GetStrength() : 0.f;
	hsScalar turnRightStrength = turnRight ? turnRight->GetStrength() : 0.f;
	
	
	// Turning based on keypress
	if ((turnLeftStrength > 0.f)
	|| (turnRightStrength > 0.f)
	|| (!fCallbackAction->IsOnGround() 
		&& !fCallbackAction->IsControlledFlight())
	) {
		float t = (float)(timeNow - fStartedTurning);
		float turnSpeed;
		if(t > timeToMaxTurn)
		{
			turnSpeed = maxTurnSpeed;
		} else {
			float n = t / timeToMaxTurn;	// normalize
			
			switch(turnCurve) {
			case kTurnLinear:
				// linear
				turnSpeed = n * maxTurnSpeed;
				break;
			case kTurnExponential:
				// exponential
				turnSpeed = (n * n) * maxTurnSpeed;
				break;
			case kTurnLogarithmic:
				// logarithmic
				turnSpeed = n > .1 ? log10(n * 10) * maxTurnSpeed : .00001f;
				break;
			default:
				hsAssert(false, "What the heck?");
				turnSpeed = 0.0f;
			}
			
		}
		result += fAvMod->GetKeyTurnStrength() * turnSpeed;
	}
	
	if (!fCallbackAction->IsControlledFlight())
		result += fAvMod->GetAnalogTurnStrength() * maxTurnSpeed;
	
	return result;	
}	

hsBool plAvBrainHuman::IHandleTaskMsg(plAvTaskMsg *msg)
{
	if(plAvSeekMsg * seekM = plAvSeekMsg::ConvertNoRef(msg))
	{
		// seek and subclasses always have a seek first
		if(seekM->fSmartSeek)
		{
			// use smart seek
			plAvTaskSeek * seek = TRACKED_NEW plAvTaskSeek(seekM);
			QueueTask(seek);
		}
		else
		if (!seekM->fNoSeek)
		{
			// use dumb seek
			plAvSeekTask *seek = TRACKED_NEW plAvSeekTask(seekM->fSeekPoint, seekM->fAlignType, seekM->fAnimName);
			QueueTask(seek);
		}
		// else don't seek at all.

		plAvOneShotMsg * oneshotM = plAvOneShotMsg::ConvertNoRef(msg);
		if(oneshotM)
		{
			// if it's a oneshot, add the oneshot task as well
			plAvOneShotTask *oneshot = TRACKED_NEW plAvOneShotTask(oneshotM, fAvMod, this);
			QueueTask(oneshot);
		}
	} else if (plAvPushBrainMsg *pushM = plAvPushBrainMsg::ConvertNoRef(msg)) {
		plAvTaskBrain * push = TRACKED_NEW plAvTaskBrain(pushM->fBrain);
		QueueTask(push);
	} else if (plAvPopBrainMsg *popM = plAvPopBrainMsg::ConvertNoRef(msg)) {
		plAvTaskBrain * pop = TRACKED_NEW plAvTaskBrain();
		QueueTask(pop);
	} else if (plAvTaskMsg *taskM = plAvTaskMsg::ConvertNoRef(msg)) {
		plAvTask *task = taskM->GetTask();
		QueueTask(task);
	} else {
		hsStatusMessageF("Couldn't recognize task message type.\n");
		return plArmatureBrain::IHandleTaskMsg(msg);
	}
	return true;
}

void plAvBrainHuman::ResetIdle()
{
	if (fBehaviors.Count() > kIdle)
		fBehaviors[kIdle]->Rewind();
}

void plAvBrainHuman::IdleOnly(bool instantOff)
{
	if (!fCallbackAction)
		return;

	hsScalar rate = instantOff ? 0.f : 1.f;

	int i;
	for (i = kWalk; i < fBehaviors.GetCount(); i++)
		fBehaviors[i]->SetStrength(0, rate);
}

bool plAvBrainHuman::IsMovementZeroBlend()
{
	int i;
	for (i = 0; i < fBehaviors.GetCount(); i++)
	{
		if (i == kIdle || i == kFall)
			continue;

		if (fBehaviors[i]->GetStrength() > 0)
			return false;
	}
	return true;
}

void plAvBrainHuman::TurnToPoint(hsPoint3 point)
{
	if (!fCallbackAction->IsOnGround() || IsRunningTask() || fAvMod->GetCurrentBrain() != this || !IsMovementZeroBlend())
		return;

	hsPoint3 avPos;
	fAvMod->GetTarget(0)->GetCoordinateInterface()->GetLocalToWorld().GetTranslate(&avPos);
	const plCoordinateInterface* subworldCI = nil;
	if (fAvMod->GetController())
		subworldCI = fAvMod->GetController()->GetSubworldCI();
	if (subworldCI)
	{
		point = subworldCI->GetWorldToLocal() * point;
		avPos = subworldCI->GetWorldToLocal() * avPos;
	}
	
	plAvSeekMsg *msg = TRACKED_NEW plAvSeekMsg(nil, fAvMod->GetKey(), nil, 1.f, true);
	hsClearBits(msg->fFlags, plAvSeekMsg::kSeekFlagForce3rdPersonOnStart);
	hsSetBits(msg->fFlags, plAvSeekMsg::kSeekFlagNoWarpOnTimeout | plAvSeekMsg::kSeekFlagRotationOnly);
	msg->fTargetLookAt = point;
	msg->fTargetPos = avPos;
	msg->SetBCastFlag(plMessage::kNetPropagate);
	msg->Send();
}

void plAvBrainHuman::IChatOn()
{
	char *chatAnimName = fAvMod->MakeAnimationName("Talk");

	// check that we aren't adding this twice...
	if (!fAvMod->FindAnimInstance(chatAnimName))
	{
		plKey avKey = fAvMod->GetKey();
		plAvAnimTask *animTask = TRACKED_NEW plAvAnimTask(chatAnimName, 0.0, 1.0, 1.0, 0.0, true, true, true);
		if (animTask)
		{
			plAvTaskMsg *taskMsg = TRACKED_NEW plAvTaskMsg(avKey, avKey, animTask);
			taskMsg->SetBCastFlag(plMessage::kNetPropagate);
			taskMsg->Send();
		}
	}

	delete [] chatAnimName;
}

void plAvBrainHuman::IChatOff()
{
	char *chatAnimName = fAvMod->MakeAnimationName("Talk");
	plKey avKey = fAvMod->GetKey();
	plAvAnimTask *animTask = TRACKED_NEW plAvAnimTask(chatAnimName, -1.0);
	if (animTask)
	{
		plAvTaskMsg *taskMsg = TRACKED_NEW plAvTaskMsg(avKey, avKey, animTask);
		taskMsg->SetBCastFlag(plMessage::kNetPropagate);
		taskMsg->Send();
	}
	delete[] chatAnimName;
}

hsBool plAvBrainHuman::IInitAnimations()
{
	hsBool result = false;

	plAGAnim *idle = fAvMod->FindCustomAnim("Idle");
	plAGAnim *walk = fAvMod->FindCustomAnim("Walk");
	plAGAnim *run = fAvMod->FindCustomAnim("Run");
	plAGAnim *walkBack = fAvMod->FindCustomAnim("WalkBack");
	plAGAnim *stepLeft = fAvMod->FindCustomAnim("StepLeft");
	plAGAnim *stepRight = fAvMod->FindCustomAnim("StepRight");
	plAGAnim *standingLeft = fAvMod->FindCustomAnim("TurnLeft");
	plAGAnim *standingRight = fAvMod->FindCustomAnim("TurnRight");
	plAGAnim *fall = fAvMod->FindCustomAnim("Fall");
	plAGAnim *standJump = fAvMod->FindCustomAnim("StandingJump");
	plAGAnim *walkJump = fAvMod->FindCustomAnim("WalkingJump");
	plAGAnim *runJump = fAvMod->FindCustomAnim("RunningJump");
	plAGAnim *groundImpact = fAvMod->FindCustomAnim("GroundImpact");
	plAGAnim *runningImpact = fAvMod->FindCustomAnim("RunningImpact");
	plAGAnim *movingLeft = nil; // fAvMod->FindCustomAnim("LeanLeft");
	plAGAnim *movingRight = nil; // fAvMod->FindCustomAnim("LeanRight");
	plAGAnim *pushWalk = fAvMod->FindCustomAnim("BallPushWalk");

	//plAGAnim *pushIdle = fAvMod->FindCustomAnim("BallPushIdle");
	
	const float kDefaultFade = 3.0;		// most animations fade in and out in 1/4 of a second.

	if (idle && walk && run && walkBack && standingLeft && standingRight && stepLeft && stepRight)
	{
		plHBehavior *behavior;
		fBehaviors.SetCountAndZero(kHuBehaviorMax);
		fBehaviors[kIdle] = behavior = TRACKED_NEW Idle;
		behavior->Init(idle, true, this, fAvMod, kDefaultFade, kDefaultFade, kIdle, plHBehavior::kBehaviorTypeIdle);
		behavior->SetStrength(1.f, 0.f);
		
		fBehaviors[kWalk] = behavior = TRACKED_NEW Walk;
		behavior->Init(walk, true, this, fAvMod, kDefaultFade, 5.f, kWalk, plHBehavior::kBehaviorTypeWalk);
		
		fBehaviors[kRun] = behavior = TRACKED_NEW Run;
		behavior->Init(run, true, this, fAvMod, kDefaultFade, 2.0, kRun, plHBehavior::kBehaviorTypeRun);

		fBehaviors[kWalkBack] = behavior = TRACKED_NEW WalkBack;
		behavior->Init(walkBack, true, this, fAvMod, kDefaultFade, kDefaultFade, kWalkBack, plHBehavior::kBehaviorTypeWalkBack);

		fBehaviors[kStandingTurnLeft] = behavior = TRACKED_NEW StandingTurnLeft;
		behavior->Init(standingLeft, true, this, fAvMod, 3.0f, 6.0f, kStandingTurnLeft, plHBehavior::kBehaviorTypeTurnLeft);

		fBehaviors[kStandingTurnRight] = behavior = TRACKED_NEW StandingTurnRight;
		behavior->Init(standingRight, true, this, fAvMod, 3.0f, 6.0f, kStandingTurnRight, plHBehavior::kBehaviorTypeTurnRight);

		fBehaviors[kStepLeft] = behavior = TRACKED_NEW StepLeft;
		behavior->Init(stepLeft, true, this, fAvMod, kDefaultFade, kDefaultFade, kStepLeft, plHBehavior::kBehaviorTypeSidestepLeft);

		fBehaviors[kStepRight] = behavior = TRACKED_NEW StepRight;
		behavior->Init(stepRight, true, this, fAvMod, kDefaultFade, kDefaultFade, kStepRight, plHBehavior::kBehaviorTypeSidestepRight);

		// Warning: Changing the blend times of the jump animations will affect the path you take, because until we're fully blended,
		// we won't be using the full motion defined in the animation. This isn't an issue for standing jump, but you need to be
		// aware of it for the walk/run jumps.
		fBehaviors[kFall] = behavior = TRACKED_NEW Fall;
		behavior->Init(fall, true, this, fAvMod, 1.0f, 10, kFall, plHBehavior::kBehaviorTypeFall);

		fBehaviors[kStandingJump] = behavior = TRACKED_NEW StandingJump;
		behavior->Init(standJump, false, this, fAvMod, kDefaultFade, kDefaultFade, kStandingJump, plHBehavior::kBehaviorTypeStandingJump);

		fBehaviors[kWalkingJump] = behavior = TRACKED_NEW WalkingJump;
		behavior->Init(walkJump, false, this, fAvMod, 10, 3.0, kWalkingJump, plHBehavior::kBehaviorTypeWalkingJump);

		fBehaviors[kRunningJump] = behavior = TRACKED_NEW RunningJump;
		behavior->Init(runJump, false, this, fAvMod, 10, 2.0, kRunningJump, plHBehavior::kBehaviorTypeRunningJump);
		
		fBehaviors[kGroundImpact] = behavior = TRACKED_NEW GroundImpact;
		behavior->Init(groundImpact, false, this, fAvMod, 6.0f, kDefaultFade, kGroundImpact, plHBehavior::kBehaviorTypeGroundImpact);
		
		fBehaviors[kRunningImpact] = behavior = TRACKED_NEW RunningImpact;
		behavior->Init(runningImpact, false, this, fAvMod, 6.0f, kDefaultFade, kRunningImpact, plHBehavior::kBehaviorTypeRunningImpact);
		
		fBehaviors[kMovingTurnLeft] = behavior = TRACKED_NEW MovingTurnLeft;
		behavior->Init(movingLeft, true, this, fAvMod, kDefaultFade, kDefaultFade, kMovingTurnLeft, plHBehavior::kBehaviorTypeMovingTurnLeft);

		fBehaviors[kMovingTurnRight] = behavior = TRACKED_NEW MovingTurnRight;
		behavior->Init(movingRight, true, this, fAvMod, kDefaultFade, kDefaultFade, kMovingTurnRight, plHBehavior::kBehaviorTypeMovingTurnRight);

		fBehaviors[kPushWalk] = behavior = TRACKED_NEW PushWalk;
		behavior->Init(pushWalk, true, this, fAvMod, kDefaultFade, kDefaultFade, kPushWalk, plHBehavior::kBehaviorTypePushWalk);
		
		//fBehaviors[kPushIdle] = behavior = TRACKED_NEW PushIdle;
		//behavior->Init(pushIdle, true, this, fAvMod, kDefaultFade, kDefaultFade, kPushIdle, plHBehavior::kBehaviorTypePushIdle);

		result = true;
	}

	return result;
}

hsBool plAvBrainHuman::RunStandardBehaviors(double timeNow, float elapsed)
{
	int i;
	for (i = 0; i < fBehaviors.GetCount(); i++)
	{
		plHBehavior *behavior = (plHBehavior*)fBehaviors[i];
		if (behavior->PreCondition(timeNow, elapsed))
		{
			behavior->SetStrength(1.f, behavior->fFadeIn);
			behavior->Process(timeNow, elapsed);
			fPreconditions |= behavior->GetType();
		}
		else
		{
			behavior->SetStrength(0.f, behavior->fFadeOut);
			fPreconditions &= ~behavior->GetType();
		}
	}
	return true;
}

void plAvBrainHuman::SetStartedTurning(double when)
{
	fStartedTurning = when;
}

void plAvBrainHuman::Spawn(double timeNow)
{
	plArmatureBrain::Spawn(timeNow);
	IdleOnly(true);		// reset any behavior state we may have accumulated while waiting to spawn

	// IdleOnly will set the blends of all anims to zero, and setting the blends will tell the AGModifier
	// that it needs to compile. Trouble is, the modifier only checks once per frame. MoveViaAnimation
	// works on the physics timestep, and will get called before compilation happens. It will go straight
	// to the old compiled channel, ignore the blends and still move via any anim that was playing before
	// we linked (only for the first frame).
	// 
	// Trouble is, that first frame is usually a large physics step which we don't fully resolve. This means
	// we could miss our spawn point, or worse, spawn into collision geometry and fall through.
	//
	// So we force the modifier to recompile.
	if (fAvMod)
		fAvMod->Compile(timeNow);
}

hsBool plAvBrainHuman::LeaveAge()
{
	plPhysicalControllerCore* controller = fAvMod->GetController();
	if(!controller->BehavingLikeAnAnimatedPhysical())
	{
		controller->BehaveLikeAnimatedPhysical(true);
		delete fCallbackAction;
		plSceneObject* avObj = fArmature->GetTarget(0);
		plAGModifier* agMod = const_cast<plAGModifier*>(plAGModifier::ConvertNoRef(FindModifierByClass(avObj, plAGModifier::Index())));
		fCallbackAction= TRACKED_NEW plWalkingController(avObj, agMod->GetApplicator(kAGPinTransform), controller);
		fCallbackAction->ActivateController();
	}
	plArmatureBrain::LeaveAge();
	

	
	// pin the physical so it doesn't fall when the world is deleted
	fAvMod->EnablePhysics(false);
	// this will get set to true when we hit ground
	fCallbackAction->Reset(true);
	return false;
}

void plAvBrainHuman::DumpToDebugDisplay(int &x, int &y, int lineHeight, char *strBuf, plDebugText &debugTxt)
{
	sprintf(strBuf, "Brain type: Human");
	debugTxt.DrawString(x, y, strBuf);
	y += lineHeight;
	
 	const char *grounded = fCallbackAction->IsOnGround() ? "yes" : "no";
	const char *falseGrounded = fCallbackAction->IsOnFalseGround() ? "yes" : "no";
 	const char *pushing = (fCallbackAction->GetPushingPhysical() ? (fCallbackAction->GetFacingPushingPhysical() ? "facing" : "behind") : "none");
	sprintf(strBuf, "Ground: %3s, FalseGround: %3s, AirTime: %5.2f (Peak: %5.2f), PushingPhys: %6s",
 			grounded, falseGrounded, fCallbackAction->GetAirTime(), fCallbackAction->GetImpactTime(), pushing);
 	debugTxt.DrawString(x, y, strBuf);
 	y += lineHeight;

	int i;
	//strBuf[0] = '\0';
	//for (i = 0; i < 32; i++)
	//	strcat(strBuf, fPreconditions & (0x1 << i) ? "1" : "0");
	//debugTxt.DrawString(x, y, strBuf);
	//y += lineHeight;	

	for (i = 0; i < fBehaviors.GetCount(); i++)
		fBehaviors[i]->DumpDebug(x, y, lineHeight, strBuf, debugTxt);

	debugTxt.DrawString(x, y, "Tasks:");
	y += lineHeight;

	if(fCurTask)
	{
		debugTxt.DrawString(x, y, "Current task:");
		y += lineHeight;

		int indentedX = x + 4;
		fCurTask->DumpDebug("-", indentedX, y, lineHeight, strBuf, debugTxt);
	}
	int tasks = fTaskQueue.size();
	if(tasks > 0)
	{
		debugTxt.DrawString(x, y, "Tasks in the Queue:");
		y += lineHeight;

		int indentedX = x + 4;

		for (int i = 0; i < tasks; i++)
		{
			plAvTask *each = fTaskQueue[i];
			each->DumpDebug("-", indentedX, y, lineHeight, strBuf, debugTxt);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
// BEHAVIOR

plHBehavior::plHBehavior() : 
	fAvMod(nil),
	fHuBrain(nil),
	fFadeIn(1.0f),
	fFadeOut(-1.0f),
	fMaxBlend(1.0f)
{
}

plHBehavior::~plHBehavior()
{
}

void plHBehavior::Init(plAGAnim *anim, hsBool loop, plAvBrainHuman *brain,
					   plArmatureMod *body, float fadeIn, float fadeOut,
					   UInt8 index, UInt32 type /* = 0 */)
{
	plArmatureBehavior::Init(anim, loop, brain, body, index);
	fAvMod = body;
	fHuBrain = brain;
	fFadeIn = fadeIn;
	fFadeOut = fadeOut;
	fType = type;
	
	fStartMsgSent = false; // start message hasn't been sent yet
	fStopMsgSent = false; // stop message hasn't been sent yet
}

void plHBehavior::IStart()
{
	plArmatureBehavior::IStart();
	fAvMod->SynchIfLocal(hsTimer::GetSysSeconds(), false);

	if (!fStartMsgSent)
		fAvMod->IFireBehaviorNotify(fType);

	fStartMsgSent = true; // we just sent a start message
	fStopMsgSent = false; // we haven't sent a stop message yet
}

void plHBehavior::IStop()
{
	plArmatureBehavior::IStop();
	fAvMod->SynchIfLocal(hsTimer::GetSysSeconds(), false);

	if (!fStopMsgSent)
		fAvMod->IFireBehaviorNotify(fType, false);

	fStartMsgSent = false; // we haven't sent a start message yet
	fStopMsgSent = true; // we just sent a stop message
}

static plRandom sRandom;
void Idle::IStart()
{
	plHBehavior::IStart();
	if (fAnim)
	{		
		hsScalar newStart = sRandom.RandZeroToOne() * fAnim->GetAnimation()->GetLength();
		fAnim->SetCurrentTime(newStart, true);
	}
}

hsBool Run::PreCondition(double time, float elapsed)
{
	if (fAnim)
	{
		if (fAvMod->ForwardKeyDown() && fAvMod->FastKeyDown() && fHuBrain->fCallbackAction->IsOnGround() &&
			(!fHuBrain->fCallbackAction->GetPushingPhysical() || !fHuBrain->fCallbackAction->GetFacingPushingPhysical()))
			return true;	
	}
	return false;
}

hsBool Walk::PreCondition(double time, float elapsed)
{
	if (fAnim)
	{
		if (fAvMod->ForwardKeyDown() && !fAvMod->FastKeyDown() && fHuBrain->fCallbackAction->IsOnGround() &&
			(!fHuBrain->fCallbackAction->GetPushingPhysical() || !fHuBrain->fCallbackAction->GetFacingPushingPhysical()))
			return true;
	}
	return false;
}

hsBool WalkBack::PreCondition(double time, float elapsed)
{
	if (fAnim)
	{
		if (fAvMod->BackwardKeyDown() && !fAvMod->ForwardKeyDown() && fHuBrain->fCallbackAction->IsOnGround() &&
			(!fHuBrain->fCallbackAction->GetPushingPhysical() || fHuBrain->fCallbackAction->GetFacingPushingPhysical()))
			return true;
	}
	return false;
}

hsBool StepLeft::PreCondition(double time, float elapsed)
{
	if (fAnim)
	{
		return ((fAvMod->StrafeLeftKeyDown() || (fAvMod->StrafeKeyDown() && fAvMod->TurnLeftKeyDown())) &&
			!(fAvMod->StrafeRightKeyDown() || (fAvMod->StrafeKeyDown() && fAvMod->TurnRightKeyDown())) &&
			!(fAvMod->ForwardKeyDown() || fAvMod->BackwardKeyDown()) &&
			fHuBrain->fCallbackAction->IsOnGround());
	}
	return false;
}

hsBool StepRight::PreCondition(double time, float elapsed)
{
	if (fAnim)
	{
		return ((fAvMod->StrafeRightKeyDown() || (fAvMod->StrafeKeyDown() && fAvMod->TurnRightKeyDown())) &&
				!(fAvMod->StrafeLeftKeyDown() || (fAvMod->StrafeKeyDown() && fAvMod->TurnLeftKeyDown())) &&
				!(fAvMod->ForwardKeyDown() || fAvMod->BackwardKeyDown()) &&
				fHuBrain->fCallbackAction->IsOnGround());
	}
	return false;
}

hsBool StandingTurnLeft::PreCondition(double time, float elapsed)
{
	if (fAnim)
	{
		if (fAvMod->TurnLeftKeyDown() && !fAvMod->TurnRightKeyDown() && !fAvMod->StrafeKeyDown() &&
			!fAvMod->ForwardKeyDown() && !fAvMod->BackwardKeyDown())
		{
			return true;
		}
	}
	return false;
}

hsBool StandingTurnRight::PreCondition(double time, float elapsed)
{
	if (fAnim)
	{
		if (fAvMod->TurnRightKeyDown() && !fAvMod->TurnLeftKeyDown() && !fAvMod->StrafeKeyDown() &&
			!fAvMod->ForwardKeyDown() && !fAvMod->BackwardKeyDown())
		{
			return true;
		}
	}
	return false;
}

void MovingTurn::IStart()
{
	plHBehavior::IStart();
	fHuBrain->SetStartedTurning(hsTimer::GetSysSeconds());
}

hsBool MovingTurnLeft::PreCondition(double time, float elapsed)
{
	if (fAvMod->GetTurnStrength() > 0) 
	{
		if (fHuBrain->fCallbackAction->IsOnGround() && (fAvMod->ForwardKeyDown() || fAvMod->BackwardKeyDown()) &&
			(!fHuBrain->fCallbackAction->GetPushingPhysical() || !fHuBrain->fCallbackAction->GetFacingPushingPhysical())) 
			return true;
	}
	return false;
}

hsBool MovingTurnRight::PreCondition(double time, float elapsed)
{
	if (fAvMod->GetTurnStrength() < 0) 
	{
		if (fHuBrain->fCallbackAction->IsOnGround() && (fAvMod->ForwardKeyDown() || fAvMod->BackwardKeyDown()) &&
			(!fHuBrain->fCallbackAction->GetPushingPhysical() || !fHuBrain->fCallbackAction->GetFacingPushingPhysical())) 
			return true;
	}
	
	return false;
}

void Jump::IStart()
{
	fHuBrain->fCallbackAction->EnableControlledFlight(true);
	
	plHBehavior::IStart();
}

void Jump::IStop()
{
	fHuBrain->fCallbackAction->EnableControlledFlight(false);

	plHBehavior::IStop();
}

hsBool StandingJump::PreCondition(double time, float elapsed)
{
	if (fAnim)
	{
		if (GetStrength() > 0.f)
		{
			if (!fHuBrain->fCallbackAction->IsControlledFlight() ||
				fAnim->GetTimeConvert()->WorldToAnimTimeNoUpdate(time) >= fAnim->GetTimeConvert()->GetEnd())
			{
				return false;
			}
			return !fAnim->IsFinished();
		} 
		else 
		{
			if (fAvMod->JumpKeyDown() &&
				!fAvMod->ForwardKeyDown() &&
				fAnim->GetBlend() == 0.0f &&
				fHuBrain->fCallbackAction->IsOnGround())
			{
				if (fAvMod->ConsumeJump())
					return true;
			}
		}
	}
	return false;
}

hsBool WalkingJump::PreCondition(double time, float elapsed)
{
	if (fAnim)
	{
		if (GetStrength() > 0.f)
		{
			if (!fHuBrain->fCallbackAction->IsControlledFlight() ||
				fAnim->GetTimeConvert()->WorldToAnimTimeNoUpdate(time) >= fAnim->GetTimeConvert()->GetEnd())
			{
				return false;
			}
			return !fAnim->IsFinished();
		} 
		else 
		{
			if (fAvMod->JumpKeyDown() &&
				!fAvMod->FastKeyDown() &&
				fAvMod->ForwardKeyDown() &&
				fAnim->GetBlend() == 0.0f &&
				fHuBrain->fCallbackAction->IsOnGround() &&
				(!fHuBrain->fCallbackAction->GetPushingPhysical() || !fHuBrain->fCallbackAction->GetFacingPushingPhysical()))
			{
				if (fAvMod->ConsumeJump())
					return true;
			}
		}
	}
	return false;
}

hsBool RunningJump::PreCondition(double time, float elapsed)
{
	if (fAnim)
	{
		if (GetStrength() > 0.f)
		{
			if (!fHuBrain->fCallbackAction->IsControlledFlight() ||
				fAnim->GetTimeConvert()->WorldToAnimTimeNoUpdate(time) >= fAnim->GetTimeConvert()->GetEnd())
			{
				return false;
			}
			return !fAnim->IsFinished();
		} 
		else 
		{
			if (fAvMod->JumpKeyDown() &&
				fAvMod->ForwardKeyDown() &&
				fAvMod->FastKeyDown() &&
				fAnim->GetBlend() == 0.0f &&
				fHuBrain->fCallbackAction->IsOnGround() &&
				(!fHuBrain->fCallbackAction->GetPushingPhysical() || !fHuBrain->fCallbackAction->GetFacingPushingPhysical()))
			{
				if (fAvMod->ConsumeJump())
					return true;
			}
		}
	}
	return false;
}


static const float kRunningImpactThresh = -1.0f;
static const float kFullImpactVel = 30.0f; // At this velocity (or greater) we blend the impact at full strength.
static const float kMinImpactVel = 10.f;

// If we just test IsOnGround(), we do a lot of impacts while running down stairs, so the impact
// behaviors have a more forgiving threshold.
static const float kMinAirTime = .5f;

RunningImpact::RunningImpact() : fDuration(0.0f) {}

hsBool RunningImpact::PreCondition(double time, float elapsed)
{	
	if (fDuration > 0.0f)
		fDuration = fDuration - elapsed;
	else if (fHuBrain->fCallbackAction->IsOnGround() && fHuBrain->fCallbackAction->GetImpactTime() > kMinAirTime) 
	{
		if (fHuBrain->fCallbackAction->GetImpactVelocity().fZ < -kMinImpactVel)
		{
			if (fHuBrain->fCallbackAction->GetImpactVelocity().fY < kRunningImpactThresh)
			{
				fMaxBlend = 0.5f + (0.5f * (-fHuBrain->fCallbackAction->GetImpactVelocity().fZ / (kFullImpactVel - kMinImpactVel)));
				if (fMaxBlend > 1)
					fMaxBlend = 1;
				fDuration = 1.0f / fFadeIn;
			}
		}
	}
	return(fDuration > 0.0f);
}

void RunningImpact::IStop()
{
	fDuration = 0.0f;
	plHBehavior::IStop();
}

GroundImpact::GroundImpact() : fDuration(0.0f) {}

hsBool GroundImpact::PreCondition(double time, float elapsed)
{
	
	bool result = false;
	if (fDuration > 0.0f)
		fDuration = fDuration - elapsed;
	else if (fHuBrain->fCallbackAction->IsOnGround() && fHuBrain->fCallbackAction->GetImpactTime() > kMinAirTime) 
	{
		if (fHuBrain->fCallbackAction->GetImpactVelocity().fZ < -kMinImpactVel)
		{
			if (fHuBrain->fCallbackAction->GetImpactVelocity().fY >= kRunningImpactThresh)
			{
				fMaxBlend = 0.5f + (0.5f * (-fHuBrain->fCallbackAction->GetImpactVelocity().fZ / (kFullImpactVel - kMinImpactVel)));
				if (fMaxBlend > 1)
					fMaxBlend = 1;
				fDuration = 1.0f / fFadeIn;
			}
		}
	}
	
	return(fDuration > 0.0f);
}

void GroundImpact::IStop()
{
	fDuration = 0.0f;
	plHBehavior::IStop();
}

hsBool Fall::PreCondition(double time, float elapsed)
{
	return !fHuBrain->fCallbackAction->IsOnGround() && fHuBrain->fCallbackAction->HitGroundInThisAge();
}

void Fall::Process(double time, float elapsed)
{
	// We don't see remote players panic link (from timeouts) because we don't know if they're
	// really falling, or if our understanding of their physical location is just not up-to-date.
	if (plAvatarMgr::GetInstance()->GetLocalAvatar() == fAvMod)
	{
		if (fAnim && fAnim->GetBlend() > 0.8)
		{
			float panicThresh = plAvBrainHuman::kAirTimePanicThreshold;
			if (panicThresh > 0.0f && fHuBrain->fCallbackAction->GetAirTime() > panicThresh)
			{
				fHuBrain->IdleOnly();	// clear the fall state; we're going somewhere new
				fAvMod->PanicLink();
			} 
		}
	}
}

extern float QuatAngleDiff(const hsQuat &a, const hsQuat &b);
void Push::Process(double time, float elapsed)
{
	hsQuat rot;
	hsPoint3 pos;
	fAvMod->GetPositionAndRotationSim(&pos, &rot);

	hsPoint3 lookAt;
	fHuBrain->fCallbackAction->GetPushingPhysical()->GetPositionSim(lookAt);
	hsVector3 up(0.f, 0.f, 1.f);
	hsScalar angle = hsATan2(lookAt.fY - pos.fY, lookAt.fX - pos.fX) + hsScalarPI / 2;
	hsQuat targRot(angle, &up);
	
	const hsScalar kTurnSpeed = 3.f;
	hsScalar angDiff = QuatAngleDiff(rot, targRot);
	hsScalar turnSpeed = (angDiff > elapsed * kTurnSpeed ? kTurnSpeed : angDiff / elapsed);

	hsQuat invRot = targRot.Conjugate();
	hsPoint3 globFwd = invRot.Rotate(&kAvatarForward);
	globFwd = rot.Rotate(&globFwd);
		
	if (globFwd.fX < 0)
		fHuBrain->fCallbackAction->SetTurnStrength(-turnSpeed);
	else
		fHuBrain->fCallbackAction->SetTurnStrength(turnSpeed);
}
	
//hsBool PushIdle::PreCondition(double time, float elapsed)
//{
//	return (fHuBrain->fCallbackAction->GetPushingPhysical() &&
//			fHuBrain->fCallbackAction->IsOnGround() && 
//			!fAvMod->TurnLeftKeyDown() && !fAvMod->TurnRightKeyDown()
//			&& fAvMod->GetTurnStrength() == 0);
//}

hsBool PushWalk::PreCondition(double time, float elapsed)
{
	return (fHuBrain->fCallbackAction->GetPushingPhysical() && fHuBrain->fCallbackAction->GetFacingPushingPhysical() &&
			fHuBrain->fCallbackAction->IsOnGround() &&
			fAvMod->ForwardKeyDown());
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// UTIL FUNCTIONS
//
/////////////////////////////////////////////////////////////////////////////////////////

bool PushSimpleMultiStage(plArmatureMod *avatar, const char *enterAnim, const char *idleAnim, const char *exitAnim,
						  bool netPropagate, bool autoExit, plAGAnim::BodyUsage bodyUsage, plAvBrainGeneric::BrainType type /* = kGeneric */)
{
	plAvBrainHuman *huBrain = plAvBrainHuman::ConvertNoRef(avatar->FindBrainByClass(plAvBrainHuman::Index()));
	const char *names[3] = {enterAnim, idleAnim, exitAnim};
	if (!huBrain || !huBrain->fCallbackAction->IsOnGround() || !huBrain->fCallbackAction->HitGroundInThisAge() || huBrain->IsRunningTask() ||
		!avatar->IsPhysicsEnabled() || avatar->FindMatchingGenericBrain(names, 3))
		return false;

	// XXX
	if (type == plAvBrainGeneric::kSit || type == plAvBrainGeneric::kSitOnGround)
	{
		plAvBrainSwim *swimBrain = plAvBrainSwim::ConvertNoRef(avatar->GetCurrentBrain());
		if (swimBrain && !swimBrain->IsWalking())
			return false;
	}

	// if autoExit is true, then we will immediately exit the idle loop when the user hits a move
	// key. otherwise, we'll loop until someone sends a message telling us explicitly to advance
	plAnimStage::AdvanceType idleAdvance = autoExit ? plAnimStage::kAdvanceOnMove : plAnimStage::kAdvanceNone;

	plAnimStageVec *v = TRACKED_NEW plAnimStageVec;
	plAnimStage *s1 = TRACKED_NEW plAnimStage(enterAnim,
									  0,
									  plAnimStage::kForwardAuto, plAnimStage::kBackNone,
									  plAnimStage::kAdvanceAuto, plAnimStage::kRegressNone,
									  0);
	v->push_back(s1);
	plAnimStage *s2 = TRACKED_NEW plAnimStage(idleAnim, 0,
									  plAnimStage::kForwardAuto, plAnimStage::kBackNone,
									  idleAdvance, plAnimStage::kRegressNone,
									  -1);
	v->push_back(s2);
	plAnimStage *s3 = TRACKED_NEW plAnimStage(exitAnim, 0,
									  plAnimStage::kForwardAuto, plAnimStage::kBackNone,
									  plAnimStage::kAdvanceAuto, plAnimStage::kRegressNone,
									  0);
	v->push_back(s3);

	plAvBrainGeneric *b = TRACKED_NEW plAvBrainGeneric(v, nil, nil, nil, plAvBrainGeneric::kExitAnyTask | plAvBrainGeneric::kExitNewBrain,
											   2.0f, 2.0f, plAvBrainGeneric::kMoveStandstill);

	b->SetBodyUsage(bodyUsage);
	b->SetType(type);

	plAvTaskBrain *bt = TRACKED_NEW plAvTaskBrain(b);
	plAvTaskMsg *btm = TRACKED_NEW plAvTaskMsg(plAvatarMgr::GetInstance()->GetKey(), avatar->GetKey(), bt);
	if(netPropagate)
		btm->SetBCastFlag(plMessage::kNetPropagate);
	btm->Send();

	return true;
}

bool AvatarEmote(plArmatureMod *avatar, const char *emoteName)
{
	bool result = false;
	char *fullName = avatar->MakeAnimationName(emoteName);
	plAGAnim *anim = plAGAnim::FindAnim(fullName);
	plEmoteAnim *emote = plEmoteAnim::ConvertNoRef(anim);
	hsBool alreadyActive = avatar->FindAnimInstance(fullName) != nil;
	plAvBrainHuman *huBrain = plAvBrainHuman::ConvertNoRef(avatar->FindBrainByClass(plAvBrainHuman::Index()));
	delete[] fullName;

	// XXX
	plAvBrainSwim *swimBrain = plAvBrainSwim::ConvertNoRef(avatar->GetCurrentBrain());
	if (swimBrain && swimBrain->IsSwimming())
		return false;
	
	if (huBrain && huBrain->fCallbackAction->IsOnGround() && huBrain->fCallbackAction->HitGroundInThisAge() && !huBrain->IsRunningTask() && 
		emote && !alreadyActive && avatar->IsPhysicsEnabled())
	{
		plKey avKey = avatar->GetKey();
		float fadeIn = emote->GetFadeIn();
		float fadeOut = emote->GetFadeOut();
		plAnimStage *s1 = TRACKED_NEW plAnimStage(emoteName,
										  0,
										  plAnimStage::kForwardAuto,
										  plAnimStage::kBackNone,
										  plAnimStage::kAdvanceOnMove,
										  plAnimStage::kRegressNone,
										  0);
		plAnimStageVec *v = TRACKED_NEW plAnimStageVec;
		v->push_back(s1);

		plAvBrainGeneric *b = TRACKED_NEW plAvBrainGeneric(v, nil, nil, nil, 
												   plAvBrainGeneric::kExitAnyInput | plAvBrainGeneric::kExitNewBrain | plAvBrainGeneric::kExitAnyTask, 
												   2.0f, 2.0f, huBrain->IsActor() ? plAvBrainGeneric::kMoveRelative : plAvBrainGeneric::kMoveStandstill);
		b->SetType(plAvBrainGeneric::kEmote);
		b->SetBodyUsage(emote->GetBodyUsage());
		plAvTaskBrain *bt = TRACKED_NEW plAvTaskBrain(b);
		plAvTaskMsg *btm = TRACKED_NEW plAvTaskMsg(plAvatarMgr::GetInstance()->GetKey(), avKey, bt);
		btm->SetBCastFlag(plMessage::kNetPropagate);
		btm->Send();

		result = true;
	}
	return result;
}

