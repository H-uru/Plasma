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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCore/pnAcTimer.cpp
*   
***/

#include "pnAcTimer.h"
#include "hsThread.h"
#include "pnUtils/pnUtils.h"
#pragma hdrstop


/****************************************************************************
*
*   Private
*
***/

struct AsyncTimer::Private {
    struct Thread;
    static Thread *     s_thread;
    static std::mutex   mutex;

    PRIORITY_TIME(Private)  priority;
    FProc                   proc;
    void *                  param;
    LINK(Private)           deleteLink;
    bool                    doDelete;
    hsSemaphore             deleteEvent;

    static unsigned Run ();
    
    void Update (unsigned timeMs);
    void UpdateIfHigher (unsigned timeMs);
};

//===========================================================================
struct AsyncTimer::Private::Thread : hsThread {
    hsBinarySemaphore                                   event;
    PRIQDECL(Private, PRIORITY_TIME(Private), priority) procsList;
    LISTDECL(Private, deleteLink)                       deleteList;
    
    void Run();
    using hsThread::SetQuit;
};
AsyncTimer::Private::Thread *   AsyncTimer::Private::s_thread = nullptr;
std::mutex                      AsyncTimer::Private::mutex;


/****************************************************************************
*
*   Timer implementation
*
***/

//===========================================================================
AsyncTimer::~AsyncTimer() { delete timer; }

//===========================================================================
// inline because it is called only once
inline unsigned AsyncTimer::Private::Run () {
    unsigned currTimeMs = TimeGetMs();
    for (;;) {
        FProc proc;
        AsyncTimer::Private * timer;
        {
            std::unique_lock<std::mutex> lock(mutex);
            
            // Delete old timers
            while (timer = s_thread->deleteList.Head()) {
                if (!timer->doDelete)
                    timer->deleteEvent.Signal();
                else {
                    if (timer->proc) {
                        lock.unlock();
                        timer->proc(timer->param);
                        lock.lock();
                    }
                    delete timer;
                }
            }

            // Get first timer to run
            timer = s_thread->procsList.Root();
            if (!timer)
                return kPosInfinity32;

            // If it isn't time to run this timer then exit
            if (timer->priority.Get() > currTimeMs)
                return timer->priority.Get() - currTimeMs;
            
            proc = timer->proc;
        }

        unsigned sleepMs = proc(timer->param);

        // Note if return is kPosInfinity32, we do not remove the timer
        // from the queue.  Some users depend on the fact that they can
        // call AsyncTimerUpdate and not get overridden by a return from the
        // handler at the same time.

        // timer update
        currTimeMs = TimeGetMs();
        if (sleepMs != kPosInfinity32)
            timer->priority.Set(sleepMs + currTimeMs);
        else {
            std::lock_guard<std::mutex> lock(mutex);
            s_thread->procsList.Dequeue();
        }
    }
}

//===========================================================================
// inline because it is called only once
inline void AsyncTimer::Private::Update (unsigned timeMs) {
    // If the timer isn't already linked then it doesn't
    // matter whether kAsyncTimerUpdateSetPriorityHigher is
    // set; just add the timer to the queue
    if (!priority.IsLinked()) {
        priority.Set(timeMs);
        s_thread->procsList.Enqueue(this);
    }
    else
        priority.Set(timeMs);
}

//===========================================================================
// inline because it is called only once
inline void AsyncTimer::Private::UpdateIfHigher (unsigned timeMs) {
    // If the timer isn't already linked then it doesn't
    // matter whether kAsyncTimerUpdateSetPriorityHigher is
    // set; just add the timer to the queue
    if (!priority.IsLinked()) {
        priority.Set(timeMs);
        s_thread->procsList.Enqueue(this);
    }
    else if (!priority.IsPriorityHigher(timeMs))
        priority.Set(timeMs);
}

//===========================================================================
void AsyncTimer::Private::Thread::Run() {
    do {
        const unsigned sleepMs = Private::Run();

        s_thread->event.Wait(std::chrono::milliseconds(sleepMs));
    } while (!s_thread->GetQuit());
}


/****************************************************************************
*
*   Module functions
*
***/

