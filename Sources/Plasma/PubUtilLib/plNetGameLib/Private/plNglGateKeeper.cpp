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
*   $/Plasma20/Sources/Plasma/PubUtilLib/plNetGateKeeperLib/Private/plNglGateKeeper.cpp
*   
***/

#include "../Pch.h"

namespace Ngl { namespace GateKeeper {

/*****************************************************************************
*
*   Private
*
***/
    
struct CliGkConn : hsRefCnt {
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

    void Send (const uintptr_t fields[], unsigned count);

    std::recursive_mutex  critsect;
    AsyncSocket     sock;
    NetCli *        cli;
    ST::string      name;
    plNetAddress    addr;
    plUUID          token;
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
    std::vector<uint8_t>                    m_payload;
    
    PingRequestTrans (
        FNetCliGateKeeperPingRequestCallback    callback,
        void *                                  param,
        unsigned                                pingAtMs,
        unsigned                                payloadBytes,
        const void *                            payload
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// FileSrvIpAddressRequestTrans
//============================================================================
struct FileSrvIpAddressRequestTrans : NetGateKeeperTrans {
    FNetCliGateKeeperFileSrvIpAddressRequestCallback    m_callback;
    void *                                              m_param;
    ST::string                                          m_addr;
    bool                                                m_isPatcher;
    
    FileSrvIpAddressRequestTrans (
        FNetCliGateKeeperFileSrvIpAddressRequestCallback    callback,
        void *                                              param,
        bool                                                isPatcher
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// AuthSrvIpAddressRequestTrans
//============================================================================
struct AuthSrvIpAddressRequestTrans : NetGateKeeperTrans {
    FNetCliGateKeeperAuthSrvIpAddressRequestCallback    m_callback;
    void *                                              m_param;
    ST::string                                          m_addr;

    AuthSrvIpAddressRequestTrans (
        FNetCliGateKeeperAuthSrvIpAddressRequestCallback    callback,
        void *                                              param
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
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
static std::recursive_mutex         s_critsect;
static CliGkConn* s_conn = nullptr;
static CliGkConn *                  s_active;

static std::atomic<long>            s_perf[kNumPerf];



/*****************************************************************************
*
*   Internal functions
*
***/

//===========================================================================
static unsigned GetNonZeroTimeMs () {
    if (unsigned ms = hsTimer::GetMilliSeconds<uint32_t>())
        return ms;
    return 1;
}

//============================================================================
static CliGkConn * GetConnIncRef_CS (const char tag[]) {
    if (CliGkConn * conn = s_active) {
        conn->Ref(tag);
        return conn;
    }
    return nullptr;
}

//============================================================================
static CliGkConn * GetConnIncRef (const char tag[]) {
    hsLockGuard(s_critsect);
    return GetConnIncRef_CS(tag);
}

//============================================================================
static void AbandonConn(CliGkConn* conn) {
    hsLockGuard(s_critsect);
    conn->abandoned = true;

    conn->StopAutoReconnect();

    if (conn->cancelId) {
        AsyncSocketConnectCancel(conn->cancelId);
        conn->cancelId  = nullptr;
    }
    else if (conn->sock) {
        AsyncSocketDisconnect(conn->sock, true);
    }
    else {
        conn->UnRef("Lifetime");
    }
}

//============================================================================
static bool ConnEncrypt (ENetError error, void * param) {
    CliGkConn * conn = (CliGkConn *) param;
        
    if (IS_NET_SUCCESS(error)) {

        if (!s_perf[kPingDisabled])
            conn->AutoPing();

        {
            hsLockGuard(s_critsect);
            std::swap(s_active, conn);
        }

    //  AuthConnectedNotifyTrans * trans = new AuthConnectedNotifyTrans;
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
        nullptr,
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
        conn->StopAutoReconnect();
        conn->UnRef("Lifetime");
        ReportNetError(kNetProtocolCli2GateKeeper, error);
    }
    else {
        if (conn->token != kNilUuid)
            // previously encrypted; reconnect quickly
            conn->reconnectStartMs = GetNonZeroTimeMs() + 500;
        else
            // never encrypted; reconnect slowly
            conn->reconnectStartMs = GetNonZeroTimeMs() + kMaxReconnectIntervalMs;

        // clean up the socket and start reconnect
        if (conn->cli)
            NetCliDelete(conn->cli, true);
        conn->cli = nullptr;
        conn->sock = nullptr;
        
        conn->StartAutoReconnect();
    }
}

//============================================================================
static void NotifyConnSocketConnectFailed (CliGkConn * conn) {

    {
        hsLockGuard(s_critsect);
        conn->cancelId = nullptr;
        if (s_conn == conn) {
            s_conn = nullptr;
        }

        if (conn == s_active)
            s_active = nullptr;
    }
    
    CheckedReconnect(conn, kNetErrConnectFailed);
    
    conn->UnRef("Connecting");
}

//============================================================================
static void NotifyConnSocketDisconnect (CliGkConn * conn) {

    conn->StopAutoPing();

    {
        hsLockGuard(s_critsect);
        conn->cancelId = nullptr;
        if (s_conn == conn) {
            s_conn = nullptr;
        }
            
        if (conn == s_active)
            s_active = nullptr;
    }

    // Cancel all transactions in process on this connection.
    NetTransCancelByConnId(conn->seq, kNetErrTimeout);
    conn->UnRef("Connected");
    conn->UnRef("Lifetime");
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
            {
                hsLockGuard(s_critsect);
                conn->sock      = sock;
                conn->cancelId  = nullptr;
                abandoned       = conn->abandoned;
            }
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

        case kNotifySocketWrite:
            // No action
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

    {
        hsLockGuard(s_critsect);
        if (CliGkConn* oldConn = s_conn) {
            s_conn = nullptr;
            if (oldConn != conn) {
                AbandonConn(oldConn);
            }
        }
        s_conn = conn;
    }
    
    Cli2GateKeeper_Connect connect;
    connect.hdr.connType        = kConnTypeCliToGateKeeper;
    connect.hdr.hdrBytes        = hsToLE16(sizeof(connect.hdr));
    connect.hdr.buildId         = hsToLE32(plProduct::BuildId());
    connect.hdr.buildType       = hsToLE32(plProduct::BuildType());
    connect.hdr.branchId        = hsToLE32(plProduct::BranchId());
    connect.hdr.productId       = plProduct::UUID();
    connect.data.token          = conn->token;
    connect.data.dataBytes      = hsToLE32(sizeof(connect.data));

    AsyncSocketConnect(
        &conn->cancelId,
        conn->addr,
        SocketNotifyCallback,
        conn,
        &connect,
        sizeof(connect)
    );
}

//============================================================================
static void Connect (
    const ST::string&   name,
    const plNetAddress& addr
) {
    ASSERT(s_running);
    
    CliGkConn * conn        = new CliGkConn;
    conn->addr              = addr;
    conn->seq               = ConnNextSequence();
    conn->lastHeardTimeMs   = GetNonZeroTimeMs();   // used in connect timeout, and ping timeout
    conn->name              = name;

    conn->Ref("Lifetime");
    conn->AutoReconnect();
}


/*****************************************************************************
*
*   CliGkConn
*
***/

//============================================================================
CliGkConn::CliGkConn ()
    : hsRefCnt(0), reconnectTimer(), reconnectStartMs()
    , pingTimer(), pingSendTimeMs(), lastHeardTimeMs()
    , sock(), cli(), seq(), serverChallenge()
    , cancelId(), abandoned()
{
    ++s_perf[kPerfConnCount];
}

//============================================================================
CliGkConn::~CliGkConn () {
    if (cli)
        NetCliDelete(cli, true);
    --s_perf[kPerfConnCount];
}

//===========================================================================
void CliGkConn::TimerReconnect () {
    ASSERT(!sock);
    ASSERT(!cancelId);
    
    if (!s_running) {
        hsLockGuard(s_critsect);
        if (s_conn == this) {
            s_conn = nullptr;
        }
        AbandonConn(this);
    }
    else {
        Ref("Connecting");

        // Remember the time we started the reconnect attempt, guarding against
        // hsTimer::GetMilliSeconds() returning zero (unlikely), as a value of zero indicates
        // a first-time connect condition to StartAutoReconnect()
        reconnectStartMs = GetNonZeroTimeMs();

        Connect(this);
    }
}

//===========================================================================
// This function is called when after a disconnect to start a new connection
void CliGkConn::StartAutoReconnect () {
    hsLockGuard(critsect);
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
            LogMsg(kLogPerf, "GateKeeper auto-reconnecting in {} ms", remainingMs);
        }
        AsyncTimerUpdate(reconnectTimer, remainingMs);
    }
}

//===========================================================================
// This function should be called during object construction
// to initiate connection attempts to the remote host whenever
// the socket is disconnected.
void CliGkConn::AutoReconnect () {
        
    ASSERT(!reconnectTimer);
    Ref("ReconnectTimer");
    hsLockGuard(critsect);
    reconnectTimer = AsyncTimerCreate(0, [this]() { // immediate callback
        TimerReconnect();
        return kAsyncTimeInfinite;
    });
}

//============================================================================
void CliGkConn::StopAutoReconnect () {
    hsLockGuard(critsect);
    if (AsyncTimer * timer = reconnectTimer) {
        reconnectTimer = nullptr;
        AsyncTimerDeleteCallback(timer, [this]() {
            UnRef("ReconnectTimer");
            return kAsyncTimeInfinite;
        });
    }
}

//============================================================================
bool CliGkConn::AutoReconnectEnabled () {
    
    return (reconnectTimer != nullptr) && !s_perf[kAutoReconnectDisabled];
}

//============================================================================
void CliGkConn::AutoPing () {
    ASSERT(!pingTimer);
    Ref("PingTimer");
    hsLockGuard(critsect);
    pingTimer = AsyncTimerCreate(sock ? 0 : kAsyncTimeInfinite, [this]() {
        TimerPing();
        return kPingIntervalMs;
    });
}

//============================================================================
void CliGkConn::StopAutoPing () {
    hsLockGuard(critsect);
    if (pingTimer) {
        AsyncTimerDeleteCallback(pingTimer, [this]() {
            UnRef("PingTimer");
            return kAsyncTimeInfinite;
        });
        pingTimer = nullptr;
    }
}

//============================================================================
void CliGkConn::TimerPing () {
    // Send a ping request
    pingSendTimeMs = GetNonZeroTimeMs();

    const uintptr_t msg[] = {
        kCli2GateKeeper_PingRequest,
        pingSendTimeMs,
        0,      // not a transaction
        0,      // no payload
        reinterpret_cast<uintptr_t>(nullptr)
    };

    Send(msg, std::size(msg));
}

//============================================================================
void CliGkConn::Send (const uintptr_t fields[], unsigned count) {
    hsLockGuard(critsect);
    NetCliSend(cli, fields, count);
    NetCliFlush(cli);
}


/*****************************************************************************
*
*   Cli2GateKeeper message handlers
*
***/

//============================================================================
// (none)
//============================================================================

/*****************************************************************************
*
*   Cli2Auth protocol
*
***/

#define MSG(s)  &kNetMsg_Cli2GateKeeper_##s
static NetMsgInitSend s_send[] = {
    { MSG(PingRequest)              },
    { MSG(FileSrvIpAddressRequest)  },
    { MSG(AuthSrvIpAddressRequest)  },
};
#undef MSG

#define MSG(s)  &kNetMsg_GateKeeper2Cli_##s, NetTransRecvFromMsgGeneric<GateKeeper2Cli_##s>
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
,   m_payload((const uint8_t *)payload, (const uint8_t *)payload + payloadBytes)
{
}

//============================================================================
bool PingRequestTrans::Send () {

    if (!AcquireConn())
        return false;

    const uintptr_t msg[] = {
        kCli2GateKeeper_PingRequest,
                        m_pingAtMs,
                        m_transId,
                        m_payload.size(),
        (uintptr_t)  m_payload.data(),
    };
    
    m_conn->Send(msg, std::size(msg));
    
    return true;
}

//============================================================================
void PingRequestTrans::Post () {

    m_callback(
        m_result,
        m_param,
        m_pingAtMs,
        m_replyAtMs,
        m_payload.size(),
        m_payload.data()
    );
}

//============================================================================
bool PingRequestTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const GateKeeper2Cli_PingReply & reply = *(const GateKeeper2Cli_PingReply *)msg;

    m_payload.assign(reply.payload, reply.payload + reply.payloadBytes);
    m_replyAtMs     = hsTimer::GetMilliSeconds<uint32_t>();
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

    
    const uintptr_t msg[] = {
        kCli2GateKeeper_FileSrvIpAddressRequest,
                        m_transId,
                        (uintptr_t)(m_isPatcher == true ? 1 : 0)
    };
    
    m_conn->Send(msg, std::size(msg));
    
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
    const uint8_t  msg[],
    unsigned    bytes
) {
    const GateKeeper2Cli_FileSrvIpAddressReply & reply = *(const GateKeeper2Cli_FileSrvIpAddressReply *)msg;

    
    m_result        = kNetSuccess;
    m_state         = kTransStateComplete;
    m_addr          = ST::string::from_utf16(reply.address);

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

    const uintptr_t msg[] = {
        kCli2GateKeeper_AuthSrvIpAddressRequest,
                        m_transId,
    };
    
    m_conn->Send(msg, std::size(msg));
    
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
    const uint8_t  msg[],
    unsigned    bytes
) {
    const GateKeeper2Cli_AuthSrvIpAddressReply & reply = *(const GateKeeper2Cli_AuthSrvIpAddressReply *)msg;

    m_result        = kNetSuccess;
    m_state         = kTransStateComplete;
    m_addr          = ST::string::from_utf16(reply.address);

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
,   m_conn()
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
    return m_conn != nullptr;
}

//============================================================================
void NetGateKeeperTrans::ReleaseConn () {
    if (m_conn) {
        m_conn->UnRef("AcquireConn");
        m_conn = nullptr;
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
        s_send, std::size(s_send),
        s_recv, std::size(s_recv),
        kGateKeeperDhGValue,
        plBigNum(sizeof(kGateKeeperDhXData), kGateKeeperDhXData),
        plBigNum(sizeof(kGateKeeperDhNData), kGateKeeperDhNData)
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

    {
        hsLockGuard(s_critsect);
        if (CliGkConn* conn = s_conn) {
            s_conn = nullptr;
            AbandonConn(conn);
        }
        s_active = nullptr;
    }

    if (!wait)
        return;

    while (s_perf[kPerfConnCount]) {
        NetTransUpdate();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

//============================================================================
bool GateKeeperQueryConnected () {
    hsLockGuard(s_critsect);
    return (s_active && s_active->cli);
}

//============================================================================
unsigned GateKeeperGetConnId () {
    hsLockGuard(s_critsect);
    return (s_active) ? s_active->seq : 0;
}


}   using namespace Ngl;


/*****************************************************************************
*
*   Exported functions
*
***/

//============================================================================
void NetCliGateKeeperStartConnect (
    const ST::string gateKeeperAddrList[],
    uint32_t         gateKeeperAddrCount
) {
    gateKeeperAddrCount = std::min(gateKeeperAddrCount, 1u);

    for (unsigned i = 0; i < gateKeeperAddrCount; ++i) {
        // Do we need to lookup the address?
        ST::string name = gateKeeperAddrList[i];
        const char* pos;
        for (pos = name.begin(); pos != name.end(); ++pos) {
            if (!(isdigit(*pos) || *pos == '.' || *pos == ':')) {
                AsyncAddressLookupName(name, GetClientPort(), [name](auto addrs) {
                    if (addrs.empty()) {
                        ReportNetError(kNetProtocolCli2GateKeeper, kNetErrNameLookupFailed);
                        return;
                    }

                    for (const plNetAddress& addr : addrs) {
                        Connect(name, addr);
                    }
                });
                break;
            }
        }
        if (pos == name.end()) {
            plNetAddress addr(name, GetClientPort());
            Connect(name, addr);
        }
    }
}

//============================================================================
void NetCliGateKeeperDisconnect () {
    hsLockGuard(s_critsect);
    if (CliGkConn* conn = s_conn) {
        s_conn = nullptr;
        AbandonConn(conn);
    }
    s_active = nullptr;
}

//============================================================================
void NetCliGateKeeperPingRequest (
    unsigned                                pingTimeMs,
    unsigned                                payloadBytes,
    const void *                            payload,
    FNetCliGateKeeperPingRequestCallback    callback,
    void *                                  param
) {
    PingRequestTrans * trans = new PingRequestTrans(
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
    FileSrvIpAddressRequestTrans * trans = new FileSrvIpAddressRequestTrans(callback, param, isPatcher);
    NetTransSend(trans);
}

//============================================================================
void NetCliGateKeeperAuthSrvIpAddressRequest (
    FNetCliGateKeeperAuthSrvIpAddressRequestCallback    callback,
    void *                                              param
) {
    AuthSrvIpAddressRequestTrans * trans = new AuthSrvIpAddressRequestTrans(callback, param);
    NetTransSend(trans);
}
