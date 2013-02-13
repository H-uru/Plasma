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

#include "HeadSpin.h"

#if HS_BUILD_FOR_WIN32
#   include "hsWindows.h"
#   include <shlobj.h>
#else
#   include <limits.h>
#   include <unistd.h>
#   include <sys/types.h>
#   include <dirent.h>
#   include <fnmatch.h>
#   include <cstdlib>
#   include <functional>
#   include <memory>
#endif
#include <sys/stat.h>
#pragma hdrstop

#include "plFileSystem.h"
#include "plProduct.h"

/* NOTE For this file:  Windows uses UTF-16 filenames, and does not support
 * the use of UTF-8 in their ANSI API.  In order to ensure proper unicode
 * support, we convert the UTF-8 format stored in plString to UTF-16 before
 * passing them along to Windows.
 */

plString plFileName::GetFileName() const
{
    int end = fName.FindLast('/');
    if (end < 0)
        end = fName.FindLast('\\');
    if (end < 0)
        return fName;

    return fName.Substr(end + 1);
}

plString plFileName::GetFileExt() const
{
    int dot = fName.FindLast('.');

    // Be sure not to get a dot in the directory!
    int end = fName.FindLast('/');
    if (end < 0)
        end = fName.FindLast('\\');

    if (dot > end)
        return fName.Substr(dot + 1);

    return plString::Null;
}

plString plFileName::GetFileNameNoExt() const
{
    int dot = fName.FindLast('.');

    int end = fName.FindLast('/');
    if (end < 0)
        end = fName.FindLast('\\');

    // Be sure not to get a dot in the directory!
    if (dot > end)
        return fName.Substr(end + 1, dot - end - 1);
    return fName.Substr(end + 1);
}

plFileName plFileName::StripFileName() const
{
    int end = fName.FindLast('/');
    if (end < 0)
        end = fName.FindLast('\\');
    if (end < 0)
        return "";

    return fName.Left(end);
}

plFileName plFileName::StripFileExt() const
{
    int dot = fName.FindLast('.');

    // Be sure not to get a dot in the directory!
    int end = fName.FindLast('/');
    if (end < 0)
        end = fName.FindLast('\\');

    if (dot > end)
        return fName.Left(dot);

    return *this;
}

plFileName plFileName::Normalize(char slash) const
{
    plStringBuffer<char> norm;
    char *norm_p = norm.CreateWritableBuffer(fName.GetSize());
    for (const char *p = fName.c_str(); *p; ++p) {
        if (*p == '/' || *p == '\\')
            *norm_p++ = slash;
        else
            *norm_p++ = *p;
    }
    *norm_p = 0;
    return plString(norm);
}

plFileName plFileName::AbsolutePath() const
{
    if (!IsValid())
        return *this;

    plFileName path = Normalize();

#if HS_BUILD_FOR_WIN32
    plStringBuffer<wchar_t> wideName = path.fName.ToWchar();
    wchar_t path_sm[MAX_PATH];
    uint32_t path_length = GetFullPathNameW(wideName, MAX_PATH, path_sm, nullptr);
    if (path_length >= MAX_PATH) {
        // Buffer not big enough
        wchar_t *path_lg = new wchar_t[path_length];
        GetFullPathNameW(wideName, path_length, path_lg, nullptr);
        path = plString::FromWchar(path_lg);
        delete [] path_lg;
    } else {
        path = plString::FromWchar(path_sm);
    }
#else
    char *path_a = realpath(path.fName.c_str(), nullptr);
    hsAssert(path_a, "Failure to get absolute path (unsupported libc?)");
    path = path_a;
    free(path_a);
#endif

    return path;
}

