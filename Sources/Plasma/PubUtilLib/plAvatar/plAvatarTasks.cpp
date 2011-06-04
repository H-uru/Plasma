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

// singular
#include "plAvatarTasks.h"

// local
#include "plArmatureMod.h"
#include "plSeekPointMod.h"
#include "plAvBrainHuman.h"
#include "plAGAnim.h"
#include "plAGAnimInstance.h"
#include "plAGModifier.h"
#include "plMatrixChannel.h"
#include "plAvCallbackAction.h"
#include "plAvatarMgr.h"

// global
#include "hsUtils.h"

// other
#include "plgDispatch.h"
#include "../plMessage/plAvatarMsg.h"
#include "../plMessage/plAnimCmdMsg.h"
#include "../plMessage/plOneShotCallbacks.h"
#include "../plMessage/plConsoleMsg.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../plPipeline/plDebugText.h"
#include "../plInputCore/plInputInterfaceMgr.h"
#include "../plNetClient/plNetClientMgr.h"
#include "../plNetCommon/plNetCommon.h"
#include "../plMessage/plLinkToAgeMsg.h"
#include "../pfMessage/pfKIMsg.h"

// for console hack
hsBool plAvOneShotTask::fForce3rdPerson = true;
#include "../pnMessage/plCameraMsg.h"

/////////////
//
// PLAVTASK
// Abstract definition for the avatar task class
//
/////////////
plAvTask::plAvTask()
{
}

// START
hsBool plAvTask::Start(plArmatureMod *avatar, plArmatureBrain *brain, double time, hsScalar elapsed)
{
	return true;	// true indicates the task has started succesfully
}

// PROCESS
hsBool plAvTask::Process(plArmatureMod *avatar, plArmatureBrain *brain, double time, hsScalar elapsed)
{
	return false;
}

// Finish -----------------------------------------------------------------------------------
// -------
void plAvTask::Finish(plArmatureMod *avatar, plArmatureBrain *brain, double time, hsScalar elapsed)
{
}


// DUMPDEBUG
void plAvTask::DumpDebug(const char *name, int &x, int&y, int lineHeight, char *strBuf, plDebugText &debugTxt)
{
	debugTxt.DrawString(x, y, "<anonymous task>");
	y += lineHeight;
}

// READ
void plAvTask::Read(hsStream *stream, hsResMgr *mgr)
{
	plCreatable::Read(stream, mgr);
}

// WRITE
void plAvTask::Write(hsStream *stream, hsResMgr *mgr)
{
	plCreatable::Write(stream, mgr);
}

void plAvTask::ILimitPlayersInput(plArmatureMod *avatar)
{
	// make sure this is the local avatar we are talking about	
	if (avatar == plAvatarMgr::GetInstance()->GetLocalAvatar())
	{
		plInputInterfaceMgr::GetInstance()->ForceCursorHidden(true);
		// tell the KI to be disabled while we are busy
		pfKIMsg* msg = TRACKED_NEW pfKIMsg(pfKIMsg::kTempDisableKIandBB);
		plgDispatch::MsgSend( msg );
	}
}

void plAvTask::IUndoLimitPlayersInput(plArmatureMod *avatar)
{
	// make sure this is the local avatar we are talking about	
	if (avatar == plAvatarMgr::GetInstance()->GetLocalAvatar())
	{
		plInputInterfaceMgr::GetInstance()->ForceCursorHidden(false);
		// tell the KI to be re-enabled
		pfKIMsg* msg = TRACKED_NEW pfKIMsg(pfKIMsg::kTempEnableKIandBB);
		plgDispatch::MsgSend( msg );
	}
}

/////////////
//
// AVSEEKTASK
//
/////////////

// CTOR default
plAvSeekTask::plAvSeekTask()
: fAnimName(nil),
  fAlign(kAlignHandle),
  fDuration(0.25f),
  fTarget(nil),
  fAnimInstance(nil),
  fTargetTime(nil),
  fPhysicalAtStart(false),
  fCleanup(false)
{
}

// CTOR target, align, animName
plAvSeekTask::plAvSeekTask(plKey target, plAvAlignment align, const char *animName)
: fAlign(align),
  fDuration(0.25f),
  fTarget(target),
  fAnimInstance(nil),
  fTargetTime(nil),
  fPhysicalAtStart(false),
  fCleanup(false)
{
	fAnimName = hsStrcpy(animName);
}

