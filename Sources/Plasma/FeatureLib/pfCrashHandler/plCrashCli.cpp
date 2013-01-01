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

#include "plCrashCli.h"
#include "plCrash_Private.h"

#ifdef HS_BUILD_FOR_WIN32

plCrashCli::plCrashCli()
    : fLink(nil), fLinkH(nil)
{
    char mapname[128];
    char cmdline[128];
    snprintf(mapname, arrsize(mapname), "Plasma20CrashHandler-%u", GetCurrentProcessId());
    snprintf(cmdline, arrsize(cmdline), "%s %s", CRASH_HANDLER_EXE, mapname);
    memset(&fCrashSrv, 0, sizeof(PROCESS_INFORMATION));

    // Initialize the semas
    IInit(mapname);

    // Initialize the shared memory
    fLinkH = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(plCrashMemLink), mapname);
    hsAssert(fLinkH, "Failed to create plCrashHandler mapping");
    if (!fLinkH)
        return;

    // Map the shared memory
    fLink = (plCrashMemLink*)MapViewOfFile(fLinkH, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(plCrashMemLink));
    hsAssert(fLink, "Failed to map plCrashLinkedMem");
    if (!fLink)
        return;
    memset(fLink, 0, sizeof(plCrashMemLink));
    fLink->fClientProcessID = GetCurrentProcessId();

    // Start the plCrashHandler before a crash
    STARTUPINFOA info; memset(&info, 0, sizeof(info));
    info.cb = sizeof(STARTUPINFOA);
    CreateProcessA(
                   CRASH_HANDLER_EXE, // plCrashHandler.exe
                   cmdline,           // plCrashHandler.exe Plasma20CrashHandler-%u
                   NULL,
                   NULL,
                   FALSE,
                   CREATE_NO_WINDOW, // Don't create any new windows or consoles
                   NULL,
                   NULL,             // Use the directory of the current plClient
                   &info,
                   &fCrashSrv        // Save the CrashSrv handles
    );

    HANDLE curProc = GetCurrentProcess();
    DuplicateHandle(curProc,                  // Handle to the source process
                    curProc,                  // Handle that we want duplicated
                    fCrashSrv.hProcess,       // Handle to target process
                    &fLink->fClientProcess,   // Pointer to Handle to dupliicate to
                    0,                        // Ignored
                    FALSE,
                    DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS
    );
}

plCrashCli::~plCrashCli()
{
    fCrashed->Signal(); // forces the CrashSrv to exit, if it's still running
    if (fCrashSrv.hProcess)
    {
        TerminateProcess(fCrashSrv.hProcess, 0);
        CloseHandle(fCrashSrv.hProcess);
    }
    if (fCrashSrv.hThread)
        CloseHandle(fCrashSrv.hThread);
    if (fLink)
        UnmapViewOfFile((LPCVOID)fLink);
    if (fLinkH)
        CloseHandle(fLinkH);
}

void plCrashCli::ReportCrash(PEXCEPTION_POINTERS e)
{
    hsAssert(fLink, "plCrashMemLink is nil");
    if (fLink)
    {
        fLink->fClientThreadID = GetCurrentThreadId();
        fLink->fCrashed = true;
        fLink->fExceptionPtrs  = e;
    }

    fCrashed->Signal();
}

#else
#   error "Implement plCrashCli for this platform"
#endif

void plCrashCli::WaitForHandle()
{
    // Don't deadlock... Only wait if the CrashSrv is attached
    if (fLink && fLink->fSrvReady)
        fHandled->Wait();
}
