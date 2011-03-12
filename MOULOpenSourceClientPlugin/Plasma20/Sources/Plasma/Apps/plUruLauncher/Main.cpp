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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/Apps/plUruLauncher/Main.cpp
*   
***/

#include "Pch.h"
#pragma hdrstop


#include "resource.h"
#include <commctrl.h>
#define WIN32_LEAN_AND_MEAN
#define WHITESPACE     L" \"\t\r\n\x1A"
#define UPDATE_STATUSMSG_SECONDS 30		// Must be an int

#if BUILD_TYPE == BUILD_TYPE_DEV
	#define STATUS_PATH L"www2.cyanworlds.com"
#else
	#define STATUS_PATH L"support.cyanworlds.com"
#endif


#if BUILD_TYPE == BUILD_TYPE_BETA
	static const char s_postKey[] = "";	//"betakey=6C5DC90EFD7AF8892D2A65CDE5DF46D55A2777EC3D196ED83F912B62185A74DD";
#else
	static const char s_postKey[] = "";
#endif


/*****************************************************************************
*
*   Private
*
***/

enum ELogSev {
	kLogInfo,
	kLogErr,
	kNumLogSev
};

enum {
    kEventTimer = 1,
};

enum EEventType {
	kEventSetProgress,
	kEventSetText,
	kEventSetStatusText,
	kEventSetTimeRemaining,
	kEventSetBytesRemaining,
};

// base window event
struct WndEvent {
	LINK(WndEvent)  link;
	EEventType type;
};

struct SetProgressEvent : WndEvent {
	int progress;
};

struct SetTextEvent : WndEvent {
	char text[MAX_PATH];
};

struct SetStatusTextEvent : WndEvent {
	char text[MAX_PATH];
};

struct SetTimeRemainingEvent : WndEvent {
	unsigned seconds;
};

struct SetBytesRemainingEvent : WndEvent {
	unsigned bytes;
};


//============================================================================
// TRANSGAMING detection & dialog replacement
//============================================================================
typedef BOOL (WINAPI *IsTransgaming) (void);
typedef const char * (WINAPI *TGGetOS) (void);
typedef LPVOID (WINAPI *TGLaunchUNIXApp) (const char *pPath, const char *pMode);
typedef BOOL (WINAPI *TGUNIXAppReadLine) (LPVOID pApp, char *pBuf, int bufSize);
typedef BOOL (WINAPI *TGUNIXAppWriteLine) (LPVOID pApp, const char *pLine);
typedef BOOL (WINAPI *TGUNIXAppClose) (LPVOID pApp);

static bool TGIsCider = false;
static void *pTGApp = NULL;
static TGLaunchUNIXApp pTGLaunchUNIXApp;
static TGUNIXAppReadLine pTGUNIXAppReadLine;
static TGUNIXAppWriteLine pTGUNIXAppWriteLine;
static TGUNIXAppClose pTGUNIXAppClose;

#define TG_NEW_DIALOG_PATH "C:\\Program Files\\Uru Live\\Cider\\URU Live Updater.app"
#define TG_NEW_DIALOG_POPEN_PATH "/transgaming/c_drive/Program Files/Uru Live/Cider/URU Live Updater.app/Contents/MacOS/URU Live Updater"
#define TG_OLD_DIALOG_POPEN_PATH "/URU Live Updater.app/Contents/MacOS/URU Live Updater"
#define TG_CUR_FRAMEWORK_FILE "C:\\Program Files\\Uru Live\\Cider\\current.txt"
#define TG_LATEST_FRAMEWORK_FILE "C:\\Program Files\\Uru Live\\Cider\\Frameworks\\version.txt"


/*****************************************************************************
*
*   Private data
*
***/

static bool						s_shutdown;
static bool						s_prepared;
static int						s_retCode = 1;
static long						s_terminationIssued;
static bool						s_terminated; 
static plLauncherInfo			s_launcherInfo;
static HANDLE					s_thread;
static HANDLE					s_event;
static HINSTANCE				s_hInstance;
static HWND						s_dialog;
static CEvent					s_dialogCreateEvent(kEventManualReset);
static CCritSect				s_critsect;
static LISTDECL(WndEvent, link) s_eventQ;
static CEvent					s_shutdownEvent(kEventManualReset);
static wchar					s_workingDir[MAX_PATH];
static CEvent					s_statusEvent(kEventManualReset);


