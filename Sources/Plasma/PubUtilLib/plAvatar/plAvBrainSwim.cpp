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
#include "plAvBrainSwim.h"

// local
#include "plArmatureMod.h"
#include "plAvBehaviors.h"
#include "plAvBrainHuman.h"
#include "plAGAnim.h"
#include "plAvBrainDrive.h"
#include "plMatrixChannel.h"
#include "plSwimRegion.h"
#include "plAvatarTasks.h"
#include "plArmatureEffects.h"
#include "plAvTaskBrain.h"
// global
#include "hsQuat.h"
#include "hsTimer.h"
#include "plPhysical.h"
#include "plPhysicalControllerCore.h"
// other
#include "plPhysical/plCollisionDetector.h"
#include "plPipeline/plDebugText.h"

#include "plMessage/plAvatarMsg.h"
#include "plMessage/plSwimMsg.h"
#include "plMessage/plLOSRequestMsg.h"
#include "plMessage/plLOSHitMsg.h"
#include "plMessage/plInputEventMsg.h"
#include "plMessage/plSimStateMsg.h"
#include "pnMessage/plCameraMsg.h"
#include "pfMessage/plArmatureEffectMsg.h"

class plSwimBehavior : public plArmatureBehavior
{
    friend class plAvBrainSwim;

public: 
    plSwimBehavior() : fAvMod(nil), fSwimBrain(nil) {}
    virtual ~plSwimBehavior() {}
    
    void Init(plAGAnim *anim, bool loop, plAvBrainSwim *brain, plArmatureMod *body, uint8_t index)
    {
        plArmatureBehavior::Init(anim, loop, brain, body, index);
        fAvMod = body;
        fSwimBrain = brain;
    }
    
    virtual bool PreCondition(double time, float elapsed) { return true; }
    
protected:
    virtual void IStart()
    {
        plArmatureBehavior::IStart();
        fAvMod->SynchIfLocal(hsTimer::GetSysSeconds(), false);
    }
    
    virtual void IStop()
    {
        plArmatureBehavior::IStop();
        fAvMod->SynchIfLocal(hsTimer::GetSysSeconds(), false);
    }       
    
    plArmatureMod *fAvMod;
    plAvBrainSwim *fSwimBrain;
};

class SwimForward: public plSwimBehavior
{
public:
    /** Walk key is down, fast key is not down */
    virtual bool PreCondition(double time, float elapsed)
    {
        return (fAvMod->ForwardKeyDown() && !fAvMod->FastKeyDown());
    }
};

class SwimForwardFast: public plSwimBehavior
{
public:
    virtual bool PreCondition(double time, float elapsed)
    {
        return (fAvMod->ForwardKeyDown() && fAvMod->FastKeyDown());
    }
};

class SwimBack : public plSwimBehavior
{
public:
    virtual bool PreCondition(double time, float elapsed)
    {
        return (fAvMod->BackwardKeyDown());
    }
};

class TreadWater: public plSwimBehavior
{
};

class SwimLeft : public plSwimBehavior
{
public:
    virtual bool PreCondition(double time, float elapsed)
    {
        return ((fAvMod->StrafeLeftKeyDown() || (fAvMod->StrafeKeyDown() && fAvMod->TurnLeftKeyDown())) &&
                !(fAvMod->StrafeRightKeyDown() || (fAvMod->StrafeKeyDown() && fAvMod->TurnRightKeyDown())) &&
                !(fAvMod->ForwardKeyDown() || fAvMod->BackwardKeyDown()));
    }
};

class SwimRight : public plSwimBehavior
{
public:
    virtual bool PreCondition(double time, float elapsed)
    {
        return ((fAvMod->StrafeRightKeyDown() || (fAvMod->StrafeKeyDown() && fAvMod->TurnRightKeyDown())) &&
                !(fAvMod->StrafeLeftKeyDown() || (fAvMod->StrafeKeyDown() && fAvMod->TurnLeftKeyDown())) &&
                !(fAvMod->ForwardKeyDown() || fAvMod->BackwardKeyDown()));
    }
};

