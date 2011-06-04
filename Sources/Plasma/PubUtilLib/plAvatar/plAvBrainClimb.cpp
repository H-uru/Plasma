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

/*
 Some interesting special rules:
	You can only transition from one animation to another at certain points.

	The climb, mount, and dismount animations must be at their beginning or end
	to transition.

	If a climb or mount animation finishes and the key is not being held down, the idle
	animation starts automatically.

	If a climb or mount finishes and the key is being held down, the brain will *try*
	to transition to the same stage, effectively looping it.

	The idle can transition at any point.

	The Release and FallOff aniamtions can forcibly transition *any* animation.
*/

/////////////////////////////////////////////////////////////////
//
// INCLUDES
//
/////////////////////////////////////////////////////////////////

// singular
#include "plAvBrainClimb.h"

// local
#include "plAnimStage.h"
#include "plAGAnim.h"
#include "plAGAnimInstance.h"
#include "plArmatureMod.h"
#include "plMatrixChannel.h"
#include "plAvBrainHuman.h"

// global
#include "hsTimer.h"

// other
#include "../plPipeline/plDebugText.h"
#include "../plMessage/plSimStateMsg.h"
#include "../plMessage/plLOSHitMsg.h"
#include "../plMessage/plLOSRequestMsg.h"
#include "../plMessage/plClimbEventMsg.h"
#include "../pnNetCommon/plSDLTypes.h"

/////////////////////////////////////////////////////////////////
//
// IMPLEMENTATION
//
/////////////////////////////////////////////////////////////////

// CTOR default
plAvBrainClimb::plAvBrainClimb()
: fCurMode(kInactive),
  fNextMode(kInactive),
  fDesiredDirection(plClimbMsg::kUp),
  fControlDir(0.0f),

  fAllowedDirections(plClimbMsg::kUp | plClimbMsg::kDown | plClimbMsg::kLeft | plClimbMsg::kRight),
  fPhysicallyBlockedDirections(0),
  fOldPhysicallyBlockedDirections(0),
  fAllowedDismounts(0),

  fCurStage(nil),
  fExitStage(nil),
  fVerticalProbeLength(0.0f),
  fHorizontalProbeLength(0.0f),

  fUp(nil),
  fDown(nil),
  fLeft(nil),
  fRight(nil),
  fMountUp(nil),
  fMountDown(nil),
  fMountLeft(nil),
  fMountRight(nil),
  fDismountUp(nil),
  fDismountDown(nil),
  fDismountLeft(nil),
  fDismountRight(nil),
  fIdle(nil),
  fRelease(nil),
  fFallOff(nil)
{
	IInitAnimations();
}

// PLAVBRAINCLIMB
plAvBrainClimb::plAvBrainClimb(Mode nextMode)
: fCurMode(kInactive),
  fNextMode(nextMode),
  fDesiredDirection(plClimbMsg::kUp),
  fControlDir(0.0f),

  fAllowedDirections(plClimbMsg::kUp | plClimbMsg::kDown | plClimbMsg::kLeft | plClimbMsg::kRight),
  fPhysicallyBlockedDirections(0),
  fOldPhysicallyBlockedDirections(0),
  fAllowedDismounts(0),

  fCurStage(nil),
  fExitStage(nil),
  fVerticalProbeLength(0.0f),
  fHorizontalProbeLength(0.0f),

  fUp(nil),
  fDown(nil),
  fLeft(nil),
  fRight(nil),
  fMountUp(nil),
  fMountDown(nil),
  fMountLeft(nil),
  fMountRight(nil),
  fDismountUp(nil),
  fDismountDown(nil),
  fDismountLeft(nil),
  fDismountRight(nil),
  fIdle(nil),
  fRelease(nil),
  fFallOff(nil)
{
	IInitAnimations();	
}


plAvBrainClimb::~plAvBrainClimb()
{
	if(fAvMod)
	{
		if(fCurStage)
			fCurStage->Detach(fAvMod);
		if(fExitStage)
			fExitStage->Detach(fAvMod);
	}
	if(fUp) delete fUp;
	if(fDown) delete fDown;
	if(fLeft) delete fLeft;
	if(fRight) delete fRight;
	if(fMountUp) delete fMountUp;
	if(fMountDown) delete fMountDown;
	if(fMountLeft) delete fMountLeft;
	if(fMountRight) delete fMountRight;
	if(fDismountUp) delete fDismountUp;
	if(fDismountLeft) delete fDismountLeft;
	if(fDismountRight) delete fDismountRight;
	if(fIdle) delete fIdle;
//	if(fRelease) delete fRelease;
//	if(fFallOff) delete fFallOff;
}

