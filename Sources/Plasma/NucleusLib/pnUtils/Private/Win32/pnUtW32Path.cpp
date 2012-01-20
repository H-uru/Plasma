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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/Win32/pnUtW32Path.cpp
*   
***/

#include "../../Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Local functions
*
***/

// make sure our definition is at least as big as the compiler's definition
COMPILER_ASSERT(MAX_PATH >= _MAX_PATH);


//===========================================================================
static inline bool IsSlash (wchar_t c) {
    return (c == L'\\') || (c == L'/');
}

//===========================================================================
static inline wchar_t ConvertSlash (wchar_t c) {
    return c != L'/' ? c : L'\\';
}

//===========================================================================
static inline bool IsUncPath (const wchar_t path[]) {
    return IsSlash(path[0]) && IsSlash(path[1]);
}

//===========================================================================
static const wchar_t * SkipUncDrive (const wchar_t path[]) {
    // UNC drive: "//server/share"

    // skip over leading "//"
    path += 2;

    // scan forward to end of server name
    for (;; ++path) {
        if (!*path)
            return path;
        if (IsSlash(*path))
            break;
    }

    // skip over '/'
    ++path;

    // skip over share name
    for (;; ++path) {
        if (!*path)
            return path;
        if (IsSlash(*path))
            return path;
    }
}

//===========================================================================
static wchar_t * PathSkipOverSeparator (wchar_t * path) {
    for (; *path; ++path) {
        if (IsSlash(*path))
            return path + 1;
    }

    return path;
}

//===========================================================================
static unsigned CommonPrefixLength (
    const wchar_t src1[],
    const wchar_t src2[]
) {
    ASSERT(src1);
    ASSERT(src2);

    wchar_t const * const base    = src1;
    const wchar_t * common        = nil;
    for (;;) {
        // Are the next components equal in length?
        const wchar_t * next1 = PathSkipOverSeparator(const_cast<wchar_t *>(src1));
        const wchar_t * next2 = PathSkipOverSeparator(const_cast<wchar_t *>(src2));
        const int componentLen = next1 - src1;
        if (componentLen != (next2 - src2))
            break;

        // Are the next components equal in value?
        if (!StrCmpI(src1, src2, componentLen))
            common = next1;
        else
            break;

        if (!*next1)
            break;
        src1 = next1 + 1;

        if (!*next2)
            break;
        src2 = next2 + 1;
    }

    if (!common)
        return 0;

    // Compute length of common subchunk;
    // if it is "C:" convert it to "C:\"
    unsigned commonLen = common - base;
    if ((commonLen == 2) && (base[1] == L':'))
        ++commonLen;
    return commonLen;
}

//===========================================================================
static void GetProgramName (
    void *      instance,
    wchar_t *     dst,
    unsigned    dstChars
) {
    ASSERT(dst);
    ASSERT(dstChars);

    if (!GetModuleFileNameW((HINSTANCE) instance, dst, dstChars)) {
        ErrorAssert(__LINE__, __FILE__, "GetModuleName failed");
        *dst = 0;
    }
}


/****************************************************************************
*
*   Exports
*
***/

//===========================================================================
void PathGetModuleName (
    wchar_t *     dst,
    unsigned    dstChars
) {
    GetProgramName(ModuleGetInstance(), dst, dstChars);
}

//===========================================================================
void PathGetProgramName (
    wchar_t *      dst,
    unsigned     dstChars
) {
    GetProgramName(nil, dst, dstChars);
}

//===========================================================================
bool PathFromString (
    wchar_t *      dst, 
    const wchar_t  src[], 
    unsigned     dstChars
) {
    ASSERT(dst);
    ASSERT(src);
    ASSERT(dstChars);

    for (;;) {
        // enable src and dst to be the same buffer
        wchar_t temp[MAX_PATH];
        if (dst == src) {
            StrCopy(temp, src, arrsize(temp));
            src = temp;
        }

        DWORD const result = GetFullPathNameW(src, dstChars, dst, 0);
        if (!result)
            break;
        if (dstChars < result)
            break;
        if (!dst[0])
            break;

        return true;
    }

    *dst = 0;
    return false;
}

