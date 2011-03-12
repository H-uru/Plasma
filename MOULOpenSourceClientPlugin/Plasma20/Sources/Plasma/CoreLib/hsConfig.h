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


#ifdef BUILDPS2
#define HS_BUILD_FOR_PS2		1
#define PLASMA_NO_NETWORK		1 
#define PLASMA_NO_KEYBOARD		1
#define PLASMA_NO_GLIDE			1
#define PLASMA_NO_DDRAW			1
#define HS_BUILD_PLASMA			1
#define NEXUS_NO_2D				1
#endif

//////////////////// Change the 1s and 0s //////////////////////

#define HS_CAN_USE_FLOAT			1
#define HS_SCALAR_IS_FLOAT			1

#define HS_PIN_MATH_OVERFLOW		0		// This forces hsWide versions of FixMath routines
#define HS_DEBUG_MATH_OVERFLOW		0		// This calls hsDebugMessage on k[Pos,Neg]Infinity



///////////////////////Impulse Defines////////////////////////////////////////////////

#define HS_IMPULSE_SUPPORT_GRAY4 0

///////////////////////Plasma Defines //////////////////////////////////////////////////

#ifdef   HS_BUILD_PLASMA

	#define HS_IGNORE_T2K		  1
	#define HS_SUPPORT_NFNT_FONTS 1

#endif // HS_BUILD_PLASMA



////////////////////  Specific Compiler Stuff This Section is computed  ////////////

#if defined(macintosh) && defined(__POWERPC__)
	#define HS_BUILD_FOR_MACPPC			1
	#define HS_CPU_BENDIAN				1
#elif defined(macintosh)
	#define HS_BUILD_FOR_MAC68K			1
#elif defined(_M_IX86) && defined(_WIN32)
	#define HS_BUILD_FOR_WIN32			1
	#define HS_CPU_LENDIAN				1
#elif defined(__unix__)
	#define HS_BUILD_FOR_UNIX			1
	#if defined(__intel__) || defined(__i386__)
		#define HS_CPU_LENDIAN				1
	#elif defined(__mips__)
		#define HS_CPU_BENDIAN				1
	#endif
#elif !HS_BUILD_FOR_PS2
	#define HS_BUILD_FOR_REFERENCE		1
#endif

#if defined(HS_BUILD_FOR_MAC68K) || defined(HS_BUILD_FOR_MACPPC) 
	#define HS_BUILD_FOR_MAC		1
#endif

#if defined(__INTEL__) && defined(HS_BUILD_FOR_MAC)
	#error "Can't have HS_BUILD_FOR_MAC defined"
#endif
#if (defined(GENERATING68K) || defined(GENERATINGPOWERPC)) && defined(HS_BUILD_FOR_WIN32)
	#define "Can't define HS_BUILD_FOR_WIN32"
#endif

#define HS_SCALAR_IS_FIXED			!(HS_SCALAR_IS_FLOAT)
#define HS_NEVER_USE_FLOAT			!(HS_CAN_USE_FLOAT)

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


#if HS_BUILD_FOR_PS2
#define ATTRIBUTE_FOR_PS2 __attribute__((aligned (16)))		/* SUNSOFT */
#else
#define ATTRIBUTE_FOR_PS2
#endif


/////////////////////Myst3D Defines /////////////////////////////////////

#ifdef M3DRELEASE
#define PLASMA_NO_NETWORK	1
#define NEXUS_NO_2D			1
#define NO_LOAD_MSG			1
#define PLASMA_NO_CONSOLE	1
#define NEXUS_NO_DEBUG		1
#endif


///////////////////// Required facilities ///////////////////////////////
#ifndef HeadSpinHDefined
#include "HeadSpin.h"
#endif

#endif // hsConfigDefined
