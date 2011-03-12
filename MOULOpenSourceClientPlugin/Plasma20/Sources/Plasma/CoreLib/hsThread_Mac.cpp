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

extern "C" {
	static OSStatus gEntryPoint(void* param)
	{
		return ((hsThread*)param)->Run();
	}
}

hsThread::hsThread(UInt32 stackSize) : fTaskId(0), fStackSize(stackSize), fQuit(false)
{
	if (MPLibraryIsLoaded() == false)
		throw "MPLibraryIsLoaded() returned false";
}

hsThread::~hsThread()
{
	this->Stop();
}

void hsThread::Start()
{
	if (fTaskId == 0)
	{	OSStatus	status = ::MPCreateQueue(&fNotifyQ);
		hsThrowIfOSErr(status);

		status = ::MPCreateTask(gEntryPoint, this, fStackSize, fNotifyQ, nil, nil, 0, &fTaskId);
		if (status)
		{	::MPDeleteQueue(fNotifyQ);
			throw hsOSException(status);
		}
	}
	else
		hsDebugMessage("Calling hsThread::Start() more than once", 0);
}

void hsThread::Stop()
{
	if (fTaskId)
	{	this->fQuit = true;

		OSStatus	status = ::MPTerminateTask(fTaskId, 0);
		hsThrowIfOSErr(status);
		
		//	Wait for the task to tell us that its actually quit
		status = ::MPWaitOnQueue(fNotifyQ, nil, nil, nil, kDurationForever);
		hsThrowIfOSErr(status);

		status = ::MPDeleteQueue(fNotifyQ);
		hsThrowIfOSErr(status);
		
		fTaskId = 0;
	}
}

//////////////////////////////////////////////////////////////////////////////

void* hsThread::Alloc(size_t size)
{
	return ::MPAllocate(size);
}

void hsThread::Free(void* p)
{
	if (p)
		::MPFree(p);
}

void hsThread::ThreadYield()
{
	::MPYield();
}

//////////////////////////////////////////////////////////////////////////////

hsMutex::hsMutex()
{
	OSStatus status = ::MPCreateCriticalRegion(&fCriticalRegion);
	hsThrowIfOSErr(status);
}

hsMutex::~hsMutex()
{
	OSStatus	status = ::MPDeleteCriticalRegion(fCriticalRegion);
	hsThrowIfOSErr(status);
}

void hsMutex::Lock()
{
	OSStatus	status = ::MPEnterCriticalRegion(fCriticalRegion, kDurationForever);
	hsThrowIfOSErr(status);
}

void hsMutex::Unlock()
{
	OSStatus	status = ::MPExitCriticalRegion(fCriticalRegion);
	hsThrowIfOSErr(status);
}

//////////////////////////////////////////////////////////////////////////////

hsSemaphore::hsSemaphore(int initialValue)
{
	OSStatus	status = MPCreateSemaphore(kPosInfinity32, initialValue, &fSemaId);
	hsThrowIfOSErr(status);
}

hsSemaphore::~hsSemaphore()
{
	OSStatus	status = MPDeleteSemaphore(fSemaId);
	hsThrowIfOSErr(status);
}

hsBool hsSemaphore::Wait(hsMilliseconds timeToWait)
{
	Duration	duration;

	if (timeToWait == kPosInfinity32)
		duration = kDurationForever;
	else
		duration = 0;	// THEY DON'T IMPLEMENT delay times yet !!!

	OSStatus	status = MPWaitOnSemaphore(fSemaId, duration);
/*
	if (status == kMPTimeoutErr)
		return false;
*/
	hsThrowIfOSErr(status);
	return true;
}

void hsSemaphore::Signal()
{
	OSStatus	status = MPSignalSemaphore(fSemaId);
	hsThrowIfOSErr(status);
}

