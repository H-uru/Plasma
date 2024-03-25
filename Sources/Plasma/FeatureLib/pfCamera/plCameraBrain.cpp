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

#include "plCameraBrain.h"

#include "HeadSpin.h"
#include "plgDispatch.h"
#include "plRefFlags.h"
#include "hsResMgr.h"
#include "hsTimer.h"

#include <cmath>

#include "plCameraModifier.h"
#include "plVirtualCamNeu.h"

#include "pnKeyedObject/plKey.h"
#include "pnMessage/plCameraMsg.h"
#include "pnMessage/plEnableMsg.h"
#include "pnMessage/plPlayerPageMsg.h"
#include "pnMessage/plRefMsg.h"
#include "pnMessage/plTimeMsg.h"
#include "pnNetCommon/plNetApp.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "pnSceneObject/plDrawInterface.h"
#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plSimulationInterface.h"

#include "plAvatar/plArmatureMod.h"
#include "plAvatar/plAvatarMgr.h"
#include "plAvatar/plAvBrainHuman.h"
#include "plInputCore/plInputDevice.h"
#include "plInputCore/plInputManager.h"
#include "plInterp/plController.h"
#include "plMessage/plAvatarMsg.h"
#include "plMessage/plInputEventMsg.h"
#include "plMessage/plLOSRequestMsg.h"
#include "plMessage/plLOSHitMsg.h"
#include "plScene/plSceneNode.h"

#include "pfAnimation/plLineFollowMod.h"

bool plCameraBrain1_FirstPerson::fDontFade = false;
float plCameraBrain1::fFallAccel         = 20.0f;
float plCameraBrain1::fFallDecel         = 5.0f;
float plCameraBrain1::fFallVelocity      = 50.0f;
float plCameraBrain1::fFallPOAAccel      = 10.0f;
float plCameraBrain1::fFallPOADecel      = 10.0f;
float plCameraBrain1::fFallPOAVelocity   = 50.0f;

// basic camera brain now is a fixed brain by default.
// if it doesn't have a subject (an object) it will just look straight ahead.
// if there's a subject it will follow it.

// the avatar and drive cameras are subclasses of the basic brain.

plCameraBrain1::plCameraBrain1()
    : fCurCamSpeed(), fCurViewSpeed(), fVelocity(30.f), fAccel(30.f), fDecel(30.f),
      fPOAVelocity(30.f), fPOAAccel(30.f), fPOADecel(30.f), fSubjectKey(), fRail(),
      fXPanLimit(), fZPanLimit(), fPanSpeed(0.5f), fZoomMin(), fZoomMax(), fZoomRate(),
      fOffsetLength(), fOffsetPct(1.f), fCamera(), fGoal(1.f, 1.f, 1.f),
      fLastTime(), fFOVwGoal(), fFOVhGoal(), fFOVStartTime(), fFOVEndTime(),
      fFOVwAnimRate(), fFOVhAnimRate(), fFallTimer()
{ }

plCameraBrain1::plCameraBrain1(plCameraModifier1* pMod)
    : fCurCamSpeed(), fCurViewSpeed(), fVelocity(30.f), fAccel(30.f), fDecel(30.f),
      fPOAVelocity(30.f), fPOAAccel(30.f), fPOADecel(30.f), fSubjectKey(), fRail(),
      fXPanLimit(), fZPanLimit(), fPanSpeed(0.5f), fZoomMin(), fZoomMax(), fZoomRate(),
      fOffsetLength(), fOffsetPct(1.f), fCamera(pMod), fGoal(1.f, 1.f, 1.f),
      fLastTime(), fFOVwGoal(), fFOVhGoal(), fFOVStartTime(), fFOVEndTime(),
      fFOVwAnimRate(), fFOVhAnimRate(), fFallTimer()
{
    pMod->SetBrain(this);
    hsVector3 up(0.f, 0.f, 1.f);
    fTargetMatrix.Make(&fGoal, &fPOAGoal, &up);
    fFlags.Clear();
}

void plCameraBrain1::AddTarget()
{
    fTargetMatrix = fCamera->GetTarget()->GetCoordinateInterface()->GetLocalToWorld();
    hsVector3 view;
    fTargetMatrix.GetAxis(nullptr, &view, nullptr);
    fGoal = fTargetMatrix.GetTranslate();
    fPOAGoal = fGoal - view;
    fCamera->SetTargetPos(fGoal);
    fCamera->SetTargetPOA(fPOAGoal);
}

// called when we are pushed on top of the camera stack (or re-activated by another popping off directly above)
void plCameraBrain1::Push(bool recenter)
{
    if (fFlags.IsBitSet(kRailComponent))
    {
        fRail->Init();
    }
    if (recenter)
        plInputManager::SetRecenterMouse(false);
    fOffsetPct = 1.0f;
    // force update once to pop into position before we render
    SetFlags(kCutPOAOnce);
    SetFlags(kCutPosOnce);
    Update(true);
    
}

// called when we pop off the camera stack - only if we are the current camera
void plCameraBrain1::Pop()
{
    ClearMovementFlag(S_SET_FREELOOK);
}

// set the goal to which we want to animate the fov
void plCameraBrain1::SetFOVGoal(float w, float h, double t)
{ 
    if (fFOVhGoal == h || h == fCamera->GetFOVh() &&
        fFOVwGoal == w || w == fCamera->GetFOVw())
        return;

    float dif = h - fCamera->GetFOVh();
    fFOVhAnimRate = dif / ((float)t);

    fFOVhGoal = h; 
    fFOVStartTime = hsTimer::GetSysSeconds();
    fFOVEndTime = fFOVStartTime + t; 

    if (w == 0.f)
    {
        fFOVwGoal = IMakeFOVwZoom(h);
        dif = fFOVwGoal - fCamera->GetFOVw();
        fFOVwAnimRate = dif / ((float)t);
    }
    else
    {
        dif = w - fCamera->GetFOVw();
        fFOVwAnimRate = dif / ((float)t);
        fFOVwGoal = w;
    }

    fFlags.SetBit(kAnimateFOV); 
}

// set parameters for how this camera zooms FOV based on user input (mostly for telescopes)
void plCameraBrain1::SetZoomParams(float max, float min, float rate)
{
    fZoomRate = rate;
    fZoomMax = max;
    fZoomMin = min;
    fFlags.SetBit(kZoomEnabled);
}


// periodic update - forced means we are forcing an update at a non-normal time to "prime" the camera 
// into position before it renders the first time (hence the fake 10 second frame delta)
void plCameraBrain1::Update(bool forced)
{
    double secs = hsTimer::GetDelSysSeconds();
    if (forced)
        secs = 10.0f;
    // is there a subject we are following?
    if (GetSubject())
    {
        fTargetMatrix = fCamera->GetTarget()->GetCoordinateInterface()->GetLocalToWorld();
        fGoal = fTargetMatrix.GetTranslate();
        fPOAGoal = GetSubject()->GetCoordinateInterface()->GetLocalToWorld().GetTranslate();
        fPOAGoal += fPOAOffset;
    }
    else
    {   
        // get view based on current orientation (we could be animated)
        if (fCamera->GetTarget())
        {
            fTargetMatrix = fCamera->GetTarget()->GetCoordinateInterface()->GetLocalToWorld();
            hsVector3 view;
            fTargetMatrix.GetAxis(nullptr, &view, nullptr);
            fGoal = fTargetMatrix.GetTranslate();
            fPOAGoal = fGoal - (view * 10);
        }
    }
    AdjustForInput(secs);

    IMoveTowardGoal(secs);
    IPointTowardGoal(secs);
    if (fFlags.IsBitSet(kAnimateFOV))
        IAnimateFOV(secs);
        
}

