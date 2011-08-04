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
*   $/Plasma20/Sources/Plasma/PubUtilLib/plNetGameLib/Private/plNglCsr.cpp
*   
***/

#include "../Pch.h"
#pragma hdrstop


namespace Ngl { namespace Csr {
/*****************************************************************************
*
*   Internal types
*
***/

struct ConnectParam {
    FNetCliCsrConnectedCallback callback;
    void *                      param;
};

//============================================================================
// Connection record
//============================================================================
struct CliCsConn : AtomicRef {
    LINK(CliCsConn) link;

    CCritSect       critsect;
    AsyncSocket     sock;
    AsyncCancelId   cancelId;
    NetCli *        cli;
    NetAddress      addr;
    unsigned        seq;
    bool            abandoned;
    unsigned        serverChallenge;
    unsigned        latestBuildId;
    ConnectParam *  connectParam;

    // ping
    AsyncTimer *    pingTimer;
    unsigned        pingSendTimeMs;
    unsigned        lastHeardTimeMs;

    CliCsConn ();
    ~CliCsConn ();

    void AutoPing ();
    void StopAutoPing ();
    void TimerPing ();

    void Send (const unsigned_ptr fields[], unsigned count);
};


//============================================================================
// Transaction objects
//============================================================================
struct ConnectedNotifyTrans : NetNotifyTrans {

    ConnectParam *  m_connectParam;
    unsigned        m_latestBuildId;

    ConnectedNotifyTrans (ConnectParam * cp, unsigned lbi)
    : NetNotifyTrans(kCsrConnectedNotifyTrans)
    , m_connectParam(cp)
    , m_latestBuildId(lbi)
    { }
    ~ConnectedNotifyTrans () {
        DEL(m_connectParam);
    }
    void Post ();
};

struct LoginRequestTrans : NetCsrTrans {

    wchar                   m_csrName[kMaxAccountNameLength];
    ShaDigest               m_namePassHash;
    FNetCliCsrLoginCallback m_callback;
    void *                  m_param;
    
    Uuid                    m_csrId;
    unsigned                m_csrFlags;
    
    LoginRequestTrans (
        const wchar             csrName[],
        const ShaDigest &       namePassHash,
        FNetCliCsrLoginCallback callback,
        void *                  param
    );

