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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetDiag/pnNdTcp.cpp
*   
***/

#include "Pch.h"
#pragma hdrstop

/*****************************************************************************
*
*   Local types
*
***/

struct AuthConn : AtomicRef {
    NetDiag *               diag;
    FNetDiagDumpProc        dump;
    FNetDiagTestCallback    callback;
    void *                  param;
    AsyncSocket             sock;
    AsyncCancelId           cancelId;
    NetCli *                cli;
    long                    pingsInRoute;
    long                    pingsCompleted;
    bool                    done;
    ENetError               error;
    
    ~AuthConn ();
};

struct AuthTrans : THashKeyVal<unsigned> {
    HASHLINK(AuthTrans) link;
    AuthConn *  conn;
    unsigned    pingAtMs;

    AuthTrans (AuthConn * conn);    
    ~AuthTrans ();
};

struct FileConn : AtomicRef {
    NetDiag *               diag;
    FNetDiagDumpProc        dump;
    FNetDiagTestCallback    callback;
    void *                  param;
    AsyncSocket             sock;
    AsyncCancelId           cancelId;
    ARRAY(byte)             recvBuffer;
    long                    pingsInRoute;
    long                    pingsCompleted;
    bool                    done;
    ENetError               error;
    
    ~FileConn ();
};

struct FileTrans : THashKeyVal<unsigned> {
    HASHLINK(FileTrans) link;
    FileConn *  conn;
    unsigned    pingAtMs;
    
    FileTrans (FileConn * conn);
    ~FileTrans ();
};



/*****************************************************************************
*
*   Local data
*
***/

static const unsigned kPingTimeoutMs    = 5000;
static const unsigned kTimeoutCheckMs   = 100;
static const unsigned kMaxPings         = 15;   


static long         s_authProtocolRegistered;
static unsigned     s_transId;
static CCritSect    s_critsect;
static bool         s_shutdown;
static byte         s_payload[32];
static AsyncTimer * s_timer;

static HASHTABLEDECL(
    AuthTrans,
    THashKeyVal<unsigned>,
    link
) s_authTrans;

static HASHTABLEDECL(
    FileTrans,
    THashKeyVal<unsigned>,
    link
) s_fileTrans;


/*****************************************************************************
*
*   Cli2Auth protocol
*
***/

//============================================================================
static bool Recv_PingReply (
    const byte      msg[],
    unsigned        bytes,
    void *
) {
    const Auth2Cli_PingReply & reply = *(const Auth2Cli_PingReply *)msg;
    
    AuthTrans * trans;
    s_critsect.Enter();
    {
        if (bytes < sizeof(Auth2Cli_PingReply)) {
            // beta6 compatibility
            if (nil != (trans = s_authTrans.Tail()))
                s_authTrans.Unlink(trans);
        }
        else if (nil != (trans = s_authTrans.Find(reply.transId)))
            s_authTrans.Unlink(trans);
    }
    s_critsect.Leave();

    if (trans) {
        unsigned replyAtMs = TimeGetMs();
        trans->conn->dump(L"[TCP] Reply from SrvAuth. ms=%u", replyAtMs - trans->pingAtMs);
        DEL(trans);
        return true;
    }
    else {
        return false;
    }
}

//============================================================================
#define MSG(s)  kNetMsg_Cli2Auth_##s
static NetMsgInitSend s_send[] = {
    { MSG(PingRequest)              },
};

#undef MSG
#define MSG(s)  kNetMsg_Auth2Cli_##s, Recv_##s
static NetMsgInitRecv s_recv[] = {
    { MSG(PingReply)                },
};
#undef MSG


/*****************************************************************************
*
*   Local functions
*
***/

//============================================================================
static unsigned TimerCallback (void *) {

    unsigned timeMs = TimeGetMs();
    s_critsect.Enter();
    {
        ENetError error = kNetErrTimeout;
        {for (AuthTrans * next, * curr = s_authTrans.Head(); curr; curr = next) {
            next = s_authTrans.Next(curr);
            unsigned diff = timeMs - curr->pingAtMs;
            if (diff > kPingTimeoutMs) {
                if (!curr->conn->error)
                    curr->conn->error = error;
                curr->conn->dump(L"[TCP] No reply from SrvAuth: %u, %s (ms=%u)", error, NetErrorToString(error), diff);
                DEL(curr);
            }
        }}
        {for (FileTrans * next, * curr = s_fileTrans.Head(); curr; curr = next) {
            next = s_fileTrans.Next(curr);
            unsigned diff = timeMs - curr->pingAtMs;
            if (diff > kPingTimeoutMs) {
                if (!curr->conn->error)
                    curr->conn->error = error;
                curr->conn->dump(L"[TCP] No reply from SrvFile: %u, %s (ms=%u)", error, NetErrorToString(error), diff);
                DEL(curr);
            }
        }}
    }
    s_critsect.Leave();
    
    return kTimeoutCheckMs;
}

