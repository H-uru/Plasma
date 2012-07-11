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

#if (defined(_DEBUG) || defined(UNIX_DEBUG))
#   define HS_DEBUGGING
#endif // defined(_DEBUG) || defined(UNIX_DENUG)

//======================================
// Winblows Hacks
//======================================
#ifdef HS_BUILD_FOR_WIN32
    // 4244: Conversion
    // 4305: Truncation
    // 4503: 'identifier' : decorated name length exceeded, name was truncated
    // 4018: signed/unsigned mismatch
    // 4786: 255 character debug limit
    // 4284: STL template defined operator-> for a class it doesn't make sense for (int, etc)
    // 4800: 'int': forcing value to bool 'true' or 'false' (performance warning)
#   ifdef _MSC_VER
#      pragma warning( disable : 4305 4503 4018 4786 4284 4800)
#   endif // _MSC_VER

    // Terrible hacks for MinGW because they don't have a reasonable
    // default for the Windows version. We cheat and say it's XP.
#   ifdef __MINGW32__
#       undef _WIN32_WINNT
#       define _WIN32_WINNT 0x501
#       undef _WIN32_IE
#       define _WIN32_IE    0x400
#   endif

    // Windows.h includes winsock.h (winsocks 1), so we need to manually include winsock2 
    // and tell Windows.h to only bring in modern headers
#   include <WinSock2.h>
#   include <ws2tcpip.h>

#   define WIN32_LEAN_AND_MEAN
#   ifndef NOMINMAX
#      define NOMINMAX // Needed to prevent NxMath conflicts
#   endif
#   include <Windows.h>

    // Just some fun typedefs...
    typedef HWND hsWindowHndl;
    typedef HINSTANCE hsWindowInst;
#else
    typedef int32_t* hsWindowHndl;
    typedef int32_t* hsWindowInst;
#endif // HS_BUILD_FOR_WIN32

//======================================
// We don't want the Windows.h min/max!
//======================================
#ifdef max
#   undef max
#endif

#ifdef min
#   undef min
#endif

//======================================
// Some standard includes
//======================================
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <stdarg.h>
#include <stdint.h>


//======================================
// Basic macros
//======================================
#ifdef __cplusplus
    #define hsCTypeDefStruct(foo)
#else
    #define hsCTypeDefStruct(foo)       typedef struct foo foo;
#endif

#ifdef HS_BUILD_FOR_WIN32
#    ifndef CDECL
#        define CDECL __cdecl
#    endif
#else
#   define CDECL
#endif

#define kPosInfinity16      (32767)
#define kNegInfinity16      (-32768)

#define kPosInfinity32      (0x7fffffff)
#define kNegInfinity32      (0x80000000)

#ifndef M_PI
#   define M_PI       3.14159265358979323846
#endif

#ifdef __cplusplus
    typedef int     hsBool;
#endif

#ifndef nil
#   define nil (0)
#endif

typedef int32_t   hsError;
typedef uint32_t  hsGSeedValue;

#define hsOK                0
#define hsFail              -1
#define hsFailed(r)         ((hsError)(r)<hsOK)
#define hsSucceeded(r)      ((hsError)(r)>=hsOK)

#define hsLongAlign(n)      (((n) + 3) & ~3L)

#define hsMaximum(a, b)     ((a) > (b) ? (a) : (b))
#define hsMinimum(a, b)     ((a) < (b) ? (a) : (b))
#define hsABS(x)            ((x) < 0 ? -(x) : (x))
#define hsSGN(x)            (((x) < 0) ? -1 : ( ((x) > 0) ? 1 : 0 ))

#define hsBitTst2Bool(value, mask)      (((value) & (mask)) != 0)

#define hsFourByteTag(a, b, c, d)       (((uint32_t)(a) << 24) | ((uint32_t)(b) << 16) | ((uint32_t)(c) << 8) | (d))


//======================================
// Endian swap funcitions
//======================================
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
inline float hsSwapEndianFloat(float fvalue)
{
    uint32_t value = *(uint32_t*)&fvalue;
    value = hsSwapEndian32(value);
    return *(float*)&value;
}
inline double hsSwapEndianDouble(double dvalue)
{
    uint64_t value = *(uint64_t*)&dvalue;
    value = hsSwapEndian64(value);
    return *(double*)&value;
}

