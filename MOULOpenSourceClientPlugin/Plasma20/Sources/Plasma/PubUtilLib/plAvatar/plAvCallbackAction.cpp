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
#include "plAvCallbackAction.h"
#include "../plMessage/plLOSHitMsg.h"

#include "plArmatureMod.h" // for LOS enum type
#include "plMatrixChannel.h"
#include "hsTimer.h"
#include "plPhysicalControllerCore.h"

// Generic geom utils.
hsBool LinearVelocity(hsVector3 &outputV, float elapsed, hsMatrix44 &prevMat, hsMatrix44 &curMat);
void AngularVelocity(hsScalar &outputV, float elapsed, hsMatrix44 &prevMat, hsMatrix44 &curMat);
float AngleRad2d (float x1, float y1, float x3, float y3);
inline hsVector3 GetYAxis(hsMatrix44 &mat)
{
	return hsVector3(mat.fMap[1][0], mat.fMap[1][1], mat.fMap[1][2]);
}

plAnimatedController::plAnimatedController(plSceneObject* rootObject, plAGApplicator* rootApp, plPhysicalControllerCore* controller)
	: fRootObject(rootObject)
	, fRootApp(rootApp)
	, fController(controller)
	, fTurnStr(0.f)
	, fAnimAngVel(0.f)
	, fAnimPosVel(0.f, 0.f, 0.f)
{
}	

void plAnimatedController::RecalcVelocity(double timeNow, double timePrev, hsBool useAnim /* = true */)
{
	if (useAnim)
	{
		// while you may think it would be correct to cache this,
		// what we're actually asking is "what would the animation's
		// position be at the previous time given its *current*
		// parameters (particularly blends)"
		hsMatrix44 prevMat = ((plMatrixChannel *)fRootApp->GetChannel())->Value(timePrev, true);		
		hsMatrix44 curMat = ((plMatrixChannel *)fRootApp->GetChannel())->Value(timeNow, true);
		
		// If we get a valid linear velocity (ie, we didn't wrap around in the anim),
		// use it.  Otherwise just reuse the previous frames velocity.
		hsVector3 linearVel;
		if (LinearVelocity(linearVel, (float)(timeNow - timePrev), prevMat, curMat))
			fAnimPosVel = linearVel;

		// Automatically sets fAnimAngVel
		AngularVelocity(fAnimAngVel, (float)(timeNow - timePrev), prevMat, curMat);
	}
	else
	{
		fAnimPosVel.Set(0.f, 0.f, 0.f);
		fAnimAngVel = 0.f;
	}

	if (fController)
		fController->SetVelocities(fAnimPosVel, fAnimAngVel + fTurnStr);
}

///////////////////////////////////////////////////////////////////////////

const hsScalar plWalkingController::kControlledFlightThreshold = 1.f; // seconds

plWalkingController::plWalkingController(plSceneObject* rootObject, plAGApplicator* rootApp, plPhysicalControllerCore* controller)
	: plAnimatedController(rootObject, rootApp, controller)
	, fHitGroundInThisAge(false)
	, fWaitingForGround(false)
	, fControlledFlightTime(0)
	, fControlledFlight(0)
	, fImpactTime(0.f)
	, fImpactVelocity(0.f, 0.f, 0.f)
	, fClearImpact(false)
	, fGroundLastFrame(false)
{
	if (fController)
	{
		fWalkingStrategy= TRACKED_NEW plWalkingStrategy(fController);
		fController->SetMovementSimulationInterface(fWalkingStrategy);
	}
	else
		fWalkingStrategy = nil;
}

