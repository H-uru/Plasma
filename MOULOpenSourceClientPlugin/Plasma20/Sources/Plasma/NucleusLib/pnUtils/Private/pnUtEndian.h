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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtEndian.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTENDIAN_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtEndian.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTENDIAN_H

// NOTE: Because we predominantly run on little-endian CPUs, we don't
// convert to "network order" when sending integers across the network
// (tcp uses big-endian regardless of underlying hardware) and instead
// use little-endian as the "native" language of our network messages.


/*****************************************************************************
*
*   Types and constants
*
***/

#ifdef _M_IX86
# define LITTLE_ENDIAN  1
#else
# define BIG_ENDIAN     1
// That was a pretty weak check for endian-ness, if it
// failed then we probably need to strengthen it a bit.
# error "Are you sure this is a big-endian CPU?"
#endif


/*****************************************************************************
*
*   Little endian functions
*
***/

#ifdef LITTLE_ENDIAN

//============================================================================
inline word  Endian (word value)    { return value; }
inline dword Endian (dword value)   { return value; }
inline qword Endian (qword value)   { return value; }

//===========================================================================
inline void EndianConvert (
    word *      array, 
    unsigned    count
) {
    ref(array); 
    ref(count); 
    return; 
}

//===========================================================================
inline void EndianConvert (
    dword *     array, 
    unsigned    count
) {
    ref(array); 
    ref(count); 
    return; 
}

//===========================================================================
inline void EndianConvert (
    qword *     array, 
    unsigned    count
) {
    ref(array); 
    ref(count); 
    return; 
}

//===========================================================================
inline void EndianConvert (
    byte *      data,
    unsigned    elemCount,
    unsigned    elemBytes
) {
    ref(data);
    ref(elemCount);
    ref(elemBytes);
    return;
}

//===========================================================================
inline void EndianCopy (
    word *      dst, 
    const word  src[], 
    unsigned    count
) {
    MemCopy(dst, src, count * sizeof(word));
}

//===========================================================================
inline void EndianCopy (
    dword *     dst, 
    const dword src[], 
    unsigned    count
) {
    MemCopy(dst, src, count * sizeof(dword));
}

//===========================================================================
inline void EndianCopy (
    qword *     dst, 
    const qword src[], 
    unsigned    count
) {
    MemCopy(dst, src, count * sizeof(qword));
}

//===========================================================================
inline void EndianCopy (
    void *      dst, 
    const byte  src[], 
    unsigned    elemCount,
    unsigned    elemBytes
) {
    MemCopy(dst, src, elemCount * elemBytes);
}



#endif // LITTLE_ENDIAN


/*****************************************************************************
*
*   Big endian functions
*
***/

#ifdef BIG_ENDIAN

//===========================================================================
inline word Endian (word value) { 
    return (value >> 8) | (value << 8); 
}

//===========================================================================
inline dword Endian (dword value) {
    return ((value)              << 24) | 
           ((value & 0x0000ff00) << 8)  |
           ((value & 0x00ff0000) >> 8)  |
           ((value)              >> 24);
}

//===========================================================================
inline qword Endian (qword value) {
    return ((value)                      << 56) | 
           ((value & 0x000000000000ff00) << 40) |
           ((value & 0x0000000000ff0000) << 24) |
           ((value & 0x00000000ff000000) << 8)  |
           ((value & 0x000000ff00000000) >> 8)  |
           ((value & 0x0000ff0000000000) >> 24) |
           ((value & 0x00ff000000000000) >> 40) |
           ((value)                      >> 56);
}

void EndianConvert (word * array, unsigned count);
void EndianConvert (dword * array, unsigned count);
void EndianConvert (qword * array, unsigned count);
void EndianConvert (byte * data, unsigned elemCount, unsigned elemBytes);
void EndianCopy (word * dst, const word src[], unsigned count);
void EndianCopy (dword * dst, const dword src[], unsigned count);
void EndianCopy (qword * dst, const dword src[], unsigned count);
void EndianCopy (void * dst, const byte src[], unsigned elemCount, unsigned elemBytes);

#endif
