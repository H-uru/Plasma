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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCoreExe/Private/Nt/pnAceNt.cpp
*   
***/

#include "../../Pch.h"
#pragma hdrstop

#include "pnAceNtInt.h"


namespace Nt {

/****************************************************************************
*
*   Private data
*
***/

// Use non-allocated arrays for worker threads since they're used so frequently.
const unsigned kMaxWorkerThreads = 32;  // handles 8-processor computer w/hyperthreading

static bool                     s_running;
static HANDLE                   s_waitEvent;

static long                     s_ioThreadCount;
static HANDLE                   s_ioThreadHandles[kMaxWorkerThreads];

static HANDLE                   s_ioPort;
static unsigned                 s_pageSizeMask;


/****************************************************************************
*
*   Waitable event handles
*
***/

//===========================================================================
CNtWaitHandle::CNtWaitHandle () {
    m_refCount = 1;
    m_event = CreateEvent(
        (LPSECURITY_ATTRIBUTES) nil,
        true,   // manual reset
        false,  // initial state
        (LPCTSTR) nil
    );
}

//===========================================================================
CNtWaitHandle::~CNtWaitHandle () {
    CloseHandle(m_event);
}

//===========================================================================
void CNtWaitHandle::IncRef () {
    InterlockedIncrement(&m_refCount);
}

//===========================================================================
void CNtWaitHandle::DecRef () {
    if (!InterlockedDecrement(&m_refCount))
        DEL(this);
}

//===========================================================================
bool CNtWaitHandle::WaitForObject (unsigned timeMs) const {
    return WAIT_TIMEOUT != WaitForSingleObject(m_event, timeMs);
}

//===========================================================================
void CNtWaitHandle::SignalObject () const {
    SetEvent(m_event);
}


/****************************************************************************
*
*   OPERATIONS
*
***/

//===========================================================================
static void INtOpDispatch (
    NtObject *  ntObj,
    Operation * op,
    dword       bytes
) {
    for (;;) {
        switch (op->opType) {
            case kOpConnAttempt:
                INtSocketOpCompleteSocketConnect((NtOpConnAttempt *) op);
            // operation not associated with ntObj so there is no next operation.
            // operation has already been deleted by OpCompleteSocketConnect.
            return;

            case kOpQueuedSocketWrite:
                INtSocketOpCompleteQueuedSocketWrite((NtSock *) ntObj, (NtOpSocketWrite *) op);
            // operation converted into kOpSocketWrite so we cannot move
            // to next operation until write operation completes
            return;

            case kOpSocketRead:
                ASSERT(bytes != (dword) -1);
                INtSocketOpCompleteSocketRead((NtSock *) ntObj, bytes);
            return;

            case kOpSocketWrite:
                ASSERT(bytes != (dword) -1);
                INtSocketOpCompleteSocketWrite((NtSock *) ntObj, (NtOpSocketWrite *) op);
            break;

            case kOpQueuedFileRead:
            case kOpQueuedFileWrite:
                INtFileOpCompleteQueuedReadWrite((NtFile *) ntObj, (NtOpFileReadWrite *) op);
            // operation converted into kOpFileWrite so we cannot move
            // to next operation until write operation completes
            return;

            case kOpFileRead:
            case kOpFileWrite:
                ASSERT(bytes != (dword) -1);
                if (!INtFileOpCompleteReadWrite((NtFile *) ntObj, (NtOpFileReadWrite *) op, bytes))
                    return;
            break;

            case kOpFileFlush:
                INtFileOpCompleteFileFlush((NtFile *) ntObj, (NtOpFileFlush *) op);
            break;

            case kOpSequence:
                INtFileOpCompleteSequence((NtFile *) ntObj, (NtOpFileSequence *) op);
            break;

            DEFAULT_FATAL(opType);
        }

        // if this operation is not at the head of the list then it can't be completed
        // because nextCompleteSequence would be prematurely incremented. Instead
        // convert the operation to OP_NULL, which will get completed when it reaches
        // the head of the list.
        ntObj->critsect.Enter();
        if (ntObj->opList.Prev(op)) {
            // setting the completion flag must be done inside the critical section
            // because it will be checked by sibling operations when they have the
            // critical section.
            op->pending = 0;
            ntObj->critsect.Leave();
            return;
        }

        // complete processing this event, and, since we're still inside the critical
        // section, finish all completed operations since we don't have to leave the
        // critical section to do so. This is a big win because a single operation
        // that takes a long time to complete can backlog a long list of completed ops.
        bool continueDispatch;
        for (;;) {
            // wake up any other threads waiting on this event
            CNtWaitHandle * signalComplete = op->signalComplete;
            op->signalComplete = nil;

            // since this operation is at the head of the list we can complete it
            if (op->asyncId && !++ntObj->nextCompleteSequence)
                ++ntObj->nextCompleteSequence;
            Operation * next = ntObj->opList.Next(op);
            ntObj->opList.Delete(op);
            op = next;

            // set event *after* operation is complete
            if (signalComplete) {
                signalComplete->SignalObject();
                signalComplete->DecRef();
            }

            // if we just deleted the last operation then stop dispatching
            if (!op) {
                continueDispatch = false;
                break;
            }

            // opTypes >= kOpSequence complete when they reach the head of the list
            continueDispatch = op->opType >= kOpSequence;
            if (op->pending)
                break;

            InterlockedDecrement(&ntObj->ioCount);
        }
        ntObj->critsect.Leave();

        INtConnCompleteOperation(ntObj);

        if (!continueDispatch)
            break;

        // certain operations which depend upon the value of bytes (reads & writes)
        // can only be dispatched when they are completed normally. To ensure that
        // we're not accidentally processing an operation that shouldn't be executed,
        // set the bytes field to an invalid value.
        bytes = (dword) -1;
    }
}

//===========================================================================
static unsigned THREADCALL NtWorkerThreadProc (AsyncThread * thread) {
	ref(thread);
	
    ThreadDenyBlock();

    unsigned sleepMs    = INFINITE;
    while (s_running) {

        // process I/O operations
        {
            dword bytes;
            NtObject *  ntObj;
            Operation * op;
            (void) GetQueuedCompletionStatus(
                s_ioPort,
                &bytes,
            #ifdef _WIN64
                (PULONG_PTR) &ntObj,
            #else
                (LPDWORD) &ntObj,
            #endif
                (LPOVERLAPPED *) &op,
                sleepMs
            );

            if (op) {
				// Queue for deadlock detection
				#ifdef SERVER
				void * check = CrashAddDeadlockCheck(thread->handle, L"pnAceNt.NtWorkerThread");
				#endif

				// Dispatch event to app
                INtOpDispatch(ntObj, op, bytes);
                
                // Unqueue from deadlock detection
				#ifdef SERVER
                CrashRemoveDeadlockCheck(check);
				#endif
				                
                sleepMs = 0;
                continue;
            }
        }

        sleepMs = INFINITE;
        continue;
    }

    return 0;
}


/****************************************************************************
*
*   Module functions
*
***/

//===========================================================================
void INtConnPostOperation (NtObject * ntObj, Operation * op, unsigned bytes) {
    PostQueuedCompletionStatus(
        s_ioPort,
        bytes,
        #ifdef _WIN64
            (ULONG_PTR) ntObj,
        #else
            (DWORD) ntObj,
        #endif
        &op->overlapped
    );
}

//===========================================================================
AsyncId INtConnSequenceStart (NtObject * ntObj) {
    unsigned result;
    if (0 == (result = ++ntObj->nextStartSequence))
        result = ++ntObj->nextStartSequence;
    return (AsyncId) result;
}

//===========================================================================
bool INtConnInitialize (NtObject * ntObj) {
    if (!CreateIoCompletionPort(ntObj->handle, s_ioPort, (DWORD) ntObj, 0)) {
        LogMsg(kLogFatal, "CreateIoCompletionPort failed");
        return false;
    }

    return true;
}

//===========================================================================
void INtConnCompleteOperation (NtObject * ntObj) {
    // are we completing the last operation for this object?
    if (InterlockedDecrement(&ntObj->ioCount))
        return;

    DWORD err = GetLastError();
    ref(err);
    switch (ntObj->ioType) {
        case kNtFile:
            INtFileDelete((NtFile *) ntObj);
        break;

        case kNtSocket:
            INtSockDelete((NtSock *) ntObj);
        break;

        default:
            LogMsg(kLogError, "NtConnCompleteOp %p %u", ntObj, ntObj->ioType);
        break;
    }
}

/*****************************************************************************
*
*   Module exports
*
***/

//===========================================================================
void NtInitialize () {
    // ensure initialization only occurs once
    if (s_running)
        return;
    s_running = true;

    // create a cleanup event
    s_waitEvent = CreateEvent(
        (LPSECURITY_ATTRIBUTES) 0,
        true,           // manual reset
        false,          // initial state off
        (LPCTSTR) nil   // name
    );
    if (!s_waitEvent)
        ErrorFatal(__LINE__, __FILE__, "CreateEvent %#x", GetLastError());        

    // create IO completion port
    if (0 == (s_ioPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0)))
        ErrorFatal(__LINE__, __FILE__, "CreateIoCompletionPort %#x", GetLastError());        

