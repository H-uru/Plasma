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

#include "plVirtualCamNeu.h"
#include "plCameraModifier.h"
#include "plCameraBrain.h"
#include "plPipeline.h"
#include "pfCameraProxy.h"
#include "plgDispatch.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnKeyedObject/plFixedKey.h"
#include "hsMatrix44.h"
#include "../pnSceneObject/plSceneObject.h"
#include "hsResMgr.h"
#include "hsTimer.h"
#include "../plAudio/plAudioSystem.h"
#include "../plInputCore/plInputDevice.h"
#include "../plInputCore/plInputManager.h"
#include "../pnInputCore/plKeyDef.h"
#include "../plScene/plSceneNode.h"
#include "../plMessage/plAvatarMsg.h"
#include "../pnMessage/plWarpMsg.h"
#include "../pnMessage/plCameraMsg.h"
#include "../pnMessage/plEnableMsg.h"
#include "../pnMessage/plTimeMsg.h"
#include "../pnMessage/plObjRefMsg.h"
#include "../pnMessage/plCmdIfaceModMsg.h"
#include "../pnMessage/plPlayerPageMsg.h"
#include "../plMessage/plInputEventMsg.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../plDrawable/plDrawableGenerator.h"
#include "../plScene/plSceneNode.h"
#include "../plInputCore/plDebugInputInterface.h"
#include "../plInputCore/plAvatarInputInterface.h"
#include "../plMessage/plInputIfaceMgrMsg.h"
#include "../plStatusLog/plStatusLog.h"
#include "../plPipeline/plPlates.h"
#include "../plGImage/plMipmap.h"
#include "../plSurface/plLayer.h"
#include "../plSurface/hsGMaterial.h"
#include "../pnNetCommon/plNetApp.h"
#include "../plNetClient/plNetClientMgr.h"
#include "../plAvatar/plAvBrainHuman.h"
#include "../plAvatar/plAvatarMgr.h"

#include "hsGeometry3.h"
#include "hsConfig.h"
#include "hsQuat.h"

hsScalar plVirtualCam1::fFOVw			=  45.0f;
hsScalar plVirtualCam1::fFOVh			=  33.75f;
hsScalar plVirtualCam1::fHither			=	0.3f;
hsScalar plVirtualCam1::fYon			= 500.0f;
hsBool   plVirtualCam1::printFOV		= false;
hsBool	 plVirtualCam1::fUseAccelOverride	= 1;
hsBool	 plVirtualCam1::freeze	= 0;
//hsScalar plVirtualCam1::fAccel			= 5.0f;
//hsScalar plVirtualCam1::fDecel			= 5.0f;
//hsScalar plVirtualCam1::fVel			= 10.0f;
hsScalar plVirtualCam1::fAccel			= 50.0f;
hsScalar plVirtualCam1::fDecel			= 50.0f;
hsScalar plVirtualCam1::fVel			= 100.0f;
hsScalar plVirtualCam1::fPanResponseTime	= 3.0f;
hsScalar plVirtualCam1::fFallTimerDelay	= 0.25f;
hsBool	 plVirtualCam1::alwaysCutForColin = false;
hsBool	 plVirtualCam1::WalkPan3rdPerson = false;
hsBool     plVirtualCam1::StayInFirstPersonForever = false;
float	 plVirtualCam1::fAspectRatio = fFOVw / fFOVh;

// #define STATUS_LOG

#ifdef STATUS_LOG
static plStatusLog *camLog = nil;
#endif

// static functions
void plVirtualCam1::AddMsgToLog(const char* msg)
{
#ifdef STATUS_LOG
	if (camLog)
		camLog->AddLine(msg);
#endif
}

hsBool plVirtualCam1::IsCurrentCamera(const plCameraModifier1* mod)
{
	if (plVirtualCam1::Instance())
	{
		if (plVirtualCam1::Instance()->InTransition())
			return(plVirtualCam1::Instance()->GetTransitionCamera() == mod);
		
		return (plVirtualCam1::Instance()->GetCurrentCamera() == mod);
	}
	return false;
}

plVirtualCam1* plVirtualCam1::fInstance = nil;

void plVirtualCam1::Deactivate() 
{ 
}


plVirtualCam1::plVirtualCam1() 
{
	fFlags.Clear();
	fPythonOverride = nil;
	fFirstPersonOverride = nil;
	fThirdPersonCam = nil;
	fTransPos = POS_TRANS_OFF;
	fPrevCam = nil;
	fTransitionCamera = TRACKED_NEW plCameraModifier1;
	fTransitionCamera->RegisterAs(kTransitionCamera_KEY);
	// set initial view position
	fOutputPos.Set(100,100,100);
	fOutputPOA.Set(0,0,0);
	fEffectPlate = nil;
	fFreezeCounter = 0;
	fFadeCounter = 0;
	fX = fY = 0.5f;
	fXPanLimit = 0;
	fZPanLimit = 0;
	fRetainedFY = 0.5f;
	// create built-in drive mode camera
	fCameraDriveInterface = plDebugInputInterface::GetInstance();
	hsRefCnt_SafeRef( fCameraDriveInterface );

	fDriveCamera = TRACKED_NEW plCameraModifier1;
	plCameraBrain1* pDriveBrain = TRACKED_NEW plCameraBrain1_Drive(fDriveCamera);

	PushCamera(fDriveCamera);
	fForceCutOnce=false;

	// static accessor hack
	plVirtualCam1::fInstance = this;
	
	#ifdef STATUS_LOG
	if (!camLog)
		camLog = plStatusLogMgr::GetInstance().CreateStatusLog(40, "Camera.log", plStatusLog::kFilledBackground | plStatusLog::kDeleteForMe | plStatusLog::kAlignToTop);
//		camLog = plStatusLogMgr::GetInstance().CreateStatusLog(40, "Camera", plStatusLog::kFilledBackground | plStatusLog::kDeleteForMe | plStatusLog::kDontWriteFile | plStatusLog::kAlignToTop);
	#endif

	ICreatePlate();

	foutLog = nil;
#ifndef PLASMA_EXTERNAL_RELEASE
	// only open log file if logging is on
	if ( !plStatusLog::fLoggingOff )
	{
		wchar fileAndPath[MAX_PATH];
		PathGetLogDirectory(fileAndPath, arrsize(fileAndPath));
		PathAddFilename(fileAndPath, fileAndPath, L"camLog.txt", arrsize(fileAndPath));
		foutLog = _wfopen( fileAndPath, L"wt" );
	}
#endif

	SetFlags(kFirstPersonEnabled);

}

plVirtualCam1::~plVirtualCam1()
{
	if(fTransitionCamera->GetBrain())
	{	
		delete(fTransitionCamera->GetBrain());
	}
	fTransitionCamera->UnRegisterAs(kTransitionCamera_KEY);
	delete(fDriveCamera->GetBrain());
	delete(fDriveCamera);
	hsRefCnt_SafeUnRef( fCameraDriveInterface );

	if(fThirdPersonCam)
	{	
		delete(fThirdPersonCam->GetBrain());
		fThirdPersonCam->UnRegisterAs(kBuiltIn3rdPersonCamera_KEY);
	}
	plUoid U(kDefaultCameraMod1_KEY);
	plKey pKey = hsgResMgr::ResMgr()->FindKey(U);
	if (pKey)
	{	
		delete((plCameraModifier1*)pKey->GetObjectPtr())->GetBrain();
		pKey->GetObjectPtr()->UnRegisterAs(kDefaultCameraMod1_KEY);	
	}
	if (fEffectPlate)
		plPlateManager::Instance().DestroyPlate(fEffectPlate);
}

// for saving camera stack
plCameraModifier1* plVirtualCam1::GetCameraNumber(int camNumber)
{ 
	return (fCameraStack[camNumber]); 
}
// for rebuilding camera stack
void plVirtualCam1::RebuildStack(const plKey& key)
{
	if (fCameraStack.Count() == 1)
	{
		plUoid U(kDefaultCameraMod1_KEY);
		plKey pKey = hsgResMgr::ResMgr()->FindKey(U);
		if (pKey)
		{
			if (fCameraStack[0]->GetKey() == pKey)
				fCameraStack.SetCountAndZero(0);
		}
	}
	plSceneObject* pObj = plSceneObject::ConvertNoRef(key->GetObjectPtr());
	if (pObj)
	{
		
		plCameraModifier1* pMod = (plCameraModifier1*)pObj->GetModifierByType(plCameraModifier1::Index());
		if (pMod)
			AddCameraToStack(pMod);
		else
			PushThirdPerson();
	}
	else
	{
		plCameraModifier1* pMod = plCameraModifier1::ConvertNoRef(key->GetObjectPtr());
		if (pMod)
			AddCameraToStack(pMod);
		else
			PushThirdPerson();
	}
	if (!HasFlags(kFirstPersonAtLinkOut))
	{
		plEnableMsg* pMsg = TRACKED_NEW plEnableMsg;
		pMsg->SetSender(GetKey());
		pMsg->SetCmd(plEnableMsg::kEnable);
		pMsg->AddType(plEnableMsg::kDrawable);
		pMsg->SetBCastFlag(plMessage::kPropagateToModifiers);
		pMsg->AddReceiver(plNetClientMgr::GetInstance()->GetLocalPlayerKey());
		plgDispatch::MsgSend(pMsg);
	}
	
	fForceCutOnce=true;
	
}

// hack for console command to force offset 
void plVirtualCam1::SetOffset(float x, float y, float z)
{
	plCameraModifier1* pCam = plVirtualCam1::Instance()->GetCurrentCamera();
	if (!pCam)
		return;
	hsVector3 pt(x,y,z);
	if (plCameraBrain1_Avatar::ConvertNoRef(pCam->GetBrain()))
		((plCameraBrain1_Avatar*)(pCam->GetBrain()))->SetOffset(pt);
}