class SwimTurn: public plSwimBehavior
{
public:
    virtual void Process(double time, float elapsed)
    {
        static const float maxTurnSpeed = 1.0f;         // radians per second;
        static const float timeToMaxTurn = 0.5f;
        static const float incPerSec = maxTurnSpeed / timeToMaxTurn;

        float oldSpeed = fabs(fSwimBrain->fSwimStrategy->GetTurnStrength());
        float thisInc = elapsed * incPerSec;
        float newSpeed = min(oldSpeed + thisInc, maxTurnSpeed);
        fSwimBrain->fSwimStrategy->SetTurnStrength(newSpeed * fAvMod->GetKeyTurnStrength() + fAvMod->GetAnalogTurnStrength());
        // the turn is actually applied during PhysicsUpdate
    }
    virtual void IStop()
    {
        if (fSwimBrain->fSwimStrategy)
            fSwimBrain->fSwimStrategy->SetTurnStrength(0.0f);
        plSwimBehavior::IStop();
    }
};

class SwimTurnLeft : public SwimTurn
{
public:
    virtual bool PreCondition(double time, float elapsed)
    {
        return (fAvMod->GetTurnStrength() > 0 && (fAvMod->ForwardKeyDown() || fAvMod->BackwardKeyDown()));
    }
};

class SwimTurnRight : public SwimTurn
{
public:
    virtual bool PreCondition(double time, float elapsed)
    {
        return (fAvMod->GetTurnStrength() < 0 && (fAvMod->ForwardKeyDown() || fAvMod->BackwardKeyDown()));
    }
};

class TreadTurnLeft : public plSwimBehavior
{
public:
    virtual bool PreCondition(double time, float elapsed)
    {
        return (fAvMod->TurnLeftKeyDown() && !fAvMod->ForwardKeyDown() && !fAvMod->BackwardKeyDown());
    }
};

class TreadTurnRight : public plSwimBehavior
{
public:
    virtual bool PreCondition(double time, float elapsed)
    {
        return (fAvMod->TurnRightKeyDown() && !fAvMod->ForwardKeyDown() && !fAvMod->BackwardKeyDown());
    }
};  

///////////////////////////////////////////////////////////////////////////////////////////

const float plAvBrainSwim::kMinSwimDepth = 4.0f;

plAvBrainSwim::plAvBrainSwim() : 
    fSwimStrategy(nil),
    fMode(kWalking),
    fSurfaceDistance(0.f)
{
    fSurfaceProbeMsg = new plLOSRequestMsg();
    fSurfaceProbeMsg->SetReportType(plLOSRequestMsg::kReportHitOrMiss);
    fSurfaceProbeMsg->SetRequestType(plSimDefs::kLOSDBSwimRegion);
    fSurfaceProbeMsg->SetTestType(plLOSRequestMsg::kTestAny);
    fSurfaceProbeMsg->SetRequestID(plArmatureMod::kAvatarLOSSwimSurface);
}
    
plAvBrainSwim::~plAvBrainSwim()
{
    delete fSwimStrategy;
    fSwimStrategy = nil;

    fSurfaceProbeMsg->UnRef();

    int i;
    for (i = 0; i < fBehaviors.GetCount(); i++)
        delete fBehaviors[i];
}

