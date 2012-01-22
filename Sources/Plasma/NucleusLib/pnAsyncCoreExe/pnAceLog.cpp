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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCoreExe/pnAceLog.cpp
*   
***/

#include "Pch.h"
#pragma hdrstop

#if defined(PLASMA_EXTERNAL_RELEASE)
    // If this is an external build then don't write log files
    #define ACELOG_NO_LOG_FILES
#endif


namespace AsyncLog {

/****************************************************************************
*
*   Private
*
***/

static const unsigned kLogFlushMs = 10 * 1000;

enum ELogType {
#ifdef SERVER
    kLogTypeDebug,
    kLogTypePerf,
    kLogTypeError,
#else
    kLogTypeDebug,
#endif
    kNumLogTypes
};

static bool         s_breakOnErrors;
static wchar_t        s_directory[MAX_PATH];
static CCritSect    s_logCrit[kNumLogTypes];
static char *       s_logBuf[kNumLogTypes];
static unsigned     s_logPos[kNumLogTypes];
static uint64_t        s_logWritePos[kNumLogTypes];
static TimeDesc     s_logTime[kNumLogTypes];
static unsigned     s_logWriteMs[kNumLogTypes];
static AsyncFile    s_logFile[kNumLogTypes];
static long         s_opsPending;
static bool         s_running;
static AsyncTimer * s_timer;

static unsigned s_logSize[kNumLogTypes] = {
#ifdef SERVER
    64 * 1024,
    64 * 1024,
     8 * 1024,
#else
    64 * 1024,
#endif
};

static const wchar_t * s_logNameFmt[kNumLogTypes] = {
#ifdef SERVER
    L"Dbg%02u%02u%02u.log",
    L"Inf%02u%02u%02u.log",
    L"Err%02u%02u%02u.log",
#else
    L"%s%02u%02u%02u.log",
#endif
};

static ELogType s_logSeverityToType[kNumLogSeverity] = {
#ifdef SERVER
    kLogTypeDebug,      // kLogDebug
    kLogTypePerf,       // kLogPerf
    kLogTypeError,      // kLogError
    kLogTypeError,      // kLogFatal
#else
    kLogTypeDebug,      // kLogDebug
    kLogTypeDebug,      // kLogPerf
    kLogTypeDebug,      // kLogError
    kLogTypeDebug,      // kLogFatal
#endif
};

static char * s_logSeverityToText[kNumLogSeverity] = {
    "Debug",
    "Info",
    "Error",
    "Fatal",
};


/****************************************************************************
*
*   Local functions
*
***/

//============================================================================
static void LogFileNotifyProc (
    AsyncFile           file,
    EAsyncNotifyFile    code,
    AsyncNotifyFile *   notify,
    void **             userState
) {
    switch (code) {
        case kNotifyFileWrite:
            free(notify->param);
            AtomicAdd(&s_opsPending, -1);
        break;

        case kNotifyFileFlush:
            AsyncFileClose(file, kAsyncFileDontTruncate);
            AtomicAdd(&s_opsPending, -1);
        break;

        DEFAULT_FATAL(code);
    }
}

//============================================================================
static void AllocLogBuffer_CS (unsigned index) {
    ASSERT(!s_logBuf[index]);
    s_logBuf[index] = (char *)malloc(s_logSize[index]);
    s_logPos[index] = 0;

    if (!s_logBuf[index])
        ErrorAssert(__LINE__, __FILE__, "Out of memory");
}

//============================================================================
static void FreeLogBuffer_CS (unsigned index) {
    if (s_logBuf[index]) {
        free(s_logBuf[index]);
        s_logBuf[index] = nil;
    }
}

//============================================================================
static void GetLogFilename (
    unsigned    index,
    TimeDesc    timeDesc,
    wchar_t *     filename,
    unsigned    chars
) {
    StrPrintf(
        filename,
        chars,
        s_logNameFmt[index],
#ifndef SERVER
        ProductShortName(),
#endif
        timeDesc.year % 100,
        timeDesc.month,
        timeDesc.day
    );
    PathAddFilename(filename, s_directory, filename, chars);
}

//============================================================================
static bool OpenLogFile_CS (unsigned index) {
    if (s_logFile[index] != nil)
        return true;
        
    // Build filename
    wchar_t filename[MAX_PATH];
    GetLogFilename(
        index,
        s_logTime[index],
        filename,
        arrsize(filename)
    );
    
    // Open file
    uint64_t       fileTime;
    EFileError  fileError;
    bool fileExist = PathDoesFileExist(filename);
    s_logFile[index] = AsyncFileOpen(
        filename,
        LogFileNotifyProc,
        &fileError,
        kAsyncFileWriteAccess,
        kAsyncFileModeOpenAlways,
        kAsyncFileShareRead,
        nil,        // userState
        &s_logWritePos[index],
        &fileTime
    );

    if (s_logFile[index] == nil)
        return false;

    TimeGetDesc(fileTime, &s_logTime[index]);
    s_logWriteMs[index] = TimeGetMs();
        
    // Seek to end of file
    AsyncFileSeek(s_logFile[index], s_logWritePos[index], kFileSeekFromBegin);

    // If this is a new file, write uint8_t Order Mark
    if (!fileExist) {
        static const char s_bom[] = "\xEF\xBB\xBF";
        AsyncFileWrite(
            s_logFile[index],
            s_logWritePos[index],
            s_bom,
            arrsize(s_bom)- 1,
            kAsyncFileRwSync,   // perform blocking write
            nil                 // param
        );
        s_logWritePos[index] += arrsize(s_bom) - 1;
    }
    
    // Write a sentinel in case there are multiple runs in one day
    static const char s_logOpened[] = "Log Opened\r\n";
    AsyncFileWrite(
        s_logFile[index],
        s_logWritePos[index],
        s_logOpened,
        arrsize(s_logOpened)- 1,
        kAsyncFileRwSync,   // perform blocking write
        nil
    );
    s_logWritePos[index] += arrsize(s_logOpened) - 1;
    
    return true;
}

//============================================================================
static void WriteLogFile_CS (unsigned index, bool close) {
    unsigned flags = kAsyncFileRwSync;  // kAsyncFileRwNotify
    if (s_logPos[index]) {
        if (OpenLogFile_CS(index)) {
            AsyncFileWrite(
                s_logFile[index],
                s_logWritePos[index],
                s_logBuf[index],
                s_logPos[index],
                flags,
                s_logBuf[index]
            );
            if (flags == kAsyncFileRwSync)
                delete s_logBuf[index];
            else
                AtomicAdd(&s_opsPending, 1);
            s_logWritePos[index] += s_logPos[index];
            s_logWriteMs[index]  = TimeGetMs();
            s_logBuf[index]      = nil;
            s_logPos[index]      = 0;
        }
    }

    if (close && s_logFile[index]) {
        if (flags == kAsyncFileRwNotify) {
            AtomicAdd(&s_opsPending, 1);
            AsyncFileFlushBuffers(
                s_logFile[index],
                kAsyncFileDontTruncate,
                true,
                nil
            );
        }
        else {
            AsyncFileClose(
                s_logFile[index],
                kAsyncFileDontTruncate
            );
        }
        s_logFile[index] = nil;
    }
}

//============================================================================
static void FlushLogFile_CS (
    unsigned index,
    TimeDesc timeDesc
) {
    bool close = !s_running || (s_logTime[index].day != timeDesc.day);
    WriteLogFile_CS(index, close);
    if (close)
        s_logTime[index] = timeDesc;
}

//============================================================================
static unsigned FlushLogsTimerCallback (void *) {
    AsyncLogFlush();
    return kAsyncTimeInfinite;
}


} using namespace AsyncLog;