// List of files we need to delete from the client directory
#ifdef PLASMA_EXTERNAL_RELEASE

static const wchar * s_deleteFiles[] = {
	L"NetDiag.exe",
	L"UruExplorer.pdb",
	L"UruExplorer.map",
};

#else

static const wchar * s_deleteFiles[] = {
	L"NetDiag.exe",
};

#endif


/*****************************************************************************
*
*   Local functions
*
***/

// Detect whether we're running under TRANSGAMING Cider
//==============================================================================
static void TGDoCiderDetection () {

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

//============================================================================
static void Abort () {
	s_retCode = 0;
	s_shutdown = true;
}

//============================================================================
static void PostEvent (WndEvent *event) {
	s_critsect.Enter();
	s_eventQ.Link(event);
	s_critsect.Leave();
}

//============================================================================
static void LogV (ELogSev sev, const wchar fmt[], va_list args) {
	static struct { FILE * file; const wchar * pre; } s_log[] = {
		{ stdout, L"Inf" },
		{ stderr, L"Err" },
	};
	COMPILER_ASSERT(arrsize(s_log) == kNumLogSev);
	
	fwprintf (s_log[sev].file, L"%s: ", s_log[sev].pre);
	vfwprintf(s_log[sev].file, fmt, args);
	fwprintf (s_log[sev].file, L"\n");

	if (sev >= kLogErr)
		Abort();
}

//============================================================================
static void Log (ELogSev sev, const wchar fmt[], ...) {
	va_list args;
	va_start(args, fmt);
	LogV(sev, fmt, args);
	va_end(args);
}

//============================================================================
// NOTE: Must use LocalFree() on the return value of this function when finished with the string
static wchar *TranslateErrorCode(DWORD errorCode) {
	LPVOID lpMsgBuf;
					
	FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(wchar *) &lpMsgBuf,
		0, 
		NULL 
	);
	return (wchar *)lpMsgBuf;
}

//============================================================================
static BOOL WINAPI CtrlHandler (DWORD) {
	static unsigned s_ctrlCount;
	if (++s_ctrlCount == 3)
		_exit(1);	// exit process immediately upon 3rd Ctrl-C.
	Abort();
    return TRUE;
}

//============================================================================
static void PrepareGame () {
	SetText("Connecting to server...");
	(void)_beginthread(UruPrepProc, 0, (void *) &s_launcherInfo); 
}

//============================================================================
static void InitGame () {
	s_launcherInfo.initCallback(kStatusOk, nil);
}

//============================================================================
static void StartGame () {
	(void)_beginthread(UruStartProc, 0, (void *) &s_launcherInfo);
}

//============================================================================
static void StopGame () {
	(void)_beginthread(PlayerStopProc, 0, (void *) &s_launcherInfo);
}

//============================================================================
static void TerminateGame () {
	if (!AtomicSet(&s_terminationIssued, 1))
	_beginthread(PlayerTerminateProc, 0, (void *) &s_launcherInfo);
}

//============================================================================
static void Recv_SetProgress (HWND hwnd, const SetProgressEvent &event) {  
	SendMessage(GetDlgItem(s_dialog, IDC_PROGRESS), PBM_SETPOS, event.progress, NULL);

	if (pTGApp)
	{
		char buf[64];

		sprintf (buf, "bar:%d", event.progress);
		if (!pTGUNIXAppWriteLine (pTGApp, buf))
		{
			pTGUNIXAppClose (pTGApp);
			pTGApp = NULL;
			PostQuitMessage (0);
		}
	}
}

//============================================================================
static void Recv_SetText (HWND hwnd, const SetTextEvent &event) { 
	bool b = SendMessage(GetDlgItem(s_dialog, IDC_TEXT), WM_SETTEXT, 0, (LPARAM) event.text);

	if (pTGApp)
	{
		char buf[MAX_PATH + 5];

		sprintf (buf, "text:%s", event.text);
		if (!pTGUNIXAppWriteLine (pTGApp, buf))
		{
			pTGUNIXAppClose (pTGApp);
			pTGApp = NULL;
			PostQuitMessage (0);
		}
	}
}

