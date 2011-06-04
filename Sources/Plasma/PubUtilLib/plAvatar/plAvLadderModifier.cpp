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
#include "../plAvatar/plAvCallbackAction.h"

#include "hsTypes.h"

// singular
#include "plAvLadderModifier.h"

// local
#include "plArmatureMod.h"
#include "plAvatarMgr.h"
#include "plAvBrainGeneric.h"
#include "plAGAnim.h"
#include "plAnimStage.h"

// global
#include "plCreatableIndex.h"
// #include "plgDispatch.h"						// Message Dependencies
#include "hsStream.h"

//other
#include "../plMessage/plCollideMsg.h"
#include "../plMessage/plAvatarMsg.h"
#include "../pnMessage/plNotifyMsg.h"
#include "../plStatusLog/plStatusLog.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnMessage/plEnableMsg.h"

#include "../pnMessage/plTimeMsg.h"
#include "plgDispatch.h"
#include "../pnNetCommon/plNetApp.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../plAvatar/plAvBrainHuman.h"
#include "../plModifier/plDetectorLog.h"

enum NotifyType
{
	kNotifyTrigger,
	kNotifyAvatarOnLadder,
};

// CTOR default
plAvLadderMod::plAvLadderMod()
: fGoingUp(true),
  fType(kBig),
  fLoops(0),
  fEnabled(true),
  fAvatarInBox(false),
  fLadderView(0,0,0),
  fAvatarMounting(false)
{
	fTarget = nil;
}

// CTOR goingUp, type, loops
plAvLadderMod::plAvLadderMod(bool goingUp, int type, int loops, bool enabled, hsVector3& ladderView)
: fGoingUp(goingUp),
  fType(type),
  fLoops(loops),
  fEnabled(enabled),
  fAvatarInBox(false),
  fLadderView(ladderView)
{
	fTarget = nil;
}

// Must be facing within 45 degrees of the ladder
static const hsScalar kTolerance = hsCosine(hsScalarDegToRad(45));

bool plAvLadderMod::IIsReadyToClimb()
{
	if (fAvatarMounting)
		return false;

	plArmatureMod* armMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
	plSceneObject* avatar = armMod->GetTarget(0);
	if (avatar)
	{
		hsVector3 playerView = avatar->GetCoordinateInterface()->GetLocalToWorld().GetAxis(hsMatrix44::kView);
		playerView.fZ = 0;

		// Are we facing towards the ladder?
		hsScalar dot = playerView * fLadderView;

		bool movingForward = false;

		// And are we walking towards it?
		hsAssert(armMod, "Avatar doesn't have an armature mod");
		if (armMod)
		{
			plAvBrainHuman* brain = plAvBrainHuman::ConvertNoRef(armMod->GetCurrentBrain());
			if (brain && brain->IsMovingForward() && brain->fCallbackAction->IsOnGround())
				movingForward = true;
		}

		if (dot >= kTolerance && movingForward)
		{
			DetectorLogSpecial("%s: Ladder starting climb (%f)", GetKeyName(), hsScalarRadToDeg(hsACosine(dot)));
			return true;
		}
		else if (movingForward)
		{
//			DetectorLog("%s: Ladder rejecting climb (%f)", GetKeyName(), hsScalarRadToDeg(hsACosine(dot)));
			return false;
		}
	}

	return false;
}

// use a plNotify (to ourself) to propagate across the network
void plAvLadderMod::ITriggerSelf(plKey avKey)
{
	if (fEnabled)
	{
		plKey avPhysKey = avKey;
		// I'm going to lie and pretend it's from the avatar. the alternative is lengthy and unreadable.
		plNotifyMsg *notifyMsg = TRACKED_NEW plNotifyMsg(avPhysKey, GetKey());
		notifyMsg->fID = kNotifyTrigger;
		notifyMsg->Send();
		fAvatarMounting = true;
	}
}

