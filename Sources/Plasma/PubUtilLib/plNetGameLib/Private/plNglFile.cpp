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
*   $/Plasma20/Sources/Plasma/PubUtilLib/plNetGameLib/Private/plNglFile.cpp
*   
***/

#include "../Pch.h"

#include "pnUtils/pnUtStr.h"

// Define this if the file servers are running behind load-balancing hardware.
// It changes the logic by which the decision to attempt a reconnect is made.
#define LOAD_BALANCER_HARDWARE

static void StrCopyLE16(char16_t* dest, const char16_t source[], size_t chars) {
    while ((chars > 1) && ((*dest = hsToLE16(*source++)) != 0)) {
        --chars;
        ++dest;
    }
    if (chars)
        *dest = 0;
}


namespace Ngl { namespace File {
/*****************************************************************************
*
*   Private
*
***/

struct CliFileConn : hsRefCnt {
    hsReaderWriterLock  socketLock; // to protect the socket pointer so we don't nuke it while using it
    AsyncSocket         socket;
    ST::string          name;
    plNetAddress        addr;
    unsigned            seq;
    std::vector<uint8_t> recvBuffer;
    AsyncCancelId       cancelId;
    bool                abandoned;
    unsigned            buildId;
    unsigned            serverType;

    std::recursive_mutex timerCritsect; // critsect for both timers

    // Reconnection
    AsyncTimer *        reconnectTimer;
    unsigned            reconnectStartMs;
    unsigned            connectStartMs;
    unsigned            numImmediateDisconnects;
    unsigned            numFailedConnects;

    // Ping
    AsyncTimer *        pingTimer;
    unsigned            pingSendTimeMs;
    unsigned            lastHeardTimeMs;

    CliFileConn ();
    ~CliFileConn ();

    // Callbacks
    void NotifyConnSocketConnect();
    void NotifyConnSocketConnectFailed();
    void NotifyConnSocketDisconnect();
    bool NotifyConnSocketRead(AsyncNotifySocketRead * read);
    bool SocketNotifyCallback(AsyncSocket sock, EAsyncNotifySocket code, AsyncNotifySocket* notify);

    // This function should be called during object construction
    // to initiate connection attempts to the remote host whenever
    // the socket is disconnected.
    void AutoReconnect ();
    bool AutoReconnectEnabled() { return (reconnectTimer != nullptr); }
    void StopAutoReconnect (); // call before destruction
    void StartAutoReconnect ();
    void TimerReconnect ();

    // ping
    void AutoPing ();
    void StopAutoPing ();
    void TimerPing ();
    
    void Send (const void * data, unsigned bytes);

    void Destroy(); // cleans up the socket and buffer

    void Dispatch (Cli2File_MsgHeader * msg);
    bool Recv_PingReply (File2Cli_PingReply * msg);
    bool Recv_BuildIdReply (File2Cli_BuildIdReply * msg);
    bool Recv_BuildIdUpdate (File2Cli_BuildIdUpdate * msg);
    bool Recv_ManifestReply (File2Cli_ManifestReply * msg);
    bool Recv_FileDownloadReply (File2Cli_FileDownloadReply * msg);
};


//============================================================================
// BuildIdRequestTrans
//============================================================================
struct BuildIdRequestTrans : NetFileTrans {
    FNetCliFileBuildIdRequestCallback   m_callback;
    void *                              m_param;

    unsigned                            m_buildId;
    