// adjust FOV based on elapsed time
void plCameraBrain1::IAnimateFOV(double time)
{
    float dH = fFOVhAnimRate * hsTimer::GetDelSysSeconds();
    float dW = fFOVwAnimRate * hsTimer::GetDelSysSeconds();
    dH += fCamera->GetFOVh();
    dW += fCamera->GetFOVw();

    if ( (fFOVhAnimRate < 0.0f && dH <= fFOVhGoal) ||
         (fFOVhAnimRate > 0.0f && dH >= fFOVhGoal)  )
    {
        dH = fFOVhGoal;
    }
    if ( (fFOVwAnimRate < 0.0f && dW <= fFOVwGoal) ||
         (fFOVwAnimRate > 0.0f && dW >= fFOVwGoal)  )
    {
        dW = fFOVwGoal;
    }

    if (dW == fFOVwGoal && dH == fFOVhGoal)
        fFlags.ClearBit(kAnimateFOV);
    fCamera->SetFOV( dW, dH );
}

// move the camera's origin point (not where it is looking) toward where it is going
void plCameraBrain1::IMoveTowardGoal(double elapsedTime)
{
    bool current = plVirtualCam1::Instance()->IsCurrentCamera(GetCamera());

    if (fFlags.IsBitSet(kCutPos) || fFlags.IsBitSet(kNonPhys) || !current)
    {
        fCamera->SetTargetPos(fGoal);
        return;
    }

    if (fFlags.IsBitSet(kCutPosOnce))
    {
        fCamera->SetTargetPos(fGoal);
        fFlags.ClearBit(kCutPosOnce);
        return;
    }
    hsVector3 dir(fGoal - fCamera->GetTargetPos());
    float distToGoal=dir.Magnitude();
    
    //smooth out stoppage...
    float adjMaxVel = fVelocity;
    if (distToGoal <= 5.0f && distToGoal > 0.1f)
    {
        float mult = (distToGoal - 5.0f)*0.1f;
        adjMaxVel = fVelocity - fabs(fVelocity*mult);
    }


    if (distToGoal > 0.0f)
        dir.Normalize();

    hsVector3 vel( dir * fCurCamSpeed );
    
    if (fFlags.IsBitSet(kFalling))
        IAdjustVelocity(plCameraBrain1::fFallAccel, plCameraBrain1::fFallDecel, &dir, &vel, plCameraBrain1::fFallVelocity, distToGoal, elapsedTime);
    else
    if (plVirtualCam1::Instance()->fUseAccelOverride)
        IAdjustVelocity(plVirtualCam1::Instance()->fAccel, plVirtualCam1::Instance()->fDecel, &dir, &vel, plVirtualCam1::Instance()->fVel, distToGoal, elapsedTime);
    else
    if (fFlags.IsBitSet(kPanicVelocity))
        IAdjustVelocity(fAccel, fDecel, &dir, &vel, 1000.0f, distToGoal, elapsedTime);
    else
    if (fFlags.IsBitSet(kRunning) && fVelocity < 16.0f) // speed up when we run if necessary
        IAdjustVelocity(fAccel, fDecel, &dir, &vel, 16.0f, distToGoal, elapsedTime);
    else
        IAdjustVelocity(fAccel, fDecel, &dir, &vel, adjMaxVel, distToGoal, elapsedTime);
        
    fCurCamSpeed = vel.Magnitude();

    float distMoved;
    if (fFlags.IsBitSet(kPanicVelocity))
        distMoved = IClampVelocity(&vel, 1000.0f, elapsedTime);
    else
    if (fFlags.IsBitSet(kFalling))
        distMoved = IClampVelocity(&vel, plCameraBrain1::fFallVelocity, elapsedTime);
    else
    if (fFlags.IsBitSet(kRunning) && fVelocity < 16.0f)
        distMoved = IClampVelocity(&vel, 16.0f, elapsedTime);
    else
        distMoved = IClampVelocity(&vel, fVelocity, elapsedTime);

    // compute final pos
    if (distMoved > distToGoal)
        fCamera->SetTargetPos(fGoal);
    else
        fCamera->SetTargetPos(fCamera->GetTargetPos() + vel);

}

void plCameraBrain1::SetMovementFlag(int f)
{ 
    fMoveFlags.SetBit(f); 
}


void plCameraBrain1::IPointTowardGoal(double elapsedTime)
{
    bool current = plVirtualCam1::Instance()->IsCurrentCamera(GetCamera());
    
    if (fFlags.IsBitSet(kCutPOA) || fFlags.IsBitSet(kNonPhys) || !current)
    {
        fCamera->SetTargetPOA(fPOAGoal);
        return;
    }
    if (fFlags.IsBitSet(kCutPOAOnce))
    {
        fCamera->SetTargetPOA(fPOAGoal);
        fFlags.ClearBit(kCutPOAOnce);
        return;
    }


    hsVector3 dir(fPOAGoal - fCamera->GetTargetPOA());
    float distToGoal=dir.Magnitude();
    
    if (distToGoal > 0.0f)
        dir.Normalize();

    // smooth out stoppage
    float adjMaxVel = fPOAVelocity;
    if (distToGoal <= 5.0f && distToGoal > 0.1f)
    {   
        float mult = (distToGoal - 5.0f)*0.1f;
        adjMaxVel = fPOAVelocity - fabs(fPOAVelocity*mult);
    }   
    

    hsVector3 vel( dir * fCurViewSpeed );
    
    if (fFlags.IsBitSet(kFalling))
        IAdjustVelocity(plCameraBrain1::fFallPOAAccel, plCameraBrain1::fFallPOADecel, &dir, &vel, plCameraBrain1::fFallPOAVelocity, distToGoal, elapsedTime);
    else
    if (plVirtualCam1::Instance()->fUseAccelOverride)
        IAdjustVelocity(plVirtualCam1::Instance()->fAccel, plVirtualCam1::Instance()->fDecel, &dir, &vel, plVirtualCam1::Instance()->fVel, distToGoal, elapsedTime);
    else
    if (fFlags.IsBitSet(kPanicVelocity))
        IAdjustVelocity(fPOAAccel, fPOADecel, &dir, &vel, 1000.0f, distToGoal, elapsedTime);
    else
    if (fFlags.IsBitSet(kRunning) && fPOAVelocity < 16.0f)
        IAdjustVelocity(fPOAAccel, fPOADecel, &dir, &vel,  16.0f, distToGoal, elapsedTime);
    else
        IAdjustVelocity(fPOAAccel, fPOADecel, &dir, &vel,  adjMaxVel, distToGoal, elapsedTime);
    
    fCurViewSpeed = vel.Magnitude();
    
    float distMoved;
    if (fFlags.IsBitSet(kPanicVelocity))
        distMoved = IClampVelocity(&vel, 1000.0f, elapsedTime);
    else
    if (fFlags.IsBitSet(kFalling))
        distMoved = IClampVelocity(&vel, plCameraBrain1::fFallPOAVelocity, elapsedTime);
    else
    if (fFlags.IsBitSet(kRunning) && fPOAVelocity < 16.0f)
        distMoved = IClampVelocity(&vel, 16.0f, elapsedTime);
    else
        distMoved = IClampVelocity(&vel, fPOAVelocity, elapsedTime);

    // compute final pos
    if (distMoved > distToGoal)
        fCamera->SetTargetPOA(fPOAGoal);
    else
        fCamera->SetTargetPOA(fCamera->GetTargetPOA() + vel);
}


void plCameraBrain1::IAdjustVelocity(float adjAccelRate, float adjDecelRate, 
                                       hsVector3* dir, hsVector3* vel, float maxSpeed, 
                                       float distToGoal, double elapsedTime)
{
    float speed = vel->Magnitude();      // save current speed
    *vel = *dir * speed;                    // change vel to correct dir

    // compute accel/decel
    float finalAccelRate;

    if (IShouldDecelerate(adjDecelRate, speed, distToGoal))
    {
        finalAccelRate = -adjDecelRate;
    }
    else
    {
        finalAccelRate = adjAccelRate;
    }

    if (finalAccelRate  != 0)
    {
        // compute accel vector in the direction of the goal
        hsVector3 accelVec = *dir * finalAccelRate;
        accelVec = accelVec * (float)elapsedTime;

        // add acceleration to velocity
        *vel = *vel + accelVec;
    }
    else
    {
        *vel = *dir * maxSpeed;
    }
}

