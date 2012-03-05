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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCoreExe/Private/W9x/pnAceW9xInt.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNASYNCCOREEXE_PRIVATE_W9X_PNACEW9XINT_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCoreExe/Private/W9x/pnAceW9xInt.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNASYNCCOREEXE_PRIVATE_W9X_PNACEW9XINT_H


namespace W9x {

/*****************************************************************************
*
*   Internal types
*
***/

class CThreadDispObject : public AtomicRef {
public:
    CThreadDispObject ();
    virtual ~CThreadDispObject () { }
    void Close ();
    virtual void Complete (void * op, CCritSect * critSect, AsyncId asyncId) = 0;
    virtual void Delete (void * op) = 0;
    AsyncId Queue (void * op);
};


/*****************************************************************************
*
*   W9x internal async API
*
***/

void W9xThreadInitialize ();
void W9xThreadDestroy (unsigned exitThreadWaitMs);
void W9xThreadSignalShutdown ();
void W9xThreadWaitForShutdown ();
void W9xThreadSleep (unsigned sleepMs);
bool W9xThreadWaitId (
    AsyncFile   file, 
    AsyncId     asyncId, 
    unsigned    timeoutMs
);
void W9xSocketConnect (
    AsyncCancelId *         cancelId,
    const NetAddress &      netAddr,
    FAsyncNotifySocketProc  notifyProc,
    void *                  param,
    const void *            sendData,
    unsigned                sendBytes,
    unsigned                connectMs,
    unsigned                localPort
);
void W9xSocketConnectCancel (
    FAsyncNotifySocketProc  notifyProc,
    AsyncCancelId           cancelId
);
void W9xSocketDisconnect (
    AsyncSocket     sock,
    bool            hardClose
);
void W9xSocketDelete (AsyncSocket sock);
void W9xSocketDestroy ();
bool W9xSocketSend (
    AsyncSocket     sock,
    const void *    data,
    unsigned        bytes
);
bool W9xSocketWrite (
    AsyncSocket     sock,
    const void *    buffer,
    unsigned        bytes,
    void *          param
);
void W9xSocketSetNotifyProc (
    AsyncSocket             sock,
    FAsyncNotifySocketProc  notifyProc
);
void W9xSocketSetBacklogAlloc (
    AsyncSocket     sock,
    unsigned        bufferSize
);
unsigned W9xSocketStartListening (
    const NetAddress &      listenAddr,
    FAsyncNotifySocketProc  notifyProc
);
void W9xSocketStopListening (
    const NetAddress &      listenAddr,
    FAsyncNotifySocketProc  notifyProc
);
void W9xSocketEnableNagling (
    AsyncSocket             conn,
    bool                    enable
);


}   // namespace W9x
