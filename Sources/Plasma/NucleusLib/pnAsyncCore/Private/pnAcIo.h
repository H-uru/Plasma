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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCore/Private/pnAcIo.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNASYNCCORE_PRIVATE_PNACIO_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCore/Private/pnAcIo.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNASYNCCORE_PRIVATE_PNACIO_H


/****************************************************************************
*
*   Global types and constants
*
***/

typedef struct AsyncIdStruct *         AsyncId;
typedef struct AsyncFileStruct *       AsyncFile;
typedef struct AsyncSocketStruct *     AsyncSocket;
typedef struct AsyncCancelIdStruct *   AsyncCancelId;

const unsigned kAsyncSocketBufferSize   = 1460;

enum EFileError {
    kFileSuccess,
    kFileErrorInvalidParameter,
    kFileErrorFileNotFound,
    kFileErrorPathNotFound,
    kFileErrorAccessDenied,
    kFileErrorSharingViolation,
    kNumFileErrors
};

EFileError AsyncGetLastFileError ();

const wchar * FileErrorToString (EFileError error);


/****************************************************************************
*
*   File notifications
*
***/

enum EAsyncNotifyFile {
    kNotifyFileFlush,
    kNotifyFileRead,
    kNotifyFileWrite,
    kNotifyFileSequence,
    kNumFileNotifications
};

struct AsyncNotifyFile {
    void *          param;
    AsyncId         asyncId;
};

struct AsyncNotifyFileConnect : AsyncNotifyFile {
    qword           fileSize;
    qword           fileLastWriteTime;
};

struct AsyncNotifyFileFlush : AsyncNotifyFile {
    EFileError      error;
    qword           truncateSize;
};

struct AsyncNotifyFileRead : AsyncNotifyFile {
    qword           offset;
    byte *          buffer;
    unsigned        bytes;
};

typedef AsyncNotifyFileRead AsyncNotifyFileWrite;

struct AsyncNotifyFileSequence : AsyncNotifyFile {
    // no additional fields
};

typedef void (* FAsyncNotifyFileProc)(
    AsyncFile           file,
    EAsyncNotifyFile    code,
    AsyncNotifyFile *   notify,
    void **             userState
);


/****************************************************************************
*
*   File I/O functions
*
***/

// Desired access
const unsigned kAsyncFileReadAccess         = 0x80000000;
const unsigned kAsyncFileWriteAccess        = 0x40000000;
// Open mode (creation disposition)
const unsigned kAsyncFileModeCreateNew      = 1;
const unsigned kAsyncFileModeCreateAlways   = 2;
const unsigned kAsyncFileModeOpenExisting   = 3;
const unsigned kAsyncFileModeOpenAlways     = 4;
// Share mode flags
const unsigned kAsyncFileShareRead          = 0x00000001;
const unsigned kAsyncFileShareWrite         = 0x00000002;

AsyncFile AsyncFileOpen (
    const wchar             fullPath[],
    FAsyncNotifyFileProc    notifyProc,
    EFileError *            error,
    unsigned                desiredAccess,
    unsigned                openMode,
    unsigned                shareModeFlags,     // optional
    void *                  userState,          // optional
    qword *                 fileSize,           // optional
    qword *                 fileLastWriteTime   // optional
);

// Use with AsyncFileDelete/AsyncFileFlushBuffers
const qword kAsyncFileDontTruncate          = (qword) -1;

// This function may ONLY be called when there is no outstanding I/O against a file
// and no more I/O will be initiated against it.  This function guarantees that it
// will close the system file handle before it returns to that another open against
// the same filename can succeed.
void AsyncFileClose (
    AsyncFile   file,
    qword       truncateSize
);

void AsyncFileSetLastWriteTime (
    AsyncFile   file,
    qword       lastWriteTime
);

qword AsyncFileGetLastWriteTime (
    const wchar fileName[]
);

// Truncation occurs atomically, any writes which occur after
// AsyncFileFlushBuffers will be queued until the truncation completes
AsyncId AsyncFileFlushBuffers (
    AsyncFile   file, 
    qword       truncateSize,
    bool        notify,
    void *      param
);

const unsigned kAsyncFileRwNotify  = 1<<0;
const unsigned kAsyncFileRwSync    = 1<<1;

AsyncId AsyncFileRead (
    AsyncFile       file,
    qword           offset,
    void *          buffer,
    unsigned        bytes,
    unsigned        flags,
    void *          param
);

// Buffer must stay valid until I/O is completed
AsyncId AsyncFileWrite (
    AsyncFile       file,
    qword           offset,
    const void *    buffer,
    unsigned        bytes,
    unsigned        flags,
    void *          param
);

// Inserts a "null operation" into the list of reads and writes. The callback
// will be called when all preceding operations have successfully completed.
AsyncId AsyncFileCreateSequence (
    AsyncFile       file, 
    bool            notify, 
    void *          param
);

enum EFileSeekFrom {
    kFileSeekFromBegin,
    kFileSeekFromCurrent,
    kFileSeekFromEnd,
    kNumFileSeekFroms
};

bool AsyncFileSeek (
    AsyncFile       file,
    qword           distance,
    EFileSeekFrom   seekFrom
);


/****************************************************************************
*
*   Socket connect packet
*
***/

#include <PshPack1.h>
struct AsyncSocketConnectPacket {
    byte		connType;
    word		hdrBytes;
    dword		buildId;
    dword		buildType;
    dword		branchId;
    Uuid		productId;
};
#include <PopPack.h>


/****************************************************************************
*
*   Socket event notifications
*
***/

enum EAsyncNotifySocket {
    kNotifySocketConnectFailed,
    kNotifySocketConnectSuccess,
    kNotifySocketDisconnect,
    kNotifySocketListenSuccess,
    kNotifySocketRead,
    kNotifySocketWrite
};

