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

#include "HeadSpin.h"
#include "plCmdParser.h"
#include "hsDebug.h"
#include "hsEndian.h"
#include "plPipeline.h"
#include "plProduct.h"
#include "hsStream.h"
#include "hsThread.h"
#include "hsWindows.h"

#include <process.h>
#include <shellapi.h>   // ShellExecuteA
#include <shlobj.h>
#include <algorithm>
#include <regex>
#include <unordered_set>

#include <curl/curl.h>

#include "plClient.h"
#include "plClientLoader.h"
#include "res/resource.h"

#include "pnEncryption/plChallengeHash.h"
#include "pnNetBase/pnNbSrvs.h"

#include "plFile/plEncryptedStream.h"
#include "plInputCore/plInputDevice.h"
#include "plInputCore/plInputManager.h"
#include "plNetClient/plNetClientMgr.h"
#include "plNetGameLib/plNetGameLib.h"
#include "plMessage/plDisplayScaleChangedMsg.h"
#include "plMessageBox/hsMessageBox.h"
#include "plPhysX/plPXSimulation.h"
#include "plPipeline/hsG3DDeviceSelector.h"
#include "plResMgr/plLocalization.h"
#include "plResMgr/plResManager.h"
#include "plResMgr/plVersion.h"
#include "plStatusLog/plStatusLog.h"
#include "plWinDpi/plWinDpi.h"

#include "pfConsoleCore/pfConsoleEngine.h"
#include "pfConsoleCore/pfServerIni.h"
#include "pfCrashHandler/plCrashCli.h"
#include "pfDisplayHelpers/plWinDisplayHelper.h"
#include "pfPasswordStore/pfPasswordStore.h"

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
ITaskbarList3* gTaskbarList = nullptr; // NT 6.1+ taskbar stuff

extern bool gDataServerLocal;
extern bool gPythonLocal;
extern bool gSDLLocal;

enum
{
    kArgSkipLoginDialog,
    kArgServerIni,
    kArgLocalData,
    kArgLocalPython,
    kArgLocalSDL,
    kArgPlayerId,
    kArgStartUpAgeName,
    kArgPvdFile,
    kArgSkipIntroMovies,
    kArgRenderer
};

static const plCmdArgDef s_cmdLineArgs[] = {
    { kCmdArgFlagged  | kCmdTypeBool,       "SkipLoginDialog", kArgSkipLoginDialog },
    { kCmdArgFlagged  | kCmdTypeString,     "ServerIni",       kArgServerIni },
    { kCmdArgFlagged  | kCmdTypeBool,       "LocalData",       kArgLocalData   },
    { kCmdArgFlagged  | kCmdTypeBool,       "LocalPython",     kArgLocalPython },
    { kCmdArgFlagged  | kCmdTypeBool,       "LocalSDL",        kArgLocalSDL },
    { kCmdArgFlagged  | kCmdTypeInt,        "PlayerId",        kArgPlayerId },
    { kCmdArgFlagged  | kCmdTypeString,     "Age",             kArgStartUpAgeName },
    { kCmdArgFlagged  | kCmdTypeString,     "PvdFile",         kArgPvdFile },
    { kCmdArgFlagged  | kCmdTypeBool,       "SkipIntroMovies", kArgSkipIntroMovies },
    { kCmdArgFlagged  | kCmdTypeString,     "Renderer",        kArgRenderer },
};

plClientLoader  gClient;
bool            gPendingActivate = false;
bool            gPendingActivateFlag = false;

#ifndef HS_DEBUGGING
static plCrashCli s_crash;
#endif

static std::atomic<bool>  s_loginDlgRunning(false);
static std::thread      s_statusThread;
static UINT             s_WmTaskbarList = RegisterWindowMessage("TaskbarButtonCreated");

FILE *errFP = nullptr;
HINSTANCE               gHInst = nullptr;      // Instance of this app

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
// LoginDialogParam
//============================================================================
struct LoginDialogParam {
    ENetError   authError;
    wchar_t     username[kMaxAccountNameLength];
    ShaDigest   namePassHash;
    bool        remember;
    int         focus;
};

static bool AuthenticateNetClientComm(ENetError* result, HWND parentWnd);
static void SaveUserPass (LoginDialogParam *pLoginParam, char *password);
static void LoadUserPass (LoginDialogParam *pLoginParam);
static void AuthFailedStrings (ENetError authError,
                                         const char **ppStr1, const char **ppStr2,
                                         const wchar_t **ppWStr);

void DebugMsgF(const char* format, ...);

static void HandleDpiChange(HWND hWnd, UINT dpi, float scale, const RECT& rect)
{
    // Inform the engine about the new DPI.
    auto* msg = new plDisplayScaleChangedMsg(scale, plDisplayScaleChangedMsg::ConvertRect(rect));
    msg->Send();
}

