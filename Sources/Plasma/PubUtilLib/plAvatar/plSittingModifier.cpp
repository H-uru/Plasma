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
// singular
#include "plSittingModifier.h"

//other
#include "../plMessage/plAvatarMsg.h"
#include "../pnMessage/plNotifyMsg.h"
#include "../pnMessage/plCameraMsg.h"
#include "../plAvatar/plArmatureMod.h"
#include "../plAvatar/plAnimStage.h"
#include "../plAvatar/plAvTaskBrain.h"
#include "../plAvatar/plAvBrainGeneric.h"
#include "../plAvatar/plAvBrainHuman.h"
#include "../plAvatar/plAvatarMgr.h"
#include "../pnNetCommon/plNetApp.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../plInputCore/plAvatarInputInterface.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
// IMPLEMENTATION
//
/////////////////////////////////////////////////////////////////////////////////////////

// CTOR ------------------------------
// -----
plSittingModifier::plSittingModifier()
: fTriggeredAvatarKey(nil),
  fMiscFlags(0)
{
}

// CTOR ------------------------------------------------------------------------
// -----
plSittingModifier::plSittingModifier(bool hasFront, bool hasLeft, bool hasRight)
: fTriggeredAvatarKey(nil),
  fMiscFlags(0)
{
	if (hasFront)
		fMiscFlags |= kApproachFront;
	if (hasLeft)
		fMiscFlags |= kApproachLeft;
	if (hasRight)
		fMiscFlags |= kApproachRight;
}

// DTOR -------------------------------
// -----
plSittingModifier::~plSittingModifier()
{
}

// Read -----------------------------------------------------
// -----
void plSittingModifier::Read(hsStream *stream, hsResMgr *mgr)
{
	plSingleModifier::Read(stream, mgr);

	fMiscFlags = stream->ReadByte();

	int keyCount = stream->ReadSwap32();
	for (int i = 0; i < keyCount; i++ )
		fNotifyKeys.Append(mgr->ReadKey(stream));
}

// Write -----------------------------------------------------
// ------
void plSittingModifier::Write(hsStream *stream, hsResMgr *mgr)
{
	plSingleModifier::Write(stream, mgr);

	stream->WriteByte(fMiscFlags);
	
	stream->WriteSwap32(fNotifyKeys.GetCount());
	for (int i = 0; i < fNotifyKeys.GetCount(); i++)
		mgr->WriteKey(stream, fNotifyKeys[i]);
}

// ISetupNotify -------------------------------------------------------------------------
// -------------
void plSittingModifier::ISetupNotify(plNotifyMsg *notifyMsg, plNotifyMsg *originalNotify)
{
	// Copy the original events to the new notify (some notify receivers need to have events)
	int i;
	for (i = 0; i < originalNotify->GetEventCount(); i++)
		notifyMsg->AddEvent(originalNotify->GetEventRecord(i));
	for (i = 0; i < fNotifyKeys.Count(); i++)
	{
		plKey receiver = fNotifyKeys[i];
		notifyMsg->AddReceiver(receiver);
	}
	notifyMsg->SetSender(GetKey());
	plNetClientApp::InheritNetMsgFlags(originalNotify, notifyMsg, true);
}

// MsgReceive --------------------------------------
// -----------
hsBool plSittingModifier::MsgReceive(plMessage *msg)
{
	hsBool result = false;
	
	plNotifyMsg* notifyMsg = plNotifyMsg::ConvertNoRef(msg);
	if(notifyMsg)
	{
		if (notifyMsg->fState == 1.0)
		{
			// In case the previous occupant quit, lost-connection, etc.
			if (fTriggeredAvatarKey && !fTriggeredAvatarKey->ObjectIsLoaded())
				UnTrigger();
			
			// only fire if we're not already triggered
			if (!fTriggeredAvatarKey)
			{
				// a notify message with a state of 1 tells us to fire.
				// we'll copy any events from this notify and use them when we send our
				// own notify messages -- 
				plKey avatarKey = notifyMsg->GetAvatarKey();
				plSceneObject * obj = plSceneObject::ConvertNoRef(avatarKey->ObjectIsLoaded());
				const plArmatureMod * avMod = (plArmatureMod*)obj->GetModifierByType(plArmatureMod::Index());
				plAvBrainHuman *brain = (avMod ? plAvBrainHuman::ConvertNoRef(avMod->GetCurrentBrain()) : nil);
				if (brain && !brain->IsRunningTask())
				{
					plNotifyMsg *notifyEnter = TRACKED_NEW plNotifyMsg();	// a message to send back when the brain starts
					notifyEnter->fState = 1.0f;						// it's an "on" event
					ISetupNotify(notifyEnter, notifyMsg);			// copy events and address to sender
			
					plNotifyMsg *notifyExit = nil;
					if (avatarKey == plNetClientApp::GetInstance()->GetLocalPlayerKey())
					{
						notifyExit = TRACKED_NEW plNotifyMsg();			// a new message to send back when the brain's done
						notifyExit->fState = 0.0f;				// it's an "off" event
						ISetupNotify(notifyExit, notifyMsg);	// copy events and address to sender
						notifyExit->AddReceiver(GetKey());		// have this message come back to us as well

						// A player may have joined while we're sitting. We can't update them with the exit notify at
						// that point (security hole), so instead the local avatar sends the message out to everybody
						// when done.
						notifyExit->SetBCastFlag(plMessage::kNetStartCascade, false);
						notifyExit->SetBCastFlag(plMessage::kNetPropagate, true);
						notifyExit->SetBCastFlag(plMessage::kNetForce, true);
					}
					Trigger(avMod, notifyEnter, notifyExit);
				}
			}
			// eat the message either way
			result = true;
		} else if (notifyMsg->fState == 0.0f && msg->GetSender() == GetKey()) {
			// it's a notify, but off; untrigger
			UnTrigger();
			result = true;
		}
		else if (notifyMsg->GetEventCount() > 0 && notifyMsg->GetEventRecord(0)->fEventType == proEventData::kMultiStage && notifyMsg->GetAvatarKey() == plNetClientApp::GetInstance()->GetLocalPlayerKey())
		{
			proMultiStageEventData* mStage = (proMultiStageEventData*)notifyMsg->GetEventRecord(0);
			if (mStage->fEvent == proMultiStageEventData::kEnterStage && mStage->fStage == 1)
			{
				plKey avatarKey = notifyMsg->GetAvatarKey();
				plSceneObject * obj = plSceneObject::ConvertNoRef(avatarKey->ObjectIsLoaded());
				plArmatureMod * avMod = (plArmatureMod*)obj->GetModifierByType(plArmatureMod::Index());

				UInt32 flags = kBCastToClients | kUseRelevanceRegions | kForceFullSend;
				avMod->DirtyPhysicalSynchState(flags);
			}
		}
	}

	return result || plSingleModifier::MsgReceive(msg);
}

