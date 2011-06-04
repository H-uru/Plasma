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
#include "plAvCallbackAction.h"		// havok-contaminated file: must go first

// singular
#include "plAvBrainGeneric.h"

// local
#include "plAnimStage.h"
#include "plArmatureMod.h"
// #include "plAvatarTasks.h"
#include "plAvTask.h"
#include "plAvTaskBrain.h"
#include "plAvBrainHuman.h"
#include "plAGAnimInstance.h"
#include "plMatrixChannel.h"

// global
#include "hsTimer.h"
#include "plgDispatch.h"

// other
#include "../pnNetCommon/plSDLTypes.h"
#include "../pnMessage/plCameraMsg.h"
#include "../pnMessage/plNotifyMsg.h"
#include "../plMessage/plAvatarMsg.h"
#include "../plMessage/plInputEventMsg.h"
#include "../plMessage/plSimStateMsg.h"
#include "../plMessage/plConsoleMsg.h"
#include "../plPipeline/plDebugText.h"
#include "../plInputCore/plAvatarInputInterface.h"
#include "../plMessage/plInputIfaceMgrMsg.h"

#ifdef DEBUG_MULTISTAGE
#include "plAvatarMgr.h"
#include "../plStatusLog/plStatusLog.h"
#endif

hsBool plAvBrainGeneric::fForce3rdPerson = true;
const hsScalar plAvBrainGeneric::kDefaultFadeIn = 6.f; // 1/6th of a second to fade in
const hsScalar plAvBrainGeneric::kDefaultFadeOut = 0.f; // instant fade out.

// plAvBrainGeneric ----------------
// -----------------
plAvBrainGeneric::plAvBrainGeneric()
: fRecipient(nil),
  fStages(TRACKED_NEW plAnimStageVec),
  fCurStage(0),
  fType(kGeneric),
  fExitFlags(kExitNormal),
  fMode(kEntering),
  fForward(true),
  fStartMessage(nil),
  fEndMessage(nil),
  fFadeIn(0.0f),
  fFadeOut(0.0f),
  fMoveMode(kMoveRelative),
  fCallbackAction(nil),
  fBodyUsage(plAGAnim::kBodyUnknown)
{
}

// plAvBrainGeneric --------------------------------------
// -----------------
plAvBrainGeneric::plAvBrainGeneric(plAnimStageVec *stages,
								   plMessage *startMessage,
								   plMessage *endMessage,
								   plKey recipient,
								   UInt32 exitFlags,
								   float fadeIn,
								   float fadeOut,
								   MoveMode moveMode)
: plArmatureBrain(),
  fRecipient(recipient),
  fStages(stages),
  fCurStage(0),
  fType(kGeneric),
  fExitFlags(exitFlags),
  fMode(kEntering),
  fForward(true),
  fStartMessage(startMessage),
  fEndMessage(endMessage),
  fFadeIn(fadeIn),
  fFadeOut(fadeOut),
  fMoveMode(moveMode),
  fCallbackAction(nil),
  fBodyUsage(plAGAnim::kBodyUnknown)
{
}

// plAvBrainGeneric 
plAvBrainGeneric::plAvBrainGeneric(UInt32 exitFlags, float fadeIn, float fadeOut, MoveMode moveMode)
: fRecipient(nil),
  fStages(nil),
  fCurStage(0),
  fType(kGeneric),
  fExitFlags(exitFlags),
  fMode(kEntering),
  fForward(true),
  fStartMessage(nil),
  fEndMessage(nil),
  fFadeIn(fadeIn),
  fFadeOut(fadeOut),
  fMoveMode(moveMode),
  fCallbackAction(nil),
  fBodyUsage(plAGAnim::kBodyUnknown)
{
	
}


// ~plAvBrainGeneric ----------------
// ------------------
plAvBrainGeneric::~plAvBrainGeneric()
{
	int fNumStages = fStages->size();

	for(int i = 0; i < fNumStages; i++)
	{
		plAnimStage *stage = (*fStages)[i];
		(*fStages)[i] = nil;
		stage->Detach(fAvMod);
		delete stage;
	}
	delete fStages;
}

