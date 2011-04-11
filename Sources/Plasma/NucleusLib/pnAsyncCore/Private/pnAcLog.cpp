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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCore/Private/pnAcLog.cpp
*   
***/

#include "../Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Private
*
***/

static const unsigned   kMaxHandlers = 8;
static CCritSect        s_critsect;
static FLogHandler      s_asyncHandlers[kMaxHandlers];


/*****************************************************************************
*
*   Internal functions
*
***/

//===========================================================================
static void Dispatch (ELogSeverity severity, const wchar msg[]) {

    // Dispatch to default debug handler
    char dbg[1024];
    StrToAnsi(dbg, msg, arrsize(dbg));
    DEBUG_MSG(dbg);

    // We don't need to enter a critical section to read the handlers because they
    // are atomically set and cleared
    for (unsigned i = 0; i < arrsize(s_asyncHandlers); ++i) {
        if (FLogHandler asyncHandler = s_asyncHandlers[i])
            asyncHandler(severity, msg);
    }
}


/*****************************************************************************
*
*   Exports
*
***/

//===========================================================================
void LogRegisterHandler (FLogHandler callback) {
    ASSERT(callback);

    unsigned i;
    s_critsect.Enter();
    for (i = 0; i < arrsize(s_asyncHandlers); ++i) {
        if (!s_asyncHandlers[i]) {
            s_asyncHandlers[i] = callback;
            break;
        }
    }
    s_critsect.Leave();

    #ifdef HS_DEBUGGING
    if (i >= arrsize(s_asyncHandlers))
        FATAL("Maximum number of log handlers exceeded.");
    #endif
}

//===========================================================================
void LogUnregisterHandler (FLogHandler callback) {
    s_critsect.Enter();
    for (unsigned i = 0; i < arrsize(s_asyncHandlers); ++i) {
        if (s_asyncHandlers[i] == callback) {
            s_asyncHandlers[i] = nil;
            break;
        }
    }
    s_critsect.Leave();
}

//===========================================================================
void __cdecl LogMsg (ELogSeverity severity, const char format[], ...) {
    ASSERT(format);

    va_list args;
    va_start(args, format);
    LogMsgV(severity, format, args);
    va_end(args);
}

//===========================================================================
void __cdecl LogMsg (ELogSeverity severity, const wchar format[], ...) {
    ASSERT(format);

    va_list args;
    va_start(args, format);
    LogMsgV(severity, format, args);
    va_end(args);
}

//===========================================================================
void LogMsgV (ELogSeverity severity, const char format[], va_list args) {
    ASSERT(format);

    char msg[1024];
    StrPrintfV(msg, arrsize(msg), format, args);

    wchar uniMsg[1024];
    StrToUnicode(uniMsg, msg, arrsize(uniMsg));

    Dispatch(severity, uniMsg);
}

//===========================================================================
void LogMsgV (ELogSeverity severity, const wchar format[], va_list args) {
    ASSERT(format);
    ASSERT(args);

    wchar msg[1024];
    StrPrintfV(msg, arrsize(msg), format, args);

    Dispatch(severity, msg);
}

//============================================================================
#ifdef HS_DEBUGGING
void LogMsgDebug (const char  format[], ...) {
    ASSERT(format);

    va_list args;
    va_start(args, format);
    LogMsgV(kLogDebug, format, args);
    va_end(args);
}
#endif

//============================================================================
#ifdef HS_DEBUGGING
void LogMsgDebug (const wchar format[], ...) {
    ASSERT(format);

    va_list args;
    va_start(args, format);
    LogMsgV(kLogDebug, format, args);
    va_end(args);
}
#endif
