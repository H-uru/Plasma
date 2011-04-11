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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/Win32/pnUtW32Sync.cpp
*   
***/

#include "../../Pch.h"
#pragma hdrstop


/****************************************************************************
*
*   Spin lock functions
*
***/

//===========================================================================
static inline void EnterSpinLock (long * spinLock) {
    for (;;)
        if (*spinLock < 0)
            if (!InterlockedIncrement(spinLock))
                return;
            else
                InterlockedDecrement(spinLock);
}

//===========================================================================
static inline void LeaveSpinLock (long * spinLock) {
    InterlockedDecrement(spinLock);
}


/****************************************************************************
*
*   CLockWaitSet / CLockWaitSetAllocator
*
***/

class CLockWaitSet {
private:
    unsigned m_refCount;
    HANDLE   m_waitEvent;

public:
    LINK(CLockWaitSet) link;

    inline CLockWaitSet ();
    inline ~CLockWaitSet ();
    inline void DecRef ();
    inline void IncRef ();
    inline void Signal ();
    inline void Wait ();
};

class CLockWaitSetAllocator {
private:
    CLockWaitSet                   m_array[256];
    CLockWaitSetAllocator *        m_prev;
    LISTDECL(CLockWaitSet, link)   m_spareList;
    LISTDECL(CLockWaitSet, link)   m_usedList;

    static CLockWaitSetAllocator * s_allocator;
    static long                    s_spinLock;

public:
    CLockWaitSetAllocator (CLockWaitSetAllocator * prev);
    ~CLockWaitSetAllocator ();
    static CLockWaitSet * Alloc ();
    static void Free (CLockWaitSet * waitSet);
    static void __cdecl Shutdown ();
};

CLockWaitSetAllocator * CLockWaitSetAllocator::s_allocator;
long                    CLockWaitSetAllocator::s_spinLock = -1;

//===========================================================================
CLockWaitSet::CLockWaitSet () {
    m_refCount  = 0;
    m_waitEvent = CreateEvent(nil, true, false, nil);
}

//===========================================================================
CLockWaitSet::~CLockWaitSet () {
    ASSERT(!m_refCount);
    CloseHandle(m_waitEvent);
    m_waitEvent = 0;
}

//===========================================================================
void CLockWaitSet::DecRef () {
    ASSERT(m_refCount);
    if (!--m_refCount) {
        ResetEvent(m_waitEvent);
        CLockWaitSetAllocator::Free(this);
    }
}

//===========================================================================
void CLockWaitSet::IncRef () {
    ++m_refCount;
}

//===========================================================================
void CLockWaitSet::Signal () {
    ASSERT(m_refCount);
    SetEvent(m_waitEvent);
}

//===========================================================================
void CLockWaitSet::Wait () {
    ASSERT(m_refCount);
    WaitForSingleObject(m_waitEvent, INFINITE);
}

//===========================================================================
CLockWaitSetAllocator::CLockWaitSetAllocator (CLockWaitSetAllocator * prev) {
    m_prev = prev;
    if (prev) {
        m_spareList.Link(&prev->m_spareList);
        m_usedList.Link(&prev->m_usedList);
    }
    for (unsigned index = arrsize(m_array); index--; )
        m_spareList.Link(&m_array[index]);
}

//===========================================================================
CLockWaitSetAllocator::~CLockWaitSetAllocator () {
    DEL(m_prev);
}

//===========================================================================
CLockWaitSet * CLockWaitSetAllocator::Alloc () {
    EnterSpinLock(&s_spinLock);

    // If there is no active allocator or if the active allocator is full,
    // create a new one
    if (!s_allocator || !s_allocator->m_spareList.Head()) {
        if (!s_allocator)
            atexit(Shutdown);
        s_allocator = NEW(CLockWaitSetAllocator)(s_allocator);
    }

    // Get an available wait set from the active allocator
    CLockWaitSet * waitSet = s_allocator->m_spareList.Head();
    s_allocator->m_usedList.Link(waitSet);

    LeaveSpinLock(&s_spinLock);
    return waitSet;
}

//===========================================================================
void CLockWaitSetAllocator::Free (CLockWaitSet * waitSet) {
    EnterSpinLock(&s_spinLock);

    // Return this wait set to the active allocator's spare list
    ASSERT(s_allocator);
    s_allocator->m_spareList.Link(waitSet);

    LeaveSpinLock(&s_spinLock);
}

