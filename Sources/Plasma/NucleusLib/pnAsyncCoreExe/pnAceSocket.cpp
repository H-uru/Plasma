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

#include "Pch.h"

using tcp = asio::ip::tcp;

// handles 8 CPU computer w/hyperthreading (we use 2 threads per logical CPU)
static constexpr unsigned int kMaxWorkerThreads = 32;

// wait before checking for backlog problems
static constexpr unsigned kBacklogInitMs = 3 * 60 * 1000;

// destroy a connection if it has a backlog "problem"
static constexpr unsigned kBacklogFailMs = 2 * 60 * 1000;

static constexpr size_t kMinBacklogBytes = 4 * 1024;

struct AsyncIoPool
{
    asio::io_context                                           fContext;
    asio::executor_work_guard<asio::io_context::executor_type> fWorkGuard;
    AsyncThreadRef                                             fThreadHandles[kMaxWorkerThreads];

    AsyncIoPool() : fWorkGuard(fContext.get_executor())
    {
        // calculate number of IO worker threads to create
        unsigned int threadCount = std::max(std::thread::hardware_concurrency() * 2, 2U);
        if (threadCount > kMaxWorkerThreads) {
            threadCount = kMaxWorkerThreads;
            LogMsg(kLogError, "kMaxWorkerThreads too small!");
        }

        // create IO worker threads
        size_t i = 0;
        for (auto& threadHandle : fThreadHandles) {
            threadHandle = AsyncThreadCreate([this, i] {
                hsThread::SetThisThreadName(ST::format("AceSocketPool{02d}", i));
                // This can be run concurrently from several threads
                fContext.run();
            });
            i++;
        }
    }

    void Destroy(unsigned exitThreadWaitMs)
    {
        fWorkGuard.reset();

        for (AsyncThreadRef& thread : fThreadHandles) {
            if (thread.joinable())
                AsyncThreadTimedJoin(thread, exitThreadWaitMs);
        }

        // Ensure the event loop exits without processing any more tasks,
        // in case the threads take more than exitThreadWaitMs to finish
        fContext.stop();
    }
};

struct ConnectOperation
{
    tcp::socket             fSock;
    plNetAddress            fRemoteAddr;
    AsyncCancelId           fCancelId;
    FAsyncNotifySocketProc  fNotifyProc;
    void*                   fParam;
    std::vector<uint8_t>    fConnectBuffer;
    unsigned int            fConnectionType;

    ConnectOperation(asio::io_context& context)
        : fSock(context), fCancelId(), fParam(), fConnectionType()
    { }
    ConnectOperation(const ConnectOperation&) = delete;
};

struct WriteOperation
{
    size_t                  fAllocSize;
    AsyncNotifySocketWrite  fNotify;
    unsigned                queueTimeMs;

    WriteOperation() : fAllocSize() { }

    asio::const_buffer AsBuffer() const
    {
        return asio::buffer(fNotify.buffer + fNotify.bytesCommitted, fNotify.bytes - fNotify.bytesCommitted);
    }
};

struct AsyncSocketStruct
{
    std::recursive_mutex        fCritsect;
    tcp::socket                 fSock;
    FAsyncNotifySocketProc      fNotifyProc;
    void*                       fParam;
    unsigned int                fConnectionType;
    void*                       fUserState;
    uint8_t                     fBuffer[kAsyncSocketBufferSize];
    size_t                      fBytesLeft;
    std::list<WriteOperation *> fWriteOps;
    unsigned                    initTimeMs;
    unsigned                    closeTimeMs;

    AsyncSocketStruct(ConnectOperation& op)
        : fSock(std::move(op.fSock)), fNotifyProc(op.fNotifyProc),
          fParam(op.fParam), fConnectionType(op.fConnectionType),
          fUserState(), fBuffer(), fBytesLeft(), initTimeMs(), closeTimeMs()
    { }
};

static AsyncIoPool*                 s_ioPool;

static std::recursive_mutex         s_connectCrit;
static std::list<ConnectOperation>  s_connectList;
static uintptr_t                    s_nextConnectCancelId = 1;

void SocketInitialize()
{
    if (s_ioPool)
        return;

    s_ioPool = new AsyncIoPool;
}

void SocketDestroy(unsigned exitThreadWaitMs)
{
    // Cancel any pending connect requests
    {
        hsLockGuard(s_connectCrit);
        for (ConnectOperation& op : s_connectList) {
            asio::error_code err;
            op.fSock.cancel(err);  // This will wake up async_connect()
            if (err)
                LogMsg(kLogError, "Failed to cancel socket connect operation: {}", err.message());
        }
    }

    if (s_ioPool) {
        s_ioPool->Destroy(exitThreadWaitMs);
        delete s_ioPool;
        s_ioPool = nullptr;
    }
}

