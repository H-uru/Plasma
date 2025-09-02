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

#include "plAvBrainHuman.h"

#include "HeadSpin.h"

#include "plgDispatch.h"
#include "hsGeometry3.h"
#include "plPhysical.h"
#include "hsQuat.h"
#include "hsTimer.h"

#include "plAnimStage.h"
#include "plArmatureMod.h"
#include "plAvatarClothing.h"
#include "plAvatarMgr.h"
#include "plAvBrainClimb.h"
#include "plAvBrainDrive.h"
#include "plAvBrainGeneric.h"
#include "plAvBrainSwim.h"
#include "plAvTask.h"
#include "plAvTaskBrain.h"
#include "plAvTaskSeek.h"
#include "plPhysicalControllerCore.h"

#include <cmath>
#include <string_theory/format>
#include <vector>

#include "pnEncryption/plRandom.h"
#include "pnSceneObject/plCoordinateInterface.h"

#include "plAnimation/plAGAnim.h"
#include "plAnimation/plAGAnimInstance.h"
#include "plAnimation/plAGModifier.h"
#include "plAnimation/plMatrixChannel.h"
#include "plInputCore/plAvatarInputInterface.h"
#include "plInputCore/plInputDevice.h"
#include "plInterp/plAnimTimeConvert.h"
#include "plMessage/plAvatarMsg.h"
#include "plMessage/plClimbMsg.h"
#include "plMessage/plInputEventMsg.h"
#include "plMessage/plLOSHitMsg.h"
#include "plMessage/plLOSRequestMsg.h"
#include "plMessage/plRideAnimatedPhysMsg.h"
#include "plMessage/plSwimMsg.h"
#include "pnMessage/plWarpMsg.h"
#include "plNetClient/plNetClientMgr.h"
#include "plPipeline/plDebugText.h"
#include "plStatusLog/plStatusLog.h"

float plAvBrainHuman::fWalkTimeToMaxTurn = .3f;
float plAvBrainHuman::fRunTimeToMaxTurn = .1f;
float plAvBrainHuman::fWalkMaxTurnSpeed = 2.0f;
float plAvBrainHuman::fRunMaxTurnSpeed = 1.7f;
plAvBrainHuman::TurnCurve plAvBrainHuman::fWalkTurnCurve = plAvBrainHuman::kTurnExponential;
plAvBrainHuman::TurnCurve plAvBrainHuman::fRunTurnCurve = plAvBrainHuman::kTurnExponential;

const float plAvBrainHuman::kAirTimePanicThreshold = 10; // seconds

void plAvBrainHuman::SetTimeToMaxTurn(float time, bool walk)
{
    if (walk)
        fWalkTimeToMaxTurn = time;
    else
        fRunTimeToMaxTurn = time;
}

float plAvBrainHuman::GetTimeToMaxTurn(bool walk)
{
    return (walk ? fWalkTimeToMaxTurn : fRunTimeToMaxTurn);
}

void plAvBrainHuman::SetMaxTurnSpeed(float radsPerSec, bool walk)
{
    if (walk)
        fWalkMaxTurnSpeed = radsPerSec;
    else
        fRunMaxTurnSpeed = radsPerSec;
}

float plAvBrainHuman::GetMaxTurnSpeed(bool walk)
{
    return (walk ? fWalkMaxTurnSpeed : fRunMaxTurnSpeed);
}

void plAvBrainHuman::SetTurnCurve(TurnCurve curve, bool walk)
{
    if (walk)
        fWalkTurnCurve = curve;
    else
        fRunTurnCurve = curve;
}

plAvBrainHuman::TurnCurve plAvBrainHuman::GetTurnCurve(bool walk)
{
    return (walk ? fWalkTurnCurve : fRunTurnCurve);
}

plAvBrainHuman::plAvBrainHuman(bool isActor /* = false */) : 
    fHandleAGMod(),
    fStartedTurning(-1.0f),
    fWalkingStrategy(),
    fPreconditions(),
    fIsActor(isActor)
{
}

bool plAvBrainHuman::Apply(double timeNow, float elapsed)
{
#ifndef HS_DEBUGGING
    try
    {
#endif
        // SetTurnStrength runs first to make sure it's set to a sane value
        // (or cleared). RunStandardBehaviors may overwrite it.
        fWalkingStrategy->SetTurnStrength(IGetTurnStrength(timeNow));
        RunStandardBehaviors(timeNow, elapsed);
        fWalkingStrategy->RecalcVelocity(timeNow, elapsed, (fPreconditions & plHBehavior::kBehaviorTypeNeedsRecalcMask));

        plArmatureBrain::Apply(timeNow, elapsed);
#ifndef HS_DEBUGGING
    } catch (std::exception &e) {
        plStatusLog *log = plAvatarMgr::GetInstance()->GetLog();
        log->AddLineF("plAvBrainHuman::Apply - exception caught: {}", e.what());
    } catch (...) {
        // just catch all the crashes on exit...
        plStatusLog *log = plAvatarMgr::GetInstance()->GetLog();
        log->AddLine("plAvBrainHuman::Apply - exception caught");
    }
#endif
    
    return true;
}

void plAvBrainHuman::Activate(plArmatureModBase *avMod)
{
    plArmatureBrain::Activate(avMod);
    
    IInitBoneMap();
    IInitAnimations();
    if (!fWalkingStrategy)
    {
        plSceneObject* avObj = fArmature->GetTarget(0);
        plAGModifier* agMod = const_cast<plAGModifier*>(plAGModifier::ConvertNoRef(FindModifierByClass(avObj, plAGModifier::Index())));
        plPhysicalControllerCore* controller = avMod->GetController();
        fWalkingStrategy = new plWalkingStrategy(agMod->GetApplicator(kAGPinTransform), controller);
        controller->SetMovementStrategy(fWalkingStrategy);
    }
    
    
    if (fAvMod->GetClothingOutfit() && fAvMod->GetClothingOutfit()->fGroup != plClothingMgr::kClothingBaseNoOptions)
    {
        if (fAvMod->IsLocalAvatar())
        {
            fAvMod->GetClothingOutfit()->ReadClothing();
        }
        else 
        {
            fAvMod->GetClothingOutfit()->WearDefaultClothing();
            fAvMod->GetClothingOutfit()->ForceUpdate(true);
        }
    }
    if (fAvMod == plAvatarMgr::GetInstance()->GetLocalAvatar())
        plAvatarInputInterface::GetInstance()->ForceAlwaysRun(plKeyboardDevice::GetInstance()->IsCapsLockKeyOn() != 0);
}

