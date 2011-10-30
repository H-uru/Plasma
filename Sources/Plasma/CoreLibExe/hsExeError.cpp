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
*   $/Plasma20/Sources/Plasma/CoreLibExe/hsExeError.cpp
*   
***/

#include "Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Private
*
***/

static bool         s_skipBreak;
#if HS_BUILD_FOR_WIN32
static CCritSect *  s_critsect;
#endif

// User options
static bool         s_options[kNumErrorOptions];


/*****************************************************************************
*
*   Internal functions
*
***/

//===========================================================================
#if HS_BUILD_FOR_WIN32
AUTO_INIT_FUNC(hsExeErrorInit) {
    // The critical section has to be initialized
    // before program startup and never freed
    static byte rawMemory[sizeof(CCritSect)];
    s_critsect = new(rawMemory) CCritSect;
}
#endif

//============================================================================
static void DoAssert (int line, const char file[], const char msg[]) {

    ErrorMinimizeAppWindow();

    #ifdef HS_BUILD_FOR_WIN32

    if (!s_options[kErrOptNonGuiAsserts]) {
        #ifdef HS_DEBUGGING
            bool wasLeakChecking = ErrorSetOption(kErrOptDisableMemLeakChecking, true);
            if (s_critsect)
                s_critsect->Enter();
            if (_CrtDbgReport(_CRT_ASSERT, file, line, NULL, msg))
                DebugBreak();
            if (s_critsect)
                s_critsect->Leave();
        (void) ErrorSetOption(kErrOptDisableMemLeakChecking, wasLeakChecking);
        #else
            DebugBreakIfDebuggerPresent();
        #endif
    }
    else {
        DebugMsg(msg);
        DebugBreakIfDebuggerPresent();
    }
        
    #else // !HS_BUILD_FOR_WIN32
    
        DebugMsg(msg);
        DebugBreakIfDebuggerPresent();
        
    #endif
}

/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
#pragma auto_inline(off)
void CDECL ErrorFatal (int line, const char file[], const char fmt[], ...) {
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    hsVsnprintf(buffer, arrsize(buffer), fmt, args);
    va_end(args);
    
    ErrorSetOption(kErrOptDisableMemLeakChecking, true);
    DoAssert(line, file, buffer);

    // Ensure thread crashes immediately by writing to invalid memory
    * (int *) 0 = 0;
}
#pragma auto_inline()

//============================================================================
#pragma auto_inline(off)
void CDECL ErrorAssert (int line, const char file[], const char fmt[], ...) {
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    hsVsnprintf(buffer, arrsize(buffer), fmt, args);
    va_end(args);

    DoAssert(line, file, buffer);
}
#pragma auto_inline()

//============================================================================
void ErrorMinimizeAppWindow () {
    #ifdef HS_BUILD_FOR_WIN32
        // If the application's topmost window is a fullscreen
        // popup window, minimize it before displaying an error
        HWND appWindow = GetActiveWindow();
        if ( ((GetWindowLong(appWindow, GWL_STYLE) & WS_POPUP) != 0) )
            SetWindowPos(
                appWindow,
                HWND_NOTOPMOST,
                0, 0,  // position
                0, 0,  // size
                SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE
            );
    #endif
}

//============================================================================
bool ErrorSetOption (EErrorOption option, bool on) {
    SWAP(s_options[option], on);
    if (option == kErrOptDisableMemLeakChecking)
        MemSetLeakChecking(on); // reverse logic, so use prev value
    return on;
}

//============================================================================
bool ErrorGetOption (EErrorOption option) {
    return s_options[option];
}

//============================================================================
bool DebugIsDebuggerPresent () {
    bool status = false;

#ifdef HS_BUILD_FOR_WIN32
    status = 0 != IsDebuggerPresent();
#endif

    return status;
}

//============================================================================
void DebugBreakIfDebuggerPresent () {
#ifdef HS_DEBUGGING
    // try breakpoint?
    if (s_skipBreak)
        return;

    __try {
        // break into debugger
        #ifdef _M_IX86
        __asm int 3
        #else
        __debugbreak();
        #endif
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        // debugger not present, stop attempting breaks
        s_skipBreak = true;
    }
#endif
}

//============================================================================
void DebugMsgV (const char fmt[], va_list args) {
#ifdef HS_DEBUGGING

    // Don't bother with debug output if no debugger attached
    if (!DebugIsDebuggerPresent())
        return;

    char msg[512];
    hsVsnprintf(msg, arrsize(msg), fmt, args);

    // MsDev trashes strings with colons in them; replace with period instead
    for (char * ptr = msg; *ptr; ++ptr) {
        if (*ptr == ':')
            *ptr = '.';
    }

    // Too many threads printing to OutputDebugString causes bizarre
    // results in developer studio; use critsect to serialize writes
    if (s_critsect)
        s_critsect->Enter();

    #ifdef HS_BUILD_FOR_WIN32    
    
        OutputDebugStringA(msg);
        OutputDebugStringA("\n");

    #else

        fprintf(stdout, msg);
        fprintf(stdout, "\n");

    #endif

    if (s_critsect)
        s_critsect->Leave();

#endif
}

//============================================================================
void CDECL DebugMsg (const char fmt[], ...) {
#ifdef HS_DEBUGGING

    va_list args;
    va_start(args, fmt);
    DebugMsgV(fmt, args);
    va_end(args);

#endif
}
