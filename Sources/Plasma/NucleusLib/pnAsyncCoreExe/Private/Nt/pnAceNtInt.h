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

    Operation()
        : opType((EOpType)0), asyncId(nil), notify(false), pending(0),
          signalComplete(nil)
    {
        memset(&overlapped, 0, sizeof(overlapped));
    }

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

    NtObject()
        : ioType((EIoType)0), handle(nil), userState(nil),
          nextCompleteSequence(0), nextStartSequence(0),
          ioCount(0), closed(false) { }
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
void NtSocketConnect (
    AsyncCancelId *         cancelId,
    const plNetAddress&     netAddr,
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
    const plNetAddress&     listenAddr,
    FAsyncNotifySocketProc  notifyProc
);
void NtSocketStopListening (
    const plNetAddress&     listenAddr,
    FAsyncNotifySocketProc  notifyProc
);
void NtSocketEnableNagling (
    AsyncSocket             conn,
    bool                    enable
);

}   // namespace Nt