// Activate -------------------------------------------
// ---------
void plAvBrainGeneric::Activate(plArmatureModBase *avMod)
{
	plArmatureBrain::Activate(avMod);

	if ((GetType() == kEmote || GetType() == kAFK || GetType() == kSitOnGround) && fAvMod->IsLocalAvatar())
	{
		plInputIfaceMgrMsg* msg = TRACKED_NEW plInputIfaceMgrMsg(plInputIfaceMgrMsg::kDisableClickables );
		plgDispatch::MsgSend(msg);
	}

	int numStages = fStages->size();
	if (!numStages)
		return;	
	plAnimStage *stage = (*fStages)[fCurStage];

	bool useFadeIn = fFadeIn > 0.0f;
	float initialBlend = useFadeIn ? 0.0f : 1.0f;

	if (GetType() == kEmote)
		((plArmatureMod*)avMod)->SendBehaviorNotify(plHBehavior::kBehaviorTypeEmote,true);
	double worldTime = hsTimer::GetSysSeconds();

	if (fMoveMode == kMoveRelative || fMoveMode == kMoveAbsolute)
	{
		// enable kinematic... ignore outside forces... but still collide with detector regions
		fAvMod->EnablePhysicsKinematic( true );
	}
	else if(fMoveMode == kMoveStandstill)
	{
		// Avatar stands still automatically now, so we do nothing here
	}
	if (stage->Attach(fAvMod, this, initialBlend, worldTime))
	{
		if(fStartMessage)
		{
			fStartMessage->Send();
			fStartMessage = nil;
		}
		
		if (plAvBrainGeneric::fForce3rdPerson && fAvMod->IsLocalAvatar())
		{
			// create message to force 3rd person mode
			plCameraMsg* pMsg = TRACKED_NEW plCameraMsg;
			pMsg->SetBCastFlag(plMessage::kBCastByExactType);
			pMsg->SetCmd(plCameraMsg::kResponderSetThirdPerson);
			pMsg->SetBCastFlag(plMessage::kNetPropagate, false);
			plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
		}
		
	}

	if (fType == kLadder && fAvMod->IsLocalAvatar())
		plAvatarInputInterface::GetInstance()->SetLadderMode();	

	if (fReverseFBControlsOnRelease)
		fAvMod->SetReverseFBOnIdle(true);
}

hsBool plAvBrainGeneric::IsRunningTask()
{
	if ( fStages->size() > 0 )
		return true;
	return false;
}

bool plAvBrainGeneric::MatchAnimNames(const char *names[], int count)
{
	if (count != GetStageCount())
		return false;

	int i;
	for (i = 0; i < count; i++)
	{
		if (strcmp(names[i], GetStage(i)->GetAnimName()))
			return false; // different names.
	}

	return true;
}

// Apply ----------------------------------------------------
// ------
hsBool plAvBrainGeneric::Apply(double time, hsScalar elapsed)
{
	hsBool result = false;

	switch(fMode)
	{
	case kAbort:
		break;
	case kEntering:
	case kFadingIn:
		result = IProcessFadeIn(time, elapsed);
		break;
	case kExit:
	case kFadingOut:
		// go through the fade logic whether or not we actually need a fade;
		// centralizes some exit conditions.
		result = IProcessFadeOut(time, elapsed);
		break;
	case kNormal:
		result = IProcessNormal(time, elapsed);
		break;
	}
	plArmatureBrain::Apply(time, elapsed);
	return result;
}

// Deactivate -----------------------
// -----------
void plAvBrainGeneric::Deactivate()
{
	if (fEndMessage)
	{
		fEndMessage->Send();
		fEndMessage = nil;
	}
	if (fMode != kAbort)		// we're being forcibly removed...
		IExitMoveMode();

	if (fMoveMode == kMoveRelative || fMoveMode == kMoveAbsolute) 
	{
		// re-enable normal physics... outside forces affect us
		fAvMod->EnablePhysicsKinematic( false );
	} 
	else if (fMoveMode == kMoveStandstill) 
	{
		// Avatar stands still automaticaly now, so we do nothing here
	}

	if (fType == plAvBrainGeneric::kLadder && fAvMod->IsLocalAvatar())
	{
		plAvatarInputInterface::GetInstance()->ClearLadderMode();
	}			
	
	if (fReverseFBControlsOnRelease)
		fAvMod->SetReverseFBOnIdle(false);
	
	if (plAvBrainGeneric::fForce3rdPerson && fAvMod->IsLocalAvatar())
	{
		// create message to force 3rd person mode
		plCameraMsg* pMsg = TRACKED_NEW plCameraMsg;
		pMsg->SetBCastFlag(plMessage::kBCastByExactType);
		pMsg->SetBCastFlag(plMessage::kNetPropagate, false);
		pMsg->SetCmd(plCameraMsg::kResponderUndoThirdPerson);
		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
	}
		
	plArmatureBrain::Deactivate();

	if ((GetType() == kEmote || GetType() == kAFK || GetType() == kSitOnGround) && fAvMod->IsLocalAvatar())
	{
		plInputIfaceMgrMsg* msg = TRACKED_NEW plInputIfaceMgrMsg(plInputIfaceMgrMsg::kEnableClickables );
		plgDispatch::MsgSend(msg);
	}
}