// static function
void plVirtualCam1::SetFOV(hsScalar x, hsScalar y)
{

	float fovW = y * fAspectRatio;

	fFOVw = fovW;
	fFOVh = y;

	if (! plVirtualCam1::Instance()->fPipe)	
		return;

	plVirtualCam1::Instance()->SetFlags(plVirtualCam1::kSetFOV);

	#ifdef STATUS_LOG
if (printFOV)
		camLog->AddLineF("FOV changed by console command to %f", fFOVw);
#endif

}
// static function
void plVirtualCam1::SetFOV(hsScalar x, hsScalar y, plCameraModifier1* pCam)
{
	if (plVirtualCam1::Instance()->GetCurrentCamera() != pCam)
		return;

	hsScalar diff = hsABS(fFOVw - x);
	if (diff > 10.0f)
	{
#ifdef STATUS_LOG
		camLog->AddLineF("Radical FOV change of %f", diff);
#endif

	}

	float fovW = y * fAspectRatio;

	fFOVw = fovW;
	fFOVh = y;

	if (! plVirtualCam1::Instance()->fPipe)	
		return;
	
	plVirtualCam1::Instance()->SetFlags(plVirtualCam1::kSetFOV);
	
#ifdef STATUS_LOG
	if (printFOV)
		camLog->AddLineF("FOV changed to %f", fFOVw);
#endif
}

// static function

void plVirtualCam1::SetDepth(hsScalar h, hsScalar y)
{
	return;
	fHither = h;
	fYon = y;
	if (! plVirtualCam1::Instance()->fPipe)
		return;
	plVirtualCam1::Instance()->fPipe->SetDepth(fHither, fYon);
	plVirtualCam1::Instance()->fPipe->RefreshMatrices();
	#ifdef STATUS_LOG
	camLog->AddLineF("Hither, Yon changed to %f %f", fHither, fYon);
	#endif
}

// force drive mode from console
void plVirtualCam1::Drive()
{
	if (GetCurrentCamera() == fDriveCamera)
	{	
		fCameraDriveInterface->SetEnabled( false );
		PopCamera(fDriveCamera);

		#ifdef STATUS_LOG
		camLog->AddLineF("Camera Drive Mode Disabled");
		#endif
	}
	else
	{
		// set it to the current camera position -
		fDriveCamera->GetBrain()->SetGoal(GetCurrentCamera()->GetTargetPos());
		fDriveCamera->GetBrain()->SetPOAGoal(GetCurrentCamera()->GetTargetPOA());
		fDriveCamera->SetTargetPos(GetCurrentCamera()->GetTargetPos());
		fDriveCamera->SetTargetPOA(GetCurrentCamera()->GetTargetPOA());

		PushCamera(fDriveCamera);
	
		// push the interface on
		fCameraDriveInterface->SetEnabled( true );

		#ifdef STATUS_LOG
		camLog->AddLineF("Camera Drive Mode Enabled");
		#endif
	}
}


void plVirtualCam1::SetPipeline(plPipeline* p)
{ 
	fPipe = p; 
	SetFOV(plVirtualCam1::fFOVw, plVirtualCam1::fFOVh);
	SetRender(false);
}

void  plVirtualCam1::Reset(hsBool bRender)
{
	if (fPythonOverride)
		fPythonOverride = nil;
	if (fFirstPersonOverride)
		fFirstPersonOverride = nil;
	fCamerasLoaded.SetCountAndZero(0);
	fCameraStack.SetCountAndZero(0);
	fCameraStack.Append(fDriveCamera);
	plUoid U(kDefaultCameraMod1_KEY);
	plKey pKey = hsgResMgr::ResMgr()->FindKey(U);
	if (pKey)
		PushCamera((plCameraModifier1*)pKey->GetObjectPtr());
		//fCameraStack.Append((plCameraModifier1*)pKey->GetObjectPtr());
	
	if (fThirdPersonCam)
		PushCamera(fThirdPersonCam);
	SetRender(bRender);
	
	fX = fY = 0.5f;
	fRetainedFY = 0.5f;
	ClearFlags(kAvatarWalking);
	ClearFlags(kUnPanCamera);
	ClearFlags(kInterpPanLimits);
	ClearFlags(kResponderForced3rd);
	ClearFlags(kScriptsDisabled1st);
	ClearFlags(kFalling);
	ClearFlags(kFirstPersonEnabled);
	
	#ifdef STATUS_LOG
	camLog->AddLineF("Virtual Camera Reset");
	#endif

}

void  plVirtualCam1::ClearStack()
{
	fCameraStack.SetCountAndZero(0);
	plUoid U(kDefaultCameraMod1_KEY);
	plKey pKey = hsgResMgr::ResMgr()->FindKey(U);
	if (pKey)
		PushCamera((plCameraModifier1*)pKey->GetObjectPtr());

	#ifdef STATUS_LOG
	camLog->AddLineF("Camera Stack Cleared");
	#endif

}

plCameraModifier1* plVirtualCam1::GetCurrentCamera()
{
	if (GetCurrentStackCamera() == fDriveCamera)
		return fDriveCamera;
	if (fPythonOverride)
		return (fPythonOverride);
	if (fFirstPersonOverride)
		return(fFirstPersonOverride);
	if (fTransPos == POS_TRANS_FOLLOW)
		return(fTransitionCamera);

	if (fCameraStack.Count())
		return fCameraStack[fCameraStack.Count() - 1];
	else return nil;
}

hsBool plVirtualCam1::Is1stPersonCamera()
{
	if (GetCurrentStackCamera() == fDriveCamera)
		return false;
	if (fPythonOverride)
		return false;
	if (fFirstPersonOverride)
		return true;

	else return false;
}

plCameraModifier1* plVirtualCam1::GetCurrentStackCamera()
{
	if (fCameraStack.Count())
		return fCameraStack[fCameraStack.Count() - 1];
	else return nil;
}

void plVirtualCam1::ICreatePlate()
{
	int		x, y;


	fEffectPlate = nil;

	// +0.01 to deal with the half-pixel antialiasing stuff
	plPlateManager::Instance().CreatePlate( &fEffectPlate, 0, 0, 2.01, 2.01 );

	// hack for now--create a black layer that we will animate the opacity on
	plMipmap *ourMip = fEffectPlate->CreateMaterial( 16, 16, true );
	for( y = 0; y < ourMip->GetHeight(); y++ )
	{
		UInt32	*pixels = ourMip->GetAddr32( 0, y );
		for( x = 0; x < ourMip->GetWidth(); x++ )
			pixels[ x ] = 0xff000000;
	}
	if( fEffectPlate == nil )
		ICreatePlate();
	fEffectPlate->SetVisible( false );
}


void plVirtualCam1::SetCutNextTrans()
{
	SetFlags(kCutNextTrans);
	SetRender(true);
#ifdef STATUS_LOG
		camLog->AddLineF("Set Camera to cut on next transition");
#endif
}

void plVirtualCam1::SetRender(hsBool render)
{
	fFlags.SetBit(kRender,render);
	if (render)
	{
		#ifdef STATUS_LOG
		camLog->AddLineF("Virtual Camera Render Updates Enabled");
		if (fEffectPlate)
			fEffectPlate->SetVisible(false);
		#endif
	}
	else
	{	
		#ifdef STATUS_LOG
		camLog->AddLineF("Virtual Camera Render Updates Disabled");
		if (fEffectPlate)
			fEffectPlate->SetVisible(true);
		#endif
	}


}

// hack, hack, hack
hsBool plVirtualCam1::RestoreFromName(const char* name)
{
	for(int i = 0; i < fCamerasLoaded.Count(); i++)
	{
		if (strcmp(name, fCamerasLoaded[i]->GetKeyName()) == 0)
		{
			RebuildStack(fCamerasLoaded[i]->GetKey());
			return true;
		}
	}
	return false;
}
void plVirtualCam1::Next()
{
}

void plVirtualCam1::Prev()
{
}

void plVirtualCam1::PushThirdPerson()
{
	if (fThirdPersonCam)
	{
#ifdef STATUS_LOG
		camLog->AddLineF("Restore failed, forcing built-in 3rd person camera");
#endif
		ClearStack();
		SetCutNextTrans();
		PushCamera(fThirdPersonCam);
		return;
	}
#ifdef STATUS_LOG
		camLog->AddLineF("Restore failed, 3rd person camera not available for switch: attempting to force 1st person");
#endif
	FirstPersonOverride();

}


//
// Make adjustments to camera position based on 
// user input - NOTE this is for mouse-cursor based adjustment
//
void plVirtualCam1::StartInterpPanLimits()
{
	plCameraBrain1* pBrain = 0;
	if (fPythonOverride && fPythonOverride->GetBrain())
		pBrain = fPythonOverride->GetBrain();
	if (pBrain == 0 && GetCurrentStackCamera() && GetCurrentStackCamera()->GetBrain())
		pBrain = GetCurrentStackCamera()->GetBrain();

	if (pBrain)
	{
		if (pBrain->GetXPanLimit() == fXPanLimit &&
			pBrain->GetZPanLimit() == fZPanLimit )
		{	
			ClearFlags(kInterpPanLimits);
			return;
		}
		fXPanLimitGoal = pBrain->GetXPanLimit();
		fZPanLimitGoal = pBrain->GetZPanLimit();
		fXPanInterpRate = fXPanLimitGoal - fXPanLimit;
		fZPanInterpRate = fZPanLimitGoal - fZPanLimit;
		SetFlags(kInterpPanLimits);
		fInterpPanLimitTime = hsTimer::GetSysSeconds() + 1.0f;
	}
}