void plAvBrainHuman::IInitBoneMap()
{
    struct tuple {
        HumanBoneID fID;
        const char * fName;
    };

    tuple tupleMap[] = 
    {
        { Pelvis,           "Bone_Root" },
        // left leg
        { LThigh,           "Bone_LThigh" },
        { LCalf,            "Bone_LCalf" },
        { LFoot,            "Bone_LFoot" },
        { LFootPrint,       "Print_L Foot" },
        { LToe0,            "Bone_LToe" },

        // right leg
        { RThigh,           "Bone_RThigh" },
        { RCalf,            "Bone_RCalf" },
        { RFoot,            "Bone_RFoot" },
        { RFootPrint,       "Print_R Foot" },
        { RToe0,            "Bone_RToe" },

        // spine and head, starting at base of spine
        { Spine,            "Bone_Spine0" },
        { TrunkPrint,       "Print_Trunk" },
        { Spine1,           "Bone_Spine1" },
        { Spine2,           "Bone_Spine2" },
        { Neck,             "Bone_Neck" },
        { Head,             "Bone_Head" },
        { Jaw,              "Bone_Jaw" },

        // left face bones
        { LMouthLower,      "Bone_LMouthLower" },
        { RMouthLower,      "Bone_RMouthLower" },
        { LBrowInner,       "Bone_LBrowInner" },
        { LBrowOuter,       "Bone_LBrowOuter" },
        { LCheek,           "Bone_LCheek" },
        { LEye,             "Bone_LEye" },
        { LEyeLid01,        "Bone_LEyeLid1" },
        { LEyeLid02,        "Bone_LEyeLid2" },
        { LMouthCorner,     "Bone_LMouthCorner" },
        { LMouthUpper,      "Bone_LMouthUpper" },

        // right face bones
        { RBrowInner,       "Bone_RBrowInner" },
        { RBrowOuter,       "Bone_RBrowOuter" },
        { RCheek,           "Bone_RCheek" },
        { REye,             "Bone_REye" },
        { REyeLid01,        "Bone_REyeLid1" },
        { REyeLid02,        "Bone_REyeLid2" },
        { RMouthCorner,     "Bone_RMouthCorner" },
        { RMouthUpper,      "Bone_RMouthUpper" },
        
        // Left Arm
        { LClavicle,        "Bone_LClavicle" },
        { LUpperArm,        "Bone_LUpperArm" },
        { LForearm,         "Bone_LForearm" },
        { LHand,            "Bone_LHand" },
        { LHandPrint,       "Print_L Hand" },
        { LMiddleFinger1,   "Bone_LMiddle1" },
        { LMiddleFinger2,   "Bone_LMiddle2" },
        { LMiddleFinger3,   "Bone_LMiddle3" },
        { LPinkyFinger1,    "Bone_LPinky1" },
        { LPinkyFinger2,    "Bone_LPinky2" },
        { LPinkyFinger3,    "Bone_LPinky3" },
        { LPointerFinger1,  "Bone_LPointer1" },
        { LPointerFinger2,  "Bone_LPointer2" },
        { LPointerFinger3,  "Bone_LPointer3" },
        { LRingFinger1,     "Bone_LRing1" },
        { LRingFinger2,     "Bone_LRing2" },
        { LRingFinger3,     "Bone_LRing3" },
        { LThumb1,          "Bone_LThumb1" },
        { LThumb2,          "Bone_LThumb2" },
        { LThumb3,          "Bone_LThumb3" },
        
        // Right Arm
        { RClavicle,        "Bone_RClavicle" },
        { RUpperArm,        "Bone_RUpperArm" },
        { RForearm,         "Bone_RForearm" },
        { RHand,            "Bone_RHand" },
        { RHandPrint,       "Print_R Hand" },
        { RMiddleFinger1,   "Bone_RMiddle1" },
        { RMiddleFinger2,   "Bone_RMiddle2" },
        { RMiddleFinger3,   "Bone_RMiddle3" },
        { RPinkyFinger1,    "Bone_RPinky1" },
        { RPinkyFinger2,    "Bone_RPinky2" },
        { RPinkyFinger3,    "Bone_RPinky3" },
        { RPointerFinger1,  "Bone_RPointer1" },
        { RPointerFinger2,  "Bone_RPointer2" },
        { RPointerFinger3,  "Bone_RPointer3" },
        { RRingFinger1,     "Bone_RRing1" },
        { RRingFinger2,     "Bone_RRing2" },
        { RRingFinger3,     "Bone_RRing3" },
        { RThumb1,          "Bone_RThumb1" },
        { RThumb2,          "Bone_RThumb2" },
        { RThumb3,          "Bone_RThumb3" },
    };

    int numTuples = sizeof(tupleMap) / sizeof(tuple);

    for(int i = 0; i < numTuples; i++)
    {
        HumanBoneID id = tupleMap[i].fID;
        ST::string name = tupleMap[i].fName;
        
        const plSceneObject * bone = this->fAvMod->FindBone(name);
        if( bone )
        {
            fAvMod->AddBoneMapping(id, bone);
        }
        else
            hsStatusMessage(ST::format("Couldn't find standard bone {}.", name).c_str());
    }
}

plAvBrainHuman::~plAvBrainHuman()
{
    for (plArmatureBehavior* behavior : fBehaviors)
        delete behavior;
    fBehaviors.clear();

    delete fWalkingStrategy;
    fWalkingStrategy = nullptr;
}

void plAvBrainHuman::Deactivate()
{
    // fAvMod will be nil here when exporting.
    if (fAvMod)
        plAvatarMgr::GetInstance()->RemoveAvatar(fAvMod);       // unregister

    plArmatureBrain::Deactivate();
}

void plAvBrainHuman::Suspend()
{
    // Kind of hacky... but this is a rather rare case.
    // If the user lets up on the PushToTalk key in another brain
    // we'll miss the message to take off the animation.
    ST::string chatAnimName = fAvMod->MakeAnimationName("Talk");
    plAGAnimInstance *anim = fAvMod->FindAnimInstance(chatAnimName);
    if (anim)
        anim->FadeAndDetach(0, 1);

    IdleOnly();
    plArmatureBrain::Suspend();
}

void plAvBrainHuman::Resume()
{
    // If we were in another brain when the key was pressed, we missed it.
    if (fAvMod->GetInputFlag(S_PUSH_TO_TALK))
        IChatOn();
    
    fWalkingStrategy->Reset(false);
    
    plArmatureBrain::Resume();
}

bool plAvBrainHuman::IHandleControlMsg(plControlEventMsg* msg)
{
    ControlEventCode moveCode = msg->GetControlCode();

    if( msg->ControlActivated() )
    {
        switch(moveCode)
        {
        case B_CONTROL_TOGGLE_PHYSICAL:
            {
#ifndef PLASMA_EXTERNAL_RELEASE     // external clients can't go non-physical
                plAvBrainDrive *driver = new plAvBrainDrive(20, 1);
                fAvMod->PushBrain(driver);
#endif
                return true;
            }
            break;
        case S_PUSH_TO_TALK:
            fAvMod->SetInputFlag(S_PUSH_TO_TALK, true);
            IChatOn();
            return true;
            break;
        default:
            break;
        }
    } else {
        switch(moveCode)
        {
        case S_PUSH_TO_TALK:
            fAvMod->SetInputFlag(S_PUSH_TO_TALK, false);            
            IChatOff();
            return true;
            break;
        default:
            break;
        }
    }
    return false;
}

bool plAvBrainHuman::IsMovingForward()
{
    if ((fBehaviors.size() <= kWalk) || (fBehaviors.size() <= kRun))
        return false; // behaviors aren't set up yet
    return (fBehaviors[kWalk]->GetStrength() > 0 || fBehaviors[kRun]->GetStrength() > 0);
}

bool plAvBrainHuman::IsBehaviorPlaying(int behavior)
{
    if ((behavior < 0) || ((size_t)behavior >= fBehaviors.size()))
        return false;
    if (!fBehaviors[behavior])
        return false;
    return (fBehaviors[behavior]->GetStrength() > 0);
}

