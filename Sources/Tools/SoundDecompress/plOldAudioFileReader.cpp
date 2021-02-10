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
//  plOldAudioFileReader                                                    //
//                                                                          //
//// Notes ///////////////////////////////////////////////////////////////////
//                                                                          //
//  3.5.2001 - Created by mcn.                                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "plOldAudioFileReader.h"

#include "HeadSpin.h"

#include "plAudioCore/plBufferedFileReader.h"
#include "plAudioCore/plCachedFileReader.h"
#include "plAudioCore/plFastWavReader.h"
#include "plAudioCore/plOGGCodec.h"
#include "plAudioCore/plWavFile.h"

#define kCacheDirName   "streamingCache"

plAudioFileReader* plOldAudioFileReader::CreateReader(const plFileName& path, plAudioCore::ChannelSelect whichChan, StreamType type)
{
    ST::string ext = path.GetFileExt();

    if (type == kStreamWAV) {
        bool isWav = (ext.compare_i("wav") == 0);
        // We want to stream a wav off disk, but this is a compressed file.
        // Get the uncompressed path. Ignore the requested channel, since it 
        // will have already been split into two files if that is necessary.
        if (!isWav) {
            plFileName cachedPath = IGetCachedPath(path, whichChan);
            std::unique_ptr<plAudioFileReader> r = std::make_unique<plFastWAV>(cachedPath, plAudioCore::kAll);
            if (!r->IsValid()) {
                // So we tried to play a cached file and it didn't exist
                // Oops... we should cache it now
                ICacheFile(path, true, whichChan);
                return new plFastWAV(cachedPath, plAudioCore::kAll);
            }
            return r.release();
        }

        return new plFastWAV(path, whichChan);
    } else if (type == kStreamRAM) {
        return new plBufferedFileReader(path, whichChan);
    } else if (type == kStreamNative) {
        return new plOGGCodec(path, whichChan);
    }

    return nullptr;
}

plAudioFileReader* plOldAudioFileReader::CreateWriter(const plFileName& path, plWAVHeader& header)
{
#ifndef HS_BUILD_FOR_WIN32
    static_assert(false, "WAV files can only be generated on Windows");
#endif

    CWaveFile* writer = new CWaveFile(path, plAudioCore::kAll);
    writer->OpenForWriting(path, header);
    return writer;
}

plFileName plOldAudioFileReader::IGetCachedPath(const plFileName& path, plAudioCore::ChannelSelect whichChan)
{
    // Get the file's path and add our streaming cache folder to it
    plFileName cachedPath = plFileName::Join(path.StripFileName(), kCacheDirName);

    // Create the directory first
    plFileSystem::CreateDir(cachedPath);

    const char *suffix = "";
    if (whichChan == plAudioCore::kLeft)
        suffix = "-Left.wav";
    else if (whichChan == plAudioCore::kRight)
        suffix = "-Right.wav";
    else if (whichChan == plAudioCore::kAll)
        suffix = ".wav";

    // Get the path to the cached version of the file, without the extension
    return plFileName::Join(cachedPath, path.GetFileNameNoExt() + suffix);
}

void plOldAudioFileReader::ICacheFile(const plFileName& path, bool noOverwrite, plAudioCore::ChannelSelect whichChan)
{
    plFileName cachedPath = IGetCachedPath(path, whichChan);
    if (!noOverwrite || !plFileInfo(cachedPath).Exists()) {
        std::unique_ptr<plAudioFileReader> reader(plAudioFileReader::CreateReader(path, whichChan, kStreamNative));
        if (!reader || !reader->IsValid())
            return;

        std::unique_ptr<plAudioFileReader> writer(CreateWriter(cachedPath, reader->GetHeader()));
        if (!writer || !writer->IsValid())
            return;

        uint8_t buffer[4096];
        uint32_t numLeft;
        while ((numLeft = reader->NumBytesLeft()) > 0) {
            uint32_t toRead = (numLeft < sizeof(buffer)) ? numLeft : sizeof(buffer);
            reader->Read(toRead, buffer);
            writer->Write(toRead, buffer);
        }
        writer->Close();
    }
}

void plOldAudioFileReader::CacheFile(const plFileName& path, bool splitChannels, bool noOverwrite)
{
    if (splitChannels) {
        ICacheFile(path, noOverwrite, plAudioCore::kLeft);
        ICacheFile(path, noOverwrite, plAudioCore::kRight);
    } else {
        ICacheFile(path, noOverwrite, plAudioCore::kAll);
    }
}
