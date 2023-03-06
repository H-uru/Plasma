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
#include "plAvBrainGeneric.h"

// local
#include "plAnimStage.h"
#include "plArmatureMod.h"
#include "plAvatarMgr.h"
#include "plAvBrainHuman.h"
#include "plAvTask.h"
#include "plAvTaskBrain.h"

// global
#include "hsTimer.h"
#include "plgDispatch.h"

// other
#include "pnMessage/plCameraMsg.h"
#include "pnMessage/plNotifyMsg.h"
#include "pnNetCommon/plSDLTypes.h"

#include "plAnimation/plAGAnimInstance.h"
#include "plAnimation/plMatrixChannel.h"
#include "plInputCore/plAvatarInputInterface.h"
#include "plMessage/plAvatarMsg.h"
#include "plMessage/plInputEventMsg.h"
#include "plMessage/plInputIfaceMgrMsg.h"
#include "plPipeline/plDebugText.h"

bool plAvBrainGeneric::fForce3rdPerson = true;
const float plAvBrainGeneric::kDefaultFadeIn = 6.f; // 1/6th of a second to fade in
const float plAvBrainGeneric::kDefaultFadeOut = 0.f; // instant fade out.

// plAvBrainGeneric ----------------
// -----------------
plAvBrainGeneric::plAvBrainGeneric()
: fStages(new plAnimStageVec),
  fCurStage(),
  fType(kGeneric),
  fExitFlags(kExitNormal),
  fMode(kEntering),
  fForward(true),
  fStartMessage(),
  fEndMessage(),
  fFadeIn(),
  fFadeOut(),
  fMoveMode(kMoveRelative),
  fBodyUsage(plAGAnim::kBodyUnknown)
{
}

// plAvBrainGeneric --------------------------------------
// -----------------
plAvBrainGeneric::plAvBrainGeneric(plAnimStageVec *stages,
                                   plMessage *startMessage,
                                   plMessage *endMessage,
                                   plKey recipient,
                                   uint32_t exitFlags,
                                   float fadeIn,
                                   float fadeOut,
                                   MoveMode moveMode)
: plArmatureBrain(),
  fRecipient(std::move(recipient)),
  fStages(stages),
  fCurStage(),
  fType(kGeneric),
  fExitFlags(exitFlags),
  fMode(kEntering),
  fForward(true),
  fStartMessage(startMessage),
  fEndMessage(endMessage),
  fFadeIn(fadeIn),
  fFadeOut(fadeOut),
  fMoveMode(moveMode),
  fBodyUsage(plAGAnim::kBodyUnknown)
{
}

// plAvBrainGeneric 
plAvBrainGeneric::plAvBrainGeneric(uint32_t exitFlags, float fadeIn, float fadeOut, MoveMode moveMode)
: fStages(),
  fCurStage(),
  fType(kGeneric),
  fExitFlags(exitFlags),
  fMode(kEntering),
  fForward(true),
  fStartMessage(),
  fEndMessage(),
  fFadeIn(fadeIn),
  fFadeOut(fadeOut),
  fMoveMode(moveMode),
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
        (*fStages)[i] = nullptr;
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
            fStartMessage = nullptr;
        }
        
        if (plAvBrainGeneric::fForce3rdPerson && fAvMod->IsLocalAvatar())
        {
            // create message to force 3rd person mode
            plCameraMsg* pMsg = new plCameraMsg;
            pMsg->SetBCastFlag(plMessage::kBCastByExactType);
            pMsg->SetCmd(plCameraMsg::kResponderSetThirdPerson);
            pMsg->SetBCastFlag(plMessage::kNetPropagate, false);
            plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
        }
        
    }

    if (fType == kLadder && fAvMod->IsLocalAvatar())
        plAvatarInputInterface::GetInstance()->SetLadderMode(); 

    if (fReverseFBControlsOnRelease)
        fAvMod->SetReverseFBOnIdle(true);
}

bool plAvBrainGeneric::IsRunningTask() const
{
    if ( fStages->size() > 0 )
        return true;
    return false;
}

bool plAvBrainGeneric::MatchAnimNames(const std::vector<ST::string>& names)
{
    if (names.size() != GetStageCount())
        return false;

    for (size_t i = 0; i < names.size(); i++)
    {
        if (GetStage(i)->GetAnimName() != names[i])
            return false; // different names.
    }

    return true;
}

