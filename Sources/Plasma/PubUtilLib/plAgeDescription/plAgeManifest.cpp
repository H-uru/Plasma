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

#include <algorithm>
#include <wchar.h>

#include "plAgeManifest.h"
#include "hsStream.h"

static const char* s_TimeFormat = "%m/%d/%y %H:%M:%S";

plMfsLine::plMfsLine(const plString& data)
{
    std::vector<plString> toks = data.Tokenize(",");
    if (toks.size() >= 6)
    {
        fName = toks[0];
        fDownload = toks[1];
        fSize = toks[2].ToUInt();
        fModifyTime.FromString(toks[3].c_str(), s_TimeFormat);
        fChecksum.SetFromHexString(toks[4].c_str());
        fFlags = toks[5].ToUInt();
        if ((fFlags & plManifestFile::kFlagZipped) && toks.size() >= 7)
            fCompressedSize = toks[6].ToUInt();
    }
    else
        hsAssert(false, "invalid manifest line size");
}

plManifestFile::~plManifestFile()
{
    std::for_each(fPatches.begin(), fPatches.end(),
        [] (Patch* patch) { delete patch; }
    );
}

plManifestFile::Patch::Patch(const plString& data)
{
    std::vector<plString> toks = data.Tokenize(",");
    if (toks.size() >= 7)
    {
        fName = toks[0];
        fDownload = toks[1];
        fSize = toks[2].ToUInt();
        fModifyTime.FromString(toks[3].c_str(), s_TimeFormat);
        fBeforeChecksum.SetFromHexString(toks[4].c_str());
        fAfterChecksum.SetFromHexString(toks[5].c_str());
        fFlags = toks[6].ToUInt();
        if ((fFlags & plManifestFile::kFlagZipped) && toks.size() >= 8)
            fCompressedSize = toks[7].ToUInt();
    }
    else
        hsAssert(false, "invalid patch line size");
}

//////////////////////////////////////////////////////////////////////////////

plManifest::~plManifest()
{
    std::for_each(fFiles.begin(), fFiles.end(),
        [] (plManifestFile* file) { delete file; }
    );
}

static bool IReadEapString(hsStream* const s, plString& result)
{
    uint16_t dest[260];
    for (size_t i = 0; i < arrsize(dest); ++i)
    {
        dest[i] = s->ReadLE16();
        if (dest[i] == 0)
        {
            result = plString::FromUtf16(dest, i);
            break;
        }
    }
    if (result.IsEmpty())
        return false;
    return !(s->AtEnd());
}

static bool IReadEapUInt(hsStream* const s, uint32_t& result)
{
    if (s->GetSizeLeft() < (3 * sizeof(uint16_t)))
        return false;
    result = (s->ReadLE16() << 16 | s->ReadLE16() & 0xFFFF);
    return s->ReadLE16() == 0;
}

bool plManifest::ReadLegacy(hsStream* const s)
{
    while (!s->AtEnd())
    {
        // Read in a ManifestFile
        plString filename;
        if (!IReadEapString(s, filename))
            return false;
        plString download;
        if (!IReadEapString(s, download))
            return false;
        plString md5;
        if (!IReadEapString(s, md5))
            return false;
        plString zipMd5;
        if (!IReadEapString(s, zipMd5))
            return false;
        uint32_t fileSize;
        if (!IReadEapUInt(s, fileSize))
            return false;
        uint32_t zipSize;
        if (!IReadEapUInt(s, zipSize))
            return false;
        uint32_t flags;
        if (!IReadEapUInt(s, flags))
            return false;

        // Fix the hash
        plMD5Checksum theMd5;
        theMd5.SetFromHexString(md5.c_str());
        // All files from legacy sources are zipped
        flags |= plManifestFile::kFlagZipped;

        plManifestFile* file = new plManifestFile(filename, download, theMd5, fileSize, zipSize, flags, plUnifiedTime::GetCurrent());
        fFiles.push_back(file);

        // A null short means we're done
        if (!s->AtEnd())
        {
            if (s->ReadLE16() == 0)
                break;
            else
                s->SetPosition(s->GetPosition()-sizeof(uint16_t));
        }
    }
    return true;
}

bool plManifest::ReadLegacy(const void* buf, size_t bufsz)
{
    hsReadOnlyStream stream(bufsz, buf);
    return ReadLegacy(&stream);
}

NetCliFileManifestEntry::NetCliFileManifestEntry(const plManifestFile& mfs)
        : md5(mfs.GetChecksum()), fileSize(mfs.GetFileSize()),
          zipSize(mfs.GetCompressedSize()), flags(mfs.GetFlags())
{
    wcsncpy(clientName, mfs.GetFileName().AsString().ToWchar(), arrsize(clientName));
    wcsncpy(downloadName, mfs.GetDownloadPath().AsString().ToWchar(), arrsize(downloadName));
}

NetCliFileManifestEntry* NetCliFileManifestEntry::FromManifest(const plManifest* const mfs)
{
    NetCliFileManifestEntry* arr = new NetCliFileManifestEntry[mfs->GetFiles().size()];
    for (size_t i = 0; i < mfs->GetFiles().size(); ++i)
        arr[i] = *(mfs->GetFiles().at(i));
    return arr;
}