// Handles all the windows messages we might receive
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static bool gDragging = false;
    static uint8_t mouse_down = 0;

    // DPI Helper can eat messages.
    auto result = plWinDpi::Instance().WndProc(hWnd, message, wParam, lParam, HandleDpiChange);
    if (result.has_value())
        return result.value();

    // Messages we registered for manually (no const value)
    if (message == s_WmTaskbarList)
    {
        hsRequireCOM();

        // Grab the Windows 7 taskbar list stuff
        if (gTaskbarList)
            gTaskbarList->Release();
        HRESULT result = CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_ALL, IID_ITaskbarList3, (void**)&gTaskbarList);
        if (FAILED(result))
            gTaskbarList = nullptr;
        return 0;
    }

    // Handle messages
    switch (message) {
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        case WM_MBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
            // Ensure we don't leave the client area during clicks
            if (!(mouse_down++))
                SetCapture(hWnd);
            // fall through to old case
        case WM_KEYDOWN:
        case WM_CHAR:
            // If they did anything but move the mouse, quit any intro movie playing.
            if (gClient)
            {
                gClient->SetQuitIntro(true);

                // normal input processing
                if (gClient->WindowActive() && gClient->GetInputManager())
                    gClient->GetInputManager()->HandleWin32ControlEvent(message, wParam, lParam, hWnd);
            }
            break;
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
            // Stop hogging the cursor
            if (!(--mouse_down))
                ReleaseCapture();
            // fall through to input processing
        case WM_MOUSEWHEEL:
        case WM_KEYUP:
           if (gClient && gClient->WindowActive() && gClient->GetInputManager())
               gClient->GetInputManager()->HandleWin32ControlEvent(message, wParam, lParam, hWnd);
            break;

        case WM_MOUSEMOVE:
            {
                if (gClient && gClient->GetInputManager())
                    gClient->GetInputManager()->HandleWin32ControlEvent(message, wParam, lParam, hWnd);
            }
            break;

        case WM_SYSKEYUP:
        case WM_SYSKEYDOWN:
            {
                if (gClient && gClient->WindowActive() && gClient->GetInputManager())
                {
                    gClient->GetInputManager()->HandleWin32ControlEvent(message, wParam, lParam, hWnd);
                }
                //DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
            
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
                        mgr->QueueDisableNet(false, nullptr);
                    DestroyWindow(gClient->GetWindowHandle());
                    break;
            }
            break;

        case WM_SETCURSOR:
            {
                static bool winCursor = true;
                bool enterWnd = LOWORD(lParam) == HTCLIENT;
                if (enterWnd && winCursor)
                {
                    winCursor = !winCursor;
                    ShowCursor(winCursor != 0);
                    plMouseDevice::ShowCursor();
                }
                else if (!enterWnd && !winCursor)
                {
                    winCursor = !winCursor;
                    ShowCursor(winCursor != 0);
                    plMouseDevice::HideCursor();
                }
                return TRUE;
            }
            break;

        case WM_ACTIVATE:
            {
                bool active = (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE);
                bool minimized = (HIWORD(wParam) != 0);

                if (gClient && !minimized && !gClient->GetDone())
                {
                    gClient->WindowActivate(active);
                }
                else
                {
                    gPendingActivate = true;
                    gPendingActivateFlag = active;
                }
            }
            break;

        // Let go of the mouse if the window is being moved.
        case WM_ENTERSIZEMOVE:
            gDragging = true;
            if( gClient )
                gClient->WindowActivate(false);
            break;

        // Redo the mouse capture if the window gets moved
        case WM_EXITSIZEMOVE:
            gDragging = false;
            if( gClient )
                gClient->WindowActivate(true);
            break;

        // Redo the mouse capture if the window gets moved (special case for Colin
        // and his cool program that bumps windows out from under the taskbar)
        case WM_MOVE:
            if (!gDragging && gClient && gClient->GetInputManager())
                gClient->GetInputManager()->Activate(true);
            break;

        /// Resize the window
        // (we do WM_SIZING here instead of WM_SIZE because, for some reason, WM_SIZE is
        //  sent to the window when we do fullscreen, and what's more, it gets sent BEFORE
        //  the fullscreen flag is sent. How does *that* happen? Anyway, WM_SIZING acts
        //  just like WM_SIZE, except that it ONLY gets sent when the user changes the window
        //  size, not when the window is minimized or restored)
        case WM_SIZING:
            {
                RECT r;
                ::GetClientRect(hWnd, &r);
                gClient->GetPipeline()->Resize(r.right - r.left, r.bottom - r.top);
                gClient->GetPipeline()->SetBackingScale(plWinDpi::Instance().GetScale(hWnd));
            }
            break;

        case WM_SIZE:
            // Let go of the mouse if the window is being minimized
            if (wParam == SIZE_MINIMIZED)
            {
                if (gClient)
                    gClient->WindowActivate(false);
            }
            // Redo the mouse capture if the window gets restored
            else if (wParam == SIZE_RESTORED)
            {
                if (gClient)
                    gClient->WindowActivate(true);
            }
            break;
        
        case WM_CLOSE:
            gClient.ShutdownStart();
            DestroyWindow(gClient->GetWindowHandle());
            break;
        case WM_DESTROY:
            gClient.ShutdownStart();
            PostQuitMessage(0);
            break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}
 
