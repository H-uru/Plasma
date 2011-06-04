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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCoreExe/Private/Nt/pnAceNtSocket.cpp
*   
***/

#include "../../Pch.h"
#pragma hdrstop

#include "pnAceNtInt.h"


namespace Nt {

/****************************************************************************
*
*   Private
*
***/

// how long to wait for connect() to complete
static const unsigned   kConnectTimeMs      = 10*1000;

static const int        kTcpSndBufSize      = 64*1024-1;
static const int        kTcpRcvBufSize      = 64*1024-1;
static const int        kListenBacklog      = 400;

// wait before checking for backlog problems
static const unsigned   kBacklogInitMs      = 3*60*1000;

// destroy a connection if it has a backlog "problem"
static const unsigned   kBacklogFailMs      = 2*60*1000;

static const unsigned   kMinBacklogBytes    = 4 * 1024;

struct NtListener {
    LINK(NtListener)        nextPort;
    SOCKET                  hSocket;
    NetAddress              addr;
    FAsyncNotifySocketProc  notifyProc;
    int                     listenCount;

    ~NtListener () {
        if (hSocket != INVALID_SOCKET)
            closesocket(hSocket);
    }
};

struct NtOpConnAttempt : Operation {
    AsyncCancelId           cancelId;
    bool                    canceled;
    unsigned                localPort;
    NetAddress              remoteAddr;
    FAsyncNotifySocketProc  notifyProc;
    void *                  param;
    SOCKET                  hSocket;
    unsigned                failTimeMs;
    unsigned                sendBytes;
    byte                    sendData[1];    // actually [sendBytes]
    // no additional fields
};

struct NtOpSocketWrite : Operation {
    unsigned                queueTimeMs;
    unsigned                bytesAlloc;
    AsyncNotifySocketWrite  write;
};

struct NtOpSocketRead : Operation {
    AsyncNotifySocketRead   read;
};

struct NtSock : NtObject {
    LINK(NtSock)            link;
    NetAddress              addr;
    unsigned                closeTimeMs;
    unsigned                connType;
    FAsyncNotifySocketProc  notifyProc;
    unsigned                bytesLeft;
    NtOpSocketRead          opRead;
    unsigned                backlogAlloc;
    unsigned                initTimeMs;
    byte                    buffer[kAsyncSocketBufferSize];

