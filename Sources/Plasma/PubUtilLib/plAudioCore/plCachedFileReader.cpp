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
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  plCachedFileReader - Reads (and writes, how ironic) decompressed sound  //
//                       data that is temporarily held in a cache.          //
//                                                                          //
//// NOTES ///////////////////////////////////////////////////////////////////
//                                                                          //
//  2011.04.24 - Created by dpogue.                                         //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "plCachedFileReader.h"

//// Constructor/Destructor //////////////////////////////////////////////////

plCachedFileReader::plCachedFileReader(const plFileName &path,
                                       plAudioCore::ChannelSelect whichChan)
    : fFilename(path), fFileHandle(), fCurPosition(), fHeader(),
      fDataLength()       
{
    hsAssert(path.IsValid(), "Invalid path specified in plCachedFileReader");

    /// Open the file as a plain binary stream
    fFileHandle = plFileSystem::Open(path, "rb");
    if (fFileHandle != nullptr)
    {
        if (fread(&fHeader, 1, sizeof(plWAVHeader), fFileHandle)
                != sizeof(plWAVHeader))
        {
            IError("Invalid WAV file header in plCachedFileReader");
            return;
        }

        // Check format
        if (fHeader.fFormatTag != kPCMFormatTag)
        {
            IError("Invalid format in plCachedFileReader");
            return;
        }

        fseek(fFileHandle, 0, SEEK_END);
        fDataLength = ftell(fFileHandle) - sizeof(plWAVHeader);

        fseek(fFileHandle, sizeof(plWAVHeader), SEEK_SET);
    }
}

plCachedFileReader::~plCachedFileReader()
{
    if (fFileHandle != nullptr) {
        fclose(fFileHandle);
    }
}

void plCachedFileReader::IError(const char *msg)
{
    hsAssert(false, msg);
    Close();
}

plWAVHeader &plCachedFileReader::GetHeader()
{
    hsAssert(IsValid(), "GetHeader() called on an invalid cache file");

    return fHeader;
}

void plCachedFileReader::Close()
{
    if (fFileHandle != nullptr)
    {
        fclose(fFileHandle);
        fFileHandle = nullptr;
    }
}

uint32_t plCachedFileReader::GetDataSize()
{
    hsAssert(IsValid(), "GetDataSize() called on an invalid cache file");

    return fDataLength;
}

float plCachedFileReader::GetLengthInSecs()
{
    hsAssert(IsValid(), "GetLengthInSecs() called on an invalid cache file");

    return (float)fDataLength / (float)fHeader.fAvgBytesPerSec;
}

bool plCachedFileReader::SetPosition(uint32_t numBytes)
{
    hsAssert(IsValid(), "SetPosition() called on an invalid cache file");

    fCurPosition = numBytes;

    hsAssert(fCurPosition <= fDataLength, "Invalid position while seeking");

    return !fseek(fFileHandle, sizeof(plWAVHeader) + fCurPosition, SEEK_SET);
}

bool plCachedFileReader::Read(uint32_t numBytes, void *buffer)
{
    hsAssert(IsValid(), "Read() called on an invalid cache file");

    size_t numRead = fread(buffer, 1, numBytes, fFileHandle);

    fCurPosition += numRead;
    hsAssert(fCurPosition <= fDataLength, "Invalid position while reading");

    return numRead >= numBytes;
}

uint32_t plCachedFileReader::NumBytesLeft()
{
    hsAssert(IsValid(), "NumBytesLeft() called on an invalid cache file");
    hsAssert(fCurPosition <= fDataLength, "Invalid position while reading");

    return fDataLength - fCurPosition;
}

bool plCachedFileReader::OpenForWriting(const plFileName &path, plWAVHeader &header)
{
    hsAssert(path.IsValid(), "Invalid path specified in plCachedFileReader");

    fHeader = header;
    fCurPosition = 0;
    fDataLength = 0;
    fFilename = path;

    /// Open the file as a plain binary stream
    fFileHandle = plFileSystem::Open(path, "wb");

    if (fFileHandle != nullptr)
    {
        if (fwrite(&fHeader, 1, sizeof(plWAVHeader), fFileHandle)
                != sizeof(plWAVHeader))
        {
            IError("Could not write WAV file header in plCachedFileReader");
            return false;
        }
    }

    return fFileHandle != nullptr;
}

uint32_t plCachedFileReader::Write(uint32_t bytes, void* buffer)
{
    hsAssert(IsValid(), "Write() called on an invalid cache file");

    size_t written = fwrite(buffer, 1, bytes, fFileHandle);

    fCurPosition += written;
    fDataLength += written;

    return (uint32_t)written;
}
