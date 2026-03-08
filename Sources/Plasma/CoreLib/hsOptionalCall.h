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

#ifndef _hsOptionalCall_h
#define _hsOptionalCall_h

#include <optional>
#include <type_traits>

#include "HeadSpin.h"
#include "plFileSystem.h"

#ifdef HS_BUILD_FOR_WIN32
#   include "hsWindows.h"
#else
#   include <dlfcn.h>
#endif

template<class, class...>
class hsOptionalCall;

class hsOptionalCallBase
{
protected:
    hsLibraryHndl fLibrary;
    void* fValue;

public:
    template<class _StrT>
    hsOptionalCallBase(const _StrT* lib, const char* func)
        : fLibrary(), fValue()
    {
        static_assert(std::is_same_v<_StrT, char> || std::is_same_v<_StrT, wchar_t>);

#ifdef HS_BUILD_FOR_WIN32
        if constexpr (std::is_same_v<_StrT, char>)
            fLibrary = LoadLibraryA(lib);
        else
            fLibrary = LoadLibraryW(lib);

        if (fLibrary) {
            fValue = reinterpret_cast<void*>(GetProcAddress(fLibrary, func));
            if (!fValue) {
                FreeLibrary(fLibrary);
                fLibrary = nullptr;
            }
        }
#else
        static_assert(std::is_same_v<_StrT, char>);

        fLibrary = dlopen(lib, RTLD_LAZY | RTLD_LOCAL);
        if (fLibrary) {
            fValue = dlsym(fLibrary, func);
            if (!fValue) {
                dlclose(fLibrary);
                fLibrary = nullptr;
            }
        }
#endif
    }

    ~hsOptionalCallBase()
    {
#ifdef HS_BUILD_FOR_WIN32
        if (fLibrary)
            FreeLibrary(fLibrary);
#else
        if (fLibrary)
            dlclose(fLibrary);
#endif
    }

    operator bool() const
    {
        return fValue != nullptr;
    }
};

template<class _ReturnT, class... _ArgsT>
class hsOptionalCall<_ReturnT(_ArgsT...)> : public hsOptionalCallBase
{
    using _FuncPtrT = _ReturnT(*)(_ArgsT...);
    using _ResultT = typename std::conditional<std::is_void<_ReturnT>::value, std::nullptr_t, _ReturnT>::type;

public:
    template<class _StrT>
    hsOptionalCall(const _StrT* lib, const char* func)
        : hsOptionalCallBase(lib, func)
    { }

    std::optional<_ResultT> operator()(_ArgsT... args) const
    {
        _FuncPtrT proc = Get();
        hsAssert(proc, "Trying to invoke undefined function");

        if (proc) {
            if constexpr (std::is_void<_ReturnT>::value) {
                return proc(std::forward<_ArgsT>(args)...), nullptr;
            } else {
                return (_ResultT)proc(std::forward<_ArgsT>(args)...);
            }
        }
        return std::nullopt;
    }

    _FuncPtrT Get() const
    {
        return reinterpret_cast<_FuncPtrT>(fValue);
    }
};

template<class _ReturnT, class... _ArgsT>
class hsOptionalCall<_ReturnT(_ArgsT..., ...)> : public hsOptionalCallBase
{
    using _FuncPtrT = _ReturnT(*)(_ArgsT..., ...);
    using _ResultT = typename std::conditional<std::is_void<_ReturnT>::value, std::nullptr_t, _ReturnT>::type;

public:
    template<class _StrT>
    hsOptionalCall(const _StrT* lib, const char* func)
        : hsOptionalCallBase(lib, func)
    { }

    template<class... _VarArgsT>
    std::optional<_ResultT> operator()(_ArgsT... args, _VarArgsT... varArgs) const
    {
        _FuncPtrT proc = Get();
        hsAssert(proc, "Trying to invoke undefined function");

        if (proc) {
            if constexpr (std::is_void<_ReturnT>::value) {
                return proc(std::forward<_ArgsT>(args)..., std::forward<_VarArgsT>(varArgs)...), nullptr;
            } else {
                return (_ResultT)proc(std::forward<_ArgsT>(args)..., std::forward<_VarArgsT>(varArgs)...);
            }
        }
        return std::nullopt;
    }

    _FuncPtrT Get() const
    {
        return reinterpret_cast<_FuncPtrT>(fValue);
    }
};

template<class _ValueT>
class hsOptionalCall<_ValueT> : public hsOptionalCallBase
{
public:
    template<class _StrT>
    hsOptionalCall(const _StrT* lib, const char* func)
        : hsOptionalCallBase(lib, func)
    { }

    _ValueT operator *() const
    {
        return Get();
    }

    _ValueT* operator &() const
    {
        return reinterpret_cast<_ValueT*>(fValue);
    }

    _ValueT Get() const
    {
        return *reinterpret_cast<_ValueT*>(fValue);
    }
};

#if defined(HS_BUILD_FOR_WIN32)
template<class, class...>
class plOptionalWinCall;

template<class _ReturnT, class... _ArgsT>
class plOptionalWinCall<_ReturnT(_ArgsT...)> : public hsOptionalCallBase
{
    using _FuncPtrT = _ReturnT(WINAPI *)(_ArgsT...);
    using _ResultT = typename std::conditional<std::is_void<_ReturnT>::value, std::nullptr_t, _ReturnT>::type;

public:
    template<class _StrT>
    plOptionalWinCall(const _StrT* lib, const char* func)
        : hsOptionalCallBase(lib, func)
    { }

    std::optional<_ResultT> operator()(_ArgsT... args) const
    {
        _FuncPtrT proc = Get();
        hsAssert(proc, "Trying to invoke undefined function");

        if (proc) {
            if constexpr (std::is_void<_ReturnT>::value) {
                return proc(std::forward<_ArgsT>(args)...), nullptr;
            } else {
                return (_ResultT)proc(std::forward<_ArgsT>(args)...);
            }
        }
        return std::nullopt;
    }

    _FuncPtrT Get() const
    {
        return reinterpret_cast<_FuncPtrT>(fValue);
    }
};
#endif


#if defined(HS_BUILD_FOR_WIN32)
#   define HS_OPTIONAL_LIB_SUFFIX ""
#elif defined(HS_BUILD_FOR_APPLE)
#   define HS_OPTIONAL_LIB_SUFFIX ".dylib"
#else
#   define HS_OPTIONAL_LIB_SUFFIX ".so"
#endif

#define hsOptionalCallDecl(lib, func) \
    static hsOptionalCall<decltype(func)> __##func(lib HS_OPTIONAL_LIB_SUFFIX, #func);

#endif // _hsOptionalCall_h
