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
#ifndef plZlibStream_h_inc
#define plZlibStream_h_inc

#include "hsStream.h"
#include "hsStlUtils.h"

//
// This is for reading a .gz file from a buffer, and writing the uncompressed data to a file.
// Call open with the name of the uncompressed file, then call write with the compressed data.
//
class plZlibStream : public hsStream
{
protected:
    hsStream* fOutput;
    void* fZStream;
    bool fDecompressedOk;

    enum Validate { kNeedMoreData, kInvalidHeader, kValidHeader };
    Validate fHeader;
    std::vector<UInt8> fHeaderCache;

    std::wstring fFilename, fMode; // needed for rewind function

    int IValidateGzHeader(UInt32 byteCount, const void* buffer);

public:
    plZlibStream();
    virtual ~plZlibStream();

    virtual hsBool  Open(const char* filename, const char* mode);
    virtual hsBool  Open(const wchar* filename, const wchar* mode);
    virtual hsBool  Close();
    virtual UInt32  Write(UInt32 byteCount, const void* buffer);

    // Since most functions don't check the return value from Write, you can
    // call this after you've passed in all your data to determine if it
    // decompressed ok
    bool DecompressedOk() { return fDecompressedOk; }

    // You can't use these
    virtual hsBool  AtEnd();
    virtual UInt32  Read(UInt32 byteCount, void* buffer);
    virtual void    Skip(UInt32 deltaByteCount);
    virtual void    Rewind();
    virtual void    FastFwd();
    virtual UInt32  GetEOF();
};

#endif // plZlibStream_h_inc