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
#include "HeadSpin.h"
#include "plManifest.h"

#include "../pnEncryption/plChecksum.h"
#include "../plCompression/plZlibStream.h"
#include "../plFile/plEncryptedStream.h"
#include "../plFile/plFileUtils.h"
#include "../plUnifiedTime/plUnifiedTime.h"

class plManifestFile
{
public:
    char* fFilename;
    plMD5Checksum fSum;
    plMD5Checksum fLocalSum;
    uint32_t fSize;
    uint32_t fCompressedSize;
    uint32_t fFlags;
};

plManifest::plManifest(LogFunc log) :
    fDownloadFiles(0),
    fDownloadBytes(0),
    fDirtySums(false),
    fLog(log)
{
}

plManifest::~plManifest()
{
    if (fDirtySums)
        IWriteCache();

    delete [] fManifestName;

    for (int i = 0; i < fFiles.size(); i++)
    {
        delete [] fFiles[i]->fFilename;
        delete fFiles[i];
    }
}

bool plManifest::Read(hsStream* mfsStream, const char* basePath, const char* mfsName)
{
    fBasePath = basePath;
    fManifestName = hsStrcpy(mfsName);

    fLog("--- Reading manifest for %s", fManifestName);

    char buf[256];
    while (mfsStream->ReadLn(buf, sizeof(buf)))
    {
        plManifestFile* file = new plManifestFile;

        char* tok = strtok(buf, "\t");
        file->fFilename = hsStrcpy(tok);

        tok = strtok(nil, "\t");
        file->fSum.SetFromHexString(tok);

        tok = strtok(nil, "\t");
        file->fSize = atoi(tok);

        tok = strtok(nil, "\t");
        file->fCompressedSize = atoi(tok);

        tok = strtok(nil, "\t");
        file->fFlags = atoi(tok);

        fFiles.push_back(file);
    }

    return true;
}

void plManifest::ValidateFiles(ProgressFunc progress)
{
    if (fFiles.empty())
        return;

    fLog("--- Validating files for %s", fManifestName);

    IReadCache(progress);

    fDownloadFiles = 0;
    fDownloadBytes = 0;

    for (int i = 0; i < fFiles.size(); i++)
    {
        plManifestFile* file = fFiles[i];

        // If the local checksum is invalid, this file wasn't in our cache.
        // Get the sum, and update the progress bar.
        if (!file->fLocalSum.IsValid())
        {
            fLog("    No sum for %s, calculating", file->fFilename);
            file->fLocalSum.CalcFromFile(file->fFilename);
            fDirtySums = true;
            progress(file->fFilename, 1);
        }

        if (file->fLocalSum != file->fSum)
        {
            fLog("    Incorrect sum for %s", file->fFilename);
            fDownloadFiles++;
            fDownloadBytes += file->fCompressedSize;
        }
    }

    fLog("---  Need to download %d files, %.1f MB", fDownloadFiles, float(fDownloadBytes) / (1024.f*1024.f));
}

void plManifest::DownloadUpdates(ProgressFunc progress, plFileGrabber* grabber)
{
    for (int i = 0; i < fFiles.size(); i++)
    {
        plManifestFile* file = fFiles[i];
        if (file->fLocalSum != file->fSum)
        {
            char serverPath[MAX_PATH];

            sprintf(serverPath, "%s%s.gz", fBasePath.c_str(), file->fFilename);
            grabber->MakeProperPath(serverPath);

            hsRAMStream serverStream;
            if (grabber->FileToStream(serverPath, &serverStream))
            {
                plFileUtils::EnsureFilePathExists(file->fFilename);

                plFileUtils::RemoveFile(file->fFilename, true);

                plZlibStream localStream;
                if (localStream.Open(file->fFilename, "wb"))
                {
                    char dataBuf[1024];
                    uint32_t sizeLeft = serverStream.GetSizeLeft();
                    while (uint32_t amtRead = serverStream.Read( (sizeof(dataBuf) > sizeLeft) ? sizeLeft : sizeof(dataBuf), dataBuf))
                    {
                        progress(file->fFilename, amtRead);

                        localStream.Write(amtRead, dataBuf);
                        sizeLeft = serverStream.GetSizeLeft();
                    }

                    localStream.Close();

                    // FIXME - Should we recalc this?
                    file->fLocalSum = file->fSum;
                    fDirtySums = true;

                    if (file->fFlags != 0)
                        IDecompressSound(file);
                }
            }
        }
    }
}

