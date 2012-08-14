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

#pragma once
#ifndef plClient_inc
#define plClient_inc


//#define NEW_CAMERA_CODE

#include "HeadSpin.h"
#include "hsBitVector.h"
#include "hsTemplates.h"

#include "hsStlUtils.h"
#include "pnKeyedObject/hsKeyedObject.h"
#include "pnKeyedObject/plUoid.h"
#include "plScene/plRenderRequest.h"

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
class plResPatcherMsg;

typedef void (*plMessagePumpProc)( void );

class plClient : public hsKeyedObject
{
protected:

    class plRoomRec
    {
        public:
            plSceneNode *fNode;
            uint32_t      fFlags;

            plRoomRec() { fNode = nil; fFlags = 0; }
            plRoomRec( plSceneNode *n, uint32_t f ) : fNode( n ), fFlags( f ) {}

            enum Flags
            {
                kHeld = 0x00000001
            };
    };

    hsBitVector             fFlags;

    plInputManager*         fInputManager;

    plPageTreeMgr*          fPageMgr;
    hsTArray<plRoomRec>     fRooms;
    plSceneNode*            fCurrentNode;

    plPipeline*             fPipeline;
    hsColorRGBA             fClearColor;
    plTransitionMgr         *fTransitionMgr;
    plLinkEffectsMgr        *fLinkEffectsMgr;
    plFontCache             *fFontCache;

    pfConsoleEngine*        fConsoleEngine;
    pfConsole*              fConsole;

    pfKI                    *fKIGUIGlue;

    bool                    fDone;
    bool                    fWindowActive;

    hsWindowHndl            fWindowHndl;

    double                  fLastProgressUpdate;
    plOperationProgress     *fProgressBar;

    pfGameGUIMgr            *fGameGUIMgr;

    virtual hsG3DDeviceModeRecord ILoadDevMode(const char* devModeFile);

    bool                    IUpdate();
    bool                    IDraw();
    bool                    IDrawProgress();
    
    plVirtualCam1*          fNewCamera;

    static plClient*        fInstance;
    char *                  fpAuxInitDir;
    static bool             fDelayMS;

    int                     fClampCap;
    int                     fQuality;

    bool                    fQuitIntro;
    hsTArray<plBinkPlayer*> fMovies;

    plMessagePumpProc       fMessagePumpProc;
    
#ifndef PLASMA_EXTERNAL_RELEASE
    bool                    bPythonDebugConnected;
#endif

    hsTArray<plRenderRequest*>      fPreRenderRequests;
    hsTArray<plRenderRequest*>      fPostRenderRequests;

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
    int fNumLoadingRooms;   // Number of rooms we're waiting for load callbacks on
    std::vector<plLocation> fRoomsLoading; // the locations we are currently in the middle of loading

    int fNumPostLoadMsgs;
    float fPostLoadMsgInc;
    
    void                    ICompleteInit ();
    void                    IOnAsyncInitComplete ();
    void                    IHandlePatcherMsg (plResPatcherMsg * msg);
    void                    IHandlePreloaderMsg (plPreloaderMsg * msg);
    void                    IHandleNetCommAuthMsg (plNetCommAuthMsg * msg);
    bool                    IHandleAgeLoaded2Msg (plAgeLoaded2Msg * msg);

    bool                    IFlushRenderRequests();
    void                    IProcessPreRenderRequests();
    void                    IProcessPostRenderRequests();
    void                    IProcessRenderRequests(hsTArray<plRenderRequest*>& reqs);
    void                    IAddRenderRequest(plRenderRequest* req);

    bool                    IPlayIntroBink(const char* movieName, float endDelay, float posX, float posY, float scaleX, float scaleY, float volume = 1.0);
    bool                    IHandleMovieMsg(plMovieMsg* mov);
    void                    IKillMovies();
    void                    IServiceMovies();

    void    IStartProgress( const char *title, float len );
    void    IIncProgress( float byHowMuch, const char *text );
    void    IStopProgress( void );

    static void IDispatchMsgReceiveCallback();
    static void IReadKeyedObjCallback(plKey key);
    static void IProgressMgrCallbackProc( plOperationProgress *progress );

    void    IPatchGlobalAgeFiles( void );

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

    static plClient*    GetInstance() { return fInstance; }
    static void         SetInstance(plClient* v) { fInstance=v; }
    
    virtual bool MsgReceive(plMessage* msg);
    
    bool        InitPipeline();

    void        InitInputs();

    void        InitDLLs();
    void        ShutdownDLLs();

    void        InitAuxInits();

    virtual bool StartInit();
    virtual bool Shutdown();
    virtual bool MainLoop();

    plClient&   SetDone(bool done) { fDone = done; return *this; }
    bool        GetDone() { return fDone; }

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

    bool HasFlag(int f) const { return fFlags.IsBitSet(f); }
    void SetFlag(int f, bool on=true) { fFlags.SetBit(f, on); }

    virtual plClient& SetWindowHandle(hsWindowHndl hndl) { fWindowHndl=hndl; return *this; }
    hsWindowHndl    GetWindowHandle() { return fWindowHndl; }

    plInputManager*     GetInputManager() { return fInputManager; }

    plPipeline*     GetPipeline() { return fPipeline; }

    plSceneNode*    GetCurrentScene() { return fCurrentNode; }

    pfConsoleEngine *GetConsoleEngine() { return fConsoleEngine; }

    void SetAuxInitDir(const char *p) { delete [] fpAuxInitDir; fpAuxInitDir = hsStrcpy(p); }

    static void EnableClientDelay() { plClient::fDelayMS = true; }

    // These are a hack to let the console fake a lesser capabile board and test out quality settings.
    // They should go away once we have this built into ClientSetup et.al.
    void SetClampCap(int c) { fClampCap = c; }
    int GetClampCap() const { return fClampCap; }
    void SetQuality(int q) { fQuality = q; }
    int GetQuality() const { return fQuality; }

    bool GetQuitIntro() const { return fQuitIntro; }
    void SetQuitIntro(bool on) { fQuitIntro = on; }

    void            SetClearColor( hsColorRGBA &color );
    hsColorRGBA     GetClearColor() const { return fClearColor; }

    // The client window has focus (true) or lost it (false)
    virtual void WindowActivate(bool active);
    virtual bool WindowActive() const { return fWindowActive; }

    void FlashWindow();
    void    SetMessagePumpProc( plMessagePumpProc proc ) { fMessagePumpProc = proc; }
    void ResetDisplayDevice(int Width, int Height, int ColorDepth, bool Windowed, int NumAASamples, int MaxAnisotropicSamples, bool VSync = false);
    void ResizeDisplayDevice(int Width, int Height, bool Windowed);
    void IDetectAudioVideoSettings();
    void IWriteDefaultGraphicsSettings(const wchar_t* destFile);

    plAnimDebugList *fAnimDebugList;

#if 0
    std::string fUsername;
    std::string fPasswordDigest;
    std::string fServer;
    int         fPlayerID;
    bool        fRecreatePlayer;
    bool        fAuthPassed;
#endif
};

#endif // plClient_inc
