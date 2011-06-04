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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetCli/pnNcCli.cpp
*   
***/

#include "Pch.h"
#pragma hdrstop

//#define NCCLI_DEBUGGING
#ifdef NCCLI_DEBUGGING
# pragma message("Compiling pnNetCli with debugging on")
# define NCCLI_LOG	LogMsg
#else
# define NCCLI_LOG	NULL_STMT
#endif

//#define NO_ENCRYPTION

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
    CryptKey *              cryptIn;
    CryptKey *              cryptOut;
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
		ref(temp);

	if (cli->mode == kNetCliModeEncrypted) {
		// Encrypt data...
#ifndef NO_ENCRYPTION
		if (bytes <= 2048)
			// byte count is small, use stack-based buffer
			temp = ALLOCA(byte, bytes);
		else
			// byte count is large, use heap-based buffer
			temp = heap = (byte *)ALLOC(bytes);

		MemCopy(temp, data, bytes);
		CryptEncrypt(cli->cryptOut, bytes, temp);
		data = temp;
#endif
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

	#define WRITE_SWAPPED_INT(t,c) {        \
		ASSERT(sizeof(t) == sizeof(c));     \
		t endianCount = Endian((t)c);       \
		AddToSendBuffer(cli, sizeof(t), (const void *) &endianCount);   \
	}


	ASSERT(cli);
	ASSERT(msg);
	ASSERT(fieldCount);

	if (!cli->sock)
		return;

	unsigned_ptr const * const msgEnd = msg + fieldCount;
	ref(msgEnd);

	const NetMsgInitSend * sendMsg = NetMsgChannelFindSendMessage(cli->channel, msg[0]);
	ASSERT(msg[0] == sendMsg->msg.messageId);
	ASSERT(fieldCount-1 == sendMsg->msg.count);

	// insert messageId into command stream
	const word msgId = (word) msg[0];
	WRITE_SWAPPED_INT(word, msgId);
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
					// Single values are passed by value
					EndianCopy(temp, (const byte *) msg, count, cmd->size);
				else
					// Value arrays are passed in by ptr
					EndianCopy(temp, (const byte *) *msg, count, cmd->size);
	            
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
				WRITE_SWAPPED_INT(word, length);
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
				varCount = (dword) *msg;
				WRITE_SWAPPED_INT(dword, varCount);
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

			msgId = Endian(msgId);

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
					EndianConvert(
						data,
						count,
						cli->recvField->size
					);

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
					EndianConvert((dword *) data, 1);

					// Prepare to read var-length field
					cli->recvFieldBytes = *(dword *)data * cli->recvField->size;

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
						cli->recvFieldBytes = Endian(length) * sizeof(wchar);

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
		const void * data = clientSeed.GetData(&bytes);
		MemCopy(cli->seed, data, min(bytes, sizeof(cli->seed)));
	}

	// Send server seed
	if (cli->sock) {
		unsigned bytes;
		NetCli_Cli2Srv_Connect msg;
		const void * data = serverSeed.GetData(&bytes);
		ASSERTMSG(bytes <= sizeof(msg.dh_y_data), "4");
		msg.message    = kNetCliCli2SrvConnect;
		msg.length     = (byte) (sizeof(msg) - sizeof(msg.dh_y_data) +  bytes);
		MemCopy(msg.dh_y_data, data, bytes);
		AsyncSocketSend(cli->sock, &msg, msg.length);
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

	// Send the server seed to the client (unencrypted)
	if (cli->sock) {
		NetCli_Srv2Cli_Encrypt reply;
		reply.message   = kNetCliSrv2CliEncrypt;
		reply.length    = sizeof(reply);
		MemCopy(reply.serverSeed, cli->seed, sizeof(reply.serverSeed));
		AsyncSocketSend(cli->sock, &reply, sizeof(reply));
	}

	// Compute client seed
	byte clientSeed[kNetMaxSymmetricSeedBytes];
	{
		BigNum clientSeedValue;
		NetMsgCryptServerConnect(
			cli->channel,
			msg.length - sizeof(pkt),
			msg.dh_y_data,
			&clientSeedValue
		);

		ZERO(clientSeed);
		unsigned bytes;
		const void * data = clientSeedValue.GetData(&bytes);
		MemCopy(clientSeed, data, min(bytes, sizeof(clientSeed)));
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
	cli->mode = kNetCliModeEncrypted;
	cli->cryptIn  = CryptKeyCreate(kCryptRc4, sizeof(sharedSeed), sharedSeed);
	cli->cryptOut = CryptKeyCreate(kCryptRc4, sizeof(sharedSeed), sharedSeed);

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

	// Validate message size
	const NetCli_Srv2Cli_Encrypt & msg =
		* (const NetCli_Srv2Cli_Encrypt *) &pkt;
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
	cli->mode = kNetCliModeEncrypted;
	cli->cryptIn  = CryptKeyCreate(kCryptRc4, sizeof(sharedSeed), sharedSeed);
	cli->cryptOut = CryptKeyCreate(kCryptRc4, sizeof(sharedSeed), sharedSeed);

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

	NetCli * const cli	= NEWZERO(NetCli);
	cli->sock           = sock;
	cli->protocol       = (ENetProtocol) protocol;
	cli->channel        = channel;
	cli->mode           = mode;
	cli->SetValue(kNilGuid);

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
			ref(temp);

#ifndef NO_ENCRYPTION
			if (bytes <= 2048)
				// byte count is small, use stack-based buffer
				temp = ALLOCA(byte, bytes);
			else
				// byte count is large, use heap-based buffer
				temp = heap = (byte *)ALLOC(bytes);

			MemCopy(temp, data, bytes);
			CryptDecrypt(cli->cryptIn, bytes, temp);
			data = temp;
#endif

			// Add data to accumulator and dispatch
			cli->input.Add(bytes, data);
			bool result = DispatchData(cli, param);
			ref(result);

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
