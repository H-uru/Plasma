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
#ifndef PLPHYSICALCONTROLLERCORE_H
#define PLPHYSICALCONTROLLERCORE_H
#include "hsGeometry3.h"
#include "hsMatrix44.h"
#include "hsTemplates.h"
#include "pnKeyedObject/plKey.h"
#include "plPhysical/plSimDefs.h"
#include "pnMessage/plMessage.h"

#include "hsQuat.h"
#define PHYSX_ONLY_TRIGGER_FROM_KINEMATIC 1
#define kSLOPELIMIT (cosf(hsDegreesToRadians(45.f)))

class plCoordinateInterface;
class plPhysical;
class plPXPhysical;
class plSwimRegionInterface;
//Replacement for for plPhysicalController stripped out some walk specific code
//plPhysicalControllerCore needs to have movement strategies registered to it these will then
//be called by the controller during the simulation steps. The Strategies need to at least have an
// Apply and Update definition. Everything else should be movement specific. I hope to come back  and 
//and refactor when I have time this in the future. 
enum plControllerCollisionFlags
{   
    kSides=1,
    kTop= (1<<1),
    kBottom=(1<<2),
};

class plMovementStrategySimulationInterface
{
public:

    virtual void Apply(float delSecs)=0;
    virtual void Update(float delSecs)=0;
    //most strategies don't require this. Only the ones that require behavior like a physical or need
    //something after the sim step. this used to be taken care of by Update, but this was moved to take care of
    //some of the frame lag
    virtual void PostStep(float delSecs){};
    virtual void IAddContactNormals(hsVector3& vec){fContactNormals.Append(vec);}
    virtual void AddOnTopOfObject(plPhysical* phys){ fOnTopOf.Append(phys);}
    virtual void LeaveAge()
    {
        fContactNormals.SetCount(0);
        fOnTopOf.SetCount(0);
    }
protected:
    hsTArray<hsVector3> fContactNormals;
    hsTArray<plPhysical* > fOnTopOf;
};

class plControllerSweepRecord
{
public:
    plPhysical *ObjHit;
    hsPoint3 locHit;//World space
    float TimeHit;//Normalized between 0 and 1
    hsVector3 Norm;
};
bool operator<(const plControllerSweepRecord left, const plControllerSweepRecord right);
class plPhysicalControllerCore
{
public:
    virtual ~plPhysicalControllerCore();
    virtual void Move(hsVector3 displacement, unsigned int collideWith, unsigned int &collisionResults)=0;
    virtual void SetMovementSimulationInterface(plMovementStrategySimulationInterface* strat){fMovementInterface=strat;}
    virtual void Apply(float delSecs);
    virtual void Update(float delSecs);
    virtual void PostStep(float delSecs);
    // A disabled avatar doesn't move or accumulate air time if he's off the ground.
    virtual void Enable(bool enable) = 0;
    virtual bool IsEnabled() {return fEnabled;}
    virtual plKey GetSubworld() {return fWorldKey;}
    virtual void SetSubworld(plKey world) = 0;
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
    virtual void SetSeek(bool seek){fSeeking=seek;}
    virtual bool IsSeeking(){return fSeeking;}
    static plPhysicalControllerCore* Create(plKey ownerSO, float height, float radius);
    virtual plMovementStrategySimulationInterface* GetMovementInterface(){return fMovementInterface;}
    plPhysicalControllerCore(plKey ownerSceneObject, float height, float radius);
    virtual plKey GetOwner(){return fOwner;};
    // Set the LOS DB this avatar will be in (only one)
    virtual void SetLOSDB(plSimDefs::plLOSDB losDB) { fLOSDB = losDB; } ;
    virtual plSimDefs::plLOSDB GetLOSDB() {return fLOSDB ; } 
    virtual const hsMatrix44& GetLastGlobalLoc()=0;
    virtual void SetKinematicLoc(const hsMatrix44& l2w)=0;
    virtual void SetGlobalLoc(const hsMatrix44& l2w)=0;
    virtual bool IsEnabledChanged(){return fEnableChanged;}
    virtual void HandleEnableChanged()=0;
    virtual bool IsKinematicChanged(){return fKinematicChanged;}
    virtual void GetPositionSim(hsPoint3& pos)=0;
    virtual void HandleKinematicChanged()=0;
    virtual bool IsKinematicEnableNextUpdate(){return fKinematicEnableNextUpdate;}
    virtual void HandleKinematicEnableNextUpdate()=0;
    virtual void MoveKinematicToController(hsPoint3& pos)=0;
    virtual void UpdateControllerAndPhysicalRep()=0;
    virtual void CheckAndHandleAnyStateChanges();
    virtual void UpdateSubstepNonPhysical();
    virtual const hsPoint3& GetLocalPosition()=0;
    const hsQuat& GetLocalRotation() { return fLocalRotation; }
    virtual void MoveActorToSim();
    
