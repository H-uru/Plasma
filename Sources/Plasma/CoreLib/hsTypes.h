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

#ifndef hsTypes_Defined
#define hsTypes_Defined


/************************** Other Includes *****************************/
#include <cstdlib>
#include <cstdio>
        
#if HS_CAN_USE_FLOAT
    #include <math.h>
#endif


/************************** Basic Macros *****************************/

#ifdef __cplusplus
    #define hsCTypeDefStruct(foo)
#else
    #define hsCTypeDefStruct(foo)       typedef struct foo foo;
#endif

/************************** Basic Types *****************************/

#ifdef _MSC_VER
  typedef signed __int8     int8_t;
  typedef unsigned __int8   uint8_t;
  typedef signed __int16    int16_t;
  typedef unsigned __int16  uint16_t;
  typedef signed __int32    int32_t;
  typedef unsigned __int32  uint32_t;
  typedef signed __int64    int64_t;
  typedef unsigned __int64  uint64_t;
#else
  #include <stdint.h>
#endif

typedef uint8_t       byte;
typedef uint16_t      word;
typedef uint32_t      dword;
typedef uint64_t      qword;

typedef uintptr_t           unsigned_ptr;

typedef wchar_t             wchar;

#define kPosInfinity16      (32767)
#define kNegInfinity16      (-32768)

#define kPosInfinity32      (0x7fffffff)
#define kNegInfinity32      (0x80000000)

typedef int8_t     Int8;
typedef int16_t    Int16;
typedef int32_t    Int32;
typedef int64_t    Int64;

typedef uint8_t   UInt8;
typedef uint16_t  UInt16;
typedef uint32_t  UInt32;
typedef uint64_t  UInt64;

#ifndef Byte
    typedef uint8_t Byte;
#endif

#ifndef false
    #define false       0
#endif
#ifndef true
    #define true        1
#endif
#ifndef Boolean
    typedef uint8_t     Boolean;
#endif


typedef Int32           hsFixed;
typedef Int32           hsFract;

#ifdef __cplusplus

typedef int     hsBool;

#endif

#include "hsScalar.h"

#if HS_CAN_USE_FLOAT
    #define HS_PI       3.1415927
#endif

#ifndef nil
#define nil (0)
#endif

typedef Int32   hsError;
typedef UInt32  hsGSeedValue;

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

#define hsFourByteTag(a, b, c, d)       (((UInt32)(a) << 24) | ((UInt32)(b) << 16) | ((UInt32)(c) << 8) | (d))


/************************** Swap Macros *****************************/

#ifdef __cplusplus
    inline UInt16 hsSwapEndian16(UInt16 value)
    {
        return (value >> 8) | (value << 8);
    }
    inline UInt32 hsSwapEndian32(UInt32 value)
    {
        return ((value)              << 24) | 
               ((value & 0x0000ff00) << 8)  |
               ((value & 0x00ff0000) >> 8)  |
               ((value)              >> 24);
    }
    inline UInt64 hsSwapEndian64(UInt64 value)
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
    #if HS_CAN_USE_FLOAT
    inline float hsSwapEndianFloat(float fvalue)
    {
        UInt32 value = *(UInt32*)&fvalue;
        value = hsSwapEndian32(value);
        return *(float*)&value;
    }
    inline double hsSwapEndianDouble(double dvalue)
    {
        UInt64 value = *(UInt64*)&dvalue;
        value = hsSwapEndian64(value);
        return *(double*)&value;
    }
    #endif

    #if LITTLE_ENDIAN
        #define hsUNSWAP16(n)       hsSwapEndian16(n)
        #define hsUNSWAP32(n)       hsSwapEndian32(n)
        #define hsUNSWAP64(n)       hsSwapEndian64(n)
        #define hsUNSWAPFloat(n)    hsSwapEndianFloat(n)
        #define hsUNSWAPDouble(n)   hsSwapEndianDouble(n)
        #define hsSWAP16(n)         (n)
        #define hsSWAP32(n)         (n)
        #define hsSWAP64(n)         (n)
        #define hsSWAPFloat(n)      (n)
        #define hsSWAPDouble(n)     (n)
    #else
        #define hsUNSWAP16(n)       (n)
        #define hsUNSWAP32(n)       (n)
        #define hsUNSWAP64(n)       (n)
        #define hsUNSWAPFloat(n)    (n)
        #define hsUNSWAPDouble(n)   (n)
        #define hsSWAP16(n)         hsSwapEndian16(n)
        #define hsSWAP32(n)         hsSwapEndian32(n)
        #define hsSWAP64(n)         hsSwapEndian64(n)
        #define hsSWAPFloat(n)      hsSwapEndianFloat(n)
        #define hsSWAPDouble(n)     hsSwapEndianDouble(n)
    #endif

    inline void hsSwap(Int32& a, Int32& b)
    {
        Int32   c = a;
        a = b;
        b = c;
    }

    inline void hsSwap(UInt32& a, UInt32& b)
    {
        UInt32  c = a;
        a = b;
        b = c;
    }

    #if HS_CAN_USE_FLOAT
        inline void hsSwap(float& a, float& b)
        {
            float   c = a;
            a = b;
            b = c;
        }
    #endif
