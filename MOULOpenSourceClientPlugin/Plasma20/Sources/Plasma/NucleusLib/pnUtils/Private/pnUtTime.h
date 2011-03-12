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

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTTIME_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtTime.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTTIME_H


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
    qword             time,
    TimeDesc *        desc
);

void TimeGetElapsedDesc (
    dword             minutesElapsed,
    TimeElapsedDesc * desc
);

void TimePrettyPrint (
	qword       time,
	unsigned    chars,
	wchar *     buffer
);


/*****************************************************************************
*
*   Time query functions
*
***/

const qword kTimeIntervalsPerMs         = 10000;
const qword kTimeIntervalsPerSecond     = 1000 * kTimeIntervalsPerMs;
const qword kTimeIntervalsPerMinute     = 60 * kTimeIntervalsPerSecond;
const qword kTimeIntervalsPerHour       = 60 * kTimeIntervalsPerMinute;
const qword kTimeIntervalsPerDay        = 24 * kTimeIntervalsPerHour;

// millisecond timer; wraps ~49 days
dword TimeGetMs ();

// 100 nanosecond intervals; won't wrap in our lifetimes
qword TimeGetTime ();
qword TimeGetLocalTime ();

// Minutes elapsed since 2001 UTC
dword TimeGetMinutes ();

// Seconds elapsed since 00:00:00 January 1, 2001 UTC
dword TimeGetSecondsSince2001Utc ();

// Seconds elapsed since 00:00:00 January 1, 1970 UTC (the Unix Epoch)
dword TimeGetSecondsSince1970Utc ();


// These magic numbers taken from Microsoft's "Shared Source CLI implementation" source code.
// http://msdn.microsoft.com/library/en-us/Dndotnet/html/mssharsourcecli.asp

static const qword kTime1601To1970	= 11644473600 * kTimeIntervalsPerSecond;
static const qword kTime1601To2001	= 12622780800 * kTimeIntervalsPerSecond;
