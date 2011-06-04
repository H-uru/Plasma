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
#ifndef plCameraBrain_inc
#define plCameraBrain_inc

#include "../pnKeyedObject/hsKeyedObject.h"
#include "hsMatrix44.h"
#include "hsBitVector.h"
#include "hsTemplates.h"

class plMessage;
class plCameraModifier1;
class plSceneObject;
class plRailCameraMod;

class plCameraBrain1 : public hsKeyedObject
{
	
public:
	enum
	{
		kCutPos = 0,
		kCutPosOnce,
		kCutPOA,
		kCutPOAOnce,
		kAnimateFOV,
		kFollowLocalAvatar,
		kPanicVelocity,
		kRailComponent,
		kSubject,
		kCircleTarget,
		kMaintainLOS,
		kZoomEnabled,
		kIsTransitionCamera,
		kWorldspacePOA,
		kWorldspacePos,
		kCutPosWhilePan,
		kCutPOAWhilePan,
		kNonPhys,
		kNeverAnimateFOV,
		kIgnoreSubworldMovement,
		kFalling,
		kRunning,
		kVerticalWhenFalling,
		kSpeedUpWhenRunning,
		kFallingStopped,
		kBeginFalling,
	};
	plCameraBrain1(plCameraModifier1* pMod);
	plCameraBrain1();
	~plCameraBrain1();
	
	CLASSNAME_REGISTER( plCameraBrain1 );
	GETINTERFACE_ANY( plCameraBrain1, hsKeyedObject );

	void SetCamera(plCameraModifier1* pMod) { fCamera = pMod; }
	
	void SetAccel		(hsScalar f) { fAccel = f; }
	void SetDecel		(hsScalar f) { fDecel = f; }
	void SetVelocity	(hsScalar f) { fVelocity = f; }
	void SetPOAAccel	(hsScalar f) { fPOAAccel = f; }
	void SetPOADecel	(hsScalar f) { fPOADecel = f; }
	void SetPOAVelocity	(hsScalar f) { fPOAVelocity = f; }

	const plCameraModifier1* GetCamera() { return fCamera; }

	virtual void		Update(hsBool forced = false);
	virtual hsBool		MsgReceive(plMessage* msg);

	virtual plSceneObject*	GetSubject();
	virtual void SetSubject(plSceneObject* sub);
	
	virtual hsVector3	GetPOAOffset() { return fPOAOffset; }	
	void		SetPOAOffset(hsVector3 pt) { fPOAOffset = pt; }
	void AddTarget();
 
	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	virtual hsBool	GetFaded() { return false; }
	virtual hsBool	SetFaded(hsBool b) { return false; }

	hsBool	HasMovementFlag(int f) { return fMoveFlags.IsBitSet(f); }
	void	SetMovementFlag(int f); 
	void	ClearMovementFlag(int which) { fMoveFlags.ClearBit( which ); }
	void	SetFlags(int i) { fFlags.SetBit(i); }
	void	ClearFlags(int which) { fFlags.ClearBit( which ); }
	hsBool	HasFlag(int f) { return fFlags.IsBitSet(f); }

	void	SetGoal(hsPoint3 pt) { fGoal = pt; }
	void	SetPOAGoal(hsPoint3 pt) { fPOAGoal = pt; }
	void	SetFOVGoal(hsScalar h, double t);
	void	SetZoomParams(hsScalar max, hsScalar min, hsScalar rate);
	
	void	SetXPanLimit(hsScalar x) {fXPanLimit = x;}
	void	SetZPanLimit(hsScalar y) {fZPanLimit = y;}
	hsScalar	GetXPanLimit() {return fXPanLimit;}
	hsScalar	GetZPanLimit() {return fZPanLimit;}

	void	SetRail(plRailCameraMod* m) { fRail = m; }

	hsPoint3 GetGoal() { return fGoal; }
	hsPoint3 GetPOAGoal() { return fPOAGoal; }

	virtual void Push(hsBool recenter = true);
	virtual void Pop();
	
	hsScalar GetVelocity()		{ return fVelocity; }
	hsScalar GetAccel()			{ return fAccel; }
	hsScalar GetDecel()			{ return fDecel; }
	hsScalar GetPOAAccel()		{ return fPOAAccel; }
	hsScalar GetPOAVelocity()	{ return fPOAVelocity; }
	hsScalar GetPOADecel()		{ return fPOADecel; }

	hsScalar GetCurrentCamSpeed() { return fCurCamSpeed; }
	hsScalar GetCurrentViewSpeed() { return fCurViewSpeed; }