// GETRECIPIENT
plKey plAvBrainGeneric::GetRecipient()
{
	return fRecipient;
}

// SETRECIPIENT
void plAvBrainGeneric::SetRecipient(const plKey &recipient)
{
	fRecipient = recipient;
}

// RELAYNOTIFYMSG
bool plAvBrainGeneric::RelayNotifyMsg(plNotifyMsg *msg)
{
	if(fRecipient)
	{
		msg->AddReceiver(fRecipient);
		msg->Send();
		return true;
	} else {
		return false;
	}
}

// IGetAnimDelta ------------------------------------------------
// --------------
float plAvBrainGeneric::IGetAnimDelta(double time, float elapsed)
{
	float delta = 0.0f;
	plAnimStage *curStage = (*fStages)[fCurStage];
	plAnimStage::ForwardType forward = curStage->GetForwardType();
	plAnimStage::BackType back = curStage->GetBackType();
	bool fwdIsDown = (fReverseFBControlsOnRelease && fAvMod->IsFBReversed()) ? fAvMod->BackwardKeyDown() : fAvMod->ForwardKeyDown();
	hsBool backIsDown = (fReverseFBControlsOnRelease && fAvMod->IsFBReversed()) ? fAvMod->ForwardKeyDown() : fAvMod->BackwardKeyDown();

	// forward with a key down gets top priority
	if(forward == plAnimStage::kForwardKey && fwdIsDown)
	{
		// key drive forward, forward key is down
		delta = elapsed;
		fForward = true;
	} else if(back == plAnimStage::kBackKey && backIsDown)
	{
		// key drive back, back key is down
		delta = -elapsed;
		fForward = false;
	} else if (forward == plAnimStage::kForwardAuto && fForward)
	{
		// auto drive forward
		delta = elapsed;
	} else if (back == plAnimStage::kBackAuto && ! fForward)
	{
		// auto drive backward
		delta = -elapsed;
	} 
	return delta;
}

// IProcessNormal -------------------------------------------------
// ---------------
hsBool plAvBrainGeneric::IProcessNormal(double time, float elapsed)
{
	plAnimStage *curStage = (*fStages)[fCurStage];
	if(curStage)
	{
		float animDelta = IGetAnimDelta(time, elapsed);		// how far to move the anim (may be negative)
		float overage;
		hsBool done = curStage->MoveRelative(time, animDelta, overage, fAvMod);

		if(done)
		{
			bool forward = animDelta > 0.0f;
			int nextStage = forward ? curStage->GetNextStage(fCurStage) : curStage->GetPrevStage(fCurStage);

			if((nextStage == -1) || (nextStage >= fStages->size()))
			{
				// ran off one end; we're done.
				fMode = kExit;
			} else {
				ISwitchStages(fCurStage, nextStage, overage, false, 0.0f, 1.0f, -1.0f, time);
			}
		}

		return true;
	} else {
		// current stage is missing; abort
		return false;
	}
}

// IProcessFadeIn -------------------------------------------------
// ---------------
hsBool plAvBrainGeneric::IProcessFadeIn(double time, float elapsed)
{
	plAnimStage *curStage = (*fStages)[fCurStage];

	if(fMode != kFadingIn)
	{
		bool needFade = fFadeIn != 0.0f;

		if(fFadeIn == 0.0f)
		{
			IEnterMoveMode(time);	// if fadeIn's not zero, we have to wait until fade's done
											// before animating
		} else {
			curStage->GetAnimInstance()->Fade(1.0f, fFadeIn);
		}
		fMode = kFadingIn;
	} else {
		float curBlend = curStage->GetAnimInstance()->GetBlend();
		if(curBlend == 1.0f)
		{
			IEnterMoveMode(time);
		}
	}
	return true;
}