// ACTIVATE
void plAvBrainClimb::Activate(plArmatureModBase *avMod)
{
	plArmatureBrain::Activate(avMod);
	ICalcProbeLengths();

	fAvMod->GetRootAnimator()->Enable(true);
	fAvMod->EnablePhysicsKinematic(true);
}

void plAvBrainClimb::Deactivate()
{
	fAvMod->GetRootAnimator()->Enable(false);
	fAvMod->EnablePhysicsKinematic(false);
}

// APPLY
hsBool plAvBrainClimb::Apply(double time, hsScalar elapsed)
{
	hsBool result = true;

	IGetDesiredDirection();

	float overage = 0.0f;		// if we ran past the end of the current stage, remember how much
	bool done = false;

	if(fExitStage)
		done = IProcessExitStage(time, elapsed);
	else
		done = IAdvanceCurrentStage(time, elapsed, overage);

	if(done || fCurMode == kIdle)
	{
		// if the transition is to one of the terminal modes, we're going to abort
		result = ITryStageTransition(time, overage);
	}

	if(!result && fExitStage)
	{
		fExitStage->Detach(fAvMod);
	}

	fAvMod->ApplyAnimations(time, elapsed);

	IProbeEnvironment();
	return result;
}

// MSGRECEIVE
hsBool plAvBrainClimb::MsgReceive(plMessage *msg)
{
	plClimbMsg *climbMsg;
	plLOSHitMsg *losMsg;

	if(climbMsg = plClimbMsg::ConvertNoRef(msg))
	{
		return IHandleClimbMsg(climbMsg);
	} else if(losMsg = plLOSHitMsg::ConvertNoRef(msg))
	{
		return IHandleLOSMsg(losMsg);
	} else {
		return plArmatureBrain::MsgReceive(msg);
	}
}

// IHANDLECLIMBMSG
hsBool plAvBrainClimb::IHandleClimbMsg(plClimbMsg *msg)
{
	switch(msg->fCommand)
	{
	case plClimbMsg::kEnableClimb:
		if(msg->fStatus)
			this->fAllowedDirections |= msg->fDirection;
		else
			this->fAllowedDirections &= ~msg->fDirection;
		break;
	case plClimbMsg::kEnableDismount:
		if(msg->fStatus)
			this->fAllowedDismounts |= msg->fDirection;
		else
			this->fAllowedDismounts &= ~msg->fDirection;
		break;
	case plClimbMsg::kRelease:
		IRelease(true);
		break;
	case plClimbMsg::kFallOff:
		{
			if(fCurMode != kReleasing
			&& fCurMode != kFallingOff
			&& fCurMode != kFinishing)
			{
				plClimbEventMsg* pMsg = TRACKED_NEW plClimbEventMsg;
				pMsg->SetSender(msg->fTarget);
				pMsg->SetBCastFlag(plMessage::kBCastByExactType);
				pMsg->SetBCastFlag(plMessage::kLocalPropagate);
				pMsg->SetBCastFlag(plMessage::kNetPropagate);
				pMsg->SetBCastFlag(plMessage::kNetForce);
				pMsg->Send();
			}
			IRelease(false);
			
		break;
		}
	}
	return true;
}

// IHANDLELOSMSG
hsBool plAvBrainClimb::IHandleLOSMsg(plLOSHitMsg *msg)
{
	plClimbMsg::Direction blockDir = static_cast<plClimbMsg::Direction>(msg->fRequestID);
	// this is a weak test because someone else could be using the same bits to mean something different
	// the real strategy is that we should only receive LOS messages of our own creation
	hsBool oneOfOurs = blockDir == plClimbMsg::kUp || blockDir == plClimbMsg::kDown || blockDir == plClimbMsg::kLeft || blockDir == plClimbMsg::kRight;
	if(oneOfOurs)
	{
		fPhysicallyBlockedDirections |= blockDir;
		return true;
	} else {
		return false;
	}
}

