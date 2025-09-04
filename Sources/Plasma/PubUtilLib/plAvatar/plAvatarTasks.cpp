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
#include "plAvatarTasks.h"

// local
#include "plArmatureMod.h"
#include "plSeekPointMod.h"
#include "plAvBrainHuman.h"
#include "plPhysicalControllerCore.h"
#include "plAvatarMgr.h"

// global
#include "HeadSpin.h"
#include "plgDispatch.h"

#include <string_theory/format>

// other
#include "pnKeyedObject/plKey.h"
#include "pnMessage/plCameraMsg.h"
#include "pnSceneObject/plCoordinateInterface.h"

#include "plAnimation/plAGAnim.h"
#include "plAnimation/plAGAnimInstance.h"
#include "plAnimation/plAGModifier.h"
#include "plAnimation/plMatrixChannel.h"
#include "plInputCore/plInputInterfaceMgr.h"
#include "plInterp/plAnimTimeConvert.h"
#include "plMessage/plAnimCmdMsg.h"
#include "plMessage/plAvatarMsg.h"
#include "plMessage/plConsoleMsg.h"
#include "plMessage/plLinkToAgeMsg.h"
#include "plMessage/plOneShotCallbacks.h"
#include "plNetClient/plNetClientMgr.h"
#include "plNetCommon/plNetCommon.h"
#include "plPipeline/plDebugText.h"

#include "pfMessage/pfKIMsg.h"

// for console hack
bool plAvOneShotTask::fForce3rdPerson = true;

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
bool plAvTask::Start(plArmatureMod *avatar, plArmatureBrain *brain, double time, float elapsed)
{
    return true;    // true indicates the task has started succesfully
}

// PROCESS
bool plAvTask::Process(plArmatureMod *avatar, plArmatureBrain *brain, double time, float elapsed)
{
    return false;
}

// Finish -----------------------------------------------------------------------------------
// -------
void plAvTask::Finish(plArmatureMod *avatar, plArmatureBrain *brain, double time, float elapsed)
{
}