    virtual void OverrideAchievedVelocity(hsVector3 newAchievedVel)
    {//because of things like superjumps this is needed I'd rather not, but can't help it
        fAchievedLinearVelocity=newAchievedVel;
    }
    //any clean up for the controller should go here
    virtual void LeaveAge()=0;
    hsVector3  DisplacementLastStep(){return fDisplacementThisStep;}
    hsVector3  MeanVelocityForLastStep()
    {
        hsVector3 vel=fDisplacementThisStep;
        return vel/fSimLength;
    }
    void SendCorrectionMessages();
    void IncrementAngle(float deltaAngle);
    void UpdateWorldRelativePos();
    virtual void SetLinearVelocity(const hsVector3& linearVel){fLinearVelocity=linearVel;}
    //should actually be a 3 vector but everywhere else it is assumed to be just around  Z 
    virtual void SetAngularVelocity(const float angvel){ fAngularVelocity=angvel;}
    virtual void SetVelocities(const hsVector3& linearVel, float angVel)
    {
        fLinearVelocity=linearVel;
        fAngularVelocity=angVel;
    }
    
    virtual const hsVector3& GetLinearVelocity()  ;
    virtual float GetAngularVelocity(){return fAngularVelocity;}
    virtual const hsVector3& GetAchievedLinearVelocity()const {return fAchievedLinearVelocity;}
    plPhysical* GetPushingPhysical();
    bool GetFacingPushingPhysical();
    virtual void SetPushingPhysical(plPhysical* pl){fPushingPhysical=pl;}
    virtual void SetFacingPushingPhysical(bool ans){fFacingPushingPhysical=ans;}
    //To be Used during runtime conversions, say to switch a tall controller to a ball for swimming
    virtual void SetControllerDimensions(float radius, float height)=0;
    virtual float GetControllerWidth(){return fRadius;}
    virtual float GetControllerHeight(){return fHeight;}
    virtual void ResetAchievedLinearVelocity()
    {
        fAchievedLinearVelocity.Set(0.f,0.f,0.f);
    }
    virtual int SweepControllerPath(const hsPoint3& startPos,const hsPoint3& endPos, hsBool vsDynamics, hsBool vsStatics, uint32_t& vsSimGroups, std::multiset< plControllerSweepRecord >& WhatWasHitOut)=0;
    //this should only be used to force a move it could place your head into a wall and that would be good
    virtual float GetHeight() {return fHeight;}
    virtual float GetRadius() {return fRadius;}
    //Wether the avatar thing has mass and forces things down or not, and changes the way things move
    //This is an attempt fix things like riding on an animated physical
    virtual void BehaveLikeAnimatedPhysical(hsBool actLikeAnAnimatedPhys)=0;
    virtual hsBool BehavingLikeAnAnimatedPhysical()=0;
protected:
    
    plKey fOwner;
    float fHeight;
    float fRadius;
    plKey fWorldKey;
    plSimDefs::plLOSDB fLOSDB;
    bool fSeeking;
    bool fEnabled;
    bool fEnableChanged;
    bool fKinematic;
    bool fKinematicEnableNextUpdate;
    bool fKinematicChanged;
    plMovementStrategySimulationInterface* fMovementInterface;
    hsMatrix44 fLastGlobalLoc;
    hsPoint3 fLocalPosition;
    hsQuat fLocalRotation;
    hsMatrix44 fPrevSubworldW2L;
    hsVector3 fDisplacementThisStep;
    float fSimLength;
    
    //physical properties
    hsVector3 fLinearVelocity;
    float fAngularVelocity;
    hsVector3 fAchievedLinearVelocity;
    plPhysical* fPushingPhysical;
    bool fFacingPushingPhysical;
    bool fNeedsResize;
};

