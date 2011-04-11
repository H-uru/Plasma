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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCoreExe/Private/Nt/pnAceNtInt.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNASYNCCOREEXE_PRIVATE_NT_PNACENTINT_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCoreExe/Private/Nt/pnAceNtInt.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNASYNCCOREEXE_PRIVATE_NT_PNACENTINT_H


namespace Nt {

/****************************************************************************
*
*   Type definitions
*
***/

enum EIoType {
    kNtFile,
    kNtSocket,
    kIoTypes
};

enum EOpType {
    // Completed by GetQueuedCompletionStatus
    kOpConnAttempt,
    kOpSocketRead,
    kOpSocketWrite,
    kOpFileRead,
    kOpFileWrite,
    
    // opType >= kOpSequence complete when they reach the head of the list
    kOpSequence,
    kOpFileFlush,
    kOpQueuedFileRead,
    kOpQueuedFileWrite,
    kOpQueuedSocketWrite,
    kNumOpTypes
};

class CNtCritSect : public CCritSect {
public:
    BOOL TryEnter () { return TryEnterCriticalSection(&m_handle); }
};

class CNtWaitHandle {
    long    m_refCount;
    HANDLE  m_event;

public:
    CNtWaitHandle ();
    ~CNtWaitHandle ();
    void IncRef ();
    void DecRef ();
    bool WaitForObject (unsigned timeMs) const;
    void SignalObject () const;
};

struct Operation {
    OVERLAPPED      overlapped;
    EOpType         opType;
    AsyncId         asyncId;
    bool            notify;
    unsigned        pending;
    CNtWaitHandle * signalComplete;
    LINK(Operation) link;
    
    #ifdef HS_DEBUGGING
    ~Operation () {
        ASSERT(!signalComplete);
    }
    #endif    
};

struct NtObject {
    CNtCritSect                 critsect;
    EIoType                     ioType;
    HANDLE                      handle;
    void *                      userState;
    LISTDECL(Operation, link)   opList;
    long                        nextCompleteSequence;
    long                        nextStartSequence;
    long                        ioCount;
    bool                        closed;
};


/****************************************************************************
*
*   Nt.cpp internal functions
*
***/

void INtWakeupMainIoThreads ();
void INtConnPostOperation (NtObject * ntObj, Operation * op, unsigned bytes);
AsyncId INtConnSequenceStart (NtObject * ntObj);
bool INtConnInitialize (NtObject * ntObj);
void INtConnCompleteOperation (NtObject * ntObj);


/*****************************************************************************
*
*   NtFile.cpp internal functions
*
***/

struct NtFile;
struct NtOpFileFlush;
struct NtOpFileReadWrite;
struct NtOpFileSequence;

void INtFileInitialize ();
void INtFileStartCleanup ();
void INtFileDestroy ();

void INtFileDelete (
    NtFile * file
);

bool INtFileOpCompleteReadWrite (
    NtFile *            ioConn,
    NtOpFileReadWrite * op,
    unsigned            bytes
);
void INtFileOpCompleteQueuedReadWrite (
    NtFile *            ioConn,
    NtOpFileReadWrite * op
);
void INtFileOpCompleteFileFlush (
    NtFile *        ioConn,
    NtOpFileFlush * op
);
void INtFileOpCompleteSequence (
    NtFile *            ioConn,
    NtOpFileSequence *  op
);

void INtFileStartCleanup ();


/*****************************************************************************
*
*   NtSocket.cpp internal functions
*
***/

struct NtSock;
struct NtOpConnAttempt;
struct NtOpSocketWrite;

void INtSocketInitialize ();
void INtSocketStartCleanup (unsigned exitThreadWaitMs);
void INtSocketDestroy ();

void INtSockDelete (
    NtSock * sock
);

void INtSocketOpCompleteSocketConnect (
    NtOpConnAttempt * op
);
void INtSocketOpCompleteSocketRead (
    NtSock *    sock,
    unsigned    bytes
);
void INtSocketOpCompleteSocketWrite (
    NtSock *            sock, 
    NtOpSocketWrite *   op
);
bool INtSocketOpCompleteQueuedSocketWrite (
    NtSock *            sock, 
    NtOpSocketWrite *   op
);


/*****************************************************************************
*
*   NT Async API functions
*
***/

void NtInitialize ();
void NtDestroy (unsigned exitThreadWaitMs);
void NtSignalShutdown ();
void NtWaitForShutdown ();
void NtSleep (unsigned sleepMs);
AsyncFile NtFileOpen (
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
void NtFileClose (
    AsyncFile   file,
    qword       truncateSize
);
void NtFileSetLastWriteTime (
    AsyncFile   file,
    qword       lastWriteTime
);
qword NtFileGetLastWriteTime (
    const wchar fileName[]
);
AsyncId NtFileFlushBuffers (
    AsyncFile   file, 
    qword       truncateSize,
    bool        notify,
    void *      param
);
AsyncId NtFileRead (
    AsyncFile   file,
    qword       offset,
    void *      buffer,
    unsigned    bytes,
    unsigned    flags,
    void *      param
);
AsyncId NtFileWrite (
    AsyncFile   file,
    qword       offset,
    const void *buffer,
    unsigned    bytes,
    unsigned    flags,
    void *      param
);
AsyncId NtFileCreateSequence (
    AsyncFile   file, 
    bool        notify, 
    void *      param
);
bool NtFileSeek (
    AsyncFile       file,
    qword           distance,
    EFileSeekFrom   from
);
void NtSocketConnect (
    AsyncCancelId *         cancelId,
    const NetAddress &      netAddr,
    FAsyncNotifySocketProc  notifyProc,
    void *                  param,
    const void *            sendData,
    unsigned                sendBytes,
    unsigned                connectMs,
    unsigned                localPort
);
void NtSocketConnectCancel (
    FAsyncNotifySocketProc  notifyProc,
    AsyncCancelId           cancelId
);
void NtSocketDisconnect (
    AsyncSocket     sock,
    bool            hardClose
);
void NtSocketDelete (AsyncSocket sock);
bool NtSocketSend (
    AsyncSocket     sock,
    const void *    data,
    unsigned        bytes
);
bool NtSocketWrite (
    AsyncSocket     sock,
    const void *    buffer,
    unsigned        bytes,
    void *          param
);
void NtSocketSetNotifyProc (
    AsyncSocket             sock,
    FAsyncNotifySocketProc  notifyProc
);
void NtSocketSetBacklogAlloc (
    AsyncSocket     sock,
    unsigned        bufferSize
);
unsigned NtSocketStartListening (
    const NetAddress &      listenAddr,
    FAsyncNotifySocketProc  notifyProc
);
void NtSocketStopListening (
    const NetAddress &      listenAddr,
    FAsyncNotifySocketProc  notifyProc
);
void NtSocketEnableNagling (
    AsyncSocket             conn,
    bool                    enable
);

}   // namespace Nt
