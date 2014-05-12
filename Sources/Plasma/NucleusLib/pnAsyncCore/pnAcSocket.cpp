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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCore/pnAcSocket.cpp
*   
***/

#include "pnAcSocket.h"
#include "pnAcInt.h"
#include "pnAcLog.h"
#include "pnAcCore.h"
#include "hsThread.h"
#include "pnUtils/pnUtils.h"
#include "plSockets/plNet.h"
#include "plSockets/plTcpSocket.h"
#include "inc/hsTimer.h"

/*****************************************************************************
*
*   private
*   
***/

struct AsyncSocket::Private : AsyncSocket {
    struct Thread;
    struct Operation;
    struct SendBaseOp;
    struct ConnectOp;
    struct SendOp;
    struct WriteOp;
    struct ReadOp;
    
    static Thread *   fThread;
    
    plTcpSocket     socket;
    FNotifyProc     notifyProc;
    bool            close;
    bool            hardClose;
    std::mutex      sockLock;
    // SendOp fields
    SendBaseOp *    lastSend; // chained send operation list (to keep send order)
    std::mutex      lastSendLock;
    // ReadOp fields
    bool            reading;

    static void Add (Operation * op);
    
    Private (plTcpSocket, FNotifyProc);
    ~Private ();
    
    void ReadEnd ();
    void SendEnd ();
};
AsyncSocket::Private::Thread *  AsyncSocket::Private::fThread = nullptr;

struct AsyncSocket::Private::Operation : IWorkerThreads::Operation {
    enum Mode {
        kRead     = 0,
        kWrite    = 1,
        kCancel, // do not use this value when initialise an operation. sockMode is set to this on timeout
    };
    
    SOCKET                  hSocket;
    Mode                    sockMode;
    uint32_t                timeout; // set to 0 to cancel
    
    Operation(SOCKET s, Mode m, uint32_t t) : hSocket(s), sockMode(m), timeout(t) {}
};

//===========================================================================
AsyncSocket::Private::Private (plTcpSocket s, FNotifyProc p) : notifyProc(p), close(false), hardClose(false), lastSend(nullptr), reading() {
    PerfAddCounter(kAsyncPerfSocketsCurr, 1);
    PerfAddCounter(kAsyncPerfSocketsTotal, 1);
}

//===========================================================================
AsyncSocket::Private::~Private () {
    PerfSubCounter(kAsyncPerfSocketsCurr, 1);
}


/*****************************************************************************
*
*   Non blocking I/O operation thread
*   
***/

// thread that wait for I/O operation to be non-blocking.
struct AsyncSocket::Private::Thread : hsThread {
    Operation *         opList; // chained operation list on IWorkerThreads::Operation::next
    std::mutex          listLock;
    hsBinarySemaphore   listSem;

    // ConnectOp fields
    ConnectOp *     coList; // chained operation list on ConnectOp::next (for cancelation)
    std::mutex      coLock;
    
    Thread () : opList(nullptr), coList(nullptr) {}
    
    void Run();
    void Stop ();
    void ListFds (fd_set (&fds)[2]);
};

//===========================================================================
void AsyncSocket::Private::Thread::Run() {
    while (!GetQuit()) {
        fd_set fds[2];
        ListFds(fds);
        
        // wait for connection or timeout
        struct timeval timeout = { 0, 250*1000 }; // seconds, microseconds
        switch (select(0, &fds[0], &fds[1], 0, &timeout)) {
        case -1: LogMsg(kLogError, "socket select failed");
        case  0: continue;
        };

        std::lock_guard<std::mutex> lock(listLock);
        for (Operation * op = opList, ** old = &opList; op; op = *old) {
            if (FD_ISSET(op->hSocket, fds + op->sockMode)) {
                *old = (Operation *) op->next;
                IWorkerThreads::Add(op);
            } else
                old = (Operation **) &op->next;
        }
    }

    // cleanup all ConnectOp
    std::lock_guard<std::mutex> lock(listLock);
    for (Operation * op = opList, * next; op; op = next) {
        next = (Operation *) op->next;
        op->sockMode = Operation::kCancel;
        IWorkerThreads::Add(op);
    }
    opList = nullptr;
}


