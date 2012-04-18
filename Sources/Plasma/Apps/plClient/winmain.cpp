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

#include <stdio.h>
#include <direct.h>     // windows directory handling fxns (for chdir)
#include <process.h>

//#define DETACH_EXE  // Microsoft trick to force loading of exe to memory 
#ifdef DETACH_EXE
    #include <dmdfm.h>      // Windows Load EXE into memory suff
#endif

#ifdef HAVE_SSE
#   include <intrin.h>
#endif

#include <curl/curl.h>

#include "HeadSpin.h"
#include "hsStream.h"

#include "plClient.h"
#include "plClientResMgr/plClientResMgr.h"
#include "pfCrashHandler/plCrashCli.h"
#include "plNetClient/plNetClientMgr.h"
#include "plNetClient/plNetLinkingMgr.h"
#include "plInputCore/plInputManager.h"
#include "plUnifiedTime/plUnifiedTime.h"
#include "plPipeline.h"
#include "plResMgr/plResManager.h"
#include "plResMgr/plLocalization.h"
#include "plFile/plEncryptedStream.h"

#include "pnEncryption/plChallengeHash.h"

#include "plStatusLog/plStatusLog.h"
#include "pnProduct/pnProduct.h"
#include "plNetGameLib/plNetGameLib.h"
#include "plFile/plFileUtils.h"

#include "plPhysX/plSimulationMgr.h"

#include "res/resource.h"

//#include <shellapi.h>
//
// Defines
//

#define CLASSNAME   "Plasma"    // Used in WinInit()
#define PARABLE_NORMAL_EXIT     0   // i.e. exited WinMain normally

#define TIMER_UNITS_PER_SEC (float)1e3
#define UPDATE_STATUSMSG_SECONDS 30
#define WM_USER_SETSTATUSMSG WM_USER+1

//
// Globals
//
hsBool gHasMouse = false;
ITaskbarList3* gTaskbarList = nil; // NT 6.1+ taskbar stuff

extern hsBool gDataServerLocal;

enum
{
    kArgSkipLoginDialog,
    kArgServerIni,
    kArgLocalData,
};

static const CmdArgDef s_cmdLineArgs[] = {
    { kCmdArgFlagged  | kCmdTypeBool,       L"SkipLoginDialog", kArgSkipLoginDialog },
    { kCmdArgFlagged  | kCmdTypeString,     L"ServerIni",       kArgServerIni },
    { kCmdArgFlagged  | kCmdTypeBool,       L"LocalData",       kArgLocalData   },
};

/// Made globals now, so we can set them to zero if we take the border and 
/// caption styles out ala fullscreen (8.11.2000 mcn)
int gWinBorderDX    = GetSystemMetrics( SM_CXSIZEFRAME );
int gWinBorderDY    = GetSystemMetrics( SM_CYSIZEFRAME );
int gWinMenuDY      = GetSystemMetrics( SM_CYCAPTION );

//#include "global.h"
plClient        *gClient;
bool            gPendingActivate = false;
bool            gPendingActivateFlag = false;

#ifndef HS_DEBUGGING
static plCrashCli s_crash;
#endif

static bool     s_loginDlgRunning = false;
static hsSemaphore      s_statusEvent(0);   // Start non-signalled
static UINT     s_WmTaskbarList = RegisterWindowMessage("TaskbarButtonCreated");

FILE *errFP = nil;
HINSTANCE               gHInst = NULL;      // Instance of this app

static const unsigned   AUTH_LOGIN_TIMER    = 1;
static const unsigned   AUTH_FAILED_TIMER   = 2;

#define FAKE_PASS_STRING "********"

//============================================================================
// External patcher file (directly starting plClient is not allowed)
//============================================================================
#ifdef PLASMA_EXTERNAL_RELEASE

static wchar_t s_patcherExeName[] = L"UruLauncher.exe";

#endif // PLASMA_EXTERNAL_RELEASE

//============================================================================
// PhysX installer
//============================================================================
static wchar_t s_physXSetupExeName[] = L"PhysX_Setup.exe";

//============================================================================
// TRANSGAMING detection  & dialog replacement
//============================================================================
typedef BOOL (WINAPI *IsTransgaming) (void);
typedef const char * (WINAPI *TGGetOS) (void);
typedef LPVOID (WINAPI *TGLaunchUNIXApp) (const char *pPath, const char *pMode);
typedef BOOL (WINAPI *TGUNIXAppReadLine) (LPVOID pApp, char *pBuf, int bufSize);
typedef BOOL (WINAPI *TGUNIXAppWriteLine) (LPVOID pApp, const char *pLine);
typedef BOOL (WINAPI *TGUNIXAppClose) (LPVOID pApp);

static bool TGIsCider = false;
static TGLaunchUNIXApp pTGLaunchUNIXApp;
static TGUNIXAppReadLine pTGUNIXAppReadLine;
static TGUNIXAppWriteLine pTGUNIXAppWriteLine;
static TGUNIXAppClose pTGUNIXAppClose;

#define TG_NEW_LOGIN_PATH "C:\\Program Files\\Uru Live\\Cider\\URU Live Login.app"
#define TG_NEW_LOGIN_POPEN_PATH "/transgaming/c_drive/Program Files/Uru Live/Cider/URU Live Login.app/Contents/MacOS/URU Live Login"
#define TG_OLD_LOGIN_POPEN_PATH "/URU Live Login.app/Contents/MacOS/URU Live Login"

#define TG_NEW_EULA_PATH "C:\\Program Files\\Uru Live\\Cider\\URU Live EULA.app"
#define TG_NEW_EULA_POPEN_PATH "/transgaming/c_drive/Program Files/Uru Live/Cider/URU Live EULA.app/Contents/MacOS/URU Live EULA"
#define TG_OLD_EULA_POPEN_PATH "/URU Live EULA.app/Contents/MacOS/URU Live EULA"

//============================================================================
// LoginDialogParam
//============================================================================
struct LoginDialogParam {
    ENetError   authError;
    char        username[kMaxAccountNameLength];
    ShaDigest   namePassHash;
    bool        remember;
    int         focus;
};

static bool AuthenticateNetClientComm(ENetError* result, HWND parentWnd);
static void GetCryptKey(uint32_t* cryptKey, unsigned size);
static void SaveUserPass (LoginDialogParam *pLoginParam, char *password);
static void LoadUserPass (LoginDialogParam *pLoginParam);
static void AuthFailedStrings (ENetError authError,
                                         const char **ppStr1, const char **ppStr2,
                                         const wchar_t **ppWStr);


// Detect whether we're running under TRANSGAMING Cider
//==============================================================================
static void TGDoCiderDetection ()
{
    HMODULE hMod = GetModuleHandle ("ntdll");
    if (!hMod)
        return;

    IsTransgaming pIsTg = (IsTransgaming)GetProcAddress (hMod, "IsTransgaming");
    if (!pIsTg || !pIsTg ())
        return;

    TGGetOS pTGOS = (TGGetOS)GetProcAddress (hMod, "TGGetOS");
    const char *pOS = NULL;
    if (pTGOS)
        pOS = pTGOS ();
    if (!pOS || strcmp (pOS, "MacOSX"))
        return;

    TGIsCider = true;
    pTGLaunchUNIXApp = (TGLaunchUNIXApp)GetProcAddress (hMod, "TGLaunchUNIXApp");
    pTGUNIXAppReadLine = (TGUNIXAppReadLine)GetProcAddress (hMod, "TGUNIXAppReadLine");
    pTGUNIXAppWriteLine = (TGUNIXAppWriteLine)GetProcAddress (hMod, "TGUNIXAppWriteLine");
    pTGUNIXAppClose = (TGUNIXAppClose)GetProcAddress (hMod, "TGUNIXAppClose");
}

