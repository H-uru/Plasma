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

typedef uint32_t hsMilliseconds;

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
public:
#if HS_BUILD_FOR_WIN32
    typedef uint32_t ThreadId;
#elif HS_BUILD_FOR_UNIX
    typedef pthread_t ThreadId;
#endif
private:
    bool        fQuit;
    uint32_t    fStackSize;
#if HS_BUILD_FOR_WIN32
    ThreadId    fThreadId;
    HANDLE      fThreadH;
    HANDLE      fQuitSemaH;
#elif HS_BUILD_FOR_UNIX
    ThreadId    fPThread;
    bool        fIsValid;
    pthread_mutex_t fMutex;
#endif
protected:
    bool        GetQuit() const { return fQuit; }
    void        SetQuit(bool value) { fQuit = value; }
public:
    hsThread(uint32_t stackSize = 0);
    virtual     ~hsThread();    // calls Stop()
#if HS_BUILD_FOR_WIN32
    ThreadId        GetThreadId() { return fThreadId; }
    static ThreadId GetMyThreadId();
#elif HS_BUILD_FOR_UNIX
    ThreadId            GetThreadId() { return fPThread; }
    static ThreadId     GetMyThreadId() { return pthread_self(); }
    pthread_mutex_t* GetStartupMutex() { return &fMutex;  }
#endif
                
    virtual hsError Run() = 0;      // override this to do your work
    virtual void    Start();        // initializes stuff and calls your Run() method
    virtual void    Stop();     // sets fQuit = true and the waits for the thread to stop
                
    //  Static functions
    static void*    Alloc(size_t size); // does not call operator::new(), may return nil
    static void Free(void* p);      // does not call operator::delete()
    static void ThreadYield();
};

//////////////////////////////////////////////////////////////////////////////

class hsMutex {
#if HS_BUILD_FOR_WIN32
    HANDLE  fMutexH;
#elif HS_BUILD_FOR_UNIX
    pthread_mutex_t fPMutex;
#endif
public:
    hsMutex();
    virtual ~hsMutex();

#ifdef HS_BUILD_FOR_WIN32
    HANDLE GetHandle() const { return fMutexH; }
#endif

    void        Lock();
    bool        TryLock();
    void        Unlock();
};

class hsTempMutexLock {
    hsMutex*    fMutex;
public:
    hsTempMutexLock(hsMutex* mutex) : fMutex(mutex)
    {
        fMutex->Lock();
    }
    hsTempMutexLock(hsMutex& mutex) : fMutex(&mutex)
    {
        fMutex->Lock();
    }
    ~hsTempMutexLock()
    {
        fMutex->Unlock();
    }
};

//////////////////////////////////////////////////////////////////////////////

class hsSemaphore {
#if HS_BUILD_FOR_WIN32
    HANDLE  fSemaH;
#elif HS_BUILD_FOR_UNIX
#ifdef USE_SEMA
    sem_t*  fPSema;
    bool    fNamed;
#else
    pthread_mutex_t fPMutex;
    pthread_cond_t  fPCond;
    int32_t       fCounter;
#endif
#endif
public:
    hsSemaphore(int initialValue=0, const char* name=nil);
    ~hsSemaphore();

#ifdef HS_BUILD_FOR_WIN32
    HANDLE GetHandle() const { return fSemaH; }
#endif

    bool        TryWait();
    bool        Wait(hsMilliseconds timeToWait = kPosInfinity32);
    void        Signal();
};

//////////////////////////////////////////////////////////////////////////////
class hsEvent
{
#if HS_BUILD_FOR_UNIX
#ifndef PSEUDO_EVENT
    pthread_mutex_t fMutex;
    pthread_cond_t  fCond;
    bool  fTriggered;
#else
    enum { kRead, kWrite };
    int     fFds[2];
    hsMutex fWaitLock;
    hsMutex fSignalLock;
#endif // PSEUDO_EVENT
#elif HS_BUILD_FOR_WIN32
    HANDLE fEvent;
#endif
public:
    hsEvent();
    ~hsEvent();

#ifdef HS_BUILD_FOR_WIN32
    HANDLE GetHandle() const { return fEvent; }
#endif

    bool  Wait(hsMilliseconds timeToWait = kPosInfinity32);
    void  Signal();
};

//////////////////////////////////////////////////////////////////////////////
class hsSleep
{
public:
    static void Sleep(uint32_t millis);
};

//////////////////////////////////////////////////////////////////////////////
// Allows multiple readers, locks out readers for writing.

class hsReaderWriterLock
{
public:
    struct Callback
    {
        virtual void OnLockingForRead( hsReaderWriterLock * lock ) {}
        virtual void OnLockedForRead( hsReaderWriterLock * lock ) {}
        virtual void OnUnlockingForRead( hsReaderWriterLock * lock ) {}
        virtual void OnUnlockedForRead( hsReaderWriterLock * lock ) {}
        virtual void OnLockingForWrite( hsReaderWriterLock * lock ) {}
        virtual void OnLockedForWrite( hsReaderWriterLock * lock ) {}
        virtual void OnUnlockingForWrite( hsReaderWriterLock * lock ) {}
        virtual void OnUnlockedForWrite( hsReaderWriterLock * lock ) {}
    };
    hsReaderWriterLock( const char * name="<unnamed>", Callback * cb=nil );
    ~hsReaderWriterLock();
    void LockForReading();
    void UnlockForReading();
    void LockForWriting();
    void UnlockForWriting();
    const char * GetName() const { return fName; }

private:
    int     fReaderCount;
    hsMutex fReaderCountLock;
    hsMutex fReaderLock;
    hsSemaphore fWriterSema;
    Callback *  fCallback;
    char *  fName;
};

class hsLockForReading
{
    hsReaderWriterLock * fLock;
public:
    hsLockForReading( hsReaderWriterLock & lock ): fLock( &lock )
    {
        fLock->LockForReading();
    }
    hsLockForReading( hsReaderWriterLock * lock ): fLock( lock )
    {
        fLock->LockForReading();
    }
    ~hsLockForReading()
    {
        fLock->UnlockForReading();
    }
};

class hsLockForWriting
{
    hsReaderWriterLock * fLock;
public:
    hsLockForWriting( hsReaderWriterLock & lock ): fLock( &lock )
    {
        fLock->LockForWriting();
    }
    hsLockForWriting( hsReaderWriterLock * lock ): fLock( lock )
    {
        fLock->LockForWriting();
    }
    ~hsLockForWriting()
    {
        fLock->UnlockForWriting();
    }
};

#endif

