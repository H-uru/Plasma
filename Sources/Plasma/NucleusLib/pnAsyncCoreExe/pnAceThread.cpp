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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCoreExe/pnAceThread.cpp
*   
***/

#include "Pch.h"

#if defined(HAVE_PTHREAD_TIMEDJOIN_NP)
#include <pthread.h>
#include <time.h>
#endif

static void CreateThreadProc(AsyncThreadRef thread)
{
#ifdef USE_VLD
    VLDEnable();
#endif

    PerfAddCounter(kAsyncPerfThreadsTotal, 1);
    PerfAddCounter(kAsyncPerfThreadsCurr, 1);

    // Call thread procedure
    thread.impl->proc();

    PerfSubCounter(kAsyncPerfThreadsCurr, 1);
    
    thread.impl->completion->unlock();
}

/*****************************************************************************
*
*   Module functions
*
***/

//============================================================================
void ThreadDestroy (unsigned exitThreadWaitMs) {

    unsigned bailAt = TimeGetMs() + exitThreadWaitMs;
    while (AsyncPerfGetCounter(kAsyncPerfThreadsCurr) && signed(bailAt - TimeGetMs()) > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

//============================================================================
AsyncThreadRef AsyncThreadCreate (
    std::function<void()>    threadProc
                               ) {
    AsyncThreadRef ref;
    ref.impl                  = std::make_shared<AsyncThread>();
    ref.impl->proc            = threadProc;
    ref.impl->handle          = nullptr;
    ref.impl->workTimeMs      = kAsyncTimeInfinite;
    
    auto completion = std::make_shared<std::timed_mutex>();
    completion->lock();
    ref.impl->completion = completion;
    
    auto handle = std::make_shared<std::thread>(&CreateThreadProc, ref);
    ref.impl->handle = handle;
    return ref;
}

void AsyncThreadTimedJoin(AsyncThreadRef& ref, unsigned timeoutMs)
{
    bool threadFinished = ref.impl->completion->try_lock_for(std::chrono::milliseconds(timeoutMs));
    if (threadFinished) {
        //thread is finished, safe to join with no deadlock risk
        ref.impl->completion->unlock();
        ref.impl->handle->join();
    } else {
        ref.impl->handle->detach();
    }
}

void AsyncThreadTimedJoin(std::thread& thread, unsigned timeoutMs)
{
    // HACK: No cross-platform way to perform a timed join :(
#if defined(HS_BUILD_FOR_WIN32)
    DWORD rc = WaitForSingleObject((HANDLE)thread.native_handle(), timeoutMs);
    if (rc == WAIT_TIMEOUT)
        LogMsg(kLogDebug, "Thread did not terminate after {} ms", timeoutMs);
    thread.detach();
/*#elif defined(HAVE_PTHREAD_TIMEDJOIN_NP)
    struct timespec deadline;
    if (clock_gettime(CLOCK_REALTIME, &deadline) < 0)
        hsAssert(false, "Could not get the realtime clock");
    deadline.tv_sec += timeoutMs / 1000;
    deadline.tv_nsec += (timeoutMs % 1000) * 1'000'000;
    if (deadline.tv_nsec > 1'000'000'000) {
        deadline.tv_nsec -= 1'000'000'000;
        deadline.tv_sec += 1;
    }
    if (pthread_timedjoin_np(thread.native_handle(), nullptr, &deadline) != 0) {
        LogMsg(kLogDebug, "Thread did not terminate after {} ms", timeoutMs);
        thread.detach();
    }*/
#else
    LogMsg(kLogDebug, "No timed thread join support for this system... "
                      "Performing a blocking join instead.");
    thread.join();
#endif
}

std::shared_ptr<std::thread> AsyncThreadRef::thread() {
    return impl->handle;
}

bool AsyncThreadRef::joinable() {
    if (!impl) {
        return false;
    } else {
        return impl->handle->joinable();
    }
}
