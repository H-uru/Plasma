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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCoreExe/Private/Nt/pnAceNtSocket.cpp
*   
***/

#include "../../Pch.h"

#include "pnAceNtInt.h"

#include <mutex>


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

// wait before checking for backlog problems
static const unsigned   kBacklogInitMs      = 3*60*1000;

// destroy a connection if it has a backlog "problem"
static const unsigned   kBacklogFailMs      = 2*60*1000;

static const unsigned   kMinBacklogBytes    = 4 * 1024;

struct NtOpConnAttempt : Operation {
    AsyncCancelId           cancelId;
    bool                    canceled;
    plNetAddress            remoteAddr;
    FAsyncNotifySocketProc  notifyProc;
    void *                  param;
    SOCKET                  hSocket;
    unsigned                failTimeMs;
    unsigned                sendBytes;
    uint8_t                 sendData[1];    // actually [sendBytes]
    // no additional fields
};

struct NtOpSocketWrite : Operation {
    unsigned                queueTimeMs;
    unsigned                bytesAlloc;
    AsyncNotifySocketWrite  write;

    NtOpSocketWrite() : queueTimeMs(0), bytesAlloc(0) { }
};

struct NtOpSocketRead : Operation {
    AsyncNotifySocketRead   read;
};

struct NtSock : NtObject {
    LINK(NtSock)            link;
    plNetAddress            addr;
    unsigned                closeTimeMs;
    unsigned                connType;
    FAsyncNotifySocketProc  notifyProc;
    unsigned                bytesLeft;
    NtOpSocketRead          opRead;
    unsigned                backlogAlloc;
    unsigned                initTimeMs;
    uint8_t                 buffer[kAsyncSocketBufferSize];

    NtSock ();
    ~NtSock ();
};


static std::recursive_mutex             s_listenCrit;
static LISTDECL(NtOpConnAttempt, link)  s_connectList;
static bool                             s_runListenThread;
static uintptr_t                        s_nextConnectCancelId = 1;
static std::thread                      s_listenThread;
static HANDLE                           s_listenEvent;


const unsigned kCloseTimeoutMs = 8*1000;
static std::recursive_mutex             s_socketCrit;
static AsyncTimer *                     s_socketTimer;
static LISTDECL(NtSock, link)           s_socketList;


//===========================================================================
inline NtSock::NtSock ()
    : closeTimeMs(), connType(), notifyProc(), bytesLeft()
    , backlogAlloc(), initTimeMs()
{
    memset(buffer, 0, sizeof(buffer));

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
        hsLockGuard(s_socketCrit);
        link.Unlink();
    }

    if (handle != INVALID_HANDLE_VALUE)
        closesocket((SOCKET) handle);

    PerfSubCounter(kAsyncPerfSocketsCurr, 1);
}

//===========================================================================
static void SocketGetAddresses (
    NtSock *        sock,
    plNetAddress*   localAddr,
    plNetAddress*   remoteAddr
) {
    // don't have to enter critsect or validate socket before referencing it
    // because this routine is called before the user has a chance to close it
    sockaddr_in addrBuf;
    int nameLen = (int)sizeof(addrBuf);
    if (getsockname((SOCKET) sock->handle, (sockaddr *) &addrBuf, &nameLen) != 0)
        LogMsg(kLogError, "getsockname failed");
    localAddr->SetHost(addrBuf.sin_addr.s_addr);
    localAddr->SetPort(ntohs(addrBuf.sin_port));

    nameLen = (int)sizeof(addrBuf);
    if (getpeername((SOCKET) sock->handle, (sockaddr *) &addrBuf, &nameLen) != 0)
        LogMsg(kLogError, "getpeername failed");
    remoteAddr->SetHost(addrBuf.sin_addr.s_addr);
    remoteAddr->SetPort(ntohs(addrBuf.sin_port));
}