#if LITTLE_ENDIAN
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
#else
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
#endif

inline void hsSwap(int32_t& a, int32_t& b)
{
    int32_t   c = a;
    a = b;
    b = c;
}

inline void hsSwap(uint32_t& a, uint32_t& b)
{
    uint32_t  c = a;
    a = b;
    b = c;
}

inline void hsSwap(float& a, float& b)
{
    float   c = a;
    a = b;
    b = c;
}

//======================================
// Color32 Type
//======================================
struct hsColor32 {

    uint8_t   b, g, r, a;

    inline void SetARGB(uint8_t aa, uint8_t rr, uint8_t gg, uint8_t bb)
    {
        this->a = aa;
        this->r = rr;
        this->g = gg;
        this->b = bb;
    }

    //  Compatibility inlines, should be depricated
    inline void Set(uint8_t rr, uint8_t gg, uint8_t bb)
    {
        this->r = rr;
        this->g = gg;
        this->b = bb;
    }
    inline void Set(uint8_t aa, uint8_t rr, uint8_t gg, uint8_t bb)
    {
        this->SetARGB(aa, rr, gg, bb);
    }

    int operator==(const hsColor32& aa) const
    {
            return *(uint32_t*)&aa == *(uint32_t*)this;
    }
    int operator!=(const hsColor32& aa) { return !(aa == *this); }
};
hsCTypeDefStruct(hsColor32)
typedef hsColor32 hsRGBAColor32;


//===========================================================================
// Define a NOOP (null) statement
//===========================================================================
#ifdef _MSC_VER
# define  NULL_STMT  __noop
#else
# define  NULL_STMT  ((void)0)
#endif

//===========================================================================
template<class T>
inline T max (const T & a, const T & b) {
    return (a > b) ? a : b;
}

//===========================================================================
inline unsigned max (int a, unsigned b) {
    return ((unsigned)a > b) ? a : b;
}

//===========================================================================
inline unsigned max (unsigned a, int b) {
    return (a > (unsigned)b) ? a : b;
}

//===========================================================================
template<class T>
inline T min (const T & a, const T & b) {
    return (a < b) ? a : b;
}

//===========================================================================
inline unsigned min (int a, unsigned b) {
    return ((unsigned)a < b) ? a : b;
}

//===========================================================================
inline unsigned min (unsigned a, int b) {
    return (a < (unsigned)b) ? a : b;
}


/****************************************************************************
*
*   MAX/MIN macros
*   These are less safe than the inline function versions, since they
*   evaluate parameters twice. However, they can be used to produce
*   compile-time constants.
*
***/
#define  MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define  MIN(a, b)  (((a) < (b)) ? (a) : (b))


/****************************************************************************
*
*   SWAP
*   Swaps the values of two variables
*
***/

//===========================================================================
template<class T>
void SWAP (T & a, T & b) {
    T temp = a;
    a = b;
    b = temp;
}


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
*   arrsize
*   arrsize returns the number of elements in an array variable
*
*   Example:
*
*   StrPrintf(buffer, arrsize(buffer), "%u", value);
*
***/
#define  arrsize(a)     (sizeof(a) / sizeof((a)[0]))


/****************************************************************************
*
*   IS_POW2
*
***/
#define IS_POW2(val) (!((val) & ((val) - 1)))

#ifdef PLASMA_EXTERNAL_RELEASE
#   define hsStatusMessage(x)                  NULL_STMT
#   define hsStatusMessageF(x, ...)            NULL_STMT
#else
    void    hsStatusMessage(const char message[]);
    void    hsStatusMessageF(const char * fmt, ...);
#endif // PLASMA_EXTERNAL_RELEASE

int     hsStrlen(const char src[]);
char*   hsStrcpy(char dstOrNil[], const char src[]);
void    hsStrcat(char dst[], const char src[]);
bool    hsStrEQ(const char s1[], const char s2[]);
bool    hsStrCaseEQ(const char* s1, const char* s2);
char*   hsScalarToStr(float);
int     hsRemove(const char* filename);
void    hsCPathToMacPath(char* dst, char* fname);   
void    hsStrLower(char *s);
char *  hsFormatStr(const char * fmt, ...); // You are responsible for returned memory.
char *  hsFormatStrV(const char * fmt, va_list args);   // You are responsible for returned memory.