void plWalkingController::RecalcVelocity(double timeNow, double timePrev, hsBool useAnim)
{
	if (!fHitGroundInThisAge && fController && fController->IsEnabled() && fWalkingStrategy->IsOnGround())
		fHitGroundInThisAge = true;	// if we're not pinned and we're not in an age yet, we are now.

	if (fClearImpact)
	{
		fImpactTime = 0.f;
		fImpactVelocity.Set(0.f, 0.f, 0.f);
	}

	if (fController && !fWalkingStrategy->IsOnGround())
	{
		fImpactTime = fWalkingStrategy->GetAirTime();
		fImpactVelocity = fController->GetLinearVelocity();
		fClearImpact = false;
	}
	else
		fClearImpact = true;
	
	if (IsControlledFlight())
	{
		if (fWalkingStrategy && fWalkingStrategy->IsOnGround())
			fControlledFlightTime = fWalkingStrategy->GetAirTime();
		if(fGroundLastFrame&&(fWalkingStrategy && !fWalkingStrategy->IsOnGround()))
		{
			//we have started to leave the ground tell the movement strategy in case it cares
			fWalkingStrategy->StartJump();
		}
		if (fControlledFlightTime > kControlledFlightThreshold)
			EnableControlledFlight(false);
	}
	if (fWalkingStrategy)
		fGroundLastFrame = fWalkingStrategy->IsOnGround();
	else
		fGroundLastFrame=false;
	plAnimatedController::RecalcVelocity(timeNow, timePrev, useAnim);
}

void plWalkingController::Reset(bool newAge)
{
	
	ActivateController();
	if (newAge)
	{
		if (fWalkingStrategy)
		fWalkingStrategy->ResetAirTime();
		fHitGroundInThisAge = false;
	}
}
 void plWalkingController::ActivateController()
{
	if (fWalkingStrategy)
	{
		fWalkingStrategy->RefreshConnectionToControllerCore();
	}
	else
	{	
		fWalkingStrategy= TRACKED_NEW plWalkingStrategy(fController);
		fWalkingStrategy->RefreshConnectionToControllerCore();

	}
}