// IProcessFadeOut -------------------------------------------------
// ----------------
hsBool plAvBrainGeneric::IProcessFadeOut(double time, float elapsed)
{
	plAnimStage *curStage = (*fStages)[fCurStage];

	if(fMode != kFadingOut)
	{
		// haven't actually started fading; see if we need to
		if(fFadeOut > 0.0f)
		{
			plAGAnimInstance *curAnim = curStage->GetAnimInstance();
			if(curAnim)
			{
				curStage->GetAnimInstance()->Fade(0.0f, fFadeOut);	
				IExitMoveMode();
				fMode = kFadingOut;
			} else {
				fMode = kAbort;
				return false;
			}
		} else {
			curStage->Detach(fAvMod);
			IExitMoveMode();
			fMode = kAbort;
		}
	} else {
		// already fading; just keeping looking for the anim to zero out.
		float curBlend = curStage->GetAnimInstance()->GetBlend();
		if(curBlend == 0.0f)
		{
			curStage->Detach(fAvMod);
			fMode = kAbort;
		}
	}
	return true;
}

// ISwitchStages ---------------------------------------------------------------------------------------------------
// --------------
hsBool plAvBrainGeneric::ISwitchStages(int oldStageNum, int newStageNum, float delta, hsBool setTime, float newTime,
									   float fadeNew, hsScalar fadeOld, double worldTime)
{
#ifdef DEBUG_MULTISTAGE
	char sbuf[256];
	sprintf(sbuf,"ISwitchStage - old=%d new=%d (fCurStage=%d)",oldStageNum,newStageNum,fCurStage);
	plAvatarMgr::GetInstance()->GetLog()->AddLine(sbuf);
#endif
	if(oldStageNum != newStageNum) {
		plAnimStage *newStage = fStages->at(newStageNum);
		plAnimStage *oldStage = fStages->at(oldStageNum);
		if(setTime)
			newStage->SetLocalTime(newTime);

		hsAssert(oldStageNum < fStages->size(), "PLAVBRAINGENERIC: Stage out of range.");

		oldStage->Detach(fAvMod);
		newStage->Attach(fAvMod, this, 1.0f, worldTime);

		fCurStage = newStageNum;
		fAvMod->DirtySynchState(kSDLAvatar, 0);		// write our new stage to the server
	}
	if(setTime) {
		plAnimStage *curStage = fStages->at(fCurStage);
		curStage->SetLocalTime(newTime);
	}

	if(fMoveMode == kMoveRelative)
		fAvMod->GetRootAnimator()->Reset(worldTime);
	return true;
}

void plAvBrainGeneric::IEnterMoveMode(double time)
{
	if(fMoveMode == kMoveRelative)
	{
		fAvMod->GetRootAnimator()->Enable(true);
		fAvMod->GetRootAnimator()->Reset(time);
	}
	fMode = kNormal;
}

