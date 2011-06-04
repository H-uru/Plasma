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
#include "plAvBrainRideAnimatedPhysical.h"
#include "plArmatureMod.h"

#include "plAvBrainHuman.h"
#include "plAvBrain.h"
#include "plAvCallbackAction.h"
#include "../plMessage/plRideAnimatedPhysMsg.h"


void plAvBrainRideAnimatedPhysical::Activate(plArmatureModBase *avMod)
{
	plArmatureBrain::Activate(avMod);
	IInitAnimations();
	if (!fCallbackAction)
	{
		plSceneObject* avObj = fArmature->GetTarget(0);
		plAGModifier* agMod = const_cast<plAGModifier*>(plAGModifier::ConvertNoRef(FindModifierByClass(avObj, plAGModifier::Index())));
		plPhysicalControllerCore* controller = avMod->GetController();
		fCallbackAction = TRACKED_NEW plRidingAnimatedPhysicalController(avObj, agMod->GetApplicator(kAGPinTransform), controller);
		fCallbackAction->ActivateController();
	}
}
plAvBrainRideAnimatedPhysical::~plAvBrainRideAnimatedPhysical()
{
	delete fCallbackAction;
	fCallbackAction=nil;

}

void plAvBrainRideAnimatedPhysical::Deactivate()
{
	plArmatureBrain::Deactivate();
}
hsBool plAvBrainRideAnimatedPhysical::MsgReceive(plMessage *msg)
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
hsBool plAvBrainRideAnimatedPhysical::IInitAnimations()
{
	hsBool result = false;

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
	plAGAnim *movingLeft = nil; // fAvMod->FindCustomAnim("LeanLeft");
	plAGAnim *movingRight = nil; // fAvMod->FindCustomAnim("LeanRight");
	plAGAnim *pushWalk = fAvMod->FindCustomAnim("BallPushWalk");

	//plAGAnim *pushIdle = fAvMod->FindCustomAnim("BallPushIdle");
	
	const float kDefaultFade = 3.0;		// most animations fade in and out in 1/4 of a second.

	if (idle && walk && run && walkBack && standingLeft && standingRight && stepLeft && stepRight)
	{
		plHBehavior *behavior;
		fBehaviors.SetCountAndZero(kHuBehaviorMax);
		fBehaviors[kIdle] = behavior = TRACKED_NEW Idle;
		behavior->Init(idle, true, this, fAvMod, kDefaultFade, kDefaultFade, kIdle, plHBehavior::kBehaviorTypeIdle);
		behavior->SetStrength(1.f, 0.f);
		
		fBehaviors[kWalk] = behavior = TRACKED_NEW Walk;
		behavior->Init(walk, true, this, fAvMod, kDefaultFade, 5.f, kWalk, plHBehavior::kBehaviorTypeWalk);
		
		fBehaviors[kRun] = behavior = TRACKED_NEW Run;
		behavior->Init(run, true, this, fAvMod, kDefaultFade, 2.0, kRun, plHBehavior::kBehaviorTypeRun);

		fBehaviors[kWalkBack] = behavior = TRACKED_NEW WalkBack;
		behavior->Init(walkBack, true, this, fAvMod, kDefaultFade, kDefaultFade, kWalkBack, plHBehavior::kBehaviorTypeWalkBack);

		fBehaviors[kStandingTurnLeft] = behavior = TRACKED_NEW StandingTurnLeft;
		behavior->Init(standingLeft, true, this, fAvMod, 3.0f, 6.0f, kStandingTurnLeft, plHBehavior::kBehaviorTypeTurnLeft);

		fBehaviors[kStandingTurnRight] = behavior = TRACKED_NEW StandingTurnRight;
		behavior->Init(standingRight, true, this, fAvMod, 3.0f, 6.0f, kStandingTurnRight, plHBehavior::kBehaviorTypeTurnRight);

		fBehaviors[kStepLeft] = behavior = TRACKED_NEW StepLeft;
		behavior->Init(stepLeft, true, this, fAvMod, kDefaultFade, kDefaultFade, kStepLeft, plHBehavior::kBehaviorTypeSidestepLeft);

		fBehaviors[kStepRight] = behavior = TRACKED_NEW StepRight;
		behavior->Init(stepRight, true, this, fAvMod, kDefaultFade, kDefaultFade, kStepRight, plHBehavior::kBehaviorTypeSidestepRight);

		// Warning: Changing the blend times of the jump animations will affect the path you take, because until we're fully blended,
		// we won't be using the full motion defined in the animation. This isn't an issue for standing jump, but you need to be
		// aware of it for the walk/run jumps.
		fBehaviors[kFall] = behavior = TRACKED_NEW Fall;
		behavior->Init(fall, true, this, fAvMod, 1.0f, 10, kFall, plHBehavior::kBehaviorTypeFall);

		fBehaviors[kStandingJump] = behavior = TRACKED_NEW StandingJump;
		behavior->Init(standJump, false, this, fAvMod, kDefaultFade, kDefaultFade, kStandingJump, plHBehavior::kBehaviorTypeStandingJump);

		fBehaviors[kWalkingJump] = behavior = TRACKED_NEW WalkingJump;
		behavior->Init(walkJump, false, this, fAvMod, 10, 3.0, kWalkingJump, plHBehavior::kBehaviorTypeWalkingJump);

		fBehaviors[kRunningJump] = behavior = TRACKED_NEW RunningJump;
		behavior->Init(runJump, false, this, fAvMod, 10, 2.0, kRunningJump, plHBehavior::kBehaviorTypeRunningJump);
		
		fBehaviors[kGroundImpact] = behavior = TRACKED_NEW GroundImpact;
		behavior->Init(groundImpact, false, this, fAvMod, 6.0f, kDefaultFade, kGroundImpact, plHBehavior::kBehaviorTypeGroundImpact);
		
		fBehaviors[kRunningImpact] = behavior = TRACKED_NEW RunningImpact;
		behavior->Init(runningImpact, false, this, fAvMod, 6.0f, kDefaultFade, kRunningImpact, plHBehavior::kBehaviorTypeRunningImpact);
		
		fBehaviors[kMovingTurnLeft] = behavior = TRACKED_NEW MovingTurnLeft;
		behavior->Init(movingLeft, true, this, fAvMod, kDefaultFade, kDefaultFade, kMovingTurnLeft, plHBehavior::kBehaviorTypeMovingTurnLeft);

		fBehaviors[kMovingTurnRight] = behavior = TRACKED_NEW MovingTurnRight;
		behavior->Init(movingRight, true, this, fAvMod, kDefaultFade, kDefaultFade, kMovingTurnRight, plHBehavior::kBehaviorTypeMovingTurnRight);

		fBehaviors[kPushWalk] = behavior = TRACKED_NEW PushWalk;
		behavior->Init(pushWalk, true, this, fAvMod, kDefaultFade, kDefaultFade, kPushWalk, plHBehavior::kBehaviorTypePushWalk);
		
		//fBehaviors[kPushIdle] = behavior = TRACKED_NEW PushIdle;
		//behavior->Init(pushIdle, true, this, fAvMod, kDefaultFade, kDefaultFade, kPushIdle, plHBehavior::kBehaviorTypePushIdle);

		result = true;
	}
	return result;
}
hsBool plAvBrainRideAnimatedPhysical::LeaveAge()
{
	return plArmatureBrain::LeaveAge();
}
hsBool plAvBrainRideAnimatedPhysical::Apply(double timeNow, hsScalar elapsed)
{
	if(this->fMode==kAbort) return false;
	else return plAvBrainHuman::Apply(timeNow, elapsed);

}
