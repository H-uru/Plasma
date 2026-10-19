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

#if defined(HAVE_NEON) && defined(__aarch64__)
static inline void ISkinDpNEON(const float* src, float* dst, const float32x4_t& mc0,
                               const float32x4_t& mc1, const float32x4_t& mc2, const float32x4_t& mwt)
{
    float32x4_t msr = vld1q_f32(src);
    float32x4_t _x = vmulq_f32(vmulq_f32(mc0, msr), mwt);
    float32x4_t _y = vmulq_f32(vmulq_f32(mc1, msr), mwt);
    float32x4_t _z = vmulq_f32(vmulq_f32(mc2, msr), mwt);

    float32x4_t hbuf1 = vpaddq_f32(_x, _y);
    float32x4_t hbuf2 = vpaddq_f32(_z, _z);
    hbuf1 = vpaddq_f32(hbuf1, hbuf2);
    float32x4_t _dst = vld1q_f32(dst);
    _dst = vaddq_f32(_dst, hbuf1);
    vst1q_f32(dst, _dst);
}
#endif // HAVE_NEON

namespace plSoftwareMeshSkinner::_detail
{
void ISkinVertexNEON(const hsMatrix44& xfm, float wgt,
                     const float* srcBuf, float* dstBuf)
{
#if defined(HAVE_NEON) && defined(__aarch64__)
    float32x4_t mc0 = vld1q_f32(xfm.fMap[0]);
    float32x4_t mc1 = vld1q_f32(xfm.fMap[1]);
    float32x4_t mc2 = vld1q_f32(xfm.fMap[2]);
    float32x4_t mwt = vdupq_n_f32(wgt);

    const float* pt_src = &srcBuf[0];
    const float* vec_src = &srcBuf[4];

    float* pt_dst = &dstBuf[0];
    float* vec_dst = &dstBuf[4];

    ISkinDpNEON(pt_src, pt_dst, mc0, mc1, mc2, mwt);
    ISkinDpNEON(vec_src, vec_dst, mc0, mc1, mc2, mwt);
#endif // HAVE_NEON
}
}; // namespace plSoftwareMeshSkinner::_detail
