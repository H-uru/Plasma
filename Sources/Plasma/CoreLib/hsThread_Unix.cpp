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
#include "hsThread.h"
#include "hsExceptions.h"
#include <sys/errno.h>
#include <string.h>

#define NO_POSIX_CLOCK 1

#if NO_POSIX_CLOCK
#include <sys/time.h>
#include <unistd.h>
#define CLOCK_REALTIME 0

//
// A linux hack b/c we're not quite POSIX
//
int clock_gettime(int clocktype, struct timespec* ts)
{
  struct timezone tz;
  struct timeval tv;

  int result = gettimeofday(&tv, &tz);
  ts->tv_sec = tv.tv_sec;
  ts->tv_nsec = tv.tv_usec * 1000 + 500;  // sice we're losing accuracy round up by 500 nanos

  return result;
}

#endif

extern "C" {
	static void* gEntryPoint(void* param)
	{
		pthread_mutex_lock(((hsThread*)param)->GetStartupMutex());
		void* ret = (void*)((hsThread*)param)->Run();
		pthread_mutex_unlock(((hsThread*)param)->GetStartupMutex());
		pthread_exit(ret);
		return ret;
	}
}

#define kInvalidStackSize	UInt32(~0)

hsThread::hsThread(UInt32 stackSize) : fStackSize(stackSize), fQuit(false)
{
	fIsValid = false;
	pthread_mutex_init(&fMutex,nil);
}

hsThread::~hsThread()
{
	this->Stop();
}

void hsThread::Start()
{
	if (fIsValid == false)
	{
		pthread_mutex_lock(GetStartupMutex());

		int status = ::pthread_create(&fPThread, nil, gEntryPoint, this);
		pthread_mutex_unlock(GetStartupMutex());

		hsThrowIfOSErr(status);

		fIsValid = true;
	}
	else
		hsDebugMessage("Calling hsThread::Start() more than once", 0);
}

void hsThread::Stop()
{
	if (fIsValid)
	{	this->fQuit = true;

		int	status = ::pthread_join(fPThread, nil);
		hsThrowIfOSErr(status);

		fIsValid = false;
	}
}

//////////////////////////////////////////////////////////////////////////////

void* hsThread::Alloc(size_t size)
{
	return ::malloc(size);
}

void hsThread::Free(void* p)
{
	if (p)
		::free(p);
}

void hsThread::ThreadYield()
{
//	::sched_yield();
}

//////////////////////////////////////////////////////////////////////////////

//#define MUTEX_TIMING
#ifdef MUTEX_TIMING

#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include "hsWide.h"


static FILE * gMutexTimerFile = nil;
static void InitMutexTimerFile()
{
	if ( !gMutexTimerFile )
	{
		gMutexTimerFile = fopen( "log/MutexTimes.log", "wt" );
		if ( gMutexTimerFile )
			fprintf( gMutexTimerFile, "------------------------------------\n" );
	}
}

#endif

//#define EVENT_LOGGING
#ifdef EVENT_LOGGING

#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include "../NucleusLib/inc/hsTimer.h"


static FILE * gEventLoggingFile = nil;
static void InitEventLoggingFile()
{
	if ( !gEventLoggingFile )
	{
		char fname[256];
		sprintf(fname,"log/Events-%u.log",getpid());
		gEventLoggingFile = fopen( fname, "wt" );
		if ( gEventLoggingFile )
			fprintf( gEventLoggingFile, "------------------------------------\n" );
	}
}

#endif

hsMutex::hsMutex()
{

#ifdef MUTEX_TIMING
	InitMutexTimerFile();
#endif

	// create mutex attributes
	pthread_mutexattr_t attr;
	int	status = ::pthread_mutexattr_init(&attr);
	hsThrowIfOSErr(status);

	// make the mutex attributes recursive
	status = ::pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);
	hsThrowIfOSErr(status);

	//init the mutex
	status = ::pthread_mutex_init(&fPMutex, &attr);
	hsThrowIfOSErr(status);

	// destroy the attributes
	status = ::pthread_mutexattr_destroy(&attr);
	hsThrowIfOSErr(status);
}

hsMutex::~hsMutex()
{
	int	status = ::pthread_mutex_destroy(&fPMutex);
	hsThrowIfOSErr(status);
}

void hsMutex::Lock()
{
#ifdef MUTEX_TIMING
# ifndef HS_DEBUGGING
	timeval tv;
	hsWide start;
	gettimeofday( &tv, nil );
	start.Mul( tv.tv_sec, 1000000 )->Add( tv.tv_usec );
# endif
#endif

	int	status = ::pthread_mutex_lock(&fPMutex);
	hsThrowIfOSErr(status);

#ifdef MUTEX_TIMING
# ifndef HS_DEBUGGING
	hsWide diff;
	gettimeofday( &tv, nil );
	diff.Mul( tv.tv_sec, 1000000 )->Add( tv.tv_usec )->Sub( &start )->Div( 1000000 );
	double duration = diff.AsDouble();
	if ( gMutexTimerFile && duration>0.005 )
	{
		time_t t;
		time( &t );
		struct tm *now = localtime( &t );
		char tmp[30];
		strftime( tmp, 30, "%c", now );
		fprintf( gMutexTimerFile, "[%s] [%lu:%lu] %f\n", tmp, getpid(), hsThread::GetMyThreadId(), duration );
	}
# endif
#endif
}

