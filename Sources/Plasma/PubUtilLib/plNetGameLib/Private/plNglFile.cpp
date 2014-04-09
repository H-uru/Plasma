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
#pragma hdrstop

// Define this if the file servers are running behind load-balancing hardware.
// It changes the logic by which the decision to attempt a reconnect is made.
#define LOAD_BALANCER_HARDWARE


namespace Ngl { namespace File {
/*****************************************************************************
*
*   Private
*
***/

struct CliFileConn : hsAtomicRefCnt {
    LINK(CliFileConn)   link;
    hsReaderWriterLock  sockLock; // to protect the socket pointer so we don't nuke it while using it
    AsyncSocket         sock;
    char                name[MAX_PATH];
    plNetAddress        addr;
    unsigned            seq;
    ARRAY(uint8_t)      recvBuffer;
    AsyncCancelId       cancelId;
    bool                abandoned;
    unsigned            buildId;
    unsigned            serverType;

    CCritSect           timerCritsect; // critsect for both timers

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

    // This function should be called during object construction
    // to initiate connection attempts to the remote host whenever
    // the socket is disconnected.
    void AutoReconnect ();
    bool AutoReconnectEnabled () {return (reconnectTimer != nil);}
    void StopAutoReconnect (); // call before destruction
    void StartAutoReconnect ();
    void TimerReconnect ();

    // ping
    void AutoPing ();
    void StopAutoPing ();
    void TimerPing ();
    
    void Send (const void * data, unsigned bytes);

    void Destroy(); // cleans up the socket and buffer

    void Dispatch (const Cli2File_MsgHeader * msg);
    bool Recv_PingReply (const File2Cli_PingReply * msg);
    bool Recv_BuildIdReply (const File2Cli_BuildIdReply * msg);
    bool Recv_BuildIdUpdate (const File2Cli_BuildIdUpdate * msg);
    bool Recv_ManifestReply (const File2Cli_ManifestReply * msg);
    bool Recv_FileDownloadReply (const File2Cli_FileDownloadReply * msg);
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

    bool Send ();
    void Post ();
    bool Recv (
        const uint8_t  msg[],
        unsigned    bytes
    );
};

//============================================================================
// ManifestRequestTrans
//============================================================================
struct ManifestRequestTrans : NetFileTrans {
    FNetCliFileManifestRequestCallback  m_callback;
    void *                              m_param;
    wchar_t                             m_group[MAX_PATH];
    unsigned                            m_buildId;

    ARRAY(NetCliFileManifestEntry)      m_manifest;
    unsigned                            m_numEntriesReceived;

    ManifestRequestTrans (
        FNetCliFileManifestRequestCallback  callback,
        void *                              param,
        const wchar_t                       group[],
        unsigned                            buildId
    );

    bool Send ();
    void Post ();
    bool Recv (
        const uint8_t  msg[],
        unsigned    bytes
    );
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

    bool Send ();
    void Post ();
    bool Recv (
        const uint8_t  msg[],
        unsigned    bytes
    );
};

//============================================================================
// RcvdFileDownloadChunkTrans
//============================================================================
struct RcvdFileDownloadChunkTrans : NetNotifyTrans {

    unsigned    bytes;
    uint8_t *      data;
    hsStream *  writer;

