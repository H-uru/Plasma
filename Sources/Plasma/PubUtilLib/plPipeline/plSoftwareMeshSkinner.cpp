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
#include "plDrawable/plGBufferGroup.h"
#include "plDrawable/plSpanTypes.h"

#if !defined(__has_include)
#   define __has_include(x) 0
#endif

template<typename T>
static inline void inlCopy(uint8_t*& src, uint8_t*& dst)
{
    T* src_ptr = reinterpret_cast<T*>(src);
    T* dst_ptr = reinterpret_cast<T*>(dst);
    *dst_ptr = *src_ptr;
    src += sizeof(T);
    dst += sizeof(T);
}

static inline void inlCopy(const uint8_t*& src, uint8_t*& dst, size_t sz)
{
    memcpy(dst, src, sz);
    src += sz;
    dst += sz;
}

template<typename T>
static inline const uint8_t* inlExtract(const uint8_t* src, T* val)
{
    const T* ptr = reinterpret_cast<const T*>(src);
    *val = *ptr++;
    return reinterpret_cast<const uint8_t*>(ptr);
}

template<>
inline const uint8_t* inlExtract<hsPoint3>(const uint8_t* src, hsPoint3* val)
{
    const float* src_ptr = reinterpret_cast<const float*>(src);
    float* dst_ptr = reinterpret_cast<float*>(val);
    *dst_ptr++ = *src_ptr++;
    *dst_ptr++ = *src_ptr++;
    *dst_ptr++ = *src_ptr++;
    *dst_ptr = 1.f;
    return reinterpret_cast<const uint8_t*>(src_ptr);
}

template<>
inline const uint8_t* inlExtract<hsVector3>(const uint8_t* src, hsVector3* val)
{
    const float* src_ptr = reinterpret_cast<const float*>(src);
    float* dst_ptr = reinterpret_cast<float*>(val);
    *dst_ptr++ = *src_ptr++;
    *dst_ptr++ = *src_ptr++;
    *dst_ptr++ = *src_ptr++;
    *dst_ptr = 0.f;
    return reinterpret_cast<const uint8_t*>(src_ptr);
}

template<typename T, size_t N>
static inline void inlSkip(uint8_t*& src)
{
    src += sizeof(T) * N;
}

template<typename T>
static inline uint8_t* inlStuff(uint8_t* dst, const T* val)
{
    T* ptr = reinterpret_cast<T*>(dst);
    *ptr++ = *val;
    return reinterpret_cast<uint8_t*>(ptr);
}


#if defined(HS_BUILD_FOR_APPLE) && __has_include(<simd/simd.h>)
#   include <simd/simd.h>

    static inline const matrix_float4x4 hsMatrix2SIMD(const hsMatrix44& src)
    {
        constexpr auto matrixSize = sizeof(matrix_float4x4);
        if (src.fFlags & hsMatrix44::kIsIdent) {
            return matrix_identity_float4x4;
        }

        simd_float4x4 dst;
        memcpy(&dst, &src.fMap, matrixSize);
        return dst;
    }

    void plSoftwareMeshSkinner::BlendVertBuffer(
            const plSpan* span, hsMatrix44* matrixPalette, int numMatrices,
            const uint8_t* src, uint8_t format, uint32_t srcStride,
            uint8_t* dest, uint32_t destStride, uint32_t count,
            uint16_t localUVWChans)
    {
        simd_float4 pt_buf = {0.f, 0.f, 0.f, 1.f};
        simd_float4 vec_buf = {0.f, 0.f, 0.f, 0.f};

        hsPoint3*  pt = reinterpret_cast<hsPoint3*>(&pt_buf);
        hsVector3* vec = reinterpret_cast<hsVector3*>(&vec_buf);

        uint32_t indices;
        float    weights[4];

        // Dropped support for localUVWChans at templatization of code
        hsAssert(localUVWChans == 0, "support for skinned UVWs dropped. reimplement me?");

        const size_t uvChanSize = plGBufferGroup::CalcNumUVs(format) * sizeof(float) * 3;
        uint8_t      numWeights = (format & plGBufferGroup::kSkinWeightMask) >> 4;

        for (uint32_t i = 0; i < count; ++i) {
            // Extract data
            src = inlExtract<hsPoint3>(src, pt);

            float weightSum = 0.f;
            for (uint8_t j = 0; j < numWeights; ++j) {
                src = inlExtract<float>(src, &weights[j]);
                weightSum += weights[j];
            }
            weights[numWeights] = 1.f - weightSum;

            if (format & plGBufferGroup::kSkinIndices)
                src = inlExtract<uint32_t>(src, &indices);
            else
                indices = 1 << 8;
            src = inlExtract<hsVector3>(src, vec);

            // Destination buffers (float4 for SSE alignment)
            simd_float4 destNorm_buf = { 0.f, 0.f, 0.f, 0.f };
            simd_float4 destPt_buf = { 0.f, 0.f, 0.f, 1.f };

            // Blend
            for (uint32_t j = 0; j < numWeights + 1; ++j) {
                float weight = weights[j];
                if (weight) {
                    const simd_float4x4& simdMatrix = hsMatrix2SIMD(matrixPalette[indices & 0xFF]);

                    // Note: This uses Accelerate.framework so this is also accelerated on ARM through NEON or maybe even the Neural Engine.
                    destPt_buf += simd_mul(pt_buf, simdMatrix) * weight;
                    destNorm_buf += simd_mul(vec_buf, simdMatrix) * weight;
                }
                indices >>= 8;
            }

            // Probably don't really need to renormalize this. There errors are
            // going to be subtle and "smooth".
            /* hsFastMath::NormalizeAppr(destNorm); */

            // Slam data into position now
            dest = inlStuff<hsPoint3>(dest, reinterpret_cast<hsPoint3*>(&destPt_buf));
            dest = inlStuff<hsVector3>(dest, reinterpret_cast<hsVector3*>(&destNorm_buf));

            // memcpy the colors and UVs
            inlCopy(src, dest, sizeof(uint32_t) * 2 + uvChanSize);
        }
    }

