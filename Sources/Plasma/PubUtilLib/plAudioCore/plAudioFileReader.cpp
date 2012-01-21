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
//  plAudioFileReader                                                       //
//                                                                          //
//// Notes ///////////////////////////////////////////////////////////////////
//                                                                          //
//  3.5.2001 - Created by mcn.                                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
#include <string.h>

#include "HeadSpin.h"
#include "plAudioFileReader.h"
#include "plAudioCore.h"
//#include "hsTimer.h"

#include "plFile/hsFiles.h"
#include "plFile/plFileUtils.h"
#include "plUnifiedTime/plUnifiedTime.h"
#include "plBufferedFileReader.h"
#include "plCachedFileReader.h"
#include "plFastWavReader.h"
#include "plOGGCodec.h"
#include "plWavFile.h"

#define kCacheDirName   "temp"

static void hsStrUpper(char *s)
{
    if (s)
    {
        int len = hsStrlen(s);
        for (int i = 0; i < len; i++)
            s[i] = toupper(s[i]); 
    }
}

plAudioFileReader* plAudioFileReader::CreateReader(const char* path, plAudioCore::ChannelSelect whichChan, StreamType type)
{
    const char* ext = plFileUtils::GetFileExt(path);

    if (type == kStreamWAV)
    {
        bool isWav = (stricmp(ext, "wav") == 0);
        // We want to stream a wav off disk, but this is a compressed file.
        // Get the uncompressed path. Ignore the requested channel, since it 
        // will have already been split into two files if that is necessary.
        if (!isWav)
        {
            char cachedPath[256];
            IGetCachedPath(path, cachedPath, whichChan);
            plAudioFileReader *r =  new plCachedFileReader(cachedPath, plAudioCore::kAll);
            if (!r->IsValid()) {
                // So we tried to play a cached file and it didn't exist
                // Oops... we should cache it now
                delete r;
                ICacheFile(path, true, whichChan);
                r = new plCachedFileReader(cachedPath, plAudioCore::kAll);
            }
            return r;
        }
        
        plAudioFileReader *r =  new plFastWAV(path, whichChan);
        return r;
    }
    else if (type == kStreamRAM)
        return new plBufferedFileReader(path, whichChan);
    else if (type == kStreamNative)
        return new plOGGCodec(path, whichChan);

    return nil;
}

plAudioFileReader* plAudioFileReader::CreateWriter(const char* path, plWAVHeader& header)
{
    const char* ext = plFileUtils::GetFileExt(path);

    plAudioFileReader* writer = new plCachedFileReader(path, plAudioCore::kAll);
    writer->OpenForWriting(path, header);
    return writer;
}

void plAudioFileReader::IGetCachedPath(const char* path, char* cachedPath, plAudioCore::ChannelSelect whichChan)
{
    // Get the file's path and add our streaming cache folder to it
    strcpy(cachedPath, path);
    plFileUtils::StripFile(cachedPath);
    strcat(cachedPath, kCacheDirName"\\");

    // Create the directory first
    plFileUtils::CreateDir(cachedPath);

    // Get the path to the cached version of the file, without the extension
    const char* fileName = plFileUtils::GetFileName(path);
    const char* fileExt = plFileUtils::GetFileExt(fileName);
    strncat(cachedPath, fileName, fileExt-fileName-1);

    if (whichChan == plAudioCore::kLeft)
        strcat(cachedPath, "-Left.tmp");
    else if (whichChan == plAudioCore::kRight)
        strcat(cachedPath, "-Right.tmp");
    else if (whichChan == plAudioCore::kAll)
        strcat(cachedPath, ".tmp");
}

void plAudioFileReader::ICacheFile(const char* path, bool noOverwrite, plAudioCore::ChannelSelect whichChan)
{
    char cachedPath[256];
    IGetCachedPath(path, cachedPath, whichChan);
    if (!noOverwrite || !plFileUtils::FileExists(cachedPath))
    {
        plAudioFileReader* reader = plAudioFileReader::CreateReader(path, whichChan, kStreamNative);
        if (!reader || !reader->IsValid())
        {
            delete reader;
            return;
        }
        plAudioFileReader* writer = CreateWriter(cachedPath, reader->GetHeader());
        if (!writer || !writer->IsValid())
        {
            delete reader;
            delete writer;
            return;
        }

        uint8_t buffer[4096];
        uint32_t numLeft;
        while ((numLeft = reader->NumBytesLeft()) > 0)
        {
            uint32_t toRead = (numLeft < sizeof(buffer)) ? numLeft : sizeof(buffer);
            reader->Read(toRead, buffer);
            writer->Write(toRead, buffer);
        }
        writer->Close();

        delete writer;
        delete reader;
    }
}

void plAudioFileReader::CacheFile(const char* path, bool splitChannels, bool noOverwrite)
{
    if (splitChannels)
    {
        ICacheFile(path, noOverwrite, plAudioCore::kLeft);
        ICacheFile(path, noOverwrite, plAudioCore::kRight);
    }
    else
    {
        ICacheFile(path, noOverwrite, plAudioCore::kAll);
    }
}
