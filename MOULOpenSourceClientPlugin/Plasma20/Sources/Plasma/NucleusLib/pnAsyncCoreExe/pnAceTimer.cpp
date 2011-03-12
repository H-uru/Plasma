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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCoreExe/pnAceTimer.cpp
*   
***/

#include "Pch.h"
#pragma hdrstop


/****************************************************************************
*
*   Private
*
***/

// timer callbacks
struct AsyncTimer {
    PRIORITY_TIME(AsyncTimer)   priority;
    FAsyncTimerProc             timerProc;
    FAsyncTimerProc             destroyProc;
    void *                      param;
    LINK(AsyncTimer)            deleteLink;
};

static CCritSect            s_timerCrit;
static FAsyncTimerProc      s_timerCurr;
static HANDLE               s_timerThread;
static HANDLE               s_timerEvent;
static bool                 s_running;

static PRIQDECL(
    AsyncTimer,
    PRIORITY_TIME(AsyncTimer),
    priority
) s_timerProcs;

static LISTDECL(
    AsyncTimer,
    deleteLink
) s_timerDelete;


/****************************************************************************
*
*   Timer implementation
*
***/

//===========================================================================
static void UpdateTimer (
    AsyncTimer *    timer,
    unsigned        timeMs,
    unsigned        flags
) {
    // If the timer isn't already linked then it doesn't
    // matter whether kAsyncTimerUpdateSetPriorityHigher is
    // set; just add the timer to the queue
    if (!timer->priority.IsLinked()) {
        timer->priority.Set(timeMs);
        s_timerProcs.Enqueue(timer);
    }
    else if (((flags & kAsyncTimerUpdateSetPriorityHigher) == 0)
    || !timer->priority.IsPriorityHigher(timeMs)
    ) {
        timer->priority.Set(timeMs);
    }
}

//===========================================================================
static unsigned CallTimerProc (AsyncTimer * t, FAsyncTimerProc timerProc) {
    // Cache parameters to make timer callback outside critical section
    s_timerCurr = timerProc;

    // Leave critical section to make timer callback
    s_timerCrit.Leave();

    unsigned sleepMs = s_timerCurr(t->param);
    s_timerCurr = nil;

    s_timerCrit.Enter();

    return sleepMs;
}

//===========================================================================
// inline because it is called only once
static inline unsigned RunTimers () {
    unsigned currTimeMs = TimeGetMs();
    for (;;) {
        // Delete old timers
        while (AsyncTimer * t = s_timerDelete.Head()) {
            if (t->destroyProc)
                CallTimerProc(t, t->destroyProc);
            DEL(t);
        }

        // Get first timer to run
        AsyncTimer * t = s_timerProcs.Root();
        if (!t)
            return INFINITE;

        // If it isn't time to run this timer then exit
        unsigned sleepMs;
        if (0 < (signed) (sleepMs = (unsigned) t->priority.Get() - currTimeMs))
            return sleepMs;

        // Remove from timer queue and call timer
        s_timerProcs.Dequeue();
        sleepMs = CallTimerProc(t, t->timerProc);

        // Note if return is kAsyncTimeInfinite, we do not remove the timer
        // from the queue.  Some users depend on the fact that they can
        // call AsyncTimerUpdate and not get overridden by a return from the
        // handler at the same time.

        // Requeue timer
        currTimeMs = TimeGetMs();
        if (sleepMs != kAsyncTimeInfinite)
            UpdateTimer(t, sleepMs + currTimeMs, kAsyncTimerUpdateSetPriorityHigher);
    }
}

//===========================================================================
static unsigned THREADCALL TimerThreadProc (AsyncThread *) {
    do {
        s_timerCrit.Enter();
        const unsigned sleepMs = RunTimers();
        s_timerCrit.Leave();

        WaitForSingleObject(s_timerEvent, sleepMs);
    } while (s_running);
    return 0;
}

//===========================================================================
// inline because it is called only once
static inline void InitializeTimer () {
    if (!s_timerThread) {
        s_running = true;

        s_timerEvent = CreateEvent(
            (LPSECURITY_ATTRIBUTES) nil,
            false,                  // auto-reset event
            false,                  // initial state = off
            (LPCTSTR) nil
        );
        if (!s_timerEvent)
            ErrorFatal(__LINE__, __FILE__, "CreateEvent %u", GetLastError());

        s_timerThread = (HANDLE) AsyncThreadCreate(
            TimerThreadProc,
            nil,
            L"AsyncTimerThread"
        );
    }
}


/****************************************************************************
*
*   Module functions
*
***/