#endif

/************************** Color32 Type *****************************/

struct hsColor32 {

    UInt8   b, g, r, a;

#ifdef __cplusplus
    void        SetARGB(UInt8 aa, UInt8 rr, UInt8 gg, UInt8 bb)
            {
                this->a = aa; this->r = rr; this->g = gg; this->b = bb;
            }

    //  Compatibility inlines, should be depricated
    void        Set(UInt8 rr, UInt8 gg, UInt8 bb)
            {
                this->r = rr; this->g = gg; this->b = bb;
            }
    void        Set(UInt8 aa, UInt8 rr, UInt8 gg, UInt8 bb)
            {
                this->SetARGB(aa, rr, gg, bb);
            }

#if 0
    friend int  operator==(const hsColor32& a, const hsColor32& b)
            {
                return *(UInt32*)&a == *(UInt32*)&b;
            }
    friend int  operator!=(const hsColor32& a, const hsColor32& b) { return !(a == b); }
#else
    int operator==(const hsColor32& aa) const
    {
            return *(UInt32*)&aa == *(UInt32*)this;
    }
    int operator!=(const hsColor32& aa) { return !(aa == *this); }
#endif
#endif
};
hsCTypeDefStruct(hsColor32)

typedef hsColor32 hsRGBAColor32;


/****************************************************************************
*
*   NULL_STMT
*   Declares a null statement
*
*   Example:
*
*   for (; *str && (*str != ch); ++str)
*       NULL_STMT;
*
***/

#if _MSC_VER >= 7
# define  NULL_STMT  __noop
#else
# define  NULL_STMT  ((void)0)
#endif


/****************************************************************************
*
*   UNIQUE_SYMBOL
*   Creates a symbol that is unique within a file
*
***/

#define  UNIQUE_SYMBOL_EXPAND_1(prefix,line)  UNIQUE_SYMBOL_##prefix##_##line
#define  UNIQUE_SYMBOL_EXPAND_0(prefix,line)  UNIQUE_SYMBOL_EXPAND_1(prefix,line)
#define  UNIQUE_SYMBOL(prefix)                UNIQUE_SYMBOL_EXPAND_0(prefix,__LINE__)



/****************************************************************************
*
*   COMPILER_ASSERT
*   Performs a "compile-time" assertion
*   Can be used at function or file scope
*   Upon assertion failure, creates a negative subscript error
*   Use COMPILER_ASSERT_HEADER in header files to prevent symbol collision
*
***/

#define COMPILER_ASSERT(expr) static char UNIQUE_SYMBOL(a)[(expr) ? 1 : -1]
#define COMPILER_ASSERT_HEADER(prefix,expr) static char UNIQUE_SYMBOL(prefix)[(expr) ? 1 : -1]
#define COMPILER_ASSERT_SYMBOL(symbol,expr) static char symbol[(expr) ? 1 : -1]


/****************************************************************************
*
*   max/min inline functions
*
***/

#ifdef max
#undef max
#endif

#ifdef min
#undef min
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
*   Calculate the address to the base of a structure given its type, and the
*   address of a field within the structure.
*
*   Example:
*
*   CONTAINING_STRUCT(trans, INetTrans, m_trans);
*
***/

#define CONTAINING_STRUCT(a, t, f)  ((t *) ((byte *)(a) - (unsigned_ptr)(&((t *)0)->f)))


