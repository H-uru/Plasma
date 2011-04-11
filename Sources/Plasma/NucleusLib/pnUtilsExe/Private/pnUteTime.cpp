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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtilsExe/Private/pnUteTime.cpp
*   
***/

#include "../Pch.h"
#pragma hdrstop


namespace pnUtilsExe {

/*****************************************************************************
*
*   Time adjustment functions
*
*   For debug builds, adjust the initial time value so that the high
*   bit or the time value itself is about to wrap, to catch application
*   bugs that don't handle wrapping or depend on the high bit's value.
*
***/

static dword s_adjustment;

//===========================================================================
static void InitializeAdjustment () {
    ASSERT(!s_adjustment);
    dword currTime  = TimeGetTickCount();
    dword startBits = (currTime & 0x80) ? 0x7fff0000 : 0xffff0000;
    dword startMask = 0xffff0000;
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

//============================================================================
dword TimeGetMs () {
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