plFileName plFileName::Join(const plFileName &base, const plFileName &path)
{
    if (!base.IsValid())
        return path;
    if (!path.IsValid())
        return base;

    char last = base.fName.CharAt(base.GetSize() - 1);
    char first = path.fName.CharAt(0);
    if (last != '/' && last != '\\') {
        if (first != '/' && first != '\\') {
            return plString::Format("%s" PATH_SEPARATOR_STR "%s",
                                    base.fName.c_str(), path.fName.c_str());
        }
        return base.fName + path.fName;
    } else if (first != '/' && first != '\\') {
        return base.fName + path.fName;
    }
    // Both have a slash, but we only need one
    return base.fName + path.fName.Substr(1);
}


/* plFileInfo */
plFileInfo::plFileInfo(const plFileName &filename)
    : fFileSize(-1), fCreateTime(), fModifyTime(), fFlags()
{
    if (!filename.IsValid())
        return;

#if HS_BUILD_FOR_WIN32
    struct __stat64 info;
    if (!_wstat64(filename.AsString().ToWchar(), &info) == 0)
        return;
#else
    struct stat info;
    if (!stat(filename.AsString().c_str(), &info) == 0)
        return;
#endif

    fFlags |= kEntryExists;
    fFileSize = info.st_size;
    fCreateTime = info.st_ctime;
    fModifyTime = info.st_mtime;
    if (info.st_mode & S_IFDIR)
        fFlags |= kIsDirectory;
    if (info.st_mode & S_IFREG)
        fFlags |= kIsNormalFile;
}


/* plFileSystem */
plFileName plFileSystem::GetCWD()
{
    plFileName cwd;

#if HS_BUILD_FOR_WIN32
    wchar_t cwd_sm[MAX_PATH];
    uint32_t cwd_length = GetCurrentDirectoryW(MAX_PATH, cwd_sm);
    if (cwd_length >= MAX_PATH) {
        // Buffer not big enough
        wchar_t *cwd_lg = new wchar_t[cwd_length];
        GetCurrentDirectoryW(cwd_length, cwd_lg);
        cwd = plString::FromWchar(cwd_lg);
        delete [] cwd_lg;
    } else {
        cwd = plString::FromWchar(cwd_sm);
    }
#else
    char *cwd_a = getcwd(nullptr, 0);
    hsAssert(cwd_a, "Failure to get working directory (unsupported libc?)");
    cwd = cwd_a;
    free(cwd_a);
#endif

    return cwd;
}

bool plFileSystem::SetCWD(const plFileName &cwd)
{
#if HS_BUILD_FOR_WIN32
    return SetCurrentDirectoryW(cwd.AsString().ToWchar());
#else
    return (chdir(cwd.AsString().c_str()) == 0);
#endif
}

FILE *plFileSystem::Open(const plFileName &filename, const char *mode)
{
#if HS_BUILD_FOR_WIN32
    wchar_t wmode[8];
    size_t mlen = strlen(mode);
    hsAssert(mlen < arrsize(wmode), "Mode string too long");

    // Quick and dirty, because mode should only ever be ANSI chars
    for (size_t i = 0; i < mlen; ++i) {
        hsAssert(!(mode[i] & 0x80), "I SAID mode should ONLY ever be ANSI chars!");
        wmode[i] = static_cast<wchar_t>(mode[i]);
    }
    wmode[mlen] = 0;

    return _wfopen(filename.AsString().ToWchar(), wmode);
#else
    return fopen(filename.AsString().c_str(), mode);
#endif
}

bool plFileSystem::Unlink(const plFileName &filename)
{
#if HS_BUILD_FOR_WIN32
    plStringBuffer<wchar_t> wfilename = filename.AsString().ToWchar();
    _wchmod(wfilename, S_IWRITE);
    return _wunlink(wfilename) == 0;
#else
    chmod(filename.AsString().c_str(), S_IWRITE);
    return unlink(filename.AsString().c_str()) == 0;
#endif
}

bool plFileSystem::Move(const plFileName &from, const plFileName &to)
{
#if HS_BUILD_FOR_WIN32
    return MoveFileExW(from.AsString().ToWchar(), to.AsString().ToWchar(),
                       MOVEFILE_REPLACE_EXISTING);
#else
    if (!Copy(from, to))
        return false;
    return Unlink(from);
#endif
}

