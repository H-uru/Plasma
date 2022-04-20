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

/*
 Some interesting special rules:
    You can only transition from one animation to another at certain points.

    The climb, mount, and dismount animations must be at their beginning or end
    to transition.

    If a climb or mount animation finishes and the key is not being held down, the idle
    animation starts automatically.

    If a climb or mount finishes and the key is being held down, the brain will *try*
    to transition to the same stage, effectively looping it.

    The idle can transition at any point.

    The Release and FallOff aniamtions can forcibly transition *any* animation.
*/

/////////////////////////////////////////////////////////////////
//
// INCLUDES
//
/////////////////////////////////////////////////////////////////

// singular
#include "plAvBrainClimb.h"

// local
#include "plAnimStage.h"
#include "plArmatureMod.h"
#include "plAvBrainHuman.h"
#include "plAvatarSDLModifier.h"

// global
#include "hsTimer.h"

// other
#include "pnNetCommon/plSDLTypes.h"
#include "pnSceneObject/plSceneObject.h"

#include "plAnimation/plAGAnimInstance.h"
#include "plAnimation/plMatrixChannel.h"
#include "plMessage/plClimbEventMsg.h"
#include "plMessage/plLOSHitMsg.h"
#include "plMessage/plLOSRequestMsg.h"
#include "plPipeline/plDebugText.h"
#include "plSDL/plSDL.h"


/////////////////////////////////////////////////////////////////
//
// IMPLEMENTATION
//
/////////////////////////////////////////////////////////////////

// CTOR default
plAvBrainClimb::plAvBrainClimb()
: fCurMode(kInactive),
  fNextMode(kInactive),
  fDesiredDirection(plClimbMsg::kUp),
  fControlDir(),

  fAllowedDirections(plClimbMsg::kUp | plClimbMsg::kDown | plClimbMsg::kLeft | plClimbMsg::kRight),
  fPhysicallyBlockedDirections(),
  fOldPhysicallyBlockedDirections(),
  fAllowedDismounts(),

  fCurStage(),
  fExitStage(),
  fVerticalProbeLength(),
  fHorizontalProbeLength(),

  fUp(),
  fDown(),
  fLeft(),
  fRight(),
  fMountUp(),
  fMountDown(),
  fMountLeft(),
  fMountRight(),
  fDismountUp(),
  fDismountDown(),
  fDismountLeft(),
  fDismountRight(),
  fIdle(),
  fRelease(),
  fFallOff()
{
    IInitAnimations();
}

// PLAVBRAINCLIMB
plAvBrainClimb::plAvBrainClimb(Mode nextMode)
: fCurMode(kInactive),
  fNextMode(nextMode),
  fDesiredDirection(plClimbMsg::kUp),
  fControlDir(),

  fAllowedDirections(plClimbMsg::kUp | plClimbMsg::kDown | plClimbMsg::kLeft | plClimbMsg::kRight),
  fPhysicallyBlockedDirections(),
  fOldPhysicallyBlockedDirections(),
  fAllowedDismounts(),

  fCurStage(),
  fExitStage(),
  fVerticalProbeLength(),
  fHorizontalProbeLength(),

  fUp(),
  fDown(),
  fLeft(),
  fRight(),
  fMountUp(),
  fMountDown(),
  fMountLeft(),
  fMountRight(),
  fDismountUp(),
  fDismountDown(),
  fDismountLeft(),
  fDismountRight(),
  fIdle(),
  fRelease(),
  fFallOff()
{
    IInitAnimations();  
}


plAvBrainClimb::~plAvBrainClimb()
{
    if(fAvMod)
    {
        if(fCurStage)
            fCurStage->Detach(fAvMod);
        if(fExitStage)
            fExitStage->Detach(fAvMod);
    }
    delete fUp;
    delete fDown;
    delete fLeft;
    delete fRight;
    delete fMountUp;
    delete fMountDown;
    delete fMountLeft;
    delete fMountRight;
    delete fDismountUp;
    delete fDismountDown;
    delete fDismountLeft;
    delete fDismountRight;
    delete fIdle;
    delete fRelease;
    delete fFallOff;
}

// ACTIVATE
void plAvBrainClimb::Activate(plArmatureModBase *avMod)
{
    plArmatureBrain::Activate(avMod);
    ICalcProbeLengths();

    fAvMod->GetRootAnimator()->Enable(true);
    fAvMod->EnablePhysicsKinematic(true);
}

