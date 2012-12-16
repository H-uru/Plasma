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

#if defined(_MSC_VER) || ((defined(_WIN32) || defined(_WIN64)) && defined(__INTEL_COMPILER))
#  include <intrin.h>
#  define MSC_COMPATIBLE
#elif defined(__GNUC__)
#  include <cpuid.h>
#  define GCC_COMPATIBLE
#endif

#include "hsCpuID.h"

hsCpuId::hsCpuId() {
    const unsigned int sse1_flag = 1<<25;
    const unsigned int sse2_flag = 1<<26;
    const unsigned int sse3_flag = 1<<0;
    const unsigned int ssse3_flag = 1<<9;
    const unsigned int sse41_flag = 1<<19;
    const unsigned int sse42_flag = 1<<20;
    const unsigned int avx_flag = 1 << 28;

    unsigned int ax = 0, bx = 0, cx = 0, dx = 0;


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
      int CPUInfo[4] = {ax, bx, cx, dx};
      __cpuid(CPUInfo, 0);

      // check if the CPU supports the cpuid instruction.
      if (CPUInfo[0] != 0) {
        __cpuid(CPUInfo, 1);
        ax = CPUInfo[0];
        bx = CPUInfo[1];
        cx = CPUInfo[2];
        dx = CPUInfo[3];
      }
    #elif defined(GCC_COMPATIBLE)
      __get_cpuid(1, &ax, &bx, &cx, &dx);
    #endif


    has_sse1    = (dx & sse1_flag)  || false;
    has_sse2    = (dx & sse2_flag)  || false;
    has_sse3    = (cx & sse3_flag)  || false;
    has_ssse3   = (cx & ssse3_flag) || false;
    has_sse41   = (cx & sse41_flag) || false;
    has_sse42   = (cx & sse42_flag) || false;
    has_avx     = (cx & avx_flag)   || false;
}

const hsCpuId& hsCpuId::instance()
{
    static hsCpuId self;
    return self;
}
