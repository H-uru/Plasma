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

#ifndef hsMatrixMath_inc
#define hsMatrixMath_inc

#include "HeadSpin.h"
#include "hsGeometry3.h"
#include "hsMatrix44.h"

#ifdef HS_BUILD_FOR_APPLE
#import <Accelerate/Accelerate.h>
#endif

static inline hsMatrix44 IMatrixMul34(const hsMatrix44& lhs, const hsMatrix44& rhs)
{
    hsMatrix44 ret;
    ret.NotIdentity();
    
#ifdef HS_BUILD_FOR_APPLE
    vDSP_mmul((const float*)lhs.fMap, 1, (const float*)rhs.fMap, 1, (float*)&(ret.fMap), 1, 3, 4, 4);
#else
    ret.fMap[0][0] = lhs.fMap[0][0] * rhs.fMap[0][0]
        + lhs.fMap[0][1] * rhs.fMap[1][0]
        + lhs.fMap[0][2] * rhs.fMap[2][0];

    ret.fMap[0][1] = lhs.fMap[0][0] * rhs.fMap[0][1]
        + lhs.fMap[0][1] * rhs.fMap[1][1]
        + lhs.fMap[0][2] * rhs.fMap[2][1];

    ret.fMap[0][2] = lhs.fMap[0][0] * rhs.fMap[0][2]
        + lhs.fMap[0][1] * rhs.fMap[1][2]
        + lhs.fMap[0][2] * rhs.fMap[2][2];

    ret.fMap[0][3] = lhs.fMap[0][0] * rhs.fMap[0][3]
        + lhs.fMap[0][1] * rhs.fMap[1][3]
        + lhs.fMap[0][2] * rhs.fMap[2][3]
        + lhs.fMap[0][3];

    ret.fMap[1][0] = lhs.fMap[1][0] * rhs.fMap[0][0]
        + lhs.fMap[1][1] * rhs.fMap[1][0]
        + lhs.fMap[1][2] * rhs.fMap[2][0];

    ret.fMap[1][1] = lhs.fMap[1][0] * rhs.fMap[0][1]
        + lhs.fMap[1][1] * rhs.fMap[1][1]
        + lhs.fMap[1][2] * rhs.fMap[2][1];

    ret.fMap[1][2] = lhs.fMap[1][0] * rhs.fMap[0][2]
        + lhs.fMap[1][1] * rhs.fMap[1][2]
        + lhs.fMap[1][2] * rhs.fMap[2][2];

    ret.fMap[1][3] = lhs.fMap[1][0] * rhs.fMap[0][3]
        + lhs.fMap[1][1] * rhs.fMap[1][3]
        + lhs.fMap[1][2] * rhs.fMap[2][3]
        + lhs.fMap[1][3];

    ret.fMap[2][0] = lhs.fMap[2][0] * rhs.fMap[0][0]
        + lhs.fMap[2][1] * rhs.fMap[1][0]
        + lhs.fMap[2][2] * rhs.fMap[2][0];

    ret.fMap[2][1] = lhs.fMap[2][0] * rhs.fMap[0][1]
        + lhs.fMap[2][1] * rhs.fMap[1][1]
        + lhs.fMap[2][2] * rhs.fMap[2][1];

    ret.fMap[2][2] = lhs.fMap[2][0] * rhs.fMap[0][2]
        + lhs.fMap[2][1] * rhs.fMap[1][2]
        + lhs.fMap[2][2] * rhs.fMap[2][2];

    ret.fMap[2][3] = lhs.fMap[2][0] * rhs.fMap[0][3]
        + lhs.fMap[2][1] * rhs.fMap[1][3]
        + lhs.fMap[2][2] * rhs.fMap[2][3]
        + lhs.fMap[2][3];
#endif
    ret.fMap[3][0] = ret.fMap[3][1] = ret.fMap[3][2] = 0;
    ret.fMap[3][3] = 1.f;

    return ret;
}

#endif
