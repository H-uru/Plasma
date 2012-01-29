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

#ifdef _HSUTILS_H
#   error "Do not include hsUtils.h directly--use HeadSpin.h"
#endif // _HSUTILS_H
#define   _HSUTILS_H

#include "HeadSpin.h"
#include <cctype>
#include <cstring>
#include <stdarg.h>

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
int hsMessageBoxWithOwner(void* owner, const char message[], const char caption[], int kind, int icon=hsMessageBoxIconAsterisk);
int hsMessageBoxWithOwner(void* owner, const wchar_t message[], const wchar_t caption[], int kind, int icon=hsMessageBoxIconAsterisk);

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

#   define MAX_PATH 1024
#endif

// Useful floating point utilities
inline float hsDegreesToRadians(float deg) { return float(deg * (M_PI / 180)); }
inline float hsRadiansToDegrees(float rad) { return float(rad * (180 / M_PI)); }
#define hsInvert(a) (1 / (a))

#include <new>
#define NEWZERO(t)              new(calloc(sizeof(t), 1)) t

/////////////////////////////
// Physical memory functions
/////////////////////////////
enum MemSpec
{
    kBlows = 0,     // Less than 128
    kAcceptable,    // Less than 256
    kOptimal        // 256 or greater
};

uint32_t hsPhysicalMemory();
MemSpec hsMemorySpec();

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
void ErrorMinimizeAppWindow ();

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