void    PumpMessageQueueProc()
{
    MSG msg;

    // Look for a message
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
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

INT_PTR CALLBACK AuthDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    static bool* cancelled = nullptr;

    switch( uMsg )
    {
    case WM_INITDIALOG:
        cancelled = (bool*)lParam;
        SetTimer(hwndDlg, AUTH_LOGIN_TIMER, 10, nullptr);
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
        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LONG_PTR)HTCAPTION);
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
    NetCommAuthenticate(nullptr);

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

//
// For error logging
//
static plStatusLog* s_DebugLog = nullptr;

static void _StatusMessageProc(const ST::string& msg)
{
#if defined(HS_DEBUGGING) || !defined(PLASMA_EXTERNAL_RELEASE)
    s_DebugLog->AddLine(msg);
#endif // defined(HS_DEBUGGING) || !defined(PLASMA_EXTERNAL_RELEASE)
}

static void DebugInit()
{
#if defined(HS_DEBUGGING) || !defined(PLASMA_EXTERNAL_RELEASE)
    plStatusLogMgr& mgr = plStatusLogMgr::GetInstance();
    s_DebugLog = mgr.CreateStatusLog(30, "plasmadbg.log", plStatusLog::kFilledBackground |
                 plStatusLog::kDeleteForMe | plStatusLog::kAlignToTop | plStatusLog::kTimestamp);
    hsSetStatusMessageProc(_StatusMessageProc);
#endif // defined(HS_DEBUGGING) || !defined(PLASMA_EXTERNAL_RELEASE)
}

static void AuthFailedStrings (ENetError authError,
                                         const char **ppStr1, const char **ppStr2,
                                         const wchar_t **ppWStr)
{
  *ppStr1 = nullptr;
  *ppStr2 = nullptr;
  *ppWStr = nullptr;

    switch (plLocalization::GetLanguage())
    {
        case plLocalization::kEnglish:
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


INT_PTR CALLBACK AuthFailedDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
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
            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LONG_PTR)HTCAPTION);
            return TRUE;

    }
    return FALSE;
}

INT_PTR CALLBACK UruTOSDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
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
                ST::char_buffer eula;
                eula.allocate(dataLen);
                stream.Read(dataLen, eula.data());

                SetDlgItemTextW(hwndDlg, IDC_URULOGIN_EULATEXT,
                                ST::string(eula, ST::substitute_invalid).to_wchar().data());
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
        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LONG_PTR)HTCAPTION);
        return TRUE;
    }
    return FALSE;
}

static void StoreHash(const ST::string& username, const ST::string& password, LoginDialogParam *pLoginParam)
{
    //  Hash username and password before sending over the 'net.
    //  -- Legacy compatibility: @gametap (and other usernames with domains in them) need
    //     to be hashed differently.
    static const std::regex re_domain("[^@]+@([^.]+\\.)*([^.]+)\\.[^.]+");
    std::cmatch match;
    std::regex_search(username.c_str(), match, re_domain);
    if (match.empty() || ST::string(match[2].str()).compare_i("gametap") == 0) {
        //  Plain Usernames...
        plSHA1Checksum shasum(password.size(), reinterpret_cast<const uint8_t*>(password.c_str()));
        uint32_t* dest = reinterpret_cast<uint32_t*>(pLoginParam->namePassHash);
        const uint32_t* from = reinterpret_cast<const uint32_t*>(shasum.GetValue());

        dest[0] = hsToBE32(from[0]);
        dest[1] = hsToBE32(from[1]);
        dest[2] = hsToBE32(from[2]);
        dest[3] = hsToBE32(from[3]);
        dest[4] = hsToBE32(from[4]);
    }
    else {
        //  Domain-based Usernames...
        CryptHashPassword(username, password, pLoginParam->namePassHash);
    }
}

