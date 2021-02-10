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
#ifndef plCameraBrain_inc
#define plCameraBrain_inc

#include "hsBitVector.h"
#include "hsGeometry3.h"
#include "hsMatrix44.h"

#include "pnKeyedObject/hsKeyedObject.h"

class plCameraModifier1;
class plMessage;
class plRailCameraMod;
class plSceneObject;

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
    plCameraBrain1();
    plCameraBrain1(plCameraModifier1* pMod);
    ~plCameraBrain1() { }
    
    CLASSNAME_REGISTER( plCameraBrain1 );
    GETINTERFACE_ANY( plCameraBrain1, hsKeyedObject );

    void SetCamera(plCameraModifier1* pMod) { fCamera = pMod; }
    
    void SetAccel       (float f) { fAccel = f; }
    void SetDecel       (float f) { fDecel = f; }
    void SetVelocity    (float f) { fVelocity = f; }
    void SetPOAAccel    (float f) { fPOAAccel = f; }
    void SetPOADecel    (float f) { fPOADecel = f; }
    void SetPOAVelocity (float f) { fPOAVelocity = f; }

    const plCameraModifier1* GetCamera() { return fCamera; }

    virtual void        Update(bool forced = false);
    bool        MsgReceive(plMessage* msg) override;

    virtual plSceneObject*  GetSubject();
    virtual void SetSubject(plSceneObject* sub);
    
    virtual hsVector3   GetPOAOffset() { return fPOAOffset; }   
    void        SetPOAOffset(hsVector3 pt) { fPOAOffset = pt; }
    void AddTarget();
 
    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    virtual bool    GetFaded() { return false; }
    virtual bool    SetFaded(bool b) { return false; }

    bool    HasMovementFlag(int f) { return fMoveFlags.IsBitSet(f); }
    void    SetMovementFlag(int f); 
    void    ClearMovementFlag(int which) { fMoveFlags.ClearBit( which ); }
    void    SetFlags(int i) { fFlags.SetBit(i); }
    void    ClearFlags(int which) { fFlags.ClearBit( which ); }
    bool    HasFlag(int f) { return fFlags.IsBitSet(f); }

    void    SetGoal(hsPoint3 pt) { fGoal = pt; }
    void    SetPOAGoal(hsPoint3 pt) { fPOAGoal = pt; }
    void    SetFOVGoal(float w, float h, double t);
    void    SetZoomParams(float max, float min, float rate);
    
    void    SetXPanLimit(float x) {fXPanLimit = x;}
    void    SetZPanLimit(float y) {fZPanLimit = y;}
    float    GetXPanLimit() {return fXPanLimit;}
    float    GetZPanLimit() {return fZPanLimit;}

    void    SetRail(plRailCameraMod* m) { fRail = m; }

    hsPoint3 GetGoal() { return fGoal; }
    hsPoint3 GetPOAGoal() { return fPOAGoal; }

    virtual void Push(bool recenter = true);
    virtual void Pop();
    
    float GetVelocity()      { return fVelocity; }
    float GetAccel()         { return fAccel; }
    float GetDecel()         { return fDecel; }
    float GetPOAAccel()      { return fPOAAccel; }
    float GetPOAVelocity()   { return fPOAVelocity; }
    float GetPOADecel()      { return fPOADecel; }

    float GetCurrentCamSpeed() { return fCurCamSpeed; }
    float GetCurrentViewSpeed() { return fCurViewSpeed; }

    void SetCurrentCamSpeed(float s) { fCurCamSpeed = s; }
    void SetCurrentViewSpeed(float s) { fCurViewSpeed = s;   }
    
    hsMatrix44 GetTargetMatrix() { return fTargetMatrix; }

    static float fFallVelocity;
    static float fFallAccel;
    static float fFallDecel;

    static float fFallPOAVelocity;
    static float fFallPOAAccel;
    static float fFallPOADecel;

