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
#if ! HS_BUILD_FOR_WIN32
    #include <netinet/tcp.h> // TCP_NODELAY
#endif

/*****************************************************************************
*
*   private
*   
***/

struct AsyncSocket::P : AsyncSocket {
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
    
    P ();
    ~P ();
    
    void ReadEnd ();
    void SendEnd ();
};
AsyncSocket::P::Thread *  AsyncSocket::P::fThread = nullptr;

struct AsyncSocket::P::Operation : IWorkerThreads::Operation {
    enum Mode {
        kRead     = 0,
        kWrite    = 1,
        kCancel, // do not use this value when initialise an operation. sockMode is set to this on timeout
    };
    
    SOCKET                  hSocket;
    Mode                    sockMode;
    uint32_t                timeout; // set to 0 to cancel
};

//===========================================================================
AsyncSocket::P::P () : close(false), hardClose(false), lastSend(nullptr), reading(false) {
    PerfAddCounter(kAsyncPerfSocketsCurr, 1);
    PerfAddCounter(kAsyncPerfSocketsTotal, 1);
}

//===========================================================================
AsyncSocket::P::~P () {
    PerfSubCounter(kAsyncPerfSocketsCurr, 1);
}


/*****************************************************************************
*
*   Non blocking I/O operation thread
*   
***/

