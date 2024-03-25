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

#include "plAvBrainRideAnimatedPhysical.h"

#include "plArmatureMod.h"
#include "plAvBrain.h"
#include "plAvBrainHuman.h"
#include "plPhysicalControllerCore.h"

#include "plAnimation/plAGModifier.h"
#include "plMessage/plRideAnimatedPhysMsg.h"


void plAvBrainRideAnimatedPhysical::Activate(plArmatureModBase *avMod)
{
    plArmatureBrain::Activate(avMod);
    IInitAnimations();
    if (!fWalkingStrategy)
    {
        plSceneObject* avObj = fArmature->GetTarget(0);
        plAGModifier* agMod = const_cast<plAGModifier*>(plAGModifier::ConvertNoRef(FindModifierByClass(avObj, plAGModifier::Index())));
        plPhysicalControllerCore* controller = avMod->GetController();
        fWalkingStrategy = new plWalkingStrategy(agMod->GetApplicator(kAGPinTransform), controller);
        fWalkingStrategy->ToggleRiding(true);
        controller->SetMovementStrategy(fWalkingStrategy);
    }
}
plAvBrainRideAnimatedPhysical::~plAvBrainRideAnimatedPhysical()
{
    delete fWalkingStrategy;
    fWalkingStrategy = nullptr;
}

void plAvBrainRideAnimatedPhysical::Deactivate()
{
    plArmatureBrain::Deactivate();
}
bool plAvBrainRideAnimatedPhysical::MsgReceive(plMessage *msg)
{
    plRideAnimatedPhysMsg *ride = plRideAnimatedPhysMsg::ConvertNoRef(msg);
    if(ride)
    {
        if(!ride->Entering())
        {
            /*this->fArmature->PopBrain();
            delete this;
            return true;
            */
            fMode=kAbort;
        }
    }
    return plArmatureBrain::MsgReceive(msg);
}
bool plAvBrainRideAnimatedPhysical::IInitAnimations()
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
bool plAvBrainRideAnimatedPhysical::LeaveAge()
{
    return plArmatureBrain::LeaveAge();
}
bool plAvBrainRideAnimatedPhysical::Apply(double timeNow, float elapsed)
{
    if(this->fMode==kAbort) return false;
    else return plAvBrainHuman::Apply(timeNow, elapsed);

}
