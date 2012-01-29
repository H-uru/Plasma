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
//  plAgeManifest - Collection of version-specific info about an age, such  //
//                  as the actual files constructing it, timestamps, and    //
//                  release versions.                                       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _plAgeManifest_h
#define _plAgeManifest_h

#include "HeadSpin.h"
#include "hsTemplates.h"

#include "plUnifiedTime/plUnifiedTime.h"
#include "plFile/plInitFileReader.h"
#include "plEncryption/plChecksum.h"


//// Small Container Classes for a Single File ///////////////////////////////

class plManifestFile
{
protected:
    std::string     fName;
    std::string     fServerPath;
    plMD5Checksum   fChecksum;
    uint32_t          fSize;
    uint32_t          fZippedSize;
    uint32_t          fFlags;

    bool            fMd5Checked;
    bool            fIsLocalUpToDate;
    bool            fLocalExists;

public:
    // fUser flags
    enum
    {
        // Sound files only
        kSndFlagCacheSplit          = 1<<0,
        kSndFlagStreamCompressed    = 1<<1,
        kSndFlagCacheStereo         = 1<<2,
        // Any file
        kFlagZipped                 = 1<<3,
    };

    plManifestFile(const char* name, const char* serverPath, const plMD5Checksum& check, uint32_t size, uint32_t zippedSize, uint32_t flags, bool md5Now = true);
    virtual ~plManifestFile();

    const char* GetName() const { return fName.c_str(); }
    const char* GetServerPath() const { return fServerPath.c_str(); }
    const plMD5Checksum& GetChecksum() const { return fChecksum; }
    uint32_t GetDiskSize() const { return fSize; }
    uint32_t GetDownloadSize() const { return hsCheckBits(fFlags, kFlagZipped) ? fZippedSize : fSize; }
    uint32_t GetFlags() const { return fFlags; }

    void    DoMd5Check();
    bool    IsLocalUpToDate();
    bool    LocalExists();
};

//// Actual Manifest Class ///////////////////////////////////////////////////

class plManifest
{
protected:
    uint32_t fFormatVersion;
    char* fAgeName;     // Mostly just for debugging

    hsTArray<plManifestFile*> fFiles;
    
    void IReset();

public:
    static const char* fTimeFormat;       // Standard string for the printed version of our timestamps

    void SetFormatVersion(uint32_t v) { fFormatVersion = v; }
    void AddFile(plManifestFile* file);

    plManifest();
    virtual ~plManifest();

    bool Read(const char* filename);
    bool Read(hsStream* stream);

    uint32_t GetFormatVersion() const { return fFormatVersion; }

    uint32_t GetNumFiles() const { return fFiles.GetCount(); }
    const plManifestFile& GetFile(uint32_t i) const { return *fFiles[i]; }
};

#endif //_plAgeManifest_h