protected:
        
    virtual void AdjustForInput(double secs);
    void IMoveTowardGoal(double time);
    void IPointTowardGoal(double time);
    void IAnimateFOV(double time);
    void IAdjustVelocity(float adjAccelRate,
                         float adjDecelRate,
                         hsVector3* dir,
                         hsVector3* vel,
                         float maxSpeed,
                         float distToGoal,
                         double elapsedTime);

    float IClampVelocity(hsVector3* vel, float maxSpeed, double elapsedTime);
    bool  IShouldDecelerate(float decelSpeed, float curSpeed, float distToGoal);
    float IMakeFOVwZoom(float fovH) const;

    plCameraModifier1*  fCamera;
    plKey               fSubjectKey;
    plRailCameraMod*    fRail;
    float            fCurCamSpeed;
    float            fCurViewSpeed;
    double              fLastTime;

    float            fVelocity;
    float            fAccel;
    float            fDecel;
    float            fPOAVelocity;
    float            fPOAAccel;
    float            fPOADecel;
    hsVector3        fPOAOffset;
    hsPoint3         fGoal;
    hsPoint3         fPOAGoal;
    float            fXPanLimit;
    float            fZPanLimit;
    float            fPanSpeed;
    float            fFOVwGoal, fFOVhGoal;
    double           fFOVStartTime;
    double           fFOVEndTime;
    float            fFOVwAnimRate, fFOVhAnimRate;
    float            fZoomRate;
    float            fZoomMax;
    float            fZoomMin;
    hsBitVector      fMoveFlags;
    hsBitVector      fFlags;
    hsMatrix44       fTargetMatrix;
    float            fOffsetLength;
    float            fOffsetPct;
    double           fFallTimer;
};

class plControlEventMsg;

class plCameraBrain1_Drive : public plCameraBrain1
{
protected:
    hsPoint3    fDesiredPosition;
    hsPoint3    fFacingTarget;
    bool        bUseDesiredFacing;
    float    deltaX;
    float    deltaY;
    bool        bDisregardY; // these are here to prevent
    bool        bDisregardX; // the camera from jumping when the mouse cursor recenters / wraps around.
    hsVector3   fUp;

public:

    plCameraBrain1_Drive();
    plCameraBrain1_Drive(plCameraModifier1* pMod);

    static void SetSensitivity(float f) { fTurnRate = f; }
    
    CLASSNAME_REGISTER( plCameraBrain1_Drive );
    GETINTERFACE_ANY( plCameraBrain1_Drive, plCameraBrain1 );

    void Update(bool forced = false) override;
    bool MsgReceive(plMessage* msg) override;
    void Push(bool recenter = true) override;
    void Pop() override;

    static float fAcceleration;
    static float fDeceleration;
    static float fMaxVelocity;
    static float fTurnRate;
};


class plCameraBrain1_Avatar : public plCameraBrain1
{
public:
    
    plCameraBrain1_Avatar();
    plCameraBrain1_Avatar(plCameraModifier1* pMod);
    ~plCameraBrain1_Avatar();
    
    CLASSNAME_REGISTER( plCameraBrain1_Avatar );
    GETINTERFACE_ANY( plCameraBrain1_Avatar, plCameraBrain1 );


    void        Update(bool forced = false) override;
    bool        MsgReceive(plMessage* msg) override;
    
    virtual void        CalculatePosition();                
    hsVector3   GetOffset() { return fOffset; } 
    void        SetOffset(hsVector3 pt) { fOffset = pt; }

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;
    
    bool    GetFaded() override { return fFaded; }
    bool    SetFaded(bool b) override { fFaded = b; return true; }

    void Pop() override;
    void Push(bool recenter = true) override;

protected:
    void ISendFadeMsg(bool fade);
    void IHandleObstacle();

    hsPoint3            fHitPoint;
    hsVector3           fOffset;
    hsVector3           fHitNormal;
    bool                bObscured;
    bool                fFaded;
    plSceneObject*      fObstacle;
};

