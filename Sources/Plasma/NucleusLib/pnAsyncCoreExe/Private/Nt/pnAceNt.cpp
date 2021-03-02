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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCoreExe/Private/Nt/pnAceNt.cpp
*   
***/

#include "../../Pch.h"

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

static unsigned int             s_ioThreadCount;
static std::thread              s_ioThreadHandles[kMaxWorkerThreads];

static HANDLE                   s_ioPort;


/****************************************************************************
*
*   OPERATIONS
*
***/

//===========================================================================
static void INtOpDispatch (
    NtObject *  ntObj,
    Operation * op,
    uint32_t    bytes
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
                ASSERT(bytes != (uint32_t) -1);
                INtSocketOpCompleteSocketRead((NtSock *) ntObj, bytes);
            return;

            case kOpSocketWrite:
                ASSERT(bytes != (uint32_t) -1);
                INtSocketOpCompleteSocketWrite((NtSock *) ntObj, (NtOpSocketWrite *) op);
            break;

            DEFAULT_FATAL(opType);
        }

        // if this operation is not at the head of the list then it can't be completed
        // because nextCompleteSequence would be prematurely incremented. Instead
        // convert the operation to OP_NULL, which will get completed when it reaches
        // the head of the list.
        bool continueDispatch;
        {
            hsLockGuard(ntObj->critsect);
            if (ntObj->opList.Prev(op)) {
                // setting the completion flag must be done inside the critical section
                // because it will be checked by sibling operations when they have the
                // critical section.
                op->pending = 0;
                return;
            }

            // complete processing this event, and, since we're still inside the critical
            // section, finish all completed operations since we don't have to leave the
            // critical section to do so. This is a big win because a single operation
            // that takes a long time to complete can backlog a long list of completed ops.
            for (;;) {
                // since this operation is at the head of the list we can complete it
                if (op->asyncId && !++ntObj->nextCompleteSequence)
                    ++ntObj->nextCompleteSequence;
                Operation * next = ntObj->opList.Next(op);
                ntObj->opList.Delete(op);
                op = next;

                // if we just deleted the last operation then stop dispatching
                if (!op) {
                    continueDispatch = false;
                    break;
                }

                // opTypes >= kOpSequence complete when they reach the head of the list
                continueDispatch = op->opType >= kOpSequence;
                if (op->pending)
                    break;

                --ntObj->ioCount;
            }
        }

        INtConnCompleteOperation(ntObj);

        if (!continueDispatch)
            break;

        // certain operations which depend upon the value of bytes (reads & writes)
        // can only be dispatched when they are completed normally. To ensure that
        // we're not accidentally processing an operation that shouldn't be executed,
        // set the bytes field to an invalid value.
        bytes = (uint32_t) -1;
    }
}

//===========================================================================
static void NtWorkerThreadProc() {
    unsigned sleepMs    = INFINITE;
    while (s_running) {

        // process I/O operations
        {
            DWORD bytes;
            NtObject *  ntObj;
            Operation * op;
            (void) GetQueuedCompletionStatus(
                s_ioPort,
                &bytes,
                (PULONG_PTR) &ntObj,
                (LPOVERLAPPED *) &op,
                sleepMs
            );

            if (op) {
                // Dispatch event to app
                INtOpDispatch(ntObj, op, bytes);

                sleepMs = 0;
                continue;
            }
        }

        sleepMs = INFINITE;
    }
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
        (ULONG_PTR) ntObj,
        &op->overlapped
    );
}

//===========================================================================
AsyncId INtConnSequenceStart (NtObject * ntObj) {
    intptr_t result;
    if (0 == (result = ++ntObj->nextStartSequence))
        result = ++ntObj->nextStartSequence;
    return (AsyncId) result;
}

//===========================================================================
bool INtConnInitialize (NtObject * ntObj) {
    if (!CreateIoCompletionPort(ntObj->handle, s_ioPort, (ULONG_PTR) ntObj, 0)) {
        LogMsg(kLogFatal, "CreateIoCompletionPort failed");
        return false;
    }

    return true;
}

//===========================================================================
void INtConnCompleteOperation (NtObject * ntObj) {
    // are we completing the last operation for this object?
    if (--ntObj->ioCount)
        return;

    DWORD err = GetLastError();
    switch (ntObj->ioType) {
        case kNtSocket:
            INtSockDelete((NtSock *) ntObj);
        break;

        default:
            LogMsg(kLogError, "NtConnCompleteOp {#x} {}", (uintptr_t)ntObj, ntObj->ioType);
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

    // create IO completion port
    if (s_ioPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0); s_ioPort == nullptr)
        ErrorAssert(__LINE__, __FILE__, "CreateIoCompletionPort {#x}", GetLastError());

    // calculate number of IO worker threads to create
    if (!s_ioThreadCount) {
        // Set worker thread count
        s_ioThreadCount = std::max(std::thread::hardware_concurrency() * 2, 2U);
        if (s_ioThreadCount > kMaxWorkerThreads) {
            s_ioThreadCount = kMaxWorkerThreads;
            LogMsg(kLogError, "kMaxWorkerThreads too small!");
        }
    }

    // create IO worker threads
    for (unsigned thread = 0; thread < s_ioThreadCount; thread++) {
        s_ioThreadHandles[thread] = std::thread([] {
#ifdef USE_VLD
            VLDEnable();
#endif
            PerfAddCounter(kAsyncPerfThreadsTotal, 1);
            PerfAddCounter(kAsyncPerfThreadsCurr, 1);

            NtWorkerThreadProc();

            PerfSubCounter(kAsyncPerfThreadsCurr, 1);
        });
    }

    INtSocketInitialize();
}

//===========================================================================
// DANGER: calling this function will slam closed any files which are still open.
// MOST PROGRAMS DO NOT NEED TO CALL THIS FUNCTION. In general, the best way to
// shut down the program is to simply let the atexit() handler take care of it.
void NtDestroy (unsigned exitThreadWaitMs) {
    // cleanup modules that post completion notifications as part of their shutdown
    INtSocketStartCleanup(exitThreadWaitMs);

    // cleanup worker threads
    s_running = false;

    if (s_ioPort) {
        // Post a completion notification to worker threads to wake them up
        long thread;
        for (thread = 0; thread < s_ioThreadCount; thread++)
            PostQueuedCompletionStatus(s_ioPort, 0, 0, nullptr);

        // Close each thread
        for (thread = 0; thread < s_ioThreadCount; thread++) {
            if (s_ioThreadHandles[thread].joinable()) {
                AsyncThreadTimedJoin(s_ioThreadHandles[thread], exitThreadWaitMs);
                s_ioThreadHandles[thread] = {};
            }
        }

        // Cleanup port
        CloseHandle(s_ioPort);
        s_ioPort = nullptr;
    }

    INtSocketDestroy();
}

} // namespace Nt