    NtSock ();
    ~NtSock ();
};


static CNtCritSect                      s_listenCrit;
static LISTDECL(NtListener, nextPort)   s_listenList;
static LISTDECL(NtOpConnAttempt, link)  s_connectList;
static bool                             s_runListenThread;
static unsigned                         s_nextConnectCancelId = 1;
static HANDLE                           s_listenThread;
static HANDLE                           s_listenEvent;


const unsigned kCloseTimeoutMs = 8*1000;
static CNtCritSect                      s_socketCrit;
static AsyncTimer *                     s_socketTimer;
static LISTDECL(NtSock, link)           s_socketList;


//===========================================================================
inline NtSock::NtSock () {
    PerfAddCounter(kAsyncPerfSocketsCurr, 1);
    PerfAddCounter(kAsyncPerfSocketsTotal, 1);
}

//===========================================================================
NtSock::~NtSock () {
    // Make sure socket can only be deleted after receiving NOTIFY_DISCONNECT
    ASSERT(closed);

    // To avoid a race condition, the socket must be unlinked from
    // the soft disconnect list prior to closing the handle
    if (link.IsLinked()) {
        s_socketCrit.Enter();
        link.Unlink();
        s_socketCrit.Leave();
    }

    if (handle != INVALID_HANDLE_VALUE)
        closesocket((SOCKET) handle);

    PerfSubCounter(kAsyncPerfSocketsCurr, 1);
}

//===========================================================================
// must be called inside s_listenCrit
static bool ListenPortIncrement (
    const NetAddress &      listenAddr,
    FAsyncNotifySocketProc  notifyProc,
    int                     count
) {
    NtListener * listener;
    for (listener = s_listenList.Head(); listener; listener = s_listenList.Next(listener)) {
        if (!NetAddressEqual(listener->addr, listenAddr))
            continue;
        if (listener->notifyProc != notifyProc)
            continue;

        listener->listenCount += count;
        ASSERT(listener->listenCount >= 0);
        break;
    }
    return listener != 0;
}

//===========================================================================
static void SocketGetAddresses (
    NtSock *        sock,
    NetAddress *    localAddr,
    NetAddress *    remoteAddr
) {
    // NetAddress may be bigger than sockaddr_in so start by zeroing the whole thing
    ZEROPTR(localAddr);
    ZEROPTR(remoteAddr);

    // don't have to enter critsect or validate socket before referencing it
    // because this routine is called before the user has a chance to close it
    int nameLen = sizeof(*localAddr);
    if (getsockname((SOCKET) sock->handle, (sockaddr *) localAddr, &nameLen))
        LogMsg(kLogError, "getsockname failed");

    nameLen = sizeof(*remoteAddr);
    if (getpeername((SOCKET) sock->handle, (sockaddr *) remoteAddr, &nameLen))
        LogMsg(kLogError, "getpeername failed");
}

//===========================================================================
static void SocketStartAsyncRead (NtSock * sock) {
    // enter critical section in case someone attempts to close socket from another thread
    bool readResult;
    sock->critsect.Enter();
    if (sock->handle != INVALID_HANDLE_VALUE) {
        InterlockedIncrement(&sock->ioCount);
        readResult = ReadFile(
            sock->handle,
            sock->buffer + sock->bytesLeft,
            sizeof(sock->buffer) - sock->bytesLeft,
            0,
            &sock->opRead.overlapped
        );
    }
    else {
        readResult = true;
    }
    sock->critsect.Leave();

    DWORD err = GetLastError();
    if (!readResult && (err != ERROR_IO_PENDING))
        InterlockedDecrement(&sock->ioCount);
}

//===========================================================================
static bool SocketDispatchRead (NtSock * sock) {
//    LogMsg(kLogPerf, L"Nt sock %p recv %u bytes", sock, sock->opRead.read.bytes);

    // put "fast case" first -- connType already established
    if (sock->notifyProc)
        return sock->notifyProc((AsyncSocket) sock, kNotifySocketRead, &sock->opRead.read, &sock->userState);

    ASSERT(sock->opRead.read.buffer == sock->buffer);
    ASSERT(sock->opRead.read.bytes);

    // make sure there's an event procedure to handle this event
    AsyncNotifySocketListen notify;
    unsigned bytesProcessed;
    sock->notifyProc = AsyncSocketFindNotifyProc(
        sock->opRead.read.buffer,
        sock->opRead.read.bytes,
        &bytesProcessed,
        &notify.connType, 
        &notify.buildId,
        &notify.buildType,
        &notify.branchId,
        &notify.productId
    );
    if (!sock->notifyProc)
        return false;

    // perform kNotifySocketListenSuccess
    SocketGetAddresses(sock, &notify.localAddr, &notify.remoteAddr);
    notify.param            = nil;
    notify.asyncId          = 0;
    notify.addr             = sock->addr;
    sock->userState         = nil;
    sock->connType          = notify.connType;
    notify.buffer           = sock->opRead.read.buffer + bytesProcessed;
    notify.bytes            = sock->opRead.read.bytes - bytesProcessed;
    notify.bytesProcessed   = 0;
    if (!sock->notifyProc((AsyncSocket) sock, kNotifySocketListenSuccess, &notify, &sock->userState))
        return false;
    bytesProcessed += notify.bytesProcessed;

    // if we didn't use up all the bytes, dispatch a read operation
    if (0 != (sock->opRead.read.bytes -= bytesProcessed)) {
        sock->opRead.read.buffer += bytesProcessed;
        if (!sock->notifyProc((AsyncSocket) sock, kNotifySocketRead, &sock->opRead.read, &sock->userState))
            return false;
    }

    // add bytes used by IOsFindListenProc and kNotifySocketListenSuccess
    sock->opRead.read.bytesProcessed += bytesProcessed;
    return true;
}

//===========================================================================
static NtOpSocketWrite * SocketQueueAsyncWrite (
    NtSock *        sock,
    const byte *    data,
    unsigned        bytes
) {
    // check for data backlog
    Operation * opCurr;
    for (opCurr = sock->opList.Head(); opCurr; opCurr = sock->opList.Next(opCurr)) {
        if (opCurr->opType != kOpQueuedSocketWrite)
            continue;
        NtOpSocketWrite * firstQueuedWrite = (NtOpSocketWrite *) opCurr;

        unsigned currTimeMs = TimeGetMs();
        if (((long) (currTimeMs - firstQueuedWrite->queueTimeMs) >= (long) kBacklogFailMs)
        &&  ((long) (currTimeMs - sock->initTimeMs) >= (long) kBacklogInitMs)
        ) {
            PerfAddCounter(kAsyncPerfSocketDisconnectBacklog, 1);

            if (sock->connType) {
                LogMsg(
                    kLogPerf,
                    "Backlog, c:%u q:%u, i:%u",
                    sock->connType,
                    currTimeMs - firstQueuedWrite->queueTimeMs,
                    currTimeMs - sock->initTimeMs
                );
            }
            NtSocketDisconnect((AsyncSocket) sock, true);
            return nil;
        }

        break;
    }

    // if the last buffer still has space available then add data to it
    if (opCurr) {
        for (opCurr = sock->opList.Tail(); opCurr; opCurr = sock->opList.Prev(opCurr)) {
            if (opCurr->opType != kOpQueuedSocketWrite)
                continue;
            NtOpSocketWrite * lastQueuedWrite = (NtOpSocketWrite *) opCurr;

            unsigned bytesLeft = lastQueuedWrite->bytesAlloc - lastQueuedWrite->write.bytes;
            bytesLeft = min(bytesLeft, bytes);
            if (bytesLeft) {
                PerfAddCounter(kAsyncPerfSocketBytesWaitQueued, bytesLeft);
                MemCopy(lastQueuedWrite->write.buffer + lastQueuedWrite->write.bytes, data, bytesLeft);
                lastQueuedWrite->write.bytes += bytesLeft;
                data += bytesLeft;
                if (0 == (bytes -= bytesLeft))
                    return lastQueuedWrite;
            }
            break;
        }
    }

    // allocate a buffer large enough to hold the data, plus
    // extra space in case more data needs to be queued later
    unsigned bytesAlloc = max(bytes, sock->backlogAlloc);
    bytesAlloc          = max(bytesAlloc, kMinBacklogBytes);
    NtOpSocketWrite * op = new(ALLOC(sizeof(NtOpSocketWrite) + bytesAlloc)) NtOpSocketWrite;

    // init Operation
    const AsyncId asyncId       = INtConnSequenceStart(sock);
    op->overlapped.Offset       = 0;
    op->overlapped.OffsetHigh   = 0;
    op->overlapped.hEvent       = nil;
    op->opType                  = kOpQueuedSocketWrite;
    op->asyncId                 = asyncId;
    op->notify                  = false;
    op->pending                 = 1;
    op->signalComplete          = nil;
    sock->opList.Link(op, kListTail);

    // init OpWrite
    op->queueTimeMs             = TimeGetMs();
    op->bytesAlloc              = bytesAlloc;
    op->write.param             = nil;
    op->write.asyncId           = asyncId;
    op->write.buffer            = (byte *) (op + 1);
    op->write.bytes             = bytes;
    op->write.bytesProcessed    = bytes;
    MemCopy(op->write.buffer, data, bytes);

    InterlockedIncrement(&sock->ioCount);
    PerfAddCounter(kAsyncPerfSocketBytesWaitQueued, bytes);

    return op;
}

//===========================================================================
static NtSock * SocketInitCommon (SOCKET hSocket) {
    // make socket non-blocking
    u_long nonBlocking = true;
    if (ioctlsocket(hSocket, FIONBIO, &nonBlocking))
        LogMsg(kLogError, "ioctlsocket failed (make non-blocking)");

    // set socket buffer sizes
    int result = setsockopt(
        hSocket, 
        SOL_SOCKET, 
        SO_SNDBUF, 
        (const char *) &kTcpSndBufSize, 
        sizeof(kTcpSndBufSize)
    );
    if (result)
        LogMsg(kLogError, "setsockopt(send) failed (set send buffer size)");

    result = setsockopt(
        hSocket, 
        SOL_SOCKET, 
        SO_RCVBUF, 
        (const char *) &kTcpRcvBufSize, 
        sizeof(kTcpRcvBufSize)
    );
    if (result)
        LogMsg(kLogError, "setsockopt(recv) failed (set recv buffer size)");

    // allocate a new socket
    NtSock * sock       = NEWZERO(NtSock);
    sock->ioType        = kNtSocket;
    sock->handle        = (HANDLE) hSocket;
    sock->initTimeMs    = TimeGetMs();
    sock->ioCount       = 1;
    sock->opRead.opType = kOpSocketRead;

    return sock;
}

//===========================================================================
static bool SocketInitConnect (
    NtSock * const          sock,
    NtOpConnAttempt const & op
) {
    bool notified = false;
    for (;;) {
        // attach to I/O completion port
        if (!INtConnInitialize(sock))
            break;

        // send initial data
        if (op.sendBytes && !NtSocketSend((AsyncSocket) sock, op.sendData, op.sendBytes))
            break;

        // Determine connType
        for (;op.sendBytes;) {
            sock->connType = op.sendData[0];
            if (IS_TEXT_CONNTYPE(sock->connType))
                break;

            if (op.sendBytes < sizeof(AsyncSocketConnectPacket))
                return false;

            if (sock->connType != ((const AsyncSocketConnectPacket *) op.sendData)->connType)
                return false;

            break;
        }

        // perform callback notification
        notified = true;
        AsyncNotifySocketConnect notify;
        SocketGetAddresses(sock, &notify.localAddr, &notify.remoteAddr);
        notify.param        = op.param;
        notify.asyncId      = 0;
        notify.connType     = sock->connType;
        sock->notifyProc    = op.notifyProc;
        if (!sock->notifyProc((AsyncSocket) sock, kNotifySocketConnectSuccess, &notify, &sock->userState))
            break;

        // start reading from the socket
        SocketStartAsyncRead(sock);
        break;
    }

    INtConnCompleteOperation(sock);
    return notified;
}

//===========================================================================
static void SocketInitListen (
    NtSock * const      sock,
    const NetAddress &  listenAddr,
    FAsyncNotifySocketProc notifyProc
) {
    for (;;) {
        // attach to I/O completion port
        if (!INtConnInitialize(sock))
            break;

        sock->addr = listenAddr;

        if (notifyProc) {
            // perform kNotifySocketListenSuccess
            AsyncNotifySocketListen notify;
            SocketGetAddresses(sock, &notify.localAddr, &notify.remoteAddr);
            notify.param            = nil;
            notify.asyncId          = 0;
            notify.connType         = 0;
            notify.buildId          = 0;
            notify.buildType		= 0;
            notify.branchId			= 0;
            notify.productId        = 0;
            notify.addr             = listenAddr;
            notify.buffer           = sock->opRead.read.buffer;
            notify.bytes            = 0;
            notify.bytesProcessed   = 0;
            sock->notifyProc        = notifyProc;
            if (!sock->notifyProc((AsyncSocket) sock, kNotifySocketListenSuccess, &notify, &sock->userState))
                break;
        }

        // start reading from the socket
        SocketStartAsyncRead(sock);
        break;
    }

    INtConnCompleteOperation(sock);
}

//===========================================================================
static SOCKET ListenSocket (NetAddress * listenAddr) {
    // create a new socket to listen
    SOCKET s;
    if (INVALID_SOCKET == (s = socket(AF_INET, SOCK_STREAM, 0))) {
        LogMsg(kLogError, "socket create failed");
        return INVALID_SOCKET;
    }

    for (;;) { // actually for (ONCE)
    /* this code was an attempt to enable the server to close and then re-open the same port
       for listening. It doesn't appear to work, and moreover, it causes the bind to signal
       success even though the port cannot be opened (for example, because another application
       has already opened the port).
        static const BOOL s_reuseAddr = true;
        setsockopt(
            s, 
            SOL_SOCKET, 
            SO_REUSEADDR, 
            (const char *) &s_reuseAddr, 
            sizeof(s_reuseAddr)
        );
        static const LINGER s_linger = { true, 0 };
        setsockopt(
            s, 
            SOL_SOCKET, 
            SO_LINGER, 
            (const char *) &s_linger, 
            sizeof(s_linger)
        );
    */

        NetAddressNode  node = NetAddressGetNode(*listenAddr);
        unsigned        port = NetAddressGetPort(*listenAddr);

        // bind socket to port
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port   = htons((word)port);
        addr.sin_addr.S_un.S_addr = htonl(node);
        MemZero(addr.sin_zero, sizeof(addr.sin_zero));
        if (bind(s, (sockaddr *) &addr, sizeof(addr))) {
            wchar str[32];
            NetAddressToString(*listenAddr, str, arrsize(str), kNetAddressFormatAll);
            LogMsg(kLogError, "bind to addr %s failed (err %u)", str, WSAGetLastError());
            break;
        }

        // get portNumber if unknown
        if (!port) {
            int addrLen = sizeof(addr);
            if (getsockname(s, (sockaddr *) &addr, &addrLen)) {
                LogMsg(kLogError, "getsockname failed");
                break;
            }

            if (0 == (port = ntohs(((const sockaddr_in *) &addr)->sin_port))) {
                LogMsg(kLogError, "bad listen port");
                break;
            }
        }

        // make socket non-blocking
        u_long nonBlocking = true;
        if (ioctlsocket(s, FIONBIO, &nonBlocking))
            LogMsg(kLogError, "ioctlsocket failed (make non-blocking)");

        if (listen(s, kListenBacklog)) {
            LogMsg(kLogError, "socket listen failed");
            break;
        }
 
        // success!
        NetAddressSetPort(port, listenAddr);
        return s;
    }

    // failure!
    closesocket(s);
    NetAddressSetPort(0, listenAddr);
    return INVALID_SOCKET;
}

//===========================================================================
// must be called while inside s_listenCrit!
static void ListenPrepareListeners (fd_set * readfds) {
    FD_ZERO(readfds);
    for (NtListener *next, *port = s_listenList.Head(); port; port = next) {
        next = s_listenList.Next(port);

        // destroy unused ports
        if (!port->listenCount) {
            DEL(port);
            continue;
        }

        // add port to listen list
        ASSERT(port->hSocket != INVALID_SOCKET);
        #pragma warning(disable:4127) // ignore "do { } while (0)" warning
        FD_SET(port->hSocket, readfds);
        #pragma warning(default:4127)
    }
}

//===========================================================================
static SOCKET ConnectSocket (unsigned localPort, const NetAddress & addr) {
    SOCKET s;
    if (INVALID_SOCKET == (s = socket(AF_INET, SOCK_STREAM, 0))) {
        LogMsg(kLogError, "socket create failed");
        return INVALID_SOCKET;
    }

    for (;;) { // actually for (ONCE)
        // make socket non-blocking
        u_long nonBlocking = true;
        if (ioctlsocket(s, FIONBIO, &nonBlocking))
            LogMsg(kLogError, "ioctlsocket failed (make non-blocking)");

        // bind socket to port
        if (localPort) {
            sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port   = htons((word) localPort);
            addr.sin_addr.S_un.S_addr = INADDR_ANY;
            MemZero(addr.sin_zero, sizeof(addr.sin_zero));
            if (bind(s, (sockaddr *) &addr, sizeof(addr))) {
                LogMsg(kLogError, "bind(port %u) failed (%u)", localPort, WSAGetLastError());
                break;
            }
        }

        if (connect(s, (const sockaddr *) &addr, sizeof(addr))) {
            if (WSAGetLastError() != WSAEWOULDBLOCK) {
                LogMsg(kLogError, "sockegt connect failed");
                break;
            }
        }

        // success!
        return s;
    }

    // failure!
    closesocket(s);
    return INVALID_SOCKET;
}

//===========================================================================
static void ListenPrepareConnectors (fd_set * writefds) {
    const unsigned currTimeMs = TimeGetMs();

    FD_ZERO(writefds);
    for (NtOpConnAttempt *next, *op = s_connectList.Head(); op; op = next) {
        next = s_connectList.Next(op);

        // if the socket has taken too long to connect then abort attempt
        if (op->hSocket != INVALID_SOCKET) {
            if ((int) (currTimeMs - op->failTimeMs) > 0)
                op->canceled = true;
        }

        // if this connection attempt has been canceled then post
        // operation to worker threads to complete the callback
        if (op->canceled) {
            if (op->hSocket) {
                closesocket(op->hSocket);
                op->hSocket = INVALID_SOCKET;
            }
            s_connectList.Unlink(op);
            INtConnPostOperation(nil, op, 0);
            continue;
        }

        // open new sockets
        if (op->hSocket == INVALID_SOCKET) {
            if (INVALID_SOCKET == (op->hSocket = ConnectSocket(op->localPort, op->remoteAddr))) {
                s_connectList.Unlink(op);
                INtConnPostOperation(nil, op, 0);
                continue;
            }

            // start failure timer
            op->failTimeMs += currTimeMs;
        }

        // add socket to fdset
        #pragma warning(disable:4127) // ignore "do { } while (0)" warning
        FD_SET(op->hSocket, writefds);
        #pragma warning(default:4127)
    }
}

//===========================================================================
static unsigned THREADCALL ListenThreadProc (AsyncThread *) {
    fd_set readfds;
    fd_set writefds;
    for (;;) {
        s_listenCrit.Enter();
        ListenPrepareListeners(&readfds);
        ListenPrepareConnectors(&writefds);
        s_listenCrit.Leave();
        if (!s_runListenThread)
            break;

        // wait until there is something on listen or connect list
        if (!readfds.fd_count && !writefds.fd_count) {
            WaitForSingleObject(s_listenEvent, INFINITE);
            continue;
        }

        // wait for connection or timeout
        const struct timeval timeout = { 0, 250*1000 }; // seconds, microseconds
        int result = select(0, &readfds, &writefds, 0, &timeout);
        if (result == SOCKET_ERROR) {
            LogMsg(kLogError, "socket select failed");
            continue;
        }
        if (!result)
            continue;

        s_listenCrit.Enter();

        // complete listen() operations
        unsigned count = 0;
        for (NtListener * listener = s_listenList.Head(); listener; listener = s_listenList.Next(listener)) {
            if (FD_ISSET(listener->hSocket, &readfds)) {
                SOCKET s;
                while (INVALID_SOCKET != (s = accept(listener->hSocket, 0, 0))) {
                    SocketInitListen(
                        SocketInitCommon(s),
                        listener->addr,
                        listener->notifyProc
                    );
                    ++count;
                }
            }
        }
        PerfAddCounter(kAsyncPerfSocketConnAttemptsInTotal, count);

        // complete connect() operations
        for (NtOpConnAttempt *next, *op = s_connectList.Head(); op; op = next) {
            next = s_connectList.Next(op);

            if (FD_ISSET(op->hSocket, &writefds)) {
                s_connectList.Unlink(op);
                INtConnPostOperation(nil, op, 0);
            }
        }

        s_listenCrit.Leave();
    }

    // cleanup all connectors
    s_listenCrit.Enter();
    for (NtOpConnAttempt * op; (op = s_connectList.Head()) != nil; s_connectList.Unlink(op)) {
        if (op->hSocket != INVALID_SOCKET) {
            closesocket(op->hSocket);
            op->hSocket = nil;
        }
        INtConnPostOperation(nil, op, 0);
    }
    s_listenCrit.Leave();

    return 0;
}

//===========================================================================
static void StartListenThread () {
    if (s_listenThread)
        return;

    s_listenEvent = CreateEvent(
        (LPSECURITY_ATTRIBUTES) nil,
        false,  // auto-reset event
        false,  // initial state unsignaled
        (LPCTSTR) nil
    );
    ASSERT(s_listenEvent);

    // create a low-priority thread to listen on ports
    s_runListenThread = true;
    s_listenThread = (HANDLE) AsyncThreadCreate(
        ListenThreadProc,
        nil,
        L"NtListenThread"
    );
}

//===========================================================================
#ifdef HS_DEBUGGING
#include <StdIo.h>
static void __cdecl DumpInvalidData (
    const wchar filename[],
    unsigned    bytes,
    const byte  data[],
    const char  fmt[],
    ...
) {
    ref(filename);
    ref(bytes);
    ref(data);
    ref(fmt);
    
    wchar path[MAX_PATH];
    PathGetProgramDirectory(path, arrsize(path));
    PathAddFilename(path, path, L"Log", arrsize(path));
    PathAddFilename(path, path, filename, arrsize(path));
    if (FILE * f = _wfopen(path, L"wb")) {
        va_list args;
        va_start(args, fmt);
        vfprintf(f, fmt, args);
        va_end(args);
        fwrite(data, bytes, 1, f);
        fclose(f);
    }
}
#endif  // ifdef HS_DEBUGGING

//===========================================================================
static void HardCloseSocket (SOCKET sock) {
    static const LINGER s_linger = { true, 0 };
    setsockopt(
        sock, 
        SOL_SOCKET, 
        SO_LINGER, 
        (const char *) &s_linger, 
        sizeof(s_linger)
    );
    closesocket(sock);
}

//===========================================================================
static unsigned SocketCloseTimerCallback (void *) {
    ARRAY(SOCKET) sockets;

    unsigned sleepMs;
    unsigned currTimeMs = TimeGetMs();
    s_socketCrit.Enter();
    for (;;) {
        // If there are no more sockets pending destruction then
        // wait forever; the timer will be restarted when the
        // next socket is queued onto the list.
        NtSock * sock = s_socketList.Head();
        if (!sock) {
            sleepMs = kAsyncTimeInfinite;
            break;
        }

        // Wait until the socket close timer expires
        if (0 < (signed) (sleepMs = sock->closeTimeMs - currTimeMs))
            break;
        
        // Get the socket (safely)
        HANDLE handle;
        sock->critsect.Enter();
        {
            handle = sock->handle;
            sock->handle = INVALID_HANDLE_VALUE;
        }
        sock->critsect.Leave();
        if (handle != INVALID_HANDLE_VALUE)
            sockets.Push((SOCKET) handle);

        // To avoid a race condition, this unlink must occur
        // after the socket handle has been cleared
        s_socketList.Unlink(sock);
    }
    s_socketCrit.Leave();

    // Abortive close all open sockets; any unsent data is lost
    SOCKET * cur = sockets.Ptr();
    SOCKET * end = sockets.Term();
    for (; cur < end; ++cur)
        HardCloseSocket(*cur);

    // Don't run too frequently
    return max(sleepMs, 2000);
}


/****************************************************************************
*
*   Module functions
*
***/

//===========================================================================
void INtSocketInitialize () {
    AsyncTimerCreate(
        &s_socketTimer,
        SocketCloseTimerCallback,
        kAsyncTimeInfinite
    );
}

//===========================================================================
void INtSocketStartCleanup (unsigned exitThreadWaitMs) {
    s_runListenThread = false;
    if (s_listenThread) {
        SetEvent(s_listenEvent);
        WaitForSingleObject(s_listenThread, exitThreadWaitMs);
        CloseHandle(s_listenThread);
        s_listenThread = nil;
    }
    if (s_listenEvent) {
        CloseHandle(s_listenEvent);
        s_listenEvent = nil;
    }

    s_listenCrit.Enter();
    ASSERT(!s_connectList.Head());
    ASSERT(!s_listenList.Head());
    s_listenCrit.Leave();
}

//===========================================================================
void INtSocketDestroy () {
    if (s_socketTimer) {
        AsyncTimerDelete(s_socketTimer, kAsyncTimerDestroyWaitComplete);
        s_socketTimer = nil;
    }
}

//===========================================================================
void INtSockDelete (
    NtSock * sock
) {
    ASSERT(!sock->closed);
    sock->closed = true;

    if (sock->notifyProc) {
        // We have to be extremely careful from this point because
        // sockets can be deleted during the notification callback.
        // After this call, the application becomes responsible for
        // calling NtSocketDelete at some later point in time.
        FAsyncNotifySocketProc notifyProc  = sock->notifyProc;
        sock->notifyProc                = nil;
        notifyProc((AsyncSocket) sock, kNotifySocketDisconnect, nil, &sock->userState);
        DWORD err = GetLastError();
        ref(err);
    }
    else {
        // Since the no application notification procedure was
        // ever set, the socket can now be deleted safely.
        NtSocketDelete((AsyncSocket) sock);
    }
}

//===========================================================================
void INtSocketOpCompleteSocketConnect (NtOpConnAttempt * op) {

    // connect socket to local end
    bool notified;
    if (op->hSocket != INVALID_SOCKET) {
        notified = SocketInitConnect(
            SocketInitCommon(op->hSocket),
            *op
        );
    }
    else {
        notified = false;
    }

    // handle connection failure
    if (!notified) {
        AsyncNotifySocketConnect failed;
        failed.param      = op->param;
        failed.connType   = op->sendData[0];
        failed.remoteAddr = op->remoteAddr;
        ZERO(failed.localAddr);
        op->notifyProc(nil, kNotifySocketConnectFailed, &failed, nil);
    }

    // we can delete the operation outside an NtConn * critical
    // section because it isn't linked into an opList
    // and because connection attempts are not waitable
    ASSERT(!op->link.IsLinked());
    ASSERT(!op->signalComplete);
    DEL(op);

    PerfSubCounter(kAsyncPerfSocketConnAttemptsOutCurr, 1);
}

//===========================================================================
void INtSocketOpCompleteSocketRead (
    NtSock *    sock,
    unsigned    bytes
) {
    ASSERT(sock->ioType == kNtSocket);

    for (ONCE) {
        // a zero-byte read means the socket is going
        // to shutdown, so don't start another read
        if (!bytes)
            break;

        // add new bytes to buffer bytes
        sock->bytesLeft += bytes;

        // dispatch data
        sock->opRead.read.param             = nil;
        sock->opRead.read.asyncId           = 0;
        sock->opRead.read.buffer            = sock->buffer;
        sock->opRead.read.bytes             = sock->bytesLeft;
        sock->opRead.read.bytesProcessed    = 0;

        if (sock->connType == kConnTypeCliToAuth) {
            int x = 0;
            ref(x);
        }

        if (!SocketDispatchRead(sock))
            break;

        // if only some of the bytes were used then shift
        // remaining bytes down otherwise clear buffer.
        if (0 != (sock->bytesLeft -= sock->opRead.read.bytesProcessed)) {

            if ((sock->bytesLeft > sizeof(sock->buffer))
            ||  ((sock->opRead.read.bytesProcessed + sock->bytesLeft) > sizeof(sock->buffer))
            ) {
                #ifdef HS_DEBUGGING
                static long s_once;
                if (!AtomicAdd(&s_once, 1)) {
                    DumpInvalidData(
                        L"NtSockErr.log",
                        sizeof(sock->buffer),
                        sock->buffer,
                        "SocketDispatchRead error for %p: %d %d %d\r\n",
                        sock->notifyProc,
                        sock->bytesLeft,
                        sock->opRead.read.bytes,
                        sock->opRead.read.bytesProcessed
                    );
                }
                #endif  // ifdef HS_DEBUGGING

                LogMsg(
                    kLogError,
                    "SocketDispatchRead error for %p: %d %d %d\r\n",
                    sock->notifyProc,
                    sock->bytesLeft,
                    sock->opRead.read.bytes,
                    sock->opRead.read.bytesProcessed
                );
                break;
            }

            if (sock->opRead.read.bytesProcessed) {
                MemMove(
                    sock->buffer, 
                    sock->buffer + sock->opRead.read.bytesProcessed, 
                    sock->bytesLeft
                );
            }

            // make sure there's enough space left in the buffer for another read  
            if (sock->bytesLeft >= sizeof(sock->buffer))
                break;
        }

        SocketStartAsyncRead(sock);
    }

    INtConnCompleteOperation(sock);
}

//===========================================================================
void INtSocketOpCompleteSocketWrite (
    NtSock *            sock, 
    NtOpSocketWrite *   op
) {
    PerfSubCounter(kAsyncPerfSocketBytesWriteQueued, op->write.bytes);

    // callback notification procedure if requested
    if (op->notify) {
        if (!sock->notifyProc((AsyncSocket) sock, kNotifySocketWrite, &op->write, &sock->userState))
            NtSocketDisconnect((AsyncSocket) sock, false);
    }
}

//===========================================================================
bool INtSocketOpCompleteQueuedSocketWrite (
    NtSock *            sock, 
    NtOpSocketWrite *   op
) {
    PerfSubCounter(kAsyncPerfSocketBytesWaitQueued, op->write.bytes);
    PerfAddCounter(kAsyncPerfSocketBytesWriteQueued, op->write.bytes);

    // must enter critical section in case someone attempts to close socket from another thread
    sock->critsect.Enter();
    op->opType = kOpSocketWrite;
    ASSERT(!op->overlapped.hEvent);
    const bool writeResult = WriteFile(
        sock->handle, 
        op->write.buffer, 
        op->write.bytes, 
        0, 
        &op->overlapped
    );
    sock->critsect.Leave();

//    LogMsg(kLogPerf, L"Nt sock %p wrote %u bytes", sock, op->write.bytes);
    
    if (!writeResult && (GetLastError() != ERROR_IO_PENDING)) {
        op->write.bytesProcessed = 0;

        // No further operations must be allowed to complete. The disconnect
        // must occur before posting a completion notification for this
        // operation, because otherwise another thread might delete the socket
        // before this disconnect could complete (race condition).
        NtSocketDisconnect((AsyncSocket) sock, true);

        // complete operation by posting it
        INtConnPostOperation(sock, op, 0);
        return false;
    }

    return true;
}


/****************************************************************************
*
*   Exported functions
*
***/

//===========================================================================
unsigned NtSocketStartListening (
    const NetAddress &      listenAddr,
    FAsyncNotifySocketProc  notifyProc
) {
    s_listenCrit.Enter();
    StartListenThread();
    NetAddress addr = listenAddr;
    for (;;) {
        // if the port is already open then just increment the reference count
        if (ListenPortIncrement(addr, notifyProc, 1))
            break;

        SOCKET s;
        if (INVALID_SOCKET == (s = ListenSocket(&addr)))
            break;

        // create a new listener record
        NtListener * listener   = s_listenList.New(kListTail, nil, __FILE__, __LINE__);
        listener->hSocket       = s;
        listener->addr          = addr;
        listener->notifyProc    = notifyProc;
        listener->listenCount   = 1;
        break;
    }
    s_listenCrit.Leave();

    unsigned port = NetAddressGetPort(addr);
    if (port)
        SetEvent(s_listenEvent);

    return port;
}

//===========================================================================
void NtSocketStopListening (
    const NetAddress &      listenAddr,
    FAsyncNotifySocketProc  notifyProc
) {
    s_listenCrit.Enter();
    ListenPortIncrement(listenAddr, notifyProc, -1);
    s_listenCrit.Leave();
}

//===========================================================================
void NtSocketConnect (
    AsyncCancelId *         cancelId,
    const NetAddress &      netAddr,
    FAsyncNotifySocketProc  notifyProc,
    void *                  param,
    const void *            sendData,
    unsigned                sendBytes,
    unsigned                connectMs,
    unsigned                localPort
) {
    ASSERT(notifyProc);

    // create async connection record with enough extra bytes for sendData
    NtOpConnAttempt * op = 
     new(ALLOC(sizeof(NtOpConnAttempt) - sizeof(op->sendData) + sendBytes)) NtOpConnAttempt;

    // init Operation
    op->overlapped.Offset       = 0;
    op->overlapped.OffsetHigh   = 0;
    op->overlapped.hEvent       = nil;
    op->opType                  = kOpConnAttempt;
    op->asyncId                 = nil;
    op->notify                  = true;
    op->pending                 = 1;
    op->signalComplete          = nil;

    // init OpConnAttempt
    op->canceled                = false;
    op->localPort               = localPort;
    op->remoteAddr              = netAddr;
    op->notifyProc              = notifyProc;
    op->param                   = param;
    op->hSocket                 = INVALID_SOCKET;
    op->failTimeMs              = connectMs ? connectMs : kConnectTimeMs;
    if (0 != (op->sendBytes = sendBytes))
        MemCopy(op->sendData, sendData, sendBytes);
    else
        op->sendData[0] = kConnTypeNil;

    PerfAddCounter(kAsyncPerfSocketConnAttemptsOutCurr, 1);
    PerfAddCounter(kAsyncPerfSocketConnAttemptsOutTotal, 1);

    s_listenCrit.Enter();
    StartListenThread();

    // get cancel id; we can avoid checking for zero by always using an odd number
    ASSERT(s_nextConnectCancelId & 1);
    s_nextConnectCancelId += 2;

    *cancelId = op->cancelId = (AsyncCancelId) s_nextConnectCancelId;
    s_connectList.Link(op, kListTail);
    s_listenCrit.Leave();
    SetEvent(s_listenEvent);
}

//===========================================================================
// due to the asynchronous nature sockets, the connect may occur
// before the cancel can complete... you have been warned
void NtSocketConnectCancel (
    FAsyncNotifySocketProc notifyProc,
    AsyncCancelId          cancelId        // nil = cancel all with specified notifyProc
) {
    s_listenCrit.Enter();
    for (NtOpConnAttempt * op = s_connectList.Head(); op; op = s_connectList.Next(op)) {
        if (cancelId && (op->cancelId != cancelId))
            continue;
        if (op->notifyProc != notifyProc)
            continue;
        op->canceled = true;
    }
    s_listenCrit.Leave();
}

//===========================================================================
// This function must ONLY be called after receiving a NOTIFY_DISCONNECT message
// for a socket. After a NOTIFY_DISCONNECT, the socket will fail all I/O initiated
// against it, but will otherwise continue to exist. The memory for the socket will
// only be freed when NtSocketDelete is called.
void NtSocketDelete (AsyncSocket conn) {
    NtSock * sock = (NtSock *) conn;
    if (sock->ioType != kNtSocket) {
        LogMsg(kLogError, "NtSocketDelete %u %p", sock->ioType, sock->notifyProc);
        return;
    }

    DEL(sock);
}

//===========================================================================
void NtSocketDisconnect (AsyncSocket conn, bool hardClose) {
    NtSock * sock = (NtSock *) conn;
    ASSERT(sock->ioType == kNtSocket);

    // must enter critical section in case someone attempts to close socket from another thread
    HANDLE handle;
    sock->critsect.Enter();
    if (hardClose) {
        // Prepare to close the socket immediately once we leave the critsect
        handle          = sock->handle;
        sock->handle    = INVALID_HANDLE_VALUE;

        // Mark the socket closed in such a way that, if it has already been
        // soft closed, the mark won't invalidate the ordering of s_socketList
        sock->closeTimeMs |= 1;
    }
    else if (!sock->closeTimeMs) {
        // The socket hasn't been closed previously; perform shutdown,
        // and mark the socket closed with a time value that indicates
        // its ordering in s_socketList;
        handle              = INVALID_HANDLE_VALUE;
        sock->closeTimeMs   = (TimeGetMs() + kCloseTimeoutMs) | 1;
        shutdown((SOCKET) sock->handle, SD_SEND);
    }
    else {
        // The socket has already been closed previously
        handle      = INVALID_HANDLE_VALUE;
        hardClose   = true;
    }
    sock->critsect.Leave();

    if (handle != INVALID_HANDLE_VALUE) {
        HardCloseSocket((SOCKET) handle);
    }
    else if (!hardClose) {
        // Add the socket to the close list in sorted order by close time;
        // if this socket is the first on the list then start the timer
        bool startTimer;
        s_socketCrit.Enter();
        {
            s_socketList.Link(sock, kListTail);
            startTimer = s_socketList.Head() == sock;
        }
        s_socketCrit.Leave();

        // If this is the first item queued in the socket list then start timer.
        // This operation should be safe to perform outside the critical section
        // because s_socketTimer should not be deleted before application shutdown
        if (startTimer)
            AsyncTimerUpdate(s_socketTimer, kCloseTimeoutMs);
    }
}

//===========================================================================
bool NtSocketSend (
    AsyncSocket     conn,
    const void *    data,
    unsigned        bytes
) {
    NtSock * sock = (NtSock *) conn;
    ASSERT(sock);
    ASSERT(data);
    ASSERT(bytes);
    ASSERT(sock->ioType == kNtSocket);

//    LogMsg(kLogPerf, L"Nt sock %p sending %u bytes", sock, bytes);

    bool result;
    sock->critsect.Enter();
    for (;;) {
        // Is the socket closing?
        if (sock->closeTimeMs) {
            result = false;
            break;
        }

        // if there isn't any data queued, send this batch immediately
        bool dataQueued = sock->opList.Head() != nil;
        if (!dataQueued) {
            int bytesSent = send((SOCKET) sock->handle, (const char *) data, bytes, 0);
            if (bytesSent != SOCKET_ERROR) {
                result = true;

                // if we sent all the data then exit
                if ((unsigned) bytesSent >= bytes)
                    break;

                // subtract the data we already sent
                data = (const byte *) data + bytesSent;
                bytes -= bytesSent;
                // and queue it below
            }
            else if (WSAEWOULDBLOCK != WSAGetLastError()) {
                // an error occurred -- destroy connection
                NtSocketDisconnect((AsyncSocket) sock, true);
                result = false;
                break;
            }
        }

        NtOpSocketWrite * op = SocketQueueAsyncWrite(sock, (const byte *) data, bytes);
        if (op && !dataQueued)
            result = INtSocketOpCompleteQueuedSocketWrite(sock, op);
        else
            result = true;
        break;
    }
    sock->critsect.Leave();

    return result;
}

//===========================================================================
bool NtSocketWrite (
    AsyncSocket     conn,
    const void *    buffer,
    unsigned        bytes,
    void *          param
) {
    NtSock * sock = (NtSock *) conn;
    ASSERT(buffer);
    ASSERT(bytes);
    ASSERT(sock->ioType == kNtSocket);

//    LogMsg(kLogPerf, L"Nt sock %p writing %u bytes", sock, bytes);

    bool result;
    sock->critsect.Enter();
    for (;;) {
        // Is the socket closing?
        if (sock->closeTimeMs) {
            result = false;
            break;
        }

        // init Operation
        NtOpSocketWrite * op        = NEW(NtOpSocketWrite);
        op->overlapped.Offset       = 0;
        op->overlapped.OffsetHigh   = 0;
        op->overlapped.hEvent       = nil;
        op->opType                  = kOpQueuedSocketWrite;
        op->asyncId                 = INtConnSequenceStart(sock);
        op->notify                  = true;
        op->pending                 = 1;
        op->signalComplete          = nil;
        sock->opList.Link(op, kListTail);

        // init OpWrite
        op->queueTimeMs             = TimeGetMs();
        op->bytesAlloc              = bytes;
        op->write.param             = param;
        op->write.asyncId           = op->asyncId;
        op->write.buffer            = (byte *) buffer;
        op->write.bytes             = bytes;
        op->write.bytesProcessed    = bytes;
        PerfAddCounter(kAsyncPerfSocketBytesWaitQueued, bytes);

        InterlockedIncrement(&sock->ioCount);

        if (op == sock->opList.Head())
            result = INtSocketOpCompleteQueuedSocketWrite((NtSock *) sock, op);
        else
            result = true;
        break;
    }
    sock->critsect.Leave();
    return result;
}

//===========================================================================
// -- use only for server<->client connections, not server<->server!
// -- Note that Nagling is enabled by default
void NtSocketEnableNagling (AsyncSocket conn, bool enable) {
    NtSock * sock = (NtSock *) conn;
    ASSERT(sock->ioType == kNtSocket);
    
    // must enter critical section in case someone attempts to close socket from another thread
    sock->critsect.Enter();
    if (sock->handle != INVALID_HANDLE_VALUE) {
        BOOL noDelay = !enable;
        const int result = setsockopt(
            (SOCKET) sock->handle, 
            IPPROTO_TCP, 
            TCP_NODELAY, 
            (const char *) &noDelay, 
            sizeof(noDelay)
        );
        if (result)
            LogMsg(kLogError, "setsockopt failed (nagling)");
    }
    sock->critsect.Leave();
}

//===========================================================================
void NtSocketSetNotifyProc (
    AsyncSocket            conn,
    FAsyncNotifySocketProc notifyProc
) {
    NtSock * sock = (NtSock *) conn;
    ASSERT(sock->ioType == kNtSocket);
    ((NtSock *) sock)->notifyProc = notifyProc;
}

//===========================================================================
void NtSocketSetBacklogAlloc (AsyncSocket conn, unsigned bufferSize) {
    NtSock * sock = (NtSock *) conn;
    ASSERT(sock->ioType == kNtSocket);
    ((NtSock *) sock)->backlogAlloc = bufferSize;
}

} using namespace Nt;