class plMovementStrategy: public plMovementStrategySimulationInterface
{
public:
    virtual void SetControllerCore(plPhysicalControllerCore* core)
    {
        fCore=core;
        fCore->SetMovementSimulationInterface(this);
    }
    virtual void RefreshConnectionToControllerCore()
    {
        fCore->SetMovementSimulationInterface(this);
        //fCore->SetControllerDimensions(fPreferedControllerWidth,fPreferedControllerHeight);
        fCore->BehaveLikeAnimatedPhysical(this->IRequireBehaviourLikeAnAnimatedPhysical());
    }
    plMovementStrategy(plPhysicalControllerCore* core);
    //should actually be a 3 vector but everywhere else it is assumed to be just around  Z 
    virtual void SetLinearAcceleration(const hsVector3& accel){fLinearAcceleration=accel;}
    virtual const hsVector3& GetLinearAcceleration()const{return fLinearAcceleration;}
    //should actually be a 3 vector but everywhere else it is assumed to be just around  Z 
    virtual void ResetAchievedLinearVelocity()
    {
        hsVector3 AchievedLinearVelocity(0.f,0.f,0.f);
        if(fCore)fCore->OverrideAchievedVelocity(AchievedLinearVelocity);
    }
//proxy functions for Controller Core
    virtual float GetAirTime() const { return fTimeInAir; }
    virtual void ResetAirTime() { fTimeInAir = 0.f; }   
    
protected:
    virtual hsBool IRequireBehaviourLikeAnAnimatedPhysical()=0;
    virtual void IApplyKinematic();
    plPhysicalControllerCore* fCore;
    hsVector3 fLinearAcceleration;
    float fAngularAcceleration;
    plKey fOwner;
    static const float kAirTimeThreshold;
    float fTimeInAir;
    float fPreferedControllerWidth;
    float fPreferedControllerHeight;
    

};

class plWalkingStrategy: public plMovementStrategy
{
public:
    plWalkingStrategy(plPhysicalControllerCore* core):plMovementStrategy(core)
    {
        fGroundHit=false;
        fFalseGround=false;
        fHitHead=false;
        fCore->SetMovementSimulationInterface(this);
        fPreferedControllerWidth=core->GetControllerWidth();
        fPreferedControllerHeight=core->GetControllerHeight();
        fOnTopOfAnimatedPhysLastFrame=false;
    }
    virtual ~plWalkingStrategy(){};
    virtual void Apply(float delSecs);
    virtual void Update(float delSecs);      

    bool IsOnGround() const { return fTimeInAir < kAirTimeThreshold || fFalseGround; }
    bool IsOnFalseGround() const { return fFalseGround && !fGroundHit; }
    void GroundHit() { fGroundHit = true; }
    virtual void IAddContactNormals(hsVector3& vec);
    virtual void StartJump(){};
    

protected:
    
    void ICheckForFalseGround();
    bool fGroundHit;
    bool fFalseGround;
    bool fHitHead;
    bool fOnTopOfAnimatedPhysLastFrame;
    hsTArray<hsVector3> fPrevSlidingNormals;
    virtual hsBool IRequireBehaviourLikeAnAnimatedPhysical(){return true;}

};
class plSwimStrategy: public plMovementStrategy
{
public:
    plSwimStrategy(plPhysicalControllerCore *core);
    virtual ~plSwimStrategy(){};
    void SetSurface(plSwimRegionInterface *region, float surfaceHeight);
    virtual void Apply(float delSecs);
    virtual void Update(float delSecs);      
    float GetBuoyancy() { return fBuoyancy; }
    hsBool IsOnGround() { return fOnGround; }
    hsBool HadContacts() { return fHadContacts; }
    virtual void IAddContactNormals(hsVector3& vec);
protected:
    virtual hsBool IRequireBehaviourLikeAnAnimatedPhysical(){return true;}
private:
    void IAdjustBuoyancy();
    float fBuoyancy;
    hsBool fOnGround;
    hsBool fHadContacts;
    float fSurfaceHeight;
    plSwimRegionInterface *fCurrentRegion;
};
class plRidingAnimatedPhysicalStrategy : public plWalkingStrategy
{
public:
    plRidingAnimatedPhysicalStrategy(plPhysicalControllerCore *core ) : 
        fNeedVelocityOverride(false),fStartJump(false),plWalkingStrategy(core){};
    virtual ~plRidingAnimatedPhysicalStrategy(){};
    virtual void Apply(float delSecs);
    virtual void Update(float delSecs);  
    virtual void PostStep(float delSecs);
    bool IsOnGround() const { return fTimeInAir < kAirTimeThreshold || fFalseGround; }
    bool IsOnFalseGround() const { return fFalseGround && !fGroundHit; }
    void GroundHit() { fGroundHit = true; }
    virtual void StartJump(){fStartJump = true;}
protected:
    virtual hsBool IRequireBehaviourLikeAnAnimatedPhysical(){return false;}
    bool ICheckMove(const hsPoint3& startPos, const hsPoint3& desiredPos);
    hsBool fNeedVelocityOverride;
    hsVector3 fOverrideVelocity;
    bool fStartJump;
};
#endif// PLPHYSICALCONTROLLERCORE_H