//===========================================================================
static void SocketStartAsyncRead (NtSock * sock) {
    // enter critical section in case someone attempts to close socket from another thread
    bool readResult;
    {
        hsLockGuard(sock->critsect);

        if (sock->handle != INVALID_HANDLE_VALUE) {
            ++sock->ioCount;
            readResult = ReadFile(
                sock->handle,
                sock->buffer + sock->bytesLeft,
                sizeof(sock->buffer) - sock->bytesLeft,
                nullptr,
                &sock->opRead.overlapped
            );
        }
        else {
            readResult = true;
        }
    }

    DWORD err = GetLastError();
    if (!readResult && (err != ERROR_IO_PENDING))
        --sock->ioCount;
}

//===========================================================================
static bool SocketDispatchRead (NtSock * sock) {
//    LogMsg(kLogPerf, "Nt sock {#x} recv {} bytes", (uintptr_t)sock, sock->opRead.read.bytes);

    // put "fast case" first -- connType already established
    if (sock->notifyProc)
        return sock->notifyProc((AsyncSocket) sock, kNotifySocketRead, &sock->opRead.read, &sock->userState);

    ASSERT(sock->opRead.read.buffer == sock->buffer);
    ASSERT(sock->opRead.read.bytes);

    return false;
}

//===========================================================================
static NtOpSocketWrite * SocketQueueAsyncWrite (
    NtSock *        sock,
    const uint8_t * data,
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
                    "Backlog, c:{} q:{}, i:{}",
                    sock->connType,
                    currTimeMs - firstQueuedWrite->queueTimeMs,
                    currTimeMs - sock->initTimeMs
                );
            }
            AsyncSocketDisconnect((AsyncSocket) sock, true);
            return nullptr;
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
            bytesLeft = std::min(bytesLeft, bytes);
            if (bytesLeft) {
                PerfAddCounter(kAsyncPerfSocketBytesWaitQueued, bytesLeft);
                memcpy(lastQueuedWrite->write.buffer + lastQueuedWrite->write.bytes, data, bytesLeft);
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
    unsigned bytesAlloc = std::max(bytes, sock->backlogAlloc);
    bytesAlloc          = std::max(bytesAlloc, kMinBacklogBytes);
    NtOpSocketWrite * op = new(malloc(sizeof(NtOpSocketWrite) + bytesAlloc)) NtOpSocketWrite;

    // init Operation
    const AsyncId asyncId       = INtConnSequenceStart(sock);
    op->overlapped.Offset       = 0;
    op->overlapped.OffsetHigh   = 0;
    op->overlapped.hEvent       = nullptr;
    op->opType                  = kOpQueuedSocketWrite;
    op->asyncId                 = asyncId;
    op->notify                  = false;
    op->pending                 = 1;
    sock->opList.Link(op, kListTail);

    // init OpWrite
    op->queueTimeMs             = TimeGetMs();
    op->bytesAlloc              = bytesAlloc;
    op->write.param             = nullptr;
    op->write.asyncId           = asyncId;
    op->write.buffer            = (uint8_t *) (op + 1);
    op->write.bytes             = bytes;
    op->write.bytesProcessed    = bytes;
    memcpy(op->write.buffer, data, bytes);

    ++sock->ioCount;
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
    NtSock * sock       = new NtSock;
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
        if (op.sendBytes && !AsyncSocketSend((AsyncSocket) sock, op.sendData, op.sendBytes))
            break;

        // Determine connType
        while (op.sendBytes) {
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
        notify.asyncId      = nullptr;
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
static SOCKET ConnectSocket(const plNetAddress& addr) {
    SOCKET s;
    if (INVALID_SOCKET == (s = socket(AF_INET, SOCK_STREAM, 0))) {
        LogMsg(kLogError, "socket create failed");
        return INVALID_SOCKET;
    }

    do {
        // make socket non-blocking
        u_long nonBlocking = true;
        if (ioctlsocket(s, FIONBIO, &nonBlocking))
            LogMsg(kLogError, "ioctlsocket failed (make non-blocking)");

        sockaddr_in saddr;
        saddr.sin_family = AF_INET;
        saddr.sin_port = htons(addr.GetPort());
        saddr.sin_addr.s_addr = addr.GetHost();
        memset(saddr.sin_zero, 0, sizeof(saddr.sin_zero));
        if (connect(s, (const sockaddr *) &saddr, sizeof(saddr))) {
            if (WSAGetLastError() != WSAEWOULDBLOCK) {
                LogMsg(kLogError, "socket connect failed");
                break;
            }
        }

        // success!
        return s;
    } while (false);

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
            INtConnPostOperation(nullptr, op, 0);
            continue;
        }

        // open new sockets
        if (op->hSocket == INVALID_SOCKET) {
            if (INVALID_SOCKET == (op->hSocket = ConnectSocket(op->remoteAddr))) {
                s_connectList.Unlink(op);
                INtConnPostOperation(nullptr, op, 0);
                continue;
            }

            // start failure timer
            op->failTimeMs += currTimeMs;
        }

        // add socket to fdset
        FD_SET(op->hSocket, writefds);
    }
}

