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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCoreExe/Private/W9x/pnAceW9xInt.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNASYNCCOREEXE_PRIVATE_W9X_PNACEW9XINT_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCoreExe/Private/W9x/pnAceW9xInt.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNASYNCCOREEXE_PRIVATE_W9X_PNACEW9XINT_H


namespace W9x {

/*****************************************************************************
*
*   Internal types
*
***/

class CThreadDispObject : public AtomicRef {
public:
    CThreadDispObject ();
    virtual ~CThreadDispObject () { }
    void Close ();
    virtual void Complete (void * op, CCritSect * critSect, AsyncId asyncId) = 0;
    virtual void Delete (void * op) = 0;
    AsyncId Queue (void * op);
};


/*****************************************************************************
*
*   W9x internal async API
*
***/

void W9xThreadInitialize ();
void W9xThreadDestroy (unsigned exitThreadWaitMs);
void W9xThreadSignalShutdown ();
void W9xThreadWaitForShutdown ();
void W9xThreadSleep (unsigned sleepMs);
bool W9xThreadWaitId (
    AsyncFile   file, 
    AsyncId     asyncId, 
    unsigned    timeoutMs
);

AsyncFile W9xFileOpen (
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
void W9xFileClose (
    AsyncFile   file,
    qword       truncateSize
);
void W9xFileSetLastWriteTime (
    AsyncFile   file,
    qword       lastWriteTime
);
qword W9xFileGetLastWriteTime (
    const wchar fileName[]
);
AsyncId W9xFileFlushBuffers (
    AsyncFile   file, 
    qword       truncateSize,
    bool        notify,
    void *      param
);
AsyncId W9xFileRead (
    AsyncFile   file,
    qword       offset,
    void *      buffer,
    unsigned    bytes,
    unsigned    flags,
    void *      param
);
AsyncId W9xFileWrite (
    AsyncFile   file,
    qword       offset,
    const void *buffer,
    unsigned    bytes,
    unsigned    flags,
    void *      param
);
AsyncId W9xFileCreateSequence (
    AsyncFile   file, 
    bool        notify, 
    void *      param
);
bool W9xFileSeek (
    AsyncFile       file,
    qword           distance,
    EFileSeekFrom   from
);
void W9xSocketConnect (
    AsyncCancelId *         cancelId,
    const NetAddress &      netAddr,
    FAsyncNotifySocketProc  notifyProc,
    void *                  param,
    const void *            sendData,
    unsigned                sendBytes,
    unsigned                connectMs,
    unsigned                localPort
);
void W9xSocketConnectCancel (
    FAsyncNotifySocketProc  notifyProc,
    AsyncCancelId           cancelId
);
void W9xSocketDisconnect (
    AsyncSocket     sock,
    bool            hardClose
);
void W9xSocketDelete (AsyncSocket sock);
void W9xSocketDestroy ();
bool W9xSocketSend (
    AsyncSocket     sock,
    const void *    data,
    unsigned        bytes
);
bool W9xSocketWrite (
    AsyncSocket     sock,
    const void *    buffer,
    unsigned        bytes,
    void *          param
);
void W9xSocketSetNotifyProc (
    AsyncSocket             sock,
    FAsyncNotifySocketProc  notifyProc
);
void W9xSocketSetBacklogAlloc (
    AsyncSocket     sock,
    unsigned        bufferSize
);
unsigned W9xSocketStartListening (
    const NetAddress &      listenAddr,
    FAsyncNotifySocketProc  notifyProc
);
void W9xSocketStopListening (
    const NetAddress &      listenAddr,
    FAsyncNotifySocketProc  notifyProc
);
void W9xSocketEnableNagling (
    AsyncSocket             conn,
    bool                    enable
);


}   // namespace W9x