    bool Send ();
    void Post ();
    bool Recv (
        const byte  msg[],
        unsigned    bytes
    );
};


/*****************************************************************************
*
*   Internal data
*
***/

enum {
    kPerfConnCount,
    kPingDisabled,
    kNumPerf
};

static bool                         s_running;
static CCritSect                    s_critsect;
static LISTDECL(CliCsConn, link)    s_conns;
static CliCsConn *                  s_active;
static long                         s_perf[kNumPerf];


/*****************************************************************************
*
*   Internal functions
*
***/

//===========================================================================
static unsigned GetNonZeroTimeMs () {
    if (unsigned ms = TimeGetMs())
        return ms;
    return 1;
}

//============================================================================
static CliCsConn * GetConnIncRef_CS (const char tag[]) {

    if (CliCsConn * conn = s_active)
        if (conn->cli) {
            conn->IncRef(tag);
            return conn;
        }
    return nil;
}

//============================================================================
static CliCsConn * GetConnIncRef (const char tag[]) {
    CliCsConn * conn;
    s_critsect.Enter();
    {
        conn = GetConnIncRef_CS(tag);
    }
    s_critsect.Leave();
    return conn;
}

//============================================================================
static void UnlinkAndAbandonConn_CS (CliCsConn * conn) {

    s_conns.Unlink(conn);
    conn->abandoned = true;
    if (conn->cancelId) {
        AsyncSocketConnectCancel(nil, conn->cancelId);
        conn->cancelId  = 0;
    }
    else if (conn->sock) {
        AsyncSocketDisconnect(conn->sock, true);
    }
    else {
        conn->DecRef("Lifetime");
    }
}

//============================================================================
static void SendRegisterRequest (CliCsConn * conn) {
    
    const unsigned_ptr msg[] = {
        kCli2Csr_RegisterRequest,
        0
    };

    conn->Send(msg, arrsize(msg));
}

//============================================================================
static bool ConnEncrypt (ENetError error, void * param) {

    CliCsConn * conn = (CliCsConn *) param;

    if (IS_NET_SUCCESS(error)) {
        s_critsect.Enter();
        {
            s_active = conn;
            conn->AutoPing();
            conn->IncRef();
        }
        s_critsect.Leave();
        
        SendRegisterRequest(conn);
        
        conn->DecRef();
    }

    return IS_NET_SUCCESS(error);
}

//============================================================================
static void NotifyConnSocketConnect (CliCsConn * conn) {

    conn->cli = NetCliConnectAccept(
        conn->sock,
        kNetProtocolCli2Csr,
        false,
        ConnEncrypt,
        0,
        nil,
        conn
    );
}

//============================================================================
static void NotifyConnSocketConnectFailed (CliCsConn * conn) {

    bool notify;
    s_critsect.Enter();
    {
        conn->cancelId = 0;
        s_conns.Unlink(conn);
        
        notify
            =  s_running
            && !conn->abandoned
            && (!s_active || conn == s_active);
            
        if (conn == s_active)
            s_active = nil;
    }
    s_critsect.Leave();

    NetTransCancelByConnId(conn->seq, kNetErrTimeout);
    conn->DecRef("Connecting");
    conn->DecRef("Lifetime");

    if (notify)
        ReportNetError(kNetProtocolCli2Csr, kNetErrConnectFailed);
}

//============================================================================
static void NotifyConnSocketDisconnect (CliCsConn * conn) {

    conn->StopAutoPing();

    bool notify;
    s_critsect.Enter();
    {
        s_conns.Unlink(conn);

        notify
            =  s_running
            && !conn->abandoned
            && (!s_active || conn == s_active);

        if (conn == s_active)
            s_active = nil;
    }
    s_critsect.Leave();

    // Cancel all transactions in process on this connection.
    NetTransCancelByConnId(conn->seq, kNetErrTimeout);
    conn->DecRef("Connected");
    conn->DecRef("Lifetime");

    if (notify)
        ReportNetError(kNetProtocolCli2Csr, kNetErrDisconnected);
}

//============================================================================
static bool NotifyConnSocketRead (CliCsConn * conn, AsyncNotifySocketRead * read) {

    conn->lastHeardTimeMs = GetNonZeroTimeMs();
    bool result = NetCliDispatch(conn->cli, read->buffer, read->bytes, conn);
    read->bytesProcessed += read->bytes;
    return result;
}

//============================================================================
static bool SocketNotifyCallback (
    AsyncSocket         sock,
    EAsyncNotifySocket  code,
    AsyncNotifySocket * notify,
    void **             userState
) {
    bool result = true;
    CliCsConn * conn;

    switch (code) {
        case kNotifySocketConnectSuccess: {
            conn = (CliCsConn *) notify->param;
            *userState = conn;
            conn->TransferRef("Connecting", "Connected");
            bool abandoned = true;
            
            if (abandoned)
                AsyncSocketDisconnect(sock, true);
            else
                NotifyConnSocketConnect(conn);
        }
        break;

        case kNotifySocketConnectFailed:
            conn = (CliCsConn *) notify->param;
            NotifyConnSocketConnectFailed(conn);
        break;

        case kNotifySocketDisconnect:
            conn = (CliCsConn *) *userState;
            NotifyConnSocketDisconnect(conn);
        break;

        case kNotifySocketRead:
            conn = (CliCsConn *) *userState;
            result = NotifyConnSocketRead(conn, (AsyncNotifySocketRead *) notify);
        break;
    }

    return result;
}

//============================================================================
static void Connect (
    const NetAddress &  addr,
    ConnectParam *      cp
) {
    CliCsConn * conn = NEWZERO(CliCsConn);
    conn->addr              = addr;
    conn->seq               = ConnNextSequence();
    conn->lastHeardTimeMs   = GetNonZeroTimeMs();
    conn->connectParam      = cp;

    conn->IncRef("Lifetime");
    conn->IncRef("Connecting");

    s_critsect.Enter();
    {
        while (CliCsConn * conn = s_conns.Head())
            UnlinkAndAbandonConn_CS(conn);
        s_conns.Link(conn);
    }
    s_critsect.Leave();

    Cli2Csr_Connect connect;
    connect.hdr.connType    = kConnTypeCliToCsr;
    connect.hdr.hdrBytes    = sizeof(connect.hdr);
    connect.hdr.buildId     = BuildId();
    connect.hdr.buildType   = BUILD_TYPE_LIVE;
    connect.hdr.branchId    = BranchId();
    connect.hdr.productId   = ProductId();
    connect.data.dataBytes  = sizeof(connect.data);

    AsyncSocketConnect(
        &conn->cancelId,
        addr,
        SocketNotifyCallback,
        conn,
        &connect,
        sizeof(connect),
        0,
        0
    );
}

//============================================================================
static void AsyncLookupCallback (
    void *              param,
    const wchar         name[],
    unsigned            addrCount,
    const NetAddress    addrs[]
) {
    if (!addrCount) {
        ReportNetError(kNetProtocolCli2Auth, kNetErrNameLookupFailed);
        return;
    }

    // Only connect to one server   
    addrCount = MIN(addrCount, 1);

    for (unsigned i = 0; i < addrCount; ++i) {
        Connect(addrs[i], (ConnectParam *)param);
    }
}


/*****************************************************************************
*
*   Message handlers
*
***/

//============================================================================
static bool Recv_PingReply (
    const byte  msg[],
    unsigned    bytes,
    void *
) {
    const Csr2Cli_PingReply & reply = *(const Csr2Cli_PingReply *)msg;

    NetTransRecv(reply.transId, msg, bytes);

    return true;
}

//============================================================================
static bool Recv_RegisterReply (
    const byte  msg[],
    unsigned    ,
    void *      param
) {
    CliCsConn * conn = (CliCsConn *)param;

    const Csr2Cli_RegisterReply & reply = *(const Csr2Cli_RegisterReply *)msg;

    conn->serverChallenge   = reply.serverChallenge;
    conn->latestBuildId     = reply.csrBuildId;

    ConnectedNotifyTrans * trans = NEW(ConnectedNotifyTrans)(
        conn->connectParam,
        conn->latestBuildId
    );
    NetTransSend(trans);
    
    conn->connectParam = nil;

    return true;
}

//============================================================================
static bool Recv_LoginReply (
    const byte  msg[],
    unsigned    bytes,
    void *
) {
    const Csr2Cli_LoginReply & reply = *(const Csr2Cli_LoginReply *)msg;

    NetTransRecv(reply.transId, msg, bytes);

    return true;
}


/*****************************************************************************
*
*   Protocol
*
***/

#define MSG(s)  kNetMsg_Cli2Csr_##s
static NetMsgInitSend s_send[] = {
    { MSG(PingRequest)          },
    { MSG(RegisterRequest)      },
    { MSG(LoginRequest)         },
};
#undef MSG

#define MSG(s)  kNetMsg_Csr2Cli_##s, Recv_##s
static NetMsgInitRecv s_recv[] = {
    { MSG(PingReply)            },
    { MSG(RegisterReply)        },
    { MSG(LoginReply)           },
};
#undef MSG


/*****************************************************************************
*
*   CliCsConn
*
***/

//===========================================================================
static unsigned CliCsConnTimerDestroyed (void * param) {
    
    CliCsConn * conn = (CliCsConn *) param;
    conn->DecRef("PingTimer");
    return kAsyncTimeInfinite;
}

//===========================================================================
static unsigned CliCsConnPingTimerProc (void * param) {
    
    ((CliCsConn *) param)->TimerPing();
    return kPingIntervalMs;
}

//============================================================================
CliCsConn::CliCsConn () {
    
    AtomicAdd(&s_perf[kPerfConnCount], 1);
}

//============================================================================
CliCsConn::~CliCsConn () {

    // Delete 'cli' after all refs have been removed    
    if (cli)
        NetCliDelete(cli, true);
        
    DEL(connectParam);

    AtomicAdd(&s_perf[kPerfConnCount], -1);
}

//============================================================================
void CliCsConn::AutoPing () {
    ASSERT(!pingTimer);
    
    IncRef("PingTimer");
    critsect.Enter();
    {
        AsyncTimerCreate(
            &pingTimer,
            CliCsConnPingTimerProc,
            sock ? 0 : kAsyncTimeInfinite,
            this
        );
    }
    critsect.Leave();
}

//============================================================================
void CliCsConn::StopAutoPing () {
    critsect.Enter();
    {
        if (AsyncTimer * timer = pingTimer) {
            pingTimer = nil;
            AsyncTimerDeleteCallback(timer, CliCsConnTimerDestroyed);
        }
    }
    critsect.Leave();
}

//============================================================================
void CliCsConn::TimerPing () {

    // Send a ping request
    pingSendTimeMs = GetNonZeroTimeMs();
    
    const unsigned_ptr msg[] = {
        kCli2Auth_PingRequest,
                    0,      // not a transaction
                    pingSendTimeMs,
                    0,      // no payload
                    nil
    };
    
    Send(msg, arrsize(msg));
}

//============================================================================
void CliCsConn::Send (const unsigned_ptr fields[], unsigned count) {

    critsect.Enter();
    {
        NetCliSend(cli, fields, count);
        NetCliFlush(cli);
    }
    critsect.Leave();
}


/*****************************************************************************
*
*   ConnectedNotifyTrans
*
***/

//============================================================================
void ConnectedNotifyTrans::Post () {

    if (m_connectParam && m_connectParam->callback)
        m_connectParam->callback(m_connectParam->param, m_latestBuildId);
}


/*****************************************************************************
*
*   LoginRequestTrans
*
***/

//============================================================================
LoginRequestTrans::LoginRequestTrans (
    const wchar             csrName[],
    const ShaDigest &       namePassHash,
    FNetCliCsrLoginCallback callback,
    void *                  param
) : NetCsrTrans(kCsrLoginTrans)
,   m_namePassHash(namePassHash)
,   m_callback(callback)
,   m_param(param)
{
    ASSERT(callback);
    StrCopy(m_csrName, csrName, arrsize(m_csrName));
}

//============================================================================
bool LoginRequestTrans::Send () {

    if (!AcquireConn())
        return false;
        
    ShaDigest challengeHash;
    dword clientChallenge = 0;
    
    CryptCreateRandomSeed(
        sizeof(clientChallenge),
        (byte *) &clientChallenge
    );

    CryptHashPasswordChallenge(
        clientChallenge,
        s_active->serverChallenge,
        m_namePassHash,
        &challengeHash
    );

    const unsigned_ptr msg[] = {
        kCli2Csr_LoginRequest,
                        m_transId,
                        clientChallenge,
        (unsigned_ptr)  m_csrName,
        (unsigned_ptr)  &challengeHash
    };

    m_conn->Send(msg, arrsize(msg));

    return true;
}

//============================================================================
void LoginRequestTrans::Post () {
    m_callback(
        m_result,
        m_param,
        m_csrId,
        m_csrFlags
    );
}

//============================================================================
bool LoginRequestTrans::Recv (
    const byte  msg[],
    unsigned    bytes
) {
    const Csr2Cli_LoginReply & reply = *(const Csr2Cli_LoginReply *) msg;
    
    m_result    = reply.result;
    m_csrId     = reply.csrId;
    m_csrFlags  = reply.csrFlags;
    
    m_state     = kTransStateComplete;
    
    return true;
}


} using namespace Csr;


/*****************************************************************************
*
*   NetCsrTrans
*
***/

//============================================================================
NetCsrTrans::NetCsrTrans (ETransType transType)
:   NetTrans(kNetProtocolCli2Csr, transType)
,   m_conn(nil)
{
}

//============================================================================
NetCsrTrans::~NetCsrTrans () {
    ReleaseConn();
}

//============================================================================
bool NetCsrTrans::AcquireConn () {
    if (!m_conn)
        m_conn = GetConnIncRef("AcquireConn");
    return m_conn != nil;
}

//============================================================================
void NetCsrTrans::ReleaseConn () {
    if (m_conn) {
        m_conn->DecRef("AcquireConn");
        m_conn = nil;
    }
}


/*****************************************************************************
*
*   Module functions
*
***/

//============================================================================
void CsrInitialize () {

    s_running = true;

    NetMsgProtocolRegister(
        kNetProtocolCli2Csr,
        false,
        s_send, arrsize(s_send),
        s_recv, arrsize(s_recv),
        kCsrDhGValue,
        BigNum(sizeof(kCsrDhXData), kCsrDhXData),
        BigNum(sizeof(kCsrDhNData), kCsrDhNData)
    );
}

//============================================================================
void CsrDestroy (bool wait) {

    s_running = false;
    
    NetTransCancelByProtocol(
        kNetProtocolCli2Csr,
        kNetErrRemoteShutdown
    );    
    NetMsgProtocolDestroy(
        kNetProtocolCli2Csr,
        false
    );

    s_critsect.Enter();
    {
        while (CliCsConn * conn = s_conns.Head())
            UnlinkAndAbandonConn_CS(conn);
        s_active = nil;
    }
    s_critsect.Leave();

    if (!wait)
        return;
        
    while (s_perf[kPerfConnCount]) {
        NetTransUpdate();
        AsyncSleep(10);
    }
}

//============================================================================
bool CsrQueryConnected () {

    bool result;
    s_critsect.Enter();
    {
        if (nil != (result = s_active))
            result &= (nil != s_active->cli);
    }
    s_critsect.Leave();
    return result;
}

//============================================================================
unsigned CsrGetConnId () {

    unsigned connId;
    s_critsect.Enter();
    {
        connId = (s_active) ? s_active->seq : 0;
    }
    s_critsect.Leave();
    return connId;
}

} using namespace Ngl;