//===========================================================================
bool PathFromString (
    wchar_t *     dst,                // ASSERT(dst);
    const wchar_t src[],              // ASSERT(src);
    unsigned    dstChars,           // ASSERT(dstChars);
    const wchar_t baseDir[]           // ASSERT(baseDir);
) {
    ASSERT(baseDir);
    ASSERT(dstChars);

    // Save current directory
    wchar_t curr[MAX_PATH];
    PathGetCurrentDirectory(curr, arrsize(curr));

    // Perform string conversion from specified directory
    bool result;
    if (0 != (result = PathSetCurrentDirectory(baseDir)))
        result = PathFromString(dst, src, dstChars);
    else
        *dst = 0;

    // Restore directory
    PathSetCurrentDirectory(curr);
    return result;
}

//===========================================================================
// this function was originally derived from _tsplitpath in the MSVCRT library,
// but has been updated to support UNC paths and to avoid blasting off the end
// of the buffers.
void PathSplitPath (
    const wchar_t     path[],
    wchar_t *         drive,
    wchar_t *         dir,
    wchar_t *         fname,
    wchar_t *         ext
) {
    ASSERT(path);
    ASSERT(path != drive);
    ASSERT(path != dir);
    ASSERT(path != fname);
    ASSERT(path != ext);

    // check for UNC path
    if (IsUncPath(path)) {
        const wchar_t * pathStart = path;
        path = SkipUncDrive(path);

        if (drive)
            StrCopy(drive, pathStart, min(MAX_DRIVE, path - pathStart + 1));
    }
    // regular DOS path
    else if (path[0] && (path[1] == L':')) {
        if (drive) {
            ASSERT(MAX_DRIVE >= 3);
            drive[0] = path[0];
            drive[1] = L':';
            drive[2] = L'\0';
        }

        path += 2; // skip over 'C' ':'
    }
    else if (drive) {
        *drive = 0;
    }

    // extract path string, if any.  Path now points to the first character
    // of the path, if any, or the filename or extension, if no path was
    // specified.  Scan ahead for the last occurence, if any, of a '/' or
    // '\' path separator character.  If none is found, there is no path.
    // We will also note the last '.' character found, if any, to aid in
    // handling the extension.
    const wchar_t *last_slash = nil, *last_dot = nil, *p = path;
    for (; *p; p++) {
        if (IsSlash(*p))
            last_slash = p + 1; // point to one beyond for later copy
        else if (*p == L'.')
            last_dot = p;
    }

    if (last_slash) {
        if (dir)
            StrCopy(dir, path, min(MAX_DIR, last_slash - path + 1));
        path = last_slash;
    }
    else if (dir) {
        *dir = 0;
    }

    // extract file name and extension, if any.  Path now points to the
    // first character of the file name, if any, or the extension if no
    // file name was given.  Dot points to the '.' beginning the extension,
    // if any.
    if (last_dot && (last_dot >= path)) {
        if (fname)
            StrCopy(fname, path, min(MAX_FNAME, last_dot - path + 1));
        if (ext)
            StrCopy(ext, last_dot, MAX_EXT);
    }
    else {
        if (fname)
            StrCopy(fname, path, MAX_FNAME);
        if (ext)
            *ext = 0;
    }
}

//===========================================================================
void PathMakePath (
    wchar_t *         path,
    unsigned        chars,
    const wchar_t     drive[],
    const wchar_t     dir[],
    const wchar_t     fname[],
    const wchar_t     ext[]
) {
    ASSERT(path);
    ASSERT(path != drive);
    ASSERT(path != dir);
    ASSERT(path != fname);
    ASSERT(path != ext);

    // save space for string terminator
    if (!chars--)
        return;

    // copy drive
    if (drive && *drive && chars) {
        do {
            *path++ = ConvertSlash(*drive++);
        } while (--chars && *drive);
        ASSERT(!IsSlash(path[-1]));
    }

    // copy directory
    if (dir && *dir && chars) {
        do {
            *path++ = ConvertSlash(*dir++);
        } while (--chars && *dir);

        // add trailing backslash
        if (chars && (path[-1] != '\\')) {
            *path++ = L'\\';
            chars--;
        }
    }

    // copy filename
    if (fname && *fname && chars) {
        // skip leading backslash
        if (IsSlash(*fname))
            ++fname;

        do {
            *path++ = ConvertSlash(*fname++);
        } while (--chars && *fname);
    }

    // copy extension
    if (ext && *ext && chars) {
        if (*ext != L'.') {
            *path++ = L'.';
            chars--;
        }
        while (chars-- && *ext)
            *path++ = ConvertSlash(*ext++);
    }

    // add string terminator
    *path = L'\0';
}

