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
#include "../plAvatar/plPhysicalControllerCore.h"
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
class plSceneObject;
class PXControllerHitReportWalk;
class plCollideMsg;
#ifndef PLASMA_EXTERNAL_RELEASE

class plDbgCollisionInfo
{
public:
	plSceneObject *fSO;
	hsVector3 fNormal;
	hsBool fOverlap;
};
#endif // PLASMA_EXTERNAL_RELEASE
class plPXPhysicalControllerCore: public plPhysicalControllerCore
{
	friend PXControllerHitReportWalk;
public:
	plPXPhysicalControllerCore(plKey ownerSO, hsScalar height, hsScalar radius);
	~plPXPhysicalControllerCore();
	//should actually be a 3 vector but everywhere else it is assumed to be just around  Z 
		
	inline virtual void Move(hsVector3 displacement, unsigned int collideWith, unsigned int &collisionResults);
	// A disabled avatar doesn't move or accumulate air time if he's off the ground.
	virtual void Enable(bool enable);
	
	virtual void SetSubworld(plKey world) ;
	virtual const plCoordinateInterface* GetSubworldCI() const ;
	// For the avatar SDL only
	virtual void GetState(hsPoint3& pos, float& zRot);
	virtual void SetState(const hsPoint3& pos, float zRot);
	// kinematic stuff .... should be just for when playing a behavior...
	virtual void Kinematic(bool state);
	virtual bool IsKinematic();
	virtual void GetKinematicPosition(hsPoint3& pos);
	virtual const hsMatrix44& GetPrevSubworldW2L(){ return fPrevSubworldW2L; }
	//when seeking no longer want to interact with exclusion regions
	virtual void GetWorldSpaceCapsule(NxCapsule& cap) const;
	static void RebuildCache();
	virtual const hsMatrix44& GetLastGlobalLoc(){return  fLastGlobalLoc;}
	virtual void SetKinematicLoc(const hsMatrix44& l2w){ISetKinematicLoc(l2w);}
	virtual void SetGlobalLoc(const hsMatrix44& l2w){ISetGlobalLoc(l2w);}
	virtual void HandleEnableChanged();
	virtual void HandleKinematicChanged();
	virtual void HandleKinematicEnableNextUpdate();
	virtual void GetPositionSim(hsPoint3& pos){IGetPositionSim(pos);}
	virtual void MoveKinematicToController(hsPoint3& pos);
	virtual const hsPoint3& GetLocalPosition(){return fLocalPosition;}
	virtual void SetControllerDimensions(hsScalar radius, hsScalar height);
	virtual void LeaveAge();
	virtual void UpdateControllerAndPhysicalRep();

//////////////////////////////////////////
//Static Helper Functions
////////////////////////////////////////
	// Used by the LOS mgr to find the controller for an actor it hit
	static plPXPhysicalControllerCore* GetController(NxActor& actor, bool* isController);
	// test to see if there are any controllers (i.e. avatars) in this subworld
	static bool AnyControllersInThisWorld(plKey world);
	static int NumControllers();
	static int GetControllersInThisSubWorld(plKey world, int maxToReturn, 
		plPXPhysicalControllerCore** bufferout);
	static int GetNumberOfControllersInThisSubWorld(plKey world);
	static void UpdatePrestep(hsScalar delSecs);
	static void UpdatePoststep(hsScalar delSecs);
	static void UpdatePostSimStep(hsScalar delSecs);
	virtual plDrawableSpans* CreateProxy(hsGMaterial* mat, hsTArray<UInt32>& idx, plDrawableSpans* addTo);
#ifndef PLASMA_EXTERNAL_RELEASE
	static hsBool fDebugDisplay;
#endif // PLASMA_EXTERNAL_RELEASE
	static void plPXPhysicalControllerCore::SetMaxNumberOfControllers(int max) { fPXControllersMax = max; }
	static int fPXControllersMax;
	virtual int SweepControllerPath(const hsPoint3& startPos, const hsPoint3& endPos, hsBool vsDynamics, hsBool vsStatics, UInt32& vsSimGroups, std::multiset< plControllerSweepRecord >& WhatWasHitOut);
	virtual void BehaveLikeAnimatedPhysical(hsBool actLikeAnAnimatedPhys);
	virtual hsBool BehavingLikeAnAnimatedPhysical();
	virtual const hsVector3& GetLinearVelocity();

	virtual void SetLinearVelocity(const hsVector3& linearVel);
	//should actually be a 3 vector but everywhere else it is assumed to be just around  Z 
	virtual void SetAngularVelocity(const hsScalar angvel);
	virtual void SetVelocities(const hsVector3& linearVel, hsScalar angVel);

protected:
	friend class PXControllerHitReport;
	static plPXPhysicalControllerCore* FindController(NxController* controller);
	void ISetGlobalLoc(const hsMatrix44& l2w);
	void IMatchKinematicToController();
	void IMatchControllerToKinematic();
	void ISetKinematicLoc(const hsMatrix44& l2w);
	void IGetPositionSim(hsPoint3& pos) const;
	void ICreateController();
	void IDeleteController();
	void IInformDetectors(bool entering,bool deferUntilNextSim);
	void plPXPhysicalControllerCore::ICreateController(const hsPoint3& pos);
	NxActor* fKinematicActor;
	NxCapsuleController* fController;
#ifndef PLASMA_EXTERNAL_RELEASE
	hsTArray<plDbgCollisionInfo> fDbgCollisionInfo;
	void IDrawDebugDisplay();
#endif
	void plPXPhysicalControllerCore::IHandleResize();
	hsTArray<plCollideMsg*> fQueuedCollideMsgs;
	hsScalar fPreferedRadius;
	hsScalar fPreferedHeight;
	// The global position and rotation of the avatar last time we set it (so we
	// can detect if someone else moves him)
	plPhysicalProxy* fProxyGen;		
	hsBool fBehavingLikeAnimatedPhys;
};