static bool TGRunLoginDialog (LoginDialogParam *pLoginParam)
{
    while (true)
    {
        LPVOID pApp;
        if (GetFileAttributes (TG_NEW_LOGIN_PATH) != INVALID_FILE_ATTRIBUTES)
            pApp = pTGLaunchUNIXApp (TG_NEW_LOGIN_POPEN_PATH, "r+");
        else
            pApp = pTGLaunchUNIXApp (TG_OLD_LOGIN_POPEN_PATH, "r+");

        if (!pApp)
        {
            hsMessageBox ("Incomplete or corrupted installation!\nUnable to locate Login dialog",
                "Error", hsMessageBoxNormal);
            return false;
        }

        // Send user/pwd/remember
        pTGUNIXAppWriteLine (pApp, pLoginParam->username);
        if (pLoginParam->remember)
          pTGUNIXAppWriteLine (pApp, FAKE_PASS_STRING);
        else
          pTGUNIXAppWriteLine (pApp, "");
        if (pLoginParam->remember)
          pTGUNIXAppWriteLine (pApp, "y");
        else
          pTGUNIXAppWriteLine (pApp, "n");

        if (!pTGUNIXAppReadLine (pApp, pLoginParam->username, sizeof (pLoginParam->username)))
        {
            pTGUNIXAppClose (pApp);
            hsMessageBox ("Incomplete or corrupted installation!\nUnable to locate Login dialog",
                "Error", hsMessageBoxNormal);
            return false;
        }

        // Check if user selected 'Cancel'
        if (StrCmp (pLoginParam->username, "text:", 5) != 0)
        {
            pTGUNIXAppClose (pApp);
            return false;
        }
        memmove (pLoginParam->username, pLoginParam->username + 5, StrLen (pLoginParam->username) - 5);
        pLoginParam->username[StrLen (pLoginParam->username) - 5] = '\0';

        char Password[kMaxPasswordLength];
        if (!pTGUNIXAppReadLine (pApp, Password, sizeof (Password)))
        {
            pTGUNIXAppClose (pApp);
            hsMessageBox ("Incomplete or corrupted installation!\nLogin dialog not found or working",
                "Error", hsMessageBoxNormal);
            return false;
        }

        char Remember[16];
        if (!pTGUNIXAppReadLine (pApp, Remember, sizeof (Remember)))
        {
            pTGUNIXAppClose (pApp);
            hsMessageBox ("Incomplete or corrupted installation!\nLogin dialog not found or working",
                "Error", hsMessageBoxNormal);
            return false;
        }

        pTGUNIXAppClose (pApp);

        pLoginParam->remember = (Remember[0] == 'y');
        SaveUserPass (pLoginParam, Password);

        // Do login & see if it failed
        ENetError auth;
        bool cancelled = AuthenticateNetClientComm(&auth, NULL);

        if (IS_NET_SUCCESS (auth) && !cancelled)
            break;

        if (!cancelled)
          {
                const char *pStr1, *pStr2;
                const wchar_t *pWStr;
                unsigned int Len;
                char *pTmpStr;

                AuthFailedStrings (auth, &pStr1, &pStr2, &pWStr);

                Len = StrLen (pStr1) + 1;
                if (pStr2)
                  Len += StrLen (pStr2) + 2;
                if (pWStr)
                  Len += StrLen (pWStr) + 2;

                pTmpStr = new char[Len];
                StrCopy (pTmpStr, pStr1, StrLen (pStr1));
                if (pStr2)
                  {
                     StrCopy (pTmpStr + StrLen (pTmpStr), "\n\n", 2);
                     StrCopy (pTmpStr + StrLen (pTmpStr), pStr2, StrLen (pStr2));
                  }
                if (pWStr)
                  {
                     StrCopy (pTmpStr + StrLen (pTmpStr), "\n\n", 2);
                     StrToAnsi (pTmpStr + StrLen (pTmpStr), pWStr, StrLen (pWStr));
                  }

                hsMessageBox (pTmpStr, "Error", hsMessageBoxNormal);
                delete [] pTmpStr;
          }
        else
            NetCommDisconnect();
    };

    return true;
}

bool TGRunTOSDialog ()
{
    char Buf[16];
    LPVOID pApp;

    if (GetFileAttributes (TG_NEW_EULA_PATH) != INVALID_FILE_ATTRIBUTES)
        pApp = pTGLaunchUNIXApp (TG_NEW_EULA_POPEN_PATH, "r");
    else
        pApp = pTGLaunchUNIXApp (TG_OLD_EULA_POPEN_PATH, "r");

    if (!pApp)
    {
        hsMessageBox ("Incomplete or corrupted installation!\nTOS dialog not found or working",
                "Error", hsMessageBoxNormal);
        return false;
    }

    if (!pTGUNIXAppReadLine (pApp, Buf, sizeof (Buf)))
    {
        hsMessageBox ("Incomplete or corrupted installation!\nTOS dialog not found or working",
                "Error", hsMessageBoxNormal);
        pTGUNIXAppClose (pApp);
        return false;
    }

    pTGUNIXAppClose (pApp);

    return (StrCmp (Buf, "accepted") == 0);
}

void DebugMsgF(const char* format, ...);