//============================================================================
static void Recv_SetStatusText (HWND hwnd, const SetStatusTextEvent &event) {
	bool b = SendMessage(GetDlgItem(s_dialog, IDC_STATUS_TEXT), WM_SETTEXT, 0, (LPARAM) event.text);
}

//============================================================================
static void Recv_SetTimeRemaining (HWND hwnd, const SetTimeRemainingEvent &event) {
	unsigned days;
	unsigned hours;
	unsigned minutes;
	unsigned seconds;
	
	if(event.seconds == 0xffffffff)
	{
		SendMessage(GetDlgItem(s_dialog, IDC_TIMEREMAINING), WM_SETTEXT, 0, (LPARAM) "estimating...");
		return;
	}

	seconds = event.seconds;

	days = seconds / (60 * 60 * 24);
	seconds -= (days * 60 * 60 * 24);
	hours = seconds / (60 * 60);
	seconds -= hours * 60 * 60;
	minutes = seconds / 60;
	seconds -= minutes * 60;
	seconds = seconds;

	char text[64] = {0};
	if(days)
	{
		if(days > 1)
			StrPrintf(text, arrsize(text), "%d days ", days);
		else
			StrPrintf(text, arrsize(text), "%d day ", days);
	}
	if(hours)
	{
		if(hours > 1)
			StrPrintf(text, arrsize(text), "%s%d hours ", text, hours);
		else
			StrPrintf(text, arrsize(text), "%s%d hour ", text, hours);
	}
	if(minutes)
		StrPrintf(text, arrsize(text), "%s%d min ", text, minutes);
	if( seconds || !text[0])
		StrPrintf(text, arrsize(text), "%s%d sec", text, seconds);
	bool b = SendMessage(GetDlgItem(s_dialog, IDC_TIMEREMAINING), WM_SETTEXT, 0, (LPARAM) text);
}

//============================================================================
static void Recv_SetBytesRemaining (HWND hwnd, const SetBytesRemainingEvent &event) {
	char text[32];
	unsigned MB;
	unsigned decimal;
	unsigned bytes = event.bytes;

	unsigned GB = bytes / 1000000000;
	if(GB)
	{
		bytes -= GB * 1000000000;
		decimal = bytes / 100000000;	// to two decimal places
		StrPrintf(text, arrsize(text), "%d.%d GB", GB, decimal);
	}
	else
	{
		MB = bytes / 1000000;
		bytes -= MB * 1000000;
		decimal = bytes / 100000;	// to one decimal place
		StrPrintf(text, arrsize(text), "%d.%d MB", MB, decimal);
	}
	bool b = SendMessage(GetDlgItem(s_dialog, IDC_BYTESREMAINING), WM_SETTEXT, 0, (LPARAM) text);
}

//============================================================================
static void DispatchEvents (HWND hwnd) {
	LISTDECL(WndEvent, link) eventQ;

	s_critsect.Enter();
    { 
		eventQ.Link(&s_eventQ);
    }
	s_critsect.Leave();

#define DISPATCH(a) case kEvent##a: Recv_##a(hwnd, *(const a##Event *) event); break
	while (WndEvent *event = eventQ.Head()) { 
		switch (event->type) {
			DISPATCH(SetProgress);
			DISPATCH(SetText);
			DISPATCH(SetStatusText);
			DISPATCH(SetTimeRemaining);
			DISPATCH(SetBytesRemaining);
            DEFAULT_FATAL(event->type);
		}
		DEL(event);  // unlinks from list 
	}
#undef DISPATCH
}

//============================================================================
static void OnTimer(HWND hwnd, unsigned int timerId) {
	if(s_shutdown) return;
	switch (timerId) {
        case kEventTimer:
            DispatchEvents(hwnd);
        break;

        DEFAULT_FATAL(timerId);
	}
}