void plAvBrainClimb::Deactivate()
{
    fAvMod->GetRootAnimator()->Enable(false);
    fAvMod->EnablePhysicsKinematic(false);
}

// APPLY
bool plAvBrainClimb::Apply(double time, float elapsed)
{
    bool result = true;

    IGetDesiredDirection();

    float overage = 0.0f;       // if we ran past the end of the current stage, remember how much
    bool done = false;

    if(fExitStage)
        done = IProcessExitStage(time, elapsed);
    else
        done = IAdvanceCurrentStage(time, elapsed, overage);

    if(done || fCurMode == kIdle)
    {
        // if the transition is to one of the terminal modes, we're going to abort
        result = ITryStageTransition(time, overage);
    }

    if(!result && fExitStage)
    {
        fExitStage->Detach(fAvMod);
    }

    fAvMod->ApplyAnimations(time, elapsed);

    IProbeEnvironment();
    return result;
}

// MSGRECEIVE
bool plAvBrainClimb::MsgReceive(plMessage *msg)
{
    plClimbMsg* climbMsg = plClimbMsg::ConvertNoRef(msg);
    if (climbMsg)
    {
        return IHandleClimbMsg(climbMsg);
    }

    plLOSHitMsg *losMsg = plLOSHitMsg::ConvertNoRef(msg);
    if (losMsg)
    {
        return IHandleLOSMsg(losMsg);
    }

    return plArmatureBrain::MsgReceive(msg);
}

// IHANDLECLIMBMSG
bool plAvBrainClimb::IHandleClimbMsg(plClimbMsg *msg)
{
    switch(msg->fCommand)
    {
    case plClimbMsg::kEnableClimb:
        if(msg->fStatus)
            this->fAllowedDirections |= msg->fDirection;
        else
            this->fAllowedDirections &= ~msg->fDirection;
        break;
    case plClimbMsg::kEnableDismount:
        if(msg->fStatus)
            this->fAllowedDismounts |= msg->fDirection;
        else
            this->fAllowedDismounts &= ~msg->fDirection;
        break;
    case plClimbMsg::kRelease:
        IRelease(true);
        break;
    case plClimbMsg::kFallOff:
        {
            if(fCurMode != kReleasing
            && fCurMode != kFallingOff
            && fCurMode != kFinishing)
            {
                plClimbEventMsg* pMsg = new plClimbEventMsg;
                pMsg->SetSender(msg->fTarget);
                pMsg->SetBCastFlag(plMessage::kBCastByExactType);
                pMsg->SetBCastFlag(plMessage::kLocalPropagate);
                pMsg->SetBCastFlag(plMessage::kNetPropagate);
                pMsg->SetBCastFlag(plMessage::kNetForce);
                pMsg->Send();
            }
            IRelease(false);
            
        break;
        }
    default:
        break;
    }
    return true;
}

// IHANDLELOSMSG
bool plAvBrainClimb::IHandleLOSMsg(plLOSHitMsg *msg)
{
    plClimbMsg::Direction blockDir = static_cast<plClimbMsg::Direction>(msg->fRequestID);
    // this is a weak test because someone else could be using the same bits to mean something different
    // the real strategy is that we should only receive LOS messages of our own creation
    bool oneOfOurs = blockDir == plClimbMsg::kUp || blockDir == plClimbMsg::kDown || blockDir == plClimbMsg::kLeft || blockDir == plClimbMsg::kRight;
    if(oneOfOurs)
    {
        fPhysicallyBlockedDirections |= blockDir;
        return true;
    } else {
        return false;
    }
}

// IPROCESSEXITSTAGE
bool plAvBrainClimb::IProcessExitStage(double time, float elapsed)
{
    plAGAnimInstance *ai = fExitStage->GetAnimInstance();
    bool animDone = ai->IsAtEnd();
    float unused;

    // if we have an exit stage running, move it instead of the base stage
    if(!animDone)
        fExitStage->MoveRelative(time, elapsed, unused, fAvMod);    // only advance if it's not finished yet

    float curBlend = ai->GetBlend();
    
    if(curBlend > .99)      // reached peak strength
    {
        fCurStage->Detach(fAvMod);  // remove the (now completely masked) underlying anim
        fCurStage = nullptr;
        ai->Fade(0, 2.0f);      // start fading the exit anim
    } else if(animDone && curBlend == 0.0f) {   
        return true;        // finished and faded; we're really done now
    }
    return false;
}