//============================================================================
static void AuthPingProc (void * param) {

    AuthConn * conn = (AuthConn *)param;

    while (!conn->done && conn->pingsCompleted < kMaxPings) {

        if (!conn->pingsInRoute) {

            AuthTrans * trans = NEW(AuthTrans)(conn);
            trans->pingAtMs = TimeGetMs();

            s_critsect.Enter();
            for (;;) {
                if (conn->done) {
                    conn->pingsCompleted = kMaxPings;
                    DEL(trans);
                    break;
                }
                while (++s_transId == 0)
                    NULL_STMT;
                trans->SetValue(s_transId);
                s_authTrans.Add(trans);

                const unsigned_ptr msg[] = {
                    kCli2Auth_PingRequest,
                                    trans->pingAtMs,
                                    trans->GetValue(),
                                    sizeof(s_payload),
                    (unsigned_ptr)  s_payload,
                };
                
                NetCliSend(conn->cli, msg, arrsize(msg));
                NetCliFlush(conn->cli);
                break;
            }
            s_critsect.Leave();
        }
        AsyncSleep(10);
    }

    s_critsect.Enter();
    {   
        conn->done = true;
        AsyncSocketDisconnect(conn->sock, true);
        NetCliDelete(conn->cli, false);
        conn->cli = nil;
    }
    s_critsect.Leave();
        
    conn->DecRef("Pinging");
}

//============================================================================
static bool AuthConnEncrypt (ENetError error, void * param) {

    AuthConn * conn = (AuthConn *)param;
    
    if (IS_NET_SUCCESS(error)) {
        conn->dump(L"[TCP] SrvAuth stream encrypted.");
        conn->dump(L"[TCP] Pinging SrvAuth with 32 bytes of data...");
        conn->IncRef("Pinging");
        _beginthread(AuthPingProc, 0, conn);
    }
    else {
        conn->dump(L"[TCP] SrvAuth stream encryption failed: %u, %s", error, NetErrorToString(error));
    }
    
    return IS_NET_SUCCESS(error);
}

//============================================================================
static void NotifyAuthConnSocketConnect (AuthConn * conn) {

    conn->dump(L"[TCP] SrvAuth socket established, encrypting stream...");
    
    conn->TransferRef("Connecting", "Connected");
    conn->cli = NetCliConnectAccept(
        conn->sock,
        kNetProtocolCli2Auth,
        false,
        AuthConnEncrypt,
        0,
        nil,
        conn
    );
}

//============================================================================
static void NotifyAuthConnSocketConnectFailed (AuthConn * conn) {

    conn->error = kNetErrConnectFailed;

    conn->cancelId = 0;
    conn->dump(L"[TCP] SrvAuth socket connection failed %u, %s", conn->error, NetErrorToString(conn->error));

    conn->DecRef("Connecting");
}

//============================================================================
static void NotifyAuthConnSocketDisconnect (AuthConn * conn) {

    if (!conn->done && !conn->error)
        conn->error = kNetErrDisconnected;

    conn->cancelId = 0;
    conn->dump(L"[TCP] SrvAuth socket closed: %u, %s", conn->error, NetErrorToString(conn->error));

    HASHTABLEDECL(
        AuthTrans,
        THashKeyVal<unsigned>,
        link
    ) authTrans;
    
    s_critsect.Enter();
    {
        conn->done = true;
        while (AuthTrans * trans = s_authTrans.Head())
            authTrans.Add(trans);
    }
    s_critsect.Leave();
    
    while (AuthTrans * trans = authTrans.Head()) {
        conn->dump(L"[TCP] No reply from SrvAuth: %u, %s", conn->error, NetErrorToString(conn->error));
        DEL(trans);
    }
    
    conn->DecRef("Connected");
}

//============================================================================
static bool NotifyAuthConnSocketRead (AuthConn * conn, AsyncNotifySocketRead * read) {

    NetCliDispatch(conn->cli, read->buffer, read->bytes, conn);
    read->bytesProcessed += read->bytes;
    
    return true;
}

