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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtEndian.cpp
*   
***/

#include "../Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Big endian functions
*
***/

#ifdef BIG_ENDIAN

//===========================================================================
void EndianConvert (word * array, unsigned count) {
    for (; count--; ++array)
        *array = Endian(*array);
}

//===========================================================================
void EndianConvert (dword * array, unsigned count) {
    for (; count--; ++array)
        *array = Endian(*array);
}

//===========================================================================
void EndianConvert (qword * array, unsigned count) {
    for (; count--; ++array)
        *array = Endian(*array);
}

//============================================================================
void EndianConvert (byte * data, unsigned elemCount, unsigned elemBytes) {
    switch (elemBytes) {
        case sizeof(byte):
        break;

        case sizeof(word):
            EndianConvert((word *)data, elemCount);
        break;

        case sizeof(dword):
            EndianConvert((dword *)data, elemCount);
        break;

        case sizeof(qword):
            EndianConvert((qword *)data, elemCount);
        break;

        DEFAULT_FATAL(elemBytes);
    }
}

//===========================================================================
void EndianCopy (unsigned * dst, const word src[], unsigned count) {
    for (; count--; ++src, ++dst)
        *dst = (unsigned)Endian(*src);
}

//===========================================================================
void EndianCopy (unsigned * dst, const dword src[], unsigned count) {
    for (; count--; ++src, ++dst)
        *dst = (unsigned)Endian(*src);
}

//===========================================================================
void EndianCopy (unsigned * dst, const qword src[], unsigned count) {
    for (; count--; ++src, ++dst)
        *dst = (unsigned)Endian(*src);
}

//============================================================================
void EndianCopy (
    void *      dst,
    const byte  src[],
    unsigned    elemCount,
    unsigned    elemBytes
) {
    switch (elemBytes) {
        case sizeof(byte):
            MemCopy(dst, src, elemCount);
        break;

        case sizeof(word):
            EndianCopy((word *)dst, (const word *)src, elemCount);
        break;

        case sizeof(dword):
            EndianCopy((dword *)dst, (const dword *)src, elemCount);
        break;

        case sizeof(qword):
            EndianCopy((qword *)dst, (const qword *)src, elemCount);
        break;

        DEFAULT_FATAL(elemBytes);
    }
}

#endif // BIG_ENDIAN
