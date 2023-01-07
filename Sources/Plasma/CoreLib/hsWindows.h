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

#ifndef _hsWindows_inc_
#define _hsWindows_inc_

#include <optional>
#include <string_theory/format>

/** \file hsWindows.h
 *  \brief Pulls in Windows core headers
 *
 *  This file pulls in the core Windows headers and Winsock2. It is separate from
 *  HeadSpin.h to improve build times and to facillitate adding precompiled headers.
 *  You should avoid including this header from other headers!
 */

#ifdef HS_BUILD_FOR_WIN32
    // Force Windows headers to assume Windows 7 compatibility
#   ifdef _WIN32_WINNT
#       undef _WIN32_WINNT
#   endif
#   define _WIN32_WINNT 0x601

    // HACK: Max headers depend on the min() and max() macros normally pulled
    // in by windows.h... However, we usually disable those, since they break
    // std::min and std::max.  Therefore, we bring the std:: versions down to
    // the global namespace so we can still compile max code without breaking
    // everything else :/
#   ifndef NOMINMAX
#       define NOMINMAX
#       include <algorithm>
        using std::min;
        using std::max;
#   endif

#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   include <ws2tcpip.h> // Pulls in WinSock 2 for us

    // This needs to be after #include <windows.h>, since it also includes windows.h
#   ifdef USE_VLD
#       include <vld.h>
#   endif // USE_VLD

    // Conflicts with plDynamicTextMap
#   ifdef DrawText
#       undef DrawText
#   endif

    // Conflicts with plSynchedObject::StateDefn
#   ifdef GetObject
#       undef GetObject
#   endif


    const RTL_OSVERSIONINFOEXW& hsGetWindowsVersion();

    // Initializes COM exactly once the first time it's called.
    void hsRequireCOM();

    /** Represents the last Win32 error. */
    struct hsLastWin32Error_Type {};
    constexpr hsLastWin32Error_Type hsLastWin32Error;

    /** COM Result holder used for formatting to log. */
    struct hsCOMError
    {
        HRESULT fResult;

        hsCOMError() : fResult() { }
        hsCOMError(hsLastWin32Error_Type, DWORD error) : fResult(HRESULT_FROM_WIN32(error)) { }
        hsCOMError(HRESULT r) : fResult(r) { }
        hsCOMError& operator =(const hsCOMError&) = delete;
        hsCOMError& operator =(HRESULT r) { fResult = r; return *this; }
        operator HRESULT() const { return fResult; }

        ST::string ToString() const
        {
            wchar_t* msg = nullptr;
            auto result = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                         nullptr, fResult, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&msg, 0, nullptr);
            if (result && msg) {
                ST::char_buffer utf8 = ST::string::from_wchar(msg, result, ST::assume_valid).to_utf8();
                LocalFree(msg);
                return utf8;
            } else {
                return ST::format("unknown HRESULT 0x{8X}", fResult);
            }
        }
    };

    inline void format_type(const ST::format_spec& format, ST::format_writer& output, const hsCOMError& hr)
    {
        wchar_t* msg = nullptr;
        auto result = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                     nullptr, hr.fResult, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&msg, 0, nullptr);
        if (result && msg) {
            ST::char_buffer utf8 = ST::string::from_wchar(msg, result, ST::assume_valid).to_utf8();
            output.append(utf8.data(), utf8.size());
            LocalFree(msg);
        } else {
            output.append("unknown HRRESULT 0x");
            ST::format_type(format, output, hr.fResult);
        }
    }

    template<class, class...>
    class plOptionalWinCall;

    template<
        class _ReturnT,
        class... _ArgsT
    >
    class plOptionalWinCall<_ReturnT(_ArgsT...)>
    {
        using _FuncPtrT = _ReturnT(*)(_ArgsT...);

        HMODULE fHModule;
        _FuncPtrT fProc;

    public:
        plOptionalWinCall(const wchar_t* mod, const char* func)
            : fHModule(), fProc()
        {
            fHModule = LoadLibraryW(mod);
            if (fHModule) {
                fProc = (_FuncPtrT)GetProcAddress(fHModule, func);
                if (!fProc) {
                    FreeLibrary(fHModule);
                    fHModule = nullptr;
                }
            }
        }

        ~plOptionalWinCall()
        {
            if (fHModule)
                FreeLibrary(fHModule);
        }

        std::optional<_ReturnT> operator()(_ArgsT... args) const
        {
            if (fProc)
                return (_ReturnT)fProc(std::forward<_ArgsT>(args)...);
            return std::nullopt;
        }

        operator bool() const
        {
            return fProc != nullptr;
        }

        _FuncPtrT Get() const { return fProc; }
    };
#endif // HS_BUILD_FOR_WIN32

#endif // _hsWindows_inc_