void plVirtualCam1::InterpPanLimits()
{
	if (fXPanLimitGoal == fXPanLimit && fZPanLimitGoal == fZPanLimit)
	{
		ClearFlags(kInterpPanLimits);
		return;
	}
	fXPanLimit += fXPanInterpRate * hsTimer::GetDelSysSeconds();
	fZPanLimit += fZPanInterpRate * hsTimer::GetDelSysSeconds();
	if ((fZPanInterpRate > 0 && fZPanLimit >= fZPanLimitGoal) ||
		(fZPanInterpRate < 0 && fZPanLimit <= fZPanLimitGoal) )
	{
		fZPanLimit = fZPanLimitGoal;
		fZPanInterpRate = 0;
	}
	if ((fXPanInterpRate > 0 && fXPanLimit >= fXPanLimitGoal) ||
		(fXPanInterpRate < 0 && fXPanLimit <= fXPanLimitGoal) )
	{
		fXPanLimit = fXPanLimitGoal;
		fXPanInterpRate = 0;
	}
	if (fXPanInterpRate == 0)
		fXPanLimit = fXPanLimitGoal;
	if (fZPanInterpRate == 0)
		fZPanLimit = fZPanLimitGoal;
}

void plVirtualCam1::StartUnPan()
{
	if (!HasFlags(kUnPanCamera))
	{
		SetFlags(kUnPanCamera);
		if (HasFlags(kFalling))
		{
			fUnPanEndTime = hsTimer::GetSysSeconds() + 0.5;
			fXUnPanRate = (0.5f - fX) / 0.5f; 
			fZUnPanRate = (0.5f - fY) / 0.5f; 
		
		}
		else
		{
			fUnPanEndTime = hsTimer::GetSysSeconds() + fPanResponseTime;
			fXUnPanRate = (0.5f - fX) / fPanResponseTime; 
			fZUnPanRate = (0.5f - fY) / fPanResponseTime; 
		}	
	}
}

void plVirtualCam1::UnPanIfNeeded()
{
	if (HasFlags(kUnPanCamera))
	{
		if (fX == 0.5f && fY == 0.5f)
		{
			ClearFlags(kUnPanCamera);
			return;
		}
		fX += fXUnPanRate * hsTimer::GetDelSysSeconds();
		fY += fZUnPanRate * hsTimer::GetDelSysSeconds();
		if ((fXUnPanRate > 0 && fX >= 0.5f) || (fXUnPanRate < 0 && fX <= 0.5f))
		{
			fX = 0.5f;
			fXUnPanRate = 0;
		}
		if ((fZUnPanRate > 0 && fY >= 0.5f) || (fZUnPanRate < 0 && fY <= 0.5f))
		{
			fY = 0.5f;
			fZUnPanRate = 0;
		}
	}
}


void plVirtualCam1::AdjustForInput()
{
	if (!fFirstPersonOverride)
	{	
		if (HasFlags(kInterpPanLimits))
			InterpPanLimits();

		UnPanIfNeeded();

		hsScalar panSpeed = 0.5f;
		double secs = hsTimer::GetDelSysSeconds();
		
		if (HasMovementFlag(B_CAMERA_PAN_UP))
			fY -= (hsScalar)(panSpeed * secs);
		if (HasMovementFlag(B_CAMERA_PAN_DOWN))
			fY += (hsScalar)(panSpeed * secs);	
		if (HasMovementFlag(B_CAMERA_PAN_LEFT))
			fX -= (hsScalar)(panSpeed * secs);
		if (HasMovementFlag(B_CAMERA_PAN_RIGHT))
			fX += (hsScalar)(panSpeed * secs);
	}
	if ((fY == 0.5f && fX == 0.5f) &&
		fFirstPersonOverride == nil) 
		return;
	
	if (fY > 1.0f)
		fY = 1.0f;
	if (fY < 0.0f)
		fY = 0.0f;
	if (fX > 1.0f)
		fX = 1.0f;
	if (fX < 0.0f)
		fX = 0.0f;

	if ((fXPanLimit == 0.0f && fZPanLimit == 0.0f) &&
		fFirstPersonOverride == nil) 
		return;

	hsMatrix44 m;
	
	hsVector3 v1(fOutputPOA - fOutputPos);
	hsVector3 v2(0,0,1);
	v1.Normalize();
	hsVector3 up = (v2 % v1) % v1;

	m.Make(&fOutputPos, &fOutputPOA, &up);


	// scale maximum angle by % mouse input

	hsScalar scaledX;
	if (fFirstPersonOverride)
		scaledX = 3.14159;
	else
		scaledX = (hsScalar)(3.14159 - (fXPanLimit * ( (fX - 0.5f) / 0.5f)));

	hsScalar scaledZ; 
	if (fFirstPersonOverride)
		scaledZ = (hsScalar)(3.14159 - (0.872f * ( (fY - 0.5f) / 0.5f)));
	else
		scaledZ = (hsScalar)(3.14159 - (fZPanLimit * ( (fY - 0.5f) / 0.5f)));

	hsMatrix44 mX;
	hsMatrix44 mZ;

	hsVector3 right = m.GetAxis(hsMatrix44::kRight);
	up = m.GetAxis(hsMatrix44::kUp);
	up *= -1;
	hsQuat qX(scaledX, &up);
	hsQuat qZ(scaledZ, &right);
	
	qX.MakeMatrix(&mX);
	qZ.MakeMatrix(&mZ);

	m = mX * m;
	m = mZ * m;

	hsVector3 view = m.GetAxis(hsMatrix44::kView);
	fOutputPOA = fOutputPos + (view * 15);

}


void plVirtualCam1::IUpdate()
{
	if (!HasFlags(kRender))
		return;

	
	if (fDriveCamera)
		fDriveCamera->Update();
	if (fPythonOverride)
		fPythonOverride->Update();
	if (fFirstPersonOverride)
		fFirstPersonOverride->Update();
	if (fTransitionCamera)
		fTransitionCamera->Update();

	RunTransition();
	
	for (int i=0; i < fCameraStack.Count(); i++)
	{	
		hsBool update = true;
		for (int j=i+1; j<fCameraStack.Count(); j++)
		{
			if (fCameraStack[i] == fCameraStack[j])
			{
				update = false;
				break;
			}
		}
		if ((fCameraStack[i] == GetCurrentStackCamera()) && fTransPos != POS_TRANS_OFF)
			update = false;
		if(update)
		{
			// eek...
			if(alwaysCutForColin && fCameraStack[i]->GetBrain())
			{
				fCameraStack[i]->GetBrain()->SetFlags(plCameraBrain1::kCutPos);
				fCameraStack[i]->GetBrain()->SetFlags(plCameraBrain1::kCutPOA);
			}
			if(fForceCutOnce)
			{
				fCameraStack[i]->GetBrain()->SetFlags(plCameraBrain1::kCutPosOnce);
				fCameraStack[i]->GetBrain()->SetFlags(plCameraBrain1::kCutPOAOnce);
			}
			fCameraStack[i]->Update();
		}
	}
	if(fForceCutOnce)fForceCutOnce=false;
	
	Output();
}


void plVirtualCam1::Output()
{
	if (fFreezeCounter)
	{
		fFreezeCounter-=1;
		GetCurrentCamera()->GetBrain()->SetFlags(plCameraBrain1::kCutPOAOnce);
		GetCurrentCamera()->GetBrain()->SetFlags(plCameraBrain1::kCutPosOnce);
		return;
	}
	if (fFadeCounter)
	{
		fFadeCounter-=1;
		if (fFadeCounter == 0 && fFirstPersonOverride == nil)
		{
			plEnableMsg* pMsg = TRACKED_NEW plEnableMsg;
			pMsg->SetSender(GetKey());
			pMsg->SetCmd(plEnableMsg::kEnable);
			pMsg->AddType(plEnableMsg::kDrawable);
			pMsg->SetBCastFlag(plMessage::kPropagateToModifiers);
			pMsg->AddReceiver(plNetClientMgr::GetInstance()->GetLocalPlayerKey());
			plgDispatch::MsgSend(pMsg);
		}
	}
	hsMatrix44 targetMatrix;
	hsMatrix44 inverse;
	if (GetCurrentCamera())
	{	
		fOutputPos = GetCurrentCamera()->GetTargetPos();
		fOutputPOA = GetCurrentCamera()->GetTargetPOA();
		AdjustForInput();
	}
	else
	{
		return;
	}
	// construct output matrix
	hsVector3 abUp(0,0,1);
	hsVector3 view(fOutputPos - fOutputPOA);
	view.Normalize();
	// Now passing in Up for up parameter to MakeCamera. Negates sense of up. mf_flip_up - mf
	hsVector3 up = (view % abUp) % view;
	if (GetCurrentCamera()->IsAnimated())
		up = -GetCurrentCamera()->GetTarget()->GetCoordinateInterface()->GetLocalToWorld().GetAxis(hsMatrix44::kView);
	
	fOutputUp = up;
	targetMatrix.MakeCamera(&fOutputPos,&fOutputPOA, &up);
	targetMatrix.GetInverse(&inverse);
	fPipe->SetWorldToCamera( targetMatrix, inverse );
	if (HasFlags(kSetFOV)) // are we changing the field of view?
	{
		ClearFlags(kSetFOV);
		fPipe->SetFOV(fFOVw,fFOVh);
		fPipe->RefreshMatrices();
		if (foutLog)
		{
			fprintf(foutLog, "****************************************************************\n");
			fprintf(foutLog, "FOV changed to %f %f\n",fFOVh, fFOVw);
			fprintf(foutLog, "****************************************************************\n");
		}	

	}
/*	if (foutLog)
	{
		fprintf(foutLog, "output pos %f %f %f\n", fOutputPos.fX,fOutputPos.fY,fOutputPos.fZ);
		fprintf(foutLog, "output poa %f %f %f\n", fOutputPOA.fX,fOutputPOA.fY,fOutputPOA.fZ);
		fprintf(foutLog, "\n");
	}	*/
}