static void SaveUserPass(LoginDialogParam* pLoginParam, wchar_t* password)
{
    ST::string theUser = pLoginParam->username;
    ST::string thePass = password;

    HKEY hKey;
    RegCreateKeyEx(HKEY_CURRENT_USER, ST::format("Software\\Cyan, Inc.\\{}\\{}", plProduct::LongName(), GetServerDisplayName()).c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr);
    RegSetValueExW(hKey, L"LastAccountName", 0, REG_SZ, (LPBYTE) pLoginParam->username, sizeof(pLoginParam->username));
    uint32_t rememberAccount = pLoginParam->remember;
    RegSetValueExW(hKey, L"RememberPassword", 0, REG_DWORD, (LPBYTE) &rememberAccount, sizeof(rememberAccount));
    RegCloseKey(hKey);

    // If the password field is the fake string
    // then we've already loaded the hash.
    if (thePass.compare(FAKE_PASS_STRING) != 0)
    {
        StoreHash(theUser, thePass, pLoginParam);

        pfPasswordStore* store = pfPasswordStore::Instance();
        if (pLoginParam->remember)
            store->SetPassword(pLoginParam->username, thePass);
        else
            store->SetPassword(pLoginParam->username, ST::string());
    }

    NetCommSetAccountUsernamePassword(theUser, pLoginParam->namePassHash);

    // FIXME: Real OS detection
    NetCommSetAuthTokenAndOS(nullptr, u"win");
}

static void LoadUserPass(LoginDialogParam *pLoginParam)
{
    HKEY hKey;
    wchar_t accountName[kMaxAccountNameLength];
    memset(accountName, 0, sizeof(accountName));
    uint32_t rememberAccount = 0;
    DWORD acctLen = sizeof(accountName), remLen = sizeof(rememberAccount);
    RegOpenKeyEx(HKEY_CURRENT_USER, ST::format("Software\\Cyan, Inc.\\{}\\{}", plProduct::LongName(), GetServerDisplayName()).c_str(), 0, KEY_QUERY_VALUE, &hKey);
    RegQueryValueExW(hKey, L"LastAccountName", nullptr, nullptr, (LPBYTE) &accountName, &acctLen);
    RegQueryValueExW(hKey, L"RememberPassword", nullptr, nullptr, (LPBYTE) &rememberAccount, &remLen);
    RegCloseKey(hKey);

    pLoginParam->remember = false;
    pLoginParam->username[0] = 0;

    if (acctLen > 0)
        wcsncpy(pLoginParam->username, accountName, kMaxAccountNameLength);
    pLoginParam->remember = (rememberAccount != 0);
    if (pLoginParam->remember && pLoginParam->username[0])
    {
        pfPasswordStore* store = pfPasswordStore::Instance();
        ST::string password = store->GetPassword(pLoginParam->username);
        if (!password.empty())
            StoreHash(pLoginParam->username, password, pLoginParam);
        pLoginParam->focus = IDOK;
    }
    else if (!pLoginParam->username[0])
        pLoginParam->focus = IDC_URULOGIN_USERNAME;
    else
        pLoginParam->focus = IDC_URULOGIN_PASSWORD;
}

static size_t CurlCallback(void *buffer, size_t size, size_t nmemb, void *param)
{
    static char status[256];

    HWND hwnd = (HWND)param;

    size_t count = std::min<size_t>(size * nmemb, std::size(status) - 1);
    memcpy(status, buffer, count);
    status[count] = 0;
    PostMessage(hwnd, WM_USER_SETSTATUSMSG, 0, (LPARAM) status);
    return size * nmemb;
}

