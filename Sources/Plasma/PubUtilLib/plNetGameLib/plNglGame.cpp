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

namespace Ngl { namespace Game {
/*****************************************************************************
*
*   Private
*
***/

struct CliGmConn : hsRefCnt, AsyncNotifySocketCallbacks {
    std::recursive_mutex  critsect;
    AsyncSocket     socket;
    AsyncCancelId   cancelId;
    NetCli *        cli;
    plNetAddress    addr;
    unsigned        seq;
    bool            abandoned;

    // ping
    AsyncTimer *    pingTimer;
    unsigned        pingSendTimeMs;
    unsigned        lastHeardTimeMs;

    CliGmConn ();
    ~CliGmConn ();

    // Callbacks
    void AsyncNotifySocketConnectFailed(plNetAddress remoteAddr) override;
    bool AsyncNotifySocketConnectSuccess(AsyncSocket sock, const plNetAddress& localAddr, const plNetAddress& remoteAddr) override;
    void AsyncNotifySocketDisconnect(AsyncSocket sock) override;
    std::optional<size_t> AsyncNotifySocketRead(AsyncSocket sock, uint8_t* buffer, size_t bytes) override;

    // ping
    void AutoPing ();
    void StopAutoPing ();
    void TimerPing ();

    void Send (const uintptr_t fields[], unsigned count);
};


//============================================================================
// JoinAgeRequestTrans
//============================================================================
struct JoinAgeRequestTrans : NetGameTrans {
    FNetCliGameJoinAgeRequestCallback   m_callback;
    // sent
    unsigned                            m_ageMcpId;
    plUUID                              m_accountUuid;
    unsigned                            m_playerInt;