hsBool hsMutex::TryLock()
{
	int	status = ::pthread_mutex_trylock(&fPMutex);
	hsThrowIfOSErr(status);
	return status==EBUSY?false:true;
}

void hsMutex::Unlock()
{
	int	status = ::pthread_mutex_unlock(&fPMutex);
	hsThrowIfOSErr(status);
}

/////////////////////////////////////////////////////////////////////////////

hsSemaphore::hsSemaphore(int initialValue)
{
#ifdef USE_SEMA
	int	shared = 0;	// 1 if sharing between processes
	int	status = ::sem_init(&fPSema, shared, initialValue);
	hsThrowIfOSErr(status);
#else
	int	status = ::pthread_mutex_init(&fPMutex, nil);
	hsThrowIfOSErr(status);

	status = ::pthread_cond_init(&fPCond, nil);
	hsThrowIfOSErr(status);

	fCounter = initialValue;
#endif
}

hsSemaphore::~hsSemaphore()
{
#ifdef USE_SEMA
	int	status = ::sem_destroy(&fPSema);
	hsThrowIfOSErr(status);
#else
	int	status = ::pthread_cond_destroy(&fPCond);
	hsThrowIfOSErr(status);

	status = ::pthread_mutex_destroy(&fPMutex);
	hsThrowIfOSErr(status);
#endif
}

hsBool hsSemaphore::Wait(hsMilliseconds timeToWait)
{
#ifdef USE_SEMA  // SHOULDN'T THIS USE timeToWait??!?!? -rje
	hsAssert( timeToWait==kPosInfinity32, "sem_t does not support wait with timeout. #undef USE_SEMA and recompile." );
	int	status = ::sem_wait(&fPSema);
	hsThrowIfOSErr(status);
	return true;
#else
	hsBool	retVal = true;
	int	status = ::pthread_mutex_lock(&fPMutex);
	hsThrowIfOSErr(status);

	if (timeToWait == kPosInfinity32)
	{	while (fCounter == 0)
		{	status = ::pthread_cond_wait(&fPCond, &fPMutex);
			hsThrowIfOSErr(status);
		}
	}
	else
	{	timespec spec;
		int	 result;

		result = ::clock_gettime(CLOCK_REALTIME, &spec);
		hsThrowIfFalse(result == 0);

		spec.tv_sec += timeToWait / 1000;
		spec.tv_nsec += (timeToWait % 1000) * 1000 * 1000;
		while (spec.tv_nsec >= 1000 * 1000 * 1000)
		{	spec.tv_sec += 1;
			spec.tv_nsec -= 1000 * 1000 * 1000;
		}

		while (fCounter == 0)
		{	status = ::pthread_cond_timedwait(&fPCond, &fPMutex, &spec);
			if (status == ETIMEDOUT)
			{	retVal = false;
				goto EXIT;
			}
			hsThrowIfOSErr(status);
		}
	}

	hsAssert(fCounter > 0, "oops");
	fCounter -= 1;
EXIT:
	status = ::pthread_mutex_unlock(&fPMutex);
	hsThrowIfOSErr(status);
	return retVal;
#endif
}

void hsSemaphore::Signal()
{
#ifdef USE_SEMA
	int	status = ::sem_post(&fPSema);
	hsThrowIfOSErr(status);
#else
	int	status = ::pthread_mutex_lock(&fPMutex);
	hsThrowIfOSErr(status);

	fCounter += 1;

	status = ::pthread_mutex_unlock(&fPMutex);
	hsThrowIfOSErr(status);

	status = ::pthread_cond_signal(&fPCond);
	hsThrowIfOSErr(status);
#endif
}


///////////////////////////////////////////////////////////////

#ifndef PSEUDO_EVENT

hsEvent::hsEvent() : fTriggered(false)
{
#ifdef EVENT_LOGGING
	InitEventLoggingFile();
#endif
	int	status = ::pthread_mutex_init(&fMutex, nil);
	hsAssert(status == 0, "hsEvent Mutex Init");
	hsThrowIfOSErr(status);

	//	fCond = PTHREAD_COND_INITIALIZER;
	status = ::pthread_cond_init(&fCond, nil);
	hsAssert(status == 0, "hsEvent Cond Init");
	hsThrowIfOSErr(status);
}

hsEvent::~hsEvent()
{
	int	status = ::pthread_cond_destroy(&fCond);
	hsAssert(status == 0, "hsEvent Cond De-Init");
	hsThrowIfOSErr(status);

	status = ::pthread_mutex_destroy(&fMutex);
	hsAssert(status == 0, "hsEvent Mutex De-Init");
	hsThrowIfOSErr(status);
}

