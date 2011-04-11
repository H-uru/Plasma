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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#include "hsFiles.h"

#if HS_BUILD_FOR_WIN32

#include <windows.h>

#include "hsExceptions.h"

struct hsFolderIterator_Data {
	HANDLE			fSearchHandle;
	WIN32_FIND_DATA	fFindData;
	Boolean			fValid;
};

hsFolderIterator::hsFolderIterator(const char path[], bool useCustomFilter)
{
   fCustomFilter = useCustomFilter;

	fData = TRACKED_NEW hsFolderIterator_Data;
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
   {	::strcat(fPath, "\\");
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
	{	FindClose(fData->fSearchHandle);
		fData->fSearchHandle = nil;
	}
	fData->fValid = true;
}

hsBool hsFolderIterator::NextFile()
{
	if (fData->fValid == false)
		return false;

	if (fData->fSearchHandle == nil)
	{	int	len = ::strlen(fPath);

      if(fCustomFilter == false)
      {
         fPath[len] = '*';
         fPath[len+1] = 0;
      }
      
		fData->fSearchHandle = FindFirstFile(fPath, &fData->fFindData);
		fPath[len] = 0;

		if (fData->fSearchHandle == INVALID_HANDLE_VALUE)
		{	fData->fSearchHandle = nil;
			fData->fValid = false;
			return false;
		}
	}
	else
	{	if (FindNextFile(fData->fSearchHandle, &fData->fFindData) == false)
		{	FindClose(fData->fSearchHandle);
			fData->fSearchHandle = nil;
			fData->fValid = false;
			return false;
		}
	}

	return true;
}

hsBool	hsFolderIterator::IsDirectory( void ) const
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
	HANDLE				fSearchHandle;
	WIN32_FIND_DATAW	fFindData;
	Boolean				fValid;
};

hsWFolderIterator::hsWFolderIterator(const wchar path[], bool useCustomFilter)
{
	fCustomFilter = useCustomFilter;

	fData = TRACKED_NEW hsWFolderIterator_Data;
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

void hsWFolderIterator::SetPath(const wchar path[])
{
	fCustomFilter = false;
	fPath[0] = 0;
	if (path)
	{
		wcscpy(fPath, path);

		// Make sure the dir ends with a slash
		wchar lastchar = fPath[wcslen(fPath)-1];
		if (lastchar != L'\\' && lastchar != L'/')
			wcscat(fPath, L"\\");
	}

	Reset();
}

void hsWFolderIterator::SetWinSystemDir(const wchar subdir[])
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

void hsWFolderIterator::SetFileFilterStr(const wchar filterStr[])
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

hsBool hsWFolderIterator::NextFile()
{
	if (fData->fValid == false)
		return false;

	if (fData->fSearchHandle == nil)
	{
		int	len = wcslen(fPath);

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

hsBool	hsWFolderIterator::IsDirectory( void ) const
{
	if( fData->fValid && ( fData->fFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
		return true;

	return false;
}

const wchar* hsWFolderIterator::GetFileName() const
{
	if (fData->fValid == false)
		hsThrow( "end of folder");

	return fData->fFindData.cFileName;
}

#endif	// HS_BUILD_FOR_WIN32