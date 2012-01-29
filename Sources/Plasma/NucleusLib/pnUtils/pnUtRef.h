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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtRef.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTREF_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtRef.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTREF_H


/****************************************************************************
*
*   AtomicRef
*   Thread safe reference count
*
***/

class AtomicRef {
#ifdef HS_DEBUGGING
    bool    zeroed;
#endif
public:
    inline AtomicRef ()
    : m_ref(0)
    #ifdef HS_DEBUGGING
    , zeroed(false)
    #endif
    {}
    
    inline void AcknowledgeZeroRef () {
        #ifdef HS_DEBUGGING
        zeroed = false;
        #endif
    }

    inline long IncRef () {
        #ifdef HS_DEBUGGING
        ASSERT(!zeroed);
        #endif
        long prev = AtomicAdd(&m_ref, 1);
        #ifdef REFCOUNT_DEBUGGING
        DEBUG_MSG("Inc %p: %u", this, prev+1);
        #endif
        return prev+1;
    }
    inline long IncRef (const char tag[]) {
        #ifdef HS_DEBUGGING
        ASSERT(!zeroed);
        #endif
        long prev = AtomicAdd(&m_ref, 1);
        #ifdef REFCOUNT_DEBUGGING
        DEBUG_MSG("Inc %p %s: %u", this, tag, prev+1);
        #endif
        return prev+1;
    }
    inline long IncRef (unsigned n) {
        #ifdef HS_DEBUGGING
        ASSERT(!zeroed);
        #endif
        long prev = AtomicAdd(&m_ref, n);
        #ifdef REFCOUNT_DEBUGGING
        DEBUG_MSG("Inc %p: %u", this, prev+n);
        #endif
        return prev+n;
    }
    inline long IncRef (unsigned n, const char tag[]) {
        #ifdef HS_DEBUGGING
        ASSERT(!zeroed);
        #endif
        long prev = AtomicAdd(&m_ref, n);
        #ifdef REFCOUNT_DEBUGGING
        DEBUG_MSG("Inc %p %s: %u", this, tag, prev+n);
        #endif
        return prev+n;
    }

    inline long DecRef () {
        #ifdef HS_DEBUGGING
        ASSERT(!zeroed);
        #endif
        long prev;
        if ((prev = AtomicAdd(&m_ref, -1)) == 1) {
            #ifdef HS_DEBUGGING
            zeroed = true;
            #endif
            OnZeroRef();
        }
        #ifdef REFCOUNT_DEBUGGING
        DEBUG_MSG("Dec %p: %u", this, prev-1);
        #endif
        return prev-1;
    }
    inline long DecRef (const char tag[]) {
        #ifdef HS_DEBUGGING
        ASSERT(!zeroed);
        #endif
        long prev;
        if ((prev = AtomicAdd(&m_ref, -1)) == 1) {
            #ifdef HS_DEBUGGING
            zeroed = true;
            #endif
            OnZeroRef();
        }
        #ifdef REFCOUNT_DEBUGGING
        DEBUG_MSG("Dec %p %s: %u", this, tag, prev-1);
        #endif
        return prev-1;
    }

    inline void TransferRef (
        const char oldTag[],
        const char newTag[]
    ) {
        #ifdef HS_DEBUGGING
        ASSERT(!zeroed);
        #endif
        #ifdef REFCOUNT_DEBUGGING
        DEBUG_MSG("Inc %p %s: (xfer)", this, newTag);
        DEBUG_MSG("Dec %p %s: (xfer)", this, oldTag);
        #endif
    }

    inline unsigned GetRefCount () {
        return m_ref;
    }

    inline virtual void OnZeroRef () {
        delete this;
    }

protected:
    inline virtual ~AtomicRef () {
        ASSERT(!m_ref);
    }

private:
    long    m_ref;
};
