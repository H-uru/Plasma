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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtTime.cpp
*   
***/

#include "../Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Exports
*
***/

//===========================================================================
void TimeGetElapsedDesc (
    dword             minutesElapsed,
    TimeElapsedDesc * desc
) {

    const unsigned kMinutesPerHour  = 60;
    const unsigned kMinutesPerDay   = 1440;
    const unsigned kMinutesPerWeek  = 10080;
    const unsigned kMinutesPerMonth = 43830;
    const unsigned kMinutesPerYear  = 525960;

    dword & elapsed = minutesElapsed;
    desc->years   = (elapsed / kMinutesPerYear);  elapsed -= desc->years  * kMinutesPerYear;
    desc->months  = (elapsed / kMinutesPerMonth); elapsed -= desc->months * kMinutesPerMonth;
    desc->weeks   = (elapsed / kMinutesPerWeek);  elapsed -= desc->weeks  * kMinutesPerWeek;
    desc->days    = (elapsed / kMinutesPerDay);   elapsed -= desc->days   * kMinutesPerDay;
    desc->hours   = (elapsed / kMinutesPerHour);  elapsed -= desc->hours  * kMinutesPerHour;
    desc->minutes = elapsed;

}

//============================================================================
dword TimeGetSecondsSince2001Utc () {
	qword time    = TimeGetTime();
	dword seconds = (dword)((time - kTime1601To2001) / kTimeIntervalsPerSecond);
	return seconds;
}

//============================================================================
dword TimeGetSecondsSince1970Utc () {
	qword time    = TimeGetTime();
	dword seconds = (dword)((time  - kTime1601To1970) / kTimeIntervalsPerSecond);
	return seconds;
}