// Handles all the windows messages we might receive
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{ 
    static bool gDragging = false;
    static uint32_t keyState=0;

    // Handle messages
    switch (message) {
        case WM_TIMECHANGE:
            // To prevent cheating and keep things better synchronized,
            // we will completely re-eval the offsets on the next NetMsg we
            // get from the server
            if (plNetClientMgr* nc = plNetClientMgr::GetInstance())
                nc->ResetServerTimeOffset(true);
            break;

        case WM_KEYDOWN :
        case WM_LBUTTONDOWN :
        case WM_RBUTTONDOWN :
        case WM_LBUTTONDBLCLK :     // The left mouse button was double-clicked. 
        case WM_MBUTTONDBLCLK :     // The middle mouse button was double-clicked. 
        case WM_MBUTTONDOWN :       // The middle mouse button was pressed. 
        case WM_RBUTTONDBLCLK :     // The right mouse button was double-clicked. 
            // If they did anything but move the mouse, quit any intro movie playing.
            {
                if( gClient )
                    gClient->SetQuitIntro(true);
            }
            // Fall through to other events
        case WM_CHAR:
        case WM_KEYUP :
        case WM_LBUTTONUP :
        case WM_RBUTTONUP :
        case WM_MBUTTONUP :         // The middle mouse button was released. 
        case WM_MOUSEMOVE :
        case 0x020A:                // fuc&ing windows b.s...
            {
                if (gClient && gClient->WindowActive() && gClient->GetInputManager())
                {
                    gClient->GetInputManager()->HandleWin32ControlEvent(message, wParam, lParam, hWnd);
                }
            }
            return TRUE;

#if 0
        case WM_KILLFOCUS:
            SetForegroundWindow(hWnd);
            return TRUE;
#endif

        case WM_SYSKEYUP:
        case WM_SYSKEYDOWN:
            {
                if (gClient && gClient->WindowActive() && gClient->GetInputManager())
                {
                    gClient->GetInputManager()->HandleWin32ControlEvent(message, wParam, lParam, hWnd);
                }
                //DefWindowProc(hWnd, message, wParam, lParam);
            }
            return TRUE;
            
        case WM_SYSCOMMAND:
            switch (wParam) {
                // Trap ALT so it doesn't pause the app
                case SC_KEYMENU :
                //// disable screensavers and monitor power downs too.
                case SC_SCREENSAVE:
                case SC_MONITORPOWER:
                    return 0;
                case SC_CLOSE :
                    // kill game if window is closed
                    gClient->SetDone(TRUE);
                    if (plNetClientMgr * mgr = plNetClientMgr::GetInstance())
                        mgr->QueueDisableNet(false, nil);
                    DestroyWindow(gClient->GetWindowHandle());
                    return TRUE;
            }
            return TRUE;

        case WM_ACTIVATE:
            {
                bool active = (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE);
                bool minimized = (HIWORD(wParam) != 0);

                DebugMsgF("Got WM_ACTIVATE, active=%s, minimized=%s, clicked=%s",
                    active ? "true" : "false",
                    minimized ? "true" : "false",
                    (LOWORD(wParam) == WA_CLICKACTIVE) ? "true" : "false");

                if (gClient && !minimized && !gClient->GetDone())
                {
                    if (LOWORD(wParam) == WA_CLICKACTIVE)
                    {
                        // See if they've clicked on the frame, in which case they just want to
                        // move, not activate, us.
                        POINT pt;
                        GetCursorPos(&pt);
                        ScreenToClient(hWnd, &pt);

                        RECT rect;
                        GetClientRect(hWnd, &rect);

                        if( (pt.x < rect.left)
                            ||(pt.x >= rect.right)
                            ||(pt.y < rect.top)
                            ||(pt.y >= rect.bottom) )
                        {
                            active = false;
                        }
                    }
                    gClient->WindowActivate(active);
                }
                else
                {
                    gPendingActivate = true;
                    gPendingActivateFlag = active;
                }
            }
            return TRUE;

        // Let go of the mouse if the window is being moved.
        case WM_ENTERSIZEMOVE:
            DebugMsgF("Got WM_ENTERSIZEMOVE%s", gClient ? "" : ", no client, ignoring");
            gDragging = true;
            if( gClient )
                gClient->WindowActivate(false);
            return TRUE;

        // Redo the mouse capture if the window gets moved
        case WM_EXITSIZEMOVE:
            DebugMsgF("Got WM_EXITSIZEMOVE%s", gClient ? "" : ", no client, ignoring");
            gDragging = false;
            if( gClient )
                gClient->WindowActivate(true);
            return TRUE;

        // Redo the mouse capture if the window gets moved (special case for Colin
        // and his cool program that bumps windows out from under the taskbar)
        case WM_MOVE:
            if (!gDragging && gClient && gClient->GetInputManager())
            {
                gClient->GetInputManager()->Activate(true);
                DebugMsgF("Got WM_MOVE");
            }
            else
                DebugMsgF("Got WM_MOVE, but ignoring");
            return TRUE;

        /// Resize the window
        // (we do WM_SIZING here instead of WM_SIZE because, for some reason, WM_SIZE is
        //  sent to the window when we do fullscreen, and what's more, it gets sent BEFORE
        //  the fullscreen flag is sent. How does *that* happen? Anyway, WM_SIZING acts
        //  just like WM_SIZE, except that it ONLY gets sent when the user changes the window
        //  size, not when the window is minimized or restored)
        case WM_SIZING:
            {
                DebugMsgF("Got WM_SIZING");
                RECT r;
                ::GetClientRect(hWnd, &r);
                gClient->GetPipeline()->Resize(r.right - r.left, r.bottom - r.top);
            }
            return TRUE;

        case WM_SIZE:
            // Let go of the mouse if the window is being minimized
            if (wParam == SIZE_MINIMIZED)
            {
                DebugMsgF("Got WM_SIZE, SIZE_MINIMIZED%s", gClient ? "" : ", but no client, ignoring");
                if (gClient)
                    gClient->WindowActivate(false);
            }
            // Redo the mouse capture if the window gets restored
            else if (wParam == SIZE_RESTORED)
            {
                DebugMsgF("Got WM_SIZE, SIZE_RESTORED%s", gClient ?  "" : ", but no client, ignoring");
                if (gClient)
                    gClient->WindowActivate(true);
            }
            return TRUE;
        
        case WM_CLOSE:
            gClient->SetDone(TRUE);
            if (plNetClientMgr * mgr = plNetClientMgr::GetInstance())
                mgr->QueueDisableNet(false, nil);
            DestroyWindow(gClient->GetWindowHandle());
            return TRUE;
        case WM_DESTROY:
            gClient->SetDone(TRUE);
            if (plNetClientMgr * mgr = plNetClientMgr::GetInstance())
                mgr->QueueDisableNet(false, nil);
            PostQuitMessage(0);
            return TRUE;
        case WM_CREATE:
            // Create renderer
            return TRUE;
    }

    // Messages we registered for manually (no const value)
    if (message == s_WmTaskbarList)
    {
        // Grab the Windows 7 taskbar list stuff
        if (gTaskbarList)
            gTaskbarList->Release();
        HRESULT result = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_ALL, IID_ITaskbarList3, (void**)&gTaskbarList);
        if (FAILED(result))
            gTaskbarList = nil;
        return TRUE;
    }
    else
        return DefWindowProc(hWnd, message, wParam, lParam);
}
 
void    PumpMessageQueueProc( void )
{
    MSG msg;

    // Look for a message
    while (PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ))
    {
        // Handle the message
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
}

void InitNetClientComm()
{
    NetCommStartup();
}

BOOL CALLBACK AuthDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    static bool* cancelled = NULL;

    switch( uMsg )
    {
    case WM_INITDIALOG:
        cancelled = (bool*)lParam;
        SetTimer(hwndDlg, AUTH_LOGIN_TIMER, 10, NULL);
        return TRUE;

    case WM_TIMER:
        if (wParam == AUTH_LOGIN_TIMER)
        {
            if (NetCommIsLoginComplete())
                EndDialog(hwndDlg, 1);
            else
                NetCommUpdate();

            return TRUE;
        }

        return FALSE;

    case WM_NCHITTEST:
        SetWindowLongPtr(hwndDlg, DWL_MSGRESULT, (LONG_PTR)HTCAPTION);
        return TRUE;

    case WM_DESTROY:
        KillTimer(hwndDlg, AUTH_LOGIN_TIMER);
        return TRUE;

    case WM_COMMAND:
        if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDCANCEL)
        {
            *cancelled = true;
            EndDialog(hwndDlg, 1);
        }
        return TRUE;
    }
    
    return DefWindowProc(hwndDlg, uMsg, wParam, lParam);
}

static bool AuthenticateNetClientComm(ENetError* result, HWND parentWnd)
{
    if (!NetCliAuthQueryConnected())
        NetCommConnect();

    bool cancelled = false;
    NetCommAuthenticate(nil);

    ::DialogBoxParam(gHInst, MAKEINTRESOURCE( IDD_AUTHENTICATING ), parentWnd, AuthDialogProc, (LPARAM)&cancelled);

    if (!cancelled)
        *result = NetCommGetAuthResult();
    else
        *result = kNetSuccess;
    
    return cancelled;
}

void DeInitNetClientComm()
{
    NetCommShutdown();
}

BOOL CALLBACK WaitingForPhysXDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch( uMsg )
    {
    case WM_INITDIALOG:
        ::SetDlgItemText( hwndDlg, IDC_STARTING_TEXT, "Waiting for PhysX install...");
        return true; 

    }
    return 0;
}