// Apply ----------------------------------------------------
// ------
bool plAvBrainGeneric::Apply(double time, float elapsed)
{
    bool result = false;

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
    default:
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
        fEndMessage = nullptr;
    }
    if (fMode != kAbort)        // we're being forcibly removed...
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
        plCameraMsg* pMsg = new plCameraMsg;
        pMsg->SetBCastFlag(plMessage::kBCastByExactType);
        pMsg->SetBCastFlag(plMessage::kNetPropagate, false);
        pMsg->SetCmd(plCameraMsg::kResponderUndoThirdPerson);
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
        
    plArmatureBrain::Deactivate();
}

// GETRECIPIENT
plKey plAvBrainGeneric::GetRecipient()
{
    return fRecipient;
}

// SETRECIPIENT
void plAvBrainGeneric::SetRecipient(plKey recipient)
{
    fRecipient = std::move(recipient);
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
    bool backIsDown = (fReverseFBControlsOnRelease && fAvMod->IsFBReversed()) ? fAvMod->ForwardKeyDown() : fAvMod->BackwardKeyDown();

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
bool plAvBrainGeneric::IProcessNormal(double time, float elapsed)
{
    plAnimStage *curStage = (*fStages)[fCurStage];
    if(curStage)
    {
        float animDelta = IGetAnimDelta(time, elapsed);     // how far to move the anim (may be negative)
        float overage;
        bool done = curStage->MoveRelative(time, animDelta, overage, fAvMod);

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
bool plAvBrainGeneric::IProcessFadeIn(double time, float elapsed)
{
    plAnimStage *curStage = (*fStages)[fCurStage];

    if(fMode != kFadingIn)
    {
        if(fFadeIn == 0.0f)
        {
            IEnterMoveMode(time);   // if fadeIn's not zero, we have to wait until fade's done
                                            // before animating
        } else {
            plAGAnimInstance *curAnim = curStage->GetAnimInstance();
            if(curAnim)
                curAnim->Fade(1.0f, fFadeIn);
        }
        fMode = kFadingIn;
    } else {
        plAGAnimInstance *curAnim = curStage->GetAnimInstance();
        float curBlend = 1.0f;
        if(curAnim)
            curBlend = curAnim->GetBlend();
        if(curBlend == 1.0f)
        {
            IEnterMoveMode(time);
        }
    }
    return true;
}

// IProcessFadeOut -------------------------------------------------
// ----------------
bool plAvBrainGeneric::IProcessFadeOut(double time, float elapsed)
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
                curAnim->Fade(0.0f, fFadeOut);
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
        plAGAnimInstance *curAnim = curStage->GetAnimInstance();
        float curBlend = 0.0f;
        if(curAnim)
            curBlend = curAnim->GetBlend();
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
bool plAvBrainGeneric::ISwitchStages(int oldStageNum, int newStageNum, float delta, bool setTime, float newTime,
                                       float fadeNew, float fadeOld, double worldTime)
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
        fAvMod->DirtySynchState(kSDLAvatar, 0);     // write our new stage to the server
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
bool plAvBrainGeneric::MsgReceive(plMessage *msg)
{
    bool result = false;

    plAvBrainGenericMsg *genMsg = plAvBrainGenericMsg::ConvertNoRef(msg);
    //plAvExitModeMsg *exitMsg = plAvExitModeMsg::ConvertNoRef(msg);
    plAvTaskMsg *taskMsg = plAvTaskMsg::ConvertNoRef(msg);
    plControlEventMsg *ctrlMsg = plControlEventMsg::ConvertNoRef(msg);
    

    if(genMsg)
    {
        result = IHandleGenBrainMsg(genMsg);
    } 
//  else if(exitMsg) {
//      fMode = kExit;
//      result = true;
//  } 
    else if (taskMsg) {
        result =  IHandleTaskMsg(taskMsg);
    } 
    else if (ctrlMsg && (fExitFlags & kExitAnyInput) ) {
        fMode = kExit;
    }

    if(result == false)                                     // if still haven't handled msg
    {
        if(fMode == kExit)                                      // if we're exiting
        {
            result = fAvMod->GetNextBrain(this)->MsgReceive(msg);       // pass msg to next brain
        } else {                                                // otherwise
            result = plArmatureBrain::MsgReceive(msg);              // pass msg to base class
        }
    }
    
    return result;
}

// IHandleGenBrainMsg -----------------------------------------------------
// -------------------
bool plAvBrainGeneric::IHandleGenBrainMsg(const plAvBrainGenericMsg *msg)
{
    bool setTime = msg->fSetTime;
    float newTime = msg->fNewTime;
    bool setDirection = msg->fSetDirection;
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
                fMode = kExit;  // walked off the beginning of the brain
            } else {
                ISwitchStages(fCurStage, wantStage, 0.0f, setTime, 0.0f, 1.0f, -1.0f, worldTime);
                if(setDirection)
                    fForward = newDirection;
            }
        }
        break;
    default:
#ifdef DEBUG_MULTISTAGE
        {
            char sbuf[256];
            sprintf(sbuf,"GenericMsg - Unknown command %d ",msg->fType);
            plAvatarMgr::GetInstance()->GetLog()->AddLine(sbuf);
        }
#endif
        break;
    }
    return true;
}

