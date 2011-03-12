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
#include "plPhysicalControllerCore.h"
#include "../plMessage/plLOSHitMsg.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../../NucleusLib/inc/plPhysical.h"
#include "../../NucleusLib/pnMessage/plCorrectionMsg.h"
#include "plSwimRegion.h"
#include "plArmatureMod.h" // for LOS enum type
#include "plMatrixChannel.h"
#include "hsTimer.h"
#include "../plPhysx/plSimulationMgr.h"
#include "../plPhysx/plPXPhysical.h"
#include "../pnMessage/plSetNetGroupIDMsg.h"
#define kSWIMRADIUS 1.1f
#define kSWIMHEIGHT 2.8f
#define kGENERICCONTROLLERRADIUS 1.1f
#define kGENERICCONTROLLERHEIGHT 2.8f

//#define kSWIMMINGCONTACTSLOPELIMIT (cosf(hsScalarDegToRad(80.f)))
const  hsScalar plMovementStrategy::kAirTimeThreshold = .1f; // seconds
bool CompareMatrices(const hsMatrix44 &matA, const hsMatrix44 &matB, float tolerance);
bool operator<(const plControllerSweepRecord left, const plControllerSweepRecord right)
{
	if(left.TimeHit < right.TimeHit) return true;
		else return false;
}
plMovementStrategy::plMovementStrategy(plPhysicalControllerCore* core)
{
	this->fTimeInAir=0.0f;
	fCore=core;
	fOwner=core->GetOwner();
	this->fPreferedControllerHeight=kGENERICCONTROLLERHEIGHT;
	this->fPreferedControllerWidth=kGENERICCONTROLLERRADIUS;
}
void plMovementStrategy::IApplyKinematic()
{
	// first apply sceneobject update to the kinematic
	plSceneObject* so = plSceneObject::ConvertNoRef(fOwner->ObjectIsLoaded());
	if (so)
	{
		// If we've been moved since the last physics update (somebody warped us),
		// update the physics before we apply velocity.
		const hsMatrix44& l2w = so->GetCoordinateInterface()->GetLocalToWorld();
		if (!CompareMatrices(l2w, fCore->GetLastGlobalLoc(), .0001f))
		{
			fCore->SetKinematicLoc(l2w);
			//fCore->SetGlobalLoc(l2w);
		}
	}
}
plPhysicalControllerCore::~plPhysicalControllerCore()
{
}
void plPhysicalControllerCore::Apply(hsScalar delSecs)
{
	fSimLength=delSecs;
	hsAssert(fMovementInterface, "plPhysicalControllerCore::Apply() missing a movement interface");
	if(fMovementInterface)fMovementInterface->Apply(delSecs);
}
void plPhysicalControllerCore::PostStep(hsScalar delSecs)
{
	hsAssert(fMovementInterface, "plPhysicalControllerCore::PostStep() missing a movement interface");
	if(fMovementInterface)fMovementInterface->PostStep(delSecs);
}
void plPhysicalControllerCore::Update(hsScalar delSecs)
{
	hsAssert(fMovementInterface, "plPhysicalControllerCore::Update() missing a movement interface");
	if(fMovementInterface)fMovementInterface->Update(delSecs);
	
}
void plPhysicalControllerCore::SendCorrectionMessages()
{
	plSceneObject* so = plSceneObject::ConvertNoRef(fOwner->ObjectIsLoaded());
	plCorrectionMsg* corrMsg = TRACKED_NEW plCorrectionMsg;
	corrMsg->fLocalToWorld = fLastGlobalLoc;
	corrMsg->fLocalToWorld.GetInverse(&corrMsg->fWorldToLocal);
	corrMsg->fDirtySynch = true;
	// Send the new position to the plArmatureMod and the scene object
	const plArmatureMod* armMod = plArmatureMod::ConvertNoRef(so->GetModifierByType(plArmatureMod::Index()));
	if (armMod)
		corrMsg->AddReceiver(armMod->GetKey());
	corrMsg->AddReceiver(fOwner);
	corrMsg->Send();
}
plPhysicalControllerCore::plPhysicalControllerCore(plKey OwnerSceneObject, hsScalar height, hsScalar radius)
:fMovementInterface(nil)
,fOwner(OwnerSceneObject)
,fHeight(height)
,fRadius(radius)
,fWorldKey(nil)
,fLinearVelocity(0.f,0.f,0.f)
,fAngularVelocity(0.f)
,fAchievedLinearVelocity(0.0f,0.0f,0.0f)
,fLocalPosition(0.0f,0.0f,0.0f)
,fLocalRotation(0.0f,0.0f,0.0f,1.0f)
,fSeeking(false)
,fEnabled(true)
,fEnableChanged(false)
,fLOSDB(plSimDefs::kLOSDBNone)
,fDisplacementThisStep(0.f,0.f,0.f)
,fSimLength(0.f)
,fKinematic(false)
,fKinematicEnableNextUpdate(false)
,fNeedsResize(false)
,fPushingPhysical(nil)
{
}

