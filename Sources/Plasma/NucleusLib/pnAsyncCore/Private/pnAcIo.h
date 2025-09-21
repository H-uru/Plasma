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

#ifndef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNASYNCCORE_PRIVATE_PNACIO_H
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNASYNCCORE_PRIVATE_PNACIO_H

#include <functional>
#include <optional>

#include "pnNetCommon/plNetAddress.h"
#include "pnUUID/pnUUID.h"


/****************************************************************************
*
*   Global types and constants
*
***/

typedef struct AsyncSocketStruct *     AsyncSocket;
typedef struct AsyncCancelIdStruct *   AsyncCancelId;

constexpr unsigned kAsyncSocketBufferSize   = 1460;

class AsyncNotifySocketCallbacks
{
public:
    virtual void AsyncNotifySocketConnectFailed(plNetAddress remoteAddr) = 0;
    virtual bool AsyncNotifySocketConnectSuccess(AsyncSocket sock, const plNetAddress& localAddr, const plNetAddress& remoteAddr) = 0;
    virtual void AsyncNotifySocketDisconnect(AsyncSocket sock) = 0;
    virtual std::optional<size_t> AsyncNotifySocketRead(AsyncSocket sock, uint8_t* buffer, size_t bytes) = 0;
};


/****************************************************************************
*
*   Socket functions
*
***/

void AsyncSocketConnect (
    AsyncCancelId *         cancelId,
    const plNetAddress&     netAddr,
    AsyncNotifySocketCallbacks* callbacks,
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
    size_t                  bytes
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

typedef std::function<void(const std::vector<plNetAddress>& /* addrs */)> FAsyncLookupProc;

void AsyncAddressLookupName (
    const ST::string& name,
    unsigned port,
    FAsyncLookupProc lookupProc
);

#endif // PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNASYNCCORE_PRIVATE_PNACIO_H