//===========================================================================
void CLockWaitSetAllocator::Shutdown () {
    EnterSpinLock(&s_spinLock);

    // Free all allocators
    while (s_allocator) {
        CLockWaitSetAllocator * prev = s_allocator->m_prev;
        DEL(s_allocator);
        s_allocator = prev;
    }

    LeaveSpinLock(&s_spinLock);
}


/****************************************************************************
*
*   CLock
*
***/

//===========================================================================
CLock::CLock () {
    m_waitSet     = nil;
    m_spinLock    = -1;
    m_readerCount = 0;
    m_writerCount = 0;
}

//===========================================================================
CLock::~CLock () {
    ASSERT(!m_waitSet);
    ASSERT(m_spinLock == -1);
    ASSERT(!m_readerCount);
    ASSERT(!m_writerCount);
}

//===========================================================================
void CLock::EnterRead () {
    EnterSpinLock(&m_spinLock);
    for (;;) {

        // If there are no writers, claim this lock for reading
        if (!m_writerCount) {
            ++m_readerCount;
            break;
        }

        // Otherwise, wait until the existing writer releases the lock
        CLockWaitSet * waitSet = m_waitSet = (m_waitSet ? m_waitSet : CLockWaitSetAllocator::Alloc());
        waitSet->IncRef();
        LeaveSpinLock(&m_spinLock);
        waitSet->Wait();
        EnterSpinLock(&m_spinLock);
        waitSet->DecRef();

    }
    LeaveSpinLock(&m_spinLock);
}

//===========================================================================
void CLock::EnterWrite () {
    EnterSpinLock(&m_spinLock);
    for (;;) {

        // If there are no readers or writers, claim this lock for writing
        if (!m_readerCount && !m_writerCount) {
            ++m_writerCount;
            break;
        }

        // Otherwise, wait until the existing writer or all existing readers
        // release the lock
        CLockWaitSet * waitSet = m_waitSet = (m_waitSet ? m_waitSet : CLockWaitSetAllocator::Alloc());
        waitSet->IncRef();
        LeaveSpinLock(&m_spinLock);
        waitSet->Wait();
        EnterSpinLock(&m_spinLock);
        waitSet->DecRef();

    }
    LeaveSpinLock(&m_spinLock);
}

//===========================================================================
void CLock::LeaveRead () {
    EnterSpinLock(&m_spinLock);

    // If this is the last reader, signal waiting threads to try claiming
    // the lock again
    ASSERT(m_readerCount);
    if (!--m_readerCount)
        if (m_waitSet) {
            m_waitSet->Signal();
            m_waitSet = nil;
        }

    LeaveSpinLock(&m_spinLock);
}

//===========================================================================
void CLock::LeaveWrite () {
    EnterSpinLock(&m_spinLock);

    // This is the last writer. Signal waiting threads to try claiming the
    // lock again.
    ASSERT(m_writerCount == 1);
    --m_writerCount;
    if (m_waitSet) {
        m_waitSet->Signal();
        m_waitSet = nil;
    }

    LeaveSpinLock(&m_spinLock);
}


/*****************************************************************************
*
*   CEvent
*
***/

//============================================================================
CEvent::CEvent (
    ECEventResetBehavior resetType,
    bool                 initialSet
) {
    m_handle = CreateEvent(
        nil,    // security attributes
        (resetType == kEventManualReset) ? true : false,
        initialSet,
        nil     // name
    );
}

//============================================================================
CEvent::~CEvent () {
    (void) CloseHandle(m_handle);
}

//============================================================================
void CEvent::Signal () {
    SetEvent(m_handle);
}

//============================================================================
void CEvent::Reset () {
    ResetEvent(m_handle);
}

//============================================================================
bool CEvent::Wait (unsigned waitMs) {
    ThreadAssertCanBlock(__FILE__, __LINE__);
    return WaitForSingleObject(m_handle, waitMs) == WAIT_OBJECT_0;
}


/****************************************************************************
*
* Exported functions
*
***/

//===========================================================================
long AtomicAdd (long * value, long increment) {
    return InterlockedExchangeAdd(value, increment);
}

//===========================================================================
long AtomicSet (long * value, long set) {
    return InterlockedExchange(value, set);
}
