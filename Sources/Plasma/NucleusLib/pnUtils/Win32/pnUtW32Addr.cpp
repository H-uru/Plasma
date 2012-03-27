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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/Win32/pnUtW32Addr.cpp
*   
***/

#include "../pnUtils.h"


/*****************************************************************************
*
*   Internal functions
*
***/

//===========================================================================
static NetAddressNode NodeFromString (const wchar_t * string[]) {
    // skip leading whitespace
    const wchar_t * str = *string;
    while (iswspace(*str))
        ++str;

    // This function handles partial ip addresses (61.33)
    // as well as full dotted quads. The address can be
    // terminated by whitespace or ':' as well as '\0'
    uint8_t data[4];
    * (uint32_t *) data = 0;
    for (unsigned i = sizeof(data); i--; ) {
        if (!iswdigit(*str))
            return (unsigned)-1;

        unsigned value = StrToUnsigned(str, &str, 10);
        if (value >= 256)
            return (unsigned)-1;
        data[i] = (uint8_t) value;

        if (!*str || (*str == ':') || iswspace(*str))
            break;

        static const wchar_t s_separator[] = L"\0...";
        if (*str++ != s_separator[i])
            return (unsigned)-1;
    }

    *string = str;
    return * (NetAddressNode *) &data[0];
}


/*****************************************************************************
*
*   Exports
*
***/

//===========================================================================
bool NetAddressFromString (NetAddress * addr, const wchar_t str[], uint16_t defaultPort) {
    ASSERT(addr);
    ASSERT(str);

    for (;;) {
        NetAddressNode node = NodeFromString(&str);
        if (node == (unsigned)-1)
            break;

        if (*str == L':')
            defaultPort = StrToUnsigned(str + 1, nil, 10);
   
        addr->SetPort((uint16_t)defaultPort);
        addr->SetHostLE(node);
        return true;
    }

    // address already zeroed
    return false;
}