#else // HS_BUILD_FOR_APPLE && simd.h
#   include "hsSIMD.h"

    typedef void (*blend_vert_buffer_ptr)(const plSpan*, hsMatrix44*, int, const uint8_t*, uint8_t, uint32_t, uint8_t*, uint32_t, uint32_t, uint16_t);
    typedef void (*skin_vert_ptr)(const hsMatrix44&, float, const float*, float*);

#   define SPLIT_SKIN_BUFFER(src, pt, vec) \
        decltype(src) pt = &src[0];         \
        decltype(src) vec = &src[4];        //

#   define SPLIT_SKIN_TRIPLES(src, pt, vec)                    \
        hsPoint3* pt = reinterpret_cast<hsPoint3*>(&src[0]);    \
        hsVector3* vec = reinterpret_cast<hsVector3*>(&src[4]); //

    static inline void ISkinVertexFPU(const hsMatrix44& xfm, float wgt,
                                  const float* srcBuf, float* dstBuf)
    {
        const float& m00 = xfm.fMap[0][0];
        const float& m01 = xfm.fMap[0][1];
        const float& m02 = xfm.fMap[0][2];
        const float& m03 = xfm.fMap[0][3];
        const float& m10 = xfm.fMap[1][0];
        const float& m11 = xfm.fMap[1][1];
        const float& m12 = xfm.fMap[1][2];
        const float& m13 = xfm.fMap[1][3];
        const float& m20 = xfm.fMap[2][0];
        const float& m21 = xfm.fMap[2][1];
        const float& m22 = xfm.fMap[2][2];
        const float& m23 = xfm.fMap[2][3];

        SPLIT_SKIN_BUFFER(srcBuf, pt_src, vec_src);
        SPLIT_SKIN_BUFFER(dstBuf, pt_dst, vec_dst);

        // position
        {
            const float& srcX = pt_src[0];
            const float& srcY = pt_src[1];
            const float& srcZ = pt_src[2];

            pt_dst[0] += (srcX * m00 + srcY * m01 + srcZ * m02 + m03) * wgt;
            pt_dst[1] += (srcX * m10 + srcY * m11 + srcZ * m12 + m13) * wgt;
            pt_dst[2] += (srcX * m20 + srcY * m21 + srcZ * m22 + m23) * wgt;
        }

        // normal
        {
            const float& srcX = vec_src[0];
            const float& srcY = vec_src[1];
            const float& srcZ = vec_src[2];

            vec_dst[0] += (srcX * m00 + srcY * m01 + srcZ * m02) * wgt;
            vec_dst[1] += (srcX * m10 + srcY * m11 + srcZ * m12) * wgt;
            vec_dst[2] += (srcX * m20 + srcY * m21 + srcZ * m22) * wgt;
        }
    }