static void SocketStartAsyncRead(AsyncSocket sock)
{
    hsLockGuard(sock->fCritsect);
    uint8_t* start = sock->fBuffer + sock->fBytesLeft;
    size_t   count = sizeof(sock->fBuffer) - sock->fBytesLeft;
    sock->fSock.async_read_some(asio::buffer(start, count),
        [sock](const asio::error_code& err, size_t bytes) {
        if (err) {
            bool isEOFError     = (err.category() == asio::error::get_misc_category() && err.value() == asio::error::eof);
            bool isAbortedError = (err.category() == asio::error::get_system_category() && err.value() == asio::error::operation_aborted);
            if (isEOFError || isAbortedError) {
                if (sock->fNotifyProc) {
                    // We have to be extremely careful from this point because
                    // sockets can be deleted during the notification callback.
                    // After this call, the application becomes responsible for
                    // calling AsyncSocketDelete at some later point in time.
                    FAsyncNotifySocketProc notifyProc = sock->fNotifyProc;
                    sock->fNotifyProc = nullptr;
                    notifyProc((AsyncSocket)sock, kNotifySocketDisconnect, nullptr, &sock->fUserState);
                }
            } else {
                LogMsg(kLogError, "Failed to read from socket: {}", err.message());
            }
            return;
        }

        if (!bytes)
            return;

        sock->fBytesLeft += bytes;

        AsyncNotifySocketRead notifyRead;
        notifyRead.buffer = sock->fBuffer;
        notifyRead.bytes = sock->fBytesLeft;

        if (!sock->fNotifyProc || !sock->fNotifyProc(sock, kNotifySocketRead, &notifyRead, &sock->fUserState)) {
            // No callback, or the callback told us to stop reading
            return;
        }

        // if only some of the bytes were used, then shift
        // remaining bytes down.  Otherwise, clear the buffer.
        sock->fBytesLeft -= notifyRead.bytesProcessed;
        if (sock->fBytesLeft != 0) {
            if ((sock->fBytesLeft > sizeof(sock->fBuffer)) || ((notifyRead.bytesProcessed + sock->fBytesLeft) > sizeof(sock->fBuffer))) {
                LogMsg(kLogError, "SocketDispatchRead error: {} {} {}",
                       sock->fBytesLeft, notifyRead.bytes, notifyRead.bytesProcessed);
                return;
            }

            if (notifyRead.bytesProcessed) {
                memmove(sock->fBuffer, sock->fBuffer + notifyRead.bytesProcessed,
                        sock->fBytesLeft);
            }
        }

        SocketStartAsyncRead(sock);
    });
}

static void SocketGetAddresses(AsyncSocket sock, plNetAddress* localAddr,
                               plNetAddress* remoteAddr)
{
    // don't have to enter critsect or validate socket before referencing it
    // because this routine is called before the user has a chance to close it

    asio::error_code err;
    tcp::endpoint    localEndpoint = sock->fSock.local_endpoint(err);
    if (err) {
        LogMsg(kLogError, "Failed to get local socket endpoint: {}", err.message());
    } else if (localEndpoint.address().is_v4()) {
        localAddr->SetHost(localEndpoint.address().to_v4().to_bytes());
        localAddr->SetPort(localEndpoint.port());
    } else {
        localAddr->Clear();
        LogMsg(kLogError, "Local socket endpoint is not IPv4");
    }

    tcp::endpoint remoteEndpoint = sock->fSock.remote_endpoint(err);
    if (err) {
        LogMsg(kLogError, "Failed to get remote socket endpoint: {}", err.message());
    } else if (remoteEndpoint.address().is_v4()) {
        remoteAddr->SetHost(remoteEndpoint.address().to_v4().to_bytes());
        remoteAddr->SetPort(remoteEndpoint.port());
    } else {
        remoteAddr->Clear();
        LogMsg(kLogError, "Remote socket endpoint is not IPv4");
    }
}

static bool SocketInitConnect(ConnectOperation& op)
{
    // This steals ownership of op->fSock
    auto sock = std::make_unique<AsyncSocketStruct>(op);

    sock->initTimeMs = hsTimer::GetMilliSeconds<unsigned>();

    asio::error_code err;
    sock->fSock.non_blocking(true, err);
    if (err)
        LogMsg(kLogError, "Failed to set socket to non-blocking: {}", err.message());

    // Send initial data
    if (!op.fConnectBuffer.empty()) {
        if (!AsyncSocketSend(sock.get(), op.fConnectBuffer.data(), op.fConnectBuffer.size()))
            return false;
    }

    if (!IS_TEXT_CONNTYPE(op.fConnectionType)
        && op.fConnectBuffer.size() < sizeof(AsyncSocketConnectPacket)) {
        return false;
    }

    // perform callback notification
    AsyncNotifySocketConnect notify;
    SocketGetAddresses(sock.get(), &notify.localAddr, &notify.remoteAddr);
    notify.param        = sock->fParam;
    notify.connType     = sock->fConnectionType;
    if (!sock->fNotifyProc(sock.get(), kNotifySocketConnectSuccess, &notify, &sock->fUserState))
        return false;

    // start reading from the socket
    SocketStartAsyncRead(sock.release());
    return true;
}

