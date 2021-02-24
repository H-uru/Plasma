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

#ifndef _plcachedfilereader_h
#define _plcachedfilereader_h

#include "plAudioFileReader.h"
#include "plFileSystem.h"

//// Class Definition ////////////////////////////////////////////////////////

class plCachedFileReader : public plAudioFileReader
{
public:
    plCachedFileReader(const plFileName &path,
                    plAudioCore::ChannelSelect whichChan = plAudioCore::kAll);
    virtual ~plCachedFileReader();

    plWAVHeader &GetHeader() override;

    void    Close() override;

    uint32_t  GetDataSize() override;
    float   GetLengthInSecs() override;

    bool    SetPosition(uint32_t numBytes) override;
    bool    Read(uint32_t numBytes, void *buffer) override;
    uint32_t  NumBytesLeft() override;

    bool    OpenForWriting(const plFileName &path, plWAVHeader &header) override;
    uint32_t  Write(uint32_t bytes, void *buffer) override;

    bool    IsValid() override { return fFileHandle != nullptr; }

protected:
    enum
    {
        kPCMFormatTag = 1
    };

    plFileName      fFilename;
    FILE *          fFileHandle;
    plWAVHeader     fHeader;
    uint32_t        fDataLength;
    uint32_t        fCurPosition;

    void IError(const char *msg);
};

#endif //_plcachedfilereader_h