bool InitPhysX()
{
    bool physXInstalled = false;
    while (!physXInstalled)
    {
        plSimulationMgr::Init();
        if (!plSimulationMgr::GetInstance())
        {
            int ret = hsMessageBox("PhysX is not installed, or an older version is installed.\nInstall new version? (Game will exit if you click \"No\")",
                "Missing PhysX", hsMessageBoxYesNo);
            if (ret == hsMBoxNo) // exit if no
                return false;

            // launch the PhysX installer
            STARTUPINFOW startupInfo;
            PROCESS_INFORMATION processInfo; 
            memset(&startupInfo, 0, sizeof(startupInfo));
            memset(&processInfo, 0, sizeof(processInfo));
            startupInfo.cb = sizeof(startupInfo);
            if(!CreateProcessW(NULL, s_physXSetupExeName, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo))
            {
                hsMessageBox("Failed to launch PhysX installer.\nPlease re-run URU to ensure you have the latest version.", "Error", hsMessageBoxNormal);
                return false;
            }

            // let the user know what's going on
            HWND waitingDialog = ::CreateDialog(gHInst, MAKEINTRESOURCE(IDD_LOADING), NULL, WaitingForPhysXDialogProc);

            // run a loop to wait for it to quit, pumping the windows message queue intermittently
            DWORD waitRet = WaitForSingleObject(processInfo.hProcess, 100);
            MSG msg;
            while (waitRet == WAIT_TIMEOUT)
            {
                if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
                waitRet = WaitForSingleObject(processInfo.hProcess, 100);
            }

            // cleanup
            CloseHandle(processInfo.hThread);
            CloseHandle(processInfo.hProcess);
            ::DestroyWindow(waitingDialog);
        }
        else
        {
            plSimulationMgr::GetInstance()->Suspend();
            physXInstalled = true;
        }
    }
    return true;
}

bool    InitClient( HWND hWnd )
{
    plResManager *resMgr = new plResManager;
    resMgr->SetDataPath("dat");
    hsgResMgr::Init(resMgr);

    if(!plFileUtils::FileExists("resource.dat"))
    {
        hsMessageBox("Required file 'resource.dat' not found.", "Error", hsMessageBoxNormal);
        return false;
    }
    plClientResMgr::Instance().ILoadResources("resource.dat");

    gClient = new plClient;
    if( gClient == nil )
        return false;

    if (!InitPhysX())
        return false;

    gClient->SetWindowHandle( hWnd );

#ifdef DETACH_EXE
    hInstance = ((LPCREATESTRUCT) lParam)->hInstance;

    // This Function loads the EXE into Virtual memory...supposedly
    HRESULT hr = DetachFromMedium(hInstance, DMDFM_ALWAYS | DMDFM_ALLPAGES);
#endif

    if( gClient->InitPipeline() )
        gClient->SetDone(true);
    else
    {
        gClient->ResizeDisplayDevice(gClient->GetPipeline()->Width(), gClient->GetPipeline()->Height(), !gClient->GetPipeline()->IsFullScreen());
    }
    
    if( gPendingActivate )
    {
        // We need this because the window gets a WM_ACTIVATE before we get to this function, so 
        // the above flag lets us know that we need to fake a late activate msg to the client
        gClient->WindowActivate( gPendingActivateFlag );
    }

    gClient->SetMessagePumpProc( PumpMessageQueueProc );

    return true;
}

// Initializes all that windows junk, creates class then shows main window
BOOL WinInit(HINSTANCE hInst, int nCmdShow)
{
    // Fill out WNDCLASS info
    WNDCLASS wndClass;
    wndClass.style              = CS_DBLCLKS;   // CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc        = WndProc;
    wndClass.cbClsExtra         = 0;
    wndClass.cbWndExtra         = 0;
    wndClass.hInstance          = hInst;
    wndClass.hIcon              = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_DIRT));

    wndClass.hCursor            = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground      = (struct HBRUSH__*) (GetStockObject(BLACK_BRUSH));
    wndClass.lpszMenuName       = CLASSNAME;
    wndClass.lpszClassName      = CLASSNAME;
    
    // can only run one at a time anyway, so just quit if another is running
    if (!RegisterClass(&wndClass)) 
        return FALSE;

    /// 8.11.2000 - Test for OpenGL fullscreen, and if so use no border, no caption;
    /// else, use our normal styles

    char windowName[256];
    wchar_t productString[256];
    StrCopy(productString, ProductLongName(), arrsize(productString));
    StrToAnsi(windowName, productString, arrsize(windowName));
    
    // Create a window
    HWND hWnd = CreateWindow(
        CLASSNAME, windowName, 
        WS_OVERLAPPEDWINDOW,
        0, 0, 
        800 + gWinBorderDX * 2,
        600 + gWinBorderDY * 2 + gWinMenuDY,
         NULL, NULL, hInst, NULL
    );
//  gClient->SetWindowHandle((hsWindowHndl)

    if( !InitClient( hWnd ) )
        return FALSE;

    // Return false if window creation failed
    if (!gClient->GetWindowHandle())
    {
        OutputDebugString("Create window failed\n");
        return FALSE;
    }
    else
    {
        OutputDebugString("Create window OK\n");
    }
    return TRUE;
}

//
// For error logging
//
static FILE* gDebugFile=NULL;
void DebugMessageProc(const char* msg)
{
    OutputDebugString(msg);
    OutputDebugString("\n");
    if (gDebugFile != NULL)
    {
        fprintf(gDebugFile, "%s\n", msg);
        fflush(gDebugFile);
    }
}

void DebugMsgF(const char* format, ...)
{
#ifndef PLASMA_EXTERNAL_RELEASE
    va_list args;
    va_start(args, format);

    char buf[256];
    int numWritten = _vsnprintf(buf, sizeof(buf), format, args);
    hsAssert(numWritten > 0, "Buffer too small");

    va_end(args);

    DebugMessageProc(buf);
#endif
}

static void AuthFailedStrings (ENetError authError,
                                         const char **ppStr1, const char **ppStr2,
                                         const wchar_t **ppWStr)
{
  *ppStr1 = NULL;
  *ppStr2 = NULL;
  *ppWStr = NULL;

    switch (plLocalization::GetLanguage())
    {
        case plLocalization::kFrench:
        case plLocalization::kGerman:
        case plLocalization::kJapanese:
            *ppStr1 = "Authentication Failed. Please try again.";
            break;

        default:
            *ppStr1 = "Authentication Failed. Please try again.";

            switch (authError)
            {
                case kNetErrAccountNotFound:
                    *ppStr2 = "Account Not Found.";
                    break;
                case kNetErrAccountNotActivated:
                    *ppStr2 = "Account Not Activated.";
                    break;
                case kNetErrConnectFailed:
                    *ppStr2 = "Unable to connect to Myst Online.";
                    break;
                case kNetErrDisconnected:
                    *ppStr2 = "Disconnected from Myst Online.";
                    break;
                case kNetErrAuthenticationFailed:
                    *ppStr2 = "Incorrect password.\n\nMake sure CAPS LOCK is not on.";
                    break;
                case kNetErrGTServerError:
                case kNetErrGameTapConnectionFailed:
                    *ppStr2 = "Unable to connect to GameTap, please try again in a few minutes.";
                    break;
                case kNetErrAccountBanned:
                    *ppStr2 = "Your account has been banned from accessing Myst Online.  If you are unsure as to why this happened please contact customer support.";
                    break;
                default:
                    *ppWStr =  NetErrorToString (authError);
                    break;
            }
            break;
    }
}