void plAvBrainHuman::Write(hsStream *stream, hsResMgr *mgr)
{
    plArmatureBrain::Write(stream, mgr);

    stream->WriteBool(fIsActor);
}

void plAvBrainHuman::Read(hsStream *stream, hsResMgr *mgr)
{
    plArmatureBrain::Read(stream, mgr);

    fIsActor = stream->ReadBool();
}

bool plAvBrainHuman::MsgReceive(plMessage * msg)
{
    plControlEventMsg *ctrlMsg = plControlEventMsg::ConvertNoRef(msg);
    if (ctrlMsg) 
    {
        return IHandleControlMsg(ctrlMsg);
    }
    
    plClimbMsg *climb = plClimbMsg::ConvertNoRef(msg);
    if (climb) {
        return IHandleClimbMsg(climb);
    }

    plSwimMsg *swim = plSwimMsg::ConvertNoRef(msg);
    if (swim) 
    {
        if (swim->GetIsEntering())
        {
            plAvBrainSwim *swimBrain = new plAvBrainSwim();
            swimBrain->MsgReceive(swim);
            fAvMod->PushBrain(swimBrain);
        } 
        else 
        {
            hsStatusMessage("Got non-entering swim message. Discarding.");
        }
        return true;
    }
    plRideAnimatedPhysMsg *ride = plRideAnimatedPhysMsg::ConvertNoRef(msg);
    if(ride)
    {
        fWalkingStrategy->ToggleRiding(ride->Entering());
        return true;
    }

    return plArmatureBrain::MsgReceive(msg);
}

bool plAvBrainHuman::IHandleClimbMsg(plClimbMsg *msg)
{
    bool isStartClimb = msg->fCommand == plClimbMsg::kStartClimbing;

    if(isStartClimb)
    {
        // Warp the player to the Seekpoint
        plSceneObject *avatarObj = plSceneObject::ConvertNoRef(plNetClientMgr::GetInstance()->GetLocalPlayer());
        plSceneObject *obj = plSceneObject::ConvertNoRef(msg->fTarget->ObjectIsLoaded());
        plArmatureMod *localAvatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
        plArmatureMod *climbAvatar = plArmatureMod::ConvertNoRef(fArmature);
        if (climbAvatar == localAvatar) // is it our avatar who has to seek?
        {
            hsMatrix44 target = obj->GetLocalToWorld();
            plWarpMsg *warp = new plWarpMsg(nullptr, avatarObj->GetKey(), plWarpMsg::kFlushTransform, target);
            warp->SetBCastFlag(plMessage::kNetPropagate);
            plgDispatch::MsgSend(warp);
        }

        // build the Climb brain
        plAvBrainClimb::Mode startMode;
        switch(msg->fDirection)
        {
        case plClimbMsg::kUp:
            startMode = plAvBrainClimb::kMountingUp;
            break;
        case plClimbMsg::kDown:
            startMode = plAvBrainClimb::kMountingDown;
            break;
        case plClimbMsg::kLeft:
            startMode = plAvBrainClimb::kMountingLeft;
            break;
        case plClimbMsg::kRight:
            startMode = plAvBrainClimb::kMountingRight;
            break;
        default:
            break;
        }

        plAvBrainClimb *brain = new plAvBrainClimb(startMode);
        climbAvatar->PushBrain(brain);
    }
    // ** potentially controversial:
    // It's fairly easy for a human brain to hit a climb trigger - like when falling off a wall.
    // When this happens, We should just eat the message to keep it from travelling any further.
    // The argument against is that there might be a climb brain that actually wants the message,
    // but if that were true the message would have been given to that climb brain first.
    return true;
}

float plAvBrainHuman::IGetTurnStrength(double timeNow)
{
    float result = 0.f;
    float timeToMaxTurn, maxTurnSpeed;
    plAvBrainHuman::TurnCurve turnCurve;
    if (fAvMod->FastKeyDown())
    {
        timeToMaxTurn = fRunTimeToMaxTurn;
        maxTurnSpeed = fRunMaxTurnSpeed;
        turnCurve = fRunTurnCurve;
    }
    else
    {
        timeToMaxTurn = fWalkTimeToMaxTurn;
        maxTurnSpeed = fWalkMaxTurnSpeed;
        turnCurve = fWalkTurnCurve;
    }
    
    plArmatureBehavior * turnLeft  = fBehaviors.size() >= kMovingTurnLeft  ? fBehaviors[kMovingTurnLeft]  : nullptr;
    plArmatureBehavior * turnRight = fBehaviors.size() >= kMovingTurnRight ? fBehaviors[kMovingTurnRight] : nullptr;
    
    float turnLeftStrength = turnLeft ? turnLeft->GetStrength() : 0.f;
    float turnRightStrength = turnRight ? turnRight->GetStrength() : 0.f;
    
    
    // Turning based on keypress
    if ((turnLeftStrength > 0.f)
    || (turnRightStrength > 0.f)
    || (!fWalkingStrategy->IsOnGround() 
        && !fWalkingStrategy->IsControlledFlight())
    ) {
        float t = (float)(timeNow - fStartedTurning);
        float turnSpeed;
        if(t > timeToMaxTurn)
        {
            turnSpeed = maxTurnSpeed;
        } else {
            float n = t / timeToMaxTurn;    // normalize
            
            switch(turnCurve) {
            case kTurnLinear:
                // linear
                turnSpeed = n * maxTurnSpeed;
                break;
            case kTurnExponential:
                // exponential
                turnSpeed = (n * n) * maxTurnSpeed;
                break;
            case kTurnLogarithmic:
                // logarithmic
                turnSpeed = n > .1 ? log10(n * 10) * maxTurnSpeed : .00001f;
                break;
            default:
                hsAssert(false, "What the heck?");
                turnSpeed = 0.0f;
            }
            
        }
        result += fAvMod->GetKeyTurnStrength() * turnSpeed;
    }
    
    if (!fWalkingStrategy->IsControlledFlight())
        result += fAvMod->GetAnalogTurnStrength() * maxTurnSpeed;
    
    return result;  
}   

bool plAvBrainHuman::IHandleTaskMsg(plAvTaskMsg *msg)
{
    if(plAvSeekMsg * seekM = plAvSeekMsg::ConvertNoRef(msg))
    {
        // seek and subclasses always have a seek first
        if(seekM->fSmartSeek)
        {
            // use smart seek
            plAvTaskSeek * seek = new plAvTaskSeek(seekM);
            QueueTask(seek);
        }
        else
        if (!seekM->fNoSeek)
        {
            // use dumb seek
            plAvSeekTask *seek = new plAvSeekTask(seekM->fSeekPoint, seekM->fAlignType, seekM->fAnimName);
            QueueTask(seek);
        }
        // else don't seek at all.

        plAvOneShotMsg * oneshotM = plAvOneShotMsg::ConvertNoRef(msg);
        if(oneshotM)
        {
            // if it's a oneshot, add the oneshot task as well
            plAvOneShotTask *oneshot = new plAvOneShotTask(oneshotM, fAvMod, this);
            QueueTask(oneshot);
        }
    } else if (plAvPushBrainMsg *pushM = plAvPushBrainMsg::ConvertNoRef(msg)) {
        plAvTaskBrain * push = new plAvTaskBrain(pushM->fBrain);
        QueueTask(push);
    } else if (plAvPopBrainMsg *popM = plAvPopBrainMsg::ConvertNoRef(msg)) {
        plAvTaskBrain * pop = new plAvTaskBrain();
        QueueTask(pop);
    } else if (plAvTaskMsg *taskM = plAvTaskMsg::ConvertNoRef(msg)) {
        plAvTask *task = taskM->GetTask();
        QueueTask(task);
    } else {
        hsStatusMessage("Couldn't recognize task message type.");
        return plArmatureBrain::IHandleTaskMsg(msg);
    }
    return true;
}

