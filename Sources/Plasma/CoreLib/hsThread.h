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
#ifndef hsThread_Defined
#define hsThread_Defined

#include "HeadSpin.h"
#include "hsLockGuard.h"

#include <atomic>
#include <condition_variable>
#include <limits>
#include <mutex>
#include <thread>

#include <string_theory/string>

typedef uint32_t hsMilliseconds;
static constexpr hsMilliseconds kWaitForever = std::numeric_limits<hsMilliseconds>::max();

#ifdef HS_BUILD_FOR_UNIX
    #include <pthread.h>
    #include <semaphore.h>
    //  We can't wait with a timeout with semas
    #define USE_SEMA
    // Linux kernel 2.4 w/ NTPL threading patch and O(1) scheduler
    // seems to have a problem in it's cond_t implementation that
    // causes a hang under heavy load. This is a workaround that
    // uses select() and pipes.
//  #define PSEUDO_EVENT
#endif

class hsThread
{
    std::atomic<bool>   fQuit;
    std::thread         fThread;

protected:
    bool        GetQuit() const { return fQuit; }
    void        SetQuit(bool value) { fQuit = value; }

public:
    hsThread() : fQuit(false) { }

    virtual ~hsThread()
    {
        this->Stop();
    }

    // Disable copying
    hsThread(const hsThread &) = delete;
    void operator=(const hsThread &) = delete;

    virtual void    Run() = 0;      // override this to do your work
    virtual void    Start();        // initializes stuff and calls your Run() method
    virtual void    Stop();         // sets fQuit = true and the waits for the thread to stop
    virtual void    OnQuit() { }

    // Start the thread in a detached state, so it will continue running
    // in the background, and doesn't need to be joined.  NOTE: The thread
    // must be able to manage itself -- destroying the hsThread object
    // WILL NOT stop a detached thread!
    void StartDetached();

    // Set a name for the current thread, to be displayed in debuggers and such.
    // If possible, don't use names longer than 15 characters,
    // because Linux has a really low limit.
    static void SetThisThreadName(const ST::string& name);

    static inline size_t ThisThreadHash()
    {
        return std::hash<std::thread::id>()(std::this_thread::get_id());
    }
};

//////////////////////////////////////////////////////////////////////////////
class hsSemaphore
{
    std::mutex fMutex;
    std::condition_variable fCondition;
    unsigned fValue;

public:
    hsSemaphore(unsigned initial = 0) : fValue(initial) { }

    inline void Wait()
    {
        std::unique_lock<std::mutex> lock(fMutex);
        fCondition.wait(lock, [this]() { return fValue > 0; });
        --fValue;
    }

    template <class _Rep, class _Period>
    inline bool Wait(const std::chrono::duration<_Rep, _Period> &duration)
    {
        std::unique_lock<std::mutex> lock(fMutex);

        bool result = fCondition.wait_for(lock, duration, [this]() { return fValue > 0; });
        if (result)
            --fValue;

        return result;
    }

    inline void Signal()
    {
        std::unique_lock<std::mutex> lock(fMutex);
        ++fValue;
        fCondition.notify_one();
    }
};

//////////////////////////////////////////////////////////////////////////////
class hsGlobalSemaphore {
#if HS_BUILD_FOR_WIN32
    HANDLE  fSemaH;
#elif HS_BUILD_FOR_UNIX
#ifdef USE_SEMA
    sem_t*  fPSema;
    bool    fNamed;
#else
    pthread_mutex_t fPMutex;
    pthread_cond_t  fPCond;
    int32_t         fCounter;
#endif
#endif
public:
    hsGlobalSemaphore(int initialValue = 0, const ST::string& name = {});
    ~hsGlobalSemaphore();

#ifdef HS_BUILD_FOR_WIN32
    HANDLE GetHandle() const { return fSemaH; }
#endif

    bool Wait(hsMilliseconds timeToWait = kWaitForever);
    void Signal();
};

//////////////////////////////////////////////////////////////////////////////
class hsEvent
{
    std::mutex fMutex;
    std::condition_variable fCondition;
    bool fEvent;

public:
    hsEvent() : fEvent(false) { }

    inline void Wait()
    {
        std::unique_lock<std::mutex> lock(fMutex);
        fCondition.wait(lock, [this]() { return fEvent; });
        fEvent = false;
    }

    template <class _Rep, class _Period>
    inline bool Wait(const std::chrono::duration<_Rep, _Period> &duration)
    {
        std::unique_lock<std::mutex> lock(fMutex);

        bool result = fCondition.wait_for(lock, duration, [this]() { return fEvent; });
        if (result)
            fEvent = false;

        return result;
    }

    inline void Signal()
    {
        std::unique_lock<std::mutex> lock(fMutex);
        fEvent = true;
        fCondition.notify_one();
    }
};

//////////////////////////////////////////////////////////////////////////////
// Allows multiple readers, locks out readers for writing.

class hsReaderWriterLock
{
public:
    hsReaderWriterLock() : fReaderCount(0), fWriterSem(1) { }

private:
    void LockForReading()
    {
        // Don't allow us to start reading if there's still an active writer
        hsLockGuard(fReaderLock);

        fReaderCount++;
        if (fReaderCount == 1) {
            // Block writers from starting (wait is a misnomer here)
            fWriterSem.Wait();
        }
    }

    void UnlockForReading()
    {
        fReaderCount--;
        if (fReaderCount == 0)
            fWriterSem.Signal();
    }

    void LockForWriting()
    {
        // Blocks new readers from starting
        fReaderLock.lock();

        // Wait until all readers are done
        fWriterSem.Wait();
    }

    void UnlockForWriting()
    {
        fWriterSem.Signal();
        fReaderLock.unlock();
    }

    std::atomic<int>    fReaderCount;
    std::mutex          fReaderLock;
    hsSemaphore         fWriterSem;

    friend class hsLockForReading;
    friend class hsLockForWriting;
};

class hsLockForReading
{
    hsReaderWriterLock& fLock;

public:
    hsLockForReading(hsReaderWriterLock& lock) : fLock(lock)
    {
        fLock.LockForReading();
    }

    ~hsLockForReading()
    {
        fLock.UnlockForReading();
    }
};

class hsLockForWriting
{
    hsReaderWriterLock& fLock;

public:
    hsLockForWriting(hsReaderWriterLock& lock) : fLock(lock)
    {
        fLock.LockForWriting();
    }

    ~hsLockForWriting()
    {
        fLock.UnlockForWriting();
    }
};

#endif
