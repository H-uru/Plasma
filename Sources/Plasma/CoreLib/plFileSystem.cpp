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

#include "plFileSystem.h"
#include "plProduct.h"

#include "HeadSpin.h"

#if HS_BUILD_FOR_WIN32
#   include "hsWindows.h"
#   include <shlobj.h>
#else
#   include <cstdlib>
#   include <functional>
#   include <memory>

#   include <fnmatch.h>
#   include <dirent.h>
#   include <limits.h>
#   include <sys/param.h>
#   include <sys/types.h>
#   include <unistd.h>
#endif

#ifdef HS_BUILD_FOR_APPLE
#   ifdef HAVE_SYSDIR
#       include <sysdir.h>
#   endif
#
#   include <CoreFoundation/CoreFoundation.h>
#   include <mach-o/dyld.h>
#endif


#ifdef HS_BUILD_FOR_MAC
#   include <NSSystemDirectories.h>
#endif

#include <sys/stat.h>


#include <string_theory/format>

/* NOTE For this file:  Windows uses UTF-16 filenames, and does not support
 * the use of UTF-8 in their ANSI API.  In order to ensure proper unicode
 * support, we convert the UTF-8 format stored in ST::string to UTF-16 before
 * passing them along to Windows.
 */

ST::string plFileName::GetFileName() const
{
    if (fName.ends_with("/")) {
        return plFileName(fName.before_last('/')).GetFileName();
    }
    ST_ssize_t end = fName.find_last('/');
    if (end < 0)
        end = fName.find_last('\\');
    if (end < 0)
        return fName;

    return fName.substr(end + 1);
}

ST::string plFileName::GetFileExt() const
{
    ST_ssize_t dot = fName.find_last('.');

    // Be sure not to get a dot in the directory!
    ST_ssize_t end = fName.find_last('/');
    if (end < 0)
        end = fName.find_last('\\');

    if (dot > end)
        return fName.substr(dot + 1);

    return ST::string();
}

ST::string plFileName::GetFileNameNoExt() const
{
    ST_ssize_t dot = fName.find_last('.');

    ST_ssize_t end = fName.find_last('/');
    if (end < 0)
        end = fName.find_last('\\');

    // Be sure not to get a dot in the directory!
    if (dot > end)
        return fName.substr(end + 1, dot - end - 1);
    return fName.substr(end + 1);
}

plFileName plFileName::StripFileName() const
{
    ST_ssize_t end = fName.find_last('/');
    if (end < 0)
        end = fName.find_last('\\');
    if (end < 0)
        return "";

    return fName.left(end);
}

plFileName plFileName::StripFileExt() const
{
    ST_ssize_t dot = fName.find_last('.');

    // Be sure not to get a dot in the directory!
    ST_ssize_t end = fName.find_last('/');
    if (end < 0)
        end = fName.find_last('\\');

    if (dot > end)
        return fName.left(dot);

    return *this;
}

plFileName plFileName::Normalize(char slash) const
{
    ST::char_buffer norm;
    norm.allocate(fName.size());
    char *norm_p = norm.data();
    for (const char *p = fName.c_str(); *p; ++p) {
        if (*p == '/' || *p == '\\')
            *norm_p++ = slash;
        else
            *norm_p++ = *p;
    }
    return ST::string(norm, ST::assume_valid);
}

