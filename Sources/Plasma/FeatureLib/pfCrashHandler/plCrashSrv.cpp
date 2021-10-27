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

#include "plCrashSrv.h"
#include "plCrash_Private.h"
#include "plProduct.h"
#include "plFileSystem.h"

#ifdef HS_BUILD_FOR_WIN32

#include <dbghelp.h>
#include <shlobj.h>

plCrashSrv::plCrashSrv(const ST::string& file)
    : fLink(), fLinkH()
{
    // Init semas
    IInit(file);

    // Open the linked memory
    fLinkH = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, file.to_wchar().data());
    hsAssert(fLinkH, "Failed to open plCrashHandler mapping");
    if (!fLinkH)
        return;

    // Try to map it
    fLink = (plCrashMemLink*)MapViewOfFile(fLinkH, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(plCrashMemLink));
    hsAssert(fLink, "Failed to map plCrashMemLink");
}

plCrashSrv::~plCrashSrv()
{
    if (fLink)
        UnmapViewOfFile((LPCVOID)fLink);
    if (fLinkH)
        CloseHandle(fLinkH);
}

void plCrashSrv::IHandleCrash()
{
    plFileName dumpPath = plFileName::Join(plFileSystem::GetLogPath(), "crash.dmp");
    HANDLE file = CreateFileW(dumpPath.WideString().data(),
                              GENERIC_WRITE,
                              0,
                              nullptr,
                              CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL,
                              nullptr
    );

    MINIDUMP_EXCEPTION_INFORMATION e{};
    e.ClientPointers = TRUE;
    e.ExceptionPointers = fLink->fExceptionPtrs;
    e.ThreadId = fLink->fClientThreadID;
    MiniDumpWriteDump(fLink->fClientProcess, fLink->fClientProcessID, file, MiniDumpNormal, &e, nullptr, nullptr);
    CloseHandle(file);
}

#else
#   error "Implement plCrashSrv for this platform"
#endif

void plCrashSrv::HandleCrash()
{
    if (!fLink)
        FATAL("plCrashMemLink is nil!");
    fLink->fSrvReady = true; // mark us as ready to receive crashes

#ifdef HS_BUILD_FOR_WIN32
    // In Win32 land we have to hackily handle the client process exiting, so we'll wait on both
    // the crashed semaphore and the client process...
    HANDLE hack[2] = { fLink->fClientProcess, fCrashed->GetHandle() };
    DWORD result = WaitForMultipleObjects(std::size(hack), hack, FALSE, INFINITE);
    hsAssert(result != WAIT_FAILED, "WaitForMultipleObjects failed");
#else
    fCrashed->Wait();
#endif
    if (fLink->fCrashed)
        IHandleCrash();
    fHandled->Signal(); // Tell CrashCli we handled it
}
