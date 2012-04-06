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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnSimpleNet/pnSimpleNet.cpp
*   
***/

#include "pnSimpleNet.h"
#include "hsThread.h"

#include <list>
#include <map>

/*****************************************************************************
*
*   Local types
*
***/

struct SimpleNetConn : AtomicRef {
    AsyncSocket                 sock;
    AsyncCancelId               cancelId;
    uint32_t                    channelId;
    bool                        abandoned;
    struct ConnectParam *       connectParam;

    SimpleNet_MsgHeader *       oversizeMsg;
    ARRAY(uint8_t)              oversizeBuffer;
};

struct SimpleNetChannel : AtomicRef {
    FSimpleNetOnMsg             onMsg;
    FSimpleNetOnError           onError;
    uint32_t                    channelId;
    std::list<SimpleNetConn*>   conns;

    SimpleNetChannel (uint32_t channel) : channelId(channel) { }
    ~SimpleNetChannel () {
        ASSERT(!conns.size());
    }
};

struct ConnectParam {
    SimpleNetChannel *      channel;
    FSimpleNetOnConnect     callback;
    void *                  param;

    ~ConnectParam () {
        if (channel)
            channel->DecRef();
    }
};


/*****************************************************************************
*
*   Local data
*
***/

static bool                         s_running;
static hsMutex                      s_critsect;
static FSimpleNetQueryAccept        s_queryAccept;
static void *                       s_queryAcceptParam;
static std::map<uint32_t, SimpleNetChannel*>    s_channels;


/*****************************************************************************
*
*   Local functions
*
***/

//============================================================================
static void NotifyConnSocketConnect (SimpleNetConn * conn) {

    conn->TransferRef("Connecting", "Connected");

    conn->connectParam->callback(
        conn->connectParam->param,
        conn,
        kNetSuccess
    );

    delete conn->connectParam;
    conn->connectParam = nil;
}

//============================================================================
static void NotifyConnSocketConnectFailed (SimpleNetConn * conn) {

    s_critsect.Lock();
    {
        std::map<uint32_t, SimpleNetChannel*>::iterator it;
        if ((it = s_channels.find(conn->channelId)) != s_channels.end()) {
            it->second->conns.remove(conn);
        }
    }
    s_critsect.Unlock();

    conn->connectParam->callback(
        conn->connectParam->param,
        nil,
        kNetErrConnectFailed
    );

    delete conn->connectParam;
    conn->connectParam = nil;

    conn->DecRef("Connecting");
    conn->DecRef("Lifetime");
}

//============================================================================
static void NotifyConnSocketDisconnect (SimpleNetConn * conn) {

    bool abandoned;
    SimpleNetChannel* channel = nil;
    s_critsect.Lock();
    {
        abandoned = conn->abandoned;
        std::map<uint32_t, SimpleNetChannel*>::iterator it;
        if ((it = s_channels.find(conn->channelId)) != s_channels.end()) {
            channel = it->second;
            channel->IncRef();
            channel->conns.remove(conn);
        }
    }
    s_critsect.Unlock();

    if (channel && !abandoned) {
        channel->onError(conn, kNetErrDisconnected);
        channel->DecRef();
    }

    conn->DecRef("Connected");
}

//============================================================================
static bool NotifyConnSocketRead (SimpleNetConn * conn, AsyncNotifySocketRead * read) {

    SimpleNetChannel* channel = nil;
    s_critsect.Lock();
    {
        std::map<uint32_t, SimpleNetChannel*>::iterator it;
        if ((it = s_channels.find(conn->channelId)) != s_channels.end()) {
            channel = it->second;
            channel->IncRef();
        }
    }
    s_critsect.Unlock();

    if (!channel)
        return false;

    bool result = true;

    const uint8_t * curr = read->buffer;
    const uint8_t * term = curr + read->bytes;

    while (curr < term) {
        // Reading oversize msg?
        if (conn->oversizeBuffer.Count()) {
            unsigned spaceLeft = conn->oversizeMsg->messageBytes - conn->oversizeBuffer.Count();
            unsigned copyBytes = min(spaceLeft, term - curr);
            conn->oversizeBuffer.Add(curr, copyBytes);

            curr += copyBytes;

            // Wait until we have received the entire message
            if (copyBytes != spaceLeft)
                break;

            // Dispatch oversize msg
            if (!channel->onMsg(conn, conn->oversizeMsg)) {
                result = false;
                break;
            }

            conn->oversizeBuffer.SetCount(0);
            continue;
        }

        // Wait until we receive the entire message header
        if (term - curr < sizeof(SimpleNet_MsgHeader))
            break;

        SimpleNet_MsgHeader * msg = (SimpleNet_MsgHeader *) read->buffer;

        // Sanity check message size
        if (msg->messageBytes < sizeof(*msg)) {
            result = false;
            break;
        }

        // Handle oversized messages
        if (msg->messageBytes > kAsyncSocketBufferSize) {

            conn->oversizeBuffer.SetCount(msg->messageBytes);
            conn->oversizeMsg = (SimpleNet_MsgHeader *) conn->oversizeBuffer.Ptr();
            *conn->oversizeMsg = *msg;

            curr += sizeof(*msg);
            continue;
        }

        // Wait until we have received the entire message
        const uint8_t * msgTerm = (const uint8_t *) curr + msg->messageBytes;
        if (msgTerm > term)
            break;
        curr = msgTerm;

        // Dispatch msg
        if (!channel->onMsg(conn, msg)) {
            result = false;
            break;
        }
    }

    // Return count of bytes we processed
    read->bytesProcessed = curr - read->buffer;

    channel->DecRef();
    return result;
}