bool plWalkingController::EnableControlledFlight(bool status)
{
	if (status)
	{
		if (fControlledFlight == 0)
			fControlledFlightTime = 0.f;

		++fControlledFlight;
		fWaitingForGround = true;
	}
	else
		fControlledFlight = __max(--fControlledFlight, 0);

	return status;
}
plWalkingController::~plWalkingController()
{
	delete fWalkingStrategy;
	if (fController)
			fController->SetMovementSimulationInterface(nil);
}
#if 0
void plWalkingController::Update()
{
// 	double elapsed = time.asDouble() - getRefresh().asDouble();
// 	setRefresh(time);
// 	
// 	hsBool isPhysical = !fPhysical->GetProperty(plSimulationInterface::kPinned);
// 	const Havok::Vector3 straightUp(0.0f, 0.0f, 1.0f);
// 	hsBool alreadyInAge = fHitGroundInThisAge;
// 	
// 	int numContacts = fPhysical->GetNumContacts();
// 	bool ground = false;
// 	fPushingPhysical = nil;
// 	int i, j;

/*	for(i = 0; i < numContacts; i++)
	{
		plHKPhysical *contactPhys = fPhysical->GetContactPhysical(i);
		if (!contactPhys)
			continue; // Physical no longer exists. Skip it.

		const Havok::ContactPoint *contact = fPhysical->GetContactPoint(i);
		hsScalar dotUp = straightUp.dot(contact->m_normal);
		if (dotUp > .5)
			ground = true;
		else if (contactPhys->GetProperty(plSimulationInterface::kAvAnimPushable))
		{
			hsPoint3 position;
			hsQuat rotation;
			fPhysical->GetPositionAndRotationSim(&position, &rotation);
		
			hsQuat inverseRotation = rotation.Inverse();
			hsVector3 normal(contact->m_normal.x, contact->m_normal.y, contact->m_normal.z);
			fFacingPushingPhysical = (inverseRotation.Rotate(&kAvatarForward).InnerProduct(normal) < 0 ? true : false);
		
			fPushingPhysical = contactPhys;
		}
	}

	// We need to check for the case where the avatar hasn't collided with "ground", but is colliding
	// with a few other objects so that he's not actually falling (wedged in between some slopes).
	// We do this by answering the following question (in 2d top-down space): "If you sort the contact 
	// normals by angle, is there a large enough gap between normals?"
	// 
	// If you think in terms of geometry, this means a collection of surfaces are all pushing on you.
	// If they're pushing from all sides, you have nowhere to go, and you won't fall. There needs to be
	// a gap, so that you're pushed out and have somewhere to fall. This is the same as finding a gap 
	// larger than 180 degrees between sorted normals.
	// 
	// The problem is that on top of that, the avatar needs enough force to shove him out that gap (he
	// has to overcome friction). I deal with that by making the threshold (360 - (180 - 60) = 240). I've
	// seen up to 220 reached in actual gameplay in a situation where we'd want this to take effect. 
	// This is the same running into 2 walls where the angle between them is 60.
	const hsScalar threshold = hsScalarDegToRad(240);
	if (!ground && numContacts >= 2)
	{
		// Can probably do a special case for exactly 2 contacts. Not sure if it's worth it...

		fCollisionAngles.SetCount(numContacts);
		for (i = 0; i < numContacts; i++)
		{
			const Havok::ContactPoint *contact = fPhysical->GetContactPoint(i);
			fCollisionAngles[i] = hsATan2(contact->m_normal.y, contact->m_normal.x);
		}

		// numContacts is rarely larger than 6, so let's do a simple bubble sort.
		for (i = 0; i < numContacts; i++)
		{
			for (j = i + 1; j < numContacts; j++)
			{
				if (fCollisionAngles[i] > fCollisionAngles[j])
				{
					hsScalar tempAngle = fCollisionAngles[i];
					fCollisionAngles[i] = fCollisionAngles[j];
					fCollisionAngles[j] = tempAngle;
				}
			}
		}
		
		// sorted, now we check.
		for (i = 1; i < numContacts; i++)
		{
			if (fCollisionAngles[i] - fCollisionAngles[i - 1] >= threshold)
				break;
		}
		
		if (i == numContacts)
		{
			// We got to the end. Check the last with the first and make your decision.
			if (!(fCollisionAngles[0] - fCollisionAngles[numContacts - 1] >= (threshold - 2 * hsScalarPI)))
				ground = true;
		}
	}
*/

	bool ground = fController ? fController->GotGroundHit() : true;
	bool isPhysical = true;

	if (!fHitGroundInThisAge && isPhysical)
		fHitGroundInThisAge = true;	// if we're not pinned and we're not in an age yet, we are now.
	
	if (IsControlledFlight())
		fControlledFlightTime += (hsScalar)elapsed;
	if (fControlledFlightTime > kControlledFlightThreshold && numContacts > 0)
		EnableControlledFlight(false);
	
	if (ground || !isPhysical)
	{
		if (!IsControlledFlight() && !IsOnGround())
		{
			// The first ground contact in an age doesn't count.
// 			if (alreadyInAge)
// 			{
// 				hsVector3 vel;
// 				fPhysical->GetLinearVelocitySim(vel);
// 				fImpactVel = vel.fZ;
//				fTimeInAirPeak = (hsScalar)(fTimeInAir + elapsed);
//			}
			
			fWaitingForGround = false;				
		}			
		fTimeInAir = 0;
	}
	else if (elapsed < plSimulationMgr::GetInstance()->GetMaxDelta())
	{
		// If the simultation skipped a huge chunk of time, we didn't process the
		// collisions, which could trick us into thinking we've just gone a long
		// time without hitting ground. So we only count the time if this wasn't
		// the case.
		fTimeInAir += (hsScalar)elapsed;
	}
	
	
	// Tweakage so that we still fall under the right conditions.
	// If we're in controlled flight, or standing still with ground solidly under us (probe hit). We only use anim velocity.
// 	if (!IsControlledFlight() && !(ground && fProbeHitGround && fAnimPosVel.fX == 0 && fAnimPosVel.fY == 0)) 
// 	{
// 		hsVector3 curV;
// 		fPhysical->GetLinearVelocitySim(curV);
// 		fAnimPosVel.fZ = curV.fZ;
// 
// 		// Prevents us from going airborn from running up bumps/inclines.
// 		if (IsOnGround() && fAnimPosVel.fZ > 0.f)
// 			fAnimPosVel.fZ = 0.f;
// 		
// 		// Unless we're on the ground and moving, or standing still with a probe hit, we use the sim's other axes too.
// 		if (!(IsOnGround() && (fProbeHitGround || fAnimPosVel.fX != 0 || fAnimPosVel.fY != 0)))
// 		{
// 			fAnimPosVel.fX = curV.fX;
// 			fAnimPosVel.fY = curV.fY;
// 		}
// 	}
// 
// 	fPhysical->SetLinearVelocitySim(fAnimPosVel);
// 	fPhysical->SetSpin(fAnimAngVel + fTurnStr, hsVector3(0.0f, 0.0f, 1.0f));	
}
#endif


