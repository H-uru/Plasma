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

#include "hsWindows.h"
#include "pnEncryption/plChallengeHash.h"
#include "pnEncryption/plEncryption.h"
#include "pnUUID/pnUUID.h"
#include "hsLockGuard.h"
#include <mutex>
#include <string>

#ifdef HS_DEBUGGING
# define NCCLI_LOG  LogMsg
#else
# define NCCLI_LOG(...) ((void)0)
#endif

#if !defined(PLASMA_EXTERNAL_RELEASE) && defined(HS_BUILD_FOR_WIN32)

struct NetLogMessage_Header
{
    unsigned    m_protocol;
    int         m_direction;
    unsigned    m_time;
    unsigned    m_size;
};

#define HURU_PIPE_NAME L"\\\\.\\pipe\\H-Uru_NetLog"

static std::recursive_mutex s_pipeCritical;
static HANDLE               s_netlog = nullptr;
static ULARGE_INTEGER       s_timeOffset;

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
struct NetCli {

    // communication channel
    AsyncSocket             sock;
    ENetProtocol            protocol;
    NetMsgChannel *         channel;
    bool                    server;

    // message send/recv
    const NetMsgInitRecv *  recvMsg;
    const NetMsgField *     recvField;
    unsigned                recvFieldBytes;
    bool                    recvDispatch;
    uint8_t *               sendCurr;       // points into sendBuffer
    CInputAccumulator       input;

    // Message encryption
    ENetCliMode             mode;
    FNetCliEncrypt          encryptFcn;
    uint8_t                 seed[kNetMaxSymmetricSeedBytes];
    CryptKey *              cryptIn; // nil if encrytpion is disabled
    CryptKey *              cryptOut; // nil if encrytpion is disabled
    void *                  encryptParam;

    // Message buffers
    uint8_t                    sendBuffer[kAsyncSocketBufferSize];
    std::vector<uint8_t>       recvBuffer;

    NetCli()
        : sock(), protocol(), channel(), server(), recvMsg()
        , recvField(), recvFieldBytes(), recvDispatch(), sendCurr(), mode()
        , encryptFcn(), seed(), cryptIn(), cryptOut(), encryptParam()
        , sendBuffer()
    {
    }
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

    uint8_t * temp = nullptr;

#if !defined(PLASMA_EXTERNAL_RELEASE) && defined(HS_BUILD_FOR_WIN32)
    // Write to the netlog
    if (s_netlog) {
        NetLogMessage_Header header;
        header.m_protocol = cli->protocol;
        header.m_direction = 0; // kCli2Srv
        header.m_time = GetAdjustedTimer();
        header.m_size = bytes;

        hsLockGuard(s_pipeCritical);
        DWORD bytesWritten;
        WriteFile(s_netlog, &header, sizeof(header), &bytesWritten, nullptr);
        WriteFile(s_netlog, data, bytes, &bytesWritten, nullptr);
    }
#endif // PLASMA_EXTERNAL_RELEASE

    if (cli->mode == kNetCliModeEncrypted && cli->cryptOut) {
        temp = (uint8_t *)malloc(bytes);

        memcpy(temp, data, bytes);
        CryptEncrypt(cli->cryptOut, bytes, temp);
        data = temp;
    }
    if (cli->sock)
        AsyncSocketSend(cli->sock, data, bytes);
        
    // free heap buffer (if any)
    free(temp);
}

//============================================================================
static void FlushSendBuffer (NetCli * cli) {
    const unsigned bytes = (unsigned)(cli->sendCurr - cli->sendBuffer);
    ASSERT(bytes <= std::size(cli->sendBuffer));
    PutBufferOnWire(cli, cli->sendBuffer, bytes);
    cli->sendCurr = cli->sendBuffer;
}

