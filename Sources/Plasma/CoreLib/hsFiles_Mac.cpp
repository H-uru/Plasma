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
#include "hsUtils.h"
#include "hsMemory.h"

#if HS_BUILD_FOR_MAC

#include <Files.h>
#include <Folders.h>
#include <Errors.h>

///////////////////////////////////////////////////////////////////////////

hsMacFile::hsMacFile() : fFlags(kRefNum_Dirty)
{
    fSpec.name[0] = 0;
}

hsMacFile::hsMacFile(const char pathAndName[]) : hsFile(pathAndName), fFlags(kRefNum_Dirty)
{
    this->SetSpecFromName();
}

hsMacFile::hsMacFile(const FSSpec* spec) : fFlags(kRefNum_Dirty)
{
    this->SetSpec(spec);
}

hsMacFile::~hsMacFile()
{
    this->Close();
}

void hsMacFile::SetSpec(const FSSpec* spec)
{
    if (spec)
        fSpec = *spec;
    else
        fSpec.name[0] = 0;
    fFlags |= kPathName_Dirty;
}

void hsMacFile::SetSpecFromName()
{
    Str255  pstr;

    if (fPathAndName == nil)
        fSpec.name[0] = 0;
    else
    {   hsC2PString(fPathAndName, pstr);
        ::FSMakeFSSpec(0, 0, pstr,  &fSpec);
    }
}

void hsMacFile::SetNameFromSpec()
{
    CInfoPBRec  pb;
    Str255      dirNameP;
    char            dirName[256], temp[256];
    int         err;
    
    hsP2CString(fSpec.name, temp);
    
    pb.dirInfo.ioNamePtr = dirNameP;
    pb.dirInfo.ioVRefNum = fSpec.vRefNum;
    pb.dirInfo.ioDrParID = fSpec.parID;
    pb.dirInfo.ioFDirIndex = -1;
    
    do {
        pb.dirInfo.ioDrDirID = pb.dirInfo.ioDrParID;
        err = PBGetCatInfoSync(&pb);
        hsThrowIfOSErr(err);

        hsP2CString(dirNameP, dirName);
        strcat(dirName,":");
        strcat(dirName, temp);
        strcpy(temp, dirName);  
    } while( pb.dirInfo.ioDrDirID != fsRtDirID);

    hsAssert(fPathAndName == nil, "pathname should be nil");
    fPathAndName = hsStrcpy(temp);
}

bool hsMacFile::Create(OSType creator, OSType fileType, ScriptCode scriptCode)
{
    this->Close();

    OSErr   err;

    (void)::FSpDelete(&fSpec);
    err = ::FSpCreate(&fSpec, creator, fileType, scriptCode);
    hsIfDebugMessage(err != 0, "FSpCreate failed", err);

    return err == 0;
}

#define kFileNotFound_Err       -43

bool hsMacFile::OpenDataFork(Sint8_t perm, int16_t* refnum)
{
    this->Close();

    OSErr   err;
    
    err = ::FSpOpenDF(&fSpec, perm, &fRefNum);
    if (err == kFileNotFound_Err && (perm & fsWrPerm) && (perm & fsRdPerm) == 0)
    {   if (this->Create('HdSp', '????'))
            err = ::FSpOpenDF(&fSpec, perm, &fRefNum);
    }

    if (err == 0)
    {   fFlags &= ~kRefNum_Dirty;
        if (refnum)
            *refnum = fRefNum;
        return true;
    }
    return false;
}

const char* hsMacFile::GetPathAndName()
{
    if (fFlags & kPathName_Dirty)
    {   this->SetNameFromSpec();
        fFlags &= ~kPathName_Dirty;
    }
    return fPathAndName;
}

void hsMacFile::SetPathAndName(const char pathAndName[])
{
    this->hsFile::SetPathAndName(pathAndName);
    this->SetSpecFromName();
}

hsStream* hsMacFile::OpenStream(const char mode[], bool throwIfFailure)
{
    hsThrowIfNilParam(mode);

    short   refnum;
    Sint8_t   perm = 0;
    
    if (::strchr(mode, 'r'))
        perm |= fsRdPerm;
    if (::strchr(mode, 'w'))
        perm |= fsWrPerm;
    
    if (this->OpenDataFork(perm, &refnum))
    {   hsFileStream*   stream = new hsFileStream;
        stream->SetFileRef(refnum);
        return stream;
    }
    
    hsThrowIfTrue(throwIfFailure);
    return nil;
}