// Trigger ----------------------------------------------------------------------------------------
// --------
void plSittingModifier::Trigger(const plArmatureMod *avMod, plNotifyMsg *enterNotify, plNotifyMsg *exitNotify)
{
	if (avMod)
	{
		plKey avModKey = avMod->GetKey();			// key to the avatar MODIFIER
		plKey ourKey = GetKey();					// key to this modifier
		plKey seekKey = GetTarget()->GetKey();		// key to the scene object this modifier's on

		// send the SEEK message


		char *animName = nil;	// this will be the name of our sit animation, which we
								// need to know so we can seek properly.
		
		plAvBrainGeneric *brain = IBuildSitBrain(avModKey, seekKey, &animName, enterNotify, exitNotify);
		if(brain)
		{
			plAvSeekMsg *seekMsg = TRACKED_NEW plAvSeekMsg(ourKey, avModKey, seekKey, 0.0f, true, kAlignHandleAnimEnd, animName);
			seekMsg->Send();
			plAvTaskBrain *brainTask = TRACKED_NEW plAvTaskBrain(brain);
			plAvTaskMsg *brainMsg = TRACKED_NEW plAvTaskMsg(ourKey, avModKey, brainTask);

			brainMsg->Send();
			
			if (avModKey == plAvatarMgr::GetInstance()->GetLocalAvatarKey())
			{
				plCameraMsg* pCam = TRACKED_NEW plCameraMsg;
				pCam->SetBCastFlag(plMessage::kBCastByExactType);
				pCam->SetCmd(plCameraMsg::kResetPanning);
				pCam->Send();
			
				plCameraMsg* pCam2 = TRACKED_NEW plCameraMsg;
				pCam2->SetBCastFlag(plMessage::kBCastByExactType);
				pCam2->SetCmd(plCameraMsg::kPythonSetFirstPersonOverrideEnable);
				pCam2->SetActivated(false);
				pCam2->Send();	
	
				plCameraMsg* pCam3 = TRACKED_NEW plCameraMsg;
				pCam3->SetBCastFlag(plMessage::kBCastByExactType);
					pCam3->SetCmd(plCameraMsg::kPythonUndoFirstPerson);
				pCam3->Send();
			}
			fTriggeredAvatarKey = avModKey;

			if (fMiscFlags & kDisableForward)
			{
				if (fTriggeredAvatarKey == plAvatarMgr::GetInstance()->GetLocalAvatarKey())
					plAvatarInputInterface::GetInstance()->EnableControl(false, B_CONTROL_MOVE_FORWARD);
			}
		}
	}
}

// IIsClosestAnim -------------------------------------------------------------------
// ---------------
bool IIsClosestAnim(const char *animName, hsMatrix44 &sitGoal, hsScalar &closestDist,
					hsPoint3 curPosition, const plArmatureMod *avatar)
{
	plAGAnim *anim = avatar->FindCustomAnim(animName);
	if(anim)
	{
		hsMatrix44 animEndToStart;
		// The sit target is the position we want to be at the END of the sit animation.
		// We have several animations to choose from, each starting from a different
		// position.
		// This will look at one of those animations and figure out how far we are from
		// its starting position.
		// The first step is to get the transform from the end to the beginning of the
		// animation. That's what this next line is doing. It's a bit unintuitive
		// until you look at the parameter definitions.
		GetStartToEndTransform(anim, nil, &animEndToStart, "Handle");
		hsMatrix44 candidateGoal = sitGoal * animEndToStart;
		hsPoint3 distP = candidateGoal.GetTranslate() - curPosition;
		hsVector3 distV(distP.fX, distP.fY, distP.fZ);
		hsScalar dist = distP.Magnitude();
		if(closestDist == 0.0 || dist < closestDist)
		{
			closestDist = dist;
			return true;
		}
	} else {
		char buffy[256];
		sprintf(buffy, "Missing sit animation: %s", animName);
		hsAssert(false, buffy);
	}
	return false;
}

