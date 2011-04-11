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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCore/Private/pnAcLog.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNASYNCCORE_PRIVATE_PNACLOG_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCore/Private/pnAcLog.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNASYNCCORE_PRIVATE_PNACLOG_H


/****************************************************************************
*
*   Log API
*
***/

enum ELogSeverity {
    // For indicating design problems
    kLogDebug,
    
    // For indicating performance warnings
    // (e.g. transaction failed, retrying...)
    kLogPerf,
    
    // For indicating error conditions that change program behavior
    // (e.g. socket connect failed)
    kLogError,
    
    // For indicating failures that may lead to program termination
    // (e.g. out of memory)
    kLogFatal,
    
    kNumLogSeverity
};

void LogMsg  (ELogSeverity severity, const char  format[], ...);
void LogMsg  (ELogSeverity severity, const wchar format[], ...);
void LogMsgV (ELogSeverity severity, const char  format[], va_list args);
void LogMsgV (ELogSeverity severity, const wchar format[], va_list args);

void LogBreakOnErrors (bool breakOnErrors);

void AsyncLogInitialize (
    const wchar logDirName[],
    bool        breakOnErrors
);
void AsyncLogDestroy ();
void AsyncLogFlush ();

void AsyncLogGetDirectory (wchar * dest, unsigned destChars);


// Low(er) level log API; call this from your LogHander function
// if you want to use the asynchronous log facility.
void AsyncLogWriteMsg (
    const wchar     facility[],
    ELogSeverity    severity,
    const wchar     msg[]
);

// FLogHandler must be capable of handling multiple threads and re-entrancy
typedef void (* FLogHandler) (ELogSeverity severity, const wchar msg[]);

void LogRegisterHandler   (FLogHandler callback);
void LogUnregisterHandler (FLogHandler callback);


/****************************************************************************
*
*   Debugging API
*
***/

#ifdef HS_DEBUGGING

    void LogMsgDebug (const char  format[], ...);
    void LogMsgDebug (const wchar format[], ...);

#else

    inline void LogMsgDebug (const char  *, ...) { }
    inline void LogMsgDebug (const wchar *, ...) { }

#endif
