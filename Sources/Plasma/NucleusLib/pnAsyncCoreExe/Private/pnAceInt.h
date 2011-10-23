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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCoreExe/Private/pnAceInt.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNASYNCCOREEXE_PRIVATE_PNACEINT_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCoreExe/Private/pnAceInt.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNASYNCCOREEXE_PRIVATE_PNACEINT_H


/*****************************************************************************
*
*   Core.cpp
*
***/

// Performance counter functions
long PerfAddCounter (unsigned id, unsigned n);
long PerfSubCounter (unsigned id, unsigned n);
long PerfSetCounter (unsigned id, unsigned n);


/*****************************************************************************
*
*   Dns.cpp
*
***/

void DnsDestroy (unsigned exitThreadWaitMs);


/*****************************************************************************
*
*   Thread.cpp
*
***/

void ThreadDestroy (unsigned exitThreadWaitMs);


/*****************************************************************************
*
*   Timer.cpp
*
***/

void TimerDestroy (unsigned exitThreadWaitMs);


/****************************************************************************
*
*   Async API function types
*
***/

// Core
typedef void (* FInitialize) ();
typedef void (* FDestroy) (unsigned exitThreadWaitMs);
typedef void (* FSignalShutdown) ();
typedef void (* FWaitForShutdown) ();
typedef void (* FSleep) (unsigned sleepMs);

// Files
typedef AsyncFile (* FAsyncFileOpen) (
    const wchar             fullPath[],
    FAsyncNotifyFileProc    notifyProc,
    EFileError *            error,
    unsigned                desiredAccess,
    unsigned                openMode,
    unsigned                shareModeFlags,
    void *                  userState,
    qword *                 fileSize,
    qword *                 fileLastWriteTime
);

typedef void (* FAsyncFileClose) (
    AsyncFile   file,
    qword       truncateSize
);

typedef void (* FAsyncFileSetLastWriteTime) (
    AsyncFile   file,
    qword       lastWriteTime
);

typedef qword (* FAsyncFileGetLastWriteTime) (
    const wchar fileName[]
);

typedef AsyncId (* FAsyncFileFlushBuffers) (
    AsyncFile   file, 
    qword       truncateSize,
    bool        notify,
    void *      param
);

typedef AsyncId (* FAsyncFileRead) (
    AsyncFile   file,
    qword       offset,
    void *      buffer,
    unsigned    bytes,
    unsigned    flags,
    void *      param
);

typedef AsyncId (* FAsyncFileWrite) (
    AsyncFile       file,
    qword           offset,
    const void *    buffer,
    unsigned        bytes,
    unsigned        flags,
    void *          param
);

typedef AsyncId (* FAsyncFileCreateSequence) (
    AsyncFile   file, 
    bool        notify, 
    void *      param
);

typedef bool (* FAsyncFileSeek) (
    AsyncFile       file,
    qword           distance,
    EFileSeekFrom   from
);

typedef bool (* FAsyncFileWaitId) (
    AsyncFile   file, 
    AsyncId     asyncId, 
    unsigned    timeoutMs
);

// Sockets
typedef void (* FAsyncSocketConnect) (
    AsyncCancelId *         cancelId,
    const NetAddress &      netAddr,
    FAsyncNotifySocketProc  notifyProc,
    void *                  param,
    const void *            sendData,
    unsigned                sendBytes,
    unsigned                connectMs,
    unsigned                localPort
);

typedef void (* FAsyncSocketConnectCancel) (
    FAsyncNotifySocketProc  notifyProc,
    AsyncCancelId           cancelId
);

typedef void (* FAsyncSocketDisconnect) (
    AsyncSocket     sock,
    bool            hardClose
);

typedef void (* FAsyncSocketDelete) (AsyncSocket sock);

typedef bool (* FAsyncSocketSend) (
    AsyncSocket     sock,
    const void *    data,
    unsigned        bytes
);

typedef bool (* FAsyncSocketWrite) (
    AsyncSocket     sock,
    const void *    buffer,
    unsigned        bytes,
    void *          param
);

typedef void (* FAsyncSocketSetNotifyProc) (
    AsyncSocket             sock,
    FAsyncNotifySocketProc  notifyProc
);

typedef void (* FAsyncSocketSetBacklogAlloc) (
    AsyncSocket             sock,
    unsigned                bufferSize
);

typedef unsigned (* FAsyncSocketStartListening) (
    const NetAddress &      listenAddr,
    FAsyncNotifySocketProc  notifyProc
);

typedef void (* FAsyncSocketStopListening) (
    const NetAddress &      listenAddr,
    FAsyncNotifySocketProc  notifyProc
);

typedef void (* FAsyncSocketEnableNagling) (
    AsyncSocket             conn,
    bool                    enable
);



/****************************************************************************
*
*   I/O API
*
***/

struct AsyncApi {

    // Init
    FInitialize                     initialize;
    FDestroy                        destroy;
    FSignalShutdown                 signalShutdown;
    FWaitForShutdown                waitForShutdown;
    FSleep                          sleep;
    
    // Files
    FAsyncFileOpen                  fileOpen;
    FAsyncFileClose                 fileClose;
    FAsyncFileRead                  fileRead;
    FAsyncFileWrite                 fileWrite;
    FAsyncFileFlushBuffers          fileFlushBuffers;
    FAsyncFileSetLastWriteTime      fileSetLastWriteTime;
    FAsyncFileGetLastWriteTime      fileGetLastWriteTime;
    FAsyncFileCreateSequence        fileCreateSequence;
    FAsyncFileSeek                  fileSeek;
    
    // Sockets
    FAsyncSocketConnect             socketConnect;
    FAsyncSocketConnectCancel       socketConnectCancel;
    FAsyncSocketDisconnect          socketDisconnect;
    FAsyncSocketDelete              socketDelete;
    FAsyncSocketSend                socketSend;
    FAsyncSocketWrite               socketWrite;
    FAsyncSocketSetNotifyProc       socketSetNotifyProc;
    FAsyncSocketSetBacklogAlloc     socketSetBacklogAlloc;
    FAsyncSocketStartListening      socketStartListening;
    FAsyncSocketStopListening       socketStopListening;
    FAsyncSocketEnableNagling       socketEnableNagling;
};

extern AsyncApi g_api;