INT_PTR CALLBACK UruLoginDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    static LoginDialogParam* pLoginParam;
    static bool showAuthFailed = false;

    switch( uMsg )
    {
        case WM_INITDIALOG:
        {
            s_loginDlgRunning = true;
            s_statusThread = hsThread::StartSimpleThread([hwndDlg] {
                hsThread::SetThisThreadName(ST_LITERAL("LoginDialogShardStatus"));

                ST::string statusUrl = GetServerStatusUrl();
                CURL* hCurl = curl_easy_init();

                // For reporting errors
                char curlError[CURL_ERROR_SIZE];
                curl_easy_setopt(hCurl, CURLOPT_ERRORBUFFER, curlError);
                curl_easy_setopt(hCurl, CURLOPT_FOLLOWLOCATION, 1);
                curl_easy_setopt(hCurl, CURLOPT_MAXREDIRS, 5);

                while (s_loginDlgRunning) {
                    curl_easy_setopt(hCurl, CURLOPT_URL, statusUrl.c_str());
                    curl_easy_setopt(hCurl, CURLOPT_USERAGENT, "UruClient/1.0");
                    curl_easy_setopt(hCurl, CURLOPT_WRITEFUNCTION, &CurlCallback);
                    curl_easy_setopt(hCurl, CURLOPT_WRITEDATA, hwndDlg);

                    if (!statusUrl.empty() && curl_easy_perform(hCurl) != 0) {
                        // only perform request if there's actually a URL set
                        PostMessage(hwndDlg, WM_USER_SETSTATUSMSG, 0,
                                    reinterpret_cast<LPARAM>(curlError));
                    }

                    for (unsigned i = 0; i < UPDATE_STATUSMSG_SECONDS && s_loginDlgRunning; ++i)
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                }

                curl_easy_cleanup(hCurl);
            });
            pLoginParam = (LoginDialogParam*)lParam;

            SetWindowText(hwndDlg, "Login");
            SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(gHInst, MAKEINTRESOURCE(IDI_ICON_DIRT)));

            EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);

            SetDlgItemTextW(hwndDlg, IDC_URULOGIN_USERNAME, pLoginParam->username);
            CheckDlgButton(hwndDlg, IDC_URULOGIN_REMEMBERPASS, pLoginParam->remember ? BST_CHECKED : BST_UNCHECKED);
            if (pLoginParam->remember)
                SetDlgItemText(hwndDlg, IDC_URULOGIN_PASSWORD, FAKE_PASS_STRING);

            SetFocus(GetDlgItem(hwndDlg, pLoginParam->focus));

            if (IS_NET_ERROR(pLoginParam->authError))
            {
                showAuthFailed = true;
            }

            SendMessage(GetDlgItem(hwndDlg, IDC_PRODUCTSTRING), WM_SETTEXT, 0,
                        (LPARAM)plProduct::ProductString().c_str());

            for (auto lang : plLocalization::GetAllLanguages()) {
                ST::string langName = plLocalization::GetLanguageName(lang);
                if (!plLocalization::IsLanguageUsable(lang)) {
#if defined(PLASMA_EXTERNAL_RELEASE)
                    // External clients only allow selecting usable languages.
                    continue;
#else
                    // Internal clients allow choosing unsupported languages as well.
                    langName += ST_LITERAL(" (unsupported)");
#endif
                }
                SendMessageW(GetDlgItem(hwndDlg, IDC_LANGUAGE), CB_ADDSTRING, 0, (LPARAM)langName.to_wchar().c_str());
            }
            SendMessage(GetDlgItem(hwndDlg, IDC_LANGUAGE), CB_SETCURSEL, (WPARAM)plLocalization::GetLanguage(), 0);

            EnableWindow(GetDlgItem(hwndDlg, IDC_URULOGIN_NEWACCTLINK), !GetServerSignupUrl().empty());

            SetTimer(hwndDlg, AUTH_LOGIN_TIMER, 10, nullptr);
            return FALSE;
        }

        case WM_USER_SETSTATUSMSG:
             SendMessage(GetDlgItem(hwndDlg, IDC_STATUS_TEXT), WM_SETTEXT, 0, lParam);
             return TRUE;

        case WM_DESTROY:
        {
            s_loginDlgRunning = false;
            s_statusThread.join();
            KillTimer(hwndDlg, AUTH_LOGIN_TIMER);
            return TRUE;
        }
    
        case WM_NCHITTEST:
        {
            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LONG_PTR)HTCAPTION);
            return TRUE;
        }
    
        case WM_PAINT:
        {
            if (showAuthFailed)
            {
                SetTimer(hwndDlg, AUTH_FAILED_TIMER, 10, nullptr);
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
                    wchar_t password[kMaxPasswordLength];

                    GetDlgItemTextW(hwndDlg, IDC_URULOGIN_USERNAME, pLoginParam->username, kMaxAccountNameLength);
                    GetDlgItemTextW(hwndDlg, IDC_URULOGIN_PASSWORD, password, kMaxPasswordLength);
                    pLoginParam->remember = (IsDlgButtonChecked(hwndDlg, IDC_URULOGIN_REMEMBERPASS) == BST_CHECKED);

                    plLocalization::Language new_language = (plLocalization::Language)SendMessage(GetDlgItem(hwndDlg, IDC_LANGUAGE), CB_GETCURSEL, 0, 0L);
                    plLocalization::SetLanguage(new_language);

                    SaveUserPass (pLoginParam, password);

                    // Welcome to HACKland, population: Branan
                    // The code to write general.ini really doesn't belong here, but it works... for now.
                    // When general.ini gets expanded, this will need to find a proper home somewhere.
                    {
                        plFileName gipath = plFileName::Join(plFileSystem::GetInitPath(), "general.ini");
                        ST::string ini_str = ST::format("App.SetLanguage {}\n", plLocalization::GetLanguageName(new_language));
                        std::unique_ptr<hsStream> gini = plEncryptedStream::OpenEncryptedFileWrite(gipath);
                        if (gini) {
                            gini->WriteString(ini_str);
                        }
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
                wchar_t username[kMaxAccountNameLength];
                GetDlgItemTextW(hwndDlg, IDC_URULOGIN_USERNAME, username, kMaxAccountNameLength);
                EnableWindow(GetDlgItem(hwndDlg, IDOK), username[0] != 0);
                return TRUE;
            }
            else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_URULOGIN_NEWACCTLINK)
            {
                ST::string signupurl = GetServerSignupUrl();
                ShellExecuteW(nullptr, L"open", signupurl.to_wchar().data(), nullptr, nullptr, SW_SHOWNORMAL);

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

INT_PTR CALLBACK SplashDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        ST::string message;
        switch (plLocalization::GetLanguage())
        {
        case plLocalization::kFrench:
            message = ST_LITERAL("Démarrage d'URU. Veuillez patienter...");
            break;
        case plLocalization::kGerman:
            message = ST_LITERAL("Starte URU, bitte warten ...");
            break;
        case plLocalization::kSpanish:
            message = ST_LITERAL("Iniciando URU, por favor espera...");
            break;
        case plLocalization::kItalian:
            message = ST_LITERAL("Avvio di URU, attendere...");
            break;
        case plLocalization::kRussian:
            message = ST_LITERAL("Запуск URU. Пожалуйста, подождите...");
            break;
            // default is English
        default:
            message = ST_LITERAL("Starting URU. Please wait...");
            break;
        }
        SetDlgItemTextW(hwndDlg, IDC_STARTING_TEXT, message.to_wchar().c_str());
        return true;

    }
    return 0;
}

