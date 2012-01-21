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
#include "hsFiles.h"
#include <string.h>


#include "hsExceptions.h"

#if HS_BUILD_FOR_WIN32
    #define kDirChar        '\\'
#else
    #define kDirChar        '/'
#endif


static const char* FindNameInPath(const char path[])
{
    const char* name = ::strrchr(path, kDirChar);
    
    if (name == nil)
        name = path;
    return name;
}

///////////////////////////////////////////////////////////////////////

hsFile::hsFile() : fPathAndName(nil), fFILE(nil)
{
}

hsFile::hsFile(const char pathAndName[]) : fPathAndName(nil), fFILE(nil)
{
    if (pathAndName)
        fPathAndName = hsStrcpy(pathAndName);
}

hsFile::~hsFile()
{
    this->SetPathAndName(nil);
}

const char* hsFile::GetPathAndName()
{
    return fPathAndName;
}

void hsFile::SetPathAndName(const char pathAndName[])
{
    this->Close();

    if (fPathAndName)
    {   delete[] fPathAndName;
        fPathAndName = nil;
    }
    if (pathAndName)
        fPathAndName = hsStrcpy(pathAndName);
}

const char* hsFile::GetName()
{
    return FindNameInPath(this->GetPathAndName());
}

FILE* hsFile::OpenFILE(const char mode[], hsBool throwIfFailure)
{
    this->Close();

    //  We call the virtual method here rather than using
    //  fPathAndName directly, allowing a subclass to construct
    //  the name if necessary
    //
    const char* name = this->GetPathAndName();
    if (name)
        fFILE = ::fopen(name, mode);

    hsThrowIfTrue(throwIfFailure && fFILE == nil);
    return fFILE;
}

hsStream* hsFile::OpenStream(const char mode[], hsBool throwIfFailure)
{
    FILE* file = this->OpenFILE(mode, throwIfFailure);

    if (file)
    {   hsUNIXStream*   stream = new hsUNIXStream;
        stream->SetFILE(file);
        return stream;
    }
    return nil;
}

void hsFile::Close()
{
    if (fFILE)
    {   int err = ::fflush(fFILE);
        hsIfDebugMessage(err != 0, "fflush failed", err);
        err = ::fclose(fFILE);
        hsIfDebugMessage(err != 0, "fclose failed", err);
        fFILE = nil;
    }
}

///////////////////////////////////////////////////////////////////////

hsBool hsFolderIterator::NextFileSuffix(const char suffix[])
{
    while (this->NextFile())
    {   const char* fileSuffix = ::strrchr(this->GetFileName(), '.');
        if (fileSuffix != nil && stricmp(fileSuffix, suffix) == 0)
            return true;
    }
    return false;
}

int hsFolderIterator::GetPathAndName(char pathandname[])
{
    const char* name = this->GetFileName();
    int         pathLen = hsStrlen(fPath);

    // add 1 for null terminator
    int totalLen = pathLen + sizeof(kDirChar) + hsStrlen(name) + 1;
    hsAssert(totalLen <= kFolderIterator_MaxPath, "Overrun kFolderIterator_MaxPath");

    if (pathandname)
    {   hsStrcpy(pathandname, fPath);
        if (pathLen > 0 && pathandname[pathLen - 1] != kDirChar)
            pathandname[pathLen++] = kDirChar;
        hsStrcpy(pathandname + pathLen, name);
    }
    return totalLen;
}

FILE* hsFolderIterator::OpenFILE(const char mode[])
{
    char fileName[kFolderIterator_MaxPath];

    (void)this->GetPathAndName(fileName);

    return ::fopen(fileName, mode);
}