// MSGRECEIVE
hsBool plAvLadderMod::MsgReceive(plMessage* msg)
{
	// Avatar is entering or exiting our detector box
	plCollideMsg* collMsg = plCollideMsg::ConvertNoRef(msg);
	if (collMsg)
	{
		// make sure this is the local player... the notify will be the thing that propagates over the network
		if (plNetClientApp::GetInstance()->GetLocalPlayerKey() != collMsg->fOtherKey)
			return true;

		fAvatarInBox = (collMsg->fEntering != 0);

		// If entering, check if ready to climb.  If not, register for eval so
		// we can check every frame
		if (fAvatarInBox)
		{
			DetectorLogSpecial("%s: Avatar entered ladder region", GetKeyName());

			if (IIsReadyToClimb())
				ITriggerSelf(collMsg->fOtherKey);
			else
				plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
		}
		else
		{
			DetectorLogSpecial("%s: Avatar exited ladder region", GetKeyName());

			plgDispatch::Dispatch()->UnRegisterForExactType(plEvalMsg::Index(), GetKey());
		}

		return true;
	}

	// Avatar is inside our detector box, so every frame we check if he's ready to climb 
	plEvalMsg* evalMsg = plEvalMsg::ConvertNoRef(msg);
	if (evalMsg)
	{
		if (IIsReadyToClimb())
			ITriggerSelf(plNetClientApp::GetInstance()->GetLocalPlayerKey());
		return true;
	}

	plNotifyMsg *notifyMsg = plNotifyMsg::ConvertNoRef(msg);
	if (notifyMsg)
	{
		if (notifyMsg->fID == kNotifyTrigger && fEnabled)
		{
			const plKey avPhysKey = notifyMsg->GetSender();
			EmitCommand(avPhysKey);
		}
		else if (notifyMsg->fID == kNotifyAvatarOnLadder)
		{
			DetectorLogSpecial("%s: Avatar mounted ladder", GetKeyName());
			fAvatarMounting = false;
		}

		return true;
	}

	plEnableMsg *enableMsg = plEnableMsg::ConvertNoRef(msg);
	if (enableMsg)
	{
		if (enableMsg->Cmd(plEnableMsg::kDisable))
			fEnabled = false;
		else if (enableMsg->Cmd(plEnableMsg::kEnable))
			fEnabled = true;
	}

	return plSingleModifier::MsgReceive(msg);
}