    RcvdFileDownloadChunkTrans () : NetNotifyTrans (kFileRcvdFileDownloadChunkTrans) {}
    ~RcvdFileDownloadChunkTrans ();
    void Post ();
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
static CCritSect                    s_critsect;
static LISTDECL(CliFileConn, link)  s_conns;
static CliFileConn *                s_active;
static std::atomic<long>            s_perf[kNumPerf];
static unsigned                     s_connectBuildId;
static unsigned                     s_serverType;

static FNetCliFileBuildIdUpdateCallback s_buildIdCallback = nil;

const unsigned kMinValidConnectionMs                = 25 * 1000;



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
static CliFileConn * GetConnIncRef_CS (const char tag[]) {
    if (CliFileConn * conn = s_active) {
        conn->Ref(tag);
        return conn;
    }
    return nil;
}

//============================================================================
static CliFileConn * GetConnIncRef (const char tag[]) {
    CliFileConn * conn;
    s_critsect.Enter();
    {
        conn = GetConnIncRef_CS(tag);
    }
    s_critsect.Leave();
    return conn;
}

//============================================================================
static void UnlinkAndAbandonConn_CS (CliFileConn * conn) {
    s_conns.Unlink(conn);
    conn->abandoned = true;

    if (conn->AutoReconnectEnabled())
        conn->StopAutoReconnect();

    bool needsDecref = true;
    if (conn->cancelId) {
        AsyncSocketConnectCancel(nil, conn->cancelId);
        conn->cancelId  = 0;
        needsDecref = false;
    }
    else {
        conn->sockLock.LockForReading();
        if (conn->sock) {
            AsyncSocketDisconnect(conn->sock, true);
            needsDecref = false;
        }
        conn->sockLock.UnlockForReading();
    }
    if (needsDecref) {
        conn->UnRef("Lifetime");
    }
}

//============================================================================
static void NotifyConnSocketConnect (CliFileConn * conn) {

    conn->TransferRef("Connecting", "Connected");
    conn->connectStartMs = TimeGetMs();
    conn->numFailedConnects = 0;

    // Make this the active server
    s_critsect.Enter();
    {
        if (!conn->abandoned) {
            conn->AutoPing();
            s_active = conn;
        }
        else
        {
            conn->sockLock.LockForReading();
            AsyncSocketDisconnect(conn->sock, true);
            conn->sockLock.UnlockForReading();
        }
    }
    s_critsect.Leave();
}

//============================================================================
static void NotifyConnSocketConnectFailed (CliFileConn * conn) {
    s_critsect.Enter();
    {
        conn->cancelId = 0;
        s_conns.Unlink(conn);

        if (conn == s_active)
            s_active = nil;
    }
    s_critsect.Leave();
    
    // Cancel all transactions in progress on this connection.
    NetTransCancelByConnId(conn->seq, kNetErrTimeout);
    
#ifndef SERVER
    // Client apps fail if unable to connect for a time
    if (++conn->numFailedConnects >= kMaxFailedConnects) {
        ReportNetError(kNetProtocolCli2File, kNetErrConnectFailed);
    }
    else
#endif // ndef SERVER
    {
        // start reconnect, if we are doing that
        if (s_running && conn->AutoReconnectEnabled())
            conn->StartAutoReconnect();
        else
            conn->UnRef("Lifetime"); // if we are not reconnecting, this socket is done, so remove the lifetime ref
    }
    conn->UnRef("Connecting");
}

//============================================================================
static void NotifyConnSocketDisconnect (CliFileConn * conn) {
    conn->StopAutoPing();
    s_critsect.Enter();
    {
        conn->cancelId = 0;
        s_conns.Unlink(conn);
            
        if (conn == s_active)
            s_active = nil;
    }
    s_critsect.Leave();

    // Cancel all transactions in progress on this connection.
    NetTransCancelByConnId(conn->seq, kNetErrTimeout);


    bool notify = false;

#ifdef SERVER
    {
        if (TimeGetMs() - conn->connectStartMs > kMinValidConnectionMs)
            conn->reconnectStartMs = 0;
        else
            conn->reconnectStartMs = GetNonZeroTimeMs() + kMaxReconnectIntervalMs;
    }
#else
    {
    #ifndef LOAD_BALANCER_HARDWARE
        // If the connection to the remote server was open for longer than
        // kMinValidConnectionMs then assume that the connection was to
        // a valid server and try to perform reconnection immediately. If
        // less time elapsed then the connection was likely to a server
        // with an open port but with no notification procedure registered
        // for this type of communication channel.
        if (TimeGetMs() - conn->connectStartMs > kMinValidConnectionMs) {
            conn->reconnectStartMs = 0;
        }
        else {
            if (++conn->numImmediateDisconnects < kMaxImmediateDisconnects)
                conn->reconnectStartMs = GetNonZeroTimeMs() + kMaxReconnectIntervalMs;
            else
                notify = true;
        }
    #else
        // File server is running behind a load-balancer, so the next connection may
        // send us to a new server, therefore attempt a reconnection to the same
        // address even if the disconnect was immediate.  This is safe because the
        // file server is stateless with respect to clients.
        if (TimeGetMs() - conn->connectStartMs <= kMinValidConnectionMs) {
            if (++conn->numImmediateDisconnects < kMaxImmediateDisconnects)
                conn->reconnectStartMs = GetNonZeroTimeMs() + kMaxReconnectIntervalMs;
            else
                notify = true;
        }
        else {
            // disconnect was not immediate. attempt a reconnect unless we're shutting down
            conn->numImmediateDisconnects = 0;
            conn->reconnectStartMs = 0;
        }
    #endif  // LOAD_BALANCER
    }
#endif // ndef SERVER

    if (notify) {
        ReportNetError(kNetProtocolCli2File, kNetErrDisconnected);
    }
    else {  
        // clean up the socket and start reconnect, if we are doing that
        conn->Destroy();
        if (conn->AutoReconnectEnabled())
            conn->StartAutoReconnect();
        else
            conn->UnRef("Lifetime"); // if we are not reconnecting, this socket is done, so remove the lifetime ref
    }

    conn->UnRef("Connected");
}

//============================================================================
static bool NotifyConnSocketRead (CliFileConn * conn, AsyncNotifySocketRead * read) {
    conn->lastHeardTimeMs = GetNonZeroTimeMs();
    conn->recvBuffer.Add(read->buffer, read->bytes);
    read->bytesProcessed += read->bytes;

    for (;;) {
        if (conn->recvBuffer.Count() < sizeof(uint32_t))
            return true;

        uint32_t msgSize = *(uint32_t *)conn->recvBuffer.Ptr();
        if (conn->recvBuffer.Count() < msgSize)
            return true;

        const Cli2File_MsgHeader * msg = (const Cli2File_MsgHeader *) conn->recvBuffer.Ptr();
        conn->Dispatch(msg);

        conn->recvBuffer.Move(0, msgSize, conn->recvBuffer.Count() - msgSize);
        conn->recvBuffer.ShrinkBy(msgSize);
    }
}

//============================================================================
static bool SocketNotifyCallback (
    AsyncSocket         sock,
    EAsyncNotifySocket  code,
    AsyncNotifySocket * notify,
        void **             userState
) {
    bool result = true;
    CliFileConn * conn;

    switch (code) {
        case kNotifySocketConnectSuccess:
            conn = (CliFileConn *) notify->param;
            *userState = conn;
            s_critsect.Enter();
            {
                conn->sockLock.LockForWriting();
                conn->sock      = sock;
                conn->sockLock.UnlockForWriting();
                conn->cancelId  = 0;
            }
            s_critsect.Leave();
            NotifyConnSocketConnect(conn);
        break;

        case kNotifySocketConnectFailed:
            conn = (CliFileConn *) notify->param;
            NotifyConnSocketConnectFailed(conn);
        break;

        case kNotifySocketDisconnect:
            conn = (CliFileConn *) *userState;
            NotifyConnSocketDisconnect(conn);
        break;

        case kNotifySocketRead:
            conn = (CliFileConn *) *userState;
            result = NotifyConnSocketRead(conn, (AsyncNotifySocketRead *) notify);
        break;
    }

    return result;
}

//============================================================================
static void Connect (CliFileConn * conn) {
    ASSERT(s_running);

    conn->pingSendTimeMs = 0;

    s_critsect.Enter();
    {
        while (CliFileConn * oldConn = s_conns.Head()) {
            if (oldConn != conn)
                UnlinkAndAbandonConn_CS(oldConn);
            else
                s_conns.Unlink(oldConn);
        }
        s_conns.Link(conn);
    }
    s_critsect.Leave();

    Cli2File_Connect connect;
    connect.hdr.connType    = kConnTypeCliToFile;
    connect.hdr.hdrBytes    = sizeof(connect.hdr);
    connect.hdr.buildId     = kFileSrvBuildId;
    connect.hdr.buildType   = plProduct::BuildType();
    connect.hdr.branchId    = plProduct::BranchId();
    connect.hdr.productId   = plProduct::UUID();
    connect.data.buildId    = conn->buildId;
    connect.data.serverType = conn->serverType;
    connect.data.dataBytes  = sizeof(connect.data);

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
    const char          name[],
    const plNetAddress& addr
) {
    ASSERT(s_running);
    
    CliFileConn * conn = new CliFileConn;
    strncpy(conn->name, name, arrsize(conn->name));
    conn->addr          = addr;
    conn->buildId       = s_connectBuildId;
    conn->serverType    = s_serverType;
    conn->seq           = ConnNextSequence();
    conn->lastHeardTimeMs   = GetNonZeroTimeMs();   // used in connect timeout, and ping timeout

    conn->Ref("Lifetime");
    conn->AutoReconnect();
}

//============================================================================
static void AsyncLookupCallback (
    void *              param,
    const char          name[],
    unsigned            addrCount,
    const plNetAddress  addrs[]
) {
    if (!addrCount) {
        ReportNetError(kNetProtocolCli2File, kNetErrNameLookupFailed);
        return;
    }

    for (unsigned i = 0; i < addrCount; ++i) {
        Connect(name, addrs[i]);
    }
}

/*****************************************************************************
*
*   CliFileConn
*
***/

//============================================================================
CliFileConn::CliFileConn ()
    : hsAtomicRefCnt(0), sock(nil), seq(0), cancelId(nil), abandoned(false)
    , buildId(0), serverType(0)
    , reconnectTimer(nil), reconnectStartMs(0), connectStartMs(0)
    , numImmediateDisconnects(0), numFailedConnects(0)
    , pingTimer(nil), pingSendTimeMs(0), lastHeardTimeMs(0)
{
    memset(name, 0, sizeof(name));
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
    ASSERT(!sock);
    ASSERT(!cancelId);
    
    if (!s_running) {
        s_critsect.Enter();
        UnlinkAndAbandonConn_CS(this);
        s_critsect.Leave();
    }
    else {
        Ref("Connecting");

        // Remember the time we started the reconnect attempt, guarding against
        // TimeGetMs() returning zero (unlikely), as a value of zero indicates
        // a first-time connect condition to StartAutoReconnect()
        reconnectStartMs = GetNonZeroTimeMs();

        Connect(this);
    }
}

//===========================================================================
static unsigned CliFileConnTimerReconnectProc (void * param) {
    ((CliFileConn *) param)->TimerReconnect();
    return kAsyncTimeInfinite;
}

//===========================================================================
// This function is called when after a disconnect to start a new connection
void CliFileConn::StartAutoReconnect () {
    timerCritsect.Enter();
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
    timerCritsect.Leave();
}

//===========================================================================
// This function should be called during object construction
// to initiate connection attempts to the remote host whenever
// the socket is disconnected.
void CliFileConn::AutoReconnect () {
    timerCritsect.Enter();
    {
        ASSERT(!reconnectTimer);
        Ref("ReconnectTimer");
        AsyncTimerCreate(
            &reconnectTimer,
            CliFileConnTimerReconnectProc,
            0,  // immediate callback
            this
        );
    }
    timerCritsect.Leave();
}

//===========================================================================
static unsigned CliFileConnTimerDestroyed (void * param) {
    CliFileConn * sock = (CliFileConn *) param;
    sock->UnRef("TimerDestroyed");
    return kAsyncTimeInfinite;
}

//============================================================================
void CliFileConn::StopAutoReconnect () {
    timerCritsect.Enter();
    {
        if (AsyncTimer * timer = reconnectTimer) {
            reconnectTimer = nil;
            AsyncTimerDeleteCallback(timer, CliFileConnTimerDestroyed);
        }
    }
    timerCritsect.Leave();
}

//===========================================================================
static unsigned CliFileConnPingTimerProc (void * param) {
    ((CliFileConn *) param)->TimerPing();
    return kPingIntervalMs;
}

//============================================================================
void CliFileConn::AutoPing () {
    ASSERT(!pingTimer);
    Ref("PingTimer");
    timerCritsect.Enter();
    {
        sockLock.LockForReading();
        unsigned timerPeriod = sock ? 0 : kAsyncTimeInfinite;
        sockLock.UnlockForReading();

        AsyncTimerCreate(
            &pingTimer,
            CliFileConnPingTimerProc,
            timerPeriod,
            this
        );
    }
    timerCritsect.Leave();
}

//============================================================================
void CliFileConn::StopAutoPing () {
    timerCritsect.Enter();
    {
        if (AsyncTimer * timer = pingTimer) {
            pingTimer = nil;
            AsyncTimerDeleteCallback(timer, CliFileConnTimerDestroyed);
        }
    }
    timerCritsect.Leave();
}

//============================================================================
void CliFileConn::TimerPing () {
    sockLock.LockForReading();
    for (;;) {
        if (!sock) // make sure it exists
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
            msg.messageId = kCli2File_PingRequest;
            msg.messageBytes = sizeof(msg);
            msg.pingTimeMs = pingSendTimeMs;

            // read locks are reentrant, so calling Send is ok here within the read lock
            Send(&msg, msg.messageBytes);
        }
        break;
    }
    sockLock.UnlockForReading();
}

//============================================================================
void CliFileConn::Destroy () {
    AsyncSocket oldSock = nil;

    sockLock.LockForWriting();
    {
        SWAP(oldSock, sock);
    }
    sockLock.UnlockForWriting();

    if (oldSock)
        AsyncSocketDelete(oldSock);
    recvBuffer.Clear();
}

//============================================================================
void CliFileConn::Send (const void * data, unsigned bytes) {
    sockLock.LockForReading();
    if (sock) {
        AsyncSocketSend(sock, data, bytes);
    }
    sockLock.UnlockForReading();
}

//============================================================================
void CliFileConn::Dispatch (const Cli2File_MsgHeader * msg) {

#define DISPATCH(a) case kFile2Cli_##a: Recv_##a((const File2Cli_##a *) msg); break
    switch (msg->messageId) {
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
    const File2Cli_PingReply * msg
) {
    return true;
}

//============================================================================
bool CliFileConn::Recv_BuildIdReply (
    const File2Cli_BuildIdReply * msg
) {
    NetTransRecv(msg->transId, (const uint8_t *)msg, msg->messageBytes);

    return true;
}

//============================================================================
bool CliFileConn::Recv_BuildIdUpdate (
    const File2Cli_BuildIdUpdate * msg
) {
    if (s_buildIdCallback)
        s_buildIdCallback(msg->buildId);
    return true;
}

//============================================================================
bool CliFileConn::Recv_ManifestReply (
    const File2Cli_ManifestReply * msg
) {
    NetTransRecv(msg->transId, (const uint8_t *)msg, msg->messageBytes);

    return true;
}

//============================================================================
bool CliFileConn::Recv_FileDownloadReply (
    const File2Cli_FileDownloadReply * msg
) {
    NetTransRecv(msg->transId, (const uint8_t *)msg, msg->messageBytes);

    return true;
}


/*****************************************************************************
*
*   BuildIdRequestTrans
*
***/

//============================================================================
BuildIdRequestTrans::BuildIdRequestTrans (
    FNetCliFileBuildIdRequestCallback   callback,
    void *                              param
) : NetFileTrans(kBuildIdRequestTrans)
,   m_callback(callback)
,   m_param(param)
{}

//============================================================================
bool BuildIdRequestTrans::Send () {
    if (!AcquireConn())
        return false;

    Cli2File_BuildIdRequest buildIdReq;
    buildIdReq.messageId = kCli2File_BuildIdRequest;
    buildIdReq.transId = m_transId;
    buildIdReq.messageBytes = sizeof(buildIdReq);

    m_conn->Send(&buildIdReq, buildIdReq.messageBytes); 
    
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
    const wchar_t                       group[],
    unsigned                            buildId
) : NetFileTrans(kManifestRequestTrans)
,   m_callback(callback)
,   m_param(param)
,   m_numEntriesReceived(0)
,   m_buildId(buildId)
{
    if (group)
        StrCopy(m_group, group, arrsize(m_group));
    else
        m_group[0] = L'\0';
}

//============================================================================
bool ManifestRequestTrans::Send () {
    if (!AcquireConn())
        return false;

    Cli2File_ManifestRequest manifestReq;
    StrCopy(manifestReq.group, m_group, arrsize(manifestReq.group));
    manifestReq.messageId = kCli2File_ManifestRequest;
    manifestReq.transId = m_transId;
    manifestReq.messageBytes = sizeof(manifestReq);
    manifestReq.buildId = m_buildId;

    m_conn->Send(&manifestReq, manifestReq.messageBytes);   

    return true;
}

//============================================================================
void ManifestRequestTrans::Post () {
    m_callback(m_result, m_param, m_group, m_manifest.Ptr(), m_manifest.Count());
}

//============================================================================
void ReadStringFromMsg(const wchar_t* curMsgPtr, wchar_t* destPtr, unsigned* length) {
    if (!(*length)) {
        size_t maxlen = wcsnlen(curMsgPtr, MAX_PATH - 1);   // Hacky sack
        (*length) = maxlen;
        destPtr[maxlen] = 0;    // Don't do this on fixed length, because there's no room for it
    }
    memcpy(destPtr, curMsgPtr, *length * sizeof(wchar_t));
}

//============================================================================
void ReadUnsignedFromMsg(const wchar_t* curMsgPtr, unsigned* val) {
    (*val) = ((*curMsgPtr) << 16) + (*(curMsgPtr + 1));
}

//============================================================================
bool ManifestRequestTrans::Recv (
    const uint8_t  msg[],
    unsigned    bytes
) {
    m_timeoutAtMs = TimeGetMs() + NetTransGetTimeoutMs(); // Reset the timeout counter

    const File2Cli_ManifestReply & reply = *(const File2Cli_ManifestReply *) msg;

    uint32_t numFiles = reply.numFiles;
    uint32_t wchar_tCount = reply.wchar_tCount;
    const wchar_t* curChar = reply.manifestData; // the pointer is not yet dereferenced here!

    // tell the server we got the data
    Cli2File_ManifestEntryAck manifestAck;
    manifestAck.messageId = kCli2File_ManifestEntryAck;
    manifestAck.transId = reply.transId;
    manifestAck.messageBytes = sizeof(manifestAck);
    manifestAck.readerId = reply.readerId;

    m_conn->Send(&manifestAck, manifestAck.messageBytes);   

    // if wchar_tCount is 2 or less, the data only contains the terminator "\0\0" and we
    // don't need to convert anything (and we are done)
    if ((IS_NET_ERROR(reply.result)) || (wchar_tCount <= 2)) {
        // we have a problem... or we have nothing to so, so we're done
        m_result    = reply.result;
        m_state     = kTransStateComplete;
        return true;
    }

    if (numFiles > m_manifest.Count())
        m_manifest.SetCount(numFiles); // reserve the space ahead of time

    // manifestData format: "clientFile\0downloadFile\0md5\0filesize\0zipsize\0flags\0...\0\0"
    bool done = false;
    while (!done) {
        if (wchar_tCount == 0)
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
        wchar_tCount -= filenameLen; // keep track of the amount remaining
        if ((*curChar != L'\0') || (wchar_tCount <= 0))
            return false; // something is screwy, abort and disconnect

        // point it at the downloadFile
        curChar++;
        wchar_tCount--;

        // --------------------------------------------------------------------
        // read in the downloadFilename
        filenameLen = 0;
        ReadStringFromMsg(curChar, entry.downloadName, &filenameLen);
        curChar += filenameLen; // advance the pointer
        wchar_tCount -= filenameLen; // keep track of the amount remaining
        if ((*curChar != L'\0') || (wchar_tCount <= 0))
            return false; // something is screwy, abort and disconnect

        // point it at the md5
        curChar++;
        wchar_tCount--;

        // --------------------------------------------------------------------
        // read in the md5
        filenameLen = 32;
        ReadStringFromMsg(curChar, entry.md5, &filenameLen);
        curChar += filenameLen; // advance the pointer
        wchar_tCount -= filenameLen; // keep track of the amount remaining
        if ((*curChar != L'\0') || (wchar_tCount <= 0))
            return false; // something is screwy, abort and disconnect

        // point it at the md5 for compressed files
        curChar++; 
        wchar_tCount--;

        // --------------------------------------------------------------------
        // read in the md5 for compressed files
        filenameLen = 32;
        ReadStringFromMsg(curChar, entry.md5compressed, &filenameLen);
        curChar += filenameLen; // advance the pointer
        wchar_tCount -= filenameLen; // keep track of the amount remaining
        if ((*curChar != L'\0') || (wchar_tCount <= 0))
            return false; // something is screwy, abort and disconnect

        // point it at the first part of the filesize value (format: 0xHHHHLLLL)
        curChar++; 
        wchar_tCount--;

        // --------------------------------------------------------------------
        if (wchar_tCount < 2) // we have to have 2 chars for the size
            return false; // screwy data
        ReadUnsignedFromMsg(curChar, &entry.fileSize);
        curChar += 2;
        wchar_tCount -= 2;
        if ((*curChar != L'\0') || (wchar_tCount <= 0))
            return false; // screwy data

        // point it at the first part of the zipsize value (format: 0xHHHHLLLL)
        curChar++; 
        wchar_tCount--;

        // --------------------------------------------------------------------
        if (wchar_tCount < 2) // we have to have 2 chars for the size
            return false; // screwy data
        ReadUnsignedFromMsg(curChar, &entry.zipSize);
        curChar += 2;
        wchar_tCount -= 2;
        if ((*curChar != L'\0') || (wchar_tCount <= 0))
            return false; // screwy data

        // point it at the first part of the flags value (format: 0xHHHHLLLL)
        curChar++; 
        wchar_tCount--;

        // --------------------------------------------------------------------
        if (wchar_tCount < 2) // we have to have 2 chars for the size
            return false; // screwy data
        ReadUnsignedFromMsg(curChar, &entry.flags);
        curChar += 2;
        wchar_tCount -= 2;
        if ((*curChar != L'\0') || (wchar_tCount <= 0))
            return false; // screwy data

        // --------------------------------------------------------------------
        // point it at either the second part of the terminator, or the next filename
        curChar++;
        wchar_tCount--;

        // do sanity checking
        if (*curChar == L'\0') {
            // we hit the terminator
            if (wchar_tCount != 1)
                return false; // invalid data, we shouldn't have any more
            done = true; // we're done
        }
        else if (wchar_tCount < 14)
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
    StrCopy(filedownloadReq.filename, m_filename.AsString().ToWchar(), arrsize(filedownloadReq.filename));
    filedownloadReq.messageId = kCli2File_FileDownloadRequest;
    filedownloadReq.transId = m_transId;
    filedownloadReq.messageBytes = sizeof(filedownloadReq);
    filedownloadReq.buildId = m_buildId;

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
    m_timeoutAtMs = TimeGetMs() + NetTransGetTimeoutMs(); // Reset the timeout counter

    const File2Cli_FileDownloadReply & reply = *(const File2Cli_FileDownloadReply *) msg;

    uint32_t byteCount = reply.byteCount;
    const uint8_t* data = reply.fileData;

    // tell the server we got the data
    Cli2File_FileDownloadChunkAck fileAck;
    fileAck.messageId = kCli2File_FileDownloadChunkAck;
    fileAck.transId = reply.transId;
    fileAck.messageBytes = sizeof(fileAck);
    fileAck.readerId = reply.readerId;

    m_conn->Send(&fileAck, fileAck.messageBytes);

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
,   m_conn(nil)
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
    return m_conn != nil;
}

//============================================================================
void NetFileTrans::ReleaseConn () {
    if (m_conn) {
        m_conn->UnRef("AcquireConn");
        m_conn = nil;
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

    s_critsect.Enter();
    {
        while (CliFileConn * conn = s_conns.Head())
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
bool FileQueryConnected () {
    bool result;
    s_critsect.Enter();
    result = s_active != nil;
    s_critsect.Leave();
    return result;
}

//============================================================================
unsigned FileGetConnId () {
    unsigned connId;
    s_critsect.Enter();
    connId = (s_active) ? s_active->seq : 0;
    s_critsect.Leave();
    return connId;
}

} using namespace Ngl;

/*****************************************************************************
*
*   Exported functions
*
***/

//============================================================================
void NetCliFileStartConnect (
    const char*     fileAddrList[],
    uint32_t        fileAddrCount,
    bool            isPatcher /* = false */
) {
    // TEMP: Only connect to one file server until we fill out this module
    // to choose the "best" file connection.
    fileAddrCount = min(fileAddrCount, 1);
    s_connectBuildId = isPatcher ? kFileSrvBuildId : plProduct::BuildId();
    s_serverType = kSrvTypeNone;

    for (unsigned i = 0; i < fileAddrCount; ++i) {
        // Do we need to lookup the address?
        const char* name = fileAddrList[i];
        while (unsigned ch = *name) {
            ++name;
            if (!(isdigit(ch) || ch == L'.' || ch == L':')) {
                AsyncCancelId cancelId;
                AsyncAddressLookupName(
                    &cancelId,
                    AsyncLookupCallback,
                    fileAddrList[i],
                    kNetDefaultClientPort,
                    nil
                );
                break;
            }
        }
        if (!name[0]) {
            plNetAddress addr(fileAddrList[i], kNetDefaultClientPort);
            Connect(fileAddrList[i], addr);
        }
    }
}

//============================================================================
void NetCliFileStartConnectAsServer (
    const char*     fileAddrList[],
    uint32_t        fileAddrCount,
    unsigned        serverType,
    unsigned        serverBuildId
) {
    // TEMP: Only connect to one file server until we fill out this module
    // to choose the "best" file connection.
    fileAddrCount = min(fileAddrCount, 1);
    s_connectBuildId = serverBuildId;
    s_serverType = serverType;

    for (unsigned i = 0; i < fileAddrCount; ++i) {
        // Do we need to lookup the address?
        const char* name = fileAddrList[i];
        while (unsigned ch = *name) {
            ++name;
            if (!(isdigit(ch) || ch == L'.' || ch == L':')) {
                AsyncCancelId cancelId;
                AsyncAddressLookupName(
                    &cancelId,
                    AsyncLookupCallback,
                    fileAddrList[i],
                    kNetDefaultClientPort,
                    nil
                );
                break;
            }
        }
        if (!name[0]) {
            plNetAddress addr(fileAddrList[i], kNetDefaultServerPort);
            Connect(fileAddrList[i], addr);
        }
    }
}

//============================================================================
void NetCliFileDisconnect () {
    s_critsect.Enter();
    {
        while (CliFileConn * conn = s_conns.Head())
            UnlinkAndAbandonConn_CS(conn);
        s_active = nil;
    }
    s_critsect.Leave();
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
    const wchar_t                       group[],
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
