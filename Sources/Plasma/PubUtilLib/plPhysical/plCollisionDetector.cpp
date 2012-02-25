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
#include "plAvatar/plAvCallbackAction.h"

#include "HeadSpin.h"
#include "plCollisionDetector.h"
#include "plMessage/plCollideMsg.h"
#include "plgDispatch.h"
#include "plMessage/plActivatorMsg.h"
#include "pnMessage/plCameraMsg.h"
#include "pnMessage/plTimeMsg.h"
#include "plMessage/plInputIfaceMgrMsg.h"
#include "pnInputCore/plControlEventCodes.h"
#include "pnNetCommon/plNetApp.h"
#include "pnSceneObject/plSceneObject.h"
#include "pnNetCommon/plNetApp.h"
#include "plNetClient/plNetLinkingMgr.h"

#include "plPhysical.h"

#include "pnMessage/plPlayerPageMsg.h"
#include "plMessage/plSimStateMsg.h"

#include "pnSceneObject/plCoordinateInterface.h"
#include "plAvatar/plArmatureMod.h"
#include "plAvatar/plAvatarMgr.h"
#include "plAvatar/plAvBrainHuman.h"
#include "plAvatar/plAvBrainDrive.h"

#include "plModifier/plDetectorLog.h"

#define USE_PHYSX_MULTIPLE_CAMREGION_ENTER 1
#define USE_PHYSX_COLLISION_FLUTTER_WORKAROUND 1

plArmatureMod* plCollisionDetector::IGetAvatarModifier(plKey key)
{
    if (!key)
        return nil;

    plSceneObject* avObj = plSceneObject::ConvertNoRef(key->ObjectIsLoaded());
    if (avObj)
    {
        // search through its modifiers to see if one of them is an avatar modifier
        plArmatureMod* avMod = nil;
        for (int i = 0; i < avObj->GetNumModifiers(); i++)
        {
            const plModifier* mod = avObj->GetModifier(i);
            // see if it is an avatar mod base class
            avMod = const_cast<plArmatureMod*>(plArmatureMod::ConvertNoRef(mod));
            if (avMod)
                return avMod;
        }
    }

    return nil;
}

bool plCollisionDetector::IIsDisabledAvatar(plKey key)
{
    plArmatureMod* avMod = IGetAvatarModifier(key);
    plArmatureBrain* avBrain = avMod ? avMod->GetCurrentBrain() : nil;
    return (plAvBrainDrive::ConvertNoRef(avBrain) != nil);
}

