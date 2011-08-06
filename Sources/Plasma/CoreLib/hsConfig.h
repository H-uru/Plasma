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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#include "HeadSpin.h"

#ifndef hsConfigDefined
#define hsConfigDefined


#ifndef SERVER
# define CLIENT
#endif


//////////////////// Change the 1s and 0s //////////////////////

#define HS_CAN_USE_FLOAT            1
#define HS_SCALAR_IS_FLOAT          1

#define HS_PIN_MATH_OVERFLOW        0       // This forces hsWide versions of FixMath routines
#define HS_DEBUG_MATH_OVERFLOW      0       // This calls hsDebugMessage on k[Pos,Neg]Infinity



////////////////////  Specific Compiler Stuff This Section is computed  ////////////

#if defined(macintosh) && defined(__POWERPC__)
    #define HS_BUILD_FOR_MACPPC         1
#elif defined(macintosh)
    #define HS_BUILD_FOR_MAC68K         1
#elif defined(_WIN32)
    #define HS_BUILD_FOR_WIN32          1
#elif defined(__unix__)
    #define HS_BUILD_FOR_UNIX           1
#endif

#define HS_SCALAR_IS_FIXED          !(HS_SCALAR_IS_FLOAT)
#define HS_NEVER_USE_FLOAT          !(HS_CAN_USE_FLOAT)

#if HS_DEBUG_MATH_OVERFLOW && !(HS_PIN_MATH_OVERFLOW)
    #error "Can't debug overflow unless HS_PIN_MATH_OVERFLOW is ON"
#endif


///////////////////////Windows Specific Defines /////////////////////////////

#if HS_BUILD_FOR_WIN32

// 4244: Conversion
// 4305: Truncation
// 4503: 'identifier' : decorated name length exceeded, name was truncated
// 4018: signed/unsigned mismatch
// 4786: 255 character debug limit
// 4284: STL template defined operator-> for a class it doesn't make sense for (int, etc)
#if !__MWERKS__
#pragma warning( disable : 4305 4503 4018 4786 4284)
#endif

// VC++ version greater than 6.0, must be building for .NET
#if defined(_MSC_VER) && (_MSC_VER > 1200)
#define HS_BUILD_FOR_WIN32_NET
#endif

#pragma optimize( "y", off )

#endif


/////////////////////Debugging Defines ///////////////////////////////////

#if (defined(_DEBUG)||defined(UNIX_DEBUG)) && !defined(HS_DISABLE_ASSERT)
#define HS_DEBUGGING
#if (!defined(HS_NO_MEM_TRACKER))
#define HS_FIND_MEM_LEAKS
#endif
#endif


///////////////////// Required facilities ///////////////////////////////
#ifndef HeadSpinHDefined
#include "HeadSpin.h"
#endif

#endif // hsConfigDefined
