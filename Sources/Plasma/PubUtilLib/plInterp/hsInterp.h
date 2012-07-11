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
        kIgnoreAlpha        = 0x1,
        kIgnoreLastMatRow   = 0x2,
        kIgnorePartsPos     = 0x4,
        kIgnorePartsRot     = 0x8,
        kIgnorePartsScale   = 0x10,     // result gets no scale
        kIgnorePartsDet     = 0x20,
        kPreservePartsScale = 0x40      // result gets the scale of key1
    };

    static void BezScalarEval(const float value1, const float outTan,
                              const float value2, const float inTan,
                              const float t, const float scale, float *result);
    static void BezInterp(const hsBezPoint3Key *k1, const hsBezPoint3Key *k2, const float t, hsScalarTriple *result);
    static void BezInterp(const hsBezScalarKey *k1, const hsBezScalarKey *k2, const float t, float *result);
    static void BezInterp(const hsBezScaleKey *k1, const hsBezScaleKey *k2, const float t, hsScaleValue *result);

    // simple linear interpolation
    static void LinInterp(const float k1, const float k2, const float t, float *result);
    static void LinInterp(const hsScalarTriple *k1, const hsScalarTriple *k2, const float t, hsScalarTriple *result);
    static void LinInterp(const hsColorRGBA *k1, const hsColorRGBA *k2, const float t, hsColorRGBA *result, uint32_t ignoreFlags=0);
    static void LinInterp(const hsMatrix33 *k1, const hsMatrix33 *k2, const float t, hsMatrix33 *result, uint32_t ignoreFlags=0);
    static void LinInterp(const hsMatrix44 *mat1, const hsMatrix44 *mat2, const float t, hsMatrix44 *out, uint32_t ignoreFlags=0);
    static void LinInterp(const hsQuat *k1, const hsQuat *k2, const float t, hsQuat *result);
    static void LinInterp(const hsScaleValue *k1, const hsScaleValue *k2, const float t, hsScaleValue *result);
    static void LinInterp(const hsAffineParts *k1, const hsAffineParts *k2, const float t, hsAffineParts *result, uint32_t ignoreFlags=0);

    // Given a time value, find the enclosing keyframes and normalize time (0-1)
    static void GetBoundaryKeyFrames(float time, uint32_t numKeys, void *keys, 
        uint32_t keySize, hsKeyFrame **kF1, hsKeyFrame **kF2, uint32_t *lastKeyIdx, float *p, bool forwards);

};

#define MAX_FRAMES_PER_SEC 30.0f
#define MAX_TICKS_PER_FRAME 160.0f
#define MAX_TICKS_PER_SEC (MAX_TICKS_PER_FRAME*MAX_FRAMES_PER_SEC)

#endif  // #ifndef HSINTERP_inc