void plVirtualCam1::Init()
{
	plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plCameraMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plMouseEventMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plIfaceFadeAvatarMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plPlayerPageMsg::Index(), GetKey());

	// register for control messages
	plCmdIfaceModMsg* pModMsg = TRACKED_NEW plCmdIfaceModMsg;
	pModMsg->SetBCastFlag(plMessage::kBCastByExactType);
	pModMsg->SetSender(GetKey());
	pModMsg->SetCmd(plCmdIfaceModMsg::kAdd);
	plgDispatch::MsgSend(pModMsg);
}

//
// toggle between player-enforced 1st person mode and back
//
void plVirtualCam1::FirstPersonOverride()
{
	if (!HasFlags(kRender))
		return;
	
	if (fFirstPersonOverride)
	{
		fFirstPersonOverride->Pop();
		GetCurrentStackCamera()->Push(!HasFlags(kAvatarWalking));
		GetCurrentStackCamera()->GetBrain()->SetFlags(plCameraBrain1::kCutPosOnce);
		GetCurrentStackCamera()->GetBrain()->SetFlags(plCameraBrain1::kCutPOAOnce);
		fFirstPersonOverride = nil;
#ifdef STATUS_LOG
		camLog->AddLineF("Built-In First Person Camera Disabled");
#endif
		SetFOV(GetCurrentStackCamera()->GetFOVw(), GetCurrentStackCamera()->GetFOVh(), GetCurrentStackCamera()); 
		GetCurrentStackCamera()->Push(!HasFlags(kAvatarWalking));
		plAvatarInputInterface::GetInstance()->CameraInThirdPerson(true);
		FreezeOutput(2);	
		UnFadeAvatarIn(1);
		fRetainedFY = fY;
		fX = fY = 0.5f;
	}
	else
	if (HasFlags(kFirstPersonEnabled))
	{
//		plCameraBrain1* pBrain = nil;
//		if (GetCurrentStackCamera() && GetCurrentStackCamera()->GetBrain())
//		{
//			pBrain = plCameraBrain1_FirstPerson::ConvertNoRef(GetCurrentStackCamera()->GetBrain());
//			if (pBrain) // already in 1st person mode, don't use override
//				return;
//		}
		plUoid U(kDefaultCameraMod1_KEY);
		plKey pKey = hsgResMgr::ResMgr()->FindKey(U);
		if (pKey)
		{
			fFirstPersonOverride = (plCameraModifier1*)pKey->GetObjectPtr();
			GetCurrentStackCamera()->Pop();
			fFirstPersonOverride->Push(!HasFlags(kAvatarWalking));
			SetFOV(fFirstPersonOverride->GetFOVw(), fFirstPersonOverride->GetFOVh(), fFirstPersonOverride); 
			plAvatarInputInterface::GetInstance()->CameraInThirdPerson(false);
			// no need to keep transitioning if we are currently...
			if (fTransPos == POS_TRANS_FOLLOW)
				FinishTransition();
			fY = fRetainedFY;
		

#ifdef STATUS_LOG
			camLog->AddLineF("Built-In First Person Camera Enabled");
#endif
		}
	}
}

