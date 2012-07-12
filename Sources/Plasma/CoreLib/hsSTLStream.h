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
#include "hsStream.h"
#include "hsStlUtils.h"

//
// In-memory only
// Erase function lets you cut a chunk out of the middle of the stream
//
class hsVectorStream : public hsStream
{
protected:
    std::vector<uint8_t> fVector;
    uint32_t fEnd;    // End of file (one past the last byte)

public:
    hsVectorStream();
    hsVectorStream(uint32_t chunkSize);
    virtual ~hsVectorStream();

    virtual bool      Open(const char *, const char *)    { hsAssert(0, "hsVectorStream::Open Not Implemented"); return false; }
    virtual bool      Open(const wchar_t *, const wchar_t *)  { hsAssert(0, "hsVectorStream::Open Not Implemented"); return false; }
    virtual bool      Close()             { hsAssert(0, "hsVectorStream::Close Not Implemented"); return false; }
    
    virtual bool      AtEnd();
    virtual uint32_t  Read(uint32_t byteCount, void * buffer);
    virtual uint32_t  Write(uint32_t byteCount, const void* buffer);
    virtual void      Skip(uint32_t deltaByteCount);
    virtual void      Rewind();
    virtual void      FastFwd();
    virtual void      Truncate();

    virtual uint32_t  GetEOF();
    virtual void    CopyToMem(void* mem);

    virtual void    Reset();        // clears the buffers

    // Erase number of bytes at the current position
    virtual void    Erase(uint32_t bytes);
    // A pointer to the beginning of the data in the stream.  This is only valid
    // until someone modifies the stream.
    const void      *GetData();
    // In case you want to try and be efficient with your memory allocations
    void Reserve(uint32_t bytes) { fVector.reserve(bytes); }
};
