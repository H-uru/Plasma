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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetCli/pnNcCli.cpp
*   
***/

#include "Pch.h"
#pragma hdrstop

//#define NCCLI_DEBUGGING
#ifdef NCCLI_DEBUGGING
# pragma message("Compiling pnNetCli with debugging on")
# define NCCLI_LOG  LogMsg
#else
# define NCCLI_LOG  NULL_STMT
#endif

#ifndef PLASMA_EXTERNAL_RELEASE

struct NetLogMessage_Header
{
    unsigned    m_protocol;
    int         m_direction;
    unsigned    m_time;
    unsigned    m_size;
};

#define HURU_PIPE_NAME "\\\\.\\pipe\\H-Uru_NetLog"

static CRITICAL_SECTION s_pipeCritical;
static HANDLE           s_netlog = 0;
static ULARGE_INTEGER   s_timeOffset;

static unsigned GetAdjustedTimer()
{
    FILETIME time;
    ULARGE_INTEGER maths;
    GetSystemTimeAsFileTime(&time);
    maths.HighPart = time.dwHighDateTime;
    maths.LowPart = time.dwLowDateTime;
    maths.QuadPart -= s_timeOffset.QuadPart;
    return maths.LowPart % 864000000;
}

#endif // PLASMA_EXTERNAL_RELEASE

namespace pnNetCli {

/*****************************************************************************
*
*   Private types and constants
*
***/

enum ENetCliMode {
    kNetCliModeServerStart,
    kNetCliModeClientStart,
    kNetCliModeEncrypted,
    kNumNetCliModes
};

} using namespace pnNetCli;


/*****************************************************************************
*
*   Opaque types
*
***/

// connection structure attached to each socket
struct NetCli : THashKeyVal<Uuid> {

    // communication channel
    AsyncSocket             sock;
    ENetProtocol            protocol;
    NetMsgChannel *         channel;
    bool                    server;

    // message queue    
    LINK(NetCli)            link;
    NetCliQueue *           queue;

    // message send/recv
    const NetMsgInitRecv *  recvMsg;
    const NetMsgField *     recvField;
    unsigned                recvFieldBytes;
    bool                    recvDispatch;
    byte *                  sendCurr;       // points into sendBuffer
    CInputAccumulator       input;

    // Message encryption
    ENetCliMode             mode;
    FNetCliEncrypt          encryptFcn;
    byte                    seed[kNetMaxSymmetricSeedBytes];
    CryptKey *              cryptIn; // nil if encrytpion is disabled
    CryptKey *              cryptOut; // nil if encrytpion is disabled
    void *                  encryptParam;

    // Message buffers
    byte                    sendBuffer[kAsyncSocketBufferSize];
    ARRAY(byte)             recvBuffer;
};

struct NetCliQueue {
    LISTDECL(NetCli, link)      list;
    unsigned                    lastSendMs;
    unsigned                    flushTimeMs;
};


