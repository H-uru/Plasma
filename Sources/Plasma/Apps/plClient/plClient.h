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

#ifndef plClient_inc
#define plClient_inc


//#define NEW_CAMERA_CODE

#include "HeadSpin.h"
#include "hsBitVector.h"
#include "plFileSystem.h"

#include <list>

#include "pnKeyedObject/hsKeyedObject.h"
#include "pnKeyedObject/plUoid.h"

#include "plPipeline/hsG3DDeviceSelector.h"
#include "plScene/plRenderRequest.h"

class plAgeLoaded2Msg;
struct plAnimDebugList;
class plAudioSystem;
class plClientMsg;
class pfConsole;
class pfConsoleEngine;
class plFactory;
class plFontCache;
class pfGameGUIMgr;
class plInputController;
class plInputManager;
class plKey;
class pfKI;
class plLinkEffectsMgr;
class plLocation;
class plMovieMsg;
class plMoviePlayer;
class plNetClientApp;
class plNetCommAuthMsg;
class plOperationProgress;
class plPageTreeMgr;
class plPipeline;
class plPreloaderMsg;
class hsResMgr;
class plResPatcherMsg;
class plSceneNode;
class plSceneObject;
class plTimerCallbackManager;
class plTimerShare;
class plTransitionMgr;
class plVirtualCam1;

typedef void (*plMessagePumpProc)();

class plClient : public hsKeyedObject
{
protected:
    typedef void (*pInitGlobalsFunc)(hsResMgr*, plFactory*, plTimerCallbackManager*, plTimerShare*, plNetClientApp*);

    class plRoomRec
    {
        public:
            plSceneNode *fNode;
            uint32_t      fFlags;

            plRoomRec() : fNode(), fFlags() { }
            plRoomRec( plSceneNode *n, uint32_t f ) : fNode( n ), fFlags( f ) {}

            enum Flags
            {
                kHeld = 0x00000001
            };
    };

    hsBitVector             fFlags;

    plInputManager*         fInputManager;

    plPageTreeMgr*          fPageMgr;
    std::vector<plRoomRec>  fRooms;
    plSceneNode*            fCurrentNode;

    plPipeline*             fPipeline;
    hsColorRGBA             fClearColor;
    plTransitionMgr         *fTransitionMgr;
    plLinkEffectsMgr        *fLinkEffectsMgr;
    plFontCache             *fFontCache;

    pfConsoleEngine*        fConsoleEngine;
    pfConsole*              fConsole;

    bool                    fDone;
    bool                    fWindowActive;

    hsWindowHndl            fWindowHndl;

    double                  fLastProgressUpdate;
    plOperationProgress     *fProgressBar;

    pfGameGUIMgr            *fGameGUIMgr;

    bool                    IUpdate();
    bool                    IDraw();
    bool                    IDrawProgress();
    
    plVirtualCam1*          fNewCamera;

    static plClient*        fInstance;
    plFileName              fpAuxInitDir;
    static bool             fDelayMS;

    int                     fClampCap;
    int                     fQuality;

    bool                    fQuitIntro;
    std::vector<plMoviePlayer*> fMovies;

    plMessagePumpProc       fMessagePumpProc;
    
#ifndef PLASMA_EXTERNAL_RELEASE
    bool                    bPythonDebugConnected;
#endif

    std::vector<plRenderRequest*>   fPreRenderRequests;
    std::vector<plRenderRequest*>   fPostRenderRequests;

    bool fHoldLoadRequests;
    class LoadRequest
    {
    public:
        LoadRequest(const plLocation& loc, bool hold) { this->loc = loc; this->hold = hold; }
        plLocation loc;
        bool hold;
    };
    std::list<LoadRequest*> fLoadRooms;
    int fNumLoadingRooms;   // Number of rooms we're waiting for load callbacks on
    std::vector<plLocation> fRoomsLoading; // the locations we are currently in the middle of loading

    int fNumPostLoadMsgs;
    float fPostLoadMsgInc;
    
