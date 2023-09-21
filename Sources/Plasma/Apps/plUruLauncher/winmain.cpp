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
#include "plFileSystem.h"
#include "plProduct.h"

#include "pfPatcher/plManifests.h"
#include "pfPatcher/pfPatcher.h"

#include "plWinDpi/plWinDpi.h"

#include "plClientLauncher.h"

#include "hsWindows.h"
#include "resource.h"
#include <string_theory/format>
#include <commctrl.h>
#include <shellapi.h>
#include <shlobj.h>
#include <ShellScalingApi.h>

// ===================================================

#ifndef ERROR_ELEVATION_REQUIRED
    // MinGW is missing this definition
#   define ERROR_ELEVATION_REQUIRED 740
#endif

#define PLASMA_PHAILURE 1
#define PLASMA_OK 0

static HWND             s_dialog;
static ST::string       s_error; // This is highly unfortunate.
static plClientLauncher* s_launcher;
static UINT             s_taskbarCreated = RegisterWindowMessageW(L"TaskbarButtonCreated");
static ITaskbarList3*   s_taskbar = nullptr;

typedef std::unique_ptr<void, std::function<BOOL(HANDLE)>> handleptr_t;

// ===================================================

/** Create a global patcher mutex that is backwards compatible with eap's */
static handleptr_t CreatePatcherMutex()
{
    return handleptr_t(CreateMutexW(nullptr, TRUE, plManifest::PatcherExecutable().WideString().data()),
                       CloseHandle);
}

static bool IsPatcherRunning()
{
    handleptr_t mut = CreatePatcherMutex();
    return WaitForSingleObject(mut.get(), 0) != WAIT_OBJECT_0;
}

static void WaitForOldPatcher()
{
    handleptr_t mut = CreatePatcherMutex();
    DWORD wait = WaitForSingleObject(mut.get(), 0);
    while (wait != WAIT_OBJECT_0) // :( :( :(
        wait = WaitForSingleObject(mut.get(), 100);
    Sleep(1000); // :(
}

// ===================================================

static inline void IShowErrorDialog(const wchar_t* msg)
{
    // This bypasses all that hsClientMinimizeGuard crap we have in CoreLib.
    MessageBoxW(nullptr, msg, L"Error", MB_ICONERROR | MB_OK);
}

static inline void IQuit(int exitCode=PLASMA_OK)
{
    // hey, guess what?
    // PostQuitMessage doesn't work if you're not on the main thread...
    PostMessageW(s_dialog, WM_QUIT, exitCode, 0);
}

static inline void IShowMarquee(bool marquee=true)
{
    // NOTE: This is a HACK to workaround a bug that causes progress bars that were ever
    //       marquees to reanimate when changing the range or position
    ShowWindow(GetDlgItem(s_dialog, IDC_MARQUEE), marquee ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(s_dialog, IDC_PROGRESS), marquee ? SW_HIDE : SW_SHOW);
    PostMessageW(GetDlgItem(s_dialog, IDC_MARQUEE), PBM_SETMARQUEE, static_cast<WPARAM>(marquee), 0);
}

INT_PTR CALLBACK PatcherDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // DPI Helper can eat messages.
    auto result = plWinDpi::Instance().WndProc(hwndDlg, uMsg, wParam, lParam, nullptr);
    if (result.has_value())
        return result.value();

    // NT6 Taskbar Majick
    if (uMsg == s_taskbarCreated) {
        hsRequireCOM();

        if (s_taskbar)
            s_taskbar->Release();
        HRESULT result = CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_ALL, IID_ITaskbarList3, (void**)&s_taskbar);
        if (FAILED(result))
            s_taskbar = nullptr;
    }

    switch (uMsg) {
    case WM_COMMAND:
        // Did they press cancel?
        if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDCANCEL) {
            EnableWindow(GetDlgItem(s_dialog, IDCANCEL), false);
            SetWindowTextW(GetDlgItem(s_dialog, IDC_TEXT), L"Shutting Down...");
            IQuit();
        }
        break;
    case WM_DESTROY:
        if (s_taskbar)
            s_taskbar->Release();
        PostQuitMessage(PLASMA_OK);
        s_dialog = nullptr;
        break;
    case WM_NCHITTEST:
        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LONG_PTR)HTCAPTION);
        return TRUE;
    case WM_QUIT:
        s_launcher->ShutdownNetCore();
        DestroyWindow(hwndDlg);
        break;
    }

    return DefWindowProcW(hwndDlg, uMsg, wParam, lParam);
}

static void ShowPatcherDialog(HINSTANCE hInstance)
{
    s_dialog = ::CreateDialogW(hInstance, MAKEINTRESOURCEW(IDD_DIALOG), nullptr, PatcherDialogProc);
    SetDlgItemTextW(s_dialog, IDC_TEXT, L"Connecting...");
    SetDlgItemTextW(s_dialog, IDC_PRODUCTSTRING, plProduct::ProductString().to_wchar().data());
    SetDlgItemTextW(s_dialog, IDC_DLSIZE, L"");
    SetDlgItemTextW(s_dialog, IDC_DLSPEED, L"");
    IShowMarquee();
}