float plCameraBrain1::IClampVelocity(hsVector3* vel, float maxSpeed, double elapsedTime)
{
    *vel = *vel * (float)elapsedTime;
    maxSpeed *= (float)elapsedTime;

    // clamp speed  (clamp if going negative?)
    float distMoved = vel->Magnitude();
    if (distMoved > maxSpeed)
    {
        vel->Normalize();
        *vel = *vel * maxSpeed;
        return maxSpeed;
    }
    return distMoved;
}

bool plCameraBrain1::IShouldDecelerate(float decelSpeed, float curSpeed, float distToGoal)
{
    if (decelSpeed == 0)
        // no deceleration
        return false;

    // compute distance required to stop, given decel speed (in units/sec sq)
    float stopTime = curSpeed / decelSpeed;      
    float avgSpeed = curSpeed * .5f;
    float stopDist = avgSpeed * stopTime;

    return (fabs(distToGoal) <= fabs(stopDist));  // stopDist+avgSpeed?
}

//
// Make adjustments to camera position based on 
// user input - NOTE this is for mouse-cursor based adjustment
//
void plCameraBrain1::AdjustForInput(double secs)
{
    // move closer to camera target based on user input
    if (fOffsetPct < 1.0f)
    {
        hsVector3 v(fPOAGoal - fGoal);
        float len = v.Magnitude();
        len = len - (len * fOffsetPct);
        v.Normalize();
        fGoal = fGoal + (v * len);
    }
}

void plCameraBrain1::Read(hsStream* stream, hsResMgr* mgr)
{
    hsKeyedObject::Read(stream, mgr);
    fPOAOffset.Read(stream);
    plGenRefMsg* msg = new plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kSubject ); // SceneObject
    mgr->ReadKeyNotifyMe( stream, msg, plRefFlags::kActiveRef );
    
    plGenRefMsg* msg2 = new plGenRefMsg( GetKey(), plRefMsg::kOnRequest, 0, kRailComponent ); // SceneObject
    mgr->ReadKeyNotifyMe( stream, msg2, plRefFlags::kActiveRef );

    fFlags.Read(stream);

    if (fFlags.IsBitSet(kFollowLocalAvatar))
    {
        if (plNetClientApp::GetInstance() && plNetClientApp::GetInstance()->GetLocalPlayer())
            SetSubject((plSceneObject*)plNetClientApp::GetInstance()->GetLocalPlayer());
            
        plgDispatch::Dispatch()->RegisterForExactType(plPlayerPageMsg::Index(), GetKey());
    }
    fAccel = stream->ReadLEFloat();
    fDecel = stream->ReadLEFloat();
    fVelocity = stream->ReadLEFloat();
    fPOAAccel = stream->ReadLEFloat();
    fPOADecel = stream->ReadLEFloat();
    fPOAVelocity = stream->ReadLEFloat();
    fXPanLimit = stream->ReadLEFloat();
    fZPanLimit = stream->ReadLEFloat();
    fZoomRate = stream->ReadLEFloat();
    fZoomMin = stream->ReadLEFloat();
    fZoomMax = stream->ReadLEFloat();

}

void plCameraBrain1::Write(hsStream* stream, hsResMgr* mgr)
{
    hsKeyedObject::Write(stream, mgr);
    fPOAOffset.Write(stream);
    mgr->WriteKey(stream, GetSubject());
    mgr->WriteKey(stream, fRail);
    fFlags.Write(stream);
    stream->WriteLEFloat(fAccel);
    stream->WriteLEFloat(fDecel);
    stream->WriteLEFloat(fVelocity);
    stream->WriteLEFloat(fPOAAccel);
    stream->WriteLEFloat(fPOADecel);
    stream->WriteLEFloat(fPOAVelocity);
    stream->WriteLEFloat(fXPanLimit);
    stream->WriteLEFloat(fZPanLimit);
    stream->WriteLEFloat(fZoomRate);
    stream->WriteLEFloat(fZoomMin);
    stream->WriteLEFloat(fZoomMax);
}

float plCameraBrain1::IMakeFOVwZoom(float fovH) const
{
    float num = tan(hsDegreesToRadians(fovH / 2)) * tan(hsDegreesToRadians(fCamera->GetFOVw() / 2));
    float denom = tan(hsDegreesToRadians(fCamera->GetFOVh() / 2));
    return 2 * fabs(hsRadiansToDegrees(atan(num / denom)));
}