//============================================================================
static bool AuthSocketNotifyCallback (
    AsyncSocket         sock,
    EAsyncNotifySocket  code,
    AsyncNotifySocket * notify,
    void **             userState
) {
    bool result = true;
    AuthConn * conn;

    switch (code) {
        case kNotifySocketConnectSuccess:
            conn = (AuthConn *) notify->param;
            *userState      = conn;
            conn->sock      = sock;
            conn->cancelId  = 0;
            NotifyAuthConnSocketConnect(conn);
        break;

        case kNotifySocketConnectFailed:
            conn = (AuthConn *) notify->param;
            NotifyAuthConnSocketConnectFailed(conn);
        break;

        case kNotifySocketDisconnect:
            conn = (AuthConn *) *userState;
            NotifyAuthConnSocketDisconnect(conn);
        break;

        case kNotifySocketRead:
            conn = (AuthConn *) *userState;
            result = NotifyAuthConnSocketRead(conn, (AsyncNotifySocketRead *) notify);
        break;
    }
    
    return result;
}

//============================================================================
static bool Recv_File2Cli_ManifestReply (FileConn * conn, const File2Cli_ManifestReply & msg) {

    FileTrans * trans;
    s_critsect.Enter();
    {
        if (nil != (trans = s_fileTrans.Find(msg.transId)))
            s_fileTrans.Unlink(trans);
    }
    s_critsect.Leave();

    if (trans) {
        unsigned replyAtMs = TimeGetMs();
        trans->conn->dump(L"[TCP] Reply from SrvFile. ms=%u", replyAtMs - trans->pingAtMs);
        DEL(trans);
        return true;
    }
    else {
        return false;
    }
}

//============================================================================
static void FilePingProc (void * param) {

    FileConn * conn = (FileConn *)param;

    while (!conn->done && conn->pingsCompleted < kMaxPings) {

        if (!conn->pingsInRoute) {

            FileTrans * trans = NEW(FileTrans)(conn);
            trans->pingAtMs = TimeGetMs();

            s_critsect.Enter();
            for (;;) {
                if (conn->done) {
                    conn->pingsCompleted = kMaxPings;
                    DEL(trans);
                    break;
                }
                while (++s_transId == 0)
                    NULL_STMT;
                trans->SetValue(s_transId);
                s_fileTrans.Add(trans);

                Cli2File_ManifestRequest msg;
                StrCopy(msg.group, L"External", arrsize(msg.group));
                msg.messageId       = kCli2File_ManifestRequest;
                msg.transId         = trans->GetValue();
                msg.messageBytes    = sizeof(msg);
                msg.buildId         = 0;

                AsyncSocketSend(conn->sock, &msg, sizeof(msg));
                break;
            }
            s_critsect.Leave();
        }
        AsyncSleep(10);
    }

    s_critsect.Enter();
    {   
        conn->done = true;
        AsyncSocketDisconnect(conn->sock, true);
    }
    s_critsect.Leave();
        
    conn->DecRef("Pinging");
}

//============================================================================
static void NotifyFileConnSocketConnect (FileConn * conn) {

    conn->TransferRef("Connecting", "Connected");

    conn->dump(L"[TCP] SrvFile socket established");
    conn->dump(L"[TCP] Pinging SrvFile...");
    conn->IncRef("Pinging");
    _beginthread(FilePingProc, 0, conn);
}

//============================================================================
static void NotifyFileConnSocketConnectFailed (FileConn * conn) {

    conn->error = kNetErrConnectFailed;

    conn->cancelId = 0;
    conn->dump(L"[TCP] SrvFile socket connection failed %u, %s", conn->error, NetErrorToString(conn->error));

    conn->DecRef("Connecting");
}

//============================================================================
static void NotifyFileConnSocketDisconnect (FileConn * conn) {

    if (!conn->done && !conn->error)
        conn->error = kNetErrDisconnected;

    conn->cancelId = 0;
    conn->dump(L"[TCP] SrvFile socket closed: %u, %s", conn->error, NetErrorToString(conn->error));
    
    HASHTABLEDECL(
        FileTrans,
        THashKeyVal<unsigned>,
        link
    ) fileTrans;
    
    s_critsect.Enter();
    {
        conn->done = true;
        while (FileTrans * trans = s_fileTrans.Head())
            fileTrans.Add(trans);
    }
    s_critsect.Leave();
    
    while (FileTrans * trans = fileTrans.Head()) {
        conn->dump(L"[TCP] No reply from SrvFile: %u, %s", conn->error, NetErrorToString(conn->error));
        DEL(trans);
    }
    
    conn->DecRef("Connected");
}