// IPROCESSEXITSTAGE
bool plAvBrainClimb::IProcessExitStage(double time, float elapsed)
{
	plAGAnimInstance *ai = fExitStage->GetAnimInstance();
	hsBool animDone = ai->IsAtEnd();
	float unused;

	// if we have an exit stage running, move it instead of the base stage
	if(!animDone)
		fExitStage->MoveRelative(time, elapsed, unused, fAvMod);	// only advance if it's not finished yet

	float curBlend = ai->GetBlend();
	
	if(curBlend > .99)		// reached peak strength
	{
		fCurStage->Detach(fAvMod);	// remove the (now completely masked) underlying anim
		fCurStage = nil;
		ai->Fade(0, 2.0f);		// start fading the exit anim
	} else if(animDone && curBlend == 0.0f) {	
		return true;		// finished and faded; we're really done now
	}
	return false;
}

// IRELEASE
void plAvBrainClimb::IRelease(bool intentional)
{
	if(fCurMode != kReleasing
		&& fCurMode != kFallingOff
		&& fCurMode != kFinishing)
	{
		if(intentional)
		{
			// fNextMode = kReleasing;
			fCurMode = kReleasing;
			fExitStage = fRelease;
		} else {
			// fNextMode = kFallingOff;
			fCurMode = kFallingOff;
			fExitStage = fFallOff;
		}
		fNextMode = kFinishing;
		double time = hsTimer::GetSysSeconds();
		// attach the exit stage atop the current stage. from here on out we'll only advance
		// the current stage. 
		fAvMod->GetRootAnimator()->Enable(false);
		fAvMod->EnablePhysicsKinematic(false);
		fExitStage->Attach(fAvMod, this, 1.0f, time);
	}
}

// IADVANCECURRENTSTAGE
bool plAvBrainClimb::IAdvanceCurrentStage(double time, float elapsed, float &overage)
{
	bool stageDone = false;
	if(fCurStage)
	{
		// elapsed tells us how far in time to move the animation
		// we must combine it with the key state to figure out whether
		// we're moving forward or backward in the animation
		fControlDir = 0.0f;	// 0 is still; -1 is backwards; 1 is forwards

		switch(fCurMode)
		{
		case kDismountingUp:			// if dismounting or mounting become reversable, move
		case kMountingUp:				// these cases to be with "kClimbingUp"; same for the rest
		case kDismountingRight:
		case kMountingRight:
		case kDismountingDown:
		case kMountingDown:
		case kDismountingLeft:
		case kMountingLeft:
		case kFallingOff:
		case kReleasing:
		case kFinishing:
		case kIdle:
		case kClimbingUp:
		case kClimbingRight:
		case kClimbingDown:
		case kClimbingLeft:
			fControlDir = 1.0f;			// these guys all auto-advance 
			break;
		case kInactive:
		case kUnknown:
			// fControlDir is already 0
			break;
		default:
			hsStatusMessage("Unknown mode in plAvBrainClimb::IAdvanceCurrentStage");
		}
		float delta = elapsed * fControlDir;
		stageDone = fCurStage->MoveRelative(time, delta, overage, fAvMod);
	} else {
		stageDone = true;
	}
	return stageDone;
}

// ITRYSTAGETRANSITION
bool plAvBrainClimb::ITryStageTransition(double time, float overage)
{
//	hsStatusMessageF("Got overage %f", overage);
	IChooseNextMode();

	bool result = true;
													// and vice versa
	if(fCurStage && fCurStage != fIdle)
	{
		hsStatusMessage("Wrapping externally.");
		bool atStart = overage >= 0.0f ? true : false;	// if we went off the end, back to start
		fCurStage->Reset(time, fAvMod, atStart);
		// any time we start a stage besides idle, clear the climbing and dismount restrictions
//		this->fAllowedDirections = plClimbMsg::kUp | plClimbMsg::kDown | plClimbMsg::kLeft | plClimbMsg::kRight;
//		this->fAllowedDismounts = 0;
	}

	if(fNextMode != fCurMode)
	{
		if(fCurStage)
			fCurStage->Detach(fAvMod);
		fCurStage = IGetStageFromMode(fNextMode);
		if(fCurStage)
		{
			hsAssert(fCurStage, "Couldn't get next stage - mode has no stage. (Matt)");
			fCurMode = fNextMode;
			if(fCurStage)
				result = (fCurStage->Attach(fAvMod, this, 1.0f, time) != nil);
			fAvMod->DirtySynchState(kSDLAvatar, 0);		// write our new stage to the server
		} else {
			result = false;
		}
	} else {
//		hsStatusMessage("Wrapping externally.");
//		bool atStart = overage >= 0.0f ? true : false;	// if we went off the end, back to start
//														// and vice versa
//		if(fCurStage)
//			fCurStage->Reset(time, fAvMod, atStart);
	}

	fNextMode = kUnknown;

	if(fCurStage)
	{
		if(overage < 0.0f)
		{
			float length = fCurStage->GetLength();
			fCurStage->SetLocalTime(length + overage);
		} else {
			fCurStage->SetLocalTime(overage);
		}
		fAvMod->GetRootAnimator()->Reset(time);
	}

	return result;
}