void plAvBrainGeneric::IExitMoveMode()
{
	if(fAvMod)
	{
		if(fMoveMode == kMoveRelative)
		{
			if(fAvMod->GetRootAnimator())
				fAvMod->GetRootAnimator()->Enable(false);
		}

		if (fFadeOut == 0.f)
		{
			// if we're exiting instantly (no fade out) then the end of the animation expects to line up with
			// the first frame of the idle animation, so we need to reset it.
			plAvBrainHuman *brain = plAvBrainHuman::ConvertNoRef(fAvMod->FindBrainByClass(plAvBrainHuman::Index()));
			if (brain)
				brain->ResetIdle();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// MESSAGE HANDLING
//
/////////////////////////////////////////////////////////////////////////////////////////

// MsgReceive -------------------------------------
// -----------
hsBool plAvBrainGeneric::MsgReceive(plMessage *msg)
{
	hsBool result = false;

	plAvBrainGenericMsg *genMsg = plAvBrainGenericMsg::ConvertNoRef(msg);
	//plAvExitModeMsg *exitMsg = plAvExitModeMsg::ConvertNoRef(msg);
	plAvTaskMsg *taskMsg = plAvTaskMsg::ConvertNoRef(msg);
	plControlEventMsg *ctrlMsg = plControlEventMsg::ConvertNoRef(msg);
	

	if(genMsg)
	{
		result = IHandleGenBrainMsg(genMsg);
	} 
//	else if(exitMsg) {
//		fMode = kExit;
//		result = true;
//	} 
	else if (taskMsg) {
		result =  IHandleTaskMsg(taskMsg);
	} 
	else if (ctrlMsg && (fExitFlags & kExitAnyInput) ) {
		fMode = kExit;
	}

	if(result == false)										// if still haven't handled msg
	{
		if(fMode == kExit)										// if we're exiting
		{
			result = fAvMod->GetNextBrain(this)->MsgReceive(msg);		// pass msg to next brain
		} else {												// otherwise
			result = plArmatureBrain::MsgReceive(msg);				// pass msg to base class
		}
	}
	
	return result;
}

// IHandleGenBrainMsg -----------------------------------------------------
// -------------------
hsBool plAvBrainGeneric::IHandleGenBrainMsg(const plAvBrainGenericMsg *msg)
{
	hsBool setTime = msg->fSetTime;
	float newTime = msg->fNewTime;
	hsBool setDirection = msg->fSetDirection;
	bool newDirection = msg->fNewDirection ? true : false;
	double worldTime = hsTimer::GetSysSeconds();

	switch(msg->fType)
	{
	case plAvBrainGenericMsg::kGotoStage:
		{
			int wantStage = msg->fWhichStage;
#ifdef DEBUG_MULTISTAGE
			char sbuf[256];
			sprintf(sbuf,"GenericMsg - Goto Stage %d (oldstage %d)",wantStage,fCurStage);
			plAvatarMgr::GetInstance()->GetLog()->AddLine(sbuf);
#endif
			if(wantStage == -1) {
				fMode = kExit;
			} else {
				int count = fStages->size();
				if(wantStage < count && wantStage >= 0)
				{
					ISwitchStages(fCurStage, wantStage, 0.0f, setTime, newTime, 1.0f, -1.0f, worldTime);
					// direction is set within the brain, not the stage
					if(setDirection)
						fForward = newDirection;
				}
			}
		}
		break;
	case plAvBrainGenericMsg::kNextStage:
		{
			int wantStage = fCurStage + 1;
#ifdef DEBUG_MULTISTAGE
			char sbuf[256];
			sprintf(sbuf,"GenericMsg - Next Stage %d (oldstage %d)",wantStage,fCurStage);
			plAvatarMgr::GetInstance()->GetLog()->AddLine(sbuf);
#endif
			if(wantStage == fStages->size())
			{
				fMode = kExit;  // walked off the end of the brain
			} else {
				ISwitchStages(fCurStage, wantStage, 0.0f, setTime, newTime, 1.0f, -1.0f, worldTime);
				if(setDirection)
					fForward = newDirection;
			}
		}
		break;
	case plAvBrainGenericMsg::kPrevStage:
		{
			int wantStage = fCurStage - 1;
#ifdef DEBUG_MULTISTAGE
			char sbuf[256];
			sprintf(sbuf,"GenericMsg - PrevStage %d (oldstage %d)",wantStage,fCurStage);
			plAvatarMgr::GetInstance()->GetLog()->AddLine(sbuf);
#endif
			if(wantStage < 0)
			{
				fMode = kExit;	// walked off the beginning of the brain
			} else {
				ISwitchStages(fCurStage, wantStage, 0.0f, setTime, 0.0f, 1.0f, -1.0f, worldTime);
				if(setDirection)
					fForward = newDirection;
			}
		}
		break;
#ifdef DEBUG_MULTISTAGE
	default:
		{
			char sbuf[256];
			sprintf(sbuf,"GenericMsg - Unknown command %d ",msg->fType);
			plAvatarMgr::GetInstance()->GetLog()->AddLine(sbuf);
		}
		break;
#endif
	}
	return true;
}

hsBool plAvBrainGeneric::IHandleTaskMsg(plAvTaskMsg *msg)
{
	plAvTask *task = msg->GetTask();
	plAvTaskBrain *brainTask = plAvTaskBrain::ConvertNoRef(task);

	if(brainTask)
	{
		plArmatureBrain * brain = brainTask->GetBrain();

		if(brain)
		{
			if(fExitFlags & kExitNewBrain)
			{
				// RULE 1: if kExitNewBrain, exit on any new brain
				fMode = kExit;
				return false;		// we didn't consume the message
			} else {
				plAvBrainGeneric * gBrain = plAvBrainGeneric::ConvertNoRef(brain);

				if(gBrain && IBrainIsCompatible(gBrain))
				{
					// RULE 2: if not kExitNewBrain and brain is compatible, apply it
					QueueTask(brainTask);
					return true;
				} else {
					if(fMode == kExit || fMode == kFadingOut)
					{
						// RULE 3: if brain is incompatible and we're exiting anyway,
						// queue it to be next
						fAvMod->GetNextBrain(this)->QueueTask(brainTask);
						return true;
					}
					// RULE 4: if brain is incompatible and we're still running, ignore.
				}
			}
		} else {
			// no brain; it's an exit task, exit and CONSUME it.
			fMode = kExit;
			return true;
		}
	} else {
		// note that this check has to come after the brain task check; if it's a brain
		// task we need to examine it so we can say whether we consumed it or not.
		// popbrain messages get consumed, even if we exit on any task.
		if(fExitFlags & kExitAnyTask)
		{
			// RULE 4: if kExitAnyTask, exit on any task (but if it was an exit brain task,
			//			make sure to consume it )
			fMode = kExit;
			return false;
		}
	}

	// RULE 4: if brain is incompatible and we're still running, ignore.	
	return false;
}

bool plAvBrainGeneric::IBrainIsCompatible(plAvBrainGeneric *otherBrain)
{
	plAGAnim::BodyUsage otherUsage = otherBrain->GetBodyUsage();

	switch(fBodyUsage)
	{
	case plAGAnim::kBodyUnknown:
		return false;
		break;
	case plAGAnim::kBodyFull:
		return false;
		break;
	case plAGAnim::kBodyUpper:
		if(otherUsage == plAGAnim::kBodyLower)
			return true;
		else
			return false;
		break;
	case plAGAnim::kBodyLower:
		if(otherUsage == plAGAnim::kBodyUpper)
			return true;
		else
			return false;
		break;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// READ/WRITE
//
/////////////////////////////////////////////////////////////////////////////////////////

// Write ----------------------------------------------------
// ------
void plAvBrainGeneric::Write(hsStream *stream, hsResMgr *mgr)
{
	plArmatureBrain::Write(stream, mgr);
	int numStages = fStages->size();
	stream->WriteSwap32(numStages);

	for(int i = 0; i < numStages; i++)
	{
		plAnimStage *stage = (*fStages)[i];
		plCreatable *cre = reinterpret_cast<plCreatable *>(stage);
		mgr->WriteCreatable(stream, cre);		// save base state
		// ** replace this with Write(..)
		stage->SaveAux(stream, mgr);			// save ephemeral state 
	}

	stream->WriteSwap32(fCurStage);
	stream->WriteSwap32(fType);
	stream->WriteSwap32(fExitFlags);
	stream->WriteByte(fMode);
	stream->Writebool(fForward);

	if(fStartMessage) {
		stream->WriteBool(true);
		mgr->WriteCreatable(stream, fStartMessage);
	} else {
		stream->WriteBool(false);
	}

	if(fEndMessage) {
		stream->WriteBool(true);
		mgr->WriteCreatable(stream, fEndMessage);
	} else {
		stream->WriteBool(false);
	}

	stream->WriteSwapScalar(fFadeIn);
	stream->WriteSwapScalar(fFadeOut);
	stream->WriteByte(fMoveMode);
	stream->WriteByte(fBodyUsage);
	mgr->WriteKey(stream, fRecipient);
}

// Read ----------------------------------------------------
// -----
void plAvBrainGeneric::Read(hsStream *stream, hsResMgr *mgr)
{
	plArmatureBrain::Read(stream, mgr);
	int numStages = stream->ReadSwap32();

	for(int i = 0; i < numStages; i++)
	{
		plCreatable *created = mgr->ReadCreatable(stream);				// load base state
		plAnimStage *stage = reinterpret_cast<plAnimStage *>(created);
		// Replace this with Read(..)
		stage->LoadAux(stream, mgr, 0.0);								// load ephemeral state

		fStages->push_back(stage);
	}

	fCurStage = stream->ReadSwap32();
	fType = static_cast<plAvBrainGeneric::BrainType>(stream->ReadSwap32());
	fExitFlags = stream->ReadSwap32();
	fMode = static_cast<Mode>(stream->ReadByte());
	fForward = stream->Readbool();

	if(stream->ReadBool()) {
		fStartMessage = plMessage::ConvertNoRef(mgr->ReadCreatable(stream));
	} else {
		fStartMessage = nil;
	}
	if(stream->ReadBool()) {
		fEndMessage = plMessage::ConvertNoRef(mgr->ReadCreatable(stream));
	} else {
		fEndMessage = nil;
	}

	fFadeIn = stream->ReadSwapScalar();
	fFadeOut = stream->ReadSwapScalar();
	fMoveMode = static_cast<MoveMode>(stream->ReadByte());
	fBodyUsage = static_cast<plAGAnim::BodyUsage>(stream->ReadByte());
	fRecipient = mgr->ReadKey(stream);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// MINOR FNs / GETTERS & SETTERS
//
/////////////////////////////////////////////////////////////////////////////////////////

// LeaveAge ---------------------
// ---------
hsBool plAvBrainGeneric::LeaveAge()
{
	IExitMoveMode();

	fMode = kAbort;
	return true;
}

// AddStage --------------------------------------
// ---------
int plAvBrainGeneric::AddStage(plAnimStage *stage)
{
	if(!fStages)
		fStages = TRACKED_NEW plAnimStageVec;
	fStages->push_back(stage);
	return fStages->size() - 1;
}

// GetStageNum --------------------------------------
// ------------
int plAvBrainGeneric::GetStageNum(plAnimStage *stage)
{
	int count = fStages->size();
	for(int i = 0; i < count; i++)
	{
		plAnimStage *any = (*fStages)[i];
		if(any == stage)
		{
			return i;
		}
	}
	return -1;
}

// GetCurStageNum --------------------
// ---------------
int plAvBrainGeneric::GetCurStageNum()
{
	return fCurStage;
}

// GetStageCount --------------------
// --------------
int plAvBrainGeneric::GetStageCount()
{
	return fStages->size();
}

// GetStage ---------------------------------------
// ---------
plAnimStage * plAvBrainGeneric::GetStage(int which)
{
	return fStages->at(which);
}

// GetCurStage ------------------------------
// ------------
plAnimStage * plAvBrainGeneric::GetCurStage()
{
	return fStages->at(fCurStage);
}

// SetType -------------------------------------------------------------------------------
// --------
plAvBrainGeneric::BrainType plAvBrainGeneric::SetType(plAvBrainGeneric::BrainType newType)
{
	BrainType oldType = fType;
	fType = newType;
	return oldType;
}

// GetType --------------------------------------------
// --------
plAvBrainGeneric::BrainType plAvBrainGeneric::GetType()
{
	return fType;
}

plAGAnim::BodyUsage plAvBrainGeneric::GetBodyUsage()
{
	return fBodyUsage;
}

void plAvBrainGeneric::SetBodyUsage(plAGAnim::BodyUsage bodyUsage)
{
	fBodyUsage = bodyUsage;
}


/////////////////////////////////////////////////////////////////////////////////////////
//
// DEBUGGING
//
/////////////////////////////////////////////////////////////////////////////////////////

// DumpToDebugDisplay ----------------------------------------------------------------------------------------
// -------------------
void plAvBrainGeneric::DumpToDebugDisplay(int &x, int &y, int lineHeight, char *strBuf, plDebugText &debugTxt)
{
	debugTxt.DrawString(x, y, "Brain type: Generic AKA Multistage");
	y += lineHeight;

	int stageCount = fStages->size();
	for(int i = 0; i < stageCount; i++)
	{
		plAnimStage *stage = (*fStages)[i];
		stage->DumpDebug(i == fCurStage, x, y, lineHeight, strBuf, debugTxt);
	}
}




