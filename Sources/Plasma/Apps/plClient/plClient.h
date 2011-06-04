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

#pragma once
#ifndef plClient_inc
#define plClient_inc


//#define NEW_CAMERA_CODE

#include "hsWindowHndl.h"
#include "hsBitVector.h"
#include "hsTemplates.h"
#include "hsUtils.h"
#include "hsStlUtils.h"
#include "../pnKeyedObject/hsKeyedObject.h"
#include "../pnKeyedObject/plUoid.h"
#include "../plScene/plRenderRequest.h"

class plSceneNode;
class plPipeline;
class hsG3DDeviceModeRecord;
class plInputManager;
class plInputController;
class plSceneObject;
class pfConsoleEngine;
class pfConsole;
class plAudioSystem;
class plVirtualCam1;
class plKey;
class plPageTreeMgr;
class plTransitionMgr;
class plLinkEffectsMgr;
class plOperationProgress;
class pfGameGUIMgr;
class pfKI;
class plAnimDebugList;
class plFontCache;
class plClientMsg;
class plLocation;
class plMovieMsg;
class plBinkPlayer;
class plPreloaderMsg;
class plNetCommAuthMsg;
class plAgeLoaded2Msg;


typedef void (*plMessagePumpProc)( void );

class plClient : public hsKeyedObject
{
protected:

	class plRoomRec
	{
		public:
			plSceneNode	*fNode;
			UInt32		fFlags;

			plRoomRec() { fNode = nil; fFlags = 0; }
			plRoomRec( plSceneNode *n, UInt32 f ) : fNode( n ), fFlags( f ) {}

			enum Flags
			{
				kHeld = 0x00000001
			};
	};

	hsBitVector				fFlags;

	plInputManager*			fInputManager;

	plPageTreeMgr*			fPageMgr;
	hsTArray<plRoomRec>		fRooms;
	plSceneNode*			fCurrentNode;

	plPipeline*				fPipeline;
	hsColorRGBA				fClearColor;
	plTransitionMgr			*fTransitionMgr;
	plLinkEffectsMgr		*fLinkEffectsMgr;
	plFontCache				*fFontCache;

	pfConsoleEngine*		fConsoleEngine;
	pfConsole*				fConsole;

	pfKI					*fKIGUIGlue;

	hsBool					fDone;
	hsBool					fWindowActive;

	hsWindowHndl			fWindowHndl;

	double					fLastProgressUpdate;
	plOperationProgress		*fProgressBar;

	pfGameGUIMgr			*fGameGUIMgr;

	virtual hsG3DDeviceModeRecord ILoadDevMode(const char* devModeFile);

	hsBool					IUpdate();
	hsBool					IDraw();
	hsBool					IDrawProgress();
	
	plVirtualCam1*			fNewCamera;

	static plClient*		fInstance;
	char *					fpAuxInitDir;
	static hsBool			fDelayMS;

	int						fClampCap;
	int						fQuality;

	hsBool					fQuitIntro;
	hsTArray<plBinkPlayer*>	fMovies;

	hsBool					fPatchGlobalAges;

	plMessagePumpProc		fMessagePumpProc;
	
#ifndef PLASMA_EXTERNAL_RELEASE
	bool					bPythonDebugConnected;
#endif

	hsTArray<plRenderRequest*>		fPreRenderRequests;
	hsTArray<plRenderRequest*>		fPostRenderRequests;

	bool fHoldLoadRequests;
	class LoadRequest
	{
	public:
		LoadRequest(const plLocation& loc, bool hold) { this->loc = loc; this->hold = hold; }
		plLocation loc;
		bool hold;
	};
	typedef std::list<LoadRequest*> LoadList;
	LoadList fLoadRooms;
	int fNumLoadingRooms;	// Number of rooms we're waiting for load callbacks on
	std::vector<plLocation> fRoomsLoading; // the locations we are currently in the middle of loading

	int fNumPostLoadMsgs;
	float fPostLoadMsgInc;
	
	void					ICompleteInit ();
	void					IOnAsyncInitComplete ();
	void					IHandlePreloaderMsg (plPreloaderMsg * msg);
	void					IHandleNetCommAuthMsg (plNetCommAuthMsg * msg);
	bool					IHandleAgeLoaded2Msg (plAgeLoaded2Msg * msg);

	hsBool					IFlushRenderRequests();
	void					IProcessPreRenderRequests();
	void					IProcessPostRenderRequests();
	void					IProcessRenderRequests(hsTArray<plRenderRequest*>& reqs);
	void					IAddRenderRequest(plRenderRequest* req);