bool plFileSystem::Copy(const plFileName &from, const plFileName &to)
{
#if HS_BUILD_FOR_WIN32
    return CopyFileW(from.AsString().ToWchar(), to.AsString().ToWchar(), FALSE);
#else
    typedef std::unique_ptr<FILE, std::function<int (FILE *)>> _FileRef;

    _FileRef ffrom(Open(from, "rb"), fclose);
    _FileRef fto(Open(to, "wb"), fclose);
    if (!ffrom.get() || !fto.get())
        return false;

    size_t count;
    uint8_t buffer[4096];
    while (!feof(ffrom.get())) {
        count = fread(buffer, sizeof(uint8_t), arrsize(buffer), ffrom.get());
        if (ferror(ffrom.get()))
            return false;
        fwrite(buffer, sizeof(uint8_t), count, fto.get());
    }

    return true;
#endif
}

bool plFileSystem::CreateDir(const plFileName &dir, bool checkParents)
{
    plFileName fdir = dir;
    if (fdir.GetFileName().IsEmpty()) {
        hsDebugMessage("WARNING: CreateDir called with useless trailing slash", 0);
        fdir = fdir.StripFileName();
    }

    if (checkParents) {
        plFileName parent = fdir.StripFileName();
        if (parent.IsValid() && !plFileInfo(parent).Exists() && !CreateDir(parent, true))
            return false;
    }

    if (plFileInfo(fdir).Exists())
        return true;

#if HS_BUILD_FOR_WIN32
    return CreateDirectoryW(fdir.AsString().ToWchar(), nullptr);
#else
    return (mkdir(fdir.AsString().c_str(), 0755) == 0);
#endif
}

std::vector<plFileName> plFileSystem::ListDir(const plFileName &path, const char *pattern)
{
    std::vector<plFileName> contents;

#if HS_BUILD_FOR_WIN32
    if (!pattern || !pattern[0])
        pattern = "*";
    plFileName searchPattern = plFileName::Join(path, pattern);

    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(searchPattern.AsString().ToWchar(), &findData);
    if (hFind == INVALID_HANDLE_VALUE)
        return contents;

    do {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // Should also handle . and ..
            continue;
        }

        contents.push_back(plFileName::Join(path, plString::FromWchar(findData.cFileName)));
    } while (FindNextFileW(hFind, &findData));

    FindClose(hFind);
#else
    DIR *dir = opendir(path.AsString().c_str());
    if (!dir)
        return contents;

    struct dirent *de;
    while (de = readdir(dir)) {
        plFileName dir_name = plFileName::Join(path, de->d_name);
        if (plFileInfo(dir_name).IsDirectory()) {
            // Should also handle . and ..
            continue;
        }

        if (pattern && pattern[0] && fnmatch(pattern, de->d_name, 0))
            contents.push_back(dir_name);
        else if (!pattern || !pattern[0])
            contents.push_back(dir_name);
    }

    closedir(dir);
#endif

    return contents;
}

std::vector<plFileName> plFileSystem::ListSubdirs(const plFileName &path)
{
    std::vector<plFileName> contents;

#if HS_BUILD_FOR_WIN32
    plFileName searchPattern = plFileName::Join(path, "*");

    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(searchPattern.AsString().ToWchar(), &findData);
    if (hFind == INVALID_HANDLE_VALUE)
        return contents;

    do {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            plFileName name = plString::FromWchar(findData.cFileName);
            if (name != "." && name != "..")
                contents.push_back(plFileName::Join(path, name));
        }
    } while (FindNextFileW(hFind, &findData));

    FindClose(hFind);
#else
    DIR *dir = opendir(path.AsString().c_str());
    if (!dir)
        return contents;

    struct dirent *de;
    while (de = readdir(dir)) {
        if (plFileInfo(de->d_name).IsDirectory()) {
            plFileName name = de->d_name;
            if (name != "." && name != "..")
                contents.push_back(plFileName::Join(path, name));
        }
    }

    closedir(dir);