BOOL CALLBACK AuthFailedDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch( uMsg )
    {
        case WM_INITDIALOG:
            {
                LoginDialogParam* loginParam = (LoginDialogParam*)lParam;
                const char *pStr1, *pStr2;
                const wchar_t *pWStr;

                AuthFailedStrings (loginParam->authError,
                                         &pStr1, &pStr2, &pWStr);

                if (pStr1)
                        ::SetDlgItemText( hwndDlg, IDC_AUTH_TEXT, pStr1);
                if (pStr2)
                        ::SetDlgItemText( hwndDlg, IDC_AUTH_MESSAGE, pStr2);
                if (pWStr)
                        ::SetDlgItemTextW( hwndDlg, IDC_AUTH_MESSAGE, pWStr);
            }
            return TRUE;

        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDOK)
            {
                EndDialog(hwndDlg, 1);
            }
            return TRUE;

        case WM_NCHITTEST:
            SetWindowLongPtr(hwndDlg, DWL_MSGRESULT, (LONG_PTR)HTCAPTION);
            return TRUE;

    }
    return FALSE;
}

BOOL CALLBACK UruTOSDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch( uMsg )
    {
    case WM_INITDIALOG:
        {
            SetWindowText(hwndDlg, "End User License Agreement");
            SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon((HINSTANCE)lParam, MAKEINTRESOURCE(IDI_ICON_DIRT)));

            hsUNIXStream stream;
            if (stream.Open("TOS.txt", "rt"))
            {
                uint32_t dataLen = stream.GetSizeLeft();
                char* eulaData = new char[dataLen + 1];
                memset(eulaData, 0, dataLen + 1);
                stream.Read(dataLen, eulaData);

                plString str = plString::Steal(eulaData);
                SetDlgItemTextW(hwndDlg, IDC_URULOGIN_EULATEXT, _TEMP_CONVERT_TO_WCHAR_T(str));
            }
            else // no TOS found, go ahead
                EndDialog(hwndDlg, true);

            break;
        }
    case WM_COMMAND:
        if (HIWORD(wParam) == BN_CLICKED && (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL))
        {
            bool ok = (LOWORD(wParam) == IDOK);
            EndDialog(hwndDlg, ok);
            return TRUE;
        }
        break;

    case WM_NCHITTEST:
        SetWindowLongPtr(hwndDlg, DWL_MSGRESULT, (LONG_PTR)HTCAPTION);
        return TRUE;
    }
    return FALSE;
}

static void SaveUserPass (LoginDialogParam *pLoginParam, char *password)
{
    uint32_t cryptKey[4];
    memset(cryptKey, 0, sizeof(cryptKey));
    GetCryptKey(cryptKey, arrsize(cryptKey));

    plString theUser = _TEMP_CONVERT_FROM_LITERAL(pLoginParam->username);
    plString thePass = (_TEMP_CONVERT_FROM_LITERAL(password)).Left(kMaxPasswordLength);

    // if the password field is the fake string then we've already
    // loaded the namePassHash from the file
    if (thePass.Compare(FAKE_PASS_STRING) != 0)
    {
        wchar_t domain[15];
        PathSplitEmail(_TEMP_CONVERT_TO_WCHAR_T(theUser), nil, 0, domain, arrsize(domain), nil, 0, nil, 0, 0);

        if (StrLen(domain) == 0 || StrCmpI(domain, L"gametap") == 0) {
            plSHA1Checksum shasum(StrLen(password) * sizeof(password[0]), (uint8_t*)password);
            uint32_t* dest = reinterpret_cast<uint32_t*>(pLoginParam->namePassHash);
            const uint32_t* from = reinterpret_cast<const uint32_t*>(shasum.GetValue());

            // I blame eap for this ass shit
            dest[0] = hsToBE32(from[0]);
            dest[1] = hsToBE32(from[1]);
            dest[2] = hsToBE32(from[2]);
            dest[3] = hsToBE32(from[3]);
            dest[4] = hsToBE32(from[4]);
        }
        else
        {
            CryptHashPassword(theUser, thePass, pLoginParam->namePassHash);
        }
    }

    NetCommSetAccountUsernamePassword(_TEMP_CONVERT_TO_WCHAR_T(theUser), pLoginParam->namePassHash);
    if (TGIsCider)
        NetCommSetAuthTokenAndOS(nil, L"mac");
    else
        NetCommSetAuthTokenAndOS(nil, L"win");

    wchar_t fileAndPath[MAX_PATH];
    PathGetInitDirectory(fileAndPath, arrsize(fileAndPath));
    PathAddFilename(fileAndPath, fileAndPath, L"login.dat", arrsize(fileAndPath));
#ifndef PLASMA_EXTERNAL_RELEASE
    // internal builds can use the local init directory
    wchar_t localFileAndPath[MAX_PATH];
    StrCopy(localFileAndPath, L"init\\login.dat", arrsize(localFileAndPath));
    if (PathDoesFileExist(localFileAndPath))
        StrCopy(fileAndPath, localFileAndPath, arrsize(localFileAndPath));
#endif
    hsStream* stream = plEncryptedStream::OpenEncryptedFileWrite(fileAndPath, cryptKey);
    if (stream)
    {
        stream->Write(sizeof(cryptKey), cryptKey);
        stream->WriteSafeString(pLoginParam->username);
        stream->Writebool(pLoginParam->remember);
        if (pLoginParam->remember)
            stream->Write(sizeof(pLoginParam->namePassHash), pLoginParam->namePassHash);
        stream->Close();
        delete stream;
    }
}


static void LoadUserPass (LoginDialogParam *pLoginParam)
{
    uint32_t cryptKey[4];
    ZeroMemory(cryptKey, sizeof(cryptKey));
    GetCryptKey(cryptKey, arrsize(cryptKey));

    char* temp;
    pLoginParam->remember = false;
    pLoginParam->username[0] = '\0';

    wchar_t fileAndPath[MAX_PATH];
    PathGetInitDirectory(fileAndPath, arrsize(fileAndPath));
    PathAddFilename(fileAndPath, fileAndPath, L"login.dat", arrsize(fileAndPath));
#ifndef PLASMA_EXTERNAL_RELEASE
    // internal builds can use the local init directory
    wchar_t localFileAndPath[MAX_PATH];
    StrCopy(localFileAndPath, L"init\\login.dat", arrsize(localFileAndPath));
    if (PathDoesFileExist(localFileAndPath))
        StrCopy(fileAndPath, localFileAndPath, arrsize(localFileAndPath));
#endif
    hsStream* stream = plEncryptedStream::OpenEncryptedFile(fileAndPath, true, cryptKey);
    if (stream && !stream->AtEnd())
    {
        uint32_t savedKey[4];
        stream->Read(sizeof(savedKey), savedKey);

        if (memcmp(cryptKey, savedKey, sizeof(savedKey)) == 0)
        {
            temp = stream->ReadSafeString();

            if (temp)
            {
                StrCopy(pLoginParam->username, temp, kMaxAccountNameLength);
                delete temp;
            }

            pLoginParam->remember = stream->Readbool();

            if (pLoginParam->remember)
            {
                stream->Read(sizeof(pLoginParam->namePassHash), pLoginParam->namePassHash);
                pLoginParam->focus = IDOK;
            }
            else
            {
                pLoginParam->focus = IDC_URULOGIN_PASSWORD;
            }
        }

        stream->Close();
        delete stream;
    }
}

static size_t CurlCallback(void *buffer, size_t size, size_t nmemb, void *param)
{
    static char status[256];

    HWND hwnd = (HWND)param;

    strncpy(status, (const char *)buffer, std::min<size_t>(size * nmemb, 256));
    status[255] = 0;
    PostMessage(hwnd, WM_USER_SETSTATUSMSG, 0, (LPARAM) status);
    return size * nmemb;
}