//============================================================================
static bool NotifyFileConnSocketRead (FileConn * conn, AsyncNotifySocketRead * read) {

    conn->recvBuffer.Add(read->buffer, read->bytes);
    read->bytesProcessed += read->bytes;

    for (;;) {
        if (conn->recvBuffer.Count() < sizeof(dword))
            return true;

        dword msgSize = *(dword *)conn->recvBuffer.Ptr();
        if (conn->recvBuffer.Count() < msgSize)
            return true;

        const Cli2File_MsgHeader * msg = (const Cli2File_MsgHeader *) conn->recvBuffer.Ptr();
        
        if (msg->messageId != kFile2Cli_ManifestReply) {
            conn->dump(L"[TCP] SrvFile received unexpected message. id: %u", msg->messageId);
            return false;
        }
        
        if (!Recv_File2Cli_ManifestReply(conn, *(const File2Cli_ManifestReply *)msg))
            return false;

        conn->recvBuffer.Move(0, msgSize, conn->recvBuffer.Count() - msgSize);
        conn->recvBuffer.ShrinkBy(msgSize);
    }
}

//============================================================================
static bool FileSocketNotifyCallback (
    AsyncSocket         sock,
    EAsyncNotifySocket  code,
    AsyncNotifySocket * notify,
    void **             userState
) {
    bool result = true;
    FileConn * conn;

    switch (code) {
        case kNotifySocketConnectSuccess:
            conn = (FileConn *) notify->param;
            *userState      = conn;
            conn->sock      = sock;
            conn->cancelId  = 0;
            NotifyFileConnSocketConnect(conn);
        break;

        case kNotifySocketConnectFailed:
            conn = (FileConn *) notify->param;
            NotifyFileConnSocketConnectFailed(conn);
        break;

        case kNotifySocketDisconnect:
            conn = (FileConn *) *userState;
            NotifyFileConnSocketDisconnect(conn);
        break;

        case kNotifySocketRead:
            conn = (FileConn *) *userState;
            result = NotifyFileConnSocketRead(conn, (AsyncNotifySocketRead *) notify);
        break;
    }
    
    return result;
}

//============================================================================
static void StartAuthTcpTest (
    NetDiag *               diag,
    const NetAddress &      addr,
    FNetDiagDumpProc        dump,
    FNetDiagTestCallback    callback,
    void *                  param
) {
    if (0 == AtomicSet(&s_authProtocolRegistered, 1)) {
        MemSet(
            s_payload,
            (byte)((unsigned_ptr)&s_payload >> 4),
            sizeof(s_payload)
        );
        NetMsgProtocolRegister(
            kNetProtocolCli2Auth,
            false,
            s_send, arrsize(s_send),
            s_recv, arrsize(s_recv),
            kAuthDhGValue,
            BigNum(sizeof(kAuthDhXData), kAuthDhXData),
            BigNum(sizeof(kAuthDhNData), kAuthDhNData)
        );
    }

    wchar addrStr[128]; 
    NetAddressToString(addr, addrStr, arrsize(addrStr), kNetAddressFormatAll);
    dump(L"[TCP] Connecting to SrvAuth at %s...", addrStr);

    diag->IncRef("TCP");    
    
    AuthConn * conn = NEWZERO(AuthConn);
    conn->diag      = diag;
    conn->dump      = dump;
    conn->callback  = callback;
    conn->param     = param;
    conn->IncRef("Connecting");

    Cli2Auth_Connect connect;
    connect.hdr.connType    = (byte) kConnTypeCliToAuth;
    connect.hdr.hdrBytes    = sizeof(connect.hdr);
    connect.hdr.buildId     = BuildId();
    connect.hdr.buildType   = BUILD_TYPE_LIVE;
    connect.hdr.branchId    = BranchId();
    connect.hdr.productId   = ProductId();
    connect.data.token      = kNilGuid;
    connect.data.dataBytes  = sizeof(connect.data);

    AsyncSocketConnect(
        &conn->cancelId,
        addr,
        AuthSocketNotifyCallback,
        conn,
        &connect,
        sizeof(connect),
        0,
        0
    );
}