//===========================================================================
void AsyncSocket::Private::Thread::ListFds (fd_set (& fds)[2]) {
    FD_ZERO(fds+0);
    FD_ZERO(fds+1);

    bool haveOp = false;
    while (true) {
        uint32_t currTime = hsTimer::GetPrecTickCount();
        
        {
            std::lock_guard<std::mutex> lock(listLock);
            for (Operation * op = opList, ** old = &opList; op; op = *old) {
                if (op->timeout != kPosInfinity32 && currTime > op->timeout) {
                    *old = (Operation *) op->next; // unlink
                    op->sockMode = Operation::kCancel;
                    IWorkerThreads::Add(op);
                    continue;
                }
                old = (Operation **) &op->next;
                
                #pragma warning(disable:4127) // ignore "do { } while (0)" warning
                FD_SET(op->hSocket, fds + op->sockMode);
                #pragma warning(default:4127)
                
                haveOp = true;
            }
        }
        
        if (haveOp || GetQuit())
            break;
        
        listSem.Wait();
    }
}

//===========================================================================
void AsyncSocket::Private::Add (Operation * op) {
    ASSERT(fThread);
    
    if (op->timeout != kPosInfinity32)
        op->timeout += hsTimer::GetPrecTickCount();
    
    {
        std::lock_guard<std::mutex> lock(fThread->listLock);
        op->next = fThread->opList;
        fThread->opList = op;
    }
    
    fThread->listSem.Signal();
}

//===========================================================================
void AsyncSocket::Private::Thread::Stop () {
    SetQuit(true);
    listSem.Signal();
    hsThread::Stop();
}


/****************************************************************************
*
*   Socket read
*
***/

struct AsyncSocket::Private::ReadOp : AsyncSocket::Private::Operation {
    Private *   sock;
    uint8_t     buffer[kBufferSize];
    unsigned    bytesUsed;

    ReadOp (Private * s) : Operation(s->socket.GetSocket(), Mode::kRead, kPosInfinity32), sock(s), bytesUsed(0) {}
    void Callback ();
};

//===========================================================================
void AsyncSocket::Private::ReadOp::Callback() {
    ssize_t size;
    bool active;
    {
        std::lock_guard<std::mutex> lock(sock->sockLock);
        if (active = sock->Active()) {
            size = sock->socket.RecvData((char *) buffer + bytesUsed, sizeof(buffer) - bytesUsed);
            if (size == -1 && plNet::GetError() == kBlockingError)
                    size = 0;
        }
    }
    
    if (!active) { // closing
        sock->ReadEnd();
        delete this;
        return;
    }

    if (size == 0) { // pending
        Private::Add(this);
        return;
    }
    if (size == -1) { // error
        sock->reading = false;
        sock->Disconnect();
        delete this;
        return;
    }

    NotifyRead notify(buffer, size + bytesUsed);

    if (!sock->notifyProc(sock, kNotifyRead, &notify))
        sock->Disconnect();

    {
        std::lock_guard<std::mutex> lock(sock->sockLock);
        if (sock->Active()) {
            if (notify.bytesProcessed < notify.bytes)
                memmove(buffer, buffer + notify.bytesProcessed, bytesUsed = notify.bytes - notify.bytesProcessed);
            if (bytesUsed >= sizeof(buffer))
                ; //TODO
            Private::Add(this);
            return;
        }
    }

    sock->reading = false;
    sock->ReadEnd();
    delete this;
}


/*****************************************************************************
*
*   Socket connect
*
***/

struct AsyncSocket::Private::ConnectOp : AsyncSocket::Private::Operation {
    ConnectOp *             next;
    ConnectOp *             prev;
    plTcpSocket             socket;
    
    plNetAddress            remoteAddr;
    
    FNotifyProc             notifyProc;
    void *                  param;
    unsigned                sendBytes;
    uint8_t                 sendData[1]; // must be the last field

    void Callback ();
    void operator delete (void * ptr) throw() { free(ptr); }
    
    ConnectOp(const plNetAddress& a, FNotifyProc n, void* p, unsigned s, unsigned t)
     : Operation(plNet::NewTCP(), Private::Operation::kWrite, t ? hsTimer::PrecSecsToTicks(t * 1.e3f) : kPosInfinity32),
       next(), prev(nullptr), socket(), remoteAddr(a), notifyProc(n),
       param(p), sendBytes(s) {}
};