// Use "correct" stricmp based on the selected compiler / library
#if HS_BUILD_FOR_WIN32
#    define stricmp     _stricmp
#    define strnicmp    _strnicmp
#    define wcsicmp     _wcsicmp
#    define wcsnicmp    _wcsnicmp
#    define strlwr      _strlwr
#else
#    define stricmp     strcasecmp
#    define strnicmp    strncasecmp
#    define wcsicmp     wcscasecmp
#    define wcsnicmp    wcsncasecmp
#    define strlwr      hsStrLower
#endif


//  A pstring has a length uint8_t at the beginning, and no trailing 0
char*   hsP2CString(const uint8_t pstring[], char cstring[]);
uint8_t*  hsC2PString(const char cstring[], uint8_t pstring[]);

inline char* hsStrcpy(const char src[])
{
    return hsStrcpy(nil, src);
}

inline char *hsStrncpy(char *strDest, const char *strSource, size_t count)
{
    char *temp = strncpy(strDest, strSource, count-1);
    strDest[count-1] = 0;
    return temp;
}

wchar_t *hsStringToWString( const char *str );
char    *hsWStringToString( const wchar_t *str );

enum {              // Kind of MessageBox...passed to hsMessageBox
    hsMessageBoxAbortRetyIgnore,
    hsMessageBoxNormal,             // Just Ok
    hsMessageBoxOkCancel,
    hsMessageBoxRetryCancel,
    hsMessageBoxYesNo,
    hsMessageBoxYesNoCancel,
};

enum {
    hsMessageBoxIconError,
    hsMessageBoxIconQuestion,
    hsMessageBoxIconExclamation,
    hsMessageBoxIconAsterisk,
};

enum {          // RETURN VALUES FROM hsMessageBox
    hsMBoxOk = 1,       // OK button was selected. 
    hsMBoxCancel,   // Cancel button was selected. 
    hsMBoxAbort,    // Abort button was selected. 
    hsMBoxRetry,    // Retry button was selected. 
    hsMBoxIgnore,   // Ignore button was selected. 
    hsMBoxYes,      // Yes button was selected. 
    hsMBoxNo        // No button was selected. 
};

extern bool hsMessageBox_SuppressPrompts;
int hsMessageBox(const char message[], const char caption[], int kind, int icon=hsMessageBoxIconAsterisk);
int hsMessageBox(const wchar_t message[], const wchar_t caption[], int kind, int icon=hsMessageBoxIconAsterisk);
int hsMessageBoxWithOwner(hsWindowHndl owner, const char message[], const char caption[], int kind, int icon=hsMessageBoxIconAsterisk);
int hsMessageBoxWithOwner(hsWindowHndl owner, const wchar_t message[], const wchar_t caption[], int kind, int icon=hsMessageBoxIconAsterisk);

inline hsBool hsCompare(float a, float b, float delta=0.0001);

// flag testing / clearing
#define hsCheckBits(f,c) ((f & c)==c)
#define hsTestBits(f,b)  ( (f) & (b)   )
#define hsSetBits(f,b)   ( (f) |= (b)  )
#define hsClearBits(f,b) ( (f) &= ~(b) )
#define hsToggleBits(f,b) ( (f) ^= (b) )
#define hsChangeBits(f,b,t) ( t ? hsSetBits(f,b) : hsClearBits(f,b) )


#if HS_BUILD_FOR_WIN32
     // This is for Windows
#    define hsVsnprintf     _vsnprintf
#    define hsVsnwprintf    _vsnwprintf
#    define hsSnprintf      _snprintf
#    define hsSnwprintf     _snwprintf

#    define snprintf        _snprintf
#    define snwprintf       _snwprintf
#    define swprintf        _snwprintf

#    ifndef fileno
#        define fileno(__F)       _fileno(__F)
#    endif

#   define hsWFopen(name, mode)     _wfopen(name, mode)
#else
     // This is for Unix, Linux, OSX, etc.
#    define hsVsnprintf     vsnprintf
#    define hsVsnwprintf    vswprintf
#    define hsSnprintf      snprintf
#    define hsSnwprintf     swprintf

#   define hsWFopen(name, mode)     fopen(hsWStringToString(name), hsWStringToString(mode))

#   include <limits.h>
#   define MAX_PATH PATH_MAX
#endif

