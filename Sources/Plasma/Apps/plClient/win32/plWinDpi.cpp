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

#include "plWinDpi.h"

#include <memory>
#include <type_traits>

#include "hsResMgr.h"

#include "pnKeyedObject/plFixedKey.h"
#include "pnMessage/plClientMsg.h"

#ifndef WM_GETDPISCALEDSIZE
#   define WM_GETDPISCALEDSIZE 0x02E4
#endif

static std::unique_ptr<plWinDpi> s_instance;

plWinDpi& plWinDpi::Instance()
{
    if (!s_instance)
        s_instance = std::make_unique<plWinDpi>();
    return *s_instance.get();
}

plWinDpi::plWinDpi()
    : fAdjustWindowRectExForDpi(L"user32", "AdjustWindowRectExForDpi"),
      fAreDpiAwarenessContextsEqual(L"user32", "AreDpiAwarenessContextsEqual"),
      fEnableNonClientDpiScaling(L"user32", "EnableNonClientDpiScaling"),
      fGetDpiForMonitor(L"shcore", "GetDpiForMonitor"),
      fGetDpiForSystem(L"user32", "GetDpiForSystem"),
      fGetDpiForWindow(L"user32", "GetDpiForWindow"),
      fGetSystemDpiForProcess(L"user32", "GetSystemDpiForProcess"),
      fGetSystemMetricsForDpi(L"user32", "GetSystemMetricsForDpi"),
      fGetThreadDpiAwarenessContext(L"user32", "GetThreadDpiAwarenessContext"),
      fSetProcessDpiAwareness(L"shcore", "SetProcessDpiAwareness"),
      fSetProcessDpiAwarenessContext(L"user32", "SetProcessDpiAwarenessContext")
{
    Initialize();
}

void plWinDpi::Initialize() const
{
    // There are three different levels of DPI awareness that we can deal with,
    // each was added in newer versions of Windows as monitors became more and
    // more sexy.
    // In Vista, the entire system had an adjustable display scale.
    // In Windows 8, each monitor was allowed to have a separate scale.
    // In Windows 10, the per-monitor scaling was slowly improved to automatically
    // scale dialog boxes and things like that.
    // So we need to try to handle all of that. We will assume the baseline is Vista.
    // Any API added later than that will need to be called on a provisional basis.
    LogWhite("--- Begin DPI Awareness ---");
    do {
        // Per Monitor V2 for Windows 10 v1703.
        auto perMonitorV2Result = fSetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        if (perMonitorV2Result.has_value()) {
            if (perMonitorV2Result.value() == TRUE) {
                LogGreen("Using Per Monitor v2");
                break;
            }
            LogRed("Failed to set DPI Awareness Per Monitor v2: {}", hsCOMError(hsLastWin32Error, GetLastError()));
        } else {
            LogYellow("Per Monitor v2 isn't supported on this OS.");
        }

        // Per Monitor v1 for Windows 8.1.
        auto perMonitorV1Result = fSetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
        if (perMonitorV1Result.has_value()) {
            if (SUCCEEDED(perMonitorV1Result.value())) {
                LogGreen("Using Per Monitor V1");
                break;
            } else {
                LogRed("Failed to set DPI Awareness Per Monitor v1: {}", hsCOMError(perMonitorV1Result.value()));
            }
        }

        // System DPI Awareness.
        if (SetProcessDPIAware() != FALSE) {
            LogGreen("Using System DPI Awareness");
            break;
        } else {
            LogRed("Failed to set System DPI Awareness: {}", hsCOMError(hsLastWin32Error, GetLastError()));
        }
    } while (false);
    LogWhite("--- End DPI Awareness ---");
}

BOOL plWinDpi::AdjustWindowRectEx(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle, UINT dpi) const
{
    // If the application is per-monitor aware, MSDN explicitly says to NOT use AdjustWindowRectEx, so
    // we use the proper function if it's available.
    auto dpiResult = fAdjustWindowRectExForDpi(lpRect, dwStyle, bMenu, dwExStyle, dpi);
    if (dpiResult.has_value())
        return dpiResult.value();

    // The ForDpi variant only scales the non-client part of the window rect. If we've enabled
    // non-client scaling, then the ForDpi variant should be available, so it's ok to simply
    // fall through to the old non-dpi aware variant.
    return ::AdjustWindowRectEx(lpRect, dwStyle, bMenu, dwExStyle);
}

UINT plWinDpi::GetDpi(HWND hWnd) const
{
    if (hWnd == nullptr) {
        // Windows 10 v1803 for per-process system DPI.
        auto systemDpiForProcess = fGetSystemDpiForProcess(GetCurrentProcess());
        if (systemDpiForProcess.has_value())
            return systemDpiForProcess.value();

        // Windows 10 v1607 for system DPI.
        auto systemDpi = fGetDpiForSystem();
        if (systemDpi.has_value())
            return systemDpi.value();
    } else {
        // Windows 10 v107 for per-window DPI.
        auto windowDpi = fGetDpiForWindow(hWnd);
        if (windowDpi.has_value())
            return windowDpi.value();
    }

    // Windows 8.1 for per-monitor DPI.
    HMONITOR monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
    UINT dpiX, dpiY;
    auto dpiForMonitor = fGetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
    if (dpiForMonitor.has_value()) {
        if (SUCCEEDED(dpiForMonitor.value()))
            return UINT(float(dpiY) / 96.0f);
        LogRed("Per-Monitor DPI failed: {}", hsCOMError(dpiForMonitor.value()));
    }

    // Legacy DPI computation... This is both slow and lacks much knowledge about DPI.
    HDC hdc = GetDC(hWnd);
    int ydpi = GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(hWnd, hdc);
    return UINT(float(ydpi) / 96.0f);
}

