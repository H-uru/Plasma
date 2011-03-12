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

/////////////////////////////////////////////////////////////////////////////////////////
//
// INCLUDES
//
/////////////////////////////////////////////////////////////////////////////////////////

// singular
#include "plAvTaskSeek.h"

// local
#include "plAvBrainHuman.h"
#include "plAGAnim.h"
#include "plArmatureMod.h"
#include "plAvatarMgr.h"
#include "plAvCallbackAction.h"

// other
#include "../plMessage/plAvatarMsg.h"
#include "../pnMessage/plCameraMsg.h"
#include "../pnInputCore/plControlEventCodes.h"
#include "../plPipeline/plDebugText.h"
#include "../plStatusLog/plStatusLog.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "hsTimer.h"
#include "plgDispatch.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
// PROTOTYPES
//
/////////////////////////////////////////////////////////////////////////////////////////
float QuatAngleDiff(const hsQuat &a, const hsQuat &b);
void MakeMatrixUpright(hsMatrix44 &mat);

/////////////////////////////////////////////////////////////////////////////////////////
//
// DEFINES
//
/////////////////////////////////////////////////////////////////////////////////////////

#define	kSeekTimeout 5.0f
#define kRotSpeed 1.0f		// normal rotation speed is 1.0 radians per second
#define kFloatSpeed 3.0f
#define kMaxRadiansPerSecond 1.5

#define kDefaultShuffleRange 0.5f
#define kDefaultMaxSidleRange 4.0f
#define kDefaultMaxSidleAngle 0.2f

hsBool plAvTaskSeek::fLogProcess = false;

/////////////////////////////////////////////////////////////////////////////////////////
//
// IMPLEMENTATION
//
/////////////////////////////////////////////////////////////////////////////////////////

void plAvTaskSeek::IInitDefaults()
{
	fSeekObject = nil;
	fMovingTarget = false;
	fAlign = kAlignHandle;
	fAnimName = nil;
	fPosGoalHit = false;
	fRotGoalHit = false;
	fStillPositioning = true;
	fStillRotating = true;
	fShuffleRange = kDefaultShuffleRange;
	fMaxSidleRange = kDefaultMaxSidleRange;
	fMaxSidleAngle = kDefaultMaxSidleAngle;
	fFlags = kSeekFlagForce3rdPersonOnStart;  
	fState = kSeekRunNormal;
	fNotifyFinishedKey = nil;
}
// plAvTaskSeek ------------
// -------------
plAvTaskSeek::plAvTaskSeek() {}

plAvTaskSeek::plAvTaskSeek(plAvSeekMsg *msg)
{
	IInitDefaults();

	fAlign = msg->fAlignType;
	fAnimName = msg->fAnimName;

	plKey &target = msg->fSeekPoint;
	if (target)
		SetTarget(target);
	else
		SetTarget(msg->fTargetPos, msg->fTargetLookAt);
	
	if (msg->UnForce3rdPersonOnFinish())
		fFlags |= kSeekFlagUnForce3rdPersonOnFinish;
	else
		fFlags &= ~kSeekFlagUnForce3rdPersonOnFinish;

	if (msg->Force3rdPersonOnStart())
		fFlags |= kSeekFlagForce3rdPersonOnStart;
	else
		fFlags &= ~kSeekFlagForce3rdPersonOnStart;

	if (msg->NoWarpOnTimeout())
		fFlags |= kSeekFlagNoWarpOnTimeout;
	else
		fFlags &= ~kSeekFlagNoWarpOnTimeout;

	if (msg->RotationOnly())
	{
		fFlags |= kSeekFlagRotationOnly;
		fStillPositioning = false;
		fPosGoalHit = true;
	}
	else
		fFlags &= ~kSeekFlagRotationOnly;

	fNotifyFinishedKey = msg->fFinishKey;
}

// plAvTaskSeek ------------------------
// -------------
plAvTaskSeek::plAvTaskSeek(plKey target)
{
	IInitDefaults();

	SetTarget(target);
}

// plAvTaskSeek -------------------------------------------------------------------------------------------
// -------------
plAvTaskSeek::plAvTaskSeek(plKey target, plAvAlignment align, const char *animName, bool moving)
{
	IInitDefaults();

	fMovingTarget = moving;
	fAlign = align;
	fAnimName = animName;

	SetTarget(target);
}