// EMITCOMMAND
void plAvLadderMod::EmitCommand(const plKey receiver)
{
	hsKeyedObject *object = receiver->ObjectIsLoaded();
	plSceneObject *SO = plSceneObject::ConvertNoRef(object);
	if(SO)
	{
		const plArmatureMod *constAvMod = (plArmatureMod*)SO->GetModifierByType(plArmatureMod::Index());
		if(constAvMod)
		{
			plAvBrainGeneric *curGenBrain = (plAvBrainGeneric *)constAvMod->FindBrainByClass(plAvBrainGeneric::Index());
			// *** warning; if there's more than one generic brain active, this will only look at the first
			bool alreadyOnLadder =  ( curGenBrain  && curGenBrain->GetType() == plAvBrainGeneric::kLadder );

			if( ! alreadyOnLadder)
			{
				plSceneObject *seekObj = GetTarget();
				plKey seekKey = seekObj->GetKey();		// this modifier's target is the seek object

				char *mountName, *dismountName, *traverseName;

				if(fGoingUp)
				{
					mountName = "LadderUpOn";
					dismountName = "LadderUpOff";
					traverseName = "LadderUp";
				} else {
					mountName = "LadderDownOn";
					dismountName = "LadderDownOff";
					traverseName = "LadderDown";
				}

				plAnimStageVec *v = TRACKED_NEW plAnimStageVec;

				plAnimStage *s1 = TRACKED_NEW plAnimStage(mountName,
												  0,
												  plAnimStage::kForwardAuto,
												  plAnimStage::kBackAuto,
												  plAnimStage::kAdvanceAuto,
												  plAnimStage::kRegressAuto,
												  0);
				if( ! fGoingUp)
					s1->SetReverseOnIdle(true);
				v->push_back(s1);
				
				// if loops is zero, we don't need the traverse animation at all.
				if(fLoops)
				{
					plAnimStage *s2 = TRACKED_NEW plAnimStage(traverseName,
													  0,
													  plAnimStage::kForwardKey,
													  plAnimStage::kBackKey,
													  plAnimStage::kAdvanceAuto,
													  plAnimStage::kRegressAuto,
													  fLoops - 1	// first loop is implied; zero loops means
																	// play this anim once, 1 loop means "loop
																	// once after reaching the end."
													  );
					if( ! fGoingUp)
						s2->SetReverseOnIdle(true);
					v->push_back(s2);
				}
				plAnimStage *s3 = TRACKED_NEW plAnimStage(dismountName,
												  0,
												  plAnimStage::kForwardAuto,
												  plAnimStage::kBackAuto,
												  plAnimStage::kAdvanceAuto,
												  plAnimStage::kRegressAuto,
												  0);
				if( ! fGoingUp)
					s3->SetReverseOnIdle(true);
				v->push_back(s3);

				plNotifyMsg* enterNotify = TRACKED_NEW plNotifyMsg(GetKey(), GetKey());
				enterNotify->fID = kNotifyAvatarOnLadder;

				UInt32 exitFlags = plAvBrainGeneric::kExitNormal;

				plAvBrainGeneric *ladBrain = TRACKED_NEW plAvBrainGeneric(v, enterNotify, nil, nil, exitFlags, plAvBrainGeneric::kDefaultFadeIn, 
																  plAvBrainGeneric::kDefaultFadeOut, plAvBrainGeneric::kMoveRelative);
				ladBrain->SetType(plAvBrainGeneric::kLadder);
				ladBrain->SetReverseFBControlsOnRelease(!fGoingUp);

				plKey avKey = constAvMod->GetKey();

				// Very important that we dumb seek here. Otherwise you can run off the edge of a ladder, and seek will be helpless
				// until you hit the ground, at which point you have no hope of successfully seeking.
				plAvSeekMsg *seeker = TRACKED_NEW plAvSeekMsg(nil, avKey, seekKey, 1.0f, false);
				seeker->Send();
				plAvPushBrainMsg *brainer = TRACKED_NEW plAvPushBrainMsg(nil, avKey, ladBrain);
				brainer->Send();
			}
		}
	}
}

void plAvLadderMod::Read(hsStream *stream, hsResMgr *mgr)
{
	plSingleModifier::Read(stream, mgr);
	
	fType = stream->ReadSwap32();
	fLoops = stream->ReadSwap32();
	fGoingUp = stream->Readbool();
	fEnabled = stream->Readbool();
	fLadderView.fX = stream->ReadSwapScalar();
	fLadderView.fY = stream->ReadSwapScalar();
	fLadderView.fZ = stream->ReadSwapScalar();
}

void plAvLadderMod::Write(hsStream *stream, hsResMgr *mgr)
{
	plSingleModifier::Write(stream, mgr);

	stream->WriteSwap32(fType);
	stream->WriteSwap32(fLoops);
	stream->Writebool(fGoingUp);
	stream->WriteBool(fEnabled);
	stream->WriteSwapScalar(fLadderView.fX);
	stream->WriteSwapScalar(fLadderView.fY);
	stream->WriteSwapScalar(fLadderView.fZ);
}

// true is up; false is down
bool plAvLadderMod::GetGoingUp() const
{
	return fGoingUp;
}

void plAvLadderMod::SetGoingUp(bool up)
{
	fGoingUp = up;
}

int plAvLadderMod::GetLoops() const
{
	return fLoops;
}

void plAvLadderMod::SetLoops(int loops)
{
	fLoops = loops;
}

int plAvLadderMod::GetType() const 
{
	return fType;
}

void plAvLadderMod::SetType(int type)
{
	if(type >= kNumOfTypeFields || type < 0)
	{
		hsStatusMessage("Invalid param to plAvLadderMod::SetType: defaulting to kBig");
		fType = kBig;
	} else {
		fType = type;
	}
}
