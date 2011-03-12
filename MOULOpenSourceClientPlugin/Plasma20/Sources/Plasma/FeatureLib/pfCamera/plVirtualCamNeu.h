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

#ifndef plVirtualCam1_inc
#define plVirtualCam1_inc

#include "../pnKeyedObject/hsKeyedObject.h"
#include "hsMatrix44.h"
#include "hsBitVector.h"

class plPipeline;
class plCameraModifier1;
class plCameraBrain1;
class plSceneObject;
class plKey;
class hsGMaterial;
class plDrawableSpans;
class plCameraProxy;
class plSceneNode;
class plDebugInputInterface;
class plPlate;

#include "hsTemplates.h"

struct CamTrans;
struct hsColorRGBA;

#define POS_TRANS_OFF		0
#define POS_TRANS_FIXED		1
#define POS_TRANS_FOLLOW	2
#define POA_TRANS_OFF		3
#define POA_TRANS_FIXED		4
#define POA_TRANS_FOLLOW	5

class plVirtualCam1 : public hsKeyedObject
{

protected:
	

	void Output();
	void IUpdate();
	void INext();
		
public:
	enum flags
	{
		kSetFOV,
		kCutNextTrans,
		kRender,
		kRegionIgnore,
		kFirstPersonEnabled,
		kResponderForced3rd,
		kScriptsForced3rd,
		kScriptsDisabled1st,
		kAvatarWalking,
		kUnPanCamera,
		kInterpPanLimits,
		kFalling,
		//kRegisteredForBehaviors, // not reliable anymore since we have a dummy avatar in the startup age
		kFirstPersonAtLinkOut,
		kJustLinkedIn,
		kFirstPersonUserSelected,
	};
	
	enum action
	{
		kPush	= 0,
		kPop,
		kReplacement,
		kBackgroundPop,
		kRefCamera,
	};
	plVirtualCam1();
	virtual ~plVirtualCam1();

	CLASSNAME_REGISTER( plVirtualCam1 );
	GETINTERFACE_ANY( plVirtualCam1, hsKeyedObject );

	void SetPipeline(plPipeline* p); 
	void Init();

	virtual hsBool MsgReceive(plMessage* msg);
	static void SetFOV(hsScalar w, hsScalar h);
	static void SetFOV(hsScalar w, hsScalar h, plCameraModifier1* pCam);
	static void SetDepth(hsScalar h, hsScalar y);
	static hsScalar GetFOVw() { return fFOVw; }
	static hsScalar GetFOVh() { return fFOVh; }
	static hsScalar GetHither() { return fHither; }
	static hsScalar GetYon()	{ return fYon; }
	static void		SetOffset(float x, float y, float z);
	static void SetAspectRatio(float aspect) { fAspectRatio = aspect; }
	static float GetAspectRatio() { return fAspectRatio; }
	
	hsBool InTransition() { return fTransPos != POS_TRANS_OFF; }
	plCameraModifier1* GetCurrentCamera();
	plCameraModifier1* GetCurrentStackCamera();
	plCameraModifier1* GetTransitionCamera(){return fTransitionCamera;}
	hsBool Is1stPersonCamera();

	hsBool	HasMovementFlag(int f) { return fMoveFlags.IsBitSet(f); }
	void	SetMovementFlag(int f, hsBool on = true) { fMoveFlags.SetBit(f, on);} 
		
	hsPoint3 GetCameraPos() { return fOutputPos; }
	hsPoint3 GetCameraPOA() { return fOutputPOA; }
	hsVector3 GetCameraUp() { return fOutputUp; }
	void	SetCutNextTrans(); // used when player warps into a new camera region

	const hsMatrix44 GetCurrentMatrix() { return fMatrix; }
	static plVirtualCam1* Instance() { return fInstance; }
	
	int GetNumCameras() { return fCameraStack.Count(); }
	plCameraModifier1* GetCameraNumber(int camNumber); 
	void RebuildStack(const plKey& key);