//===========================================================================
AsyncSocket::Cancel AsyncSocket::Connect (
    const plNetAddress&     netAddr,
    FNotifyProc             notifyProc,
    void *                  param,
    const void *            sendData,
    unsigned                sendBytes,
    unsigned                connectMs,
    uint16_t                localPort
) {
    ASSERT(notifyProc);

    // create async connection record with enough extra bytes for sendData
    Private::ConnectOp * op;
    if (sendBytes) {
        op = new(malloc(sizeof(Private::ConnectOp) + sendBytes - 1)) Private::ConnectOp(
            netAddr, notifyProc, param, sendBytes, connectMs
        );
        memcpy(op->sendData, sendData, sendBytes);
    } else {
         // All ConnectOp must be allocated with malloc!
        op = new(malloc(sizeof(Private::ConnectOp))) Private::ConnectOp(
            netAddr, notifyProc, param, sendBytes, connectMs
        );
        op->sendData[0] = kConnTypeNil;
    }

    // check created socket
    if (op->hSocket == kBadSocket) {
        LogMsg(kLogError, "create socket failed: %s", plNet::GetErrorMsg(plNet::GetError()));
        delete op;
        return Cancel(nullptr);
    }
    
    // bind socket to local port
    op->socket.CloseOnDestroy(true);
    op->socket.SetSocket(op->hSocket);
    if (localPort && plNet::Bind(op->hSocket, &plNetAddress(localPort).GetAddressInfo())) {
        LogMsg(kLogError, "bind(port %u) failed: %s", (unsigned)localPort, plNet::GetErrorMsg(plNet::GetError()));
        delete op;
        return Cancel(nullptr);
    }

    // connect
    op->socket.SetBlocking(false);
    if(!plNet::Connect(op->hSocket, &netAddr.GetAddressInfo()) && plNet::GetError() != kBlockingError) {
        LogMsg(kLogError, "socket connect failed: %s", plNet::GetErrorMsg(plNet::GetError()));
        delete op;
        return Cancel(nullptr);
    }
    op->socket.CloseOnDestroy(false);
    
    // ioThread initialisation
    if (!Private::fThread) {
        Private::fThread = new Private::Thread();
        Private::fThread->Start();
    }
    
    // coList update
    {
        std::lock_guard<std::mutex> lock(Private::fThread->coLock);
        op->next = Private::fThread->coList;
        if (op->next)
            op->next->prev = op;
        Private::fThread->coList = op;
    }

    // counters update
    PerfAddCounter(kAsyncPerfSocketConnAttemptsOutCurr, 1);
    PerfAddCounter(kAsyncPerfSocketConnAttemptsOutTotal, 1);

    Private::Add(op);
    
    return Cancel(op);
}

//===========================================================================
bool AsyncSocket::Cancel::ConnectCancel () {
    std::lock_guard<std::mutex> lock(Private::fThread->coLock);
    
    for (Private::ConnectOp * op = Private::fThread->coList; op; op = op->next) {
        if (ptr == op) {
            ptr = nullptr;
            op->timeout = 0;
            Private::fThread->listSem.Signal();
            return true; // cancelId can be found only once!
        }
    }
    ptr = nullptr;
    return false;
}

//===========================================================================
void AsyncSocket::ConnectCancel(FNotifyProc notifyProc) {
    std::lock_guard<std::mutex> lock(Private::fThread->coLock);
    
    for (Private::ConnectOp * op = Private::fThread->coList; op; op = op->next)
        if (op->notifyProc == notifyProc)
            op->timeout = 0;
    Private::fThread->listSem.Signal();
}

//===========================================================================
bool AsyncSocket::Active() {
    Private * sock = static_cast<Private *>(this);
    return !sock->close && sock->socket.Active();
}

//===========================================================================
void AsyncSocket::Private::ConnectOp::Callback () {
    {
        std::lock_guard<std::mutex> lock(fThread->coLock);
        // revmove this from coList
        prev ? prev->next : fThread->coList = next;
        if (next)
            next->prev = prev;
    }

    //ASSERT(!op->signalComplete);
    PerfSubCounter(kAsyncPerfSocketConnAttemptsOutCurr, 1);

    AsyncSocket::NotifyConnect notify(param, (EConnType) sendData[0]);

    do {
        if (sockMode == kCancel)
            break;
        
        Private * sock = new Private(socket, notifyProc);
        
        socket.CloseOnDestroy(true);
        if (sendBytes)
            if (!sock->Send(sendData, sendBytes)) {
                delete sock;
                break;
            }

        // TODO: move this to plSocket
        socklen_t len = sizeof(AddressType);
        if (getsockname(sock->socket.GetSocket(), (sockaddr *) &notify.localAddr, &len))
            LogMsg(kLogError, "getsockname failed");
        len = sizeof(AddressType);
        if (getpeername(sock->socket.GetSocket(), (sockaddr *) &notify.remoteAddr, &len))
            LogMsg(kLogError, "getsockname failed");

        sock->reading = true; // block socket deletion
        if (!notifyProc(sock, kNotifyConnectSuccess, &notify)) {
            sock->Disconnect();
            sock->ReadEnd();
        } else {
            ReadOp * read = new ReadOp(sock);
            
            bool readAdded;
            {
                std::lock_guard<std::mutex> lock(sock->sockLock);
                if (readAdded = *sock)
                    Private::Add(read);
            }
            if (!readAdded) {
                delete read;
                sock->ReadEnd();
            }
        }

        delete this;
        return;
    } while (0);

    // handle connection failure
    notify.remoteAddr = remoteAddr;
    notify.localAddr.Clear();
    notifyProc(nullptr, kNotifyConnectFailed, &notify);

    socket.Close();
    delete this;
}


