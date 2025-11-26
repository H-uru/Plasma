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

#ifndef plFileSystem_Defined
#define plFileSystem_Defined

#include <string_theory/formatter>
#include <cstddef>

#if HS_BUILD_FOR_WIN32
#   define PATH_SEPARATOR       '\\'
#   define PATH_SEPARATOR_STR   "\\"
#else
#   define PATH_SEPARATOR       '/'
#   define PATH_SEPARATOR_STR   "/"
#endif

/** Represents a filename or path, including utilities for manipulating,
 *  splitting, and joining path components.
 *  \sa plFileInfo
 */
class plFileName
{
public:
    /** Construct an empty filename. */
    plFileName() { }

    /** Construct a filename from the UTF-8 character data in \a cstr. */
    plFileName(const char *cstr) : fName(cstr) { }

    /** Construct a filename from the ST::string argument \a copy. */
    plFileName(const ST::string &copy) : fName(copy) { }

    /** Copy constructor. */
    plFileName(const plFileName &copy) : fName(copy.fName) { }

    /** Move constructor. */
    plFileName(plFileName &&move) : fName(std::move(move.fName)) { }

    /** Assignment operator.  Same as plFileName(const char *). */
    plFileName &operator=(const char *cstr)
    {
        fName.operator=(cstr);
        return *this;
    }

    /** Assignment operator.  Same as plFileName(const ST::string &). */
    plFileName &operator=(const ST::string &copy)
    {
        fName.operator=(copy);
        return *this;
    }

    /** Assignment operator.  Same as plFileName(const plFileName &). */
    plFileName &operator=(const plFileName &copy)
    {
        fName.operator=(copy.fName);
        return *this;
    }

    /** Assignment operator.  Same as plFileName(plFileName &&). */
    plFileName &operator=(plFileName &&move)
    {
        fName.operator=(std::move(move.fName));
        return *this;
    }

    /** Comparison operator. */
    bool operator==(const char *other) const { return fName.operator==(other); }

    /** Comparison operator. */
    bool operator==(const plFileName &other) const { return fName.operator==(other.fName); }

    /** Inverse of operator==(const char *other) const. */
    bool operator!=(const char *other) const { return fName.operator!=(other); }

    /** Inverse of operator==(const plFileName &other) const. */
    bool operator!=(const plFileName &other) const { return fName.operator!=(other.fName); }

    /** Operator overload for use in containers which depend on \c std::less. */
    bool operator<(const plFileName &other) const { return fName.compare(other.fName) < 0; }

    /** Functor which compares two filenames case-insensitively for sorting. */
    struct less_i
    {
        bool operator()(const plFileName &lhs, const plFileName &rhs) const
        { return lhs.fName.compare_i(rhs.fName) < 0; }
    };

    /** Return whether this filename is valid (not empty). */
    bool IsValid() const { return !fName.empty(); }

    /** Return the length of the filename string (UTF-8). */
    size_t GetSize() const { return fName.size(); }

    /** Convert the filename to a string.  This does not resolve relative
     *  paths or normalize slashes, it just returns the stored name string.
     */
    const ST::string &AsString() const { return fName; }

    /** Convert the filename to a wchar_t buffer (for use in Win32 APIs). */
    ST::wchar_buffer WideString() const { return fName.to_wchar(); }

    /** Return the name portion of the path (including extension).
     *  For example:
     *  <pre>plFileName("C:\\Path\\Filename.ext") => "Filename.ext"</pre>
     */
    ST::string GetFileName() const;

    /** Return the file extension from the filename.
     *  For example:
     *  <pre>plFileName("C:\\Path\\Filename.ext") => "ext"</pre>
     */
    ST::string GetFileExt() const;

    /** Return the name portion of the path, excluding its extension.
     *  For example:
     *  <pre>plFileName("C:\\Path\\Filename.ext") => "Filename"</pre>
     */
    ST::string GetFileNameNoExt() const;

    /** Return the path with the filename portion stripped off.
     *  For example:
     *  <pre>plFileName("C:\\Path\\Filename.ext") => "C:\\Path"</pre>
     */
    plFileName StripFileName() const;

    /** Return the filename with the extension stripped off.
     *  For example:
     *  <pre>plFileName("C:\\Path\\Filename.ext") => "C:\\Path\\Filename"</pre>
     */
    plFileName StripFileExt() const;

    /** Normalize slashes to a particular format.  By default, we use the
     *  OS's native slash format.
     *  For example:
     *  <pre>plFileName("C:\\Path/Filename.ext").Normalize('\\') => "C:\\Path\\Filename.ext"</pre>
     */
    plFileName Normalize(char slash = PATH_SEPARATOR) const;

    /** Expand relative filenames and ./.. pieces to an absolute path. */
    plFileName AbsolutePath() const;

    /** Join two path components together with the correct path separator.
     *  For example:
     *  <pre>plFileName::Join("C:\\Path", "Filename.ext") => "C:\\Path\\Filename.ext"</pre>
     */
    static plFileName Join(const plFileName &base, const plFileName &path);

    /** Join three path components together with the correct path separator.
     *  \todo Make this more efficient.
     */
    static plFileName Join(const plFileName &base, const plFileName &path,
                           const plFileName& path2)
    {
        return Join(Join(base, path), path2);
    }

