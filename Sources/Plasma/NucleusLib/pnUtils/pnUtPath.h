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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtPath.h
*   
***/

#ifndef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTPATH_H
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTPATH_H

#include "Pch.h"
#include "pnUtArray.h"

/*****************************************************************************
*
*   Path definitions
*
***/

#ifndef MAX_PATH
#define MAX_PATH    260
#endif

#define MAX_DRIVE   256
#define MAX_DIR     256
#define MAX_FNAME   256
#define MAX_EXT     256



const unsigned kPathFlagFile        = 1<<0;
const unsigned kPathFlagDirectory   = 1<<1;
const unsigned kPathFlagHidden      = 1<<2;
const unsigned kPathFlagSystem      = 1<<3;
const unsigned kPathFlagRecurse     = 1<<4;   // also set if "**" used in filespec

struct PathFind {
    unsigned    flags;
    uint64_t       fileLength;
    uint64_t       lastWriteTime;
    wchar_t       name[MAX_PATH];
};


/*****************************************************************************
*
*   Path "get" functions
*
***/

void PathFindFiles (
    ARRAY(PathFind) *   paths,
    const wchar_t         fileSpec[],
    unsigned            pathFlags
);
void PathGetProgramName (
    wchar_t *     dst,
    unsigned    dstChars
);
void PathGetModuleName (
    wchar_t *     dst,
    unsigned    dstChars
);

// this function will use the current directory if <src> is a relative path
// (see PathSetCurrentDirectory/PathGetCurrentDirectory)
bool PathFromString (
    wchar_t *     dst,
    const wchar_t src[],
    unsigned    dstChars
);
bool PathFromString (
    wchar_t *     dst,
    const wchar_t src[],
    unsigned    dstChars,
    const wchar_t baseDir[]
);

bool PathDoesFileExist (
    const wchar_t fileName[]
);
bool PathDoesDirectoryExist (
    const wchar_t directory[]
);

bool PathDeleteFile (
    const wchar_t file[]
);
bool PathMoveFile (
    const wchar_t src[],
    const wchar_t dst[]
);
bool PathCopyFile (
    const wchar_t src[],
    const wchar_t dst[]
);


/*****************************************************************************
*
*   Path building functions
*
***/

void PathSplitPath (
    const wchar_t     path[],
    wchar_t *         drive,
    wchar_t *         dir,
    wchar_t *         fname,
    wchar_t *         ext
);

void PathMakePath (
    wchar_t *         path,
    unsigned        chars,
    const wchar_t     drive[],
    const wchar_t     dir[],
    const wchar_t     fname[],
    const wchar_t     ext[]
);

// c:\dir1 + dir2\file.txt => c:\dir1\dir2\file.txt
void PathAddFilename (
    wchar_t *      dst,
    const wchar_t  src[], 
    const wchar_t  fname[],
    unsigned     dstChars
);

// c:\dir1\dir2\file.txt => c:\dir1\dir2\    * note trailing backslash
void PathRemoveFilename (
    wchar_t *      dst,
    const wchar_t  src[],
    unsigned     dstChars
);

// c:\file.txt => c:\dir1\dir2\file
void PathRemoveExtension (
    wchar_t *      dst,
    const wchar_t  src[],
    unsigned     dstChars
);

// c:\file      + .out => c:\file.out
// c:\file.     + .out => c:\file.out
// c:\file.txt  + .out => c:\file.out
void PathSetExtension (
    wchar_t *      dst,
    const wchar_t  src[],
    const wchar_t  ext[],
    unsigned     dstChars
);

// c:\file      + .out => c:\file.out
// c:\file.     + .out => c:\file.
// c:\file.txt  + .out => c:\file.txt
void PathAddExtension (
    wchar_t *      dst,
    const wchar_t  src[],
    const wchar_t  ext[],
    unsigned     dstChars
);

// c:\dir1\dir2\file.txt => file.txt
void PathRemoveDirectory (
    wchar_t *      dst,
    const wchar_t  src[],
    unsigned     dstChars
);

// c:\dir1\dir2\file.txt - c:\dir1 => .\dir2\file.txt
// c:\dir1\dir2\file1.txt - c:\dir1\dir4\file2.txt => ..\dir4\file2.txt
bool PathMakeRelative (
    wchar_t *     dst,
    unsigned    fromFlags,
    const wchar_t from[],
    unsigned    toFlags,
    const wchar_t to[],
    unsigned    dstChars
);
bool PathIsRelative (
    const wchar_t src[]
);

const wchar_t * PathFindFilename (const wchar_t path[]);
inline wchar_t * PathFindFilename (wchar_t * path) {
    return const_cast<wchar_t *>(PathFindFilename(const_cast<const wchar_t *>(path)));
}

const wchar_t * PathFindExtension (const wchar_t path[]);
inline wchar_t * PathFindExtension (wchar_t * path) {
    return const_cast<wchar_t *>(PathFindExtension(const_cast<const wchar_t *>(path)));
}


/*****************************************************************************
*
*   Directory functions
*
***/

// Create directory
enum EPathCreateDirError {
    kPathCreateDirSuccess,
    kPathCreateDirErrInvalidPath,
    kPathCreateDirErrAccessDenied,
    kPathCreateDirErrFileWithSameName,
    kPathCreateDirErrDirExists,         // Directory exists and kPathCreateDirFlagCreateNew was specified
};

// Setting this flag causes the function to create the entire directory
// tree from top to bottom. Clearing this flag causes the function to
// create only the last entry in the path.
const unsigned kPathCreateDirFlagEntireTree = 1<<0;

// Setting this flag causes the function to create the last entry in the path
// ONLY if it doesn't already exist. If it does exist the function will return
// kPathCreateDirErrDirExistes.
const unsigned kPathCreateDirFlagCreateNew  = 1<<1;


EPathCreateDirError PathCreateDirectory (
    const wchar_t path[],
    unsigned    flags
);
void PathDeleteDirectory (
    const wchar_t path[],
    unsigned    flags
);


// Set directory
bool PathSetCurrentDirectory (const wchar_t path[]);
void PathSetProgramDirectory ();


// Get directory
void PathGetProgramDirectory (
    wchar_t *     dst,
    unsigned    dstChars
);
void PathGetCurrentDirectory (
    wchar_t *     dst,
    unsigned    dstChars
);
void PathGetTempDirectory (
    wchar_t *     dst,
    unsigned    dstChars
);


// Product and user-specific common directory locations
void PathGetUserDirectory (
    wchar_t *     dst,
    unsigned    dstChars
);
void PathGetLogDirectory (
    wchar_t *     dst,
    unsigned    dstChars
);
void PathGetInitDirectory (
    wchar_t *     dst,
    unsigned    dstChars
);


/*****************************************************************************
*
*   Email formatting functions
*
***/

// you may send nil for any fields you don't care about
void PathSplitEmail (
    const wchar_t emailAddr[],
    wchar_t *     user,
    unsigned    userChars,
    wchar_t *     domain,
    unsigned    domainChars,
    wchar_t *     tld,
    unsigned    tldChars,
    wchar_t *     subDomains,     // (wchar_t *)&subs   --> wchar_t subs[16][256];
    unsigned    subDomainChars, // arrsize(subs[0]) --> 256
    unsigned    subDomainCount  // arrsize(subs)    --> 16
);
#endif