void hsMacFile::Close()
{
    if (fFlags & kRefNum_Dirty)
    {   OSErr   err = ::FSClose(fRefNum);
        hsIfDebugMessage(err != 0, "FSClose failed", err);
        fFlags &= ~kRefNum_Dirty;
    }
    this->hsFile::Close();
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

struct hsFolderIterator_Data {
    FSSpec  fSpec;
    OSType  fFileType;
    OSType  fCreator;
    char    fCName[_MAX_PATH];
    int16_t   fCurrIndex;
    bool    fValid;
};

hsFolderIterator::hsFolderIterator(const char path[])
{
    fData = new hsFolderIterator_Data;

    fData->fCurrIndex = 0;
    fData->fValid = false;
#if HS_BUILD_FOR_WIN32
    this->SetPath(path);
#else 
    this->SetMacFolder(path);
#endif
}

hsFolderIterator::hsFolderIterator(const struct FSSpec* spec)       // Alt Constructor - pass in FSSpec from OpenDlg()
{
    fData = new hsFolderIterator_Data;

    fData->fCurrIndex = 0;
    fData->fValid = false;
    
    SetMacFolder(spec->vRefNum,  spec->parID);
}

hsFolderIterator::~hsFolderIterator()
{
    delete fData;
}

void hsFolderIterator::SetPath(const char path[])
{
    fPath[0] = 0;
    fData->fValid = false;
    fData->fCurrIndex = 0;

    if (path)
    {   
        ::strcpy(fPath, path);
    }
}

///////////////////////////////////////////////////////////////////////////

void hsFolderIterator::SetMacFolder(OSType folderType)
{
    fData->fCurrIndex = 0;
    fData->fValid = ::FindFolder(kOnSystemDisk, folderType, false,
                        &fData->fSpec.vRefNum, &fData->fSpec.parID) == 0;
    this->Reset();
}

void hsFolderIterator::SetMacFolder(int16_t vRefNum, int32_t dirID)
{
    fData->fSpec.vRefNum    = vRefNum;
    fData->fSpec.parID      = dirID;
    fData->fCurrIndex = 0;
    fData->fValid = true;
    this->Reset();
}

void hsFolderIterator::SetMacFolder(const char path[])
{
    char tmp[255];
    FSSpec fileSpec; 
    OSErr err; 
    
    hsCPathToMacPath(&tmp[1], (char*)path); 
    tmp[0] = hsStrlen(&tmp[1]);
    SetPath((char*)&tmp[1]);
    
    err = FSMakeFSSpec(0, 0, (const unsigned char*)tmp, &fileSpec);
    if(err == fnfErr) 
    {
        HSDebugProc("XCmd directory does not exist.");
        return;
    }
    hsAssert(err == noErr, "Error making file spec.");
    
    // by now we should have the file spec for the given 
    // directory, however, the DirID is the PARENT directory, 
    // not the child directory. The following steps should 
    // give us the items we want.
    
    CInfoPBRec pb;
    pb.hFileInfo.ioVRefNum      = fileSpec.vRefNum;
    pb.hFileInfo.ioNamePtr      = (StringPtr)fileSpec.name; // The name of the child directory.
    pb.hFileInfo.ioDirID        = fileSpec.parID;           // The ID of the parent directory.
    pb.hFileInfo.ioFDirIndex    = 0;
    pb.hFileInfo.ioCompletion   = 0;
    
    err = ::PBGetCatInfoSync(&pb);
    hsAssert(err == noErr, "PBGetCatInfoSync() failure.");
    
    fData->fSpec.vRefNum    = fileSpec.vRefNum;             // Volume reference
    fData->fSpec.parID      = pb.dirInfo.ioDrDirID;         // child directory ID (Finally!)
    fData->fCurrIndex = 0;
    fData->fValid = true;
    
    this->Reset();
}

///////////////////////////////////////////////////////////////////////////

void hsFolderIterator::Reset()
{
    if (fData->fValid)
        fData->fCurrIndex = 1;
#ifdef HS_DEBUGGING
    else
        hsAssert(fData->fCurrIndex == 0, "bad currindex");
#endif
}

bool hsFolderIterator::NextFile()
{
    if (fData->fCurrIndex == 0)
        return false;

    CInfoPBRec  pb;

    do {
        pb.hFileInfo.ioVRefNum      = fData->fSpec.vRefNum;
        pb.hFileInfo.ioNamePtr      = (StringPtr)fData->fSpec.name;
        pb.hFileInfo.ioDirID        = fData->fSpec.parID;
        pb.hFileInfo.ioFDirIndex    = fData->fCurrIndex++;

        OSErr err = ::PBGetCatInfoSync(&pb);
        if (err)
        {   
            fData->fCurrIndex = 0;
            return false;
        }
    } while (pb.hFileInfo.ioFlAttrib & ioDirMask);

    fData->fFileType = pb.hFileInfo.ioFlFndrInfo.fdType;
    fData->fCreator  = pb.hFileInfo.ioFlFndrInfo.fdCreator;
    return true;
}

const char* hsFolderIterator::GetFileName() const
{
    if (fData->fCurrIndex == 0)
        throw "end of folder";

    //  Copy our filename (in pascal format) into a cstring and then return
    HSMemory::BlockMove(&fData->fSpec.name[1], fData->fCName, fData->fSpec.name[0]);
    fData->fCName[fData->fSpec.name[0]] = 0;
    return fData->fCName;
}

////////////////////////////////////////////////////////////////////////////

bool hsFolderIterator::NextMacFile(OSType targetFileType, OSType targetCreator)
{
    for (;;)
    {   if (this->NextFile() == false)
            return false;
        if ((targetFileType == 0 || targetFileType == this->GetMacFileType()) &&
            (targetCreator == 0 || targetCreator == this->GetMacCreator()))
                return true;
    }       
}

const FSSpec* hsFolderIterator::GetMacSpec() const
{
    if (fData->fCurrIndex == 0)
        throw "end of folder";
    return &fData->fSpec;
}


OSType hsFolderIterator::GetMacFileType() const
{
    if (fData->fCurrIndex == 0)
        throw "end of folder";
    return fData->fFileType;
}

OSType hsFolderIterator::GetMacCreator() const
{
    if (fData->fCurrIndex == 0)
        throw "end of folder";
    return fData->fCreator;
}

bool    hsFolderIterator::IsDirectory( void ) const
{
    hsAssert( false, "hsFolderIterator::IsDirectory() not defined on this platform!!!" );
    return false;
}

#endif  // HS_BUILD_FOR_MAC
