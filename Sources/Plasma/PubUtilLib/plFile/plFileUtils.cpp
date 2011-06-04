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
/////////////////////////////////////////////////////////////////////////////
//
//	plFileUtils - Namespace of fun file utilities
//
//// History /////////////////////////////////////////////////////////////////
//
//	5.7.2002 mcn	- Created
//	4.8.2003 chip	- added FileCopy and FileMove for Unix
//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "hsStlUtils.h"
#include "plFileUtils.h"
#include "hsFiles.h"
#include "hsStringTokenizer.h"
#include "hsWindows.h"

#include "../plUnifiedTime/plUnifiedTime.h"

#include "plSecureStream.h" // for the default key

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>


#if HS_BUILD_FOR_WIN32
#include <direct.h>
#include <io.h>
#endif

#if HS_BUILD_FOR_UNIX
#include <unistd.h>
#include <errno.h>
#include <utime.h>
#endif


//// CreateDir ///////////////////////////////////////////////////////////////
//	Creates the directory specified. Returns false if unsuccessful or 
//	directory already exists

hsBool	plFileUtils::CreateDir( const char *path )
{
	// Create our directory
#if HS_BUILD_FOR_WIN32
	return ( mkdir( path ) == 0 ) ? true : ( errno==EEXIST );
#elif HS_BUILD_FOR_UNIX
	return ( mkdir( path, 0777 ) == 0 ) ? true : ( errno==EEXIST );
#endif
}

hsBool	plFileUtils::CreateDir( const wchar *path )
{
	// Create our directory
#if HS_BUILD_FOR_WIN32
	return ( _wmkdir( path ) == 0 ) ? true : ( errno==EEXIST );
#elif HS_BUILD_FOR_UNIX
	return ( mkdir( path, 0777 ) == 0 ) ? true : ( errno==EEXIST );
#endif
}

hsBool plFileUtils::RemoveDir(const char* path)
{
	return (rmdir(path) == 0);
}

hsBool plFileUtils::RemoveDirTree(const char * path)
{
	hsFolderIterator it(path);
	while (it.NextFile())
	{
		const char * fname = it.GetFileName();
		if ( fname[0]=='.' )
			continue;
		char pathAndName[128];
		it.GetPathAndName(pathAndName);
		if ( it.IsDirectory() )
		{
			RemoveDirTree( pathAndName );
			RemoveDir( pathAndName );
		}
		else
		{
			RemoveFile( pathAndName );
		}
	}
	RemoveDir( path );

	return 1;
}

//// RemoveFile ////////////////////////////////////////////////////////////

bool plFileUtils::RemoveFile(const char* filename, bool delReadOnly)
{
	if (delReadOnly)
		chmod(filename, S_IWRITE);
	return (unlink(filename) == 0);
}

bool plFileUtils::RemoveFile(const wchar* filename, bool delReadOnly)
{
	if (delReadOnly)
		_wchmod(filename, S_IWRITE);
	return (_wunlink(filename) == 0);
}

bool plFileUtils::FileCopy(const char* existingFile, const char* newFile)
{
	wchar* wExisting = hsStringToWString(existingFile);
	wchar* wNew = hsStringToWString(newFile);
	bool ret = FileCopy(wExisting, wNew);
	delete [] wExisting;
	delete [] wNew;
	return ret;
}

bool plFileUtils::FileCopy(const wchar* existingFile, const wchar* newFile)
{
#if HS_BUILD_FOR_WIN32
	return (::CopyFileW(existingFile, newFile, FALSE) != 0);
#elif HS_BUILD_FOR_UNIX
	char data[1500];
	FILE* fp = fopen(existingFile, "rb");
	FILE* fw = fopen(newFile, "w");
	int num = 0;
	bool retVal =  true;
	if (fp && fw){
		while(!feof(fp)){
			num = fread(data, sizeof( char ), 1500, fp);
			if( ferror( fp ) ) {
				retVal = false;
				break;
			}
			fwrite(data, sizeof( char ), num, fw);
		}
		fclose(fp);
		fclose(fw);
	} else {
		retVal = false;
	}
	return retVal;
#else
	hsAssert(0, "Not implemented");
	return false;
#endif
}

bool plFileUtils::FileMove(const char* existingFile, const char* newFile)
{
#if HS_BUILD_FOR_WIN32
	return (::MoveFile(existingFile, newFile) != 0);
#elif HS_BUILD_FOR_UNIX
	FileCopy(existingFile,newFile);
	return( RemoveFile( existingFile )==0);
#else
	hsAssert(0, "Not implemented");
	return false;
#endif
}