// IRELEASE
void plAvBrainClimb::IRelease(bool intentional)
{
    if(fCurMode != kReleasing
        && fCurMode != kFallingOff
        && fCurMode != kFinishing)
    {
        if(intentional)
        {
            // fNextMode = kReleasing;
            fCurMode = kReleasing;
            fExitStage = fRelease;
        } else {
            // fNextMode = kFallingOff;
            fCurMode = kFallingOff;
            fExitStage = fFallOff;
        }
        fNextMode = kFinishing;
        double time = hsTimer::GetSysSeconds();
        // attach the exit stage atop the current stage. from here on out we'll only advance
        // the current stage. 
        fAvMod->GetRootAnimator()->Enable(false);
        fAvMod->EnablePhysicsKinematic(false);
        fExitStage->Attach(fAvMod, this, 1.0f, time);
    }
}

// IADVANCECURRENTSTAGE
bool plAvBrainClimb::IAdvanceCurrentStage(double time, float elapsed, float &overage)
{
    bool stageDone = false;
    if(fCurStage)
    {
        // elapsed tells us how far in time to move the animation
        // we must combine it with the key state to figure out whether
        // we're moving forward or backward in the animation
        fControlDir = 0.0f; // 0 is still; -1 is backwards; 1 is forwards

        switch(fCurMode)
        {
        case kDismountingUp:            // if dismounting or mounting become reversable, move
        case kMountingUp:               // these cases to be with "kClimbingUp"; same for the rest
        case kDismountingRight:
        case kMountingRight:
        case kDismountingDown:
        case kMountingDown:
        case kDismountingLeft:
        case kMountingLeft:
        case kFallingOff:
        case kReleasing:
        case kFinishing:
        case kIdle:
        case kClimbingUp:
        case kClimbingRight:
        case kClimbingDown:
        case kClimbingLeft:
            fControlDir = 1.0f;         // these guys all auto-advance 
            break;
        case kInactive:
        case kUnknown:
            // fControlDir is already 0
            break;
        default:
            hsStatusMessage("Unknown mode in plAvBrainClimb::IAdvanceCurrentStage");
        }
        float delta = elapsed * fControlDir;
        stageDone = fCurStage->MoveRelative(time, delta, overage, fAvMod);
    } else {
        stageDone = true;
    }
    return stageDone;
}

// ITRYSTAGETRANSITION
bool plAvBrainClimb::ITryStageTransition(double time, float overage)
{
//  hsStatusMessageF("Got overage %f", overage);
    IChooseNextMode();

    bool result = true;
                                                    // and vice versa
    if(fCurStage && fCurStage != fIdle)
    {
        hsStatusMessage("Wrapping externally.");
        bool atStart = overage >= 0.0f ? true : false;  // if we went off the end, back to start
        fCurStage->Reset(time, fAvMod, atStart);
        // any time we start a stage besides idle, clear the climbing and dismount restrictions
//      this->fAllowedDirections = plClimbMsg::kUp | plClimbMsg::kDown | plClimbMsg::kLeft | plClimbMsg::kRight;
//      this->fAllowedDismounts = 0;
    }

    if(fNextMode != fCurMode)
    {
        if(fCurStage)
            fCurStage->Detach(fAvMod);
        fCurStage = IGetStageFromMode(fNextMode);
        if(fCurStage)
        {
            hsAssert(fCurStage, "Couldn't get next stage - mode has no stage. (Matt)");
            fCurMode = fNextMode;
            if(fCurStage)
                result = (fCurStage->Attach(fAvMod, this, 1.0f, time) != nullptr);
            fAvMod->DirtySynchState(kSDLAvatar, 0);     // write our new stage to the server
        } else {
            result = false;
        }
    } else {
//      hsStatusMessage("Wrapping externally.");
//      bool atStart = overage >= 0.0f ? true : false;  // if we went off the end, back to start
//                                                      // and vice versa
//      if(fCurStage)
//          fCurStage->Reset(time, fAvMod, atStart);
    }

    fNextMode = kUnknown;

    if(fCurStage)
    {
        if(overage < 0.0f)
        {
            float length = fCurStage->GetLength();
            fCurStage->SetLocalTime(length + overage);
        } else {
            fCurStage->SetLocalTime(overage);
        }
        fAvMod->GetRootAnimator()->Reset(time);
    }

    return result;
}