// CTOR target
plAvSeekTask::plAvSeekTask(plKey target)
: fAnimName(nil),
fAlign(kAlignHandle),
fDuration(0.25f),
fTarget(target),
fAnimInstance(nil),
fTargetTime(nil),
fPhysicalAtStart(false),
fCleanup(false)
{
}

void GetPositionAndRotation(hsMatrix44 transform, hsScalarTriple *position, hsQuat *rotation)
{
	hsPoint3 p = (hsPoint3)transform.GetTranslate();
	position->fX = p.fX; position->fY = p.fY; position->fZ = p.fZ;
	
	
	transform.RemoveScale();
	
	rotation->SetFromMatrix(&transform);
	rotation->Normalize();
	
	float angle;
	hsVector3 axis;
	
	rotation->GetAngleAxis(&angle, &axis);
}

// START
// Adjust our goal time based on our duration and the current time
hsBool plAvSeekTask::Start(plArmatureMod *avatar, plArmatureBrain *brain, double time, hsScalar elapsed)
{
	fTargetTime = time + fDuration;		// clock starts now....
	fPhysicalAtStart = avatar->IsPhysicsEnabled();
	avatar->EnablePhysics(false);		// always turn physics off for seek
	plAvBrainHuman *huBrain = plAvBrainHuman::ConvertNoRef(brain);
	if(huBrain)
		huBrain->IdleOnly();
	
	ILimitPlayersInput(avatar);
	
	if (!fTarget || !fTarget->ObjectIsLoaded())
	{
		fCleanup = true;
		return true;
	}
	
	plSceneObject* seekTarget = plSceneObject::ConvertNoRef(fTarget->ObjectIsLoaded());
	hsMatrix44 targetL2W = seekTarget->GetLocalToWorld();
	const plCoordinateInterface* subworldCI = nil;
	if (avatar->GetController())
		subworldCI = avatar->GetController()->GetSubworldCI();
	if (subworldCI)
		targetL2W = subworldCI->GetWorldToLocal() * targetL2W;

	switch(fAlign)
	{
		// just match our handle to the target matrix
		case kAlignHandle:
			// targetL2Sim is already correct
			break;
		// match our handle to the target matrix at the end of the given animation
		case kAlignHandleAnimEnd:
			{
				hsMatrix44 adjustment;
				plAGAnim *anim = avatar->FindCustomAnim(fAnimName);
				GetStartToEndTransform(anim, nil, &adjustment, "Handle");	// actually getting end-to-start
				targetL2W = targetL2W * adjustment;
			}
			break;
		default:
			break;
	};

	GetPositionAndRotation(targetL2W, &fTargetPosition, &fTargetRotation);
	Process(avatar, brain, time, elapsed);
	return true;
}

// CALCHANDLETARGETPOSITION
void CalcHandleTargetPosition(hsMatrix44 &result, plSceneObject *handle, plSceneObject *target, hsMatrix44 &bodyToHandle)
{
	hsMatrix44 targetToWorld = target->GetLocalToWorld();
	
	result = bodyToHandle * targetToWorld;
}

// CALCHANDLETARGETPOSITION
// where should I move my insertion point so that my bodyRoot lines up with the target?
void CalcHandleTargetPosition(hsMatrix44 &result, plSceneObject *insert, plSceneObject *target, plSceneObject *bodyRoot)
{
	hsMatrix44 bodyToHandle = bodyRoot->GetLocalToParent();
	CalcHandleTargetPosition(result, insert, target, bodyToHandle);	
}