hsBool plCollisionDetector::MsgReceive(plMessage* msg)
{
    plCollideMsg* pCollMsg = plCollideMsg::ConvertNoRef(msg);

    if (pCollMsg)
    {
        // If the avatar is disabled (flying around), don't trigger
        if (IIsDisabledAvatar(pCollMsg->fOtherKey))
            return false;

        if (fType & kTypeBump) 
        {
            if (!fBumped && !fTriggered)
            {
                for (int i = 0; i < fReceivers.Count(); i++)
                {
                    plActivatorMsg* pMsg = new plActivatorMsg;
                    pMsg->AddReceiver( fReceivers[i] );

                    if (fProxyKey)
                        pMsg->fHiteeObj = fProxyKey;
                    else
                        pMsg->fHiteeObj = GetTarget()->GetKey();
                    pMsg->fHitterObj = pCollMsg->fOtherKey;
                    pMsg->SetSender(GetKey());
                    pMsg->SetTriggerType( plActivatorMsg::kCollideContact );
                    plgDispatch::MsgSend( pMsg );
                }
                fBumped = true;
                fTriggered = true;
                plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
                return true;
            }
            if (fTriggered)
            {
                fBumped = true;
                return true;
            }
            return false;
        }

        for (int i = 0; i < fReceivers.Count(); i++)
        {
            plActivatorMsg* pMsg = new plActivatorMsg;
            pMsg->AddReceiver( fReceivers[i] );
            if (fProxyKey)
                pMsg->fHiteeObj = fProxyKey;
            else
                pMsg->fHiteeObj = GetTarget()->GetKey();
            pMsg->fHitterObj = pCollMsg->fOtherKey;
            pMsg->SetSender(GetKey());
            
            if (fType & kTypeEnter && pCollMsg->fEntering)
            {
                pMsg->SetTriggerType( plActivatorMsg::kCollideEnter );
                plgDispatch::MsgSend( pMsg );
                continue;
            }
            if (fType & kTypeUnEnter && pCollMsg->fEntering)
            {
                pMsg->SetTriggerType( plActivatorMsg::kEnterUnTrigger );
                plgDispatch::MsgSend( pMsg );
                continue;
            }
            if(fType & kTypeExit && !pCollMsg->fEntering)
            {
                pMsg->SetTriggerType( plActivatorMsg::kCollideExit );
                plgDispatch::MsgSend( pMsg );
                continue;
            }
            if(fType & kTypeUnExit && !pCollMsg->fEntering)
            {
                pMsg->SetTriggerType( plActivatorMsg::kExitUnTrigger );
                plgDispatch::MsgSend( pMsg );
                continue;
            }
            if (fType & kTypeAny)
            {
                pMsg->SetTriggerType( plActivatorMsg::kCollideContact );
                plgDispatch::MsgSend( pMsg );
                continue;
            }

            delete (pMsg);
        }
        return true;
    }

    plEvalMsg* pEval = plEvalMsg::ConvertNoRef(msg);
    if (pEval)
    {
        if (!fBumped && fTriggered)
        {
            plgDispatch::Dispatch()->UnRegisterForExactType(plEvalMsg::Index(), GetKey());
            for (int i = 0; i < fReceivers.Count(); i++)
            {
                plActivatorMsg* pMsg = new plActivatorMsg;
                pMsg->AddReceiver( fReceivers[i] );
                if (fProxyKey)
                    pMsg->fHiteeObj = fProxyKey;
                else
                    pMsg->fHiteeObj = GetTarget()->GetKey();
                pMsg->SetSender(GetKey());
                pMsg->SetTriggerType( plActivatorMsg::kCollideUnTrigger );
                plgDispatch::MsgSend( pMsg );
                fTriggered = false;
            }
        }
        else
        if (fTriggered && fBumped)
        {
            fBumped = false;
        }
        return true;
    }

    return plDetectorModifier::MsgReceive(msg);
}

void plCollisionDetector::Read(hsStream* stream, hsResMgr* mgr)
{
    plDetectorModifier::Read(stream, mgr);
    stream->ReadLE(&fType);
}
void plCollisionDetector::Write(hsStream* stream, hsResMgr* mgr)
{
    plDetectorModifier::Write(stream, mgr);
    stream->WriteLE(fType);
}

/////////////////////////////////
/////////////////////////////////
/////////////////////////////////
/////////////////////////////////
// camera region detector

plCameraRegionDetector::~plCameraRegionDetector()
{
    for (plCameraMsgVec::iterator it = fMessages.begin(); it != fMessages.end(); ++it)
        hsRefCnt_SafeUnRef(*it);
}

void plCameraRegionDetector::ITrigger(plKey hitter, bool entering, bool immediate)
{
    if (fIsInside && entering)
        DetectorLogRed("%s: Duplicate enter! Did we miss an exit?", GetKeyName().c_str());
    else if (!fIsInside && !entering)
        DetectorLogRed("%s: Duplicate exit! Did we miss an enter?", GetKeyName().c_str());

    fSavingSendMsg = true;
    fSavedMsgEnterFlag = entering;
    if (entering)
    {
        DetectorLog("%s: Saving camera Entering volume - Evals=%d", GetKeyName().c_str(),fNumEvals);
        fLastEnterEval = fNumEvals;
    }
    else
    {
        DetectorLog("%s: Saving camera Exiting volume - Evals=%d", GetKeyName().c_str(),fNumEvals);
        fLastExitEval = fNumEvals;
    }

    if (immediate)
        ISendSavedTriggerMsgs();
}

