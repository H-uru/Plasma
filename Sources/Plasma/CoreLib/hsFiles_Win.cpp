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
#include "HeadSpin.h"

#if HS_BUILD_FOR_WIN32

#include "hsExceptions.h"

struct hsFolderIterator_Data {
    HANDLE          fSearchHandle;
    WIN32_FIND_DATA fFindData;
    bool         fValid;
};

hsFolderIterator::hsFolderIterator(const char path[], bool useCustomFilter)
{
   fCustomFilter = useCustomFilter;

    fData = new hsFolderIterator_Data;
   fData->fSearchHandle = nil;
   fData->fValid = true;
   
   if(useCustomFilter)
   {
      this->SetFileFilterStr(path);
   }
   else
   {
      this->SetPath(path);
   }
}

hsFolderIterator::~hsFolderIterator()
{
   delete fData;
}

void hsFolderIterator::SetPath(const char path[])
{
   fCustomFilter = false;
   fPath[0] = 0;
   if (path)
   {
      ::strcpy(fPath, path);
      
      // Make sure the dir ends with a slash
      char lastchar = fPath[strlen(fPath)-1];
      if (lastchar != '\\' && lastchar != '/')
         strcat(fPath, "\\");
   }
   
   this->Reset();
}

void hsFolderIterator::SetWinSystemDir(const char subdir[])
{
   int ret = GetWindowsDirectory(fPath, _MAX_PATH);
   hsAssert(ret != 0, "Error getting windows directory in UseWindowsFontsPath");
   
   if (subdir)
   {    ::strcat(fPath, "\\");
   ::strcat(fPath, subdir);
   ::strcat(fPath, "\\");
   }
   this->Reset();
}

void hsFolderIterator::SetFileFilterStr(const char filterStr[])
{
   fPath[0] = 0;
   if (filterStr)
   {
      fCustomFilter = true;
      ::strcpy(fPath, filterStr);
   }
   
   this->Reset();
}

///////////////////////////////////////////////////////////////////////////////

void hsFolderIterator::Reset()
{
    if (fData->fSearchHandle)
    {   FindClose(fData->fSearchHandle);
        fData->fSearchHandle = nil;
    }
    fData->fValid = true;
}

bool hsFolderIterator::NextFile()
{
    if (fData->fValid == false)
        return false;

    if (fData->fSearchHandle == nil)
    {   int len = ::strlen(fPath);

      if(fCustomFilter == false)
      {
         fPath[len] = '*';
         fPath[len+1] = 0;
      }
      
        fData->fSearchHandle = FindFirstFile(fPath, &fData->fFindData);
        fPath[len] = 0;

        if (fData->fSearchHandle == INVALID_HANDLE_VALUE)
        {   fData->fSearchHandle = nil;
            fData->fValid = false;
            return false;
        }
    }
    else
    {   if (FindNextFile(fData->fSearchHandle, &fData->fFindData) == false)
        {   FindClose(fData->fSearchHandle);
            fData->fSearchHandle = nil;
            fData->fValid = false;
            return false;
        }
    }

    return true;
}

bool    hsFolderIterator::IsDirectory( void ) const
{
    if( fData->fValid && ( fData->fFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
        return true;

    return false;
}

const char* hsFolderIterator::GetFileName() const
{
    if (fData->fValid == false)
        hsThrow( "end of folder");

    return fData->fFindData.cFileName;
}

///////////////////////////////////////////////////////////////////////////////

struct hsWFolderIterator_Data {
    HANDLE              fSearchHandle;
    WIN32_FIND_DATAW    fFindData;
    bool             fValid;
};

hsWFolderIterator::hsWFolderIterator(const wchar_t path[], bool useCustomFilter)
{
    fCustomFilter = useCustomFilter;

    fData = new hsWFolderIterator_Data;
    fData->fSearchHandle = nil;
    fData->fValid = true;

    if(useCustomFilter)
        SetFileFilterStr(path);
    else
        SetPath(path);
}

hsWFolderIterator::~hsWFolderIterator()
{
    delete fData;
}

void hsWFolderIterator::SetPath(const wchar_t path[])
{
    fCustomFilter = false;
    fPath[0] = 0;
    if (path)
    {
        wcscpy(fPath, path);

        // Make sure the dir ends with a slash
        wchar_t lastchar = fPath[wcslen(fPath)-1];
        if (lastchar != L'\\' && lastchar != L'/')
            wcscat(fPath, L"\\");
    }

    Reset();
}

void hsWFolderIterator::SetWinSystemDir(const wchar_t subdir[])
{
    int ret = GetWindowsDirectoryW(fPath, _MAX_PATH);
    hsAssert(ret != 0, "Error getting windows directory in UseWindowsFontsPath");

    if (subdir)
    {
        wcscat(fPath, L"\\");
        wcscat(fPath, subdir);
        wcscat(fPath, L"\\");
    }
    Reset();
}

void hsWFolderIterator::SetFileFilterStr(const wchar_t filterStr[])
{
    fPath[0] = 0;
    if (filterStr)
    {
        fCustomFilter = true;
        wcscpy(fPath, filterStr);
    }

    Reset();
}

///////////////////////////////////////////////////////////////////////////////

void hsWFolderIterator::Reset()
{
    if (fData->fSearchHandle)
    {
        FindClose(fData->fSearchHandle);
        fData->fSearchHandle = nil;
    }
    fData->fValid = true;
}

bool hsWFolderIterator::NextFile()
{
    if (fData->fValid == false)
        return false;

    if (fData->fSearchHandle == nil)
    {
        int len = wcslen(fPath);

        if(fCustomFilter == false)
        {
            fPath[len] = L'*';
            fPath[len+1] = L'\0';
        }

        fData->fSearchHandle = FindFirstFileW(fPath, &fData->fFindData);
        fPath[len] = 0;

        if (fData->fSearchHandle == INVALID_HANDLE_VALUE)
        {
            fData->fSearchHandle = nil;
            fData->fValid = false;
            return false;
        }
    }
    else
    {
        if (FindNextFileW(fData->fSearchHandle, &fData->fFindData) == false)
        {
            FindClose(fData->fSearchHandle);
            fData->fSearchHandle = nil;
            fData->fValid = false;
            return false;
        }
    }

    return true;
}

bool    hsWFolderIterator::IsDirectory( void ) const
{
    if( fData->fValid && ( fData->fFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
        return true;

    return false;
}

const wchar_t* hsWFolderIterator::GetFileName() const
{
    if (fData->fValid == false)
        hsThrow( "end of folder");

    return fData->fFindData.cFileName;
}

#endif  // HS_BUILD_FOR_WIN32