INT_PTR CALLBACK ExceptionDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    static char *sLastMsg = nullptr;

    switch( uMsg )
    {
        case WM_COMMAND:
            EndDialog( hwndDlg, IDOK );
    }
    return 0;
}

LONG WINAPI plCustomUnhandledExceptionFilter( struct _EXCEPTION_POINTERS *ExceptionInfo )
{
#ifndef HS_DEBUGGING
    // Before we do __ANYTHING__, pass the exception to plCrashHandler
    s_crash.ReportCrash(ExceptionInfo);

    // For maximum safety, the crash handler process will do all of the GUI work,
    // however, if that seems to take too long, then we'll just have to assume life
    // is not going well in plCrashHandler land and show the crappy "welp, we died"
    // dialog.
    if (gClient)
        ShowWindow(gClient->GetWindowHandle(), SW_HIDE);
    if (!s_crash.WaitForHandle())
        DialogBoxParamW(gHInst, MAKEINTRESOURCEW(IDD_EXCEPTION), nullptr, ExceptionDialogProc, 0L);

    // Means that we have handled this.
    return EXCEPTION_EXECUTE_HANDLER;
#else
    // This allows the CRT level __except statement to handle the crash, allowing the debugger to be attached.
    return EXCEPTION_CONTINUE_SEARCH;
#endif // HS_DEBUGGING
}

uint32_t ParseRendererArgument(const ST::string& requested)
{
    using namespace ST::literals;

    static std::unordered_map<ST::string, uint32_t, ST::hash_i, ST::equal_i> args{
        {"directx"_st, hsG3DDeviceSelector::kDevTypeDirect3D},
        {"direct3d"_st, hsG3DDeviceSelector::kDevTypeDirect3D},
        {"dx"_st, hsG3DDeviceSelector::kDevTypeDirect3D},
        {"d3d"_st, hsG3DDeviceSelector::kDevTypeDirect3D},
        {"opengl"_st, hsG3DDeviceSelector::kDevTypeOpenGL},
        {"gl"_st, hsG3DDeviceSelector::kDevTypeOpenGL}};

    auto it = args.find(requested);
    if (it != args.end())
        return it->second;
    return hsG3DDeviceSelector::kDevTypeUnknown;
}

PF_CONSOLE_LINK_ALL()