//===========================================================================
static void MessagePump (HWND hwnd) {
	for (;;) {
        // wait for a message or the shutdown event
        const DWORD result = MsgWaitForMultipleObjects(
            1,
            &s_event,
            false,
            INFINITE,
            QS_ALLEVENTS
        );
        if (result == WAIT_OBJECT_0)
            return;

		// process windows messages
		MSG msg;
		
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			if (!IsDialogMessage(s_dialog, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			if (msg.message == WM_QUIT) {
				return;
			}
		}
	}
}

//============================================================================
BOOL CALLBACK SplashDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
		case WM_INITDIALOG: 
		{
			PostMessage( GetDlgItem(hwndDlg, IDC_PROGRESS), PBM_SETRANGE, 0, MAKELPARAM(0, 1000)); 
		}
        break;  

		case WM_COMMAND:
			if(HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDCANCEL) {
				// we dont shutdown the window here, but instead let the patcher know it needs to shutdown, and display our shutting down message. 
				// setting s_shutdown also wont allow any more Set text messages.
				if(!s_shutdown)
				{
					s_shutdown = true;
					SendMessage(GetDlgItem(s_dialog, IDC_TEXT), WM_SETTEXT, 0, (LPARAM) "Shutting Down...");
					EnableWindow(GetDlgItem(s_dialog, IDCANCEL), false);
				}
			}
		break;

		case WM_KEYDOWN:
			break;

		case WM_NCHITTEST:
			SetWindowLongPtr(hwndDlg, DWL_MSGRESULT, (LONG_PTR)HTCAPTION);
		return TRUE;

		case WM_TIMER:
			OnTimer(hwndDlg, wParam);
		break;

		case WM_QUIT:
			::DestroyWindow(hwndDlg);
		break;

		case WM_DESTROY:
			PostQuitMessage(0);
		break;

		default:
			return DefWindowProc(hwndDlg, uMsg, wParam, lParam);
	}
	return TRUE;
}

//============================================================================
static void WindowThreadProc(void *) {

	InitCommonControls();
    s_event = CreateEvent(
        (LPSECURITY_ATTRIBUTES) 0,
        false,          // auto reset
        false,          // initial state off
        (LPCTSTR) 0     // name
    );

	if (TGIsCider)
	{
		if (GetFileAttributes (TG_NEW_DIALOG_PATH) != INVALID_FILE_ATTRIBUTES)
			pTGApp = pTGLaunchUNIXApp (TG_NEW_DIALOG_POPEN_PATH, "w");
		else
			pTGApp = pTGLaunchUNIXApp (TG_OLD_DIALOG_POPEN_PATH, "w");
	}

	s_dialog = ::CreateDialog( s_hInstance, MAKEINTRESOURCE( IDD_DIALOG ), NULL, SplashDialogProc );
	SetWindowText(s_dialog, "URU Launcher");


	::SetDlgItemText( s_dialog, IDC_TEXT, "Initializing patcher...");
	SetTimer(s_dialog, kEventTimer, 250, 0);
	
	char productString[256];
	wchar productStringW[256];
	ProductString(productStringW, arrsize(productStringW));
	StrToAnsi(productString, productStringW, arrsize(productString));
	SendMessage(GetDlgItem(s_dialog, IDC_PRODUCTSTRING), WM_SETTEXT, 0, (LPARAM) productString);
	
	s_dialogCreateEvent.Signal();

	MessagePump(s_dialog);

	if (pTGApp)
	{
		pTGUNIXAppWriteLine (pTGApp, "done");
		pTGUNIXAppClose (pTGApp);
		pTGApp = NULL;
	}

	s_dialog = 0;
	s_shutdown = true;
	s_shutdownEvent.Signal();
}

//============================================================================
static bool TGCheckForFrameworkUpdate ()
{
    // If current.txt doesn't exist, then this is the first time we've been
    // run. Copy version.txt to current.txt and continue starting up
    if (GetFileAttributes (TG_CUR_FRAMEWORK_FILE) == INVALID_FILE_ATTRIBUTES)
    {
		CopyFile (TG_LATEST_FRAMEWORK_FILE, TG_CUR_FRAMEWORK_FILE, FALSE);
		return false;
    }

    // If it does exist, then compare its contents to the contents of the latest version
    // If they match, continue starting up
    FILE *CurFile, *LatestFile;
    CurFile = fopen (TG_CUR_FRAMEWORK_FILE, "rt");
    LatestFile = fopen (TG_LATEST_FRAMEWORK_FILE, "rt");

    char CurVer[64], LatestVer[64];
    CurVer[0] = '\0';
    LatestVer[0] = '\0';
    if (CurFile)
    {
		fgets (CurVer, sizeof (CurVer), CurFile);
		fclose (CurFile);
    }
    if (LatestFile)
    {
		fgets (LatestVer, sizeof (LatestVer), LatestFile);
		fclose (LatestFile);
    }

    if (strcmp (CurVer, LatestVer) == 0)
    return false;

    // Contents don't match. Copy the latest to the current, put up a message box
    // informing the user to restart the game, and exit
    CopyFile (TG_LATEST_FRAMEWORK_FILE, TG_CUR_FRAMEWORK_FILE, FALSE);
    MessageBox (nil, "Game framework requires updating. Please restart URU",
    "URU Launcher", MB_ICONINFORMATION);
    return true;
}