void plAvTaskSeek::SetTarget(plKey target)
{
	hsAssert(target, "Bad key to seek task");
	if(target)
	{
		fSeekObject = plSceneObject::ConvertNoRef(target->ObjectIsLoaded());
	}
	else
	{
		fSeekObject = nil;
	}
}
	
void plAvTaskSeek::SetTarget(hsPoint3 &pos, hsPoint3 &lookAt)
{
	fSeekPos = pos;
	hsVector3 up(0.f, 0.f, 1.f);
	hsScalar angle = hsATan2(lookAt.fY - pos.fY, lookAt.fX - pos.fX) + hsScalarPI / 2;
	fSeekRot.SetAngleAxis(angle, up);
}

// Start -----------------------------------------------------------------------------------------
// ------
hsBool plAvTaskSeek::Start(plArmatureMod *avatar, plArmatureBrain *brain, double time, hsScalar elapsed)
{
	plAvBrainHuman *huBrain = plAvBrainHuman::ConvertNoRef(brain);
	hsAssert(huBrain, "Seek task only works on human brains");

	plAvatarMgr::GetInstance()->GetLog()->AddLine("Starting SMART SEEK");
	//controller needs to know we are seeking. prevents controller from interacting with exclusion regions
	
	if (avatar->GetController() )
		avatar->GetController()->SetSeek(true);
	fStartTime = time;
	if(huBrain)
	{
		avatar->SuspendInput();		// stop accepting input from the user, but queue any messages
									// ...and save our current input state.
		
		ILimitPlayersInput(avatar);
		
		if (plAvOneShotTask::fForce3rdPerson && avatar->IsLocalAvatar() && (fFlags & plAvSeekMsg::kSeekFlagForce3rdPersonOnStart))
		{
			// create message
			plCameraMsg* pMsg = TRACKED_NEW plCameraMsg;
			pMsg->SetBCastFlag(plMessage::kBCastByExactType);
			pMsg->SetBCastFlag(plMessage::kNetPropagate, false);
			pMsg->SetCmd(plCameraMsg::kResponderSetThirdPerson);
			plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
		}		

		huBrain->IdleOnly(); // Makes sure to kill jumps too. Just calling ClearInputFlags isn't enough
		IUpdateObjective(avatar);
		return true;
	}
	else
	{
		return false;
	}
}

// Process -------------------------------------------------------------------------------------------
// --------
hsBool plAvTaskSeek::Process(plArmatureMod *avatar, plArmatureBrain *brain, double time, hsScalar elapsed)
{
	if (fState == kSeekAbort)
		return false;
	
	plAvBrainHuman *uBrain = plAvBrainHuman::ConvertNoRef(brain);
	if (uBrain)
	{
		if (fMovingTarget)
		{
			IUpdateObjective(avatar);
		}
		
		IAnalyze(avatar);
		hsBool result = IMoveTowardsGoal(avatar, uBrain, time, elapsed);
		if (fLogProcess)
			DumpToAvatarLog(avatar);
		return result;
	}

	return false;
}

// Finish ---------------------------------------------------------------------------------------
// -------
void plAvTaskSeek::Finish(plArmatureMod *avatar, plArmatureBrain *brain, double time, hsScalar elapsed)
{
	plAvBrainHuman *huBrain = plAvBrainHuman::ConvertNoRef(brain);
	
	if(huBrain)
	{
		// this will process any queued input messages so if the user pressed or released a key while we were busy, we'll note it now.
		avatar->ResumeInput();	
		IUndoLimitPlayersInput(avatar);
		
		if (plAvOneShotTask::fForce3rdPerson && avatar->IsLocalAvatar() && (fFlags & plAvSeekMsg::kSeekFlagUnForce3rdPersonOnFinish))
		{
			// create message
			plCameraMsg* pMsg = TRACKED_NEW plCameraMsg;
			pMsg->SetBCastFlag(plMessage::kBCastByExactType);
			pMsg->SetBCastFlag(plMessage::kNetPropagate, false);
			pMsg->SetCmd(plCameraMsg::kResponderUndoThirdPerson);
			plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
		}
		
		avatar->SynchIfLocal(hsTimer::GetSysSeconds(), false);
	}

	if (fNotifyFinishedKey)
	{
		plAvTaskSeekDoneMsg *msg = TRACKED_NEW plAvTaskSeekDoneMsg(avatar->GetKey(), fNotifyFinishedKey);
		msg->fAborted = (fState == kSeekAbort);
		msg->Send();
	}
	plAvatarMgr::GetInstance()->GetLog()->AddLine("Finished SMART SEEK");	
	//inform controller we are done seeking
	if (avatar->GetController())
		avatar->GetController()->SetSeek(false);
}

