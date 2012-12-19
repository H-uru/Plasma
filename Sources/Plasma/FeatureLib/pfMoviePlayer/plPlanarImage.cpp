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

#include "plPlanarImage.h"

///////////////////////////////////////////////////////////////////////////////

static uint8_t Clip(int32_t val) {
    if (val < 0) {
        return 0;
    } else if (val > 255) {
        return 255;
    }
    return static_cast<uint8_t>(val);
}

#define YG 74 /* static_cast<int8>(1.164 * 64 + 0.5) */

#define UB 127 /* min(63,static_cast<int8>(2.018 * 64)) */
#define UG -25 /* static_cast<int8>(-0.391 * 64 - 0.5) */
#define UR 0

#define VB 0
#define VG -52 /* static_cast<int8>(-0.813 * 64 - 0.5) */
#define VR 102 /* static_cast<int8>(1.596 * 64 + 0.5) */

// Bias
#define BB UB * 128 + VB * 128
#define BG UG * 128 + VG * 128
#define BR UR * 128 + VR * 128

void plPlanarImage::Yuv420ToRgba(uint32_t w, uint32_t h, const int32_t* stride, uint8_t** planes, uint8_t* const dest)
{
    const uint8_t* y_src = planes[0];
    const uint8_t* u_src = planes[1];
    const uint8_t* v_src = planes[2];

    for (uint32_t i = 0; i < h; ++i)
    {
        for (uint32_t j = 0; j < w; ++j)
        {
            size_t y_idx = stride[0] * i + j;
            size_t u_idx = stride[1] * (i/2) + (j/2);
            size_t v_idx = stride[2] * (i/2) + (j/2);
            size_t dest_idx = w * i + j;

            int32_t y = static_cast<int32_t>(y_src[y_idx]);
            int32_t u = static_cast<int32_t>(u_src[u_idx]);
            int32_t v = static_cast<int32_t>(v_src[v_idx]);
            int32_t y1 = (y - 16) * YG;

            dest[dest_idx*4+0] = Clip(((u * UB + v * VB) - (BB) + y1) >> 6);
            dest[dest_idx*4+1] = Clip(((u * UG + v * VG) - (BG) + y1) >> 6);
            dest[dest_idx*4+2] = Clip(((u * UR + v * VR) - (BR) + y1) >> 6);
            dest[dest_idx*4+3] = 0xff;
        }
    }
}
