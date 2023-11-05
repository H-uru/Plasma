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

#include "Pch.h"

struct AsyncTimer
{
    asio::steady_timer fTimer;
    FAsyncTimerProc    fTimerProc;
    FAsyncTimerProc    fDestroyProc;
    void*              fParam;

    AsyncTimer(asio::io_context& context, FAsyncTimerProc&& timerProc, void* param)
        : fTimer(context),
          fTimerProc(std::move(timerProc)),
          fParam(param)
    {
    }

    void Start(unsigned callbackMs);
};

static std::recursive_mutex s_timerCrit;

struct AsyncTimerManager
{
    asio::io_context                                           fContext;
    asio::executor_work_guard<asio::io_context::executor_type> fWorkGuard;
    std::list<AsyncTimer>                                      fTimers;
    AsyncThreadRef                                             fTimerThread;

    AsyncTimerManager() : fWorkGuard(fContext.get_executor())
    {
        fTimerThread = AsyncThreadCreate([this] {
            hsThread::SetThisThreadName(ST_LITERAL("AceTimerMgr"));
            fContext.run();
        });
    }

    AsyncTimer* AddTimer(FAsyncTimerProc&& timerProc, void* param)
    {
        return &fTimers.emplace_back(fContext, std::move(timerProc), param);
    }

    void DelTimer(AsyncTimer* timer)
    {
        // This needs to happen on the same thread that calls the timers to
        // ensure we don't have both a timer callback and a deletion occurring
        // at the same time.
        asio::post(fContext, [timer, this] {
            if (timer->fDestroyProc)
                timer->fDestroyProc(timer->fParam);

            hsLockGuard(s_timerCrit);
            auto iter = std::find_if(fTimers.cbegin(), fTimers.cend(),
                                     [timer](const AsyncTimer& t) { return &t == timer; });
            if (iter != fTimers.cend())
                fTimers.erase(iter);
        });
    }

    void Destroy(unsigned exitThreadWaitMs)
    {
        fWorkGuard.reset();
        AsyncThreadTimedJoin(fTimerThread, exitThreadWaitMs);

        // Ensure the event loop exits without processing any more tasks,
        // in case the thread takes more than exitThreadWaitMs to finish
        fContext.stop();

        // Cancel all remaining timers
        for (AsyncTimer& timer : fTimers) {
            if (timer.fDestroyProc)
                timer.fDestroyProc(timer.fParam);
        }
        fTimers.clear();
    }
};

static AsyncTimerManager* s_timerMgr = nullptr;

void AsyncTimer::Start(unsigned callbackMs)
{
    if (callbackMs == kAsyncTimeInfinite)
        return;

    fTimer.expires_after(std::chrono::milliseconds(callbackMs));
    fTimer.async_wait([this](const asio::error_code& err) {
        if (err == asio::error::operation_aborted)
            return;

        unsigned timerMs = fTimerProc(fParam);

        // Requeue timer
        hsLockGuard(s_timerCrit);
        Start(timerMs);
    });
}

void TimerDestroy(unsigned exitThreadWaitMs)
{
    if (s_timerMgr) {
        s_timerMgr->Destroy(exitThreadWaitMs);
        delete s_timerMgr;
        s_timerMgr = nullptr;
    }
}

AsyncTimer* AsyncTimerCreate(FAsyncTimerProc timerProc, unsigned callbackMs, void* param)
{
    ASSERT(timerProc);

    hsLockGuard(s_timerCrit);

    if (!s_timerMgr)
        s_timerMgr = new AsyncTimerManager;

    AsyncTimer* timer = s_timerMgr->AddTimer(std::move(timerProc), param);
    timer->Start(callbackMs);

    return timer;
}

void AsyncTimerDelete(AsyncTimer* timer)
{
    ASSERT(!timer->fDestroyProc);
    AsyncTimerDeleteCallback(timer, nullptr);
}

void AsyncTimerDeleteCallback(AsyncTimer* timer, FAsyncTimerProc destroyProc)
{
    ASSERT(timer);
    ASSERT(s_timerMgr);

    hsLockGuard(s_timerCrit);
    timer->fDestroyProc = std::move(destroyProc);
    s_timerMgr->DelTimer(timer);
}

void AsyncTimerUpdate(AsyncTimer* timer, unsigned callbackMs)
{
    ASSERT(timer);

    hsLockGuard(s_timerCrit);
    timer->fTimer.cancel();
    timer->Start(callbackMs);
}
