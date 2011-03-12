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
#ifndef plPXPhysicalController_h_inc
#define plPXPhysicalController_h_inc

#include "../plAvatar/plAvCallbackAction.h"
#include "hsQuat.h"

#define PHYSX_ONLY_TRIGGER_FROM_KINEMATIC 1

class NxController;
class NxCapsuleController;
class NxActor;
class plCoordinateInterface;
class plPhysicalProxy;
class plDrawableSpans;
class hsGMaterial;
class NxCapsule;
#ifndef PLASMA_EXTERNAL_RELEASE
class plDbgCollisionInfo
{
public:
	plSceneObject *fSO;
	hsVector3 fNormal;
	hsBool fOverlap;
};
#endif // PLASMA_EXTERNAL_RELEASE

class plPXPhysicalController : public plPhysicalController
{
public:
	plPXPhysicalController(plKey ownerSO, hsScalar height, hsScalar radius);
	virtual ~plPXPhysicalController();

	virtual void Enable(bool enable);
	virtual bool IsEnabled() const { return fEnable; }

	virtual void SetLOSDB(plSimDefs::plLOSDB losDB) { fLOSDB = losDB; }
	plSimDefs::plLOSDB GetLOSDB() const { return fLOSDB; }

	virtual void SetVelocities(const hsVector3& linearVel, hsScalar angVel)
	{
		fLinearVelocity = linearVel;
		fAngularVelocity = angVel;
	}

	virtual const hsVector3& GetLinearVelocity() const { return fAchievedLinearVelocity; }
	virtual void ResetAchievedLinearVelocity() { fAchievedLinearVelocity.Set(0.f, 0.f, 0.f); }

	virtual plKey GetSubworld() const { return fWorldKey; }
	virtual void SetSubworld(plKey world);

	virtual bool IsOnGround() const { return fTimeInAir < kAirTimeThreshold || fFalseGround; }
	virtual bool IsOnFalseGround() const { return fFalseGround && !fGroundHit; }
	virtual void GroundHit() { fGroundHit = true; }
	virtual hsScalar GetAirTime() const { return fTimeInAir; }
	virtual void ResetAirTime() { fTimeInAir = 0.f; }
	virtual void AddSlidingNormal(hsVector3 vec);
	virtual hsTArray<hsVector3>* GetSlidingNormals() { return &fSlidingNormals; }

	virtual plPhysical*	GetPushingPhysical() const { return fPushingPhysical; }
	virtual bool		GetFacingPushingPhysical() const { return fFacingPushingPhysical; }

	virtual const plCoordinateInterface* GetSubworldCI() const;

	virtual void GetState(hsPoint3& pos, float& zRot);
	virtual void SetState(const hsPoint3& pos, float zRot);

	plKey GetOwner() const { return fOwner; }

	// Called by the simulation mgr each frame
	static void Update(bool prestep, hsScalar delSecs);
	// Used by the LOS mgr to find the controller for an actor it hit
	static plPXPhysicalController* GetController(NxActor& actor, bool* isController);
	// test to see if there are any controllers (i.e. avatars) in this subworld
	static bool plPXPhysicalController::AnyControllersInThisWorld(plKey world);
	static int plPXPhysicalController::NumControllers();
	static int plPXPhysicalController::GetControllersInThisSubWorld(plKey world, int maxToReturn, 
		plPXPhysicalController** bufferout);
	static int plPXPhysicalController::GetNumberOfControllersInThisSubWorld(plKey world);
	// Call this if a static physical in the scene has changed (unloaded,
	// collision enabled/disabled, etc)
	static void RebuildCache();

	virtual void GetPositionSim(hsPoint3& pos) const { IGetPositionSim(pos); }

	virtual void Kinematic(bool state);
	virtual bool IsKinematic();
	virtual void GetKinematicPosition(hsPoint3& pos);

	virtual plDrawableSpans* CreateProxy(hsGMaterial* mat, hsTArray<UInt32>& idx, plDrawableSpans* addTo);

	virtual const hsMatrix44& GetPrevSubworldW2L() { return fPrevSubworldW2L; }

	virtual void SetSeek(bool seek){fSeeking=seek;}
	virtual void GetWorldSpaceCapsule(NxCapsule& cap);
#ifndef PLASMA_EXTERNAL_RELEASE
	static hsBool fDebugDisplay;
#endif // PLASMA_EXTERNAL_RELEASE

protected:
	static const hsScalar kAirTimeThreshold;

	friend class PXControllerHitReport;
	static plPXPhysicalController* FindController(NxController* controller);

	void IApply(hsScalar delSecs);
	void ISendUpdates(hsScalar delSecs);
	void ICheckForFalseGround();
	void ISetGlobalLoc(const hsMatrix44& l2w);
	void IMatchKinematicToController();
	void IMoveKinematicToController(hsPoint3& pos);
	void ISetKinematicLoc(const hsMatrix44& l2w);
	void IGetPositionSim(hsPoint3& pos) const;

	void ICreateController();
	void IDeleteController();

	void IInformDetectors(bool entering);

	plKey fOwner;
	plKey fWorldKey;
	hsScalar fRadius, fHeight;
	NxCapsuleController* fController;

	// this is the kinematic actor for triggering things when the avatar is collision-less during behaviors
	NxActor* fKinematicActor;

	hsVector3 fLinearVelocity;
	hsScalar fAngularVelocity;

	hsVector3 fAchievedLinearVelocity;

	// The global position and rotation of the avatar last time we set it (so we
	// can detect if someone else moves him)
	hsMatrix44 fLastGlobalLoc;
	//
	hsPoint3 fLocalPosition;
	hsQuat fLocalRotation;

	hsMatrix44 fPrevSubworldW2L;

	bool fEnable;
	bool fEnableChanged;
	plSimDefs::plLOSDB fLOSDB;

	bool fKinematic;
	bool fKinematicChanged;
	bool fKinematicEnableNextUpdate;

	bool fGroundHit;
	bool fFalseGround;
	hsScalar fTimeInAir;
	hsTArray<hsVector3> fSlidingNormals;
	hsTArray<hsVector3> fPrevSlidingNormals;

#ifndef PLASMA_EXTERNAL_RELEASE
	hsTArray<plDbgCollisionInfo> fDbgCollisionInfo;
	void IDrawDebugDisplay();
#endif // PLASMA_EXTERNAL_RELEASE

	plPhysical* fPushingPhysical;
	bool fFacingPushingPhysical;

	plPhysicalProxy* fProxyGen;				// visual proxy for debugging

	bool fHitHead;
	
	bool fSeeking;
};

#endif // plPXPhysicalController_h_inc