/*****************************************************************************
*
*   Socket send/write
*
***/

struct AsyncSocket::Private::SendBaseOp : AsyncSocket::Private::Operation {
    SendBaseOp *                next;
    Private *                   sock;
    unsigned                    sendBytes;
    const uint8_t *             sendData;

    SendBaseOp(Private * s, unsigned l, const uint8_t* d)
     : Operation(s->socket.GetSocket(), kWrite, -1),
       next(nullptr), sock(s), sendBytes(l), sendData(d) {}
    
    void Callback();
    virtual bool Notify() = 0;
};
struct AsyncSocket::Private::SendOp : AsyncSocket::Private::SendBaseOp {
    uint8_t                     data[1]; // must be the last field

    SendOp(Private * s, unsigned l) : SendBaseOp(s, l, data) {}
    bool Notify() { return true; }
    void operator delete (void * ptr) throw() { free(ptr); }
};
struct AsyncSocket::Private::WriteOp : AsyncSocket::Private::SendBaseOp {
    void *                      param;
    unsigned                    bytes;
    const uint8_t *             buffer;

    WriteOp(Private * s, void * p, unsigned l, const uint8_t* b)
     : SendBaseOp(s, l, b), param(p), bytes(l), buffer(b) {}
    bool Notify();
};

//===========================================================================
bool AsyncSocket::Send (
    const void *            data,
    unsigned                bytes
) {
    ASSERT(data);
    ASSERT(bytes);
    if (!Active())
        return false;

    Private * sock = static_cast<Private *>(this);
    Private::SendOp * op;
    op = new(malloc(sizeof(Private::SendOp) + bytes - 1)) Private::SendOp(sock, bytes);

    memcpy(op->data, data, bytes);

    bool pending;
    {
        std::lock_guard<std::mutex> lock(sock->lastSendLock);
        pending = sock->lastSend;
        if (pending)
            sock->lastSend->next = op;
        sock->lastSend = op;
    }

    if (pending)
        PerfAddCounter(kAsyncPerfSocketBytesWaitQueued, bytes);
    else {
        PerfAddCounter(kAsyncPerfSocketBytesWriteQueued, bytes);
        IWorkerThreads::Add(op);
    }

    return true;
}

//===========================================================================
bool AsyncSocket::Write (
    const void *            buffer,
    unsigned                bytes,
    void *                  param
) {
    ASSERT(buffer);
    ASSERT(bytes);
    if (!Active())
        return false;
    
    Private * sock = static_cast<Private *>(this);
    Private::WriteOp * op = new Private::WriteOp(
        sock, param, bytes, static_cast<const uint8_t*>(buffer)
    );
    
    bool pending;
    {
        std::lock_guard<std::mutex> lock(sock->lastSendLock);
        pending = sock->lastSend;

        if (pending)
            sock->lastSend->next = op;
        sock->lastSend = op;
    }

    if (pending)
        PerfAddCounter(kAsyncPerfSocketBytesWaitQueued, bytes);
    else {
        PerfAddCounter(kAsyncPerfSocketBytesWriteQueued, bytes);
        IWorkerThreads::Add(op);
    }

    return true;
}