hsBool plVirtualCam1::MsgReceive(plMessage* msg)
{
	plPlayerPageMsg* pPMsg = plPlayerPageMsg::ConvertNoRef(msg);
	if (pPMsg)
	{
		if (!pPMsg->fUnload && pPMsg->fPlayer == plNetClientMgr::GetInstance()->GetLocalPlayerKey())
		{
			/*if (HasFlags(kRegisteredForBehaviors))
				return true;*/ // not reliable anymore since we have a dummy avatar in the startup age
			plSceneObject* avSO = plSceneObject::ConvertNoRef(plNetClientMgr::GetInstance()->GetLocalPlayer());
			if (avSO)
			{

				plArmatureMod* avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
				if (avMod)
				{	
					avMod->RegisterForBehaviorNotify(GetKey());
					// SetFlags(kRegisteredForBehaviors); // not reliable anymore since we have a dummy avatar in the startup age		
				}
			}
		}
		return true;
	}
	plAvatarBehaviorNotifyMsg *behNotifymsg = plAvatarBehaviorNotifyMsg::ConvertNoRef(msg);
	if (behNotifymsg)
	{
		if (behNotifymsg->fType == plHBehavior::kBehaviorTypeLinkIn && behNotifymsg->state == true)
		{
			if (!HasFlags(kScriptsForced3rd) && !HasFlags(kScriptsDisabled1st))
				SetFlags(kFirstPersonEnabled);
			
			if (HasFlags(kFirstPersonUserSelected))
			{
				ClearFlags(kFirstPersonAtLinkOut);
				ClearFlags(kScriptsForced3rd);
				if (!HasFlags(kScriptsForced3rd))
				{	
					FirstPersonOverride();
					//SetFlags(kJustLinkedIn);
				}
				else
					plAvatarInputInterface::GetInstance()->CameraInThirdPerson(true);
			}
			else
			if (fFirstPersonOverride == nil)
			{
				plAvatarInputInterface::GetInstance()->CameraInThirdPerson(true);
			}
		}
		else
		if (behNotifymsg->fType == plHBehavior::kBehaviorTypeLinkOut && behNotifymsg->state == true)
		{
			ClearFlags(kFirstPersonEnabled);
			if (fFirstPersonOverride || HasFlags(kResponderForced3rd))
				SetFlags(kFirstPersonAtLinkOut);
		}
		return true;
	}
	plControlEventMsg* pCtrlMsg = plControlEventMsg::ConvertNoRef(msg);
	if (pCtrlMsg)
	{
		SetMovementFlag(pCtrlMsg->GetControlCode(), pCtrlMsg->ControlActivated());
		
		if (pCtrlMsg->GetControlCode() == S_SET_FREELOOK && pCtrlMsg->ControlActivated())
			ClearFlags(kUnPanCamera);

		if (pCtrlMsg->GetControlCode() == B_CONTROL_MOVE_FORWARD ||
			pCtrlMsg->GetControlCode() == B_CONTROL_MOVE_BACKWARD ||
			pCtrlMsg->GetControlCode() == B_CONTROL_ROTATE_RIGHT ||
			pCtrlMsg->GetControlCode() == B_CONTROL_ROTATE_LEFT )
		{
			if (!HasMovementFlag(S_SET_FREELOOK))
				StartUnPan();
		}

		if (pCtrlMsg->GetControlCode() == B_CONTROL_MOVE_FORWARD || pCtrlMsg->GetControlCode() == B_CONTROL_MOVE_BACKWARD)
		{
			fFlags.SetBit(kAvatarWalking, pCtrlMsg->ControlActivated());	
		}
		else
		if (pCtrlMsg->GetControlCode() == S_SET_FIRST_PERSON_MODE && pCtrlMsg->ControlActivated())
		{
			if (HasFlags(kFirstPersonEnabled))
			{
				if (HasFlags(kFirstPersonUserSelected))
					ClearFlags(kFirstPersonUserSelected);
				else
					SetFlags(kFirstPersonUserSelected);
			}
			FirstPersonOverride();	
			return true;
		}
		else
		if (pCtrlMsg->GetControlCode() == B_CAMERA_RECENTER && pCtrlMsg->ControlActivated())
		{
			fX = fY = fRetainedFY = 0.5f;
		}
		if (pCtrlMsg->GetControlCode() == B_TOGGLE_DRIVE_MODE && pCtrlMsg->ControlActivated())
			Drive();
		else
		{	
			if (fPythonOverride)
				fPythonOverride->MsgReceive(msg);
			else
			if (fFirstPersonOverride)
				fFirstPersonOverride->MsgReceive(msg);
			GetCurrentStackCamera()->MsgReceive(msg);	
		}
		return true;
	}
	plMouseEventMsg* pMouseMsg = plMouseEventMsg::ConvertNoRef(msg);
	if( pMouseMsg )
	{
		if (!HasFlags(kFalling))
		{
			hsScalar dX = pMouseMsg->GetDX();
			hsScalar dY = pMouseMsg->GetDY();
			if (plMouseDevice::GetInverted())
			{
				dX *= -1.f;
				dY *= -1.f;
			}
			if (HasMovementFlag(S_SET_FREELOOK))
			{
				if (pMouseMsg->GetDX() < 0.4 && pMouseMsg->GetDX() > -0.4)
				{	
					fX -= dX;
				}
				if (pMouseMsg->GetDY() < 0.4 && pMouseMsg->GetDY() > -0.4)
				{	
					fY -= dY;
				}
			}
			if ((HasMovementFlag(B_CONTROL_CAMERA_WALK_PAN)) &&
				(fFirstPersonOverride || WalkPan3rdPerson == true)) 
			{
				if (pMouseMsg->GetDY() < 0.4 && pMouseMsg->GetDY() > -0.4)
				{	
					fY -= dY;
				}
			}
		}
		fDriveCamera->MsgReceive(msg);	
		return true;
	}
	plWarpMsg* pWarpMsg = plWarpMsg::ConvertNoRef(msg);
	if (pWarpMsg)
	{
		SetCutNextTrans();
		return true;
	}

	plCameraMsg* pCam = plCameraMsg::ConvertNoRef(msg);
	if (pCam)
	{	
		if (pCam->Cmd(plCameraMsg::kResetPanning))
		{
			fX = fY = fRetainedFY = 0.5f;
			return true;
		}
		else
		if (pCam->Cmd(plCameraMsg::kUpdateCameras))
		{
			IUpdate();
			return true;
		}
		else
		if (pCam->Cmd(plCameraMsg::kResetOnEnter))
		{
			// don't send this message to the virtual camera!
			Reset(false);
			// this only happens when the player links into the world
			return true;
		}
		else
		if (pCam->Cmd(plCameraMsg::kResetOnExit))
		{
			/*
			Kind of an ugly hack, but it works.  The avatar is being loaded/unloaded when
			the player enters/leaves the age, and the following Reset starts a camera transition
			which may reference the avatar's sceneobject and crash the client when the next
			update happens.  With the kCutNextTrans flag set it doesn't bother with the
			transition so there isn't any reading of invalid memory.
			*/
			SetFlags(kCutNextTrans);

			// don't send this message to the virtual camera!
			Reset(false);
			// this only happens when the player links out of the world
			return true;
		}
		else
		if (pCam->Cmd(plCameraMsg::kCreateNewDefaultCam))
		{
			if (pCam->GetSubject() == plNetClientMgr::GetInstance()->GetLocalPlayer())
				CreateDefaultCamera(pCam->GetSubject());
			return true;
		}
		else
		if (pCam->Cmd(plCameraMsg::kPythonOverridePop))
		{
			if (pCam->GetTriggerer() && pCam->GetTriggerer() != plNetClientApp::GetInstance()->GetLocalPlayerKey())
				return true;
			{
				if (fPythonOverride)
				{	
					fPythonOverride->Pop();
					FinishTransition();
					if (plCameraBrain1_FirstPerson::ConvertNoRef(fPythonOverride->GetBrain()) &&
						!fFirstPersonOverride)
					{
						FreezeOutput(2);	
						UnFadeAvatarIn(1);
					}
					GetCurrentStackCamera()->GetBrain()->SetFlags(plCameraBrain1::kCutPosOnce);
					GetCurrentStackCamera()->GetBrain()->SetFlags(plCameraBrain1::kCutPOAOnce);
					fX = fY = 0.5f;
				}
				fPythonOverride = nil;
				SetFlags(kFirstPersonEnabled);
#ifdef STATUS_LOG
				camLog->AddLineF("Override Python Camera Disabled");
#endif
				if (fFirstPersonOverride)
				{
					SetFOV(fFirstPersonOverride->GetFOVw(), fFirstPersonOverride->GetFOVh(), fFirstPersonOverride);
					fFirstPersonOverride->Push(!HasFlags(kAvatarWalking));
				}
				else
				{
					SetFOV(GetCurrentStackCamera()->GetFOVw(), GetCurrentStackCamera()->GetFOVh(), GetCurrentStackCamera()); 
					GetCurrentStackCamera()->Push(!HasFlags(kAvatarWalking));
				}
				
				StartInterpPanLimits();
				if (fFirstPersonOverride)
					plAvatarInputInterface::GetInstance()->CameraInThirdPerson(false);
				
				if (foutLog)
				{
					fprintf(foutLog, "********************************************\n");
					fprintf(foutLog, "popped python camera\n");
					fprintf(foutLog, "********************************************\n");
				}	

			}
		}
		else
		if (pCam->Cmd(plCameraMsg::kPythonOverridePush) || pCam->Cmd(plCameraMsg::kPythonOverridePushCut))
		{
			if (pCam->GetTriggerer() && pCam->GetTriggerer() != plNetClientApp::GetInstance()->GetLocalPlayerKey())
				return true;
			{
				plCameraModifier1* pCamMod = plCameraModifier1::ConvertNoRef(pCam->GetNewCam()->GetObjectPtr());
				if (pCamMod)
				{
					
					if (fPythonOverride)
					{
						fPythonOverride->Pop();
						if (plCameraBrain1_FirstPerson::ConvertNoRef(fPythonOverride->GetBrain()))
						{
							FreezeOutput(2);	
							UnFadeAvatarIn(1);
						}
						
#ifdef STATUS_LOG
						camLog->AddLineF("Override Python Camera Popped because new one coming in");
#endif
					}
					fPythonOverride = pCamMod;
#ifdef STATUS_LOG
					camLog->AddLineF("Override Python Camera Pushing onto stack");
#endif
					if (foutLog)
					{
						fprintf(foutLog, "********************************************\n");
						fprintf(foutLog, "changed to new camera\n");
						fprintf(foutLog, "********************************************\n");
					}	
					if (fFirstPersonOverride)
						plAvatarInputInterface::GetInstance()->CameraInThirdPerson(true);
				
					fPythonOverride->Push(!HasFlags(kAvatarWalking));
					
					CamTrans* pTrans = TRACKED_NEW CamTrans(fPythonOverride->GetKey());
					if (pCam->Cmd(plCameraMsg::kPythonOverridePushCut))
						pTrans->fCutPOA = pTrans->fCutPos = true; 
					StartTransition(pTrans);
					delete(pTrans);
					SetFOV(fPythonOverride->GetFOVw(), fPythonOverride->GetFOVh(), fPythonOverride);
					ClearFlags(kFirstPersonEnabled);
				}
			}
		}
		else
		if ( pCam->Cmd(plCameraMsg::kPythonSetFirstPersonOverrideEnable))
		{
			if (!pCam->HasBCastFlag(plMessage::kNetNonLocal))
			{
				// make sure it was locally sent
				if (pCam->GetActivated())
				{
					SetFlags(kFirstPersonEnabled);
					ClearFlags(kScriptsDisabled1st);
					if (HasFlags(kScriptsForced3rd))
					{
						ClearFlags(kScriptsForced3rd);
						FirstPersonOverride();
					}
				}
				else
				{
					ClearFlags(kFirstPersonEnabled);
					SetFlags(kScriptsDisabled1st);
				}
			}
		}
		else
		if ( pCam->Cmd(plCameraMsg::kPythonUndoFirstPerson))
		{
			if (!pCam->HasBCastFlag(plMessage::kNetNonLocal))
			{
				// make sure it was locally sent
				if (HasFlags(kFirstPersonAtLinkOut))
				{
					SetFlags(kScriptsForced3rd);
				}
				else
				if (fFirstPersonOverride)
				{
					SetFlags(kScriptsForced3rd);
					FirstPersonOverride();
#ifdef STATUS_LOG
					camLog->AddLineF("Forcing 3rd Person from scripts");
#endif
					SetFOV(GetCurrentStackCamera()->GetFOVw(), GetCurrentStackCamera()->GetFOVh(), GetCurrentStackCamera()); 
				}
			}
		}
		else
		if ( pCam->Cmd(plCameraMsg::kResponderSetThirdPerson))
		{
			if (plVirtualCam1::StayInFirstPersonForever)
				return true;
			
			if (HasFlags(kJustLinkedIn))
			{
				ClearFlags(kJustLinkedIn);
				plCameraTargetFadeMsg* pMsg = TRACKED_NEW plCameraTargetFadeMsg;
				pMsg->SetFadeOut(true);
				pMsg->SetSubjectKey(plNetClientMgr::GetInstance()->GetLocalPlayerKey());
				pMsg->SetBCastFlag(plMessage::kBCastByExactType);
				pMsg->SetBCastFlag(plMessage::kNetPropagate, FALSE);
				pMsg->AddReceiver(plNetClientMgr::GetInstance()->GetLocalPlayerKey());
				plgDispatch::MsgSend(pMsg);
				return true;
			}
			if (HasFlags(kScriptsForced3rd))
				return true;
			if (fFirstPersonOverride)
			{
				SetFlags(kResponderForced3rd);
				FirstPersonOverride();
#ifdef STATUS_LOG
				camLog->AddLineF("Forcing 3rd Person from code");
#endif
				SetFOV(GetCurrentStackCamera()->GetFOVw(), GetCurrentStackCamera()->GetFOVh(), GetCurrentStackCamera());
			}
			ClearFlags(kFirstPersonEnabled);
#ifdef STATUS_LOG
			camLog->AddLineF("1st person override disabled");
#endif
		}
		else
		if ( pCam->Cmd(plCameraMsg::kResponderUndoThirdPerson))
		{
			if (HasFlags(kScriptsForced3rd) || HasFlags(kScriptsDisabled1st))
				return true;
			SetFlags(kFirstPersonEnabled);
#ifdef STATUS_LOG
			camLog->AddLineF("1st person override enabled");
#endif

			if (HasFlags(kResponderForced3rd))
			{
				FirstPersonOverride();
				ClearFlags(kResponderForced3rd);
#ifdef STATUS_LOG
				camLog->AddLineF("Restoring 1st Person from code");
#endif
			}
			
		}
		else
		if (pCam->Cmd(plCameraMsg::kRegionPushCamera))
		{
			if (HasFlags(kRegionIgnore))
				return true;
			if (pCam->GetTriggerer() && pCam->GetTriggerer() != plNetClientApp::GetInstance()->GetLocalPlayerKey())
				return true;
			{
				hsBool bDef = pCam->Cmd(plCameraMsg::kSetAsPrimary);
				plKey pCamKey = pCam->GetNewCam();
				if (pCamKey)
				{
					plCameraModifier1* pCamMod = plCameraModifier1::ConvertNoRef(pCamKey->GetObjectPtr());
					if (!pCamMod)
					{
						plSceneObject* pObj = plSceneObject::ConvertNoRef( pCamKey->GetObjectPtr() );
						if (!pObj)
							return true;
						for (int i = 0; i < pObj->GetNumModifiers(); i++)
						{
							plKey pModKey = pObj->GetModifier(i)->GetKey();
							pCamMod = plCameraModifier1::ConvertNoRef( pModKey->GetObjectPtr() );
							if ( pCamMod )
								break;
						}
					}
					if (!pCamMod)
						return true;
					if (pCam->Cmd(plCameraMsg::kEntering) || pCam->Cmd(plCameraMsg::kResponderTrigger))
						PushCamera(pCamMod, bDef);
					else
						PopCamera(pCamMod);
					if (pCam->Cmd(plCameraMsg::kCut))
						SetFlags(kCutNextTrans);
				}
			}
		}
	}
	plGenRefMsg* pRefMsg = plGenRefMsg::ConvertNoRef(msg);
	if (pRefMsg )
	{
		if( pRefMsg->GetContext() & (plRefMsg::kOnDestroy | plRefMsg::kOnRemove) )
		{
			// unfortunately we cannot rely on the ref message to point at a valid camera
			// (the pointer will match our stack, but it might not be pointing at a valid
			// chunk of memory). Therefore, we cannot use the ConvertNoRef() call, and we
			// cannot call PopCamera. We simply have to settle for removing it from our
			// array. Since this message indicates it was destroyed anyway, this should be
			// ok.
			plCameraModifier1* pMod = (plCameraModifier1*)(pRefMsg->GetRef());
			// we go in reverse so that removes don't mess up our index
			for (int i = fCameraStack.Count() - 1; i >= 0; --i)
			{
				if (fCameraStack[i] == pMod)
					fCameraStack.Remove(i);
			}
		}
		return true;
	}
	return hsKeyedObject::MsgReceive(msg);
}