void StatusCallback(void *param)
{
    HWND hwnd = (HWND)param;

    const char *statusUrl = GetServerStatusUrl();
    CURL *hCurl = curl_easy_init();

    // For reporting errors
    char curlError[CURL_ERROR_SIZE];
    curl_easy_setopt(hCurl, CURLOPT_ERRORBUFFER, curlError);

    while(s_loginDlgRunning)
    {
        curl_easy_setopt(hCurl, CURLOPT_URL, statusUrl);
        curl_easy_setopt(hCurl, CURLOPT_USERAGENT, "UruClient/1.0");
        curl_easy_setopt(hCurl, CURLOPT_WRITEFUNCTION, &CurlCallback);
        curl_easy_setopt(hCurl, CURLOPT_WRITEDATA, param);

        if (statusUrl[0] && curl_easy_perform(hCurl) != 0) // only perform request if there's actually a URL set
            PostMessage(hwnd, WM_USER_SETSTATUSMSG, 0, (LPARAM) curlError);
        
        for(unsigned i = 0; i < UPDATE_STATUSMSG_SECONDS && s_loginDlgRunning; ++i)
        {
            Sleep(1000);
        }
    }

    curl_easy_cleanup(hCurl);

    s_statusEvent.Signal(); // Signal the semaphore
}

BOOL CALLBACK UruLoginDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    static LoginDialogParam* pLoginParam;
    static bool showAuthFailed = false;

    switch( uMsg )
    {
        case WM_INITDIALOG:
        {
            s_loginDlgRunning = true;
            _beginthread(StatusCallback, 0, hwndDlg);
            pLoginParam = (LoginDialogParam*)lParam;

            SetWindowText(hwndDlg, "Login");
            SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(gHInst, MAKEINTRESOURCE(IDI_ICON_DIRT)));

            EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);

            SetDlgItemText(hwndDlg, IDC_URULOGIN_USERNAME, pLoginParam->username);
            CheckDlgButton(hwndDlg, IDC_URULOGIN_REMEMBERPASS, pLoginParam->remember ? BST_CHECKED : BST_UNCHECKED);
            if (pLoginParam->remember)
                SetDlgItemText(hwndDlg, IDC_URULOGIN_PASSWORD, FAKE_PASS_STRING);

            SetFocus(GetDlgItem(hwndDlg, pLoginParam->focus));

            if (IS_NET_ERROR(pLoginParam->authError))
            {
                showAuthFailed = true;
            }

            char windowName[256];
            wchar_t productString[256];
            ProductString(productString, arrsize(productString));
            StrToAnsi(windowName, productString, arrsize(windowName));
            SendMessage(GetDlgItem(hwndDlg, IDC_PRODUCTSTRING), WM_SETTEXT, 0, (LPARAM) windowName);

            for (int i = 0; i < plLocalization::GetNumLocales(); i++)
            {
                SendMessage(GetDlgItem(hwndDlg, IDC_LANGUAGE), CB_ADDSTRING, 0, (LPARAM)plLocalization::GetLanguageName((plLocalization::Language)i));
            }
            SendMessage(GetDlgItem(hwndDlg, IDC_LANGUAGE), CB_SETCURSEL, (WPARAM)plLocalization::GetLanguage(), 0);

            SetTimer(hwndDlg, AUTH_LOGIN_TIMER, 10, NULL);
            return FALSE;
        }

        case WM_USER_SETSTATUSMSG:
             SendMessage(GetDlgItem(hwndDlg, IDC_STATUS_TEXT), WM_SETTEXT, 0, lParam);
             return TRUE;

        case WM_DESTROY:
        {
            s_loginDlgRunning = false;
            s_statusEvent.Wait();
            KillTimer(hwndDlg, AUTH_LOGIN_TIMER);
            return TRUE;
        }
    
        case WM_NCHITTEST:
        {
            SetWindowLongPtr(hwndDlg, DWL_MSGRESULT, (LONG_PTR)HTCAPTION);
            return TRUE;
        }
    
        case WM_PAINT:
        {
            if (showAuthFailed)
            {
                SetTimer(hwndDlg, AUTH_FAILED_TIMER, 10, NULL);
                showAuthFailed = false;
            }
            return FALSE;
        }
    
        case WM_COMMAND:
        {
            if (HIWORD(wParam) == BN_CLICKED && (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL))
            {
                bool ok = (LOWORD(wParam) == IDOK);
                if (ok)
                {
                    char password[kMaxPasswordLength];

                    GetDlgItemText(hwndDlg, IDC_URULOGIN_USERNAME, pLoginParam->username, kMaxAccountNameLength);
                    GetDlgItemText(hwndDlg, IDC_URULOGIN_PASSWORD, password, kMaxPasswordLength);
                    pLoginParam->remember = (IsDlgButtonChecked(hwndDlg, IDC_URULOGIN_REMEMBERPASS) == BST_CHECKED);

                    plLocalization::Language new_language = (plLocalization::Language)SendMessage(GetDlgItem(hwndDlg, IDC_LANGUAGE), CB_GETCURSEL, 0, 0L);
                    plLocalization::SetLanguage(new_language);

                    SaveUserPass (pLoginParam, password);

                    // Welcome to HACKland, population: Branan
                    // The code to write general.ini really doesn't belong here, but it works... for now.
                    // When general.ini gets expanded, this will need to find a proper home somewhere.
                    {
                        wchar_t gipath[MAX_PATH];
                        PathGetInitDirectory(gipath, arrsize(gipath));
                        PathAddFilename(gipath, gipath, L"general.ini", arrsize(gipath));
                        plString ini_str = plString::Format("App.SetLanguage %s\n", plLocalization::GetLanguageName(new_language));
                        hsStream* gini = plEncryptedStream::OpenEncryptedFileWrite(gipath);
                        gini->WriteString(ini_str);
                        gini->Close();
                        delete gini;
                    }

                    memset(&pLoginParam->authError, 0, sizeof(pLoginParam->authError));
                    bool cancelled = AuthenticateNetClientComm(&pLoginParam->authError, hwndDlg);

                    if (IS_NET_SUCCESS(pLoginParam->authError) && !cancelled)
                        EndDialog(hwndDlg, ok);
                    else {
                        if (!cancelled)
                            ::DialogBoxParam(gHInst, MAKEINTRESOURCE( IDD_AUTHFAILED ), hwndDlg, AuthFailedDialogProc, (LPARAM)pLoginParam);
                        else
                        {
                            NetCommDisconnect();
                        }
                    }
                }
                else
                    EndDialog(hwndDlg, ok);

                return TRUE;
            }
            else if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == IDC_URULOGIN_USERNAME)
            {
                char username[kMaxAccountNameLength];
                GetDlgItemText(hwndDlg, IDC_URULOGIN_USERNAME, username, kMaxAccountNameLength);

                if (StrLen(username) == 0)
                    EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
                else
                    EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);

                return TRUE;
            }
            else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_URULOGIN_GAMETAPLINK)
            {
                const char* signupurl = GetServerSignupUrl();
                ShellExecuteA(NULL, "open", signupurl, NULL, NULL, SW_SHOWNORMAL);

                return TRUE;
            }
            break;
        }
    
        case WM_TIMER:
        {
            switch(wParam)
            {
            case AUTH_FAILED_TIMER:
                KillTimer(hwndDlg, AUTH_FAILED_TIMER);
                ::DialogBoxParam(gHInst, MAKEINTRESOURCE( IDD_AUTHFAILED ), hwndDlg, AuthFailedDialogProc, (LPARAM)pLoginParam);
                return TRUE;

            case AUTH_LOGIN_TIMER:
                NetCommUpdate();
                return TRUE;
            }
            return FALSE;
        }
    }
    return FALSE;
}