void plAvTaskSeek::LeaveAge(plArmatureMod *avatar)
{
	fSeekObject = nil;
	fState = kSeekAbort;
}

// IAnalyze ----------------------------------------
// ---------
hsBool plAvTaskSeek::IAnalyze(plArmatureMod *avatar)
{
	avatar->GetPositionAndRotationSim(&fPosition, &fRotation);
	fGoalVec.Set(&(hsScalarTriple)(fSeekPos - fPosition));
	hsVector3 normalizedGoalVec(fGoalVec);
	normalizedGoalVec.Normalize();

	fDistance = hsSquareRoot(fGoalVec.fX * fGoalVec.fX + fGoalVec.fY * fGoalVec.fY);

	if(fDistance < 3.0f)
	{
		// we're in "near target" mode
		fMinFwdAngle = .5f;			// walk forward if target's in 90' cone straight ahead
		fMaxBackAngle = -.2f;		// walk backward if target's in a 144' cone behind
	}
	else
	{
		// we're in "far target" mode
		fMinFwdAngle = .2f;			// walk forward if target's in a 144' cone ahead
		fMaxBackAngle = -2.0;		// disable backing up if goal is far out (-1 is the minimum usable value here)
	}
	
	hsQuat invRot = fRotation.Conjugate();
	hsPoint3 globFwd = invRot.Rotate(&kAvatarForward);
	hsPoint3 globRight = invRot.Rotate(&kAvatarRight);

	hsPoint3 locGoalVec = fRotation.Rotate(&fGoalVec);

	fDistForward = -(locGoalVec.fY);
	fDistRight = -(locGoalVec.fX);

	fAngForward = globFwd.InnerProduct(normalizedGoalVec);
	fAngRight = globRight.InnerProduct(normalizedGoalVec);
	return true;
}

// IMoveTowardsGoal --------------------------------------------------------------
// -----------------
hsBool plAvTaskSeek::IMoveTowardsGoal(plArmatureMod *avatar, plAvBrainHuman *brain,
									  double time, hsScalar elapsed)
{
	bool stillRunning = true;
	avatar->ClearInputFlags(false, false);

	double duration = time - fStartTime;

	if(duration > kSeekTimeout)
	{
		if (fFlags & kSeekFlagNoWarpOnTimeout)
		{
			fState = kSeekAbort;
			return false;
		}
		fSeekRot.Normalize();
		avatar->SetPositionAndRotationSim(&fSeekPos, &fSeekRot);
		IAnalyze(avatar); // Recalcs fPosition, fDistance, etc.
		hsStatusMessage("Timing out on smart seek - jumping to target.");
		stillRunning = false;

		// We just set the pos/rot, so we know these are hit.
		fPosGoalHit = true;
		fRotGoalHit = true;
	}

	if (!(fDistance > fShuffleRange))
		fPosGoalHit = true;

	if (!fPosGoalHit)
	{
		bool right = fAngRight > 0.0f;
		bool inSidleRange = fDistance < fMaxSidleRange;
		
		bool sidling = fabs(fDistRight) > fabs(fDistForward) && inSidleRange;

		if(sidling)
		{
			if(right)
				avatar->SetStrafeRightKeyDown();
			else
				avatar->SetStrafeLeftKeyDown();
		}
		else
		{
			if(fAngForward < fMaxBackAngle)
				avatar->SetBackwardKeyDown();

			else
			{
				if(fAngForward > fMinFwdAngle)
					avatar->SetForwardKeyDown();

				if(right)
					avatar->SetTurnRightKeyDown();
				else
					avatar->SetTurnLeftKeyDown();
			}
		}
	}
	else 
	{
		if (!(QuatAngleDiff(fRotation, fSeekRot) > .1))
			fRotGoalHit = true;

		if (!fRotGoalHit)
		{
			hsQuat invRot = fSeekRot.Conjugate();
			hsPoint3 globFwd = invRot.Rotate(&kAvatarForward);
			globFwd = fRotation.Rotate(&globFwd);
		
			if (globFwd.fX < 0)
				avatar->SetTurnRightKeyDown();
			else
				avatar->SetTurnLeftKeyDown();
		}
	}		

	if (fPosGoalHit && fRotGoalHit)
		stillRunning = ITryFinish(avatar, brain, time, elapsed);

	return stillRunning;
}