//============================================================================
static void HttpRequestPost(HINTERNET hConnect)
{
	const wchar *path = BuildTypeServerStatusPath();  
	HINTERNET hRequest = 0;
	char data[256] = {0};
	DWORD bytesRead;

	hRequest = WinHttpOpenRequest(
		hConnect, 
		L"POST",
		path,
		NULL, 
		WINHTTP_NO_REFERER, 
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		0
	);
	if(hRequest)
	{	
		BOOL b = WinHttpSendRequest(
			hRequest, 
			L"Content-Type: application/x-www-form-urlencoded\r\n",
			0, 
			(void *)s_postKey, 
			sizeof(s_postKey),
			sizeof(s_postKey), 
			0
		);
		DWORD err = GetLastError();
		WinHttpReceiveResponse(hRequest, 0);
		WinHttpReadData(hRequest, data, arrsize(data)-1, &bytesRead);
		data[bytesRead] = 0;
	}
	if(bytesRead)
		SetStatusText(data);
	WinHttpCloseHandle(hRequest);
}

//============================================================================
static void HttpRequestGet(HINTERNET hConnect)
{
	const wchar *path = BuildTypeServerStatusPath(); 
	HINTERNET hRequest = 0;
	char data[256] = {0};
	DWORD bytesRead;
	
	hRequest = WinHttpOpenRequest(
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
		BOOL b = WinHttpSendRequest(
			hRequest, 
			WINHTTP_NO_ADDITIONAL_HEADERS,
			0, 
			WINHTTP_NO_REQUEST_DATA, 
			0,
			0, 
			0
		);
		if(b)
		{
			DWORD err = GetLastError();
			WinHttpReceiveResponse(hRequest, 0);
			WinHttpReadData(hRequest, data, arrsize(data)-1, &bytesRead);
			data[bytesRead] = 0;
		}
	}
	if(bytesRead)
		SetStatusText(data);
	WinHttpCloseHandle(hRequest);
}

//============================================================================
static void StatusCallback(void *)
{
	HINTERNET hSession = 0;
	HINTERNET hConnect = 0;

	// update while we are running
	while(!s_shutdown)
	{
		if(BuildTypeServerStatusPath())
		{
			hSession = WinHttpOpen(
				L"UruClient/1.0",
				WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
				WINHTTP_NO_PROXY_NAME,
				WINHTTP_NO_PROXY_BYPASS, 
				0
			);

			if(hSession)
			{
				hConnect = WinHttpConnect( 
					hSession, 
					STATUS_PATH, 
					INTERNET_DEFAULT_HTTP_PORT,
					0
				);
				if(hConnect)
				{
					if(StrLen(s_postKey))
						HttpRequestPost(hConnect);
					else
						HttpRequestGet(hConnect);
				}
			}
			
			WinHttpCloseHandle(hConnect);
			WinHttpCloseHandle(hSession);
		}

		for(unsigned i = 0; i < UPDATE_STATUSMSG_SECONDS && !s_shutdown; ++i)
		{
			Sleep(1000);
		}
	}

	s_statusEvent.Signal();
}


/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
void PrepCallback (int id, void *param) {
	s_prepared = true;
	if (id)
		s_shutdown = true;
	else if (TGIsCider && TGCheckForFrameworkUpdate ())
		  s_shutdown = true;

	if (!s_shutdown)
		InitGame();
}

//============================================================================
void InitCallback (int id, void *param) {
	if (id)
		s_shutdown = true;
	if (!s_shutdown)
		StartGame();
}

//=============================================================================
void StartCallback( int id, void *param) {
	if(id == kStatusError) {
		MessageBox(nil, "Failed to launch URU", "URU Launcher", MB_ICONERROR);
	}
	StopGame();
}