bool plAvBrainSwim::Apply(double time, float elapsed)
{
    IProbeSurface();
    if (fMode == kWalking)
    {
        if (fSurfaceDistance >= 0.f)
        {
            fMode = kWading;

            plAvBrainHuman *huBrain = plAvBrainHuman::ConvertNoRef(fAvMod->GetNextBrain(this));
            if (huBrain && !huBrain->fWalkingStrategy->IsOnGround())
            {
                // We're jumping in! Trigger splash effect (sound)              
                plArmatureEffectMsg *msg = new plArmatureEffectMsg(fAvMod->GetArmatureEffects()->GetKey(), kTime);
                msg->fEventTime = (float)time;
                msg->fTriggerIdx = plArmatureMod::kImpact;

                plEventCallbackInterceptMsg *iMsg = new plEventCallbackInterceptMsg;
                iMsg->AddReceiver(fAvMod->GetArmatureEffects()->GetKey());
                iMsg->fEventTime = (float)time;
                iMsg->fEvent = kTime;
                iMsg->SetMessage(msg);
                iMsg->Send();
            }
        }
    }
    
    plArmatureBrain *nextBrain = fAvMod->GetNextBrain(this);
    if (fMode == kWading)
    {
        if (fSurfaceDistance > kMinSwimDepth && fSurfaceDistance < 100.0f) 
            IStartSwimming(true);
        else if (fSurfaceDistance < 0.f)
            fMode = kWalking;
    } 

    int i;
    if (fMode == kWalking || fMode == kWading || nextBrain->IsRunningTask())
    {
        nextBrain->Apply(time, elapsed); // Let brain below process for us              

        for (i = 0; i < kSwimBehaviorMax; i++)
            fBehaviors[i]->SetStrength(0.f, 2.f);
    }
    else if (fMode == kAbort)
        return false;
    else 
    {
        if (fMode == kSwimming2D) 
        {
            IProcessSwimming2D(time, elapsed);

            // The contact check is so that if buoyancy bobs us a little too high, we don't
            // switch to wading only to fall again.
            if (fSurfaceDistance < kMinSwimDepth-.5  && fSwimStrategy->HadContacts())
                IStartWading();
        } 
        else if (fMode == kSwimming3D) 
            IProcessSwimming3D(time, elapsed);
    }
    return plArmatureBrain::Apply(time, elapsed);
}

bool plAvBrainSwim::MsgReceive(plMessage *msg)
{
    plLOSHitMsg *losHit = plLOSHitMsg::ConvertNoRef(msg);
    if (losHit)
    {
        if (losHit->fRequestID == plArmatureMod::kAvatarLOSSwimSurface)
        {
            plSwimRegionInterface *region = nil;
            if (!losHit->fNoHit)
            {
                plSceneObject *hitObj = plSceneObject::ConvertNoRef(losHit->fObj->ObjectIsLoaded());
                region = hitObj ? plSwimRegionInterface::ConvertNoRef(hitObj->GetGenericInterface(plSwimRegionInterface::Index())) : nil;
                //100-fDistance because of casting the ray from above to get around physxs Raycast requirments
                fSurfaceDistance = 100.f-losHit->fDistance;
            }
            else
                fSurfaceDistance = -100.f;

            if (fSwimStrategy)
            {
                if (region)
                    fSwimStrategy->SetSurface(region, fArmature->GetTarget(0)->GetLocalToWorld().GetTranslate().fZ + fSurfaceDistance);
                else
                    fSwimStrategy->SetSurface(nil, 0.f);
            }
            return true;
        }
    }
    
    plSwimMsg *swimMsg = plSwimMsg::ConvertNoRef(msg);
    if (swimMsg)
    {
        if (swimMsg->GetIsLeaving())
            fMode = kAbort;
        
        return true;
    }
    
    plControlEventMsg *ctrlMsg = plControlEventMsg::ConvertNoRef(msg);
    if (ctrlMsg)
        return IHandleControlMsg(ctrlMsg);  
    
    if (fMode == kWalking || fMode == kWading)
        return fAvMod->GetNextBrain(this)->MsgReceive(msg);         
    
    if (plAvSeekMsg *seekM = plAvSeekMsg::ConvertNoRef(msg))
    {
        
        // seek and subclasses always have a seek first
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
        return true;        
    }

    if (plArmatureBrain::MsgReceive(msg))
        return true;

    if (fMode == kWading) // Things like LOS need to go to the human brain below us.
        return fAvMod->GetNextBrain(this)->MsgReceive(msg);
    
    return false;
}

void plAvBrainSwim::Activate(plArmatureModBase* avMod)
{
    plArmatureBrain::Activate(avMod);
    
    IInitAnimations();
    fSurfaceProbeMsg->SetSender(fAvMod->GetKey());
    
    // turn our underlying brain back on until we're all the way in the water.
    fAvMod->GetNextBrain(this)->Resume();
}

void plAvBrainSwim::Deactivate()
{
    plArmatureBrain::Deactivate();
}

void plAvBrainSwim::Suspend()
{
}