    // calculate number of IO worker threads to create
    if (!s_pageSizeMask) {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        s_pageSizeMask = si.dwPageSize - 1;

        // Set worker thread count
        s_ioThreadCount = si.dwNumberOfProcessors * 2;
        if (s_ioThreadCount > kMaxWorkerThreads) {
            s_ioThreadCount = kMaxWorkerThreads;
            LogMsg(kLogError, "kMaxWorkerThreads too small!");
        }
    }

    // create IO worker threads
    for (long thread = 0; thread < s_ioThreadCount; thread++) {
        s_ioThreadHandles[thread] = (HANDLE) AsyncThreadCreate(
            NtWorkerThreadProc,
            (void *) thread,
            L"NtWorkerThread"
        );
    }

    INtFileInitialize();
    INtSocketInitialize();
}

//===========================================================================
// DANGER: calling this function will slam closed any files which are still open.
// MOST PROGRAMS DO NOT NEED TO CALL THIS FUNCTION. In general, the best way to
// shut down the program is to simply let the atexit() handler take care of it.
void NtDestroy (unsigned exitThreadWaitMs) {
    // cleanup modules that post completion notifications as part of their shutdown
    INtFileStartCleanup();
    INtSocketStartCleanup(exitThreadWaitMs);

    // cleanup worker threads
    s_running = false;

    if (s_ioPort) {
        // Post a completion notification to worker threads to wake them up
        long thread;
        for (thread = 0; thread < s_ioThreadCount; thread++)
            PostQueuedCompletionStatus(s_ioPort, 0, 0, 0);

        // Close each thread
        for (thread = 0; thread < s_ioThreadCount; thread++) {
            if (s_ioThreadHandles[thread]) {
                WaitForSingleObject(s_ioThreadHandles[thread], exitThreadWaitMs);
                CloseHandle(s_ioThreadHandles[thread]);
                s_ioThreadHandles[thread] = nil;
            }
        }

        // Cleanup port
        CloseHandle(s_ioPort);
        s_ioPort = 0;
    }

    if (s_waitEvent) {
        CloseHandle(s_waitEvent);
        s_waitEvent = 0;
    }

    INtFileDestroy();
    INtSocketDestroy();
}

