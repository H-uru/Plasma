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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCore/pnAcCore.cpp
*   
***/

#include "pnAcInt.h"
#include "HeadSpin.h"
#if HS_BUILD_FOR_WIN32
#   include <windows.h>
#endif

#pragma hdrstop

#include <atomic>

/*****************************************************************************
*
*   Private data
*
***/

static std::atomic<long> s_perf[kNumAsyncPerfCounters];


/*****************************************************************************
*
*   Module exports
*
***/

//============================================================================
long PerfAddCounter (EAsyncPerfCounter id, unsigned n) {
    ASSERT(id < kNumAsyncPerfCounters);
    return s_perf[id].fetch_add(n);
}

//============================================================================
long PerfSubCounter (EAsyncPerfCounter id, unsigned n) {
    ASSERT(id < kNumAsyncPerfCounters);
    return s_perf[id].fetch_sub(n);
}

//============================================================================
long PerfSetCounter (EAsyncPerfCounter id, unsigned n) {
    ASSERT(id < kNumAsyncPerfCounters);
    return s_perf[id].exchange(n);
}


/*****************************************************************************
*
*   Public exports
*
***/

//============================================================================
long AsyncPerfGetCounter (EAsyncPerfCounter id) {
    static_assert(arrsize(s_perf) == kNumAsyncPerfCounters, "Max async counters and array size do not match.");
    ASSERT(id < kNumAsyncPerfCounters);
    return s_perf[id];
}
