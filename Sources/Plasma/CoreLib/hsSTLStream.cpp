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
#include "hsSTLStream.h"

bool hsVectorStream::AtEnd()
{
    return (fBytesRead >= fEnd);
}

uint32_t hsVectorStream::Read(uint32_t byteCount, void *buffer)
{
    if (fBytesRead + byteCount > fEnd)
    {
//      hsStatusMessageF("Reading past end of hsVectorStream (read %u of %u requested bytes)", fEnd-fBytesRead, byteCount);
        byteCount = fEnd - fBytesRead;
    }
    
    memcpy(buffer, &fVector[fBytesRead], byteCount);

    fBytesRead += byteCount;
    fPosition += byteCount;

    return byteCount;
}

uint32_t hsVectorStream::Write(uint32_t byteCount, const void* buffer)
{
    // If we are at the end of the vector, we can just do a block insert of the data
    if (fPosition == fVector.size())
        fVector.insert(fVector.end(), (uint8_t*)buffer, (uint8_t*)buffer+byteCount);
    // If we are in the middle, I don't know how to just overwrite a block of the vector.
    // So, we make sure there is enough space and copy the elements one by one
    else
    {
        fVector.reserve(fPosition+byteCount);
        for (uint32_t i = 0; i < byteCount; i++)
            fVector[fPosition+i] = ((uint8_t*)buffer)[i];
    }

    fPosition += byteCount;

    if (fPosition > fEnd)
        fEnd = fPosition;

    return byteCount;
}

void hsVectorStream::Skip(uint32_t deltaByteCount)
{
    fBytesRead += deltaByteCount;
    fPosition += deltaByteCount;
}

void hsVectorStream::Rewind()
{
    fBytesRead = 0;
    fPosition = 0;
}

void hsVectorStream::FastFwd()
{
    fBytesRead = fPosition = fEnd;
}

void hsVectorStream::Truncate()
{
    fVector.erase(fVector.begin()+fPosition, fVector.end());
    fEnd = fPosition-1;
}

uint32_t hsVectorStream::GetEOF()
{
    return fEnd;
}

void hsVectorStream::CopyToMem(void* mem)
{
    memcpy(mem, &fVector[0], fEnd);
}

void hsVectorStream::Erase(uint32_t bytes)
{
    hsAssert(fPosition+bytes <= fEnd, "Erasing past end of stream");

    fVector.erase(fVector.begin()+fPosition, fVector.begin()+fPosition+bytes);
    fEnd -= bytes;
}

void hsVectorStream::Reset()
{
    fBytesRead = 0;
    fPosition = 0;
    fEnd = 0;
    fVector.clear();
}

const void *hsVectorStream::GetData()
{
    if (fVector.size() > 0)
        return &fVector[0];
    else
        return nullptr;
}
