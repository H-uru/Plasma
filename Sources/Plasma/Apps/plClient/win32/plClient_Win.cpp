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

#include "hsWindows.h"
#include "hsResMgr.h"
#include "hsTimer.h"
#include "plTimerCallbackManager.h"

#include <shlobj.h>
#include <shellapi.h>

#include "plClient.h"
#include "win32/plWinDpi.h"

#include "pnFactory/plFactory.h"
#include "pnNetCommon/plNetApp.h"
#include "plProgressMgr/plProgressMgr.h"

extern ITaskbarList3* gTaskbarList;
static std::vector<HMODULE> fLoadedDLLs;

void plClient::IResizeNativeDisplayDevice(int width, int height, bool windowed)
{
    uint32_t winStyle, winExStyle;
    if (windowed) {
        // WS_VISIBLE appears necessary to avoid leaving behind framebuffer junk when going from windowed to a smaller window
        winStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE;
        winExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    } else {
        winStyle = WS_VISIBLE;
        winExStyle = WS_EX_APPWINDOW;
    }
    SetWindowLongPtr(fWindowHndl, GWL_STYLE, winStyle);
    SetWindowLongPtr(fWindowHndl, GWL_EXSTYLE, winExStyle);

    uint32_t flags = SWP_NOCOPYBITS | SWP_SHOWWINDOW | SWP_FRAMECHANGED;

    // The window rect will be (left, top, width, height)
    RECT winRect{ 0, 0, width, height };
    if (windowed) {
        if (GetClientRect(fWindowHndl, &winRect) != FALSE) {
            MapWindowPoints(fWindowHndl, nullptr, reinterpret_cast<LPPOINT>(&winRect), 2);
            winRect.right = winRect.left + width;
            winRect.bottom = winRect.top + height;
        }

        UINT dpi = plWinDpi::Instance().GetDpi(fWindowHndl);
        plWinDpi::Instance().AdjustWindowRectEx(&winRect, winStyle, false, winExStyle, dpi);

        winRect.right = winRect.right - winRect.left;
        winRect.bottom = winRect.bottom - winRect.top;
    }
    SetWindowPos(fWindowHndl, HWND_NOTOPMOST, winRect.left, winRect.top, winRect.right, winRect.bottom, flags);
}

void plClient::IChangeResolution(int width, int height)
{
    // First, we need to be mindful that we may not be operating on the primary display device
    // I unfortunately cannot test this works as expected, but it will likely save us some cursing
    HMONITOR monitor = MonitorFromWindow(fWindowHndl, MONITOR_DEFAULTTONULL);
    if (!monitor)
        return;
    MONITORINFOEXW moninfo;
    memset(&moninfo, 0, sizeof(moninfo));
    moninfo.cbSize = sizeof(moninfo);
    GetMonitorInfoW(monitor, &moninfo);

    // Fetch a base display settings
    DEVMODEW devmode;
    memset(&devmode, 0, sizeof(devmode));
    devmode.dmSize = sizeof(devmode);
    EnumDisplaySettingsW(moninfo.szDevice, ENUM_REGISTRY_SETTINGS, &devmode);

    // Actually update the resolution
    if (width != 0 && height != 0) {
        devmode.dmPelsWidth = width;
        devmode.dmPelsHeight = height;
    }
    ChangeDisplaySettingsExW(moninfo.szDevice, &devmode, nullptr, CDS_FULLSCREEN, nullptr);
}

// Increments the taskbar progress [Windows 7+]
void plClient::IUpdateProgressIndicator(plOperationProgress* progress)
{
    if (gTaskbarList && fInstance->GetWindowHandle())
    {
        static TBPFLAG lastState = TBPF_NOPROGRESS;
        TBPFLAG myState;

        // So, calling making these kernel calls is kind of SLOW. So, let's
        // hide that behind a userland check--this helps linking go faster!
        if (progress->IsAborting())
            myState = TBPF_ERROR;
        else if (progress->IsLastUpdate())
            myState = TBPF_NOPROGRESS;
        else if (progress->GetMax() == 0.f)
            myState = TBPF_INDETERMINATE;
        else
            myState = TBPF_NORMAL;

        if (myState == TBPF_NORMAL)
            // This sets us to TBPF_NORMAL
            gTaskbarList->SetProgressValue(fInstance->GetWindowHandle(), (ULONGLONG)progress->GetProgress(), (ULONGLONG)progress->GetMax());
        else if (myState != lastState)
            gTaskbarList->SetProgressState(fInstance->GetWindowHandle(), myState);
        lastState = myState;
    }
}

void plClient::InitDLLs()
{
    hsStatusMessage("Init dlls client\n");
    std::vector<plFileName> dlls = plFileSystem::ListDir("ModDLL", "*.dll");
    for (auto iter = dlls.begin(); iter != dlls.end(); ++iter)
    {
        HMODULE hMod = LoadLibraryW(iter->WideString().data());
        if (hMod)
        {
            pInitGlobalsFunc initGlobals = (pInitGlobalsFunc)GetProcAddress(hMod, "InitGlobals");
            (*initGlobals)(hsgResMgr::ResMgr(), plFactory::GetTheFactory(), plgTimerCallbackMgr::Mgr(),
                hsTimer::GetTheTimer(), plNetClientApp::GetInstance());
            fLoadedDLLs.emplace_back(hMod);
        }
    }
}

void plClient::ShutdownDLLs()
{
    for (HMODULE dll : fLoadedDLLs)
    {
        BOOL ret = FreeLibrary(dll);
        if (!ret)
            hsStatusMessage("Failed to free lib\n");
    }
    fLoadedDLLs.clear();
}

// Show the client window
void plClient::ShowClientWindow()
{
    ShowWindow(fWindowHndl, SW_SHOW);
    BringWindowToTop(fWindowHndl);
}

void plClient::FlashWindow()
{
    FLASHWINFO info;
    info.cbSize = sizeof(info);
    info.dwFlags = FLASHW_TIMERNOFG | FLASHW_ALL;
    info.hwnd = fWindowHndl;
    info.uCount = -1;
    FlashWindowEx(&info);
}