// ICHOOSENEXTMODE
bool plAvBrainClimb::IChooseNextMode()
{
	// bear in mind this is only called when we're at a point where
	// we can change direction (usually because a climb loop has
	// just finished)
	switch (fCurMode)
	{
	case kInactive:
	case kUnknown:
	case kFinishing:
		break;		// no change

	case kDismountingUp:
	case kDismountingDown:
	case kDismountingLeft:
	case kDismountingRight:
	case kFallingOff:
	case kReleasing:
		fNextMode = kFinishing;
		break;
	case kMountingUp:
	case kClimbingUp:
	case kMountingDown:
	case kClimbingDown:
	case kMountingLeft:
	case kClimbingLeft:
	case kMountingRight:
	case kClimbingRight:
	case kIdle:
		fNextMode = kIdle;
		if(fAllowedDismounts & fDesiredDirection)
		{
			switch(fDesiredDirection)
			{
			case plClimbMsg::kUp:
				fNextMode = kDismountingUp;
				break;
			case plClimbMsg::kDown:
				fNextMode = kDismountingDown;
				break;
			case plClimbMsg::kLeft:
				fNextMode = kDismountingLeft;
				break;
			case plClimbMsg::kRight:
				fNextMode = kDismountingRight;
				break;
			case plClimbMsg::kCenter:
				fNextMode = kIdle;
				break;
			default:
				hsAssert(false, "Error in fDesiredDirection. (Matt)");
			}
		} else if(fAllowedDirections & fDesiredDirection & ~fPhysicallyBlockedDirections)
		{
			switch(fDesiredDirection)
			{
			case plClimbMsg::kUp:
				fNextMode = kClimbingUp;
				break;
			case plClimbMsg::kDown:
				fNextMode = kClimbingDown;
				break;
			case plClimbMsg::kLeft:
				fNextMode = kClimbingLeft;
				break;
			case plClimbMsg::kRight:
				fNextMode = kClimbingRight;
				break;
			case plClimbMsg::kCenter:
				fNextMode = kIdle;
				break;
			default:
				hsAssert(false, "Error in fDesiredDirection. (Matt)");
			}
		}
		break;
	default:
		hsAssert(false, "Error in fCurMode. (Matt)");
	}
	return true;
}

// IGETSTAGE
plAnimStage * plAvBrainClimb::IGetStageFromMode(Mode mode)
{
	switch(mode)
	{
	case kClimbingUp:
		return fUp;
	case kClimbingDown:
		return fDown;
	case kClimbingLeft:
		return fLeft;
	case kClimbingRight:
		return fRight;
	case kMountingUp:
		return fMountUp;
	case kMountingDown:
		return fMountDown;
	case kMountingLeft:
		return fMountLeft;
	case kMountingRight:
		return fMountRight;
	case kDismountingUp:
		return fDismountUp;
	case kDismountingDown:
		return fDismountDown;
	case kDismountingLeft:
		return fDismountLeft;
	case kDismountingRight:
		return fDismountRight;
	case kIdle:
		return fIdle;
	case kReleasing:
		return fRelease;
	case kFallingOff:
		return fFallOff;
	case kInactive:
	case kFinishing:
	case kUnknown:
	case kDone:
		return nil;
	default:
		hsAssert(false, "Unknown mode.");
		return nil;
	}
}

plAvBrainClimb::Mode plAvBrainClimb::IGetModeFromStage(plAnimStage *stage)
{
	if(stage == fUp)
		return kClimbingUp;
	else if(stage == fDown)
		return kClimbingDown;
	else if(stage == fLeft)
		return kClimbingLeft;
	else if(stage == fRight)
		return kClimbingRight;
	else if(stage == fMountUp)
		return kMountingUp;
	else if(stage == fMountDown)
		return kMountingDown;
	else if(stage == fMountLeft)
		return kMountingLeft;
	else if(stage == fMountRight)
		return kMountingRight;
	else if(stage == fDismountUp)
		return kDismountingUp;
	else if(stage == fDismountDown)
		return kDismountingDown;
	else if(stage == fDismountLeft)
		return kDismountingLeft;
	else if(stage == fDismountRight)
		return kDismountingRight;
	else if(stage == fIdle)
		return kIdle;
	else if(stage == fRelease)
		return kReleasing;
	else if(stage == fFallOff)
		return kFallingOff;
	else
		return kUnknown;	
}

