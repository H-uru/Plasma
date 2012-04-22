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

#include <intrin.h>

#include "hsCpuID.h"

hsCpuId::hsCpuId() {
    const unsigned int sse1_flag = 1<<25;
    const unsigned int sse2_flag = 1<<26;
    const unsigned int sse3_flag = 1<<0;
    const unsigned int ssse3_flag = 1<<9;
    const unsigned int sse41_flag = 1<<19;
    const unsigned int sse42_flag = 1<<20;
    const unsigned int avx_flag = 1 << 28;

    unsigned int cpu_info[4];
    __cpuid((int*)cpu_info, 1);
    has_sse1 = (cpu_info[3] & sse1_flag) || false;
    has_sse2 = (cpu_info[3] & sse2_flag) || false;
    has_sse3 = (cpu_info[2] & sse3_flag) || false;
    has_ssse3 = (cpu_info[2] & ssse3_flag) || false;
    has_sse41 = (cpu_info[2] & sse41_flag) || false;
    has_sse42 = (cpu_info[2] & sse42_flag) || false;
    has_avx = (cpu_info[2] & avx_flag) || false;
}

const hsCpuId& hsCpuId::instance()
{
    static hsCpuId self;
    return self;
}