int plWinDpi::GetSystemMetrics(int nIndex, std::variant<UINT, HWND, std::monostate> dpiArg) const
{
    UINT dpi;
    std::visit(
        [this, &dpi](auto&& arg) {
            using _DpiArgT = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<_DpiArgT, UINT>) {
                dpi = arg;
            } else if constexpr (std::is_same_v<_DpiArgT, HWND>) {
                dpi = GetDpi(arg);
            } else {
                dpi = GetDpi();
            }
        }, dpiArg
    );

    // Windows 10 v1607
    std::optional<int> metricsForDpi = fGetSystemMetricsForDpi(nIndex, dpi);
    if (metricsForDpi.has_value())
        return metricsForDpi.value();

    // Welcome to the late Triassic
    int metrics = ::GetSystemMetrics(nIndex);
    return MulDiv(metrics, dpi, 96);
}

BOOL plWinDpi::ICalcWinSize(HWND hWnd, UINT dpi, SIZE& hWndSize) const
{
    // Assumption: You only care about doing this on Windows 10 v1703.
    auto* adjustWindowRect = fAdjustWindowRectExForDpi.Get();
    if (adjustWindowRect) {
        RECT rect{};
        GetClientRect(hWnd, &rect);
        MapWindowPoints(hWnd, nullptr, (LPPOINT)&rect, 2);

        LONG_PTR dwStyle = GetWindowLongPtrW(hWnd, GWL_STYLE);
        LONG_PTR dwExStyle = GetWindowLongPtrW(hWnd, GWL_EXSTYLE);
        BOOL result = adjustWindowRect(&rect, (DWORD)dwStyle, FALSE, (DWORD)dwExStyle, dpi);
        if (result != FALSE) {
            hWndSize.cx = rect.right - rect.left;
            hWndSize.cy = rect.bottom - rect.top;
            return TRUE;
        }
    }
    return FALSE;
}

void plWinDpi::IEnableNCScaling(HWND hWnd) const
{
    // The code would be more complicated with std::optional, so just grab
    // weak references to the function pointers and bail if one isn't available.
    auto* GetThreadDpiAwarenessContext = fGetThreadDpiAwarenessContext.Get();
    auto* AreDpiAwarenessContextsEqual = fAreDpiAwarenessContextsEqual.Get();
    auto* EnableNonClientDpiScaling = fEnableNonClientDpiScaling.Get();
    if (!(GetThreadDpiAwarenessContext && AreDpiAwarenessContextsEqual && EnableNonClientDpiScaling)) {
        LogYellow("Non-client DPI scaling is not supported on this OS.");
        return;
    }

    // Windows 10 v1607 added non-client area scaling to Per Monitor Awareness v1, but
    // it needs to be explicitly enabled. Per Monitor Awareness v2 does this automatically,
    // so there is no need to do anything.
    DPI_AWARENESS_CONTEXT dpiAwarenessContext = GetThreadDpiAwarenessContext();
    if (AreDpiAwarenessContextsEqual(dpiAwarenessContext, DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE)) {
        LogWhite("Enabling non-client DPI scaling...");
        if (EnableNonClientDpiScaling(hWnd) != FALSE)
            LogGreen("... success!");
        else
            LogRed("... failed: {}", hsCOMError(hsLastWin32Error, GetLastError()));
    } else if (AreDpiAwarenessContextsEqual(dpiAwarenessContext, DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
        LogGreen("We already have non-client DPI scaling!");
    } else {
        LogYellow("The DPI Awareness mode we're using doesn't support non-client DPI scaling :(");
    }
}

void plWinDpi::IHandleDpiChange(HWND hWnd, UINT dpi, const RECT& rect) const
{
    float scale = float(dpi) / 96.0f;
    LogWhite("Window DPI changed to {} (scale factor: {.02f})", dpi, scale);

    // Inform the engine about the new DPI.
    auto* msg = new plDisplayScaleChangedMsg(scale, ConvertRect(rect));
    msg->Send();
}

std::optional<LRESULT> plWinDpi::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) const
{
    switch (msg) {
    case WM_NCCREATE:
        IEnableNCScaling(hWnd);
        break;
    case WM_DPICHANGED:
        // For Windows 8.1 and higher. Allows us to handle DPI changes while the game is running,
        // eg by moving the game window to another monitor with a different scale factor or
        // the user changed the scale factor while the game is running.
        IHandleDpiChange(hWnd, LOWORD(wParam), *((LPRECT)lParam));
        return 0;
    case WM_GETDPISCALEDSIZE:
        // For Windows 10 v1703 and higher. This window message allows us to tell the
        // OS the desired size of the application window sent by WM_DPICHANGED. This
        // does mean that on Windows 10 v1607 and lower that the game window itself
        // will change size. These versions of Windows are all EOL, however.
        return ICalcWinSize(hWnd, (UINT)wParam, *(SIZE*)lParam);
    }

    // Indicates that the user may process the window message.
    return std::nullopt;
}