plManifestFile* plManifest::IFindFile(const char* name)
{
    // FIXME
    for (int i = 0; i < fFiles.size(); i++)
    {
        if (hsStrEQ(fFiles[i]->fFilename, name))
            return fFiles[i];
    }

    return nil;
}

// KLUDGE - Put age checksums in the dat dir, for backwards compatability
const char* plManifest::IGetCacheDir()
{
    const char* prefix = "";
    if (strncmp(fFiles[0]->fFilename, "dat\\", strlen("dat\\")) == 0)
        return "dat\\";
    else
        return "";
}

#define kCacheFileVersion 1

void plManifest::IWriteCache()
{
    plEncryptedStream s;

    bool openedFile = false;

    uint32_t numFiles = 0;
    for (int i = 0; i < fFiles.size(); i++)
    {
        plManifestFile* file = fFiles[i];

        plUnifiedTime modifiedTime;
        if (file->fLocalSum.IsValid() &&
            plFileUtils::GetFileTimes(file->fFilename, nil, &modifiedTime))
        {
            if (!openedFile)
            {
                openedFile = true;
                char buf[256];
                sprintf(buf, "%s%s.sum", IGetCacheDir(), fManifestName);
                s.Open(buf, "wb");
                s.WriteSwap32(0);
                s.WriteSwap32(kCacheFileVersion);
            }

            s.WriteSafeString(file->fFilename);

            plMD5Checksum& checksum = file->fLocalSum;
            s.Write(checksum.GetSize(), checksum.GetValue());

            modifiedTime.Write(&s);

            numFiles++;
        }
    }

    if (openedFile)
    {
        s.Rewind();
        s.WriteSwap32(numFiles);

        s.Close();
    }
}

void plManifest::IReadCache(ProgressFunc progress)
{
    //
    // Load valid cached checksums
    //
    char buf[256];
    sprintf(buf, "%s%s.sum", IGetCacheDir(), fManifestName);
    hsStream* s = plEncryptedStream::OpenEncryptedFile(buf);

    if (s)
    {
        uint32_t numCached = s->ReadSwap32();
        uint32_t cacheFileVersion = s->ReadSwap32();

        if (cacheFileVersion != kCacheFileVersion)
        {
            s->Close();
            delete s;
            return;
        }

        fLog("  Reading cache...found %d cached sums", numCached);

        for (int i = 0; i < numCached; i++)
        {
            char* name = s->ReadSafeString();

            uint8_t checksumBuf[MD5_DIGEST_LENGTH];
            s->Read(sizeof(checksumBuf), checksumBuf);
            plMD5Checksum checksum;
            checksum.SetValue(checksumBuf);

            plUnifiedTime modifiedTime;
            modifiedTime.Read(s);

            plManifestFile* file = IFindFile(name);
            if (file)
            {
                plUnifiedTime curModifiedTime;
                if (plFileUtils::GetFileTimes(file->fFilename, nil, &curModifiedTime))
                {
                    if (curModifiedTime == modifiedTime)
                        file->fLocalSum = checksum;
                    else
                        fLog("    Invalid modified time for %s", name);
                }
                else
                    fLog("    Couldn't get modified time for %s", name);

                progress(file->fFilename, 1);
            }
            else
                fLog("    Couldn't find cached file '%s' in manifest, discarding", name);


            delete [] name;
        }

        s->Close();
        delete s;
    }
}

#include "../plAudioCore/plAudioFileReader.h"
#include "../plAudio/plOGGCodec.h"
#include "../plAudio/plWavFile.h"


bool plManifest::IDecompressSound(plManifestFile* file)
{
    enum
    {
        kSndFlagCacheSplit          = 1<<0,
        kSndFlagCacheStereo         = 1<<2,
    };

    if (hsCheckBits(file->fFlags, kSndFlagCacheSplit) ||
        hsCheckBits(file->fFlags, kSndFlagCacheStereo))
    {
        plAudioFileReader* reader = plAudioFileReader::CreateReader(file->fFilename, plAudioCore::kAll, plAudioFileReader::kStreamNative);
        if (!reader)
            return false;
        uint32_t size = reader->GetDataSize();
        delete reader;

        if (hsCheckBits(file->fFlags, kSndFlagCacheSplit))
            plAudioFileReader::CacheFile(file->fFilename, true);
        if (hsCheckBits(file->fFlags, kSndFlagCacheStereo))
            plAudioFileReader::CacheFile(file->fFilename, false);
    }

    return true;
}