struct AsyncNotifySocket {
    void *          param;
    AsyncId         asyncId;
};

struct AsyncNotifySocketConnect : AsyncNotifySocket {
    NetAddress      localAddr;
    NetAddress      remoteAddr;
    unsigned        connType;
};

struct AsyncNotifySocketListen : AsyncNotifySocketConnect {
    unsigned        buildId;
    unsigned        buildType;
    unsigned		branchId;
    Uuid            productId;
    NetAddress      addr;
    byte *          buffer;
    unsigned        bytes;
    unsigned        bytesProcessed;
};

struct AsyncNotifySocketRead : AsyncNotifySocket {
    byte *          buffer;
    unsigned        bytes;
    unsigned        bytesProcessed;
};

typedef AsyncNotifySocketRead AsyncNotifySocketWrite;

typedef bool (* FAsyncNotifySocketProc) (    // return false to disconnect
    AsyncSocket         sock,
    EAsyncNotifySocket  code,
    AsyncNotifySocket * notify,
    void **             userState
);


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
	kConnTypeCliToFile				= 16,
	kConnTypeSrvToState				= 17,
	kConnTypeSrvToLog 				= 18,
	kConnTypeSrvToScore				= 19,
	kConnTypeCliToCsr				= 20,
    kConnTypeSimpleNet				= 21,
	kConnTypeCliToGateKeeper		= 22,
    
    // Text connections
    kConnTypeAdminInterface         = 97,   // 'a'

    kNumConnTypes
};
COMPILER_ASSERT_HEADER(EConnType, kNumConnTypes < 256);

#define IS_TEXT_CONNTYPE(c)     \
    (((int)(c)) == kConnTypeAdminInterface)


void AsyncSocketRegisterNotifyProc (
    byte                    connType,
    FAsyncNotifySocketProc  notifyProc,
    unsigned                buildId = 0,
    unsigned                buildType = 0,
    unsigned				branchId = 0,
    const Uuid &            productId = kNilGuid
);

void AsyncSocketUnregisterNotifyProc (
    byte                    connType,
    FAsyncNotifySocketProc  notifyProc,
    unsigned                buildId = 0,
    unsigned                buildType = 0,
    unsigned				branchId = 0,
    const Uuid &            productId = kNilGuid
);

FAsyncNotifySocketProc AsyncSocketFindNotifyProc (
    const byte              buffer[],
    unsigned                bytes,
    unsigned *              bytesProcessed,
    unsigned *              connType,
    unsigned *              buildId,
    unsigned *              buildType,
    unsigned *				branchId,
    Uuid *                  productId
);


/****************************************************************************
*
*   Socket functions
*
***/

void AsyncSocketConnect (
    AsyncCancelId *         cancelId,
    const NetAddress &      netAddr,
    FAsyncNotifySocketProc  notifyProc,
    void *                  param = nil,
    const void *            sendData = nil,
    unsigned                sendBytes = 0,
    unsigned                connectMs = 0,      // 0 => use default value
    unsigned                localPort = 0       // 0 => don't bind local port
);

// Due to the asynchronous nature of sockets, the connect may complete
// before the cancel does... you have been warned.
void AsyncSocketConnectCancel (
    FAsyncNotifySocketProc  notifyProc,
    AsyncCancelId           cancelId
);

void AsyncSocketDisconnect (
    AsyncSocket             sock,
    bool                    hardClose
);

// This function must only be called after receiving a kNotifySocketDisconnect
void AsyncSocketDelete (AsyncSocket sock);

// Returns false of socket has been closed
bool AsyncSocketSend (
    AsyncSocket             sock,
    const void *            data,
    unsigned                bytes
);

// Buffer must stay valid until I/O has completed
// Returns false if socket has been closed
bool AsyncSocketWrite (
    AsyncSocket             sock,
    const void *            buffer,
    unsigned                bytes,
    void *                  param
);

// This function must only be called from with a socket notification callback.
// Calling at any other time is a crash bug waiting to happen!
void AsyncSocketSetNotifyProc (
    AsyncSocket             sock,
    FAsyncNotifySocketProc  notifyProc
);

// A backlog of zero (the default) means that no buffering is performed when
// the TCP send buffer is full, and the send() function will close the socket
// on send fail
void AsyncSocketSetBacklogAlloc (
    AsyncSocket             sock,
    unsigned                bufferSize
);

// On failure, returns 0
// On success, returns bound port (if port number was zero, returns assigned port)
// For connections that will use kConnType* connections, set notifyProc = nil;
// the handler will be found when connection packet is received.
// for connections with hard-coded behavior, set the notifyProc here (e.g. for use
// protocols like SNMP on port 25)
unsigned AsyncSocketStartListening (
    const NetAddress &      listenAddr,
    FAsyncNotifySocketProc  notifyProc = nil
);
void AsyncSocketStopListening (
    const NetAddress &      listenAddr,
    FAsyncNotifySocketProc  notifyProc = nil
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

typedef void (* FAsyncLookupProc) (
    void *              param,
    const wchar         name[],
    unsigned            addrCount,
    const NetAddress    addrs[]
);

void AsyncAddressLookupName (
    AsyncCancelId *     cancelId,
    FAsyncLookupProc    lookupProc,
    const wchar         name[],
    unsigned            port,
    void *              param
);

void AsyncAddressLookupAddr (
    AsyncCancelId *     cancelId,
    FAsyncLookupProc    lookupProc,
    const NetAddress &  address,
    void *              param
);

void AsyncAddressLookupCancel (
    FAsyncLookupProc    lookupProc,
    AsyncCancelId       cancelId
);