// ICHOOSENEXTMODE
bool plAvBrainClimb::IChooseNextMode()
{
    // bear in mind this is only called when we're at a point where
    // we can change direction (usually because a climb loop has
    // just finished)
    switch (fCurMode)
    {
    case kInactive:
    case kUnknown:
    case kFinishing:
        break;      // no change

    case kDismountingUp:
    case kDismountingDown:
    case kDismountingLeft:
    case kDismountingRight:
    case kFallingOff:
    case kReleasing:
        fNextMode = kFinishing;
        break;
    case kMountingUp:
    case kClimbingUp:
    case kMountingDown:
    case kClimbingDown:
    case kMountingLeft:
    case kClimbingLeft:
    case kMountingRight:
    case kClimbingRight:
    case kIdle:
        fNextMode = kIdle;
        if(fAllowedDismounts & fDesiredDirection)
        {
            switch(fDesiredDirection)
            {
            case plClimbMsg::kUp:
                fNextMode = kDismountingUp;
                break;
            case plClimbMsg::kDown:
                fNextMode = kDismountingDown;
                break;
            case plClimbMsg::kLeft:
                fNextMode = kDismountingLeft;
                break;
            case plClimbMsg::kRight:
                fNextMode = kDismountingRight;
                break;
            case plClimbMsg::kCenter:
                fNextMode = kIdle;
                break;
            default:
                hsAssert(false, "Error in fDesiredDirection. (Matt)");
            }
        } else if(fAllowedDirections & fDesiredDirection & ~fPhysicallyBlockedDirections)
        {
            switch(fDesiredDirection)
            {
            case plClimbMsg::kUp:
                fNextMode = kClimbingUp;
                break;
            case plClimbMsg::kDown:
                fNextMode = kClimbingDown;
                break;
            case plClimbMsg::kLeft:
                fNextMode = kClimbingLeft;
                break;
            case plClimbMsg::kRight:
                fNextMode = kClimbingRight;
                break;
            case plClimbMsg::kCenter:
                fNextMode = kIdle;
                break;
            default:
                hsAssert(false, "Error in fDesiredDirection. (Matt)");
            }
        }
        break;
    default:
        hsAssert(false, "Error in fCurMode. (Matt)");
    }
    return true;
}

// IGETSTAGE
plAnimStage * plAvBrainClimb::IGetStageFromMode(Mode mode)
{
    switch(mode)
    {
    case kClimbingUp:
        return fUp;
    case kClimbingDown:
        return fDown;
    case kClimbingLeft:
        return fLeft;
    case kClimbingRight:
        return fRight;
    case kMountingUp:
        return fMountUp;
    case kMountingDown:
        return fMountDown;
    case kMountingLeft:
        return fMountLeft;
    case kMountingRight:
        return fMountRight;
    case kDismountingUp:
        return fDismountUp;
    case kDismountingDown:
        return fDismountDown;
    case kDismountingLeft:
        return fDismountLeft;
    case kDismountingRight:
        return fDismountRight;
    case kIdle:
        return fIdle;
    case kReleasing:
        return fRelease;
    case kFallingOff:
        return fFallOff;
    case kInactive:
    case kFinishing:
    case kUnknown:
    case kDone:
        return nullptr;
    default:
        hsAssert(false, "Unknown mode.");
        return nullptr;
    }
}

plAvBrainClimb::Mode plAvBrainClimb::IGetModeFromStage(plAnimStage *stage)
{
    if(stage == fUp)
        return kClimbingUp;
    else if(stage == fDown)
        return kClimbingDown;
    else if(stage == fLeft)
        return kClimbingLeft;
    else if(stage == fRight)
        return kClimbingRight;
    else if(stage == fMountUp)
        return kMountingUp;
    else if(stage == fMountDown)
        return kMountingDown;
    else if(stage == fMountLeft)
        return kMountingLeft;
    else if(stage == fMountRight)
        return kMountingRight;
    else if(stage == fDismountUp)
        return kDismountingUp;
    else if(stage == fDismountDown)
        return kDismountingDown;
    else if(stage == fDismountLeft)
        return kDismountingLeft;
    else if(stage == fDismountRight)
        return kDismountingRight;
    else if(stage == fIdle)
        return kIdle;
    else if(stage == fRelease)
        return kReleasing;
    else if(stage == fFallOff)
        return kFallingOff;
    else
        return kUnknown;    
}