// IGETDESIREDDIRECTION
void plAvBrainClimb::IGetDesiredDirection()
{
	if(fAvMod->ForwardKeyDown()) {
		fDesiredDirection = plClimbMsg::kUp;
	} else if (fAvMod->BackwardKeyDown()) {
		fDesiredDirection = plClimbMsg::kDown;
	} else if (fAvMod->TurnLeftKeyDown()) {
		fDesiredDirection = plClimbMsg::kLeft;
	} else if (fAvMod->TurnRightKeyDown()) {
		fDesiredDirection = plClimbMsg::kRight;
	} else {
		fDesiredDirection = plClimbMsg::kCenter;
	}
}

/** Look left, right, up, and down to see which directions are clear
	for our movement. We could do this by positioning our actual collision
	body and testing for hits, but it gives a lot more false positives *and*
	we won't get the normals of intersection, so it will be more complex
	to figure out which directions are actually blocked.
	The approach here is to do a raycast in the aforementioned directions
	and fail that direction if the raycast hits anything. */
void plAvBrainClimb::IProbeEnvironment()
{
	hsMatrix44 l2w = fAvMod->GetTarget(0)->GetLocalToWorld();
	// we're just going to pull the axes out of the 
	
	hsPoint3 up = hsPoint3(l2w.GetAxis(hsMatrix44::kUp) * fVerticalProbeLength);
	hsPoint3 down = -up;
	hsPoint3 right = hsPoint3(l2w.GetAxis(hsMatrix44::kRight) * fHorizontalProbeLength);
	hsPoint3 left = -right;
	hsPoint3 start = l2w.GetTranslate();

	start.fZ += 3.0f;	// move the origin from the feet to the bellybutton
	up += start;
	down += start;
	left += start;
	right += start;

	plKey ourKey = fAvMod->GetKey();

	// *** would be cool if we could hint that these should be batched for spatial coherence optimization
	plLOSRequestMsg *upReq = TRACKED_NEW plLOSRequestMsg(ourKey, start, up, plSimDefs::kLOSDBCustom, plLOSRequestMsg::kTestAny, plLOSRequestMsg::kReportHit);
	upReq->SetRequestID(static_cast<UInt32>(plClimbMsg::kUp));
	upReq->Send();

	plLOSRequestMsg *downReq = TRACKED_NEW plLOSRequestMsg(ourKey, start, down, plSimDefs::kLOSDBCustom, plLOSRequestMsg::kTestAny, plLOSRequestMsg::kReportHit);
	downReq->SetRequestID(static_cast<UInt32>(plClimbMsg::kDown));
	downReq->Send();

	plLOSRequestMsg *leftReq = TRACKED_NEW plLOSRequestMsg(ourKey, start, left, plSimDefs::kLOSDBCustom, plLOSRequestMsg::kTestAny, plLOSRequestMsg::kReportHit);
	leftReq->SetRequestID(static_cast<UInt32>(plClimbMsg::kLeft));
	leftReq->SetRequestType(plSimDefs::kLOSDBCustom);
	leftReq->Send();

	plLOSRequestMsg *rightReq = TRACKED_NEW plLOSRequestMsg(ourKey, start, right, plSimDefs::kLOSDBCustom, plLOSRequestMsg::kTestAny, plLOSRequestMsg::kReportHit);
	rightReq->SetRequestID(static_cast<UInt32>(plClimbMsg::kRight));
	rightReq->Send();

	fOldPhysicallyBlockedDirections = fPhysicallyBlockedDirections;
	fPhysicallyBlockedDirections = 0;	// clear our blocks until the new reports come in....
}

// ICalcProbeLengths -------------------
// -----------------
void plAvBrainClimb::ICalcProbeLengths()
{
	// we assume that the up and down climbs go the same distance;
	// same for the left and right climbs
	plAGAnim *up = fAvMod->FindCustomAnim("ClimbUp");
	plAGAnim *left = fAvMod->FindCustomAnim("ClimbLeft");

	hsMatrix44 upMove, leftMove;

	hsAssert(up, "Couldn't find ClimbUp animation.");
	if(up)
	{
		GetStartToEndTransform(up, &upMove, nil, "Handle");
		fVerticalProbeLength = upMove.GetTranslate().fZ;
	} else
		fVerticalProbeLength = 4.0f;	// guess

	hsAssert(left, "Couldn't find ClimbLeft animation.");
	if(left)
	{
		GetStartToEndTransform(left, &leftMove, nil, "Handle");
		fHorizontalProbeLength = leftMove.GetTranslate().fX;
	} else
		fHorizontalProbeLength = 3.0f;	// guess
}