//===========================================================================
void AsyncSocket::Private::SendBaseOp::Callback() {
    size_t size = static_cast<size_t>(-1);
    {
        std::lock_guard<std::mutex> lock(sock->sockLock);
        if (sock->hardClose)
            sockMode = kCancel;
        else if (sendData) {
            size = sock->socket.SendData((const char*)sendData, sendBytes);
            if (size == -1 && plNet::GetError() == kBlockingError)
                size = 0; // not an error
        }
    }

    if (sockMode == kCancel) { // cancelled
        if (*sock) {
            {
                std::lock_guard<std::mutex> lock(sock->lastSendLock);
                if (!next)
                    sock->lastSend = nullptr;
            }
            if (next)
                Private::Add(next);
        }
        else if (next)
            IWorkerThreads::Add(next);
        else
            sock->SendEnd();

        Notify();
        delete this;
        return;
    }

    if (size == 0) { // delayed
        //Notify();
        Private::Add(this);
        return;
    }

    if (size == -1) { // error
        if (*sock)
            sock->Disconnect();
        {
            std::lock_guard<std::mutex> lock(sock->lastSendLock);
            if (!next)
                sock->lastSend = nullptr;
        }
        if (next)
            IWorkerThreads::Add(next);

        Notify();
        delete this;
        return;
    }

    PerfSubCounter(kAsyncPerfSocketBytesWriteQueued, size);
    if (size >= sendBytes) {
        // operation end
        ASSERT(sendBytes == size);
        {
            std::lock_guard<std::mutex> lock(sock->lastSendLock);
            if (!next)
                sock->lastSend = nullptr;
        }
        // next operation
        if (next) {
            PerfSubCounter(kAsyncPerfSocketBytesWaitQueued, next->sendBytes);
            PerfAddCounter(kAsyncPerfSocketBytesWriteQueued, next->sendBytes);
            IWorkerThreads::Add(next);
        }

        if (!Notify())
            sock->Disconnect();
        else if (!next && !*sock)
            sock->SendEnd();
        delete this;
        return;
    }

    // operation not completted:
    sendData += size;
    sendBytes -= size;

    Private::Add(this);
}

//===========================================================================
bool AsyncSocket::Private::WriteOp::Notify() {
    AsyncSocket::NotifyWrite notify(param, buffer, bytes, bytes - sendBytes);
    //notify.asyncId           = op->asyncId;
    
    return sock->notifyProc(sock, kNotifyWrite, &notify);
}


/*****************************************************************************
*
*   Socket misc functions
*
***/

//===========================================================================
void AsyncSocket::SetNotifyProc (FNotifyProc  notifyProc) {
    ASSERT(notifyProc);
    static_cast<Private*>(this)->notifyProc = notifyProc;
}

//===========================================================================
void AsyncSocket::EnableNagling(bool nagling) {
    int result;
    Private * sock = static_cast<Private*>(this);
    {
        std::lock_guard<std::mutex> lock(sock->sockLock);
        if (!Active())
            return;
        
        sock->socket.SetNoDelay(!nagling);
    }
    if (result)
        LogMsg(kLogError, "setsockptr failed (nagling): %s", plNet::GetErrorMsg(plNet::GetError()));
}

//===========================================================================
void AsyncSocket::Disconnect (bool hardClose) {
    Private * sock = static_cast<Private*>(this);
    {
        std::lock_guard<std::mutex> lock(sock->sockLock);
        if (!Active() && (!hardClose || sock->hardClose))
            return; // already closed...
        sock->close = true;
        sock->hardClose = hardClose;
    }
    
    SOCKET handle = sock->socket.GetSocket();
    if (sock->hardClose)
        sock->socket.Close();

    // cancel pending operations
    std::lock_guard<std::mutex> lock(Private::fThread->listLock);
    for (Private::Operation * op = Private::fThread->opList; op; op = (Private::Operation *) op->next) {
        if (op->hSocket == handle)
            op->timeout = 0;
    }
    
    Private::fThread->listSem.Signal();
}

//===========================================================================
void AsyncSocket::Delete () {
    delete static_cast<Private*>(this);
}

//===========================================================================
void AsyncSocket::Private::SendEnd() {
    if (reading) {
        lastSend = nullptr;
        return;
    }
    {
        std::lock_guard<std::mutex> lock(sockLock);
        if (!hardClose) {
            hardClose = true;
            Disconnect();
        }
    }
    notifyProc(this, kNotifyDisconnect, nullptr);
}

//===========================================================================
void AsyncSocket::Private::ReadEnd() {
    if (lastSend) {
        reading = false;
        return;
    }
    {
        std::lock_guard<std::mutex> lock(sockLock);
        if (!hardClose) {
            hardClose = true;
            Disconnect();
        }
    }
    notifyProc(this, kNotifyDisconnect, nullptr);
}

//===========================================================================
void SocketDestroy() {
    if (AsyncSocket::Private::fThread)
        AsyncSocket::Private::fThread->Stop();
}
