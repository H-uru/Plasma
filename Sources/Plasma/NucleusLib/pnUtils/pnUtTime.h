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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtTime.h
*   
***/

#ifndef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTTIME_H
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTTIME_H

#include "Pch.h"

/*****************************************************************************
*
*   Time formatting functions
*
***/

struct TimeDesc {
    unsigned year;
    unsigned month;         // [1, 12]
    unsigned day;
    unsigned dayOfWeek;     // [0,  6]
    unsigned hour;          // [0, 23]
    unsigned minute;        // [0, 59]
    unsigned second;        // [0, 59]
}; 

struct TimeElapsedDesc {
    unsigned years;
    unsigned months;        // [0, 12]
    unsigned days;          // [0,  7]
    unsigned weeks;         // [0,  6]
    unsigned hours;         // [0, 23]
    unsigned minutes;       // [0, 59]
};

void TimeGetDesc (
    uint64_t             time,
    TimeDesc *        desc
);

void TimeGetElapsedDesc (
    uint32_t             minutesElapsed,
    TimeElapsedDesc * desc
);

void TimePrettyPrint (
    uint64_t       time,
    unsigned    chars,
    wchar_t *     buffer
);


/*****************************************************************************
*
*   Time query functions
*
***/

const uint64_t kTimeIntervalsPerMs         = 10000;
const uint64_t kTimeIntervalsPerSecond     = 1000 * kTimeIntervalsPerMs;
const uint64_t kTimeIntervalsPerMinute     = 60 * kTimeIntervalsPerSecond;
const uint64_t kTimeIntervalsPerHour       = 60 * kTimeIntervalsPerMinute;
const uint64_t kTimeIntervalsPerDay        = 24 * kTimeIntervalsPerHour;

// millisecond timer; wraps ~49 days
uint32_t TimeGetMs ();

// 100 nanosecond intervals; won't wrap in our lifetimes
uint64_t TimeGetTime ();
uint64_t TimeGetLocalTime ();

// Minutes elapsed since 2001 UTC
uint32_t TimeGetMinutes ();

// Seconds elapsed since 00:00:00 January 1, 2001 UTC
uint32_t TimeGetSecondsSince2001Utc ();

// Seconds elapsed since 00:00:00 January 1, 1970 UTC (the Unix Epoch)
uint32_t TimeGetSecondsSince1970Utc ();


// These magic numbers taken from Microsoft's "Shared Source CLI implementation" source code.
// http://msdn.microsoft.com/library/en-us/Dndotnet/html/mssharsourcecli.asp

static const uint64_t kTime1601To1970  = 11644473600 * kTimeIntervalsPerSecond;
static const uint64_t kTime1601To2001  = 12622780800 * kTimeIntervalsPerSecond;
#endif
