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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetCli/pnNcChannel.cpp
*   
***/

#include "Pch.h"
#include "hsThread.h"
#include <list>
#pragma hdrstop


namespace pnNetCli {

/*****************************************************************************
*
*   Private
*
***/

struct ChannelCrit {
    ~ChannelCrit ();
    ChannelCrit ();
    inline void Enter ()        { m_critsect.Lock();               }
    inline void Leave ()        { m_critsect.Unlock();             }
    inline void EnterSafe ()    { if (m_init) m_critsect.Lock();   }
    inline void LeaveSafe ()    { if (m_init) m_critsect.Unlock(); }

private:
    bool        m_init;
    hsMutex     m_critsect;
};

struct NetMsgChannel : AtomicRef {
    uint32_t                m_protocol;
    bool                    m_server;

    // Message definitions
    uint32_t                m_largestRecv;
    ARRAY(NetMsgInitSend)   m_sendMsgs;
    ARRAY(NetMsgInitRecv)   m_recvMsgs;

    // Diffie-Hellman constants
    uint32_t                m_dh_g;
    plBigNum                m_dh_xa;    // client: dh_x     server: dh_a
    plBigNum                m_dh_n;
};

static ChannelCrit                  s_channelCrit;
static std::list<NetMsgChannel*>*   s_channels;


/****************************************************************************
*
*   ChannelCrit
*
***/

//===========================================================================
ChannelCrit::ChannelCrit () {
    m_init = true;
}

//===========================================================================
ChannelCrit::~ChannelCrit () {
    EnterSafe();
    if (s_channels) {
        while (s_channels->size()) {
            NetMsgChannel* const channel = s_channels->front();
            s_channels->remove(channel);
            channel->DecRef("ChannelLink");
        }

        delete s_channels;
        s_channels = nil;
    }
    LeaveSafe();
}


/*****************************************************************************
*
*   Internal functions
*
***/

//===========================================================================
// Returns max size of message in bytes
static unsigned ValidateMsg (const NetMsg & msg) {
    ASSERT(msg.fields);
    ASSERT(msg.count);

    unsigned maxBytes = sizeof(uint16_t);    // for message id
    bool prevFieldWasVarCount = false;

    for (unsigned i = 0; i < msg.count; i++) {
        const NetMsgField & field = msg.fields[i];

        for (;;) {
            bool gotVarCount = false;
            bool gotVarField = false;
            if (field.type == kNetMsgFieldVarCount) {
                if (gotVarField || gotVarCount)
                    FATAL("Msg definition may only include one variable length field");
                gotVarCount = true;
                break;
            }
            if (field.type == kNetMsgFieldVarPtr || field.type == kNetMsgFieldRawVarPtr) {
                if (gotVarField || gotVarCount)
                    FATAL("Msg definition may only include one variable length field");
                if (!prevFieldWasVarCount)
                    FATAL("Variable length field must preceded by variable length count field");
                gotVarField = true;
                break;
            }
            if (gotVarField)
                FATAL("Variable length field must be the last field in message definition");
            break;
        }

        prevFieldWasVarCount = false;
        switch (field.type) {
            case kNetMsgFieldInteger:
                maxBytes += sizeof(uint64_t);
            break;

            case kNetMsgFieldReal:
                maxBytes += sizeof(double);
            break;

            case kNetMsgFieldVarPtr:
            case kNetMsgFieldRawVarPtr:
            break;

            case kNetMsgFieldVarCount:
                prevFieldWasVarCount = true;
                // fall-thru...
            case kNetMsgFieldString:
            case kNetMsgFieldPtr:
            case kNetMsgFieldRawPtr:
            case kNetMsgFieldData:
            case kNetMsgFieldRawData:
                maxBytes += msg.fields[i].count * msg.fields[i].size;
            break;

            DEFAULT_FATAL(field.type);
        }
    }

    return maxBytes;
}


//===========================================================================
template<class T>
static unsigned MaxMsgId (const T msgs[], unsigned count) {
    unsigned maxMsgId = 0;

    for (unsigned i = 0; i < count; i++) {
        ASSERT(msgs[i].msg.count);
        maxMsgId = max(msgs[i].msg.messageId, maxMsgId);
    }
    return maxMsgId;
}

//===========================================================================
static void AddSendMsgs_CS (
    NetMsgChannel *         channel,
    const NetMsgInitSend    src[],
    unsigned                count
) {
    channel->m_sendMsgs.GrowToFit(MaxMsgId(src, count), true);

    for (const NetMsgInitSend * term = src + count; src < term; ++src) {
        NetMsgInitSend * const dst = &channel->m_sendMsgs[src[0].msg.messageId];

        // check to ensure that the message id isn't already used
        ASSERT(!dst->msg.count);

        *dst = *src;
        ValidateMsg(dst->msg);
    }
}

//===========================================================================
static void AddRecvMsgs_CS (
    NetMsgChannel *         channel,
    const NetMsgInitRecv    src[],
    unsigned                count
) {
    channel->m_recvMsgs.GrowToFit(MaxMsgId(src, count), true);

    for (const NetMsgInitRecv * term = src + count; src < term; ++src) {
        ASSERT(src->recv);
        NetMsgInitRecv * const dst = &channel->m_recvMsgs[src[0].msg.messageId];

        // check to ensure that the message id isn't already used
        ASSERT(!dst->msg.count);

        // copy the message handler
        *dst = *src;

        const uint32_t bytes = ValidateMsg(dst->msg);
        channel->m_largestRecv = max(channel->m_largestRecv, bytes);
    }
}

//===========================================================================
static NetMsgChannel* FindChannel_CS (uint32_t protocol, bool server) {
    if (!s_channels)
        return nil;

    std::list<NetMsgChannel*>::iterator it = s_channels->begin();
    for (; it != s_channels->end(); ++it) {
        if (((*it)->m_protocol == protocol) && ((*it)->m_server == server))
            return *it;
    }

    return nil;
}

//===========================================================================
static NetMsgChannel* FindOrCreateChannel_CS (uint32_t protocol, bool server) {
    if (!s_channels) {
        s_channels = new std::list<NetMsgChannel*>();
    }

    // find or create protocol
    NetMsgChannel * channel = FindChannel_CS(protocol, server);
    if (!channel) {
        channel                 = new NetMsgChannel();
        channel->m_protocol     = protocol;
        channel->m_server       = server;
        channel->m_largestRecv  = 0;

        s_channels->push_back(channel);
        channel->IncRef("ChannelLink");
    }

    return channel;
}


/*****************************************************************************
*
*   Module functions
*
***/

//============================================================================
NetMsgChannel * NetMsgChannelLock (
    unsigned        protocol,
    bool            server,
    uint32_t *      largestRecv
) {
    NetMsgChannel * channel;
    s_channelCrit.Enter();
    if (nil != (channel = FindChannel_CS(protocol, server))) {
        *largestRecv = channel->m_largestRecv;
        channel->IncRef("ChannelLock");
    }
    else {
        *largestRecv = 0;
    }
    s_channelCrit.Leave();
    return channel;
}

//============================================================================
void NetMsgChannelUnlock (
    NetMsgChannel * channel
) {
    s_channelCrit.Enter();
    {
        channel->DecRef("ChannelLock");
    }
    s_channelCrit.Leave();
}

//============================================================================
const NetMsgInitRecv * NetMsgChannelFindRecvMessage (
    NetMsgChannel * channel,
    unsigned        messageId
) {
    // Is message in range?
    if (messageId >= channel->m_recvMsgs.Count())
        return nil;

    // Is message defined?
    const NetMsgInitRecv * recvMsg = &channel->m_recvMsgs[messageId];
    if (!recvMsg->msg.count)
        return nil;

    // Success!
    return recvMsg;
}

//============================================================================
const NetMsgInitSend * NetMsgChannelFindSendMessage (
    NetMsgChannel * channel,
    unsigned        messageId
) {
    // Is message in range?
    ASSERT(messageId < channel->m_sendMsgs.Count());

    // Is message defined?
    const NetMsgInitSend * sendMsg = &channel->m_sendMsgs[messageId];
    ASSERTMSG(sendMsg->msg.count, "NetMsg not found for send");

    return sendMsg;
}

//============================================================================
void NetMsgChannelGetDhConstants (
    const NetMsgChannel *   channel,
    uint32_t *              dh_g,
    const plBigNum**        dh_xa,
    const plBigNum**        dh_n
) {
    if (dh_g) *dh_g   =  channel->m_dh_g;
    if (dh_xa) *dh_xa  = &channel->m_dh_xa;
    if (dh_n) *dh_n   = &channel->m_dh_n;
}


}   // namespace pnNetCli