/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
void NetCliCsrStartConnect (
    const wchar *               addrList[],
    unsigned                    addrCount,
    FNetCliCsrConnectedCallback callback,
    void *                      param
) {
    // Only connect to one server
    addrCount = min(addrCount, 1);

    for (unsigned i = 0; i < addrCount; ++i) {
        // Do we need to lookup the address?
        const wchar * name = addrList[i];
        while (unsigned ch = *name) {
            ++name;
            if (!(isdigit(ch) || ch == L'.' || ch == L':')) {
                ConnectParam * cp = NEW(ConnectParam);
                cp->callback    = callback;
                cp->param       = param;

                AsyncCancelId cancelId;
                AsyncAddressLookupName(
                    &cancelId,
                    AsyncLookupCallback,
                    addrList[i],
                    kNetDefaultClientPort,
                    cp
                );
                break;
            }
        }
        if (!name[0]) {
            NetAddress addr;
            NetAddressFromString(&addr, addrList[i], kNetDefaultClientPort);
            
            ConnectParam * cp = NEW(ConnectParam);
            cp->callback    = callback;
            cp->param       = param;

            Connect(addr, cp);
        }
    }
}

//============================================================================
void NetCliCsrDisconnect () {
    
    s_critsect.Enter();
    {
        while (CliCsConn * conn = s_conns.Head())
            UnlinkAndAbandonConn_CS(conn);
        s_active = nil;
    }
    s_critsect.Leave();
}

//============================================================================
void NetCliCsrLoginRequest (
    const wchar             csrName[],
    const ShaDigest &       namePassHash,
    FNetCliCsrLoginCallback callback,
    void *                  param
) {
    LoginRequestTrans * trans = NEW(LoginRequestTrans)(
        csrName,
        namePassHash,
        callback,
        param
    );
    NetTransSend(trans);
}