static void PumpMessages()
{
    MSG msg;
    do {
        // Pump all Win32 messages
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (!IsDialogMessageW(s_dialog, &msg)) {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
        }

        // Now we need to pump the netcore while we have some spare time...
    } while (s_launcher->PumpNetCore());
}

// ===================================================

static void IOnDownloadBegin(const plFileName& file)
{
    ST::string msg = ST::format("Downloading... {}", file);
    SetDlgItemTextW(s_dialog, IDC_TEXT, msg.to_wchar().data());
}

static void IOnProgressTick(uint64_t curBytes, uint64_t totalBytes, const ST::string& status)
{
    // Swap marquee/real progress
    IShowMarquee(false);

    // DL size
    ST::string size = ST::format("{} / {}", plFileSystem::ConvertFileSize(curBytes),
                                 plFileSystem::ConvertFileSize(totalBytes));
    SetDlgItemTextW(s_dialog, IDC_DLSIZE, size.to_wchar().data());

    // DL speed
    SetDlgItemTextW(s_dialog, IDC_DLSPEED, status.to_wchar().data());
    HWND progress = GetDlgItem(s_dialog, IDC_PROGRESS);

    // hey look... ULONGLONG. that's exactly what we need >.<
    if (s_taskbar)
        s_taskbar->SetProgressValue(s_dialog, curBytes, totalBytes);

    // Windows can only do signed 32-bit int progress bars.
    // So, chop it into smaller chunks until we get something we can represent.
    while (totalBytes > INT32_MAX) {
        totalBytes /= 1024;
        curBytes /= 1024;
    }

    PostMessageW(progress, PBM_SETRANGE32, 0, static_cast<int32_t>(totalBytes));
    PostMessageW(progress, PBM_SETPOS, static_cast<int32_t>(curBytes), 0);
}

// ===================================================

static void ISetDownloadStatus(const ST::string& status)
{
    SetDlgItemTextW(s_dialog, IDC_TEXT, status.to_wchar().data());

    // consider this a reset of the download status...
    IShowMarquee();
    SetDlgItemTextW(s_dialog, IDC_DLSIZE, L"");
    SetDlgItemTextW(s_dialog, IDC_DLSPEED, L"");

    if (s_taskbar)
        s_taskbar->SetProgressState(s_dialog, TBPF_INDETERMINATE);
}


static handleptr_t ICreateProcess(const plFileName& exe, const ST::string& args)
{
    STARTUPINFOW        si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    si.cb = sizeof(si);

    // Create wchar things and stuff :/
    ST::string cmd = ST::format("{} {}", exe, args);
    ST::wchar_buffer file = exe.WideString();
    ST::wchar_buffer params = cmd.to_wchar();

    // Guess what? CreateProcess isn't smart enough to throw up an elevation dialog... We need ShellExecute for that.
    // But guess what? ShellExecute won't run ".exe.tmp" files. GAAAAAAAAHHHHHHHHH!!!!!!!
    BOOL result = CreateProcessW(
        file.data(),
        const_cast<wchar_t*>(params.data()),
        nullptr,
        nullptr,
        FALSE,
        DETACHED_PROCESS,
        nullptr,
        nullptr,
        &si,
        &pi
    );

    // So maybe it needs elevation... Or maybe everything arseploded.
    if (result != FALSE) {
        CloseHandle(pi.hThread);
        return handleptr_t(pi.hProcess, CloseHandle);
    } else if (GetLastError() == ERROR_ELEVATION_REQUIRED) {
        SHELLEXECUTEINFOW info;
        memset(&info, 0, sizeof(info));
        info.cbSize = sizeof(info);
        info.lpFile = file.data();
        info.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NOASYNC;
        ST::wchar_buffer argsW = args.to_wchar();
        info.lpParameters = argsW.data();
        ShellExecuteExW(&info);

        return handleptr_t(info.hProcess, CloseHandle);
    } else {
        wchar_t* msg = nullptr;
        FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            nullptr,
            GetLastError(),
            LANG_USER_DEFAULT,
            msg,
            0,
            nullptr
        );
        s_error = ST::string::from_wchar(msg);
        LocalFree(msg);
    }

    return nullptr;
}