//===========================================================================
void TimerDestroy (unsigned exitThreadWaitMs) {
    s_running = false;

    if (s_timerThread) {
        SetEvent(s_timerEvent);
        WaitForSingleObject(s_timerThread, exitThreadWaitMs);
        CloseHandle(s_timerThread);
        s_timerThread = nil;
    }

    if (s_timerEvent) {
        CloseHandle(s_timerEvent);
        s_timerEvent = nil;
    }

    // Cleanup any timers that have been stopped but not deleted
    s_timerCrit.Enter();
    while (AsyncTimer * t = s_timerDelete.Head()) {
        if (t->destroyProc)
            CallTimerProc(t, t->destroyProc);
        DEL(t);
    }
    s_timerCrit.Leave();

    if (AsyncTimer * timer = s_timerProcs.Root())
        ErrorFatal(__LINE__, __FILE__, "TimerProc not destroyed: %p", timer->timerProc);
}


/****************************************************************************
*
*   Exported functions
*
***/

//===========================================================================
// 1. Timer procs do not get starved by I/O, they are called periodically.
// 2. Timer procs will never be called by multiple threads simultaneously.
void AsyncTimerCreate (
    AsyncTimer **   timer,
    FAsyncTimerProc timerProc, 
    unsigned        callbackMs,
    void *          param
) {
    ASSERT(timer);
    ASSERT(timerProc);

    // Allocate timer outside critical section
    AsyncTimer * t  = NEW(AsyncTimer);
    t->timerProc    = timerProc;
    t->destroyProc  = nil;
    t->param        = param;
    t->priority.Set(TimeGetMs() + callbackMs);

    // Set result pointer before queueing timer
    // so that the value is set before a callback
    *timer = t;

    bool setEvent;
    s_timerCrit.Enter();
    {
        InitializeTimer();

        // Does this timer need to be queued?
        if (callbackMs != kAsyncTimeInfinite)
            s_timerProcs.Enqueue(t);

        // Does the timer thread need to be awakened?
        setEvent = t == s_timerProcs.Root();
    }
    s_timerCrit.Leave();

    if (setEvent)
        SetEvent(s_timerEvent);
}

//===========================================================================
// Timer procs can be in the process of getting called in
// another thread during the unregister function -- be careful!
// -- waitComplete = will wait until the timer has been unregistered and is
//    no longer in the process of being called before returning. The flag may only
//    be set by init/destruct threads, not I/O worker threads. In addition, extreme
//    care should be used to avoid a deadlock when this flag is set; in general, it
//    is a good idea not to hold any locks or critical sections when setting the flag.
void AsyncTimerDelete (
    AsyncTimer *    timer,
    unsigned        flags
) {
    // If the timer has already been destroyed then exit
    ASSERT(timer);

    // Wait for timer before exiting function?
    FAsyncTimerProc timerProc;
    if (flags & kAsyncTimerDestroyWaitComplete)
        timerProc = timer->timerProc;
    else
        timerProc = nil;

    AsyncTimerDeleteCallback(timer, nil);

    // Wait until the timer procedure completes
    if (timerProc) {
        // ensure that I/O worker threads don't call this function with waitComplete=true
        // to prevent a possible deadlock of a timer callback waiting for itself to complete
        ThreadAssertCanBlock(__FILE__, __LINE__);

        while (s_timerCurr == timerProc)
            Sleep(1);
    }
}

//===========================================================================
void AsyncTimerDeleteCallback (
    AsyncTimer *    timer,
    FAsyncTimerProc destroyProc
) {
    // If the timer has already been destroyed then exit
    ASSERT(timer);
    ASSERT(!timer->deleteLink.IsLinked());

    // Link the timer to the deletion list
    s_timerCrit.Enter();
    {
        timer->destroyProc = destroyProc;
        s_timerDelete.Link(timer);
    }
    s_timerCrit.Leave();

    // Force the timer thread to wake up and perform the deletion
    if (destroyProc)
        SetEvent(s_timerEvent);
}

//===========================================================================
// To set the time value for a timer, use this function with flags = 0.
// To set the time to MoreRecentOf(nextTimerCallbackMs, callbackMs), use SETPRIORITYHIGHER
void AsyncTimerUpdate (
    AsyncTimer *    timer,
    unsigned        callbackMs,
    unsigned        flags
) {
    ASSERT(timer);

    bool setEvent;
    s_timerCrit.Enter();
    {
        if (callbackMs != kAsyncTimeInfinite) {
            UpdateTimer(timer, callbackMs + TimeGetMs(), flags);
            setEvent = timer == s_timerProcs.Root();
        }
        else {
            if ((flags & kAsyncTimerUpdateSetPriorityHigher) == 0)
                timer->priority.Unlink();
            setEvent = false;
        }
    }
    s_timerCrit.Leave();

    if (setEvent)
        SetEvent(s_timerEvent);
}
