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

#include "hsRefCnt.h"


namespace pnNetCli {

/*****************************************************************************
*
*   Private
*
***/

struct NetMsgChannel : hsRefCnt {
    NetMsgChannel(uint32_t protocol, uint32_t dh_g, const plBigNum& dh_x, const plBigNum& dh_n) :
        m_protocol(protocol),
        m_dh_g(dh_g),
        m_dh_x(dh_x),
        m_dh_n(dh_n)
    {}

    uint32_t                m_protocol;

    // Message definitions
    std::vector<NetMsgInitSend>  m_sendMsgs;
    std::vector<NetMsgInitRecv>  m_recvMsgs;

    // Diffie-Hellman constants
    uint32_t                m_dh_g;
    plBigNum                m_dh_x;
    plBigNum                m_dh_n;
};


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
            if (field.type == kNetMsgFieldVarPtr) {
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

            case kNetMsgFieldVarPtr:
            break;

            case kNetMsgFieldVarCount:
                prevFieldWasVarCount = true;
                // fall-thru...
            case kNetMsgFieldString:
            case kNetMsgFieldData:
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
        ASSERT(msgs[i].msg->count);
        maxMsgId = std::max(msgs[i].msg->messageId, maxMsgId);
    }
    return maxMsgId;
}

//===========================================================================
static void AddSendMsgs(
    NetMsgChannel *         channel,
    const NetMsgInitSend    src[],
    unsigned                count
) {
    const size_t reqSize = MaxMsgId(src, count) + 1;
    if (channel->m_sendMsgs.size() < reqSize)
        channel->m_sendMsgs.resize(reqSize);

    for (const NetMsgInitSend * term = src + count; src < term; ++src) {
        NetMsgInitSend * const dst = &channel->m_sendMsgs[src[0].msg->messageId];

        // check to ensure that the message id isn't already used
        ASSERT(!dst->msg);

        *dst = *src;
        ValidateMsg(*dst->msg);
    }
}

//===========================================================================
static void AddRecvMsgs(
    NetMsgChannel *         channel,
    const NetMsgInitRecv    src[],
    unsigned                count
) {
    const size_t reqSize = MaxMsgId(src, count) + 1;
    if (channel->m_recvMsgs.size() < reqSize)
        channel->m_recvMsgs.resize(reqSize);

    for (const NetMsgInitRecv * term = src + count; src < term; ++src) {
        ASSERT(src->recv);
        NetMsgInitRecv * const dst = &channel->m_recvMsgs[src[0].msg->messageId];

        // check to ensure that the message id isn't already used
        ASSERT(!dst->msg);

        // copy the message handler
        *dst = *src;

        ValidateMsg(*dst->msg);
    }
}


/*****************************************************************************
*
*   Module functions
*
***/

//============================================================================
void NetMsgChannelLock(NetMsgChannel* channel)
{
    channel->Ref("ChannelLock");
}

//============================================================================
void NetMsgChannelUnlock(NetMsgChannel* channel)
{
    channel->UnRef("ChannelLock");
}

//============================================================================
const NetMsgInitRecv * NetMsgChannelFindRecvMessage (
    NetMsgChannel * channel,
    unsigned        messageId
) {
    // Is message in range?
    if (messageId >= channel->m_recvMsgs.size())
        return nullptr;

    // Is message defined?
    const NetMsgInitRecv * recvMsg = &channel->m_recvMsgs[messageId];
    if (!recvMsg->msg->count)
        return nullptr;

    // Success!
    return recvMsg;
}

//============================================================================
const NetMsgInitSend * NetMsgChannelFindSendMessage (
    NetMsgChannel * channel,
    uintptr_t       messageId
) {
    // Is message in range?
    ASSERT(messageId < channel->m_sendMsgs.size());

    // Is message defined?
    const NetMsgInitSend * sendMsg = &channel->m_sendMsgs[messageId];
    ASSERTMSG(sendMsg->msg->count, "NetMsg not found for send");

    return sendMsg;
}

//============================================================================
uint32_t NetMsgChannelGetProtocol(NetMsgChannel* channel)
{
    return channel->m_protocol;
}

void NetMsgChannelGetDhConstants (
    const NetMsgChannel *   channel,
    uint32_t *              dh_g,
    const plBigNum**        dh_x,
    const plBigNum**        dh_n
) {
    if (dh_g) *dh_g = channel->m_dh_g;
    if (dh_x) *dh_x = &channel->m_dh_x;
    if (dh_n) *dh_n = &channel->m_dh_n;
}


}   // namespace pnNetCli


/*****************************************************************************
*
*   Exports
*
***/

//===========================================================================
NetMsgChannel* NetMsgChannelCreate(
    uint32_t                protocol,
    const NetMsgInitSend    sendMsgs[],
    uint32_t                sendMsgCount,
    const NetMsgInitRecv    recvMsgs[],
    uint32_t                recvMsgCount,
    uint32_t                dh_g,
    const plBigNum&         dh_x,
    const plBigNum&         dh_n
) {
    NetMsgChannel* channel = new NetMsgChannel(protocol, dh_g, dh_x, dh_n);

    if (sendMsgCount) {
        AddSendMsgs(channel, sendMsgs, sendMsgCount);
    }
    if (recvMsgCount) {
        AddRecvMsgs(channel, recvMsgs, recvMsgCount);
    }

    return channel;
}

//===========================================================================
void NetMsgChannelDelete(NetMsgChannel* channel)
{
    channel->UnRef("Lifetime");
}
