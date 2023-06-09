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
#include "plStreamSource.h"
#include "plSecureStream.h"
#include "plEncryptedStream.h"
#include "hsLockGuard.h"

#if HS_BUILD_FOR_UNIX
#    include <wctype.h>
#endif

plStreamSource::plStreamSource()
{
    memset(fServerKey, 0, std::size(fServerKey));
}

void plStreamSource::ICleanup()
{
    hsLockGuard(fMutex);

    // loop through all the file data records, and delete the streams
    decltype(fFileData.begin()) curData;
    for (curData = fFileData.begin(); curData != fFileData.end(); curData++)
    {
        delete curData->second.fStream;
        curData->second.fStream = nullptr;
    }

    fFileData.clear();
}

hsStream* plStreamSource::GetFile(const plFileName& filename)
{
    hsLockGuard(fMutex);

    plFileName sFilename = filename.Normalize('/');
    if (fFileData.find(sFilename) == fFileData.end())
    {
#ifndef PLASMA_EXTERNAL_RELEASE
        // internal releases can pull from disk
        if (plFileInfo(filename).Exists())
        {
            // file exists on disk, cache it
            fFileData[sFilename].fFilename = sFilename;
            fFileData[sFilename].fDir = sFilename.StripFileName();
            fFileData[sFilename].fExt = sFilename.GetFileExt();
            if (plSecureStream::IsSecureFile(filename))
            {
                hsStream* ss = nullptr;

                uint32_t encryptionKey[4];
                if (plSecureStream::GetSecureEncryptionKey(filename, encryptionKey, 4))
                    ss = plSecureStream::OpenSecureFile(filename, 0, encryptionKey);
                else
                    ss = plSecureStream::OpenSecureFile(filename, 0, fServerKey);
                fFileData[sFilename].fStream = ss;
                hsAssert(ss, "failed to open a SecureStream for a disc file!");
            }
            else // otherwise it is an encrypted or plain stream, this call handles both
                fFileData[sFilename].fStream = plEncryptedStream::OpenEncryptedFile(filename);

            return fFileData[sFilename].fStream;
        }
#endif // PLASMA_EXTERNAL_RELEASE
        return nullptr;
    }
    return fFileData[sFilename].fStream;
}

std::vector<plFileName> plStreamSource::GetListOfNames(const plFileName& dir, const ST::string& ext)
{
    plFileName sDir = dir.Normalize('/');
    hsAssert(ext.front() != '.', "Don't add a dot");
    hsLockGuard(fMutex);

    // loop through all the file data records, and create the list
    std::vector<plFileName> retVal;
    for (auto curData = fFileData.begin(); curData != fFileData.end(); curData++)
    {
        if ((curData->second.fDir.AsString().compare_i(sDir.AsString()) == 0) &&
            (curData->second.fExt.compare_i(ext) == 0))
            retVal.push_back(curData->second.fFilename);
    }

#ifndef PLASMA_EXTERNAL_RELEASE
    // in internal releases, we can use on-disk files if they exist
    // Build the search string as "dir/*.ext"
    std::vector<plFileName> files = plFileSystem::ListDir(sDir, ("*." + ext).c_str());
    for (auto iter = files.begin(); iter != files.end(); ++iter)
    {
        plFileName norm = iter->Normalize('/');
        if (fFileData.find(norm) == fFileData.end()) // we haven't added it yet
            retVal.push_back(norm);
    }
#endif // PLASMA_EXTERNAL_RELEASE

    return retVal;
}

bool plStreamSource::InsertFile(const plFileName& filename, hsStream* stream)
{
    plFileName sFilename = filename.Normalize('/');

    hsLockGuard(fMutex);
    if (fFileData.find(sFilename) != fFileData.end())
        return false; // duplicate entry, return failure

    // copy the data over (takes ownership of the stream!)
    fFileData[sFilename].fFilename = sFilename;
    fFileData[sFilename].fDir = sFilename.StripFileName();
    fFileData[sFilename].fExt = sFilename.GetFileExt();
    fFileData[sFilename].fStream = stream;

    return true;
}

plStreamSource* plStreamSource::GetInstance()
{
    static plStreamSource source;
    return &source;
}