void plVirtualCam1::CreateDefaultCamera(plSceneObject* subject)
{
	// If a default cam already exists, we just want to replace the subject (unless it's the same)
	plUoid U(kDefaultCameraMod1_KEY);
	plKey pKey = hsgResMgr::ResMgr()->FindKey(U);
	if (pKey)
	{
		plCameraModifier1* mod = plCameraModifier1::ConvertNoRef(pKey->GetObjectPtr());
		if (mod->GetSubject() == subject)
			return;

		plGenRefMsg* msg = TRACKED_NEW plGenRefMsg(mod->GetKey(), plRefMsg::kOnReplace, 0, plCameraBrain1::kSubject );
		msg->SetOldRef(mod->GetSubject());
		hsgResMgr::ResMgr()->AddViaNotify(subject->GetKey(), msg, plRefFlags::kPassiveRef);
	}
	else
	{
		plCameraModifier1* pMod = TRACKED_NEW plCameraModifier1;
		plCameraBrain1_FirstPerson* pBrain = TRACKED_NEW plCameraBrain1_FirstPerson(pMod);
		pMod->RegisterAs( kDefaultCameraMod1_KEY );
		//pBrain->SetSubject(subject);
		plGenRefMsg* msg = TRACKED_NEW plGenRefMsg(pMod->GetKey(), plRefMsg::kOnCreate, 0, plCameraBrain1::kSubject ); // SceneObject
		hsgResMgr::ResMgr()->AddViaNotify(subject->GetKey(), msg, plRefFlags::kPassiveRef);
		
		plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), pMod->GetKey());
		plgDispatch::Dispatch()->RegisterForExactType(plCameraMsg::Index(), pMod->GetKey());
		
		pMod->SetFOVw(90.0f);
		pMod->SetFOVh(66.7f);
		// set up the brain and to be first-person 
		hsVector3 pt(0,0.0f,5.5);
		pBrain->SetOffset(pt);
		pt.Set(0,-10,5.5);
		pBrain->SetPOAOffset(pt);
		pBrain->SetZPanLimit(0.872f); //radians, == 50 degrees as Decreed By Rand!
		pBrain->SetXPanLimit(0.872f);
		pBrain->SetFlags(plCameraBrain1::kCutPOA);
		pBrain->SetFlags(plCameraBrain1::kCutPos);
		PushCamera(pMod);
	}
	// now deal with the third person camera
	// If a default 3rd person cam already exists, we just want to replace the subject (unless it's the same)
	plUoid Ux(kBuiltIn3rdPersonCamera_KEY);
	plKey pKeyx = hsgResMgr::ResMgr()->FindKey(Ux);
	if (pKeyx)
	{
		plCameraModifier1* mod = plCameraModifier1::ConvertNoRef(pKeyx->GetObjectPtr());
		if (mod->GetSubject() == subject)
			return;

		plGenRefMsg* msg = TRACKED_NEW plGenRefMsg(mod->GetKey(), plRefMsg::kOnReplace, 0, plCameraBrain1::kSubject );
		msg->SetOldRef(mod->GetSubject());
		hsgResMgr::ResMgr()->AddViaNotify(subject->GetKey(), msg, plRefFlags::kPassiveRef);
	}
	else
	{
		plCameraModifier1* pModx = TRACKED_NEW plCameraModifier1;
		plCameraBrain1_Avatar* pBrainx = TRACKED_NEW plCameraBrain1_Avatar(pModx);
		pModx->RegisterAs( kBuiltIn3rdPersonCamera_KEY );
		plGenRefMsg* msgx = TRACKED_NEW plGenRefMsg(pModx->GetKey(), plRefMsg::kOnCreate, 0, plCameraBrain1::kSubject ); // SceneObject
		hsgResMgr::ResMgr()->AddViaNotify(subject->GetKey(), msgx, plRefFlags::kPassiveRef);

		plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), pModx->GetKey());
		plgDispatch::Dispatch()->RegisterForExactType(plCameraMsg::Index(), pModx->GetKey());

		pModx->SetFOVw(90.0f);
		pModx->SetFOVh(66.7f);
		// set up the brain and to be third-person 
		hsVector3 ptx(0, 15, 10);
		pBrainx->SetOffset(ptx);
		ptx.Set(0,0,5.5);
		pBrainx->SetPOAOffset(ptx);
		pBrainx->SetZPanLimit(0.872f);
		pBrainx->SetXPanLimit(0.872f);
		pBrainx->SetVelocity(20.0f);
		pBrainx->SetDecel(10.0f);
		pBrainx->SetAccel(5.0f);
		pBrainx->SetFlags(plCameraBrain1::kCutPOA);
		pBrainx->SetFlags(plCameraBrain1::kMaintainLOS);
		PushCamera(pModx);
		fThirdPersonCam = pModx;
	}
}

void plVirtualCam1::AddCameraToStack(plCameraModifier1* pCam)
{
	fCameraStack.Append(pCam);
	if (pCam->GetBrain())
	{
		if (HasMovementFlag(B_CONTROL_CAMERA_WALK_PAN))
			pCam->GetBrain()->SetMovementFlag(B_CONTROL_CAMERA_WALK_PAN);
	}

	if (pCam->GetKey())
		hsgResMgr::ResMgr()->AddViaNotify(pCam->GetKey(), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefCamera), plRefFlags::kPassiveRef);
}

void plVirtualCam1::PushCamera(plCameraModifier1* pCam, hsBool bDefault)
{
	// pushing the same camera, folks?
	if (pCam == GetCurrentStackCamera())
	{
		AddCameraToStack(pCam);
		return;
	}
	// make sure that we don't keep adding the default camera if we're already in it
	if (bDefault && pCam == GetCurrentStackCamera())
		return;
	
	// look up whatever transition we might have specified
	CamTrans* pTrans = nil;
	if (pCam->GetNumTrans())
	{
		for (int i = 0; i < pCam->GetNumTrans(); i++)
		{
			if (pCam->GetTrans(i)->fTransTo == GetCurrentStackCamera()->GetKey())
			{	// if it is specifically for this camera, this is the one to use	
				pTrans = pCam->GetTrans(i);
				break;
			}
			else // if it's generic, assign it but keep looking for a specific one...
			if (pCam->GetTrans(i)->fTransTo == nil)
				pTrans = pCam->GetTrans(i);
		}
	}
	// bail out if we are supposed to be ignoring this camera
	if (pTrans && pTrans->fIgnore)
		return;

	// lose the drive camera if that's where we are at
	if (GetCurrentStackCamera() == fDriveCamera)
		PopCamera(fDriveCamera);

	if (plCameraBrain1_Drive::ConvertNoRef(pCam->GetBrain()) ||
		!GetCurrentStackCamera())
	{
		// special camera mode (like drive) just add it
		if (GetCurrentStackCamera())
			GetCurrentStackCamera()->Pop();
		pCam->Push(!HasFlags(kAvatarWalking));
		AddCameraToStack(pCam);
		return;
	}
	
#ifdef STATUS_LOG
	IHandleCameraStatusLog(pCam, kPush);
#endif

	// are we mouse-looking?
	if (GetCurrentStackCamera() && GetCurrentStackCamera()->GetBrain() && GetCurrentStackCamera()->GetBrain()->HasMovementFlag(S_SET_FREELOOK))
		pCam->GetBrain()->SetMovementFlag(S_SET_FREELOOK);

	// do anything special upon activating this camera
	if (GetCurrentStackCamera())
	{
		// is the avatar faded out?
		if (GetCurrentStackCamera()->GetFaded() &&!fFirstPersonOverride)
		{
			if (!pCam->SetFaded(true))
			{
				// new camera doesn't support fading, fade him back in
				plCameraTargetFadeMsg* pMsg = TRACKED_NEW plCameraTargetFadeMsg;
				pMsg->SetFadeOut(false);
				pMsg->SetSubjectKey(GetCurrentStackCamera()->GetBrain()->GetSubject()->GetKey());
				pMsg->SetBCastFlag(plMessage::kBCastByExactType);
				plgDispatch::MsgSend(pMsg);
			}
		}
		else
		{
			// check the rare instance that maybe we have fallen down to the bottom of the stack and hit the 
			// built-in first person cam, and are now pushing something on top of it...
			plUoid U(kDefaultCameraMod1_KEY);
			plKey pKey = hsgResMgr::ResMgr()->FindKey(U);
			if ( (pKey && pKey == GetCurrentStackCamera()->GetKey()) ||
				 (plCameraBrain1_FirstPerson::ConvertNoRef(GetCurrentStackCamera()->GetBrain())) )
			{
				FreezeOutput(2);	
				UnFadeAvatarIn(1);
			}

		}
		GetCurrentStackCamera()->Pop();
	}
	pCam->Push(!HasFlags(kAvatarWalking));

	// handle transition between the cameras
	
	// if the player is warping, just cut
	plUoid U(kDefaultCameraMod1_KEY);
	plKey pDefKey = hsgResMgr::ResMgr()->FindKey(U);
	
	if (HasFlags(kCutNextTrans))
	{
		if (fTransPos == POS_TRANS_FOLLOW)
			FinishTransition();
		if (pCam->GetKey() != pDefKey)
			ClearFlags(kCutNextTrans);
		AddCameraToStack(pCam);
		if (pCam->GetBrain())
		{
			pCam->GetBrain()->SetFlags(plCameraBrain1::kCutPosOnce);
			pCam->GetBrain()->SetFlags(plCameraBrain1::kCutPOAOnce);
		}
		StartInterpPanLimits();
	}
	else
	if (!fPythonOverride)
	{
		// check to see if the new camera has a transition override for the current camera
		if (!pTrans)
		{
			// do a stock transition
			if (plCameraBrain1_Avatar::ConvertNoRef(GetCurrentStackCamera()->GetBrain()) ||
				plCameraBrain1_Avatar::ConvertNoRef(pCam->GetBrain()) )
			{
				// do a track transition here;
				fPrevCam = GetCurrentStackCamera();
				AddCameraToStack(pCam);
				pTrans = TRACKED_NEW CamTrans(pCam->GetKey());
				StartTransition(pTrans);
				delete(pTrans);
#ifdef STATUS_LOG
				camLog->AddLineF("Performing stock track transition between %s and %s",fPrevCam->GetKeyName(), pCam->GetKeyName());
#endif
			}
			else
			{
				// both fixed brains, cut between them
				AddCameraToStack(pCam);
				pTrans = TRACKED_NEW CamTrans(pCam->GetKey());
				pTrans->fCutPOA = true;
				pTrans->fCutPos = true;
				StartTransition(pTrans);
				delete(pTrans);
#ifdef STATUS_LOG
				camLog->AddLineF("Performing stock cut transition between %s and %s",fPrevCam->GetKeyName(), pCam->GetKeyName());
#endif
			}
		}
		else
		{
			// do the specified transition
			fPrevCam = GetCurrentStackCamera();
			AddCameraToStack(pCam);
			StartTransition(pTrans);
#ifdef STATUS_LOG
			camLog->AddLineF("Performing custom transition between %s and %s",fPrevCam->GetKeyName(), pCam->GetKeyName());
#endif

		}
	}
	else
	{
		// just push it on, since we have a python camera present.
		AddCameraToStack(pCam);
#ifdef STATUS_LOG
		camLog->AddLineF("No transition between %s and %s, python camera is currently displaying",fPrevCam->GetKeyName(), pCam->GetKeyName());
#endif
	}
	// make this the default camera if that's what we want...
	if (fCameraStack.Count() > 0 && bDefault)
	{	
		fCameraStack.SetCountAndZero(0);
		AddCameraToStack(pCam);
#ifdef STATUS_LOG	
		camLog->AddLineF("Camera %s is now the DEFAULT camera for this age", pCam->GetKeyName());
#endif	
	}
	SetFOV(GetCurrentStackCamera()->GetFOVw(), GetCurrentStackCamera()->GetFOVh(), GetCurrentStackCamera()); 
}