    /** Join four path components together with the correct path separator.
     *  \todo Make this more efficient.
     */
    static plFileName Join(const plFileName &base, const plFileName &path,
                           const plFileName& path2, const plFileName &path3)
    {
        return Join(Join(Join(base, path), path2), path3);
    }

    /** Append UTF-8 data from a C-style string pointer to the end of this
     *  filename object.  Not to be confused with Join() -- do not use this
     *  for joining path components, or you will be shot by Zrax.
     */
    plFileName &operator+=(const char *cstr) { return operator=(fName + cstr); }

    /** Append the string \a str to the end of this filename object.
     *  Not to be confused with Join() -- do not use this for joining path
     *  components, or you will be shot by Zrax.
     */
    plFileName &operator+=(const ST::string &str) { return operator=(fName + str); }

private:
    ST::string fName;

    // See the comments in plString's nullptr_t constructors for more info:
    plFileName(std::nullptr_t) { }
    void operator=(std::nullptr_t) { }
    void operator==(std::nullptr_t) const { }
    void operator!=(std::nullptr_t) const { }
};

inline ST_FORMAT_TYPE(const plFileName &)
{
    ST_FORMAT_FORWARD(value.AsString());
}

/** Concatentate a plFileName with a string constant.  Not to be confused with
 *  plFileName::Join() -- do not use this for joining path components, or you
 *  will be shot by Zrax.
 */
inline plFileName operator+(const plFileName &left, const char *right)
{
    return left.AsString() + right;
}

/** Concatentate a plFileName with a string constant.  Not to be confused with
 *  plFileName::Join() -- do not use this for joining path components, or you
 *  will be shot by Zrax.
 */
inline plFileName operator+(const char *left, const plFileName &right)
{
    return left + right.AsString();
}


/** Structure to get information about a file by name.
 *  \sa plFileName
 */
class plFileInfo
{
public:
    /** Construct an invalid plFileInfo which points to no file. */
    plFileInfo()
        : fFileSize(-1), fCreateTime(), fModifyTime(), fFlags() { }

    /** Construct a plFileInfo and fill it with info about the specified
     *  file, if it exists.
     */
    explicit plFileInfo(const plFileName &filename);

    /** Retrieve the filename associated with this info structure. */
    const plFileName &FileName() const { return fName; }

    /** Return whether the plFileInfo has been initialized. */
    bool IsValid() const { return fName.IsValid(); }

    /** Determine whether the file exists on the filesystem. */
    bool Exists() const { return (fFlags & kEntryExists); }

    /** Returns the size of the file on the disk, in bytes. */
    int64_t FileSize() const { return fFileSize; }

    /** Returns the creation time of the file. */
    int64_t CreateTime() const { return fCreateTime; }

    /** Returns the last modification time of the file. */
    int64_t ModifyTime() const { return fModifyTime; }

    /** Returns \p true if this file is a directory. */
    bool IsDirectory() const { return (fFlags & kIsDirectory) != 0; }

    /** Returns \p true if this file is a regular file. */
    bool IsFile() const { return (fFlags & kIsNormalFile) != 0; }

private:
    plFileName fName;
    int64_t fFileSize;
    int64_t fCreateTime, fModifyTime;

    enum {
        kEntryExists    = (1<<0),
        kIsDirectory    = (1<<1),
        kIsNormalFile   = (1<<2),
    };
    uint32_t fFlags;
};


namespace plFileSystem
{
    /** Get the current working directory of the application. */
    plFileName GetCWD();

    /** Change the current working directory. */
    bool SetCWD(const plFileName &cwd);

    /** Open a file using the correct platform fopen API. */
    FILE *Open(const plFileName &filename, const char *mode);

    /** Delete a file from the filesystem. */
    bool Unlink(const plFileName &filename);

    /** Move or rename a file. */
    bool Move(const plFileName &from, const plFileName &to);

    /** Copy a file to a new location. */
    bool Copy(const plFileName &from, const plFileName &to);

    /** Create a directory.  If \a checkParents is \p true, this will also
     *  check the whole path and create any parent directories as needed.
     */
    bool CreateDir(const plFileName &dir, bool checkParents = false);

    /** Fetch a list of files contained in the supplied \a path.
     *  If \a pattern is specified (e.g. "*.tmp"), use that to filter
     *  matches.  Otherwise, all files in the path will be returned.
     *  Note that returned filenames include the provided path -- to
     *  get only the filename, call .GetFileName() on an entry.
     */
    std::vector<plFileName> ListDir(const plFileName &path,
                                    const char *pattern = nullptr);

    /** Fetch a list of subdirectories in the specified \a path.
     *  The returned list does not include the "." or ".." entries.
     */
    std::vector<plFileName> ListSubdirs(const plFileName &path);

    /** Get the User's data directory.  If it doesn't exist, this will
     *  create it.
     */
    plFileName GetUserDataPath();

    /** Get the Init script direcotory.  If it doesn't exist, this will
     *  create it. */
    plFileName GetInitPath();

    /** Get the Log output directory.  If it doesn't exist, this will
     *  create it. */
    plFileName GetLogPath();

    /** Get the full path and filename of the current process. */
    plFileName GetCurrentAppPath();

    /** Convert a file size from bytes to a human readable size. */
    ST::string ConvertFileSize(uint64_t size);
}

#endif // plFileSystem_Defined