//===========================================================================
void NtSignalShutdown () {
    SetEvent(s_waitEvent);
}

//===========================================================================
void NtWaitForShutdown () {
    if (s_waitEvent)
        WaitForSingleObject(s_waitEvent, INFINITE);
}

} using namespace Nt;


/****************************************************************************
*
*   Public exports
*
***/

//===========================================================================
void NtGetApi (AsyncApi * api) {
    api->initialize             = NtInitialize;
    api->destroy                = NtDestroy;
    api->signalShutdown         = NtSignalShutdown;
    api->waitForShutdown        = NtWaitForShutdown;
    api->sleep                  = NtSleep;
    
    api->fileOpen               = NtFileOpen;
    api->fileClose              = NtFileClose;
    api->fileRead               = NtFileRead;
    api->fileWrite              = NtFileWrite;
    api->fileFlushBuffers       = NtFileFlushBuffers;
    api->fileSetLastWriteTime   = NtFileSetLastWriteTime;
    api->fileGetLastWriteTime   = NtFileGetLastWriteTime;
    api->fileCreateSequence     = NtFileCreateSequence;
    api->fileSeek               = NtFileSeek;
    
    api->socketConnect          = NtSocketConnect;
    api->socketConnectCancel    = NtSocketConnectCancel;
    api->socketDisconnect       = NtSocketDisconnect;
    api->socketDelete           = NtSocketDelete;
    api->socketSend             = NtSocketSend;
    api->socketWrite            = NtSocketWrite;
    api->socketSetNotifyProc    = NtSocketSetNotifyProc;
    api->socketSetBacklogAlloc  = NtSocketSetBacklogAlloc;
    api->socketStartListening   = NtSocketStartListening;
    api->socketStopListening    = NtSocketStopListening;
    api->socketEnableNagling    = NtSocketEnableNagling;
}