//===========================================================================
static void ListenThreadProc() {
    fd_set writefds;
    for (;;) {
        {
            hsLockGuard(s_listenCrit);
            ListenPrepareConnectors(&writefds);
        }
        if (!s_runListenThread)
            break;

        // wait until there is something on connect list
        if (!writefds.fd_count) {
            WaitForSingleObject(s_listenEvent, INFINITE);
            continue;
        }

        // wait for connection or timeout
        const struct timeval timeout = { 0, 250*1000 }; // seconds, microseconds
        int result = select(0, nullptr, &writefds, nullptr, &timeout);
        if (result == SOCKET_ERROR) {
            LogMsg(kLogError, "socket select failed");
            continue;
        }
        if (!result)
            continue;

        hsLockGuard(s_listenCrit);

        // complete connect() operations
        for (NtOpConnAttempt *next, *op = s_connectList.Head(); op; op = next) {
            next = s_connectList.Next(op);

            if (FD_ISSET(op->hSocket, &writefds)) {
                s_connectList.Unlink(op);
                INtConnPostOperation(nullptr, op, 0);
            }
        }
    }

    // cleanup all connectors
    hsLockGuard(s_listenCrit);
    for (NtOpConnAttempt * op; (op = s_connectList.Head()) != nullptr; s_connectList.Unlink(op)) {
        if (op->hSocket != INVALID_SOCKET) {
            closesocket(op->hSocket);
            op->hSocket = 0;
        }
        INtConnPostOperation(nullptr, op, 0);
    }
}

//===========================================================================
static void StartListenThread () {
    if (s_listenThread.joinable())
        return;

    s_listenEvent = CreateEvent(
        nullptr,
        false,  // auto-reset event
        false,  // initial state unsignaled
        nullptr
    );
    ASSERT(s_listenEvent);

    // create a low-priority thread to listen on ports
    s_runListenThread = true;
    s_listenThread = std::thread([] {
#ifdef USE_VLD
        VLDEnable();
#endif
        PerfAddCounter(kAsyncPerfThreadsTotal, 1);
        PerfAddCounter(kAsyncPerfThreadsCurr, 1);

        ListenThreadProc();

        PerfSubCounter(kAsyncPerfThreadsCurr, 1);
    });
}

