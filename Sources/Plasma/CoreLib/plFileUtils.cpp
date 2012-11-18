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
/////////////////////////////////////////////////////////////////////////////
//
//  plFileUtils - Namespace of fun file utilities
//
//// History /////////////////////////////////////////////////////////////////
//
//  5.7.2002 mcn    - Created
//  4.8.2003 chip   - added FileCopy and FileMove for Unix
//
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "hsStlUtils.h"
#include "plFileUtils.h"
#include "hsFiles.h"
#include "hsStringTokenizer.h"


#include "plUnifiedTime.h"

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>


#if HS_BUILD_FOR_WIN32
#include <direct.h>
#include <io.h>
#define mkdir _mkdir
#define chdir _chdir
#define chmod _chmod
#define rmdir _rmdir
#define unlink _unlink
#endif

#if HS_BUILD_FOR_UNIX
#include <unistd.h>
#include <utime.h>
#endif


//// CreateDir ///////////////////////////////////////////////////////////////
//  Creates the directory specified. Returns false if unsuccessful or 
//  directory already exists

bool    plFileUtils::CreateDir( const char *path )
{
    // Create our directory
#if HS_BUILD_FOR_WIN32
    return ( mkdir( path ) == 0 ) ? true : ( errno==EEXIST );
#elif HS_BUILD_FOR_UNIX
    return ( mkdir( path, 0777 ) == 0 ) ? true : ( errno==EEXIST );
#endif
}

bool    plFileUtils::CreateDir( const wchar_t *path )
{
    // Create our directory
#if HS_BUILD_FOR_WIN32
    return ( _wmkdir( path ) == 0 ) ? true : ( errno==EEXIST );
#elif HS_BUILD_FOR_UNIX
    const char* cpath = hsWStringToString(path);
    bool ret = CreateDir(cpath);
    delete[] cpath; /* Free the string */

    return ret;
#endif
}

bool plFileUtils::RemoveDir(const char* path)
{
    return (rmdir(path) == 0);
}

bool plFileUtils::RemoveDirTree(const char * path)
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

bool plFileUtils::RemoveFile(const wchar_t* filename, bool delReadOnly)
{
#ifdef HS_BUILD_FOR_WIN32
    if (delReadOnly)
        _wchmod(filename, S_IWRITE);
    return (_wunlink(filename) == 0);
#elif HS_BUILD_FOR_UNIX
    const char* cfilename = hsWStringToString(filename);
    bool ret = RemoveFile(cfilename, delReadOnly);
    delete[] cfilename; /* Free the string */

    return ret;
#endif
}

bool plFileUtils::FileCopy(const char* existingFile, const char* newFile)
{
    wchar_t* wExisting = hsStringToWString(existingFile);
    wchar_t* wNew = hsStringToWString(newFile);
    bool ret = FileCopy(wExisting, wNew);
    delete [] wExisting;
    delete [] wNew;
    return ret;
}

bool plFileUtils::FileCopy(const wchar_t* existingFile, const wchar_t* newFile)
{
#if HS_BUILD_FOR_WIN32
    return (::CopyFileW(existingFile, newFile, FALSE) != 0);
#elif HS_BUILD_FOR_UNIX
    char data[1500];
    const char* cexisting = hsWStringToString(existingFile);
    const char* cnew = hsWStringToString(newFile);
    FILE* fp = fopen(cexisting, "rb");
    FILE* fw = fopen(cnew, "w");
    delete[] cexisting;
    delete[] cnew;
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

bool plFileUtils::FileMove(const wchar_t* existingFile, const wchar_t* newFile)
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

bool plFileUtils::FileExists(const wchar_t* file)
{
    FILE* fp = hsWFopen(file, L"rb");
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
//  Given a filename with path, makes sure the file's path exists

bool    plFileUtils::EnsureFilePathExists( const char *filename )
{
    wchar_t* wFilename = hsStringToWString(filename);
    bool ret = EnsureFilePathExists(wFilename);
    delete [] wFilename;
    return ret;
}

bool    plFileUtils::EnsureFilePathExists( const wchar_t *filename )
{
    hsWStringTokenizer  izer( filename, L"\\/" );

    bool    lastWorked = false;
    wchar_t   token[ kFolderIterator_MaxPath ];


    while( izer.Next( token, arrsize( token ) ) && izer.HasMoreTokens() )
    {
        // Want the full path from the start of the string
        lastWorked = CreateDir( izer.fString );
        izer.RestoreLastTerminator();
    }

    return lastWorked;
}

//// GetFileTimes ////////////////////////////////////////////////////////////
//  Gets the creation and modification dates of the file specified. Returns 
//  false if unsuccessful

bool    plFileUtils::GetFileTimes( const char *path, plUnifiedTime *createTimeOut, plUnifiedTime *modifyTimeOut )
{
    struct stat fileInfo;

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
    struct stat sbuf;
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

const wchar_t* plFileUtils::GetFileName(const wchar_t* path)
{
    const wchar_t* c = wcsrchr(path, L'/');
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

void plFileUtils::StripFile(wchar_t* pathAndName)
{
    wchar_t* fileName = (wchar_t*)GetFileName(pathAndName);
    if (fileName != pathAndName)
        *fileName = L'\0';
}

void plFileUtils::StripExt(char* fileName)
{
    char* ext = (char*)GetFileExt(fileName);
    if (ext)
        *(ext-1) = '\0';
}

void plFileUtils::StripExt(wchar_t* fileName)
{
    wchar_t* ext = (wchar_t*)GetFileExt(fileName);
    if (ext)
        *(ext-1) = L'\0';
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

const wchar_t* plFileUtils::GetFileExt(const wchar_t* pathAndName)
{
    const wchar_t* fileName = GetFileName(pathAndName);
    if (fileName)
    {
        const wchar_t* ext = wcsrchr(fileName, L'.');
        if (ext)
            return ext+1;
    }

    return nil;
}

void plFileUtils::AddSlash(char* path)
{
    char lastChar = path[strlen(path)-1];
    if (lastChar != '\\' && lastChar != '/')
#if HS_BUILD_FOR_WIN32
        strcat(path, "\\");
#else
        strcat(path, "/");
#endif
}

void plFileUtils::AddSlash(wchar_t* path)
{
    wchar_t lastChar = path[wcslen(path)-1];
    if (lastChar != L'\\' && lastChar != L'/')
#if HS_BUILD_FOR_WIN32
        wcscat(path, L"\\");
#else
        wcscat(path, L"/");
#endif
}

void plFileUtils::ConcatFileName(char* path, const char* fileName)
{
    AddSlash(path);
    strcat(path, fileName);
}

void plFileUtils::ConcatFileName(wchar_t* path, const wchar_t* fileName)
{
    AddSlash(path);
    wcscat(path, fileName);
}

//// GetFileSize /////////////////////////////////////////////////////////////

uint32_t  plFileUtils::GetFileSize( const char *path )
{
    wchar_t* wPath = hsStringToWString(path);
    uint32_t ret = GetFileSize(wPath);
    delete [] wPath;
    return ret;
}

uint32_t  plFileUtils::GetFileSize( const wchar_t *path )
{
    uint32_t len = 0;

    hsUNIXStream str;
    if (str.Open(path, L"rb"))
    {
        len = str.GetEOF();
        str.Close();
    }

    return len;
}