hsBool hsEvent::Wait(hsMilliseconds timeToWait)
{
	hsBool	retVal = true;
	int	status = ::pthread_mutex_lock(&fMutex);
	hsAssert(status == 0, "hsEvent Mutex Lock");
	hsThrowIfOSErr(status);

#ifdef EVENT_LOGGING
	fprintf(gEventLoggingFile,"Event: %p - In Wait (pre trig check), Triggered: %d, t=%f\n",this,fTriggered,hsTimer::GetSeconds());
#endif

	if ( !fTriggered )
	{
		if (timeToWait == kPosInfinity32)
		{
			status = ::pthread_cond_wait(&fCond, &fMutex);
			hsAssert(status == 0, "hsEvent Cond Wait");
			hsThrowIfOSErr(status);
		}
		else
		{	timespec spec;
			int	 result;

			result = ::clock_gettime(CLOCK_REALTIME, &spec);
			hsThrowIfFalse(result == 0);

			spec.tv_sec += timeToWait / 1000;
			spec.tv_nsec += (timeToWait % 1000) * 1000 * 1000;
			while (spec.tv_nsec >= 1000 * 1000 * 1000)
			{	spec.tv_sec += 1;
				spec.tv_nsec -= 1000 * 1000 * 1000;
			}

			status = ::pthread_cond_timedwait(&fCond, &fMutex, &spec);

			if (status == ETIMEDOUT)
			{
				// It's a conditional paired with a variable!
				//   Pthread docs all use a variable in conjunction with the conditional
				retVal = fTriggered;
				status = 0;
#ifdef EVENT_LOGGING
				fprintf(gEventLoggingFile,"Event: %p - In Wait (wait timed out), Triggered: %d, t=%f\n",this,fTriggered,hsTimer::GetSeconds());
#endif
			}
			else
			{
#ifdef EVENT_LOGGING
				fprintf(gEventLoggingFile,"Event: %p - In Wait (wait recvd signal), Triggered: %d, t=%f\n",this,fTriggered,hsTimer::GetSeconds());
#endif
			}

			hsAssert(status == 0, "hsEvent Cond Wait");
			hsThrowIfOSErr(status);
		}
	}
	else
	{
#ifdef EVENT_LOGGING
		fprintf(gEventLoggingFile,"Event: %p - In Wait (post triggerd), Triggered: %d, t=%f\n",this,fTriggered,hsTimer::GetSeconds());
#endif
	}

	fTriggered = false;
	status = ::pthread_mutex_unlock(&fMutex);
	hsAssert(status == 0, "hsEvent Mutex Unlock");
	hsThrowIfOSErr(status);
	return retVal;
}

void hsEvent::Signal()
{
	int	status = ::pthread_mutex_lock(&fMutex);
	hsAssert(status == 0, "hsEvent Mutex Lock");
	hsThrowIfOSErr(status);
#ifdef EVENT_LOGGING
	fprintf(gEventLoggingFile,"Event: %p - In Signal, Triggered: %d, t=%f\n",this,fTriggered,hsTimer::GetSeconds());
#endif
	fTriggered = true;
	status = ::pthread_cond_broadcast(&fCond);
	hsAssert(status == 0, "hsEvent Cond Broadcast");
	hsThrowIfOSErr(status);
	status = ::pthread_mutex_unlock(&fMutex);
	hsAssert(status == 0, "hsEvent Mutex Unlock");
	hsThrowIfOSErr(status);
}

#else

hsEvent::hsEvent()
{
	pipe( fFds );
}

hsEvent::~hsEvent()
{
	close( fFds[kRead] );
	close( fFds[kWrite] );
}

hsBool hsEvent::Wait( hsMilliseconds timeToWait )
{
	hsTempMutexLock lock( fWaitLock );

	fd_set	fdset;
	FD_ZERO( &fdset );
	FD_SET( fFds[kRead], &fdset );

	int ans;
	if( timeToWait==kPosInfinity32 )                
	{
		ans = select( fFds[kRead]+1, &fdset, nil, nil, nil );
	}
	else
	{
		struct timeval tv;
		tv.tv_sec = timeToWait / 1000;
		tv.tv_usec = ( timeToWait % 1000 ) * 1000;
		
		ans = select( fFds[kRead]+1, &fdset, nil, nil, &tv );
	}

	bool signaled = false;

	if ( ans>0 )
	{
		char buf[2];
		int n = read( fFds[kRead], buf, 1 );
		signaled = ( n==1 );
	}

	return signaled;
}

void hsEvent::Signal()
{
	hsTempMutexLock lock( fSignalLock );
	write( fFds[kWrite], "*", 1 );
}


#endif

void hsSleep::Sleep(UInt32 millis)
{
	UInt32 secs = millis / 1000;
	if (secs > 0)
	{
		millis %= 1000;
		::sleep(secs);
	}
	usleep(millis*1000);
}