bool plFileUtils::FileMove(const wchar* existingFile, const wchar* newFile)
{
#if HS_BUILD_FOR_WIN32
	return (::MoveFileW(existingFile, newFile) != 0);
#elif HS_BUILD_FOR_UNIX
	FileCopy(existingFile,newFile);
	return( RemoveFile( existingFile )==0);
#else
	hsAssert(0, "Not implemented");
	return false;
#endif
}

bool plFileUtils::FileExists(const wchar* file)
{
	FILE* fp = _wfopen(file, L"rb");
	bool retVal = (fp != nil);
	if (fp)
		fclose(fp);
	return retVal;
}

bool plFileUtils::FileExists(const char* file)
{
	FILE* fp = fopen(file, "rb");
	bool retVal = (fp != nil);
	if (fp)
		fclose(fp);
	return retVal;
}

//// EnsureFilePathExists ////////////////////////////////////////////////////
//	Given a filename with path, makes sure the file's path exists

hsBool	plFileUtils::EnsureFilePathExists( const char *filename )
{
	wchar* wFilename = hsStringToWString(filename);
	hsBool ret = EnsureFilePathExists(wFilename);
	delete [] wFilename;
	return ret;
}

hsBool	plFileUtils::EnsureFilePathExists( const wchar *filename )
{
	hsWStringTokenizer	izer( filename, L"\\/" );

	hsBool	lastWorked = false;
	wchar	token[ kFolderIterator_MaxPath ];


	while( izer.Next( token, arrsize( token ) ) && izer.HasMoreTokens() )
	{
		// Want the full path from the start of the string
		lastWorked = CreateDir( izer.fString );
		izer.RestoreLastTerminator();
	}

	return lastWorked;
}

//// GetFileTimes ////////////////////////////////////////////////////////////
//	Gets the creation and modification dates of the file specified. Returns 
//	false if unsuccessful

hsBool	plFileUtils::GetFileTimes( const char *path, plUnifiedTime *createTimeOut, plUnifiedTime *modifyTimeOut )
{
	struct stat	fileInfo;

	int result = stat( path, &fileInfo );
	if( result != 0 )
		return false;

	if( createTimeOut != nil )
		*createTimeOut = plUnifiedTime( fileInfo.st_ctime );
	if( modifyTimeOut != nil )
		*modifyTimeOut = plUnifiedTime( fileInfo.st_mtime );

	return true;
}

plFileUtils::Modify plFileUtils::CompareModifyTimes(const char* file1, const char* file2)
{
	plUnifiedTime modTime1, modTime2;
	if (GetFileTimes(file1, nil, &modTime1) &&
		GetFileTimes(file2, nil, &modTime2))
	{
		double diff = plUnifiedTime::GetTimeDifference(modTime1, modTime2);

		if (hsABS(diff) <= 2)
			return kFilesEqual;
		else if (diff > 0)
			return kFile1Newer;
		else
			return kFile2Newer;
	}

	return kFileError;
}

bool plFileUtils::SetModifyTime( const char * filename, const plUnifiedTime & timestamp )
{
#ifdef HS_BUILD_FOR_WIN32
	HANDLE hFile = CreateFile(filename,
		GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,
		nil,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nil);
	if (hFile==INVALID_HANDLE_VALUE)
		return false;
	SYSTEMTIME systime;
	systime.wDay = timestamp.GetDay();
	systime.wDayOfWeek = timestamp.GetDayOfWeek();
	systime.wHour = timestamp.GetHour();
	systime.wMilliseconds = 0;
	systime.wMinute = timestamp.GetMinute();
	systime.wMonth = timestamp.GetMonth();
	systime.wSecond = timestamp.GetSecond();
	systime.wYear = timestamp.GetYear();
	FILETIME localFileTime, filetime;
	SystemTimeToFileTime(&systime,&localFileTime);
	LocalFileTimeToFileTime(&localFileTime,&filetime);
	SetFileTime(hFile,nil,nil,&filetime);
	CloseHandle(hFile);
	return true;

#elif HS_BUILD_FOR_UNIX
	struct stat	sbuf;
	int result = stat( filename, &sbuf );
	if( result )
		return false;
	struct utimbuf utb;
	utb.actime = sbuf.st_atime;
	utb.modtime = timestamp.GetSecs();
	result = utime( filename, &utb );
	if( result )
		return false;
	return true;

#endif
}