void plCameraRegionDetector::ISendSavedTriggerMsgs()
{
    if (fSavingSendMsg)
    {
        for (size_t i = 0; i < fMessages.size(); ++i)
        {   
            hsRefCnt_SafeRef(fMessages[i]);
            if (fSavedMsgEnterFlag)
            {
                fMessages[i]->SetCmd(plCameraMsg::kEntering);
                DetectorLog("Entering cameraRegion: %s - Evals=%d -msg %d of %d\n", GetKeyName().c_str(),fNumEvals,i+1,fMessages.size());
                fIsInside = true;
            }
            else
            {
                fMessages[i]->ClearCmd(plCameraMsg::kEntering);
                DetectorLog("Exiting cameraRegion: %s - Evals=%d -msg %d of %d\n", GetKeyName().c_str(),fNumEvals,i+1,fMessages.size());
                fIsInside = false;
            }
            plgDispatch::MsgSend(fMessages[i]);
        }
    }
    fSavingSendMsg = false;
}


hsBool plCameraRegionDetector::MsgReceive(plMessage* msg)
{
    plCollideMsg* pCollMsg = plCollideMsg::ConvertNoRef(msg);
    if (pCollMsg)
    {
        // camera collisions are only for the local player
        if (plNetClientApp::GetInstance()->GetLocalPlayerKey() != pCollMsg->fOtherKey)
            return true;
        // Fall through to plObjectInVolumeDetector, which will register us for plEvalMsg
        // and handle it for us. (Hint: See ISendSavedTriggerMsgs)
    }

    return plObjectInVolumeDetector::MsgReceive(msg);
}
void plCameraRegionDetector::Read(hsStream* stream, hsResMgr* mgr)
{
    plDetectorModifier::Read(stream, mgr);
    int n = stream->ReadLE32();
    fMessages.resize(n);
    for(size_t i = 0; i < n; i++ )
    {   
        plCameraMsg* pMsg =  plCameraMsg::ConvertNoRef(mgr->ReadCreatable(stream));
        fMessages[i] = pMsg;
    }

}
void plCameraRegionDetector::Write(hsStream* stream, hsResMgr* mgr)
{
    plDetectorModifier::Write(stream, mgr);
    stream->WriteLE32(fMessages.size());
    for(plCameraMsgVec::iterator it = fMessages.begin(); it != fMessages.end(); ++it)
        mgr->WriteCreatable( stream, *it );

}

/////////////////////////////////
/////////////////////////////////
/////////////////////////////////
/////////////////////////////////
// object-in-volume detector

void plObjectInVolumeDetector::ITrigger(plKey hitter, bool entering, bool immediate)
{
    hsRefCnt_SafeUnRef(fSavedActivatorMsg);
    fSavedActivatorMsg = new plActivatorMsg;
    fSavedActivatorMsg->AddReceivers(fReceivers);

    if (fProxyKey)
        fSavedActivatorMsg->fHiteeObj = fProxyKey;
    else
        fSavedActivatorMsg->fHiteeObj = GetTarget()->GetKey();

    fSavedActivatorMsg->fHitterObj = hitter;
    fSavedActivatorMsg->SetSender(GetKey());

    if (entering)
    {
        DetectorLog("%s: Saving Entering volume - Evals=%d", GetKeyName().c_str(), fNumEvals);
        fSavedActivatorMsg->SetTriggerType(plActivatorMsg::kVolumeEnter);
        fLastEnterEval = fNumEvals;
    }
    else
    {
        DetectorLog("%s: Saving Exiting volume - Evals=%d", GetKeyName().c_str(), fNumEvals);
        fSavedActivatorMsg->SetTriggerType(plActivatorMsg::kVolumeExit);
        fLastExitEval = fNumEvals;
    }

    if (immediate)
        ISendSavedTriggerMsgs();
}

void plObjectInVolumeDetector::ISendSavedTriggerMsgs()
{
    if (fSavedActivatorMsg)
    {
        if (fSavedActivatorMsg->fTriggerType == plActivatorMsg::kVolumeEnter)
            DetectorLog("%s: Sending Entering volume - Evals=%d", GetKeyName().c_str(), fNumEvals);
        else
            DetectorLog("%s: Sending Exiting volume - Evals=%d", GetKeyName().c_str(), fNumEvals);

        // we're saving the message to be dispatched later...
        plgDispatch::MsgSend(fSavedActivatorMsg);
    }
    fSavedActivatorMsg = nil;
}