// IGETDESIREDDIRECTION
void plAvBrainClimb::IGetDesiredDirection()
{
    if(fAvMod->ForwardKeyDown()) {
        fDesiredDirection = plClimbMsg::kUp;
    } else if (fAvMod->BackwardKeyDown()) {
        fDesiredDirection = plClimbMsg::kDown;
    } else if (fAvMod->TurnLeftKeyDown()) {
        fDesiredDirection = plClimbMsg::kLeft;
    } else if (fAvMod->TurnRightKeyDown()) {
        fDesiredDirection = plClimbMsg::kRight;
    } else {
        fDesiredDirection = plClimbMsg::kCenter;
    }
}

/** Look left, right, up, and down to see which directions are clear
    for our movement. We could do this by positioning our actual collision
    body and testing for hits, but it gives a lot more false positives *and*
    we won't get the normals of intersection, so it will be more complex
    to figure out which directions are actually blocked.
    The approach here is to do a raycast in the aforementioned directions
    and fail that direction if the raycast hits anything. */
void plAvBrainClimb::IProbeEnvironment()
{
    hsMatrix44 l2w = fAvMod->GetTarget(0)->GetLocalToWorld();
    // we're just going to pull the axes out of the 
    
    hsPoint3 up = hsPoint3(l2w.GetAxis(hsMatrix44::kUp) * fVerticalProbeLength);
    hsPoint3 down = -up;
    hsPoint3 right = hsPoint3(l2w.GetAxis(hsMatrix44::kRight) * fHorizontalProbeLength);
    hsPoint3 left = -right;
    hsPoint3 start = l2w.GetTranslate();

    start.fZ += 3.0f;   // move the origin from the feet to the bellybutton
    up += start;
    down += start;
    left += start;
    right += start;

    plKey ourKey = fAvMod->GetKey();

    // *** would be cool if we could hint that these should be batched for spatial coherence optimization
    plLOSRequestMsg *upReq = new plLOSRequestMsg(ourKey, start, up, plSimDefs::kLOSDBCustom, plLOSRequestMsg::kTestAny, plLOSRequestMsg::kReportHit);
    upReq->SetRequestID(static_cast<uint32_t>(plClimbMsg::kUp));
    upReq->SetRequestName(ST_LITERAL("Ladder Brain: Climb Up"));
    upReq->Send();

    plLOSRequestMsg *downReq = new plLOSRequestMsg(ourKey, start, down, plSimDefs::kLOSDBCustom, plLOSRequestMsg::kTestAny, plLOSRequestMsg::kReportHit);
    downReq->SetRequestID(static_cast<uint32_t>(plClimbMsg::kDown));
    downReq->SetRequestName(ST_LITERAL("Ladder Brain: Climb Down"));
    downReq->Send();

    plLOSRequestMsg *leftReq = new plLOSRequestMsg(ourKey, start, left, plSimDefs::kLOSDBCustom, plLOSRequestMsg::kTestAny, plLOSRequestMsg::kReportHit);
    leftReq->SetRequestID(static_cast<uint32_t>(plClimbMsg::kLeft));
    leftReq->SetRequestName(ST_LITERAL("Ladder Brain: Climb Left"));
    leftReq->SetRequestType(plSimDefs::kLOSDBCustom);
    leftReq->Send();

    plLOSRequestMsg *rightReq = new plLOSRequestMsg(ourKey, start, right, plSimDefs::kLOSDBCustom, plLOSRequestMsg::kTestAny, plLOSRequestMsg::kReportHit);
    rightReq->SetRequestID(static_cast<uint32_t>(plClimbMsg::kRight));
    rightReq->SetRequestName(ST_LITERAL("Ladder Brain: Climb Right"));
    rightReq->Send();

    fOldPhysicallyBlockedDirections = fPhysicallyBlockedDirections;
    fPhysicallyBlockedDirections = 0;   // clear our blocks until the new reports come in....
}

