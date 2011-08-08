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
*   $/Plasma20/Sources/Plasma/PubUtilLib/plNetGateKeeperLib/Private/plNglGateKeeper.cpp
*   
***/

#include "../Pch.h"
#pragma hdrstop

namespace Ngl { namespace GateKeeper {

/*****************************************************************************
*
*   Private
*
***/
    
struct CliGkConn : AtomicRef {
    CliGkConn ();
    ~CliGkConn ();

    // Reconnection
    AsyncTimer *        reconnectTimer;
    unsigned            reconnectStartMs;
    
    // Ping
    AsyncTimer *        pingTimer;
    unsigned            pingSendTimeMs;
    unsigned            lastHeardTimeMs;

    // This function should be called during object construction
    // to initiate connection attempts to the remote host whenever
    // the socket is disconnected.
    void AutoReconnect ();
    bool AutoReconnectEnabled ();
    void StopAutoReconnect (); // call before destruction
    void StartAutoReconnect ();
    void TimerReconnect ();
    
    // ping
    void AutoPing ();
    void StopAutoPing ();
    void TimerPing ();
    
    void Send (const unsigned_ptr fields[], unsigned count);

    CCritSect       critsect;
    LINK(CliGkConn) link;
    AsyncSocket     sock;
    NetCli *        cli;
    wchar           name[MAX_PATH];
    NetAddress      addr;
    Uuid            token;
    unsigned        seq;
    unsigned        serverChallenge;
    AsyncCancelId   cancelId;
    bool            abandoned;
};


//============================================================================
// PingRequestTrans
//============================================================================
struct PingRequestTrans : NetGateKeeperTrans {
    FNetCliGateKeeperPingRequestCallback    m_callback;
    void *                                  m_param;
    unsigned                                m_pingAtMs;
    unsigned                                m_replyAtMs;
    ARRAY(byte)                             m_payload;
    
    PingRequestTrans (
        FNetCliGateKeeperPingRequestCallback    callback,
        void *                                  param,
        unsigned                                pingAtMs,
        unsigned                                payloadBytes,
        const void *                            payload
    );

    bool Send ();
    void Post ();
    bool Recv (
        const byte  msg[],
        unsigned    bytes
    );
};

//============================================================================
// FileSrvIpAddressRequestTrans
//============================================================================
struct FileSrvIpAddressRequestTrans : NetGateKeeperTrans {
    FNetCliGateKeeperFileSrvIpAddressRequestCallback    m_callback;
    void *                                              m_param;
    wchar                                               m_addr[64];
    bool                                                m_isPatcher;
    
    FileSrvIpAddressRequestTrans (
        FNetCliGateKeeperFileSrvIpAddressRequestCallback    callback,
        void *                                              param,
        bool                                                isPatcher
    );

    bool Send ();
    void Post ();
    bool Recv (
        const byte  msg[],
        unsigned    bytes
    );
};

//============================================================================
// AuthSrvIpAddressRequestTrans
//============================================================================
struct AuthSrvIpAddressRequestTrans : NetGateKeeperTrans {
    FNetCliGateKeeperAuthSrvIpAddressRequestCallback    m_callback;
    void *                                              m_param;
    wchar                                               m_addr[64];
    
