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

#include "hsFILELock.h"

#include <string_theory/format>
#include <system_error>

#include "hsWindows.h"
#ifdef _MSC_VER
#   include <io.h>
#endif
#ifndef HS_BUILD_FOR_WIN32
#   include <sys/file.h>
#endif

bool hsFILELock::ILock(bool block) const
{
    if (fRef == nullptr)
        return true;

#ifdef HS_BUILD_FOR_WIN32
    OVERLAPPED o{};
    DWORD flags = LOCKFILE_EXCLUSIVE_LOCK;
    if (block)
        flags |= LOCKFILE_FAIL_IMMEDIATELY;

    // Lock only the first byte of the file - we have
    // to provide the exact same region to UnlockFileEx(),
    // and this is a mandatory lock. Totally different than
    // POSIX file locks, which are advisory and operate on
    // the whole file.
    BOOL result = LockFileEx(
#ifdef _MSC_VER
        (HANDLE)_get_osfhandle(_fileno(fRef)),
#else
        (HANDLE)fileno(fRef),
#endif // _MSC_VER
        flags,
        0,
        1,
        0,
        &o
    );
    if (result == 0) {
        DWORD error = GetLastError();
        if (!block && error == ERROR_IO_PENDING)
            return false;
        // BasicLockable requires an exception to indicate that the
        // lock was not acquired.
        throw std::system_error(
            std::error_code(error, std::system_category()),
            ST::format(
                "LockFileEx() failed: {}",
                hsCOMError(hsLastWin32Error, error)
            ).to_std_string()
        );
    }

#else
    int op = LOCK_EX;
    if (!block)
        op |= LOCK_NB;
    int result = flock(fileno(fRef), op);
    if (result == -1) {
        if (!block && errno == EWOULDBLOCK)
            return false;
        // BasicLockable requires an exception to indicate that the
        // lock was not acquired. Even though EINTR is probably not
        // an error condition, we should still throw to indicate no
        // lock was acquired.
        throw std::system_error(
            std::error_code(errno, std::system_category()),
            ST::format(
                "flock() LOCK_EX failed: {}",
                strerror(errno)
            ).to_std_string()
        );
    }
#endif // HS_BUILD_FOR_WIN32

    return true;
}

void hsFILELock::unlock() const
{
    if (fRef == nullptr)
        return;

#ifdef HS_BUILD_FOR_WIN32
    OVERLAPPED o{};
    BOOL result = UnlockFileEx(
#ifdef _MSC_VER
        (HANDLE)_get_osfhandle(_fileno(fRef)),
#else
        (HANDLE)fileno(fRef),
#endif // _MSC_VER
        0,
        1,
        0,
        &o
    );
    // BasicLockable doesn't allow exceptions to be
    // thrown in the unlock method.
    hsAssert(
        result,
        ST::format(
            "UnlockFileEx() failed: {}",
            hsCOMError(hsLastWin32Error, GetLastError())
        ).c_str()
    );

#else
    int result = flock(fileno(fRef), LOCK_UN);
    hsAssert(
        result == 0,
        ST::format(
            "flock() LOCK_UN failed: {}",
            strerror(errno)
        ).c_str()
    );
#endif // HS_BUILD_FOR_WIN32
}
