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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

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

#include "plTimerCallbackManager.h"

// local
#include "plArmatureMod.h"
#include "plAvatarMgr.h"
#include "plAvBrainCoop.h"
#include "plAvTaskBrain.h"
#include "plAvTaskSeek.h"

// other
#include "pnMessage/plNotifyMsg.h"
#include "pnNetCommon/plNetApp.h"
#include "pnSceneObject/plSceneObject.h"

#include "plMessage/plAvatarMsg.h"
#include "plMessage/plAvCoopMsg.h"
#include "plMessage/plInputIfaceMgrMsg.h"
#include "plMessage/plTimerCallbackMsg.h"
#include "plStatusLog/plStatusLog.h"

const int kAbortTimer = 1;
const float kAbortTimerDuration = 15; // 15 seconds

/////////////////////////////////////////////////////////////////////////////////////////
//
// CONSTRUCTORS
//
/////////////////////////////////////////////////////////////////////////////////////////

plCoopCoordinator::plCoopCoordinator()
: fHostBrain(),
  fGuestBrain(),
  fInitiatorID(),
  fInitiatorSerial(),
  fHostOfferStage(),
  fGuestAcceptStage(),
  fGuestAcceptMsg(),
  fAutoStartGuest(),
  fGuestAccepted()
{
}

// plCoopCoordinator ----------------------------------------
// ------------------
plCoopCoordinator::plCoopCoordinator(plKey host, plKey guest,
                                     plAvBrainCoop *hostBrain, plAvBrainCoop *guestBrain,
                                     const ST::string &synchBone,
                                     uint32_t hostOfferStage, uint32_t guestAcceptStage,
                                     plMessage *guestAcceptMsg,
                                     bool autoStartGuest)
