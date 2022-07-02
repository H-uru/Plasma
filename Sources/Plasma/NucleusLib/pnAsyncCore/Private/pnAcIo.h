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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCore/Private/pnAcIo.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNASYNCCORE_PRIVATE_PNACIO_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCore/Private/pnAcIo.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNASYNCCORE_PRIVATE_PNACIO_H

#include <functional>

#include "pnNetCommon/plNetAddress.h"
#include "pnUUID/pnUUID.h"


/****************************************************************************
*
*   Global types and constants
*
***/

typedef struct AsyncIdStruct *         AsyncId;
typedef struct AsyncSocketStruct *     AsyncSocket;
typedef struct AsyncCancelIdStruct *   AsyncCancelId;

constexpr unsigned kAsyncSocketBufferSize   = 1460;

/****************************************************************************
*
*   Socket connect packet
*
***/

#pragma pack(push,1)
struct AsyncSocketConnectPacket {
    uint8_t     connType;
    uint16_t    hdrBytes;
    uint32_t    buildId;
    uint32_t    buildType;
    uint32_t    branchId;
    plUUID      productId;
};
#pragma pack(pop)


/****************************************************************************
*
*   Socket event notifications
*
***/

enum EAsyncNotifySocket {
    kNotifySocketConnectFailed,
    kNotifySocketConnectSuccess,
    kNotifySocketDisconnect,
    kNotifySocketRead,
    kNotifySocketWrite
};

struct AsyncNotifySocket {
    void *          param;
    AsyncId         asyncId;

    AsyncNotifySocket() : param(), asyncId() { }
};

struct AsyncNotifySocketConnect : AsyncNotifySocket {
    plNetAddress    localAddr;
    plNetAddress    remoteAddr;
    unsigned        connType;

    AsyncNotifySocketConnect() : connType(0) { }
};

struct AsyncNotifySocketListen : AsyncNotifySocketConnect {
    unsigned        buildId;
    unsigned        buildType;
    unsigned        branchId;
    plUUID          productId;
    plNetAddress    addr;
    uint8_t *       buffer;
    unsigned        bytes;
    unsigned        bytesProcessed;

    AsyncNotifySocketListen()
        : buildId(), buildType(), branchId(), buffer(), bytes(),
          bytesProcessed() { }
};

struct AsyncNotifySocketRead : AsyncNotifySocket {
    uint8_t *       buffer;
    unsigned        bytes;
    unsigned        bytesProcessed;

    AsyncNotifySocketRead() : buffer(), bytes(), bytesProcessed() { }
};


struct AsyncNotifySocketWrite : AsyncNotifySocketRead {
    unsigned        bytesCommitted;
    
    AsyncNotifySocketWrite() : AsyncNotifySocketRead(), bytesCommitted() { }
};

// return false to disconnect
typedef std::function<bool (AsyncSocket /* sock */,
                            EAsyncNotifySocket /* code */,
                            AsyncNotifySocket* /* notify */,
                            void** /* userState */)> FAsyncNotifySocketProc;


/****************************************************************************
*
*   Connection type functions
*
***/

// These codes may not be changed unless ALL servers and clients are
// simultaneously replaced; so basically forget it =)
enum EConnType {
    kConnTypeNil                    = 0,
    
    // For test applications
    kConnTypeDebug                  = 1,

    // Binary connections
    kConnTypeCliToAuth              = 10,
    kConnTypeCliToGame              = 11,
    kConnTypeSrvToAgent             = 12,
    kConnTypeSrvToMcp               = 13,
    kConnTypeSrvToVault             = 14,
    kConnTypeSrvToDb                = 15,
    kConnTypeCliToFile              = 16,
    kConnTypeSrvToState             = 17,
    kConnTypeSrvToLog               = 18,
    kConnTypeSrvToScore             = 19,
    kConnTypeCliToCsr               = 20, // DEAD
    kConnTypeSimpleNet              = 21, // DEAD
    kConnTypeCliToGateKeeper        = 22,
    
    // Text connections
    kConnTypeAdminInterface         = 97,   // 'a'

    kNumConnTypes
};
static_assert(kNumConnTypes <= 0xFF, "EConnType overflows uint8");

#define IS_TEXT_CONNTYPE(c)     \
    (((int)(c)) == kConnTypeAdminInterface)


/****************************************************************************
*
*   Socket functions
*
***/

void AsyncSocketConnect (
    AsyncCancelId *         cancelId,
    const plNetAddress&     netAddr,
    FAsyncNotifySocketProc  notifyProc,
    void *                  param = nullptr,
    const void *            sendData = nullptr,
    unsigned                sendBytes = 0
);

// Due to the asynchronous nature of sockets, the connect may complete
// before the cancel does... you have been warned.
void AsyncSocketConnectCancel(AsyncCancelId cancelId);

void AsyncSocketDisconnect (
    AsyncSocket             sock,
    bool                    hardClose
);

// This function must only be called after receiving a kNotifySocketDisconnect
void AsyncSocketDelete (AsyncSocket sock);

// Returns false if socket has been closed
bool AsyncSocketSend (
    AsyncSocket             sock,
    const void *            data,
    unsigned                bytes
);

void AsyncSocketEnableNagling (
    AsyncSocket             sock,
    bool                    enable
);


/****************************************************************************
*
*   Dns functions
*
***/

typedef std::function<void (void* /* param */, const ST::string& /* name */,
                            const std::vector<plNetAddress>& /* addrs */)> FAsyncLookupProc;

void AsyncAddressLookupName (
    FAsyncLookupProc    lookupProc,
    const ST::string &  name,
    unsigned            port,
    void *              param
);
