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

#include <stdio.h>
#include <direct.h>		// windows directory handling fxns (for chdir)
#include <process.h>

//#define DETACH_EXE  // Microsoft trick to force loading of exe to memory 
#ifdef DETACH_EXE
	#include "dmdfm.h"		// Windows Load EXE into memory suff
#endif


#include "HeadSpin.h"
#include "hsStream.h"
#include "hsUtils.h"
#include "plClient.h"
#include "../plNetClient/plNetClientMgr.h"
#include "../plNetClient/plNetLinkingMgr.h"
#include "../plInputCore/plInputManager.h"
#include "../plUnifiedTime/plUnifiedTime.h"
#include "plPipeline.h"
#include "../plResMgr/plResManager.h"
#include "../plResMgr/plLocalization.h"
#include "../plFile/plEncryptedStream.h"

#include "../plStatusLog/plStatusLog.h"
#include "../pnProduct/pnProduct.h"
#include "../plNetGameLib/plNetGameLib.h"
#include "../plFile/plFileUtils.h"

#include "../plPhysX/plSimulationMgr.h"

#include "res\resource.h"

#include <shellapi.h>
#include "WinHttp.h"
//
// Defines
//

#define	CLASSNAME	"Plasma"	// Used in WinInit()
#define PARABLE_NORMAL_EXIT		0	// i.e. exited WinMain normally

#define TIMER_UNITS_PER_SEC (float)1e3
#define UPDATE_STATUSMSG_SECONDS 30
#define WM_USER_SETSTATUSMSG WM_USER+1

#if BUILD_TYPE == BUILD_TYPE_DEV
	#define STATUS_PATH L"www2.cyanworlds.com"
#else
	#define STATUS_PATH L"support.cyanworlds.com"
#endif

//
// Globals
//
hsBool gHasMouse = false;

extern hsBool gDataServerLocal;
extern hsBool gUseBackgroundDownloader;

enum
{
	kArgToDni,
	kArgSkipLoginDialog,
	kArgAuthSrv,
	kArgFileSrv,
	kArgGateKeeperSrv,
	kArgLocalData,
	kArgBackgroundDownloader,
};

static const CmdArgDef s_cmdLineArgs[] = {
	{ kCmdArgFlagged  | kCmdTypeBool,		L"ToDni",			kArgToDni	},
	{ kCmdArgFlagged  | kCmdTypeBool,		L"SkipLoginDialog",	kArgSkipLoginDialog	},
	{ kCmdArgFlagged  | kCmdTypeString,		L"AuthSrv",			kArgAuthSrv	},
	{ kCmdArgFlagged  | kCmdTypeString,		L"FileSrv",			kArgFileSrv	},
	{ kCmdArgFlagged  | kCmdTypeString,		L"GateKeeperSrv",	kArgGateKeeperSrv },
	{ kCmdArgFlagged  | kCmdTypeBool,		L"LocalData",		kArgLocalData	},
	{ kCmdArgFlagged  | kCmdTypeBool,		L"BGDownload",		kArgBackgroundDownloader	},
};

/// Made globals now, so we can set them to zero if we take the border and 
///	caption styles out ala fullscreen (8.11.2000 mcn)
int gWinBorderDX	= GetSystemMetrics( SM_CXSIZEFRAME );
int gWinBorderDY	= GetSystemMetrics( SM_CYSIZEFRAME );
int gWinMenuDY		= GetSystemMetrics( SM_CYCAPTION );

//#include "global.h"
plClient		*gClient;
bool			gPendingActivate = false;
bool			gPendingActivateFlag = false;

static bool		s_loginDlgRunning = false;
static CEvent   s_statusEvent(kEventManualReset);

FILE *errFP = nil;
HINSTANCE				gHInst = NULL;		// Instance of this app

static const unsigned	AUTH_LOGIN_TIMER	= 1;
static const unsigned	AUTH_FAILED_TIMER	= 2;

#define FAKE_PASS_STRING "********"

//============================================================================
// External patcher file
//============================================================================
#ifdef PLASMA_EXTERNAL_RELEASE

static wchar s_patcherExeName[] = L"UruLauncher.exe";

//============================================================================
// Internal patcher file
//============================================================================
#else

static wchar s_patcherExeName[] = L"plUruLauncher.exe";

#endif // PLASMA_EXTERNAL_RELEASE

//============================================================================
// PhysX installer
//============================================================================
static wchar s_physXSetupExeName[] = L"PhysX_Setup.exe";

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
	bool		fromGT;
	ENetError	authError;
	wchar		accountName[kMaxAccountNameLength];
};

bool AuthenticateNetClientComm(ENetError* result, HWND parentWnd);
bool IsExpired();
void GetCryptKey(UInt32* cryptKey, unsigned size);
static void SaveUserPass (char *username, char *password, ShaDigest *pNamePassHash, bool remember_password,
								  bool fromGT);
static void LoadUserPass (const wchar *accountName, char *username, ShaDigest *pNamePassHash, bool *pRemember,
								  bool fromGT, int *pFocus);
static void AuthFailedStrings (ENetError authError, bool fromGT,
										 const char **ppStr1, const char **ppStr2,
										 const wchar **ppWStr);