    BuildIdRequestTrans (
        FNetCliFileBuildIdRequestCallback   callback,
        void *                              param
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// ManifestRequestTrans
//============================================================================
struct ManifestRequestTrans : NetFileTrans {
    FNetCliFileManifestRequestCallback  m_callback;
    void *                              m_param;
    char16_t                            m_group[kNetDefaultStringSize];
    unsigned                            m_buildId;

    std::vector<NetCliFileManifestEntry>  m_manifest;
    unsigned                            m_numEntriesReceived;

    ManifestRequestTrans (
        FNetCliFileManifestRequestCallback  callback,
        void *                              param,
        const char16_t                      group[],
        unsigned                            buildId
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// DownloadRequestTrans
//============================================================================
struct DownloadRequestTrans : NetFileTrans {
    FNetCliFileDownloadRequestCallback  m_callback;
    void *                              m_param;

    plFileName                          m_filename;
    hsStream *                          m_writer;
    unsigned                            m_buildId;
    
    unsigned                            m_totalBytesReceived;

    DownloadRequestTrans (
        FNetCliFileDownloadRequestCallback  callback,
        void *                              param,
        const plFileName &                  filename,
        hsStream *                          writer,
        unsigned                            buildId
    );

    bool Send() override;
    void Post() override;
    bool Recv(
        const uint8_t  msg[],
        unsigned    bytes
    ) override;
};

//============================================================================
// RcvdFileDownloadChunkTrans
//============================================================================
struct RcvdFileDownloadChunkTrans : NetNotifyTrans {

    unsigned    bytes;
    uint8_t *      data;
    hsStream *  writer;

    RcvdFileDownloadChunkTrans()
        : NetNotifyTrans(kFileRcvdFileDownloadChunkTrans),
          bytes(), data(), writer()
    { }
    ~RcvdFileDownloadChunkTrans ();
    void Post() override;
};


/*****************************************************************************
*
*   Private data
*
***/

enum {
    kPerfConnCount,
    kNumPerf
};

static bool                         s_running;
static std::recursive_mutex         s_critsect;
static CliFileConn* s_conn = nullptr;
static CliFileConn *                s_active;
static std::atomic<long>            s_perf[kNumPerf];
static unsigned                     s_connectBuildId;
static unsigned                     s_serverType;

static FNetCliFileBuildIdUpdateCallback s_buildIdCallback = nullptr;

const unsigned kMinValidConnectionMs                = 25 * 1000;



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
static CliFileConn * GetConnIncRef_CS (const char tag[]) {
    if (CliFileConn * conn = s_active) {
        conn->Ref(tag);
        return conn;
    }
    return nullptr;
}

//============================================================================
static CliFileConn * GetConnIncRef (const char tag[]) {
    hsLockGuard(s_critsect);
    return GetConnIncRef_CS(tag);
}

//============================================================================
static void AbandonConn(CliFileConn* conn) {
    hsLockGuard(s_critsect);
    conn->abandoned = true;

    if (conn->AutoReconnectEnabled())
        conn->StopAutoReconnect();

    bool needsDecref = true;
    if (conn->cancelId) {
        AsyncSocketConnectCancel(conn->cancelId);
        conn->cancelId  = nullptr;
        needsDecref = false;
    }
    else {
        hsLockForReading lock(conn->socketLock);
        if (conn->socket) {
            AsyncSocketDisconnect(conn->socket, true);
            needsDecref = false;
        }
    }
    if (needsDecref) {
        conn->UnRef("Lifetime");
    }
}

//============================================================================
void CliFileConn::NotifyConnSocketConnect()
{
    TransferRef("Connecting", "Connected");
    connectStartMs = hsTimer::GetMilliSeconds<uint32_t>();
    numFailedConnects = 0;

    // Make this the active server
    hsLockGuard(s_critsect);
    if (!abandoned) {
        AutoPing();
        s_active = this;
    } else {
        hsLockForReading lock(socketLock);
        AsyncSocketDisconnect(socket, true);
    }
}

//============================================================================
void CliFileConn::NotifyConnSocketConnectFailed()
{
    {
        hsLockGuard(s_critsect);
        cancelId = nullptr;
        if (s_conn == this) {
            s_conn = nullptr;
        }

        if (s_active == this) {
            s_active = nullptr;
        }
    }

    // Cancel all transactions in progress on this connection.
    NetTransCancelByConnId(seq, kNetErrTimeout);

    // Client apps fail if unable to connect for a time
    if (++numFailedConnects >= kMaxFailedConnects) {
        ReportNetError(kNetProtocolCli2File, kNetErrConnectFailed);
    } else {
        // start reconnect, if we are doing that
        if (s_running && AutoReconnectEnabled()) {
            StartAutoReconnect();
        } else {
            UnRef("Lifetime"); // if we are not reconnecting, this socket is done, so remove the lifetime ref
        }
    }
    UnRef("Connecting");
}

//============================================================================
void CliFileConn::NotifyConnSocketDisconnect()
{
    StopAutoPing();
    {
        hsLockGuard(s_critsect);
        cancelId = nullptr;
        if (s_conn == this) {
            s_conn = nullptr;
        }

        if (s_active == this) {
            s_active = nullptr;
        }
    }

    // Cancel all transactions in progress on this connection.
    NetTransCancelByConnId(seq, kNetErrTimeout);

    bool notify = false;

    {
    #ifndef LOAD_BALANCER_HARDWARE
        // If the connection to the remote server was open for longer than
        // kMinValidConnectionMs then assume that the connection was to
        // a valid server and try to perform reconnection immediately. If
        // less time elapsed then the connection was likely to a server
        // with an open port but with no notification procedure registered
        // for this type of communication channel.
        if (hsTimer::GetMilliSeconds<uint32_t>() - connectStartMs > kMinValidConnectionMs) {
            reconnectStartMs = 0;
        } else {
            if (++numImmediateDisconnects < kMaxImmediateDisconnects) {
                reconnectStartMs = GetNonZeroTimeMs() + kMaxReconnectIntervalMs;
            } else {
                notify = true;
            }
        }
    #else
        // File server is running behind a load-balancer, so the next connection may
        // send us to a new server, therefore attempt a reconnection to the same
        // address even if the disconnect was immediate.  This is safe because the
        // file server is stateless with respect to clients.
        if (hsTimer::GetMilliSeconds<uint32_t>() - connectStartMs <= kMinValidConnectionMs) {
            if (++numImmediateDisconnects < kMaxImmediateDisconnects) {
                reconnectStartMs = GetNonZeroTimeMs() + kMaxReconnectIntervalMs;
            } else {
                notify = true;
            }
        } else {
            // disconnect was not immediate. attempt a reconnect unless we're shutting down
            numImmediateDisconnects = 0;
            reconnectStartMs = 0;
        }
    #endif  // LOAD_BALANCER
    }

    if (notify) {
        ReportNetError(kNetProtocolCli2File, kNetErrDisconnected);
    } else {  
        // clean up the socket and start reconnect, if we are doing that
        Destroy();
        if (AutoReconnectEnabled()) {
            StartAutoReconnect();
        } else {
            UnRef("Lifetime"); // if we are not reconnecting, this socket is done, so remove the lifetime ref
        }
    }

    UnRef("Connected");
}

//============================================================================
bool CliFileConn::NotifyConnSocketRead(AsyncNotifySocketRead* read)
{
    lastHeardTimeMs = GetNonZeroTimeMs();
    recvBuffer.insert(recvBuffer.end(), read->buffer, read->buffer + read->bytes);
    read->bytesProcessed += read->bytes;

    for (;;) {
        if (recvBuffer.size() < sizeof(uint32_t)) {
            return true;
        }

        uint32_t msgSize = hsToLE32(*(uint32_t*)recvBuffer.data());
        if (recvBuffer.size() < msgSize) {
            return true;
        }

        Cli2File_MsgHeader* msg = (Cli2File_MsgHeader*)recvBuffer.data();
        Dispatch(msg);

        recvBuffer.erase(recvBuffer.begin(), recvBuffer.begin() + msgSize);
    }
}

//============================================================================
bool CliFileConn::SocketNotifyCallback(
    AsyncSocket sock,
    EAsyncNotifySocket code,
    AsyncNotifySocket* notify
) {
    bool result = true;
    switch (code) {
        case kNotifySocketConnectSuccess:
            {
                hsLockGuard(s_critsect);
                hsLockForWriting lock(socketLock);
                socket = sock;
                cancelId = nullptr;
            }
            NotifyConnSocketConnect();
        break;

        case kNotifySocketConnectFailed:
            NotifyConnSocketConnectFailed();
        break;

        case kNotifySocketDisconnect:
            NotifyConnSocketDisconnect();
        break;

        case kNotifySocketRead:
            result = NotifyConnSocketRead((AsyncNotifySocketRead*)notify);
        break;
    }

    return result;
}

//============================================================================
static void Connect (CliFileConn * conn) {
    ASSERT(s_running);

    conn->pingSendTimeMs = 0;

    {
        hsLockGuard(s_critsect);
        if (CliFileConn* oldConn = s_conn) {
            s_conn = nullptr;
            if (oldConn != conn) {
                AbandonConn(oldConn);
            }
        }
        s_conn = conn;
    }

    Cli2File_Connect connect;
    connect.hdr.connType    = kConnTypeCliToFile;
    connect.hdr.hdrBytes    = hsToLE16(sizeof(connect.hdr));
    connect.hdr.buildId     = hsToLE32(kFileSrvBuildId);
    connect.hdr.buildType   = hsToLE32(plProduct::BuildType());
    connect.hdr.branchId    = hsToLE32(plProduct::BranchId());
    connect.hdr.productId   = plProduct::UUID();
    connect.data.buildId    = hsToLE32(conn->buildId);
    connect.data.serverType = hsToLE32(conn->serverType);
    connect.data.dataBytes  = hsToLE32(sizeof(connect.data));

    AsyncSocketConnect(
        &conn->cancelId,
        conn->addr,
        [conn](auto sock, auto code, auto notify) {
            return conn->SocketNotifyCallback(sock, code, notify);
        },
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
    
    CliFileConn * conn = new CliFileConn;
    conn->name            = name;
    conn->addr            = addr;
    conn->buildId         = s_connectBuildId;
    conn->serverType      = s_serverType;
    conn->seq             = ConnNextSequence();
    conn->lastHeardTimeMs = GetNonZeroTimeMs();   // used in connect timeout, and ping timeout

    conn->Ref("Lifetime");
    conn->AutoReconnect();
}

/*****************************************************************************
*
*   CliFileConn
*
***/

//============================================================================
CliFileConn::CliFileConn ()
    : hsRefCnt(0), socket(), seq(), cancelId(), abandoned()
    , buildId(), serverType()
    , reconnectTimer(), reconnectStartMs(), connectStartMs()
    , numImmediateDisconnects(), numFailedConnects()
    , pingTimer(), pingSendTimeMs(), lastHeardTimeMs()
{
    ++s_perf[kPerfConnCount];
}

//============================================================================
CliFileConn::~CliFileConn () {
    ASSERT(!cancelId);
    ASSERT(!reconnectTimer);
    Destroy();
    --s_perf[kPerfConnCount];
}

//===========================================================================
void CliFileConn::TimerReconnect () {
    ASSERT(!socket);
    ASSERT(!cancelId);
    
    if (!s_running) {
        hsLockGuard(s_critsect);
        if (s_conn == this) {
            s_conn = nullptr;
            AbandonConn(this);
        }
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
void CliFileConn::StartAutoReconnect () {
    hsLockGuard(timerCritsect);
    if (reconnectTimer) {
        // Make reconnect attempts at regular intervals. If the last attempt
        // took more than the specified max interval time then reconnect
        // immediately; otherwise wait until the time interval is up again
        // then reconnect.
        unsigned remainingMs = 0;
        if (reconnectStartMs) {
            remainingMs = reconnectStartMs - GetNonZeroTimeMs();
            if ((signed)remainingMs < 0)
                remainingMs = 0;
        }
        AsyncTimerUpdate(reconnectTimer, remainingMs);
    }
}

//===========================================================================
// This function should be called during object construction
// to initiate connection attempts to the remote host whenever
// the socket is disconnected.
void CliFileConn::AutoReconnect () {
    hsLockGuard(timerCritsect);
    ASSERT(!reconnectTimer);
    Ref("ReconnectTimer");
    reconnectTimer = AsyncTimerCreate(0, [this]() { // immediate callback
        TimerReconnect();
        return kAsyncTimeInfinite;
    });
}

//============================================================================
void CliFileConn::StopAutoReconnect () {
    hsLockGuard(timerCritsect);
    if (AsyncTimer * timer = reconnectTimer) {
        reconnectTimer = nullptr;
        AsyncTimerDeleteCallback(timer, [this]() {
            UnRef("ReconnectTimer");
            return kAsyncTimeInfinite;
        });
    }
}

//============================================================================
void CliFileConn::AutoPing () {
    ASSERT(!pingTimer);
    Ref("PingTimer");
    hsLockGuard(timerCritsect);
    unsigned timerPeriod;
    {
        hsLockForReading lock(socketLock);
        timerPeriod = socket ? 0 : kAsyncTimeInfinite;
    }

    pingTimer = AsyncTimerCreate(timerPeriod, [this]() {
        TimerPing();
        return kPingIntervalMs;
    });
}

//============================================================================
void CliFileConn::StopAutoPing () {
    hsLockGuard(timerCritsect);
    if (AsyncTimer * timer = pingTimer) {
        pingTimer = nullptr;
        AsyncTimerDeleteCallback(timer, [this]() {
            UnRef("PingTimer");
            return kAsyncTimeInfinite;
        });
    }
}

//============================================================================
void CliFileConn::TimerPing () {
    hsLockForReading lock(socketLock);

    for (;;) {
        if (!socket) // make sure it exists
            break;
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

            Cli2File_PingRequest msg;
            msg.messageId = hsToLE32(kCli2File_PingRequest);
            msg.messageBytes = hsToLE32(sizeof(msg));
            msg.pingTimeMs = hsToLE32(pingSendTimeMs);

            // read locks are reentrant, so calling Send is ok here within the read lock
            Send(&msg, sizeof(msg));
        }
        break;
    }
}

//============================================================================
void CliFileConn::Destroy () {
    AsyncSocket oldSock = nullptr;

    {
        hsLockForWriting lock(socketLock);
        std::swap(oldSock, socket);
    }

    if (oldSock)
        AsyncSocketDelete(oldSock);
    recvBuffer.clear();
}

//============================================================================
void CliFileConn::Send (const void * data, unsigned bytes) {
    hsLockForReading lock(socketLock);

    if (socket) {
        AsyncSocketSend(socket, data, bytes);
    }
}

//============================================================================
void CliFileConn::Dispatch (Cli2File_MsgHeader * msg) {

#define DISPATCH(a) case kFile2Cli_##a: Recv_##a((File2Cli_##a *) msg); break
    switch (hsToLE32(msg->messageId)) {
        DISPATCH(PingReply);
        DISPATCH(BuildIdReply);
        DISPATCH(BuildIdUpdate);
        DISPATCH(ManifestReply);
        DISPATCH(FileDownloadReply);
        DEFAULT_FATAL(msg->messageId)
    }
#undef DISPATCH
}

//============================================================================
bool CliFileConn::Recv_PingReply (
    File2Cli_PingReply* msg
) {
    return true;
}

//============================================================================
bool CliFileConn::Recv_BuildIdReply (
    File2Cli_BuildIdReply* msg
) {
    msg->messageId = hsToLE32(msg->messageId);
    msg->messageBytes = hsToLE32(msg->messageBytes);
    msg->transId = hsToLE32(msg->transId);
    msg->result = (ENetError)hsToLE32(msg->result);
    msg->buildId = hsToLE32(msg->buildId);

    NetTransRecv(msg->transId, (const uint8_t *)msg, msg->messageBytes);

    return true;
}

//============================================================================
bool CliFileConn::Recv_BuildIdUpdate (
    File2Cli_BuildIdUpdate* msg
) {
    if (s_buildIdCallback)
        s_buildIdCallback(hsToLE32(msg->buildId));
    return true;
}

//============================================================================
bool CliFileConn::Recv_ManifestReply (
    File2Cli_ManifestReply* msg
) {
    msg->messageId = hsToLE32(msg->messageId);
    msg->messageBytes = hsToLE32(msg->messageBytes);
    msg->transId = hsToLE32(msg->transId);
    msg->result = (ENetError)hsToLE32(msg->result);
    msg->readerId = hsToLE32(msg->readerId);
    msg->numFiles = hsToLE32(msg->numFiles);
    msg->wcharCount = hsToLE32(msg->wcharCount);

    // The manifest format includes \0 characters, so we can't just StrCopy
    for (size_t i = 0; i < msg->wcharCount; i++) {
        msg->manifestData[i] = hsToLE16(msg->manifestData[i]);
    }

    NetTransRecv(msg->transId, (const uint8_t *)msg, msg->messageBytes);

    return true;
}

//============================================================================
bool CliFileConn::Recv_FileDownloadReply (
    File2Cli_FileDownloadReply* msg
) {
    msg->messageId = hsToLE32(msg->messageId);
    msg->messageBytes = hsToLE32(msg->messageBytes);
    msg->transId = hsToLE32(msg->transId);
    msg->result = (ENetError)hsToLE32(msg->result);
    msg->readerId = hsToLE32(msg->readerId);
    msg->totalFileSize = hsToLE32(msg->totalFileSize);
    msg->byteCount = hsToLE32(msg->byteCount);

    NetTransRecv(msg->transId, (const uint8_t *)msg, msg->messageBytes);

    return true;
}


/*****************************************************************************
*
*   BuildIdRequestTrans
*
***/

//============================================================================
BuildIdRequestTrans::BuildIdRequestTrans(
        FNetCliFileBuildIdRequestCallback callback, void* param)
    : NetFileTrans(kBuildIdRequestTrans),
      m_callback(callback), m_param(param), m_buildId()
{ }

//============================================================================
bool BuildIdRequestTrans::Send () {
    if (!AcquireConn())
        return false;

    Cli2File_BuildIdRequest buildIdReq;
    buildIdReq.messageId = hsToLE32(kCli2File_BuildIdRequest);
    buildIdReq.transId = hsToLE32(m_transId);
    buildIdReq.messageBytes = hsToLE32(sizeof(buildIdReq));

    m_conn->Send(&buildIdReq, sizeof(buildIdReq));

    return true;
}

//============================================================================
void BuildIdRequestTrans::Post () {
    m_callback(m_result, m_param, m_buildId);
}

//============================================================================
bool BuildIdRequestTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    const File2Cli_BuildIdReply & reply = *(const File2Cli_BuildIdReply *) msg;

    if (IS_NET_ERROR(reply.result)) {
        // we have a problem...
        m_result    = reply.result;
        m_state     = kTransStateComplete;
        return true;
    }

    m_buildId = reply.buildId;

    // mark as complete
    m_result    = reply.result;
    m_state     = kTransStateComplete;

    return true;
}


/*****************************************************************************
*
*   ManifestRequestTrans
*
***/

//============================================================================
ManifestRequestTrans::ManifestRequestTrans (
    FNetCliFileManifestRequestCallback  callback,
    void *                              param,
    const char16_t                      group[],
    unsigned                            buildId
) : NetFileTrans(kManifestRequestTrans)
,   m_callback(callback)
,   m_param(param)
,   m_numEntriesReceived(0)
,   m_buildId(buildId)
{
    if (group)
        StrCopy(m_group, group, std::size(m_group));
    else
        m_group[0] = L'\0';
}

//============================================================================
bool ManifestRequestTrans::Send () {
    if (!AcquireConn())
        return false;

    Cli2File_ManifestRequest manifestReq;
    StrCopyLE16(manifestReq.group, m_group, std::size(manifestReq.group));
    manifestReq.messageId = hsToLE32(kCli2File_ManifestRequest);
    manifestReq.transId = hsToLE32(m_transId);
    manifestReq.messageBytes = hsToLE32(sizeof(manifestReq));
    manifestReq.buildId = hsToLE32(m_buildId);

    m_conn->Send(&manifestReq, sizeof(manifestReq));

    return true;
}

//============================================================================
void ManifestRequestTrans::Post () {
    m_callback(m_result, m_param, m_group, m_manifest.data(), m_manifest.size());
}

// Neither char_traits nor C's string library have a "strnlen" equivalent for
// char16_t strings...
inline size_t FIXME_u16snlen(const char16_t* str, size_t maxlen)
{
    const char16_t* end = std::char_traits<char16_t>::find(str, maxlen, 0);
    return end ? size_t(end - str) : maxlen;
}

//============================================================================
void ReadStringFromMsg(const char16_t* curMsgPtr, char16_t* destPtr, unsigned* length) {
    if (!(*length)) {
        size_t maxlen = FIXME_u16snlen(curMsgPtr, kNetDefaultStringSize - 1);   // Hacky sack
        (*length) = maxlen;
        destPtr[maxlen] = 0;    // Don't do this on fixed length, because there's no room for it
    }
    memcpy(destPtr, curMsgPtr, *length * sizeof(char16_t));
}

//============================================================================
void ReadUnsignedFromMsg(const char16_t* curMsgPtr, unsigned* val) {
    (*val) = ((*curMsgPtr) << 16) + (*(curMsgPtr + 1));
}

//============================================================================
bool ManifestRequestTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    m_timeoutAtMs = hsTimer::GetMilliSeconds<uint32_t>() + NetTransGetTimeoutMs(); // Reset the timeout counter

    const File2Cli_ManifestReply & reply = *(const File2Cli_ManifestReply *) msg;

    uint32_t numFiles = reply.numFiles;
    uint32_t wcharCount = reply.wcharCount;
    const char16_t* curChar = reply.manifestData; // the pointer is not yet dereferenced here!

    // tell the server we got the data
    Cli2File_ManifestEntryAck manifestAck;
    manifestAck.messageId = hsToLE32(kCli2File_ManifestEntryAck);
    manifestAck.transId = hsToLE32(reply.transId);
    manifestAck.messageBytes = hsToLE32(sizeof(manifestAck));
    manifestAck.readerId = hsToLE32(reply.readerId);

    m_conn->Send(&manifestAck, sizeof(manifestAck));

    // if wcharCount is 2 or less, the data only contains the terminator "\0\0" and we
    // don't need to convert anything (and we are done)
    if ((IS_NET_ERROR(reply.result)) || (wcharCount <= 2)) {
        // we have a problem... or we have nothing to so, so we're done
        m_result    = reply.result;
        m_state     = kTransStateComplete;
        return true;
    }

    if (numFiles > m_manifest.size())
        m_manifest.resize(numFiles); // reserve the space ahead of time

    // manifestData format: "clientFile\0downloadFile\0md5\0filesize\0zipsize\0flags\0...\0\0"
    bool done = false;
    while (!done) {
        if (wcharCount == 0)
        {
            done = true;
            break;
        }

        // copy the data over to our array (m_numEntriesReceived is the current index)
        NetCliFileManifestEntry& entry = m_manifest[m_numEntriesReceived];

        // --------------------------------------------------------------------
        // read in the clientFilename
        unsigned filenameLen = 0;
        ReadStringFromMsg(curChar, entry.clientName, &filenameLen);
        curChar += filenameLen; // advance the pointer
        wcharCount -= filenameLen; // keep track of the amount remaining
        if ((*curChar != L'\0') || (wcharCount <= 0))
            return false; // something is screwy, abort and disconnect

        // point it at the downloadFile
        curChar++;
        wcharCount--;

        // --------------------------------------------------------------------
        // read in the downloadFilename
        filenameLen = 0;
        ReadStringFromMsg(curChar, entry.downloadName, &filenameLen);
        curChar += filenameLen; // advance the pointer
        wcharCount -= filenameLen; // keep track of the amount remaining
        if ((*curChar != L'\0') || (wcharCount <= 0))
            return false; // something is screwy, abort and disconnect

        // point it at the md5
        curChar++;
        wcharCount--;

        // --------------------------------------------------------------------
        // read in the md5
        filenameLen = 32;
        ReadStringFromMsg(curChar, entry.md5, &filenameLen);
        curChar += filenameLen; // advance the pointer
        wcharCount -= filenameLen; // keep track of the amount remaining
        if ((*curChar != L'\0') || (wcharCount <= 0))
            return false; // something is screwy, abort and disconnect

        // point it at the md5 for compressed files
        curChar++; 
        wcharCount--;

        // --------------------------------------------------------------------
        // read in the md5 for compressed files
        filenameLen = 32;
        ReadStringFromMsg(curChar, entry.md5compressed, &filenameLen);
        curChar += filenameLen; // advance the pointer
        wcharCount -= filenameLen; // keep track of the amount remaining
        if ((*curChar != L'\0') || (wcharCount <= 0))
            return false; // something is screwy, abort and disconnect

        // point it at the first part of the filesize value (format: 0xHHHHLLLL)
        curChar++; 
        wcharCount--;

        // --------------------------------------------------------------------
        if (wcharCount < 2) // we have to have 2 chars for the size
            return false; // screwy data
        ReadUnsignedFromMsg(curChar, &entry.fileSize);
        curChar += 2;
        wcharCount -= 2;
        if ((*curChar != L'\0') || (wcharCount <= 0))
            return false; // screwy data

        // point it at the first part of the zipsize value (format: 0xHHHHLLLL)
        curChar++; 
        wcharCount--;

        // --------------------------------------------------------------------
        if (wcharCount < 2) // we have to have 2 chars for the size
            return false; // screwy data
        ReadUnsignedFromMsg(curChar, &entry.zipSize);
        curChar += 2;
        wcharCount -= 2;
        if ((*curChar != L'\0') || (wcharCount <= 0))
            return false; // screwy data

        // point it at the first part of the flags value (format: 0xHHHHLLLL)
        curChar++; 
        wcharCount--;

        // --------------------------------------------------------------------
        if (wcharCount < 2) // we have to have 2 chars for the size
            return false; // screwy data
        ReadUnsignedFromMsg(curChar, &entry.flags);
        curChar += 2;
        wcharCount -= 2;
        if ((*curChar != L'\0') || (wcharCount <= 0))
            return false; // screwy data

        // --------------------------------------------------------------------
        // point it at either the second part of the terminator, or the next filename
        curChar++;
        wcharCount--;

        // do sanity checking
        if (*curChar == L'\0') {
            // we hit the terminator
            if (wcharCount != 1)
                return false; // invalid data, we shouldn't have any more
            done = true; // we're done
        }
        else if (wcharCount < 14)
            // we must have at least three 1-char strings, three nulls, three 32-bit ints, and 2-char terminator left (3+3+6+2)
            return false; // screwy data

        // increment entries received
        m_numEntriesReceived++;
        if ((m_numEntriesReceived >= numFiles) && !done) {
            // too much data, abort
            return false;
        }
    }
    
    // check for completion
    if (m_numEntriesReceived >= numFiles)
    {
        // all entires received, mark as complete
        m_result    = reply.result;
        m_state     = kTransStateComplete;
    }
    return true;
}

/*****************************************************************************
*
*   FileDownloadRequestTrans
*
***/

//============================================================================
DownloadRequestTrans::DownloadRequestTrans (
    FNetCliFileDownloadRequestCallback  callback,
    void *                              param,
    const plFileName &                  filename,
    hsStream *                          writer,
    unsigned                            buildId
) : NetFileTrans(kDownloadRequestTrans)
,   m_callback(callback)
,   m_param(param)
,   m_filename(filename)
,   m_writer(writer)
,   m_totalBytesReceived(0)
,   m_buildId(buildId)
{
    // This transaction issues "sub transactions" which must complete
    // before this one even though they were issued after us.
    m_hasSubTrans = true;
}

//============================================================================
bool DownloadRequestTrans::Send () {
    if (!AcquireConn())
        return false;

    Cli2File_FileDownloadRequest filedownloadReq;
    const ST::utf16_buffer buffer = m_filename.AsString().to_utf16();
    StrCopyLE16(filedownloadReq.filename, buffer.data(), std::size(filedownloadReq.filename));
    filedownloadReq.messageId = hsToLE32(kCli2File_FileDownloadRequest);
    filedownloadReq.transId = hsToLE32(m_transId);
    filedownloadReq.messageBytes = hsToLE32(sizeof(filedownloadReq));
    filedownloadReq.buildId = hsToLE32(m_buildId);

    m_conn->Send(&filedownloadReq, sizeof(filedownloadReq));
    
    return true;
}

//============================================================================
void DownloadRequestTrans::Post () {
    m_callback(m_result, m_param, m_filename, m_writer);
}

//============================================================================
bool DownloadRequestTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    m_timeoutAtMs = hsTimer::GetMilliSeconds<uint32_t>() + NetTransGetTimeoutMs(); // Reset the timeout counter

    const File2Cli_FileDownloadReply & reply = *(const File2Cli_FileDownloadReply *) msg;

    uint32_t byteCount = reply.byteCount;
    const uint8_t* data = reply.fileData;

    // tell the server we got the data
    Cli2File_FileDownloadChunkAck fileAck;
    fileAck.messageId = hsToLE32(kCli2File_FileDownloadChunkAck);
    fileAck.transId = hsToLE32(reply.transId);
    fileAck.messageBytes = hsToLE32(sizeof(fileAck));
    fileAck.readerId = hsToLE32(reply.readerId);

    m_conn->Send(&fileAck, sizeof(fileAck));

    if (IS_NET_ERROR(reply.result)) {
        // we have a problem... indicate we are done and abort
        m_result    = reply.result;
        m_state     = kTransStateComplete;
        return true;
    }

    // we have data to write, so queue it for write in the main thread (we're
    // currently in a net recv thread)
    if (byteCount > 0) {
        RcvdFileDownloadChunkTrans * writeTrans = new RcvdFileDownloadChunkTrans;
        writeTrans->writer  = m_writer;
        writeTrans->bytes   = byteCount;
        writeTrans->data    = (uint8_t *)malloc(byteCount);
        memcpy(writeTrans->data, data, byteCount);
        NetTransSend(writeTrans);
    }
    m_totalBytesReceived += byteCount;

    if (m_totalBytesReceived >= reply.totalFileSize) {
        // all bytes received, mark as complete
        m_result    = reply.result;
        m_state     = kTransStateComplete;
    }
    return true;
}

/*****************************************************************************
*
*   RcvdFileDownloadChunkTrans
*
***/

//============================================================================
RcvdFileDownloadChunkTrans::~RcvdFileDownloadChunkTrans () {
    free(data);
}

//============================================================================
void RcvdFileDownloadChunkTrans::Post () {
    writer->Write(bytes, data);
    m_result = kNetSuccess;
    m_state  = kTransStateComplete;
}


} using namespace File;


/*****************************************************************************
*
*   NetFileTrans
*
***/

//============================================================================
NetFileTrans::NetFileTrans (ETransType transType)
:   NetTrans(kNetProtocolCli2File, transType)
,   m_conn()
{
}

//============================================================================
NetFileTrans::~NetFileTrans () {
    ReleaseConn();
}

//============================================================================
bool NetFileTrans::AcquireConn () {
    if (!m_conn)
        m_conn = GetConnIncRef("AcquireConn");
    return m_conn != nullptr;
}

//============================================================================
void NetFileTrans::ReleaseConn () {
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
void FileInitialize () {
    s_running = true;
}

//============================================================================
void FileDestroy (bool wait) {
    s_running = false;

    NetTransCancelByProtocol(
        kNetProtocolCli2File,
        kNetErrRemoteShutdown
    );    
    NetMsgProtocolDestroy(
        kNetProtocolCli2File,
        false
    );

    {
        hsLockGuard(s_critsect);
        if (CliFileConn* conn = s_conn) {
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
bool FileQueryConnected () {
    hsLockGuard(s_critsect);
    return s_active != nullptr;
}

//============================================================================
unsigned FileGetConnId () {
    hsLockGuard(s_critsect);
    return (s_active) ? s_active->seq : 0;
}

} using namespace Ngl;

/*****************************************************************************
*
*   Exported functions
*
***/

//============================================================================
void NetCliFileStartConnect (
    const ST::string  fileAddrList[],
    uint32_t        fileAddrCount,
    bool            isPatcher /* = false */
) {
    // TEMP: Only connect to one file server until we fill out this module
    // to choose the "best" file connection.
    fileAddrCount = std::min(fileAddrCount, 1u);
    s_connectBuildId = isPatcher ? kFileSrvBuildId : plProduct::BuildId();
    s_serverType = kSrvTypeNone;

    for (unsigned i = 0; i < fileAddrCount; ++i) {
        // Do we need to lookup the address?
        ST::string name = fileAddrList[i];
        const char* pos;
        for (pos = name.begin(); pos != name.end(); ++pos) {
            if (!(isdigit(*pos) || *pos == '.' || *pos == ':')) {
                AsyncAddressLookupName(name, GetClientPort(), [name](auto addrs) {
                    if (addrs.empty()) {
                        ReportNetError(kNetProtocolCli2File, kNetErrNameLookupFailed);
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
bool NetCliFileQueryConnected () {
    return FileQueryConnected();
}

//============================================================================
void NetCliFileDisconnect () {
    hsLockGuard(s_critsect);
    if (CliFileConn* conn = s_conn) {
        s_conn = nullptr;
        AbandonConn(conn);
    }
    s_active = nullptr;
}

//============================================================================
void NetCliFileBuildIdRequest (
    FNetCliFileBuildIdRequestCallback   callback,
    void *                              param
) {
    BuildIdRequestTrans * trans = new BuildIdRequestTrans(
        callback,
        param
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliFileRegisterBuildIdUpdate (FNetCliFileBuildIdUpdateCallback callback) {
    s_buildIdCallback = callback;
}

//============================================================================
void NetCliFileManifestRequest (
    FNetCliFileManifestRequestCallback  callback,
    void *                              param,
    const char16_t                      group[],
    unsigned                            buildId /* = 0 */
) {
    ManifestRequestTrans * trans = new ManifestRequestTrans(
        callback,
        param,
        group,
        buildId
    );
    NetTransSend(trans);
}

//============================================================================
void NetCliFileDownloadRequest (
    const plFileName &                  filename,
    hsStream *                          writer,
    FNetCliFileDownloadRequestCallback  callback,
    void *                              param,
    unsigned                            buildId /* = 0 */
) {
    DownloadRequestTrans * trans = new DownloadRequestTrans(
        callback,
        param,
        filename,
        writer,
        buildId
    );
    NetTransSend(trans);
}
