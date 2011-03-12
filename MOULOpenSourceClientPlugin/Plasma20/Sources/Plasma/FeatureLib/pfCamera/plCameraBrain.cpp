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
#include "HeadSpin.h"

#include "plCameraBrain.h"
#include "hsTimer.h"
#include "hsResMgr.h"
#include "plRefFlags.h"
#include "plCameraModifier.h"
#include "plVirtualCamNeu.h"
#include "plgDispatch.h"
#include "../plInterp/plController.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../pnSceneObject/plSimulationInterface.h"
#include "../pnSceneObject/plDrawInterface.h"
#include "../plScene/plSceneNode.h"
#include "../pnMessage/plCameraMsg.h"
#include "../pnMessage/plPlayerPageMsg.h"
#include "../pnMessage/plEnableMsg.h"
#include "../pnMessage/plTimeMsg.h"
#include "../plMessage/plInputEventMsg.h"
#include "../plMessage/plLOSRequestMsg.h"
#include "../plMessage/plLOSHitMsg.h"
#include "../plMessage/plAvatarMsg.h"
#include "../pnKeyedObject/plKey.h"
#include "../plInputCore/plInputDevice.h"
#include "../plInputCore/plInputManager.h"
#include "../pnNetCommon/plNetApp.h"
#include "../pfAnimation/plLineFollowMod.h"
#include "../plAvatar/plAvatarMgr.h"
#include "../plAvatar/plArmatureMod.h"
#include "../plAvatar/plAvBrainHuman.h"
#include "../plNetClient/plNetClientMgr.h"
//#define aspect_HDTV // maybe someday we'll be on the xbox...

#ifdef aspect_HDTV
#define FOV_RATIO 1.78
#else
#define FOV_RATIO 1.33333333
#endif

hsBool plCameraBrain1_FirstPerson::fDontFade = false;
hsScalar plCameraBrain1::fFallAccel			= 20.0f;
hsScalar plCameraBrain1::fFallDecel			= 5.0f;
hsScalar plCameraBrain1::fFallVelocity		= 50.0f;
hsScalar plCameraBrain1::fFallPOAAccel		= 10.0f;
hsScalar plCameraBrain1::fFallPOADecel		= 10.0f;
hsScalar plCameraBrain1::fFallPOAVelocity	= 50.0f;

// basic camera brain now is a fixed brain by default.
// if it doesn't have a subject (an object) it will just look straight ahead.
// if there's a subject it will follow it.

// the avatar and drive cameras are subclasses of the basic brain.

plCameraBrain1::plCameraBrain1() :
fCurCamSpeed(0.0f),
fCurViewSpeed(0.0f),
fVelocity(30.0f),
fAccel(30.0f),
fDecel(30.0f),	
fPOAVelocity(30.0f),
fPOAAccel(30.0f),
fPOADecel(30.0f),
fSubjectKey(nil),
fRail(nil),
fXPanLimit(0.0f),
fZPanLimit(0.0f),
fPanSpeed(0.5f),
fZoomMin(0.0f),
fZoomMax(0.0f),
fZoomRate(0.0f),
fOffsetLength(0.0f),
fOffsetPct(1.0f)
{
}

plCameraBrain1::plCameraBrain1(plCameraModifier1* pMod) :
fCurCamSpeed(0.0f),
fCurViewSpeed(0.0f),
fVelocity(30.0f),
fAccel(30.0f),
fDecel(30.0f),	
fPOAVelocity(30.0f),
fPOAAccel(30.0f),
fPOADecel(30.0f),
fSubjectKey(nil),
fRail(nil),
fXPanLimit(0.0f),
fZPanLimit(0.0f),
fZoomMin(0.0f),
fZoomMax(0.0f),
fZoomRate(0.0f),
fOffsetLength(0.0f),
fOffsetPct(1.0f)
{
	fCamera = pMod;
	pMod->SetBrain(this);
	fPOAGoal.Set(0,0,0);
	fGoal.Set(1,1,1);
	fPOAOffset.Set(0,0,0);
	fTargetMatrix.Make(&fGoal, &fPOAGoal, &hsVector3(0,0,1));
	fFlags.Clear();
}

