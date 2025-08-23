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

// Based on std::numbers from C++20
namespace hsConstants
{
    template <typename T>
    inline constexpr T pi =
        std::enable_if_t<std::is_floating_point_v<T>, T>(3.141592653589793238462643383279502884L);

    template <typename T>
    inline constexpr T half_pi = pi<T> / T(2.0);

    template <typename T>
    inline constexpr T two_pi = pi<T> * T(2.0);

    template <typename T>
    inline constexpr T sqrt2 =
        std::enable_if_t<std::is_floating_point_v<T>, T>(1.414213562373095048801688724209698079L);

    template <typename T>
    inline constexpr T inv_sqrt2 = T(1.0) / hsConstants::sqrt2<T>;
}

// Indirection required for joining preprocessor macros together
#define _hsMacroJoin_(lhs, rhs) lhs ## rhs
#define hsMacroJoin(lhs, rhs)   _hsMacroJoin_(lhs, rhs)

// Macro to string-ize its arguments
#define hsStringize(exp) #exp

// Declare a file-unique identifier without caring what its full name is
#define hsUniqueIdentifier(prefix) hsMacroJoin(prefix, __LINE__)

// Why can't this just be part of the standard already?
using hsSsize_t = std::make_signed<size_t>::type;

//======================================
// Endian swap funcitions
//======================================
#ifdef _MSC_VER
    #define hsSwapEndian16(val) _byteswap_ushort(val)
    #define hsSwapEndian32(val) _byteswap_ulong(val)
    #define hsSwapEndian64(val) _byteswap_uint64(val)
#elif defined(__llvm__) || (defined(__GNUC__) && ((__GNUC__ * 100) + __GNUC_MINOR__) >= 408)
    #define hsSwapEndian16(val) __builtin_bswap16(val)
    #define hsSwapEndian32(val) __builtin_bswap32(val)
    #define hsSwapEndian64(val) __builtin_bswap64(val)
#else
inline uint16_t hsSwapEndian16(uint16_t value)
{
    return (value >> 8) | (value << 8);
}
inline uint32_t hsSwapEndian32(uint32_t value)
{
    return ((value)              << 24) |
           ((value & 0x0000ff00) << 8)  |
           ((value & 0x00ff0000) >> 8)  |
           ((value)              >> 24);
}
inline uint64_t hsSwapEndian64(uint64_t value)
{
    return ((value)                      << 56) |
           ((value & 0x000000000000ff00) << 40) |
           ((value & 0x0000000000ff0000) << 24) |
           ((value & 0x00000000ff000000) << 8)  |
           ((value & 0x000000ff00000000) >> 8)  |
           ((value & 0x0000ff0000000000) >> 24) |
           ((value & 0x00ff000000000000) >> 40) |
           ((value)                      >> 56);
}
#endif

inline float hsSwapEndianFloat(float fvalue)
{
    union {
        uint32_t i;
        float    f;
    } value;

    value.f = fvalue;
    value.i = hsSwapEndian32(value.i);
    return value.f;
}
inline double hsSwapEndianDouble(double dvalue)
{
    union {
        uint64_t i;
        double   f;
    } value;

    value.f = dvalue;
    value.i = hsSwapEndian64(value.i);
    return value.f;
}

#ifdef HS_BIG_ENDIAN
    #define hsToBE16(n)         (n)
    #define hsToBE32(n)         (n)
    #define hsToBE64(n)         (n)
    #define hsToBEFloat(n)      (n)
    #define hsToBEDouble(n)     (n)
    #define hsToLE16(n)         hsSwapEndian16(n)
    #define hsToLE32(n)         hsSwapEndian32(n)
    #define hsToLE64(n)         hsSwapEndian64(n)
    #define hsToLEFloat(n)      hsSwapEndianFloat(n)
    #define hsToLEDouble(n)     hsSwapEndianDouble(n)
#else
    #define hsToBE16(n)         hsSwapEndian16(n)
    #define hsToBE32(n)         hsSwapEndian32(n)
    #define hsToBE64(n)         hsSwapEndian64(n)
    #define hsToBEFloat(n)      hsSwapEndianFloat(n)
    #define hsToBEDouble(n)     hsSwapEndianDouble(n)
    #define hsToLE16(n)         (n)
    #define hsToLE32(n)         (n)
    #define hsToLE64(n)         (n)
    #define hsToLEFloat(n)      (n)
    #define hsToLEDouble(n)     (n)
#endif

// Generic versions for use in templates
template <typename T> inline T hsToLE(T value) = delete;
template <> inline char hsToLE(char value) { return value; }
template <> inline signed char hsToLE(signed char value) { return value; }
template <> inline unsigned char hsToLE(unsigned char value) { return value; }
template <> inline int16_t hsToLE(int16_t value) { return (int16_t)hsToLE16((uint16_t)value); }
template <> inline uint16_t hsToLE(uint16_t value) { return hsToLE16(value); }
template <> inline int32_t hsToLE(int32_t value) { return (int32_t)hsToLE32((uint32_t)value); }
template <> inline uint32_t hsToLE(uint32_t value) { return hsToLE32(value); }
template <> inline int64_t hsToLE(int64_t value) { return (int64_t)hsToLE64((uint64_t)value); }
template <> inline uint64_t hsToLE(uint64_t value) { return hsToLE64(value); }
template <> inline float hsToLE(float value) { return hsToLEFloat(value); }
template <> inline double hsToLE(double value) { return hsToLEDouble(value); }


/****************************************************************************
*
*   AUTO_INIT_FUNC
*   Declares a function that is automatically called at program startup time
*
*   Example:
*
*   AUTO_INIT_FUNC(BuildLookupTables) {
*       ...
*   }
*
***/

#define AUTO_INIT_FUNC(name)  namespace { struct name { name (); } name##_instance; } name::name ()


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

// Useful floating point utilities
constexpr float hsDegreesToRadians(float deg) { return deg * (hsConstants::pi<float> / 180.f); }
constexpr float hsRadiansToDegrees(float rad) { return rad * (180.f / hsConstants::pi<float>); }
constexpr float hsInvert(float a) { return 1.f / a; }

/************************ Debug/Error Macros **************************/

void hsDebugEnableGuiAsserts(bool enabled);

#ifndef HS_DEBUGGING
[[noreturn]]
#endif
void hsDebugAssertionFailed(int line, const char* file, const char* msg);

bool hsDebugIsDebuggerPresent();
void hsDebugBreakIfDebuggerPresent();
void hsDebugBreakAlways();

/**
 * Print a message to stderr (and to the Windows debugger output, if on Windows with a debugger attached).
 * This function's output is never redirected to a log file (unlike hsStatusMessage).
 *
 * Be aware that this function's output is impossible to see for the average player/tester.
 * Prefer using other logging functions instead.
 * Please use hsDebugPrintToTerminal ONLY for debugging messages aimed at developers
 * that must not go to a log file for some reason.
 *
 * @param msg message to print
 */
void hsDebugPrintToTerminal(const char* msg);

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

typedef void (*hsStatusMessageProc)(const char message[]);

extern hsStatusMessageProc gHSStatusProc;
hsStatusMessageProc hsSetStatusMessageProc(hsStatusMessageProc newProc);

#ifdef PLASMA_EXTERNAL_RELEASE
#   define hsStatusMessage(x) ((void)0)
#   define hsStatusMessageF(x, ...) ((void)0)
#else
    void hsStatusMessage(const char* message);
    void hsStatusMessageF(const char* fmt, ...);
#endif // PLASMA_EXTERNAL_RELEASE

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
