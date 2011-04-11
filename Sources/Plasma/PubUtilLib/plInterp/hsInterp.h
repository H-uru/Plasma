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
#ifndef HSINTERP_inc
#define HSINTERP_inc

#include "HeadSpin.h"
#include "hsKeys.h"

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

//
// Performs interpolation of keyframes & values.
// t param should be 0-1
//
struct hsColorRGBA;
class hsAffineParts;
class hsInterp
{
public:
	enum IgnoreFlags
	{
		kIgnoreAlpha		= 0x1,
		kIgnoreLastMatRow	= 0x2,
		kIgnorePartsPos		= 0x4,
		kIgnorePartsRot		= 0x8,
		kIgnorePartsScale	= 0x10,		// result gets no scale
		kIgnorePartsDet		= 0x20,
		kPreservePartsScale = 0x40		// result gets the scale of key1
	};

	static void BezScalarEval(const hsScalar value1, const hsScalar outTan,
							  const hsScalar value2, const hsScalar inTan,
							  const hsScalar t, const hsScalar scale, hsScalar *result);
	static void BezInterp(const hsBezPoint3Key *k1, const hsBezPoint3Key *k2, const hsScalar t, hsScalarTriple *result);
	static void BezInterp(const hsBezScalarKey *k1, const hsBezScalarKey *k2, const hsScalar t, hsScalar *result);
	static void BezInterp(const hsBezScaleKey *k1, const hsBezScaleKey *k2, const hsScalar t, hsScaleValue *result);

	// simple linear interpolation
	static void LinInterp(const hsScalar k1, const hsScalar k2, const hsScalar t, hsScalar *result);
	static void LinInterp(const hsScalarTriple *k1, const hsScalarTriple *k2, const hsScalar t, hsScalarTriple *result);
	static void LinInterp(const hsColorRGBA *k1, const hsColorRGBA *k2, const hsScalar t, hsColorRGBA *result, UInt32 ignoreFlags=0);
	static void LinInterp(const hsMatrix33 *k1, const hsMatrix33 *k2, const hsScalar t, hsMatrix33 *result, UInt32 ignoreFlags=0);
	static void LinInterp(const hsMatrix44 *mat1, const hsMatrix44 *mat2, const hsScalar t, hsMatrix44 *out, UInt32 ignoreFlags=0);
	static void LinInterp(const hsQuat *k1, const hsQuat *k2, const hsScalar t, hsQuat *result);
	static void LinInterp(const hsScaleValue *k1, const hsScaleValue *k2, const hsScalar t, hsScaleValue *result);
	static void LinInterp(const hsAffineParts *k1, const hsAffineParts *k2, const hsScalar t, hsAffineParts *result, UInt32 ignoreFlags=0);

	// Given a time value, find the enclosing keyframes and normalize time (0-1)
	static void GetBoundaryKeyFrames(hsScalar time, UInt32 numKeys, void *keys, 
		UInt32 keySize, hsKeyFrame **kF1, hsKeyFrame **kF2, UInt32 *lastKeyIdx, hsScalar *p, hsBool forwards);

};

#define MAX_FRAMES_PER_SEC 30.0f
#define MAX_TICKS_PER_FRAME 160.0f
#define MAX_TICKS_PER_SEC (MAX_TICKS_PER_FRAME*MAX_FRAMES_PER_SEC)

#endif	// #ifndef HSINTERP_inc
