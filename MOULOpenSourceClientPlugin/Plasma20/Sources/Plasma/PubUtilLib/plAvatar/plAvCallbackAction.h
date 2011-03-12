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
#ifndef PL_HK_CALLBACK_ACTION_H
#define PL_HK_CALLBACK_ACTION_H

#include "hsGeometry3.h"
#include "hsMatrix44.h"
#include "hsTemplates.h"
#include "../pnKeyedObject/plKey.h"
#include "../plPhysical/plSimDefs.h"
#include "../pnMessage/plMessage.h"
#include "plPhysicalControllerCore.h"
class plLOSHitMsg;
class plAGApplicator;
class plSceneObject;
class plPhysical;
class plAvatarController;
class plCoordinateInterface;
class plPhysicalControllerCore;
// Used by the other controllers to actually move the avatar.  The actual
// implementation is in the physics system.
/*class plPhysicalController
{
public:
	// Implemented in the physics system.  If you're linking this without that for
	// some reason, just stub this function out.
	//
	// Pass in the key to the root sceneobject for the avatar
	static plPhysicalController* Create(plKey ownerSO, hsScalar height, hsScalar width);

	virtual ~plPhysicalController() {}

	// A disabled avatar doesn't move or accumulate air time if he's off the ground.
	virtual void Enable(bool enable) = 0;
	virtual bool IsEnabled() const = 0;

	// Set the LOS DB this avatar will be in (only one)
	virtual void SetLOSDB(plSimDefs::plLOSDB losDB) = 0;

	// Call this once per frame with the velocities of the avatar in avatar space.
	virtual void SetVelocities(const hsVector3& linearVel, hsScalar angVel) = 0;

	// Gets the actual velocity we achieved in the last step (relative to our subworld)
	virtual const hsVector3& GetLinearVelocity() const = 0;
	virtual void ResetAchievedLinearVelocity() = 0;

	// Get and set the current subworld for the avatar.  Use nil for the main world.
	virtual plKey GetSubworld() const = 0;
	virtual void SetSubworld(plKey world) = 0;

	// If IsOnGround returns false, GetAirTime will tell you how long the avatar
	// has been airborne.  Use ResetAirTime to reset the air time to zero, for
	// cases like when the avatar spawns into a new age.
	virtual bool IsOnGround() const = 0;
	virtual bool IsOnFalseGround() const = 0;
	virtual hsScalar GetAirTime() const = 0;
	virtual void ResetAirTime() = 0;

	virtual plPhysical*	GetPushingPhysical() const = 0;
	virtual bool		GetFacingPushingPhysical() const = 0;

	// A helper function to get the coordinate interface for the avatars current
	// world.  Handy if you need to convert points to and from that.  This will
	// return nil if the avatar is in the main world (ie, you don't need to do
	// any translation).
	virtual const plCoordinateInterface* GetSubworldCI() const = 0;

	// For the avatar SDL only
	virtual void GetState(hsPoint3& pos, float& zRot) = 0;
	virtual void SetState(const hsPoint3& pos, float zRot) = 0;

	// kinematic stuff .... should be just for when playing a behavior...
	virtual void Kinematic(bool state) = 0;
	virtual bool IsKinematic() = 0;
	virtual void GetKinematicPosition(hsPoint3& pos) = 0;

	virtual const hsMatrix44& GetPrevSubworldW2L() = 0;

	//when seeking no longer want to interact with exclusion regions
	virtual void SetSeek(bool seek)=0;


};
*/
class plAvatarController
{
public:
	virtual ~plAvatarController() {}
};

class plAnimatedController : public plAvatarController
{
public:
	plAnimatedController(plSceneObject* rootObject, plAGApplicator* rootApp, plPhysicalControllerCore* controller);

