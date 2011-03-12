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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetCli/pnNcChannel.cpp
*   
***/

#include "Pch.h"
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
    inline void Enter ()        { m_critsect.Enter();               }
    inline void Leave ()        { m_critsect.Leave();               }
    inline void EnterSafe ()    { if (m_init) m_critsect.Enter();   }
    inline void LeaveSafe ()    { if (m_init) m_critsect.Leave();   }

private:
    bool        m_init;
    CCritSect   m_critsect;
};

struct NetMsgChannel : AtomicRef {
    LINK(NetMsgChannel)     m_link;
    unsigned                m_protocol;
    bool                    m_server;

    // Message definitions
    unsigned                m_largestRecv;
    ARRAY(NetMsgInitSend)   m_sendMsgs;
    ARRAY(NetMsgInitRecv)   m_recvMsgs;

    // Diffie-Hellman constants
    unsigned                m_dh_g;
    BigNum                  m_dh_xa;    // client: dh_x     server: dh_a
    BigNum                  m_dh_n;
};

static ChannelCrit              s_channelCrit;
static LIST(NetMsgChannel) *    s_channels;


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
        while (NetMsgChannel * const channel = s_channels->Head()) {
            s_channels->Unlink(channel);
            channel->DecRef("ChannelLink");
        }

        DEL(s_channels);
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

    unsigned maxBytes = sizeof(word);    // for message id
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
                maxBytes += sizeof(qword);
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

        const unsigned bytes = ValidateMsg(dst->msg);
        channel->m_largestRecv = max(channel->m_largestRecv, bytes);
    }
}

//===========================================================================
static NetMsgChannel * FindChannel_CS (unsigned protocol, bool server) {
    if (!s_channels)
        return nil;

    NetMsgChannel * channel = s_channels->Head();
    for (; channel; channel = s_channels->Next(channel)) {
        if ((channel->m_protocol == protocol) && (channel->m_server == server))
            break;
    }

    return channel;
}

//===========================================================================
static NetMsgChannel * FindOrCreateChannel_CS (unsigned protocol, bool server) {
    if (!s_channels) {
        s_channels = NEW(LIST(NetMsgChannel));
        s_channels->SetLinkOffset(offsetof(NetMsgChannel, m_link));
    }

    // find or create protocol
    NetMsgChannel * channel = FindChannel_CS(protocol, server);
    if (!channel) {
        channel                 = NEW(NetMsgChannel);
        channel->m_protocol     = protocol;
        channel->m_server       = server;
        channel->m_largestRecv  = 0;

        s_channels->Link(channel);
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
    unsigned *      largestRecv
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
    unsigned *              dh_g,
    const BigNum **         dh_xa,
    const BigNum **         dh_n
) {
    *dh_g   =  channel->m_dh_g;
    *dh_xa  = &channel->m_dh_xa;
    *dh_n   = &channel->m_dh_n;
}


}   // namespace pnNetCli


/*****************************************************************************
*
*   Exports
*
***/

//===========================================================================
void NetMsgProtocolRegister (
    unsigned                protocol,
    bool                    server,
    const NetMsgInitSend    sendMsgs[],
    unsigned                sendMsgCount,
    const NetMsgInitRecv    recvMsgs[],
    unsigned                recvMsgCount,
    unsigned                dh_g,
    const BigNum &          dh_xa,    // client: dh_x     server: dh_a
    const BigNum &          dh_n
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
void NetMsgProtocolDestroy (unsigned protocol, bool server) {
    s_channelCrit.EnterSafe();
    if (NetMsgChannel * channel = FindChannel_CS(protocol, server)) {
        s_channels->Unlink(channel);
        channel->DecRef("ChannelLink");
    }
    s_channelCrit.LeaveSafe();
}