plCameraBrain1::~plCameraBrain1()
{
}

void plCameraBrain1::AddTarget()
{
	fTargetMatrix = fCamera->GetTarget()->GetCoordinateInterface()->GetLocalToWorld();
	hsVector3 view;
	fTargetMatrix.GetAxis(0, &view, 0);
	fGoal = fTargetMatrix.GetTranslate();
	fPOAGoal = fGoal - view;
	fCamera->SetTargetPos(fGoal);
	fCamera->SetTargetPOA(fPOAGoal);
}

// called when we are pushed on top of the camera stack (or re-activated by another popping off directly above)
void plCameraBrain1::Push(hsBool recenter)
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
void plCameraBrain1::SetFOVGoal(hsScalar h,	double t)
{ 
	if (fFOVGoal == h || h == fCamera->GetFOVh())
		return;
	
	hsScalar dif = h - fCamera->GetFOVh();
	fFOVAnimRate = dif / ((hsScalar)t);

	fFOVGoal = h; 
	fFOVStartTime = hsTimer::GetSysSeconds();
	fFOVEndTime = fFOVStartTime + t; 

	fFlags.SetBit(kAnimateFOV); 
}

// set parameters for how this camera zooms FOV based on user input (mostly for telescopes)
void plCameraBrain1::SetZoomParams(hsScalar max, hsScalar min, hsScalar rate)
{
	fZoomRate = rate;
	fZoomMax = max;
	fZoomMin = min;
	fFlags.SetBit(kZoomEnabled);
}


// periodic update - forced means we are forcing an update at a non-normal time to "prime" the camera 
// into position before it renders the first time (hence the fake 10 second frame delta)
void plCameraBrain1::Update(hsBool forced)
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
			fTargetMatrix.GetAxis(0, &view, 0);
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
	hsScalar dH = fFOVAnimRate * hsTimer::GetDelSysSeconds();
	
	dH += fCamera->GetFOVh();

	if ( (fFOVAnimRate < 0.0f && dH <= fFOVGoal) ||
		 (fFOVAnimRate > 0.0f && dH >= fFOVGoal)  )
	{
		fFlags.ClearBit(kAnimateFOV);
		dH = fFOVGoal;
	}

	fCamera->SetFOVw( (hsScalar)(dH * FOV_RATIO) );
	fCamera->SetFOVh( dH );

}