//============================================================================
void StopCallback (int id, void *param) { 
	s_shutdown = true;
	TerminateGame();
}

//============================================================================
void TerminateCallback (int id, void *param) {
	s_shutdown = true;
	s_terminated = true;
}

//============================================================================
void ExitCallback (int id, void *param) {
	TerminateGame();
}

//============================================================================
void ProgressCallback (int id, void *param) {
	PatchInfo *patchInfo = (PatchInfo *)param;
	SetProgress(patchInfo->progress);
}

//============================================================================
void SetTextCallback (const char text[]) {
	SetText(text);
}

//============================================================================
void SetStatusTextCallback (const char text[]) {
	SetStatusText(text);
}

//============================================================================
void SetTimeRemainingCallback (unsigned seconds) {
	SetTimeRemaining(seconds);
}

//============================================================================
void SetBytesRemainingCallback (unsigned bytes) {
	SetBytesRemaining(bytes);
}


enum {
	kArgAuthSrv,
	kArgFileSrv,
	kArgGateKeeperSrv,
	kArgNoSelfPatch,
	kArgBuildId,
	kArgCwd,
};

static const CmdArgDef s_cmdLineArgs[] = {
	{ kCmdArgFlagged | kCmdTypeString,		L"AuthSrv",			kArgAuthSrv			},
	{ kCmdArgFlagged | kCmdTypeString,		L"FileSrv",			kArgFileSrv			},
	{ kCmdArgFlagged | kCmdTypeString,		L"GateKeeperSrv",	kArgGateKeeperSrv	},
	{ kCmdArgFlagged | kCmdTypeBool,		L"NoSelfPatch",		kArgNoSelfPatch		},
	{ kCmdArgFlagged | kCmdTypeInt,			L"BuildId",			kArgBuildId			},
	{ kCmdArgFlagged | kCmdTypeBool,		L"Cwd",				kArgCwd				},
};