// ITRYFINISH
bool plAvTaskSeek::ITryFinish(plArmatureMod *avatar, plAvBrainHuman *brain, double time, hsScalar elapsed)
{
	hsBool animsDone = brain->IsMovementZeroBlend();

	hsPoint3 newPosition = fPosition;
	hsQuat newRotation = fRotation;

	if (!(fFlags & kSeekFlagRotationOnly) && (fStillPositioning || !animsDone))
		fStillPositioning = IFinishPosition(newPosition, avatar, brain, time, elapsed);
	if (fStillRotating || !animsDone)
		fStillRotating = IFinishRotation(newRotation, avatar, brain, time, elapsed);

	newRotation.Normalize();
  	if (hsCheckBits(fFlags, kSeekFlagRotationOnly))
		avatar->SetPositionAndRotationSim(nil, &newRotation);
	else
		avatar->SetPositionAndRotationSim(&newPosition, &newRotation);

	return fStillPositioning || fStillRotating || !animsDone;
}

hsBool plAvTaskSeek::IFinishPosition(hsPoint3 &newPosition,
									 plArmatureMod *avatar, plAvBrainHuman *brain,
									 double time, hsScalar elapsed)
{
	// While warping, we might be hovering just above the ground. Don't want that to
	// trigger any falling behavior.
	if(brain&&brain->fCallbackAction)
	{
		
		brain->fCallbackAction->ResetAirTime();
	}
	// how far will we translate this frame?
	float thisDist = kFloatSpeed * elapsed;
	// what percentage of the remaining distance will we cover?
	float thisPct = (fDistance ? thisDist / fDistance : 1.f);
	
	if(thisPct > 0.9f)
	{
		// we're pretty much done; just hop the rest of the way
		newPosition = fSeekPos;
		return false; // we're done
	}
	else
	{
		// move incrementally toward the target position
		hsVector3 thisMove = fGoalVec * thisPct;
		newPosition = fPosition + thisMove;
		return true;		// we're still processing
	}
	return true;
}



// IFinishRotation --------------------------------------
// ----------------
hsBool plAvTaskSeek::IFinishRotation(hsQuat &newRotation,
									 plArmatureMod *avatar, plAvBrainHuman *brain,
									 double time, hsScalar elapsed)
{
	// we're pretty much done; just hop the rest of the way
	newRotation = fSeekRot;
	return false;
}

// IUpdateObjective ----------------------------------------
// -----------------
hsBool plAvTaskSeek::IUpdateObjective(plArmatureMod *avatar)
{
	// This is an entirely valid case. It just means our goal is fixed.
	if (fSeekObject == nil)
		return true;

	// goal here is to express the target matrix in the avatar's PHYSICAL space
	hsMatrix44 targL2W = fSeekObject->GetLocalToWorld();
	const plCoordinateInterface* subworldCI = nil;
	if (avatar->GetController())
		subworldCI = avatar->GetController()->GetSubworldCI();
	if (subworldCI)
		targL2W = subworldCI->GetWorldToLocal() * targL2W;

	MakeMatrixUpright(targL2W);

	switch(fAlign)
	{
		// match our handle to the target matrix at the end of the given animation
		// This case isn't currently used but will be important someday. The idea
		// is that you have a target point and an animation, and you want to seek
		// the avatar to a point where he can start playing the animation and wind
		// up, after the animation completes, at the target location.
		// Hence "AlignHandleAnimEnd" = "align the avatar so the animation will
		// end on the target."
		case kAlignHandleAnimEnd:
			{
				hsMatrix44 adjustment;
				plAGAnim *anim = avatar->FindCustomAnim(fAnimName);
				// don't need to do this every frame; the animation doesn't change.
				// *** cache the adjustment;
				GetStartToEndTransform(anim, nil, &adjustment, "Handle");	// actually getting end-to-start
				// ... but we do still need to multiply by the (potentially changed) target
				targL2W = targL2W * adjustment;
			}
			break;
		case kAlignHandle:		// targetMat is already correct
		default:
			break;
	};

	GetPositionAndRotation(targL2W, &fSeekPos, &fSeekRot);

	return true;
}