/****************************************************************************
*
*   Exported functions
*
***/

//============================================================================
void AsyncLogInitialize (
    const wchar_t logDirName[],
    bool        breakOnErrors
) {
    s_running = true;

    // Save options
    s_breakOnErrors = breakOnErrors;

    // Build log directory name
#ifdef SERVER
    PathGetProgramDirectory(s_directory, arrsize(s_directory));
#else
    PathGetUserDirectory(s_directory, arrsize(s_directory));
#endif
    PathAddFilename(s_directory, s_directory, logDirName, arrsize(s_directory));

#ifndef ACELOG_NO_LOG_FILES
    // Create log directory
    if (kPathCreateDirSuccess != PathCreateDirectory(s_directory, 0))
        PathRemoveFilename(s_directory, s_directory, arrsize(s_directory));
        
    // Allocate log buffers
    for (unsigned index = 0; index < kNumLogTypes; ++index) {
        s_logCrit[index].Enter();
        AllocLogBuffer_CS(index);
        s_logCrit[index].Leave();
    }

    AsyncTimerCreate(&s_timer, FlushLogsTimerCallback, kAsyncTimeInfinite, nil);
#endif // ndef ACELOG_NO_LOG_FILES
}

//============================================================================
void AsyncLogDestroy () {
    s_running = false;

#ifndef ACELOG_NO_LOG_FILES
    AsyncTimerDelete(s_timer, kAsyncTimerDestroyWaitComplete);

    for (unsigned index = 0; index < kNumLogTypes; ++index) {
        s_logCrit[index].Enter();
        {
            WriteLogFile_CS(index, true);
            FreeLogBuffer_CS(index);
        }
        s_logCrit[index].Leave();
    }
    while (s_opsPending)
        AsyncSleep(10);
#endif // ndef ACELOG_NO_LOG_FILES
}