void plAvBrainHuman::ResetIdle()
{
    if (fBehaviors.size() > kIdle)
        fBehaviors[kIdle]->Rewind();
}

void plAvBrainHuman::IdleOnly(bool instantOff)
{
    if (!fWalkingStrategy)
        return;

    float rate = instantOff ? 0.f : 1.f;

    for (size_t i = kWalk; i < fBehaviors.size(); i++)
        fBehaviors[i]->SetStrength(0, rate);
}

bool plAvBrainHuman::IsMovementZeroBlend()
{
    for (size_t i = 0; i < fBehaviors.size(); i++)
    {
        if (i == kIdle || i == kFall)
            continue;

        if (fBehaviors[i]->GetStrength() > 0)
            return false;
    }
    return true;
}

void plAvBrainHuman::TurnToPoint(hsPoint3 point)
{
    if (!fWalkingStrategy->IsOnGround() || IsRunningTask() || fAvMod->GetCurrentBrain() != this || !IsMovementZeroBlend())
        return;

    hsPoint3 avPos;
    fAvMod->GetTarget(0)->GetCoordinateInterface()->GetLocalToWorld().GetTranslate(&avPos);
    const plCoordinateInterface* subworldCI = nullptr;
    if (fAvMod->GetController())
        subworldCI = fAvMod->GetController()->GetSubworldCI();
    if (subworldCI)
    {
        point = subworldCI->GetWorldToLocal() * point;
        avPos = subworldCI->GetWorldToLocal() * avPos;
    }
    
    plAvSeekMsg *msg = new plAvSeekMsg(nullptr, fAvMod->GetKey(), nullptr, 1.f, true);
    hsClearBits(msg->fFlags, plAvSeekMsg::kSeekFlagForce3rdPersonOnStart);
    hsSetBits(msg->fFlags, plAvSeekMsg::kSeekFlagNoWarpOnTimeout | plAvSeekMsg::kSeekFlagRotationOnly);
    msg->fTargetLookAt = point;
    msg->fTargetPos = avPos;
    msg->SetBCastFlag(plMessage::kNetPropagate);
    msg->Send();
}

void plAvBrainHuman::IChatOn()
{
    ST::string chatAnimName = fAvMod->MakeAnimationName("Talk");

    // check that we aren't adding this twice...
    if (!fAvMod->FindAnimInstance(chatAnimName))
    {
        plKey avKey = fAvMod->GetKey();
        plAvAnimTask *animTask = new plAvAnimTask(chatAnimName, 0.0, 1.0, 1.0, 0.0, true, true, true);
        if (animTask)
        {
            plAvTaskMsg *taskMsg = new plAvTaskMsg(avKey, avKey, animTask);
            taskMsg->SetBCastFlag(plMessage::kNetPropagate);
            taskMsg->Send();
        }
    }
}

void plAvBrainHuman::IChatOff()
{
    ST::string chatAnimName = fAvMod->MakeAnimationName("Talk");
    plKey avKey = fAvMod->GetKey();
    plAvAnimTask *animTask = new plAvAnimTask(chatAnimName, -1.0);
    if (animTask)
    {
        plAvTaskMsg *taskMsg = new plAvTaskMsg(avKey, avKey, animTask);
        taskMsg->SetBCastFlag(plMessage::kNetPropagate);
        taskMsg->Send();
    }
}

bool plAvBrainHuman::IInitAnimations()
{
    bool result = false;

    plAGAnim *idle = fAvMod->FindCustomAnim("Idle");
    plAGAnim *walk = fAvMod->FindCustomAnim("Walk");
    plAGAnim *run = fAvMod->FindCustomAnim("Run");
    plAGAnim *walkBack = fAvMod->FindCustomAnim("WalkBack");
    plAGAnim *stepLeft = fAvMod->FindCustomAnim("StepLeft");
    plAGAnim *stepRight = fAvMod->FindCustomAnim("StepRight");
    plAGAnim *standingLeft = fAvMod->FindCustomAnim("TurnLeft");
    plAGAnim *standingRight = fAvMod->FindCustomAnim("TurnRight");
    plAGAnim *fall = fAvMod->FindCustomAnim("Fall");
    plAGAnim *standJump = fAvMod->FindCustomAnim("StandingJump");
    plAGAnim *walkJump = fAvMod->FindCustomAnim("WalkingJump");
    plAGAnim *runJump = fAvMod->FindCustomAnim("RunningJump");
    plAGAnim *groundImpact = fAvMod->FindCustomAnim("GroundImpact");
    plAGAnim *runningImpact = fAvMod->FindCustomAnim("RunningImpact");
    plAGAnim *movingLeft = nullptr; // fAvMod->FindCustomAnim("LeanLeft");
    plAGAnim *movingRight = nullptr; // fAvMod->FindCustomAnim("LeanRight");
    plAGAnim *pushWalk = fAvMod->FindCustomAnim("BallPushWalk");

    //plAGAnim *pushIdle = fAvMod->FindCustomAnim("BallPushIdle");
    
    const float kDefaultFade = 3.0;     // most animations fade in and out in 1/4 of a second.

    if (idle && walk && run && walkBack && standingLeft && standingRight && stepLeft && stepRight)
    {
        plHBehavior *behavior;
        fBehaviors.assign(kHuBehaviorMax, nullptr);
        fBehaviors[kIdle] = behavior = new Idle;
        behavior->Init(idle, true, this, fAvMod, kDefaultFade, kDefaultFade, kIdle, plHBehavior::kBehaviorTypeIdle);
        behavior->SetStrength(1.f, 0.f);
        
        fBehaviors[kWalk] = behavior = new Walk;
        behavior->Init(walk, true, this, fAvMod, kDefaultFade, 5.f, kWalk, plHBehavior::kBehaviorTypeWalk);
        
        fBehaviors[kRun] = behavior = new Run;
        behavior->Init(run, true, this, fAvMod, kDefaultFade, 2.0, kRun, plHBehavior::kBehaviorTypeRun);

        fBehaviors[kWalkBack] = behavior = new WalkBack;
        behavior->Init(walkBack, true, this, fAvMod, kDefaultFade, kDefaultFade, kWalkBack, plHBehavior::kBehaviorTypeWalkBack);

        fBehaviors[kStandingTurnLeft] = behavior = new StandingTurnLeft;
        behavior->Init(standingLeft, true, this, fAvMod, 3.0f, 6.0f, kStandingTurnLeft, plHBehavior::kBehaviorTypeTurnLeft);

        fBehaviors[kStandingTurnRight] = behavior = new StandingTurnRight;
        behavior->Init(standingRight, true, this, fAvMod, 3.0f, 6.0f, kStandingTurnRight, plHBehavior::kBehaviorTypeTurnRight);

        fBehaviors[kStepLeft] = behavior = new StepLeft;
        behavior->Init(stepLeft, true, this, fAvMod, kDefaultFade, kDefaultFade, kStepLeft, plHBehavior::kBehaviorTypeSidestepLeft);

        fBehaviors[kStepRight] = behavior = new StepRight;
        behavior->Init(stepRight, true, this, fAvMod, kDefaultFade, kDefaultFade, kStepRight, plHBehavior::kBehaviorTypeSidestepRight);

        // Warning: Changing the blend times of the jump animations will affect the path you take, because until we're fully blended,
        // we won't be using the full motion defined in the animation. This isn't an issue for standing jump, but you need to be
        // aware of it for the walk/run jumps.
        fBehaviors[kFall] = behavior = new Fall;
        behavior->Init(fall, true, this, fAvMod, 1.0f, 10, kFall, plHBehavior::kBehaviorTypeFall);

        fBehaviors[kStandingJump] = behavior = new StandingJump;
        behavior->Init(standJump, false, this, fAvMod, kDefaultFade, kDefaultFade, kStandingJump, plHBehavior::kBehaviorTypeStandingJump);

        fBehaviors[kWalkingJump] = behavior = new WalkingJump;
        behavior->Init(walkJump, false, this, fAvMod, 10, 3.0, kWalkingJump, plHBehavior::kBehaviorTypeWalkingJump);

        fBehaviors[kRunningJump] = behavior = new RunningJump;
        behavior->Init(runJump, false, this, fAvMod, 10, 2.0, kRunningJump, plHBehavior::kBehaviorTypeRunningJump);
        
        fBehaviors[kGroundImpact] = behavior = new GroundImpact;
        behavior->Init(groundImpact, false, this, fAvMod, 6.0f, kDefaultFade, kGroundImpact, plHBehavior::kBehaviorTypeGroundImpact);
        
        fBehaviors[kRunningImpact] = behavior = new RunningImpact;
        behavior->Init(runningImpact, false, this, fAvMod, 6.0f, kDefaultFade, kRunningImpact, plHBehavior::kBehaviorTypeRunningImpact);
        
        fBehaviors[kMovingTurnLeft] = behavior = new MovingTurnLeft;
        behavior->Init(movingLeft, true, this, fAvMod, kDefaultFade, kDefaultFade, kMovingTurnLeft, plHBehavior::kBehaviorTypeMovingTurnLeft);

        fBehaviors[kMovingTurnRight] = behavior = new MovingTurnRight;
        behavior->Init(movingRight, true, this, fAvMod, kDefaultFade, kDefaultFade, kMovingTurnRight, plHBehavior::kBehaviorTypeMovingTurnRight);

        fBehaviors[kPushWalk] = behavior = new PushWalk;
        behavior->Init(pushWalk, true, this, fAvMod, kDefaultFade, kDefaultFade, kPushWalk, plHBehavior::kBehaviorTypePushWalk);
        
        //fBehaviors[kPushIdle] = behavior = new PushIdle;
        //behavior->Init(pushIdle, true, this, fAvMod, kDefaultFade, kDefaultFade, kPushIdle, plHBehavior::kBehaviorTypePushIdle);

        result = true;
    }

    return result;
}

