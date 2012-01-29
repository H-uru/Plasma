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
#ifndef hsFiles_Defined
#define hsFiles_Defined

#include "hsStream.h"
#include <stdio.h>

#if HS_BUILD_FOR_UNIX
    #include <limits.h>
    #define kFolderIterator_MaxPath     PATH_MAX
    #define SetCurrentDirectory chdir
#else
    #define kFolderIterator_MaxPath     _MAX_PATH
#endif


#if HS_BUILD_FOR_WIN32
# define PATH_SEPARATOR     '\\'
# define WPATH_SEPARATOR    L'\\'
# define PATH_SEPARATOR_STR "\\"
# define WPATH_SEPARATOR_STR L"\\"
#elif HS_BUILD_FOR_UNIX
# define PATH_SEPARATOR     '/'
# define WPATH_SEPARATOR    L'/'
# define PATH_SEPARATOR_STR "/"
# define WPATH_SEPARATOR_STR L"/"
#endif


///////////////////////////////////////////////////////////////////////

class hsFile {
    hsFile&     operator=(const hsFile&);       // disallow assignment
protected:
    char*       fPathAndName;
    FILE*       fFILE;
public:
                hsFile();
                hsFile(const char pathAndName[]);
    virtual     ~hsFile();

    const char* GetName();
    virtual const char* GetPathAndName();
    virtual void    SetPathAndName(const char pathAndName[]);

    virtual FILE*   OpenFILE(const char mode[], hsBool throwIfFailure = false);
    virtual hsStream* OpenStream(const char mode[], hsBool throwIfFailure = false);

    virtual void    Close();    // called automatically in the destructor
};
typedef hsFile  hsUnixFile; // for compatibility
typedef hsFile  hsOSFile;

///////////////////////////////////////////////////////////////////////

class hsFolderIterator {
    char        fPath[kFolderIterator_MaxPath];
    struct hsFolderIterator_Data* fData;
   bool fCustomFilter;
public:
#ifdef HS_BUILD_FOR_WIN32
   hsFolderIterator(const char path[] = nil, bool useCustomFilter=false);
#else
   hsFolderIterator(const char path[] = nil, bool unused=true);
   hsFolderIterator(const struct FSSpec* spec); // Alt constructor
#endif
    virtual     ~hsFolderIterator();

    const char* GetPath() const { return fPath; }
    void            SetPath(const char path[]);

    void            Reset();
    hsBool      NextFile();
    hsBool      NextFileSuffix(const char suffix[]);
    const char* GetFileName() const;
    int         GetPathAndName(char pathandname[] = nil);
    hsBool      IsDirectory( void ) const;

    FILE*       OpenFILE(const char mode[]);

#if HS_BUILD_FOR_WIN32
    void        SetWinSystemDir(const char subdir[]);   // e.g. "Fonts"
    void        SetFileFilterStr(const char filterStr[]);   // e.g. "*.*"
#endif
};

#ifdef HS_BUILD_FOR_WIN32
// only implemented on Win32 for now
class hsWFolderIterator {
    wchar_t       fPath[kFolderIterator_MaxPath];
    struct      hsWFolderIterator_Data* fData;
    bool        fCustomFilter;
public:
    hsWFolderIterator(const wchar_t path[] = nil, bool useCustomFilter=false);
    virtual     ~hsWFolderIterator();

    const wchar_t*    GetPath() const { return fPath; }
    void            SetPath(const wchar_t path[]);

    void            Reset();
    hsBool          NextFile();
    hsBool          NextFileSuffix(const wchar_t suffix[]);
    const wchar_t*    GetFileName() const;
    int             GetPathAndName(wchar_t pathandname[] = nil);
    hsBool          IsDirectory( void ) const;

    FILE*           OpenFILE(const wchar_t mode[]);

    void        SetWinSystemDir(const wchar_t subdir[]);  // e.g. "Fonts"
    void        SetFileFilterStr(const wchar_t filterStr[]);  // e.g. "*.*"
};
#endif

#endif