hsBool plObjectInVolumeDetector::MsgReceive(plMessage* msg)
{
    plCollideMsg* pCollMsg = plCollideMsg::ConvertNoRef(msg);
    if (pCollMsg)
    {
        fLastHitter = pCollMsg->fOtherKey;
        // If the avatar is disabled (flying around), don't trigger
        if (IIsDisabledAvatar(fLastHitter))
            return false;
        ITrigger(fLastHitter, (pCollMsg->fEntering != 0));

        // If we never eval before the exit...
        if (fWaitingForEval)
            plgDispatch::Dispatch()->UnRegisterForExactType(plEvalMsg::Index(), GetKey());
        else
            plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
        fWaitingForEval = !fWaitingForEval;
        return true;
    }

    plEvalMsg* pEvalMsg = plEvalMsg::ConvertNoRef(msg);
    if (pEvalMsg)
    {
        fNumEvals++;
        // Don't dispatch if we're not in the age
        if (plArmatureMod* av = IGetAvatarModifier(fLastHitter))
            if (av->IsMidLink())
                return true;
        ISendSavedTriggerMsgs();
        plgDispatch::Dispatch()->UnRegisterForExactType(plEvalMsg::Index(), GetKey());
        fWaitingForEval = false;
    }

    plPlayerPageMsg* pageMsg = plPlayerPageMsg::ConvertNoRef(msg);
    if (pageMsg && pageMsg->fUnload)
    {
        ITrigger(pageMsg->fPlayer, false);
    }

    return plCollisionDetector::MsgReceive(msg);
}

void plObjectInVolumeDetector::SetTarget(plSceneObject* so)
{
    plCollisionDetector::SetTarget(so);

    if (so)
        plgDispatch::Dispatch()->RegisterForExactType(plPlayerPageMsg::Index(), GetKey());
    else
        plgDispatch::Dispatch()->UnRegisterForExactType(plPlayerPageMsg::Index(), GetKey());
}

void plObjectInVolumeDetector::Read(hsStream* stream, hsResMgr* mgr)
{
    plCollisionDetector::Read(stream, mgr);
}

void plObjectInVolumeDetector::Write(hsStream* stream, hsResMgr* mgr)
{
    plCollisionDetector::Write(stream, mgr);
}


///////////////////////////////////////////////////////////////////////////////


plObjectInVolumeAndFacingDetector::plObjectInVolumeAndFacingDetector() :
    plObjectInVolumeDetector(),
    fFacingTolerance(0),
    fNeedWalkingForward(false),
    fAvatarInVolume(false),
    fTriggered(false)
{
}

plObjectInVolumeAndFacingDetector::~plObjectInVolumeAndFacingDetector()
{
}

void plObjectInVolumeAndFacingDetector::SetFacingTolerance(int degrees)
{
    fFacingTolerance = cos(hsDegreesToRadians(degrees));
}

void plObjectInVolumeAndFacingDetector::ICheckForTrigger()
{
    plArmatureMod* armMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
    plSceneObject* avatar = armMod ? armMod->GetTarget(0) : nil;
    plSceneObject* target = GetTarget();

    if (armMod && target)
    {
        hsVector3 playerView = avatar->GetCoordinateInterface()->GetLocalToWorld().GetAxis(hsMatrix44::kView);
        hsVector3 objView = target->GetCoordinateInterface()->GetLocalToWorld().GetAxis(hsMatrix44::kView);

        playerView.Normalize();
        objView.Normalize();

        float dot = playerView * objView;
//      hsStatusMessageF("Dot: %f Tolerance: %f", dot, fFacingTolerance);
        bool facing = dot >= fFacingTolerance;

        bool movingForward = false;
        if (fNeedWalkingForward)
        {
            // And are we walking towards it?
            plArmatureBrain* abrain =  armMod->FindBrainByClass(plAvBrainHuman::Index()); //armMod->GetCurrentBrain();
            plAvBrainHuman* brain = plAvBrainHuman::ConvertNoRef(abrain);
            if (brain && brain->IsMovingForward() && brain->fCallbackAction->IsOnGround())
                movingForward = true;
        }
        else
            movingForward = true;

        if (facing && movingForward && !fTriggered)
        {
            DetectorLog("%s: Trigger InVolume&Facing", GetKeyName().c_str());
            fTriggered = true;
            ITrigger(avatar->GetKey(), true, true);
        }
        else if (!facing && fTriggered)
        {
            DetectorLog("%s: Untrigger InVolume&Facing", GetKeyName().c_str());
            fTriggered = false;
            ITrigger(avatar->GetKey(), false, true);
        }
    }
}

