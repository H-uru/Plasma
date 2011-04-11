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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCore/Private/pnAcThread.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNASYNCCORE_PRIVATE_PNACTHREAD_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCore/Private/pnAcThread.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNASYNCCORE_PRIVATE_PNACTHREAD_H


/****************************************************************************
*
*   Type definitions
*
***/

// for IoWaitId/TimerCreate/TimerUpdate
const unsigned kAsyncTimeInfinite = (unsigned) -1;

#ifdef   _MSC_VER
#define  THREADCALL __stdcall
#else
#define  THREADCALL __cdecl
#endif

struct AsyncThread;
typedef unsigned (THREADCALL * FAsyncThreadProc)(AsyncThread * thread);


// Threads are also allowed to set the workTimeMs field of their
// structure to a nonzero value for "on", and IO_TIME_INFINITE for
// "off" to avoid the overhead of calling these functions. Note
// that this function may not be called for the main thread. I
// suggest that application code not worry that timeMs might 
// "accidentally" equal the IO_TIME_INFINITE value, as it only 
// happens for one millisecond every 49 days.
struct AsyncThread {
    LINK(AsyncThread)   link;
    FAsyncThreadProc    proc;
    void *              handle;
    void *              argument;
    unsigned            workTimeMs;
    wchar               name[16];
};

/*****************************************************************************
*
*   Thread functions
*
***/

void * AsyncThreadCreate (
    FAsyncThreadProc    proc,
    void *              argument,
    const wchar         name[]
);

// This function should ONLY be called during shutdown while waiting for things to expire
void AsyncSleep (unsigned sleepMs);


/*****************************************************************************
*
*   Thread task functions
*
***/

enum EThreadTaskPriority {
    kThreadTaskPriorityNormal = 1,
    kNumThreadTaskPriorities
};

const unsigned kThreadTaskMinThreads = 5;
const unsigned kThreadTaskDefThreads = 100;
const unsigned kThreadTaskMaxThreads = 1000;

struct AsyncThreadTaskList;

typedef void (* FAsyncThreadTask)(
    void *                  param, 
    ENetError               error
);


void AsyncThreadTaskInitialize (unsigned threads);
void AsyncThreadTaskDestroy ();

unsigned AsyncThreadTaskGetThreadCount ();
void AsyncThreadTaskSetThreadCount (unsigned threads);

AsyncThreadTaskList * AsyncThreadTaskListCreate ();
void AsyncThreadTaskListDestroy (
    AsyncThreadTaskList *   taskList,
    ENetError               error
);

void AsyncThreadTaskAdd (
    AsyncThreadTaskList *   taskList,
    FAsyncThreadTask        callback,
    void *                  param,
	const wchar				debugStr[],
    EThreadTaskPriority     priority = kThreadTaskPriorityNormal
);