void AsyncSocketConnect(AsyncCancelId* cancelId, const plNetAddress& netAddr,
                        FAsyncNotifySocketProc notifyProc, void* param,
                        const void* sendData, unsigned sendBytes)
{
    ASSERT(notifyProc);
    ASSERT(s_ioPool);

    PerfAddCounter(kAsyncPerfSocketConnAttemptsOutCurr, 1);
    PerfAddCounter(kAsyncPerfSocketConnAttemptsOutTotal, 1);

    // Locking the critical section here will not deadlock because `asio::basic_socket::async_connect`
    // will return immediately. Therefore, the lock in the callback cannot deadlock us.
    hsLockGuard(s_connectCrit);
    auto op = s_connectList.emplace(s_connectList.end(), s_ioPool->fContext);
    op->fRemoteAddr = netAddr;
    op->fNotifyProc = std::move(notifyProc);
    op->fParam = param;
    if (sendBytes) {
        auto sendBuffer = reinterpret_cast<const uint8_t*>(sendData);
        op->fConnectBuffer.assign(sendBuffer, sendBuffer + sendBytes);
        op->fConnectionType = sendBuffer[0];
    } else {
        op->fConnectionType = kConnTypeNil;
    }

    if (cancelId) {
        // get cancel id; we can avoid checking for zero by always using an odd number
        ASSERT(s_nextConnectCancelId & 1);
        s_nextConnectCancelId += 2;

        *cancelId = op->fCancelId = (AsyncCancelId)s_nextConnectCancelId;
    }

    tcp::endpoint remoteAddr(asio::ip::address_v4(netAddr.GetHostBytes()),
                             netAddr.GetPort());
    op->fSock.async_connect(remoteAddr, [op](const asio::error_code& connError) {
        // This operation is no longer a candidate for cleanup or cancellation
        // by the time we get a callback from async_connect
        bool success;
        if (connError) {
            LogMsg(kLogError, "Failed to connect: {}", connError.message());
            success = false;
        } else {
            success = SocketInitConnect(*op);
        }

        if (!success) {
            AsyncNotifySocketConnect failed;
            failed.param = op->fParam;
            failed.connType = op->fConnectBuffer[0];
            failed.remoteAddr = op->fRemoteAddr;
            failed.localAddr.Clear();
            op->fNotifyProc(nullptr, kNotifySocketConnectFailed, &failed, nullptr);
        }

        {
            hsLockGuard(s_connectCrit);
            s_connectList.erase(op);
        }

        PerfSubCounter(kAsyncPerfSocketConnAttemptsOutCurr, 1);
    });
}

void AsyncSocketConnectCancel(AsyncCancelId cancelId)
{
    hsLockGuard(s_connectCrit);
    for (ConnectOperation& op : s_connectList) {
        if (op.fCancelId != cancelId)
            continue;

        asio::error_code err;
        op.fSock.cancel(err);  // This will wake up async_connect()
        if (err)
            LogMsg(kLogError, "Failed to cancel socket connect operation: {}", err.message());
    }
}

void AsyncSocketDelete(AsyncSocket conn)
{
    delete conn;
}

static void HardCloseSocket(AsyncSocket conn)
{
    asio::error_code err;
    conn->fSock.set_option(asio::socket_base::linger(true, 0), err);
    if (err)
        LogMsg(kLogError, "Failed to set socket linger option: {}", err.message());

    conn->fSock.close(err);
    if (err)
        LogMsg(kLogError, "Failed to close socket: {}", err.message());
}

void AsyncSocketDisconnect(AsyncSocket conn, bool hardClose)
{
    if (!conn->fSock.is_open())
        return;

    asio::error_code err;
    conn->fSock.cancel(err);
    if (err)
        LogMsg(kLogError, "Failed to cancel pending operations: {}", err.message());

    hsAssert(hardClose, "Only hardClose supported on sockets");
    HardCloseSocket(conn);
    conn->closeTimeMs |= 1;
}