hsBool plObjectInVolumeAndFacingDetector::MsgReceive(plMessage* msg)
{
    // Avatar is entering or exiting our detector box
    plCollideMsg* collMsg = plCollideMsg::ConvertNoRef(msg);
    if (collMsg)
    {
        // make sure this is the local player... the notify will be the thing that propagates over the network
        if (plNetClientApp::GetInstance()->GetLocalPlayerKey() != collMsg->fOtherKey)
            return true;

        // If the avatar is disabled (flying around), don't trigger
        if (IIsDisabledAvatar(collMsg->fOtherKey))
            return false;

        fAvatarInVolume = (collMsg->fEntering != 0);

        if (fAvatarInVolume)
        {
            plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
            ICheckForTrigger();
        }
        else
        {
            plgDispatch::Dispatch()->UnRegisterForExactType(plEvalMsg::Index(), GetKey());

            // Avatar is leaving the volume, make sure to untrigger if we haven't already
            if (fTriggered)
            {
                fTriggered = false;
                ITrigger(plNetClientApp::GetInstance()->GetLocalPlayerKey(), false, true);
            }
        }

        return true;
    }

    // Avatar is inside our detector box, so every frame we check if we need to trigger
    plEvalMsg* evalMsg = plEvalMsg::ConvertNoRef(msg);
    if (evalMsg)
    {
        ICheckForTrigger();
        return true;
    }

    return plObjectInVolumeDetector::MsgReceive(msg);
}

void plObjectInVolumeAndFacingDetector::Read(hsStream* stream, hsResMgr* mgr)
{
    plObjectInVolumeDetector::Read(stream, mgr);

    fFacingTolerance = stream->ReadLEScalar();
    fNeedWalkingForward = stream->Readbool();
}

void plObjectInVolumeAndFacingDetector::Write(hsStream* stream, hsResMgr* mgr)
{
    plObjectInVolumeDetector::Write(stream, mgr);

    stream->WriteLEScalar(fFacingTolerance);
    stream->Writebool(fNeedWalkingForward);
}

/////////////////////////////////
/////////////////////////////////
/////////////////////////////////
/////////////////////////////////
// subworld region detector

plSubworldRegionDetector::~plSubworldRegionDetector()
{
}


hsBool plSubworldRegionDetector::MsgReceive(plMessage* msg)
{
    plCollideMsg* pCollMsg = plCollideMsg::ConvertNoRef(msg);

    if (pCollMsg)
    {   
        if (plNetClientApp::GetInstance()->GetLocalPlayerKey() != pCollMsg->fOtherKey)
            return true;

        plArmatureMod* avMod = IGetAvatarModifier(pCollMsg->fOtherKey);
        if (avMod)
        {
            DetectorLog("%s subworld detector %s", pCollMsg->fEntering ? "Entering" : "Exiting", GetKeyName().c_str());

            if ((pCollMsg->fEntering && !fOnExit) ||
                (!pCollMsg->fEntering && fOnExit))
            {
                if (fSub)
                {
                    plSceneObject* SO = plSceneObject::ConvertNoRef(fSub->ObjectIsLoaded());
                    if (SO)
                    {
                        DetectorLogSpecial("Switching to subworld %s", fSub->GetName().c_str());

                        plKey nilKey;
                        plSubWorldMsg* msg = new plSubWorldMsg(GetKey(), avMod->GetKey(), fSub);
                        msg->Send();
                    }
                }
                else
                {
                    DetectorLogSpecial("Switching to main subworld");
                    plSubWorldMsg* msg = new plSubWorldMsg(GetKey(), avMod->GetKey(), nil);
                    msg->Send();
                }
            }
        }

        return true;
    }

    return plCollisionDetector::MsgReceive(msg);
}

