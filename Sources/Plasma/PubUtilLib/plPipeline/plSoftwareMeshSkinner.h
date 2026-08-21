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
#ifndef _plSoftwareMeshSkinner_inc_
#define _plSoftwareMeshSkinner_inc_

#include "HeadSpin.h"
#include "hsCpuID.h"

#if !defined(__has_include)
#   define __has_include(x) 0
#endif

class plSpan;
class hsMatrix44;

namespace plSoftwareMeshSkinner
{
    typedef void (*blend_vert_buffer_ptr)(const plSpan*, hsMatrix44*, int, const uint8_t*, uint8_t, uint32_t, uint8_t*, uint32_t, uint32_t, uint16_t);
    typedef void (*skin_vert_ptr)(const hsMatrix44&, float, const float*, float*);

    namespace _detail {
        void ISkinVertexFPU(const hsMatrix44& xfm, float wgt, const float* srcBuf, float* dstBuf);
        void ISkinVertexSSE3(const hsMatrix44& xfm, float wgt, const float* srcBuf, float* dstBuf);

        template<skin_vert_ptr T>
        void IBlendVertBuffer(const plSpan* span, hsMatrix44* matrixPalette, int numMatrices,
                                     const uint8_t* src, uint8_t format, uint32_t srcStride,
                                     uint8_t* dest, uint32_t destStride, uint32_t count,
                                     uint16_t localUVWChans);
    }

    extern hsCpuFunctionDispatcher<blend_vert_buffer_ptr> blend_vert_buffer;

    /**
     * Given a pointer into a buffer of verts that have blending data in the
     * plVertCoder format, blends them into the destination buffer given
     * without the blending info.
     */
    inline void BlendVertBuffer(const plSpan* span,
            hsMatrix44* matrixPalette, int numMatrices,
            const uint8_t* src, uint8_t format, uint32_t srcStride,
            uint8_t* dest, uint32_t destStride, uint32_t count,
            uint16_t localUVWChans)
#if defined(HS_BUILD_FOR_APPLE) && __has_include(<simd/simd.h>)
    ;
#else
    {
        blend_vert_buffer.call(span, matrixPalette, numMatrices, src, format, srcStride, dest, destStride, count, localUVWChans);
    }
#endif
};

#endif // _plSoftwareMeshSkinner_inc_