// DUMPDEBUG
void plAvTask::DumpDebug(const char *name, int &x, int&y, int lineHeight, plDebugText &debugTxt)
{
    debugTxt.DrawString(x, y, ST_LITERAL("<anonymous task>"));
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
        pfKIMsg* msg = new pfKIMsg(pfKIMsg::kTempDisableKIandBB);
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
        pfKIMsg* msg = new pfKIMsg(pfKIMsg::kTempEnableKIandBB);
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
    : fAlign(kAlignHandle),
      fDuration(0.25f),
      fTarget(),
      fAnimInstance(),
      fTargetTime(),
      fPhysicalAtStart(),
      fCleanup()
{
}

// CTOR target, align, animName
plAvSeekTask::plAvSeekTask(plKey target, plAvAlignment align, ST::string animName)
    : fAnimName(std::move(animName)),
      fAlign(align),
      fDuration(0.25f),
      fTarget(std::move(target)),
      fAnimInstance(),
      fTargetTime(),
      fPhysicalAtStart(),
      fCleanup()
{
}

// CTOR target
plAvSeekTask::plAvSeekTask(plKey target)
    : fAlign(kAlignHandle),
      fDuration(0.25f),
      fTarget(std::move(target)),
      fAnimInstance(),
      fTargetTime(),
      fPhysicalAtStart(),
      fCleanup()
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
bool plAvSeekTask::Start(plArmatureMod *avatar, plArmatureBrain *brain, double time, float elapsed)
{
    fTargetTime = time + fDuration;     // clock starts now....
    fPhysicalAtStart = avatar->IsPhysicsEnabled();
    avatar->EnablePhysics(false);       // always turn physics off for seek
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
    const plCoordinateInterface* subworldCI = nullptr;
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
                GetStartToEndTransform(anim, nullptr, &adjustment, "Handle");   // actually getting end-to-start
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
bool plAvSeekTask::Process(plArmatureMod *avatar, plArmatureBrain *brain, double time, float elapsed)
{
    hsQuat rotation;
    hsPoint3 position;
    avatar->GetPositionAndRotationSim(&position, &rotation);

    // We've had a history of odd bugs caused by assuming a rotation quat is normalized.
    // This line here seems to be fixing one of them. (Avatars scaling oddly when smart seeking.)
    rotation.Normalize();

    double timeToGo = fTargetTime - time - elapsed; // time from *beginning* of this interval to the goal
    if (fCleanup)
    {
        avatar->EnablePhysics( fPhysicalAtStart );
        IUndoLimitPlayersInput(avatar);
        
        return false;       // we're done processing
    }
    else if(timeToGo < .01)
    {
        fTargetRotation.Normalize();
        avatar->SetPositionAndRotationSim(&fTargetPosition, &fTargetRotation);
        fCleanup = true;    // we're going to wait one frame for the transform to propagate
        return true;        // still running until next frame/cleanup
    }
    else
    {
        hsPoint3 posToGo = fTargetPosition - position;          // vec from here to the goal
        float thisPercentage = (float)(elapsed / timeToGo);         

        hsPoint3 newPosition = position + posToGo * thisPercentage;
        hsQuat newRotation;
        newRotation.SetFromSlerp(rotation, fTargetRotation, thisPercentage);

        newRotation.Normalize();
        avatar->SetPositionAndRotationSim(&newPosition, &newRotation);
        return true;        // we're still processing
    }
}

void plAvSeekTask::LeaveAge(plArmatureMod *avatar)
{
    fTarget = nullptr;
    fCleanup = true;
}

///////////////////
//
// PLAVANIMTASK
//
///////////////////

// CTOR default
plAvAnimTask::plAvAnimTask()
: fInitialBlend(),
  fTargetBlend(),
  fFadeSpeed(),
  fSetTime(),
  fStart(),
  fLoop(),
  fAttach(),
  fAnimInstance()
{
}

// CTOR animName, initialBlend, targetBlend, fadeSpeed, start, loop, attach
plAvAnimTask::plAvAnimTask(const ST::string &animName,
                           float initialBlend,
                           float targetBlend,
                           float fadeSpeed,
                           float setTime,
                           bool start,
                           bool loop,
                           bool attach)
: fAnimName(animName),
  fInitialBlend(initialBlend),
  fTargetBlend(targetBlend),
  fFadeSpeed(fadeSpeed),
  fSetTime(setTime),
  fStart(start),
  fLoop(loop),
  fAttach(attach),
  fAnimInstance()
{
}

// CTOR animName, fadeSpeed, attach
plAvAnimTask::plAvAnimTask(const ST::string &animName, float fadeSpeed, bool attach)
: fAnimName(animName),
  fInitialBlend(),
  fTargetBlend(),
  fFadeSpeed(fadeSpeed),
  fSetTime(),
  fStart(),
  fLoop(),
  fAttach(attach),
  fAnimInstance()
{
}




// START
bool plAvAnimTask::Start(plArmatureMod *avatar, plArmatureBrain *brain, double time, float elapsed)
{
    bool result = false;
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
            hsStatusMessageF("Couldn't find animation <{}> for plAvAnimTask: will try again", fAnimName);
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
bool plAvAnimTask::Process(plArmatureMod *avatar, plArmatureBrain *brain, double time, float elapsed)
{
    // the only reason we need this function is to watch the animation until it fades out
    bool result = false;
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
    fInitialBlend = stream->ReadLEFloat();
    fTargetBlend = stream->ReadLEFloat();
    fFadeSpeed = stream->ReadLEFloat();
    fSetTime = stream->ReadLEFloat();
    fStart = stream->ReadBool();
    fLoop = stream->ReadBool();
    fAttach = stream->ReadBool();
}

// WRITE
void plAvAnimTask::Write(hsStream *stream, hsResMgr *mgr)
{
    stream->WriteSafeString(fAnimName);
    stream->WriteLEFloat(fInitialBlend);
    stream->WriteLEFloat(fTargetBlend);
    stream->WriteLEFloat(fFadeSpeed);
    stream->WriteLEFloat(fSetTime);
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
    fMoveHandle = false;
    fAnimInstance = nullptr;
    fDrivable = false;
    fReversible = false;
    fEnablePhysicsAtEnd = false;
    fDetachAnimation = false;
    fIgnore = false;
    fCallbacks = nullptr;
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
plAvOneShotTask::plAvOneShotTask(const ST::string &animName, bool drivable, bool reversible, plOneShotCallbacks *callbacks)
{
    InitDefaults();

    fDrivable = drivable;
    fReversible = reversible;
    fCallbacks = callbacks;
    
    // we're going to use this sometime in the future, better ref it so someone else doesn't release it
    hsRefCnt_SafeRef(fCallbacks);
    fAnimName = animName;
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
    fAnimName = msg->fAnimName;
}

// DTOR
plAvOneShotTask::~plAvOneShotTask()
{
    hsRefCnt_SafeUnRef(fCallbacks);
}


// START
bool plAvOneShotTask::Start(plArmatureMod *avatar, plArmatureBrain *brain, double time, float elapsed)
{
    bool result = false;

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
            plAvatarPhysicsEnableCallbackMsg *epMsg = new plAvatarPhysicsEnableCallbackMsg(avatar->GetKey(), plEventCallbackMsg::kStop, 0, 0, 0, 0);
            fAnimInstance->GetTimeConvert()->AddCallback(epMsg);
            hsRefCnt_SafeUnRef(epMsg);
        }   

        if (fCallbacks)
        {
            fAnimInstance->AttachCallbacks(fCallbacks);
            // ok, we're done with it, release it back to the river
            hsRefCnt_SafeUnRef(fCallbacks);
            fCallbacks = nullptr;
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
            
        fWaitFrames = 2;        // wait two frames after animation finishes before finalizing


        if (fDisablePhysics)
            avatar->EnablePhysics(false);

        ILimitPlayersInput(avatar);
        
        // this is for a console command hack
        if (plAvOneShotTask::fForce3rdPerson && avatar->IsLocalAvatar())
        {
            // create message
            plCameraMsg* pMsg = new plCameraMsg;
            pMsg->SetBCastFlag(plMessage::kBCastByExactType);
            pMsg->SetBCastFlag(plMessage::kNetPropagate, false);
            pMsg->SetCmd(plCameraMsg::kResponderSetThirdPerson);
            plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
        }

        fMoveHandle = (fAnimInstance->GetAnimation()->GetChannel("Handle") != nullptr);
        if(fMoveHandle)
        {
            plMatrixDifferenceApp *differ = avatar->GetRootAnimator();
            differ->Reset(time);        // throw away any old state
            differ->Enable(true);
        }

        avatar->ApplyAnimations(time, elapsed);                         

        result = true;
    }
    else
    {
        ST::string buf = ST::format("Oneshot: Can't find animation <{}>; all bets are off.", fAnimName);
        hsAssert(false, buf.c_str());
        result = true;
    }
    return result;
}

// PROCESS
bool plAvOneShotTask::Process(plArmatureMod *avatar, plArmatureBrain *brain, double time, float elapsed)
{
    // *** if we are under mouse control, adjust it here

    avatar->ApplyAnimations(time, elapsed);
    if(fAnimInstance)
    {
        if(fAnimInstance->IsFinished())
        {
            const plAGAnim * animation = fAnimInstance->GetAnimation();
            double endTime = (fBackwards ? animation->GetStart() : animation->GetEnd());
            fAnimInstance->SetCurrentTime((float)endTime);
            avatar->ApplyAnimations(time, elapsed);

            if(--fWaitFrames == 0)
            {
                avatar->DetachAnimation(fAnimInstance);
                avatar->GetRootAnimator()->Enable(false);
                plAvBrainHuman *humanBrain = plAvBrainHuman::ConvertNoRef(brain);
                if(fEnablePhysicsAtEnd)
                {
#if 0//ndef PLASMA_EXTERNAL_RELEASE
                    if (!humanBrain || humanBrain->fWalkingStrategy->HitGroundInThisAge())
                    {
                        // For some reason, calling CheckValidPosition at the beginning of
                        // an age can cause detectors to incorrectly report collisions. So
                        // we only call this if we're in the age.
                        // 
                        // It's only debugging code anyway to help the artist check that
                        // their oneshot doesn't end while penetrating geometry.
                        char *overlaps = nullptr;
                        if (avatar->GetPhysical())
                            avatar->GetPhysical()->CheckValidPosition(&overlaps);
                        if (overlaps)
                        {
                            char *buffy = new char[64 + strlen(overlaps)];
                            sprintf(buffy, "Oneshot ends overlapping %s", overlaps);
                            plConsoleMsg *showLine = new plConsoleMsg( plConsoleMsg::kAddLine, buffy );
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
                    plCameraMsg* pMsg = new plCameraMsg;
                    pMsg->SetBCastFlag(plMessage::kBCastByExactType);
                    pMsg->SetBCastFlag(plMessage::kNetPropagate, false);
                    pMsg->SetCmd(plCameraMsg::kResponderUndoThirdPerson);
                    plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
                }
                
                return false;
            }
            else
                return true;    // still running; waiting for fWaitFrames == 0
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

void plAvOneShotTask::SetAnimName(const ST::string &name)
{
    fAnimName = name;
}

//////////////////////
//
// PLAVONESHOTLINKTASK
//
//////////////////////

plAvOneShotLinkTask::plAvOneShotLinkTask() : plAvOneShotTask(), 
fMarkerTime(-1),
fStartTime(0),
fLinkFired(false)
{
    fDisablePhysics = false;        
}

plAvOneShotLinkTask::~plAvOneShotLinkTask()
{
}

// task protocol
bool plAvOneShotLinkTask::Start(plArmatureMod *avatar, plArmatureBrain *brain, double time, float elapsed)
{
    bool result = plAvOneShotTask::Start(avatar, brain, time, elapsed);
    fStartTime = time;

    if (fAnimInstance && !fMarkerName.empty())
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

bool plAvOneShotLinkTask::Process(plArmatureMod *avatar, plArmatureBrain *brain, double time, float elapsed)
{
    bool result = plAvOneShotTask::Process(avatar, brain, time, elapsed);
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

void plAvOneShotLinkTask::SetMarkerName(const ST::string &name)
{
    fMarkerName = name;
}