void plSubworldRegionDetector::Read(hsStream* stream, hsResMgr* mgr)
{
    plDetectorModifier::Read(stream, mgr);
    fSub = mgr->ReadKey(stream);
    fOnExit = stream->ReadBool();
}
void plSubworldRegionDetector::Write(hsStream* stream, hsResMgr* mgr)
{
    plDetectorModifier::Write(stream, mgr);
    mgr->WriteKey(stream, fSub);
    stream->WriteBool(fOnExit);
}

///////////////////////////////////
///////////////////////////////////
/// plPanicLinkDetector
///////////////////////////////////
hsBool plPanicLinkRegion::MsgReceive(plMessage* msg)
{
    if (plCollideMsg* pCollMsg = plCollideMsg::ConvertNoRef(msg))
    {
        if (plNetClientApp::GetInstance()->GetLocalPlayerKey() != pCollMsg->fOtherKey)
            return true;

        if (pCollMsg->fEntering)
        {
            plArmatureMod* avMod = IGetAvatarModifier(pCollMsg->fOtherKey);
            if (avMod)
            {
                if (avMod->IsLinkedIn())
                {
                    hsPoint3 kinPos;
                    if (avMod->GetController())
                    {
                        avMod->GetController()->GetKinematicPosition(kinPos);
                        DetectorLogSpecial("Avatar is panic linking. KinPos at %f,%f,%f and is %s",kinPos.fX,kinPos.fY,kinPos.fZ,avMod->GetController()->IsEnabled() ? "enabled" : "disabled");
                    }
                    avMod->PanicLink(fPlayLinkOutAnim);
                } else
                    DetectorLogRed("PANIC LINK %s before we actually linked in!", GetKey()->GetName().c_str());
            }
        }

        return true;
    }

    return plCollisionDetector::MsgReceive(msg);
}

void plPanicLinkRegion::Read(hsStream* stream, hsResMgr* mgr)
{
    plCollisionDetector::Read(stream, mgr);

    fPlayLinkOutAnim = stream->ReadBool();
}

void plPanicLinkRegion::Write(hsStream* stream, hsResMgr* mgr)
{
    plCollisionDetector::Write(stream, mgr);

    stream->WriteBool(fPlayLinkOutAnim);
}

/////////////////////////////////////////////////////////////////
//
// PLSIMPLEREGIONSENSOR
//
/////////////////////////////////////////////////////////////////

// ctor default
plSimpleRegionSensor::plSimpleRegionSensor()
: fEnterMsg(nil), fExitMsg(nil)
{
}

// ctor canonical
plSimpleRegionSensor::plSimpleRegionSensor(plMessage *enterMsg, plMessage *exitMsg)
: fEnterMsg(enterMsg), fExitMsg(exitMsg)
{
}

// dtor
plSimpleRegionSensor::~plSimpleRegionSensor()
{
    if(fEnterMsg)
        fEnterMsg->UnRef();
    if(fExitMsg)
        fExitMsg->UnRef();
}

// WRITE
void plSimpleRegionSensor::Write(hsStream *stream, hsResMgr *mgr)
{
    plSingleModifier::Write(stream, mgr);
    if(fEnterMsg)
    {
        stream->Writebool(true);
        mgr->WriteCreatable(stream, fEnterMsg);
    } else {
        stream->Writebool(false);
    }
    if(fExitMsg)
    {
        stream->Writebool(true);
        mgr->WriteCreatable(stream, fExitMsg);
    } else {
        stream->Writebool(false);
    }
}

// READ
void plSimpleRegionSensor::Read(hsStream *stream, hsResMgr *mgr)
{
    plSingleModifier::Read(stream, mgr);
    if(stream->Readbool())
    {
        fEnterMsg = plMessage::ConvertNoRef(mgr->ReadCreatable(stream));
    } else {
        fEnterMsg = nil;
    }

    if(stream->Readbool())
    {
        fExitMsg = plMessage::ConvertNoRef(mgr->ReadCreatable(stream));
        hsAssert(fExitMsg, "Corrupted plSimpleRegionSensor during read.");
    } else {
        fExitMsg = nil;
    }
}