// ICalcProbeLengths -------------------
// -----------------
void plAvBrainClimb::ICalcProbeLengths()
{
    // we assume that the up and down climbs go the same distance;
    // same for the left and right climbs
    plAGAnim *up = fAvMod->FindCustomAnim("WallClimbUp");
    plAGAnim *left = fAvMod->FindCustomAnim("WallClimbLeft");

    hsMatrix44 upMove, leftMove;

    hsAssert(up, "Couldn't find ClimbUp animation.");
    if(up)
    {
        GetStartToEndTransform(up, &upMove, nullptr, "Handle");
        fVerticalProbeLength = upMove.GetTranslate().fZ;
    } else
        fVerticalProbeLength = 4.0f;    // guess

    hsAssert(left, "Couldn't find ClimbLeft animation.");
    if(left)
    {
        GetStartToEndTransform(left, &leftMove, nullptr, "Handle");
        fHorizontalProbeLength = leftMove.GetTranslate().fX;
    } else
        fHorizontalProbeLength = 3.0f;  // guess
}

// IInitAnimations ---------------------
// ---------------
bool plAvBrainClimb::IInitAnimations()
{
    fUp = new plAnimStage("WallClimbUp",
                          plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressAuto,
                          0);
    fDown = new plAnimStage("WallClimbDown",
                          plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressAuto,
                          0);
    fLeft = new plAnimStage("WallClimbLeft",
                          plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressAuto,
                          0);
    fRight = new plAnimStage("WallClimbRight",
                          plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressAuto,
                          0);
    // the mounts
    fMountUp = new plAnimStage("WallClimbMountUp",
                          plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressNone,
                          0);
    fMountDown = new plAnimStage("WallClimbMountDown",
                          plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressNone,
                          0);
    fMountLeft = new plAnimStage("WallClimbMountLeft",
                          plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressNone,
                          0);
    fMountRight = new plAnimStage("WallClimbMountRight",
                          plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressNone,
                          0);
    // and here's the dismount
    fDismountUp = new plAnimStage("WallClimbDismountUp",
                          plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressNone,
                          0);
    fDismountDown = new plAnimStage("WallClimbDismountDown",
                          plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressNone,
                          0);
    fDismountLeft = new plAnimStage("WallClimbDismountLeft",
                          plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressNone,
                          0);
    fDismountRight = new plAnimStage("WallClimbDismountUp",
                          plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressNone,
                          0);
    // other
    fIdle = new plAnimStage("WallClimbIdle",
                          plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressNone,
                          0);
    fRelease = new plAnimStage("WallClimbRelease",
                          plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressNone,
                          0);
    fFallOff = new plAnimStage("WallClimbFallOff",
                          plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone, plAnimStage::kAdvanceAuto, plAnimStage::kRegressNone,
                          0);
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// SDL-BASED PERSISTENCE
//
/////////////////////////////////////////////////////////////////////////////////////////

// SaveToSDL -----------------------------------------
// ---------
void plAvBrainClimb::SaveToSDL(plStateDataRecord *sdl)
{
    sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrCurMode)->Set(fCurMode);
    sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrNextMode)->Set(fNextMode);
    sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrAllowedDirections)->Set((int)fAllowedDirections);
    sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrAllowedDismounts)->Set((int)fAllowedDismounts);
    sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrVertProbeLength)->Set(fVerticalProbeLength);
    sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrHorizProbeLength)->Set(fHorizontalProbeLength);

    bool curStageAttached = fCurStage && fCurStage->GetIsAttached();
    sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrCurStageAttached)->Set(curStageAttached);
    if(curStageAttached)
    {
        // slightly abuse the "mode" semantics; it happens to work as a persistance format
        Mode curStageAsMode = IGetModeFromStage(fCurStage);
        sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrCurStage)->Set(curStageAsMode);

        float curStageTime = fCurStage->GetLocalTime();
        sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrCurStageTime)->Set(curStageTime);

        float curStageBlend = fCurStage->GetAnimInstance()->GetBlend();
        sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrCurStageStrength)->Set(curStageBlend);
    }
    bool exitStageAttached = fExitStage && fExitStage->GetIsAttached();
    sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrExitStageAttached)->Set(exitStageAttached);

    if(exitStageAttached)
    {
        Mode exitStageAsMode = IGetModeFromStage(fExitStage);
        sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrExitStage)->Set(exitStageAsMode);
        
        float exitStageTime = fExitStage->GetLocalTime();
        sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrExitStageTime)->Set(exitStageTime);

        float exitStageBlend = fExitStage->GetAnimInstance()->GetBlend();
        sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrExitStageStrength)->Set(exitStageBlend);
    }
}