void plPhysicalControllerCore::UpdateSubstepNonPhysical()
{
	// When we're in non-phys or a behavior we can't go through the rest of the function
	// so we need to get out early, but we need to update the current position if we're
	// in a subworld.
	plSceneObject* so = plSceneObject::ConvertNoRef(fOwner->ObjectIsLoaded());
	const plCoordinateInterface* ci = GetSubworldCI();
	if (ci && so)
	{
		const hsMatrix44& soL2W = so->GetCoordinateInterface()->GetLocalToWorld();
		const hsMatrix44& ciL2W = ci->GetLocalToWorld();
		hsMatrix44 l2w =GetPrevSubworldW2L()* soL2W;
		l2w = ciL2W * l2w;
		hsMatrix44 w2l;
		l2w.GetInverse(&w2l);
		((plCoordinateInterface*)so->GetCoordinateInterface())->SetTransform(l2w, w2l);
		((plCoordinateInterface*)so->GetCoordinateInterface())->FlushTransform();
		SetGlobalLoc(l2w);
	}


}
void plPhysicalControllerCore::CheckAndHandleAnyStateChanges()
{
	if (IsEnabledChanged())HandleEnableChanged();
	if (IsKinematicChanged())HandleKinematicChanged();
	if (IsKinematicEnableNextUpdate())HandleKinematicEnableNextUpdate();
}
void plPhysicalControllerCore::MoveActorToSim()
{
		// Get the current position of the physical
		hsPoint3 curLocalPos;
		hsPoint3 lastLocalPos;
		GetPositionSim(curLocalPos);
		MoveKinematicToController(curLocalPos);
		lastLocalPos=GetLocalPosition();
		fDisplacementThisStep=  hsVector3(curLocalPos - lastLocalPos);
		fLocalPosition = curLocalPos;
		if(fSimLength>0.0f)
		fAchievedLinearVelocity=fDisplacementThisStep/fSimLength;
		else fAchievedLinearVelocity.Set(0.0f,0.0f,0.0f);
}
void plPhysicalControllerCore::IncrementAngle(hsScalar deltaAngle)
{
	hsScalar angle;
	hsVector3 axis;
	fLocalRotation.NormalizeIfNeeded();
	fLocalRotation.GetAngleAxis(&angle, &axis);
	// adjust it (quaternions are weird...)
	if (axis.fZ < 0)
		angle = (2*hsScalarPI) - angle; // axis is backwards, so reverse the angle too
	angle += deltaAngle;
	// make sure we wrap around
	if (angle < 0)
		angle = (2*hsScalarPI) + angle; // angle is -, so this works like a subtract
	if (angle >= (2*hsScalarPI))
		angle = angle - (2*hsScalarPI);
	// and set the new angle
	fLocalRotation.SetAngleAxis(angle, hsVector3(0,0,1));
}