//============================================================================
static void StartFileTcpTest (
    NetDiag *               diag,
    const NetAddress &      addr,
    FNetDiagDumpProc        dump,
    FNetDiagTestCallback    callback,
    void *                  param
) {
    wchar addrStr[128]; 
    NetAddressToString(addr, addrStr, arrsize(addrStr), kNetAddressFormatAll);
    dump(L"[TCP] Connecting to SrvFile at %s...", addrStr);

    diag->IncRef("TCP");    

    FileConn * conn = NEWZERO(FileConn);
    conn->diag      = diag;
    conn->dump      = dump;
    conn->callback  = callback;
    conn->param     = param;
    conn->IncRef("Connecting");

    Cli2File_Connect connect;
    connect.hdr.connType    = kConnTypeCliToFile;
    connect.hdr.hdrBytes    = sizeof(connect.hdr);
    connect.hdr.buildId     = 0;
    connect.hdr.buildType   = BUILD_TYPE_LIVE;
    connect.hdr.branchId    = BranchId();
    connect.hdr.productId   = ProductId();
    connect.data.buildId    = BuildId();
    connect.data.serverType = kSrvTypeNone;
    connect.data.dataBytes  = sizeof(connect.data);

    AsyncSocketConnect(
        &conn->cancelId,
        addr,
        FileSocketNotifyCallback,
        conn,
        &connect,
        sizeof(connect),
        0,
        0
    );
    
}


/*****************************************************************************
*
*   AuthConn
*
***/

//============================================================================
AuthConn::~AuthConn () {
    if (cli)
        NetCliDelete(cli, false);
    if (sock)
        AsyncSocketDelete(sock);
    callback(diag, kNetProtocolCli2Auth, error, param);
    diag->DecRef("TCP");
}


/*****************************************************************************
*
*   AuthTrans
*
***/

//============================================================================
AuthTrans::AuthTrans (AuthConn * conn)
:   conn(conn)
{
    conn->IncRef("Ping");
    AtomicAdd(&conn->pingsInRoute, 1);
}

//============================================================================
AuthTrans::~AuthTrans () {

    AtomicAdd(&conn->pingsCompleted, 1);
    AtomicAdd(&conn->pingsInRoute, -1);
    conn->DecRef("Ping");
}


/*****************************************************************************
*
*   FileConn
*
***/

//============================================================================
FileConn::~FileConn () {
    if (sock)
        AsyncSocketDelete(sock);
    callback(diag, kNetProtocolCli2File, error, param);
    diag->DecRef("TCP");
}


/*****************************************************************************
*
*   FileTrans
*
***/

//============================================================================
FileTrans::FileTrans (FileConn * conn)
:   conn(conn)
{
    conn->IncRef("Ping");
    AtomicAdd(&conn->pingsInRoute, 1);
}

//============================================================================
FileTrans::~FileTrans () {

    AtomicAdd(&conn->pingsCompleted, 1);
    AtomicAdd(&conn->pingsInRoute, -1);
    conn->DecRef("Ping");
}


/*****************************************************************************
*
*   Module functions
*
***/

//============================================================================
void TcpStartup () {

    s_shutdown = false;
    AsyncTimerCreate(&s_timer, TimerCallback, 0, nil);
}

//============================================================================
void TcpShutdown () {

    s_shutdown = true;
    AsyncTimerDeleteCallback(s_timer, TimerCallback);
    s_timer = nil;
}


/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
void NetDiagTcp (
    NetDiag *               diag,
    ENetProtocol            protocol,
    unsigned                port,
    FNetDiagDumpProc        dump,
    FNetDiagTestCallback    callback,
    void *                  param
) {
    ASSERT(diag);
    ASSERT(dump);
    ASSERT(callback);

    unsigned srv = NetProtocolToSrv(protocol);
    if (srv == kNumDiagSrvs) {
        dump(L"[TCP] Unsupported protocol: %s", NetProtocolToString(protocol));
        callback(diag, protocol, kNetErrNotSupported, param);
        return;
    }

    unsigned    node;   
    NetAddress  addr;
    diag->critsect.Enter();
    {
        node = diag->nodes[srv];
    }
    diag->critsect.Leave();

    if (!node) {
        dump(L"[TCP] No address set for protocol: %s", NetProtocolToString(protocol));
        callback(diag, protocol, kNetSuccess, param);
        return;
    }

    NetAddressFromNode(node, port, &addr);
    
    switch (protocol) {
        case kNetProtocolCli2Auth:
            StartAuthTcpTest(diag, addr, dump, callback, param);
        break;
        
        case kNetProtocolCli2File:
            StartFileTcpTest(diag, addr, dump, callback, param);
        break;
        
        DEFAULT_FATAL(protocol);
    }
}
