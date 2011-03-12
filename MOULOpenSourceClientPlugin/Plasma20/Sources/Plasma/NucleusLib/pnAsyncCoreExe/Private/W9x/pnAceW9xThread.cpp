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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCoreExe/Private/W9x/pnAceW9xThread.cpp
*   
***/

#include "../../Pch.h"
#pragma hdrstop

#include "pnAceW9xInt.h"


namespace W9x {

/*****************************************************************************
*
*   Private data
*
***/

const unsigned kThreadCount = 2;

static CCritSect s_critSect;
static bool      s_destroying;
static HANDLE    s_destroyEvent;
static unsigned  s_sequence;
static HANDLE    s_shutdownEvent;
static HANDLE    s_signalEvent;
static HANDLE    s_thread[kThreadCount];


/****************************************************************************
*
*   ThreadWaitRec
*
***/

struct ThreadWaitRec {
    HANDLE              event;
    LINK(ThreadWaitRec) link;
};


/****************************************************************************
*
*   CThreadDispRec
*
***/

class CThreadDispRec {
private:
    CThreadDispObject *           m_object;
    void *                        m_op;
    AsyncId                          m_ioId;
    LISTDECL(ThreadWaitRec, link) m_waitList;

public:
    LINK(CThreadDispRec)          m_link;

    CThreadDispRec (
        CThreadDispObject * object,
        void *              op,
        AsyncId *              asyncId
    );
    ~CThreadDispRec ();