class plCameraBrain1_FirstPerson : public plCameraBrain1_Avatar
{
public:

    plCameraBrain1_FirstPerson() : plCameraBrain1_Avatar(), fPosNode() { }
    plCameraBrain1_FirstPerson(plCameraModifier1* pMod) : plCameraBrain1_Avatar(pMod), fPosNode() { }
    
    CLASSNAME_REGISTER( plCameraBrain1_FirstPerson );
    GETINTERFACE_ANY( plCameraBrain1_FirstPerson, plCameraBrain1_Avatar );
    
    void CalculatePosition() override;
    void Push(bool recenter = true) override;
    void Pop() override;
    bool MsgReceive(plMessage* msg) override;
    
    // for console hack
    static bool fDontFade;
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

    void Update(bool forced = false) override;
    void CalculatePosition();
    bool MsgReceive(plMessage* msg) override;
    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

private:
    plCameraModifier1*  fTargetPoint;

};

//
// circle cam brain
//
class plCameraBrain1_Circle : public plCameraBrain1_Fixed
{

public:
    enum CircleFlags
    {
        kLagged         = 0x01,
        kAbsoluteLag    = (0x02 | kLagged),
        kFarthest       = 0x04,
        kTargetted      = 0x08,
        kHasCenterObject = 0x10,
        kPOAObject      = 0x20,
        kCircleLocalAvatar = 0x40,
    };
protected:
    uint32_t          fCircleFlags;
    hsPoint3        fCenter;
    plSceneObject*  fCenterObject;  // optional, use instead of fCenter
    float        fRadius;
    float        fCurRad, fGoalRad;  // Radians
    plSceneObject*  fPOAObj; // in this case the subject is who we stay close to/away from
    float        fCirPerSec;

    hsPoint3    IGetClosestPointOnCircle(const hsPoint3* toThisPt);
public:
    plCameraBrain1_Circle()
        : plCameraBrain1_Fixed(), fCircleFlags(), fCenterObject(),
          fCurRad(1.f), fGoalRad(1.f), fPOAObj(nullptr),
          fCirPerSec(0.25f), fRadius()
    { }
    plCameraBrain1_Circle(plCameraModifier1* pMod)
        : plCameraBrain1_Fixed(pMod), fCircleFlags(), fCenterObject(),
          fCurRad(1.f), fGoalRad(1.f), fPOAObj(nullptr),
          fCirPerSec(0.25f), fRadius()
    { }

    CLASSNAME_REGISTER( plCameraBrain1_Circle );
    GETINTERFACE_ANY( plCameraBrain1_Circle, plCameraBrain1_Fixed );

    
    void Read(hsStream *stream, hsResMgr* mgr) override;
    void Write(hsStream *stream, hsResMgr* mgr) override;
    virtual hsPoint3    MoveTowardsFromGoal(const hsPoint3* fromGoal, double secs, bool warp = false);
    void        Update(bool forced = false) override;
    bool        MsgReceive(plMessage* msg) override;
    
    uint32_t GetCircleFlags() { return fCircleFlags; }
    hsPoint3* GetCenter() { return &fCenter; }      // use GetCenterPoint
    hsPoint3  GetCenterPoint();
    float GetRadius() { return fRadius; }
    plSceneObject* GetCenterObject() { return fCenterObject; }

    void SetCircumferencePerSec(float h) { fCirPerSec = h; }
    void SetCircleFlags(uint32_t f) { fCircleFlags|=f; }
    void SetCenter(hsPoint3* ctr) { fCenter = *ctr; }       // Circle lies in the plane z = ctr->z
    void SetRadius(float radius) {   fRadius = radius; }
    void SetFarCircleCam(bool farType) { if (farType) fCircleFlags |= kFarthest; else fCircleFlags &= ~kFarthest; }
    void SetCenterObjectKey(plKey k);
    void SetPOAObject(plSceneObject* pObj) { fPOAObj = pObj; }

};


#endif