	hsBool					IPlayIntroBink(const char* movieName, hsScalar endDelay, hsScalar posX, hsScalar posY, hsScalar scaleX, hsScalar scaleY, hsScalar volume = 1.0);
	hsBool					IHandleMovieMsg(plMovieMsg* mov);
	void					IKillMovies();
	void					IServiceMovies();

	void	IStartProgress( const char *title, hsScalar len );
	void	IIncProgress( hsScalar byHowMuch, const char *text );
	void	IStopProgress( void );

	static void IDispatchMsgReceiveCallback();
	static void	IReadKeyedObjCallback(plKey key);
	static void	IProgressMgrCallbackProc( plOperationProgress *progress );

	void	IPatchGlobalAgeFiles( void );

	int IFindRoomByLoc(const plLocation& loc);
	bool IIsRoomLoading(const plLocation& loc);
	void IQueueRoomLoad(const std::vector<plLocation>& locs, bool hold);
	void ILoadNextRoom();
	void IUnloadRooms(const std::vector<plLocation>& locs);
	void IRoomLoaded(plSceneNode* node, bool hold);
	void IRoomUnloaded(plSceneNode* node);
	void ISetGraphicsDefaults();
	
public:

	plClient();
	virtual ~plClient();

	CLASSNAME_REGISTER( plClient );
	GETINTERFACE_ANY( plClient, hsKeyedObject );

	static plClient*	GetInstance() { return fInstance; }
	static void			SetInstance(plClient* v) { fInstance=v; }
	
	virtual hsBool MsgReceive(plMessage* msg);
	
	hsBool		InitPipeline();

	void		InitInputs();

	void		InitDLLs();
	void		ShutdownDLLs();

	void		InitAuxInits();

	virtual hsBool StartInit();
	virtual hsBool Shutdown();
	virtual hsBool MainLoop();

	plClient&	SetDone(hsBool done) { fDone = done; return *this; }
	hsBool		GetDone() { return fDone; }

	// Set this to true to queue any room load requests that come in.  Set it to false to process them.
	void SetHoldLoadRequests(bool hold);

	enum
	{
		kFlagIniting,
		kFlagDBGDisableRender,
		kFlagDBGDisableRRequests,
		kFlagAsyncInitComplete,
		kFlagGlobalDataLoaded,
	};

	hsBool HasFlag(int f) const { return fFlags.IsBitSet(f); }
	void SetFlag(int f, hsBool on=true) { fFlags.SetBit(f, on); }

	virtual plClient& SetWindowHandle(hsWindowHndl hndl) { fWindowHndl=hndl; return *this; }
	hsWindowHndl	GetWindowHandle() { return fWindowHndl; }

	plInputManager*		GetInputManager() { return fInputManager; }

	plPipeline*		GetPipeline() { return fPipeline; }

	plSceneNode*	GetCurrentScene() { return fCurrentNode; }

	pfConsoleEngine	*GetConsoleEngine() { return fConsoleEngine; }

	void SetAuxInitDir(const char *p) { delete [] fpAuxInitDir; fpAuxInitDir = hsStrcpy(p); }

	static void EnableClientDelay() { plClient::fDelayMS = true; }

	// These are a hack to let the console fake a lesser capabile board and test out quality settings.
	// They should go away once we have this built into ClientSetup et.al.
	void SetClampCap(int c) { fClampCap = c; }
	int GetClampCap() const { return fClampCap; }
	void SetQuality(int q) { fQuality = q; }
	int GetQuality() const { return fQuality; }

	hsBool GetQuitIntro() const { return fQuitIntro; }
	void SetQuitIntro(hsBool on) { fQuitIntro = on; }

	void			SetClearColor( hsColorRGBA &color );
	hsColorRGBA		GetClearColor() const { return fClearColor; }

	// The client window has focus (true) or lost it (false)
	virtual void WindowActivate(bool active);
	virtual hsBool WindowActive() const { return fWindowActive; }

	void	SetMessagePumpProc( plMessagePumpProc proc ) { fMessagePumpProc = proc; }
	void ResetDisplayDevice(int Width, int Height, int ColorDepth, hsBool Windowed, int NumAASamples, int MaxAnisotropicSamples, hsBool VSync = false, hsBool windowOnly = false);
	void IDetectAudioVideoSettings();
	void IWriteDefaultGraphicsSettings(const wchar* destFile);

	plAnimDebugList *fAnimDebugList;

#if 0
	std::string fUsername;
	std::string fPasswordDigest;
	std::string fServer;
	int			fPlayerID;
	bool		fRecreatePlayer;
	bool		fAuthPassed;
#endif
};

#endif // plClient_inc
