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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtSpareList.cpp
*   
***/

#include "../Pch.h"
#pragma hdrstop


/****************************************************************************
*
*   Exported functions
*
***/

//===========================================================================
CBaseSpareList::CBaseSpareList ()
:   m_allocHead(nil),
    m_spareHead(nil),
    m_chunkSize(0)
{
    #ifdef SPARELIST_TRACK_MEMORY
    m_unfreedObjects = 0;
    #endif
}

//===========================================================================
void * CBaseSpareList::Alloc (unsigned objectSize, const char typeName[]) {
    // if there aren't any spare nodes available then make more
    if (!m_spareHead)
        GrowSpareList(objectSize, typeName);

    // dequeue the head of the spare list
    void * const object = m_spareHead;
    m_spareHead = m_spareHead->spareNext;
    #ifdef SPARELIST_TRACK_MEMORY
    m_unfreedObjects++;
    #endif

    // initialize memory to a freaky value in debug mode
    #ifdef HS_DEBUGGING
    MemSet(object, (byte) ((unsigned) object >> 4), objectSize);
    #endif

    return object;
}

//===========================================================================
void CBaseSpareList::Free (void * object, unsigned objectSize) {
    // initialize memory to a freaky value in debug mode
    #ifdef HS_DEBUGGING
    MemSet(object, (byte) ((unsigned) object >> 4), objectSize);
    #else
    ref(objectSize);
    #endif

    // link memory block onto head of spare list
    ((SpareNode *) object)->spareNext = m_spareHead;
    m_spareHead = (SpareNode *) object;
    #ifdef SPARELIST_TRACK_MEMORY
    m_unfreedObjects--;
    #endif
}

//===========================================================================
void CBaseSpareList::GrowSpareList (unsigned objectSize, const char typeName[]) {
    // Grow the allocation by a substantial amount each time
    // to reduce the time spent in memory managament
    m_chunkSize *= 2;
    const unsigned MIN_ALLOC = max(1,       256/objectSize);
    const unsigned MAX_ALLOC = max(512, 32*1024/objectSize);
    if (m_chunkSize < MIN_ALLOC)
        m_chunkSize = MIN_ALLOC;
    else if (m_chunkSize > MAX_ALLOC)
        m_chunkSize = MAX_ALLOC;

    // allocate a block of memory to hold a bunch
    // of T-objects, but allocate them as "raw" memory
    AllocNode * allocNode = (AllocNode *) MemAlloc(
        sizeof(AllocNode) + objectSize * m_chunkSize,
        0,          // flags
        typeName,   // file
        0           // line
    );

    // link allocation onto head of allocation list
    allocNode->allocNext = m_allocHead;
    m_allocHead = allocNode;

    // chain newly created raw memory units together onto the spare list
    SpareNode * spareCurr = (SpareNode *) (allocNode + 1);
    SpareNode * spareEnd  = (SpareNode *) ((byte *) spareCurr + objectSize * m_chunkSize);
    do {
        spareCurr->spareNext = m_spareHead;
        m_spareHead = spareCurr;
        spareCurr = (SpareNode *) ((byte *) spareCurr + objectSize);
    } while (spareCurr < spareEnd);
}

//===========================================================================
void CBaseSpareList::CleanUp (const char typeName[]) {
    // warn of resource leaks
    #ifdef SPARELIST_TRACK_MEMORY
    if (m_unfreedObjects && !ErrorGetOption(kErrOptDisableMemLeakChecking)) {
        #ifdef CLIENT
        {
            char buffer[256];
            StrPrintf(buffer, arrsize(buffer), "Memory leak: %s", typeName);
            FATAL(buffer);
        }
        #else
        {
            DEBUG_MSG("Memory leak: %s", typeName);
        }
        #endif
    }
    #else
    ref(typeName);
    #endif

    // walk chain of AllocNodes and free each of them
    while (m_allocHead) {
        AllocNode * allocNext = m_allocHead->allocNext;
        FREE(m_allocHead);
        m_allocHead = allocNext;
    }

    m_spareHead = nil;
}