// IInitAnimations ---------------------
// ---------------
hsBool plAvBrainClimb::IInitAnimations()
{
	fUp = TRACKED_NEW plAnimStage("WallClimbUp",
						  plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressAuto,
						  0);
	fDown = TRACKED_NEW plAnimStage("WallClimbDown",
						  plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressAuto,
						  0);
	fLeft = TRACKED_NEW plAnimStage("WallClimbLeft",
						  plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressAuto,
						  0);
	fRight = TRACKED_NEW plAnimStage("WallClimbRight",
						  plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressAuto,
						  0);
	// the mounts
	fMountUp = TRACKED_NEW plAnimStage("WallClimbMountUp",
						  plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressNone,
						  0);
	fMountDown = TRACKED_NEW plAnimStage("WallClimbMountDown",
						  plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressNone,
						  0);
	fMountLeft = TRACKED_NEW plAnimStage("WallClimbMountLeft",
						  plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressNone,
						  0);
	fMountRight = TRACKED_NEW plAnimStage("WallClimbMountRight",
						  plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressNone,
						  0);
	// and here's the dismount
	fDismountUp = TRACKED_NEW plAnimStage("WallClimbDismountUp",
						  plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressNone,
						  0);
	fDismountDown = TRACKED_NEW plAnimStage("WallClimbDismountDown",
						  plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressNone,
						  0);
	fDismountLeft = TRACKED_NEW plAnimStage("WallClimbDismountLeft",
						  plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressNone,
						  0);
	fDismountRight = TRACKED_NEW plAnimStage("WallClimbDismountUp",
						  plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressNone,
						  0);
	// other
	fIdle = TRACKED_NEW plAnimStage("WallClimbIdle",
						  plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressNone,
						  0);
	fRelease = TRACKED_NEW plAnimStage("WallClimbRelease",
						  plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressNone,
						  0);
	fFallOff = TRACKED_NEW plAnimStage("WallClimbFallOff",
						  plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressNone,
						  0);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// SDL-BASED PERSISTENCE
//
/////////////////////////////////////////////////////////////////////////////////////////
#include "../plSDL/plSDL.h"
#include "plAvatarSDLModifier.h"

// SaveToSDL -----------------------------------------
// ---------
void plAvBrainClimb::SaveToSDL(plStateDataRecord *sdl)
{
	sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrCurMode)->Set(fCurMode);
	sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrNextMode)->Set(fNextMode);
	sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrAllowedDirections)->Set((int)fAllowedDirections);
	sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrAllowedDismounts)->Set((int)fAllowedDismounts);
	sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrVertProbeLength)->Set(fVerticalProbeLength);
	sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrHorizProbeLength)->Set(fHorizontalProbeLength);

	bool curStageAttached = fCurStage && fCurStage->GetIsAttached();
	sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrCurStageAttached)->Set(curStageAttached);
	if(curStageAttached)
	{
		// slightly abuse the "mode" semantics; it happens to work as a persistance format
		Mode curStageAsMode = IGetModeFromStage(fCurStage);
		sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrCurStage)->Set(curStageAsMode);

		float curStageTime = fCurStage->GetLocalTime();
		sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrCurStageTime)->Set(curStageTime);

		float curStageBlend = fCurStage->GetAnimInstance()->GetBlend();
		sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrCurStageStrength)->Set(curStageBlend);
	}
	bool exitStageAttached = fExitStage && fExitStage->GetIsAttached();
	sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrExitStageAttached)->Set(exitStageAttached);

	if(exitStageAttached)
	{
		Mode exitStageAsMode = IGetModeFromStage(fExitStage);
		sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrExitStage)->Set(exitStageAsMode);
		
		float exitStageTime = fExitStage->GetLocalTime();
		sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrExitStageTime)->Set(exitStageTime);

		float exitStageBlend = fExitStage->GetAnimInstance()->GetBlend();
		sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrExitStageStrength)->Set(exitStageBlend);
	}
}