void plVirtualCam1::PopCamera(plCameraModifier1* pCam)
{
		// sanity / new default camera check
	if (fCameraStack.Count() <= 1)
		return;
	
	// is it the current camera AND the same camera we would otherwise switch to?
	if (pCam==GetCurrentStackCamera() && pCam == fCameraStack[fCameraStack.Count() - 2])
	{
		// pop but don't transition to a new camera
		// or do anything else:
		fCameraStack.Remove(fCameraStack.Count() - 1);
		return;
	}

	// are we mouse-looking?
	hsBool mLook = false;
	if (pCam->GetBrain() && pCam->GetBrain()->HasMovementFlag(S_SET_FREELOOK))
		mLook = true;

	// do anything special upon de-activating this camera
	pCam->Pop();
	
	if (plCameraBrain1_FirstPerson::ConvertNoRef(pCam->GetBrain()))
	{
		FreezeOutput(2);	
		UnFadeAvatarIn(1);
	}
	else
	if (plCameraBrain1_Drive::ConvertNoRef(pCam->GetBrain()) ||
		!GetCurrentStackCamera())
	{
		// special camera mode (like drive) just pop it
		fCameraStack.Remove(fCameraStack.Count() - 1);
		GetCurrentStackCamera()->Push(!HasFlags(kAvatarWalking));		
		return;
	}

	if (pCam == GetCurrentStackCamera())
	{
#ifdef STATUS_LOG
	IHandleCameraStatusLog(pCam, kPop);
#endif

		// pop and actually transition to a new camera
		fCameraStack.Remove(fCameraStack.Count() - 1);
		
		if (GetCurrentStackCamera())
		{	
			// update this camera
			GetCurrentStackCamera()->Push(!HasFlags(kAvatarWalking));


#ifdef STATUS_LOG
	IHandleCameraStatusLog(GetCurrentStackCamera(), kReplacement);
#endif
			
			if (mLook)
				GetCurrentStackCamera()->GetBrain()->SetMovementFlag(S_SET_FREELOOK);

			// is the avatar faded out?
			if (pCam->GetFaded())
			{
				if (!GetCurrentStackCamera()->SetFaded(true))
				{
					// new camera doesn't support fading, fade him back in
					plCameraTargetFadeMsg* pMsg = TRACKED_NEW plCameraTargetFadeMsg;
					pMsg->SetFadeOut(false);
					pMsg->SetSubjectKey(pCam->GetBrain()->GetSubject()->GetKey());
					pMsg->SetBCastFlag(plMessage::kBCastByExactType);
					plgDispatch::MsgSend(pMsg);
				}
			}


			// handle transition between the cameras
			// check to see if the new camera has a transition override for the current camera
			CamTrans* pTrans = nil;
			if (GetCurrentStackCamera()->GetNumTrans())
			{
				for (int i = 0; i < GetCurrentStackCamera()->GetNumTrans(); i++)
				{
					if (GetCurrentStackCamera()->GetTrans(i)->fTransTo == pCam->GetKey())
					{	
						pTrans = GetCurrentStackCamera()->GetTrans(i);
						break;
					}
					else
					if (GetCurrentStackCamera()->GetTrans(i)->fTransTo == nil)
						pTrans = GetCurrentStackCamera()->GetTrans(i);
				}
			}
			if (!pTrans)
			{
				// do a stock transition
				if (plCameraBrain1_Avatar::ConvertNoRef(GetCurrentStackCamera()->GetBrain()) ||
					plCameraBrain1_Avatar::ConvertNoRef(pCam->GetBrain()) )
				{
					// do a track transition here;
					fPrevCam = pCam;
					pTrans = TRACKED_NEW CamTrans(GetCurrentStackCamera()->GetKey());
					StartTransition(pTrans);
					delete(pTrans);
				}
				else
				{
					fPrevCam = pCam;
					pTrans = TRACKED_NEW CamTrans(GetCurrentStackCamera()->GetKey());
					pTrans->fCutPOA = true;
					pTrans->fCutPos = true;
					StartTransition(pTrans);
					delete(pTrans);
				}
			}
			else
			{
				// do the specified transition
				fPrevCam = pCam;
				StartTransition(pTrans);
			}
		}
	}
	else
	{
#ifdef STATUS_LOG
	IHandleCameraStatusLog(pCam, kBackgroundPop);
#endif
		// just remove this from the stack
		for (int i = 0; i < fCameraStack.Count(); i++)
		{
			if (fCameraStack[i] == pCam)
			{	
				fCameraStack.Remove(i);
				break;
			}
		}
	}
	if (!InTransition())
		SetFOV(GetCurrentStackCamera()->GetFOVw(), GetCurrentStackCamera()->GetFOVh(), GetCurrentStackCamera()); 
}

void plVirtualCam1::PopAll()
{
}

