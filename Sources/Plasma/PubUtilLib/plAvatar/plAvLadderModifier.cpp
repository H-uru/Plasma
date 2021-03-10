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

// singular
#include "plAvLadderModifier.h"

// global
#include "HeadSpin.h"
#include "hsStream.h"
#include "plCreatableIndex.h"
#include "plgDispatch.h"

#include <cmath>

// local
#include "plAnimStage.h"
#include "plArmatureMod.h"
#include "plAvatarMgr.h"
#include "plAvBrainGeneric.h"
#include "plAvBrainHuman.h"
#include "plPhysicalControllerCore.h"

// other
#include "pnKeyedObject/plKey.h"
#include "pnMessage/plEnableMsg.h"
#include "pnMessage/plNotifyMsg.h"
#include "pnMessage/plTimeMsg.h"
#include "pnNetCommon/plNetApp.h"
#include "pnSceneObject/plCoordinateInterface.h"

#include "plAnimation/plAGAnim.h"
#include "plMessage/plAvatarMsg.h"
#include "plMessage/plCollideMsg.h"
#include "plModifier/plDetectorLog.h"

enum NotifyType
{
    kNotifyTrigger,
    kNotifyAvatarOnLadder,
};

// Must be facing within 45 degrees of the ladder
static const float kTolerance = cos(hsDegreesToRadians(45));

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
        float dot = playerView * fLadderView;

        bool movingForward = false;

        // And are we walking towards it?
        hsAssert(armMod, "Avatar doesn't have an armature mod");
        if (armMod)
        {
            plAvBrainHuman* brain = plAvBrainHuman::ConvertNoRef(armMod->GetCurrentBrain());
            if (brain && brain->IsMovingForward() && brain->fWalkingStrategy->IsOnGround())
                movingForward = true;
        }

        if (dot >= kTolerance && movingForward)
        {
            plDetectorLog::Special("{}: Ladder starting climb ({f})",
                                  GetKeyName(), hsRadiansToDegrees(acos(dot)));
            return true;
        }
        else if (movingForward)
        {
//          plDetectorLog::Log("{}: Ladder rejecting climb ({f})", GetKeyName(), hsRadiansToDegrees(acos(dot)));
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
        plNotifyMsg *notifyMsg = new plNotifyMsg(avPhysKey, GetKey());
        notifyMsg->fID = kNotifyTrigger;
        notifyMsg->Send();
        fAvatarMounting = true;
    }
}

// MSGRECEIVE
bool plAvLadderMod::MsgReceive(plMessage* msg)
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
            plDetectorLog::Special("{}: Avatar entered ladder region", GetKeyName());

            if (IIsReadyToClimb())
                ITriggerSelf(collMsg->fOtherKey);
            else
                plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
        }
        else
        {
            plDetectorLog::Special("{}: Avatar exited ladder region", GetKeyName());

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
            plDetectorLog::Special("{}: Avatar mounted ladder", GetKeyName());
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
                plKey seekKey = seekObj->GetKey();      // this modifier's target is the seek object

                const char *mountName, *dismountName, *traverseName;

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

                plAnimStageVec *v = new plAnimStageVec;

                plAnimStage *s1 = new plAnimStage(mountName,
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
                    plAnimStage *s2 = new plAnimStage(traverseName,
                                                      0,
                                                      plAnimStage::kForwardKey,
                                                      plAnimStage::kBackKey,
                                                      plAnimStage::kAdvanceAuto,
                                                      plAnimStage::kRegressAuto,
                                                      fLoops - 1    // first loop is implied; zero loops means
                                                                    // play this anim once, 1 loop means "loop
                                                                    // once after reaching the end."
                                                      );
                    if( ! fGoingUp)
                        s2->SetReverseOnIdle(true);
                    v->push_back(s2);
                }
                plAnimStage *s3 = new plAnimStage(dismountName,
                                                  0,
                                                  plAnimStage::kForwardAuto,
                                                  plAnimStage::kBackAuto,
                                                  plAnimStage::kAdvanceAuto,
                                                  plAnimStage::kRegressAuto,
                                                  0);
                if( ! fGoingUp)
                    s3->SetReverseOnIdle(true);
                v->push_back(s3);

                plNotifyMsg* enterNotify = new plNotifyMsg(GetKey(), GetKey());
                enterNotify->fID = kNotifyAvatarOnLadder;

                uint32_t exitFlags = plAvBrainGeneric::kExitNormal;

                plAvBrainGeneric *ladBrain = new plAvBrainGeneric(v, enterNotify, nullptr, nullptr, exitFlags, plAvBrainGeneric::kDefaultFadeIn,
                                                                  plAvBrainGeneric::kDefaultFadeOut, plAvBrainGeneric::kMoveRelative);
                ladBrain->SetType(plAvBrainGeneric::kLadder);
                ladBrain->SetReverseFBControlsOnRelease(!fGoingUp);

                plKey avKey = constAvMod->GetKey();

                // Very important that we dumb seek here. Otherwise you can run off the edge of a ladder, and seek will be helpless
                // until you hit the ground, at which point you have no hope of successfully seeking.
                plAvSeekMsg *seeker = new plAvSeekMsg(nullptr, avKey, seekKey, 1.0f, false);
                seeker->Send();
                plAvPushBrainMsg *brainer = new plAvPushBrainMsg(nullptr, avKey, ladBrain);
                brainer->Send();
            }
        }
    }
}

void plAvLadderMod::Read(hsStream *stream, hsResMgr *mgr)
{
    plSingleModifier::Read(stream, mgr);
    
    fType = stream->ReadLE32();
    fLoops = stream->ReadLE32();
    fGoingUp = stream->ReadBool();
    fEnabled = stream->ReadBool();
    fLadderView.fX = stream->ReadLEFloat();
    fLadderView.fY = stream->ReadLEFloat();
    fLadderView.fZ = stream->ReadLEFloat();
}

void plAvLadderMod::Write(hsStream *stream, hsResMgr *mgr)
{
    plSingleModifier::Write(stream, mgr);

    stream->WriteLE32(fType);
    stream->WriteLE32(fLoops);
    stream->WriteBool(fGoingUp);
    stream->WriteBool(fEnabled);
    stream->WriteLEFloat(fLadderView.fX);
    stream->WriteLEFloat(fLadderView.fY);
    stream->WriteLEFloat(fLadderView.fZ);
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
