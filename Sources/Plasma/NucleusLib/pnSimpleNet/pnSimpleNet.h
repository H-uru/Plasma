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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnSimpleNet/pnSimpleNet.h
*   
***/

#ifndef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNSIMPLENET_PNSIMPLENET_H
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNSIMPLENET_PNSIMPLENET_H


#include "pnUtils/pnUtils.h"
#include "pnProduct/pnProduct.h"
#include "pnNetBase/pnNetBase.h"
#include "pnAsyncCore/pnAsyncCore.h"

/*****************************************************************************
*
*   SimpleNet:
*     - TCP only, Nagle buffered only.
*     - Asynchronous callbacks only.
*     - Not encrypted, not compressed, no transaction support.
*     - Good for trivial networked applications only. Examples include:
*         - CSR client automation by CSR tools.
*         - Bob and I (eap) talked about 3dsmax automating
*           the client for the purpose of reloading reexported
*           assets.  With SimpleNet, the client could be running
*           on a separate machine than Max.
*
***/

// Because newer clients must remain compatible with older servers,
// these values may never change.
enum ESimpleNetChannel {
    kSimpleNetChannelNil    = 0,
    kSimpleNetChannelCsr    = 1,
    kSimpleNetChannelMax    = 2,
    
    kMaxSimpleNetChannels
};
COMPILER_ASSERT_HEADER(ESimpleNetChannel, kMaxSimpleNetChannels <= 0xff);


//============================================================================
// BEGIN PACKED DATA STRUCTURES
//============================================================================
#include <PshPack1.h>

//============================================================================
// Connect packet

struct SimpleNet_ConnData {
    unsigned    channelId;
};
struct SimpleNet_Connect {
    AsyncSocketConnectPacket    hdr;
    SimpleNet_ConnData          data;
};

//============================================================================
// Message header

struct SimpleNet_MsgHeader {
private:
    uint32_t   channelId;
public:
    uint32_t   messageId;
    uint32_t   messageBytes;
    
    SimpleNet_MsgHeader (uint32_t channelId, uint32_t messageId)
    : channelId(channelId)
    , messageId(messageId)
    #ifdef HS_DEBUGGING
    , messageBytes((uint32_t)-1)
    #endif
    { }
};


/*****************************************************************************
*
*   Simple Network API
*
***/

struct SimpleNetConn;

void SimpleNetInitialize ();
void SimpleNetShutdown ();

void SimpleNetConnIncRef (SimpleNetConn * conn);
void SimpleNetConnDecRef (SimpleNetConn * conn);

typedef bool (*FSimpleNetOnMsg) (       // return false to disconnect socket
    SimpleNetConn *         conn,
    SimpleNet_MsgHeader *   msg
);
typedef void (*FSimpleNetOnError) (
    SimpleNetConn * conn,
    ENetError       error
);
typedef void (*FSimpleNetOnConnect) (
    void *          param,
    SimpleNetConn * conn,
    ENetError       result
);
typedef bool (*FSimpleNetQueryAccept) ( // return true to accept incoming connection
    void *              param,
    unsigned            channel,
    SimpleNetConn *     conn,
    const NetAddress &  addr
);

void SimpleNetCreateChannel (
    unsigned            channel,
    FSimpleNetOnMsg     onMsg,
    FSimpleNetOnError   onError
);
void SimpleNetDestroyChannel (
    unsigned            channel
);

bool SimpleNetStartListening (
    FSimpleNetQueryAccept   queryAccept,
    void *                  param
);
void SimpleNetStopListening ();

void SimpleNetStartConnecting (
    unsigned            channel,
    const wchar_t         addr[],
    FSimpleNetOnConnect onConnect,
    void *              param
);
void SimpleNetDisconnect (
    SimpleNetConn *     conn
);
void SimpleNetSend (
    SimpleNetConn *         conn,
    SimpleNet_MsgHeader *   msg
);

#endif // PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNSIMPLENET_PNSIMPLENET_H
