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
//
//  plFileUtils - Namespace of fun file utilities
//
//// History /////////////////////////////////////////////////////////////////
//
//  5.7.2002 mcn    - Created
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plFileUtils_h
#define _plFileUtils_h

class plUnifiedTime;

namespace plFileUtils
{
    // Creates the directory specified. Returns false if unsuccessful or directory already exists
    bool    CreateDir( const char *path ); 
    bool    CreateDir( const wchar_t *path ); 
    bool RemoveDir(const char* path);
    bool RemoveDirTree(const char * path);

    // delete file from disk
    bool RemoveFile(const char* filename, bool delReadOnly=false);
    bool RemoveFile(const wchar_t* filename, bool delReadOnly=false);

    bool FileCopy(const char* existingFile, const char* newFile);
    bool FileCopy(const wchar_t* existingFile, const wchar_t* newFile);
    bool FileMove(const char* existingFile, const char* newFile);
    bool FileMove(const wchar_t* existingFile, const wchar_t* newFile);

    bool FileExists(const char* file);
    bool FileExists(const wchar_t* file);

    // Given a filename with path, makes sure the file's path exists
    bool    EnsureFilePathExists( const char *filename );
    bool    EnsureFilePathExists( const wchar_t *filename );
    
    // Gets the creation and modification dates of the file specified. Returns false if unsuccessful
    bool    GetFileTimes( const char *path, plUnifiedTime *createTimeOut, plUnifiedTime *modifyTimeOut );
    // Compares file times, taking into account NTFS/FAT32 time issues
    enum Modify { kFileError, kFilesEqual, kFile1Newer, kFile2Newer };
    Modify CompareModifyTimes(const char* file1, const char* file2);
    // Set file modify time
    bool    SetModifyTime( const char * filename, const plUnifiedTime & time );

    // Return a pointer into the given string at the start of the actual filename (i.e. past any path info)
    const char* GetFileName(const char* pathAndName);
    const wchar_t* GetFileName(const wchar_t* pathAndName);
    // Get the file extension (without the .), or nil if it doesn't have one
    const char* GetFileExt(const char* pathAndName);
    const wchar_t* GetFileExt(const wchar_t* pathAndName);

    // Strips the filename off the given full path
    void StripFile(char* pathAndName);
    void StripFile(wchar_t* pathAndName);
    void StripExt(char* fileName);
    void StripExt(wchar_t* fileName);

    // Get the size of the given file in bytes
    uint32_t      GetFileSize( const char *path );
    uint32_t      GetFileSize( const wchar_t *path );

    // Adds a slash to the end of a filename (or does nothing if it's already there)
    void AddSlash(char* path);
    void AddSlash(wchar_t* path);

    // Concatenates fileName onto path, making sure to add a slash if necessary
    void ConcatFileName(char* path, const char* fileName);
    void ConcatFileName(wchar_t* path, const wchar_t* fileName);
};


#endif // _plFileUtils_h