// LoadFromSDL -----------------------------------------
// -----------
void plAvBrainClimb::LoadFromSDL(const plStateDataRecord *sdl)
{
    double curTime = hsTimer::GetSysSeconds();

    sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrCurStage)->Get((int*)&fCurMode);
    sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrNextMode)->Get((int*)&fNextMode);
    sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrAllowedDirections)->Get((int*)&fAllowedDirections);
    sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrAllowedDismounts)->Get((int*)&fAllowedDismounts);
    sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrVertProbeLength)->Get(&fVerticalProbeLength);
    sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrHorizProbeLength)->Get(&fHorizontalProbeLength);

    bool curStageAttached = false;
    sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrCurStageAttached)->Get(&curStageAttached);
    if(curStageAttached)
    {
        Mode *curStageMode;     // distinct from curMode; this is just a mode-based representation of the current stage
        sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrCurStage)->Get((int*)&curStageMode);
        plAnimStage *curStage = this->IGetStageFromMode(fCurMode);
        curStage->Attach(fAvMod, this, 0.0f, curTime);

        float curStageTime = 0.0f;
        sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrCurStageTime)->Get(&curStageTime);
        curStage->ResetAtTime(curTime, curStageTime, fAvMod);   // restart the relative-position sampler

        float curStageBlend = 0.0f;
        sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrCurStageStrength)->Get(&curStageBlend);
        curStage->GetAnimInstance()->SetBlend(curStageBlend);
    }

    bool exitStageAttached;
    sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrExitStageAttached)->Get(&exitStageAttached);
    if(exitStageAttached)
    {
        Mode exitStageMode; // the exit stage, in mode form
        sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrExitStage)->Get((int *)&exitStageMode);   
        sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrExitStageTime)->Get((int *)&fNextMode);
        sdl->FindVar(plAvatarSDLModifier::ClimbBrainVarNames::kStrExitStageStrength)->Get((int *)&fNextMode);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// READABLE TEXT FOR DEBUGGING
//
/////////////////////////////////////////////////////////////////////////////////////////

// DumpToDebugDisplay --------------------------------------------------------------------------------------
// ------------------
void plAvBrainClimb::DumpToDebugDisplay(int &x, int &y, int lineHeight, plDebugText &debugTxt)
{
    debugTxt.DrawString(x, y, "Brain type: Climb");
    y += lineHeight;
    const char * worldDir = WorldDirStr(fDesiredDirection);
    const char * modeStr = ModeStr(fCurMode);

    char buffy[256];
    sprintf(buffy, "direction: %s mode: %s controlDir: %f", worldDir, modeStr, fControlDir);
    debugTxt.DrawString(x,y, buffy);
    y += lineHeight;
    
    IDumpClimbDirections(x, y, lineHeight, debugTxt);
    IDumpDismountDirections(x, y, lineHeight, debugTxt);
    IDumpBlockedDirections(x, y, lineHeight, debugTxt);

    fUp->DumpDebug(fUp == fCurStage, x, y, lineHeight, debugTxt);
    fDown->DumpDebug(fDown == fCurStage, x, y, lineHeight, debugTxt);
    fLeft->DumpDebug(fLeft == fCurStage, x, y, lineHeight, debugTxt);
    fRight->DumpDebug(fRight == fCurStage, x, y, lineHeight, debugTxt);
    fMountUp->DumpDebug(fMountUp == fCurStage, x, y, lineHeight, debugTxt);
    fMountDown->DumpDebug(fMountDown == fCurStage, x, y, lineHeight, debugTxt);
    fMountLeft->DumpDebug(fMountLeft == fCurStage, x, y, lineHeight, debugTxt);
    fMountRight->DumpDebug(fMountRight == fCurStage, x, y, lineHeight, debugTxt);
    fDismountUp->DumpDebug(fDismountUp == fCurStage, x, y, lineHeight, debugTxt);
    fDismountDown->DumpDebug(fDismountDown == fCurStage, x, y, lineHeight, debugTxt);
    fDismountLeft->DumpDebug(fDismountLeft == fCurStage, x, y, lineHeight, debugTxt);
    fDismountRight->DumpDebug(fDismountRight == fCurStage, x, y, lineHeight, debugTxt);
    fIdle->DumpDebug(fIdle == fCurStage, x, y, lineHeight, debugTxt);
    fRelease->DumpDebug(fRelease == fCurStage, x, y, lineHeight, debugTxt);
    fFallOff->DumpDebug(fFallOff == fCurStage, x, y, lineHeight, debugTxt);

}