	void SetFlags(int flag) { fFlags.SetBit(flag); }
	hsBool HasFlags(int flag) { return fFlags.IsBitSet(flag); }
	void ClearFlags(int flag) { fFlags.ClearBit(flag); }

	// console command stuff
	static void Next();
	static void Prev();
	static void Deactivate();
	void CameraRegions(hsBool b) { fFlags.SetBit(kRegionIgnore,b); }
	void LogFOV(hsBool b) { printFOV = b; }
	void Drive();
	void PushThirdPerson();
	
	static void AddMsgToLog(const char* msg);
	static hsBool IsCurrentCamera(const plCameraModifier1* mod);
	void ClearStack();

	void AddCameraLoaded(plSceneObject* pCam) { fCamerasLoaded.Append(pCam); }
	hsBool RestoreFromName(const char* name);
	void StartUnPan();
	// these are for console access
	static hsBool fUseAccelOverride, freeze, alwaysCutForColin, WalkPan3rdPerson,StayInFirstPersonForever;
	static hsScalar fDecel, fAccel, fVel;
	static hsScalar fFallTimerDelay;
	
private:

	void Reset(hsBool bRender);
	void PushCamera(plCameraModifier1* pCam, hsBool bDefault = false);
	void PopCamera(plCameraModifier1* pCam);
	void AddCameraToStack(plCameraModifier1* pCam);
	void PopAll();
	void CreateDefaultCamera(plSceneObject* subject);
	void StartTransition(CamTrans* transition);
	void RunTransition();
	void FinishTransition();
	void SetRender(hsBool render);
	void IHandleCameraStatusLog(plCameraModifier1* pMod, int action);
	void ICreatePlate();
	void FreezeOutput(int frames) { fFreezeCounter = frames; } // I hate this and I hate myself for doing it
	void UnFadeAvatarIn(int frames) { fFadeCounter = frames; } // ditto
	void FirstPersonOverride();
	
	void AdjustForInput();
	void UnPanIfNeeded();
	void StartInterpPanLimits();
	void InterpPanLimits();
	
	plPipeline*			fPipe;
	hsMatrix44			fMatrix;
	hsPoint3			fOutputPos;
	hsPoint3			fOutputPOA;
	hsVector3			fOutputUp;
	int					fTransPos;
	plDebugInputInterface*	fCameraDriveInterface;
	plPlate*			fEffectPlate;
	FILE*				foutLog;
	hsTArray<plCameraModifier1*>	fCameraStack;
	int					fFreezeCounter;
	int					fFadeCounter;
	hsBitVector			fFlags;
	hsTArray<plSceneObject*>	fCamerasLoaded;
	hsBitVector			fMoveFlags;
	hsScalar			fX;
	hsScalar			fY;
	hsScalar			fXPanLimit;
	hsScalar			fZPanLimit;
	hsScalar			fXPanLimitGoal;
	hsScalar			fZPanLimitGoal;
	hsScalar			fXUnPanRate;
	hsScalar			fZUnPanRate;
	hsScalar			fXPanInterpRate;
	hsScalar			fZPanInterpRate;
	double				fUnPanEndTime;
	double				fInterpPanLimitTime;
	hsScalar			fRetainedFY;
	
	// built-in cameras
	plCameraModifier1*	fDriveCamera; // for driving around 
	plCameraModifier1*	fTransitionCamera; // transitions between cameras placed in scenes
	plCameraModifier1*	fPythonOverride; // a special camera pushed by python
	plCameraModifier1*	fFirstPersonOverride; // the built-in first person camera
	plCameraModifier1*	fPrevCam; // the last camera we were displaying
	plCameraModifier1*	fThirdPersonCam; // built in third person cam for ccr's when they jump about

	static hsScalar	fFOVh, fFOVw;
	static hsScalar	fHither, fYon;
	static plVirtualCam1* fInstance;
	static hsBool printFOV;	
	static hsScalar fPanResponseTime;
	static float fAspectRatio;
	hsBool fForceCutOnce;

};


#endif plVirtualCam1_inc