// DumpDebug -----------------------------------------------------------------------------------------------------
// ----------
void plAvTaskSeek::DumpDebug(const char *name, int &x, int&y, int lineHeight, char *strBuf, plDebugText &debugTxt)
{
	sprintf(strBuf, "duration: %.2f pos: (%.3f, %.3f, %.3f) goalPos: (%.3f, %.3f, %.3f) ",
			hsTimer::GetSysSeconds() - fStartTime,
			fPosition.fX, fPosition.fY, fPosition.fZ, fSeekPos.fX, fSeekPos.fY, fSeekPos.fZ);
	debugTxt.DrawString(x, y, strBuf);
	y += lineHeight;

	sprintf(strBuf, "positioning: %d rotating %d goalVec: (%.3f, %.3f, %.3f) dist: %.3f angFwd: %.3f angRt: %.3f",
			fStillPositioning, fStillRotating, fGoalVec.fX, fGoalVec.fY, fGoalVec.fZ, fDistance, fAngForward, fAngRight);
	debugTxt.DrawString(x, y, strBuf);
	y += lineHeight;
	
	sprintf(strBuf, " distFwd: %.3f distRt: %.3f shufRange: %.3f sidAngle: %.3f sidRange: %.3f, fMinWalk: %.3f",
			fDistForward, fDistRight, fShuffleRange, fMaxSidleAngle, fMaxSidleRange, fMinFwdAngle);
	debugTxt.DrawString(x, y, strBuf);
	y += lineHeight;
}

void plAvTaskSeek::DumpToAvatarLog(plArmatureMod *avatar)
{
	plStatusLog *log = plAvatarMgr::GetInstance()->GetLog();
	char strBuf[256];
	avatar->GetMoveKeyString(strBuf);
	log->AddLine(strBuf);

	sprintf(strBuf, "    duration: %.2f pos: (%.3f, %.3f, %.3f) goalPos: (%.3f, %.3f, %.3f) ",
			hsTimer::GetSysSeconds() - fStartTime,
			fPosition.fX, fPosition.fY, fPosition.fZ, fSeekPos.fX, fSeekPos.fY, fSeekPos.fZ);
	log->AddLine(strBuf);
	
	sprintf(strBuf, "    positioning: %d rotating %d goalVec: (%.3f, %.3f, %.3f) dist: %.3f angFwd: %.3f angRt: %.3f",
			fStillPositioning, fStillRotating, fGoalVec.fX, fGoalVec.fY, fGoalVec.fZ, fDistance, fAngForward, fAngRight);
	log->AddLine(strBuf);

	sprintf(strBuf, "    distFwd: %.3f distRt: %.3f shufRange: %.3f sidAngle: %.3f sidRange: %.3f, fMinWalk: %.3f",
			fDistForward, fDistRight, fShuffleRange, fMaxSidleAngle, fMaxSidleRange, fMinFwdAngle);
	log->AddLine(strBuf);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// LOCALS
//
/////////////////////////////////////////////////////////////////////////////////////////

// QuatAngleDiff ------------------------------------
// --------------
float QuatAngleDiff(const hsQuat &a, const hsQuat &b)
{
	hsScalar theta;		/* angle between A and B */
	hsScalar cos_t;		/* sine, cosine of theta */

	/* cosine theta = dot product of A and B */
	cos_t = a.Dot(b);

	/* if B is on opposite hemisphere from A, use -B instead */
 	if (cos_t < 0.0)
	{
		cos_t = -cos_t;
	} 

	// Calling acos on 1.0 is returning an undefined value. Need to check for it.
	hsScalar epsilon = 0.00001;
	if (hsABS(cos_t - 1.f) < epsilon)
		return 0;

	theta	= hsACosine(cos_t);
	return theta;
}

// MakeMatrixUpright -------------------------------------------
// ------------------
// ensure that the z axis of the given matrix points at the sky.
// does not orthonormalize
// man, I could have sworn I did this somewhere else...
void MakeMatrixUpright(hsMatrix44 &mat)
{
	mat.fMap[0][2] = 0.0f;			// eliminate any z in the x axis
	mat.fMap[1][2] = 0.0f;			// eliminate any z in the y axis
	mat.fMap[2][0] = 0.0f; mat.fMap[2][1] = 0.0f; mat.fMap[2][2] = 1.0f;	// z axis = pure sky
}