	virtual void RecalcVelocity(double timeNow, double timePrev, hsBool useAnim = true);
	void SetTurnStrength(hsScalar val) { fTurnStr = val; }
	hsScalar GetTurnStrength() { return fTurnStr; }
	virtual void ActivateController()=0;
protected:
	plSceneObject* fRootObject;
	plPhysicalControllerCore* fController;
	plAGApplicator* fRootApp;
	hsScalar fAnimAngVel;
	hsVector3 fAnimPosVel;
	hsScalar fTurnStr; // Explicit turning, separate from animations
};

class plWalkingController : public plAnimatedController
{
public:
	plWalkingController(plSceneObject* rootObject, plAGApplicator* rootApp, plPhysicalControllerCore* controller);
	virtual ~plWalkingController();
	virtual void RecalcVelocity(double timeNow, double timePrev, hsBool useAnim = true);

 	void Reset(bool newAge);
 	bool IsControlledFlight() const { return fControlledFlight != 0; }	
	bool IsOnGround() const { return fWalkingStrategy ? fWalkingStrategy->IsOnGround() : true; }
	bool IsOnFalseGround() const { return fWalkingStrategy ? fWalkingStrategy->IsOnFalseGround() : true; }
 	bool HitGroundInThisAge() const { return fHitGroundInThisAge; }
	bool EnableControlledFlight(bool status);
	hsScalar GetAirTime() const { return fWalkingStrategy ? fWalkingStrategy->GetAirTime() : 0.f; }
	void ResetAirTime() { if (fWalkingStrategy) fWalkingStrategy->ResetAirTime(); }
	hsScalar GetForwardVelocity() const;
	void ActivateController();
	// Check these after the avatar the avatar hits the ground for his total
	// hangtime and impact velocity.
	hsScalar GetImpactTime() const { return fImpactTime; }
	const hsVector3& GetImpactVelocity() const { return fImpactVelocity; }

 	plPhysical* GetPushingPhysical() const 
	{
		if(fController)return fController->GetPushingPhysical(); 
		else return nil;
	}
 	bool GetFacingPushingPhysical() const 
	{	if(fController)return fController->GetFacingPushingPhysical(); 
		else return false;
	}

	
protected:
	bool		fHitGroundInThisAge;
	bool		fWaitingForGround;		// We've gone airborne. IsOnGround() returns false until we hit ground again.
	hsScalar	fControlledFlightTime;
	int			fControlledFlight; // Count of how many are currently forcing flight		
	plWalkingStrategy* fWalkingStrategy;
	hsScalar fImpactTime;
	hsVector3 fImpactVelocity;
	bool fClearImpact;
	bool fGroundLastFrame;//used for a test to pass the event of first getting air during a jump
	static const hsScalar kControlledFlightThreshold;
};
class plSwimmingController: public plAnimatedController 
{
public :
	plSwimmingController(plSceneObject* rootObject, plAGApplicator* rootApp, plPhysicalControllerCore* controller);
	virtual ~plSwimmingController();
	void SetSurface(plSwimRegionInterface *region, hsScalar surfaceHeight){
		fSwimmingStrategy->SetSurface(region,surfaceHeight);
	}
	hsScalar GetBuoyancy() { return fSwimmingStrategy->GetBuoyancy(); }
	hsBool IsOnGround() { return fSwimmingStrategy->IsOnGround(); }
	hsBool HadContacts() { return fSwimmingStrategy->HadContacts();}
	void Enable(bool en){if (fController) fController->Enable(en);}
	plPhysicalControllerCore* GetController(){return fController;}
	virtual void ActivateController(){fSwimmingStrategy->RefreshConnectionToControllerCore();}
protected:
	plSwimStrategy* fSwimmingStrategy;

};
class plRidingAnimatedPhysicalController: public plWalkingController
{
public:
	plRidingAnimatedPhysicalController(plSceneObject* rootObject, plAGApplicator* rootApp, plPhysicalControllerCore* controller);
	virtual ~plRidingAnimatedPhysicalController();
};
#endif // PL_HK_CALLBACK_ACTION_H
