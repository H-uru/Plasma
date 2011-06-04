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
#include "plCoopCoordinator.h"

// local
#include "plAvBrainCoop.h"
#include "plAvatarMgr.h"
#include "plAvTaskBrain.h"
#include "plAvTaskSeek.h"

// global
#include "hsUtils.h"

// other
#include "../plMessage/plAvCoopMsg.h"
#include "../plMessage/plAvatarMsg.h"
#include "../plMessage/plInputIfaceMgrMsg.h"
#include "../pnMessage/plNotifyMsg.h"
#include "../pnNetCommon/plNetApp.h"
#include "../plNetClient/plNetClientMgr.h"
#include "plPhysical.h"
#include "../pnTimer/plTimerCallbackManager.h"
#include "../plMessage/plTimerCallbackMsg.h"

const unsigned kAbortTimer = 1;
const float kAbortTimerDuration = 15; // 15 seconds

/////////////////////////////////////////////////////////////////////////////////////////
//
// CONSTRUCTORS
//
/////////////////////////////////////////////////////////////////////////////////////////

plCoopCoordinator::plCoopCoordinator()
: fHostBrain(nil),
  fGuestBrain(nil),
  fInitiatorID(0),
  fInitiatorSerial(0),
  fHostOfferStage(0),
  fGuestAcceptStage(0),
  fGuestAcceptMsg(nil),
  fAutoStartGuest(nil),
  fGuestAccepted(false)
{
}

// plCoopCoordinator ----------------------------------------
// ------------------
plCoopCoordinator::plCoopCoordinator(plKey host, plKey guest,
									 plAvBrainCoop *hostBrain, plAvBrainCoop *guestBrain,
									 const char *synchBone,
									 UInt32 hostOfferStage, UInt32 guestAcceptStage,
									 plMessage *guestAcceptMsg,
									 bool autoStartGuest)
: fHostKey(host),
  fGuestKey(guest),
  fHostBrain(hostBrain),
  fGuestBrain(guestBrain),
  fInitiatorID(hostBrain->GetInitiatorID()),
  fInitiatorSerial(hostBrain->GetInitiatorSerial()),
  fHostOfferStage(hostOfferStage),
  fGuestAcceptStage(guestAcceptStage),
  fGuestAcceptMsg(guestAcceptMsg),
  fAutoStartGuest(autoStartGuest),
  fGuestAccepted(false),
  fGuestLinked(false)
{
	const char * hostName = host->GetName();
	const char * guestName = guest->GetName();
	static int serial = 0;

	int len = strlen(hostName) + strlen(guestName) + 3 /* serial num */ + 1;

	char *newName = TRACKED_NEW char[len];

	serial = serial % 999;

	sprintf(newName, "%s%s%3i\x000", hostName, guestName, serial++);
	
	plKey newKey = hsgResMgr::ResMgr()->NewKey(newName, this, host->GetUoid().GetLocation());

	delete[] newName;

	fSynchBone = hsStrcpy(synchBone);

	plKey avMgrKey = plAvatarMgr::GetInstance()->GetKey();

	guestBrain->SetRecipient(avMgrKey);
	hostBrain->SetRecipient(avMgrKey);
	// disable our clickability here if we are the guest
	if (plNetClientMgr::GetInstance()->GetLocalPlayerKey() == guest)
	{
		plInputIfaceMgrMsg* pMsg = TRACKED_NEW plInputIfaceMgrMsg(plInputIfaceMgrMsg::kGUIDisableAvatarClickable);
		pMsg->SetAvKey(guest);
		pMsg->SetBCastFlag(plMessage::kNetPropagate);
		pMsg->SetBCastFlag(plMessage::kNetForce);
		pMsg->Send();
	}
}

