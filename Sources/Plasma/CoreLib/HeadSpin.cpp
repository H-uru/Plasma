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
#include "hsRefCnt.h"
#include "hsStlUtils.h"
#include "hsExceptions.h"
#include <math.h>

#ifdef _MSC_VER
#   include <crtdbg.h>
#endif


///////////////////////////////////////////////////////////////////////////
/////////////////// For Status Messages ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////
hsDebugMessageProc gHSStatusProc = nil;

hsDebugMessageProc hsSetStatusMessageProc(hsDebugMessageProc newProc)
{
    hsDebugMessageProc oldProc = gHSStatusProc;

    gHSStatusProc = newProc;

    return oldProc;
}

//////////////////////////////////////////////////////////////////////////

hsDebugMessageProc gHSDebugProc = nil;

hsDebugMessageProc hsSetDebugMessageProc(hsDebugMessageProc newProc)
{
    hsDebugMessageProc oldProc = gHSDebugProc;

    gHSDebugProc = newProc;

    return oldProc;
}

#ifdef HS_DEBUGGING
void hsDebugMessage (const char message[], long val)
{
    char    s[1024];

    if (val)
        s[0] = snprintf(&s[1], 1022, "%s: %ld", message, val);
    else
        s[0] = snprintf(&s[1], 1022, "%s", message);

    if (gHSDebugProc)
        gHSDebugProc(&s[1]);
    else
#if HS_BUILD_FOR_WIN32
    {   OutputDebugString(&s[1]);
        OutputDebugString("\n");
    }
#elif HS_BUILD_FOR_UNIX
    {   fprintf(stderr, "%s\n", &s[1]);
//      hsThrow(&s[1]);
    }
#else
    hsThrow(&s[1]);
#endif
}
#endif

static bool s_GuiAsserts = true;
void ErrorEnableGui(bool enabled)
{
    s_GuiAsserts = enabled;
}

void ErrorAssert(int line, const char file[], const char fmt[], ...)
{
#if defined(HS_DEBUGGING) || !defined(PLASMA_EXTERNAL_RELEASE)
    char msg[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
#ifdef HS_DEBUGGING
    if (s_GuiAsserts)
    {
        if(_CrtDbgReport(_CRT_ASSERT, file, line, NULL, msg))
            DebugBreak();
    } else 
#endif // HS_DEBUGGING
      if (DebugIsDebuggerPresent()) {
        char str[] = "-------\nASSERTION FAILED:\nFile: %s   Line: %i\nMessage: %s\n-------";
        DebugMsg(str, file, line, msg);
    }
#else
    DebugBreakIfDebuggerPresent();
#endif // defined(HS_DEBUGGING) || !defined(PLASMA_EXTERNAL_RELEASE)
}

bool DebugIsDebuggerPresent()
{
#ifdef _MSC_VER
    return IsDebuggerPresent();
#else
    // FIXME
    return false;
#endif
}

void DebugBreakIfDebuggerPresent()
{
#ifdef _MSC_VER
    __try 
    {
        __debugbreak();
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        // Debugger not present or some such shwiz.
        // Whatever. Don't crash here.
    }
#endif // _MSC_VER
}

void DebugMsg(const char fmt[], ...)
{
    char msg[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);

    if (DebugIsDebuggerPresent())
    {
#ifdef _MSC_VER
        OutputDebugStringA(msg);
        OutputDebugStringA("\n");
#endif
    } else {
        fprintf(stderr, msg);
        fprintf(stderr, "\n");
    }
}

void ErrorMinimizeAppWindow () 
{
#ifdef HS_BUILD_FOR_WIN32
    // If the application's topmost window is a fullscreen
    // popup window, minimize it before displaying an error
    HWND appWindow = GetActiveWindow();
    if ( ((GetWindowLong(appWindow, GWL_STYLE) & WS_POPUP) != 0) )
        SetWindowPos(
            appWindow,
            HWND_NOTOPMOST,
            0, 0, // position
            0, 0, // size
            SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE
        );
#endif
}

///////////////////////////////////////////////////////////////////


hsRefCnt::~hsRefCnt()
{
    hsDebugCode(hsThrowIfFalse(fRefCnt == 1);)
}

void hsRefCnt::Ref()
{
    fRefCnt++;
}

void hsRefCnt::UnRef()
{
    hsDebugCode(hsThrowIfFalse(fRefCnt >= 1);)

    if (fRefCnt == 1)   // don't decrement if we call delete
        delete this;
    else
        --fRefCnt;
}


////////////////////////////////////////////////////////////////////////////

#ifndef PLASMA_EXTERNAL_RELEASE

void hsStatusMessage(const char message[])
{
  if (gHSStatusProc) {
    gHSStatusProc(message);
  } else {
#if HS_BUILD_FOR_UNIX
    printf("%s",message);
    int len = strlen(message);
    if (len>0 && message[len-1]!='\n')
        printf("\n");
#elif HS_BUILD_FOR_WIN32
    OutputDebugString(message);
    int len = strlen(message);
    if (len>0 && message[len-1]!='\n')
        OutputDebugString("\n");
#endif
  }
}

void hsStatusMessageV(const char * fmt, va_list args)
{
    char  buffer[2000];
    vsprintf(buffer, fmt, args);
    hsStatusMessage(buffer);
}

void hsStatusMessageF(const char * fmt, ...)
{
    va_list args;
    va_start(args,fmt);
    hsStatusMessageV(fmt,args);
    va_end(args);
}

#endif // not PLASMA_EXTERNAL_RELEASE
