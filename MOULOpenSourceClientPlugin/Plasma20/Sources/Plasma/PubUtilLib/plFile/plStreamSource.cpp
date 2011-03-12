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
#include "hsUtils.h"
#include "hsFiles.h"
#include "plStreamSource.h"
#include "plSecureStream.h"
#include "plEncryptedStream.h"
#include "plFileUtils.h"

void ToLower(std::wstring& str)
{
	for (unsigned i = 0; i < str.length(); i++)
		str[i] = towlower(str[i]);
}

void ReplaceSlashes(std::wstring& path, wchar replaceWith)
{
	for (unsigned i = 0; i < path.length(); i++)
	{
		if ((path[i] == L'\\') || (path[i] == L'/'))
			path[i] = replaceWith;
	}
}

void plStreamSource::ICleanup()
{
	// loop through all the file data records, and delete the streams
	std::map<std::wstring, fileData>::iterator curData;
	for (curData = fFileData.begin(); curData != fFileData.end(); curData++)
	{
		curData->second.fStream->Close();
		delete curData->second.fStream;
		curData->second.fStream = nil;
	}

	fFileData.clear();
}

void plStreamSource::IBreakupFilename(std::wstring filename, std::wstring& dir, std::wstring& ext)
{
	// break the filename up into its parts
	char* temp = hsWStringToString(filename.c_str());
	std::string sFilename = temp;
	std::string sExt = plFileUtils::GetFileExt(temp);
	plFileUtils::StripFile(temp);
	std::string sDir = temp;
	delete [] temp;

	if (sDir == sFilename) // no directory
		sDir = "";
	if (sDir != "")
		if ((sDir[sDir.length()-1] == '/') || (sDir[sDir.length()-1] == '\\'))
			sDir = sDir.substr(0, sDir.length() - 1); // trim the slash, if it has one

	wchar_t* wTemp;
	wTemp = hsStringToWString(sDir.c_str());
	dir = wTemp;
	delete [] wTemp;
	wTemp = hsStringToWString(sExt.c_str());
	ext = wTemp;
	delete [] wTemp;
}

hsStream* plStreamSource::GetFile(std::wstring filename)
{
	ToLower(filename);
	ReplaceSlashes(filename, L'/');

	if (fFileData.find(filename) == fFileData.end())
	{
#ifndef PLASMA_EXTERNAL_RELEASE
		// internal releases can pull from disk
		char* temp = hsWStringToString(filename.c_str());
		std::string sFilename = temp;
		delete [] temp;

		if (plFileUtils::FileExists(sFilename.c_str()))
		{
			// file exists on disk, cache it
			std::wstring dir, ext;
			IBreakupFilename(filename, dir, ext);
			fFileData[filename].fFilename = filename;
			fFileData[filename].fDir = dir;
			fFileData[filename].fExt = ext;
			if (plSecureStream::IsSecureFile(sFilename.c_str()))
			{
				UInt32 encryptionKey[4];
				plFileUtils::GetSecureEncryptionKey(sFilename.c_str(), encryptionKey, 4);
				fFileData[filename].fStream = plSecureStream::OpenSecureFile(sFilename.c_str(), 0, encryptionKey);
			}
			else // otherwise it is an encrypted or plain stream, this call handles both
				fFileData[filename].fStream = plEncryptedStream::OpenEncryptedFile(sFilename.c_str());

			return fFileData[filename].fStream;
		}
#endif // PLASMA_EXTERNAL_RELEASE
		return nil;
	}
	return fFileData[filename].fStream;
}

std::vector<std::wstring> plStreamSource::GetListOfNames(std::wstring dir, std::wstring ext)
{
	ToLower(ext);
	ToLower(dir);
	ReplaceSlashes(dir, L'/');

	if (ext[0] == L'.')
		ext = ext.substr(1, ext.length()); // trim the dot, if it has one
	if (dir != L"")
		if ((dir[dir.length()-1] == L'/') || (dir[dir.length()-1] == L'\\'))
			dir = dir.substr(0, dir.length() - 1); // trim the slash, if it has one

	// loop through all the file data records, and create the list
	std::vector<std::wstring> retVal;
	std::map<std::wstring, fileData>::iterator curData;
	for (curData = fFileData.begin(); curData != fFileData.end(); curData++)
	{
		if ((curData->second.fDir == dir.c_str()) && (curData->second.fExt == ext))
			retVal.push_back(curData->second.fFilename);
	}

#ifndef PLASMA_EXTERNAL_RELEASE
	// in internal releases, we can use on-disk files if they exist
	// Build the search string as "dir/*.ext"
	std::wstring wSearchStr = dir + L"/*." + ext;
	char* temp = hsWStringToString(wSearchStr.c_str());
	std::string searchStr = temp;
	delete [] temp;

	hsFolderIterator folderIter(searchStr.c_str(), true);
	while (folderIter.NextFile())
	{
		const char* filename = folderIter.GetFileName();
		wchar_t* wTemp = hsStringToWString(filename);
		std::wstring wFilename = dir + L"/" + wTemp;
		delete [] wTemp;
		ToLower(wFilename);
		
		if (fFileData.find(wFilename) == fFileData.end()) // we haven't added it yet
			retVal.push_back(wFilename);
	}
#endif // PLASMA_EXTERNAL_RELEASE

	return retVal;
}

bool plStreamSource::InsertFile(std::wstring filename, hsStream* stream)
{
	ToLower(filename);
	ReplaceSlashes(filename, L'/');

	if (fFileData.find(filename) != fFileData.end())
		return false; // duplicate entry, return failure

	// break the filename up into its parts
	std::wstring dir, ext;
	IBreakupFilename(filename, dir, ext);

	// copy the data over (takes ownership of the stream!)
	fFileData[filename].fFilename = filename;
	fFileData[filename].fDir = dir;
	fFileData[filename].fExt = ext;
	fFileData[filename].fStream = stream;

	return true;
}

plStreamSource* plStreamSource::GetInstance()
{
	static plStreamSource source;
	return &source;
}