void plPhysicalControllerCore::UpdateWorldRelativePos()
{
		
	// Apply rotation and translation
	fLocalRotation.MakeMatrix(&fLastGlobalLoc);
	fLastGlobalLoc.SetTranslate(&fLocalPosition);
	// Localize to global coords if in a subworld
	const plCoordinateInterface* ci = GetSubworldCI();
	if (ci)
	{
		const hsMatrix44& l2w = ci->GetLocalToWorld();
		fLastGlobalLoc = l2w * fLastGlobalLoc;
	}
}
plPhysical* plPhysicalControllerCore::GetPushingPhysical()
{
	return fPushingPhysical;
}
const hsVector3& plPhysicalControllerCore::GetLinearVelocity() 
{
	return fLinearVelocity;
}
bool plPhysicalControllerCore::GetFacingPushingPhysical()
{
	return fFacingPushingPhysical;
}
///////////////////////////
//Walking Strategy
void plWalkingStrategy::Apply(hsScalar delSecs)
{
	//Apply Should Only be Called from a PhysicalControllerCore
	hsAssert(fCore,"No Core shouldn't be Applying");
	UInt32 collideFlags =
		1<<plSimDefs::kGroupStatic |
		1<<plSimDefs::kGroupAvatarBlocker |
		1<<plSimDefs::kGroupDynamic;
	if(!fCore->IsSeeking())
	{
		collideFlags|=(1<<plSimDefs::kGroupExcludeRegion);
	}
	bool OnTopOfAnimatedPhys=false;
	hsVector3 LinearVelocity=fCore->GetLinearVelocity();
	hsVector3 AchievedLinearVelocity=fCore->GetAchievedLinearVelocity();
	hsPoint3 positionBegin;
	fCore->GetPositionSim(positionBegin);
	bool recovered=false;
	if (fCore->IsKinematic())
	{
			plSceneObject* so = plSceneObject::ConvertNoRef(fOwner->ObjectIsLoaded());
			if (so)
			{
				// If we've been moved since the last physics update (somebody warped us),
				// update the physics before we apply velocity.
				const hsMatrix44& l2w = so->GetCoordinateInterface()->GetLocalToWorld();
				if (!CompareMatrices(l2w, fCore->GetLastGlobalLoc(), .0001f))
				{
					fCore->SetKinematicLoc(l2w);
					fCore->SetGlobalLoc(l2w);
				}
			}
		return;
	}

	if (!fCore->IsEnabled())
		return;

	bool gotGroundHit = fGroundHit;
	fGroundHit = false;
	
	fCore->SetPushingPhysical(nil);
	fCore->SetFacingPushingPhysical( false);
	plSceneObject* so = plSceneObject::ConvertNoRef(fOwner->ObjectIsLoaded());
	if (so)
	{
		static const float kGravity = -32.f;
		// If we've been moved since the last physics update (somebody warped us),
		// update the physics before we apply velocity.
		const hsMatrix44& l2w = so->GetCoordinateInterface()->GetLocalToWorld();
		if (!CompareMatrices(l2w, fCore->GetLastGlobalLoc(), .0001f))
			fCore->SetGlobalLoc(l2w);

		// Convert our avatar relative velocity to subworld relative
		if (!LinearVelocity.IsEmpty())
		{
			LinearVelocity = l2w * LinearVelocity;
			const plCoordinateInterface* subworldCI = fCore->GetSubworldCI();
			if (subworldCI)
				LinearVelocity = subworldCI->GetWorldToLocal() * LinearVelocity;
		}

		// Add in gravity if the avatar's z velocity isn't being set explicitly
		// (Add in a little fudge factor, since the animations usually add a
		// tiny bit of z.)
		if (hsABS(LinearVelocity.fZ) < 0.001f)
		{
			// Get our previous z velocity.  If we're on the ground, clamp it to zero at
			// the largest, so we won't launch into the air if we're running uphill.
			hsScalar prevZVel = AchievedLinearVelocity.fZ;
			if (IsOnGround())
				prevZVel = hsMinimum(prevZVel, 0.f);
			hsScalar grav = kGravity * delSecs;
			// If our gravity contribution isn't high enough this frame, we won't
			// report a collision even when standing on solid ground.
			hsScalar maxGrav = -.001f / delSecs;
			if (grav > maxGrav)
				grav = maxGrav;
			LinearVelocity.fZ = prevZVel + grav;
		}

		// If we're airborne and the velocity isn't set, use the velocity from
		// the last frame so we maintain momentum.
		if (!IsOnGround() && LinearVelocity.fX == 0.f && LinearVelocity.fY == 0.f)
		{
			LinearVelocity.fX = AchievedLinearVelocity.fX;
			LinearVelocity.fY = AchievedLinearVelocity.fY;
		}
		if (!IsOnGround() || IsOnFalseGround())
		{
			// We're not on solid ground, so we should be sliding against whatever
			// we're hitting (like a rock cliff). Each vector in fSlidingNormals is
			// the surface normal of a collision that's too steep to be ground, so
			// we project our current velocity onto that plane and slide along the
			// wall.
			//
			// Also, sometimes PhysX reports a bunch of collisions from the wall,
			// but nothing from underneath (when there should be). So if we're not
			// touching ground, we offset the avatar in the direction of the 
			// surface normal(s). This doesn't fix the issue 100%, but it's a hell
			// of a lot better than nothing, and suitable duct tape until a future
			// PhysX revision fixes the issue.
			//
			// Yes, there's room for optimization here if we care.
			hsVector3 offset(0.f, 0.f, 0.f);
			for (int i = 0; i < fContactNormals.GetCount(); i++)
			{
				offset += fContactNormals[i];
				hsVector3 velNorm = LinearVelocity;

				if (velNorm.MagnitudeSquared() > 0)
					velNorm.Normalize();

				if (velNorm * fContactNormals[i] < 0)
				{
					hsVector3 proj = (velNorm % fContactNormals[i]) % fContactNormals[i];
					if (velNorm * proj < 0)
						proj *= -1.f;
					LinearVelocity = LinearVelocity.Magnitude() * proj;
				}
			}
			if (offset.MagnitudeSquared() > 0)
			{
				// 5 ft/sec is roughly the speed we walk backwards.
				// The higher the value, the less likely you'll trip
				// the bug, and this seems reasonable.
				offset.Normalize();
				LinearVelocity += offset * 5.0f;
			}
		}
		//make terminal velocity equal to k. it is wrong but has been this way and
		//don't want to break any puzzles. on top of that it will reduce tunneling behavior
		if(LinearVelocity.fZ<kGravity)LinearVelocity.fZ=kGravity;
		
		

		
		fCore->SetLinearVelocity(LinearVelocity);
		// Scale the velocity to our actual step size (by default it's feet/sec)
		hsVector3 vel(LinearVelocity.fX * delSecs, LinearVelocity.fY * delSecs, LinearVelocity.fZ * delSecs);
		unsigned int colFlags = 0;
		fGroundHit = false;
		fFalseGround = false;
		fContactNormals.Swap(fPrevSlidingNormals);
		fContactNormals.SetCount(0);
		fCore->Move(vel, collideFlags,  colFlags);
		ICheckForFalseGround();
		//if(fReqMove2) fCore->Move2(vel);
		/*If the Physx controller  thinks we have a collision from below, need to make sure we
		have at least have false ground, otherwise Autostepping can send us into the air, and we will some times 
		float/panic link. For some reason the NxControllerHitReport does not always send messages
		regarding Controller contact with ground plane, but will (almost) always return NXCC_COLLISION_DOWN
		with the move method.
		*/
		if((colFlags&kBottom ) &&(fGroundHit==false))
		{
			fFalseGround=true;
		}
		
		if(colFlags&kTop)
		{
			fHitHead=true;
			//Did you hit your head on a dynamic?
			//with Physx's wonderful controller hit report vs flags issues we need to actually sweep to see
			std::multiset< plControllerSweepRecord > HitsDynamic;
			UInt32 testFlag=1<<plSimDefs::kGroupDynamic;
			hsPoint3 startPos;
			hsPoint3 endPos;
			fCore->GetPositionSim(startPos);
			endPos= startPos + vel;
			int NumObjsHit=fCore->SweepControllerPath(startPos, endPos, true, false, testFlag, HitsDynamic);
			if(NumObjsHit>0)
			{
				for(std::multiset< plControllerSweepRecord >::iterator curObj= HitsDynamic.begin();
					curObj!=HitsDynamic.end(); curObj++)
				{

					hsAssert(curObj->ObjHit,"We allegedly hit something, but there is no plasma physical associated with it");
					if(curObj->ObjHit)
					{//really we shouldn't have to check hitObj should be nil only if we miss, or the physX object
						//doesn't have a user data associated with this either way this just shouldn't happen
						hsVector3 hitObjVel;
						curObj->ObjHit->GetLinearVelocitySim(hitObjVel);
						hsVector3 relativevel=LinearVelocity-hitObjVel;
						curObj->ObjHit->SetHitForce(relativevel * 10.0f * (*curObj).ObjHit->GetMass(), (*curObj).locHit);
					}
				}
				HitsDynamic.clear();
			}
		}
	}
}
void plWalkingStrategy::ICheckForFalseGround()
{
	if (fGroundHit)
		return; // Already collided with "real" ground.

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
	int i, j;
	const hsScalar threshold = hsScalarDegToRad(240.f);
	int numContacts = fContactNormals.GetCount() + fPrevSlidingNormals.GetCount();
	if (numContacts >= 2)
	{
		// For extra fun... PhysX will actually report some collisions every other frame, as though
		// we're bouncing back and forth between the two (or more) objects blocking us. So it's not
		// enough to look at this frame's collisions, we have to check previous frames too.
		hsTArray<hsScalar> fCollisionAngles;
		fCollisionAngles.SetCount(numContacts);
		int angleIdx = 0;
		for (i = 0; i < fContactNormals.GetCount(); i++, angleIdx++)
		{
			fCollisionAngles[angleIdx] = hsATan2(fContactNormals[i].fY, fContactNormals[i].fX);
		}
		for (i = 0; i < fPrevSlidingNormals.GetCount(); i++, angleIdx++)
		{
			fCollisionAngles[angleIdx] = hsATan2(fPrevSlidingNormals[i].fY, fPrevSlidingNormals[i].fX);
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
				fFalseGround = true;
		}
	}
}
void plWalkingStrategy::Update(hsScalar delSecs)
{
	//Update Should Only be Called from a PhysicalControllerCore
	hsAssert(fCore,"Running Update: but have no Core");
	hsScalar AngularVelocity=fCore->GetAngularVelocity();
	hsVector3 LinearVelocity=fCore->GetLinearVelocity();

	if (!fCore->IsEnabled() || fCore->IsKinematic())
	{
		fCore->UpdateSubstepNonPhysical();
		return;
	}
	fCore->CheckAndHandleAnyStateChanges();
	if (!fGroundHit && !fFalseGround)
		fTimeInAir += delSecs;
	else
		fTimeInAir = 0.f;
	plSceneObject* so = plSceneObject::ConvertNoRef(fOwner->ObjectIsLoaded());
	if (so)
	{
		fCore->MoveActorToSim();
		if (AngularVelocity != 0.f)
		{
			hsScalar deltaAngle=AngularVelocity*delSecs;
			fCore->IncrementAngle( deltaAngle);
		}
		// We can't only send updates when the physical position changes because the
		// world relative position may be changing all the time if we're in a subworld.
		fCore->UpdateWorldRelativePos();
		fCore->SendCorrectionMessages();
		bool headhit=fHitHead;
		fHitHead=false;
		hsVector3 AchievedLinearVelocity;
		AchievedLinearVelocity = fCore->DisplacementLastStep();
		AchievedLinearVelocity=AchievedLinearVelocity/delSecs;
		
		/*if we hit our head the sweep api might try to 
		move us laterally to  go as high as we requested kind of like autostep, to top it off the
		way the NxCharacter and the sweep api work as a whole  NxControllerHitReport::OnShapeHit
		wont be called regarding the head blow. 
		if we are airborne: with out this we will gain large amounts of velocity in the x y plane
		and on account of fAchievedLinearVelocity being used in the next step we will fly sideways 
		*/
		if(headhit&&!(fGroundHit||fFalseGround))
		{
			//we have hit our head and we don't have anything beneath our feet			
			//not really friction just a way to make it seem more realistic keep between 0 and 1
			hsScalar headFriction=0.0f;
			AchievedLinearVelocity.fX=(1.0f-headFriction)*LinearVelocity.fX;
			AchievedLinearVelocity.fY=(1.0f-headFriction)*LinearVelocity.fY;
			//only clamping when hitting head and going upwards, if going down leave it be
			// this should only occur when going down stairwells with low ceilings like in cleft
			//kitchen area
			if(AchievedLinearVelocity.fZ>0.0f)
			{
				AchievedLinearVelocity.fZ=0.0f;
			}
			fCore->OverrideAchievedVelocity(AchievedLinearVelocity);
		}
			fCore->OverrideAchievedVelocity(AchievedLinearVelocity);
		// Apply angular velocity
	}

	LinearVelocity.Set(0.f, 0.f, 0.f);
	AngularVelocity = 0.f;
	fCore->SetVelocities(LinearVelocity,AngularVelocity);

}


