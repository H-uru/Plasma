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
      Cyan Worlds, I
      nc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#include "hsMatrix44.h"

#ifdef HAVE_SSE3
#   include <pmmintrin.h>

#   define MULTBEGIN(i) \
        xmm[0]   = _mm_loadu_ps(a.fMap[i]);
#   define MULTCELL(i, j) \
        xmm[1]   = _mm_set_ps(b.fMap[3][j], b.fMap[2][j], b.fMap[1][j], b.fMap[0][j]); \
        xmm[j+2] = _mm_mul_ps(xmm[0], xmm[1]);
#   define MULTFINISH(i) \
        xmm[6] = _mm_hadd_ps(xmm[2], xmm[3]); \
        xmm[7] = _mm_hadd_ps(xmm[4], xmm[5]); \
        xmm[1] = _mm_hadd_ps(xmm[6], xmm[7]); \
        _mm_storeu_ps(c.fMap[i], xmm[1]);
#endif

hsMatrix44 hsMatrix44::mult_sse3(const hsMatrix44& a, const hsMatrix44& b)
{
    hsMatrix44 c;

#ifdef HAVE_SSE3
    if (a.fFlags & b.fFlags & hsMatrix44::kIsIdent) {
        c.Reset();
        return c;
    }

    if (a.fFlags & hsMatrix44::kIsIdent)
        return b;
    if (b.fFlags & hsMatrix44::kIsIdent)
        return a;

    __m128 xmm[8];

    MULTBEGIN(0);
    MULTCELL(0, 0);
    MULTCELL(0, 1);
    MULTCELL(0, 2);
    MULTCELL(0, 3);
    MULTFINISH(0);

    MULTBEGIN(1);
    MULTCELL(1, 0);
    MULTCELL(1, 1);
    MULTCELL(1, 2);
    MULTCELL(1, 3);
    MULTFINISH(1);

    MULTBEGIN(2);
    MULTCELL(2, 0);
    MULTCELL(2, 1);
    MULTCELL(2, 2);
    MULTCELL(2, 3);
    MULTFINISH(2);

    MULTBEGIN(3);
    MULTCELL(3, 0);
    MULTCELL(3, 1);
    MULTCELL(3, 2);
    MULTCELL(3, 3);
    MULTFINISH(3);
#endif

    return c;
}