    void                    ICompleteInit ();
    void                    IOnAsyncInitComplete ();
    void                    IHandlePatcherMsg (plResPatcherMsg * msg);
    bool                    IHandleAgeLoaded2Msg (plAgeLoaded2Msg * msg);

    bool                    IFlushRenderRequests();
    void                    IProcessPreRenderRequests();
    void                    IProcessPostRenderRequests();
    void                    IProcessRenderRequests(std::vector<plRenderRequest*>& reqs);
    void                    IAddRenderRequest(plRenderRequest* req);

    bool                    IPlayIntroMovie(const char* movieName, float endDelay, float posX, float posY, float scaleX, float scaleY, float volume = 1.0);
    bool                    IHandleMovieMsg(plMovieMsg* mov);
    void                    IKillMovies();
    void                    IServiceMovies();

    void    IStartProgress( const char *title, float len );
    void    IIncProgress( float byHowMuch, const char *text );
    void    IStopProgress();

    static plPipeline* ICreatePipeline(hsWindowHndl disp, hsWindowHndl hWnd, const hsG3DDeviceModeRecord* devMode);

    static void IDispatchMsgReceiveCallback();
    static void IReadKeyedObjCallback(const plKey& key);
    static void IProgressMgrCallbackProc( plOperationProgress *progress );

    void    IPatchGlobalAgeFiles();

    hsSsize_t IFindRoomByLoc(const plLocation& loc);
    bool IIsRoomLoading(const plLocation& loc);
    void IQueueRoomLoad(const std::vector<plLocation>& locs, bool hold);
    void ILoadNextRoom();
    void IUnloadRooms(const std::vector<plLocation>& locs);
    void IRoomLoaded(plSceneNode* node, bool hold);
    void IRoomUnloaded(plSceneNode* node);
    void ISetGraphicsDefaults();
    void IDetectAudioVideoSettings();
    void IWriteDefaultAudioSettings(const plFileName& destFile);
    void IWriteDefaultGraphicsSettings(const plFileName& destFile);

    // These have platform-dependent implementations
    void IResizeNativeDisplayDevice(int width, int height, bool windowed);
    void IChangeResolution(int width, int height);
    void IUpdateProgressIndicator(plOperationProgress* progress);

    void IResizeWindow(int width, int height);

public:

    plClient();
    virtual ~plClient();

    CLASSNAME_REGISTER( plClient );
    GETINTERFACE_ANY( plClient, hsKeyedObject );

    static plClient*    GetInstance() { return fInstance; }
    static void         SetInstance(plClient* v) { fInstance=v; }
    
    bool MsgReceive(plMessage* msg) override;
    
    bool        InitPipeline(hsWindowHndl display, uint32_t devType = hsG3DDeviceSelector::kDevTypeUnknown);

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
        kFlagSkipIntroMovies,
    };

    bool HasFlag(int f) const { return fFlags.IsBitSet(f); }
    void SetFlag(int f, bool on=true) { fFlags.SetBit(f, on); }

    virtual plClient& SetWindowHandle(hsWindowHndl hndl) { fWindowHndl=hndl; return *this; }
    hsWindowHndl    GetWindowHandle() { return fWindowHndl; }

    plInputManager*     GetInputManager() { return fInputManager; }

    plPipeline*     GetPipeline() { return fPipeline; }

    plSceneNode*    GetCurrentScene() { return fCurrentNode; }

    pfConsoleEngine *GetConsoleEngine() { return fConsoleEngine; }

    void SetAuxInitDir(plFileName dir) { fpAuxInitDir = std::move(dir); }

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

    bool BeginGame();

    // These have platform-dependent implementations
    void ShowClientWindow();
    void FlashWindow();

    void SetMessagePumpProc(plMessagePumpProc proc) { fMessagePumpProc = proc; }
    void ResetDisplayDevice(int Width, int Height, int ColorDepth, bool Windowed, int NumAASamples, int MaxAnisotropicSamples, bool VSync = false);
    void ResizeDisplayDevice(int Width, int Height, bool Windowed);

    plAnimDebugList *fAnimDebugList;
};

#endif // plClient_inc