// PROCESS
// Move closer to the goal position and orientation
hsBool plAvSeekTask::Process(plArmatureMod *avatar, plArmatureBrain *brain, double time, hsScalar elapsed)
{
	hsQuat rotation;
	hsPoint3 position;
	avatar->GetPositionAndRotationSim(&position, &rotation);

	// We've had a history of odd bugs caused by assuming a rotation quat is normalized.
	// This line here seems to be fixing one of them. (Avatars scaling oddly when smart seeking.)
	rotation.Normalize();

	double timeToGo = fTargetTime - time - elapsed;	// time from *beginning* of this interval to the goal
	if (fCleanup)
	{
		avatar->EnablePhysics( fPhysicalAtStart );
		IUndoLimitPlayersInput(avatar);
		
		return false;		// we're done processing
	}
	else if(timeToGo < .01)
	{
		fTargetRotation.Normalize();
		avatar->SetPositionAndRotationSim(&fTargetPosition, &fTargetRotation);
		fCleanup = true;	// we're going to wait one frame for the transform to propagate
		return true;		// still running until next frame/cleanup
	}
	else
	{
		hsPoint3 posToGo = fTargetPosition - position;			// vec from here to the goal
		float thisPercentage = (float)(elapsed / timeToGo);			

		hsPoint3 newPosition = position + posToGo * thisPercentage;
		hsQuat newRotation;
		newRotation.SetFromSlerp(rotation, fTargetRotation, thisPercentage);

		newRotation.Normalize();
		avatar->SetPositionAndRotationSim(&newPosition, &newRotation);
		return true;		// we're still processing
	}
}

void plAvSeekTask::LeaveAge(plArmatureMod *avatar)
{
	fTarget = nil;
	fCleanup = true;
}

///////////////////
//
// PLAVANIMTASK
//
///////////////////

// CTOR default
plAvAnimTask::plAvAnimTask()
: fAnimName(nil),
  fInitialBlend(0.0f),
  fTargetBlend(0.0f),
  fFadeSpeed(0.0f),
  fSetTime(0.0f),
  fStart(false),
  fLoop(false),
  fAttach(false),
  fAnimInstance(nil)
{
}

// CTOR animName, initialBlend, targetBlend, fadeSpeed, start, loop, attach
plAvAnimTask::plAvAnimTask(const char *animName,
						   hsScalar initialBlend,
						   hsScalar targetBlend,
						   hsScalar fadeSpeed,
						   hsScalar setTime,
						   hsBool start,
						   hsBool loop,
						   hsBool attach)
: fInitialBlend(initialBlend),
  fTargetBlend(targetBlend),
  fFadeSpeed(fadeSpeed),
  fSetTime(setTime),
  fStart(start),
  fLoop(loop),
  fAttach(attach),
  fAnimInstance(nil)
{
	if(animName)
		fAnimName = hsStrcpy(animName);
}

// CTOR animName, fadeSpeed, attach
plAvAnimTask::plAvAnimTask(const char *animName, hsScalar fadeSpeed, hsBool attach)
: fInitialBlend(0.0f),
  fTargetBlend(0.0f),
  fFadeSpeed(fadeSpeed),
  fSetTime(0.0f),
  fStart(false),
  fLoop(false),
  fAttach(attach),
  fAnimInstance(nil)
{
	if(animName)
		fAnimName = hsStrcpy(animName);
}




// DTOR
plAvAnimTask::~plAvAnimTask()
{
	if(fAnimName)
	{
		delete[] fAnimName;
		fAnimName = nil;
	}
}

// START
hsBool plAvAnimTask::Start(plArmatureMod *avatar, plArmatureBrain *brain, double time, hsScalar elapsed)
{
	hsBool result = false;
	if(fAttach)
	{
		plAGAnimInstance * aInstance = avatar->FindOrAttachInstance(fAnimName, fInitialBlend);

		if(aInstance)
		{
			if(fStart)
				aInstance->Start(fStart);
			if(fSetTime > 0)
				aInstance->SetCurrentTime(fSetTime, true);
			if(fTargetBlend > fInitialBlend)
			{
				aInstance->Fade(fTargetBlend, fFadeSpeed);
			}
			aInstance->SetLoop(fLoop);

			result = true;
		}
		else
		{
			hsStatusMessageF("Couldn't find animation <%s> for plAvAnimTask: will try again", fAnimName);
		}
	}
	else
	{
		fAnimInstance = avatar->FindAnimInstance(fAnimName);
		if(fAnimInstance)
		{
			// start fading towards zero
			fAnimInstance->Fade(0.0, fFadeSpeed);
		}
		// if we started the fade, we're done and ready to process
		// if we couldn't find the animation, we're still done.
		result = true;
	}
	return result;
}

