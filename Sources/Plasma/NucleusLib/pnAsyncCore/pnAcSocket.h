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

/// @warning These codes may not be changed unless ALL servers and clients are
/// simultaneously replaced; so basically forget it =)
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
static_assert(sizeof(EConnType) == sizeof(uint8_t), "typed enum not supported!");
//static_assert(kNumConnTypes <= 0xFF, "EConnType overflows uint8");

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
    static_assert(offsetof(ConnectPacket, hdrBytes) == sizeof(EConnType), "'#pragma pack()' not supported");


    /****************************************************************************
    *
    *   Socket event notifications
    *
    ***/

    /// Socket event notification type.
    /// @see FNotifyProc
    enum ENotify {
        kNotifyConnectFailed,   ///< notified when @ref Connect() fail with @ref NotifyConnect
        kNotifyConnectSuccess,  ///< notified when @ref Connect() success with @ref NotifyConnect
        kNotifyDisconnect,      ///< notified when @ref Delete() can be safly called after a @ref Disconnect() with @b notify = nil
        kNotifyListenSuccess,
        kNotifyRead,            ///< notified when data come from outside with @ref NotifyRead
        kNotifyWrite            ///< notified when @ref Write() terminate (on success and fail) with @ref NotifyWrite
    };
    
    struct Notify;
    struct NotifyConnect;
    struct NotifyListen;
    struct NotifyRead;
    struct NotifyWrite;
    
    /// Pointer-like cancel handler for @ref Connect() operation
    class Cancel {
        void *  ptr;
        Cancel (void * p) : ptr(p) {}
    public:
        inline Cancel() : ptr(nullptr) {}
        
        bool ConnectCancel ();                  ///< cancel the associate connect operation if possible. And clear itself @return true if realy canceled
        inline void Clear () { ptr = nullptr; } ///< clear the pointer content
        
        inline operator bool ()   { return  ptr; }  ///< @return true if not empty
        inline bool operator ! () { return !ptr; }  ///< @return true if empty
        
        friend class AsyncSocket;
    };
    
    /// Event notification handler function
    /// @param sock socket where event append
    /// @param code notification type
    /// @param notify notification data
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
    
    /// Create and connect an async socket to specified peer
    /// @param netAddr Address of the peer
    /// @param notifyProc function that will be notified when socket events occure
    /// @param param user-defined data placed in the next @ref NotifyConnect::param
    /// @param sendData data to send after connection success
    /// @param sendBytes size of sendData
    /// @param connectMs connection timeout. accept kPosInfinity32.
    /// @param localPort local port where bind input stream. 0 to disable binding.
    /// @return handler to cancel this connection operation
    /// @see @ref kNotifyConnectFailed
    /// @see @ref kNotifyConnectSuccess
    /// @see @ref ConnectCancel()
    static Cancel Connect (
        const plNetAddress&     netAddr,
        FNotifyProc             notifyProc,
        void *                  param = nullptr,
        const void *            sendData = nullptr,
        unsigned                sendBytes = 0,
        unsigned                connectMs = kPosInfinity32,      // 0 => use default value
        unsigned                localPort = 0       // 0 => don't bind local port
    );

    /// Cancel all connection operation using a specific notifyProc
    /// @param notifyProc function used by connect operations to cancel
    /// @see @ref kNotifyConnectFailed
    /// @see @ref Connect()
    void ConnectCancel (FNotifyProc notifyProc);

    /// Cancel all operation and close this socket
    /// @param hardClose if true, the socket is closed immediatly.
    void Disconnect (bool hardClose = false);

    /// Delete this socket and associates data (AsyncSocket::user must be deleted by the caller)
    /// @warning this function must only be called after @b notifyProc receive @ref kNotifyDisconnect
    void Delete ();

    /// send data to the peer
    /// @param data data to send
    /// @param bytes size of data
    /// @note data is internaly copied, it can be modified/destroyed immediatly after function return.
    bool Send (const void * data, unsigned bytes);

    /// send data to the peer
    /// @param buffer data to send
    /// @param bytes size of @b buffer
    /// @param param user defined data to set in @ref NotifyWrite::param
    /// @warning Buffer must stay valid until I/O has completed 
    /// @see @ref kNotifyWrite
    bool Write (
        const void *            buffer,
        unsigned                bytes,
        void *                  param
    );

    /// Change the function that will be notified when event append on this socket
    void SetNotifyProc (FNotifyProc  notifyProc);

    // A backlog of zero (the default) means that no buffering is performed when
    // the TCP send buffer is full, and the send() function will close the socket
    // on send fail
    //void SetBacklogAlloc (unsigned bufferSize); // TODO?

    // On failure, returns 0
    // On success, returns bound port (if port number was zero, returns assigned port)
    // For connections that will use kConnType* connections, set notifyProc = nil;
    // the handler will be found when connection packet is received.
    // for connections with hard-coded behavior, set the notifyProc here (e.g. for use
    // protocols like SNMP on port 25)
    /// Create a socket and wait connection from outside
    /// @param listenAddr Local address to listen
    /// @param notifyProc function taht will be notified when event append on this socket.
    /// if nil, function returned by @ref FindNotifyProc() (after necesary data are receive) is used
    /// @see @ref kNotifyListenSuccess
    /// @todo implement
    static unsigned StartListening (
        const plNetAddress&     listenAddr,
        FNotifyProc             notifyProc = nullptr
    );
    /// @todo implement
    static void StopListening (
        const plNetAddress&     listenAddr,
        FNotifyProc             notifyProc = nullptr
    );
    
    /// set usage of nagling algorithm
    /// @param enable true to enable
    /// @note by default, nagling algorithm is enable
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
    
    /// @return false after @ref Disconnect().
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
    void *          param; ///< user defined param, nil by default.
    //AsyncId         asyncId;

    Notify() : param(nullptr) { }
};

struct AsyncSocket::NotifyConnect : AsyncSocket::Notify {
    plNetAddress    localAddr;
    plNetAddress    remoteAddr;
    EConnType       connType;   

    NotifyConnect() : connType(kConnTypeNil) { }
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
    uint8_t *       buffer;         ///< readed data
    unsigned        bytes;          ///< size readed
    unsigned        bytesProcessed; ///< must be set by notifyProc to the size of proccessed data from buffer. Remain data will be placed at the start of the next read notification.

    NotifyRead() : buffer(nullptr), bytes(0), bytesProcessed(0) { }
};

/// @b buffer is the data passed to @ref Write() @n
/// @b bytes is the total size of the buffer (sended and to send) @n
/// @b byteProcessed is the total size of the buffer send to the peer.
/// @note buffer can be modified/delete when @b bytes == @b byteProcessed.
struct AsyncSocket::NotifyWrite : AsyncSocket::NotifyRead {};

#endif