//===========================================================================
static void AddToSendBuffer (
    NetCli *            cli,
    unsigned            bytes,
    void const * const  data
) {
    uint8_t const * src = (uint8_t const *) data;

    if (bytes > std::size(cli->sendBuffer)) {
        // Let the OS fragment oversize buffers
        FlushSendBuffer(cli);
        void * heap = malloc(bytes);
        memcpy(heap, data, bytes);
        PutBufferOnWire(cli, heap, bytes);
        free(heap);
    }
    else {
        for (;;) {
            // calculate the space left in the output buffer and use it
            // to determine the maximum number of bytes that will fit
            unsigned const left = (unsigned)(&cli->sendBuffer[std::size(cli->sendBuffer)] - cli->sendCurr);
            unsigned const copy = std::min(bytes, left);

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
    const uintptr_t  msg[], 
    unsigned            fieldCount
) {
    #define ASSERT_MSG_VALID(expr)          \
        ASSERTMSG(expr, "Invalid message definition");

    ASSERT(cli);
    ASSERT(msg);
    ASSERT(fieldCount);

    if (!cli->sock)
        return;

    uintptr_t const * const msgEnd = msg + fieldCount;

    const NetMsgInitSend * sendMsg = NetMsgChannelFindSendMessage(cli->channel, msg[0]);
    ASSERT(msg[0] == sendMsg->msg->messageId);
    ASSERT(fieldCount-1 == sendMsg->msg->count);

    // insert messageId into command stream
    const uint16_t msgId = hsToLE16((uint16_t)msg[0]);
    AddToSendBuffer(cli, sizeof(uint16_t), (const void*)&msgId);
    ++msg;
    ASSERT_MSG_VALID(msg < msgEnd);

    // insert fields into command stream
    uint32_t varCount  = 0;
    uint32_t varSize   = 0;
    const NetMsgField * cmd     = sendMsg->msg->fields;
    const NetMsgField * cmdEnd  = cmd + sendMsg->msg->count;
    for (; cmd < cmdEnd; ++msg, ++cmd) {
        switch (cmd->type) {
            case kNetMsgFieldInteger: {
                const unsigned count = cmd->count ? cmd->count : 1;
                const unsigned bytes = cmd->size * count;
                auto temp = std::make_unique<uint8_t[]>(bytes);

                if (count == 1) {
                    // Single values are passed by value
                    if (cmd->size == sizeof(uint8_t)) {
                        *(uint8_t*)&temp[0] = *(uint8_t*)msg;
                    } else if (cmd->size == sizeof(uint16_t)) {
                        *(uint16_t*)&temp[0] = hsToLE16(*(uint16_t*)msg);
                    } else if (cmd->size == sizeof(uint32_t)) {
                        *(uint32_t*)&temp[0] = hsToLE32(*(uint32_t*)msg);
                    } else if (cmd->size == sizeof(uint64_t)) {
                        *(uint64_t*)&temp[0] = hsToLE64(*(uint64_t*)msg);
                    }
                } else {
                    // Value arrays are passed in by ptr
                    for (size_t i = 0; i < count; i++) {
                        if (cmd->size == sizeof(uint8_t)) {
                            *(uint8_t*)&temp[i] = ((uint8_t*)*msg)[i];
                        } else if (cmd->size == sizeof(uint16_t)) {
                            *(uint16_t*)&temp[i] = hsToLE16(((uint16_t*)*msg)[i]);
                        } else if (cmd->size == sizeof(uint32_t)) {
                            *(uint32_t*)&temp[i] = hsToLE32(((uint32_t*)*msg)[i]);
                        } else if (cmd->size == sizeof(uint64_t)) {
                            *(uint64_t*)&temp[i] = hsToLE64(((uint64_t*)*msg)[i]);
                        }
                    }
                }

                // Write values to send buffer
                AddToSendBuffer(cli, bytes, temp.get());
            }
            break;

            case kNetMsgFieldReal: {
                const unsigned count = cmd->count ? cmd->count : 1;
                const unsigned bytes = cmd->size * count;
                auto temp = std::make_unique<uint8_t[]>(bytes);

                if (count == 1) {
                    // Single values are passed in by value
                    if (cmd->size == sizeof(float)) {
                        *(float*)&temp[0] = hsToLEFloat(*(float*)msg);
                    } else if (cmd->size == sizeof(double)) {
                        *(double*)&temp[0] = hsToLEFloat(*(double*)msg);
                    }
                } else {
                    // Value arrays are passed in by ptr
                    for (size_t i = 0; i < count; i++) {
                        if (cmd->size == sizeof(float)) {
                            *(float*)&temp[i] = hsToLEFloat(((float*)*msg)[i]);
                        } else if (cmd->size == sizeof(double)) {
                            *(double*)&temp[i] = hsToLEFloat(((double*)*msg)[i]);
                        }
                    }
                }

                // Write values to send buffer
                AddToSendBuffer(cli, bytes, temp.get());
            }
            break;

            case kNetMsgFieldString: {
                // Use less-than instead of less-or-equal because
                // we reserve one space for the NULL terminator
                const uint16_t length = (uint16_t) std::char_traits<char16_t>::length((const char16_t *) *msg);
                ASSERT_MSG_VALID(length < cmd->count);

                // Write actual string length
                uint16_t size = hsToLE16(length);
                AddToSendBuffer(cli, sizeof(uint16_t), (const void*)&size);

                auto temp = std::make_unique<char16_t[]>(length);
                for (size_t i = 0; i < length; i++) {
                    temp[i] = hsToLE16(((char16_t*)*msg)[i]);
                }

                // Write string data
                AddToSendBuffer(cli, length * sizeof(char16_t), temp.get());
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
                varCount = hsToLE32((uint32_t)*msg);
                AddToSendBuffer(cli, sizeof(uint32_t), (const void*)&varCount);
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
}

//===========================================================================
static bool DispatchData (NetCli * cli, void * param) {

    uint16_t msgId = 0;
    while (!cli->input.Eof()) {
        // if we're not already decompressing a message, start new message
        if (!cli->recvMsg) {
            // get next message id
            if (!cli->input.Get(sizeof(msgId), &msgId))
                goto NEED_MORE_DATA;

            msgId = hsToLE16(msgId);

            if (cli->recvMsg = NetMsgChannelFindRecvMessage(cli->channel, msgId); cli->recvMsg == nullptr)
                goto ERR_NO_HANDLER;

            // prepare to start decompressing new fields
            ASSERT(!cli->recvField);
            ASSERT(!cli->recvFieldBytes);
            cli->recvField = cli->recvMsg->msg->fields;
            cli->recvBuffer.clear();
            cli->recvBuffer.reserve(kAsyncSocketBufferSize);

            // store the message id as uint32_t into the destination buffer
            cli->recvBuffer.insert(cli->recvBuffer.end(), {
                (uint8_t)((msgId     ) & 0xFF),
                (uint8_t)((msgId >> 8) & 0xFF),
                0, 0
            });
        }

        for (
            const NetMsgField * end = cli->recvMsg->msg->fields + cli->recvMsg->msg->count;
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
                    const size_t oldSize = cli->recvBuffer.size();
                    cli->recvBuffer.resize(oldSize + bytes);
                    uint8_t * data = cli->recvBuffer.data() + oldSize;
                    if (!cli->input.Get(bytes, data)) {
                        cli->recvBuffer.resize(oldSize);
                        goto NEED_MORE_DATA;
                    }

                    // Convert to platform endianness
                    for (size_t i = 0; i < count; i++) {
                        if (cli->recvField->size == sizeof(uint16_t)) {
                            ((uint16_t*)data)[i] = hsToLE16(((uint16_t*)data)[i]);
                        } else if (cli->recvField->size == sizeof(uint32_t)) {
                            ((uint32_t*)data)[i] = hsToLE32(((uint32_t*)data)[i]);
                        } else if (cli->recvField->size == sizeof(uint64_t)) {
                            ((uint64_t*)data)[i] = hsToLE64(((uint64_t*)data)[i]);
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
                    const size_t oldSize = cli->recvBuffer.size();
                    cli->recvBuffer.resize(oldSize + bytes);
                    uint8_t * data = cli->recvBuffer.data() + oldSize;
                    if (!cli->input.Get(bytes, data)) {
                        cli->recvBuffer.resize(oldSize);
                        goto NEED_MORE_DATA;
                    }

                    // Convert to platform endianness
                    for (size_t i = 0; i < count; i++) {
                        if (cli->recvField->size == sizeof(float)) {
                            ((float*)data)[i] = hsToLEFloat(((float*)data)[i]);
                        } else if (cli->recvField->size == sizeof(double)) {
                            ((double*)data)[i] = hsToLEDouble(((double*)data)[i]);
                        }
                    }

                    // Field complete
                }
                break;

                case kNetMsgFieldData:
                case kNetMsgFieldRawData: {
                    // Read fixed-length data into destination buffer
                    const unsigned bytes = cli->recvField->count * cli->recvField->size;
                    const size_t oldSize = cli->recvBuffer.size();
                    cli->recvBuffer.resize(oldSize + bytes);
                    uint8_t * data = cli->recvBuffer.data() + oldSize;
                    if (!cli->input.Get(bytes, data)) {
                        cli->recvBuffer.resize(oldSize);
                        goto NEED_MORE_DATA;
                    }

                    // Field complete
                }
                break;

                case kNetMsgFieldVarCount: {
                    // Read var count field into destination buffer
                    const unsigned bytes = sizeof(uint32_t);
                    const size_t oldSize = cli->recvBuffer.size();
                    cli->recvBuffer.resize(oldSize + bytes);
                    uint8_t * data = cli->recvBuffer.data() + oldSize;
                    if (!cli->input.Get(bytes, data)) {
                        cli->recvBuffer.resize(oldSize);
                        goto NEED_MORE_DATA;
                    }

                    // byte-swap value
                    uint32_t val = hsToLE32(*(uint32_t*)data);

                    // Prepare to read var-length field
                    cli->recvFieldBytes = val * cli->recvField->size;

                    // Field complete
                }
                break;

                case kNetMsgFieldVarPtr:
                case kNetMsgFieldRawVarPtr: {
                    // Read var-length data into destination buffer
                    const unsigned bytes = cli->recvFieldBytes;
                    const size_t oldSize = cli->recvBuffer.size();
                    cli->recvBuffer.resize(oldSize + bytes);
                    uint8_t * data = cli->recvBuffer.data() + oldSize;
                    if (!cli->input.Get(bytes, data)) {
                        cli->recvBuffer.resize(oldSize);
                        goto NEED_MORE_DATA;
                    }

                    // Field complete
                    cli->recvFieldBytes = 0;
                }
                break;

                case kNetMsgFieldString: {
                    if (!cli->recvFieldBytes) {
                        // Read string length
                        uint16_t length;
                        if (!cli->input.Get(sizeof(uint16_t), &length))
                            goto NEED_MORE_DATA;
                        cli->recvFieldBytes = hsToLE16(length) * sizeof(char16_t);

                        // Validate size. Use >= instead of > to leave room for the NULL terminator.
                        if (cli->recvFieldBytes >= cli->recvField->count * cli->recvField->size)
                            goto ERR_BAD_COUNT;
                    }

                    const unsigned bytes = cli->recvField->count * cli->recvField->size;
                    const size_t oldSize = cli->recvBuffer.size();
                    cli->recvBuffer.resize(oldSize + bytes);
                    uint8_t * data = cli->recvBuffer.data() + oldSize;
                    // Read compressed string data (less than full field length)
                    if (!cli->input.Get(cli->recvFieldBytes, data)) {
                        cli->recvBuffer.resize(oldSize);
                        goto NEED_MORE_DATA;
                    }

                    // Convert to platform endianness
                    for (size_t i = 0; i < cli->recvField->count; i++) {
                        ((char16_t*)data)[i] = hsToLE16(((char16_t*)data)[i]);
                    }

                    // Insert NULL terminator
                    * (char16_t *)(data + cli->recvFieldBytes) = 0;

                    // IDEA: fill the remainder with a freaky uint8_t pattern

                    // Field complete
                    cli->recvFieldBytes = 0;
                }
                break;

                default: break;
            }
        }

        // dispatch message to handler function
        NCCLI_LOG(kLogPerf, "pnNetCli: Dispatching. msg: {}. cli: {#x}", cli->recvMsg ? cli->recvMsg->msg->name : "(unknown)", (uintptr_t)cli);
        if (!cli->recvMsg->recv(cli->recvBuffer.data(), cli->recvBuffer.size(), param))
            goto ERR_DISPATCH_FAILED;
        
        // prepare to start next message
        cli->recvMsg        = nullptr;
        cli->recvField      = nullptr;
        cli->recvFieldBytes = 0;

        // Release oversize message buffer
        if (cli->recvBuffer.size() > kAsyncSocketBufferSize)
            cli->recvBuffer.clear();
    }

    return true;

// these are used for convenience in setting breakpoints
NEED_MORE_DATA:
    NCCLI_LOG(kLogPerf, "pnNetCli: NEED_MORE_DATA. msg: {} ({}). cli: {#x}", cli->recvMsg ? cli->recvMsg->msg->name : "(unknown)", msgId, (uintptr_t)cli);
    return true;

ERR_BAD_COUNT:
    LogMsg(kLogError, "pnNetCli: ERR_BAD_COUNT. msg: {} ({}). cli: {#x}", cli->recvMsg ? cli->recvMsg->msg->name : "(unknown)", msgId, (uintptr_t)cli);
    return false;

ERR_NO_HANDLER:
    LogMsg(kLogError, "pnNetCli: ERR_NO_HANDLER. msg: {} ({}). cli: {#x}", cli->recvMsg ? cli->recvMsg->msg->name : "(unknown)", msgId, (uintptr_t)cli);
    return false;

ERR_DISPATCH_FAILED:
    LogMsg(kLogError, "pnNetCli: ERR_DISPATCH_FAILED. msg: {} ({}). cli: {#x}", cli->recvMsg ? cli->recvMsg->msg->name : "(unknown)", msgId, (uintptr_t)cli);
    return false;
}


namespace Connect {
/*****************************************************************************
*
*   NetCli connect protocol
*
***/

#pragma pack(push,1)
enum {
    kNetCliCli2SrvConnect,
    kNetCliSrv2CliEncrypt,
    kNetCliSrv2CliError,
    kNumNetCliMsgs
};

struct NetCli_PacketHeader {
    uint8_t    message;
    uint8_t    length;
};

struct NetCli_Cli2Srv_Connect : NetCli_PacketHeader {
    uint8_t    dh_y_data[kNetDiffieHellmanKeyBits / 8];
};

struct NetCli_Srv2Cli_Encrypt : NetCli_PacketHeader {
    uint8_t    serverSeed[kNetMaxSymmetricSeedBytes];
};

struct NetCli_Srv2Cli_Error : NetCli_PacketHeader {
    uint32_t   error;              // ENetError
};
#pragma pack(pop)


//===========================================================================
static void CreateSymmetricKey (
    unsigned        serverBytes,
    const uint8_t *    serverSeed,
    unsigned        clientBytes,
    const uint8_t *    clientSeed,
    unsigned        outputBytes,
    uint8_t *          outputSeed
) {
    ASSERT(clientBytes == kNetMaxSymmetricSeedBytes);
    ASSERT(serverBytes == kNetMaxSymmetricSeedBytes);
    ASSERT(outputBytes == kNetMaxSymmetricSeedBytes);
    for (unsigned i = 0; i < outputBytes; ++i)
        outputSeed[i] = (uint8_t) (clientSeed[i] ^ serverSeed[i]);
}

//============================================================================
static void ClientConnect (NetCli * cli) {

    // Initiate diffie-hellman for client
    plBigNum clientSeed;
    plBigNum serverSeed;
    NetMsgCryptClientStart(
        cli->channel,
        sizeof(cli->seed),
        cli->seed,
        &clientSeed,
        &serverSeed
    );

    // Save client seed
    {
        memset(&cli->seed, 0, sizeof(cli->seed));
        unsigned bytes;
        unsigned char * data = clientSeed.GetData_LE(&bytes);
        memcpy(cli->seed, data, std::min(size_t(bytes), sizeof(cli->seed)));
        delete [] data;
    }

    // Send server seed
    if (cli->sock) {
        unsigned bytes;
        NetCli_Cli2Srv_Connect msg;
        unsigned char * data = serverSeed.GetData_LE(&bytes); // will be 0 if encryption is disabled, and thereby send an empty seed
        ASSERTMSG(bytes <= sizeof(msg.dh_y_data), "4");
        msg.message    = kNetCliCli2SrvConnect;
        msg.length     = (uint8_t) (sizeof(msg) - sizeof(msg.dh_y_data) +  bytes);
        memcpy(msg.dh_y_data, data, bytes);
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
        memcpy(reply.serverSeed, cli->seed, sizeof(reply.serverSeed));
        AsyncSocketSend(cli->sock, &reply, reply.length);
    }

    if (seedLength == 0) { // client wishes no encryption (that's okay, nobody else can "fake" us as nobody has the private key, so if the client actually wants encryption it will only work with the correct peer)
        cli->cryptIn = nullptr;
        cli->cryptOut = nullptr;
    }
    else {
        // Compute client seed
        uint8_t clientSeed[kNetMaxSymmetricSeedBytes];
        plBigNum clientSeedValue;
        {
            NetMsgCryptServerConnect(
                cli->channel,
                seedLength,
                msg.dh_y_data,
                &clientSeedValue
            );

            memset(&clientSeed, 0, sizeof(clientSeed));
            unsigned bytes;
            unsigned char * data = clientSeedValue.GetData_LE(&bytes);
            memcpy(clientSeed, data, std::min(size_t(bytes), sizeof(clientSeed)));
            delete [] data;
        }

        // Create the symmetric key from a combination
        // of the client seed and the server seed
        uint8_t sharedSeed[kNetMaxSymmetricSeedBytes];
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
    const plBigNum* DH_N;
    NetMsgChannelGetDhConstants(cli->channel, nullptr, nullptr, &DH_N);
    bool encrypt = !DH_N->isZero();

    // Process message
    const NetCli_Srv2Cli_Encrypt & msg =
        * (const NetCli_Srv2Cli_Encrypt *) &pkt;
    if (encrypt) { // we insist on encryption, don't let some MitM decide for us!
        if (pkt.length != sizeof(msg))
            return false;

        // Create the symmetric key from a combination
        // of the client seed and the server seed
        uint8_t sharedSeed[kNetMaxSymmetricSeedBytes];
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
        cli->cryptIn = nullptr;
        cli->cryptOut = nullptr;
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

//===========================================================================
static unsigned DispatchPacket (
    NetCli *        cli,
    unsigned        bytes,
    const uint8_t      data[]
) {
    for (;;) {
        const NetCli_PacketHeader & pkt = * (const NetCli_PacketHeader *) data;
        if (bytes < sizeof(pkt))
            break;
        if (pkt.length > bytes)
            break;
        if (pkt.message >= kNumNetCliMsgs)
            break;

        bool result = false;
        switch (pkt.message) {
            case kNetCliCli2SrvConnect:
                result = ServerRecvConnect(cli, pkt);
                break;
            case kNetCliSrv2CliEncrypt:
                result = ClientRecvEncrypt(cli, pkt);
                break;
            case kNetCliSrv2CliError:
                result = ClientRecvError(cli, pkt);
                break;
        }
        if (!result) {
            break;
        }

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
    cli->recvMsg            = nullptr;
    cli->recvField          = nullptr;
    cli->recvFieldBytes     = 0;
    cli->recvDispatch       = true;
    cli->sendCurr           = cli->sendBuffer;
    cli->recvBuffer.clear();
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
        return nullptr;

    NetCli * const cli  = new NetCli;
    cli->sock           = sock;
    cli->protocol       = (ENetProtocol) protocol;
    cli->channel        = channel;
    cli->mode           = mode;

#if !defined(PLASMA_EXTERNAL_RELEASE) && defined(HS_BUILD_FOR_WIN32)
    // Network debug pipe
    if (!s_netlog) {
        WaitNamedPipeW(HURU_PIPE_NAME, NMPWAIT_WAIT_FOREVER);
        s_netlog = CreateFileW(
            HURU_PIPE_NAME,
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
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
    size_t          seedBytes,
    const uint8_t   seedData[]
) {
    if (seedBytes)
        memcpy(cli->seed, seedData, std::min(sizeof(cli->seed), seedBytes));
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
    const uint8_t          seedData[],
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
void NetCliClearSocket (NetCli * cli) {
    cli->sock = nullptr;
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
    cli->recvBuffer.clear();

    delete cli;
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
    const uintptr_t  msg[], 
    unsigned            count
) {
    BufferedSendData(cli, msg, count);
}

//============================================================================
bool NetCliDispatch (
    NetCli *        cli,
    const uint8_t      data[],
    unsigned        bytes,
    void *          param
) {
    if (!cli->recvDispatch)
        return false;

    do {
        if (cli->mode == kNetCliModeEncrypted) {
            // Decrypt data...
            uint8_t * temp = nullptr;

            if (cli->cryptIn) {
                temp = (uint8_t *)malloc(bytes);

                memcpy(temp, data, bytes);
                CryptDecrypt(cli->cryptIn, bytes, temp);
                data = temp;
            }

            // Add data to accumulator and dispatch
            cli->input.Add(bytes, data);
            DispatchData(cli, param);

#if !defined(PLASMA_EXTERNAL_RELEASE) && defined(HS_BUILD_FOR_WIN32)
            // Write to the netlog
            if (s_netlog) {
                NetLogMessage_Header header;
                header.m_protocol = cli->protocol;
                header.m_direction = 1; // kSrv2Cli
                header.m_time = GetAdjustedTimer();
                header.m_size = bytes;

                hsLockGuard(s_pipeCritical);
                DWORD bytesWritten;
                WriteFile(s_netlog, &header, sizeof(header), &bytesWritten, nullptr);
                WriteFile(s_netlog, data, bytes, &bytesWritten, nullptr);
            }
#endif // PLASMA_EXTERNAL_RELEASE

            // free heap buffer (if any)
            free(temp);

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