void plVirtualCam1::StartTransition(CamTrans* transition)
{
	
	if ((transition->fCutPos && transition->fCutPOA) || GetCurrentStackCamera()->IsAnimated() || alwaysCutForColin )
	{
		if (fTransPos == POS_TRANS_FOLLOW)
			FinishTransition();
		// we want to cut, set new camera to cut to current pos and FOV
		if (GetCurrentStackCamera()->GetBrain())
		{
			GetCurrentStackCamera()->GetBrain()->SetFlags(plCameraBrain1::kCutPOAOnce);
			GetCurrentStackCamera()->GetBrain()->SetFlags(plCameraBrain1::kCutPosOnce);
			SetFOV(GetCurrentStackCamera()->GetFOVw(), GetCurrentStackCamera()->GetFOVh(), GetCurrentStackCamera()); 
			fXPanLimit = GetCurrentStackCamera()->GetBrain()->GetXPanLimit();
			fZPanLimit = GetCurrentStackCamera()->GetBrain()->GetZPanLimit();
			StartInterpPanLimits();
		}
		
#ifdef STATUS_LOG
		camLog->AddLineF("Camera Cut Transition Completed");
#endif
		return;
	}

	if (fFirstPersonOverride)
	{
		FinishTransition();
		return;
	}

	plCameraModifier1* pCam = GetCurrentStackCamera();
	plCameraBrain1* pBrain = 0;

#ifdef STATUS_LOG
	if (fPrevCam->GetKey() && pCam->GetKey())
		camLog->AddLineF("Starting Camera Transition from %s to %s",fPrevCam->GetKeyName(), pCam->GetKeyName());
#endif
	
	if ( (fPythonOverride && plCameraBrain1_Avatar::ConvertNoRef(fPythonOverride->GetBrain())) ||
		 (plCameraBrain1_Avatar::ConvertNoRef(pCam->GetBrain()) && !fPythonOverride) )
	{	
		plCameraBrain1_Avatar* pAvBrain = TRACKED_NEW plCameraBrain1_Avatar;
		
		pAvBrain->SetOffset(((plCameraBrain1_Avatar*)pCam->GetBrain())->GetOffset());
		pAvBrain->SetPOAOffset(pCam->GetBrain()->GetPOAOffset());
		if (pCam->GetBrain()->HasFlag(plCameraBrain1::kMaintainLOS))
			pAvBrain->SetFlags(plCameraBrain1::kMaintainLOS);	
		
		if (((plCameraBrain1_Avatar*)pCam->GetBrain())->HasFlag(plCameraBrain1::kWorldspacePOA))
			pAvBrain->SetFlags(plCameraBrain1::kWorldspacePOA);
		if (((plCameraBrain1_Avatar*)pCam->GetBrain())->HasFlag(plCameraBrain1::kWorldspacePos))
			pAvBrain->SetFlags(plCameraBrain1::kWorldspacePos);

//		if (plCameraBrain1_Avatar::ConvertNoRef(fPrevCam->GetBrain()) && pCam->GetBrain()->GetPOAOffset() == fPrevCam->GetBrain()->GetPOAOffset())
//		{
//			pAvBrain->SetFlags(plCameraBrain1::kCutPOA);
//		}
		pAvBrain->SetSubject(pCam->GetBrain()->GetSubject());
		pBrain = pAvBrain;	
	}
	else
	{
		pBrain = TRACKED_NEW plCameraBrain1;
	}
	pBrain->SetFlags(plCameraBrain1::kIsTransitionCamera);
	
	// set up transition speeds
	pBrain->SetAccel(transition->fAccel);
	pBrain->SetDecel(transition->fDecel);
	pBrain->SetVelocity(transition->fVelocity);

//	if (!pBrain->HasFlag(plCameraBrain1::kCutPOA))
	if (0)
	{
		// see if the transition between POA's is going to swing the camera more than 90 degrees
		hsVector3 curVec(fPrevCam->GetTargetPos() - fPrevCam->GetTargetPOA());
		hsVector3 transVec(pCam->GetTargetPOA() - fPrevCam->GetTargetPOA());
		curVec.fZ = transVec.fZ = 0;
		transVec.Normalize();
		curVec.Normalize();
		hsScalar dot = curVec * transVec;
		if (dot <= 0.5f || transVec.MagnitudeSquared() != 0.0f) 
		{
			pBrain->SetPOAAccel(100);
			pBrain->SetPOADecel(100);
			pBrain->SetPOAVelocity(200);
#ifdef STATUS_LOG
		camLog->AddLineF("Congradulations you triggered the dont-swing-the-POA-more-than-90-degrees override\n");
		camLog->AddLineF("If you don't like this transition then you need to redesign the cameras involved\n");
#endif
			
		}
		else
		{
			pBrain->SetPOAAccel(transition->fPOAAccel);
			pBrain->SetPOADecel(transition->fPOADecel);
			pBrain->SetPOAVelocity(transition->fPOAVelocity);
		}
	}
	// make sure that the new camera is where it should be
	pCam->SetTargetPos(pCam->GetBrain()->GetGoal());
	pCam->SetTargetPOA(pCam->GetBrain()->GetPOAGoal());
	// and the transition is trying to go where it should
	pBrain->SetGoal(pCam->GetTargetPos());
	pBrain->SetPOAGoal(pCam->GetTargetPOA());

	// set transition camera parameters
	if (fTransPos != POS_TRANS_FOLLOW)
	{
		fTransitionCamera->SetTargetPos(fPrevCam->GetTargetPos());
		fTransitionCamera->SetTargetPOA(fPrevCam->GetTargetPOA());
		fTransitionCamera->SetTransform(fPrevCam->GetTargetPOA());
		if (fPrevCam->GetInSubworld())
		{
			fTransitionCamera->SetSubworldPos(fPrevCam->GetSubworldPos());
			fTransitionCamera->SetSubworldPOA(fPrevCam->GetSubworldPOA());
			fTransitionCamera->InSubworld(true);
		}
	}
	else
	// we're already in transition...
	{
		plCameraBrain1* pOldBrain = fTransitionCamera->GetBrain();
		// match speeds to the old brain so we don't stop mid-transition
		pBrain->SetCurrentCamSpeed(pOldBrain->GetCurrentCamSpeed());
		pBrain->SetCurrentViewSpeed(pOldBrain->GetCurrentViewSpeed());
		delete(pOldBrain);
		fTransitionCamera->SetBrain(nil);
#ifdef STATUS_LOG
		camLog->AddLineF("Stopping in-progress camera transition");
#endif
	}
	
	if (transition->fCutPos)
	{	
		pCam->GetBrain()->SetFlags(plCameraBrain1::kCutPosOnce);
		pBrain->SetFlags(plCameraBrain1::kCutPos);
	}
	if (transition->fCutPOA)
	{	
		pBrain->SetFlags(plCameraBrain1::kCutPOA);
		pCam->GetBrain()->SetFlags(plCameraBrain1::kCutPOAOnce);	
	}
	fTransitionCamera->SetBrain(pBrain);
	pBrain->SetCamera(fTransitionCamera);

	// deal with FOV - 
	hsScalar diffH = hsABS(pCam->GetFOVh() - fPrevCam->GetFOVh());
	if ( diffH )
	{
		double time = 0;
		hsVector3 dist;
		// figure out transition time
		if (transition->fCutPos)
			dist.Set(&(fTransitionCamera->GetTargetPOA() - pCam->GetTargetPOA()));
		else
			dist.Set(&(fTransitionCamera->GetTargetPos() - pCam->GetTargetPos()));
		
		time = (double)(dist.Magnitude() / pBrain->GetVelocity());
		
		// set up the transition camera to the current FOV
		fTransitionCamera->SetFOVh(GetFOVh(), false);
		fTransitionCamera->SetFOVw(GetFOVw(), false);
		fTransitionCamera->GetBrain()->SetFOVGoal(pCam->GetFOVh(), time);

	}
	StartInterpPanLimits();
	fTransPos = POS_TRANS_FOLLOW;
}

void plVirtualCam1::RunTransition()
{
	if (fTransPos != POS_TRANS_FOLLOW)
		return;
	
	plCameraModifier1* pToCam = fPythonOverride;
	if (!pToCam)
		pToCam = GetCurrentStackCamera();

	hsVector3 v1(fTransitionCamera->GetTargetPos() - pToCam->GetTargetPos());
	hsVector3 v2(fTransitionCamera->GetBrain()->GetPOAGoal() - fTransitionCamera->GetTargetPOA());
	
	if ( v1.MagnitudeSquared() <= 0.0001f && v2.MagnitudeSquared() <= 0.0001f &&
		!fTransitionCamera->GetBrain()->HasFlag(plCameraBrain1::kAnimateFOV))
	{	
		FinishTransition();
	}
	else
	{
		pToCam->Update();
		fTransitionCamera->GetBrain()->SetGoal(pToCam->GetTargetPos());
		fTransitionCamera->GetBrain()->SetPOAGoal(pToCam->GetBrain()->GetPOAGoal());
		// check for panic velocity
		plCameraBrain1* pBrain = pToCam->GetBrain();
		plCameraBrain1_Avatar* pAvBr = plCameraBrain1_Avatar::ConvertNoRef(pBrain);
		if (pAvBr)
		{
			hsScalar off = pAvBr->GetOffset().MagnitudeSquared();
			hsVector3 dist(pToCam->GetTargetPos() - fTransitionCamera->GetTargetPos());
			if (dist.MagnitudeSquared() > off)
				fTransitionCamera->GetBrain()->SetFlags(plCameraBrain1::kPanicVelocity);
			else
				fTransitionCamera->GetBrain()->ClearFlags(plCameraBrain1::kPanicVelocity);

		}

	}
}

void plVirtualCam1::FinishTransition()
{
	plCameraBrain1* pBrain = fTransitionCamera->GetBrain();
	delete(pBrain);
	fTransitionCamera->SetBrain(nil);
	
	fTransPos = POS_TRANS_OFF;
#ifdef STATUS_LOG
	camLog->AddLineF("Finished Camera Transition");
#endif

}


void plVirtualCam1::IHandleCameraStatusLog(plCameraModifier1* pMod, int action)
{
#ifdef STATUS_LOG

	if (!pMod->GetKey())
		return;
	camLog->AddLineF("..");
	plCameraBrain1* pBrain = pMod->GetBrain();
	switch(action)
	{
	case kPop:
		camLog->AddLineF("Popped Camera %s from top of stack", pMod->GetKeyName());
		break;
	case kBackgroundPop:
		camLog->AddLineF("Popped Camera %s from background", pMod->GetKeyName());
		break;
	case kPush:
		camLog->AddLineF("Pushed Camera %s", pMod->GetKeyName());
		break;
	case kReplacement:
		camLog->AddLineF("Camera %s replacing popped camera", pMod->GetKeyName());
		break;
	}
	if (pBrain)
	{
		if (plCameraBrain1_Circle::ConvertNoRef(pBrain))
		{	
			camLog->AddLineF("Brain type Circle");
		}
		else
		if (plCameraBrain1_Fixed::ConvertNoRef(pBrain))
		{
			camLog->AddLineF("Brain type Fixed");
			camLog->AddLineF("POAOffset %f %f %f", pBrain->GetPOAOffset().fX,pBrain->GetPOAOffset().fY,pBrain->GetPOAOffset().fZ); 
		}
		else
		if (plCameraBrain1_FirstPerson::ConvertNoRef(pBrain))
		{
			camLog->AddLineF("Brain type 1st Person");
		}
		else
		if (plCameraBrain1_Avatar::ConvertNoRef(pBrain))
		{
			camLog->AddLineF("Brain type 3rd Person");
		}
		camLog->AddLineF("FOV %f",pMod->GetFOVw());
		camLog->AddLineF("..");
	}
#endif
}