// IDumpClimbDirections --------------------------------------------------------------------------------------
// --------------------
void plAvBrainClimb::IDumpClimbDirections(int &x, int &y, int lineHeight, plDebugText &debugTxt)
{
    static const char prolog[] = "Allowed directions: ";
    ST::string_stream str;

    str << prolog;
    if(fAllowedDirections & plClimbMsg::kUp)
        str << "UP ";
    if(fAllowedDirections & plClimbMsg::kDown)
        str << "DOWN ";
    if(fAllowedDirections & plClimbMsg::kLeft)
        str << "LEFT ";
    if(fAllowedDirections & plClimbMsg::kRight)
        str << "RIGHT ";
    
    if(str.size() == strlen(prolog))
        str << "- NONE -";

    debugTxt.DrawString(x, y, str.to_string());
    y += lineHeight;
}

// IDumpDismountDirections --------------------------------------------------------------------------------------
// -----------------------
void plAvBrainClimb::IDumpDismountDirections(int &x, int &y, int lineHeight, plDebugText &debugTxt)
{
    static const char prolog[] = "Enabled dismounts: ";
    ST::string_stream str;

    str << prolog;
    if(fAllowedDismounts & plClimbMsg::kUp)
        str << "UP ";
    if(fAllowedDismounts & plClimbMsg::kDown)
        str << "DOWN ";
    if(fAllowedDismounts & plClimbMsg::kLeft)
        str << "LEFT ";
    if(fAllowedDismounts & plClimbMsg::kRight)
        str << "RIGHT ";
    
    if(str.size() == strlen(prolog))
        str << "- NONE -";

    debugTxt.DrawString(x, y, str.to_string());
    y += lineHeight;
}

void plAvBrainClimb::IDumpBlockedDirections(int &x, int &y, int lineHeight, plDebugText &debugTxt)
{
    static const char prolog[] = "Physically blocked: ";
    ST::string_stream str;

    str << prolog;
    if(fOldPhysicallyBlockedDirections & plClimbMsg::kUp)
        str << "UP ";
    if(fOldPhysicallyBlockedDirections & plClimbMsg::kDown)
        str << "DOWN ";
    if(fOldPhysicallyBlockedDirections & plClimbMsg::kLeft)
        str << "LEFT ";
    if(fOldPhysicallyBlockedDirections & plClimbMsg::kRight)
        str << "RIGHT ";
    
    if(str.size() == strlen(prolog))
        str << "- NONE -";

    debugTxt.DrawString(x, y, str.to_string());
    y += lineHeight;
}

const char *plAvBrainClimb::WorldDirStr(plClimbMsg::Direction dir)
{
    switch(dir)
    {
    case plClimbMsg::kUp:
        return "Up";
    case plClimbMsg::kDown:
        return "Down";
    case plClimbMsg::kLeft:
        return "Left";
    case plClimbMsg::kRight:
        return "Right";
    case plClimbMsg::kCenter:
        return "Center";
    default:
        return "WTF?";
    }
}

const char *plAvBrainClimb::ModeStr(Mode mode)
{
    switch(mode)
    {
    case kInactive:
        return "Inactive";
    case kUnknown:
        return "Unknown";
    case kFinishing:
        return "Finishing";
    case kDone:
        return "Done";
    case kClimbingUp:
        return "ClimbingUp";
    case kClimbingDown:
        return "ClimbingDown";
    case kClimbingLeft:
        return "ClimbingLeft";
    case kClimbingRight:
        return "ClimbingRight";
    case kMountingUp:
        return "MountingUp";
    case kMountingDown:
        return "MountingDown";
    case kMountingLeft:
        return "MountingLeft";
    case kMountingRight:
        return "MountingRight";
    case kDismountingUp:
        return "MountingUp";
    case kDismountingDown:
        return "DismountingDown";
    case kDismountingLeft:
        return "DismountingLeft";
    case kDismountingRight:
        return "DismountingRight";
    case kIdle:
        return "Idle";
    case kReleasing:
        return "Releasing";
    case kFallingOff:
        return "FallingOff";
    default:
        return "WTF?!";
    }
}