#   ifdef HAVE_SSE3
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
#   endif // HAVE_SSE3

    static inline void ISkinVertexSSE3(const hsMatrix44& xfm, float wgt,
                                       const float* srcBuf, float* dstBuf)
    {
#   ifdef HAVE_SSE3
        __m128 mc0 = _mm_load_ps(xfm.fMap[0]);
        __m128 mc1 = _mm_load_ps(xfm.fMap[1]);
        __m128 mc2 = _mm_load_ps(xfm.fMap[2]);
        __m128 mwt = _mm_set_ps1(wgt);

        SPLIT_SKIN_BUFFER(srcBuf, pt_src, vec_src);
        SPLIT_SKIN_BUFFER(dstBuf, pt_dst, vec_dst);

        ISkinDpSSE3(pt_src, pt_dst, mc0, mc1, mc2, mwt);
        ISkinDpSSE3(vec_src, vec_dst, mc0, mc1, mc2, mwt);
#   endif // HAVE_SSE3
    }

    template<skin_vert_ptr T>
    static void IBlendVertBuffer(const plSpan* span, hsMatrix44* matrixPalette, int numMatrices,
                                 const uint8_t* src, uint8_t format, uint32_t srcStride,
                                 uint8_t* dest, uint32_t destStride, uint32_t count,
                                 uint16_t localUVWChans)
    {
        alignas(32) float srcBuf[8]{};
        SPLIT_SKIN_TRIPLES(srcBuf, srcPt, srcVec);

        uint32_t        indices;
        float           weights[4];

        // Dropped support for localUVWChans at templatization of code
        hsAssert(localUVWChans == 0, "support for skinned UVWs dropped. reimplement me?");
        const size_t uvChanSize = plGBufferGroup::CalcNumUVs(format) * sizeof(float) * 3;
        uint8_t numWeights = (format & plGBufferGroup::kSkinWeightMask) >> 4;

        for (uint32_t i = 0; i < count; ++i) {
            // Extract data
            src = inlExtract<hsPoint3>(src, srcPt);

            float weightSum = 0.f;
            for (uint8_t j = 0; j < numWeights; ++j) {
                src = inlExtract<float>(src, &weights[j]);
                weightSum += weights[j];
            }
            weights[numWeights] = 1.f - weightSum;

            if (format & plGBufferGroup::kSkinIndices)
                src = inlExtract<uint32_t>(src, &indices);
            else
                indices = 1 << 8;
            src = inlExtract<hsVector3>(src, srcVec);

            // Destination buffers (float4 for SSE alignment)
            alignas(32) float destBuf[8]{};
            SPLIT_SKIN_TRIPLES(destBuf, destPt, destVec);

            // Blend
            for (uint32_t j = 0; j < numWeights + 1; ++j) {
                if (weights[j])
                    T(matrixPalette[indices & 0xFF], weights[j], srcBuf, destBuf);
                indices >>= 8;
            }

            // Probably don't really need to renormalize this. There errors are
            // going to be subtle and "smooth".
            /* hsFastMath::NormalizeAppr(destNorm); */

            // Slam data into position now
            dest = inlStuff<hsPoint3>(dest, reinterpret_cast<hsPoint3*>(destPt));
            dest = inlStuff<hsVector3>(dest, reinterpret_cast<hsVector3*>(destVec));

            // memcpy the colors and UVs
            inlCopy(src, dest, sizeof(uint32_t) * 2 + uvChanSize);
        }
    }

#   undef SPLIT_SKIN_TRIPLES
#   undef SPLIT_SKIN_BUFFER

    static hsCpuFunctionDispatcher<blend_vert_buffer_ptr> blend_vert_buffer {
        &IBlendVertBuffer<ISkinVertexFPU>,
        nullptr,                            // SSE1
        nullptr,                            // SSE2
        &IBlendVertBuffer<ISkinVertexSSE3>
    };

    void plSoftwareMeshSkinner::BlendVertBuffer(
            const plSpan* span, hsMatrix44* matrixPalette, int numMatrices,
            const uint8_t* src, uint8_t format, uint32_t srcStride,
            uint8_t* dest, uint32_t destStride, uint32_t count,
            uint16_t localUVWChans)
    {
        blend_vert_buffer.call(span, matrixPalette, numMatrices, src, format, srcStride, dest, destStride, count, localUVWChans);
    }

#endif // HS_BUILD_FOR_APPLE && simd.h