#if 0
// For networking
const GUID NEXUS_GUID = { 
	 0x5bfdb060, 0x6a4, 0x11d0, 0x9c, 0x4f, 0x0, 0xa0, 0xc9, 0x5, 0x42, 0x5e};
#endif


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

static bool TGRunLoginDialog (const wchar *accountName, bool fromGT)
{
	ShaDigest NamePassHash;
	char Username[kMaxAccountNameLength + 5];
	int Focus;
	bool bRemember = false;

	LoadUserPass (accountName, Username, &NamePassHash, &bRemember, fromGT, &Focus);

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
		pTGUNIXAppWriteLine (pApp, Username);
		if (bRemember)
		  pTGUNIXAppWriteLine (pApp, FAKE_PASS_STRING);
		else
		  pTGUNIXAppWriteLine (pApp, "");
		if (bRemember)
		  pTGUNIXAppWriteLine (pApp, "y");
		else
		  pTGUNIXAppWriteLine (pApp, "n");

		if (!pTGUNIXAppReadLine (pApp, Username, sizeof (Username)))
		{
			pTGUNIXAppClose (pApp);
			hsMessageBox ("Incomplete or corrupted installation!\nUnable to locate Login dialog",
				"Error", hsMessageBoxNormal);
			return false;
		}

		// Check if user selected 'Cancel'
		if (StrCmp (Username, "text:", 5) != 0)
		{
			pTGUNIXAppClose (pApp);
			return false;
		}
		memmove (Username, Username + 5, StrLen (Username) - 5);
		Username[StrLen (Username) - 5] = '\0';

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

		bRemember = false;
		if (Remember[0] == 'y')
		  bRemember = true;

		SaveUserPass (Username, Password, &NamePassHash, bRemember, fromGT);

		// Do login & see if it failed
		ENetError auth;
		bool cancelled = AuthenticateNetClientComm(&auth, NULL);

		if (IS_NET_SUCCESS (auth) && !cancelled)
			break;

		if (!cancelled)
		  {
				const char *pStr1, *pStr2;
				const wchar *pWStr;
				unsigned int Len;
				char *pTmpStr;

				AuthFailedStrings (auth, fromGT, &pStr1, &pStr2, &pWStr);

				Len = StrLen (pStr1) + 1;
				if (pStr2)
				  Len += StrLen (pStr2) + 2;
				if (pWStr)
				  Len += StrLen (pWStr) + 2;

				pTmpStr = TRACKED_NEW char[Len];
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

void GetMouseCoords(HWND hWnd, WPARAM wParam, LPARAM lParam, int* xPos, int* yPos, int* fwKeys)
{
	POINT pt;
	pt.x=LOWORD(lParam);
	pt.y=HIWORD(lParam);
#if 0
	if (ClientToScreen(hWnd, &pt) == false)
		HSDebugProc("Error converting client mouse coords to screen");
#endif

	if (xPos)
		*xPos = pt.x;  // horizontal position of cursor 
	if (yPos)
		*yPos = pt.y;  // vertical position of cursor 
 
#if 0
	char str[128];
	sprintf(str, "mx=%d my=%d\n", pt.x, pt.y);
	hsStatusMessage(str);
#endif

	if (fwKeys)
		*fwKeys = wParam;        // key flags 
 
	// key flag bits
	// MK_CONTROL  Set if the CTRL key is down. 
 	// MK_LBUTTON  Set if the left mouse button is down. 
 	// MK_MBUTTON  Set if the middle mouse button is down. 
 	// MK_RBUTTON  Set if the right mouse button is down. 
 	// MK_SHIFT  Set if the SHIFT key is down. 
}

void DebugMsgF(const char* format, ...);

// Handles all the windows messages we might receive
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{	
	static bool gDragging = false;
	static UInt32 keyState=0;

    // Handle messages
    switch (message) {		
		case WM_KEYDOWN :
		case WM_LBUTTONDOWN	:
		case WM_RBUTTONDOWN :
		case WM_LBUTTONDBLCLK :		// The left mouse button was double-clicked. 
		case WM_MBUTTONDBLCLK :		// The middle mouse button was double-clicked. 
		case WM_MBUTTONDOWN :		// The middle mouse button was pressed. 
		case WM_RBUTTONDBLCLK :		// The right mouse button was double-clicked. 
			// If they did anything but move the mouse, quit any intro movie playing.
			{
				if( gClient )
					gClient->SetQuitIntro(true);
			}
			// Fall through to other events
		case WM_KEYUP :
		case WM_LBUTTONUP :
		case WM_RBUTTONUP :
		case WM_MBUTTONUP :			// The middle mouse button was released. 
		case WM_MOUSEMOVE :
		case 0x020A:				// fuc&ing windows b.s...
			{
				if (gClient && gClient->WindowActive() && gClient->GetInputManager())
				{
					gClient->GetInputManager()->HandleWin32ControlEvent(message, wParam, lParam, hWnd);
				}
			}
			break;

#if 0
		case WM_KILLFOCUS:
			SetForegroundWindow(hWnd);
			break;
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
						mgr->QueueDisableNet(false, nil);
					DestroyWindow(gClient->GetWindowHandle());
					break;
            }
			break;

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
			break;

		// Let go of the mouse if the window is being moved.
        case WM_ENTERSIZEMOVE:
			DebugMsgF("Got WM_ENTERSIZEMOVE%s", gClient ? "" : ", no client, ignoring");
			gDragging = true;
			if( gClient )
				gClient->WindowActivate(false);
			break;

		// Redo the mouse capture if the window gets moved
        case WM_EXITSIZEMOVE:
			DebugMsgF("Got WM_EXITSIZEMOVE%s", gClient ? "" : ", no client, ignoring");
			gDragging = false;
			if( gClient )
				gClient->WindowActivate(true);
			break;

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
			break;

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
			break;

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
			break;
		
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
			break;
    }
   return DefWindowProc(hWnd, message, wParam, lParam);
}
 
void	PumpMessageQueueProc( void )
{
	MSG	msg;

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

bool AuthenticateNetClientComm(ENetError* result, HWND parentWnd)
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
			ZERO(startupInfo);
			ZERO(processInfo);
			startupInfo.cb = sizeof(startupInfo);
			if(!CreateProcessW(NULL, s_physXSetupExeName, NULL,	NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo))
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

bool	InitClient( HWND hWnd )
{
	plResManager *resMgr = TRACKED_NEW plResManager;
	resMgr->SetDataPath("dat");
	hsgResMgr::Init(resMgr);
	gClient = TRACKED_NEW plClient;
	if( gClient == nil )
		return false;

	if (!InitPhysX())
		return false;

	gClient->SetWindowHandle( hWnd );

#ifdef DETACH_EXE
	hInstance = ((LPCREATESTRUCT) lParam)->hInstance;
#endif
	// If in fullscreen mode, get rid of the window borders.  Note: this won't take
	// effect until the next SetWindowPos call

#ifdef DETACH_EXE

	// This Function loads the EXE into Virtual memory...supposedly
    HRESULT hr = DetachFromMedium(hInstance, DMDFM_ALWAYS | DMDFM_ALLPAGES);
#endif

	if( gClient->InitPipeline() )
		gClient->SetDone(true);
	else
	{
		if( gClient->GetPipeline()->IsFullScreen() )
		{
			SetWindowLong(hWnd, GWL_STYLE, WS_POPUP);
			SetWindowLong(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);
            SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			gWinBorderDX = gWinBorderDY = gWinMenuDY = 0;
		}
		else {
            SetWindowLong(hWnd, GWL_STYLE, WS_OVERLAPPED | WS_CAPTION);
            SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}

		int goodWidth = gClient->GetPipeline()->Width() + gWinBorderDX * 2;
		int goodHeight = gClient->GetPipeline()->Height() + gWinBorderDY * 2 + gWinMenuDY;

		SetWindowPos(
			hWnd,
			nil,
			0,
			0,
			goodWidth,
			goodHeight,
			SWP_NOCOPYBITS 
				| SWP_NOMOVE
				| SWP_NOOWNERZORDER
				| SWP_NOZORDER
				| SWP_FRAMECHANGED
		);
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
    wndClass.style              = CS_DBLCLKS;	// CS_HREDRAW | CS_VREDRAW;
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
	wchar productString[256];
//#ifdef PLASMA_EXTERNAL_RELEASE	
#if 0	// Show the full product string in external build window title until we roll it into the options dialog -eap
	StrCopy(productString, ProductLongName(), arrsize(productString));
#else
	ProductString(productString, arrsize(productString));
#endif
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
//	gClient->SetWindowHandle((hsWindowHndl)

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

static bool IsMachineLittleEndian() {
   int i = 1;
   char *p = (char *) &i;
   if (p[0] == 1) // Lowest address contains the least significant byte
      return true;
   else
      return false;
}

inline static dword ToBigEndian (dword value) {
	return ((value) << 24) | ((value & 0x0000ff00) << 8) | ((value & 0x00ff0000) >> 8) | ((value) >> 24);
}

static void AuthFailedStrings (ENetError authError, bool fromGT,
										 const char **ppStr1, const char **ppStr2,
										 const wchar **ppWStr)
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
					if (fromGT)
						*ppStr2 = "GameTap authentication failed, please enter your GameTap username and password.";
					else
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
				const wchar *pWStr;

				AuthFailedStrings (loginParam->authError, loginParam->fromGT,
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
				char* eulaData = NULL;
				unsigned dataLen = stream.GetSizeLeft();

				eulaData = TRACKED_NEW char[dataLen + 1];
				ZeroMemory(eulaData, dataLen + 1);

				stream.Read(dataLen, eulaData);

				SetDlgItemText(hwndDlg, IDC_URULOGIN_EULATEXT, eulaData);
				delete [] eulaData;
			}

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

static void SaveUserPass (char *username, char *password, ShaDigest *pNamePassHash, bool remember_password,
						  bool fromGT)
{
	UInt32 cryptKey[4];
	ZeroMemory(cryptKey, sizeof(cryptKey));
	GetCryptKey(cryptKey, arrsize(cryptKey));

	wchar wusername[kMaxAccountNameLength];
	wchar wpassword[kMaxPasswordLength];

	StrToUnicode(wusername, username, arrsize(wusername));

	// if the password field is the fake string then we've already
	// loaded the namePassHash from the file
	if (StrCmp(password, FAKE_PASS_STRING) != 0)
	{
		StrToUnicode(wpassword, password, arrsize(wpassword));

		wchar domain[15];
		PathSplitEmail(wusername, nil, 0, domain, arrsize(domain), nil, 0, nil, 0, 0);

		if (StrLen(domain) == 0 || StrCmpI(domain, L"gametap") == 0) {
			CryptDigest(
				kCryptSha1,
				pNamePassHash,
				StrLen(password) * sizeof(password[0]),
				password
				);

			if (IsMachineLittleEndian()) {
				pNamePassHash->data[0] = ToBigEndian(pNamePassHash->data[0]);
				pNamePassHash->data[1] = ToBigEndian(pNamePassHash->data[1]);
				pNamePassHash->data[2] = ToBigEndian(pNamePassHash->data[2]);
				pNamePassHash->data[3] = ToBigEndian(pNamePassHash->data[3]);
				pNamePassHash->data[4] = ToBigEndian(pNamePassHash->data[4]);
			}
		}
		else
			CryptHashPassword(wusername, wpassword, pNamePassHash);
	}

	NetCommSetAccountUsernamePassword(wusername, *pNamePassHash);
	if (TGIsCider)
		NetCommSetAuthTokenAndOS(nil, L"mac");
	else
		NetCommSetAuthTokenAndOS(nil, L"win");

	if (!fromGT) {
		wchar fileAndPath[MAX_PATH];
		PathGetInitDirectory(fileAndPath, arrsize(fileAndPath));
		PathAddFilename(fileAndPath, fileAndPath, L"login.dat", arrsize(fileAndPath));
#ifndef PLASMA_EXTERNAL_RELEASE
		// internal builds can use the local init directory
		wchar localFileAndPath[MAX_PATH];
		StrCopy(localFileAndPath, L"init\\login.dat", arrsize(localFileAndPath));
		if (PathDoesFileExist(localFileAndPath))
			StrCopy(fileAndPath, localFileAndPath, arrsize(localFileAndPath));
#endif
		hsStream* stream = plEncryptedStream::OpenEncryptedFileWrite(fileAndPath, cryptKey);
		if (stream)
		{
			stream->Write(sizeof(cryptKey), cryptKey);
			stream->WriteSafeString(username);
			stream->Writebool(remember_password);
			if (remember_password)
				stream->Write(sizeof(pNamePassHash->data), pNamePassHash->data);
			stream->Close();
			delete stream;
		}
	}
}


static void LoadUserPass (const wchar *accountName, char *username, ShaDigest *pNamePassHash, bool *pRemember,
						  bool fromGT, int *pFocus)
{
			UInt32 cryptKey[4];
			ZeroMemory(cryptKey, sizeof(cryptKey));
			GetCryptKey(cryptKey, arrsize(cryptKey));

			char* temp;
	*pRemember = false;
	username[0] = '\0';

	if (!fromGT)
			{
				wchar fileAndPath[MAX_PATH];
				PathGetInitDirectory(fileAndPath, arrsize(fileAndPath));
				PathAddFilename(fileAndPath, fileAndPath, L"login.dat", arrsize(fileAndPath));
#ifndef PLASMA_EXTERNAL_RELEASE
				// internal builds can use the local init directory
				wchar localFileAndPath[MAX_PATH];
				StrCopy(localFileAndPath, L"init\\login.dat", arrsize(localFileAndPath));
				if (PathDoesFileExist(localFileAndPath))
					StrCopy(fileAndPath, localFileAndPath, arrsize(localFileAndPath));
#endif
				hsStream* stream = plEncryptedStream::OpenEncryptedFile(fileAndPath, true, cryptKey);
				if (stream && !stream->AtEnd())
				{
					UInt32 savedKey[4];
					stream->Read(sizeof(savedKey), savedKey);

					if (memcmp(cryptKey, savedKey, sizeof(savedKey)) == 0)
					{
						temp = stream->ReadSafeString();

						if (temp)
						{
					StrCopy(username, temp, kMaxAccountNameLength);
							delete temp;
						}
						else
							username[0] = '\0';

				*pRemember = stream->Readbool();

				if (*pRemember)
						{
					stream->Read(sizeof(pNamePassHash->data), pNamePassHash->data);
					*pFocus = IDOK;
						}
						else
					*pFocus = IDC_URULOGIN_PASSWORD;
					}

					stream->Close();
					delete stream;
				}
			}
			else
			{
		StrToAnsi (username, accountName, kMaxAccountNameLength);
		*pFocus = IDC_URULOGIN_PASSWORD;
	}
}

void StatusCallback(void *param)
{
	HWND hwnd = (HWND)param;

	while(s_loginDlgRunning)
	{
		// get status message from webpage and display in status area.
		const wchar *path = BuildTypeServerStatusPath();  
		if(path)
		{
			HINTERNET hSession = 0;
			HINTERNET hConnect = 0;
			HINTERNET hRequest = 0;
			
			hSession = WinHttpOpen(
				L"UruClient/1.0",
				WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
				WINHTTP_NO_PROXY_NAME,
				WINHTTP_NO_PROXY_BYPASS, 0
			);
			if(hSession)
			{
				HINTERNET hConnect = WinHttpConnect( hSession, STATUS_PATH, INTERNET_DEFAULT_HTTP_PORT, 0);
				if(hConnect)
				{
					HINTERNET hRequest = WinHttpOpenRequest( 
						hConnect, 
						L"GET",
						path,
						NULL, 
						WINHTTP_NO_REFERER, 
						WINHTTP_DEFAULT_ACCEPT_TYPES,
						0
					);
					if(hRequest)
					{
						static char data[256] = {0};
						DWORD bytesRead;
						WinHttpSendRequest( 
							hRequest, 
							WINHTTP_NO_ADDITIONAL_HEADERS,
							0,
							WINHTTP_NO_REQUEST_DATA,
							0,
							0,
							0
						);
						WinHttpReceiveResponse(hRequest, 0);
						WinHttpReadData(hRequest, data, 255, &bytesRead);
						data[bytesRead] = 0;
						if(bytesRead)
							PostMessage(hwnd, WM_USER_SETSTATUSMSG, 0, (LPARAM) data);
					}
				}
			}
			WinHttpCloseHandle(hRequest);
			WinHttpCloseHandle(hConnect);
			WinHttpCloseHandle(hSession);
		}
		else 
			break;		// no status message
		
		for(unsigned i = 0; i < UPDATE_STATUSMSG_SECONDS && s_loginDlgRunning; ++i)
		{
			Sleep(1000);
		}
	}
	s_statusEvent.Signal();
}

BOOL CALLBACK UruLoginDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static ShaDigest namePassHash;
	static LoginDialogParam* loginParam;
	static showAuthFailed = false;

	switch( uMsg )
	{
		case WM_INITDIALOG:
		{
			s_loginDlgRunning = true;
			_beginthread(StatusCallback, 0, hwndDlg);
			loginParam = (LoginDialogParam*)lParam;

			SetWindowText(hwndDlg, "Login");
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(gHInst, MAKEINTRESOURCE(IDI_ICON_DIRT)));

			EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);

			char username[kMaxAccountNameLength];
			bool remember_password = false;

			int focus_control = IDC_URULOGIN_USERNAME;

			LoadUserPass (loginParam->accountName, username, &namePassHash, &remember_password, loginParam->fromGT, &focus_control);

			SetDlgItemText(hwndDlg, IDC_URULOGIN_USERNAME, username);
			CheckDlgButton(hwndDlg, IDC_URULOGIN_REMEMBERPASS, remember_password ? BST_CHECKED : BST_UNCHECKED);
			if (remember_password)
				SetDlgItemText(hwndDlg, IDC_URULOGIN_PASSWORD, FAKE_PASS_STRING);
			if (loginParam->fromGT)
				EnableWindow(GetDlgItem(hwndDlg, IDC_URULOGIN_REMEMBERPASS), FALSE);

			SetFocus(GetDlgItem(hwndDlg, focus_control));

			if (IS_NET_ERROR(loginParam->authError))
			{
				showAuthFailed = true;
			}

			char windowName[256];
			wchar productString[256];
			ProductString(productString, arrsize(productString));
			StrToAnsi(windowName, productString, arrsize(windowName));
			SendMessage(GetDlgItem(hwndDlg, IDC_PRODUCTSTRING), WM_SETTEXT, 0, (LPARAM) windowName);

			SetTimer(hwndDlg, AUTH_LOGIN_TIMER, 10, NULL);
			return FALSE;
		}

		case WM_USER_SETSTATUSMSG:
			 SendMessage(GetDlgItem(hwndDlg, IDC_STATUS_TEXT), WM_SETTEXT, 0, lParam);
			 return TRUE;

		case WM_DESTROY:
		{
			s_loginDlgRunning = false;
			s_statusEvent.Wait(kEventWaitForever);
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
					char username[kMaxAccountNameLength];
					char password[kMaxPasswordLength];
					bool remember_password = false;

					GetDlgItemText(hwndDlg, IDC_URULOGIN_USERNAME, username, kMaxAccountNameLength);
					GetDlgItemText(hwndDlg, IDC_URULOGIN_PASSWORD, password, kMaxPasswordLength);
					remember_password = (IsDlgButtonChecked(hwndDlg, IDC_URULOGIN_REMEMBERPASS) == BST_CHECKED);

					SaveUserPass (username, password, &namePassHash, remember_password, loginParam->fromGT);

					LoginDialogParam loginParam;
					MemSet(&loginParam, 0, sizeof(loginParam));
					bool cancelled = AuthenticateNetClientComm(&loginParam.authError, hwndDlg);

					if (IS_NET_SUCCESS(loginParam.authError) && !cancelled)
						EndDialog(hwndDlg, ok);
					else {
						if (!cancelled)
							::DialogBoxParam(gHInst, MAKEINTRESOURCE( IDD_AUTHFAILED ), hwndDlg, AuthFailedDialogProc, (LPARAM)&loginParam);
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
				ShellExecute(NULL, "open", "http://www.mystonline.com/signup.html", NULL, NULL, SW_SHOWNORMAL);

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
				::DialogBoxParam(gHInst, MAKEINTRESOURCE( IDD_AUTHFAILED ), hwndDlg, AuthFailedDialogProc, (LPARAM)loginParam);
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
/*				case plLocalization::kSpanish:
					::SetDlgItemText( hwndDlg, IDC_STARTING_TEXT, "Iniciando URU, por favor espera...");
					break;
				case plLocalization::kItalian:
					::SetDlgItemText( hwndDlg, IDC_STARTING_TEXT, "Avvio di URU, attendere...");
					break;
*/				// default is English
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

static char	sStackTraceMsg[ 10240 ] = "";
void printStackTrace( char* buffer, int bufferSize, unsigned long stackPtr = 0, unsigned long opPtr = 0 );
//void StackTraceFromContext( HANDLE hThread, CONTEXT *context, char *outputBuffer );

BOOL CALLBACK ExceptionDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static char	*sLastMsg = nil;

	switch( uMsg )
	{
		case WM_INITDIALOG:
			sLastMsg = (char *)lParam;
			::SetDlgItemText( hwndDlg, IDC_CRASHINFO, sLastMsg );
			return true;

		case WM_COMMAND:
			if( wParam == IDC_COPY && sLastMsg != nil )
			{
				HGLOBAL	copyText = GlobalAlloc( GMEM_DDESHARE, sizeof( TCHAR ) * ( strlen( sLastMsg ) + 1 ) );
				if( copyText != nil )
				{
					LPTSTR	copyPtr = (LPTSTR)GlobalLock( copyText );
					memcpy( copyPtr, sLastMsg, ( strlen( sLastMsg ) + 1 ) * sizeof( TCHAR ) );
					GlobalUnlock( copyText );

					::OpenClipboard( hwndDlg );
					::EmptyClipboard();
					::SetClipboardData( CF_TEXT, copyText );
					::CloseClipboard();
				}
				return true;
			}
			else if( wParam == IDOK )
				EndDialog( hwndDlg, IDOK );
			else
				break;
	}
	return 0;
}


LONG WINAPI plCustomUnhandledExceptionFilter( struct _EXCEPTION_POINTERS *ExceptionInfo )
{
	const char *type = nil;
	switch( ExceptionInfo->ExceptionRecord->ExceptionCode )
	{
		case EXCEPTION_ACCESS_VIOLATION:			type = "Access violation"; break;
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:		type = "Array bounds exceeded"; break;
		case EXCEPTION_BREAKPOINT:					type = "Breakpoint"; break;
		case EXCEPTION_DATATYPE_MISALIGNMENT:		type = "Datatype misalignment"; break;
		case EXCEPTION_FLT_DENORMAL_OPERAND:		type = "Floating operand denormal"; break;
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:			type = "Floating-point divide-by-zero"; break;
		case EXCEPTION_FLT_INEXACT_RESULT:			type = "Floating-point inexact result"; break;
		case EXCEPTION_FLT_INVALID_OPERATION:		type = "Floating-point invalid operation"; break;
		case EXCEPTION_FLT_OVERFLOW:				type = "Floating-point overflow"; break;
		case EXCEPTION_FLT_STACK_CHECK:				type = "Floating-point stack error"; break;
		case EXCEPTION_FLT_UNDERFLOW:				type = "Floating-point underflow"; break;
		case EXCEPTION_ILLEGAL_INSTRUCTION:			type = "Illegal instruction"; break;
		case EXCEPTION_IN_PAGE_ERROR:				type = "Exception in page"; break;
		case EXCEPTION_INT_DIVIDE_BY_ZERO:			type = "Integer divide-by-zero"; break;
		case EXCEPTION_INT_OVERFLOW:				type = "Integer overflow"; break;
		case EXCEPTION_INVALID_DISPOSITION:			type = "Invalid disposition"; break;
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:	type = "Noncontinuable exception"; break;
		case EXCEPTION_PRIV_INSTRUCTION:			type = "Private instruction"; break;
		case EXCEPTION_SINGLE_STEP:					type = "Single-step"; break;
		case EXCEPTION_STACK_OVERFLOW:				type = "Stack overflow"; break;
	}

	char prodName[256];
	wchar productString[256];
	ProductString(productString, arrsize(productString));
	StrToAnsi(prodName, productString, arrsize(prodName));

	sprintf( sStackTraceMsg, "%s\r\nException type: %s\r\n", prodName, ( type != nil ) ? type : "(unknown)" );

	printStackTrace( sStackTraceMsg, sizeof( sStackTraceMsg ), ExceptionInfo->ContextRecord->Ebp, (unsigned long)ExceptionInfo->ExceptionRecord->ExceptionAddress );

	/// Print the info out to a log file as well
	hsUNIXStream	log;
	wchar fileAndPath[MAX_PATH];
	PathGetLogDirectory(fileAndPath, arrsize(fileAndPath));
	PathAddFilename(fileAndPath, fileAndPath, L"stackDump.log", arrsize(fileAndPath));
	if( log.Open( fileAndPath, L"wt" ) )
	{
		log.WriteString( sStackTraceMsg );
		log.Close();
	}

	/// Hopefully we can access this resource, even given the exception (i.e. very-bad-error) we just experienced
	if(TGIsCider || (::DialogBoxParam( gHInst, MAKEINTRESOURCE( IDD_EXCEPTION ), ( gClient != nil ) ? gClient->GetWindowHandle() : nil,
							ExceptionDialogProc, (LPARAM)sStackTraceMsg ) == -1) )
	{
		// The dialog failed, so just fallback to a standard message box
		hsMessageBox( sStackTraceMsg, "UruExplorer Exception", hsMessageBoxNormal );
	}
	return EXCEPTION_EXECUTE_HANDLER;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
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
	if(cmdParser.IsSpecified(kArgBackgroundDownloader))
		gUseBackgroundDownloader = true;
#endif
	if(cmdParser.IsSpecified(kArgAuthSrv))
	{
		SetAuthSrvHostname(cmdParser.GetString(kArgAuthSrv));
	}

	if(cmdParser.IsSpecified(kArgFileSrv))
	{
		SetFileSrvHostname(cmdParser.GetString(kArgFileSrv));
	}

	if(cmdParser.IsSpecified(kArgGateKeeperSrv))
	{
		SetGateKeeperSrvHostname(cmdParser.GetString(kArgGateKeeperSrv));
	}

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
	ZERO(si);
	ZERO(pi);
	si.cb = sizeof(si);
	wchar cmdLine[MAX_PATH];
	const wchar ** addrs;
	unsigned count;
	
	if (!eventExists) // if it is missing, assume patcher wasn't launched
	{
		StrCopy(cmdLine, s_patcherExeName, arrsize(cmdLine));
		count = GetAuthSrvHostnames(&addrs);
		if(count && AuthSrvHostnameOverride())
			StrPrintf(cmdLine, arrsize(cmdLine), L"%ws /AuthSrv=%ws", cmdLine, addrs[0]);

		count = GetFileSrvHostnames(&addrs);
		if(count && FileSrvHostnameOverride())
			StrPrintf(cmdLine, arrsize(cmdLine), L"%ws /FileSrv=%ws", cmdLine, addrs[0]);

		count = GetGateKeeperSrvHostnames(&addrs);
		if(count && GateKeeperSrvHostnameOverride())
			StrPrintf(cmdLine, arrsize(cmdLine), L"%ws /GateKeeperSrv=%ws", cmdLine, addrs[0]);

		if(!CreateProcessW(NULL, cmdLine, NULL,	NULL, FALSE, 0, NULL, NULL, &si, &pi))
		{
			hsMessageBox("Failed to launch patcher", "Error", hsMessageBoxNormal);
		}
		CloseHandle( pi.hThread );
		CloseHandle( pi.hProcess );
		return PARABLE_NORMAL_EXIT;
	}
#endif

	plLocalization::SetDefaultLanguage();
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
/*			case plLocalization::kSpanish:
				hsMessageBox("En estos momentos se está ejecutando otra copia de URU", "Error", hsMessageBoxNormal);
				break;
			case plLocalization::kItalian:
				hsMessageBox("Un'altra copia di URU è già aperta", "Errore", hsMessageBoxNormal);
				break;
*/			// default is English
			default:
				hsMessageBox("Another copy of URU is already running", "Error", hsMessageBoxNormal);
				break;
		}
		return PARABLE_NORMAL_EXIT;
	}

	if (IsExpired())
	{
		hsMessageBox("This client is over 30 days old.  You need to get a new one.", "Error", hsMessageBoxNormal);
		return PARABLE_NORMAL_EXIT;
	}

	NetCliAuthAutoReconnectEnable(false);

	NetCommSetReadIniAccountInfo(!doIntroDialogs);
	InitNetClientComm();

	wchar		acctName[kMaxAccountNameLength];

	// if we're being launched from gametap then don't use the intro dialogs
	if (StrStrI(lpCmdLine, "screenname=")) {
		doIntroDialogs = false;

		wchar		authToken[kMaxPublisherAuthKeyLength];
		wchar		os[kMaxGTOSIdLength];
		ShaDigest	emptyDigest;

		MemSet(acctName, 0, sizeof(acctName));
		MemSet(authToken, 0, sizeof(authToken));
		MemSet(os, 0, sizeof(os));

		const char* temp = lpCmdLine;
		char token[128];
		while (StrTokenize(&temp, token, arrsize(token), " =")) {
			if (StrCmpI(token, "screenname") == 0) {
				if (!StrTokenize(&temp, token, arrsize(token), " ="))
					break;

				StrToUnicode(acctName, token, arrsize(acctName));
			}
			else if (StrCmpI(token, "authtoken") == 0) {
				if (!StrTokenize(&temp, token, arrsize(token), " ="))
					break;

				StrToUnicode(authToken, token, arrsize(authToken));
			}
			else if (StrCmpI(token, "os") == 0) {
				if (!StrTokenize(&temp, token, arrsize(token), " ="))
					break;

				StrToUnicode(os, token, arrsize(os));
			}
		}

		NetCommSetAccountUsernamePassword(acctName, emptyDigest);
		NetCommSetAuthTokenAndOS(authToken, os);
	}

	bool				needExit = false;
	LoginDialogParam	loginParam;
	MemSet(&loginParam, 0, sizeof(loginParam));

	if (!doIntroDialogs) {
		ENetError auth;

		bool cancelled = AuthenticateNetClientComm(&auth, NULL);

		if (IS_NET_ERROR(auth) || cancelled) {
			doIntroDialogs = true;

			loginParam.fromGT = true;
			loginParam.authError = auth;
			StrCopy(loginParam.accountName, acctName, arrsize(loginParam.accountName));

			if (cancelled)
			{
				NetCommDisconnect();
			}
		}
	}

	if (doIntroDialogs) {
		if (TGIsCider)
			needExit = !TGRunLoginDialog (loginParam.accountName, loginParam.fromGT);
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
		wchar fileAndPath[MAX_PATH];
		PathGetLogDirectory(fileAndPath, arrsize(fileAndPath));
		PathAddFilename(fileAndPath, fileAndPath, L"plasmalog.txt", arrsize(fileAndPath));
		gDebugFile = _wfopen(fileAndPath, L"wt");
		hsAssert(gDebugFile != NULL, "Error creating debug file plasmalog.txt");
		hsSetDebugMessageProc(DebugMessageProc);
		if (gDebugFile != NULL)
		{
			char prdName[256];
			wchar prdString[256];
			ProductString(prdString, arrsize(prdString));
			StrToAnsi(prdName, prdString, arrsize(prdName));
			fprintf(gDebugFile, "%s\n", prdName);
			fflush(gDebugFile);
		}
	}

	// log stackdump.log text if the log exists
	char stackDumpText[1024];
	wchar stackDumpTextW[1024];	
	memset(stackDumpText, 0, arrsize(stackDumpText));
	memset(stackDumpTextW, 0, arrsize(stackDumpTextW) * sizeof(wchar));
	wchar fileAndPath[MAX_PATH];
	PathGetLogDirectory(fileAndPath, arrsize(fileAndPath));
	PathAddFilename(fileAndPath, fileAndPath, L"stackDump.log", arrsize(fileAndPath));
 	FILE *stackDumpLog = _wfopen(fileAndPath, L"r");
	if(stackDumpLog)
	{
		fread(stackDumpText, 1, arrsize(stackDumpText) - 1, stackDumpLog);
		StrToUnicode(stackDumpTextW, stackDumpText, arrsize(stackDumpText));
		NetCliAuthLogStackDump (stackDumpTextW);
		fclose(stackDumpLog);
		plFileUtils::RemoveFile(fileAndPath);
	}

	for (;;) {
		// Create Window
		if (!WinInit(hInst, nCmdShow) || gClient->GetDone())
			break;

		// We don't have multiplayer localized assets for Italian or Spanish, so force them to English in that case.
	/*	if (!plNetClientMgr::GetInstance()->InOfflineMode() &&
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
		hsgResMgr::Shutdown();	// deletes fResMgr
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

bool IsExpired()
{
	bool expired = false;

#ifndef PLASMA_EXTERNAL_RELEASE
	char ourPath[MAX_PATH];
	GetModuleFileName(NULL, ourPath, sizeof(ourPath));
	DWORD ok = 0;
	DWORD size = GetFileVersionInfoSize(ourPath, &ok);
	if (size > 0)
	{
		void* data = TRACKED_NEW UInt8[size];
		GetFileVersionInfo(ourPath, ok, size, data);

		unsigned int descLen = 0;
		void* desc = nil;
		if (VerQueryValue(data, "\\StringFileInfo\\040904B0\\FileDescription", &desc, &descLen))
		{
			char* buildDateStart = strstr((const char*)desc, " - Built ");
			if (buildDateStart)
			{
				buildDateStart += strlen(" - Built ");
				char* buildDateEnd = strstr(buildDateStart, " at");
				if (buildDateEnd)
				{
					int len = buildDateEnd-buildDateStart;

					char buf[32];
					strncpy(buf, buildDateStart, len);
					buf[len] = '\0';

					int month = atoi(strtok(buf, "/"));
					int day = atoi(strtok(nil, "/"));
					int year = atoi(strtok(nil, "/"));

					SYSTEMTIME curTime, buildTime;
					GetLocalTime(&buildTime);
					GetLocalTime(&curTime);
					buildTime.wDay = day;
					buildTime.wMonth = month;
					buildTime.wYear = year;

					ULARGE_INTEGER iCurTime, iBuildTime;
					FILETIME ft;

					SystemTimeToFileTime(&curTime, &ft);
					iCurTime.LowPart = ft.dwLowDateTime;
					iCurTime.HighPart = ft.dwHighDateTime;

					SystemTimeToFileTime(&buildTime, &ft);
					iBuildTime.LowPart = ft.dwLowDateTime;
					iBuildTime.HighPart = ft.dwHighDateTime;

					int secsOld = (int)((iCurTime.QuadPart - iBuildTime.QuadPart) / 10000000);
					int daysOld = secsOld / (60 * 60 * 24);

					if (daysOld > 30)
						expired = true;
				}
			}
		}

		delete [] data;
	}
#endif

	return expired;
}

void GetCryptKey(UInt32* cryptKey, unsigned numElements)
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
				volName,		//LPCTSTR lpRootPathName,
				NULL,			//LPTSTR lpVolumeNameBuffer,
				0,				//DWORD nVolumeNameSize,
				&volSerialNum,	//LPDWORD lpVolumeSerialNumber,
				NULL,			//LPDWORD lpMaximumComponentLength,
				NULL,			//LPDWORD lpFileSystemFlags,
				NULL,			//LPTSTR lpFileSystemNameBuffer,
				0				//DWORD nFileSystemNameSize
			);

			cryptKey[index] = (cryptKey[index] ^ volSerialNum);

			index = (++index) % numElements;
		}
	}
}