// IBuildSitBrain ---------------------------------------------------------------------
// ----------------
plAvBrainGeneric *plSittingModifier::IBuildSitBrain(plKey avModKey, plKey seekKey,
					char **pAnimName, plNotifyMsg *enterNotify, plNotifyMsg *exitNotify)
{
	plArmatureMod *avatar = plArmatureMod::ConvertNoRef(avModKey->ObjectIsLoaded());
	plSceneObject *seekObj = plSceneObject::ConvertNoRef(seekKey->ObjectIsLoaded());
	hsMatrix44 animEndToStart;
	hsMatrix44 sitGoal = seekObj->GetLocalToWorld();
	hsMatrix44 candidateGoal;
	hsScalar closestDist = 0.0f;
	UInt8 closestApproach = 0;
	hsPoint3 curPosition = avatar->GetTarget(0)->GetLocalToWorld().GetTranslate();
	char * sitAnimName = nil;
	char * standAnimName = "StandUpFront";		// always prefer to stand facing front

	bool frontClear = fMiscFlags & kApproachFront;
	plAvBrainGeneric *brain = nil;

	if(avatar)
	{
		if(fMiscFlags & kApproachLeft && IIsClosestAnim("SitLeft", sitGoal, closestDist, curPosition, avatar))
		{
			closestApproach = kApproachLeft;
			sitAnimName = "SitLeft";
			if(!frontClear)
				standAnimName = "StandUpLeft";
		}
		if(fMiscFlags & kApproachRight && IIsClosestAnim("SitRight", sitGoal, closestDist, curPosition, avatar))
		{
			closestApproach = kApproachRight;
			sitAnimName = "SitRight";
			if(!frontClear)
				standAnimName = "StandUpRight";
		}
		if(frontClear && IIsClosestAnim("SitFront", sitGoal, closestDist, curPosition, avatar))
		{
			sitAnimName = "SitFront";
			standAnimName = "StandUpFront";
			closestApproach = kApproachFront;
		}

		if(sitAnimName)
		{
			UInt32 exitFlags = plAvBrainGeneric::kExitNormal;	// SOME stages can be interrupted, but not the brain itself
			brain = TRACKED_NEW plAvBrainGeneric(nil, enterNotify, exitNotify, nil, exitFlags, plAvBrainGeneric::kDefaultFadeIn, 
										 plAvBrainGeneric::kDefaultFadeOut, plAvBrainGeneric::kMoveRelative);

			plAnimStage *sitStage = TRACKED_NEW plAnimStage(sitAnimName, 0, plAnimStage::kForwardAuto, plAnimStage::kBackNone,
													plAnimStage::kAdvanceAuto, plAnimStage::kRegressNone, 0);
			plAnimStage *idleStage = TRACKED_NEW plAnimStage("SitIdle", plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone,
													plAnimStage::kAdvanceOnMove, plAnimStage::kRegressNone, -1);
			plAnimStage *standStage = TRACKED_NEW plAnimStage(standAnimName, 0, plAnimStage::kForwardAuto, plAnimStage::kBackNone,
													plAnimStage::kAdvanceAuto, plAnimStage::kRegressNone, 0);

			brain->AddStage(sitStage);
			brain->AddStage(idleStage);
			brain->AddStage(standStage);
			*pAnimName = sitAnimName;

			brain->SetType(plAvBrainGeneric::kSit);
			brain->SetRecipient(GetKey());
		}
	}
	return brain;
}

// UnTrigger ----------------------
// ----------
void plSittingModifier::UnTrigger()
{
	if (fTriggeredAvatarKey == plAvatarMgr::GetInstance()->GetLocalAvatarKey())
	{
		plCameraMsg* pCam = TRACKED_NEW plCameraMsg;
		pCam->SetBCastFlag(plMessage::kBCastByExactType);
		pCam->SetCmd(plCameraMsg::kResetPanning);
		pCam->Send();

		plCameraMsg* pCam2 = TRACKED_NEW plCameraMsg;
		pCam2->SetBCastFlag(plMessage::kBCastByExactType);
		pCam2->SetCmd(plCameraMsg::kPythonSetFirstPersonOverrideEnable);
		pCam2->SetActivated(true);
		pCam2->Send();	

		// The flag means we disabled it, so re-enable on UnTrigger.
		if (fMiscFlags & kDisableForward)
			plAvatarInputInterface::GetInstance()->EnableControl(true, B_CONTROL_MOVE_FORWARD);
	}
	
	fTriggeredAvatarKey = nil;
}