    AuthSrvIpAddressRequestTrans (
        FNetCliGateKeeperAuthSrvIpAddressRequestCallback    callback,
        void *                                              param
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
*   Private data
*
***/

enum {
    kPerfConnCount,
    kPingDisabled,
    kAutoReconnectDisabled,
    kNumPerf
};

static bool                         s_running;
static CCritSect                    s_critsect;
static LISTDECL(CliGkConn, link)    s_conns;
static CliGkConn *                  s_active;

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
static CliGkConn * GetConnIncRef_CS (const char tag[]) {
    if (CliGkConn * conn = s_active) {
        conn->IncRef(tag);
        return conn;
    }
    return nil;
}

//============================================================================
static CliGkConn * GetConnIncRef (const char tag[]) {
    CliGkConn * conn;
    s_critsect.Enter();
    {
        conn = GetConnIncRef_CS(tag);
    }
    s_critsect.Leave();
    return conn;
}

//============================================================================
static void UnlinkAndAbandonConn_CS (CliGkConn * conn) {
    s_conns.Unlink(conn);
    conn->abandoned = true;

    conn->StopAutoReconnect();

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
static void SendClientRegisterRequest (CliGkConn * conn) {
/*    const unsigned_ptr msg[] = {
        kCli2GateKeeper_ClientRegisterRequest,
        BuildId(),
    };

    conn->Send(msg, arrsize(msg));*/
}

//============================================================================
static bool ConnEncrypt (ENetError error, void * param) {
    CliGkConn * conn = (CliGkConn *) param;
        
    if (IS_NET_SUCCESS(error)) {

        //SendClientRegisterRequest(conn);

        if (!s_perf[kPingDisabled])
            conn->AutoPing();

        s_critsect.Enter();
        {
            SWAP(s_active, conn);
        }
        s_critsect.Leave();
            
    //  AuthConnectedNotifyTrans * trans = NEW(AuthConnectedNotifyTrans);
    //  NetTransSend(trans);
    }

    return IS_NET_SUCCESS(error);
}

//============================================================================
static void NotifyConnSocketConnect (CliGkConn * conn) {

    conn->TransferRef("Connecting", "Connected");
    conn->cli = NetCliConnectAccept(
        conn->sock,
        kNetProtocolCli2GateKeeper,
        false,
        ConnEncrypt,
        0,
        nil,
        conn
    );
}

//============================================================================
static void CheckedReconnect (CliGkConn * conn, ENetError error) {

    unsigned disconnectedMs = GetNonZeroTimeMs() - conn->lastHeardTimeMs;

    // no auto-reconnect or haven't heard from the server in a while?
    if (!conn->AutoReconnectEnabled() || (int)disconnectedMs >= (int)kDisconnectedTimeoutMs) {
        // Cancel all transactions in progress on this connection.
        NetTransCancelByConnId(conn->seq, kNetErrTimeout);
        // conn is dead.
        conn->DecRef("Lifetime");
        ReportNetError(kNetProtocolCli2GateKeeper, error);
    }
    else {
        if (conn->token != kNilGuid)
            // previously encrypted; reconnect quickly
            conn->reconnectStartMs = GetNonZeroTimeMs() + 500;
        else
            // never encrypted; reconnect slowly
            conn->reconnectStartMs = GetNonZeroTimeMs() + kMaxReconnectIntervalMs;

        // clean up the socket and start reconnect
        if (conn->cli)
            NetCliDelete(conn->cli, true);
        conn->cli = nil;
        conn->sock = nil;
        
        conn->StartAutoReconnect();
    }
}

//============================================================================
static void NotifyConnSocketConnectFailed (CliGkConn * conn) {

    s_critsect.Enter();
    {
        conn->cancelId = 0;
        s_conns.Unlink(conn);

        if (conn == s_active)
            s_active = nil;
    }
    s_critsect.Leave();
    
    CheckedReconnect(conn, kNetErrConnectFailed);
    
    conn->DecRef("Connecting");
}

//============================================================================
static void NotifyConnSocketDisconnect (CliGkConn * conn) {

    conn->StopAutoPing();

    s_critsect.Enter();
    {
        conn->cancelId = 0;
        s_conns.Unlink(conn);
            
        if (conn == s_active)
            s_active = nil;
    }
    s_critsect.Leave();

    // Cancel all transactions in process on this connection.
    NetTransCancelByConnId(conn->seq, kNetErrTimeout);
    conn->DecRef("Connected");
    conn->DecRef("Lifetime");
}

//============================================================================
static bool NotifyConnSocketRead (CliGkConn * conn, AsyncNotifySocketRead * read) {
    // TODO: Only dispatch messages from the active auth server
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
    CliGkConn * conn;

    switch (code) {
        case kNotifySocketConnectSuccess:
            conn = (CliGkConn *) notify->param;
            *userState = conn;
            bool abandoned;
            s_critsect.Enter();
            {
                conn->sock      = sock;
                conn->cancelId  = 0;
                abandoned       = conn->abandoned;
            }
            s_critsect.Leave();
            if (abandoned)
                AsyncSocketDisconnect(sock, true);
            else
                NotifyConnSocketConnect(conn);
        break;

        case kNotifySocketConnectFailed:
            conn = (CliGkConn *) notify->param;
            NotifyConnSocketConnectFailed(conn);
        break;

        case kNotifySocketDisconnect:
            conn = (CliGkConn *) *userState;
            NotifyConnSocketDisconnect(conn);
        break;

        case kNotifySocketRead:
            conn = (CliGkConn *) *userState;
            result = NotifyConnSocketRead(conn, (AsyncNotifySocketRead *) notify);
        break;
    }
    
    return result;
}

//============================================================================
static void Connect (
    CliGkConn * conn
) {
    ASSERT(s_running);

    conn->pingSendTimeMs = 0;

    s_critsect.Enter();
    {
        while (CliGkConn * oldConn = s_conns.Head()) {
            if (oldConn != conn)
                UnlinkAndAbandonConn_CS(oldConn);
            else
                s_conns.Unlink(oldConn);
        }
        s_conns.Link(conn);
    }
    s_critsect.Leave();
    
    Cli2GateKeeper_Connect connect;
    connect.hdr.connType        = kConnTypeCliToGateKeeper;
    connect.hdr.hdrBytes        = sizeof(connect.hdr);
    connect.hdr.buildId         = BuildId();
    connect.hdr.buildType       = BuildType();
    connect.hdr.branchId        = BranchId();
    connect.hdr.productId       = ProductId();
    connect.data.token          = conn->token;
    connect.data.dataBytes      = sizeof(connect.data);

    AsyncSocketConnect(
        &conn->cancelId,
        conn->addr,
        SocketNotifyCallback,
        conn,
        &connect,
        sizeof(connect),
        0,
        0
    );
}

//============================================================================
static void Connect (
    const wchar         name[],
    const NetAddress &  addr
) {
    ASSERT(s_running);
    
    CliGkConn * conn        = NEWZERO(CliGkConn);
    conn->addr              = addr;
    conn->seq               = ConnNextSequence();
    conn->lastHeardTimeMs   = GetNonZeroTimeMs();   // used in connect timeout, and ping timeout
    StrCopy(conn->name, name, arrsize(conn->name));

    conn->IncRef("Lifetime");
    conn->AutoReconnect();
}

//============================================================================
static void AsyncLookupCallback (
    void *              param,
    const wchar         name[],
    unsigned            addrCount,
    const NetAddress    addrs[]
) {
    if (!addrCount) {
        ReportNetError(kNetProtocolCli2GateKeeper, kNetErrNameLookupFailed);
        return;
    }

    for (unsigned i = 0; i < addrCount; ++i) {
        Connect(name, addrs[i]);
    }
}




/*****************************************************************************
*
*   CliGkConn
*
***/

//===========================================================================
static unsigned CliGkConnTimerDestroyed (void * param) {
    CliGkConn * conn = (CliGkConn *) param;
    conn->DecRef("TimerDestroyed");
    return kAsyncTimeInfinite;
}

//===========================================================================
static unsigned CliGkConnReconnectTimerProc (void * param) {
    ((CliGkConn *) param)->TimerReconnect();
    return kAsyncTimeInfinite;
}

//===========================================================================
static unsigned CliGkConnPingTimerProc (void * param) {
    ((CliGkConn *) param)->TimerPing();
    return kPingIntervalMs;
}

//============================================================================
CliGkConn::CliGkConn () {
    AtomicAdd(&s_perf[kPerfConnCount], 1);
}

//============================================================================
CliGkConn::~CliGkConn () {
    if (cli)
        NetCliDelete(cli, true);
    AtomicAdd(&s_perf[kPerfConnCount], -1);
}

//===========================================================================
void CliGkConn::TimerReconnect () {
    ASSERT(!sock);
    ASSERT(!cancelId);
    
    if (!s_running) {
        s_critsect.Enter();
        UnlinkAndAbandonConn_CS(this);
        s_critsect.Leave();
    }
    else {
        IncRef("Connecting");

        // Remember the time we started the reconnect attempt, guarding against
        // TimeGetMs() returning zero (unlikely), as a value of zero indicates
        // a first-time connect condition to StartAutoReconnect()
        reconnectStartMs = GetNonZeroTimeMs();

        Connect(this);
    }
}

//===========================================================================
// This function is called when after a disconnect to start a new connection
void CliGkConn::StartAutoReconnect () {
    critsect.Enter();
    if (reconnectTimer && !s_perf[kAutoReconnectDisabled]) {
        // Make reconnect attempts at regular intervals. If the last attempt
        // took more than the specified max interval time then reconnect
        // immediately; otherwise wait until the time interval is up again
        // then reconnect.
        unsigned remainingMs = 0;
        if (reconnectStartMs) {
            remainingMs = reconnectStartMs - GetNonZeroTimeMs();
            if ((signed)remainingMs < 0)
                remainingMs = 0;
            LogMsg(kLogPerf, L"GateKeeper auto-reconnecting in %u ms", remainingMs);
        }
        AsyncTimerUpdate(reconnectTimer, remainingMs);
    }
    critsect.Leave();
}

//===========================================================================
// This function should be called during object construction
// to initiate connection attempts to the remote host whenever
// the socket is disconnected.
void CliGkConn::AutoReconnect () {
        
    ASSERT(!reconnectTimer);
    IncRef("ReconnectTimer");
    critsect.Enter();
    {
        AsyncTimerCreate(
            &reconnectTimer,
            CliGkConnReconnectTimerProc,
            0,  // immediate callback
            this
        );
    }
    critsect.Leave();
}

//============================================================================
void CliGkConn::StopAutoReconnect () {
    critsect.Enter();
    {
        if (AsyncTimer * timer = reconnectTimer) {
            reconnectTimer = nil;
            AsyncTimerDeleteCallback(timer, CliGkConnTimerDestroyed);
        }
    }
    critsect.Leave();
}

//============================================================================
bool CliGkConn::AutoReconnectEnabled () {
    
    return (reconnectTimer != nil) && !s_perf[kAutoReconnectDisabled];
}

//============================================================================
void CliGkConn::AutoPing () {
    ASSERT(!pingTimer);
    IncRef("PingTimer");
    critsect.Enter();
    {
        AsyncTimerCreate(
            &pingTimer,
            CliGkConnPingTimerProc,
            sock ? 0 : kAsyncTimeInfinite,
            this
        );
    }
    critsect.Leave();
}

//============================================================================
void CliGkConn::StopAutoPing () {
    critsect.Enter();
    {
        if (AsyncTimer * timer = pingTimer) {
            pingTimer = nil;
            AsyncTimerDeleteCallback(timer, CliGkConnTimerDestroyed);
        }
    }
    critsect.Leave();
}

//============================================================================
void CliGkConn::TimerPing () {

#if 0
    // if the time difference between when we last sent a ping and when we last
    // heard from the server is >= 3x the ping interval, the socket is stale.
    if (pingSendTimeMs && abs(int(pingSendTimeMs - lastHeardTimeMs)) >= kPingTimeoutMs) {
        // ping timed out, disconnect the socket
        AsyncSocketDisconnect(sock, true);
    }
    else
#endif
    {
        // Send a ping request
        pingSendTimeMs = GetNonZeroTimeMs();
        
        const unsigned_ptr msg[] = {
            kCli2GateKeeper_PingRequest,
            pingSendTimeMs,
            0,      // not a transaction
            0,      // no payload
            nil
        };
        
        //Send(msg, arrsize(msg));
    }       
}

//============================================================================
void CliGkConn::Send (const unsigned_ptr fields[], unsigned count) {
    critsect.Enter();
    {
        NetCliSend(cli, fields, count);
        NetCliFlush(cli);
    }
    critsect.Leave();
}


/*****************************************************************************
*
*   Cli2GateKeeper message handlers
*
***/

//============================================================================
static bool Recv_PingReply (
    const byte      msg[],
    unsigned        bytes,
    void *
) {
    const GateKeeper2Cli_PingReply & reply = *(const GateKeeper2Cli_PingReply *)msg;

    if (reply.transId)
        NetTransRecv(reply.transId, msg, bytes);

    return true;
}


//============================================================================
static bool Recv_FileSrvIpAddressReply (
    const byte      msg[],
    unsigned        bytes,
    void *
) {
    const GateKeeper2Cli_FileSrvIpAddressReply & reply = *(const GateKeeper2Cli_FileSrvIpAddressReply *)msg;

    if (reply.transId)
        NetTransRecv(reply.transId, msg, bytes);

    return true;
}

//============================================================================
static bool Recv_AuthSrvIpAddressReply (
    const byte      msg[],
    unsigned        bytes,
    void *
) {
    const GateKeeper2Cli_AuthSrvIpAddressReply & reply = *(const GateKeeper2Cli_AuthSrvIpAddressReply *)msg;

    if (reply.transId)
        NetTransRecv(reply.transId, msg, bytes);

    return true;
}


/*****************************************************************************
*
*   Cli2Auth protocol
*
***/

#define MSG(s)  kNetMsg_Cli2GateKeeper_##s
static NetMsgInitSend s_send[] = {
    { MSG(PingRequest)              },
    { MSG(FileSrvIpAddressRequest)  },
    { MSG(AuthSrvIpAddressRequest)  },
};
#undef MSG

#define MSG(s)  kNetMsg_GateKeeper2Cli_##s, Recv_##s
static NetMsgInitRecv s_recv[] = {
    { MSG(PingReply)                },
    { MSG(FileSrvIpAddressReply)    },
    { MSG(AuthSrvIpAddressReply)    },
};
#undef MSG


/*****************************************************************************
*
*   PingRequestTrans
*
***/

//============================================================================
PingRequestTrans::PingRequestTrans (
    FNetCliGateKeeperPingRequestCallback    callback,
    void *                          param,
    unsigned                        pingAtMs,
    unsigned                        payloadBytes,
    const void *                    payload
) : NetGateKeeperTrans(kPingRequestTrans)
,   m_callback(callback)
,   m_param(param)
,   m_pingAtMs(pingAtMs)
{
    m_payload.Set((const byte *)payload, payloadBytes);
}

//============================================================================
bool PingRequestTrans::Send () {

    if (!AcquireConn())
        return false;

    const unsigned_ptr msg[] = {
        kCli2GateKeeper_PingRequest,
                        m_pingAtMs,
                        m_transId,
                        m_payload.Count(),
        (unsigned_ptr)  m_payload.Ptr(),
    };
    
    m_conn->Send(msg, arrsize(msg));
    
    return true;
}

//============================================================================
void PingRequestTrans::Post () {

    m_callback(
        m_result,
        m_param,
        m_pingAtMs,
        m_replyAtMs,
        m_payload.Count(),
        m_payload.Ptr()
    );
}

//============================================================================
bool PingRequestTrans::Recv (
    const byte  msg[],
    unsigned    bytes
) {
    const GateKeeper2Cli_PingReply & reply = *(const GateKeeper2Cli_PingReply *)msg;

    m_payload.Set(reply.payload, reply.payloadBytes);
    m_replyAtMs     = TimeGetMs();
    m_result        = kNetSuccess;
    m_state         = kTransStateComplete;

    return true;
}


/*****************************************************************************
*
*   FileSrvIpAddressRequestTrans
*
***/

//============================================================================
FileSrvIpAddressRequestTrans::FileSrvIpAddressRequestTrans (
    FNetCliGateKeeperFileSrvIpAddressRequestCallback    callback,
    void *                                              param,
    bool                                                isPatcher
) : NetGateKeeperTrans(kGkFileSrvIpAddressRequestTrans)
,   m_callback(callback)
,   m_param(param)
,   m_isPatcher(isPatcher)
{
}

//============================================================================
bool FileSrvIpAddressRequestTrans::Send () {

    if (!AcquireConn())
        return false;

    
    const unsigned_ptr msg[] = {
        kCli2GateKeeper_FileSrvIpAddressRequest,
                        m_transId,
                        m_isPatcher == true ? 1 : 0
    };
    
    m_conn->Send(msg, arrsize(msg));
    
    return true;
}

//============================================================================
void FileSrvIpAddressRequestTrans::Post () {

    m_callback(
        m_result,
        m_param,
        m_addr
    );
}

//============================================================================
bool FileSrvIpAddressRequestTrans::Recv (
    const byte  msg[],
    unsigned    bytes
) {
    const GateKeeper2Cli_FileSrvIpAddressReply & reply = *(const GateKeeper2Cli_FileSrvIpAddressReply *)msg;

    
    m_result        = kNetSuccess;
    m_state         = kTransStateComplete;
    StrCopy(m_addr, reply.address, 64);

    return true;
}


/*****************************************************************************
*
*   AuthSrvIpAddressRequestTrans
*
***/

//============================================================================
AuthSrvIpAddressRequestTrans::AuthSrvIpAddressRequestTrans (
    FNetCliGateKeeperFileSrvIpAddressRequestCallback    callback,
    void *                                              param
) : NetGateKeeperTrans(kGkAuthSrvIpAddressRequestTrans)
,   m_callback(callback)
,   m_param(param)
{
}

//============================================================================
bool AuthSrvIpAddressRequestTrans::Send () {

    if (!AcquireConn())
        return false;

    const unsigned_ptr msg[] = {
        kCli2GateKeeper_AuthSrvIpAddressRequest,
                        m_transId,
    };
    
    m_conn->Send(msg, arrsize(msg));
    
    return true;
}

//============================================================================
void AuthSrvIpAddressRequestTrans::Post () {

    m_callback(
        m_result,
        m_param,
        m_addr
    );
}

//============================================================================
bool AuthSrvIpAddressRequestTrans::Recv (
    const byte  msg[],
    unsigned    bytes
) {
    const GateKeeper2Cli_AuthSrvIpAddressReply & reply = *(const GateKeeper2Cli_AuthSrvIpAddressReply *)msg;

    m_result        = kNetSuccess;
    m_state         = kTransStateComplete;
    StrCopy(m_addr, reply.address, 64);

    return true;
}


}   using namespace GateKeeper;


/*****************************************************************************
*
*   NetGateKeeperTrans
*
***/

//============================================================================
NetGateKeeperTrans::NetGateKeeperTrans (ETransType transType)
:   NetTrans(kNetProtocolCli2GateKeeper, transType)
,   m_conn(nil)
{
}

//============================================================================
NetGateKeeperTrans::~NetGateKeeperTrans () {
    ReleaseConn();
}

//============================================================================
bool NetGateKeeperTrans::AcquireConn () {
    if (!m_conn)
        m_conn = GetConnIncRef("AcquireConn");
    return m_conn != nil;
}

//============================================================================
void NetGateKeeperTrans::ReleaseConn () {
    if (m_conn) {
        m_conn->DecRef("AcquireConn");
        m_conn = nil;
    }
}


/*****************************************************************************
*
*   Protected functions
*
***/

//============================================================================
void GateKeeperInitialize () {
    s_running = true;
    NetMsgProtocolRegister(
        kNetProtocolCli2GateKeeper,
        false,
        s_send, arrsize(s_send),
        s_recv, arrsize(s_recv),
        kGateKeeperDhGValue,
        BigNum(sizeof(kGateKeeperDhXData), kGateKeeperDhXData),
        BigNum(sizeof(kGateKeeperDhNData), kGateKeeperDhNData)
    );
}

//============================================================================
void GateKeeperDestroy (bool wait) {
    s_running = false;

    NetTransCancelByProtocol(
        kNetProtocolCli2GateKeeper,
        kNetErrRemoteShutdown
    );    
    NetMsgProtocolDestroy(
        kNetProtocolCli2GateKeeper,
        false
    );

    s_critsect.Enter();
    {
        while (CliGkConn * conn = s_conns.Head())
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
bool GateKeeperQueryConnected () {

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
unsigned GateKeeperGetConnId () {
    unsigned connId;
    s_critsect.Enter();
    connId = (s_active) ? s_active->seq : 0;
    s_critsect.Leave();
    return connId;
}


}   using namespace Ngl;


/*****************************************************************************
*
*   Exported functions
*
***/

//============================================================================
void NetCliGateKeeperStartConnect (
    const wchar *   gateKeeperAddrList[],
    unsigned        gateKeeperAddrCount
) {
    gateKeeperAddrCount = min(gateKeeperAddrCount, 1);

    for (unsigned i = 0; i < gateKeeperAddrCount; ++i) {
        // Do we need to lookup the address?
        const wchar * name = gateKeeperAddrList[i];
        while (unsigned ch = *name) {
            ++name;
            if (!(isdigit(ch) || ch == L'.' || ch == L':')) {
                AsyncCancelId cancelId;
                AsyncAddressLookupName(
                    &cancelId,
                    AsyncLookupCallback,
                    gateKeeperAddrList[i],
                    kNetDefaultClientPort,
                    nil
                );
                break;
            }
        }
        if (!name[0]) {
            NetAddress addr;
            NetAddressFromString(&addr, gateKeeperAddrList[i], kNetDefaultClientPort);
            Connect(gateKeeperAddrList[i], addr);
        }
    }
}

//============================================================================
void NetCliGateKeeperDisconnect () {
    s_critsect.Enter();
    {
        while (CliGkConn * conn = s_conns.Head())
            UnlinkAndAbandonConn_CS(conn);
        s_active = nil;
    }
    s_critsect.Leave();
}

//============================================================================
void NetCliGateKeeperPingRequest (
    unsigned                                pingTimeMs,
    unsigned                                payloadBytes,
    const void *                            payload,
    FNetCliGateKeeperPingRequestCallback    callback,
    void *                                  param
) {
    PingRequestTrans * trans = NEW(PingRequestTrans)(
        callback,
        param,
        pingTimeMs, 
        payloadBytes,
        payload
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliGateKeeperFileSrvIpAddressRequest (
    FNetCliGateKeeperFileSrvIpAddressRequestCallback    callback,
    void *                                              param,
    bool                                                isPatcher
) {
    FileSrvIpAddressRequestTrans * trans = NEW(FileSrvIpAddressRequestTrans)(callback, param, isPatcher);
    NetTransSend(trans);
}

//============================================================================
void NetCliGateKeeperAuthSrvIpAddressRequest (
    FNetCliGateKeeperAuthSrvIpAddressRequestCallback    callback,
    void *                                              param
) {
    AuthSrvIpAddressRequestTrans * trans = NEW(AuthSrvIpAddressRequestTrans)(callback, param);
    NetTransSend(trans);
}
