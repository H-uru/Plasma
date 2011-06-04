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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtRef.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTREF_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtRef.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTREF_H


/*****************************************************************************
*
*   Debug macros
*
***/

#ifdef REFCOUNT_DEBUGGING
    #define REFTRACE    DEBUG_MSG
#else
    #define REFTRACE    NULL_STMT
#endif


/****************************************************************************
*
*   AtomicRef
*   Thread safe reference count
*
***/

class AtomicRef {
#ifdef HS_DEBUGGING
	bool	zeroed;
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
        REFTRACE("Inc %p: %u", this, prev+1);
        return prev+1;
    }
    inline long IncRef (const char tag[]) {
		#ifdef HS_DEBUGGING
		ASSERT(!zeroed);
		#endif
        long prev = AtomicAdd(&m_ref, 1);
        ref(tag);
        REFTRACE("Inc %p %s: %u", this, tag, prev+1);
        return prev+1;
    }
    inline long IncRef (unsigned n) {
		#ifdef HS_DEBUGGING
		ASSERT(!zeroed);
		#endif
        long prev = AtomicAdd(&m_ref, n);
        REFTRACE("Inc %p: %u", this, prev+n);
        return prev+n;
    }
    inline long IncRef (unsigned n, const char tag[]) {
		#ifdef HS_DEBUGGING
		ASSERT(!zeroed);
		#endif
        long prev = AtomicAdd(&m_ref, n);
        ref(tag);
        REFTRACE("Inc %p %s: %u", this, tag, prev+n);
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
        REFTRACE("Dec %p: %u", this, prev-1);
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
        ref(tag);
        REFTRACE("Dec %p %s: %u", this, tag, prev-1);
        return prev-1;
    }

    inline void TransferRef (
        const char oldTag[],
        const char newTag[]
    ) {
		#ifdef HS_DEBUGGING
		ASSERT(!zeroed);
		#endif
        ref(oldTag);
        ref(newTag);
        REFTRACE("Inc %p %s: (xfer)", this, newTag);
        REFTRACE("Dec %p %s: (xfer)", this, oldTag);
    }

    inline unsigned GetRefCount () {
        return m_ref;
    }

    inline virtual void OnZeroRef () {
        DEL(this);
    }

protected:
    inline virtual ~AtomicRef () {
        ASSERT(!m_ref);
    }

private:
    long    m_ref;
};
