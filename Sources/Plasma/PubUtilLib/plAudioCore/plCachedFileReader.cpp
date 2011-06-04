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

#include "hsTypes.h"
#include "plCachedFileReader.h"

//// Constructor/Destructor //////////////////////////////////////////////////

plCachedFileReader::plCachedFileReader(const char *path,
                                       plAudioCore::ChannelSelect whichChan)
        : fFileHandle(nil), fCurPosition(0)
{
    hsAssert(path != nil, "Invalid path specified in plCachedFileReader");

    strncpy(fFilename, path, sizeof(fFilename));

    /// Open the file as a plain binary stream
    fFileHandle = fopen(path, "rb");
    if (fFileHandle != nil)
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
    if (fFileHandle != nil) {
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
    if (fFileHandle != nil)
    {
        fclose(fFileHandle);
        fFileHandle = nil;
    }
}

UInt32 plCachedFileReader::GetDataSize()
{
    hsAssert(IsValid(), "GetDataSize() called on an invalid cache file");

    return fDataLength;
}

float plCachedFileReader::GetLengthInSecs()
{
    hsAssert(IsValid(), "GetLengthInSecs() called on an invalid cache file");

    return (float)fDataLength / (float)fHeader.fAvgBytesPerSec;
}

hsBool plCachedFileReader::SetPosition(UInt32 numBytes)
{
    hsAssert(IsValid(), "SetPosition() called on an invalid cache file");

    fCurPosition = numBytes;

    hsAssert(fCurPosition <= fDataLength, "Invalid position while seeking");

    return !fseek(fFileHandle, sizeof(plWAVHeader) + fCurPosition, SEEK_SET);
}

hsBool plCachedFileReader::Read(UInt32 numBytes, void *buffer)
{
    hsAssert(IsValid(), "Read() called on an invalid cache file");

    size_t numRead = fread(buffer, 1, numBytes, fFileHandle);

    fCurPosition += numRead;
    hsAssert(fCurPosition <= fDataLength, "Invalid position while reading");

    return numRead >= numBytes;
}

UInt32 plCachedFileReader::NumBytesLeft()
{
    hsAssert(IsValid(), "NumBytesLeft() called on an invalid cache file");
    hsAssert(fCurPosition <= fDataLength, "Invalid position while reading");

    return fDataLength - fCurPosition;
}

hsBool plCachedFileReader::OpenForWriting(const char *path, plWAVHeader &header)
{
    hsAssert(path != nil, "Invalid path specified in plCachedFileReader");

    fHeader = header;
    fCurPosition = 0;
    fDataLength = 0;
    strncpy(fFilename, path, sizeof(fFilename));

    /// Open the file as a plain binary stream
    fFileHandle = fopen(path, "wb");

    if (fFileHandle != nil)
    {
        if (fwrite(&fHeader, 1, sizeof(plWAVHeader), fFileHandle)
                != sizeof(plWAVHeader))
        {
            IError("Could not write WAV file header in plCachedFileReader");
            return false;
        }
    }

    return fFileHandle != nil;
}

UInt32 plCachedFileReader::Write(UInt32 bytes, void* buffer)
{
    hsAssert(IsValid(), "Write() called on an invalid cache file");

    size_t written = fwrite(buffer, 1, bytes, fFileHandle);

    fCurPosition += written;
    fDataLength += written;

    return (UInt32)written;
}