//===========================================================================
#ifdef HS_DEBUGGING
#include <cstdio>
static void __cdecl DumpInvalidData (
    const plFileName & filename,
    unsigned    bytes,
    const uint8_t  data[],
    const char  fmt[],
    ...
) {
    plFileName path = plFileSystem::GetCurrentAppPath().StripFileName();
    path = plFileName::Join(path, "Log", filename);
    if (FILE * f = plFileSystem::Open(path, "wb")) {
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
    std::vector<SOCKET> sockets;

    unsigned sleepMs;
    unsigned currTimeMs = TimeGetMs();
    {
        hsLockGuard(s_socketCrit);

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
            {
                hsLockGuard(sock->critsect);
                handle = sock->handle;
                sock->handle = INVALID_HANDLE_VALUE;
            }
            if (handle != INVALID_HANDLE_VALUE)
                sockets.emplace_back((SOCKET) handle);

            // To avoid a race condition, this unlink must occur
            // after the socket handle has been cleared
            s_socketList.Unlink(sock);
        }
    }

    // Abortive close all open sockets; any unsent data is lost
    for (SOCKET cur : sockets)
        HardCloseSocket(cur);

    // Don't run too frequently
    return std::max(sleepMs, 2000u);
}


/****************************************************************************
*
*   Module functions
*
***/

//===========================================================================
void INtSocketInitialize () {
    s_socketTimer = AsyncTimerCreate(
        SocketCloseTimerCallback,
        kAsyncTimeInfinite
    );
}

//===========================================================================
void INtSocketStartCleanup (unsigned exitThreadWaitMs) {
    s_runListenThread = false;
    if (s_listenThread.joinable()) {
        SetEvent(s_listenEvent);
        AsyncThreadTimedJoin(s_listenThread, exitThreadWaitMs);
        s_listenThread = {};
    }
    if (s_listenEvent) {
        CloseHandle(s_listenEvent);
        s_listenEvent = nullptr;
    }

    hsLockGuard(s_listenCrit);
    ASSERT(!s_connectList.Head());
}

//===========================================================================
void INtSocketDestroy () {
    if (s_socketTimer) {
        AsyncTimerDelete(s_socketTimer);
        s_socketTimer = nullptr;
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
        // calling AsyncSocketDelete at some later point in time.
        FAsyncNotifySocketProc notifyProc  = sock->notifyProc;
        sock->notifyProc                = nullptr;
        notifyProc((AsyncSocket) sock, kNotifySocketDisconnect, nullptr, &sock->userState);
        DWORD err = GetLastError();
    }
    else {
        // Since the no application notification procedure was
        // ever set, the socket can now be deleted safely.
        AsyncSocketDelete((AsyncSocket) sock);
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
        memset(&failed.localAddr, 0, sizeof(failed.localAddr));
        op->notifyProc(nullptr, kNotifySocketConnectFailed, &failed, nullptr);
    }

    // we can delete the operation outside an NtConn * critical
    // section because it isn't linked into an opList
    // and because connection attempts are not waitable
    ASSERT(!op->link.IsLinked());
    delete op;

    PerfSubCounter(kAsyncPerfSocketConnAttemptsOutCurr, 1);
}