	void SetCurrentCamSpeed(hsScalar s) { fCurCamSpeed = s;	}
	void SetCurrentViewSpeed(hsScalar s) { fCurViewSpeed = s;	}
	
	hsMatrix44 GetTargetMatrix() { return fTargetMatrix; }

	static hsScalar fFallVelocity;
	static hsScalar fFallAccel;
	static hsScalar fFallDecel;

	static hsScalar fFallPOAVelocity;
	static hsScalar fFallPOAAccel;
	static hsScalar fFallPOADecel;

protected:
		
	virtual void AdjustForInput(double secs);
	void IMoveTowardGoal(double time);
	void IPointTowardGoal(double time);
	void IAnimateFOV(double time);
	void IAdjustVelocity(hsScalar adjAccelRate, 
						 hsScalar adjDecelRate, 
						 hsVector3* dir, 
						 hsVector3* vel, 
						 hsScalar maxSpeed, 
						 hsScalar distToGoal,
						 double elapsedTime);

	hsScalar IClampVelocity(hsVector3* vel, hsScalar maxSpeed, double elapsedTime);
	hsBool   IShouldDecelerate(hsScalar decelSpeed, hsScalar curSpeed, hsScalar distToGoal);

	plCameraModifier1*	fCamera;
	plKey				fSubjectKey;
	plRailCameraMod*	fRail;
	hsScalar			fCurCamSpeed;
	hsScalar			fCurViewSpeed;
	double				fLastTime;

	hsScalar			fVelocity;
	hsScalar			fAccel;
	hsScalar			fDecel;
	hsScalar			fPOAVelocity;
	hsScalar			fPOAAccel;
	hsScalar			fPOADecel;
	hsVector3			fPOAOffset;
	hsPoint3			fGoal;
	hsPoint3			fPOAGoal;
	hsScalar			fXPanLimit;
	hsScalar			fZPanLimit;
	hsScalar			fPanSpeed;
	hsScalar			fFOVGoal;
	double				fFOVStartTime;
	double				fFOVEndTime;
	hsScalar			fFOVAnimRate; 
	hsScalar			fZoomRate;
	hsScalar			fZoomMax;
	hsScalar			fZoomMin;
	hsBitVector			fMoveFlags;
	hsBitVector			fFlags;
	hsMatrix44			fTargetMatrix;
	hsScalar			fOffsetLength;
	hsScalar			fOffsetPct;
	double				fFallTimer;
};

class plControlEventMsg;

class plCameraBrain1_Drive : public plCameraBrain1
{
protected:
	hsPoint3	fDesiredPosition;
	hsPoint3	fFacingTarget;
	hsBool		bUseDesiredFacing;
	hsScalar	deltaX;
	hsScalar	deltaY;
	hsBool		bDisregardY; // these are here to prevent
	hsBool		bDisregardX; // the camera from jumping when the mouse cursor recenters / wraps around.
	hsVector3	fUp;

public:

	plCameraBrain1_Drive();
	plCameraBrain1_Drive(plCameraModifier1* pMod);
	~plCameraBrain1_Drive();

	static SetSensitivity(hsScalar f) { fTurnRate = f; }
	
	CLASSNAME_REGISTER( plCameraBrain1_Drive );
	GETINTERFACE_ANY( plCameraBrain1_Drive, plCameraBrain1 );

	virtual void		Update(hsBool forced = false);
	virtual hsBool		MsgReceive(plMessage* msg);
	virtual void Push(hsBool recenter = true);
	virtual void Pop();

	static hsScalar	fAcceleration;
	static hsScalar	fDeceleration;
	static hsScalar	fMaxVelocity;
	static hsScalar	fTurnRate;
};


class plCameraBrain1_Avatar : public plCameraBrain1
{
public:
	
	plCameraBrain1_Avatar();
	plCameraBrain1_Avatar(plCameraModifier1* pMod);
	~plCameraBrain1_Avatar();
	
	CLASSNAME_REGISTER( plCameraBrain1_Avatar );
	GETINTERFACE_ANY( plCameraBrain1_Avatar, plCameraBrain1 );


	virtual void		Update(hsBool forced = false);
	virtual hsBool		MsgReceive(plMessage* msg);
	
	virtual void		CalculatePosition();				
	hsVector3	GetOffset() { return fOffset; }	
	void		SetOffset(hsVector3 pt) { fOffset = pt; }

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);
	
	virtual hsBool	GetFaded() { return fFaded; }
	virtual hsBool	SetFaded(hsBool b) { fFaded = b; return true; }

	virtual void Pop();
	virtual void Push(hsBool recenter = true);