bool plAvBrainHuman::RunStandardBehaviors(double timeNow, float elapsed)
{
    for (plArmatureBehavior* aBehavior : fBehaviors)
    {
        plHBehavior *behavior = (plHBehavior*)aBehavior;
        if (behavior->PreCondition(timeNow, elapsed))
        {
            behavior->SetStrength(1.f, behavior->fFadeIn);
            behavior->Process(timeNow, elapsed);
            fPreconditions |= behavior->GetType();
        }
        else
        {
            behavior->SetStrength(0.f, behavior->fFadeOut);
            fPreconditions &= ~behavior->GetType();
        }
    }
    return true;
}

void plAvBrainHuman::SetStartedTurning(double when)
{
    fStartedTurning = when;
}

void plAvBrainHuman::Spawn(double timeNow)
{
    plArmatureBrain::Spawn(timeNow);
    IdleOnly(true);     // reset any behavior state we may have accumulated while waiting to spawn

    // IdleOnly will set the blends of all anims to zero, and setting the blends will tell the AGModifier
    // that it needs to compile. Trouble is, the modifier only checks once per frame. MoveViaAnimation
    // works on the physics timestep, and will get called before compilation happens. It will go straight
    // to the old compiled channel, ignore the blends and still move via any anim that was playing before
    // we linked (only for the first frame).
    // 
    // Trouble is, that first frame is usually a large physics step which we don't fully resolve. This means
    // we could miss our spawn point, or worse, spawn into collision geometry and fall through.
    //
    // So we force the modifier to recompile.
    if (fAvMod)
        fAvMod->Compile(timeNow);
}

bool plAvBrainHuman::LeaveAge()
{
    plPhysicalControllerCore* controller = fAvMod->GetController();

    // Restore a clean walking strategy in case of an animated platform.
    delete fWalkingStrategy;
    plSceneObject* avObj = fArmature->GetTarget(0);
    plAGModifier* agMod = const_cast<plAGModifier*>(plAGModifier::ConvertNoRef(FindModifierByClass(avObj, plAGModifier::Index())));
    fWalkingStrategy = new plWalkingStrategy(agMod->GetApplicator(kAGPinTransform), controller);

    fWalkingStrategy->Reset(true);

    plArmatureBrain::LeaveAge();

    // pin the physical so it doesn't fall when the world is deleted
    fAvMod->EnablePhysics(false);

    return false;
}