#if 0

/////////////////////////////////////////////////////////////////////////

plSimDefs::ActionType plHorizontalFreezeAction::GetType()
{
	return plSimDefs::kHorizontalFreeze;
}

void plHorizontalFreezeAction::apply(Havok::Subspace &s, Havok::hkTime time)
{
	double elapsed = time.asDouble() - getRefresh().asDouble();
	setRefresh(time);
	
	int numContacts = fPhysical->GetNumContacts();
	bool ground = false;
	const Havok::Vector3 straightUp(0.0f, 0.0f, 1.0f);	
	int i;
	for(i = 0; i < numContacts; i++)
	{
		const Havok::ContactPoint *contact = fPhysical->GetContactPoint(i);
		hsScalar dotUp = straightUp.dot(contact->m_normal);
		if (dotUp > .5)
			ground = true;
	}

	hsVector3 vel;
	fPhysical->GetLinearVelocitySim(vel);
	vel.fX = 0.0;
	vel.fY = 0.0;
	if (ground)
		vel.fZ = 0;
	fPhysical->SetLinearVelocitySim(vel);
	fPhysical->ClearContacts();
}
#endif
plSwimmingController::plSwimmingController(plSceneObject* rootObject, plAGApplicator* rootApp, plPhysicalControllerCore* controller)
:plAnimatedController(rootObject,rootApp,controller)
{
	if (controller)
		fSwimmingStrategy= TRACKED_NEW plSwimStrategy(controller);
	else
		fSwimmingStrategy = nil;
}
plSwimmingController::~plSwimmingController()
{
	delete fSwimmingStrategy;
}

plRidingAnimatedPhysicalController::plRidingAnimatedPhysicalController(plSceneObject* rootObject, plAGApplicator* rootApp, plPhysicalControllerCore* controller)
: plWalkingController(rootObject, rootApp, controller)
{
	if(controller)
		fWalkingStrategy = TRACKED_NEW plRidingAnimatedPhysicalStrategy(controller);
	else
		fWalkingStrategy = nil;
}
plRidingAnimatedPhysicalController::~plRidingAnimatedPhysicalController()
{
	delete fWalkingStrategy;
	fWalkingStrategy=nil;
}


//////////////////////////////////////////////////////////////////////////


/*
Purpose:

ANGLE_RAD_2D returns the angle in radians swept out between two rays in 2D.

Discussion:

Except for the zero angle case, it should be true that

ANGLE_RAD_2D(X1,Y1,X2,Y2,X3,Y3)
+ ANGLE_RAD_2D(X3,Y3,X2,Y2,X1,Y1) = 2 * PI

Modified:

19 April 1999

Author:

John Burkardt

Parameters:

Input, float X1, Y1, X2, Y2, X3, Y3, define the rays
( X1-X2, Y1-Y2 ) and ( X3-X2, Y3-Y2 ) which in turn define the
angle, counterclockwise from ( X1-X2, Y1-Y2 ).

Output, float ANGLE_RAD_2D, the angle swept out by the rays, measured
in radians.  0 <= ANGLE_DEG_2D < 2 PI.  If either ray has zero length,
then ANGLE_RAD_2D is set to 0.
*/

