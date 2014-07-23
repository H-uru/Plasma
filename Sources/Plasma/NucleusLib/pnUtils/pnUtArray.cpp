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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtArray.cpp
*   
***/

#include "pnUtArray.h"


/****************************************************************************
*
*   CBaseArray
*
***/

//===========================================================================
unsigned CBaseArray::CalcAllocGrowth (unsigned newAlloc, unsigned oldAlloc, unsigned * chunkSize) {

    // If this is the initial allocation, or if the new allocation is more
    // than twice as big as the old allocation and larger than the chunk
    // size, then allocate exactly the amount of memory requested
    if (!oldAlloc || (newAlloc >= std::max(2 * oldAlloc, *chunkSize)))
        return newAlloc;

    // Otherwise, allocate memory beyond what was requested in preparation
    // for future requests, so that we can reduce the time spent performing
    // memory management

    // For small allocations, double the size of the buffer each time
    if (newAlloc < *chunkSize)
        return std::max(newAlloc, 2 * oldAlloc);

    // For larger allocations, grow by the chunk size each time
    if (oldAlloc + *chunkSize > newAlloc) {

        // If the application appears to be growing the array a chunk size
        // at a time and has allocated at least 16 chunks, double the chunk
        // size
        if (newAlloc >= 16 * *chunkSize)
            *chunkSize *= 2;

        return oldAlloc + *chunkSize;
    }
    unsigned remainder = newAlloc % *chunkSize;
    if (remainder)
        return newAlloc + *chunkSize - remainder;
    else
        return newAlloc;

}

//===========================================================================
void * CBaseArray::ReallocPtr (void * ptr, unsigned bytes) {
    void * newPtr = nil;
    if (bytes) {
        newPtr = malloc(bytes);
    }
    return newPtr;
}
