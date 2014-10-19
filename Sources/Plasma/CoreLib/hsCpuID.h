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

//////////////////////////////////////////////////////////////////////
//
// hsCpuID - Processor feature detection and function dispatcher
//
//
//     == Example Usage ==
//
//  #ifdef HS_SIMD_INCLUDE
//  #   include HS_SIMD_INCLUDE
//  #endif
//
//  float my_func_fpu() {
//    ...
//  }
//
//  float my_func_avx() {
//  #ifdef HS_AVX
//    ...
//  #endif
//  }
//
//
//  typedef float(*func_ptr)();
//  static hsFunctionDispatcher<func_ptr> my_func;
//
//  hsFunctionDispatcher<func_ptr> my_func(my_func_fpu, 0, 0, 0, 0, 0, 0, my_func_avx);
//  my_func();
//
//////////////////////////////////////////////////////////////////////



#ifndef hsCpuID_inc
#define hsCpuID_inc

#if defined __AVX2__ || _MSC_VER >= 1600
#define HS_AVX2
#ifndef HS_SIMD_INCLUDE
# define HS_SIMD_INCLUDE "immintrin.h"
#endif
#endif
#if defined __AVX__ || _MSC_VER >= 1600
#define HS_AVX
#ifndef HS_SIMD_INCLUDE
# define HS_SIMD_INCLUDE "immintrin.h"
#endif
#endif
#if defined __SSE4_2__ || _MSC_VER >= 1600
#define HS_SSE42
#ifndef HS_SIMD_INCLUDE
# define HS_SIMD_INCLUDE "nmmintrin.h"
#endif
#endif
#if defined __SSE4_1__ || _MSC_VER >= 1600
#define HS_SSE41
#ifndef HS_SIMD_INCLUDE
# define HS_SIMD_INCLUDE "smmintrin.h"
#endif
#endif
#if defined __SSSE3__ || _MSC_VER >= 1600
#define HS_SSSE3
#ifndef HS_SIMD_INCLUDE
# define HS_SIMD_INCLUDE "tmmintrin.h"
#endif
#endif
#if defined __SSE3__ || _MSC_VER >= 1400
#define HS_SSE3
#ifndef HS_SIMD_INCLUDE
# define HS_SIMD_INCLUDE "pmmintrin.h"
#endif
#endif
#if defined __SSE2__ || _MSC_VER >= 1300
#define HS_SSE2
#ifndef HS_SIMD_INCLUDE
# define HS_SIMD_INCLUDE "emmintrin.h"
#endif
#endif
#if defined __SSE__ || _MSC_VER >= 1300
#define HS_SSE1
#ifndef HS_SIMD_INCLUDE
# define HS_SIMD_INCLUDE "xmmintrin.h"
#endif
#endif


struct hsCpuId {
    bool has_sse1;
    bool has_sse2;
    bool has_sse3;
    bool has_ssse3;
    bool has_sse41;
    bool has_sse42;
    bool has_avx;
    bool has_avx2;

    hsCpuId();
    static const hsCpuId& Instance();
};

template <typename func_ptr>
struct hsCpuFunctionDispatcher {
    hsCpuFunctionDispatcher(func_ptr fpu,
                            func_ptr sse1 = nullptr,
                            func_ptr sse2 = nullptr,
                            func_ptr sse3 = nullptr,
                            func_ptr ssse3 = nullptr,
                            func_ptr sse41 = nullptr,
                            func_ptr sse42 = nullptr,
                            func_ptr avx = nullptr,
                            func_ptr avx2 = nullptr)
    {
        hsAssert(fpu, "FPU fallback function required.");
        const hsCpuId& cpu = hsCpuId::Instance();
#ifdef HS_AVX2
        if (cpu.has_avx2 && avx2) {
            call = avx2;
        } else
#endif
#ifdef HS_AVX
        if (cpu.has_avx && avx) {
            call = avx;
        } else
#endif
#ifdef HS_SSE42
        if (cpu.has_sse42 && sse42) {
            call = sse42;
        } else
#endif
#ifdef HS_SSE41
        if (cpu.has_sse41 && sse41) {
            call = sse41;
        } else
#endif
#ifdef HS_SSSE3
        if (cpu.has_ssse3 && ssse3) {
            call = ssse3;
        } else
#endif
#ifdef HS_SSE3
        if (cpu.has_sse3 && sse3) {
            call = sse3;
        } else
#endif
#ifdef HS_SSE2
        if (cpu.has_sse2 && sse2) {
            call = sse2;
        } else
#endif
#ifdef HS_SSE1
        if (cpu.has_sse1 && sse1) {
            call = sse1;
        } else
#endif
        {
            call = fpu;
        }
    };
    func_ptr call;
};


#endif  // hsCpuID_inc