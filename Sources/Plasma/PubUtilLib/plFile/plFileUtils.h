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
//
//	plFileUtils - Namespace of fun file utilities
//
//// History /////////////////////////////////////////////////////////////////
//
//	5.7.2002 mcn	- Created
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plFileUtils_h
#define _plFileUtils_h

class plUnifiedTime;

namespace plFileUtils
{
	static const char kKeyFilename[] = "encryption.key";
	static const wchar kWKeyFilename[] = L"encryption.key";

	// Creates the directory specified. Returns false if unsuccessful or directory already exists
	hsBool	CreateDir( const char *path ); 
	hsBool	CreateDir( const wchar *path ); 
	hsBool RemoveDir(const char* path);
	hsBool RemoveDirTree(const char * path);

	// delete file from disk
	bool RemoveFile(const char* filename, bool delReadOnly=false);
	bool RemoveFile(const wchar* filename, bool delReadOnly=false);

	bool FileCopy(const char* existingFile, const char* newFile);
	bool FileCopy(const wchar* existingFile, const wchar* newFile);
	bool FileMove(const char* existingFile, const char* newFile);
	bool FileMove(const wchar* existingFile, const wchar* newFile);

	bool FileExists(const char* file);
	bool FileExists(const wchar* file);

	// Given a filename with path, makes sure the file's path exists
	hsBool	EnsureFilePathExists( const char *filename );
	hsBool	EnsureFilePathExists( const wchar *filename );
	
	// Gets the creation and modification dates of the file specified. Returns false if unsuccessful
	hsBool	GetFileTimes( const char *path, plUnifiedTime *createTimeOut, plUnifiedTime *modifyTimeOut );
	// Compares file times, taking into account NTFS/FAT32 time issues
	enum Modify { kFileError, kFilesEqual, kFile1Newer, kFile2Newer };
	Modify CompareModifyTimes(const char* file1, const char* file2);
	// Set file modify time
	bool	SetModifyTime( const char * filename, const plUnifiedTime & time );

	// Return a pointer into the given string at the start of the actual filename (i.e. past any path info)
	const char* GetFileName(const char* pathAndName);
	const wchar* GetFileName(const wchar* pathAndName);
	// Get the file extension (without the .), or nil if it doesn't have one
	const char* GetFileExt(const char* pathAndName);
	const wchar* GetFileExt(const wchar* pathAndName);

	// Strips the filename off the given full path
	void StripFile(char* pathAndName);
	void StripFile(wchar* pathAndName);
	void StripExt(char* fileName);

	// Get the size of the given file in bytes
	UInt32		GetFileSize( const char *path );
	UInt32		GetFileSize( const wchar *path );

	// Adds a slash to the end of a filename (or does nothing if it's already there)
	void AddSlash(char* path);

	// Concatenates fileName onto path, making sure to add a slash if necessary
	void ConcatFileName(char* path, const char* fileName);

	// searches the parent directory of filename for the encryption key file, and reads it
	// into the key passed in. Returns false if the key file didn't exist (and sets key to
	// the default key)
	bool GetSecureEncryptionKey(const char* filename, UInt32* key, unsigned length);
	bool GetSecureEncryptionKey(const wchar* filename, UInt32* key, unsigned length);
};


#endif // _plFileUtils_h
