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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtUuid.h
*   
***/

#ifndef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTUUID_H
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTUUID_H

#include "Pch.h"

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
bool            GuidFromString (const wchar_t str[], Uuid * uuid);
bool            GuidFromString (const char str[], Uuid * uuid);
int             GuidCompare (const Uuid & a, const Uuid & b);
inline bool     GuidsAreEqual (const Uuid & a, const Uuid & b) { return 0 == GuidCompare(a, b); }
bool            GuidIsNil (const Uuid & uuid);
unsigned        GuidHash (const Uuid & uuid);
const wchar_t *   GuidToString (const Uuid & uuid, wchar_t * dst, unsigned chars);  // returns dst
const char *    GuidToString (const Uuid & uuid, char * dst, unsigned chars);   // returns dst
const wchar_t *   GuidToHex (const Uuid & uuid, wchar_t * dst, unsigned chars);     // returns dst
bool            GuidFromHex (const uint8_t buf[], unsigned length, Uuid * uuid);


/*****************************************************************************
*
*   Uuid
*
***/

#pragma pack(push, 1)
struct Uuid {
    union {
        uint32_t   uint32_ts[4];
        uint8_t    data[16];
    };

    Uuid () {}
    Uuid (const wchar_t str[]);
    Uuid (const uint8_t buf[], unsigned length);
    operator bool ()                           const { return !GuidIsNil(*this); }
    inline bool operator ! ()                  const { return GuidIsNil(*this); }
    inline bool operator <  (const Uuid & rhs) const { return GuidCompare(*this, rhs) < 0; }
    inline bool operator == (const Uuid & rhs) const { return GuidsAreEqual(*this, rhs); }
    inline bool operator == (int rhs)          const { ASSERT(!rhs); return GuidsAreEqual(*this, kNilGuid); }
    inline bool operator != (const Uuid & rhs) const { return !GuidsAreEqual(*this, rhs); }
    inline bool operator != (int rhs)          const { ASSERT(!rhs); return !GuidsAreEqual(*this, kNilGuid); }
};
#pragma pack(pop)

#endif