static bool SocketQueueAsyncWrite(AsyncSocket conn, const void* data, size_t bytes)
{
    hsLockGuard(s_connectCrit);

    // check for data backlog
    if (!conn->fWriteOps.empty()) {
        WriteOperation* firstQueuedWrite = conn->fWriteOps.front();
        unsigned currTimeMs = hsTimer::GetMilliSeconds<unsigned>();
        if (((long)(currTimeMs - firstQueuedWrite->queueTimeMs) >= (long)kBacklogFailMs) && ((long)(currTimeMs - conn->initTimeMs) >= (long)kBacklogInitMs)) {
            PerfAddCounter(kAsyncPerfSocketDisconnectBacklog, 1);

            if (conn->fConnectionType) {
                LogMsg(
                    kLogPerf,
                    "Backlog, c:{} q:{}, i:{}",
                    conn->fConnectionType,
                    currTimeMs - firstQueuedWrite->queueTimeMs,
                    currTimeMs - conn->initTimeMs);
            }
            AsyncSocketDisconnect((AsyncSocket)conn, true);
            return false;
        }
    }

    // If the last buffer still has space available then add data to it
    if (!conn->fWriteOps.empty()) {
        WriteOperation* op = conn->fWriteOps.back();
        size_t bytesLeft = std::min(op->fAllocSize - op->fNotify.bytes, bytes);
        if (bytesLeft) {
            PerfAddCounter(kAsyncPerfSocketBytesWaitQueued, bytesLeft);
            memcpy(op->fNotify.buffer + op->fNotify.bytes, data, bytesLeft);
            op->fNotify.bytes += bytesLeft;
            data = (const uint8_t*)data + bytesLeft;
            bytes -= bytesLeft;
        }
    }

    if (bytes) {
        // Allocate storage alongside the operation structure itself
        size_t bytesAlloc = std::max(bytes, kMinBacklogBytes);
        auto membuf = new uint8_t[sizeof(WriteOperation) + bytesAlloc];

        WriteOperation* op = new (membuf) WriteOperation;
        conn->fWriteOps.emplace_back(op);

        op->fAllocSize = bytesAlloc;
        op->fNotify.buffer = membuf + sizeof(WriteOperation);
        op->fNotify.bytes = bytes;
        op->fNotify.bytesProcessed = 0;
        op->queueTimeMs = hsTimer::GetMilliSeconds<unsigned>();
        memcpy(op->fNotify.buffer, data, bytes);

        PerfAddCounter(kAsyncPerfSocketBytesWaitQueued, bytes);
    }

    std::vector<asio::const_buffer> allWrites;
    allWrites.reserve(conn->fWriteOps.size());
    for (WriteOperation* op : conn->fWriteOps) {
        if (op->fNotify.bytes - op->fNotify.bytesCommitted > 0) {
            allWrites.emplace_back(op->AsBuffer());
            op->fNotify.bytesCommitted = op->fNotify.bytes;
        }
    }
    async_write(conn->fSock, allWrites, [conn](const asio::error_code& err, size_t bytes) {
        hsLockGuard(s_connectCrit);
        while (bytes != 0) {
            hsAssert(conn->fWriteOps.size() > 0, "buffer mismatch");
            WriteOperation* op = conn->fWriteOps.front();

            size_t opBytesWritten = std::min(bytes, (size_t)op->fNotify.bytes - op->fNotify.bytesProcessed);
            op->fNotify.bytesProcessed += opBytesWritten;
            bytes -= opBytesWritten;
            if (op->fNotify.bytes == op->fNotify.bytesProcessed) {
                conn->fWriteOps.pop_front();
            }
        }
    });

    return true;
}

bool AsyncSocketSend(AsyncSocket conn, const void* data, size_t bytes)
{
    ASSERT(conn);
    ASSERT(data);
    ASSERT(bytes);

    hsLockGuard(conn->fCritsect);

    // This is why we set the socket to non-blocking... If we can write the
    // data right away, we do so in order to save extra allocations for the
    // write buffer.  Otherwise, we must queue it for an async write below.
    if (conn->fWriteOps.empty()) {
        asio::error_code err;
        size_t           bytesSent = conn->fSock.write_some(asio::buffer(data, bytes), err);
        if (!err) {
            // All data was written, nothing left to do!
            if (bytesSent >= bytes)
                return true;

            // Skip the data we already sent, in order to queue it below
            data = (const uint8_t*)data + bytesSent;
            bytes -= bytesSent;
        } else if (err != asio::error::would_block) {
            LogMsg(kLogError, "Failed to write data to socket: {}", err.message());
            AsyncSocketDisconnect(conn, true);
        }
    }

    return SocketQueueAsyncWrite(conn, data, bytes);
}

void AsyncSocketEnableNagling(AsyncSocket conn, bool enable)
{
    hsLockGuard(conn->fCritsect);
    asio::error_code err;
    if (conn->fSock.is_open()) {
        conn->fSock.set_option(tcp::no_delay(!enable), err);
        if (err)
            LogMsg(kLogError, "Failed to set nagling option: {}", err.message());
    }
}