//============================================================================
static bool AsyncNotifySocketProc (
    AsyncSocket         sock,
    EAsyncNotifySocket  code,
    AsyncNotifySocket * notify,
    void **             userState
) {
    bool result = true;
    SimpleNetConn * conn;

    switch (code) {
        case kNotifySocketListenSuccess: {

            AsyncNotifySocketListen * listen = (AsyncNotifySocketListen *) notify;

            const SimpleNet_ConnData & connect = *(const SimpleNet_ConnData *) listen->buffer;
            listen->bytesProcessed += sizeof(connect);

            SimpleNetChannel* channel = nil;
            s_critsect.Lock();
            {
                std::map<uint32_t, SimpleNetChannel*>::iterator it;
                if ((it = s_channels.find(connect.channelId)) != s_channels.end()) {
                    channel = it->second;
                    channel->IncRef();
                }
            }
            s_critsect.Unlock();

            if (!channel)
                break;

            conn = NEWZERO(SimpleNetConn);
            conn->channelId = channel->channelId;
            conn->IncRef("Lifetime");
            conn->IncRef("Connected");
            conn->sock = sock;
            *userState = conn;

            bool accepted = s_queryAccept(
                s_queryAcceptParam,
                channel->channelId,
                conn,
                listen->remoteAddr
            );

            if (!accepted) {
                SimpleNetDisconnect(conn);
            }
            else {
                s_critsect.Lock();
                {
                    channel->conns.push_back(conn);
                }
                s_critsect.Unlock();
            }

            channel->DecRef();
        }
        break;

        case kNotifySocketConnectSuccess: {
            conn = (SimpleNetConn *) notify->param;
            *userState = conn;
            bool abandoned;

            s_critsect.Lock();
            {
                conn->sock      = sock;
                conn->cancelId  = 0;
                abandoned       = conn->abandoned;
            }
            s_critsect.Unlock();

            if (abandoned)
                AsyncSocketDisconnect(sock, true);
            else
                NotifyConnSocketConnect(conn);
        }
        break;

        case kNotifySocketConnectFailed:
            conn = (SimpleNetConn *) notify->param;
            NotifyConnSocketConnectFailed(conn);
            break;

        case kNotifySocketDisconnect:
            conn = (SimpleNetConn *) *userState;
            NotifyConnSocketDisconnect(conn);
            break;

        case kNotifySocketRead:
            conn = (SimpleNetConn *) *userState;
            result = NotifyConnSocketRead(conn, (AsyncNotifySocketRead *) notify);
            break;

        default:
            break;
    }

    return result;
}

//============================================================================
static void Connect(const plNetAddress& addr, ConnectParam * cp) {

    SimpleNetConn * conn = NEWZERO(SimpleNetConn);
    conn->channelId = cp->channel->channelId;
    conn->connectParam = cp;
    conn->IncRef("Lifetime");
    conn->IncRef("Connecting");

    s_critsect.Lock();
    {
        cp->channel->conns.push_back(conn);

        SimpleNet_Connect connect;
        connect.hdr.connType    = kConnTypeSimpleNet;
        connect.hdr.hdrBytes    = sizeof(connect.hdr);
        connect.hdr.buildId     = BuildId();
        connect.hdr.buildType   = BUILD_TYPE_LIVE;
        connect.hdr.branchId    = BranchId();
        connect.hdr.productId   = ProductId();
        connect.data.channelId  = cp->channel->channelId;

        AsyncSocketConnect(
            &conn->cancelId,
            addr,
            AsyncNotifySocketProc,
            conn,
            &connect,
            sizeof(connect)
        );

        conn = nil;
        cp = nil;
    }
    s_critsect.Unlock();

    delete conn;
    delete cp;
}

//============================================================================
static void AsyncLookupCallback (
    void *              param,
    const char          name[],
    unsigned            addrCount,
    const plNetAddress  addrs[]
) {
    ConnectParam * cp = (ConnectParam *)param;

    if (!addrCount) {
        if (cp->callback)
            cp->callback(cp->param, nil, kNetErrNameLookupFailed);
        delete cp;
        return;
    }

    Connect(addrs[0], (ConnectParam *)param);
}


/*****************************************************************************
*
*   Exported functions
*
***/

//============================================================================
void SimpleNetInitialize () {

    s_running = true;

    AsyncSocketRegisterNotifyProc(
        kConnTypeSimpleNet,
        AsyncNotifySocketProc
    );
}

//============================================================================
void SimpleNetShutdown () {

    s_running = false;

    ASSERT(!s_channels.size());

    AsyncSocketUnregisterNotifyProc(
        kConnTypeSimpleNet,
        AsyncNotifySocketProc
    );
}