//============================================================================
int __stdcall WinMain (      
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
){ 
	wchar token[256];
	const wchar *appCmdLine = AppGetCommandLine();
    StrTokenize(&appCmdLine, token, arrsize(token), WHITESPACE);
	while(!StrStr(token, L".exe") && !StrStr(token, L".tmp"))	
	{
		StrTokenize(&appCmdLine, token, arrsize(token), WHITESPACE);
	} 
    while (*appCmdLine == L' ')
		++appCmdLine;

	wchar curPatcherFile[MAX_PATH];
	wchar newPatcherFile[MAX_PATH];
	bool isTempPatcher = false;

	PathGetProgramName(curPatcherFile, arrsize(curPatcherFile));
	PathRemoveFilename(newPatcherFile, curPatcherFile, arrsize(newPatcherFile));
	PathAddFilename(newPatcherFile, newPatcherFile, kPatcherExeFilename, arrsize(newPatcherFile));

	// If our exe name doesn't match the "real" patcher exe name, then we are a newly
	// downloaded patcher that needs to be copied over to the "real" exe.. so do that,
	// exec it, and exit.
	if (0 != StrCmpI(curPatcherFile, newPatcherFile)) {
		isTempPatcher = true;
	}

	CCmdParser cmdParser(s_cmdLineArgs, arrsize(s_cmdLineArgs));
	cmdParser.Parse();
	
	if (!cmdParser.IsSpecified(kArgCwd))
		PathGetProgramDirectory(s_workingDir, arrsize(s_workingDir));

	TGDoCiderDetection ();

	s_hInstance = hInstance;
	ZERO(s_launcherInfo);
	StrPrintf(s_launcherInfo.cmdLine, arrsize(s_launcherInfo.cmdLine), appCmdLine);
	s_launcherInfo.returnCode = 0;

	if(!isTempPatcher)
	{
		// create window thread
		s_thread = (HANDLE)_beginthread(
			WindowThreadProc,
			0,
			nil 
		);
		if (cmdParser.IsSpecified(kArgAuthSrv))
			SetAuthSrvHostname(cmdParser.GetString(kArgAuthSrv));
		if (cmdParser.IsSpecified(kArgFileSrv))
			SetFileSrvHostname(cmdParser.GetString(kArgFileSrv));
		if (cmdParser.IsSpecified(kArgGateKeeperSrv))
			SetGateKeeperSrvHostname(cmdParser.GetString(kArgGateKeeperSrv));
		if(cmdParser.IsSpecified(kArgBuildId))
			s_launcherInfo.buildId = cmdParser.GetInt(kArgBuildId);

		// Wait for the dialog to be created
		s_dialogCreateEvent.Wait(kEventWaitForever);
		_beginthread(StatusCallback, 0, nil);		// get status
	}

	for (;;) {
		// Wait for previous process to exit. This will happen if we just patched.
		HANDLE mutex = CreateMutexW(NULL, TRUE, kPatcherExeFilename);
		DWORD wait = WaitForSingleObject(mutex, 0);
		while(!s_shutdown && wait != WAIT_OBJECT_0)
			wait = WaitForSingleObject(mutex, 100);

		// User canceled			
		if (s_shutdown)
			break;

		// If our exe name doesn't match the "real" patcher exe name, then we are a newly
		// downloaded patcher that needs to be copied over to the "real" exe.. so do that,
		// exec it, and exit.
		if (isTempPatcher) {
//			MessageBox(nil, "Replacing patcher file", "Msg", MB_OK);
		
			// Wait for the other process to exit
			Sleep(1000);
			
			if (!PathDeleteFile(newPatcherFile)) {
				wchar error[256];
				DWORD errorCode = GetLastError();
				wchar *msg = TranslateErrorCode(errorCode);
				
				StrPrintf(error, arrsize(error), L"Failed to delete old patcher executable. %s", msg);
				MessageBoxW(GetTopWindow(nil), error, L"Error", MB_OK);
				LocalFree(msg);
				break;
			}
			if (!PathMoveFile(curPatcherFile, newPatcherFile)) {
				wchar error[256];
				DWORD errorCode = GetLastError();
				wchar *msg = TranslateErrorCode(errorCode);

				StrPrintf(error, arrsize(error), L"Failed to replace old patcher executable. %s", msg);
				MessageBoxW(GetTopWindow(nil), error, L"Error", MB_OK);
				// attempt to clean up this tmp file
				PathDeleteFile(curPatcherFile);
				LocalFree(msg);
				break;
			}

			// launch new patcher
			STARTUPINFOW        si;
			PROCESS_INFORMATION pi;
			ZERO(si);
			ZERO(pi);
			si.cb = sizeof(si);

			wchar cmdline[MAX_PATH];
			StrPrintf(cmdline, arrsize(cmdline), L"%s %s", newPatcherFile, s_launcherInfo.cmdLine);
			
			// we have only successfully patched if we actually launch the new version of the patcher
			(void)CreateProcessW( 
				NULL,
				cmdline,
				NULL,
				NULL,
				FALSE,
				DETACHED_PROCESS,
				NULL,
				NULL,
				&si,
				&pi
			);
			
			SetReturnCode( pi.dwProcessId );
			CloseHandle( pi.hThread );
			CloseHandle( pi.hProcess );
			
			// We're done.
			break;
		}

		// Clean up old temp files
		ARRAY(PathFind) paths;
		wchar fileSpec[MAX_PATH];
		PathGetProgramDirectory(fileSpec, arrsize(fileSpec));
		PathAddFilename(fileSpec, fileSpec, L"*.tmp", arrsize(fileSpec));
		PathFindFiles(&paths, fileSpec, kPathFlagFile);
		for (PathFind * path = paths.Ptr(); path != paths.Term(); ++path)
			PathDeleteFile(path->name);

		// Delete specified files
		for (unsigned i = 0; i < arrsize(s_deleteFiles); ++i) {
			StrCopy(fileSpec, s_workingDir, arrsize(fileSpec));
			PathAddFilename(fileSpec, fileSpec, s_deleteFiles[i], arrsize(fileSpec));
			PathDeleteFile(fileSpec);
		}
		

		SetConsoleCtrlHandler(CtrlHandler, TRUE);
		InitAsyncCore();	// must do this before self patch, since it needs to connect to the file server
	
		// check to see if the patcher needs to be updated, and do it if so.
		ENetError selfPatchResult;
		if (false == (SelfPatch(cmdParser.IsSpecified(kArgNoSelfPatch), &s_shutdown, &selfPatchResult, &s_launcherInfo)) && IS_NET_SUCCESS(selfPatchResult)) {
			// We didn't self-patch, so check for client updates and download them, then exec the client
			StrCopy(s_launcherInfo.path, s_workingDir, arrsize(s_launcherInfo.path));
			s_launcherInfo.prepCallback			= PrepCallback;
			s_launcherInfo.initCallback			= InitCallback;
			s_launcherInfo.startCallback		= StartCallback;
			s_launcherInfo.stopCallback			= StopCallback;
			s_launcherInfo.terminateCallback	= TerminateCallback;
			s_launcherInfo.progressCallback		= ProgressCallback;
			s_launcherInfo.exitCallback			= ExitCallback;
			s_launcherInfo.SetText				= SetTextCallback;
			s_launcherInfo.SetStatusText		= SetStatusTextCallback;
			s_launcherInfo.SetTimeRemaining     = SetTimeRemainingCallback;
			s_launcherInfo.SetBytesRemaining    = SetBytesRemainingCallback;
			s_launcherInfo.IsTGCider            = TGIsCider;
			PrepareGame();

			while (!s_shutdown)		// wait for window to be closed
				AsyncSleep(10);

			StopGame();
			
			// Wait for the PrepareGame thread to exit		
			while (!s_prepared)
				AsyncSleep(10);
			
			// Wait for the StopGame thread to exit		
			while (!s_terminated)
				Sleep(10);
		}
		else if (IS_NET_ERROR(selfPatchResult)) {
			// Self-patch failed
			SetText("Self-patch failed. Exiting...");
			if (!s_shutdown) { 
				wchar str[256];
				StrPrintf(str, arrsize(str), L"Patcher update failed. Error %u, %s", selfPatchResult, NetErrorToString(selfPatchResult));
				MessageBoxW(GetTopWindow(nil), str, L"Error", MB_OK);
			}
		}
		else {
			// We self-patched, so just exit (self-patcher already launched the new patcher.
			// it is now waiting for our process to shutdown and release the shared mutex).
			SetText("Patcher updated. Restarting...");
			s_shutdown = true;
		}
		
		ShutdownAsyncCore();
		s_statusEvent.Wait(kEventWaitForever);
	
		PostMessage(s_dialog, WM_QUIT, 0, 0);		// tell our window to shutdown
		s_shutdownEvent.Wait(kEventWaitForever);	// wait for our window to shutdown
	
		SetConsoleCtrlHandler(CtrlHandler, FALSE);

		if (s_event)
			CloseHandle(s_event);

		s_eventQ.Clear();
		break;
	}

	if (pTGApp)
	{
		pTGUNIXAppWriteLine (pTGApp, "done");
		pTGUNIXAppClose (pTGApp);
		pTGApp = NULL;
	}

    return s_launcherInfo.returnCode;
}