void plAvBrainSwim::Resume()
{
    if (fMode == kSwimming2D)
        fSwimStrategy->Reset(false);
}

bool plAvBrainSwim::IsWalking()
{
    return fMode == kWalking;
}

bool plAvBrainSwim::IsWading()
{
    return fMode == kWading;
}

bool plAvBrainSwim::IsSwimming()
{
    return (fMode == kSwimming2D || fMode == kSwimming3D);
}

void plAvBrainSwim::IStartWading()
{
    plArmatureBrain *nextBrain = fAvMod->GetNextBrain(this);
    nextBrain->Resume();
    fMode = kWading;
    
    int i;
    for (i = 0; i < fBehaviors.GetCount(); i++)
        fBehaviors[i]->SetStrength(0.f, 2.f);
    
    if (fAvMod->IsLocalAvatar())
    {
        plCameraMsg* pMsg = new plCameraMsg;
        pMsg->SetBCastFlag(plMessage::kBCastByExactType);
        pMsg->SetBCastFlag(plMessage::kNetPropagate, false);
        pMsg->SetCmd(plCameraMsg::kResponderUndoThirdPerson);
        pMsg->Send();
    }   
}

void plAvBrainSwim::IStartSwimming(bool is2D)
{
    // if we *were* wading, the next brain will be running as well. turn it off
    // if we weren't wading, there's no harm in suspending it redundantly.
    plArmatureBrain *nextBrain = fAvMod->GetNextBrain(this);
    nextBrain->Suspend();
    
    if (!fSwimStrategy)
    {
        plSceneObject * avObj = fArmature->GetTarget(0);
        plAGModifier *agMod = const_cast<plAGModifier*>(plAGModifier::ConvertNoRef(FindModifierByClass(avObj, plAGModifier::Index())));
        plPhysicalControllerCore *controller = fAvMod->GetController();
        fSwimStrategy = new plSwimStrategy(agMod->GetApplicator(kAGPinTransform), controller);
    }

    fSwimStrategy->Reset(false);

    if (is2D)
        fMode = kSwimming2D;
    else
        fMode = kSwimming3D;
    
    if (fAvMod->IsLocalAvatar())
    {
        plCameraMsg* pMsg = new plCameraMsg;
        pMsg->SetBCastFlag(plMessage::kBCastByExactType);
        pMsg->SetBCastFlag(plMessage::kNetPropagate, false);
        pMsg->SetCmd(plCameraMsg::kResponderSetThirdPerson);
        pMsg->Send();
    }   
}

bool plAvBrainSwim::IProcessSwimming2D(double time, float elapsed)
{
    int i;
    for (i = 0; i < fBehaviors.GetCount(); i++)
    {
        plSwimBehavior *behavior = (plSwimBehavior*)fBehaviors[i];
        if (behavior->PreCondition(time, elapsed) && !IsRunningTask())
        {
            behavior->SetStrength(1.f, 2.f);
            behavior->Process(time, elapsed);
        }   
        else
            behavior->SetStrength(0.f, 2.f);
    }

    fSwimStrategy->RecalcVelocity(time, elapsed);

    return true;
}

bool plAvBrainSwim::IProcessSwimming3D(double time, float elapsed)
{
    fAvMod->ApplyAnimations(time, elapsed);
    return true;
}

