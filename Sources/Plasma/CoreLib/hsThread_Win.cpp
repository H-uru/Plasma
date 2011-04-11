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

#include <process.h>

#include "hsThread.h"
#include "hsExceptions.h"
#include "hsMemory.h"

// typedef unsigned int __stdcall (*EntryPtCB)(void*);

static DWORD __stdcall gEntryPoint(void* param)
{
	return ((hsThread*)param)->WinRun();
}

static unsigned int __stdcall gEntryPointBT(void* param)
{
	return ((hsThread*)param)->WinRun();
}

hsThread::hsThread(UInt32 stackSize) : fStackSize(stackSize), fQuit(false), fThreadH(nil), fQuitSemaH(nil)
{
}

hsThread::~hsThread()
{
	this->Stop();
}

void hsThread::Start()
{
	if (fThreadH == nil)
	{
		fQuitSemaH = ::CreateSemaphore(nil, 0, kPosInfinity32, nil);
		if (fQuitSemaH == nil)
			throw hsOSException(-1);

#if 0
		fThreadH = ::CreateThread(nil, fStackSize, gEntryPoint, this, 0, &fThreadId);
#else
		fThreadH = (HANDLE)_beginthreadex(nil, fStackSize, gEntryPointBT, this, 0, (unsigned int*)&fThreadId);
#endif
		if (fThreadH == nil)
			throw hsOSException(-1);
	}
}

void hsThread::Stop()
{
	if (fThreadH != nil)
	{	this->fQuit = true;

		if (fQuitSemaH != nil)
			::WaitForSingleObject(fQuitSemaH, INFINITE);	// wait for the thread to quit

		::CloseHandle(fThreadH);
		fThreadH = nil;
		::CloseHandle(fQuitSemaH);
		fQuitSemaH = nil;
	}
}

DWORD hsThread::WinRun()
{
	DWORD	result = this->Run();

	::ReleaseSemaphore(fQuitSemaH, 1, nil);	// signal that we've quit

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////

void* hsThread::Alloc(size_t size)
{
	return HSMemory::New(size);
}

void hsThread::Free(void* p)
{
	HSMemory::Delete(p);
}

void hsThread::ThreadYield()
{
	//	Don't know how to explicitly yield on WIN32
}

//////////////////////////////////////////////////////////////////////////////

hsMutex::hsMutex()
{
	fMutexH = ::CreateMutex(nil, false, nil);
	if (fMutexH == nil)
		throw hsOSException(-1);
}

hsMutex::~hsMutex()
{
	::CloseHandle(fMutexH);
}

void hsMutex::Lock()
{
	DWORD state = ::WaitForSingleObject(fMutexH, INFINITE);
	hsAssert(state != WAIT_FAILED,"hsMutex::Lock -> Wait Failed");
	hsAssert(state != WAIT_ABANDONED,"hsMutex::Lock -> Abandoned Mutex");
	hsAssert(state != WAIT_TIMEOUT,"hsMutex::Lock -> Infinite Timeout expired?");
}

hsBool hsMutex::TryLock()
{
	DWORD state = ::WaitForSingleObject(fMutexH, 0);
	hsAssert(state != WAIT_ABANDONED,"hsMutex::TryLock -> Abandoned Mutex");
	return state == WAIT_OBJECT_0?true:false;
}

void hsMutex::Unlock()
{
	BOOL result = ::ReleaseMutex(fMutexH);
	hsAssert(result != 0, "hsMutex::Unlock Failed!");

}

//////////////////////////////////////////////////////////////////////////////

hsSemaphore::hsSemaphore(int initialValue, const char *name)
{
	fSemaH = ::CreateSemaphore(nil, initialValue, kPosInfinity32, name);
	if (fSemaH == nil)
		throw hsOSException(-1);
}

hsSemaphore::~hsSemaphore()
{
	::CloseHandle(fSemaH);
}

hsBool hsSemaphore::Wait(hsMilliseconds timeToWait)
{
	if (timeToWait == kPosInfinity32)
		timeToWait = INFINITE;
	
	DWORD result =::WaitForSingleObject(fSemaH, timeToWait);

	if (result == WAIT_OBJECT_0)
		return true;
	else
	{	hsThrowIfFalse(result == WAIT_TIMEOUT);
		return false;
	}
}

void hsSemaphore::Signal()
{
	::ReleaseSemaphore(fSemaH, 1, nil);
}

///////////////////////////////////////////////////////////////

hsEvent::hsEvent()
{
	fEvent = ::CreateEvent(nil,true,false,nil);
	if (fEvent == nil)
		throw hsOSException(-1);
}

hsEvent::~hsEvent()
{
	::CloseHandle(fEvent);
}

hsBool hsEvent::Wait(hsMilliseconds timeToWait)
{
	if (timeToWait == kPosInfinity32)
		timeToWait = INFINITE;
	
	DWORD result =::WaitForSingleObject(fEvent, timeToWait);

	if (result == WAIT_OBJECT_0)
	{
		::ResetEvent(fEvent);
		return true;
	}
	else
	{	hsThrowIfFalse(result == WAIT_TIMEOUT);
		return false;
	}
}

void hsEvent::Signal()
{
	::SetEvent(fEvent);
}