//============================================================================
void SetReturnCode (DWORD retCode) {
	s_launcherInfo.returnCode = retCode;
}


/*****************************************************************************
*
*   Window Events
*
***/

//============================================================================
void SetProgress (unsigned progress) {
	SetProgressEvent *event = NEW(SetProgressEvent);
	event->type = kEventSetProgress;
	event->progress = progress;
	PostEvent(event);
}

//============================================================================
void SetText (const char text[]) {
	SetTextEvent *event = NEW(SetTextEvent);
	event->type = kEventSetText;
	StrCopy(event->text, text, arrsize(event->text));
	PostEvent(event);
}

//============================================================================
void SetStatusText (const char text[]) {
	SetTextEvent *event = NEW(SetTextEvent);
	event->type = kEventSetStatusText;
	StrCopy(event->text, text, arrsize(event->text));
	PostEvent(event);
}

//============================================================================
void SetTimeRemaining (unsigned seconds) {
	SetTimeRemainingEvent *event = new SetTimeRemainingEvent;
	event->type = kEventSetTimeRemaining;
	event->seconds = seconds;
	PostEvent(event);
}

//============================================================================
void SetBytesRemaining (unsigned bytes) {
	SetBytesRemainingEvent *event = new SetBytesRemainingEvent;
	event->type = kEventSetBytesRemaining;
	event->bytes = bytes;
	PostEvent(event);
}