// LoadFromSDL -----------------------------------------
// -----------
void plAvBrainClimb::LoadFromSDL(const plStateDataRecord *sdl)
{
	double curTime = hsTimer::GetSysSeconds();

	sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrCurStage)->Get((int*)&fCurMode);
	sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrNextMode)->Get((int*)&fNextMode);
	sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrAllowedDirections)->Get((int*)&fAllowedDirections);
	sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrAllowedDismounts)->Get((int*)&fAllowedDismounts);
	sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrVertProbeLength)->Get(&fVerticalProbeLength);
	sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrHorizProbeLength)->Get(&fHorizontalProbeLength);

	bool curStageAttached = false;
	sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrCurStageAttached)->Get(&curStageAttached);
	if(curStageAttached)
	{
		Mode *curStageMode;		// distinct from curMode; this is just a mode-based representation of the current stage
		sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrCurStage)->Get((int*)&curStageMode);
		plAnimStage *curStage = this->IGetStageFromMode(fCurMode);
		curStage->Attach(fAvMod, this, 0.0f, curTime);

		float curStageTime = 0.0f;
		sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrCurStageTime)->Get(&curStageTime);
		curStage->ResetAtTime(curTime, curStageTime, fAvMod);	// restart the relative-position sampler

		float curStageBlend = 0.0f;
		sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrCurStageStrength)->Get(&curStageBlend);
		curStage->GetAnimInstance()->SetBlend(curStageBlend);
	}

	bool exitStageAttached;
	sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrExitStageAttached)->Get(&exitStageAttached);
	if(exitStageAttached)
	{
		Mode exitStageMode;	// the exit stage, in mode form
		sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrExitStage)->Get((int *)&exitStageMode);	
		plAnimStage *exitStage = this->IGetStageFromMode(exitStageMode);
		sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrExitStageTime)->Get((int *)&fNextMode);
		sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrExitStageStrength)->Get((int *)&fNextMode);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// READABLE TEXT FOR DEBUGGING
//
/////////////////////////////////////////////////////////////////////////////////////////

// DumpToDebugDisplay --------------------------------------------------------------------------------------
// ------------------
void plAvBrainClimb::DumpToDebugDisplay(int &x, int &y, int lineHeight, char *strBuf, plDebugText &debugTxt)
{
	debugTxt.DrawString(x, y, "Brain type: Climb");
	y += lineHeight;
	const char * worldDir = WorldDirStr(fDesiredDirection);
	const char * modeStr = ModeStr(fCurMode);

	char buffy[256];
	sprintf(buffy, "direction: %s mode: %s controlDir: %f", worldDir, modeStr, fControlDir);
	debugTxt.DrawString(x,y, buffy);
	y += lineHeight;
	
	IDumpClimbDirections(x, y, lineHeight, strBuf, debugTxt);
	IDumpDismountDirections(x, y, lineHeight, strBuf, debugTxt);
	IDumpBlockedDirections(x, y, lineHeight, strBuf, debugTxt);

	fUp->DumpDebug(fUp == fCurStage, x, y, lineHeight, strBuf, debugTxt);
	fDown->DumpDebug(fDown == fCurStage, x, y, lineHeight, strBuf, debugTxt);
	fLeft->DumpDebug(fLeft == fCurStage, x, y, lineHeight, strBuf, debugTxt);
	fRight->DumpDebug(fRight == fCurStage, x, y, lineHeight, strBuf, debugTxt);
	fMountUp->DumpDebug(fMountUp == fCurStage, x, y, lineHeight, strBuf, debugTxt);
	fMountDown->DumpDebug(fMountDown == fCurStage, x, y, lineHeight, strBuf, debugTxt);
	fMountLeft->DumpDebug(fMountLeft == fCurStage, x, y, lineHeight, strBuf, debugTxt);
	fMountRight->DumpDebug(fMountRight == fCurStage, x, y, lineHeight, strBuf, debugTxt);
	fDismountUp->DumpDebug(fDismountUp == fCurStage, x, y, lineHeight, strBuf, debugTxt);
	fDismountDown->DumpDebug(fDismountDown == fCurStage, x, y, lineHeight, strBuf, debugTxt);
	fDismountLeft->DumpDebug(fDismountLeft == fCurStage, x, y, lineHeight, strBuf, debugTxt);
	fDismountRight->DumpDebug(fDismountRight == fCurStage, x, y, lineHeight, strBuf, debugTxt);
	fIdle->DumpDebug(fIdle == fCurStage, x, y, lineHeight, strBuf, debugTxt);
	fRelease->DumpDebug(fRelease == fCurStage, x, y, lineHeight, strBuf, debugTxt);
	fFallOff->DumpDebug(fFallOff == fCurStage, x, y, lineHeight, strBuf, debugTxt);

}

