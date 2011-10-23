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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtUuid.cpp
*   
***/

#include "../Pch.h"
#pragma hdrstop


const Uuid kNilGuid;


/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
Uuid::Uuid (const wchar str[]) {
    
    GuidFromString(str, this);
}

//============================================================================
Uuid::Uuid (const byte buf[], unsigned length) {
    
    GuidFromHex(buf, length, this);
}

//============================================================================
unsigned GuidHash (const Uuid & uuid) {
    
    CHashValue hash(&uuid.data, sizeof(uuid.data));
    return hash.GetHash();
}

//============================================================================
static const wchar s_hexChars[] = L"0123456789ABCDEF";
const wchar * GuidToHex (const Uuid & uuid, wchar * dst, unsigned chars) {
    
    wchar * str = ALLOCA(wchar, sizeof(uuid.data) * 2 + 1);
    wchar * cur = str;
    
    for (unsigned i = 0; i < sizeof(uuid.data); ++i) {
        *cur++ = s_hexChars[(uuid.data[i] >> 4) & 0x0f];
        *cur++ = s_hexChars[uuid.data[i] & 0x0f];
    }
    *cur = 0;
    
    StrCopy(dst, str, chars);
    return dst;
}

//============================================================================
bool GuidFromHex (const byte buf[], unsigned length, Uuid * uuid) {

    ASSERT(length == msizeof(Uuid, data));
    MemCopy(uuid->data, buf, msizeof(Uuid, data));
    return true;
}