BOOL CALLBACK SplashDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch( uMsg )
    {
        case WM_INITDIALOG:
            switch (plLocalization::GetLanguage())
            {
                case plLocalization::kFrench:
                    ::SetDlgItemText( hwndDlg, IDC_STARTING_TEXT, "Démarrage d'URU. Veuillez patienter...");
                    break;
                case plLocalization::kGerman:
                    ::SetDlgItemText( hwndDlg, IDC_STARTING_TEXT, "Starte URU, bitte warten ...");
                    break;
/*              case plLocalization::kSpanish:
                    ::SetDlgItemText( hwndDlg, IDC_STARTING_TEXT, "Iniciando URU, por favor espera...");
                    break;
                case plLocalization::kItalian:
                    ::SetDlgItemText( hwndDlg, IDC_STARTING_TEXT, "Avvio di URU, attendere...");
                    break;
*/              // default is English
                case plLocalization::kJapanese:
                    ::SetDlgItemText( hwndDlg, IDC_STARTING_TEXT, "...");
                    break;
                default:
                    ::SetDlgItemText( hwndDlg, IDC_STARTING_TEXT, "Starting URU. Please wait...");
                    break;
            }
            return true; 

    }
    return 0;
}

BOOL CALLBACK ExceptionDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    static char *sLastMsg = nil;

    switch( uMsg )
    {
        case WM_COMMAND:
            EndDialog( hwndDlg, IDOK );
    }
    return 0;
}

#ifndef HS_DEBUGGING
LONG WINAPI plCustomUnhandledExceptionFilter( struct _EXCEPTION_POINTERS *ExceptionInfo )
{
    // Before we do __ANYTHING__, pass the exception to plCrashHandler
    s_crash.ReportCrash(ExceptionInfo);

    // Now, try to create a nice exception dialog after plCrashHandler is done.
    s_crash.WaitForHandle();
    HWND parentHwnd = (gClient == nil) ? GetActiveWindow() : gClient->GetWindowHandle();
    DialogBoxParam(gHInst, MAKEINTRESOURCE(IDD_EXCEPTION), parentHwnd, ExceptionDialogProc, NULL);

    // Trickle up the handlers
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

bool CheckCPU()
{
    const unsigned int sse3_flag = 0x00000001;
    // (any other CPU features...)

    int cpu_info[4];
    __cpuid(cpu_info, 1);
#ifdef HAVE_SSE
    if(cpu_info[2] & sse3_flag == 0)
        return false;
#endif
    // Insert additional feature checks here

    return true;
}

#include "pfConsoleCore/pfConsoleEngine.h"
PF_CONSOLE_LINK_ALL()

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
    // Check to make sure we have a good CPU before getting started
    if (!CheckCPU())
    {
        plString msg = plString::Format("Your processor does not support all of the features required to play %S", ProductLongName());
        hsMessageBox(msg.c_str(), "Error", hsMessageBoxNormal, hsMessageBoxIconError);
        return PARABLE_NORMAL_EXIT;
    }

    PF_CONSOLE_INIT_ALL()

    // Set global handle
    gHInst = hInst;

    CCmdParser cmdParser(s_cmdLineArgs, arrsize(s_cmdLineArgs));
    cmdParser.Parse();

    bool doIntroDialogs = true;
#ifndef PLASMA_EXTERNAL_RELEASE
    if(cmdParser.IsSpecified(kArgSkipLoginDialog))
        doIntroDialogs = false;
    if(cmdParser.IsSpecified(kArgLocalData))
        gDataServerLocal = true;
#endif

    const wchar_t *serverIni = L"server.ini";
    if (cmdParser.IsSpecified(kArgServerIni))
        serverIni = cmdParser.GetString(kArgServerIni);

    // check to see if we were launched from the patcher
    bool eventExists = false;
    // we check to see if the event exists that the patcher should have created
    HANDLE hPatcherEvent = CreateEventW(nil, TRUE, FALSE, L"UruPatcherEvent");
    if (hPatcherEvent != NULL)
    {
        // successfully created it, check to see if it was already created
        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            // it already existed, so the patcher is waiting, signal it so the patcher can die
            SetEvent(hPatcherEvent);
            eventExists = true;
        }
    }

    TGDoCiderDetection ();

