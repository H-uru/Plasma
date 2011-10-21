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
*   $/Plasma20/Sources/Plasma/CoreLib/hsMalloc.h
*   
***/

#ifndef PLASMA20_SOURCES_PLASMA_CORELIB_HSMALLOC_H
#define PLASMA20_SOURCES_PLASMA_CORELIB_HSMALLOC_H


/****************************************************************************
*
*   Allocation functions
*
***/

#ifdef __cplusplus
extern "C" {
#endif

// MemAlloc flags
extern const unsigned kMemReallocInPlaceOnly;   // use _expand when realloc'ing
extern const unsigned kMemZero;                 // fill allocated memory with zeros
extern const unsigned kMemIgnoreBlock;          // don't track this allocation


void *   MemAlloc (unsigned bytes, unsigned flags, const char file[], int line);
void *   MemDup (const void * ptr, unsigned bytes, unsigned flags, const char file[], int line);
void     MemFree (void * ptr, unsigned flags);
void *   MemRealloc (void * ptr, unsigned bytes, unsigned flags, const char file[], int line);
unsigned MemSize (void * ptr);


/****************************************************************************
*
*   Manipulation functions
*
***/

int  MemCmp (const void * buf1, const void * buf2, unsigned bytes);
void MemCopy (void * dest, const void * source, unsigned bytes);
void MemMove (void * dest, const void * source, unsigned bytes);
void MemSet (void * dest, unsigned value, unsigned bytes);
void MemZero (void * dest, unsigned bytes);


/*****************************************************************************
*
*   Debugging functions
*
***/

void MemDumpAllocReport ();
void MemDumpUsageReport ();
void MemValidateNow ();
void MemSetValidation (unsigned on);
void MemPushDisableTracking ();
void MemPopDisableTracking ();
void MemSetColor (unsigned short color);


#ifdef __cplusplus
}
#endif


/****************************************************************************
*
*   C++ Operators
*
***/

#ifdef __cplusplus

#include <new>

#ifndef _MSC_VER
#define THROW(x) throw(x)
#define THROW_EMPTY throw()
#else
#define THROW(x)
#define THROW_EMPTY
#endif

// standard new and delete
inline void* CDECL operator new (size_t bytes) THROW(std::bad_alloc)
    { return MemAlloc((unsigned)bytes, 0, __FILE__, __LINE__); }
inline void* CDECL operator new [](size_t bytes) THROW(std::bad_alloc)
    { return MemAlloc((unsigned)bytes, 0, __FILE__, __LINE__); }
inline void CDECL operator delete (void * ptr) THROW_EMPTY
    { MemFree(ptr, 0); }
inline void CDECL operator delete [](void * ptr) THROW_EMPTY
    { MemFree(ptr, 0); }

// memcheck-friendly new
inline void* CDECL operator new (size_t bytes, const char file[], unsigned line)
    { return MemAlloc((unsigned)bytes, 0, file, line); }
inline void* CDECL operator new [](size_t bytes, const char file[], unsigned line)
    { return MemAlloc((unsigned)bytes, 0, file, line); }
inline void CDECL operator delete (void * ptr, const char [], unsigned)
    { return MemFree(ptr, 0); }
inline void CDECL operator delete [](void * ptr, const char [], unsigned)
    { return MemFree(ptr, 0); }
#define TRACKED_NEW new(__FILE__, __LINE__)


// placement new
#if defined(_MSC_VER) && !defined(__PLACEMENT_NEW_INLINE)
#define __PLACEMENT_NEW_INLINE
inline void* CDECL operator new (size_t, void * ptr) { return ptr; }
inline void CDECL operator delete (void *, void *) {}
#endif  // ifndef __PLACEMENT_NEW_INLINE

#endif // ifdef __cplusplus


/****************************************************************************
*
*   Macros
*
***/

#define ALLOC(b)                MemAlloc(b, 0, __FILE__, __LINE__)
#define ALLOCZERO(b)            MemAlloc(b, kMemZero, __FILE__, __LINE__)
#define ALLOCFLAGS(b, f)        MemAlloc(b, (f), __FILE__, __LINE__)
#define FREE(p)                 MemFree(p, 0)
#define FREEFLAGS(p, f)         MemFree(p, (f))
#define REALLOC(p, b)           MemRealloc(p, b, 0, __FILE__, __LINE__)
#define REALLOCFLAGS(p, b, f)   MemRealloc(p, b, (f), __FILE__, __LINE__)
#define CALLOC(n, s)            MemAlloc((n)*(s), kMemZero, __FILE__, __LINE__)
#define MEMDUP(s, b)            MemDup(s, b, 0, __FILE__, __LINE__)
#define ZERO(s)                 MemSet(&s, 0, sizeof(s))
#define ZEROPTR(p)              MemSet(p, 0, sizeof(*p))
// Client must #include <malloc.h>
#define  ALLOCA(t, n)           (t *)_alloca((n) * sizeof(t))


#ifdef __cplusplus

#define NEW(t)                  new(MemAlloc(sizeof(t), 0, __FILE__, __LINE__)) t
#define NEWFLAGS(t, f)          new(MemAlloc(sizeof(t), (f), __FILE__, __LINE__)) t
#define NEWZERO(t)              new(MemAlloc(sizeof(t), kMemZero, __FILE__, __LINE__)) t
#define DEL(t)                  delete (t)

#endif // __cplusplus



/****************************************************************************
*
*   TypeInfo
*   (needed for memory leak reporting)
*
***/

#ifdef __cplusplus

#if !defined(HS_NO_TYPEINFO)
#include <typeinfo>
#endif

#endif // ifdef __cplusplus

#endif // PLASMA20_SOURCES_PLASMA_CORELIB_HSMALLOC_H
