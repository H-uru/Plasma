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

#include "hsDebug.h"

#include "hsWindows.h"

#ifdef _MSC_VER
#   include <crtdbg.h>
#endif

#if defined(HS_BUILD_FOR_UNIX)
#   include <cstring>
#   include <sys/stat.h>
#   include <sys/utsname.h>
#   include <fcntl.h>
#   include <unistd.h>
#   include <signal.h>
#endif

#include <string_theory/format>
#include <string_theory/stdio>

//////////////////////////////////////////////////////////////////////////

static bool s_GuiAsserts = true;
void hsDebugEnableGuiAsserts(bool enabled)
{
    s_GuiAsserts = enabled;
}

#if !defined(HS_DEBUGGING)
[[noreturn]]
#endif // defined(HS_DEBUGGING)
void hsDebugAssertionFailed(int line, const char* file, const char* msg)
{
#if defined(HS_DEBUGGING)
#if defined(_MSC_VER)
    if (s_GuiAsserts)
    {
        if (_CrtDbgReport(_CRT_ASSERT, file, line, nullptr, "%s", msg))
            hsDebugBreakAlways();

        // All handling was done by the GUI, so bail.
        return;
    } else
#endif // _MSC_VER
    {
        hsDebugPrintToTerminal(ST::format("-------\nASSERTION FAILED:\nFile: {}   Line: {}\nMessage: {}\n-------", file, line, msg));
        fflush(stderr);

        hsDebugBreakAlways();
    }
#else
    hsDebugBreakIfDebuggerPresent();
    // If no debugger break occurred, just crash.
    std::abort();
#endif // defined(HS_DEBUGGING)
}

bool hsDebugIsDebuggerPresent()
{
#if defined(HS_BUILD_FOR_WIN32)
    return IsDebuggerPresent();
#elif defined(HS_BUILD_FOR_LINUX)
    // From http://google-perftools.googlecode.com/svn/trunk/src/heap-checker.cc
    char buf[256];   // TracerPid comes relatively earlier in status output
    int fd = open("/proc/self/status", O_RDONLY);
    if (fd == -1) {
        return false;  // Can't tell for sure.
    }
    const int len = read(fd, buf, sizeof(buf));
    bool rc = false;
    if (len > 0) {
        const char* const kTracerPid = "TracerPid:\t";
        buf[len - 1] = '\0';
        const char* p = strstr(buf, kTracerPid);
        if (p) {
            rc = (strncmp(p + strlen(kTracerPid), "0\n", 2) != 0);
        }
    }
    close(fd);
    return rc;
#else
    // FIXME
    return false;
#endif
}

void hsDebugBreakIfDebuggerPresent()
{
#if defined(_MSC_VER)
    __try
    {
        __debugbreak();
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        // Debugger not present or some such shwiz.
        // Whatever. Don't crash here.
    }
#elif defined(HS_BUILD_FOR_UNIX)
    if (hsDebugIsDebuggerPresent())
        raise(SIGTRAP);
#else
    // FIXME
#endif // _MSC_VER
}

void hsDebugBreakAlways()
{
#if defined(_MSC_VER)
    DebugBreak();
#elif defined(HS_BUILD_FOR_UNIX)
    raise(SIGTRAP);
#else
    // FIXME
    abort();
#endif // _MSC_VER
}

void hsDebugPrintToTerminal(const ST::string& msg)
{
    ST::printf(stderr, "{}\n", msg);

#ifdef _MSC_VER
    if (hsDebugIsDebuggerPresent())
    {
        // Also print to the MSVC Output window
        OutputDebugStringW(msg.to_wchar().c_str());
        OutputDebugStringW(L"\n");
    }
#endif
}

////////////////////////////////////////////////////////////////////////////

hsStatusMessageProc gHSStatusProc = nullptr;

hsStatusMessageProc hsSetStatusMessageProc(hsStatusMessageProc newProc)
{
    hsStatusMessageProc oldProc = gHSStatusProc;

    gHSStatusProc = newProc;

    return oldProc;
}

#ifndef PLASMA_EXTERNAL_RELEASE

void hsStatusMessage(const ST::string& message)
{
    if (gHSStatusProc) {
        gHSStatusProc(message);
    } else {
#if HS_BUILD_FOR_UNIX
        ST::printf("{}\n", message);
#elif HS_BUILD_FOR_WIN32
        OutputDebugStringW(message.to_wchar().c_str());
        OutputDebugStringW(L"\n");
#endif
    }
}

#endif
