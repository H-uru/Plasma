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

#ifndef plCameraMsg_inc
#define plCameraMsg_inc

//
// camera message class
//
#include "../pnMessage/plMessage.h"
#include "hsBitVector.h"
#include "hsGeometry3.h"

class plSceneObject;
class plPipeline;
class hsStream;
class hsResMgr;

class plCameraConfig
{
public:

	plCameraConfig() : 
	fAccel(0),fDecel(0),
	fVel(0),fFPAccel(0),
	fFPDecel(0),fFPVel(0),
	fFOVw(0),fFOVh(0),fType(0),fWorldspace(false){fOffset.Set(0,0,0);}
	
	plCameraConfig(int flags) : 
	fAccel(0),fDecel(0),
	fVel(0),fFPAccel(0),
	fFPDecel(0),fFPVel(0),
	fFOVw(0),fFOVh(0),fType(0),fWorldspace(false) { fType |= flags;fOffset.Set(0,0,0);}


	enum
	{
		kOffset		= 0x0001,
		kSpeeds		= 0x0002,
		kFOV		= 0x0004,
	};

	int				fType;

	hsPoint3		fOffset;
	hsScalar		fAccel;
	hsScalar		fDecel;
	hsScalar		fVel;
	hsScalar		fFPAccel;
	hsScalar		fFPDecel;
	hsScalar		fFPVel;
	hsScalar		fFOVw, fFOVh;	
	hsBool			fWorldspace;

	void Read(hsStream* stream);
	void Write(hsStream* stream);

};

class plCameraTargetFadeMsg : public plMessage
{
protected:

	plKey			fSubject;
	hsBool			fFadeOut;

public:
	
	plKey	GetSubjectKey() { return fSubject; }
	
	void SetSubjectKey(const plKey &x) { fSubject = x; }
	hsBool FadeOut() { return fFadeOut; }
	void SetFadeOut(hsBool b) { fFadeOut = b; }

	
	plCameraTargetFadeMsg(){;}
	plCameraTargetFadeMsg(const plKey &s, 
					const plKey &r, 
					const double* t){;}
	
	CLASSNAME_REGISTER( plCameraTargetFadeMsg );
	GETINTERFACE_ANY( plCameraTargetFadeMsg, plMessage );

	void Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);

};

class plCameraMsg : public plMessage
{
protected:

	plKey			fNewCam;
	plKey			fTriggerer;
	double			fTransTime;
	plSceneObject*	fSubject;
	plPipeline*		fPipe;
	plCameraConfig	fConfig;

	hsBool			fActivated;

public:
	
	plKey	GetNewCam() { return fNewCam; }
	plKey	GetTriggerer() { return fTriggerer; }
	double	GetTransTime() { return fTransTime; }
	plSceneObject*	GetSubject() { return fSubject; }
	plPipeline*	GetPipeline() { return fPipe; }
	hsBool	 GetActivated() { return fActivated; }
	plCameraConfig* GetConfig() { return &fConfig; }

	void SetNewCam(const plKey &x) { fNewCam = x; }
	void SetTriggerer(const plKey &x) { fTriggerer = x; }
	void SetTransTime(double x) { fTransTime = x; }
	void SetSubject(plSceneObject* x) { fSubject = x; }
	void SetPipeline(plPipeline* x) { fPipe = x; }
	void SetActivated(hsBool x) { fActivated = x; }
		
	plCameraMsg();
	plCameraMsg(const plKey &s, 
					const plKey &r, 
					const double* t);
	
	CLASSNAME_REGISTER( plCameraMsg );
	GETINTERFACE_ANY( plCameraMsg, plMessage );

	enum ModCmds
	{
		kSetSubject = 0,
		kCameraMod,
		kSetAsPrimary,
		kTransitionTo,
		kPush,
		kPop,
//		kSetOffset,
//		kRegionOffset,
		kEntering,
//		kSetFirstPerson,
//		kRegionFirstPerson,
//		kRegionPush,
		kCut,
//		kModDestroy,
		kResetOnEnter,
		kResetOnExit,
		kChangeParams,
		kWorldspace,
		kCreateNewDefaultCam,
		kRegionPushCamera,
		kRegionPopCamera,
		kRegionPushPOA,
		kRegionPopPOA,
		kFollowLocalPlayer,
		kResponderTrigger,
		kSetFOV,
		kAddFOVKeyframe,
		kStartZoomIn,
		kStartZoomOut,
		kStopZoom,
		kSetAnimated,
		kPythonOverridePush,
		kPythonOverridePop,
		kPythonOverridePushCut,
		kPythonSetFirstPersonOverrideEnable,
		kPythonUndoFirstPerson,
		kUpdateCameras,
		kResponderSetThirdPerson,
		kResponderUndoThirdPerson,
		kNonPhysOn,
		kNonPhysOff,
		kResetPanning,
		kNumCmds
	};

	hsBitVector		fCmd;

	hsBool Cmd(int n) { return fCmd.IsBitSet(n); }
	void SetCmd(int n) { fCmd.SetBit(n); }
	void ClearCmd() { fCmd.Clear(); }
	void ClearCmd(int n) { fCmd.ClearBit(n); }
	

	// IO
	void Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);

};


class plIfaceFadeAvatarMsg : public plMessage
{
protected:

	plKey			fSubject;
	hsBool			fFadeOut;
	hsBool			fEnable, fDisable;
	
public:
	
	plKey	GetSubjectKey() { return fSubject; }
	
	void SetSubjectKey(const plKey &x) { fSubject = x; }
	hsBool FadeOut() { return fFadeOut; }
	void SetFadeOut(hsBool b) { fFadeOut = b; }
	void Enable() { fEnable = true; }
	void Disable() { fDisable = true; }
	hsBool GetEnable() { return fEnable; }
	hsBool GetDisable() { return fDisable; }

	plIfaceFadeAvatarMsg() : fEnable(false),fDisable(false){;}
	plIfaceFadeAvatarMsg(const plKey &s, 
					const plKey &r, 
					const double* t): fEnable(false),fDisable(false){;}
	
	CLASSNAME_REGISTER( plIfaceFadeAvatarMsg );
	GETINTERFACE_ANY( plIfaceFadeAvatarMsg, plMessage );

	void Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);

};

#endif // plCameraMsg_inc