bool plAvBrainGeneric::IHandleTaskMsg(plAvTaskMsg *msg)
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
                return false;       // we didn't consume the message
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
            //          make sure to consume it )
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
    case plAGAnim::kBodyFull:
        return false;
    case plAGAnim::kBodyUpper:
        if(otherUsage == plAGAnim::kBodyLower)
            return true;
        else
            return false;
    case plAGAnim::kBodyLower:
        if(otherUsage == plAGAnim::kBodyUpper)
            return true;
        else
            return false;
    default:
        return false;
    }
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
    stream->WriteLE32(numStages);

    for(int i = 0; i < numStages; i++)
    {
        plAnimStage *stage = (*fStages)[i];
        plCreatable *cre = reinterpret_cast<plCreatable *>(stage);
        mgr->WriteCreatable(stream, cre);       // save base state
        // ** replace this with Write(..)
        stage->SaveAux(stream, mgr);            // save ephemeral state 
    }

    stream->WriteLE32(fCurStage);
    stream->WriteLE32((uint32_t)fType);
    stream->WriteLE32(fExitFlags);
    stream->WriteByte((uint8_t)fMode);
    stream->WriteBool(fForward);

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

    stream->WriteLEFloat(fFadeIn);
    stream->WriteLEFloat(fFadeOut);
    stream->WriteByte((uint8_t)fMoveMode);
    stream->WriteByte((uint8_t)fBodyUsage);
    mgr->WriteKey(stream, fRecipient);
}

// Read ----------------------------------------------------
// -----
void plAvBrainGeneric::Read(hsStream *stream, hsResMgr *mgr)
{
    plArmatureBrain::Read(stream, mgr);
    int numStages = stream->ReadLE32();

    for(int i = 0; i < numStages; i++)
    {
        plCreatable *created = mgr->ReadCreatable(stream);              // load base state
        plAnimStage *stage = reinterpret_cast<plAnimStage *>(created);
        // Replace this with Read(..)
        stage->LoadAux(stream, mgr, 0.0);                               // load ephemeral state

        fStages->push_back(stage);
    }

    fCurStage = stream->ReadLE32();
    fType = static_cast<plAvBrainGeneric::BrainType>(stream->ReadLE32());
    fExitFlags = stream->ReadLE32();
    fMode = static_cast<Mode>(stream->ReadByte());
    fForward = stream->ReadBool();

    if(stream->ReadBool()) {
        fStartMessage = plMessage::ConvertNoRef(mgr->ReadCreatable(stream));
    } else {
        fStartMessage = nullptr;
    }
    if(stream->ReadBool()) {
        fEndMessage = plMessage::ConvertNoRef(mgr->ReadCreatable(stream));
    } else {
        fEndMessage = nullptr;
    }

    fFadeIn = stream->ReadLEFloat();
    fFadeOut = stream->ReadLEFloat();
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
bool plAvBrainGeneric::LeaveAge()
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
        fStages = new plAnimStageVec;
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
void plAvBrainGeneric::DumpToDebugDisplay(int &x, int &y, int lineHeight, plDebugText &debugTxt)
{
    debugTxt.DrawString(x, y, "Brain type: Generic AKA Multistage");
    y += lineHeight;

    int stageCount = fStages->size();
    for(int i = 0; i < stageCount; i++)
    {
        plAnimStage *stage = (*fStages)[i];
        stage->DumpDebug(i == fCurStage, x, y, lineHeight, debugTxt);
    }
}




