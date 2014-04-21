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

struct AsyncTimer::P {
    struct Thread;
    static Thread *     s_thread;
    static hsMutex      lock;

    PRIORITY_TIME(P)    priority;
    FProc               proc;
    void *              param;
    LINK(P)             deleteLink;
    bool                doDelete;
    hsSemaphore         deleteEvent;

    static unsigned Run ();
    
    void Update (unsigned timeMs);
    void UpdateIfHigher (unsigned timeMs);
};

//===========================================================================
struct AsyncTimer::P::Thread : hsThread {
    hsSemaphore                             event;
    PRIQDECL(P, PRIORITY_TIME(P), priority) procsList;
    LISTDECL(P, deleteLink)                 deleteList;
    
    hsError Run();
    using hsThread::SetQuit;
};
AsyncTimer::P::Thread *     AsyncTimer::P::s_thread = nullptr;
hsMutex                     AsyncTimer::P::lock;


/****************************************************************************
*
*   Timer implementation
*
***/

//===========================================================================
AsyncTimer::~AsyncTimer() { delete p; }

//===========================================================================
// inline because it is called only once
inline unsigned AsyncTimer::P::Run () {
    unsigned currTimeMs = TimeGetMs();
    for (;;) {
        FProc proc;
        AsyncTimer::P * t;
        { hsTempMutexLock lock_(lock);
            
            // Delete old timers
            while (t = s_thread->deleteList.Head()) {
                if (!t->doDelete)
                    t->deleteEvent.Signal();
                else {
                    if (t->proc) {
                        lock.Unlock();
                        t->proc(t->param);
                        lock.Lock();
                    }
                    delete t;
                }
            }

            // Get first timer to run
            t = s_thread->procsList.Root();
            if (!t)
                return kPosInfinity32;

            // If it isn't time to run this timer then exit
            if (t->priority.Get() > currTimeMs)
                return t->priority.Get() - currTimeMs;
            
            proc = t->proc;
        }

        unsigned sleepMs = proc(t->param);

        // Note if return is kPosInfinity32, we do not remove the timer
        // from the queue.  Some users depend on the fact that they can
        // call AsyncTimerUpdate and not get overridden by a return from the
        // handler at the same time.

        // timer update
        currTimeMs = TimeGetMs();
        if (sleepMs != kPosInfinity32)
            t->priority.Set(sleepMs + currTimeMs);
        else {
            hsTempMutexLock lock(lock);
            s_thread->procsList.Dequeue();
        }
    }
}

//===========================================================================
// inline because it is called only once
inline void AsyncTimer::P::Update (unsigned timeMs) {
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
inline void AsyncTimer::P::UpdateIfHigher (unsigned timeMs) {
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
hsError AsyncTimer::P::Thread::Run () {
    do {
        const unsigned sleepMs = P::Run();

        if (s_thread->event.Wait(sleepMs))
            while (s_thread->event.TryWait()); // binary semaphore emultation
    } while (!s_thread->GetQuit());
    return 0;
}


/****************************************************************************
*
*   Module functions
*
***/

//===========================================================================
void TimerDestroy (unsigned exitThreadWaitMs) {
    ASSERT(AsyncTimer::P::s_thread);

    AsyncTimer::P::s_thread->SetQuit(true);
    AsyncTimer::P::s_thread->event.Signal();
    //WaitForSingleObject(s_timerThread, exitThreadWaitMs);
    AsyncTimer::P::s_thread->Stop();

    // Cleanup any timers that have been stopped but not deleted
    { hsTempMutexLock lock(AsyncTimer::P::lock);
        while (AsyncTimer::P * t = AsyncTimer::P::s_thread->deleteList.Head()) {
            if (!t->doDelete)
                t->deleteEvent.Signal();
            else {
                if (t->proc) {
                    AsyncTimer::P::lock.Unlock();
                    t->proc(t->param);
                    AsyncTimer::P::lock.Lock();
                }
                delete t;
            }
        }
    }

    if (AsyncTimer::P * timer = AsyncTimer::P::s_thread->procsList.Root())
        ErrorAssert(__LINE__, __FILE__, "TimerProc not destroyed: %p", timer->proc);

    delete AsyncTimer::P::s_thread;
    AsyncTimer::P::s_thread = nullptr;
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
    ASSERT(!p);
    ASSERT(timerProc);

    // Allocate timer outside critical section
    p               = new P;
    p->proc         = timerProc;
    p->param        = param;
    p->priority.Set(TimeGetMs() + callbackMs);

    bool setEvent;
    if (!P::s_thread) {
        P::s_thread = new P::Thread();
        P::s_thread->Start();
    }
    
    { hsTempMutexLock lock(P::lock);
        // Does this timer need to be queued?
        if (callbackMs != kPosInfinity32)
            P::s_thread->procsList.Enqueue(p);

        // Does the timer thread need to be awakened?
        setEvent = p == P::s_thread->procsList.Root();
    }

    if (setEvent)
        P::s_thread->event.Signal();
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
    ASSERT(p);
    ASSERT(!p->deleteLink.IsLinked());

    p->doDelete = true;
    // Link the timer to the deletion list
    { hsTempMutexLock lock(P::lock);
        p->proc = destroyProc;
        P::s_thread->deleteList.Link(p);
    }

    // Force the timer thread to wake up and perform the deletion
    if (destroyProc)
        P::s_thread->event.Signal();
    p = nullptr;
}

//===========================================================================
void AsyncTimer::DeleteAndWait () {
    // If the timer has already been destroyed then exit
    ASSERT(p);

    p->doDelete = false;
    // Link the timer to the deletion list
    { hsTempMutexLock lock(P::lock);
        p->proc = nullptr;
        P::s_thread->deleteList.Link(p);
    }

    
    // Force the timer thread to wake up and perform the deletion
    P::s_thread->event.Signal();
    // Wait until the timer procedure completes
    p->deleteEvent.Wait();

    delete p;
    p = nullptr;
}

//===========================================================================
// To set the time value for a timer, use this function with flags = 0.
// To set the time to MoreRecentOf(nextTimerCallbackMs, callbackMs), use SETPRIORITYHIGHER
void AsyncTimer::Set (unsigned callbackMs) {
    ASSERT(p);

    bool setEvent;
    { hsTempMutexLock lock(P::lock);
        if (callbackMs != kPosInfinity32) {
            p->Update(callbackMs + TimeGetMs());
            setEvent = p == P::s_thread->procsList.Root();
        }
        else
            setEvent = false;
    }

    if (setEvent)
        P::s_thread->event.Signal();
}

//===========================================================================
void AsyncTimer::SetIfHigher (unsigned callbackMs) {
    ASSERT(p);

    bool setEvent;
    { hsTempMutexLock lock(P::lock);
        if (callbackMs != kPosInfinity32) {
            p->UpdateIfHigher(callbackMs + TimeGetMs());
            setEvent = p == P::s_thread->procsList.Root();
        }
        else {
            p->priority.Unlink();
            setEvent = false;
        }
    }
    
    if (setEvent)
        P::s_thread->event.Signal();
}
