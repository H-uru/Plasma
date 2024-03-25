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
//  plAudioFileReader - Microsoft's way of making our lives difficult when  //
//                reading in .WMA files. Hacking Winamp's plugins is        //
//                probably easier...                                        //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _plAudioFileReader_h
#define _plAudioFileReader_h

#include "plAudioCore.h"

//// Class Definition ////////////////////////////////////////////////////////

class plFileName;
class plUnifiedTime;
class plWAVHeader;
class plAudioFileReader
{
public:
    virtual ~plAudioFileReader() {}
    virtual plWAVHeader &GetHeader() = 0;

    enum StreamType
    {
        kStreamRAM,     // Stream from a WAV loaded into RAM
        kStreamWAV,     // Stream from a WAV on disk
        kStreamNative,  // Stream from the native type (ie, an Ogg on disk)
    };

    virtual void    Open(){}
    virtual void    Close() = 0;

    virtual uint32_t  GetDataSize() = 0;
    virtual float   GetLengthInSecs() = 0;

    virtual bool    SetPosition( uint32_t numBytes ) = 0;
    virtual bool    Read( uint32_t numBytes, void *buffer ) = 0;
    virtual uint32_t  NumBytesLeft() = 0;

    virtual bool    OpenForWriting( const plFileName& path, plWAVHeader &header ) { return false; }
    virtual uint32_t  Write( uint32_t bytes, void *buffer ) { return 0; }

    virtual bool    IsValid() = 0;

    static plAudioFileReader* CreateReader(const plFileName& path, plAudioCore::ChannelSelect whichChan = plAudioCore::kAll, StreamType type = kStreamWAV);
    static plAudioFileReader* CreateWriter(const plFileName& path, plWAVHeader& header);

    // Decompresses a compressed file to the cache directory
    static void CacheFile(const plFileName& path, bool splitChannels=false, bool noOverwrite=false);

protected:
    static plFileName IGetCachedPath(const plFileName& path, plAudioCore::ChannelSelect whichChan);
    static void ICacheFile(const plFileName& path, bool noOverwrite, plAudioCore::ChannelSelect whichChan);
};

#endif //_plAudioFileReader_h