#ifdef PLASMA_EXTERNAL_RELEASE
    // if the client was started directly, run the patcher, and shutdown
    STARTUPINFOW si;
    PROCESS_INFORMATION pi; 
    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    si.cb = sizeof(si);

    plStringStream cmdLine;
    const char** addrs;
    
    if (!eventExists) // if it is missing, assume patcher wasn't launched
    {
        cmdLine << _TEMP_CONVERT_FROM_WCHAR_T(s_patcherExeName);

        GetAuthSrvHostnames(&addrs);
        if(strlen(addrs[0]))
            cmdLine << plString::Format(" /AuthSrv=%s", addrs[0]);

        GetFileSrvHostnames(&addrs);
        if(strlen(addrs[0]))
            cmdLine << plString::Format(" /FileSrv=%s", addrs[0]);

        GetGateKeeperSrvHostnames(&addrs);
        if(strlen(addrs[0]))
            cmdLine << plString::Format(" /GateKeeperSrv=%s", addrs[0]);

        if(!CreateProcessW(NULL, (LPWSTR)cmdLine.GetString().ToUtf16().GetData(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        {
            hsMessageBox("Failed to launch patcher", "Error", hsMessageBoxNormal);
        }
        CloseHandle( pi.hThread );
        CloseHandle( pi.hProcess );
        return PARABLE_NORMAL_EXIT;
    }
#endif

    // Load an optional general.ini
    wchar_t gipath[MAX_PATH];
    PathGetInitDirectory(gipath, arrsize(gipath));
    PathAddFilename(gipath, gipath, L"general.ini", arrsize(gipath));
    FILE *generalini = _wfopen(gipath, L"rb");
    if (generalini)
    {
        fclose(generalini);
        pfConsoleEngine tempConsole;
        tempConsole.ExecuteFile(gipath);
    }

    // If another instance is running, exit.  We'll automatically release our
    // lock on the mutex when our process exits
    HANDLE hOneInstance = CreateMutex(nil, FALSE, "UruExplorer");
    if (WaitForSingleObject(hOneInstance,0) != WAIT_OBJECT_0)
    {
        switch (plLocalization::GetLanguage())
        {
            case plLocalization::kFrench:
                hsMessageBox("Une autre copie d'URU est déjà en cours d'exécution", "Erreur", hsMessageBoxNormal);
                break;
            case plLocalization::kGerman:
                hsMessageBox("URU wird bereits in einer anderen Instanz ausgeführt", "Fehler", hsMessageBoxNormal);
                break;
            case plLocalization::kSpanish:
                hsMessageBox("En estos momentos se está ejecutando otra copia de URU", "Error", hsMessageBoxNormal);
                break;
            case plLocalization::kItalian:
                hsMessageBox("Un'altra copia di URU è già aperta", "Errore", hsMessageBoxNormal);
                break;
            // default is English
            default:
                hsMessageBox("Another copy of URU is already running", "Error", hsMessageBoxNormal);
                break;
        }
        return PARABLE_NORMAL_EXIT;
    }

    FILE *serverIniFile = _wfopen(serverIni, L"rb");
    if (serverIniFile)
    {
        fclose(serverIniFile);
        pfConsoleEngine tempConsole;
        tempConsole.ExecuteFile(serverIni);
    }
    else
    {
        hsMessageBox("No server.ini file found.  Please check your URU installation.", "Error", hsMessageBoxNormal);
        return PARABLE_NORMAL_EXIT;
    }

    NetCliAuthAutoReconnectEnable(false);

    NetCommSetReadIniAccountInfo(!doIntroDialogs);
    InitNetClientComm();

    curl_global_init(CURL_GLOBAL_ALL);

    bool                needExit = false;
    LoginDialogParam    loginParam;
    memset(&loginParam, 0, sizeof(loginParam));
    LoadUserPass(&loginParam);

    if (!doIntroDialogs && loginParam.remember) {
        ENetError auth;

        wchar_t wusername[kMaxAccountNameLength];
        StrToUnicode(wusername, loginParam.username, arrsize(wusername));
        NetCommSetAccountUsernamePassword(wusername, loginParam.namePassHash);
        bool cancelled = AuthenticateNetClientComm(&auth, NULL);

        if (IS_NET_ERROR(auth) || cancelled) {
            doIntroDialogs = true;

            loginParam.authError = auth;

            if (cancelled)
            {
                NetCommDisconnect();
            }
        }
    }

    if (doIntroDialogs) {
        if (TGIsCider)
            needExit = !TGRunLoginDialog(&loginParam);
        else if (::DialogBoxParam( hInst, MAKEINTRESOURCE( IDD_URULOGIN_MAIN ), NULL, UruLoginDialogProc, (LPARAM)&loginParam ) <= 0)
            needExit = true;
    }

    if (doIntroDialogs && !needExit) {
        if (TGIsCider)
            needExit = !TGRunTOSDialog ();
        else
        {
            HINSTANCE hRichEdDll = LoadLibrary("RICHED20.DLL");
            INT_PTR val = ::DialogBoxParam( hInst, MAKEINTRESOURCE( IDD_URULOGIN_EULA ), NULL, UruTOSDialogProc, (LPARAM)hInst);
            FreeLibrary(hRichEdDll);
            if (val <= 0) {
                DWORD error = GetLastError();
                needExit = true;
            }
        }
    }

    curl_global_cleanup();

    if (needExit) {
        DeInitNetClientComm();
        return PARABLE_NORMAL_EXIT;
    }

    NetCliAuthAutoReconnectEnable(true);

    // VERY VERY FIRST--throw up our splash screen
    HWND splashDialog = ::CreateDialog( hInst, MAKEINTRESOURCE( IDD_LOADING ), NULL, SplashDialogProc );

    // Install our unhandled exception filter for trapping all those nasty crashes in release build
#ifndef HS_DEBUGGING
    LPTOP_LEVEL_EXCEPTION_FILTER oldFilter;
    oldFilter = SetUnhandledExceptionFilter( plCustomUnhandledExceptionFilter );
#endif

    //
    // Set up to log errors by using hsDebugMessage
    //
    gDebugFile = NULL;
    if ( !plStatusLog::fLoggingOff )
    {
        wchar_t fileAndPath[MAX_PATH];
        PathGetLogDirectory(fileAndPath, arrsize(fileAndPath));
        PathAddFilename(fileAndPath, fileAndPath, L"plasmalog.txt", arrsize(fileAndPath));
        gDebugFile = _wfopen(fileAndPath, L"wt");
        hsAssert(gDebugFile != NULL, "Error creating debug file plasmalog.txt");
        hsSetDebugMessageProc(DebugMessageProc);
        if (gDebugFile != NULL)
        {
            char prdName[256];
            wchar_t prdString[256];
            ProductString(prdString, arrsize(prdString));
            StrToAnsi(prdName, prdString, arrsize(prdName));
            fprintf(gDebugFile, "%s\n", prdName);
            fflush(gDebugFile);
        }
    }

    for (;;) {
        // Create Window
        if (!WinInit(hInst, nCmdShow) || gClient->GetDone())
            break;

        // We don't have multiplayer localized assets for Italian or Spanish, so force them to English in that case.
    /*  if (!plNetClientMgr::GetInstance()->InOfflineMode() &&
            (plLocalization::GetLanguage() == plLocalization::kItalian || 
            plLocalization::GetLanguage() == plLocalization::kSpanish))
        {
            plLocalization::SetLanguage(plLocalization::kEnglish);
        }
    */

        // Done with our splash now
        ::DestroyWindow( splashDialog );

        if (!gClient)
            break;

        // Show the main window
        ShowWindow(gClient->GetWindowHandle(), SW_SHOW);

        gHasMouse = GetSystemMetrics(SM_MOUSEPRESENT);
            
        // Be really REALLY forceful about being in the front
        BringWindowToTop( gClient->GetWindowHandle() );

        // Update the window
        UpdateWindow(gClient->GetWindowHandle());

        // 
        // Init Application here
        //
        if( !gClient->StartInit() )
            break;
        
        // I want it on top! I mean it!
        BringWindowToTop( gClient->GetWindowHandle() );

        // initialize dinput here:
        if (gClient && gClient->GetInputManager())
            gClient->GetInputManager()->InitDInput(hInst, (HWND)gClient->GetWindowHandle());
        
        // Seriously!
        BringWindowToTop( gClient->GetWindowHandle() );
        
        //
        // Main loop
        //
        MSG msg;
        do
        {   
            gClient->MainLoop();

            if( gClient->GetDone() )
                break;

            // Look for a message
            while (PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ))
            {
                // Handle the message
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }
        } while (WM_QUIT != msg.message);

        break;
    }

#ifndef _DEBUG
    try
    {
#endif
        // 
        // Cleanup
        //
        if (gClient)
        {
            gClient->Shutdown(); // shuts down PhysX for us
            gClient = nil;
        }
        hsAssert(hsgResMgr::ResMgr()->RefCnt()==1, "resMgr has too many refs, expect mem leaks");
        hsgResMgr::Shutdown();  // deletes fResMgr
        DeInitNetClientComm();
#ifndef _DEBUG
    } catch (...)
    {
        // just catch all the crashes on exit... just to keep GameTap from complaining
        if (gDebugFile)
            fprintf(gDebugFile, "Crashed on shutdown.\n");
    }
#endif

    if (gDebugFile)
        fclose(gDebugFile);

    // Uninstall our unhandled exception filter, if we installed one
#ifndef HS_DEBUGGING
    SetUnhandledExceptionFilter( oldFilter );
#endif

    // Exit WinMain and terminate the app....
//    return msg.wParam;
    return PARABLE_NORMAL_EXIT;
}

static void GetCryptKey(uint32_t* cryptKey, unsigned numElements)
{
    char volName[] = "C:\\";
    int index = 0;
    DWORD logicalDrives = GetLogicalDrives();

    for (int i = 0; i < 32; ++i)
    {
        if (logicalDrives & (1 << i))
        {
            volName[0] = ('C' + i);

            DWORD volSerialNum = 0;
            BOOL result = GetVolumeInformation(
                volName,        //LPCTSTR lpRootPathName,
                NULL,           //LPTSTR lpVolumeNameBuffer,
                0,              //DWORD nVolumeNameSize,
                &volSerialNum,  //LPDWORD lpVolumeSerialNumber,
                NULL,           //LPDWORD lpMaximumComponentLength,
                NULL,           //LPDWORD lpFileSystemFlags,
                NULL,           //LPTSTR lpFileSystemNameBuffer,
                0               //DWORD nFileSystemNameSize
            );

            cryptKey[index] = (cryptKey[index] ^ volSerialNum);

            index = (++index) % numElements;
        }
    }
}