void plAvBrainHuman::DumpToDebugDisplay(int &x, int &y, int lineHeight, plDebugText &debugTxt)
{
    debugTxt.DrawString(x, y, ST_LITERAL("Brain type: Human"));
    y += lineHeight;
    
    const char *grounded = fWalkingStrategy->IsOnGround() ? "yes" : "no";
    const char *pushing = (fWalkingStrategy->GetPushingPhysical() ? (fWalkingStrategy->GetFacingPushingPhysical() ? "facing" : "behind") : "none");
    debugTxt.DrawString(x, y, ST::format("Ground: {>3}, AirTime: {5.2f} (Peak: {5.2f}), PushingPhys: {>6}",
                grounded, fWalkingStrategy->GetAirTime(), fWalkingStrategy->GetImpactTime(), pushing));
    y += lineHeight;

    for (plArmatureBehavior* behavior : fBehaviors)
        behavior->DumpDebug(x, y, lineHeight, debugTxt);

    debugTxt.DrawString(x, y, ST_LITERAL("Tasks:"));
    y += lineHeight;

    if(fCurTask)
    {
        debugTxt.DrawString(x, y, ST_LITERAL("Current task:"));
        y += lineHeight;

        int indentedX = x + 4;
        fCurTask->DumpDebug("-", indentedX, y, lineHeight, debugTxt);
    }
    int tasks = fTaskQueue.size();
    if(tasks > 0)
    {
        debugTxt.DrawString(x, y, ST_LITERAL("Tasks in the Queue:"));
        y += lineHeight;

        int indentedX = x + 4;

        for (int i = 0; i < tasks; i++)
        {
            plAvTask *each = fTaskQueue[i];
            each->DumpDebug("-", indentedX, y, lineHeight, debugTxt);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
// BEHAVIOR

plHBehavior::plHBehavior() : 
    fAvMod(),
    fHuBrain(),
    fFadeIn(1.0f),
    fFadeOut(-1.0f),
    fMaxBlend(1.0f)
{
}

plHBehavior::~plHBehavior()
{
}

void plHBehavior::Init(plAGAnim *anim, bool loop, plAvBrainHuman *brain,
                       plArmatureMod *body, float fadeIn, float fadeOut,
                       uint8_t index, uint32_t type /* = 0 */)
{
    plArmatureBehavior::Init(anim, loop, brain, body, index);
    fAvMod = body;
    fHuBrain = brain;
    fFadeIn = fadeIn;
    fFadeOut = fadeOut;
    fType = type;
    
    fStartMsgSent = false; // start message hasn't been sent yet
    fStopMsgSent = false; // stop message hasn't been sent yet
}

void plHBehavior::IStart()
{
    plArmatureBehavior::IStart();
    fAvMod->SynchIfLocal(hsTimer::GetSysSeconds(), false);

    if (!fStartMsgSent)
        fAvMod->IFireBehaviorNotify(fType);

    fStartMsgSent = true; // we just sent a start message
    fStopMsgSent = false; // we haven't sent a stop message yet
}

void plHBehavior::IStop()
{
    plArmatureBehavior::IStop();
    fAvMod->SynchIfLocal(hsTimer::GetSysSeconds(), false);

    if (!fStopMsgSent)
        fAvMod->IFireBehaviorNotify(fType, false);

    fStartMsgSent = false; // we haven't sent a start message yet
    fStopMsgSent = true; // we just sent a stop message
}

void Idle::IStart()
{
    static plRandom sRandom;

    plHBehavior::IStart();
    if (fAnim)
    {       
        float newStart = sRandom.RandZeroToOne() * fAnim->GetAnimation()->GetLength();
        fAnim->SetCurrentTime(newStart, true);
    }
}

bool Run::PreCondition(double time, float elapsed)
{
    if (fAnim)
    {
        if (fAvMod->ForwardKeyDown() && fAvMod->FastKeyDown() && fHuBrain->fWalkingStrategy->IsOnGround() &&
            (!fHuBrain->fWalkingStrategy->GetPushingPhysical() || !fHuBrain->fWalkingStrategy->GetFacingPushingPhysical()))
            return true;    
    }
    return false;
}

bool Walk::PreCondition(double time, float elapsed)
{
    if (fAnim)
    {
        if (fAvMod->ForwardKeyDown() && !fAvMod->FastKeyDown() && fHuBrain->fWalkingStrategy->IsOnGround() &&
            (!fHuBrain->fWalkingStrategy->GetPushingPhysical() || !fHuBrain->fWalkingStrategy->GetFacingPushingPhysical()))
            return true;
    }
    return false;
}

bool WalkBack::PreCondition(double time, float elapsed)
{
    if (fAnim)
    {
        if (fAvMod->BackwardKeyDown() && !fAvMod->ForwardKeyDown() && fHuBrain->fWalkingStrategy->IsOnGround() &&
            (!fHuBrain->fWalkingStrategy->GetPushingPhysical() || fHuBrain->fWalkingStrategy->GetFacingPushingPhysical()))
            return true;
    }
    return false;
}

bool StepLeft::PreCondition(double time, float elapsed)
{
    if (fAnim)
    {
        return ((fAvMod->StrafeLeftKeyDown() || (fAvMod->StrafeKeyDown() && fAvMod->TurnLeftKeyDown())) &&
            !(fAvMod->StrafeRightKeyDown() || (fAvMod->StrafeKeyDown() && fAvMod->TurnRightKeyDown())) &&
            !(fAvMod->ForwardKeyDown() || fAvMod->BackwardKeyDown()) &&
            fHuBrain->fWalkingStrategy->IsOnGround());
    }
    return false;
}

bool StepRight::PreCondition(double time, float elapsed)
{
    if (fAnim)
    {
        return ((fAvMod->StrafeRightKeyDown() || (fAvMod->StrafeKeyDown() && fAvMod->TurnRightKeyDown())) &&
                !(fAvMod->StrafeLeftKeyDown() || (fAvMod->StrafeKeyDown() && fAvMod->TurnLeftKeyDown())) &&
                !(fAvMod->ForwardKeyDown() || fAvMod->BackwardKeyDown()) &&
                fHuBrain->fWalkingStrategy->IsOnGround());
    }
    return false;
}

bool StandingTurnLeft::PreCondition(double time, float elapsed)
{
    if (fAnim)
    {
        if (fAvMod->TurnLeftKeyDown() && !fAvMod->TurnRightKeyDown() && !fAvMod->StrafeKeyDown() &&
            !fAvMod->ForwardKeyDown() && !fAvMod->BackwardKeyDown())
        {
            return true;
        }
    }
    return false;
}

bool StandingTurnRight::PreCondition(double time, float elapsed)
{
    if (fAnim)
    {
        if (fAvMod->TurnRightKeyDown() && !fAvMod->TurnLeftKeyDown() && !fAvMod->StrafeKeyDown() &&
            !fAvMod->ForwardKeyDown() && !fAvMod->BackwardKeyDown())
        {
            return true;
        }
    }
    return false;
}

void MovingTurn::IStart()
{
    plHBehavior::IStart();
    fHuBrain->SetStartedTurning(hsTimer::GetSysSeconds());
}

bool MovingTurnLeft::PreCondition(double time, float elapsed)
{
    if (fAvMod->GetTurnStrength() > 0) 
    {
        if (fHuBrain->fWalkingStrategy->IsOnGround() && (fAvMod->ForwardKeyDown() || fAvMod->BackwardKeyDown()) &&
            (!fHuBrain->fWalkingStrategy->GetPushingPhysical() || !fHuBrain->fWalkingStrategy->GetFacingPushingPhysical())) 
            return true;
    }
    return false;
}

bool MovingTurnRight::PreCondition(double time, float elapsed)
{
    if (fAvMod->GetTurnStrength() < 0) 
    {
        if (fHuBrain->fWalkingStrategy->IsOnGround() && (fAvMod->ForwardKeyDown() || fAvMod->BackwardKeyDown()) &&
            (!fHuBrain->fWalkingStrategy->GetPushingPhysical() || !fHuBrain->fWalkingStrategy->GetFacingPushingPhysical())) 
            return true;
    }
    
    return false;
}

void Jump::IStart()
{
    fHuBrain->fWalkingStrategy->EnableControlledFlight(true);
    
    plHBehavior::IStart();
}

void Jump::IStop()
{
    fHuBrain->fWalkingStrategy->EnableControlledFlight(false);

    plHBehavior::IStop();
}

bool StandingJump::PreCondition(double time, float elapsed)
{
    if (fAnim)
    {
        if (GetStrength() > 0.f)
        {
            if (!fHuBrain->fWalkingStrategy->IsControlledFlight() ||
                fAnim->GetTimeConvert()->WorldToAnimTimeNoUpdate(time) >= fAnim->GetTimeConvert()->GetEnd())
            {
                return false;
            }
            return !fAnim->IsFinished();
        } 
        else 
        {
            if (fAvMod->JumpKeyDown() &&
                !fAvMod->ForwardKeyDown() &&
                fAnim->GetBlend() == 0.0f &&
                fHuBrain->fWalkingStrategy->IsOnGround())
            {
                if (fAvMod->ConsumeJump())
                    return true;
            }
        }
    }
    return false;
}

bool WalkingJump::PreCondition(double time, float elapsed)
{
    if (fAnim)
    {
        if (GetStrength() > 0.f)
        {
            if (!fHuBrain->fWalkingStrategy->IsControlledFlight() ||
                fAnim->GetTimeConvert()->WorldToAnimTimeNoUpdate(time) >= fAnim->GetTimeConvert()->GetEnd())
            {
                return false;
            }
            return !fAnim->IsFinished();
        } 
        else 
        {
            if (fAvMod->JumpKeyDown() &&
                !fAvMod->FastKeyDown() &&
                fAvMod->ForwardKeyDown() &&
                fAnim->GetBlend() == 0.0f &&
                fHuBrain->fWalkingStrategy->IsOnGround() &&
                (!fHuBrain->fWalkingStrategy->GetPushingPhysical() || !fHuBrain->fWalkingStrategy->GetFacingPushingPhysical()))
            {
                if (fAvMod->ConsumeJump())
                    return true;
            }
        }
    }
    return false;
}

bool RunningJump::PreCondition(double time, float elapsed)
{
    if (fAnim)
    {
        if (GetStrength() > 0.f)
        {
            if (!fHuBrain->fWalkingStrategy->IsControlledFlight() ||
                fAnim->GetTimeConvert()->WorldToAnimTimeNoUpdate(time) >= fAnim->GetTimeConvert()->GetEnd())
            {
                return false;
            }
            return !fAnim->IsFinished();
        } 
        else 
        {
            if (fAvMod->JumpKeyDown() &&
                fAvMod->ForwardKeyDown() &&
                fAvMod->FastKeyDown() &&
                fAnim->GetBlend() == 0.0f &&
                fHuBrain->fWalkingStrategy->IsOnGround() &&
                (!fHuBrain->fWalkingStrategy->GetPushingPhysical() || !fHuBrain->fWalkingStrategy->GetFacingPushingPhysical()))
            {
                if (fAvMod->ConsumeJump())
                    return true;
            }
        }
    }
    return false;
}


static const float kRunningImpactThresh = -1.0f;
static const float kFullImpactVel = 30.0f; // At this velocity (or greater) we blend the impact at full strength.
static const float kMinImpactVel = 10.f;

// If we just test IsOnGround(), we do a lot of impacts while running down stairs, so the impact
// behaviors have a more forgiving threshold.
static const float kMinAirTime = .5f;

RunningImpact::RunningImpact() : fDuration(0.0f) {}

bool RunningImpact::PreCondition(double time, float elapsed)
{   
    if (fDuration > 0.0f)
        fDuration = fDuration - elapsed;
    else if (fHuBrain->fWalkingStrategy->IsOnGround() && fHuBrain->fWalkingStrategy->GetImpactTime() > kMinAirTime) 
    {
        if (fHuBrain->fWalkingStrategy->GetImpactVelocity().fZ < -kMinImpactVel)
        {
            if (fHuBrain->fWalkingStrategy->GetImpactVelocity().fY < kRunningImpactThresh)
            {
                fMaxBlend = 0.5f + (0.5f * (-fHuBrain->fWalkingStrategy->GetImpactVelocity().fZ / (kFullImpactVel - kMinImpactVel)));
                if (fMaxBlend > 1)
                    fMaxBlend = 1;
                fDuration = 1.0f / fFadeIn;
            }
        }
    }
    return(fDuration > 0.0f);
}

void RunningImpact::IStop()
{
    fDuration = 0.0f;
    plHBehavior::IStop();
}

GroundImpact::GroundImpact() : fDuration(0.0f) {}

bool GroundImpact::PreCondition(double time, float elapsed)
{
    if (fDuration > 0.0f)
        fDuration = fDuration - elapsed;
    else if (fHuBrain->fWalkingStrategy->IsOnGround() && fHuBrain->fWalkingStrategy->GetImpactTime() > kMinAirTime) 
    {
        if (fHuBrain->fWalkingStrategy->GetImpactVelocity().fZ < -kMinImpactVel)
        {
            if (fHuBrain->fWalkingStrategy->GetImpactVelocity().fY >= kRunningImpactThresh)
            {
                fMaxBlend = 0.5f + (0.5f * (-fHuBrain->fWalkingStrategy->GetImpactVelocity().fZ / (kFullImpactVel - kMinImpactVel)));
                if (fMaxBlend > 1)
                    fMaxBlend = 1;
                fDuration = 1.0f / fFadeIn;
            }
        }
    }
    
    return(fDuration > 0.0f);
}

void GroundImpact::IStop()
{
    fDuration = 0.0f;
    plHBehavior::IStop();
}

bool Fall::PreCondition(double time, float elapsed)
{
    return !fHuBrain->fWalkingStrategy->IsOnGround() && fHuBrain->fWalkingStrategy->HitGroundInThisAge();
}

void Fall::Process(double time, float elapsed)
{
    // We don't see remote players panic link (from timeouts) because we don't know if they're
    // really falling, or if our understanding of their physical location is just not up-to-date.
    if (plAvatarMgr::GetInstance()->GetLocalAvatar() == fAvMod)
    {
        if (fAnim && fAnim->GetBlend() > 0.8)
        {
            float panicThresh = plAvBrainHuman::kAirTimePanicThreshold;
            if (panicThresh > 0.0f && fHuBrain->fWalkingStrategy->GetAirTime() > panicThresh)
            {
                fHuBrain->IdleOnly();   // clear the fall state; we're going somewhere new
                fAvMod->PanicLink();
            } 
        }
    }
}

extern float QuatAngleDiff(const hsQuat &a, const hsQuat &b);
void Push::Process(double time, float elapsed)
{
    hsQuat rot;
    hsPoint3 pos;
    fAvMod->GetPositionAndRotationSim(&pos, &rot);

    hsPoint3 lookAt;
    fHuBrain->fWalkingStrategy->GetPushingPhysical()->GetPositionSim(lookAt);
    hsVector3 up(0.f, 0.f, 1.f);
    float angle = std::atan2(lookAt.fY - pos.fY, lookAt.fX - pos.fX) + hsConstants::half_pi<float>;
    hsQuat targRot(angle, &up);
    
    const float kTurnSpeed = 3.f;
    float angDiff = QuatAngleDiff(rot, targRot);
    float turnSpeed = (angDiff > elapsed * kTurnSpeed ? kTurnSpeed : angDiff / elapsed);

    hsQuat invRot = targRot.Conjugate();
    hsPoint3 globFwd = invRot.Rotate(&kAvatarForward);
    globFwd = rot.Rotate(&globFwd);
        
    if (globFwd.fX < 0)
        fHuBrain->fWalkingStrategy->SetTurnStrength(-turnSpeed);
    else
        fHuBrain->fWalkingStrategy->SetTurnStrength(turnSpeed);
}
    
//bool PushIdle::PreCondition(double time, float elapsed)
//{
//  return (fHuBrain->fWalkingStrategy->GetPushingPhysical() &&
//          fHuBrain->fWalkingStrategy->IsOnGround() && 
//          !fAvMod->TurnLeftKeyDown() && !fAvMod->TurnRightKeyDown()
//          && fAvMod->GetTurnStrength() == 0);
//}

bool PushWalk::PreCondition(double time, float elapsed)
{
    return (fHuBrain->fWalkingStrategy->GetPushingPhysical() && fHuBrain->fWalkingStrategy->GetFacingPushingPhysical() &&
            fHuBrain->fWalkingStrategy->IsOnGround() &&
            fAvMod->ForwardKeyDown());
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// UTIL FUNCTIONS
//
/////////////////////////////////////////////////////////////////////////////////////////

static bool CanPushGenericBrain(plArmatureMod* avatar, const std::vector<ST::string>& anims, plAvBrainGeneric::BrainType type)
{
    plAvBrainHuman *huBrain = plAvBrainHuman::ConvertNoRef(avatar->FindBrainByClass(plAvBrainHuman::Index()));
    if (!huBrain || !huBrain->fWalkingStrategy->IsOnGround() || !huBrain->fWalkingStrategy->HitGroundInThisAge() || huBrain->IsRunningTask() ||
        !avatar->IsPhysicsEnabled() || avatar->FindMatchingGenericBrain(anims))
        return false;

    // XXX
    if (type == plAvBrainGeneric::kSit || type == plAvBrainGeneric::kSitOnGround)
    {
        plAvBrainSwim *swimBrain = plAvBrainSwim::ConvertNoRef(avatar->GetCurrentBrain());
        if (swimBrain && !swimBrain->IsWalking())
            return false;
    }

    // still here??? W00T!
    return true;
}

bool PushSimpleMultiStage(plArmatureMod *avatar, const ST::string& enterAnim, const ST::string& idleAnim, const ST::string& exitAnim,
                          bool netPropagate, bool autoExit, plAGAnim::BodyUsage bodyUsage, plAvBrainGeneric::BrainType type /* = kGeneric */)
{
    if (!CanPushGenericBrain(avatar, {enterAnim, idleAnim, exitAnim}, type))
        return false;

    // if autoExit is true, then we will immediately exit the idle loop when the user hits a move
    // key. otherwise, we'll loop until someone sends a message telling us explicitly to advance
    plAnimStage::AdvanceType idleAdvance = autoExit ? plAnimStage::kAdvanceOnMove : plAnimStage::kAdvanceNone;

    plAnimStageVec *v = new plAnimStageVec;
    plAnimStage *s1 = new plAnimStage(enterAnim,
                                      0,
                                      plAnimStage::kForwardAuto, plAnimStage::kBackNone,
                                      plAnimStage::kAdvanceAuto, plAnimStage::kRegressNone,
                                      0);
    v->push_back(s1);
    plAnimStage *s2 = new plAnimStage(idleAnim, 0,
                                      plAnimStage::kForwardAuto, plAnimStage::kBackNone,
                                      idleAdvance, plAnimStage::kRegressNone,
                                      -1);
    v->push_back(s2);
    plAnimStage *s3 = new plAnimStage(exitAnim, 0,
                                      plAnimStage::kForwardAuto, plAnimStage::kBackNone,
                                      plAnimStage::kAdvanceAuto, plAnimStage::kRegressNone,
                                      0);
    v->push_back(s3);

    plAvBrainGeneric *b = new plAvBrainGeneric(v, nullptr, nullptr, nullptr, plAvBrainGeneric::kExitAnyTask | plAvBrainGeneric::kExitNewBrain,
                                               2.0f, 2.0f, plAvBrainGeneric::kMoveStandstill);

    b->SetBodyUsage(bodyUsage);
    b->SetType(type);

    plAvTaskBrain *bt = new plAvTaskBrain(b);
    plAvTaskMsg *btm = new plAvTaskMsg(plAvatarMgr::GetInstance()->GetKey(), avatar->GetKey(), bt);
    if(netPropagate)
        btm->SetBCastFlag(plMessage::kNetPropagate);
    btm->Send();

    return true;
}

bool PushRepeatEmote(plArmatureMod* avatar, const ST::string& anim)
{
    if (!CanPushGenericBrain(avatar, {anim}, plAvBrainGeneric::kGeneric))
        return false;

     plAnimStageVec* v = new plAnimStageVec;
     plAnimStage* theStage = new plAnimStage(anim, 0,
                                             plAnimStage::kForwardAuto, plAnimStage::kBackNone,
                                             plAnimStage::kAdvanceOnMove, plAnimStage::kRegressNone,
                                             -1);
     v->push_back(theStage);

    plAvBrainGeneric* b = new plAvBrainGeneric(v, nullptr, nullptr, nullptr, plAvBrainGeneric::kExitAnyTask | plAvBrainGeneric::kExitNewBrain,
                                               2.0f, 2.0f, plAvBrainGeneric::kMoveStandstill);

    b->SetBodyUsage(plAGAnim::kBodyFull);
    b->SetType(plAvBrainGeneric::kGeneric);

    plAvTaskBrain* bt = new plAvTaskBrain(b);
    plAvTaskMsg* btm = new plAvTaskMsg(plAvatarMgr::GetInstance()->GetKey(), avatar->GetKey(), bt);
    btm->SetBCastFlag(plMessage::kNetPropagate, true);
    btm->Send();

    return true;
}

bool AvatarEmote(plArmatureMod *avatar, const ST::string& emoteName)
{
    bool result = false;
    ST::string fullName = avatar->MakeAnimationName(emoteName);
    plAGAnim *anim = plAGAnim::FindAnim(fullName);
    plEmoteAnim *emote = plEmoteAnim::ConvertNoRef(anim);
    bool alreadyActive = avatar->FindAnimInstance(fullName) != nullptr;
    plAvBrainHuman *huBrain = plAvBrainHuman::ConvertNoRef(avatar->FindBrainByClass(plAvBrainHuman::Index()));

    // XXX
    plAvBrainSwim *swimBrain = plAvBrainSwim::ConvertNoRef(avatar->GetCurrentBrain());
    if (swimBrain && swimBrain->IsSwimming())
        return false;
    
    if (huBrain && huBrain->fWalkingStrategy->IsOnGround() && huBrain->fWalkingStrategy->HitGroundInThisAge() && !huBrain->IsRunningTask() && 
        emote && !alreadyActive && avatar->IsPhysicsEnabled())
    {
        plAnimStage *s1 = new plAnimStage(emoteName,
                                          0,
                                          plAnimStage::kForwardAuto,
                                          plAnimStage::kBackNone,
                                          plAnimStage::kAdvanceOnMove,
                                          plAnimStage::kRegressNone,
                                          0);
        plAnimStageVec *v = new plAnimStageVec;
        v->push_back(s1);

        plAvBrainGeneric *b = new plAvBrainGeneric(v, nullptr, nullptr, nullptr,
                                                   plAvBrainGeneric::kExitAnyInput | plAvBrainGeneric::kExitNewBrain | plAvBrainGeneric::kExitAnyTask, 
                                                   2.0f, 2.0f, huBrain->IsActor() ? plAvBrainGeneric::kMoveRelative : plAvBrainGeneric::kMoveStandstill);
        b->SetType(plAvBrainGeneric::kEmote);
        b->SetBodyUsage(emote->GetBodyUsage());
        plAvTaskBrain *bt = new plAvTaskBrain(b);
        plAvTaskMsg *btm = new plAvTaskMsg(plAvatarMgr::GetInstance()->GetKey(), avatar->GetKey(), bt);
        btm->SetBCastFlag(plMessage::kNetPropagate);
        btm->Send();

        result = true;
    }
    return result;
}