    JoinAgeRequestTrans (
        unsigned                            ageMcpId,
        const plUUID&                       accountUuid,
        unsigned                            playerInt,
        FNetCliGameJoinAgeRequestCallback   callback
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// RcvdPropagatedBufferTrans
//============================================================================
struct RcvdPropagatedBufferTrans : NetNotifyTrans {

    unsigned        bufferType;
    unsigned        bufferBytes;
    uint8_t *          bufferData;

    RcvdPropagatedBufferTrans ()
        : NetNotifyTrans(kGmRcvdPropagatedBufferTrans),
          bufferType(), bufferBytes(), bufferData()
    { }
    ~RcvdPropagatedBufferTrans ();
    void Post() override;
};

//============================================================================
// RcvdGameMgrMsgTrans
//============================================================================
struct RcvdGameMgrMsgTrans : NetNotifyTrans
{

    unsigned bufferBytes;
    uint8_t* bufferData;

    RcvdGameMgrMsgTrans()
        : NetNotifyTrans(kGmRcvdGameMgrMsgTrans),
          bufferBytes(), bufferData()
    { }
    ~RcvdGameMgrMsgTrans();
    void Post() override;
};

/*****************************************************************************
*
*   Private data
*
***/

enum {
    kPerfConnCount,
    kPingDisabled,
    kNumPerf
};

static NetMsgChannel* s_channel;
static bool                             s_running;
static std::recursive_mutex             s_critsect;
static CliGmConn* s_conn = nullptr;
static CliGmConn *                      s_active;
static FNetCliGameRecvBufferHandler     s_bufHandler;
static FNetCliGameRecvGameMgrMsgHandler s_gameMgrMsgHandler;
static std::atomic<long>                s_perf[kNumPerf];


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
static CliGmConn * GetConnIncRef_CS (const char tag[]) {
    if (CliGmConn * conn = s_active)
        if (conn->cli) {
            conn->Ref(tag);
            return conn;
        }
    return nullptr;
}

//============================================================================
static CliGmConn * GetConnIncRef (const char tag[]) {
    hsLockGuard(s_critsect);
    return GetConnIncRef_CS(tag);
}

//============================================================================
static void AbandonConn(CliGmConn* conn) {
    hsLockGuard(s_critsect);
    conn->abandoned = true;
    if (conn->cancelId) {
        AsyncSocketConnectCancel(conn->cancelId);
        conn->cancelId  = nullptr;
    } else if (conn->socket) {
        AsyncSocketDisconnect(conn->socket, true);
    } else {
        conn->UnRef("Lifetime");
    }
}

//============================================================================
bool CliGmConn::AsyncNotifySocketConnectSuccess(AsyncSocket sock, const plNetAddress& localAddr, const plNetAddress& remoteAddr)
{
    bool wasAbandoned;
    {
        hsLockGuard(s_critsect);
        socket = sock;
        cancelId = nullptr;
        wasAbandoned = abandoned;
    }
    if (wasAbandoned) {
        AsyncSocketDisconnect(sock, true);
        return true;
    }

    TransferRef("Connecting", "Connected");
    cli = NetCliConnectAccept(
        sock,
        s_channel,
        true,
        [this](ENetError error) {
            if (!s_perf[kPingDisabled]) {
                AutoPing();
            }

            if (IS_NET_SUCCESS(error)) {
                hsLockGuard(s_critsect);
                s_active = this;
            }

            return IS_NET_SUCCESS(error);
        },
        0,
        nullptr
    );
    return true;
}

//============================================================================
void CliGmConn::AsyncNotifySocketConnectFailed(plNetAddress remoteAddr)
{
    bool notify;
    {
        hsLockGuard(s_critsect);
        cancelId = nullptr;
        if (s_conn == this) {
            s_conn = nullptr;
        }

        notify
            =  s_running
            && !abandoned
            && (!s_active || s_active == this);

        if (s_active == this) {
            s_active = nullptr;
        }
    }

    NetTransCancelByConnId(seq, kNetErrTimeout);
    UnRef("Connecting");
    UnRef("Lifetime");

    if (notify) {
        ReportNetError(kNetProtocolCli2Game, kNetErrConnectFailed);
    }
}

//============================================================================
void CliGmConn::AsyncNotifySocketDisconnect(AsyncSocket sock)
{
    StopAutoPing();

    bool notify;
    {
        hsLockGuard(s_critsect);
        if (s_conn == this) {
            s_conn = nullptr;
        }

        notify
            =  s_running
            && !abandoned
            && (!s_active || s_active == this);

        if (s_active == this) {
            s_active = nullptr;
        }
    }

    // Cancel all transactions in process on this connection.
    NetTransCancelByConnId(seq, kNetErrTimeout);
    UnRef("Connected");
    UnRef("Lifetime");

    // Send a fake GameMgr transaction telling all the GameClis to die.
    {
        Srv2Cli_Game_PlayerLeft msg{};
        msg.messageId = kSrv2Cli_Game_PlayerLeft;
        msg.messageBytes = sizeof(msg);

        RcvdGameMgrMsgTrans* trans = new RcvdGameMgrMsgTrans;
        trans->bufferBytes = msg.messageBytes;
        trans->bufferData = new uint8_t[msg.messageBytes];
        memcpy(trans->bufferData, &msg, msg.messageBytes);
        NetTransSend(trans);
    }

    if (notify) {
        ReportNetError(kNetProtocolCli2Game, kNetErrDisconnected);
    }
}

//============================================================================
std::optional<size_t> CliGmConn::AsyncNotifySocketRead(AsyncSocket sock, uint8_t* buffer, size_t bytes)
{
    // TODO: Only dispatch messages from the active game server
    lastHeardTimeMs = GetNonZeroTimeMs();
    if (!NetCliDispatch(cli, buffer, bytes, nullptr)) {
        return {};
    }
    return bytes;
}

//============================================================================
static void Connect (
    const plNetAddress& addr
) {
    CliGmConn * conn = new CliGmConn;
    conn->addr              = addr;
    conn->seq               = ConnNextSequence();
    conn->lastHeardTimeMs   = GetNonZeroTimeMs();

    conn->Ref("Lifetime");
    conn->Ref("Connecting");

    {
        hsLockGuard(s_critsect);
        if (CliGmConn* oldConn = s_conn) {
            s_conn = nullptr;
            AbandonConn(oldConn);
        }
        s_conn = conn;
    }

    Cli2Game_Connect connect;
    connect.hdr.connType    = kConnTypeCliToGame;
    connect.hdr.hdrBytes    = hsToLE16(sizeof(connect.hdr));
    connect.hdr.buildId     = hsToLE32(plProduct::BuildId());
    connect.hdr.buildType   = hsToLE32(plProduct::BuildType());
    connect.hdr.branchId    = hsToLE32(plProduct::BranchId());
    connect.hdr.productId   = plProduct::UUID();
    connect.data.dataBytes  = hsToLE32(sizeof(connect.data));

    AsyncSocketConnect(
        &conn->cancelId,
        addr,
        conn,
        &connect,
        sizeof(connect)
    );
}


/*****************************************************************************
*
*   CliGmConn
*
***/

//============================================================================
CliGmConn::CliGmConn ()
    : hsRefCnt(0), socket(), cancelId(), cli()
    , seq(), abandoned()
    , pingTimer(), pingSendTimeMs(), lastHeardTimeMs()
{
    ++s_perf[kPerfConnCount];
}

//============================================================================
CliGmConn::~CliGmConn () {
    if (cli)
        NetCliDelete(cli, true);
    --s_perf[kPerfConnCount];
}

//============================================================================
void CliGmConn::AutoPing () {
    ASSERT(!pingTimer);
    Ref("PingTimer");
    hsLockGuard(critsect);
    pingTimer = AsyncTimerCreate(socket ? 0 : kAsyncTimeInfinite, [this]() {
        TimerPing();
        return kPingIntervalMs;
    });
}

//============================================================================
void CliGmConn::StopAutoPing () {
    hsLockGuard(critsect);
    if (pingTimer) {
        AsyncTimerDeleteCallback(pingTimer, [this]() {
            UnRef("PingTimer");
        });
        pingTimer = nullptr;
    }
}

//============================================================================
void CliGmConn::TimerPing () {
    // Send a ping request
    pingSendTimeMs = GetNonZeroTimeMs();

    const uintptr_t msg[] = {
        kCli2Game_PingRequest,
        pingSendTimeMs
    };

    Send(msg, std::size(msg));
}

//============================================================================
void CliGmConn::Send (const uintptr_t fields[], unsigned count) {
    hsLockGuard(critsect);
    NetCliSend(cli, fields, count);
    NetCliFlush(cli);
}


/*****************************************************************************
*
*   Cli2Game protocol
*
***/

template<typename T>
bool RecvMsg(const uint8_t msg[], unsigned bytes, void* param)
{
    // Stupid nested namespaces...
    return ::NetTransRecvFromMsgGeneric<T>(msg, bytes, param);
}

//============================================================================
template<>
bool RecvMsg<Game2Cli_PingReply>(const uint8_t msg[], unsigned bytes, void* param)
{
    return true;
}

//============================================================================
template<>
bool RecvMsg<Game2Cli_PropagateBuffer>(const uint8_t msg[], unsigned bytes, void* param)
{
    const Game2Cli_PropagateBuffer & reply = *(const Game2Cli_PropagateBuffer *)msg;

    RcvdPropagatedBufferTrans * trans = new RcvdPropagatedBufferTrans;
    trans->bufferType   = reply.type;
    trans->bufferBytes  = reply.bytes;
    trans->bufferData   = (uint8_t *)malloc(reply.bytes);
    memcpy(trans->bufferData, reply.buffer, reply.bytes);
    NetTransSend(trans);

    return true;
}

//============================================================================
template<>
bool RecvMsg<Game2Cli_GameMgrMsg>(const uint8_t msg[], unsigned bytes, void* param)
{
    const Game2Cli_GameMgrMsg& reply = *(const Game2Cli_GameMgrMsg*)msg;

    RcvdGameMgrMsgTrans* trans = new RcvdGameMgrMsgTrans;
    trans->bufferBytes = reply.bytes;
    trans->bufferData = new uint8_t[reply.bytes];
    memcpy(trans->bufferData, reply.buffer, reply.bytes);
    NetTransSend(trans);

    return true;
}


//============================================================================
// Send/Recv protocol handler init
//============================================================================
#define MSG(s)  &kNetMsg_Cli2Game_##s
static NetMsgInitSend s_send[] = {
    { MSG(PingRequest),         },
    { MSG(JoinAgeRequest),      },
    { MSG(PropagateBuffer),     },
    { MSG(GameMgrMsg),          },
};
#undef MSG

#define MSG(s)  &kNetMsg_Game2Cli_##s, RecvMsg<Game2Cli_##s>
static NetMsgInitRecv s_recv[] = {
    { MSG(PingReply)            },
    { MSG(JoinAgeReply),        },
    { MSG(PropagateBuffer),     },
    { MSG(GameMgrMsg),          },
};
#undef MSG


/*****************************************************************************
*
*   JoinAgeRequestTrans
*
***/

//============================================================================
JoinAgeRequestTrans::JoinAgeRequestTrans (
    unsigned                            ageMcpId,
    const plUUID&                       accountUuid,
    unsigned                            playerInt,
    FNetCliGameJoinAgeRequestCallback   callback
) : NetGameTrans(kJoinAgeRequestTrans)
,   m_callback(std::move(callback))
,   m_ageMcpId(ageMcpId)
,   m_accountUuid(accountUuid)
,   m_playerInt(playerInt)
{
}

//============================================================================
bool JoinAgeRequestTrans::Send () {
    if (!AcquireConn())
        return false;

    const uintptr_t msg[] = {
        kCli2Game_JoinAgeRequest,
                        m_transId,
                        m_ageMcpId,
        (uintptr_t) &m_accountUuid,
                        m_playerInt,
    };

    m_conn->Send(msg, std::size(msg));
    
    return true;
}

//============================================================================
void JoinAgeRequestTrans::Post () {
    m_callback(m_result);
}

//============================================================================
bool JoinAgeRequestTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const Game2Cli_JoinAgeReply & reply = *(const Game2Cli_JoinAgeReply *) msg;
    m_result        = reply.result;
    m_state         = kTransStateComplete;
    return true;
}

/*****************************************************************************
*
*   RcvdPropagatedBufferTrans
*
***/

//============================================================================
RcvdPropagatedBufferTrans::~RcvdPropagatedBufferTrans () {
    free(bufferData);
}

//============================================================================
void RcvdPropagatedBufferTrans::Post () {
    if (s_bufHandler)
        s_bufHandler(bufferType, bufferBytes, bufferData);
}

/*****************************************************************************
*
*   RcvdGameMgrMsgTrans
*
***/

//============================================================================
RcvdGameMgrMsgTrans::~RcvdGameMgrMsgTrans()
{
    delete[] bufferData;
}

//============================================================================
void RcvdGameMgrMsgTrans::Post()
{
    if (s_gameMgrMsgHandler)
        s_gameMgrMsgHandler((GameMsgHeader*)bufferData);
}

} using namespace Game;


/*****************************************************************************
*
*   NetGameTrans
*
***/

//============================================================================
NetGameTrans::NetGameTrans (ETransType transType)
:   NetTrans(kNetProtocolCli2Game, transType)
,   m_conn()
{
}

//============================================================================
NetGameTrans::~NetGameTrans () {
    ReleaseConn();
}

//============================================================================
bool NetGameTrans::AcquireConn () {
    if (!m_conn)
        m_conn = GetConnIncRef("AcquireConn");
    return m_conn != nullptr;
}

//============================================================================
void NetGameTrans::ReleaseConn () {
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
void GameInitialize () {
    s_running = true;
    ASSERT(!s_channel);
    s_channel = NetMsgChannelCreate(
        kNetProtocolCli2Game,
        s_send, std::size(s_send),
        s_recv, std::size(s_recv),
        gNetGameDhConstants
    );
}

//============================================================================
void GameDestroy (bool wait) {
    s_running = false;
    s_bufHandler = nullptr;
    s_gameMgrMsgHandler = nullptr;

    NetTransCancelByProtocol(
        kNetProtocolCli2Game,
        kNetErrRemoteShutdown
    );
    if (s_channel != nullptr) {
        NetMsgChannelDelete(s_channel);
        s_channel = nullptr;
    }
    
    {
        hsLockGuard(s_critsect);
        if (CliGmConn* conn = s_conn) {
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
bool GameQueryConnected () {
    hsLockGuard(s_critsect);
    return (s_active && s_active->cli);
}

//============================================================================
unsigned GameGetConnId () {
    hsLockGuard(s_critsect);
    return (s_active) ? s_active->seq : 0;
}

//============================================================================
void GamePingEnable (bool enable) {
    s_perf[kPingDisabled] = !enable;
    hsLockGuard(s_critsect);
    for (;;) {
        if (!s_active)
            break;
        if (enable)
            s_active->AutoPing();
        else
            s_active->StopAutoPing();
        break;
    }
}

} using namespace Ngl;


/*****************************************************************************
*
*   Exported functions
*
***/

//============================================================================
void NetCliGameStartConnect (
    const uint32_t node
) {
    plNetAddress addr(node, GetClientPort());
    Connect(addr);
}

//============================================================================
void NetCliGameDisconnect () {
    hsLockGuard(s_critsect);
    if (CliGmConn* conn = s_conn) {
        s_conn = nullptr;
        AbandonConn(conn);
    }
    s_active = nullptr;
}

//============================================================================
void NetCliGameJoinAgeRequest (
    unsigned                            ageMcpId,
    const plUUID&                       accountUuid,
    unsigned                            playerInt,
    FNetCliGameJoinAgeRequestCallback   callback
) {
    JoinAgeRequestTrans * trans = new JoinAgeRequestTrans(
        ageMcpId,
        accountUuid,
        playerInt,
        std::move(callback)
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliGameSetRecvBufferHandler (
    FNetCliGameRecvBufferHandler    handler
) {
    s_bufHandler = std::move(handler);
}

//============================================================================
void NetCliGamePropagateBuffer (
    unsigned                        type,
    unsigned                        bytes,
    const uint8_t                      buffer[]
) {
    CliGmConn * conn = GetConnIncRef("PropBuffer");
    if (!conn)
        return;

    const uintptr_t msg[] = {
        kCli2Game_PropagateBuffer,
        type,
        bytes,
        (uintptr_t) buffer,
    };

    conn->Send(msg, std::size(msg));

    conn->UnRef("PropBuffer");
}

//============================================================================
void NetCliGameSetRecvGameMgrMsgHandler(FNetCliGameRecvGameMgrMsgHandler handler)
{
    s_gameMgrMsgHandler = std::move(handler);
}

//============================================================================
void NetCliGameSendGameMgrMsg(const GameMsgHeader* msgHdr)
{
    CliGmConn* conn = GetConnIncRef("GameMgrMsg");
    if (!conn)
        return;

    const uintptr_t msg[] = {
        kCli2Game_GameMgrMsg,
        msgHdr->messageBytes,
        (uintptr_t)msgHdr,
    };

    conn->Send(msg, std::size(msg));

    conn->UnRef("GameMgrMsg");
}
