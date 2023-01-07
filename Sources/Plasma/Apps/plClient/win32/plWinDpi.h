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

#ifndef _plWinDpi_h_inc_
#define _plWinDpi_h_inc_

#include <optional>
#include <variant>

#include "hsWindows.h"
#include <ShellScalingApi.h>

#include "plMessage/plDisplayScaleChangedMsg.h"
#include "plStatusLog/plStatusLog.h"

class plWinDpi
{
protected:
    plOptionalWinCall<BOOL(RECT*, DWORD, BOOL, DWORD, UINT)> fAdjustWindowRectExForDpi;
    plOptionalWinCall<BOOL(DPI_AWARENESS_CONTEXT, DPI_AWARENESS_CONTEXT)> fAreDpiAwarenessContextsEqual;
    plOptionalWinCall<BOOL(HWND)> fEnableNonClientDpiScaling;
    plOptionalWinCall<HRESULT(HMONITOR, MONITOR_DPI_TYPE, UINT*, UINT*)> fGetDpiForMonitor;
    plOptionalWinCall<UINT(void)> fGetDpiForSystem;
    plOptionalWinCall<UINT(HWND)> fGetDpiForWindow;
    plOptionalWinCall<UINT(HANDLE)> fGetSystemDpiForProcess;
    plOptionalWinCall<int(int, UINT)> fGetSystemMetricsForDpi;
    plOptionalWinCall<DPI_AWARENESS_CONTEXT(void)> fGetThreadDpiAwarenessContext;
    plOptionalWinCall<HRESULT(PROCESS_DPI_AWARENESS)> fSetProcessDpiAwareness;
    plOptionalWinCall<BOOL(DPI_AWARENESS_CONTEXT)> fSetProcessDpiAwarenessContext;

    template<typename... _ArgsT>
    void LogGreen(const char* fmt, _ArgsT&&... args) const
    {
        plStatusLog::AddLineSF("plasmadbg.log", plStatusLog::kGreen, fmt, std::forward<_ArgsT>(args)...);
    }

    template<typename... _ArgsT>
    void LogRed(const char* fmt, _ArgsT&&... args) const
    {
        plStatusLog::AddLineSF("plasmadbg.log", plStatusLog::kRed, fmt, std::forward<_ArgsT>(args)...);
    }

    template<typename... _ArgsT>
    void LogWhite(const char* fmt, _ArgsT&&... args) const
    {
        plStatusLog::AddLineSF("plasmadbg.log", plStatusLog::kWhite, fmt, std::forward<_ArgsT>(args)...);
    }

    template<typename... _ArgsT>
    void LogYellow(const char* fmt, _ArgsT&&... args) const
    {
        plStatusLog::AddLineSF("plasmadbg.log", plStatusLog::kYellow, fmt, std::forward<_ArgsT>(args)...);
    }

    void Initialize() const;

public:
    plWinDpi();
    static plWinDpi& Instance();

public:
    /** Polyfill for AdjustsWindowRectExForDpi */
    BOOL AdjustWindowRectEx(LPRECT lprect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle, UINT dpi) const;

    UINT GetDpi(HWND hWnd = nullptr) const;

    /**
     * Polyfill for GetSystemMetricsForDpi
     * \param in[dpi] This can be either a DPI value, a window handle, or null. If a window
     *                is supplied, we do our best to determine the DPI of the supplied window.
     *                Otherwise, the system DPI is used.
     */
    int GetSystemMetrics(int nIndex, std::variant<UINT, HWND, std::monostate> dpi = nullptr) const;

public:
    float GetScale(HWND hWnd = nullptr) const { return float(GetDpi(hWnd)) / 96.0f; }

    static RECT ConvertRect(const plDisplayScaleChangedMsg::ClientWindow& rect)
    {
        return *((const LPRECT)&rect);
    }

    static plDisplayScaleChangedMsg::ClientWindow ConvertRect(const RECT& rect)
    {
        return *((const plDisplayScaleChangedMsg::ClientWindow*)&rect);
    }

private:
    void IEnableNCScaling(HWND hWnd) const;
    void IHandleDpiChange(HWND hWnd, UINT dpi, const RECT& rect) const;

public:
    std::optional<LRESULT> WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) const;
};

#endif
