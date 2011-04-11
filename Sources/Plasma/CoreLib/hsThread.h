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
#ifndef hsThread_Defined
#define hsThread_Defined

#include "hsTypes.h"

typedef UInt32 hsMilliseconds;


#if HS_BUILD_FOR_MAC
	#include <Multiprocessing.h>
#elif HS_BUILD_FOR_WIN32
	#include "hsWindows.h"
#elif HS_BUILD_FOR_UNIX
	#include <pthread.h>
	#include <semaphore.h>
	//  We can't wait with a timeout with semas
	#define USE_SEMA
	// Linux kernel 2.4 w/ NTPL threading patch and O(1) scheduler
	// seems to have a problem in it's cond_t implementation that
	// causes a hang under heavy load. This is a workaround that
	// uses select() and pipes.
//	#define PSEUDO_EVENT
#endif

class hsThread 
{
public:
#if HS_BUILD_FOR_MAC
	typedef MPTaskId ThreadId;
#elif HS_BUILD_FOR_WIN32
	typedef DWORD ThreadId;
#elif HS_BUILD_FOR_UNIX
	typedef pthread_t ThreadId;
#endif
private:
	hsBool		fQuit;
	UInt32		fStackSize;
#if HS_BUILD_FOR_MAC
	ThreadId	fTaskId;
	MPQueueId	fNotifyQ;
#elif HS_BUILD_FOR_WIN32
	ThreadId	fThreadId;
	HANDLE		fThreadH;
	HANDLE		fQuitSemaH;
#elif HS_BUILD_FOR_UNIX
	ThreadId	fPThread;
	hsBool		fIsValid;
	pthread_mutex_t	fMutex;
#endif
protected:
	hsBool		GetQuit() const { return hsBool(fQuit); }
	void		SetQuit(hsBool value) { fQuit = value; }
public:
	hsThread(UInt32 stackSize = 0);
	virtual 	~hsThread();	// calls Stop()
#if HS_BUILD_FOR_MAC
	ThreadId		GetThreadId() { return fTaskId; }
#error "Mac is Depricated"
#elif HS_BUILD_FOR_WIN32
	ThreadId		GetThreadId() { return fThreadId; }
	static ThreadId	GetMyThreadId() { return GetCurrentThreadId(); }
#elif HS_BUILD_FOR_UNIX
	ThreadId			GetThreadId() { return fPThread; }
	static ThreadId		GetMyThreadId() { return pthread_self(); }
	pthread_mutex_t* GetStartupMutex() { return &fMutex;  }
#endif
				
	virtual hsError Run() = 0;		// override this to do your work
	virtual void	Start();		// initializes stuff and calls your Run() method
	virtual void	Stop(); 	// sets fQuit = true and the waits for the thread to stop
				
	//	Static functions
	static void*	Alloc(size_t size); // does not call operator::new(), may return nil
	static void Free(void* p);		// does not call operator::delete()
	static void ThreadYield();
				
#if HS_BUILD_FOR_WIN32
	DWORD			WinRun();
#endif
};

//////////////////////////////////////////////////////////////////////////////

class hsMutex {
#if HS_BUILD_FOR_MAC
	MPCriticalRegionId	fCriticalRegion;
#elif HS_BUILD_FOR_WIN32
	HANDLE	fMutexH;
#elif HS_BUILD_FOR_UNIX
	pthread_mutex_t	fPMutex;
#endif
public:
	hsMutex();
	virtual ~hsMutex();
	
	void		Lock();
	hsBool		TryLock();
	void		Unlock();
};

class hsTempMutexLock {
	hsMutex*	fMutex;
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
#if HS_BUILD_FOR_MAC
	MPSemaphoreId	fSemaId;
#elif HS_BUILD_FOR_WIN32
	HANDLE	fSemaH;
#elif HS_BUILD_FOR_UNIX
#ifdef USE_SEMA
	sem_t	fPSema;
#else
	pthread_mutex_t	fPMutex;
	pthread_cond_t	fPCond;
	Int32		fCounter;
#endif
#endif
public:
#ifdef HS_BUILD_FOR_WIN32
	hsSemaphore(int initialValue=0, const char *name=nil);
#else
	hsSemaphore(int initialValue=0);
#endif
	~hsSemaphore();
	
	hsBool		Wait(hsMilliseconds timeToWait = kPosInfinity32);
	void		Signal();
};

//////////////////////////////////////////////////////////////////////////////
#if !HS_BUILD_FOR_MAC
class hsEvent
{
#if HS_BUILD_FOR_UNIX
#ifndef PSEUDO_EVENT
	pthread_mutex_t fMutex;
	pthread_cond_t	fCond;
	hsBool	fTriggered;
#else
	enum { kRead, kWrite };
	int		fFds[2];
	hsMutex	fWaitLock;
	hsMutex	fSignalLock;
#endif // PSEUDO_EVENT
#elif HS_BUILD_FOR_WIN32
	HANDLE fEvent;
#endif
public:
	hsEvent();
	~hsEvent();

	hsBool	Wait(hsMilliseconds timeToWait = kPosInfinity32);
	void		Signal();
};
#endif  // HS_BUILD_FOR_MAC

//////////////////////////////////////////////////////////////////////////////
#if !HS_BUILD_FOR_MAC
class hsSleep
{
public:
#if HS_BUILD_FOR_UNIX
	static void Sleep(UInt32 millis);

#elif HS_BUILD_FOR_WIN32
	static void Sleep(UInt32 millis) { ::Sleep(millis); }

#endif
};
#endif  // HS_BUILD_FOR_MAC


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
	int		fReaderCount;
	hsMutex	fReaderCountLock;
	hsMutex	fReaderLock;
	hsSemaphore	fWriterSema;
	Callback *	fCallback;
	char *	fName;
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