// MSGRECEIVE
hsBool plSimpleRegionSensor::MsgReceive(plMessage *msg)
{
    plCollideMsg* pCollMsg = plCollideMsg::ConvertNoRef(msg);

    if (pCollMsg)
    {
        // make sure this is the local player... the notify will be the thing that propagates over the network
        if (plNetClientApp::GetInstance()->GetLocalPlayerKey() != pCollMsg->fOtherKey)
            return true;

        plKey theThingWhatDoneHitUs = pCollMsg->fOtherKey;
        
        if(pCollMsg->fEntering)
        {
            if(fEnterMsg)
            {
                fEnterMsg->ClearReceivers();
                fEnterMsg->AddReceiver(theThingWhatDoneHitUs);
                fEnterMsg->Ref();
                fEnterMsg->Send();
            }
        } 
        else {
            if(fExitMsg)
            {
                fExitMsg->ClearReceivers();
                fExitMsg->AddReceiver(theThingWhatDoneHitUs);
                fExitMsg->Ref();
                fExitMsg->Send();
            }
        }
        return true;
    }
    return plSingleModifier::MsgReceive(msg);
}

// IEVAL
hsBool plSimpleRegionSensor::IEval(double secs, float del, uint32_t dirty)
{
    return false;
}

//////////////////////////////////////////////////////////////////////////////

// Nuke the Read/Write functions on the next file format change
void plSwimDetector::Write(hsStream *stream, hsResMgr *mgr)
{
    plSimpleRegionSensor::Write(stream, mgr);

    stream->WriteByte(0);
    stream->WriteLEScalar(0);
    stream->WriteLEScalar(0);
}

void plSwimDetector::Read(hsStream *stream, hsResMgr *mgr)
{
    plSimpleRegionSensor::Read(stream, mgr);

    stream->ReadByte();
    stream->ReadLEScalar();
    stream->ReadLEScalar();
}
hsBool plSwimDetector::MsgReceive(plMessage *msg)
{
    plCollideMsg* pCollMsg = plCollideMsg::ConvertNoRef(msg);

    if (pCollMsg)
    {
        //removed local player check because this will apply the brain to the local 
        //controller of the foreign avatar which we still want.
        //and if we prop swim state by notify messages we still have a chance of missing it from players
        //who were in the region before we linked in
        plKey theThingWhatDoneHitUs = pCollMsg->fOtherKey;
        if(pCollMsg->fEntering)
        {
            if(fEnterMsg)
            {
                fEnterMsg->ClearReceivers();
                fEnterMsg->AddReceiver(theThingWhatDoneHitUs);
                fEnterMsg->Ref();
                fEnterMsg->Send();
            }
        } 
        else {
        if(fExitMsg)
            {
                fExitMsg->ClearReceivers();
                fExitMsg->AddReceiver(theThingWhatDoneHitUs);
                fExitMsg->Ref();
                fExitMsg->Send();
            }
        }
        return true;
    }
    return plSimpleRegionSensor::MsgReceive(msg);
}
hsBool  plRidingAnimatedPhysicalDetector::MsgReceive(plMessage *msg)
{
    
    plCollideMsg* pCollMsg = plCollideMsg::ConvertNoRef(msg);

    if (pCollMsg)
    {
        //removed local player check because this will apply the brain to the local 
        //controller of the foreign avatar which we still want.
        //and if we prop  state by notify messages we still have a chance of missing it from players
        //who were in the region before we linked in
        plKey theThingWhatDoneHitUs = pCollMsg->fOtherKey;
        if(pCollMsg->fEntering)
        {
            if(fEnterMsg)
            {
                fEnterMsg->ClearReceivers();
                fEnterMsg->AddReceiver(theThingWhatDoneHitUs);
                fEnterMsg->Ref();
                fEnterMsg->Send();
            }
        } 
        else {
        if(fExitMsg)
            {
                fExitMsg->ClearReceivers();
                fExitMsg->AddReceiver(theThingWhatDoneHitUs);
                fExitMsg->Ref();
                fExitMsg->Send();
            }
        }
        return true;
    }
    return plSimpleRegionSensor::MsgReceive(msg);
}