// plCoopCoordinator ------------------
// ------------------
plCoopCoordinator::~plCoopCoordinator()
{
	delete[] fSynchBone;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// CONSTRUCTORS
//
/////////////////////////////////////////////////////////////////////////////////////////

// MsgReceive --------------------------------------
// -----------
hsBool plCoopCoordinator::MsgReceive(plMessage *msg)
{
	plNotifyMsg *notify = plNotifyMsg::ConvertNoRef(msg);
	if(notify)
	{
		proMultiStageEventData * mtevt = static_cast<proMultiStageEventData *>(notify->FindEventRecord(proEventData::kMultiStage));
		if(mtevt)
		{
			int stageNum = mtevt->fStage;
			UInt32 stageState = mtevt->fEvent;

			plKey noteSender = notify->GetSender();
			bool isFromHost = (noteSender == fHostKey);
			bool isFromGuest = (noteSender == fGuestKey);

			DebugMsg("COOP: Received multi-stage callback - stageNum = %d, stageState = %d, isFromHost = %d", stageNum, stageState, isFromHost ? 1 : 0);

			if(isFromHost)
			{
				if(!fGuestAccepted)
				{
					// we've just entered the host offer stage (i.e., the offer is ready)
					if(stageNum == fHostOfferStage && stageState == proEventData::kEnterStage)
					{
						if(fAutoStartGuest)
						{
							IStartGuest();
							IStartTimeout();
						} else {
							fHostBrain->EnableGuestClick();
						}
						fGuestAccepted = true;
					}
				}

			} else if(isFromGuest)
			{
				if(stageNum == fGuestAcceptStage && stageState == proEventData::kEnterStage)
				{
					plKey localPlayer = plNetClientApp::GetInstance()->GetLocalPlayerKey();

					// we only actually fire off the guest accept message if we're on the guest machine.
					// if it needs to be netpropped, the client can set that up when they set up the coop.
					if(fGuestAcceptMsg && localPlayer == fGuestKey)
					{
						fGuestAcceptMsg->Send();
					}
					// kill the message (along with being active)
					fGuestAcceptMsg = nil;
					fGuestLinked = true;
					IAdvanceParticipant(true);	// advance the host
//					IAdvanceParticipant(false);	// advance the guest
				}
			} else {
				// not from host; not from guest
				// let's assume for the moment it's from a trigger.
				IStartHost();
			}
		}
	}

	plAvCoopMsg *coop = plAvCoopMsg::ConvertNoRef(msg);
	if(coop)
	{
		DebugMsg("COOP: Received coop message: %d", coop->fCommand);
		switch(coop->fCommand)
		{
			case plAvCoopMsg::kGuestAccepted:
				IStartGuest();
				IStartTimeout();
				break;

			case plAvCoopMsg::kGuestSeeked:
				// if they did make it to their target, then continue
				IContinueGuest();
				break;

			case plAvCoopMsg::kGuestSeekAbort:
				// if they aborted then just advance the host
				// kill the message (along with being active)
				fGuestAcceptMsg = nil;
				fGuestLinked = true;
				IAdvanceParticipant(true);	// advance the host
				break;

		}
	}
	
	plAvTaskSeekDoneMsg *seekDone = plAvTaskSeekDoneMsg::ConvertNoRef(msg);
	if (seekDone)
	{
		DebugMsg("COOP: Received avatar seek finished msg: aborted = %d", seekDone->fAborted ? 1 : 0);
		if ( seekDone->fAborted )
		{
			plAvCoopMsg *coopM = TRACKED_NEW plAvCoopMsg(plAvCoopMsg::kGuestSeekAbort,fInitiatorID,(UInt16)fInitiatorSerial);
			coopM->SetBCastFlag(plMessage::kNetPropagate);
			coopM->SetBCastFlag(plMessage::kNetForce);
			coopM->AddReceiver(GetKey());
			coopM->Send();

		}
		else
		{
			plAvCoopMsg *coopM = TRACKED_NEW plAvCoopMsg(plAvCoopMsg::kGuestSeeked,fInitiatorID,(UInt16)fInitiatorSerial);
			coopM->SetBCastFlag(plMessage::kNetPropagate);
			coopM->SetBCastFlag(plMessage::kNetForce);
			coopM->AddReceiver(GetKey());
			coopM->Send();
		}
	}

	plTimerCallbackMsg* timerMsg = plTimerCallbackMsg::ConvertNoRef(msg);
	if (timerMsg)
	{
		if (timerMsg->fID == kAbortTimer && !fGuestLinked)
			ITimeout();
	}
	return false;
}

// Run ----------------------
// ----
void plCoopCoordinator::Run()
{
	IStartHost();
}

bool plCoopCoordinator::IsActiveForReal()
{
	return fGuestAcceptMsg ? true : false;
}

// GetInitiatorID ------------------------
// ---------------
UInt32 plCoopCoordinator::GetInitiatorID()
{
	return fInitiatorID;
}

// GetInitiatorSerial ------------------------
UInt16 plCoopCoordinator::GetInitiatorSerial()
{
	return (UInt16)fInitiatorSerial;
}

// IStartHost ----------------------
// -----------
void plCoopCoordinator::IStartHost()
{
	DebugMsg("COOP: IStartHost()");
	plArmatureMod *guestAv = plAvatarMgr::FindAvatar(fGuestKey);
	plArmatureMod *hostAv = plAvatarMgr::FindAvatar(fHostKey);
	if (guestAv && hostAv)
	{
		plAvSeekMsg *msg = TRACKED_NEW plAvSeekMsg(nil, hostAv->GetKey(), nil, 1.f, true);
		hsClearBits(msg->fFlags, plAvSeekMsg::kSeekFlagForce3rdPersonOnStart);
		guestAv->GetPositionAndRotationSim(&msg->fTargetLookAt, nil);
		hostAv->GetPositionAndRotationSim(&msg->fTargetPos, nil);
		msg->Send();
	}	

	// now tell the host to initiate the thing.
	plAvTaskBrain *brainT = TRACKED_NEW plAvTaskBrain(fHostBrain);
	plAvTaskMsg *brainM = TRACKED_NEW plAvTaskMsg(GetKey(), fHostKey, brainT);
	brainM->SetBCastFlag(plMessage::kPropagateToModifiers);
	brainM->Send();
}

// IStartGuest ----------------------
// ------------
void plCoopCoordinator::IStartGuest()
{
	DebugMsg("COOP: IStartGuest()");
	plSceneObject *avSO = plSceneObject::ConvertNoRef(fHostKey->ObjectIsLoaded());
	if ( !avSO )
		return;

	const plArmatureMod *hostAv = (plArmatureMod*)avSO->GetModifierByType(plArmatureMod::Index());
	if ( hostAv )
	{
		const plSceneObject *targetBone = hostAv->FindBone(fSynchBone);
		if(targetBone)
		{
			plAvSeekMsg *seekMsg = TRACKED_NEW plAvSeekMsg( nil, nil,targetBone->GetKey(), 0, true, kAlignHandle, nil, false, plAvSeekMsg::kSeekFlagNoWarpOnTimeout, GetKey());
			plAvTaskSeek *seekT = TRACKED_NEW plAvTaskSeek(seekMsg);
			plAvTaskMsg *seekM = TRACKED_NEW plAvTaskMsg(GetKey(), fGuestKey, seekT);
			seekM->SetBCastFlag(plMessage::kPropagateToModifiers);
			seekM->Send();
		}
	}
}

// IContinueGuest ----------------------
// ------------
void plCoopCoordinator::IContinueGuest()
{
	DebugMsg("COOP: IContinueGuest()");
	plAvTaskBrain *brainT = TRACKED_NEW plAvTaskBrain(fGuestBrain);
	plAvTaskMsg *brainM = TRACKED_NEW plAvTaskMsg(GetKey(), fGuestKey, brainT);
	brainM->SetBCastFlag(plMessage::kPropagateToModifiers);
	brainM->Send();
	fGuestBrain = nil;			// the armature will destroy the brain when done.
}

// IContinueHost ----------------------
// --------------
void plCoopCoordinator::IAdvanceParticipant(bool host)
{
	DebugMsg("COOP: IAdvanceParticipant(%d)", host ? 1 : 0);
	plKey &who = host ? fHostKey : fGuestKey;

	plAvBrainGenericMsg* pMsg = TRACKED_NEW plAvBrainGenericMsg(nil, who,
		plAvBrainGenericMsg::kNextStage, 0, false, 0.0,
		false, false, 0.0);

	pMsg->SetBCastFlag(plMessage::kPropagateToModifiers);

	pMsg->Send();
}

// IStartTimeout ----------------------
// --------------
void plCoopCoordinator::IStartTimeout()
{
	plTimerCallbackMsg* timerMsg = TRACKED_NEW plTimerCallbackMsg(GetKey(), kAbortTimer);
	plgTimerCallbackMgr::NewTimer(kAbortTimerDuration, timerMsg);
}

// ITimeout ---------------------------
// --------------
void plCoopCoordinator::ITimeout()
{
	fGuestAcceptMsg = nil;
	IAdvanceParticipant(true); // advance the host
}

// Read -------------------------------------------------------------
// -----
void plCoopCoordinator::Read(hsStream *stream, hsResMgr *mgr)
{
	fHostKey = mgr->ReadKey(stream);
	fGuestKey = mgr->ReadKey(stream);

	fHostBrain = plAvBrainCoop::ConvertNoRef(mgr->ReadCreatable(stream));
	fGuestBrain = plAvBrainCoop::ConvertNoRef(mgr->ReadCreatable(stream));

	fHostOfferStage = stream->ReadByte();
	fGuestAcceptStage = stream->ReadBool();

	if(stream->Readbool())
		fGuestAcceptMsg = plMessage::ConvertNoRef(mgr->ReadCreatable(stream));
	else
		fGuestAcceptMsg = nil;

	fSynchBone = stream->ReadSafeString();
	fAutoStartGuest = stream->Readbool();
	
	fInitiatorID = fHostBrain->GetInitiatorID();
	fInitiatorSerial = fHostBrain->GetInitiatorSerial();
}

// Write -------------------------------------------------------------
// ------
void plCoopCoordinator::Write(hsStream *stream, hsResMgr *mgr)
{
	mgr->WriteKey(stream, fHostKey);
	mgr->WriteKey(stream, fGuestKey);

	mgr->WriteCreatable(stream, fHostBrain);
	mgr->WriteCreatable(stream, fGuestBrain);

	stream->WriteByte((UInt8)fHostOfferStage);
	stream->WriteByte((UInt8)fGuestAcceptStage);

	stream->Writebool(fGuestAcceptMsg != nil);
	if(fGuestAcceptMsg)
		mgr->WriteCreatable(stream, fGuestAcceptMsg);

	stream->WriteSafeString(fSynchBone);
	stream->Writebool(fAutoStartGuest);
}