plFileName plFileName::AbsolutePath() const
{
    if (!IsValid())
        return *this;

    plFileName path = Normalize();

#if HS_BUILD_FOR_WIN32
    ST::wchar_buffer wideName = path.WideString();
    wchar_t path_sm[MAX_PATH];
    uint32_t path_length = GetFullPathNameW(wideName.data(), MAX_PATH, path_sm, nullptr);
    if (path_length >= MAX_PATH) {
        // Buffer not big enough
        wchar_t *path_lg = new wchar_t[path_length];
        GetFullPathNameW(wideName.data(), path_length, path_lg, nullptr);
        path = ST::string::from_wchar(path_lg);
        delete [] path_lg;
    } else {
        path = ST::string::from_wchar(path_sm);
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

    char last = base.fName.back();
    char first = path.fName.front();
    if (last != '/' && last != '\\') {
        if (first != '/' && first != '\\')
            return ST::format("{}" PATH_SEPARATOR_STR "{}", base, path);
        return base.fName + path.fName;
    } else if (first != '/' && first != '\\') {
        return base.fName + path.fName;
    }
    // Both have a slash, but we only need one
    return base.fName + path.fName.substr(1);
}


/* plFileInfo */
plFileInfo::plFileInfo(const plFileName &filename)
    : fFileSize(-1), fCreateTime(), fModifyTime(), fFlags()
{
    if (!filename.IsValid())
        return;

#if HS_BUILD_FOR_WIN32
    struct __stat64 info;
    if (_wstat64(filename.WideString().data(), &info) != 0)
        return;
#else
    struct stat info;
    if (stat(filename.AsString().c_str(), &info) != 0)
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
        cwd = ST::string::from_wchar(cwd_lg);
        delete [] cwd_lg;
    } else {
        cwd = ST::string::from_wchar(cwd_sm);
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
    return SetCurrentDirectoryW(cwd.WideString().data());
#else
    return (chdir(cwd.AsString().c_str()) == 0);
#endif
}

FILE *plFileSystem::Open(const plFileName &filename, const char *mode)
{
#if HS_BUILD_FOR_WIN32
    wchar_t wmode[8];
    size_t mlen = strlen(mode);
    hsAssert(mlen < std::size(wmode), "Mode string too long");

    // Quick and dirty, because mode should only ever be ANSI chars
    for (size_t i = 0; i < mlen; ++i) {
        hsAssert(!(mode[i] & 0x80), "I SAID mode should ONLY ever be ANSI chars!");
        wmode[i] = static_cast<wchar_t>(mode[i]);
    }
    wmode[mlen] = 0;

    return _wfopen(filename.WideString().data(), wmode);
#else
    return fopen(filename.AsString().c_str(), mode);
#endif
}

bool plFileSystem::Unlink(const plFileName &filename)
{
#if HS_BUILD_FOR_WIN32
    ST::wchar_buffer wfilename = filename.WideString();
    _wchmod(wfilename.data(), S_IWRITE);
    return _wunlink(wfilename.data()) == 0;
#else
    chmod(filename.AsString().c_str(), S_IWRITE);
    return unlink(filename.AsString().c_str()) == 0;
#endif
}

bool plFileSystem::Move(const plFileName &from, const plFileName &to)
{
#if HS_BUILD_FOR_WIN32
    return MoveFileExW(from.WideString().data(), to.WideString().data(),
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
    return CopyFileW(from.WideString().data(), to.WideString().data(), FALSE);
#else
    typedef std::unique_ptr<FILE, std::function<int (FILE *)>> _FileRef;

    _FileRef ffrom(Open(from, "rb"), fclose);
    _FileRef fto(Open(to, "wb"), fclose);
    if (!ffrom.get() || !fto.get())
        return false;

    size_t count;
    uint8_t buffer[4096];
    while (!feof(ffrom.get())) {
        count = fread(buffer, sizeof(uint8_t), std::size(buffer), ffrom.get());
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
    if (fdir.GetFileName().empty()) {
        hsStatusMessage("WARNING: CreateDir called with useless trailing slash");
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
    return CreateDirectoryW(fdir.WideString().data(), nullptr);
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
    HANDLE hFind = FindFirstFileW(searchPattern.WideString().data(), &findData);
    if (hFind == INVALID_HANDLE_VALUE)
        return contents;

    do {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // Should also handle . and ..
            continue;
        }

        contents.push_back(plFileName::Join(path, ST::string::from_wchar(findData.cFileName)));
    } while (FindNextFileW(hFind, &findData));

    FindClose(hFind);
#else
    DIR *dir = opendir(path.AsString().c_str());
    if (!dir)
        return contents;

    struct dirent *de;
    while (de = readdir(dir), de) {
        plFileName dir_name = plFileName::Join(path, de->d_name);
        if (plFileInfo(dir_name).IsDirectory()) {
            // Should also handle . and ..
            continue;
        }

        if (pattern && pattern[0] && fnmatch(pattern, de->d_name, 0) == 0)
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
    HANDLE hFind = FindFirstFileW(searchPattern.WideString().data(), &findData);
    if (hFind == INVALID_HANDLE_VALUE)
        return contents;

    do {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            plFileName name = ST::string::from_wchar(findData.cFileName);
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
    while (de = readdir(dir), de) {
        if (plFileInfo(plFileName::Join(path, de->d_name)).IsDirectory()) {
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
        if (!SHGetSpecialFolderPathW(nullptr, path, CSIDL_LOCAL_APPDATA, TRUE))
            return "";

        _userData = plFileName::Join(ST::string::from_wchar(path), plProduct::LongName());
#elif HS_BUILD_FOR_APPLE
        char path[PATH_MAX] {};
#if defined(HAVE_BUILTIN_AVAILABLE) && defined(HAVE_SYSDIR)
        if (__builtin_available(macOS 10.12, *)) {
            sysdir_search_path_enumeration_state state;
            state = sysdir_start_search_path_enumeration(SYSDIR_DIRECTORY_APPLICATION_SUPPORT, SYSDIR_DOMAIN_MASK_USER);
            state = sysdir_get_next_search_path_enumeration(state, path);
        }
        else
#endif
        {
#ifdef HS_BUILD_FOR_MAC
            IGNORE_WARNINGS_BEGIN("deprecated-declarations")

            NSSearchPathEnumerationState state;
            state = NSStartSearchPathEnumeration(NSApplicationSupportDirectory, NSUserDomainMask);
            state = NSGetNextSearchPathEnumeration(state, path);

            IGNORE_WARNINGS_END
#endif
        }

        if (path[0] == '~') {
            char home[PATH_MAX] {};
            strlcat(home, getenv("HOME"), sizeof(home));
            strlcat(home, &path[1], sizeof(home));

            _userData = plFileName::Join(home, plProduct::LongName());
        } else {
            _userData = plFileName::Join(path, plProduct::LongName());
        }
#else
        const char* homedir = getenv("XDG_CONFIG_HOME");
        if (homedir) {
            _userData = plFileName::Join(homedir, plProduct::LongName());
        } else {
            homedir = getenv("HOME");
            if (!homedir)
                return "";

            _userData = plFileName::Join(homedir, ".config", plProduct::LongName());
        }
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
        plFileName appPath = ST::string::from_utf8(path, info.FileSize());
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
    DWORD size = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (size >= MAX_PATH) {
        // Buffer not big enough
        DWORD bigger = MAX_PATH;
        do {
            bigger *= 2;
            wchar_t *path_lg = new wchar_t[bigger];
            size = GetModuleFileNameW(nullptr, path_lg, bigger);
            if (size < bigger)
                appPath = ST::string::from_wchar(path_lg);
            delete [] path_lg;
        } while (!appPath.IsValid());
    } else {
        appPath = ST::string::from_wchar(path);
    }

    return appPath;
#elif HS_BUILD_FOR_MACOS
    CFBundleRef myBundle = CFBundleGetMainBundle();
    if (!myBundle) {
        char path[MAXPATHLEN];
        uint32_t pathLen = MAXPATHLEN;
        _NSGetExecutablePath(path, &pathLen);
        appPath = ST::string::from_utf8(path, pathLen);
    } else {
        CFURLRef url = CFBundleCopyBundleURL(myBundle);
        CFStringRef path = CFURLCopyPath(url);
        appPath = ST::string::from_utf8(CFStringGetCStringPtr(path, kCFStringEncodingUTF8));
        CFRelease(path);
        CFRelease(url);
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

    FATAL("Your OS doesn't make life easy, does it?");
    return ".";
#endif
}

ST::string plFileSystem::ConvertFileSize(uint64_t size)
{
    const char* labels[] = { "KiB", "MiB", "GiB", "TiB", "PiB", "EiB" };
    if (size < 1024)
        return ST::format("{} B", size);

    uint64_t last_div = size;
    for (size_t i = 0; i < std::size(labels); ++i) {
        uint64_t my_div = last_div / 1024;
        if (my_div < 1024) {
            float decimal = static_cast<float>(last_div) / 1024.f;
            // Kilobytes are so small that we only care about whole numbers
            if (i < 1)
                return ST::format("{.0f} {}", decimal, labels[i]);
            else
                return ST::format("{.2f} {}", decimal, labels[i]);
        }
        last_div = my_div;
    }

    // this should never happen
    return ST::format("{} {}", last_div, labels[std::size(labels) - 1]);
}