// PROCESS
plAvAnimTask::Process(plArmatureMod *avatar, plArmatureBrain *brain, double time, hsScalar elapsed)
{
	// the only reason we need this function is to watch the animation until it fades out
	hsBool result = false;
	if(fAttach)
	{
		// we finished in the Start() function
	}
	else
	{
		if(fAnimInstance)
		{
			if(fAnimInstance->GetBlend() < 0.1)
			{
				avatar->DetachAnimation(fAnimInstance);
			}
			else
			{
				// still waiting for the fadeout; keep the task alive
				result = true;
			}
		}
	}
	return result;
}

// LEAVEAGE
void plAvAnimTask::LeaveAge(plArmatureMod *avatar)
{
	// if we are supposed to be removing the animation anyway, kill it completely on link out
	if (!fAttach)
	{
		fAnimInstance = avatar->FindAnimInstance(fAnimName);
		if(fAnimInstance)
			avatar->DetachAnimation(fAnimInstance);
	}
}

// READ
void plAvAnimTask::Read(hsStream *stream, hsResMgr *mgr)
{
	fAnimName = stream->ReadSafeString();
	fInitialBlend = stream->ReadSwapScalar();
	fTargetBlend = stream->ReadSwapScalar();
	fFadeSpeed = stream->ReadSwapScalar();
	fSetTime = stream->ReadSwapScalar();
	fStart = stream->ReadBool();
	fLoop = stream->ReadBool();
	fAttach = stream->ReadBool();
}

// WRITE
void plAvAnimTask::Write(hsStream *stream, hsResMgr *mgr)
{
	stream->WriteSafeString(fAnimName);
	stream->WriteSwapScalar(fInitialBlend);
	stream->WriteSwapScalar(fTargetBlend);
	stream->WriteSwapScalar(fFadeSpeed);
	stream->WriteSwapScalar(fSetTime);
	stream->WriteBool(fStart);
	stream->WriteBool(fLoop);
	stream->WriteBool(fAttach);
}

////////////////
//
// AVONESHOTTASK
// OBSOLETE -- DEPRECATED
//
////////////////

void plAvOneShotTask::InitDefaults()
{
	fBackwards = false;
	fDisableLooping = false;
	fDisablePhysics = true;
	fAnimName = nil;
	fMoveHandle = false;
	fAnimInstance = nil;
	fDrivable = false;
	fReversible = false;
	fEnablePhysicsAtEnd = false;
	fDetachAnimation = false;
	fIgnore = false;
	fCallbacks = nil;
	fWaitFrames = 0;
}

// CTOR default
plAvOneShotTask::plAvOneShotTask()
{
	InitDefaults();
}

// CTOR (animName, drivable, reversible)
// this construct is typically used when you want to create a one-shot task as part of a sequence
// of tasks
// it's different than the message-based constructor in that fDetachAnimation and fMoveHandle default to false
plAvOneShotTask::plAvOneShotTask(const char *animName, hsBool drivable, hsBool reversible, plOneShotCallbacks *callbacks)
{
	InitDefaults();

	fDrivable = drivable;
	fReversible = reversible;
	fCallbacks = callbacks;
	
	// we're going to use this sometime in the future, better ref it so someone else doesn't release it
	hsRefCnt_SafeRef(fCallbacks);
	fAnimName = hsStrcpy(animName);
}

// CTOR (plAvOneShotMsg, plArmatureMod)
// this constructor is typically used when we're doing a classic, isolated one-shot
// fDetachAnimation and fMoveHandle both default to *true*
plAvOneShotTask::plAvOneShotTask (plAvOneShotMsg *msg, plArmatureMod *avatar, plArmatureBrain *brain)
{
	InitDefaults();

	fDrivable = msg->fDrivable;
	fReversible = msg->fReversible;
	fCallbacks = msg->fCallbacks;
	fDetachAnimation = true;
	fMoveHandle = true;

	// we're going to use this sometime in the future, better ref it so someone else doesn't release it
	hsRefCnt_SafeRef(fCallbacks);
	fAnimName = hsStrcpy(msg->fAnimName);
}

// DTOR
plAvOneShotTask::~plAvOneShotTask()
{
	if(fAnimName)
		delete[] fAnimName;
	hsRefCnt_SafeUnRef(fCallbacks);
}


