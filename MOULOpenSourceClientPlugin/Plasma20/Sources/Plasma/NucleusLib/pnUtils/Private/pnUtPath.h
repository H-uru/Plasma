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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtPath.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTPATH_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtPath.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTPATH_H


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
    qword       fileLength;
    qword       lastWriteTime;
    wchar       name[MAX_PATH];
};


/*****************************************************************************
*
*   Path "get" functions
*
***/

void PathFindFiles (
    ARRAY(PathFind) *   paths,
    const wchar         fileSpec[],
    unsigned            pathFlags
);
void PathGetProgramName (
    wchar *     dst,
    unsigned    dstChars
);
void PathGetModuleName (
    wchar *     dst,
    unsigned    dstChars
);

// this function will use the current directory if <src> is a relative path
// (see PathSetCurrentDirectory/PathGetCurrentDirectory)
bool PathFromString (
    wchar *     dst,
    const wchar src[],
    unsigned    dstChars
);
bool PathFromString (
    wchar *     dst,
    const wchar src[],
    unsigned    dstChars,
    const wchar baseDir[]
);

bool PathDoesFileExist (
    const wchar fileName[]
);
bool PathDoesDirectoryExist (
	const wchar directory[]
);

bool PathDeleteFile (
    const wchar file[]
);
bool PathMoveFile (
    const wchar src[],
    const wchar dst[]
);
bool PathCopyFile (
    const wchar src[],
    const wchar dst[]
);


/*****************************************************************************
*
*   Path building functions
*
***/

void PathSplitPath (
    const wchar     path[],
    wchar *         drive,
    wchar *         dir,
    wchar *         fname,
    wchar *         ext
);

void PathMakePath (
    wchar *         path,
    unsigned        chars,
    const wchar     drive[],
    const wchar     dir[],
    const wchar     fname[],
    const wchar     ext[]
);

// c:\dir1 + dir2\file.txt => c:\dir1\dir2\file.txt
void PathAddFilename (
    wchar *      dst,
    const wchar  src[], 
    const wchar  fname[],
    unsigned     dstChars
);

// c:\dir1\dir2\file.txt => c:\dir1\dir2\    * note trailing backslash
void PathRemoveFilename (
    wchar *      dst,
    const wchar  src[],
    unsigned     dstChars
);

// c:\file.txt => c:\dir1\dir2\file
void PathRemoveExtension (
    wchar *      dst,
    const wchar  src[],
    unsigned     dstChars
);

// c:\file      + .out => c:\file.out
// c:\file.     + .out => c:\file.out
// c:\file.txt  + .out => c:\file.out
void PathSetExtension (
    wchar *      dst,
    const wchar  src[],
    const wchar  ext[],
    unsigned     dstChars
);

// c:\file      + .out => c:\file.out
// c:\file.     + .out => c:\file.
// c:\file.txt  + .out => c:\file.txt
void PathAddExtension (
    wchar *      dst,
    const wchar  src[],
    const wchar  ext[],
    unsigned     dstChars
);

// c:\dir1\dir2\file.txt => file.txt
void PathRemoveDirectory (
    wchar *      dst,
    const wchar  src[],
    unsigned     dstChars
);

// c:\dir1\dir2\file.txt - c:\dir1 => .\dir2\file.txt
// c:\dir1\dir2\file1.txt - c:\dir1\dir4\file2.txt => ..\dir4\file2.txt
bool PathMakeRelative (
    wchar *     dst,
    unsigned    fromFlags,
    const wchar from[],
    unsigned    toFlags,
    const wchar to[],
    unsigned    dstChars
);
bool PathIsRelative (
    const wchar src[]
);

const wchar * PathFindFilename (const wchar path[]);
inline wchar * PathFindFilename (wchar * path) {
    return const_cast<wchar *>(PathFindFilename(const_cast<const wchar *>(path)));
}

const wchar * PathFindExtension (const wchar path[]);
inline wchar * PathFindExtension (wchar * path) {
    return const_cast<wchar *>(PathFindExtension(const_cast<const wchar *>(path)));
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
    const wchar path[],
    unsigned    flags
);
void PathDeleteDirectory (
    const wchar path[],
    unsigned    flags
);


// Set directory
bool PathSetCurrentDirectory (const wchar path[]);
void PathSetProgramDirectory ();


// Get directory
void PathGetProgramDirectory (
    wchar *     dst,
    unsigned    dstChars
);
void PathGetCurrentDirectory (
    wchar *     dst,
    unsigned    dstChars
);
void PathGetTempDirectory (
    wchar *     dst,
    unsigned    dstChars
);


// Product and user-specific common directory locations
void PathGetUserDirectory (
	wchar *		dst,
	unsigned	dstChars
);
void PathGetLogDirectory (
	wchar *		dst,
	unsigned	dstChars
);
void PathGetInitDirectory (
	wchar *		dst,
	unsigned	dstChars
);


/*****************************************************************************
*
*   Email formatting functions
*
***/

// you may send nil for any fields you don't care about
void PathSplitEmail (
	const wchar	emailAddr[],
	wchar *		user,
	unsigned	userChars,
	wchar *		domain,
	unsigned	domainChars,
	wchar *		tld,
	unsigned	tldChars,
	wchar *		subDomains,		// (wchar *)&subs	--> wchar subs[16][256];
	unsigned	subDomainChars,	// arrsize(subs[0])	--> 256
	unsigned	subDomainCount	// arrsize(subs)	--> 16
);