bool WinInit(HINSTANCE hInst)
{
    // Initialize the DPI helpers
    plWinDpi::Instance();

    // Fill out WNDCLASS info
    WNDCLASS wndClass;
    wndClass.style = CS_DBLCLKS;   // CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = WndProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = hInst;
    wndClass.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_DIRT));

    wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wndClass.hbrBackground = (struct HBRUSH__*) (GetStockObject(BLACK_BRUSH));
    wndClass.lpszMenuName = CLASSNAME;
    wndClass.lpszClassName = CLASSNAME;

    // can only run one at a time anyway, so just quit if another is running
    if (!RegisterClass(&wndClass))
        return false;

    int winBorderDX = plWinDpi::Instance().GetSystemMetrics(SM_CXSIZEFRAME);
    int winBorderDY = plWinDpi::Instance().GetSystemMetrics(SM_CYSIZEFRAME);
    int winMenuDY = plWinDpi::Instance().GetSystemMetrics(SM_CYCAPTION);

    // Create a window
    HWND hWnd = CreateWindow(
        CLASSNAME, plProduct::LongName().c_str(),
        WS_OVERLAPPEDWINDOW,
        0, 0,
        800 + winBorderDX * 2,
        600 + winBorderDY * 2 + winMenuDY,
        nullptr, nullptr, hInst, nullptr
        );
    HDC hDC = GetDC(hWnd);

    plDisplayHelper::SetInstance(new plWinDisplayHelper());

    gClient.SetClientWindow((hsWindowHndl)hWnd);
    gClient.SetClientDisplay((hsWindowHndl)hDC);
    gClient.Init();
    return true;
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPWSTR lpCmdLine, int nCmdShow)
{
    PF_CONSOLE_INIT_ALL()

    // Set global handle
    gHInst = hInst;

    std::vector<ST::string> args;
    args.reserve(__argc);
    for (size_t i = 0; i < __argc; i++) {
        args.push_back(ST::string::from_wchar(__wargv[i]));
    }

    plCmdParser cmdParser(s_cmdLineArgs, std::size(s_cmdLineArgs));
    cmdParser.Parse(args);

    bool doIntroDialogs = true;
#ifndef PLASMA_EXTERNAL_RELEASE
    if (cmdParser.IsSpecified(kArgSkipLoginDialog))
        doIntroDialogs = false;
    if (cmdParser.IsSpecified(kArgLocalData)) {
        gDataServerLocal = true;
        gPythonLocal = true;
    }
    if (cmdParser.IsSpecified(kArgLocalPython))
        gPythonLocal = true;
    if (cmdParser.IsSpecified(kArgLocalSDL))
        gSDLLocal = true;
    if (cmdParser.IsSpecified(kArgPlayerId))
        NetCommSetIniPlayerId(cmdParser.GetInt(kArgPlayerId));
    if (cmdParser.IsSpecified(kArgStartUpAgeName))
        NetCommSetIniStartUpAge(cmdParser.GetString(kArgStartUpAgeName));
    if (cmdParser.IsSpecified(kArgPvdFile))
        plPXSimulation::SetDefaultDebuggerEndpoint(cmdParser.GetString(kArgPvdFile));
    if (cmdParser.IsSpecified(kArgRenderer))
        gClient.SetRequestedRenderingBackend(ParseRendererArgument(cmdParser.GetString(kArgRenderer)));
#endif

    plFileName serverIni = "server.ini";
    if (cmdParser.IsSpecified(kArgServerIni))
        serverIni = cmdParser.GetString(kArgServerIni);

    // check to see if we were launched from the patcher
    bool eventExists = false;
    // we check to see if the event exists that the patcher should have created
    HANDLE hPatcherEvent = CreateEventW(nullptr, TRUE, FALSE, L"UruPatcherEvent");
    if (hPatcherEvent != nullptr)
    {
        // successfully created it, check to see if it was already created
        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            // it already existed, so the patcher is waiting, signal it so the patcher can die
            SetEvent(hPatcherEvent);
            eventExists = true;
        }
    }