/*****************************************************************************
*
*   Exports
*
***/

//===========================================================================
void NetMsgProtocolRegister (
    uint32_t                protocol,
    bool                    server,
    const NetMsgInitSend    sendMsgs[],
    uint32_t                sendMsgCount,
    const NetMsgInitRecv    recvMsgs[],
    uint32_t                recvMsgCount,
    uint32_t                dh_g,
    const plBigNum&         dh_xa,    // client: dh_x     server: dh_a
    const plBigNum&         dh_n
) {
    s_channelCrit.EnterSafe();
    {
        NetMsgChannel * channel = FindOrCreateChannel_CS(protocol, server);

        // make sure no connections have been established on this protocol, otherwise
        // we'll be modifying a live data structure; NetCli's don't lock their protocol
        // to operate on it once they have linked to it!
        ASSERT(channel->GetRefCount() == 1);

        channel->m_dh_g     = dh_g;
        channel->m_dh_xa    = dh_xa;
        channel->m_dh_n     = dh_n;

        if (sendMsgCount)
            AddSendMsgs_CS(channel, sendMsgs, sendMsgCount);
        if (recvMsgCount)
            AddRecvMsgs_CS(channel, recvMsgs, recvMsgCount);
    }
    s_channelCrit.LeaveSafe();
}

//===========================================================================
void NetMsgProtocolDestroy (uint32_t protocol, bool server) {
    s_channelCrit.EnterSafe();
    if (NetMsgChannel* channel = FindChannel_CS(protocol, server)) {
        s_channels->remove(channel);
        channel->DecRef("ChannelLink");
    }
    s_channelCrit.LeaveSafe();
}
