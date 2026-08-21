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

#include "plSoftwareMeshSkinner.h"

#include "hsMatrix44.h"
#include "hsSIMD.h"

#ifdef HAVE_SSE3
static inline void ISkinDpSSE3(const float* src, float* dst, const __m128& mc0,
                               const __m128& mc1, const __m128& mc2, const __m128& mwt)
{
    __m128 msr = _mm_load_ps(src);
    __m128 _x  = _mm_mul_ps(_mm_mul_ps(mc0, msr), mwt);
    __m128 _y  = _mm_mul_ps(_mm_mul_ps(mc1, msr), mwt);
    __m128 _z  = _mm_mul_ps(_mm_mul_ps(mc2, msr), mwt);

    __m128 hbuf1 = _mm_hadd_ps(_x, _y);
    __m128 hbuf2 = _mm_hadd_ps(_z, _z);
    hbuf1 = _mm_hadd_ps(hbuf1, hbuf2);
    __m128 _dst = _mm_load_ps(dst);
    _dst = _mm_add_ps(_dst, hbuf1);
    _mm_store_ps(dst, _dst);
}
#endif // HAVE_SSE3

namespace plSoftwareMeshSkinner::_detail {
    void ISkinVertexSSE3(const hsMatrix44& xfm, float wgt, const float* srcBuf, float* dstBuf)
    {
#   ifdef HAVE_SSE3
        __m128 mc0 = _mm_load_ps(xfm.fMap[0]);
        __m128 mc1 = _mm_load_ps(xfm.fMap[1]);
        __m128 mc2 = _mm_load_ps(xfm.fMap[2]);
        __m128 mwt = _mm_set_ps1(wgt);

        const float* pt_src = &srcBuf[0];
        const float* vec_src = &srcBuf[4];

        float* pt_dst = &dstBuf[0];
        float* vec_dst = &dstBuf[4];

        ISkinDpSSE3(pt_src, pt_dst, mc0, mc1, mc2, mwt);
        ISkinDpSSE3(vec_src, vec_dst, mc0, mc1, mc2, mwt);
#   endif // HAVE_SSE3
    }
};