//===========================================================================
bool PathMakeRelative (
    wchar_t       *dst,
    unsigned    fromFlags,  // 0 or kPathFlagDirectory
    const wchar_t from[],
    unsigned    toFlags,    // 0 or kPathFlagDirectory
    const wchar_t to[],
    unsigned    dstChars
) {
    ASSERT(dst);
    ASSERT(from);
    ASSERT(to);
    ASSERT(dstChars);
    *dst = 0;

    unsigned prefixLength = CommonPrefixLength(from, to);
    if (!prefixLength)
        return false;

    wchar_t fromBuf[MAX_PATH];
    if (fromFlags & kPathFlagDirectory)
        StrCopy(fromBuf, from, arrsize(fromBuf));
    else
        PathRemoveFilename(fromBuf, from, arrsize(fromBuf));

    wchar_t toBuf[MAX_PATH];
    if (toFlags & kPathFlagDirectory)
        StrCopy(toBuf, to, arrsize(toBuf));
    else
        PathRemoveFilename(toBuf, to, arrsize(toBuf));

    const wchar_t * curr = fromBuf + prefixLength;
    if (*curr) {
        // build ..\.. part of the path
        if (IsSlash(*curr))
            curr++;              // skip slash

        while (*curr) {
            curr = PathSkipOverSeparator(const_cast<wchar_t *>(curr));
            StrPack(dst, *curr ? L"..\\" : L"..", dstChars);
        }
    }
    else {
        StrCopy(dst, L".", dstChars);
    }

    if (to[prefixLength]) {
        // deal with root case
        if (!IsSlash(to[prefixLength]))
            --prefixLength;

        ASSERT(IsSlash(to[prefixLength]));
        StrPack(dst, to + prefixLength, dstChars);
    }

    return true;
}

//===========================================================================
bool PathIsRelative (
    const wchar_t src[]
) {
    ASSERT(src);
    if (!src[0])
        return true;
    if (IsSlash(src[0]))
        return false;
    if (src[1] == L':')
        return false;
    return true;
}

//===========================================================================
const wchar_t * PathFindFilename (
    const wchar_t path[]
) {
    ASSERT(path);

    if (IsUncPath(path))
        path = SkipUncDrive(path);

    const wchar_t * last_slash = path;
    for (const wchar_t * p = path; *p; p++) {
        if ((*p == L'/') || (*p == L'\\') || (*p == L':'))
            last_slash = p + 1;
    }

    return last_slash;
}

//===========================================================================
const wchar_t * PathFindExtension (
    const wchar_t path[]
) {
    ASSERT(path);

    const wchar_t * last_dot = 0;
    const wchar_t * p = PathFindFilename(path);
    for ( ; *p; p++) {
        if (*p == L'.')
            last_dot = p;
    }

    return last_dot ? last_dot : p;
}

//===========================================================================
void PathGetCurrentDirectory (
    wchar_t *     dst,
    unsigned    dstChars
) {
    ASSERT(dst);
    ASSERT(dstChars);

    DWORD result = GetCurrentDirectoryW(dstChars, dst);
    if (!result || (result >= dstChars)) {
        ErrorAssert(__LINE__, __FILE__, "GetDir failed");
        *dst = 0;
    }
}

//===========================================================================
void PathGetTempDirectory (
    wchar_t *     dst,
    unsigned    dstChars
) {
    ASSERT(dst);
    ASSERT(dstChars);

    DWORD result = GetTempPathW(dstChars, dst);
    if (!result || (result >= dstChars))
        StrCopy(dst, L"C:\\temp\\", dstChars);
}

//============================================================================
void PathGetUserDirectory (
    wchar_t *     dst,
    unsigned    dstChars
) {
    ASSERT(dst);
    ASSERT(dstChars);

    wchar_t temp[MAX_PATH]; // GetSpecialFolder path requires a buffer of MAX_PATH size or larger
    if (SHGetSpecialFolderPathW(NULL, temp, CSIDL_LOCAL_APPDATA, TRUE) == FALSE)
        StrCopy(temp, L"C:\\", arrsize(temp));

    // append the product name
    PathAddFilename(dst, temp, ProductLongName(), dstChars);

    // ensure it exists
    if (!PathDoesDirectoryExist(dst))
        PathCreateDirectory(dst, kPathCreateDirFlagEntireTree);
}

//============================================================================
void PathGetLogDirectory (
    wchar_t *     dst,
    unsigned    dstChars
) {
    ASSERT(dst);
    ASSERT(dstChars);
    PathGetUserDirectory(dst, dstChars);
    PathAddFilename(dst, dst, L"Log", dstChars);
    if (!PathDoesDirectoryExist(dst))
        PathCreateDirectory(dst, kPathCreateDirFlagEntireTree);
}

