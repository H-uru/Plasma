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
//////////////////////////////////////////////////////////////////////////////
//																			//
//	pfConsoleDirSrc Functions												//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "pfConsoleDirSrc.h"

#include "hsExceptions.h"

#ifdef HS_BUILD_FOR_WIN32

#define WIN32_EXTRA_LEAN
#define WIN32_LEAN_AND_MEAN
#ifndef _WINDOWS_H_	// redundant include guard to minimize compile times
#define _WINDOWS_H_
#include <windows.h>
#endif // _WINDOWS_H_

#include <winbase.h>

#include <sstream>


//// ParseDirectory //////////////////////////////////////////////////////////

hsBool	pfConsoleDirSrc::ParseDirectory(const std::string& path, const std::string& mask /* = "*.*" */)
{
	wchar* wPath = hsStringToWString(path.c_str());
	wchar* wMask = hsStringToWString(mask.c_str());
	hsBool ret = ParseDirectory(wPath, wMask);
	delete [] wPath;
	delete [] wMask;
	return ret;
}

hsBool	pfConsoleDirSrc::ParseDirectory(const std::wstring& path, const std::wstring& mask /* = L"*.*" */)
{
	std::wstringstream	search;
	std::wstring		file;
	WIN32_FIND_DATAW 	findInfo;
	HANDLE				handle;

	hsAssert( fEngine != nil, "Cannot do a dir execute without an engine!" );

	search << path << L"\\" << mask;
	handle = FindFirstFileW(search.str().c_str(), &findInfo);
	if (handle == INVALID_HANDLE_VALUE)
		return false;

	do
	{
		if (!( findInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) 
		{
			std::wstringstream fileAndPath;
			fileAndPath << path << L"\\" << findInfo.cFileName;
			if (AlreadyProcessedFile(path, findInfo.cFileName))
				continue;
			AddProcessedFile(path, findInfo.cFileName);
			if (!fEngine->ExecuteFile(fileAndPath.str().c_str()))
			{
				// Change the following line once we have a better way of reporting
				// errors in the parsing
				std::wstringstream error;
				std::wstringstream caption;
				wchar* errorMsg = hsStringToWString(fEngine->GetErrorMsg());
				wchar* errorLine = hsStringToWString(fEngine->GetLastErrorLine());

				caption << L"Error parsing " << findInfo.cFileName;
				error << errorMsg << L":\n\nCommand: '" << errorLine << L"'\n\nPress OK to continue parsing files.";

				hsMessageBox(error.str().c_str(), caption.str().c_str(), hsMessageBoxNormal);				
				
				delete [] errorMsg;
				delete [] errorLine;

				FindClose(handle);
				SetCheckProcessedFiles(true);
				return false;
			}
		}
	} while (FindNextFileW(handle, &findInfo) != 0);

	FindClose(handle);
	SetCheckProcessedFiles(true);
	return true;
}

#else

#error This needs to be implemented for this platform!!!!

#endif

void pfConsoleDirSrc::ResetProcessedFiles()
{
	int i;
	for(i=0;i<fProcessedFiles.size(); i++)
		delete fProcessedFiles[i];
	fProcessedFiles.clear();	
}

//
// note: this n^2 linear search should be replaced with something
// faster if we have lots of init files and turn on the checkProcessing option.
//
hsBool pfConsoleDirSrc::AlreadyProcessedFile(const std::wstring& path, const std::wstring& file)
{
	if (fCheckProcessedFiles)
	{
		int i;
		for(i=0; i<fProcessedFiles.size(); i++)
		{
			if (file == fProcessedFiles[i]->fFile && path == fProcessedFiles[i]->fPath)
				return true;
		}
	}
	return false;
}

void pfConsoleDirSrc::AddProcessedFile(const std::wstring& path, const std::wstring& file)
{
	fProcessedFiles.push_back(TRACKED_NEW FileName(path, file));	
}
