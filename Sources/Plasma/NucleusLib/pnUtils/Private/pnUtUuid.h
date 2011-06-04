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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtUuid.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTUUID_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtUuid.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTUUID_H


/*****************************************************************************
*
*   Types
*
***/

struct Uuid;

/*****************************************************************************
*
*   Constants
*
***/

extern const Uuid kNilGuid;


/*****************************************************************************
*
*   Functions
*
***/

// Using 'Guid' here instead of 'Uuid' to avoid name clash with windows API =(

Uuid            GuidGenerate ();
void            GuidClear (Uuid * uuid);
bool            GuidFromString (const wchar str[], Uuid * uuid);
bool            GuidFromString (const char str[], Uuid * uuid);
int             GuidCompare (const Uuid & a, const Uuid & b);
inline bool     GuidsAreEqual (const Uuid & a, const Uuid & b) { return 0 == GuidCompare(a, b); }
bool            GuidIsNil (const Uuid & uuid);
unsigned        GuidHash (const Uuid & uuid);
const wchar *   GuidToString (const Uuid & uuid, wchar * dst, unsigned chars);  // returns dst
const char *    GuidToString (const Uuid & uuid, char * dst, unsigned chars);   // returns dst
const wchar *	GuidToHex (const Uuid & uuid, wchar * dst, unsigned chars);		// returns dst
bool            GuidFromHex (const byte buf[], unsigned length, Uuid * uuid);


/*****************************************************************************
*
*   Uuid
*
***/

#include <PshPack1.h>
struct Uuid {
	union {
		dword	dwords[4];
		byte	data[16];
	};

    Uuid () {}
    Uuid (const wchar str[]);
    Uuid (const byte buf[], unsigned length);
    operator bool ()                           const { return !GuidIsNil(*this); }
    inline bool operator ! ()                  const { return GuidIsNil(*this); }
    inline bool operator <  (const Uuid & rhs) const { return GuidCompare(*this, rhs) < 0; }
    inline bool operator == (const Uuid & rhs) const { return GuidsAreEqual(*this, rhs); }
    inline bool operator == (int rhs)          const { ref(rhs); ASSERT(!rhs); return GuidsAreEqual(*this, kNilGuid); }
    inline bool operator != (const Uuid & rhs) const { return !GuidsAreEqual(*this, rhs); }
    inline bool operator != (int rhs)          const { ref(rhs); ASSERT(!rhs); return !GuidsAreEqual(*this, kNilGuid); }
};
#include <PopPack.h>