//============================================================================
void AsyncLogFlush () {
#ifndef ACELOG_NO_LOG_FILES
    TimeDesc timeDesc;
    TimeGetDesc(TimeGetTime(), &timeDesc);
    
    for (unsigned index = 0; index < kNumLogTypes; ++index) {
        s_logCrit[index].Enter();
        FlushLogFile_CS(index, timeDesc);
        s_logCrit[index].Leave();
    }
#endif // ndef ACELOG_NO_LOG_FILES
}

//============================================================================
void LogBreakOnErrors (bool breakOnErrors) {
    s_breakOnErrors = breakOnErrors;
}

//============================================================================
void AsyncLogWriteMsg (
    const wchar_t     facility[],
    ELogSeverity    severity,
    const wchar_t     msg[]
) {
    if (!s_running)
        return;

#ifndef ACELOG_NO_LOG_FILES
    TimeDesc timeDesc;
    TimeGetDesc(TimeGetTime(), &timeDesc);
    
    char buffer[2048];
    const unsigned chars = StrPrintf(
        buffer,
        arrsize(buffer),
        "%02u/%02u/%02u % 2u:%02u:%02u [%S] %s %S\r\n",
        timeDesc.month,
        timeDesc.day,
        timeDesc.year % 100,
        timeDesc.hour,
        timeDesc.minute,
        timeDesc.second,
        facility,
        s_logSeverityToText[severity],
        msg
    );
    
    unsigned index = s_logSeverityToType[severity];
    s_logCrit[index].Enter();
    {
        // If day changed then write and flush file
        if (s_logTime[index].day != timeDesc.day)
            FlushLogFile_CS(index, timeDesc);
        // Otherwise if the buffer is full then write to file
        else if (s_logPos[index] + chars > s_logSize[index])
            WriteLogFile_CS(index, false);
            
        // Allocate log buffer if necessary
        if (!s_logBuf[index])
            AllocLogBuffer_CS(index);

        // Add new data to the log buffer
        memcpy(s_logBuf[index] + s_logPos[index], buffer, chars);
        s_logPos[index] += chars;

        // Write, flush and close file immediately if this is a fatal error        
        if (severity == kLogFatal)
            WriteLogFile_CS(index, true);

        // Drop to debugger if this is an error msg and that option was specified
        if (s_breakOnErrors && severity >= kLogError)
            DebugBreakIfDebuggerPresent();
    }
    s_logCrit[index].Leave();

    // Queue flush    
    AsyncTimerUpdate(s_timer, kLogFlushMs, kAsyncTimerUpdateSetPriorityHigher);
#endif // ndef ACELOG_NO_LOG_FILES
}

//============================================================================
void AsyncLogGetDirectory (wchar_t * dest, unsigned destChars) {
    ASSERT(dest);
    StrCopy(dest, s_directory, destChars);
}