// START
hsBool plAvOneShotTask::Start(plArmatureMod *avatar, plArmatureBrain *brain, double time, hsScalar elapsed)
{
	hsBool result = false;

	if (fIgnore)
		return true;

	plAGMasterMod * master = avatar;

	fAnimInstance = master->AttachAnimationBlended(fAnimName, 0);
	fDetachAnimation = true;

	if(fAnimInstance)
	{
		fEnablePhysicsAtEnd = (avatar->IsPhysicsEnabled() && fDisablePhysics);
		if (fEnablePhysicsAtEnd)
		{
			// Must do the physics re-enable through a callback so that it happens before the "done" callback and we don't
			// step over some script's attempt to disable physics again.
			plAvatarPhysicsEnableCallbackMsg *epMsg = TRACKED_NEW plAvatarPhysicsEnableCallbackMsg(avatar->GetKey(), kStop, 0, 0, 0, 0);
			fAnimInstance->GetTimeConvert()->AddCallback(epMsg);
			hsRefCnt_SafeUnRef(epMsg);
		}	

		if (fCallbacks)
		{
			fAnimInstance->AttachCallbacks(fCallbacks);
			// ok, we're done with it, release it back to the river
			hsRefCnt_SafeUnRef(fCallbacks);
			fCallbacks = nil;
		}

		fAnimInstance->SetBlend(1.0f);
		fAnimInstance->SetSpeed(1.0f);
		plAnimTimeConvert *atc = fAnimInstance->GetTimeConvert();
		if (fBackwards)
			atc->Backwards();
		if (fDisableLooping)
			atc->Loop(false);
		
		fAnimInstance->SetCurrentTime(fBackwards ? atc->GetEnd() : atc->GetBegin(), true);
		fAnimInstance->Start(time);
			
		fWaitFrames = 2;		// wait two frames after animation finishes before finalizing


		if (fDisablePhysics)
			avatar->EnablePhysics(false);

		ILimitPlayersInput(avatar);
		
		// this is for a console command hack
		if (plAvOneShotTask::fForce3rdPerson && avatar->IsLocalAvatar())
		{
			// create message
			plCameraMsg* pMsg = TRACKED_NEW plCameraMsg;
			pMsg->SetBCastFlag(plMessage::kBCastByExactType);
			pMsg->SetBCastFlag(plMessage::kNetPropagate, false);
			pMsg->SetCmd(plCameraMsg::kResponderSetThirdPerson);
			plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
		}

		fMoveHandle = (fAnimInstance->GetAnimation()->GetChannel("Handle") != nil);
		if(fMoveHandle)
		{
			plMatrixDifferenceApp *differ = avatar->GetRootAnimator();
			differ->Reset(time);		// throw away any old state
			differ->Enable(true);
		}

		avatar->ApplyAnimations(time, elapsed);							

		result = true;
	}
	else
	{
		char buf[256];
		sprintf(buf, "Oneshot: Can't find animation <%s>; all bets are off.", fAnimName);
		hsAssert(false, buf);
		result = true;
	}
	return result;
}