static float AngleRad2d ( float x1, float y1, float x3, float y3 )
{
	float value;
	float x;
	float y;

	x = ( x1 ) * ( x3 ) + ( y1 ) * ( y3 );
	y = ( x1 ) * ( y3 ) - ( y1 ) * ( x3 );

	if ( x == 0.0 && y == 0.0 ) {
		value = 0.0;
	}
	else 
	{
		value = atan2 ( y, x );

		if ( value < 0.0 ) 
		{
			value = (float)(value + TWO_PI);
		}
	}
	return value;
}

static hsBool LinearVelocity(hsVector3 &outputV, float elapsed, hsMatrix44 &prevMat, hsMatrix44 &curMat)
{
	bool result = false;

	hsPoint3 startPos(0.0f, 0.0f, 0.0f);					// default position (at start of anim)
	hsPoint3 prevPos = prevMat.GetTranslate();				// position previous frame
	hsPoint3 nowPos = curMat.GetTranslate();				// position current frame

	hsVector3 prev2Now = (hsVector3)(nowPos - prevPos);		// frame-to-frame delta

	if (fabs(prev2Now.fX) < 0.0001f && fabs(prev2Now.fY) < 0.0001f && fabs(prev2Now.fZ) < 0.0001f)
	{
		outputV.Set(0.f, 0.f, 0.f);
		result = true;
	} 
	else 
	{
		hsVector3 start2Now = (hsVector3)(nowPos - startPos);	// start-to-frame delta

		float prev2NowMagSqr = prev2Now.MagnitudeSquared();
		float start2NowMagSqr = start2Now.MagnitudeSquared();

		float dot = prev2Now.InnerProduct(start2Now);

		// HANDLING ANIMATION WRAPPING:
		// the vector from the animation origin to the current frame should point in roughly
		// the same direction as the vector from the previous animation position to the
		// current animation position.
		//
		// If they don't agree (dot < 0,) then we probably mpst wrapped around. 
		// The right answer would be to compare the current frame to the start of
		// the anim loop, but it's cheaper to cheat and return false,
		// telling the caller to use the previous frame's velocity.
		if (dot > 0.0f)
		{
			prev2Now /= elapsed;

			float xfabs = fabs(prev2Now.fX);
			float yfabs = fabs(prev2Now.fY);
			float zfabs = fabs(prev2Now.fZ);
			static const float maxVel = 20.0f;
			hsBool valid = xfabs < maxVel && yfabs < maxVel && zfabs < maxVel;

			if (valid)
			{
				outputV = prev2Now;
				result = true;
			}
		} 
	}

	return result;
}

static void AngularVelocity(hsScalar &outputV, float elapsed, hsMatrix44 &prevMat, hsMatrix44 &curMat)
{
	outputV = 0.f;
	hsScalar appliedVelocity = 0.0f;
	hsVector3 prevForward = GetYAxis(prevMat);
	hsVector3 curForward = GetYAxis(curMat);

	hsScalar angleSincePrev = AngleRad2d(curForward.fX, curForward.fY, prevForward.fX, prevForward.fY);
	hsBool sincePrevSign = angleSincePrev > 0.0f;
	if (angleSincePrev > hsScalarPI)
		angleSincePrev = angleSincePrev - TWO_PI;

	const hsVector3 startForward = hsVector3(0, -1.0, 0);	// the Y orientation of a "resting" armature....
	hsScalar angleSinceStart = AngleRad2d(curForward.fX, curForward.fY, startForward.fX, startForward.fY);
	hsBool sinceStartSign = angleSinceStart > 0.0f;
	if (angleSinceStart > hsScalarPI)
		angleSinceStart = angleSinceStart - TWO_PI;

	// HANDLING ANIMATION WRAPPING:
	// under normal conditions, the angle from rest to the current frame will have the same
	// sign as the angle from the previous frame to the current frame.
	// if it does not, we have (most likely) wrapped the motivating animation from frame n back
	// to frame zero, creating a large angle from the previous frame to the current one
	if (sincePrevSign == sinceStartSign)
	{
		// signs are the same; didn't wrap; use the frame-to-frame angle difference
		appliedVelocity = angleSincePrev / elapsed;	// rotation / time
		if (fabs(appliedVelocity) < 3)
		{
			outputV = appliedVelocity;
		}
	}
}
