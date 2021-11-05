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

#ifndef hsSystemInfo_Defined
#define hsSystemInfo_Defined

#include <cstdint>

namespace ST { class string; }

namespace hsSystemInfo
{
    /**
     * Returns all of the system information as a string.
     */
    ST::string AsString();

    /**
     * Gets the name of the CPU the current thread is executing on.
     * \remarks Example: "Intel(R) Core(TM) i7-4770K CPU @ 3.50GHz"
     */
    ST::string GetCPUBrand();

    /**
     * Gets the name of the current operating system.
     * \note If Plasma was built for Windows but is being emulated by WINE, we will
     * detect that and return all of: the windows version, kernel name and version, and
     * the WINE version.
     */
    ST::string GetOperatingSystem();

    /**
     * Gets the total system memory in MiB.
     * \note On some platforms, the operating system may subtract an amount of memory that
     * is in use by the kernel from this total.
     */
    uint64_t GetRAM();
}

#endif