/****************************************************************************
*
*   arrsize/marrsize
*   arrsize returns the number of elements in an array variable
*   marrsize returns the number of elements in an array field in a structure
*
*   Example:
*
*   StrPrintf(buffer, arrsize(buffer), "%u", value);
*
***/

#define  arrsize(a)     (sizeof(a) / sizeof((a)[0]))
#define  marrsize(c,a)  (arrsize(((c *)0)->a))


/****************************************************************************
*
*   offsetof/moffsetof
*   offsetof returns the offset in bytes of a field inside a structure based on a type
*   moffsetof returns the offset in bytes of a field inside a structure based on a variable
*
***/

#include <stddef.h>

#ifndef  offsetof
#define  offsetof(s,m)  (size_t)&(((s *)0)->m)
#endif   // ifndef offsetof

#define  moffsetof(v,f)  (((byte *) &((v)->f)) - ((byte *) v))


/****************************************************************************
*
*   msizeof
*   Returns the size of a field in a structure
*
*   Example:
*
*   unsigned bufferSize = msizeof(CommandStruct, buffer);
*
***/

#define  msizeof(c,a)  (sizeof(((c *)0)->a))


/****************************************************************************
*
*   ONCE
*   Shortcut to create a 'for' loop that executes only once
*
*   for (ONCE) {
*       ...
*   }
*
***/

#ifndef  ONCE
#define  ONCE  bool UNIQUE_SYMBOL(ONCE) = true; UNIQUE_SYMBOL(ONCE); UNIQUE_SYMBOL(ONCE) = false
#endif


/****************************************************************************
*
*   IS_POW2
*
***/

#define IS_POW2(val) (!((val) & ((val) - 1)))


/************************ Debug/Error Macros **************************/

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*hsDebugMessageProc)(const char message[]);
extern hsDebugMessageProc gHSDebugProc;
#define HSDebugProc(m)  { if (gHSDebugProc) gHSDebugProc(m); }
hsDebugMessageProc hsSetDebugMessageProc(hsDebugMessageProc newProc);

extern hsDebugMessageProc gHSStatusProc;
hsDebugMessageProc hsSetStatusMessageProc(hsDebugMessageProc newProc);

void CDECL ErrorAssert (int line, const char file[], const char fmt[], ...);
void CDECL ErrorFatal (int line, const char file[], const char fmt[], ...);
void ErrorMinimizeAppWindow ();

enum EErrorOption {
    kErrOptNonGuiAsserts,
    kErrOptDisableMemLeakChecking,
    kNumErrorOptions
};
bool ErrorSetOption (EErrorOption option, bool on);
bool ErrorGetOption (EErrorOption option);

bool DebugIsDebuggerPresent ();
void DebugBreakIfDebuggerPresent ();
void DebugMsg (const char fmt[], ...);
void DebugMsgV (const char fmt[], va_list args);


#ifdef HS_DEBUGGING
    
    void    hsDebugMessage(const char message[], long refcon);
    #define hsDebugCode(code)                   code
    #define hsIfDebugMessage(expr, msg, ref)    (void)( ((expr) != 0) || (hsDebugMessage(msg, ref), 0) )
    #define hsAssert(expr, msg)                 (void)( ((expr) != 0) || (ErrorAssert(__LINE__, __FILE__, msg), 0) )
    #define ASSERT(expr)                        (void)( ((expr) != 0) || (ErrorAssert(__LINE__, __FILE__, #expr), 0) )
    #define ASSERTMSG(expr, msg)                (void)( ((expr) != 0) || (ErrorAssert(__LINE__, __FILE__, msg), 0) )
    #define FATAL(msg)                          ErrorAssert(__LINE__, __FILE__, msg)
    #define DEBUG_MSG                           DebugMsg
    #define DEBUG_MSGV                          DebugMsgV
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


#ifdef PLASMA_EXTERNAL_RELEASE

    #define hsStatusMessage(x)                  NULL_STMT
    #define hsStatusMessageF(x,y)               NULL_STMT

#else   /* Not external release */

    void    hsStatusMessage(const char message[]);
    void    hsStatusMessageF(const char * fmt, ...);

#endif // PLASMA_EXTERNAL_RELEASE


#ifdef __cplusplus
}
#endif

#endif