// move the camera's origin point (not where it is looking) toward where it is going
void plCameraBrain1::IMoveTowardGoal(double elapsedTime)
{
	hsBool current = plVirtualCam1::Instance()->IsCurrentCamera(GetCamera());

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
 	hsScalar distToGoal=dir.Magnitude();
	
	//smooth out stoppage...
	hsScalar adjMaxVel = fVelocity;
	if (distToGoal <= 5.0f && distToGoal > 0.1f)
	{	
		hsScalar mult = (distToGoal - 5.0f)*0.1f;
		adjMaxVel = fVelocity - hsABS(fVelocity*mult);
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

	hsScalar distMoved;
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
	hsBool current = plVirtualCam1::Instance()->IsCurrentCamera(GetCamera());
	
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
 	hsScalar distToGoal=dir.Magnitude();
	
	if (distToGoal > 0.0f)
		dir.Normalize();

	// smooth out stoppage
	hsScalar adjMaxVel = fPOAVelocity;
	if (distToGoal <= 5.0f && distToGoal > 0.1f)
	{	
		hsScalar mult = (distToGoal - 5.0f)*0.1f;
		adjMaxVel = fPOAVelocity - hsABS(fPOAVelocity*mult);
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
	
	hsScalar distMoved;
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


void plCameraBrain1::IAdjustVelocity(hsScalar adjAccelRate, hsScalar adjDecelRate, 
									   hsVector3* dir, hsVector3* vel, hsScalar maxSpeed, 
									   hsScalar distToGoal, double elapsedTime)
{
	hsScalar speed = vel->Magnitude();		// save current speed
	*vel = *dir * speed;					// change vel to correct dir

	// compute accel/decel
	hsScalar finalAccelRate;

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
		accelVec = accelVec * (hsScalar)elapsedTime;

		// add acceleration to velocity
		*vel = *vel + accelVec;
	}
	else
	{
		*vel = *dir * maxSpeed;
	}
}

hsScalar plCameraBrain1::IClampVelocity(hsVector3* vel, hsScalar maxSpeed, double elapsedTime)
{
	*vel = *vel * (hsScalar)elapsedTime;
	maxSpeed *= (hsScalar)elapsedTime;

	// clamp speed	(clamp if going negative?)
	hsScalar distMoved = vel->Magnitude();
	if (distMoved > maxSpeed)
	{
		vel->Normalize();
		*vel = *vel * maxSpeed;
		return maxSpeed;
	}
	return distMoved;
}

hsBool plCameraBrain1::IShouldDecelerate(hsScalar decelSpeed, hsScalar curSpeed, hsScalar distToGoal)
{
	if (decelSpeed == 0)
		// no deceleration
		return false;

	// compute distance required to stop, given decel speed (in units/sec sq)
	hsScalar stopTime = curSpeed / decelSpeed;		
	hsScalar avgSpeed = curSpeed * .5f;
	hsScalar stopDist = avgSpeed * stopTime;

	return (hsABS(distToGoal) <= hsABS(stopDist));	// stopDist+avgSpeed?	
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
		hsScalar len = v.Magnitude();
		len = len - (len * fOffsetPct);
		v.Normalize();
		fGoal = fGoal + (v * len);
	}
}

void plCameraBrain1::Read(hsStream* stream, hsResMgr* mgr)
{
	hsKeyedObject::Read(stream, mgr);
	fPOAOffset.Read(stream);
	plGenRefMsg* msg = TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kSubject ); // SceneObject
	mgr->ReadKeyNotifyMe( stream, msg, plRefFlags::kActiveRef );
	
	plGenRefMsg* msg2 = TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnRequest, 0, kRailComponent ); // SceneObject
	mgr->ReadKeyNotifyMe( stream, msg2, plRefFlags::kActiveRef );

	fFlags.Read(stream);

	if (fFlags.IsBitSet(kFollowLocalAvatar))
	{
		if (plNetClientApp::GetInstance() && plNetClientApp::GetInstance()->GetLocalPlayer())
			SetSubject((plSceneObject*)plNetClientApp::GetInstance()->GetLocalPlayer());
			
		plgDispatch::Dispatch()->RegisterForExactType(plPlayerPageMsg::Index(), GetKey());
	}
 	fAccel = stream->ReadSwapFloat();
	fDecel = stream->ReadSwapFloat();
	fVelocity = stream->ReadSwapFloat();
	fPOAAccel = stream->ReadSwapFloat();
	fPOADecel = stream->ReadSwapFloat();
	fPOAVelocity = stream->ReadSwapFloat();
	fXPanLimit = stream->ReadSwapFloat();
	fZPanLimit = stream->ReadSwapFloat();
	fZoomRate = stream->ReadSwapFloat();
	fZoomMin = stream->ReadSwapFloat();
	fZoomMax = stream->ReadSwapFloat();

}

