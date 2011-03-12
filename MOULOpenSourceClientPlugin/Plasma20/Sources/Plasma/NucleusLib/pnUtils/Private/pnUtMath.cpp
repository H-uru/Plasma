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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtMath.cpp
*   
***/

#include "../Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Exported bit manipulation functions
*
***/

//===========================================================================
#ifndef _M_IX86

unsigned MathHighBitPos (dword val) {
    ASSERT(val);
    double f = (double)val;
    return (*((dword *)&f + 1) >> 20) - 1023;
}

#else

__declspec(naked) unsigned __fastcall MathHighBitPos (dword) {
    __asm {
        bsr     eax, ecx
        ret     0
    };
}

#endif

//===========================================================================
#ifndef _M_IX86

unsigned MathLowBitPos (dword val) {
    val &= ~(val - 1);  // clear all but the low bit
    ASSERT(val);
    double f = (double)val;
    return (*((dword *)&f + 1) >> 20) - 1023;
}

#else

__declspec(naked) unsigned __fastcall MathLowBitPos (dword) {
    __asm {
        bsf     eax, ecx
        ret     0
    };
}

#endif