//===========================================================================
void INtSocketOpCompleteSocketRead (
    NtSock *    sock,
    unsigned    bytes
) {
    ASSERT(sock->ioType == kNtSocket);

    do {
        // a zero-byte read means the socket is going
        // to shutdown, so don't start another read
        if (!bytes)
            break;

        // add new bytes to buffer bytes
        sock->bytesLeft += bytes;

        // dispatch data
        sock->opRead.read.param             = nullptr;
        sock->opRead.read.asyncId           = nullptr;
        sock->opRead.read.buffer            = sock->buffer;
        sock->opRead.read.bytes             = sock->bytesLeft;
        sock->opRead.read.bytesProcessed    = 0;

        if (!SocketDispatchRead(sock))
            break;

        // if only some of the bytes were used then shift
        // remaining bytes down otherwise clear buffer.
        if (0 != (sock->bytesLeft -= sock->opRead.read.bytesProcessed)) {

            if ((sock->bytesLeft > sizeof(sock->buffer))
            ||  ((sock->opRead.read.bytesProcessed + sock->bytesLeft) > sizeof(sock->buffer))
            ) {
                #ifdef HS_DEBUGGING
                static std::once_flag s_once;
                std::call_once(s_once, [sock]() {
                    DumpInvalidData(
                        "NtSockErr.log",
                        sizeof(sock->buffer),
                        sock->buffer,
                        "SocketDispatchRead error for %p: %d %d %d\r\n",
                        sock->notifyProc,
                        sock->bytesLeft,
                        sock->opRead.read.bytes,
                        sock->opRead.read.bytesProcessed
                    );
                });
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
                memmove(
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
    } while(false);

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
            AsyncSocketDisconnect((AsyncSocket) sock, false);
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
    bool writeResult;
    {
        hsLockGuard(sock->critsect);
        op->opType = kOpSocketWrite;
        ASSERT(!op->overlapped.hEvent);
        writeResult = WriteFile(
            sock->handle,
            op->write.buffer,
            op->write.bytes,
            nullptr,
            &op->overlapped
        );
    }

//    LogMsg(kLogPerf, "Nt sock {#x} wrote {} bytes", (uintptr_t)sock, op->write.bytes);
    
    if (!writeResult && (GetLastError() != ERROR_IO_PENDING)) {
        op->write.bytesProcessed = 0;

        // No further operations must be allowed to complete. The disconnect
        // must occur before posting a completion notification for this
        // operation, because otherwise another thread might delete the socket
        // before this disconnect could complete (race condition).
        AsyncSocketDisconnect((AsyncSocket) sock, true);

        // complete operation by posting it
        INtConnPostOperation(sock, op, 0);
        return false;
    }

    return true;
}

} // namespace Nt

using namespace Nt;


/****************************************************************************
*
*   Exported functions
*
***/

//===========================================================================
void AsyncSocketConnect(
    AsyncCancelId *         cancelId,
    const plNetAddress&     netAddr,
    FAsyncNotifySocketProc  notifyProc,
    void *                  param,
    const void *            sendData,
    unsigned                sendBytes
) {
    ASSERT(notifyProc);

    // create async connection record with enough extra bytes for sendData
    NtOpConnAttempt * op = 
     new(malloc(sizeof(NtOpConnAttempt) - sizeof(op->sendData) + sendBytes)) NtOpConnAttempt;

    // init Operation
    op->overlapped.Offset       = 0;
    op->overlapped.OffsetHigh   = 0;
    op->overlapped.hEvent       = nullptr;
    op->opType                  = kOpConnAttempt;
    op->asyncId                 = nullptr;
    op->notify                  = true;
    op->pending                 = 1;

    // init OpConnAttempt
    op->canceled                = false;
    op->remoteAddr              = netAddr;
    op->notifyProc              = notifyProc;
    op->param                   = param;
    op->hSocket                 = INVALID_SOCKET;
    op->failTimeMs              = kConnectTimeMs;
    if (0 != (op->sendBytes = sendBytes))
        memcpy(op->sendData, sendData, sendBytes);
    else
        op->sendData[0] = kConnTypeNil;

    PerfAddCounter(kAsyncPerfSocketConnAttemptsOutCurr, 1);
    PerfAddCounter(kAsyncPerfSocketConnAttemptsOutTotal, 1);

    {
        hsLockGuard(s_listenCrit);
        StartListenThread();

        // get cancel id; we can avoid checking for zero by always using an odd number
        ASSERT(s_nextConnectCancelId & 1);
        s_nextConnectCancelId += 2;

        *cancelId = op->cancelId = (AsyncCancelId)s_nextConnectCancelId;
        s_connectList.Link(op, kListTail);
    }
    SetEvent(s_listenEvent);
}

//===========================================================================
// due to the asynchronous nature sockets, the connect may occur
// before the cancel can complete... you have been warned
void AsyncSocketConnectCancel(
    AsyncCancelId          cancelId        // nullptr = cancel all with specified notifyProc
) {
    hsLockGuard(s_listenCrit);
    for (NtOpConnAttempt * op = s_connectList.Head(); op; op = s_connectList.Next(op)) {
        if (cancelId && (op->cancelId != cancelId))
            continue;
        if (op->notifyProc != nullptr)
            continue;
        op->canceled = true;
    }
}

//===========================================================================
// This function must ONLY be called after receiving a NOTIFY_DISCONNECT message
// for a socket. After a NOTIFY_DISCONNECT, the socket will fail all I/O initiated
// against it, but will otherwise continue to exist. The memory for the socket will
// only be freed when AsyncSocketDelete is called.
void AsyncSocketDelete(AsyncSocket conn) {
    NtSock * sock = (NtSock *) conn;
    if (sock->ioType != kNtSocket) {
        LogMsg(kLogError, "AsyncSocketDelete {} {#x}", sock->ioType, (uintptr_t)sock->notifyProc);
        return;
    }

    delete sock;
}

//===========================================================================
void AsyncSocketDisconnect(AsyncSocket conn, bool hardClose) {
    NtSock * sock = (NtSock *) conn;
    ASSERT(sock->ioType == kNtSocket);

    // must enter critical section in case someone attempts to close socket from another thread
    HANDLE handle;
    {
        hsLockGuard(sock->critsect);
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
    }

    if (handle != INVALID_HANDLE_VALUE) {
        HardCloseSocket((SOCKET) handle);
    }
    else if (!hardClose) {
        // Add the socket to the close list in sorted order by close time;
        // if this socket is the first on the list then start the timer
        bool startTimer;
        {
            hsLockGuard(s_socketCrit);
            s_socketList.Link(sock, kListTail);
            startTimer = s_socketList.Head() == sock;
        }

        // If this is the first item queued in the socket list then start timer.
        // This operation should be safe to perform outside the critical section
        // because s_socketTimer should not be deleted before application shutdown
        if (startTimer)
            AsyncTimerUpdate(s_socketTimer, kCloseTimeoutMs);
    }
}

//===========================================================================
bool AsyncSocketSend(
    AsyncSocket     conn,
    const void *    data,
    unsigned        bytes
) {
    NtSock * sock = (NtSock *) conn;
    ASSERT(sock);
    ASSERT(data);
    ASSERT(bytes);
    ASSERT(sock->ioType == kNtSocket);

//    LogMsg(kLogPerf, "Nt sock {#x} sending {} bytes", (uintptr_t)sock, bytes);

    bool result;
    hsLockGuard(sock->critsect);
    for (;;) {
        // Is the socket closing?
        if (sock->closeTimeMs) {
            result = false;
            break;
        }

        // if there isn't any data queued, send this batch immediately
        bool dataQueued = sock->opList.Head() != nullptr;
        if (!dataQueued) {
            int bytesSent = send((SOCKET) sock->handle, (const char *) data, bytes, 0);
            if (bytesSent != SOCKET_ERROR) {
                result = true;

                // if we sent all the data then exit
                if ((unsigned) bytesSent >= bytes)
                    break;

                // subtract the data we already sent
                data = (const uint8_t *) data + bytesSent;
                bytes -= bytesSent;
                // and queue it below
            }
            else if (WSAEWOULDBLOCK != WSAGetLastError()) {
                // an error occurred -- destroy connection
                AsyncSocketDisconnect((AsyncSocket) sock, true);
                result = false;
                break;
            }
        }

        NtOpSocketWrite * op = SocketQueueAsyncWrite(sock, (const uint8_t *) data, bytes);
        if (op && !dataQueued)
            result = INtSocketOpCompleteQueuedSocketWrite(sock, op);
        else
            result = true;
        break;
    }

    return result;
}

//===========================================================================
// -- use only for server<->client connections, not server<->server!
// -- Note that Nagling is enabled by default
void AsyncSocketEnableNagling(AsyncSocket conn, bool enable) {
    NtSock * sock = (NtSock *) conn;
    ASSERT(sock->ioType == kNtSocket);
    
    // must enter critical section in case someone attempts to close socket from another thread
    hsLockGuard(sock->critsect);
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
}