static bool IInstallRedist(const plFileName& exe)
{
    ISetDownloadStatus(ST::format("Installing... {}", exe));
    Sleep(2500); // let's Sleep for a bit so the user can see that we're doing something before the UAC dialog pops up!

    // Try to guess some arguments... Unfortunately, the file manifest format is fairly immutable.
    ST::string_stream ss;
    if (exe.AsString().compare_i("oalinst.exe") == 0)
        ss << "/s"; // rarg nonstandard
    else
        ss << "/q";
    if (exe.AsString().find("vcredist", ST::case_insensitive) != -1)
        ss << " /norestart"; // I don't want to image the accusations of viruses and hacking if this happened...

    // Now fire up the process...
    handleptr_t process = ICreateProcess(exe, ss.to_string());
    if (process) {
        WaitForSingleObject(process.get(), INFINITE);

        // Get the exit code so we can indicate success/failure to the redist thread
        DWORD code = PLASMA_OK;
        GetExitCodeProcess(process.get(), &code);

        return code != PLASMA_PHAILURE;
    }
    return PLASMA_PHAILURE;
}

static void ILaunchClientExecutable(const plFileName& exe, const ST::string& args)
{
    // Once we start launching something, we no longer need to trumpet any taskbar status
    if (s_taskbar)
        s_taskbar->SetProgressState(s_dialog, TBPF_NOPROGRESS);

    // Only launch a client executable if we're given one. If not, that's probably a cue that we're
    // done with some service operation and need to go away.
    if (!exe.AsString().empty()) {
        handleptr_t hEvent = handleptr_t(CreateEventW(nullptr, TRUE, FALSE, L"UruPatcherEvent"), CloseHandle);
        handleptr_t process = ICreateProcess(exe, args);

        // if this is the real game client, then we need to make sure it gets this event...
        if (plManifest::ClientExecutable().AsString().compare_i(exe.AsString()) == 0) {
            WaitForInputIdle(process.get(), 1000);
            WaitForSingleObject(hEvent.get(), INFINITE);
        }
    }

    // time to hara-kiri...
    IQuit();
}

static void IOnNetError(ENetError result, const ST::string& msg)
{
    if (s_taskbar)
        s_taskbar->SetProgressState(s_dialog, TBPF_ERROR);

    s_error = ST::format("Error: {}\r\n{}", NetErrorAsString(result), msg);
    IQuit(PLASMA_PHAILURE);
}

static void ISetShardStatus(const ST::string& status)
{
    SetDlgItemTextW(s_dialog, IDC_STATUS_TEXT, status.to_wchar().data());
}

static pfPatcher* IPatcherFactory()
{
    pfPatcher* patcher = new pfPatcher();
    patcher->OnFileDownloadBegin(IOnDownloadBegin);
    patcher->OnProgressTick(IOnProgressTick);

    return patcher;
}

// ===================================================

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLink, int nCmdShow)
{
    plWinDpi::Instance();

    plClientLauncher launcher;
    s_launcher = &launcher;

    // Let's initialize our plClientLauncher friend
    launcher.ParseArguments();
    launcher.SetErrorProc(IOnNetError);
    launcher.SetInstallerProc(IInstallRedist);
    launcher.SetLaunchClientProc(ILaunchClientExecutable);
    launcher.SetPatcherFactory(IPatcherFactory);
    launcher.SetShardProc(ISetShardStatus);
    launcher.SetStatusProc(ISetDownloadStatus);

    // If we're newly updated, then our filename will be something we don't expect!
    // Let's go ahead and take care of that nao.
    if (launcher.CompleteSelfPatch(WaitForOldPatcher))
        return PLASMA_OK; // see you on the other side...

    // Load the doggone server.ini
    ST::string errorMsg = launcher.LoadServerIni();
    if (!errorMsg.empty()) {
        IShowErrorDialog(ST::format("server.ini file not found or invalid. Please check your URU installation.\n{}", errorMsg).to_wchar().c_str());
        return PLASMA_PHAILURE;
    }

    // Ensure there is only ever one patcher running...
    if (IsPatcherRunning()) {
        ST::string text = ST::format("{} is already running", plProduct::LongName());
        IShowErrorDialog(text.to_wchar().data());
        return PLASMA_OK;
    }
    HANDLE _onePatcherMut = CreatePatcherMutex().release();

    // Initialize the network core
    launcher.InitializeNetCore();

    // Welp, now that we know we're (basically) sane, let's create our client window
    // and pump window messages until we're through.
    ShowPatcherDialog(hInstance);
    PumpMessages();

    // So there appears to be some sort of issue with calling MessageBox once we've set up our dialog...
    // WTF?!?! So, to hack around that, we'll wait until everything shuts down to display any error.
    if (!s_error.empty())
        IShowErrorDialog(s_error.to_wchar().data());

    // Alrighty now we just need to clean up behind ourselves!
    // NOTE: We shut down the netcore in the WM_QUIT handler so
    //       we don't have a windowless, zombie process if that takes
    //       awhile (it can... dang eap...)
    ReleaseMutex(_onePatcherMut);
    CloseHandle(_onePatcherMut);

    // kthxbai
    return s_error.empty() ? PLASMA_OK : PLASMA_PHAILURE;
}