//============================================================================
void PathGetInitDirectory (
    wchar_t *     dst,
    unsigned    dstChars
) {
    ASSERT(dst);
    ASSERT(dstChars);
    PathGetUserDirectory(dst, dstChars);
    PathAddFilename(dst, dst, L"Init", dstChars);
    if (!PathDoesDirectoryExist(dst))
        PathCreateDirectory(dst, kPathCreateDirFlagEntireTree);
}

//===========================================================================
bool PathSetCurrentDirectory (
    const wchar_t path[]
) {
    ASSERT(path);
    return SetCurrentDirectoryW(path) != 0;
}

//===========================================================================
void PathSetProgramDirectory () {
    wchar_t dir[MAX_PATH];
    PathGetProgramDirectory(dir, arrsize(dir));
    PathSetCurrentDirectory(dir);
}

//===========================================================================
void PathFindFiles (
    ARRAY(PathFind) *   paths,
    const wchar_t         fileSpec[],
    unsigned            pathFlags
) {
    ASSERT(paths);
    ASSERT(fileSpec);

    HANDLE find;
    WIN32_FIND_DATAW fd;
    wchar_t directory[MAX_PATH];
    PathRemoveFilename(directory, fileSpec, arrsize(directory));
    if (INVALID_HANDLE_VALUE == (find = FindFirstFileW(fileSpec, &fd))) {
        DWORD err = GetLastError();
        if ((err != ERROR_FILE_NOT_FOUND) && (err != ERROR_PATH_NOT_FOUND))
            ASSERTMSG(err, "PathFindFiles failed");
    }
    else {
        // find all the items in the current directory
        do {
            unsigned fileFlags = 0;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (! (pathFlags & kPathFlagDirectory))
                    continue;

                // don't add "." and ".."
                if (fd.cFileName[0] == L'.') {
                    if (!fd.cFileName[1])
                        continue;
                    if (fd.cFileName[1] == L'.' && !fd.cFileName[2])
                        continue;
                }

                fileFlags = kPathFlagDirectory;
            }
            else {
                if (! (pathFlags & kPathFlagFile))
                    continue;
                fileFlags = kPathFlagFile;
            }
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
                if (! (pathFlags & kPathFlagHidden))
                    continue;
                fileFlags |= kPathFlagHidden;
            }
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) {
                if (! (pathFlags & kPathFlagSystem))
                    continue;
                fileFlags |= kPathFlagSystem;
            }

            // add this one to the list of found files
            PathFind * pf       = paths->New();
            pf->flags           = fileFlags;
            pf->fileLength      = ((uint64_t) fd.nFileSizeHigh << 32) | fd.nFileSizeLow;
            pf->lastWriteTime   = * (const uint64_t *) &fd.ftLastWriteTime;
            PathAddFilename(pf->name, directory, fd.cFileName, arrsize(pf->name));
        } while (FindNextFileW(find, &fd));
        FindClose(find);
    }

    // check for directory recursing
    if ((pathFlags & kPathFlagRecurse) || StrStr(fileSpec, L"**")) {
        // recurse directories
    }
    else {
        return;
    }

    wchar_t dirSpec[MAX_PATH];
    PathAddFilename(dirSpec, directory, L"*", arrsize(dirSpec));
    if (INVALID_HANDLE_VALUE == (find = FindFirstFileW(dirSpec, &fd))) {
        DWORD err = GetLastError();
        if ((err != ERROR_FILE_NOT_FOUND) && (err != ERROR_PATH_NOT_FOUND))
            ErrorAssert(__LINE__, __FILE__, "PathFindFiles failed");
        return;
    }

    // find all the directories in the current directory
    const wchar_t * spec = PathFindFilename(fileSpec);
    do {
        if (! (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            continue;
        }
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
            if (! (pathFlags & kPathFlagHidden))
                continue;
        }
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) {
            if (! (pathFlags & kPathFlagSystem))
                continue;
        }

        // don't recurse "." and ".."
        if (fd.cFileName[0] == L'.') {
            if (!fd.cFileName[1])
                continue;
            if (fd.cFileName[1] == L'.' && !fd.cFileName[2])
                continue;
        }

        // recursively search subdirectory
        PathAddFilename(dirSpec, directory, fd.cFileName, arrsize(dirSpec));
        PathAddFilename(dirSpec, dirSpec, spec, arrsize(dirSpec));
        PathFindFiles(paths, dirSpec, pathFlags);

    } while (FindNextFileW(find, &fd));
    FindClose(find);
}