bool plAvBrainSwim::IInitAnimations()
{
    plAGAnim *treadWater = fAvMod->FindCustomAnim("SwimIdle");
    plAGAnim *swimForward = fAvMod->FindCustomAnim("SwimSlow");
    plAGAnim *swimForwardFast = fAvMod->FindCustomAnim("SwimFast");
    plAGAnim *swimBack = fAvMod->FindCustomAnim("SwimBackward");
    plAGAnim *swimLeft = fAvMod->FindCustomAnim("SideSwimLeft");
    plAGAnim *swimRight = fAvMod->FindCustomAnim("SideSwimRight");
    plAGAnim *treadWaterLeft = fAvMod->FindCustomAnim("TreadWaterTurnLeft");
    plAGAnim *treadWaterRight = fAvMod->FindCustomAnim("TreadWaterTurnRight");  

    static const float defaultFade = 2.0f;
    fBehaviors.SetCountAndZero(kSwimBehaviorMax);
    plSwimBehavior *behavior;
    fBehaviors[kTreadWater] = behavior = new TreadWater;
    behavior->Init(treadWater, true, this, fAvMod, kTreadWater);

    fBehaviors[kSwimForward] = behavior = new SwimForward;
    behavior->Init(swimForward, true, this, fAvMod, kSwimForward);

    fBehaviors[kSwimForwardFast] = behavior = new SwimForwardFast;
    behavior->Init(swimForwardFast, true, this, fAvMod, kSwimForwardFast);

    fBehaviors[kSwimBack] = behavior = new SwimBack;        
    behavior->Init(swimBack, true, this, fAvMod, kSwimBack);
    
    fBehaviors[kSwimLeft] = behavior = new SwimLeft;
    behavior->Init(swimLeft, true, this, fAvMod, kSwimLeft);
    
    fBehaviors[kSwimRight] = behavior = new SwimRight;      
    behavior->Init(swimRight, true, this, fAvMod, kSwimRight);
    
    fBehaviors[kSwimTurnLeft] = behavior = new SwimTurnLeft;
    behavior->Init(nil, true, this, fAvMod, kSwimTurnLeft);
    
    fBehaviors[kSwimTurnRight] = behavior = new SwimTurnRight;
    behavior->Init(nil, true, this, fAvMod, kSwimTurnRight);
    
    fBehaviors[kTreadTurnLeft] = behavior = new TreadTurnLeft;
    behavior->Init(treadWaterLeft, true, this, fAvMod, kTreadTurnLeft);
    
    fBehaviors[kTreadTurnRight] = behavior = new TreadTurnRight;
    behavior->Init(treadWaterRight, true, this, fAvMod, kTreadTurnRight);
    
    return true;
}

void plAvBrainSwim::IProbeSurface()
{
    hsPoint3 ourPos = fAvMod->GetTarget(0)->GetLocalToWorld().GetTranslate();
    hsPoint3 up = ourPos;
    up.fZ += 100;
    fSurfaceProbeMsg->SetFrom(up);
    fSurfaceProbeMsg->SetTo(ourPos);
    fSurfaceProbeMsg->SendAndKeep();
}

bool plAvBrainSwim::IHandleControlMsg(plControlEventMsg* msg)
{
    ControlEventCode moveCode = msg->GetControlCode();
    if (msg->ControlActivated())
    {
        switch (moveCode)
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
        }
    }
    return false;
}


void plAvBrainSwim::DumpToDebugDisplay(int &x, int &y, int lineHeight, char *strBuf, plDebugText &debugTxt)
{
    sprintf(strBuf, "Brain type: Swim");
    debugTxt.DrawString(x, y, strBuf, 0, 255, 255);
    y += lineHeight;
    
    switch(fMode) {
        case kWading:
            sprintf(strBuf, "Mode: Wading");
            break;
        case kSwimming2D:
            sprintf(strBuf, "Mode: Swimming2D");
            break;
        case kSwimming3D:
            sprintf(strBuf, "Mode: Swimming3D");
            break;
        case kAbort:
            sprintf(strBuf, "Mode: Abort (you should never see this)");
            break;
    }
    debugTxt.DrawString(x, y, strBuf);
    y += lineHeight;

    float buoy = fSwimStrategy ? fSwimStrategy->GetBuoyancy() : 0.0f;
    sprintf(strBuf, "Distance to surface: %f Buoyancy: %f", fSurfaceDistance, buoy);
    debugTxt.DrawString(x, y, strBuf);
    y += lineHeight;

    hsVector3 linV = fAvMod->GetController()->GetAchievedLinearVelocity();
    sprintf(strBuf, "Linear Velocity: (%5.2f, %5.2f, %5.2f)", linV.fX, linV.fY, linV.fZ);
    debugTxt.DrawString(x, y, strBuf);
    y += lineHeight;
    
    int i;
    for (i = 0; i < fBehaviors.GetCount(); i++)
        fBehaviors[i]->DumpDebug(x, y, lineHeight, strBuf, debugTxt);
}
