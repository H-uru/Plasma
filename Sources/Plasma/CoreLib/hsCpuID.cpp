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

#include "hsConfig.h"

#if defined(HAVE_CPUID)
#   if defined(_MSC_VER) || ((defined(_WIN32) || defined(_WIN64)) && defined(__INTEL_COMPILER))
#       include <intrin.h>
#       define MSC_COMPATIBLE
#   elif defined(__GNUC__)
#       include <cpuid.h>
#       define GCC_COMPATIBLE
#   else
#       define SOFTWARE_ONLY
#   endif
#else
#   define SOFTWARE_ONLY
#endif

#include "hsCpuID.h"

hsCpuId::hsCpuId() {
    enum : unsigned int {
        // EAX=1; EDX=:
        sse1_flag  = 1U<<25,
        sse2_flag  = 1U<<26,

        // EAX=1; ECX=:
        sse3_flag  = 1U<<0,
        ssse3_flag = 1U<<9,
        sse41_flag = 1U<<19,
        sse42_flag = 1U<<20,
        avx_flag   = 1U<<28,

        // EAX=7; ECX=0; EBX=:
        avx2_flag  = 1U<<5
    };

    union RegSet {
        struct {
            unsigned int eax, ebx, ecx, edx;
        };
        int array[4];
    };

    RegSet CPUInfo_Features = { 0, 0, 0, 0 };
    RegSet CPUInfo_Ext = { 0, 0, 0, 0 };

    /**
     * Portable implementation of CPUID, successfully tested with:
     *   - Microsoft Visual Studio 2010,
     *   - GNU GCC 4.5,
     *   - Intel C++ Compiler 12.0
     *   - Sun Studio 12,
     *   - AMD x86 Open64 Compiler Suite.
     *
     * Ref: http://primesieve.googlecode.com/svn-history/r388/trunk/soe/cpuid.h
     */
#if defined(MSC_COMPATIBLE)
    __cpuid(CPUInfo_Features.array, 0);

    // check if the CPU supports the cpuid instruction.
    if (CPUInfo_Features.eax != 0) {
        __cpuid(CPUInfo_Features.array, 1);
        __cpuid(CPUInfo_Ext.array, 7);
    }
#elif defined(GCC_COMPATIBLE)
    __get_cpuid(1, &CPUInfo_Features.eax, &CPUInfo_Features.ebx,
                   &CPUInfo_Features.ecx, &CPUInfo_Features.edx);
    __get_cpuid(7, &CPUInfo_Ext.eax, &CPUInfo_Ext.ebx,
                   &CPUInfo_Ext.ecx, &CPUInfo_Ext.edx);
#endif


    has_sse1    = (CPUInfo_Features.edx & sse1_flag)  || false;
    has_sse2    = (CPUInfo_Features.edx & sse2_flag)  || false;
    has_sse3    = (CPUInfo_Features.ecx & sse3_flag)  || false;
    has_ssse3   = (CPUInfo_Features.ecx & ssse3_flag) || false;
    has_sse41   = (CPUInfo_Features.ecx & sse41_flag) || false;
    has_sse42   = (CPUInfo_Features.ecx & sse42_flag) || false;
    has_avx     = (CPUInfo_Features.ecx & avx_flag)   || false;
    has_avx2    = (CPUInfo_Ext.ebx      & avx2_flag)  || false;
}

const hsCpuId& hsCpuId::Instance()
{
    static hsCpuId self;
    return self;
}