namespace pnNetCli {

/*****************************************************************************
*
*   Private data
*
***/

 

/*****************************************************************************
*
*   Internal functions
*
***/

//============================================================================
static void PutBufferOnWire (NetCli * cli, void * data, unsigned bytes) {

        byte * temp, * heap = NULL;

#ifndef PLASMA_EXTERNAL_RELEASE
    // Write to the netlog
    if (s_netlog) {
        NetLogMessage_Header header;
        header.m_protocol = cli->protocol;
        header.m_direction = 0; // kCli2Srv
        header.m_time = GetAdjustedTimer();
        header.m_size = bytes;

        EnterCriticalSection(&s_pipeCritical);
        DWORD bytesWritten;
        WriteFile(s_netlog, &header, sizeof(header), &bytesWritten, NULL);
        WriteFile(s_netlog, data, bytes, &bytesWritten, NULL);
        LeaveCriticalSection(&s_pipeCritical);
    }
#endif // PLASMA_EXTERNAL_RELEASE

    if (cli->mode == kNetCliModeEncrypted && cli->cryptOut) {
        // Encrypt data...
        if (bytes <= 2048)
            // byte count is small, use stack-based buffer
            temp = ALLOCA(byte, bytes);
        else
            // byte count is large, use heap-based buffer
            temp = heap = (byte *)ALLOC(bytes);

        MemCopy(temp, data, bytes);
        CryptEncrypt(cli->cryptOut, bytes, temp);
        data = temp;
    }
    if (cli->sock)
        AsyncSocketSend(cli->sock, data, bytes);
        
    // free heap buffer (if any)
    FREE(heap);
}

//============================================================================
static void FlushSendBuffer (NetCli * cli) {
    const unsigned bytes = cli->sendCurr - cli->sendBuffer;
    ASSERT(bytes <= arrsize(cli->sendBuffer));
    PutBufferOnWire(cli, cli->sendBuffer, bytes);
    cli->sendCurr = cli->sendBuffer;
}

//===========================================================================
static void AddToSendBuffer (
    NetCli *            cli,
    unsigned            bytes,
    void const * const  data
) {
    byte const * src = (byte const *) data;

    if (bytes > arrsize(cli->sendBuffer)) {
        // Let the OS fragment oversize buffers
        FlushSendBuffer(cli);
        void * heap = ALLOC(bytes);
        MemCopy(heap, data, bytes);
        PutBufferOnWire(cli, heap, bytes);
        FREE(heap);
    }
    else {
        for (;;) {
            // calculate the space left in the output buffer and use it
            // to determine the maximum number of bytes that will fit
            unsigned const left = &cli->sendBuffer[arrsize(cli->sendBuffer)] - cli->sendCurr;
            unsigned const copy = min(bytes, left);

            // copy the data into the buffer
            for (unsigned i = 0; i < copy; ++i)
                cli->sendCurr[i] = src[i];
            cli->sendCurr += copy;
            ASSERT(cli->sendCurr - cli->sendBuffer <= sizeof(cli->sendBuffer));

            // if we copied all the data then bail
            if (copy < left)
                break;

            src   += copy;
            bytes -= copy;

            FlushSendBuffer(cli);
        }
    }
}

//============================================================================
static void BufferedSendData (
    NetCli *            cli,
    const unsigned_ptr  msg[], 
    unsigned            fieldCount
) {
    #define ASSERT_MSG_VALID(expr)          \
        ASSERTMSG(expr, "Invalid message definition");

    ASSERT(cli);
    ASSERT(msg);
    ASSERT(fieldCount);

    if (!cli->sock)
        return;

    unsigned_ptr const * const msgEnd = msg + fieldCount;

    const NetMsgInitSend * sendMsg = NetMsgChannelFindSendMessage(cli->channel, msg[0]);
    ASSERT(msg[0] == sendMsg->msg.messageId);
    ASSERT(fieldCount-1 == sendMsg->msg.count);

    // insert messageId into command stream
    const word msgId = hsToLE16((word)msg[0]);
    AddToSendBuffer(cli, sizeof(word), (const void*)&msgId);
    ++msg;
    ASSERT_MSG_VALID(msg < msgEnd);

    // insert fields into command stream
    dword varCount  = 0;
    dword varSize   = 0;
    const NetMsgField * cmd     = sendMsg->msg.fields;
    const NetMsgField * cmdEnd  = cmd + sendMsg->msg.count;
    for (; cmd < cmdEnd; ++msg, ++cmd) {
        switch (cmd->type) {
            case kNetMsgFieldInteger: {
                const unsigned count = cmd->count ? cmd->count : 1;
                const unsigned bytes = cmd->size * count;
                void * temp = ALLOCA(byte, bytes);
                
                if (count == 1)
                {
                    // Single values are passed by value
                    if (cmd->size == sizeof(byte)) {
                        *(byte*)temp = *(byte*)msg;
                    } else if (cmd->size == sizeof(word)) {
                        *(word*)temp = hsToLE16(*(word*)msg);
                    } else if (cmd->size == sizeof(dword)) {
                        *(dword*)temp = hsToLE32(*(dword*)msg);
                    } else if (cmd->size == sizeof(qword)) {
                        *(qword*)temp = hsToLE64(*(qword*)msg);
                    }
                }
                else
                {
                    // Value arrays are passed in by ptr
                    for (int i = 0; i < count; i++) {
                        if (cmd->size == sizeof(byte)) {
                            ((byte*)temp)[i] = ((byte*)*msg)[i];
                        } else if (cmd->size == sizeof(word)) {
                            ((word*)temp)[i] = hsToLE16(((word*)*msg)[i]);
                        } else if (cmd->size == sizeof(dword)) {
                            ((dword*)temp)[i] = hsToLE32(((dword*)*msg)[i]);
                        } else if (cmd->size == sizeof(qword)) {
                            ((qword*)temp)[i] = hsToLE64(((qword*)*msg)[i]);
                        }
                    }
                }
                
                // Write values to send buffer
                AddToSendBuffer(cli, bytes, temp);
            }
            break;

            case kNetMsgFieldReal: {
                const unsigned count = cmd->count ? cmd->count : 1;
                const unsigned bytes = cmd->size * count;
                
                if (count == 1)
                    // Single values are passed in by value
                    AddToSendBuffer(cli, bytes, (const void *) msg);
                else
                    // Value arrays are passed in by ptr
                    AddToSendBuffer(cli, bytes, (const void *) *msg);
            }
            break;

            case kNetMsgFieldString: {
                // Use less-than instead of less-or-equal because
                // we reserve one space for the NULL terminator
                const word length = (word) StrLen((const wchar *) *msg);
                ASSERT_MSG_VALID(length < cmd->count);
                // Write actual string length
                word size = hsToLE16(length);
                AddToSendBuffer(cli, sizeof(word), (const void*)&size);
                // Write string data
                AddToSendBuffer(cli, length * sizeof(wchar), (const void *) *msg);
            }
            break;

            case kNetMsgFieldData:
            case kNetMsgFieldRawData: {
                // write values to send buffer
                AddToSendBuffer(cli, cmd->count * cmd->size, (const void *) *msg);
            }
            break;

            case kNetMsgFieldVarCount: {
                ASSERT(!varCount);
                ASSERT(!varSize);
                // remember the element size
                varSize  = cmd->size;
                // write the actual element count
                varCount = hsToLE32((dword)*msg);
                AddToSendBuffer(cli, sizeof(dword), (const void*)&varCount);
            }
            break;

            case kNetMsgFieldVarPtr:
            case kNetMsgFieldRawVarPtr: {
                ASSERT(varSize);
                // write var sized array
                AddToSendBuffer(cli, varCount * varSize, (const void *) *msg);
                varCount    = 0;
                varSize     = 0;
            }
            break;

            case kNetMsgFieldPtr:
            case kNetMsgFieldRawPtr: {
                // write values
                AddToSendBuffer(cli, cmd->count * cmd->size, (const void *) *msg);
            }
            break;

            DEFAULT_FATAL(cmd->type);
        }
    }

    // prepare to flush this connection
    if (cli->queue)
        cli->queue->list.Link(cli);
}

//===========================================================================
static bool DispatchData (NetCli * cli, void * param) {

    word msgId = 0;
    while (!cli->input.Eof()) {
        // if we're not already decompressing a message, start new message
        if (!cli->recvMsg) {
            // get next message id
            if (!cli->input.Get(sizeof(msgId), &msgId))
                goto NEED_MORE_DATA;

            msgId = hsToLE16(msgId);

            if (nil == (cli->recvMsg = NetMsgChannelFindRecvMessage(cli->channel, msgId)))
                goto ERR_NO_HANDLER;

            // prepare to start decompressing new fields
            ASSERT(!cli->recvField);
            ASSERT(!cli->recvFieldBytes);
            cli->recvField = cli->recvMsg->msg.fields;
            cli->recvBuffer.ZeroCount();
            cli->recvBuffer.Reserve(kAsyncSocketBufferSize);

            // store the message id as dword into the destination buffer
            dword * recvMsgId = (dword *) cli->recvBuffer.New(sizeof(dword));
            *recvMsgId = msgId;
        }

        for (
            const NetMsgField * end = cli->recvMsg->msg.fields + cli->recvMsg->msg.count;
            cli->recvField < end;
            ++cli->recvField
        ) {
            switch (cli->recvField->type) {
                case kNetMsgFieldInteger: {
                    const unsigned count
                        = cli->recvField->count
                        ? cli->recvField->count
                        : 1;

                    // Get integer values
                    const unsigned bytes = count * cli->recvField->size;
                    byte * data = cli->recvBuffer.New(bytes);
                    if (!cli->input.Get(bytes, data)) {
                        cli->recvBuffer.ShrinkBy(bytes);
                        goto NEED_MORE_DATA;
                    }

                    // Byte-swap integers
                    // This is so screwed up >.<
                    for (int i = 0; i < count; i++) {
                        if (cli->recvField->size == sizeof(word)) {
                            ((word*)data)[i] = hsToLE16(((word*)data)[i]);
                        } else if (cli->recvField->size == sizeof(dword)) {
                            ((dword*)data)[i] = hsToLE32(((dword*)data)[i]);
                        } else if (cli->recvField->size == sizeof(qword)) {
                            ((qword*)data)[i] = hsToLE64(((qword*)data)[i]);
                        }
                    }

                    // Field complete
                }
                break;

                case kNetMsgFieldReal: {
                    const unsigned count
                        = cli->recvField->count
                        ? cli->recvField->count
                        : 1;

                    // Get float values
                    const unsigned bytes = count * cli->recvField->size;
                    byte * data = cli->recvBuffer.New(bytes);
                    if (!cli->input.Get(bytes, data)) {
                        cli->recvBuffer.ShrinkBy(bytes);
                        goto NEED_MORE_DATA;
                    }

                    // Field complete
                }
                break;

                case kNetMsgFieldData:
                case kNetMsgFieldRawData: {
                    // Read fixed-length data into destination buffer
                    const unsigned bytes = cli->recvField->count * cli->recvField->size;
                    byte * data = cli->recvBuffer.New(bytes);
                    if (!cli->input.Get(bytes, data)) {
                        cli->recvBuffer.ShrinkBy(bytes);
                        goto NEED_MORE_DATA;
                    }

                    // Field complete
                }
                break;

                case kNetMsgFieldVarCount: {
                    // Read var count field into destination buffer
                    const unsigned bytes = sizeof(dword);
                    byte * data = cli->recvBuffer.New(bytes);
                    if (!cli->input.Get(bytes, data)) {
                        cli->recvBuffer.ShrinkBy(bytes);
                        goto NEED_MORE_DATA;
                    }

                    // Byte-swap value
                    dword val = hsToLE32(*(dword*)data);

                    // Prepare to read var-length field
                    cli->recvFieldBytes = val * cli->recvField->size;

                    // Field complete
                }
                break;

                case kNetMsgFieldVarPtr:
                case kNetMsgFieldRawVarPtr: {
                    // Read var-length data into destination buffer
                    const unsigned bytes = cli->recvFieldBytes;
                    byte * data = cli->recvBuffer.New(bytes);
                    if (!cli->input.Get(bytes, data)) {
                        cli->recvBuffer.ShrinkBy(bytes);
                        goto NEED_MORE_DATA;
                    }

                    // Field complete
                    cli->recvFieldBytes = 0;
                }
                break;

                case kNetMsgFieldString: {
                    if (!cli->recvFieldBytes) {
                        // Read string length
                        word length;
                        if (!cli->input.Get(sizeof(word), &length))
                            goto NEED_MORE_DATA;
                        cli->recvFieldBytes = hsToLE16(length) * sizeof(wchar);

                        // Validate size. Use >= instead of > to leave room for the NULL terminator.
                        if (cli->recvFieldBytes >= cli->recvField->count * cli->recvField->size)
                            goto ERR_BAD_COUNT;
                    }

                    const unsigned bytes = cli->recvField->count * cli->recvField->size;
                    byte * data = cli->recvBuffer.New(bytes);
                    // Read compressed string data (less than full field length)
                    if (!cli->input.Get(cli->recvFieldBytes, data)) {
                        cli->recvBuffer.ShrinkBy(bytes);
                        goto NEED_MORE_DATA;
                    }

                    // Insert NULL terminator
                    * (wchar *)(data + cli->recvFieldBytes) = 0;

                    // IDEA: fill the remainder with a freaky byte pattern

                    // Field complete
                    cli->recvFieldBytes = 0;
                }
                break;
            }
        }

        // dispatch message to handler function
        NCCLI_LOG(kLogPerf, L"pnNetCli: Dispatching. msg: %S. cli: %p", cli->recvMsg ? cli->recvMsg->msg.name : "(unknown)", cli);
        if (!cli->recvMsg->recv(cli->recvBuffer.Ptr(), cli->recvBuffer.Count(), param))
            goto ERR_DISPATCH_FAILED;
        
        // prepare to start next message
        cli->recvMsg        = nil;
        cli->recvField      = 0;
        cli->recvFieldBytes = 0;

        // Release oversize message buffer
        if (cli->recvBuffer.Count() > kAsyncSocketBufferSize)
            cli->recvBuffer.Clear();
    }

    return true;

// these are used for convenience in setting breakpoints
NEED_MORE_DATA:
    NCCLI_LOG(kLogPerf, L"pnNetCli: NEED_MORE_DATA. msg: %S (%u). cli: %p", cli->recvMsg ? cli->recvMsg->msg.name : "(unknown)", msgId, cli);
    return true;

ERR_BAD_COUNT:
    LogMsg(kLogError, L"pnNetCli: ERR_BAD_COUNT. msg: %S (%u). cli: %p", cli->recvMsg ? cli->recvMsg->msg.name : "(unknown)", msgId, cli);
    return false;

ERR_NO_HANDLER:
    LogMsg(kLogError, L"pnNetCli: ERR_NO_HANDLER. msg: %S (%u). cli: %p", cli->recvMsg ? cli->recvMsg->msg.name : "(unknown)", msgId, cli);
    return false;

ERR_DISPATCH_FAILED:
    LogMsg(kLogError, L"pnNetCli: ERR_DISPATCH_FAILED. msg: %S (%u). cli: %p", cli->recvMsg ? cli->recvMsg->msg.name : "(unknown)", msgId, cli);
    return false;
}


namespace Connect {
/*****************************************************************************
*
*   NetCli connect protocol
*
***/

#include <PshPack1.h>
enum {
    kNetCliCli2SrvConnect,
    kNetCliSrv2CliEncrypt,
    kNetCliSrv2CliError,
    kNumNetCliMsgs
};

struct NetCli_PacketHeader {
    byte    message;
    byte    length;
};

struct NetCli_Cli2Srv_Connect : NetCli_PacketHeader {
    byte    dh_y_data[kNetDiffieHellmanKeyBits / 8];
};

struct NetCli_Srv2Cli_Encrypt : NetCli_PacketHeader {
    byte    serverSeed[kNetMaxSymmetricSeedBytes];
};

struct NetCli_Srv2Cli_Error : NetCli_PacketHeader {
    dword   error;              // ENetError
};
#include <PopPack.h>


//===========================================================================
static void CreateSymmetricKey (
    unsigned        serverBytes,
    const byte *    serverSeed,
    unsigned        clientBytes,
    const byte *    clientSeed,
    unsigned        outputBytes,
    byte *          outputSeed
) {
    ASSERT(clientBytes == kNetMaxSymmetricSeedBytes);
    ASSERT(serverBytes == kNetMaxSymmetricSeedBytes);
    ASSERT(outputBytes == kNetMaxSymmetricSeedBytes);
    for (unsigned i = 0; i < outputBytes; ++i)
        outputSeed[i] = (byte) (clientSeed[i] ^ serverSeed[i]);
}

//============================================================================
static void ClientConnect (NetCli * cli) {

    // Initiate diffie-hellman for client
    BigNum clientSeed;
    BigNum serverSeed;
    NetMsgCryptClientStart(
        cli->channel,
        sizeof(cli->seed),
        cli->seed,
        &clientSeed,
        &serverSeed
    );

    // Save client seed
    {
        ZERO(cli->seed);
        unsigned bytes;
        unsigned char * data = clientSeed.GetData_LE(&bytes);
        MemCopy(cli->seed, data, min(bytes, sizeof(cli->seed)));
        delete [] data;
    }

    // Send server seed
    if (cli->sock) {
        unsigned bytes;
        NetCli_Cli2Srv_Connect msg;
        unsigned char * data = serverSeed.GetData_LE(&bytes); // will be 0 if encryption is disabled, and thereby send an empty seed
        ASSERTMSG(bytes <= sizeof(msg.dh_y_data), "4");
        msg.message    = kNetCliCli2SrvConnect;
        msg.length     = (byte) (sizeof(msg) - sizeof(msg.dh_y_data) +  bytes);
        MemCopy(msg.dh_y_data, data, bytes);
        AsyncSocketSend(cli->sock, &msg, msg.length);
        delete [] data;
    }
}

//============================================================================
static bool ServerRecvConnect (
    NetCli *                    cli,
    const NetCli_PacketHeader & pkt
) {
    // Validate connection state
    if (cli->mode != kNetCliModeServerStart)
        return false;

    // Validate message size
    const NetCli_Cli2Srv_Connect & msg =
        * (const NetCli_Cli2Srv_Connect *) &pkt;
    if (pkt.length < sizeof(msg))
        return false;
    int seedLength = msg.length - sizeof(pkt);

    // Send the server seed to the client (unencrypted)
    if (cli->sock) {
        NetCli_Srv2Cli_Encrypt reply;
        reply.message   = kNetCliSrv2CliEncrypt;
        reply.length    = seedLength == 0 ? 0 : sizeof(reply); // reply with empty seed if we got empty seed (this means: no encryption)
        MemCopy(reply.serverSeed, cli->seed, sizeof(reply.serverSeed));
        AsyncSocketSend(cli->sock, &reply, reply.length);
    }

    if (seedLength == 0) { // client wishes no encryption (that's okay, nobody else can "fake" us as nobody has the private key, so if the client actually wants encryption it will only work with the correct peer)
        cli->cryptIn = nil;
        cli->cryptOut = nil;
    }
    else {
        // Compute client seed
        byte clientSeed[kNetMaxSymmetricSeedBytes];
        BigNum clientSeedValue;
        {
            NetMsgCryptServerConnect(
                cli->channel,
                seedLength,
                msg.dh_y_data,
                &clientSeedValue
            );

            ZERO(clientSeed);
            unsigned bytes;
            unsigned char * data = clientSeedValue.GetData_LE(&bytes);
            MemCopy(clientSeed, data, min(bytes, sizeof(clientSeed)));
            delete [] data;
        }

        // Create the symmetric key from a combination
        // of the client seed and the server seed
        byte sharedSeed[kNetMaxSymmetricSeedBytes];
        CreateSymmetricKey(
            sizeof(cli->seed),  cli->seed,  // server seed
            sizeof(clientSeed), clientSeed, // client seed
            sizeof(sharedSeed), sharedSeed  // combined seed
        );

        // Switch to encrypted mode
        cli->cryptIn  = CryptKeyCreate(kCryptRc4, sizeof(sharedSeed), sharedSeed);
        cli->cryptOut = CryptKeyCreate(kCryptRc4, sizeof(sharedSeed), sharedSeed);
    }
    
    cli->mode = kNetCliModeEncrypted; // should rather be called "established", but whatever
    return cli->encryptFcn(kNetSuccess, cli->encryptParam);
}

//============================================================================
static bool ClientRecvEncrypt (
    NetCli *                    cli,
    const NetCli_PacketHeader & pkt
) {
    // Validate connection state
    if (cli->mode != kNetCliModeClientStart)
        return false;

    // find out if we want encryption
    const BigNum * DH_N;
    NetMsgChannelGetDhConstants(cli->channel, nil, nil, &DH_N);
    bool encrypt = !DH_N->isZero();

    // Process message
    const NetCli_Srv2Cli_Encrypt & msg =
        * (const NetCli_Srv2Cli_Encrypt *) &pkt;
    if (encrypt) { // we insist on encryption, don't let some MitM decide for us!
        if (pkt.length != sizeof(msg))
            return false;

        // Create the symmetric key from a combination
        // of the client seed and the server seed
        byte sharedSeed[kNetMaxSymmetricSeedBytes];
        CreateSymmetricKey(
            sizeof(msg.serverSeed), msg.serverSeed, // server seed
            sizeof(cli->seed),      cli->seed,      // client seed
            sizeof(sharedSeed),     sharedSeed      // combined seed
        );

        // Switch to encrypted mode
        cli->cryptIn  = CryptKeyCreate(kCryptRc4, sizeof(sharedSeed), sharedSeed);
        cli->cryptOut = CryptKeyCreate(kCryptRc4, sizeof(sharedSeed), sharedSeed);
    }
    else { // honestly we do not care what the other side sends, we will send plaintext
        if (pkt.length != sizeof(pkt))
            return false;
        cli->cryptIn = nil;
        cli->cryptOut = nil;
    }

    cli->mode = kNetCliModeEncrypted; // should rather be called "established", but whatever
    return cli->encryptFcn(kNetSuccess, cli->encryptParam);
}

//============================================================================
static bool ClientRecvError (
    NetCli *                    cli,
    const NetCli_PacketHeader & pkt
) {
    // Validate connection state
    if (cli->mode != kNetCliModeClientStart)
        return false;

    // Validate message size
    const NetCli_Srv2Cli_Error & msg =
        * (const NetCli_Srv2Cli_Error *) &pkt;
    if (pkt.length < sizeof(msg))
        return false;

    cli->encryptFcn((ENetError) msg.error, cli->encryptParam);
    return false;
}

//============================================================================
typedef bool (* FNetCliPacket)(
    NetCli *                    cli,
    const NetCli_PacketHeader & pkt
);

#if 0

#ifdef SERVER
static const FNetCliPacket s_recvTbl[kNumNetCliMsgs] = {
    ServerRecvConnect,
    nil,
    nil,
};
#endif

#ifdef CLIENT
static const FNetCliPacket s_recvTbl[kNumNetCliMsgs] = {
    nil,
    ClientRecvEncrypt,
    ClientRecvError,
};
#endif

#else // 0

static const FNetCliPacket s_recvTbl[kNumNetCliMsgs] = {
    ServerRecvConnect,
    ClientRecvEncrypt,
    ClientRecvError,
};

#endif // 0

//===========================================================================
static unsigned DispatchPacket (
    NetCli *        cli,
    unsigned        bytes,
    const byte      data[]
) {
    for (;;) {
        const NetCli_PacketHeader & pkt = * (const NetCli_PacketHeader *) data;
        if (bytes < sizeof(pkt))
            break;
        if (pkt.length > bytes)
            break;
        if (pkt.message >= kNumNetCliMsgs)
            break;
        if (!s_recvTbl[pkt.message])
            break;
        if (!s_recvTbl[pkt.message](cli, pkt))
            break;

        // Success!
        return pkt.length;
    }

    // Failure!
    return 0;
}

} // namespace Connect


/*****************************************************************************
*
*   NetCli implementation
*
***/

//===========================================================================
static void ResetSendRecv (NetCli * cli) {
    cli->recvMsg            = nil;
    cli->recvField          = nil;
    cli->recvFieldBytes     = 0;
    cli->recvDispatch       = true;
    cli->sendCurr           = cli->sendBuffer;
    cli->recvBuffer.Clear();
    cli->input.Clear();
}

//===========================================================================
static NetCli * ConnCreate (
    AsyncSocket     sock,
    unsigned        protocol,
    ENetCliMode     mode
) {
    // find channel
    unsigned largestRecv;
    NetMsgChannel * channel = NetMsgChannelLock(
        protocol,
        mode == kNetCliModeServerStart,
        &largestRecv
    );
    if (!channel)
        return nil;

    NetCli * const cli  = NEWZERO(NetCli);
    cli->sock           = sock;
    cli->protocol       = (ENetProtocol) protocol;
    cli->channel        = channel;
    cli->mode           = mode;
    cli->SetValue(kNilGuid);

#ifndef PLASMA_EXTERNAL_RELEASE
    // Network debug pipe
    if (!s_netlog) {
        InitializeCriticalSection(&s_pipeCritical);
        WaitNamedPipe(HURU_PIPE_NAME, NMPWAIT_WAIT_FOREVER);
        s_netlog = CreateFileA(
            HURU_PIPE_NAME,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

        // Not exactly the start, but close enough ;)
        FILETIME timeBase;
        GetSystemTimeAsFileTime(&timeBase);
        s_timeOffset.HighPart = timeBase.dwHighDateTime;
        s_timeOffset.LowPart = timeBase.dwLowDateTime;
    }
#endif // PLASMA_EXTERNAL_RELEASE

    ResetSendRecv(cli);

    return cli;
}

//===========================================================================
static void SetConnSeed (
    NetCli *        cli,
    unsigned        seedBytes,
    const byte      seedData[]
) {
    if (seedBytes)
        MemCopy(cli->seed, seedData, min(sizeof(cli->seed), seedBytes));
    else
        CryptCreateRandomSeed(sizeof(cli->seed), cli->seed);
}

} using namespace pnNetCli;


/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
NetCli * NetCliConnectAccept (
    AsyncSocket         sock,
    unsigned            protocol,
    bool                unbuffered,
    FNetCliEncrypt      encryptFcn,
    unsigned            seedBytes,
    const byte          seedData[],
    void *              encryptParam
) {
    // Create connection
    NetCli * cli = ConnCreate(sock, protocol, kNetCliModeClientStart);
    if (cli) {
        AsyncSocketEnableNagling(sock, !unbuffered);
        cli->encryptFcn     = encryptFcn;
        cli->encryptParam   = encryptParam;
        SetConnSeed(cli, seedBytes, seedData);
        Connect::ClientConnect(cli);
    }
    return cli;
}

//============================================================================
#ifdef SERVER
NetCli * NetCliListenAccept (
    AsyncSocket         sock,
    unsigned            protocol,
    bool                unbuffered,
    FNetCliEncrypt      encryptFcn,
    unsigned            seedBytes,
    const byte          seedData[],
    void *              encryptParam
) {
    // Create connection
    NetCli * cli = ConnCreate(sock, protocol, kNetCliModeServerStart);
    if (cli) {
        AsyncSocketEnableNagling(sock, !unbuffered);
        cli->encryptFcn     = encryptFcn;
        cli->encryptParam   = encryptParam;
        SetConnSeed(cli, seedBytes, seedData);
    }
    return cli;
}
#endif

//============================================================================
#ifdef SERVER
void NetCliListenReject (
    AsyncSocket     sock,
    ENetError       error
) {
    if (sock) {
        Connect::NetCli_Srv2Cli_Error response;
        response.message    = Connect::kNetCliSrv2CliError;
        response.length     = sizeof(response);
        response.error      = error;
        AsyncSocketSend(sock, &response, sizeof(response));
    }
}
#endif

//============================================================================
void NetCliClearSocket (NetCli * cli) {
    cli->sock = nil;
}

//============================================================================
void NetCliSetQueue (
    NetCli *        cli,
    NetCliQueue *   queue
) {
    cli->queue = queue;
}

//============================================================================
void NetCliDisconnect (
    NetCli *        cli,
    bool            hardClose
) {
    // send any existing messages and allow
    // the socket layer to complete sending data
    if (!hardClose)
        NetCliFlush(cli);

    if (cli->sock)
        AsyncSocketDisconnect(cli->sock, hardClose);

    // don't allow any more messages to be received
    cli->recvDispatch = false;
}

//============================================================================
void NetCliDelete (
    NetCli *        cli,
    bool            deleteSocket
) {
    NetMsgChannelUnlock(cli->channel);

    if (cli->sock && deleteSocket)
        AsyncSocketDelete(cli->sock);

    if (cli->cryptIn)
        CryptKeyClose(cli->cryptIn);
    if (cli->cryptOut)
        CryptKeyClose(cli->cryptOut);

    cli->input.Clear();
    cli->recvBuffer.Clear();

    DEL(cli);
}

//============================================================================
void NetCliFlush (
    NetCli *        cli
) {
    if (cli->sendCurr != cli->sendBuffer)
        FlushSendBuffer(cli);
}

//============================================================================
void NetCliSend (
    NetCli *            cli,
    const unsigned_ptr  msg[], 
    unsigned            count
) {
    BufferedSendData(cli, msg, count);
}

//============================================================================
bool NetCliDispatch (
    NetCli *        cli,
    const byte      data[],
    unsigned        bytes,
    void *          param
) {
    if (!cli->recvDispatch)
        return false;

    do {
        if (cli->mode == kNetCliModeEncrypted) {
            // Decrypt data...
            byte * temp, * heap = NULL;

            if (cli->cryptIn) {
                if (bytes <= 2048)
                    // byte count is small, use stack-based buffer
                    temp = ALLOCA(byte, bytes);
                else
                    // byte count is large, use heap-based buffer
                    temp = heap = (byte *)ALLOC(bytes);

                MemCopy(temp, data, bytes);
                CryptDecrypt(cli->cryptIn, bytes, temp);
                data = temp;
            }

            // Add data to accumulator and dispatch
            cli->input.Add(bytes, data);
            bool result = DispatchData(cli, param);

#ifndef PLASMA_EXTERNAL_RELEASE
            // Write to the netlog
            if (s_netlog) {
                NetLogMessage_Header header;
                header.m_protocol = cli->protocol;
                header.m_direction = 1; // kSrv2Cli
                header.m_time = GetAdjustedTimer();
                header.m_size = bytes;

                EnterCriticalSection(&s_pipeCritical);
                DWORD bytesWritten;
                WriteFile(s_netlog, &header, sizeof(header), &bytesWritten, NULL);
                WriteFile(s_netlog, data, bytes, &bytesWritten, NULL);
                LeaveCriticalSection(&s_pipeCritical);
            }
#endif // PLASMA_EXTERNAL_RELEASE

#ifdef SERVER
            cli->recvDispatch = result;
#endif
            
            // free heap buffer (if any)
            FREE(heap);

            cli->input.Compact();
            return cli->recvDispatch;
        }

        // Dispatch connect packets until encryption starts
        unsigned used = Connect::DispatchPacket(cli, bytes, data);
        if (!used)
            return false;

        data  += used;
        bytes -= used;

    } while (bytes);

    return true;
}