//===========================================================================
void TimerDestroy (unsigned exitThreadWaitMs) {
    if (!AsyncTimer::Private::s_thread)
        return;

    AsyncTimer::Private::s_thread->SetQuit(true);
    AsyncTimer::Private::s_thread->event.Signal();
    //WaitForSingleObject(s_timerThread, exitThreadWaitMs);
    AsyncTimer::Private::s_thread->Stop();

    // Cleanup any timers that have been stopped but not deleted
    {
        std::unique_lock<std::mutex> lock(AsyncTimer::Private::mutex);
        while (AsyncTimer::Private * timer = AsyncTimer::Private::s_thread->deleteList.Head()) {
            if (!timer->doDelete)
                timer->deleteEvent.Signal();
            else {
                if (timer->proc) {
                    lock.unlock();
                    timer->proc(timer->param);
                    lock.lock();
                }
                delete timer;
            }
        }
    }

    if (AsyncTimer::Private * timer = AsyncTimer::Private::s_thread->procsList.Root())
        ErrorAssert(__LINE__, __FILE__, "TimerProc not destroyed: %p", timer->proc);

    delete AsyncTimer::Private::s_thread;
    AsyncTimer::Private::s_thread = nullptr;
}


/****************************************************************************
*
*   Exported functions
*
***/

//===========================================================================
// 1. Timer procs do not get starved by I/O, they are called periodically.
// 2. Timer procs will never be called by multiple threads simultaneously.
void AsyncTimer::Create (
    FProc           timerProc, 
    unsigned        callbackMs,
    void *          param
) {
    ASSERT(!timer);
    ASSERT(timerProc);

    // Allocate timer outside critical section
    timer               = new Private;
    timer->proc         = timerProc;
    timer->param        = param;
    timer->priority.Set(TimeGetMs() + callbackMs);

    bool setEvent;
    if (!Private::s_thread) {
        Private::s_thread = new Private::Thread();
        Private::s_thread->Start();
    }
    
    {
        std::lock_guard<std::mutex> lock(Private::mutex);
        // Does this timer need to be queued?
        if (callbackMs != kPosInfinity32)
            Private::s_thread->procsList.Enqueue(timer);

        // Does the timer thread need to be awakened?
        setEvent = timer == Private::s_thread->procsList.Root();
    }

    if (setEvent)
        Private::s_thread->event.Signal();
}

//===========================================================================
// Timer procs can be in the process of getting called in
// another thread during the unregister function -- be careful!
// -- waitComplete = will wait until the timer has been unregistered and is
//    no longer in the process of being called before returning. The flag may only
//    be set by init/destruct threads, not I/O worker threads. In addition, extreme
//    care should be used to avoid a deadlock when this flag is set; in general, it
//    is a good idea not to hold any locks or critical sections when setting the flag.
void AsyncTimer::Delete (FProc destroyProc) {
    // If the timer has already been destroyed then exit
    ASSERT(timer);
    ASSERT(!timer->deleteLink.IsLinked());

    timer->doDelete = true;
    // Link the timer to the deletion list
    {
        std::lock_guard<std::mutex> lock(Private::mutex);
        timer->proc = destroyProc;
        Private::s_thread->deleteList.Link(timer);
    }

    // Force the timer thread to wake up and perform the deletion
    if (destroyProc)
        Private::s_thread->event.Signal();
    timer = nullptr;
}

//===========================================================================
void AsyncTimer::DeleteAndWait () {
    // If the timer has already been destroyed then exit
    ASSERT(timer);

    timer->doDelete = false;
    // Link the timer to the deletion list
    {
        std::lock_guard<std::mutex> lock(Private::mutex);
        timer->proc = nullptr;
        Private::s_thread->deleteList.Link(timer);
    }

    
    // Force the timer thread to wake up and perform the deletion
    Private::s_thread->event.Signal();
    // Wait until the timer procedure completes
    timer->deleteEvent.Wait();

    delete timer;
    timer = nullptr;
}

//===========================================================================
// To set the time value for a timer, use this function with flags = 0.
// To set the time to MoreRecentOf(nextTimerCallbackMs, callbackMs), use SETPRIORITYHIGHER
void AsyncTimer::Set (unsigned callbackMs) {
    ASSERT(timer);

    bool setEvent;
    {
        std::lock_guard<std::mutex> lock(Private::mutex);
        if (callbackMs != kPosInfinity32) {
            timer->Update(callbackMs + TimeGetMs());
            setEvent = timer == Private::s_thread->procsList.Root();
        }
        else
            setEvent = false;
    }

    if (setEvent)
        Private::s_thread->event.Signal();
}

//===========================================================================
void AsyncTimer::SetIfHigher (unsigned callbackMs) {
    ASSERT(p);

    bool setEvent;
    {
        std::lock_guard<std::mutex> lock(Private::mutex);
        if (callbackMs != kPosInfinity32) {
            timer->UpdateIfHigher(callbackMs + TimeGetMs());
            setEvent = timer == Private::s_thread->procsList.Root();
        }
        else {
            timer->priority.Unlink();
            setEvent = false;
        }
    }
    
    if (setEvent)
        Private::s_thread->event.Signal();
}
