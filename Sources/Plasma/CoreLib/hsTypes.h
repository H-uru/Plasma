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

#ifdef _HSTYPES_H
#   error "Do not include hsTypes.h directly--use HeadSpin.h"
#endif // _HSTYPES_H
#define   _HSTYPES_H


/************************** Other Includes *****************************/
#include <cstdlib>
#include <cstdio>
#include <cstddef>
#include <math.h>


/************************** Basic Macros *****************************/

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

/************************** Basic Types *****************************/

#if defined(_MSC_VER) && _MSC_VER < 1600
  typedef signed char           int8_t;
  typedef unsigned char         uint8_t;
  typedef signed short int      int16_t;
  typedef unsigned short int    uint16_t;
  typedef signed int            int32_t;
  typedef unsigned int          uint32_t;
  typedef signed long long      int64_t;
  typedef unsigned long long    uint64_t;
#else
  #include <stdint.h>
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
#define nil (0)
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


/************************** Swap Macros *****************************/

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

/************************** Color32 Type *****************************/

struct hsColor32 {

    uint8_t   b, g, r, a;

    void        SetARGB(uint8_t aa, uint8_t rr, uint8_t gg, uint8_t bb)
            {
                this->a = aa; this->r = rr; this->g = gg; this->b = bb;
            }

    //  Compatibility inlines, should be depricated
    void        Set(uint8_t rr, uint8_t gg, uint8_t bb)
            {
                this->r = rr; this->g = gg; this->b = bb;
            }
    void        Set(uint8_t aa, uint8_t rr, uint8_t gg, uint8_t bb)
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


#ifdef _MSC_VER
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

    #define hsStatusMessage(x)                  NULL_STMT
    #define hsStatusMessageF(x, ...)            NULL_STMT

#else   /* Not external release */

    void    hsStatusMessage(const char message[]);
    void    hsStatusMessageF(const char * fmt, ...);

#endif // PLASMA_EXTERNAL_RELEASE