#endif

    return contents;
}

plFileName plFileSystem::GetUserDataPath()
{
    static plFileName _userData;

    if (!_userData.IsValid()) {
#if HS_BUILD_FOR_WIN32
        wchar_t path[MAX_PATH];
        if (!SHGetSpecialFolderPathW(NULL, path, CSIDL_LOCAL_APPDATA, TRUE))
            return "";

        _userData = plFileName::Join(plString::FromWchar(path), plProduct::LongName());
#else
        _userData = plFileName::Join(getenv("HOME"), "." + plProduct::LongName());
#endif
        plFileSystem::CreateDir(_userData);
    }

    return _userData;
}

plFileName plFileSystem::GetInitPath()
{
    static plFileName _initPath = plFileName::Join(GetUserDataPath(), "Init");
    plFileSystem::CreateDir(_initPath);
    return _initPath;
}

plFileName plFileSystem::GetLogPath()
{
    static plFileName _logPath = plFileName::Join(GetUserDataPath(), "Log");
    plFileSystem::CreateDir(_logPath);
    return _logPath;
}

#if !HS_BUILD_FOR_WIN32
static plFileName _CheckReadlink(const char *link_path)
{
    plFileInfo info(link_path);
    if (info.Exists()) {
        char *path = new char[info.FileSize()];
        readlink(link_path, path, info.FileSize());
        plFileName appPath = plString::FromUtf8(path, info.FileSize());
        delete [] path;
        return appPath;
    }

    return "";
}
#endif

plFileName plFileSystem::GetCurrentAppPath()
{
    plFileName appPath;

    // Neither OS makes this one simple...
#if HS_BUILD_FOR_WIN32
    wchar_t path[MAX_PATH];
    size_t size = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (size >= MAX_PATH) {
        // Buffer not big enough
        size_t bigger = MAX_PATH;
        do {
            bigger *= 2;
            wchar_t *path_lg = new wchar_t[bigger];
            size = GetModuleFileNameW(nullptr, path_lg, bigger);
            if (size < bigger)
                appPath = plString::FromWchar(path_lg);
            delete [] path_lg;
        } while (!appPath.IsValid());
    } else {
        appPath = plString::FromWchar(path);
    }

    return appPath;
#else
    // Look for /proc/self/exe (Linux), /proc/curproc/file (FreeBSD / Mac),
    // then /proc/self/path/a.out (Solaris).  If none were found, you're SOL
    appPath = _CheckReadlink("/proc/self/exe");
    if (appPath.IsValid())
        return appPath;

    appPath = _CheckReadlink("/proc/curproc/file");
    if (appPath.IsValid())
        return appPath;

    appPath = _CheckReadlink("/proc/self/path/a.out");
    if (appPath.IsValid())
        return appPath;

    hsAssert(0, "Your OS doesn't make life easy, does it?");
#endif
}

plFileName plFileSystem::GetTempFilename(const char *prefix, const plFileName &path)
{
#if HS_BUILD_FOR_WIN32
    // GetTempFileName() never uses more than 3 chars for the prefix
    wchar_t wprefix[4];
    for (size_t i=0; i<4; ++i)
        wprefix[i] = prefix[i];
    wprefix[3] = 0;

    wchar_t temp[MAX_PATH];
    if (GetTempFileNameW(path.AsString().ToWchar(), wprefix, 0, temp))
        return plString::FromWchar(temp);

    return "";
#else
    plFileName tmpdir = path;
    if (!tmpdir.IsValid())
        tmpdir = "/tmp";

    // "/tmp/prefixXXXXXX"
    size_t temp_len = tmpdir.GetSize() + strlen(prefix) + 7;
    char *temp = new char[temp_len + 1];
    snprintf(temp, temp_len + 1, "%s/%sXXXXXX", tmpdir.AsString().c_str(), prefix);
    mktemp(temp);
    plFileName result = temp;
    delete [] temp;

    return result;
#endif
}
