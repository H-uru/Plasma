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

#ifndef _plAgeManifest_h
#define _plAgeManifest_h

#include "HeadSpin.h"
#include "plFileSystem.h"
#include <vector>

#include "pnEncryption/plChecksum.h"
#include "plUnifiedTime/plUnifiedTime.h"

class hsStream;

class plMfsLine
{
protected:
    plFileName fName, fDownload;
    plMD5Checksum fChecksum;
    uint32_t fSize, fCompressedSize, fFlags;
    plUnifiedTime fModifyTime;

    plMfsLine() { }

public:
    enum Flags
    {
        // DEAD: Sound Flags
        kSndFlagCacheSplit          = 1<<0,
        kSndFlagStreamCompressed    = 1<<1,
        kSndFlagCacheStereo         = 1<<2,

        // Any file
        kFlagZipped                 = 1<<3,
    };

    plMfsLine(const plString& data);
    plMfsLine(const plFileName& fn, const plFileName& dload, const plMD5Checksum& md5, uint32_t size,
              uint32_t zipSize, uint32_t flags, const plUnifiedTime& tm)
              : fName(fn), fDownload(dload), fChecksum(md5), fSize(size), fCompressedSize(zipSize),
                fFlags(flags), fModifyTime(tm)
    { }

    const uint32_t GetDownloadSize() const
    {
        if (fFlags & kFlagZipped)
            return fCompressedSize;
        else
            return fSize;
    }

    const plFileName& GetFileName() const { return fName; }
    const plFileName& GetDownloadPath() const { return fDownload; }
    uint32_t GetFileSize() const { return fSize; }
    uint32_t GetCompressedSize() const { return fCompressedSize; }
    uint32_t GetFlags() const { return fFlags; }
    const plUnifiedTime& GetModifyTime() const { fModifyTime; }
};

class plManifestFile : public plMfsLine
{
public:
    class Patch : public plMfsLine
    {
        plMD5Checksum fBeforeChecksum;
        plMD5Checksum fAfterChecksum;

    public:
        Patch(const plString& data);

        const plMD5Checksum& GetAfterChecksum() const { return fAfterChecksum; }
        const plMD5Checksum& GetBeforeChecksum() const { return fBeforeChecksum; }
    };

private:
    std::vector<Patch*> fPatches;

public:
    plManifestFile(const plString& data) : plMfsLine(data) { }
    plManifestFile(const plFileName& fn, const plFileName& dload, const plMD5Checksum& md5, uint32_t size,
                   uint32_t zipSize, uint32_t flags, const plUnifiedTime& tm)
                   : plMfsLine(fn, dload, md5, size, zipSize, flags, tm)
    { }
    ~plManifestFile();

    const plMD5Checksum& GetChecksum() const { return fChecksum; }
    std::vector<Patch*>& GetPatches() { return fPatches; }
};

class plManifest
{
    std::vector<plManifestFile*> fFiles;

public:
    plManifest() { }
    ~plManifest();

    plManifestFile* FindFile(const plFileName& name);

    const std::vector<plManifestFile*>& GetFiles() const { return fFiles; }
    std::vector<plManifestFile*>& GetFiles() { return fFiles; }

    bool Read(hsStream* const s);
    bool Read(const void* buf, size_t bufsz);
    bool ReadLegacy(hsStream* const s);
    bool ReadLegacy(const void* buf, size_t bufsz);
    void Write(hsStream* const s) const;
};

/** Deprecated -- Only for use in plUruLauncher **/
struct NetCliFileManifestEntry {
    wchar_t       clientName[_MAX_PATH];   // path and file on client side (for comparison)
    wchar_t       downloadName[_MAX_PATH]; // path and file on server side (for download)
    plMD5Checksum md5;
    uint32_t      fileSize;
    uint32_t      zipSize;
    uint32_t      flags;

    NetCliFileManifestEntry()
        : fileSize(0), zipSize(0), flags(0)
    { }

    NetCliFileManifestEntry(const plManifestFile& mfs);

    static NetCliFileManifestEntry* FromManifest(const plManifest* const mfs);
};

#endif //_plAgeManifest_h