bool plCameraBrain1::MsgReceive(plMessage* msg)
{
    plCameraMsg* pCamMsg = plCameraMsg::ConvertNoRef(msg);
    if (pCamMsg)
    {
        if (pCamMsg->Cmd(plCameraMsg::kStartZoomIn))
        {
            fFlags.SetBit(kAnimateFOV);
            fFOVhGoal = fZoomMin;
            fFOVwGoal = IMakeFOVwZoom(fZoomMin);
            fFOVwAnimRate = fFOVhAnimRate = -1*fZoomRate;
            return true;
        }
        else
        if (pCamMsg->Cmd(plCameraMsg::kStartZoomOut))
        {
            fFlags.SetBit(kAnimateFOV);
            fFOVhGoal = fZoomMax;
            fFOVwGoal = IMakeFOVwZoom(fZoomMax);
            fFOVwAnimRate = fFOVhAnimRate = fZoomRate;
            return true;
        }
        else
        if (pCamMsg->Cmd(plCameraMsg::kStopZoom))
        {
            fFlags.ClearBit(kAnimateFOV);
            return true;
        }
        else
        if (pCamMsg->Cmd(plCameraMsg::kNonPhysOn))
        {
            fFlags.SetBit(kNonPhys);
            return true;
        }
        else
        if (pCamMsg->Cmd(plCameraMsg::kNonPhysOff))
        {
            fFlags.ClearBit(kNonPhys);
            return true;
        }
    }
    plGenRefMsg* pRefMsg = plGenRefMsg::ConvertNoRef(msg);
    if (pRefMsg)
    {
        if (pRefMsg->fType == kSubject)
        {
            if( pRefMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
                SetSubject((plSceneObject*)pRefMsg->GetRef());
            else
                SetSubject(nullptr);
            return true;
        }
        else
        if (pRefMsg->fType == kRailComponent)
        {
            if( pRefMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
            {
                fFlags.SetBit(kRailComponent);
                fRail = (plRailCameraMod*)pRefMsg->GetRef();
            }
            else
            {
                fRail = nullptr;
                fFlags.ClearBit(kRailComponent);
            }
            return true;
        }
    }
    plPlayerPageMsg* pPMsg = plPlayerPageMsg::ConvertNoRef(msg);
    if (pPMsg)
    {
        if (pPMsg->fPlayer == plNetClientApp::GetInstance()->GetLocalPlayerKey())
        {
            if (pPMsg->fUnload)
            {
                if (fFlags.IsBitSet(kFollowLocalAvatar))
                    SetSubject(nullptr);
            }
            else
            {
                plGenRefMsg* msg = new plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kSubject ); // SceneObject
                hsgResMgr::ResMgr()->AddViaNotify(pPMsg->fPlayer, msg, plRefFlags::kPassiveRef);

                fFlags.SetBit(kCutPosOnce);
                fFlags.SetBit(kCutPOAOnce);
            }
        }
        return true;
    }
    plControlEventMsg* pCMsg = plControlEventMsg::ConvertNoRef(msg);
    if (pCMsg)
    {
        if (pCMsg->ControlActivated())
        {
            SetMovementFlag(pCMsg->GetControlCode());
            if (pCMsg->GetControlCode() == S_SET_FREELOOK)
            {
                plInputManager::SetRecenterMouse(true);
                plMouseDevice::HideCursor();
            }
            if (pCMsg->GetControlCode() == B_CAMERA_ZOOM_IN && fFlags.IsBitSet(kZoomEnabled))
            {
                fFlags.SetBit(kAnimateFOV);
                fFOVhGoal = fZoomMin;
                fFOVwGoal = IMakeFOVwZoom(fZoomMin);
                fFOVwAnimRate = fFOVhAnimRate = -1*fZoomRate;
                fFOVEndTime = hsTimer::GetSysSeconds() + 60;
                return true;
            }
            else
            if (pCMsg->GetControlCode() == B_CAMERA_ZOOM_OUT && fFlags.IsBitSet(kZoomEnabled))
            {
                fFlags.SetBit(kAnimateFOV);
                fFOVhGoal = fZoomMax;
                fFOVwGoal = IMakeFOVwZoom(fZoomMax);
                fFOVwAnimRate = fFOVhAnimRate = fZoomRate;
                fFOVEndTime = hsTimer::GetSysSeconds() + 60;
                return true;
            }
            else
            if (pCMsg->GetControlCode() == B_CAMERA_RECENTER)
            {
                if (fFlags.IsBitSet(kZoomEnabled))
                {
                    fFlags.SetBit(kAnimateFOV);
                    fFOVhGoal = fZoomMin + ((fZoomMax - fZoomMin) / 2);
                    fFOVwGoal = IMakeFOVwZoom(fFOVhGoal);
                    if (fCamera->GetFOVh() >= fFOVhGoal)
                        fFOVwAnimRate = fFOVhAnimRate = -1*fZoomRate;
                    else
                        fFOVwAnimRate = fFOVhAnimRate = fZoomRate;
                    fFOVEndTime = hsTimer::GetSysSeconds() + 60;
                }
                return true;
            }
        }
        else
        {
            ClearMovementFlag(pCMsg->GetControlCode());
            if (pCMsg->GetControlCode() == S_SET_FREELOOK)
            {
                plInputManager::SetRecenterMouse(false);
                plMouseDevice::ShowCursor();
            }
            else
            if ( (pCMsg->GetControlCode() == B_CAMERA_ZOOM_IN || pCMsg->GetControlCode() == B_CAMERA_ZOOM_OUT)  
                && fFlags.IsBitSet(kZoomEnabled) )
            {
                fFlags.ClearBit(kAnimateFOV);
                return true;
            }

        }   
        return true;
    }
    
    return hsKeyedObject::MsgReceive(msg);
}

plSceneObject* plCameraBrain1::GetSubject()
{
    if (fSubjectKey)
        return plSceneObject::ConvertNoRef(fSubjectKey->ObjectIsLoaded());
    return nullptr;
}

void plCameraBrain1::SetSubject(plSceneObject* sub)
{
    if (sub)
        fSubjectKey = sub->GetKey();
    else
        fSubjectKey = nullptr;
}

//
//
// new drive mode brain
//


float plCameraBrain1_Drive::fTurnRate = 100.0f;
float plCameraBrain1_Drive::fAcceleration = 200.0f;
float plCameraBrain1_Drive::fDeceleration = 200.0f;
float plCameraBrain1_Drive::fMaxVelocity = 100.0f;


// constructor
plCameraBrain1_Drive::plCameraBrain1_Drive()
    : plCameraBrain1(), fUp(0.f, 0.f, 1.f), bUseDesiredFacing(),
      deltaX(), deltaY(), bDisregardX(), bDisregardY()
{
    fGoal.Set(100,100,100);
    fPOAGoal.Set(0,0,0);
    fCamera->SetTargetPos(fGoal);
    fCamera->SetTargetPOA(fPOAGoal);
    fLastTime = 0.f;
}

plCameraBrain1_Drive::plCameraBrain1_Drive(plCameraModifier1* pMod)
    : plCameraBrain1(pMod), fUp(0.f, 0.f, 1.f), bUseDesiredFacing(),
      deltaX(), deltaY(), bDisregardX(), bDisregardY()
{   
    fGoal.Set(100,100,100);
    fPOAGoal.Set(0,0,0);
    fCamera->SetTargetPos(fGoal);
    fCamera->SetTargetPOA(fPOAGoal);
    fLastTime = 0.f;
}

void plCameraBrain1_Drive::Push(bool recenter)
{
    plCameraBrain1::Push(recenter);
    plInputManager::SetRecenterMouse(true);
    fLastTime = hsTimer::GetSeconds();
}
void plCameraBrain1_Drive::Pop()
{   
    plInputManager::SetRecenterMouse(false);
}

// 
// Update Method
//
void plCameraBrain1_Drive::Update(bool forced)
{
    hsVector3 neg_up = -1 * fUp; 
    fTargetMatrix.Make(&fGoal, &fPOAGoal, &neg_up);
    
    // update our desired position:
    double time = hsTimer::GetSeconds();
    float eTime = (float)(time-fLastTime);
    if(eTime > 0.01f)
        eTime = 0.01f;
    fLastTime = time;
    hsPoint3 cameraPos = fCamera->GetTargetPos();
    hsVector3 view, up, right;
        
    fTargetMatrix.GetAxis(&view,&up,&right);
    float delta = 5.0f * eTime;

    // adjust speed
    if (HasMovementFlag(B_CAMERA_DRIVE_SPEED_UP))
    {
        plCameraBrain1_Drive::fAcceleration += delta;
        plCameraBrain1_Drive::fDeceleration += delta;
        plCameraBrain1_Drive::fMaxVelocity += delta;
    }
    if (HasMovementFlag(B_CAMERA_DRIVE_SPEED_DOWN))
    {
        plCameraBrain1_Drive::fAcceleration -= delta;
        plCameraBrain1_Drive::fDeceleration -= delta;
        plCameraBrain1_Drive::fMaxVelocity -= delta;

        if (plCameraBrain1_Drive::fAcceleration <= 0.0f)
            plCameraBrain1_Drive::fAcceleration = 0.0f;
        
        if (plCameraBrain1_Drive::fTurnRate <= 0.0f)
            plCameraBrain1_Drive::fTurnRate = 0.0f;
        
        if (plCameraBrain1_Drive::fDeceleration <= 0.0f)
            plCameraBrain1_Drive::fDeceleration = 0.0f;

        if (plCameraBrain1_Drive::fMaxVelocity <= 0.0f)
            plCameraBrain1_Drive::fMaxVelocity = 0.0f;

    }
    if (plVirtualCam1::Instance()->fUseAccelOverride)
    {
        fMaxVelocity = plVirtualCam1::Instance()->fVel;
    }

    float speed = fMaxVelocity;
    float turn = 1.0f;
    
    if (HasMovementFlag(B_CONTROL_MODIFIER_FAST))
    {
        turn *= 0.25;
        speed *= 10;
    }
    if (HasMovementFlag(B_CAMERA_MOVE_FORWARD))
    {
        cameraPos += view * speed * eTime;
    }
    if (HasMovementFlag(B_CAMERA_MOVE_BACKWARD))
    {
        cameraPos += view * speed * eTime * -1;
    }
    if (HasMovementFlag(B_CAMERA_MOVE_LEFT))
    {
        cameraPos += right * speed * eTime * -1;
    }
    if (HasMovementFlag(B_CAMERA_MOVE_RIGHT))
    {
        cameraPos += right * speed * eTime;
    }
    if (HasMovementFlag(B_CAMERA_MOVE_DOWN))
    {
        cameraPos += up * speed * eTime * -1;
    }
    if (HasMovementFlag(B_CAMERA_MOVE_UP))
    {
        cameraPos += up * speed * eTime;
    }
    fGoal = cameraPos;

    IMoveTowardGoal(eTime);
    hsMatrix44 rot;

    // make sure camera is perpindicular to absolute up
    fTargetMatrix.fMap[0][2] = 0.0f;
    fTargetMatrix.fMap[1][2] = 0.0f;

    fTargetMatrix.GetAxis(&view,&up,&right);

    if ( HasMovementFlag( B_CAMERA_ROTATE_RIGHT ) ||  HasMovementFlag( B_CAMERA_ROTATE_LEFT ) )
    {
        hsQuat q(turn * fTurnRate * eTime * deltaX, &up);   
        q.NormalizeIfNeeded();
        q.MakeMatrix(&rot);
        ClearMovementFlag( B_CAMERA_ROTATE_RIGHT );
        ClearMovementFlag( B_CAMERA_ROTATE_LEFT );
        fTargetMatrix = rot * fTargetMatrix;
    }
    rot.Reset();

    if ( HasMovementFlag( B_CAMERA_ROTATE_UP ) || HasMovementFlag(B_CAMERA_ROTATE_DOWN) )
    {
        hsQuat q(turn * fTurnRate* eTime * deltaY, &right); 
        q.NormalizeIfNeeded();
        q.MakeMatrix(&rot);
        ClearMovementFlag( B_CAMERA_ROTATE_UP );
        ClearMovementFlag( B_CAMERA_ROTATE_DOWN );
        fTargetMatrix = rot * fTargetMatrix;
    }

    fTargetMatrix.GetAxis(&view,&up,&right);
    hsPoint3 at(fGoal + (view * 2.0f));

    // Now passing in Up for up parameter, instead of down. mf_flip_up - mf
    fTargetMatrix.MakeCamera(&fGoal, &at, &up);
    
    fPOAGoal = at;
    fUp = up;

    fCamera->SetTargetPos(fGoal);
    fCamera->SetTargetPOA(fPOAGoal);
}

bool plCameraBrain1_Drive::MsgReceive(plMessage* msg)
{
    plMouseEventMsg* pMouseMsg = plMouseEventMsg::ConvertNoRef(msg);
    if( pMouseMsg )
    {
        if (pMouseMsg->GetDX() > 0.4 || pMouseMsg->GetDX() < -0.4)
        {
            bDisregardY = true;
            return true;
        }
        else
        if (pMouseMsg->GetDY() > 0.4 || pMouseMsg->GetDY() < -0.4)
        {
            bDisregardX = true;
            return true;
        }

        if (bDisregardY && pMouseMsg->GetDY() != 0.0)
        {
            bDisregardY = false;
            return true;
        }   
        
        if (bDisregardX && pMouseMsg->GetDX() != 0.0)
        {
            bDisregardX = false;
            return true;
        }
        if (pMouseMsg->GetDX() < 0)
        {
            SetMovementFlag( B_CAMERA_ROTATE_RIGHT );
            deltaX = pMouseMsg->GetDX();
        }
        else
        if (pMouseMsg->GetDX() > 0)
        {
            SetMovementFlag( B_CAMERA_ROTATE_LEFT );
            deltaX = pMouseMsg->GetDX();
        }
        else
        if (pMouseMsg->GetDY() > 0)
        {
            SetMovementFlag( B_CAMERA_ROTATE_DOWN );
            deltaY = pMouseMsg->GetDY();
        }
        else
        if (pMouseMsg->GetDY() < 0)
        {
            deltaY = pMouseMsg->GetDY();
            SetMovementFlag( B_CAMERA_ROTATE_UP );
        }
        
        return true;
    }
    
    return plCameraBrain1::MsgReceive(msg);
}



//
//
//
// new simplified avatar camera

// constructor
plCameraBrain1_Avatar::plCameraBrain1_Avatar()
    : plCameraBrain1(), bObscured(), fFaded(), fObstacle()
{
}

plCameraBrain1_Avatar::plCameraBrain1_Avatar(plCameraModifier1* pMod)
    : plCameraBrain1(pMod), bObscured(), fFaded(), fObstacle()
{
}

// destructor
plCameraBrain1_Avatar::~plCameraBrain1_Avatar()
{
    if (!plNetClientApp::GetInstance())
        return;
        
    if (fFaded)
    {
        plCameraTargetFadeMsg* pMsg = new plCameraTargetFadeMsg;
        pMsg->SetFadeOut(false);
        pMsg->SetSubjectKey(plNetClientApp::GetInstance()->GetLocalPlayerKey());
        pMsg->SetBCastFlag(plMessage::kBCastByExactType);
        pMsg->SetBCastFlag(plMessage::kNetPropagate, false);
        pMsg->AddReceiver(plNetClientApp::GetInstance()->GetLocalPlayerKey());
        plgDispatch::MsgSend(pMsg);
    }

}

void plCameraBrain1_Avatar::Pop()
{
    bObscured = false;
    if (fFaded)
        ISendFadeMsg(false);
    plCameraBrain1::Pop();
}

void plCameraBrain1_Avatar::Push(bool recenter)
{
    bObscured = false;
    fFallTimer = 0.0f;
    plCameraBrain1::Push(recenter);
}


// 
// Update Method
//
void plCameraBrain1_Avatar::Update(bool forced)
{
    if (!GetSubject())
        return;
    if (HasFlag(kBeginFalling) && hsTimer::GetSysSeconds() >= fFallTimer)
    {
        fFallTimer = 0.0f;
        
        fFlags.SetBit(kFalling);
        fFlags.ClearBit(kBeginFalling);
        plVirtualCam1::Instance()->SetFlags(plVirtualCam1::kFalling);
        plVirtualCam1::Instance()->StartUnPan();
        fOffsetPct = 1.0f;
        
    }
    // determine elapsed time;
    double secs = hsTimer::GetDelSysSeconds();
    if (forced)
        secs = 10.0f;

    if (fFlags.IsBitSet(kIsTransitionCamera))
    {
        if (GetKey())
            hsStatusMessageF("%s thinks it's the transition camera\n", GetKeyName().c_str());
    }
    else
    {
        CalculatePosition();
        AdjustForInput(secs);
    }
    
    IMoveTowardGoal(secs);
    IPointTowardGoal(secs);
    
    if (fFlags.IsBitSet(kAnimateFOV))
        IAnimateFOV(secs);
}

//
// Calculate the best position for the camera in basic follow mode:
//
void plCameraBrain1_Avatar::CalculatePosition()
{
    if (!GetSubject() || !GetSubject()->GetCoordinateInterface())
        return;
        
    // get target pos
    hsPoint3 targetFrom = GetSubject()->GetCoordinateInterface()->GetLocalToWorld().GetTranslate();
    
    // get view normal
    hsVector3 view = GetSubject()->GetCoordinateInterface()->GetLocalToWorld().GetAxis(hsMatrix44::kView);
    hsVector3 right = GetSubject()->GetCoordinateInterface()->GetLocalToWorld().GetAxis(hsMatrix44::kRight);
    
    // compute camera position and interest
    if (HasFlag(kWorldspacePos))
    {
        fGoal = targetFrom + GetOffset();
    }
    else
    {
        fGoal = targetFrom;
        if (!fFlags.IsBitSet(kFalling))
        {
            fGoal = fGoal - view * GetOffset().fY;
            fGoal = fGoal - right * GetOffset().fX;
            fGoal.fZ += GetOffset().fZ;
        }
        else
        {
            fGoal = fGoal - view * 0.5;
            fGoal.fZ += 20;
        }
    }

    if (HasFlag(kWorldspacePOA))
    {
        fPOAGoal = targetFrom + GetPOAOffset();
    }
    else
    {
        fPOAGoal = targetFrom;
        //if (!fFlags.IsBitSet(kFalling))
        {
            fPOAGoal = fPOAGoal - view * GetPOAOffset().fY;
            fPOAGoal = fPOAGoal - right * GetPOAOffset().fX;
            fPOAGoal.fZ += GetPOAOffset().fZ;
        }
    }

    // check to see if we've lagged too far behind
    hsVector3 dist(fCamera->GetTargetPos() - fGoal);
    if (dist.MagnitudeSquared() > (4*fOffset.MagnitudeSquared()))
        SetFlags(kPanicVelocity);
    else
        ClearFlags(kPanicVelocity);

    // check LOS
    if (GetCamera()->GetKey() && fFlags.IsBitSet(kMaintainLOS) && (plVirtualCam1::Instance()->GetCurrentStackCamera() == GetCamera()))
    {
        plLOSRequestMsg* pMsg = new plLOSRequestMsg( GetCamera()->GetKey(), fPOAGoal, fGoal, plSimDefs::kLOSDBCameraBlockers,
            plLOSRequestMsg::kTestClosest, plLOSRequestMsg::kReportHitOrMiss);
        pMsg->SetRequestName(ST::format("Camera Brain [{}]: Find Blockers", GetKeyName()));
        plgDispatch::MsgSend( pMsg );
    }
    
    if (bObscured)
    {
        fGoal = fHitPoint + (fHitNormal * 0.5f);
        
        hsVector3 newOff(fGoal - fPOAGoal);
        hsVector3 actualOff(fCamera->GetTargetPos() - fPOAGoal);
        if (newOff.MagnitudeSquared() <= 16.0f && actualOff.MagnitudeSquared() <= 16.0f && !fFaded)
        {
            ISendFadeMsg(true);
            fFaded = true;
        }
        else
        if (newOff.MagnitudeSquared() >= 16.0f && fFaded)
        {
            ISendFadeMsg(false);
            fFaded = false;
        }
    }
    else
    if (fFaded && !bObscured)
    {
        ISendFadeMsg(false);
        fFaded = false;
    }
}   


void plCameraBrain1_Avatar::IHandleObstacle()
{
    // swing around the 'obstacle'
}

void plCameraBrain1_Avatar::ISendFadeMsg(bool fade)
{
    if (plVirtualCam1::IsCurrentCamera(GetCamera()))
    {
        if (fade)
            plVirtualCam1::AddMsgToLog("current camera sending Fade Out message to Avatar");
        else
            plVirtualCam1::AddMsgToLog("current camera sending Fade In message to Avatar");
    
        plCameraTargetFadeMsg* pMsg = new plCameraTargetFadeMsg;
        pMsg->SetFadeOut(fade);
        pMsg->SetSubjectKey(GetSubject()->GetKey());
        pMsg->SetBCastFlag(plMessage::kBCastByExactType);
        pMsg->SetBCastFlag(plMessage::kNetPropagate, false);
        pMsg->AddReceiver(GetSubject()->GetKey());
        plgDispatch::MsgSend(pMsg);
    }
}

bool plCameraBrain1_Avatar::MsgReceive(plMessage* msg)
{
    plLOSHitMsg *pLOSMsg = plLOSHitMsg::ConvertNoRef( msg );
    if( pLOSMsg )
    {
        bObscured = !pLOSMsg->fNoHit;
        fHitPoint = pLOSMsg->fHitPoint;
        fHitNormal = pLOSMsg->fNormal;
        if (pLOSMsg->fHitFlags & plSimDefs::kLOS_CameraAvoidObject)
            fObstacle = (plSceneObject*)pLOSMsg->fObj->GetObjectPtr();
        else
            fObstacle = nullptr;
        return true;
    }
    plMouseEventMsg* pMouseMsg = plMouseEventMsg::ConvertNoRef(msg);
    if( pMouseMsg )
    {
        if (pMouseMsg->GetButton() == kWheelPos || pMouseMsg->GetButton() == kWheelNeg)
        {
            if (fFlags.IsBitSet(kFalling))
                return true;
        
            fOffsetPct += -1 * ( (pMouseMsg->GetWheelDelta() / 24) * 0.01f);
            if (fOffsetPct > 1.0f)
                fOffsetPct = 1.0f;
            else
            if (fOffsetPct < 0.1f)
                fOffsetPct = 0.1f;
            return true;
        }
    }
    plGenRefMsg* pRefMsg = plGenRefMsg::ConvertNoRef(msg);
    if (pRefMsg)
    {
        if (pRefMsg->fType == kSubject)
        {
            if( pRefMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
            {
                SetSubject((plSceneObject*)pRefMsg->GetRef());
                plSceneObject* avSO = nullptr;
                if (plNetClientApp::GetInstance())
                    avSO = plSceneObject::ConvertNoRef(plNetClientApp::GetInstance()->GetLocalPlayer());

                if (GetSubject() == avSO)
                {
                    plArmatureMod* avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
                    if (avMod)
                        avMod->RegisterForBehaviorNotify(GetKey());
                }
            }
            else
                SetSubject(nullptr);
            return true;
        }
    }
    plAvatarBehaviorNotifyMsg *behNotifymsg = plAvatarBehaviorNotifyMsg::ConvertNoRef(msg);
    if (behNotifymsg)
    {
        if (behNotifymsg->fType == plHBehavior::kBehaviorTypeFall && fFlags.IsBitSet(kVerticalWhenFalling))
        {
            if (behNotifymsg->state)
            {   
                SetFlags(kBeginFalling);
                fFallTimer = hsTimer::GetSysSeconds() + plVirtualCam1::fFallTimerDelay;
            }
            else
            if (!behNotifymsg->state)
            {
                if (fFlags.IsBitSet(kFalling))
                {   plVirtualCam1::Instance()->ClearFlags(plVirtualCam1::kFalling);
                    fFlags.ClearBit(kFalling);
                }
                else
                {
                    ClearFlags(kBeginFalling);
                    fFallTimer = 0.0f;
                }
            }
        }
        else
        if (behNotifymsg->fType == plHBehavior::kBehaviorTypeFall)
        {
            if (behNotifymsg->state && fFlags.IsBitSet(kSpeedUpWhenRunning))
                fFlags.SetBit(kRunning);
            else
                fFlags.ClearBit(kRunning);
        }
        return true;
    }
    return plCameraBrain1::MsgReceive(msg);
}

void plCameraBrain1_Avatar::Read(hsStream* stream, hsResMgr* mgr)
{
    plCameraBrain1::Read(stream, mgr);
    fOffset.Read(stream);
    SetFlags(kCutPOA);
    
    if (fFlags.IsBitSet(kFollowLocalAvatar) && GetSubject())
    {
        plSceneObject* avSO = plSceneObject::ConvertNoRef(plNetClientApp::GetInstance()->GetLocalPlayer());
        if (GetSubject() == avSO)
        {
            plArmatureMod* avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
            if (avMod)
                avMod->RegisterForBehaviorNotify(GetKey());
        }
    }

}

void plCameraBrain1_Avatar::Write(hsStream* stream, hsResMgr* mgr)
{
    plCameraBrain1::Write(stream, mgr);
    fOffset.Write(stream);
}


//
// first person cam, derived from avatar cam -
// only difference is push() & pop() fade avatar in/out
//

bool plCameraBrain1_FirstPerson::MsgReceive(plMessage* msg)
{

    plGenRefMsg* pRefMsg = plGenRefMsg::ConvertNoRef(msg);
    if (pRefMsg)
    {
        if (pRefMsg->fType == kSubject)
        {
            if( pRefMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
            {   
                fPosNode = nullptr;
                SetSubject((plSceneObject*)pRefMsg->GetRef());
                // are we the built-in 1st person camera?  If we are, change our subject pointer to FPCameraOrigin node on the avatar
                plUoid U(kDefaultCameraMod1_KEY);
                plKey pKey = hsgResMgr::ResMgr()->FindKey(U);
                if (pKey && pKey == fCamera->GetKey())
                {
                    const plCoordinateInterface* ci = GetSubject()->GetCoordinateInterface();
                    for (size_t i = 0; i < ci->GetNumChildren(); i++)
                    {
                        plSceneObject* child = (plSceneObject*)ci->GetChild(i)->GetOwner();
                        if (child)
                        {
                            const ST::string& name = child->GetKeyName();
                            if (name.compare("FPCameraOrigin", ST::case_insensitive) == 0)
                            {
                                fPosNode = child;
                                SetOffset({});
                                SetPOAOffset(hsVector3(0.f, -10.f, 0.f));
                                return true;
                            }
                        }
                    }
                }
            }
            else
            {
                fPosNode = nullptr;
                SetSubject(nullptr);
            }
            return true;
        }
    }
    plMouseEventMsg* pMouseMsg = plMouseEventMsg::ConvertNoRef(msg);
    if( pMouseMsg )
    {
        if (pMouseMsg->GetButton() == kWheelPos || pMouseMsg->GetButton() == kWheelNeg)
        {
            return true;
        }
    }
    return plCameraBrain1_Avatar::MsgReceive(msg);
}

void plCameraBrain1_FirstPerson::CalculatePosition()
{
    // get target pos
    const plSceneObject* target = fPosNode ? fPosNode : GetSubject();
    hsPoint3 targetFrom = target->GetCoordinateInterface()->GetLocalToWorld().GetTranslate();

    // get view normal
    hsVector3 view = GetSubject()->GetCoordinateInterface()->GetLocalToWorld().GetAxis(hsMatrix44::kView);
    hsVector3 right = GetSubject()->GetCoordinateInterface()->GetLocalToWorld().GetAxis(hsMatrix44::kRight);
    
    // compute camera position and interest
    if (HasFlag(kWorldspacePos))
    {
        fGoal = targetFrom + GetOffset();
    }
    else
    {
        fGoal = targetFrom;
        fGoal = fGoal - view * GetOffset().fY;
        fGoal = fGoal - right * GetOffset().fX;
        fGoal.fZ += GetOffset().fZ;
    }

    if (HasFlag(kWorldspacePOA))
    {
        fPOAGoal = targetFrom + GetPOAOffset();
    }
    else
    {
        fPOAGoal = targetFrom;
        fPOAGoal = fPOAGoal - view * GetPOAOffset().fY;
        fPOAGoal = fPOAGoal - right * GetPOAOffset().fX;
        fPOAGoal.fZ += GetPOAOffset().fZ;
    }

    // check to see if we've lagged too far behind
    hsVector3 dist(fCamera->GetTargetPos() - fGoal);
    if (dist.MagnitudeSquared() > (4*fOffset.MagnitudeSquared()))
        SetFlags(kPanicVelocity);
    else
        ClearFlags(kPanicVelocity);

    // check LOS
    if (GetCamera()->GetKey() && fFlags.IsBitSet(kMaintainLOS) && (plVirtualCam1::Instance()->GetCurrentStackCamera() == GetCamera()))
    {
        plLOSRequestMsg* pMsg = new plLOSRequestMsg( GetCamera()->GetKey(), fPOAGoal, fGoal, plSimDefs::kLOSDBCameraBlockers,
            plLOSRequestMsg::kTestClosest, plLOSRequestMsg::kReportHitOrMiss);
        pMsg->SetRequestName(ST::format("Camera Brain [{}]: Find Blockers", GetKeyName()));
        plgDispatch::MsgSend( pMsg );
    }
    
    if (bObscured)
    {
        fGoal = fHitPoint + (fHitNormal * 0.5f);
        
        hsVector3 newOff(fGoal - fPOAGoal);
        hsVector3 actualOff(fCamera->GetTargetPos() - fPOAGoal);
        if (newOff.MagnitudeSquared() <= 16.0f && actualOff.MagnitudeSquared() <= 16.0f && !fFaded)
        {
            ISendFadeMsg(true);
            fFaded = true;
        }
        else
        if (newOff.MagnitudeSquared() >= 16.0f && fFaded)
        {
            ISendFadeMsg(false);
            fFaded = false;
        }
    }
    else
    if (fFaded && !bObscured)
    {
        ISendFadeMsg(false);
        fFaded = false;
    }
}   


void plCameraBrain1_FirstPerson::Push(bool recenter)
{
    if (!GetSubject())
        return; 
    
    if (plCameraBrain1_FirstPerson::fDontFade)
        return;
    plEnableMsg* pMsg = new plEnableMsg;
    pMsg->SetCmd(plEnableMsg::kDisable);
    pMsg->AddType(plEnableMsg::kDrawable);
    pMsg->SetBCastFlag(plMessage::kPropagateToModifiers);
    pMsg->AddReceiver(GetSubject()->GetKey());
    pMsg->SetSender(GetCamera()->GetKey());
    plgDispatch::MsgSend(pMsg);
    plCameraBrain1::Push(recenter);
    
    ISendFadeMsg(true);
}

void plCameraBrain1_FirstPerson::Pop()
{

    plCameraBrain1::Pop();
    if (!GetSubject())
        return;
    
    ISendFadeMsg(false);
}

//
// true fixed cam, derived from basic cam -
// Has a separate interest point object it maintains
//

plCameraBrain1_Fixed::plCameraBrain1_Fixed() : plCameraBrain1()
{
    fTargetPoint = nullptr;
}

plCameraBrain1_Fixed::plCameraBrain1_Fixed(plCameraModifier1* pMod) : plCameraBrain1(pMod)
{
    fTargetPoint = nullptr;
}   

// destructor
plCameraBrain1_Fixed::~plCameraBrain1_Fixed()
{
}

void plCameraBrain1_Fixed::Read(hsStream* stream, hsResMgr* mgr)
{
    plCameraBrain1::Read(stream, mgr);
    mgr->ReadKeyNotifyMe( stream, new plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, 99), plRefFlags::kPassiveRef);
}

void plCameraBrain1_Fixed::Write(hsStream* stream, hsResMgr* mgr)
{
    plCameraBrain1::Write(stream, mgr);
    mgr->WriteKey(stream, fTargetPoint);
}

void plCameraBrain1_Fixed::Update(bool forced)
{
    double secs = hsTimer::GetDelSysSeconds();
    if (forced)
        secs = 10.0f;
    
    if (!fFlags.IsBitSet(kIsTransitionCamera))
    {
        if(fTargetPoint)
            fTargetPoint->GetBrain()->Update();

        if (GetSubject())
        {
            fTargetMatrix = fCamera->GetTarget()->GetCoordinateInterface()->GetLocalToWorld();
            fGoal = fTargetMatrix.GetTranslate();
            
            // get target pos
            hsPoint3 targetFrom = GetSubject()->GetCoordinateInterface()->GetLocalToWorld().GetTranslate();
            
            // get view normal
            hsVector3 view = GetSubject()->GetCoordinateInterface()->GetLocalToWorld().GetAxis(hsMatrix44::kView);
            hsVector3 right = GetSubject()->GetCoordinateInterface()->GetLocalToWorld().GetAxis(hsMatrix44::kRight);
            
            // compute camera position and interest
            if (HasFlag(kWorldspacePOA))
            {
                fPOAGoal = targetFrom + GetPOAOffset();
            }
            else
            {
                fPOAGoal = targetFrom;
                fPOAGoal = fPOAGoal - view * GetPOAOffset().fY;
                fPOAGoal = fPOAGoal - right * GetPOAOffset().fX;
                fPOAGoal.fZ += GetPOAOffset().fZ;
            }

        }
        else
        {   
            if (fCamera->GetTarget())
            {
                fTargetMatrix = fCamera->GetTarget()->GetCoordinateInterface()->GetLocalToWorld();
                hsVector3 up;
                fTargetMatrix.GetAxis(nullptr, &up, nullptr);
                fGoal = fTargetMatrix.GetTranslate();
                if (fTargetPoint)
                    fPOAGoal = fTargetPoint->GetBrain()->GetGoal();
                else
                    fPOAGoal = fGoal - (up * 10);
            }
        }
        if (fFlags.IsBitSet(kRailComponent) && fRail)
        {       
            if (fCurCamSpeed == 0)
                fCurCamSpeed = 1.0f;
            if (forced)
                fGoal = fRail->GetGoal(10, 10);
            else
                fGoal = fRail->GetGoal(secs, fCurCamSpeed);
        }
        AdjustForInput(secs);
    }
    IMoveTowardGoal(secs);
    IPointTowardGoal(secs);
    if (fFlags.IsBitSet(kAnimateFOV))
        IAnimateFOV(secs);
}

void plCameraBrain1_Fixed::CalculatePosition()
{
}

bool plCameraBrain1_Fixed::MsgReceive(plMessage* msg)
{
    plGenRefMsg* pRefMsg = plGenRefMsg::ConvertNoRef(msg);
    if (pRefMsg)
    {
        if (pRefMsg->GetContext() == plRefMsg::kOnCreate &&
            pRefMsg->fType == 99)
        {
            fTargetPoint = (plCameraModifier1*)(pRefMsg->GetRef());
        }
    }
    return (plCameraBrain1::MsgReceive(msg));
}



//
//
//
// circle camera crap

//
//
//
void plCameraBrain1_Circle::Update(bool forced)
{
    double secs = hsTimer::GetDelSysSeconds();
    if (forced)
        secs = 10.0f;

    if (!GetSubject() || !GetSubject()->GetCoordinateInterface())
        return;
    hsPoint3 sub = GetSubject()->GetCoordinateInterface()->GetLocalToWorld().GetTranslate();
    fGoal = IGetClosestPointOnCircle(&sub);
    
    if (fPOAObj && fPOAObj->GetCoordinateInterface())
        fPOAGoal = fPOAObj->GetCoordinateInterface()->GetLocalToWorld().GetTranslate();
    else
        fPOAGoal = GetCenterPoint();
    
    fPOAGoal += fPOAOffset;
    
    hsPoint3 goalpos = fCamera->GetTargetPos();
    if (HasFlag(kCutPosOnce))
    {
        fGoal = MoveTowardsFromGoal(&goalpos, secs, true);
        fFlags.ClearBit(kCutPos);
    }
    else
    {   
        fGoal = MoveTowardsFromGoal(&goalpos, secs);
        fFlags.SetBit(kCutPos);
    }
    AdjustForInput(secs);
    
    IMoveTowardGoal(secs);
    IPointTowardGoal(secs);
    if (fFlags.IsBitSet(kAnimateFOV))
        IAnimateFOV(secs);
}

//
// keep us on the circle
//
hsPoint3 plCameraBrain1_Circle::MoveTowardsFromGoal(const hsPoint3* fromGoal, double secs, bool warp)
{

    if (fCurRad != fGoalRad)
    {
        float dist = fabs(fGoalRad-fCurRad);
    
        hsAssert(dist >= 0.f && dist <= hsConstants::two_pi<float>, "illegal radian diff");
        bool mustWrap = (dist > hsConstants::pi<float>);  // go opposite direction for shortcut and wrap

        // compute speed
        float speed; 
        if (warp)
            speed = (float)(hsConstants::two_pi<float> * 100 * secs);
        else
            speed = (float)(hsConstants::two_pi<float> * fCirPerSec * secs);

        // move towards goalRad
        hsAssert(fCurRad >= 0.f && fCurRad <= hsConstants::two_pi<float>, "illegal radian value");

        if (fCurRad<fGoalRad)
        {
            if (mustWrap)
            {
                fCurRad-=speed;
                bool didWrap=false;
                while(fCurRad<0)
                {
                    didWrap=true;
                    fCurRad += hsConstants::two_pi<float>;
                }
                if (fCurRad<fGoalRad && didWrap)
                    fCurRad=fGoalRad;
            }
            else
            {
                fCurRad+=speed;
                if (fCurRad>fGoalRad)
                    fCurRad=fGoalRad;
            }
        }
        else
        {
            if (mustWrap)
            {
                fCurRad+=speed;
                bool didWrap=false;
                while (fCurRad > hsConstants::two_pi<float>)
                {
                    didWrap=true;
                    fCurRad -= hsConstants::two_pi<float>;
                }
                if (fCurRad>fGoalRad && didWrap)
                    fCurRad=fGoalRad;
            }
            else
            {
                fCurRad-=speed;
                if (fCurRad<fGoalRad)
                    fCurRad=fGoalRad;
            }
        }
    }
    hsAssert(fCurRad >= 0.f && fCurRad <= hsConstants::two_pi<float>, "illegal radian value");
    
    hsPoint3 x = GetCenterPoint() + hsVector3((float)cos(fCurRad)*fRadius, (float)sin(fCurRad)*fRadius, 0.0f);
    x.fZ = fCamera->GetTargetPos().fZ;
    return x;
}

hsPoint3 plCameraBrain1_Circle::IGetClosestPointOnCircle(const hsPoint3* toThis)
{
    hsPoint3 center=GetCenterPoint();
    hsPoint3 p(toThis->fX, toThis->fY, center.fZ);  // Move to plane of circle
    hsVector3 v;
    if (!(GetCircleFlags() & kFarthest) )
    {
        v = hsVector3(&p, &center);
    }
    else
    {
        // farthest
        v = hsVector3(&center, &p);
    }
    v.Normalize();
    fGoalRad = (float)atan2(v.fY, v.fX); // -pi to pi
    hsAssert(fGoalRad >= -hsConstants::pi<float> && fGoalRad <= hsConstants::pi<float>,
             "Illegal atan2 val");
    if (fGoalRad<0)
        fGoalRad = hsConstants::two_pi<float> + fGoalRad;   // 0 to 2pi
    hsAssert(fGoalRad >= 0.f && fGoalRad <= hsConstants::two_pi<float>, "Illegal atan2 val");
    v = v * fRadius;
    center = center + v;
    center.fZ = fCamera->GetTargetPos().fZ;
    return (center);
}


hsPoint3 plCameraBrain1_Circle::GetCenterPoint()
{
    if (fCenterObject && fCenterObject->GetCoordinateInterface())
    {
        return fCenterObject->GetCoordinateInterface()->GetLocalToWorld().GetTranslate();
    }
    return fCenter;
}

void plCameraBrain1_Circle::Write(hsStream* stream, hsResMgr* mgr)
{
    plCameraBrain1::Write(stream, mgr);
    stream->WriteLE32(GetCircleFlags());
    fCenter.Write(stream);
    stream->WriteLEFloat(GetRadius());
    mgr->WriteKey(stream, fCenterObject);
    mgr->WriteKey(stream, fPOAObj);
    stream->WriteLEFloat(fCirPerSec);
}

void plCameraBrain1_Circle::Read(hsStream* stream, hsResMgr* mgr)
{
    plCameraBrain1::Read(stream, mgr);
    SetCircleFlags(stream->ReadLE32());
    if (GetCircleFlags() & kCircleLocalAvatar)
    {
        if (plNetClientApp::GetInstance() && plNetClientApp::GetInstance()->GetLocalPlayer())
            SetPOAObject((plSceneObject*)plNetClientApp::GetInstance()->GetLocalPlayer());
            
        plgDispatch::Dispatch()->RegisterForExactType(plPlayerPageMsg::Index(), GetKey());
    }

    fCenter.Read(stream);
    SetRadius(stream->ReadLEFloat());
    plGenRefMsg* msg = new plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kCircleTarget ); // SceneObject
    mgr->ReadKeyNotifyMe( stream, msg, plRefFlags::kActiveRef );
    plGenRefMsg* msg2 = new plGenRefMsg( GetKey(), plRefMsg::kOnRequest, 0, kPOAObject ); // SceneObject
    mgr->ReadKeyNotifyMe( stream, msg2, plRefFlags::kActiveRef );
    fCirPerSec = stream->ReadLEFloat();
    plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
}

bool plCameraBrain1_Circle::MsgReceive(plMessage* msg)
{
    plGenRefMsg* pRefMsg = plGenRefMsg::ConvertNoRef(msg);
    if (pRefMsg)
    {
        if (pRefMsg->fType == kCircleTarget)
        {
            if( pRefMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
                fCenterObject = (plSceneObject*)pRefMsg->GetRef();
            else
                fCenterObject = nullptr;
            return true;
        }
        else
        if (pRefMsg->fType == kPOAObject)
        {
            if( pRefMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
                fPOAObj = (plSceneObject*)pRefMsg->GetRef();
            else
                fPOAObj = nullptr;
            return true;
        }
        else
        if (pRefMsg->fType == kSubject)
        {
            if( pRefMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
                SetSubject((plSceneObject*)pRefMsg->GetRef());
            else
                SetSubject(nullptr);
            return true;
        }
    }
    plPlayerPageMsg* pPMsg = plPlayerPageMsg::ConvertNoRef(msg);
    if (pPMsg && pPMsg->fPlayer == plNetClientApp::GetInstance()->GetLocalPlayerKey())
    {
        if (pPMsg->fUnload)
        {
            if (GetCircleFlags() & kCircleLocalAvatar)
                fPOAObj = nullptr;
            if (fFlags.IsBitSet(kFollowLocalAvatar))
                SetSubject(nullptr);
        }
        else
        {
            if (GetCircleFlags() & kCircleLocalAvatar)
            {   
                SetPOAObject((plSceneObject*)pPMsg->fPlayer->GetObjectPtr());
                fFlags.SetBit(kCutPOA);
            }
            if (fFlags.IsBitSet(kFollowLocalAvatar))
            {
                plGenRefMsg* msg = new plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kSubject ); // SceneObject
                hsgResMgr::ResMgr()->AddViaNotify(pPMsg->fPlayer, msg, plRefFlags::kPassiveRef);

                fFlags.SetBit(kCutPosOnce);
                fFlags.SetBit(kCutPOA);
            }
        }
        return true;
    }
    return (plCameraBrain1_Fixed::MsgReceive(msg));
}