// thread that wait for I/O operation to be non-blocking.
struct AsyncSocket::P::Thread : hsThread {
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
void AsyncSocket::P::Thread::Run() {
    while (!GetQuit()) {
        fd_set fds[2];
        ListFds(fds);
        
        // wait for connection or timeout
        // TODO: move code to plSocket/plNet
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
void AsyncSocket::P::Thread::ListFds (fd_set (& fds)[2]) {
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
void AsyncSocket::P::Add (Operation * op) {
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
void AsyncSocket::P::Thread::Stop () {
    SetQuit(true);
    listSem.Signal();
    hsThread::Stop();
}


/****************************************************************************
*
*   Socket read
*
***/

struct AsyncSocket::P::ReadOp : AsyncSocket::P::Operation {
    AsyncSocket::P *            sock;
    uint8_t                     buffer[kBufferSize];
    unsigned                    bytesUsed;

    ReadOp () : bytesUsed(0) {}
    void Callback ();
};

//===========================================================================
void AsyncSocket::P::ReadOp::Callback() {
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
        P::Add(this);
        return;
    }
    if (size == -1) { // error
        sock->reading = false;
        sock->Disconnect();
        delete this;
        return;
    }

    NotifyRead notify;

    notify.param            = nullptr;
    //notify.asyncId          = 0;
    notify.buffer           = buffer;
    notify.bytes            = size + bytesUsed;
    notify.bytesProcessed   = 0;
    if (!sock->notifyProc(sock, kNotifyRead, &notify))
        sock->Disconnect();

    {
        std::lock_guard<std::mutex> lock(sock->sockLock);
        if (sock->Active()) {
            if (notify.bytesProcessed < notify.bytes)
                memmove(buffer, buffer + notify.bytesProcessed, bytesUsed = notify.bytes - notify.bytesProcessed);
            if (bytesUsed >= sizeof(buffer))
                ; //TODO
            P::Add(this);
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

struct AsyncSocket::P::ConnectOp : AsyncSocket::P::Operation {
    ConnectOp *             next;
    ConnectOp *             prev;
    plTcpSocket             socket;
    
    //unsigned                localPort;
    plNetAddress            remoteAddr;
    
    FNotifyProc             notifyProc;
    void *                  param;
    unsigned                sendBytes;
    uint8_t                 sendData[1]; // must be the last field

    void Callback ();
    void operator delete (void * ptr) throw() { free(ptr); }
};

//===========================================================================
AsyncSocket::Cancel AsyncSocket::Connect (
    const plNetAddress&     netAddr,
    FNotifyProc             notifyProc,
    void *                  param,
    const void *            sendData,
    unsigned                sendBytes,
    unsigned                connectMs,
    unsigned                localPort
) {
    ASSERT(notifyProc);

    // create async connection record with enough extra bytes for sendData
    P::ConnectOp * op;
    if (sendBytes) {
        op = new(malloc(sizeof(P::ConnectOp) + sendBytes - 1)) P::ConnectOp;
        memcpy(op->sendData, sendData, sendBytes);
    } else {
        op = new(malloc(sizeof(P::ConnectOp))) P::ConnectOp; // All ConnectOp must be allocated with malloc!
        op->sendData[0] = kConnTypeNil;
    }

    // init ConnectOp fields
    op->prev                    = nullptr;
    //op->localPort               = localPort;
    op->remoteAddr              = netAddr;
    op->notifyProc              = notifyProc;
    op->param                   = param;
    op->sendBytes               = sendBytes;
    
    // init Operation fields
    op->hSocket                 = plNet::NewTCP();
    op->sockMode                = P::Operation::kWrite;
    op->timeout                 = connectMs ? hsTimer::PrecSecsToTicks(connectMs * 1.e3f) : kPosInfinity32;


    // check created socket
    if (op->hSocket == kBadSocket) {
        LogMsg(kLogError, "create socket failed: %s", plNet::GetErrorMsg(plNet::GetError()));
        delete op;
        return Cancel(nullptr);
    }
    
    // bind socket to local port
    op->socket.CloseOnDestroy(true);
    op->socket.SetSocket(op->hSocket);
    if (localPort) {
        // TODO: move this to plSocket
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port   = htons((uint16_t) localPort);
        addr.sin_addr.s_addr = INADDR_ANY;
        memset(addr.sin_zero, 0, sizeof(addr.sin_zero));
        if (bind(op->hSocket, (sockaddr *) &addr, sizeof(addr))) {
            LogMsg(kLogError, "bind(port %u) failed: %s", (unsigned)localPort, plNet::GetErrorMsg(plNet::GetError()));
            delete op;
            return Cancel(nullptr);
        }
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
    if (!P::fThread) {
        P::fThread = new P::Thread();
        P::fThread->Start();
    }
    
    // coList update
    {
        std::lock_guard<std::mutex> lock(P::fThread->coLock);
        op->next = P::fThread->coList;
        if (op->next)
            op->next->prev = op;
        P::fThread->coList = op;
    }

    // counters update
    PerfAddCounter(kAsyncPerfSocketConnAttemptsOutCurr, 1);
    PerfAddCounter(kAsyncPerfSocketConnAttemptsOutTotal, 1);

    P::Add(op);
    
    return Cancel(op);
}

//===========================================================================
bool AsyncSocket::Cancel::ConnectCancel () {
    std::lock_guard<std::mutex> lock(P::fThread->coLock);
    
    for (P::ConnectOp * op = P::fThread->coList; op; op = op->next) {
        if (ptr == op) {
            ptr = nullptr;
            op->timeout = 0;
            P::fThread->listSem.Signal();
            return true; // cancelId can be found only once!
        }
    }
    ptr = nullptr;
    return false;
}

//===========================================================================
void AsyncSocket::ConnectCancel(FNotifyProc notifyProc) {
    std::lock_guard<std::mutex> lock(P::fThread->coLock);
    
    for (P::ConnectOp * op = P::fThread->coList; op; op = op->next)
        if (op->notifyProc == notifyProc)
            op->timeout = 0;
    P::fThread->listSem.Signal();
}

//===========================================================================
bool AsyncSocket::Active() {
    return !((P*)this)->close && ((P*)this)->socket.Active();
}

//===========================================================================
void AsyncSocket::P::ConnectOp::Callback () {
    {
        std::lock_guard<std::mutex> lock(fThread->coLock);
        // revmove this from coList
        prev ? prev->next : fThread->coList = next;
        if (next)
            next->prev = prev;
    }

    //ASSERT(!op->signalComplete);
    PerfSubCounter(kAsyncPerfSocketConnAttemptsOutCurr, 1);

    AsyncSocket::NotifyConnect notify;
    notify.param        = param;
    notify.connType     = (EConnType) sendData[0];

    do {
        if (sockMode == kCancel)
            break;
        P * sock            = new P;
        sock->socket        = socket;
        sock->notifyProc    = notifyProc;
        sock->socket.CloseOnDestroy(true);

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
            ReadOp * read = new ReadOp;
            sock->reading = true;
            read->sock = sock;
            
            read->hSocket = hSocket;
            read->sockMode = kRead;
            read->timeout = kPosInfinity32;
            
            std::lock_guard<std::mutex> lock(sock->sockLock);
            if (*sock)
                P::Add(read);
            else {
                delete read;
                sock->ReadEnd();
            }
        }

        delete this;
        return;
    } while (0);

    // handle connection failure
    notify.remoteAddr   = remoteAddr;
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

struct AsyncSocket::P::SendBaseOp : AsyncSocket::P::Operation {
    SendBaseOp *                next;
    AsyncSocket::P *            sock;
    unsigned                    sendBytes;
    uint8_t *                   sendData;

    SendBaseOp() : next(nullptr) {}
    void Callback ();
    virtual bool Notify() = 0;
};
struct AsyncSocket::P::SendOp : AsyncSocket::P::SendBaseOp {
    uint8_t                     data[1]; // must be the last field

    SendOp() { sendData = data; }
    bool Notify() { return true; }
    void operator delete (void * ptr) throw() { free(ptr); }
};
struct AsyncSocket::P::WriteOp : AsyncSocket::P::SendBaseOp {
    void *                      param;
    unsigned                    bytes;
    uint8_t *                   buffer;

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

    P::SendOp * op;
    op = new(malloc(sizeof(P::SendOp) + bytes - 1)) P::SendOp;

    op->hSocket     = ((P*)this)->socket.GetSocket();
    op->sockMode    = P::Operation::kWrite;
    op->timeout     = -1;

    op->sock        = (P*)this;
    op->sendBytes   = bytes;

    memcpy(op->data, data, bytes);

    bool pending;
    {
        std::lock_guard<std::mutex> lock(((P*)this)->lastSendLock);
        pending = ((P*)this)->lastSend;
        if (pending)
            ((P*)this)->lastSend->next = op;
        ((P*)this)->lastSend = op;
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
    P::WriteOp * op = new P::WriteOp;
    op->hSocket     = ((P*)this)->socket.GetSocket();
    op->sockMode    = P::Operation::kWrite;
    op->timeout     = -1;

    op->sock        = (P*)this;
    op->sendBytes   = bytes;
    op->sendData    = (uint8_t *) buffer;

    op->param       = param;
    op->bytes       = bytes;
    op->buffer      = (uint8_t *) buffer;

    bool pending;
    {
        std::lock_guard<std::mutex> lock(((P*)this)->lastSendLock);
        pending = ((P*)this)->lastSend;

        if (pending)
            ((P*)this)->lastSend->next = op;
        ((P*)this)->lastSend = op;
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
void AsyncSocket::P::SendBaseOp::Callback() {
    size_t size;
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
                P::Add(next);
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
        P::Add(this);
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

    P::Add(this);
}

//===========================================================================
bool AsyncSocket::P::WriteOp::Notify() {
    AsyncSocket::NotifyWrite notify;
    notify.param             = param;
    //notify.asyncId           = op->asyncId;
    notify.buffer            = buffer;
    notify.bytes             = bytes;
    notify.bytesProcessed    = bytes - sendBytes;
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
    ((P*)this)->notifyProc = notifyProc;
}

//===========================================================================
void AsyncSocket::EnableNagling(bool nagling) {
    int result;
    {
        std::lock_guard<std::mutex> lock(((P*)this)->sockLock);
        if (!Active())
            return;
        int noDelay = !nagling;
        
        // TODO: move to plSocket/plNet
        result = setsockopt(
            ((P*)this)->socket.GetSocket(),
            IPPROTO_TCP,
            TCP_NODELAY,
            (const char*) &noDelay,
            sizeof(noDelay)
        );
    }
    if (result)
        LogMsg(kLogError, "setsockptr failed (nagling): %s", plNet::GetErrorMsg(plNet::GetError()));
}

//===========================================================================
void AsyncSocket::Disconnect (bool hardClose) {
    {
        std::lock_guard<std::mutex> lock(((P*)this)->sockLock);
        if (!Active() && (!hardClose || ((P*)this)->hardClose))
            return; // already closed...
        ((P*)this)->close = true;
        ((P*)this)->hardClose = hardClose;
    }
    
    SOCKET handle = ((P*)this)->socket.GetSocket();
    if (((P*)this)->hardClose)
        ((P*)this)->socket.Close();

    // cancel pending operations
    std::lock_guard<std::mutex> lock(P::fThread->listLock);
    for (P::Operation * op = P::fThread->opList; op; op = (P::Operation *) op->next) {
        if (op->hSocket == handle)
            op->timeout = 0;
    }
    
    P::fThread->listSem.Signal();
}

//===========================================================================
void AsyncSocket::Delete () {
    delete (P*)this;
}

//===========================================================================
void AsyncSocket::P::SendEnd() {
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
void AsyncSocket::P::ReadEnd() {
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
    if (AsyncSocket::P::fThread)
        AsyncSocket::P::fThread->Stop();
}

// TODO: listen operations!