void plWalkingStrategy::IAddContactNormals(hsVector3& vec)
{	
	//TODO: ADD in functionality to Adjust walkable slope for controller, also apply that in here
	hsScalar dot = vec * kAvatarUp;
	if ( dot >= kSLOPELIMIT ) fGroundHit=true;
	else plMovementStrategySimulationInterface::IAddContactNormals(vec);
}

//swimming strategy
plSwimStrategy::plSwimStrategy(plPhysicalControllerCore* core)
	:plMovementStrategy(core)
	,fOnGround(false)
	,fHadContacts(false)
	,fBuoyancy(0.f)
	,fSurfaceHeight(0.0f)
	,fCurrentRegion(nil)
{
	fPreferedControllerHeight=kSWIMHEIGHT;
	fPreferedControllerWidth=kSWIMRADIUS;
	fCore->SetMovementSimulationInterface(this);
}
void plSwimStrategy::IAdjustBuoyancy()
{
	// "surface depth" refers to the depth our handle object should be below
	// the surface for the avatar to be "at the surface"
	static const float surfaceDepth = 4.0f;
	// 1.0 = neutral buoyancy
	// 0 = no buoyancy (normal gravity)
	// 2.0 = opposite of gravity, floating upwards
	static const float buoyancyAtSurface = 1.0f;

	if (fCurrentRegion == nil)
	{
		fBuoyancy = 0.f;
		return;
	}

	hsMatrix44 l2w, w2l;
	hsPoint3 posSim;
	fCore->GetPositionSim(posSim);
	float depth = fSurfaceHeight - posSim.fZ;	
	//this isn't a smooth transition but hopefully it won't be too obvious
	if(depth<=0.0)//all the away above water
		fBuoyancy = 0.f; // Same as being above ground. Plain old gravity.
	else if(depth >= 5.0f) fBuoyancy=3.0f;//completely Submereged
	else fBuoyancy =(depth/surfaceDepth );
	
}
void plSwimStrategy::Apply(hsScalar delSecs)
{
	hsAssert(fCore,"PlSwimStrategy::Apply No Core shouldn't be Applying");
	UInt32 collideFlags =
		1<<plSimDefs::kGroupStatic |
		1<<plSimDefs::kGroupAvatarBlocker |
		1<<plSimDefs::kGroupDynamic;
	if(!fCore->IsSeeking())
	{
		collideFlags|=(1<<plSimDefs::kGroupExcludeRegion);
	}

	hsVector3 LinearVelocity=fCore->GetLinearVelocity();
	hsVector3 AchievedLinearVelocity=fCore->GetAchievedLinearVelocity();
	if (fCore->IsKinematic())
	{
		plSceneObject* so = plSceneObject::ConvertNoRef(fOwner->ObjectIsLoaded());
		if (so)
		{
			// If we've been moved since the last physics update (somebody warped us),
			// update the physics before we apply velocity.
			const hsMatrix44& l2w = so->GetCoordinateInterface()->GetLocalToWorld();
			if (!CompareMatrices(l2w, fCore->GetLastGlobalLoc(), .0001f))
			{
				fCore->SetKinematicLoc(l2w);
				fCore->SetGlobalLoc(l2w);
			}
		}
		return;
		
	}
	if (!fCore->IsEnabled())
		return;
	
	fCore->SetPushingPhysical(nil);
	fCore->SetFacingPushingPhysical( false);
	fHadContacts=false;
	fOnGround=false;
	plSceneObject* so = plSceneObject::ConvertNoRef(fOwner->ObjectIsLoaded());
	if (so)
	{
		// If we've been moved since the last physics update (somebody warped us),
		// update the physics before we apply velocity.
		const hsMatrix44& l2w = so->GetCoordinateInterface()->GetLocalToWorld();
		if (!CompareMatrices(l2w, fCore->GetLastGlobalLoc(), .0001f))
			fCore->SetGlobalLoc(l2w);

		// Convert our avatar relative velocity to subworld relative
		if (!LinearVelocity.IsEmpty())
		{
			LinearVelocity = l2w * LinearVelocity;
			const plCoordinateInterface* subworldCI = fCore->GetSubworldCI();
			if (subworldCI)
				LinearVelocity = subworldCI->GetWorldToLocal() * LinearVelocity;
		}
		IAdjustBuoyancy();
		hsScalar zacc;
		hsScalar retardent=0.0f;
		static hsScalar FinalBobSpeed=0.5f;
		//trying to dampen the oscillations
		if((AchievedLinearVelocity.fZ>FinalBobSpeed)||(AchievedLinearVelocity.fZ<-FinalBobSpeed))
			retardent=AchievedLinearVelocity.fZ *-.90f;
		zacc=(1-fBuoyancy)*-32.f + retardent;
		
		hsVector3 linCurrent(0.0f,0.0f,0.0f);
		hsScalar angCurrent = 0.f;
		if (fCurrentRegion != nil)
		{
		
			fCurrentRegion->GetCurrent(fCore, linCurrent, angCurrent, delSecs);
			//fAngularVelocity+= angCurrent;
		}
		hsVector3 vel(LinearVelocity.fX , LinearVelocity.fY , AchievedLinearVelocity.fZ+ LinearVelocity.fZ );
		vel.fZ= vel.fZ + zacc*delSecs;
		if(fCurrentRegion!=nil){
			if (vel.fZ > fCurrentRegion->fMaxUpwardVel)
			{
				vel.fZ = fCurrentRegion->fMaxUpwardVel;
			}
			vel+= linCurrent;
		}
		static const float kGravity = -32.f;
		if(vel.fZ<kGravity)
		{//applying this terminal velocity just to avoid shooting 100 feet below the surface
			// and losing our surface ray cast
			vel.fZ =kGravity;
		}
		hsVector3 displacement= vel*delSecs;
		unsigned int colFlags = 0;
		fContactNormals.SetCount(0);
		fCore->Move(displacement,collideFlags,colFlags);
		if((colFlags&kBottom)||(colFlags&kSides))fHadContacts=true;
		hsScalar angvel=fCore->GetAngularVelocity();
		fCore->SetAngularVelocity(angvel +angCurrent);
	}
}
void plSwimStrategy::Update(hsScalar delSecs)
{
	hsAssert(fCore,"Running Update: but have no Core");
	hsScalar AngularVelocity=fCore->GetAngularVelocity();
	hsVector3 LinearVelocity=fCore->GetLinearVelocity();
	if (!fCore->IsEnabled() || fCore->IsKinematic())
	{
		fCore->UpdateSubstepNonPhysical();
		return;
	}
	fCore->CheckAndHandleAnyStateChanges();
	plSceneObject* so = plSceneObject::ConvertNoRef(fOwner->ObjectIsLoaded());
	if (so)
	{
		fCore->MoveActorToSim();

		if (AngularVelocity != 0.f)
		{
			hsScalar deltaAngle=AngularVelocity*delSecs;
			fCore->IncrementAngle( deltaAngle);
		}
		fCore->UpdateWorldRelativePos();
		fCore->SendCorrectionMessages();	
	}
	LinearVelocity.Set(0.f, 0.f, 0.f);
	AngularVelocity = 0.f;
	fCore->SetVelocities(LinearVelocity,AngularVelocity);
}
void plSwimStrategy::IAddContactNormals(hsVector3& vec)
{	
	//TODO: ADD in functionality to Adjust walkable slope for controller, also apply that in here
	hsScalar dot = vec * kAvatarUp;
	if ( dot >= kSLOPELIMIT )
	{
		fOnGround=true;
	//	fHadContacts=true;	
	}
	else plMovementStrategySimulationInterface::IAddContactNormals(vec);
}
void plSwimStrategy::SetSurface(plSwimRegionInterface *region, hsScalar surfaceHeight)
{
	fCurrentRegion=region;
	fSurfaceHeight=surfaceHeight;
}
void plRidingAnimatedPhysicalStrategy::Apply(hsScalar delSecs)
{
	hsVector3 LinearVelocity=fCore->GetLinearVelocity();
	hsVector3 AchievedLinearVelocity=fCore->GetAchievedLinearVelocity();
	if (fCore->IsKinematic())
	{	
		//want to make sure nothing funky happens in the sim
		IApplyKinematic();
		return;
	}
	if (!fCore->IsEnabled())
	   	return;

	//need to sweep ahead to see what we might hit. 
	// if we hit anything we should probably apply the force that would normally be applied in

	
	fCore->SetPushingPhysical(nil);
	fCore->SetFacingPushingPhysical( false);
	plSceneObject* so = plSceneObject::ConvertNoRef(fOwner->ObjectIsLoaded());
	hsPoint3 startPos, desiredDestination, endPos;
	fCore->GetPositionSim(startPos);
	UInt32 collideFlags =
	1<<plSimDefs::kGroupStatic |
	1<<plSimDefs::kGroupAvatarBlocker |
	1<<plSimDefs::kGroupDynamic;
	std::multiset<plControllerSweepRecord> GroundHitRecords;
	int possiblePlatformCount =fCore->SweepControllerPath(startPos, startPos + hsPoint3(0.f,0.f, -0.002f), true, true, collideFlags, GroundHitRecords);
	float maxPlatformVel = - FLT_MAX;
	int platformCount=0;
	fGroundHit = false;
	if(possiblePlatformCount)
	{
		
		std::multiset<plControllerSweepRecord>::iterator curRecord; 

		for(curRecord = GroundHitRecords.begin(); curRecord != GroundHitRecords.end(); curRecord++)
		{
			hsBool groundlike=false;
			if((curRecord->locHit.fZ - startPos.fZ)<= .2) groundlike= true;
			if(groundlike)
			{
				if(curRecord->ObjHit !=nil)
				{
					hsVector3 vel;
					curRecord->ObjHit->GetLinearVelocitySim(vel);
					if(vel.fZ > maxPlatformVel)
					{
						maxPlatformVel= vel.fZ;
					}
				}
				platformCount ++;
				fGroundHit = true;
			}
		}
	}
	
	
	
	bool gotGroundHit = fGroundHit;
	if (so)
	{
		
		// If we've been moved since the last physics update (somebody warped us),
		// update the physics before we apply velocity.
		const hsMatrix44& l2w = so->GetCoordinateInterface()->GetLocalToWorld();
		if (!CompareMatrices(l2w, fCore->GetLastGlobalLoc(), .0001f))
			fCore->SetGlobalLoc(l2w);

		// Convert our avatar relative velocity to subworld relative
		if (!LinearVelocity.IsEmpty())
		{
			LinearVelocity = l2w * LinearVelocity;
			const plCoordinateInterface* subworldCI = fCore->GetSubworldCI();
			if (subworldCI)
				LinearVelocity = subworldCI->GetWorldToLocal() * LinearVelocity;
		}
		
		if(!IsOnGround())
		{
			if(!fNeedVelocityOverride)
			{
				LinearVelocity.fZ= AchievedLinearVelocity.fZ;
			}
			else
			{
				LinearVelocity = fOverrideVelocity;
			}
		}
		if(fStartJump)
		{
			LinearVelocity.fZ =12.0f;
		}
		if(platformCount)
		{
			LinearVelocity.fZ = LinearVelocity.fZ + maxPlatformVel;
		}

		//probably neeed to do something with contact normals in here
		//for false ground stuff
		
		fFalseGround = false;
		hsVector3 testLength = LinearVelocity * delSecs + hsVector3(0.0, 0.0, -0.00f);
	//
		hsPoint3 desiredDestination= startPos + testLength;
		if(!IsOnGround())
		{
			if(ICheckMove(startPos, desiredDestination))
			{//we can get there soley by the LinearVelocity
				
				fNeedVelocityOverride =false;
			}
			else
			{
				
 				fNeedVelocityOverride =true;
				fOverrideVelocity = LinearVelocity;
				fOverrideVelocity.fZ -=  delSecs * 32.f;
			}
		}
		else
		{
			fNeedVelocityOverride =false;
		}

		fCore->SetLinearVelocity(LinearVelocity);
	
	}
}
bool plRidingAnimatedPhysicalStrategy::ICheckMove(const hsPoint3& startPos, const hsPoint3& desiredPos)
{
	//returns false if it believes the end result can't be obtained by pure application of velocity (collides into somthing that it can't climb up)
	//used as a way to check if it needs to hack getting there like in jumping
	
	UInt32 collideFlags =
	1<<plSimDefs::kGroupStatic |
	1<<plSimDefs::kGroupAvatarBlocker |
	1<<plSimDefs::kGroupDynamic;
	bool hitBottomOfCapsule=false;
	bool hitOther=false;
	float timeOfOtherHits = FLT_MAX;
	float timeFirstBottomHit =  -1.0;
	if(!fCore->IsSeeking())
	{
		collideFlags|=(1<<plSimDefs::kGroupExcludeRegion);
	}
	if((desiredPos.fZ - startPos.fZ) < -1.f)//we will let gravity take care of it when falling
		return true;
	fContactNormals.SetCount(0);
	std::multiset< plControllerSweepRecord > DynamicHits;
	int NumberOfHits=fCore->SweepControllerPath(startPos, desiredPos, true, true, collideFlags, DynamicHits);

	hsPoint3 stepFromPoint;
	hsVector3  movementdir(&startPos, &desiredPos);
	movementdir.Normalize();
	if(NumberOfHits)
	{
		hsPoint3 initBottomPos;
		fCore->GetPositionSim(initBottomPos);
        std::multiset< plControllerSweepRecord >::iterator cur;
		hsVector3 testLength(desiredPos - startPos);
		bool freeMove=true;
		for(cur = DynamicHits.begin();	cur != DynamicHits.end(); cur++)
		{
			if(movementdir.InnerProduct(cur->Norm)>0.01f)
			{
				hsVector3 topOfBottomHemAtTimeT=hsVector3(initBottomPos + testLength * cur->TimeHit );
				topOfBottomHemAtTimeT.fZ = topOfBottomHemAtTimeT.fZ + fCore->GetControllerWidth();
				if(cur->locHit.fZ <= (topOfBottomHemAtTimeT.fZ -.5f))
				{
					hitBottomOfCapsule=true;
					hsVector3 norm= hsVector3(-1*(cur->locHit-topOfBottomHemAtTimeT));
					norm.Normalize();
					IAddContactNormals(norm);
				}
				else
				{
					return false;
				}
			}

		}
		return true;
	}
	else
	{
		return true;
	}
	
}
void plRidingAnimatedPhysicalStrategy::Update(hsScalar delSecs)
{
	if (!fCore->IsEnabled() || fCore->IsKinematic())
	{
		fCore->UpdateSubstepNonPhysical();
		return;
	}	
	fCore->CheckAndHandleAnyStateChanges();
}
void plRidingAnimatedPhysicalStrategy::PostStep(hsScalar delSecs)
{
	if(!(!fCore->IsEnabled() || fCore->IsKinematic()))
	{
		if (!fGroundHit && !fFalseGround)
			fTimeInAir += delSecs;
		else
			fTimeInAir = 0.f;
		hsVector3 AchievedLinearVelocity, LinearVelocity;
		AchievedLinearVelocity = fCore->GetLinearVelocity();
		hsScalar AngularVelocity=fCore->GetAngularVelocity();
		fCore->OverrideAchievedVelocity(AchievedLinearVelocity);
		plSceneObject* so = plSceneObject::ConvertNoRef(fOwner->ObjectIsLoaded());
		if (so)
		{
			fCore->UpdateControllerAndPhysicalRep();
			if (AngularVelocity != 0.f)
			{
				hsScalar deltaAngle=AngularVelocity*delSecs;
				fCore->IncrementAngle( deltaAngle);
			}
			fCore->UpdateWorldRelativePos();
			fCore->SendCorrectionMessages();	
		}
		LinearVelocity.Set(0.f, 0.f, 0.f);
		AngularVelocity = 0.f;
		fCore->SetVelocities(LinearVelocity, AngularVelocity);
	}
	fStartJump = false;
}
