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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCoreExe/Private/Win32/pnAceW32Thread.cpp
*   
***/

#include "../../Pch.h"
#pragma hdrstop

#include "hsRefCnt.h"


/*****************************************************************************
*
*   Private
*
***/

struct AsyncThreadTaskList : hsRefCnt {
    ENetError error;
    AsyncThreadTaskList ();
    ~AsyncThreadTaskList ();
};

struct ThreadTask {
    AsyncThreadTaskList *   taskList;
    FAsyncThreadTask        callback;
    void *                  param;
    wchar_t                   debugStr[256];
};

static HANDLE   s_taskPort;


/*****************************************************************************
*
*   AsyncThreadTaskList
*
***/

//===========================================================================
AsyncThreadTaskList::AsyncThreadTaskList ()
:   hsRefCnt(0), error(kNetSuccess)
{
    PerfAddCounter(kAsyncPerfThreadTaskListCount, 1);
}

//============================================================================
AsyncThreadTaskList::~AsyncThreadTaskList () {
    PerfSubCounter(kAsyncPerfThreadTaskListCount, 1);
}


/*****************************************************************************
*
*   Local functions
*
***/

/****************************************************************************
*
*   ThreadTaskProc
*
***/

//===========================================================================
static unsigned THREADCALL ThreadTaskProc (AsyncThread * thread) {
    PerfAddCounter(kAsyncPerfThreadTaskThreadsActive, 1);

    for (;;) {
        long desired = AsyncPerfGetCounter(kAsyncPerfThreadTaskThreadsDesired);
        if (AsyncPerfGetCounter(kAsyncPerfThreadTaskThreadsRunning) > desired) {
            long runningCount = PerfSubCounter(kAsyncPerfThreadTaskThreadsRunning, 1) - 1;
            if (runningCount >= desired) {
                if (runningCount > desired)
                    PostQueuedCompletionStatus(s_taskPort, 0, 0, 0);
                break;
            }
            PerfAddCounter(kAsyncPerfThreadTaskThreadsRunning, 1);
        }

        // Get the next work item
        DWORD bytes;
        ThreadTask * task;
        LPOVERLAPPED op;
        PerfSubCounter(kAsyncPerfThreadTaskThreadsActive, 1);
        (void) GetQueuedCompletionStatus(
            s_taskPort,
            &bytes,
            #ifdef _WIN64
            (PULONG_PTR) &task,
            #else
            (LPDWORD) &task,
            #endif
            &op,
            INFINITE
        );
        PerfAddCounter(kAsyncPerfThreadTaskThreadsActive, 1);

        if (task) {
            task->callback(task->param, task->taskList->error);

            task->taskList->Ref("task");
            delete task;
        }
    }
    PerfSubCounter(kAsyncPerfThreadTaskThreadsActive, 1);

    return 0;
}

//===========================================================================
static unsigned THREADCALL FirstThreadTaskProc (AsyncThread * param) {
    while (AsyncPerfGetCounter(kAsyncPerfThreadTaskThreadsRunning) < AsyncPerfGetCounter(kAsyncPerfThreadTaskThreadsDesired)) {
        PerfAddCounter(kAsyncPerfThreadTaskThreadsRunning, 1);
        AsyncThreadCreate(ThreadTaskProc, nil, L"AsyncThreadTaskList");
    }

    return ThreadTaskProc(param);
}

/*****************************************************************************
*
*   Module functions
*
***/

/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
void AsyncThreadTaskInitialize (unsigned threads) {
    // Create completion port
    s_taskPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
    ASSERT(s_taskPort);

    // Create threads
    AsyncThreadTaskSetThreadCount(threads);
}

//============================================================================
void AsyncThreadTaskDestroy () {
    ASSERT(!AsyncPerfGetCounter(kAsyncPerfThreadTaskListCount));

    if (s_taskPort) {
        PerfSetCounter(kAsyncPerfThreadTaskThreadsDesired, 0);
        PostQueuedCompletionStatus(s_taskPort, 0, 0, 0);

        // Wait until all threads have exited
        while (AsyncPerfGetCounter(kAsyncPerfThreadTaskThreadsActive))
            AsyncSleep(10);
        while (AsyncPerfGetCounter(kAsyncPerfThreadTaskThreadsRunning))
            AsyncSleep(10);

        // Cleanup completion port
        CloseHandle(s_taskPort);
        s_taskPort = nil;
    }
}

//===========================================================================
unsigned AsyncThreadTaskGetThreadCount () {
    return AsyncPerfGetCounter(kAsyncPerfThreadTaskThreadsDesired);
}

//===========================================================================
void AsyncThreadTaskSetThreadCount (unsigned threads) {
    ASSERT(threads >= kThreadTaskMinThreads);
    ASSERT(threads <= kThreadTaskMaxThreads);

    if (AsyncPerfGetCounter(kAsyncPerfThreadTaskThreadsDesired) == (long) threads)
        return;
    PerfSetCounter(kAsyncPerfThreadTaskThreadsDesired, (long) threads);

    if (AsyncPerfGetCounter(kAsyncPerfThreadTaskThreadsRunning) < AsyncPerfGetCounter(kAsyncPerfThreadTaskThreadsDesired)) {
        PerfAddCounter(kAsyncPerfThreadTaskThreadsRunning, 1);
        AsyncThreadCreate(FirstThreadTaskProc, nil, L"ThreadTaskList");
    }
    else {
        PostQueuedCompletionStatus(s_taskPort, 0, 0, 0);
    }
}

//===========================================================================
AsyncThreadTaskList * AsyncThreadTaskListCreate () {
    ASSERT(s_taskPort);
    AsyncThreadTaskList * taskList = new AsyncThreadTaskList;
    taskList->Ref("TaskList");
    return taskList;
}

//===========================================================================
void AsyncThreadTaskListDestroy (
    AsyncThreadTaskList *   taskList,
    ENetError               error
) {
    ASSERT(taskList);
    ASSERT(error);
    ASSERT(!taskList->error);

    taskList->error = error;
    taskList->UnRef(); // REF:TaskList
}

//===========================================================================
void AsyncThreadTaskAdd (
    AsyncThreadTaskList *   taskList,
    FAsyncThreadTask        callback,
    void *                  param,
    const wchar_t             debugStr[],
    EThreadTaskPriority     priority /* = kThreadTaskPriorityNormal */
) {
    ASSERT(s_taskPort);
    ASSERT(taskList);
    ASSERT(callback);
    ASSERT(priority == kThreadTaskPriorityNormal);

    // Allocate a new task record
    ThreadTask * task   = new ThreadTask;
    task->taskList      = taskList;
    task->callback      = callback;
    task->param         = param;
    StrCopy(task->debugStr, debugStr, arrsize(task->debugStr));     // this will be sent with the deadlock checker email if this thread exceeds time set in plServer.ini
    taskList->Ref("Task");

    PostQueuedCompletionStatus(s_taskPort, 0, (DWORD) task, NULL);
}
