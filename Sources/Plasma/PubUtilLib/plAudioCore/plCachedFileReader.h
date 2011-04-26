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

#ifndef _plcachedfilereader_h
#define _plcachedfilereader_h

#include "plAudioFileReader.h"

//// Class Definition ////////////////////////////////////////////////////////

class plCachedFileReader : public plAudioFileReader
{
public:
    plCachedFileReader(const char *path,
                    plAudioCore::ChannelSelect whichChan = plAudioCore::kAll);
    virtual ~plCachedFileReader();

    virtual plWAVHeader &GetHeader();

    virtual void    Close();

    virtual UInt32  GetDataSize();
    virtual float   GetLengthInSecs();

    virtual hsBool  SetPosition(UInt32 numBytes);
    virtual hsBool  Read(UInt32 numBytes, void *buffer);
    virtual UInt32  NumBytesLeft();

    virtual hsBool  OpenForWriting(const char *path, plWAVHeader &header);
    virtual UInt32  Write(UInt32 bytes, void *buffer);

    virtual hsBool  IsValid() { return fFileHandle != nil; }

protected:
    enum
    {
        kPCMFormatTag = 1
    };

    char            fFilename[512];
    FILE *          fFileHandle;
    plWAVHeader     fHeader;
    UInt32          fDataLength;
    UInt32          fCurPosition;

    void IError(const char *msg);
};

#endif //_plcachedfilereader_h