// PROCESS
plAvOneShotTask::Process(plArmatureMod *avatar, plArmatureBrain *brain, double time, hsScalar elapsed)
{
	// *** if we are under mouse control, adjust it here

	avatar->ApplyAnimations(time, elapsed);
	if(fAnimInstance)
	{
		if(fAnimInstance->IsFinished())
		{
			const plAGAnim * animation = fAnimInstance->GetAnimation();
			double endTime = (fBackwards ? animation->GetStart() : animation->GetEnd());
			fAnimInstance->SetCurrentTime((hsScalar)endTime);
			avatar->ApplyAnimations(time, elapsed);

			if(--fWaitFrames == 0)
			{

				plSceneObject *handle = avatar->GetTarget(0);

				avatar->DetachAnimation(fAnimInstance);
				avatar->GetRootAnimator()->Enable(false);
				plAvBrainHuman *humanBrain = plAvBrainHuman::ConvertNoRef(brain);
				if(fEnablePhysicsAtEnd)
				{
#if 0//ndef PLASMA_EXTERNAL_RELEASE
					if (!humanBrain || humanBrain->fCallbackAction->HitGroundInThisAge())
					{
						// For some reason, calling CheckValidPosition at the beginning of
						// an age can cause detectors to incorrectly report collisions. So
						// we only call this if we're in the age.
						// 
						// It's only debugging code anyway to help the artist check that
						// their oneshot doesn't end while penetrating geometry.
						char *overlaps = nil;
						if (avatar->GetPhysical())
							avatar->GetPhysical()->CheckValidPosition(&overlaps);
						if (overlaps)
						{
							char *buffy = TRACKED_NEW char[64 + strlen(overlaps)];
							sprintf(buffy, "Oneshot ends overlapping %s", overlaps);
							plConsoleMsg *showLine = TRACKED_NEW plConsoleMsg( plConsoleMsg::kAddLine, buffy );
							showLine->Send();
							delete[] overlaps;
							delete[] buffy;
						}
					}
#endif
				}					
				if (humanBrain)
					humanBrain->ResetIdle();

				IUndoLimitPlayersInput(avatar);
				// this is for a console command hack
				if (plAvOneShotTask::fForce3rdPerson && avatar->IsLocalAvatar())
				{
					// create message
					plCameraMsg* pMsg = TRACKED_NEW plCameraMsg;
					pMsg->SetBCastFlag(plMessage::kBCastByExactType);
					pMsg->SetBCastFlag(plMessage::kNetPropagate, false);
					pMsg->SetCmd(plCameraMsg::kResponderUndoThirdPerson);
					plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
				}
				
				return false;
			}
			else
				return true;	// still running; waiting for fWaitFrames == 0
		}
		else
			return true;
	}
	else
		return false;
}

void plAvOneShotTask::LeaveAge(plArmatureMod *avatar)
{
	if (fAnimInstance)
		fAnimInstance->Stop();

	if (fEnablePhysicsAtEnd)
		avatar->EnablePhysics(true);

	IUndoLimitPlayersInput(avatar);
	fIgnore = true;
}

void plAvOneShotTask::SetAnimName(char *name)
{
	delete [] fAnimName;
	fAnimName = hsStrcpy(name);
}

//////////////////////
//
// PLAVONESHOTLINKTASK
//
//////////////////////

plAvOneShotLinkTask::plAvOneShotLinkTask() : plAvOneShotTask(), 
fMarkerName(nil), 
fMarkerTime(-1),
fStartTime(0),
fLinkFired(false)
{
	fDisablePhysics = false;		
}

plAvOneShotLinkTask::~plAvOneShotLinkTask()
{
	delete [] fMarkerName;
}

// task protocol
hsBool plAvOneShotLinkTask::Start(plArmatureMod *avatar, plArmatureBrain *brain, double time, hsScalar elapsed)
{
	hsBool result = plAvOneShotTask::Start(avatar, brain, time, elapsed);
	fStartTime = time;

	if (fAnimInstance && fMarkerName)
	{
		const plATCAnim *anim = plATCAnim::ConvertNoRef(fAnimInstance->GetAnimation());
		if (anim)
		{
			// GetMarker returns -1 if the marker isn't found
			fMarkerTime = anim->GetMarker(fMarkerName);
		}
	}
	return result;
}

hsBool plAvOneShotLinkTask::Process(plArmatureMod *avatar, plArmatureBrain *brain, double time, hsScalar elapsed)
{
	hsBool result = plAvOneShotTask::Process(avatar, brain, time, elapsed);
	if (fIgnore)
		return result;

	if (avatar->GetTarget(0) == plNetClientApp::GetInstance()->GetLocalPlayer())
	{
		if (!fLinkFired && (fStartTime + fMarkerTime < time))
		{
			avatar->ILinkToPersonalAge();
			
			avatar->EnablePhysics(false, plArmatureMod::kDisableReasonLinking);
			fLinkFired = true;
		}
	}

	return result;
}

void plAvOneShotLinkTask::Write(hsStream *stream, hsResMgr *mgr)
{
	plAvOneShotTask::Write(stream, mgr);
	stream->WriteSafeString(fAnimName);
	stream->WriteSafeString(fMarkerName);
}

void plAvOneShotLinkTask::Read(hsStream *stream, hsResMgr *mgr)
{
	plAvOneShotTask::Read(stream, mgr);
	fAnimName = stream->ReadSafeString();
	fMarkerName = stream->ReadSafeString();
}

void plAvOneShotLinkTask::SetMarkerName(char *name)
{
	delete [] fMarkerName;
	fMarkerName = hsStrcpy(name);
}














