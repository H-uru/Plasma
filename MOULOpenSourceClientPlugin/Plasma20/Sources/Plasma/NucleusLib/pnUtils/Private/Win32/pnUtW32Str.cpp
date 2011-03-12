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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/Win32/pnUtW32Str.cpp
*   
***/

#include "../../Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Exports
*
***/

//===========================================================================
unsigned StrToAnsi (char * dest, const wchar source[], unsigned destChars) {
    return StrToAnsi(dest, source, destChars, CP_ACP);
}

//===========================================================================
unsigned StrToAnsi (char * dest, const wchar source[], unsigned destChars, unsigned codePage) {
    ASSERT(destChars != (unsigned)-1);
    ASSERT(dest      != nil);

    int result = WideCharToMultiByte(codePage, 0, source, -1, dest, destChars, nil, nil);
    if (result)
        return result - 1;  // return number of characters not including null terminator
    else if (destChars) {
        dest[destChars - 1] = 0;  // null terminate the destination buffer
        return destChars - 1;
    }
    else
        return 0;
}

//===========================================================================
unsigned StrToUnicode (wchar * dest, const char source[], unsigned destChars) {
    return StrToUnicode(dest, source, destChars, CP_ACP);
}

//===========================================================================
unsigned StrToUnicode (wchar * dest, const char source[], unsigned destChars, unsigned codePage) {
    ASSERT(destChars != (unsigned)-1);
    ASSERT(dest      != nil);

    int result = MultiByteToWideChar(codePage, 0, source, -1, dest, destChars);
    if (result)
        return result - 1;  // return number of characters not including null terminator
    else if (destChars) {
        dest[destChars - 1] = 0;  // null terminate the destination buffer
        return destChars - 1;
    }
    else
        return 0;
}