: fHostKey(std::move(host)),
  fGuestKey(std::move(guest)),
  fHostBrain(hostBrain),
  fGuestBrain(guestBrain),
  fInitiatorID(hostBrain->GetInitiatorID()),
  fInitiatorSerial(hostBrain->GetInitiatorSerial()),
  fHostOfferStage(hostOfferStage),
  fGuestAcceptStage(guestAcceptStage),
  fGuestAcceptMsg(guestAcceptMsg),
  fAutoStartGuest(autoStartGuest),
  fSynchBone(synchBone),
  fGuestAccepted(false),
  fGuestLinked(false)
{
    static int serial = 0;

    serial = serial % 999;

    ST::string newName = ST::format("{}{}{3}\x000", fHostKey->GetName(), fGuestKey->GetName(), serial++);
    
    plKey newKey = hsgResMgr::ResMgr()->NewKey(newName, this, fHostKey->GetUoid().GetLocation());

    plKey avMgrKey = plAvatarMgr::GetInstance()->GetKey();

    fGuestBrain->SetRecipient(avMgrKey);
    fHostBrain->SetRecipient(avMgrKey);
    // disable our clickability here if we are the guest
    if (plNetClientApp::GetInstance()->GetLocalPlayerKey() == fGuestKey)
    {
        plInputIfaceMgrMsg* pMsg = new plInputIfaceMgrMsg(plInputIfaceMgrMsg::kGUIDisableAvatarClickable);
        pMsg->SetAvKey(fGuestKey);
        pMsg->SetBCastFlag(plMessage::kNetPropagate);
        pMsg->SetBCastFlag(plMessage::kNetForce);
        pMsg->Send();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// CONSTRUCTORS
//
/////////////////////////////////////////////////////////////////////////////////////////

// MsgReceive --------------------------------------
// -----------
bool plCoopCoordinator::MsgReceive(plMessage *msg)
{
    plNotifyMsg *notify = plNotifyMsg::ConvertNoRef(msg);
    if(notify)
    {
        proMultiStageEventData * mtevt = static_cast<proMultiStageEventData *>(notify->FindEventRecord(proEventData::kMultiStage));
        if(mtevt)
        {
            int stageNum = mtevt->fStage;
            uint32_t stageState = mtevt->fEvent;

            plKey noteSender = notify->GetSender();
            bool isFromHost = (noteSender == fHostKey);
            bool isFromGuest = (noteSender == fGuestKey);

            plAvatarMgr::GetInstance()->GetLog()->AddLineF("COOP: Received multi-stage callback - stageNum = {}, stageState = {}, isFromHost = {}", stageNum, stageState, isFromHost);

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
                    fGuestAcceptMsg = nullptr;
                    fGuestLinked = true;
                    IAdvanceParticipant(true);  // advance the host
//                  IAdvanceParticipant(false); // advance the guest
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
        plAvatarMgr::GetInstance()->GetLog()->AddLineF("COOP: Received coop message: {}", coop->fCommand);
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
                fGuestAcceptMsg = nullptr;
                fGuestLinked = true;
                IAdvanceParticipant(true);  // advance the host
                break;
            default:
                break;
        }
    }
    
    plAvTaskSeekDoneMsg *seekDone = plAvTaskSeekDoneMsg::ConvertNoRef(msg);
    if (seekDone)
    {
        plAvatarMgr::GetInstance()->GetLog()->AddLineF("COOP: Received avatar seek finished msg: aborted = {}", seekDone->fAborted);
        if ( seekDone->fAborted )
        {
            plAvCoopMsg *coopM = new plAvCoopMsg(plAvCoopMsg::kGuestSeekAbort,fInitiatorID,(uint16_t)fInitiatorSerial);
            coopM->SetBCastFlag(plMessage::kNetPropagate);
            coopM->SetBCastFlag(plMessage::kNetForce);
            coopM->AddReceiver(GetKey());
            coopM->Send();

        }
        else
        {
            plAvCoopMsg *coopM = new plAvCoopMsg(plAvCoopMsg::kGuestSeeked,fInitiatorID,(uint16_t)fInitiatorSerial);
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
uint32_t plCoopCoordinator::GetInitiatorID()
{
    return fInitiatorID;
}

// GetInitiatorSerial ------------------------
uint16_t plCoopCoordinator::GetInitiatorSerial()
{
    return (uint16_t)fInitiatorSerial;
}

// IStartHost ----------------------
// -----------
void plCoopCoordinator::IStartHost()
{
    plAvatarMgr::GetInstance()->GetLog()->AddLine("COOP: IStartHost()");
    plArmatureMod *guestAv = plAvatarMgr::FindAvatar(fGuestKey);
    plArmatureMod *hostAv = plAvatarMgr::FindAvatar(fHostKey);
    if (guestAv && hostAv)
    {
        plAvSeekMsg *msg = new plAvSeekMsg(nullptr, hostAv->GetKey(), nullptr, 1.f, true);
        hsClearBits(msg->fFlags, plAvSeekMsg::kSeekFlagForce3rdPersonOnStart);
        guestAv->GetPositionAndRotationSim(&msg->fTargetLookAt, nullptr);
        hostAv->GetPositionAndRotationSim(&msg->fTargetPos, nullptr);
        msg->Send();
    }   

    // now tell the host to initiate the thing.
    plAvTaskBrain *brainT = new plAvTaskBrain(fHostBrain);
    plAvTaskMsg *brainM = new plAvTaskMsg(GetKey(), fHostKey, brainT);
    brainM->SetBCastFlag(plMessage::kPropagateToModifiers);
    brainM->Send();
}

// IStartGuest ----------------------
// ------------
void plCoopCoordinator::IStartGuest()
{
    plAvatarMgr::GetInstance()->GetLog()->AddLine("COOP: IStartGuest()");
    plSceneObject *avSO = plSceneObject::ConvertNoRef(fHostKey->ObjectIsLoaded());
    if ( !avSO )
        return;

    const plArmatureMod *hostAv = (plArmatureMod*)avSO->GetModifierByType(plArmatureMod::Index());
    if ( hostAv )
    {
        const plSceneObject *targetBone = hostAv->FindBone(fSynchBone);
        if(targetBone)
        {
            plAvSeekMsg *seekMsg = new plAvSeekMsg(GetKey(), fGuestKey, targetBone->GetKey(), 0, true, kAlignHandle, "", false, plAvSeekMsg::kSeekFlagNoWarpOnTimeout, GetKey());
            seekMsg->SetBCastFlag(plMessage::kPropagateToModifiers);
            seekMsg->Send();
        }
    }
}

// IContinueGuest ----------------------
// ------------
void plCoopCoordinator::IContinueGuest()
{
    plAvatarMgr::GetInstance()->GetLog()->AddLine("COOP: IContinueGuest()");
    plAvTaskBrain *brainT = new plAvTaskBrain(fGuestBrain);
    plAvTaskMsg *brainM = new plAvTaskMsg(GetKey(), fGuestKey, brainT);
    brainM->SetBCastFlag(plMessage::kPropagateToModifiers);
    brainM->Send();
    fGuestBrain = nullptr;          // the armature will destroy the brain when done.
}

// IContinueHost ----------------------
// --------------
void plCoopCoordinator::IAdvanceParticipant(bool host)
{
    plAvatarMgr::GetInstance()->GetLog()->AddLineF("COOP: IAdvanceParticipant({})", host);
    plKey &who = host ? fHostKey : fGuestKey;

    plAvBrainGenericMsg* pMsg = new plAvBrainGenericMsg(nullptr, who,
        plAvBrainGenericMsg::kNextStage, 0, false, 0.0,
        false, false, 0.0);

    pMsg->SetBCastFlag(plMessage::kPropagateToModifiers);

    pMsg->Send();
}

// IStartTimeout ----------------------
// --------------
void plCoopCoordinator::IStartTimeout()
{
    plTimerCallbackMsg* timerMsg = new plTimerCallbackMsg(GetKey(), kAbortTimer);
    plgTimerCallbackMgr::NewTimer(kAbortTimerDuration, timerMsg);
}

// ITimeout ---------------------------
// --------------
void plCoopCoordinator::ITimeout()
{
    fGuestAcceptMsg = nullptr;
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

    if(stream->ReadBool())
        fGuestAcceptMsg = plMessage::ConvertNoRef(mgr->ReadCreatable(stream));
    else
        fGuestAcceptMsg = nullptr;

    fSynchBone = stream->ReadSafeString();
    fAutoStartGuest = stream->ReadBool();
    
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

    stream->WriteByte((uint8_t)fHostOfferStage);
    stream->WriteByte((uint8_t)fGuestAcceptStage);

    stream->WriteBool(fGuestAcceptMsg != nullptr);
    if(fGuestAcceptMsg)
        mgr->WriteCreatable(stream, fGuestAcceptMsg);

    stream->WriteSafeString(fSynchBone);
    stream->WriteBool(fAutoStartGuest);
}