//============================================================================
void SimpleNetConnIncRef (SimpleNetConn * conn) {

    ASSERT(s_running);
    ASSERT(conn);

    conn->IncRef();
}

//============================================================================
void SimpleNetConnDecRef (SimpleNetConn * conn) {

    ASSERT(s_running);
    ASSERT(conn);

    conn->DecRef();
}

//============================================================================
bool SimpleNetStartListening (
    FSimpleNetQueryAccept   queryAccept,
    void *                  param
) {
    ASSERT(s_running);
    ASSERT(queryAccept);
    ASSERT(!s_queryAccept);

    s_queryAccept       = queryAccept;
    s_queryAcceptParam  = param;

    plNetAddress addr;
    addr.SetPort(kNetDefaultSimpleNetPort);
    addr.SetAnyAddr();
    return (0 != AsyncSocketStartListening(addr, nil));
}

//============================================================================
void SimpleNetStopListening () {

    ASSERT(s_running);

    plNetAddress addr;
    addr.SetPort(kNetDefaultSimpleNetPort);
    addr.SetAnyAddr();
    AsyncSocketStopListening(addr, nil);

    s_queryAccept       = nil;
    s_queryAcceptParam  = nil;
}

//============================================================================
void SimpleNetCreateChannel (
    unsigned            channelId,
    FSimpleNetOnMsg     onMsg,
    FSimpleNetOnError   onError
) {
    ASSERT(s_running);

    SimpleNetChannel * channel = NEWZERO(SimpleNetChannel)(channelId);
    channel->IncRef();

    s_critsect.Lock();
    {
        #ifdef HS_DEBUGGING
        {
            std::map<uint32_t, SimpleNetChannel*>::iterator it = s_channels.find(channelId);
            ASSERT(it == s_channels.end());
        }
        #endif

        channel->onMsg      = onMsg;
        channel->onError    = onError;
        s_channels[channelId] = channel;
        channel->IncRef();
    }
    s_critsect.Unlock();

    channel->DecRef();
}

//============================================================================
void SimpleNetDestroyChannel (unsigned channelId) {

    ASSERT(s_running);

    SimpleNetChannel * channel;
    s_critsect.Lock();
    {
        std::map<uint32_t, SimpleNetChannel*>::iterator it;
        if ((it = s_channels.find(channelId)) != s_channels.end()) {
            channel = it->second;

            while (channel->conns.size()) {
                SimpleNetConn* conn = channel->conns.front();
                SimpleNetDisconnect(conn);

                channel->conns.pop_front();
            }

            s_channels.erase(it);
        }
    }
    s_critsect.Unlock();

    if (channel)
        channel->DecRef();
}

//============================================================================
void SimpleNetStartConnecting (
    unsigned            channelId,
    const char          addr[],
    FSimpleNetOnConnect onConnect,
    void *              param
) {
    ASSERT(s_running);
    ASSERT(onConnect);

    ConnectParam * cp = new ConnectParam;
    cp->callback    = onConnect;
    cp->param       = param;
    cp->channel     = nil;

    s_critsect.Lock();
    {
        std::map<uint32_t, SimpleNetChannel*>::iterator it;
        if ((it = s_channels.find(channelId)) != s_channels.end()) {
            cp->channel = it->second;
            cp->channel->IncRef();
        }
    }
    s_critsect.Unlock();

    ASSERT(cp->channel);

    // Do we need to lookup the address?
    const char* name = addr;
    while (unsigned ch = *name) {
        ++name;
        if (!(isdigit(ch) || ch == L'.' || ch == L':')) {

            AsyncCancelId cancelId;
            AsyncAddressLookupName(
                &cancelId,
                AsyncLookupCallback,
                addr,
                kNetDefaultSimpleNetPort,
                cp
            );
            break;
        }
    }
    if (!name[0]) {
        plNetAddress netAddr(addr, kNetDefaultSimpleNetPort);
        Connect(netAddr, cp);
    }
}

//============================================================================
void SimpleNetDisconnect (
    SimpleNetConn * conn
) {
    ASSERT(s_running);
    ASSERT(conn);

    s_critsect.Lock();
    {
        conn->abandoned = true;
        if (conn->sock) {
            AsyncSocketDisconnect(conn->sock, true);
            conn->sock = nil;
        }
        else if (conn->cancelId) {
            AsyncSocketConnectCancel(AsyncNotifySocketProc, conn->cancelId);
            conn->cancelId = nil;
        }
    }
    s_critsect.Unlock();

    conn->DecRef("Lifetime");
}

//============================================================================
void SimpleNetSend (
    SimpleNetConn *         conn,
    SimpleNet_MsgHeader *   msg
) {
    ASSERT(s_running);
    ASSERT(msg);
    ASSERT(msg->messageBytes != (uint32_t)-1);
    ASSERT(conn);

    s_critsect.Lock();
    {
        if (conn->sock)
            AsyncSocketSend(conn->sock, msg, msg->messageBytes);
    }
    s_critsect.Unlock();
}
