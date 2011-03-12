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
#ifndef hsFixedTypesDefined
#define hsFixedTypesDefined

#include "hsTypes.h"

#if HS_BUILD_FOR_MAC
	#include <ToolUtils.h>
	#include <FixMath.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define hsIntToFixed(x)		((hsFixed)(x) << 16)
#define hsFixedToInt(x)		((x) >> 16)
#define hsFixedRound(x)		(((x) + 0x8000) >> 16)
#define hsFixed1			hsIntToFixed(1)
#define hsFixedPI			(0x3243F)
#define hsFixedPiOver2		(0x1921F)

#define hsFixedToFract(x)	((hsFract)(x) << 14)
#define hsFractToFixed(x)	((hsFixed)(x) >> 14)
#define hsFract1			hsFixedToFract(hsFixed1)
#define hsFractPiOver2		(0x6487ED34)	/* needs some work */

#define hsFixFloor(x)	\
	(hsFixed)((x) < 0 ? -(hsFixed)((-(x) + 0xFFFF) & 0xFFFF0000) : (x) & 0xFFFF0000)

#define hsFixedToFloorInt(x)	\
	(int)((x) < 0 ? -(int)((-(x) + 0xFFFF) >> 16) : ((x) >> 16))

#define hsFixCeiling(x)	\
	(hsFixed)((x) < 0 ? -(hsFixed)(-(x) & 0xFFFF0000) : ((x) + 0xFFFF) & 0xFFFF0000)

#define hsFixedToCeilingInt(x)	\
	(int)((x) < 0 ? -(int)(-(x) >> 16) : (((x) + 0xFFFF) >> 16))


#if HS_CAN_USE_FLOAT
	#define hsFixedToFloat(x)		((x) / float(hsFixed1))
	#define hsFloatToFixed(x)		hsFixed((x) * hsFixed1)
	
	#define hsFractToFloat(x)		((x) / float(hsFract1))
	#define hsFloatToFract(x)		hsFract((x) * hsFract1)
#endif

#if HS_BUILD_FOR_MAC68K && !(HS_PIN_MATH_OVERFLOW)
	#define hsFixMul(a, b)	FixMul(a, b)
#else
	hsFixed hsFixMul(hsFixed a, hsFixed b);
#endif

#if HS_BUILD_FOR_MAC && !(HS_PIN_MATH_OVERFLOW) && !(HS_MP_SAFE)
	#define hsFixDiv(a, b)	FixDiv(a, b)
	#define hsFracMul(a, b)	FracMul(a, b)
	#define hsFracDiv(a, b)	FracDiv(a, b)
#else
	hsFract hsFixDiv(hsFixed a, hsFixed b);
	hsFract hsFracMul(hsFract a, hsFract b);
	hsFract hsFracDiv(hsFract a, hsFract b);
#endif

hsFract	hsFracSqrt(hsFract value);
#define	hsFixSqrt(value)	(hsFracSqrt(value) >> 7)
hsFract	hsFracCubeRoot(hsFract value);
hsFixed	hsFixedSin(hsFixed s);
hsFixed	hsFixedCos(hsFixed s);
hsFixed	hsFixedASin(hsFixed s);
hsFixed	hsFixedACos(hsFixed s);

UInt16	hsSqrt32(UInt32 value);
UInt16	hsCubeRoot32(UInt32 value);
Int32	hsMulDiv32(Int32 numer1, Int32 numer2, Int32 denom);
Int32	hsMagnitude32(Int32 x, Int32 y);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
	struct hsFixedPlane {
		hsFixed	fA, fB, fC;

		void		Set(hsFixed a, hsFixed b, hsFixed c) { fA = a; fB = b; fC = c; }

		hsFixed	FixEval(hsFixed x, hsFixed y) const { return hsFixMul(fA, x) + hsFixMul(fB, y) + fC; }
		Int32	IntEval(Int32 x, Int32 y) const { return fA * x + fB * y + fC; }
		void 		ShiftDown(UInt32 i) { fA >>= i; fB >>= i; fC >>= i;}
	};
#endif

#endif
