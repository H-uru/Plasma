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
#ifndef HeadSpinHDefined
#define HeadSpinHDefined

// Ensure these get set consistently regardless of what module includes it
#include "hsConfig.h"

//======================================
// Some standard includes
//======================================
#include <cfloat>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cstdarg>
#include <cstdint>
#include <type_traits>

namespace ST { class string; }

//======================================
// Winblows Hacks
//======================================
#ifdef HS_BUILD_FOR_WIN32
    // Kind of nasty looking forward declarations, but this is Win32.... it'll never change!
    // If you want to argue: would you rather pull in the entire Windows.h? Windows 8 makes it
    // even more bloated than before!
    struct HWND__; typedef struct HWND__ *HWND;
    struct HINSTANCE__; typedef struct HINSTANCE__ *HINSTANCE;

    typedef HWND hsWindowHndl;
    typedef HWND hsDisplayHndl;
    typedef HINSTANCE hsWindowInst;
    typedef HINSTANCE HMODULE;
    typedef HMODULE hsLibraryHndl;
    typedef long HRESULT;
    typedef void* HANDLE;
#elif HS_BUILD_FOR_APPLE
    // Same note as Windows above - would rather not forward declare but I don't want to
    // import Foundation or CoreGraphics
#ifdef HS_BUILD_FOR_IOS
    // Exception - iOS doesn't support CGDirectDisplayID.
    // It has UIScreen but that's a Cocoa type.
    typedef void* hsDisplayHndl;
#else
    typedef uint32_t CGDirectDisplayID;
    typedef CGDirectDisplayID hsDisplayHndl;
#endif
    typedef void* hsWindowHndl;
    typedef void* hsWindowInst;
    typedef void* hsLibraryHndl;
#else
    typedef int32_t* hsWindowHndl;
    typedef int32_t* hsDisplayHndl;
    typedef int32_t* hsWindowInst;
    typedef void* hsLibraryHndl;
#endif // HS_BUILD_FOR_WIN32

//======================================
// Basic macros
//======================================
#ifdef HS_BUILD_FOR_WIN32
#   ifndef CDECL
#       define CDECL __cdecl
#   endif
#else
#   define CDECL
#endif

// Indirection required for joining preprocessor macros together
#define _hsMacroJoin_(lhs, rhs) lhs ## rhs
#define hsMacroJoin(lhs, rhs)   _hsMacroJoin_(lhs, rhs)

// Macro to string-ize its arguments
#define hsStringize(exp) #exp

// Declare a file-unique identifier without caring what its full name is
#define hsUniqueIdentifier(prefix) hsMacroJoin(prefix, __LINE__)

// Why can't this just be part of the standard already?
using hsSsize_t = std::make_signed<size_t>::type;

/****************************************************************************
*
*   IS_POW2
*
***/
#define IS_POW2(val) (!((val) & ((val) - 1)))

// Use "correct" non-standard string functions based on the
// selected compiler / library
#if HS_BUILD_FOR_WIN32
#   define stricmp     _stricmp
#   define strnicmp    _strnicmp
#else
#   define stricmp     strcasecmp
#   define strnicmp    strncasecmp
#endif

// flag testing / clearing
#define hsCheckBits(f,c) ((f & c)==c)
#define hsTestBits(f,b)  ( (f) & (b)   )
#define hsSetBits(f,b)   ( (f) |= (b)  )
#define hsClearBits(f,b) ( (f) &= ~(b) )
#define hsToggleBits(f,b) ( (f) ^= (b) )
#define hsChangeBits(f,b,t) ( t ? hsSetBits(f,b) : hsClearBits(f,b) )


#if HS_BUILD_FOR_WIN32
    // This is for Windows
#   ifndef fileno
#       define fileno(__F) _fileno(__F)
#   endif
#endif

/************************ Debug/Error Macros **************************/

// Implemented in hsDebug.cpp.
#ifndef HS_DEBUGGING
[[noreturn]]
#endif
void hsDebugAssertionFailed(int line, const char* file, const char* msg);

#ifdef HS_DEBUGGING
    
    #define hsAssert(expr, message)             (void)( (!!(expr)) || (hsDebugAssertionFailed(__LINE__, __FILE__, (message)), 0) )
    #define ASSERT(expr)                        (void)( (!!(expr)) || (hsDebugAssertionFailed(__LINE__, __FILE__, #expr), 0) )
    #define FATAL(message)                      hsDebugAssertionFailed(__LINE__, __FILE__, (message))
    
#else   /* Not debugging */

    #define hsAssert(expr, message)             ((void)0)
    #define ASSERT(expr)                        ((void)0)
    #define FATAL(message)                      ((void)0)

#endif  // HS_DEBUGGING

#define DEFAULT_FATAL(var) default: FATAL("No valid case for switch variable '" #var "'"); break

// Can be redirected using hsSetStatusMessageProc (from hsDebug.h).
#ifdef PLASMA_EXTERNAL_RELEASE
#   define hsStatusMessage(x) ((void)0)
#else
    void hsStatusMessage(const ST::string& message);
#endif // PLASMA_EXTERNAL_RELEASE

// Defined as a macro instead of a template function to avoid globally including <string_theory/format>.
#define hsStatusMessageF(message, ...) (hsStatusMessage(ST::format((message), __VA_ARGS__)))

#if defined(__clang__) || defined(__GNUC__)
#   define _COMPILER_WARNING_NAME(warning) "-W" warning

    /* Condition is either 1 (true) or 0 (false, do nothing). */
#   define IGNORE_WARNINGS_BEGIN_IMPL_1(warning) \
        _Pragma(hsStringize(GCC diagnostic ignored warning))
#   define IGNORE_WARNINGS_BEGIN_IMPL_0(warning)

#   define IGNORE_WARNINGS_BEGIN_COND(cond, warning) \
        _Pragma("GCC diagnostic push") \
        hsMacroJoin(IGNORE_WARNINGS_BEGIN_IMPL_, cond)(warning)

#   if defined(__has_warning)
#       define IGNORE_WARNINGS_BEGIN_IMPL(warning) \
            IGNORE_WARNINGS_BEGIN_COND(__has_warning(warning), warning)
#       define IGNORE_WARNINGS_END \
            _Pragma("GCC diagnostic pop")
#   else
        /* Suppress -Wpragmas to dodge warnings about attempts to suppress unrecognized warnings. */
#       define IGNORE_WARNINGS_BEGIN_IMPL(warning) \
            IGNORE_WARNINGS_BEGIN_COND(1, "-Wpragmas") \
            IGNORE_WARNINGS_BEGIN_COND(1, warning)
#       define IGNORE_WARNINGS_END \
            _Pragma("GCC diagnostic pop") \
            _Pragma("GCC diagnostic pop")
#   endif

#   define IGNORE_WARNINGS_BEGIN(warning) IGNORE_WARNINGS_BEGIN_IMPL(_COMPILER_WARNING_NAME(warning))
#else
#   define IGNORE_WARNINGS_BEGIN(warning)
#   define IGNORE_WARNINGS_END
#endif

#endif