protected:
	void ISendFadeMsg(hsBool fade);
	void IHandleObstacle();

	hsPoint3			fHitPoint;
	hsVector3			fOffset;
	hsVector3			fHitNormal;
	hsBool				bObscured;
	hsBool				fFaded;
	plSceneObject*		fObstacle;
};

class plCameraBrain1_FirstPerson : public plCameraBrain1_Avatar
{
public:
	
	plCameraBrain1_FirstPerson();
	plCameraBrain1_FirstPerson(plCameraModifier1* pMod);
	~plCameraBrain1_FirstPerson();
	
	CLASSNAME_REGISTER( plCameraBrain1_FirstPerson );
	GETINTERFACE_ANY( plCameraBrain1_FirstPerson, plCameraBrain1_Avatar );
	
	virtual void CalculatePosition();				
	virtual void Push(hsBool recenter = true);
	virtual void Pop();
	virtual hsBool		MsgReceive(plMessage* msg);
	
	// for console hack
	static hsBool fDontFade;
protected:
	plSceneObject* fPosNode;
	

};


class plCameraBrain1_Fixed : public plCameraBrain1
{
public:
	
	plCameraBrain1_Fixed();
	plCameraBrain1_Fixed(plCameraModifier1* pMod);
	~plCameraBrain1_Fixed();
	
	CLASSNAME_REGISTER( plCameraBrain1_Fixed );
	GETINTERFACE_ANY( plCameraBrain1_Fixed, plCameraBrain1 );

	void SetTargetPoint(plCameraModifier1* pt) { fTargetPoint = pt; }

	virtual void	Update(hsBool forced = false);
	void			CalculatePosition();				
	virtual hsBool MsgReceive(plMessage* msg);
	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

private:
	plCameraModifier1*	fTargetPoint;

};

//
// circle cam brain
//
class plCameraBrain1_Circle : public plCameraBrain1_Fixed
{

public:
	enum CircleFlags
	{
		kLagged			= 0x01,
		kAbsoluteLag	= (0x02 | kLagged),
		kFarthest		= 0x04,
		kTargetted		= 0x08,
		kHasCenterObject = 0x10,
		kPOAObject		= 0x20,
		kCircleLocalAvatar = 0x40,
	};
protected:
	UInt32			fCircleFlags;
	hsPoint3		fCenter;
	plSceneObject*	fCenterObject;	// optional, use instead of fCenter
	hsScalar		fRadius;
	hsScalar		fCurRad, fGoalRad;	// Radians
	plSceneObject*	fPOAObj; // in this case the subject is who we stay close to/away from
	hsScalar		fCirPerSec;

	hsPoint3	IGetClosestPointOnCircle(const hsPoint3* toThisPt);
public:
	plCameraBrain1_Circle(); 
	plCameraBrain1_Circle(plCameraModifier1* pMod); 
	~plCameraBrain1_Circle();

	CLASSNAME_REGISTER( plCameraBrain1_Circle );
	GETINTERFACE_ANY( plCameraBrain1_Circle, plCameraBrain1_Fixed );

	
    virtual void Read(hsStream *stream, hsResMgr* mgr);
    virtual void Write(hsStream *stream, hsResMgr* mgr);
	virtual hsPoint3	MoveTowardsFromGoal(const hsPoint3* fromGoal, double secs, hsBool warp = false);
	virtual void		Update(hsBool forced = false);
	virtual hsBool		MsgReceive(plMessage* msg);
	
	UInt32 GetCircleFlags() { return fCircleFlags; }
	hsPoint3* GetCenter() { return &fCenter; }		// use GetCenterPoint
	hsPoint3  GetCenterPoint();
	hsScalar GetRadius() { return fRadius; }
	plSceneObject* GetCenterObject() { return fCenterObject; }

	void SetCircumferencePerSec(hsScalar h) { fCirPerSec = h; }
	void SetCircleFlags(UInt32 f) { fCircleFlags|=f; }
	void SetCenter(hsPoint3* ctr) { fCenter = *ctr; }		// Circle lies in the plane z = ctr->z
	void SetRadius(hsScalar radius) { 	fRadius = radius; }
	void SetFarCircleCam(hsBool farType) { if (farType) fCircleFlags |= kFarthest; else fCircleFlags &= ~kFarthest; }
	void SetCenterObjectKey(plKey k);
	void SetPOAObject(plSceneObject* pObj) { fPOAObj = pObj; }

};


#endif
