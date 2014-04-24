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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCore/pnAcSocket.h
*   
***/

#ifndef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNASYNCCORE_PNACSOCKET_H
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNASYNCCORE_PNACSOCKET_H

#include "pnNetCommon/plNetAddress.h"
#include "pnUUID/pnUUID.h"


/****************************************************************************
*
*   Connection type types
*
***/

// These codes may not be changed unless ALL servers and clients are
// simultaneously replaced; so basically forget it =)
enum EConnType : uint8_t {
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

#define IS_TEXT_CONNTYPE(c) ((EConnType)c == kConnTypeAdminInterface)


/****************************************************************************
*
*   AsyncSocket module
*
***/

class AsyncSocket {
    AsyncSocket() {};
    
public:


    /****************************************************************************
    *
    *   Socket connect packet
    *
    ***/

    #pragma pack(push,1)
    struct ConnectPacket {
        EConnType   connType;
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

    enum ENotify {
        kNotifyConnectFailed,
        kNotifyConnectSuccess,
        kNotifyDisconnect,
        kNotifyListenSuccess,
        kNotifyRead,
        kNotifyWrite
    };
    
    struct Notify;
    struct NotifyConnect;
    struct NotifyListen;
    struct NotifyRead;
    struct NotifyWrite;
    
    class Cancel {
        void *  ptr;
        Cancel (void * p) : ptr(p) {}
    public:
        inline Cancel() : ptr(nullptr) {}
        
        bool ConnectCancel ();
        inline void Clear () { ptr = nullptr; }
        
        inline operator bool ()   { return  ptr; }
        inline bool operator ! () { return !ptr; }
        
        friend class AsyncSocket;
    };
    
    /// @return false to disconnect
    typedef bool (* FNotifyProc) (
        AsyncSocket *       sock,
        ENotify             code,
        Notify *            notify
    );
    
    /****************************************************************************
    *
    *   Socket data
    *
    ***/
    
    static const unsigned   kBufferSize = 1460;
    void *  user; ///< user defined data
    
    
    /****************************************************************************
    *
    *   Socket functions
    *
    ***/
    
    static Cancel Connect (
        const plNetAddress&     netAddr,
        FNotifyProc             notifyProc,
        void *                  param = nullptr,
        const void *            sendData = nullptr,
        unsigned                sendBytes = 0,
        unsigned                connectMs = 0,      // 0 => use default value
        unsigned                localPort = 0       // 0 => don't bind local port
    );

    // Due to the asynchronous nature of sockets, the connect may complete
    // before the cancel does... you have been warned.
    void ConnectCancel (FNotifyProc  notifyProc);

    void Disconnect (bool hardClose = false);

    // This function must only be called after receiving a kNotifySocketDisconnect
    void Delete ();

    // Returns false of socket has been closed
    bool Send (const void * data, unsigned bytes);

    // Buffer must stay valid until I/O has completed
    // Returns false if socket has been closed
    bool Write (
        const void *            buffer,
        unsigned                bytes,
        void *                  param
    );

    // This function must only be called from with a socket notification callback.
    // Calling at any other time is a crash bug waiting to happen!
    void SetNotifyProc (FNotifyProc  notifyProc);

    // A backlog of zero (the default) means that no buffering is performed when
    // the TCP send buffer is full, and the send() function will close the socket
    // on send fail
    void SetBacklogAlloc (unsigned bufferSize);

    // On failure, returns 0
    // On success, returns bound port (if port number was zero, returns assigned port)
    // For connections that will use kConnType* connections, set notifyProc = nil;
    // the handler will be found when connection packet is received.
    // for connections with hard-coded behavior, set the notifyProc here (e.g. for use
    // protocols like SNMP on port 25)
    static unsigned StartListening (
        const plNetAddress&     listenAddr,
        FNotifyProc             notifyProc = nullptr
    );
    static void StopListening (
        const plNetAddress&     listenAddr,
        FNotifyProc             notifyProc = nullptr
    );

    void EnableNagling (bool enable);
    
    
    /****************************************************************************
    *
    *   Connection type functions
    *
    ***/
    
    static void Register (
        EConnType       connType,
        FNotifyProc     notifyProc,
        unsigned        buildId = 0,
        unsigned        buildType = 0,
        unsigned        branchId = 0,
        const plUUID&   productId = kNilUuid
    );
    
    static void Unregister (
        EConnType       connType,
        FNotifyProc     notifyProc,
        unsigned        buildId = 0,
        unsigned        buildType = 0,
        unsigned        branchId = 0,
        const plUUID&   productId = kNilUuid
    );

    static FNotifyProc FindNotifyProc (
        const uint8_t           buffer[],
        unsigned                bytes,
        unsigned *              bytesProcessed,
        unsigned *              connType,
        unsigned *              buildId,
        unsigned *              buildType,
        unsigned *              branchId,
        plUUID*                 productId
    );
    
    
    /****************************************************************************
    *
    *   socket status
    *
    ***/
    
    /// @return false after Disconnect call.
    bool Active ();
    
    inline operator bool ()   { return  Active(); }
    inline bool operator ! () { return !Active(); }
    
    
    
    class P; // private data
    friend class P;
};

/****************************************************************************
*
*   Socket event notifications
*
***/

struct AsyncSocket::Notify {
    void *          param;
    //AsyncId         asyncId;

    Notify() : param(nullptr) { }
};

struct AsyncSocket::NotifyConnect : AsyncSocket::Notify {
    plNetAddress    localAddr;
    plNetAddress    remoteAddr;
    unsigned        connType;

    NotifyConnect() : connType(0) { }
};

struct AsyncSocket::NotifyListen : AsyncSocket::NotifyConnect {
    unsigned        buildId;
    unsigned        buildType;
    unsigned        branchId;
    plUUID          productId;
    plNetAddress    addr;
    uint8_t *       buffer;
    unsigned        bytes;
    unsigned        bytesProcessed;

    NotifyListen()
        : buildId(0), buildType(0), branchId(0), buffer(nullptr), bytes(0),
          bytesProcessed(0) { }
};

struct AsyncSocket::NotifyRead : AsyncSocket::Notify {
    uint8_t *       buffer;
    unsigned        bytes;
    unsigned        bytesProcessed;

    NotifyRead() : buffer(nullptr), bytes(0), bytesProcessed(0) { }
};

struct AsyncSocket::NotifyWrite : AsyncSocket::NotifyRead {};

#endif