    void Complete (CCritSect * critSect);
    AsyncId GetId () const { return m_ioId; }
    void LinkWait (ThreadWaitRec * wait);

};
    
static LISTDECL(CThreadDispRec, m_link) s_dispList;
static LISTDECL(CThreadDispRec, m_link) s_dispInProcList;

//===========================================================================
CThreadDispRec::CThreadDispRec (
    CThreadDispObject * object,
    void *              op,
    AsyncId *              asyncId
) :
    m_object(object),
    m_op(op)
{
    s_critSect.Enter();

    // Verify that this module is not being destroyed
    ASSERT(!s_destroying);

    // Increment the owning object's reference count
    object->IncRef();

    // Assign an id
    m_ioId = (AsyncId)++s_sequence;
    if (!m_ioId)
        m_ioId = (AsyncId)++s_sequence;
    *asyncId = m_ioId;

    // Link this record to the dispatch list
    s_dispList.Link(this);

    s_critSect.Leave();
}

//===========================================================================
CThreadDispRec::~CThreadDispRec () {

    // Delete the operation data
    CThreadDispObject * object = m_object;
    object->Delete(m_op);

    s_critSect.Enter();

    // Unlink this record
    m_link.Unlink();

    // Wake up all threads blocking on this operation. We must unlink each
    // wait record before we signal it.
    for (ThreadWaitRec * rec; (rec = m_waitList.Head()) != nil; ) {
        m_waitList.Unlink(rec);
        SetEvent(rec->event);
    }

    // Decrement the owning object's reference count
    object->DecRef();

    s_critSect.Leave();

}

//===========================================================================
void CThreadDispRec::Complete (CCritSect * critSect) {
    m_object->Complete(m_op, critSect, m_ioId);
}

//===========================================================================
void CThreadDispRec::LinkWait (ThreadWaitRec * wait) {

    // The caller should have already claimed the critical section before
    // calling this function

    m_waitList.Link(wait);

}


/****************************************************************************
*
*   CThreadDispObject
*
***/

//===========================================================================
CThreadDispObject::CThreadDispObject () {
    IncRef();
}

//===========================================================================
void CThreadDispObject::Close () {
    s_critSect.Enter();
    DecRef();
    s_critSect.Leave();
}

//===========================================================================
AsyncId CThreadDispObject::Queue (void * op) {
    AsyncId asyncId = 0;
    NEW(CThreadDispRec)(this, op, &asyncId);
    SetEvent(s_signalEvent);
    return asyncId;
}


/****************************************************************************
*
*   Thread procedure
*
***/

//===========================================================================
static unsigned CALLBACK W9xThreadProc (AsyncThread *) {

    // Perform the main thread loop
    for (;;) {
        unsigned timeout = (unsigned)-1;

        // If an operation is queued, complete it and dispatch a notification.
        // The code that processes the operation is responsible for leaving
        // our critical section. This ensures that operations are completed
        // in order.
        s_critSect.Enter();
        CThreadDispRec * rec = s_dispList.Head();
        if (rec) {
            s_dispInProcList.Link(rec);
            rec->Complete(&s_critSect);
            DEL(rec);
            timeout = 0;
        }
        else {
            s_critSect.Leave();
        }


        // Consume events, check for destruction, and block if we have
        // nothing to do.
        HANDLE events[] = {s_destroyEvent, s_signalEvent};
        dword  result   = WaitForMultipleObjects(
            arrsize(events),
            events,
            FALSE,
            INFINITE
        );
        if (result == WAIT_OBJECT_0)
            return 0;
    }
}


/****************************************************************************
*
*   Exported functions
*
***/

//===========================================================================
void W9xThreadDestroy (
    unsigned exitThreadWaitMs
) {

    // Wait until all outstanding I/O is complete. We allow new I/O 
    // operations to be queued while old ones are completing.
    s_critSect.Enter();
    while (s_dispList.Head() || s_dispInProcList.Head()) {
        s_critSect.Leave();
        Sleep(10);
        s_critSect.Enter();
    }

    // Once all I/O operations are complete, we disallow any future
    // I/O operations from being queued.
    s_destroying = true;
    s_critSect.Leave();

    // Signal thread destruction
    if (s_destroyEvent)
        SetEvent(s_destroyEvent);

    // Wait for thread destruction
    for (unsigned thread = kThreadCount; thread--; )
        if (s_thread[thread]) {

            // Wait for the thread to terminate
            WaitForSingleObject(s_thread[thread], exitThreadWaitMs);

            // Close the thread handle
            CloseHandle(s_thread[thread]);
            s_thread[thread] = 0;

        }

    // Destroy internal modules
    W9xSocketDestroy();

    // Destroy events
    if (s_destroyEvent) {
        CloseHandle(s_destroyEvent);
        s_destroyEvent = 0;
    }
    if (s_shutdownEvent) {
        CloseHandle(s_shutdownEvent);
        s_shutdownEvent = 0;
    }
    if (s_signalEvent) {
        CloseHandle(s_signalEvent);
        s_signalEvent = 0;
    }

}

//===========================================================================
void W9xThreadInitialize () {

    // Reset static variables
    s_destroying = false;

    // Create a manual reset event to use for signaling thread destruction
    if (s_destroyEvent)
        ResetEvent(s_destroyEvent);
    else {
        s_destroyEvent = CreateEvent(nil, TRUE, FALSE, nil);
        ASSERT(s_destroyEvent);
    }

    // Create an auto-reset event to use for signaling the thread to process
    // notifications
    if (!s_signalEvent) {
        s_signalEvent = CreateEvent(nil, FALSE, FALSE, nil);
        ASSERT(s_signalEvent);
    }

    // Create a manual reset event to use for signaling application shutdown
    if (s_shutdownEvent)
        ResetEvent(s_shutdownEvent);
    else {
        s_shutdownEvent = CreateEvent(nil, TRUE, FALSE, nil);
        ASSERT(s_shutdownEvent);
    }

    // Create threads
    for (unsigned thread = 0; thread < kThreadCount; ++thread) {
        if (!s_thread[thread]) {
            s_thread[thread] = (HANDLE) AsyncThreadCreate(
                W9xThreadProc,
                (void *) thread,
                L"W9xWorkerThread"
            );
        }
    }

}

//===========================================================================
void W9xThreadSignalShutdown () {
    SetEvent(s_shutdownEvent);
}

//===========================================================================
void W9xThreadSleep (
    unsigned sleepMs
) {
    Sleep(sleepMs);
}

//===========================================================================
void W9xThreadWaitForShutdown () {

    // We know that the applicaton is finished initializing at this point.
    // While it was still initializing, it may have returned an infinite
    // sleep time from the idle procedure, which would prevent us from ever
    // calling it again. Therefore, we trigger an idle callback here.
    SetEvent(s_signalEvent);

    // Wait for the application to signal shutdown
    WaitForSingleObject(s_shutdownEvent, INFINITE);

}

//===========================================================================
bool W9xThreadWaitId (
    AsyncFile   file, 
    AsyncId     asyncId, 
    unsigned    timeoutMs
) {
    ref(file);

    // Find a pending I/O operation with the given id
    s_critSect.Enter();
    CThreadDispRec * disp;
    for (disp = s_dispList.Head(); disp && (disp->GetId() != asyncId); disp = s_dispList.Next(disp))
        ;
    if (!disp)
        for (disp = s_dispInProcList.Head(); disp && (disp->GetId() != asyncId); disp = s_dispInProcList.Next(disp))
            ;

    // If we couldn't find the given id, the operation must have already
    // completed, so return true.
    if (!disp) {
        s_critSect.Leave();
        return true;
    }

    // The operation has not completed. If the timeout is zero, return 
    // false.
    if (!timeoutMs) {
        s_critSect.Leave();
        return false;
    }

    // Create a wait event
    HANDLE event = CreateEvent(nil, FALSE, FALSE, nil);

    // Create a wait record and link it to the I/O operation
    ThreadWaitRec wait;
    wait.event = event;
    disp->LinkWait(&wait);
    s_critSect.Leave();

    // Wait for the operation to complete
    DWORD result = WaitForSingleObject(event, timeoutMs);

    // If the operation completed then the dispatcher unlinked our wait 
    // record before signaling it. We can simply free the event and return.
    if (result == WAIT_OBJECT_0) {
        CloseHandle(event);
        return true;
    }

    // Unlink our wait record from the I/O operation
    s_critSect.Enter();
    wait.link.Unlink();
    s_critSect.Leave();

    // Free the event
    CloseHandle(event);

    // Return false, because the operation did not complete during the
    // timeout period
    return false;

}

}  // namespace W9x