//// StripPath ///////////////////////////////////////////////////////////////

const char* plFileUtils::GetFileName(const char* path)
{
	const char* c = strrchr(path, '/');
	if (c == nil)
		c = strrchr(path, '\\');

	if (c == nil)
		c = path;
	else
		c++;

	return c;
}

const wchar* plFileUtils::GetFileName(const wchar* path)
{
	const wchar* c = wcsrchr(path, L'/');
	if (c == nil)
		c = wcsrchr(path, L'\\');

	if (c == nil)
		c = path;
	else
		c++;

	return c;
}

void plFileUtils::StripFile(char* pathAndName)
{
	char* fileName = (char*)GetFileName(pathAndName);
	if (fileName != pathAndName)
		*fileName = '\0';
}

void plFileUtils::StripFile(wchar* pathAndName)
{
	wchar* fileName = (wchar*)GetFileName(pathAndName);
	if (fileName != pathAndName)
		*fileName = L'\0';
}

void plFileUtils::StripExt(char* fileName)
{
	char* ext = (char*)GetFileExt(fileName);
	if (ext)
		*(ext-1) = '\0';
}

const char* plFileUtils::GetFileExt(const char* pathAndName)
{
	const char* fileName = GetFileName(pathAndName);
	if (fileName)
	{
		const char* ext = strrchr(fileName, '.');
		if (ext)
			return ext+1;
	}

	return nil;
}

const wchar* plFileUtils::GetFileExt(const wchar* pathAndName)
{
	const wchar* fileName = GetFileName(pathAndName);
	if (fileName)
	{
		const wchar* ext = wcsrchr(fileName, L'.');
		if (ext)
			return ext+1;
	}

	return nil;
}

void plFileUtils::AddSlash(char* path)
{
	char lastChar = path[strlen(path)-1];
	if (lastChar != '\\' && lastChar != '/')
		strcat(path, "\\");
}

void plFileUtils::ConcatFileName(char* path, const char* fileName)
{
	AddSlash(path);
	strcat(path, fileName);
}

//// GetFileSize /////////////////////////////////////////////////////////////

UInt32	plFileUtils::GetFileSize( const char *path )
{
	wchar* wPath = hsStringToWString(path);
	UInt32 ret = GetFileSize(wPath);
	delete [] wPath;
	return ret;
}

UInt32	plFileUtils::GetFileSize( const wchar *path )
{
	UInt32 len = 0;

	hsUNIXStream str;
	if (str.Open(path, L"rb"))
	{
		len = str.GetEOF();
		str.Close();
	}

	return len;
}

//// GetSecureEncryptionKey //////////////////////////////////////////////////

bool plFileUtils::GetSecureEncryptionKey(const char* filename, UInt32* key, unsigned length)
{
	wchar* wFilename = hsStringToWString(filename);
	bool ret = GetSecureEncryptionKey(wFilename, key, length);
	delete [] wFilename;
	return ret;
}

bool plFileUtils::GetSecureEncryptionKey(const wchar* filename, UInt32* key, unsigned length)
{
	// looks for an encryption key file in the same directory, and reads it
	std::wstring sFilename = filename;

	// grab parent directory
	unsigned loc = sFilename.rfind(L"\\");
	if (loc == std::wstring::npos)
		loc = sFilename.rfind(L"/");

	std::wstring sDir;
	if (loc != std::wstring::npos)
		sDir = sFilename.substr(0, loc);
	else // no directory
		sDir = L"./";
	if ((sDir[sDir.length()-1] != L'/') && (sDir[sDir.length()-1] != L'\\'))
		sDir += L'/'; // add the slash, if it doesn't has one

	// now add the key filename
	std::wstring keyFile = sDir + kWKeyFilename;

	if (FileExists(keyFile.c_str()))
	{
		// file exists, read from it
		hsUNIXStream file;
		file.Open(keyFile.c_str(), L"rb");

		unsigned bytesToRead = length * sizeof(UInt32);
		byte* buffer = (byte*)ALLOC(bytesToRead);
		unsigned bytesRead = file.Read(bytesToRead, buffer);

		file.Close();

		unsigned memSize = min(bytesToRead, bytesRead);
		memcpy(key, buffer, memSize);
		FREE(buffer);

		return true;
	}
	else
	{
		// file doesn't exist, use default key
		unsigned memSize = min(length, arrsize(plSecureStream::kDefaultKey));
		memSize *= sizeof(UInt32);
		memcpy(key, plSecureStream::kDefaultKey, memSize);

		return false;
	}
}