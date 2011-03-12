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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtMath.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTMATH_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtMath.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTMATH_H


/*****************************************************************************
*
*   Calling conventions
*
***/

#ifdef _M_IX86
#define  MATHCALL  __fastcall
#else
#define  MATHCALL
#endif


/*****************************************************************************
*
*   Bit manipulation functions
*
***/

unsigned MATHCALL MathLowBitPos (dword val);
unsigned MATHCALL MathHighBitPos (dword val);

//===========================================================================
inline unsigned MathBitCount (dword val) {
    val = val - ((val >> 1) & 033333333333) - ((val >> 2) & 011111111111);
    val = ((val + (val >> 3)) & 030707070707);
    val = val + (val >> 6);
    val = (val + (val >> 12) + (val >> 24)) & 077;
    return val;
}

//===========================================================================
inline unsigned MathBitMaskCreate (unsigned count) {
    ASSERT(count <= 8 * sizeof(unsigned));
    return count ? ((2 << (count - 1)) - 1) : 0;
}

//===========================================================================
inline dword MathHighBitValue (dword val) {
    return val ? 1 << MathHighBitPos(val) : 0;
}

//===========================================================================
inline bool MathIsPow2 (unsigned val) {
    return !(val & (val - 1));
}

//===========================================================================
inline unsigned MathLowBitValue (unsigned val) {
    return val & ~(val - 1);
}

//===========================================================================
inline unsigned MathNextMultiplePow2 (unsigned val, unsigned multiple) {
    ASSERT(multiple);
    ASSERT(MathIsPow2(multiple));
    return (val + (multiple - 1)) & ~(multiple - 1);
}

//===========================================================================
inline dword MathNextPow2 (dword val) {
    return MathIsPow2(val) ? val : 1 << (MathHighBitPos(val) + 1);
}