// Useful floating point utilities
inline float hsDegreesToRadians(float deg) { return float(deg * (M_PI / 180)); }
inline float hsRadiansToDegrees(float rad) { return float(rad * (180 / M_PI)); }
#define hsInvert(a) (1 / (a))

#include <new>
#define NEWZERO(t)              new(calloc(sizeof(t), 1)) t

#ifdef _MSC_VER
#   define ALIGN(n) __declspec(align(n))
#else
#   define ALIGN(n) __atribute__(aligned(n))
#endif

inline int hsRandMax() { return 32767; }
inline float hsRandNorm() { return 1.f / 32767.f; } // multiply by hsRand to get randoms ranged [0..1]
int hsRand(void);
void hsRandSeed(int seed);

#define hsFopen(name, mode) fopen(name, mode)

char** DisplaySystemVersion();

/************************ Debug/Error Macros **************************/

typedef void (*hsDebugMessageProc)(const char message[]);
extern hsDebugMessageProc gHSDebugProc;
#define HSDebugProc(m)  { if (gHSDebugProc) gHSDebugProc(m); }
hsDebugMessageProc hsSetDebugMessageProc(hsDebugMessageProc newProc);

extern hsDebugMessageProc gHSStatusProc;
hsDebugMessageProc hsSetStatusMessageProc(hsDebugMessageProc newProc);

void ErrorEnableGui (bool enabled);
void ErrorAssert (int line, const char file[], const char fmt[], ...);

bool DebugIsDebuggerPresent ();
void DebugBreakIfDebuggerPresent ();
void DebugMsg(const char fmt[], ...);

#ifdef HS_DEBUGGING
    
    void    hsDebugMessage(const char message[], long refcon);
    #define hsDebugCode(code)                   code
    #define hsIfDebugMessage(expr, msg, ref)    (void)( ((expr) != 0) || (hsDebugMessage(msg, ref), 0) )
    #define hsAssert(expr, msg)                 (void)( ((expr) != 0) || (ErrorAssert(__LINE__, __FILE__, msg), 0) )
    #define ASSERT(expr)                        (void)( ((expr) != 0) || (ErrorAssert(__LINE__, __FILE__, #expr), 0) )
    #define ASSERTMSG(expr, msg)                (void)( ((expr) != 0) || (ErrorAssert(__LINE__, __FILE__, msg), 0) )
    #define FATAL(msg)                          ErrorAssert(__LINE__, __FILE__, msg)
    #define DEBUG_MSG                           DebugMsg
    #define DEBUG_BREAK_IF_DEBUGGER_PRESENT     DebugBreakIfDebuggerPresent
    
#else   /* Not debugging */

    #define hsDebugMessage(message, refcon)     NULL_STMT
    #define hsDebugCode(code)                   /* empty */
    #define hsIfDebugMessage(expr, msg, ref)    NULL_STMT
    #define hsAssert(expr, msg)                 NULL_STMT
    #define ASSERT(expr)                        NULL_STMT
    #define ASSERTMSG(expr, msg)                NULL_STMT
    #define FATAL(msg)                          NULL_STMT
    #define DEBUG_MSG                           (void)
    #define DEBUG_MSGV                          NULL_STMT
    #define DEBUG_BREAK_IF_DEBUGGER_PRESENT     NULL_STMT

#endif  // HS_DEBUGGING


#ifdef _MSC_VER
#define  DEFAULT_FATAL(var)  default: FATAL("No valid case for switch variable '" #var "'"); __assume(0); break;
#else
#define  DEFAULT_FATAL(var)  default: FATAL("No valid case for switch variable '" #var "'"); break;
#endif

/*****************************************************************************
*
*  Atomic Operations
*
***/

// *value += increment; return original value of *value; thread safe
inline long AtomicAdd(long* value, long increment)
{
#ifdef HS_BUILD_FOR_WIN32
    return InterlockedExchangeAdd(value, increment);
#elif __GNUC__
    return __sync_fetch_and_add(value, increment);
#else
#   error "No Atomic Set support on this architecture"
#endif
}

// *value = value; return original value of *value; thread safe
inline long AtomicSet(long* value, long set)
{
#ifdef HS_BUILD_FOR_WIN32
    return InterlockedExchange(value, set);
#elif __GNUC__
    return  __sync_lock_test_and_set(value, set);
#else
#   error "No Atomic Set support on this architecture"
#endif
}

#endif