// IDumpClimbDirections --------------------------------------------------------------------------------------
// --------------------
void plAvBrainClimb::IDumpClimbDirections(int &x, int &y, int lineHeight, char *strBuf, plDebugText &debugTxt)
{
	const char * prolog = "Allowed directions: ";
	std::string str;

	str = prolog;
	if(fAllowedDirections & plClimbMsg::kUp)
		str = str + "UP ";
	if(fAllowedDirections & plClimbMsg::kDown)
		str = str + "DOWN ";
	if(fAllowedDirections & plClimbMsg::kLeft)
		str = str + "LEFT ";
	if(fAllowedDirections & plClimbMsg::kRight)
		str = str + "RIGHT ";
	
	if(str.size() == strlen(prolog))
		str = str + "- NONE -";

	debugTxt.DrawString(x, y, str.c_str());
	y += lineHeight;
}

// IDumpDismountDirections --------------------------------------------------------------------------------------
// -----------------------
void plAvBrainClimb::IDumpDismountDirections(int &x, int &y, int lineHeight, char *strBuf, plDebugText &debugTxt)
{
	const char * prolog = "Enabled dismounts: ";
	std::string str;

	str = prolog;
	if(fAllowedDismounts & plClimbMsg::kUp)
		str = str + "UP ";
	if(fAllowedDismounts & plClimbMsg::kDown)
		str = str + "DOWN ";
	if(fAllowedDismounts & plClimbMsg::kLeft)
		str = str + "LEFT ";
	if(fAllowedDismounts & plClimbMsg::kRight)
		str = str + "RIGHT ";
	
	if(str.size() == strlen(prolog))
		str = str + "- NONE -";

	debugTxt.DrawString(x, y, str.c_str());
	y += lineHeight;
}

void plAvBrainClimb::IDumpBlockedDirections(int &x, int &y, int lineHeight, char *strBuf, plDebugText &debugTxt)
{
	const char * prolog = "Physically blocked: ";
	std::string str;

	str = prolog;
	if(fOldPhysicallyBlockedDirections & plClimbMsg::kUp)
		str = str + "UP ";
	if(fOldPhysicallyBlockedDirections & plClimbMsg::kDown)
		str = str + "DOWN ";
	if(fOldPhysicallyBlockedDirections & plClimbMsg::kLeft)
		str = str + "LEFT ";
	if(fOldPhysicallyBlockedDirections & plClimbMsg::kRight)
		str = str + "RIGHT ";
	
	if(str.size() == strlen(prolog))
		str = str + "- NONE -";

	debugTxt.DrawString(x, y, str.c_str());
	y += lineHeight;
}

const char * plAvBrainClimb::WorldDirStr(plClimbMsg::Direction dir)
{
	switch(dir)
	{
	case plClimbMsg::kUp:
		return "Up";
	case plClimbMsg::kDown:
		return "Down";
	case plClimbMsg::kLeft:
		return "Left";
	case plClimbMsg::kRight:
		return "Right";
	case plClimbMsg::kCenter:
		return "Center";
	default:
		return "WTF?";
	}
}

const char *plAvBrainClimb::ModeStr(Mode mode)
{
	switch(mode)
	{
	case kInactive:
		return "Inactive";
	case kUnknown:
		return "Unknown";
	case kFinishing:
		return "Finishing";
	case kDone:
		return "Done";
	case kClimbingUp:
		return "ClimbingUp";
	case kClimbingDown:
		return "ClimbingDown";
	case kClimbingLeft:
		return "ClimbingLeft";
	case kClimbingRight:
		return "ClimbingRight";
	case kMountingUp:
		return "MountingUp";
	case kMountingDown:
		return "MountingDown";
	case kMountingLeft:
		return "MountingLeft";
	case kMountingRight:
		return "MountingRight";
	case kDismountingUp:
		return "MountingUp";
	case kDismountingDown:
		return "DismountingDown";
	case kDismountingLeft:
		return "DismountingLeft";
	case kDismountingRight:
		return "DismountingRight";
	case kIdle:
		return "Idle";
	case kReleasing:
		return "Releasing";
	case kFallingOff:
		return "FallingOff";
	default:
		return "WTF???!!!";
	}
}