void plCameraBrain1::Write(hsStream* stream, hsResMgr* mgr)
{
	hsKeyedObject::Write(stream, mgr);
	fPOAOffset.Write(stream);
	mgr->WriteKey(stream, GetSubject());
	mgr->WriteKey(stream, fRail);
	fFlags.Write(stream);
	stream->WriteSwapFloat(fAccel);
	stream->WriteSwapFloat(fDecel);
	stream->WriteSwapFloat(fVelocity);
	stream->WriteSwapFloat(fPOAAccel);
	stream->WriteSwapFloat(fPOADecel);
	stream->WriteSwapFloat(fPOAVelocity);
	stream->WriteSwapFloat(fXPanLimit);
	stream->WriteSwapFloat(fZPanLimit);
	stream->WriteSwapFloat(fZoomRate);
	stream->WriteSwapFloat(fZoomMin);
	stream->WriteSwapFloat(fZoomMax);
}
hsBool plCameraBrain1::MsgReceive(plMessage* msg)
{
	plCameraMsg* pCamMsg = plCameraMsg::ConvertNoRef(msg);
	if (pCamMsg)
	{
		if (pCamMsg->Cmd(plCameraMsg::kStartZoomIn))
		{
			fFlags.SetBit(kAnimateFOV);
			fFOVGoal = fZoomMin;
			fFOVAnimRate = -1*fZoomRate;
			return true;
		}
		else
		if (pCamMsg->Cmd(plCameraMsg::kStartZoomOut))
		{
			fFlags.SetBit(kAnimateFOV);
			fFOVGoal = fZoomMax;
			fFOVAnimRate = fZoomRate;
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
				SetSubject(nil);
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
				fRail = nil;
				fFlags.ClearBit(kRailComponent);
			}
			return true;
		}
	}
	plPlayerPageMsg* pPMsg = plPlayerPageMsg::ConvertNoRef(msg);
	if (pPMsg)
	{
		if (pPMsg->fPlayer == plNetClientMgr::GetInstance()->GetLocalPlayerKey())
		{
			if (pPMsg->fUnload)
			{
				if (fFlags.IsBitSet(kFollowLocalAvatar))
					SetSubject(nil);
			}
			else
			{
				plGenRefMsg* msg = TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kSubject ); // SceneObject
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
				fFOVGoal = fZoomMin;
				fFOVAnimRate = -1*fZoomRate;
				fFOVEndTime = hsTimer::GetSysSeconds() + 60;
				return true;
			}
			else
			if (pCMsg->GetControlCode() == B_CAMERA_ZOOM_OUT && fFlags.IsBitSet(kZoomEnabled))
			{
				fFlags.SetBit(kAnimateFOV);
				fFOVGoal = fZoomMax;
				fFOVAnimRate = fZoomRate;
				fFOVEndTime = hsTimer::GetSysSeconds() + 60;
				return true;
			}
			else
			if (pCMsg->GetControlCode() == B_CAMERA_RECENTER)
			{
				if (fFlags.IsBitSet(kZoomEnabled))
				{
					fFlags.SetBit(kAnimateFOV);
					fFOVGoal = fZoomMin + ((fZoomMax - fZoomMin) / 2);
					if (fCamera->GetFOVw() >= fFOVGoal)
						fFOVAnimRate = -1*fZoomRate;
					else
						fFOVAnimRate = fZoomRate;
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
	return nil;
}

void plCameraBrain1::SetSubject(plSceneObject* sub)
{
	if (sub)
		fSubjectKey = sub->GetKey();
	else
		fSubjectKey = nil;
}

//
//
// new drive mode brain
//


hsScalar plCameraBrain1_Drive::fTurnRate = 100.0f;
hsScalar plCameraBrain1_Drive::fAcceleration = 200.0f;
hsScalar plCameraBrain1_Drive::fDeceleration = 200.0f;
hsScalar plCameraBrain1_Drive::fMaxVelocity = 100.0f;


// constructor
plCameraBrain1_Drive::plCameraBrain1_Drive() : plCameraBrain1()
{
	fGoal.Set(100,100,100);
	fPOAGoal.Set(0,0,0);
	fUp.Set(0,0,1);
	fCamera->SetTargetPos(fGoal);
	fCamera->SetTargetPOA(fPOAGoal);
	fLastTime = 0.f;
}

plCameraBrain1_Drive::plCameraBrain1_Drive(plCameraModifier1* pMod) : plCameraBrain1(pMod)
{	
	fGoal.Set(100,100,100);
	fPOAGoal.Set(0,0,0);
	fUp.Set(0,0,1);
	fCamera->SetTargetPos(fGoal);
	fCamera->SetTargetPOA(fPOAGoal);
	fLastTime = 0.f;
}

// destructor
plCameraBrain1_Drive::~plCameraBrain1_Drive()
{
}


void plCameraBrain1_Drive::Push(hsBool recenter)
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
void plCameraBrain1_Drive::Update(hsBool forced)
{
	
	fTargetMatrix.Make(&fGoal, &fPOAGoal, &(-1*fUp));
	
	// update our desired position:
	double time = hsTimer::GetSeconds();
	hsScalar eTime = (hsScalar)(time-fLastTime);
	if(eTime > 0.01f)
		eTime = 0.01f;
	fLastTime = time;
	hsPoint3 cameraPos = fCamera->GetTargetPos();
	hsVector3 view, up, right;
		
	fTargetMatrix.GetAxis(&view,&up,&right);
	hsScalar delta = 5.0f * eTime;

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

	hsScalar speed = fMaxVelocity;
	hsScalar turn = 1.0f;
	
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

hsBool plCameraBrain1_Drive::MsgReceive(plMessage* msg)
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
plCameraBrain1_Avatar::plCameraBrain1_Avatar() : plCameraBrain1()
{
	bObscured = false;
	fOffset.Set(0,0,0);
	fPOAOffset.Set(0,0,0);
	fFaded = false;
}

plCameraBrain1_Avatar::plCameraBrain1_Avatar(plCameraModifier1* pMod) : plCameraBrain1(pMod)
{
	bObscured = false;
	fOffset.Set(0,0,0);
	fPOAOffset.Set(0,0,0);
	fFaded = false;
}
	

// destructor
plCameraBrain1_Avatar::~plCameraBrain1_Avatar()
{
	if (!plNetClientMgr::GetInstance())
		return;
		
	if (fFaded)
	{
		plCameraTargetFadeMsg* pMsg = TRACKED_NEW plCameraTargetFadeMsg;
		pMsg->SetFadeOut(false);
		pMsg->SetSubjectKey(plNetClientMgr::GetInstance()->GetLocalPlayerKey());
		pMsg->SetBCastFlag(plMessage::kBCastByExactType);
		pMsg->SetBCastFlag(plMessage::kNetPropagate, FALSE);
		pMsg->AddReceiver(plNetClientMgr::GetInstance()->GetLocalPlayerKey());
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

void plCameraBrain1_Avatar::Push(hsBool recenter)
{
	bObscured = false;
	fFallTimer = 0.0f;
	plCameraBrain1::Push(recenter);
}


// 
// Update Method
//
void plCameraBrain1_Avatar::Update(hsBool forced)
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
			hsStatusMessageF("%s thinks it's the transition camera\n",GetKeyName());
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
		plLOSRequestMsg* pMsg = TRACKED_NEW plLOSRequestMsg( GetCamera()->GetKey(), fPOAGoal, fGoal, plSimDefs::kLOSDBCameraBlockers,
			plLOSRequestMsg::kTestClosest, plLOSRequestMsg::kReportHitOrMiss);
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

void plCameraBrain1_Avatar::ISendFadeMsg(hsBool fade)
{
	if (plVirtualCam1::IsCurrentCamera(GetCamera()))
	{
		if (fade)
			plVirtualCam1::AddMsgToLog("current camera sending Fade Out message to Avatar");
		else
			plVirtualCam1::AddMsgToLog("current camera sending Fade In message to Avatar");
	
		plCameraTargetFadeMsg* pMsg = TRACKED_NEW plCameraTargetFadeMsg;
		pMsg->SetFadeOut(fade);
		pMsg->SetSubjectKey(GetSubject()->GetKey());
		pMsg->SetBCastFlag(plMessage::kBCastByExactType);
		pMsg->SetBCastFlag(plMessage::kNetPropagate, FALSE);
		pMsg->AddReceiver(GetSubject()->GetKey());
		plgDispatch::MsgSend(pMsg);
	}
}

hsBool plCameraBrain1_Avatar::MsgReceive(plMessage* msg)
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
			fObstacle = nil;
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
				plSceneObject* avSO = nil;
				if (plNetClientMgr::GetInstance())
					avSO = plSceneObject::ConvertNoRef(plNetClientMgr::GetInstance()->GetLocalPlayer());

				if (GetSubject() == avSO)
				{
					plArmatureMod* avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
					if (avMod)
						avMod->RegisterForBehaviorNotify(GetKey());
				}
			}
			else
				SetSubject(nil);
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
				{	plVirtualCam1::Instance()->ClearFlags(plVirtualCam1::kFalling);
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
		plSceneObject* avSO = plSceneObject::ConvertNoRef(plNetClientMgr::GetInstance()->GetLocalPlayer());
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

plCameraBrain1_FirstPerson::plCameraBrain1_FirstPerson() : plCameraBrain1_Avatar()
{
	fPosNode = nil;
}

plCameraBrain1_FirstPerson::plCameraBrain1_FirstPerson(plCameraModifier1* pMod) : plCameraBrain1_Avatar(pMod)
{
}	

// destructor
plCameraBrain1_FirstPerson::~plCameraBrain1_FirstPerson()
{
}

hsBool plCameraBrain1_FirstPerson::MsgReceive(plMessage* msg)
{

	plGenRefMsg* pRefMsg = plGenRefMsg::ConvertNoRef(msg);
	if (pRefMsg)
	{
		if (pRefMsg->fType == kSubject)
		{
			if( pRefMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
			{	
				fPosNode = nil;
				SetSubject((plSceneObject*)pRefMsg->GetRef());
				// are we the built-in 1st person camera?  If we are, change our subject pointer to FPCameraOrigin node on the avatar
				plUoid U(kDefaultCameraMod1_KEY);
				plKey pKey = hsgResMgr::ResMgr()->FindKey(U);
				if (pKey && pKey == fCamera->GetKey())
				{
					const plCoordinateInterface* ci = GetSubject()->GetCoordinateInterface();
					for (int i = 0; i < ci->GetNumChildren(); i++)
					{
						plSceneObject* child = (plSceneObject*)ci->GetChild(i)->GetOwner();
						if (child)
						{
							const char* name = child->GetKeyName();
							if (stricmp(name, "FPCameraOrigin") == 0)
							{
								fPosNode = child;
								SetOffset(hsVector3(0,0,0));
								SetPOAOffset(hsVector3(0,-10,0));
								return true;
							}
						}
					}
				}
			}
			else
			{
				fPosNode = nil;
				SetSubject(nil);
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
	hsPoint3 targetFrom;
	if (fPosNode)
		targetFrom = fPosNode->GetCoordinateInterface()->GetLocalToWorld().GetTranslate();
	else
		targetFrom = GetSubject()->GetCoordinateInterface()->GetLocalToWorld().GetTranslate();

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
		plLOSRequestMsg* pMsg = TRACKED_NEW plLOSRequestMsg( GetCamera()->GetKey(), fPOAGoal, fGoal, plSimDefs::kLOSDBCameraBlockers,
			plLOSRequestMsg::kTestClosest, plLOSRequestMsg::kReportHitOrMiss);
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


void plCameraBrain1_FirstPerson::Push(hsBool recenter)
{
	if (!GetSubject())
		return; 
	
	if (plCameraBrain1_FirstPerson::fDontFade)
		return;
	plEnableMsg* pMsg = TRACKED_NEW plEnableMsg;
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
	fTargetPoint= nil;
}

plCameraBrain1_Fixed::plCameraBrain1_Fixed(plCameraModifier1* pMod) : plCameraBrain1(pMod)
{
	fTargetPoint = nil;
}	

// destructor
plCameraBrain1_Fixed::~plCameraBrain1_Fixed()
{
}

void plCameraBrain1_Fixed::Read(hsStream* stream, hsResMgr* mgr)
{
	plCameraBrain1::Read(stream, mgr);
	mgr->ReadKeyNotifyMe( stream, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, 99), plRefFlags::kPassiveRef);
}

void plCameraBrain1_Fixed::Write(hsStream* stream, hsResMgr* mgr)
{
	plCameraBrain1::Write(stream, mgr);
	mgr->WriteKey(stream, fTargetPoint);
}

void plCameraBrain1_Fixed::Update(hsBool forced)
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
				hsVector3 view;
				hsVector3 up;
				fTargetMatrix.GetAxis(0, &view, &up);
				fGoal = fTargetMatrix.GetTranslate();
				if (fTargetPoint)
					fPOAGoal = fTargetPoint->GetBrain()->GetGoal();
				else
					fPOAGoal = fGoal - (view * 10);
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

hsBool plCameraBrain1_Fixed::MsgReceive(plMessage* msg)
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
static const hsScalar kTwoPI	= 2.0f*hsScalarPI;

plCameraBrain1_Circle::plCameraBrain1_Circle() : plCameraBrain1_Fixed()
{ 
	fCircleFlags = 0;
	fCenterObject = nil;
	fCenter.Set(0,0,0);
	fCurRad = fGoalRad = 1;
	fPOAObj = nil;
	fCirPerSec = 0.25f;
}
plCameraBrain1_Circle::plCameraBrain1_Circle(plCameraModifier1* pMod) : plCameraBrain1_Fixed(pMod)
{ 
	fCircleFlags = 0;
	fCenterObject = nil;
	fCenter.Set(0,0,0);
	fCurRad = fGoalRad = 1;
	fPOAObj = nil;
	fCirPerSec = 0.25f;
}

plCameraBrain1_Circle::~plCameraBrain1_Circle() 
{ 
}

//
//
//
void plCameraBrain1_Circle::Update(hsBool forced)
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
	
	if (HasFlag(kCutPosOnce))
	{
		fGoal = MoveTowardsFromGoal(&fCamera->GetTargetPos(), secs, true);
		fFlags.ClearBit(kCutPos);
	}
	else
	{	
		fGoal = MoveTowardsFromGoal(&fCamera->GetTargetPos(), secs);
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
hsPoint3 plCameraBrain1_Circle::MoveTowardsFromGoal(const hsPoint3* fromGoal, double secs, hsBool warp)
{

	if (fCurRad != fGoalRad)
	{
		hsScalar dist = hsABS(fGoalRad-fCurRad);
	
		hsAssert(dist>=0 && dist<=kTwoPI, "illegal radian diff");
		hsBool mustWrap = (dist > hsScalarPI);	// go opposite direction for shortcut and wrap

		// compute speed
		hsScalar speed; 
		if (warp)
			speed = (hsScalar)(kTwoPI * 100 * secs);
		else
			speed = (hsScalar)(kTwoPI * fCirPerSec * secs);

		// move towards goalRad
		hsAssert(fCurRad>=0 && fCurRad<=kTwoPI, "illegal radian value");

		if (fCurRad<fGoalRad)
		{
			if (mustWrap)
			{
				fCurRad-=speed;
				hsBool didWrap=false;
				while(fCurRad<0)
				{
					didWrap=true;
					fCurRad+=kTwoPI;
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
				hsBool didWrap=false;
				while(fCurRad>kTwoPI)
				{
					didWrap=true;
					fCurRad-=kTwoPI;
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
	hsAssert(fCurRad>=0 && fCurRad<=kTwoPI, "illegal radian value");
	
	hsPoint3 x;
	x = GetCenterPoint() + hsVector3((hsScalar)hsCosine(fCurRad)*fRadius, (hsScalar)hsSine(fCurRad)*fRadius, 0.0f);
	x.fZ = fCamera->GetTargetPos().fZ;
	return x;
}

hsPoint3 plCameraBrain1_Circle::IGetClosestPointOnCircle(const hsPoint3* toThis)
{
	hsPoint3 center=GetCenterPoint();
	hsPoint3 p(toThis->fX, toThis->fY, center.fZ);	// Move to plane of circle
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
	fGoalRad = (hsScalar)atan2(v.fY, v.fX);	// -pi to pi
	hsAssert(fGoalRad>=-hsScalarPI && fGoalRad<=hsScalarPI, "Illegal atan2 val");
	if (fGoalRad<0)
		fGoalRad = kTwoPI + fGoalRad;	// 0 to 2pi
	hsAssert(fGoalRad>=0 && fGoalRad<=kTwoPI, "Illegal atan2 val");
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
	stream->WriteSwap32(GetCircleFlags());
	fCenter.Write(stream);
	stream->WriteSwapScalar(GetRadius());
	mgr->WriteKey(stream, fCenterObject);
	mgr->WriteKey(stream, fPOAObj);
	stream->WriteSwapScalar(fCirPerSec);
}

void plCameraBrain1_Circle::Read(hsStream* stream, hsResMgr* mgr)
{
	plCameraBrain1::Read(stream, mgr);
	SetCircleFlags(stream->ReadSwap32());
	if (GetCircleFlags() & kCircleLocalAvatar)
	{
		if (plNetClientApp::GetInstance() && plNetClientApp::GetInstance()->GetLocalPlayer())
			SetPOAObject((plSceneObject*)plNetClientApp::GetInstance()->GetLocalPlayer());
			
		plgDispatch::Dispatch()->RegisterForExactType(plPlayerPageMsg::Index(), GetKey());
	}

	fCenter.Read(stream);
	SetRadius(stream->ReadSwapScalar());
	plGenRefMsg* msg = TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kCircleTarget ); // SceneObject
	mgr->ReadKeyNotifyMe( stream, msg, plRefFlags::kActiveRef );
	plGenRefMsg* msg2 = TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnRequest, 0, kPOAObject ); // SceneObject
	mgr->ReadKeyNotifyMe( stream, msg2, plRefFlags::kActiveRef );
	fCirPerSec = stream->ReadSwapScalar();
	plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
}

hsBool plCameraBrain1_Circle::MsgReceive(plMessage* msg)
{
	plGenRefMsg* pRefMsg = plGenRefMsg::ConvertNoRef(msg);
	if (pRefMsg)
	{
		if (pRefMsg->fType == kCircleTarget)
		{
			if( pRefMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
				fCenterObject = (plSceneObject*)pRefMsg->GetRef();
			else
				fCenterObject = nil;
			return true;
		}
		else
		if (pRefMsg->fType == kPOAObject)
		{
			if( pRefMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
				fPOAObj = (plSceneObject*)pRefMsg->GetRef();
			else
				fPOAObj = nil;
			return true;
		}
		else
		if (pRefMsg->fType == kSubject)
		{
			if( pRefMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
				SetSubject((plSceneObject*)pRefMsg->GetRef());
			else
				SetSubject(nil);
			return true;
		}
	}
	plPlayerPageMsg* pPMsg = plPlayerPageMsg::ConvertNoRef(msg);
	if (pPMsg && pPMsg->fPlayer == plNetClientMgr::GetInstance()->GetLocalPlayerKey())
	{
		if (pPMsg->fUnload)
		{
			if (GetCircleFlags() & kCircleLocalAvatar)
				fPOAObj = nil;
			if (fFlags.IsBitSet(kFollowLocalAvatar))
				SetSubject(nil);
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
				plGenRefMsg* msg = TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kSubject ); // SceneObject
				hsgResMgr::ResMgr()->AddViaNotify(pPMsg->fPlayer, msg, plRefFlags::kPassiveRef);

				fFlags.SetBit(kCutPosOnce);
				fFlags.SetBit(kCutPOA);
			}
		}
		return true;
	}
	return (plCameraBrain1_Fixed::MsgReceive(msg));
}


