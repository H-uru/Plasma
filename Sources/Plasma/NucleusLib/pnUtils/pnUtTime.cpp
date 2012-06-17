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
#include "pnUtTime.h"

#if HS_BUILD_FOR_UNIX
#include <time.h>
#include <sys/time.h>
#endif

/******************************************************************************
 * I've combined all the pnUtils Time stuff in here and made it work cross-
 * platform as best as possible.
 *
 * At some point, hsTimer needs to be rewritten to ensure that it's fully
 * corss-platform and using a reliable millisecond clock.
 * When that happens, these functions should be merged into hsTimer.
 *
 * -- dpogue (April 14, 2012)
 *
 *****************************************************************************/

namespace pnUtilsExe {

uint32_t TimeGetTickCount () {
#if HS_BUILD_FOR_WIN32
    return GetTickCount();
#else
    struct timeval tv;
    if (gettimeofday(&tv, NULL) != 0)
        return 0;

    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
#endif
}

/*****************************************************************************
*
*   Time adjustment functions
*
*   For debug builds, adjust the initial time value so that the high
*   bit or the time value itself is about to wrap, to catch application
*   bugs that don't handle wrapping or depend on the high bit's value.
*
***/

static uint32_t s_adjustment;

//===========================================================================
static void InitializeAdjustment () {
    ASSERT(!s_adjustment);
    uint32_t currTime  = TimeGetTickCount();
    uint32_t startBits = (currTime & 0x80) ? 0x7fff0000 : 0xffff0000;
    uint32_t startMask = 0xffff0000;
    s_adjustment = (((currTime & ~startMask) | startBits) - currTime) | 1;
    ASSERT(s_adjustment);
}

//===========================================================================
AUTO_INIT_FUNC(AutoInitializeAdjustment) {
    if (!s_adjustment)
        InitializeAdjustment();
}

} using namespace pnUtilsExe;


/*****************************************************************************
*
*   Exports
*
***/

uint32_t TimeGetSecondsSince2001Utc () {
    uint64_t time    = TimeGetTime();
    uint32_t seconds = (uint32_t)((time - kTime1601To2001) / kTimeIntervalsPerSecond);
    return seconds;
}

uint64_t TimeGetTime () {
#ifdef HS_BUILD_FOR_WIN32
    uint64_t time;
    COMPILER_ASSERT(sizeof(uint64_t) == sizeof(FILETIME));
    GetSystemTimeAsFileTime((FILETIME *) &time);
    return time;
#else
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts))
        return 0;

    long long time = ts.tv_sec * 1000000000LL + ts.tv_nsec;

    return time / 100;
#endif
}

uint32_t TimeGetMs () {
#ifdef HS_DEBUGGING
    // For debug builds, return an adjusted timer value
    if (!s_adjustment)
        InitializeAdjustment();
    return TimeGetTickCount() + s_adjustment;
#else
    // For release builds, just return the operating system's timer
    return TimeGetTickCount();
#endif
}
