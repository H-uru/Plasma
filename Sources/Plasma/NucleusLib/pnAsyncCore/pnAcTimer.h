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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCore/pnAcTimer.h
*   
***/

#ifndef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNASYNCCORE_PNACTIMER_H
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNASYNCCORE_PNACTIMER_H

#include "pnUtils/pnUtils.h"

/*****************************************************************************
*
*   Timer functions
*
*   Timers are repeatedly called back at a scheduled interval. Not that all
*   timer procedures share the same thread, so timer procedures should:
*
*   1) Not be called too frequently
*   2) Not take too long to run or block for a long time
*
***/

struct AsyncTimer;

// Return callbackMs to wait that long until next callback.
// Return kPosInfinity32 to stop callbacks (note: does not destroy Timer structure)
typedef unsigned (* FAsyncTimerProc)(void * param);

// 1) Timer procs do not get starved by I/O, they are called periodically.
// 2) Timer procs will never be called by multiple threads simultaneously.
void AsyncTimerCreate (
    AsyncTimer **   timer,
    FAsyncTimerProc timerProc, 
    unsigned        callbackMs,
    void *          param = nil
);

// Timer procs can be in the process of getting called in
// another thread during the unregister function -- be careful!
// -- waitComplete = will wait until the timer has been unregistered and is
//    no longer in the process of being called before returning. The flag may only
//    be set by init/destruct threads, not I/O worker threads. In addition, extreme
//    care should be used to avoid a deadlock when this flag is set; in general, it
//    is a good idea not to hold any locks or critical sections when setting the flag.
const unsigned kAsyncTimerDestroyWaitComplete = 1<<0;
void AsyncTimerDelete (
    AsyncTimer *    timer,
    unsigned        flags = 0
);
void AsyncTimerDeleteCallback (
    AsyncTimer *    timer,
    FAsyncTimerProc destroyProc
);

// To set the time value for a timer, use this function with flags = 0.
// To set the time to MoreRecentOf(nextTimerCallbackMs, callbackMs), use SetPriorityHigher
const unsigned kAsyncTimerUpdateSetPriorityHigher = 1<<0;
void AsyncTimerUpdate (
    AsyncTimer *    timer,
    unsigned        callbackMs,
    unsigned        flags = 0
);

#endif