//===========================================================================
EPathCreateDirError PathCreateDirectory (const wchar_t path[], unsigned flags) {
    ASSERT(path);

    // convert from relative path to full path
    wchar_t dir[MAX_PATH];
    if (!PathFromString(dir, path, arrsize(dir))) {
        return kPathCreateDirErrInvalidPath;
    }

    // are we going to build the entire directory tree?
    wchar_t * dirEnd;
    if (flags & kPathCreateDirFlagEntireTree) {
        dirEnd = dir;

        // skip over leading slashes in UNC paths
        while (IsSlash(*dirEnd))
            ++dirEnd;

        // skip forward to first directory
        dirEnd = PathSkipOverSeparator(dirEnd);
    }
    // we're only creating the very last entry in the path
    else {
        dirEnd = dir + StrLen(dir);
    }

    bool result = true;
    for (wchar_t saveChar = L' '; saveChar; *dirEnd++ = saveChar) {
        // find the end of the current directory string and terminate it
        dirEnd = PathSkipOverSeparator(dirEnd);
        saveChar = *dirEnd;
        *dirEnd = 0;

        // create the directory and track the result from the last call
        result = CreateDirectoryW(dir, (LPSECURITY_ATTRIBUTES) nil);
    }

    // if we successfully created the directory then we're done
    if (result) {
        // Avoid check for kPathCreateDirFlagOsError
        COMPILER_ASSERT(kPathCreateDirSuccess == NO_ERROR);
        return kPathCreateDirSuccess;
    }

    unsigned error = GetLastError();
    switch (error) {
        case ERROR_ACCESS_DENIED:
        return kPathCreateDirErrAccessDenied;

        case ERROR_ALREADY_EXISTS: {
            DWORD attrib;
            if (0xffffffff == (attrib = GetFileAttributesW(dir)))
                return kPathCreateDirErrInvalidPath;

            if (! (attrib & FILE_ATTRIBUTE_DIRECTORY))
                return kPathCreateDirErrFileWithSameName;

            if (flags & kPathCreateDirFlagCreateNew)
                return kPathCreateDirErrDirExists;
        }
        return kPathCreateDirSuccess;

        default:
        return kPathCreateDirErrInvalidPath;
    }
}

//===========================================================================
void PathDeleteDirectory (const wchar_t path[], unsigned flags) {
    ASSERT(path);

    // convert from relative path to full path
    wchar_t dir[MAX_PATH];
    if (!PathFromString(dir, path, arrsize(dir)))
        return;

    for (;;) {
        // Important: in order to ensure that we don't delete NTFS
        // partition links, we must ensure that this is a directory!
        uint32_t attributes = GetFileAttributesW(dir);
        if (attributes == (uint32_t) -1)
            break;
        if ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            break;
        if (attributes & FILE_ATTRIBUTE_REPARSE_POINT)
            break;

        if (!RemoveDirectoryW(dir))
            break;

        if ((flags & kPathCreateDirFlagEntireTree) == 0)
            break;

        wchar_t * filename = PathFindFilename(dir);
        if (!filename)
            break;

        // Move up one level in the directory hierarchy
        unsigned oldLength = StrLen(dir);
        while ((filename > dir) && IsSlash(filename[-1]))
            --filename;
        *filename = 0;
        if (oldLength == StrLen(dir))
            break;
    }
}

//===========================================================================
bool PathDoesFileExist (const wchar_t fileName[]) {
    uint32_t attributes = GetFileAttributesW(fileName);
    if (attributes == (uint32_t) -1)
        return false;
    if (attributes & FILE_ATTRIBUTE_DIRECTORY)
        return false;
    return true;
}

//============================================================================
bool PathDoesDirectoryExist (const wchar_t directory[]) {
    uint32_t attributes = GetFileAttributesW(directory);
    if (attributes == (uint32_t) -1)
        return false;
    if (attributes & FILE_ATTRIBUTE_DIRECTORY)
        return true;
    return false;
}

//===========================================================================
bool PathDeleteFile (
    const wchar_t file[]
) {
    return DeleteFileW(file) != 0;
}

//===========================================================================
bool PathMoveFile (
    const wchar_t src[],
    const wchar_t dst[]
) {
    return MoveFileW(src, dst) != 0;
}

//===========================================================================
bool PathCopyFile (
    const wchar_t src[],
    const wchar_t dst[]
) {
    return CopyFileW(src, dst, FALSE) != 0;    
}
