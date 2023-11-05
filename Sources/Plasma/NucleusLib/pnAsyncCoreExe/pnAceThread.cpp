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

static void CreateThreadProc(AsyncThreadRef thread)
{
    hsThread::SetThisThreadName(ST_LITERAL("NoNameAceThread"));

#ifdef USE_VLD
    VLDEnable();
#endif

    PerfAddCounter(kAsyncPerfThreadsTotal, 1);
    PerfAddCounter(kAsyncPerfThreadsCurr, 1);

    // Call thread procedure
    thread.impl->proc();

    PerfSubCounter(kAsyncPerfThreadsCurr, 1);

    thread.impl->completion.unlock();
}

/*****************************************************************************
*
*   Module functions
*
***/

//============================================================================
void ThreadDestroy(unsigned exitThreadWaitMs)
{
    unsigned bailAt = hsTimer::GetMilliSeconds<unsigned>() + exitThreadWaitMs;
    while (AsyncPerfGetCounter(kAsyncPerfThreadsCurr) && hsTimer::GetMilliSeconds<unsigned>() < bailAt) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

//============================================================================
AsyncThreadRef AsyncThreadCreate(std::function<void()> threadProc)
{
    AsyncThreadRef ref;
    ref.impl = std::make_shared<AsyncThread>();
    ref.impl->proc = std::move(threadProc);
    ref.impl->workTimeMs = kAsyncTimeInfinite;

    ref.impl->completion.lock();

    ref.impl->handle = std::thread(&CreateThreadProc, ref);
    return ref;
}

void AsyncThreadTimedJoin(AsyncThreadRef& ref, unsigned timeoutMs)
{
    bool threadFinished = ref.impl->completion.try_lock_for(std::chrono::milliseconds(timeoutMs));
    if (threadFinished) {
        //thread is finished, safe to join with no deadlock risk
        ref.impl->completion.unlock();
        ref.impl->handle.join();
    } else {
        LogMsg(kLogDebug, "Thread did not terminate after {} ms", timeoutMs);
        ref.impl->handle.detach();
    }
}

std::thread& AsyncThreadRef::thread() const
{
    return impl->handle;
}

bool AsyncThreadRef::joinable() const
{
    if (!impl) {
        return false;
    } else {
        return impl->handle.joinable();
    }
}