#ifdef PLASMA_EXTERNAL_RELEASE
    // if the client was started directly, run the patcher, and shutdown
    STARTUPINFOW si;
    PROCESS_INFORMATION pi; 
    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    si.cb = sizeof(si);

    if (!eventExists) // if it is missing, assume patcher wasn't launched
    {
        if(!CreateProcessW(s_patcherExeName, nullptr, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
        {
            hsMessageBox(ST_LITERAL("Failed to launch patcher"), ST_LITERAL("Error"), hsMessageBoxNormal);
        }
        CloseHandle( pi.hThread );
        CloseHandle( pi.hProcess );
        return PARABLE_NORMAL_EXIT;
    }
#endif

    // Load an optional general.ini
    plFileName gipath = plFileName::Join(plFileSystem::GetInitPath(), "general.ini");
    FILE *generalini = plFileSystem::Open(gipath, "rb");
    if (generalini)
    {
        fclose(generalini);
        pfConsoleEngine tempConsole;
        tempConsole.ExecuteFile(gipath);
    }

#ifdef PLASMA_EXTERNAL_RELEASE
    // If another instance is running, exit.  We'll automatically release our
    // lock on the mutex when our process exits
    HANDLE hOneInstance = CreateMutex(nullptr, FALSE, "UruExplorer");
    if (WaitForSingleObject(hOneInstance,0) != WAIT_OBJECT_0)
    {
        ST::string caption;
        ST::string message;
        switch (plLocalization::GetLanguage())
        {
            case plLocalization::kFrench:
                caption = ST_LITERAL("Erreur");
                message = ST_LITERAL("Une autre copie d'URU est déjà en cours d'exécution");
                break;
            case plLocalization::kGerman:
                caption = ST_LITERAL("Fehler");
                message = ST_LITERAL("URU wird bereits in einer anderen Instanz ausgeführt");
                break;
            case plLocalization::kSpanish:
                caption = ST_LITERAL("Error");
                message = ST_LITERAL("En estos momentos se está ejecutando otra copia de URU");
                break;
            case plLocalization::kItalian:
                caption = ST_LITERAL("Errore");
                message = ST_LITERAL("Un'altra copia di URU è già aperta");
                break;
            case plLocalization::kRussian:
                caption = ST_LITERAL("Ошибка");
                message = ST_LITERAL("Другая копия URU уже запущена");
                break;
            // default is English
            default:
                caption = ST_LITERAL("Error");
                message = ST_LITERAL("Another copy of URU is already running");
                break;
        }
        hsMessageBox(message, caption, hsMessageBoxNormal);
        return PARABLE_NORMAL_EXIT;
    }
#endif

    // Redirect hsStatusMessage to plasmadbg.log
    DebugInit();
    hsStatusMessageF("Plasma 2.0.{}.{} - {}", PLASMA2_MAJOR_VERSION, PLASMA2_MINOR_VERSION, plProduct::ProductString());

    FILE *serverIniFile = plFileSystem::Open(serverIni, "rb");
    if (serverIniFile)
    {
        fclose(serverIniFile);
        try {
            pfServerIni::Load(serverIni);
        } catch (const pfServerIniParseException& exc) {
            hsMessageBox(ST::format("Error in server.ini file. Please check your URU installation.\n{}", exc.what()), ST_LITERAL("Error"), hsMessageBoxNormal);
            return PARABLE_NORMAL_EXIT;
        }
    }
    else
    {
        hsMessageBox(ST_LITERAL("No server.ini file found. Please check your URU installation."), ST_LITERAL("Error"), hsMessageBoxNormal);
        return PARABLE_NORMAL_EXIT;
    }

    // Begin initializing the client in the background
    if (!WinInit(hInst)) {
        hsMessageBox(ST_LITERAL("Failed to initialize plClient"), ST_LITERAL("Error"), hsMessageBoxNormal);
        return PARABLE_NORMAL_EXIT;
    }

    NetCliAuthAutoReconnectEnable(false);
    InitNetClientComm();

    curl_global_init(CURL_GLOBAL_ALL);

    bool                needExit = false;
    LoginDialogParam    loginParam;
    memset(&loginParam, 0, sizeof(loginParam));
    LoadUserPass(&loginParam);

    if (!doIntroDialogs && loginParam.remember) {
        ENetError auth;

        NetCommSetAccountUsernamePassword(loginParam.username, loginParam.namePassHash);
        bool cancelled = AuthenticateNetClientComm(&auth, nullptr);

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
        needExit = ::DialogBoxParam( hInst, MAKEINTRESOURCE( IDD_URULOGIN_MAIN ), nullptr, UruLoginDialogProc, (LPARAM)&loginParam ) <= 0;
    }

    if (doIntroDialogs && !needExit) {
        HINSTANCE hRichEdDll = LoadLibrary("RICHED20.DLL");
        INT_PTR val = ::DialogBoxParam( hInst, MAKEINTRESOURCE( IDD_URULOGIN_EULA ), nullptr, UruTOSDialogProc, (LPARAM)hInst);
        FreeLibrary(hRichEdDll);
        if (val <= 0) {
            DWORD error = GetLastError();
            needExit = true;
        }
    }

    curl_global_cleanup();

    if (needExit) {
        gClient.ShutdownStart();
        gClient.ShutdownEnd();
        DeInitNetClientComm();
        return PARABLE_NORMAL_EXIT;
    }

    NetCliAuthAutoReconnectEnable(true);

    // Install our unhandled exception filter for trapping all those nasty crashes
    SetUnhandledExceptionFilter(plCustomUnhandledExceptionFilter);

    // We should quite frankly be done initing the client by now. But, if not, spawn the good old
    // "Starting URU, please wait..." dialog (not so yay)
    if (!gClient.IsInited()) {
        HWND splashDialog = ::CreateDialog(hInst, MAKEINTRESOURCE(IDD_LOADING), nullptr, SplashDialogProc);
        gClient.Wait();
        ::DestroyWindow(splashDialog);
    }

    // Tell everybody about the current display scaling
    {
        plDisplayScaleChangedMsg* msg = new plDisplayScaleChangedMsg(plWinDpi::Instance().GetScale());
        msg->Send();
    }

    // Main loop
    if (gClient && !gClient->GetDone()) {
        // Must be done here due to the plClient* dereference.
        if (cmdParser.IsSpecified(kArgSkipIntroMovies))
            gClient->SetFlag(plClient::kFlagSkipIntroMovies);

        if (gPendingActivate)
            gClient->WindowActivate(gPendingActivateFlag);
        gClient->SetMessagePumpProc(PumpMessageQueueProc);
        gClient.Start();

        // PhysX installs its own exception handler somewhere in PhysXCore.dll. Unfortunately, this code appears to suck
        // the big one. It actually makes us unable to attach with the Visual Studio debugger! We're going to override that
        // donkey snot. This can be removed when PhysX is replaced.
        SetUnhandledExceptionFilter(plCustomUnhandledExceptionFilter);

        MSG msg;
        do {
            gClient->MainLoop();
            if (gClient->GetDone())
                break;

            // Look for a message
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                // Handle the message
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        } while (WM_QUIT != msg.message);
    }

    gClient.ShutdownEnd();
    DeInitNetClientComm();

    // Exit WinMain and terminate the app....
    return PARABLE_NORMAL_EXIT;
}
