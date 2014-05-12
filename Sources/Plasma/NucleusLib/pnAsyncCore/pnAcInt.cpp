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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCore/pnAcInt.cpp
*   
***/

#include "pnAcInt.h"
#include "hsThread.h"
#if HS_BUILD_FOR_WIN32
#   include <windows.h>
#endif
#pragma hdrstop


/*****************************************************************************
*
*   Worker Threads
*
***/

struct IWorkerThreads::Private {
    struct Thread;
    static Private * This;
    
    Thread *        threads;
    unsigned        threadCount;
    
    Operation *     listHead;
    Operation *     listQueu;
    std::mutex      headLock;
    std::mutex      queuLock;
    hsSemaphore     listSem;
    
    Private() : threads(), threadCount(0), listHead(), listQueu() {}
};
IWorkerThreads::Private * IWorkerThreads::Private::This = nullptr;

struct IWorkerThreads::Private::Thread : hsThread {
    void Run();
};

//===========================================================================
void IWorkerThreads::Private::Thread::Run() {
    while (1) {
        This->listSem.Wait();
        
        // op = pop()
        Operation * op;
        {
            std::lock_guard<std::mutex> hlock(This->headLock);
            op = This->listHead;
            if (!op)
                break; // end of running
            if (op->next)
                This->listHead = op->next; // have a next: no need to lock list queu
            else {
                std::lock_guard<std::mutex> qlock(This->queuLock);
                This->listHead = op->next; // 'next' can change before list queu is locked
                if (!op->next) // last operation
                    This->listQueu = nullptr;
            }
        }
        
        op->Callback();
    }
}

//===========================================================================
void IWorkerThreads::Add (Operation * op) {
    ASSERT(op);
    ASSERT(Private::This);
    
    // push(op)
    op->next = nullptr;
    std::lock_guard<std::mutex> lock(Private::This->queuLock);
    if (Private::This->listQueu)
        Private::This->listQueu->next = op;
    else
        // list is empty: no 'pop' can append (and is not dangerous if append because op is initialised, and affectation is atomic)
        // and no other 'push' can append because queu is locked
        // => no lock needed on listHead!
        Private::This->listHead = op;
    
    Private::This->listQueu = op;
    Private::This->listSem.Signal();
}

//===========================================================================
void IWorkerThreads::Create () {
    ASSERT(!P::This);
    
    Private::This = new Private();
    Private::This->threadCount = 8; // TODO: hsThread::hardware_concurrency() * 2 (or use directly std::thread ?)
    Private::This->threads = new Private::Thread[Private::This->threadCount];
    for (int i = 0; i < Private::This->threadCount; i++)
        Private::This->threads[i].Start();
}

//===========================================================================
void IWorkerThreads::Delete (unsigned timeoutMs) {
    ASSERT(Private::This);
    
    for (int i = 0; i < Private::This->threadCount; i++)
        Private::This->listSem.Signal();
    
    for (int i = 0; i < Private::This->threadCount; i++)
        Private::This->threads[i].Stop(); // TODO: timeout
}


/*****************************************************************************
*
*   Public exports
*
***/

//===========================================================================
void AsyncCoreInitialize () {
#if HS_BUILD_FOR_WIN32
    // Initialize WinSock
    WSADATA wsaData;
    if (WSAStartup(0x101, &wsaData))
        ErrorAssert(__LINE__, __FILE__, "WSA startup failed");
    if (wsaData.wVersion != 0x101)
        ErrorAssert(__LINE__, __FILE__, "WSA version failed");
#endif
    
    IWorkerThreads::Create();
}

//===========================================================================
// DANGER: calling this function will slam closed any files which are still open.
// MOST PROGRAMS DO NOT NEED TO CALL THIS FUNCTION. In general, the best way to
// shut down the program is to simply let the atexit() handler take care of it.
void AsyncCoreDestroy (unsigned waitMs) {
    // cleanup modules that post completion notifications as part of their shutdown
    //INtSocketStartCleanup(waitMs);

    // cleanup worker threads
    //s_running = false;

    SocketDestroy();
    DnsDestroy(waitMs);
    TimerDestroy(waitMs);
    IWorkerThreads::Delete(waitMs);
}


