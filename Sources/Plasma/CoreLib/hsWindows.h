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

#ifndef _hsWindows_inc_
#define _hsWindows_inc_

/** \file hsWindows.h
 *  \brief Pulls in Windows core headers
 *
 *  This file pulls in the core Windows headers and Winsock2. It is separate from
 *  HeadSpin.h to improve build times and to facillitate adding precompiled headers.
 *  You should avoid including this header from other headers!
 */

#ifdef HS_BUILD_FOR_WIN32
    // Terrible hacks for MinGW because they don't have a reasonable
    // default for the Windows version. We cheat and say it's XP.
#   ifdef __MINGW32__
#       undef _WIN32_WINNT
#       define _WIN32_WINNT 0x501
#       undef _WIN32_IE
#       define _WIN32_IE    0x400
#   endif

    // HACK: Max headers depend on the min() and max() macros normally pulled
    // in by windows.h... However, we usually disable those, since they break
    // std::min and std::max.  Therefore, we bring the std:: versions down to
    // the global namespace so we can still compile max code without breaking
    // everything else :/
#   ifndef NOMINMAX
#       define NOMINMAX
#       include <algorithm>
        using std::min;
        using std::max;
#   endif

#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   include <ws2tcpip.h> // Pulls in WinSock 2 for us

    // This needs to be after #include <windows.h>, since it also includes windows.h
#   ifdef USE_VLD
#       include <vld.h>
#   endif // USE_VLD
#endif // HS_BUILD_FOR_WIN32

#endif // _hsWindows_inc_
