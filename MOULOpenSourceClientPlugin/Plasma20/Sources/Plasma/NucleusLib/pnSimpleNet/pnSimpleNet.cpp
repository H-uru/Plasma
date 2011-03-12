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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnSimpleNet/pnSimpleNet.cpp
*   
***/

#include "Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Local types
*
***/

struct SimpleNetConn : AtomicRef {
	LINK(SimpleNetConn)			link;
	AsyncSocket					sock;
	AsyncCancelId				cancelId;
	unsigned					channelId;
	bool						abandoned;
	struct ConnectParam *		connectParam;

	SimpleNet_MsgHeader *		oversizeMsg;
	ARRAY(byte)					oversizeBuffer;

	~SimpleNetConn () {
		ASSERT(!link.IsLinked());
	}
};

struct SimpleNetChannel : AtomicRef, THashKeyVal<unsigned> {
	HASHLINK(SimpleNetChannel)	link;
	
	FSimpleNetOnMsg				onMsg;
	FSimpleNetOnError			onError;
	
	LISTDECL(SimpleNetConn, link)	conns;

	SimpleNetChannel (unsigned channel)
	: THashKeyVal<unsigned>(channel)
	{ }
	~SimpleNetChannel () {
		ASSERT(!link.IsLinked());
		ASSERT(!conns.Head());
	}
};

struct ConnectParam {
	SimpleNetChannel *		channel;
	FSimpleNetOnConnect		callback;
	void *					param;
	
	~ConnectParam () {
		if (channel)
			channel->DecRef();
	}
};


/*****************************************************************************
*
*   Local data
*
***/

static bool							s_running;
static CCritSect					s_critsect;
static FSimpleNetQueryAccept		s_queryAccept;
static void *						s_queryAcceptParam;

static HASHTABLEDECL(
	SimpleNetChannel,
	THashKeyVal<unsigned>,
	link
) s_channels;


/*****************************************************************************
*
*   Local functions
*
***/

//============================================================================
static void NotifyConnSocketConnect (SimpleNetConn * conn) {

	conn->TransferRef("Connecting", "Connected");
	
	conn->connectParam->callback(
		conn->connectParam->param,
		conn,
		kNetSuccess
	);
	
	DEL(conn->connectParam);
	conn->connectParam = nil;
}

//============================================================================
static void NotifyConnSocketConnectFailed (SimpleNetConn * conn) {

	s_critsect.Enter();
	{
		conn->link.Unlink();
	}
	s_critsect.Leave();

	conn->connectParam->callback(
		conn->connectParam->param,
		nil,
		kNetErrConnectFailed
	);
	
	DEL(conn->connectParam);
	conn->connectParam = nil;

	conn->DecRef("Connecting");
	conn->DecRef("Lifetime");
}

//============================================================================
static void NotifyConnSocketDisconnect (SimpleNetConn * conn) {

	bool abandoned;
	SimpleNetChannel * channel;
	s_critsect.Enter();
	{
		abandoned = conn->abandoned;
		if (nil != (channel = s_channels.Find(conn->channelId)))
			channel->IncRef();
		conn->link.Unlink();
	}
	s_critsect.Leave();
	
	if (channel && !abandoned) {
		channel->onError(conn, kNetErrDisconnected);
		channel->DecRef();
	}
	
	conn->DecRef("Connected");
}

//============================================================================
static bool NotifyConnSocketRead (SimpleNetConn * conn, AsyncNotifySocketRead * read) {

	SimpleNetChannel * channel;
	s_critsect.Enter();
	{
		if (nil != (channel = s_channels.Find(conn->channelId)))
			channel->IncRef();
	}
	s_critsect.Leave();
	
	if (!channel)
		return false;
		
	bool result = true;

    const byte * curr = read->buffer;
    const byte * term = curr + read->bytes;

	while (curr < term) {
		// Reading oversize msg?
		if (conn->oversizeBuffer.Count()) {
			unsigned spaceLeft = conn->oversizeMsg->messageBytes - conn->oversizeBuffer.Count();
			unsigned copyBytes = min(spaceLeft, term - curr);
			conn->oversizeBuffer.Add(curr, copyBytes);
			
			curr += copyBytes;

			// Wait until we have received the entire message
			if (copyBytes != spaceLeft)
				break;
				
			// Dispatch oversize msg
			if (!channel->onMsg(conn, conn->oversizeMsg)) {
				result = false;
				break;
			}
			
			conn->oversizeBuffer.SetCount(0);
			continue;
		}

		// Wait until we receive the entire message header
		if (term - curr < sizeof(SimpleNet_MsgHeader))
			break;

		SimpleNet_MsgHeader * msg = (SimpleNet_MsgHeader *) read->buffer;

		// Sanity check message size		
		if (msg->messageBytes < sizeof(*msg)) {
			result = false;
			break;
		}
		
		// Handle oversized messages
		if (msg->messageBytes > kAsyncSocketBufferSize) {
			
			conn->oversizeBuffer.SetCount(msg->messageBytes);
			conn->oversizeMsg = (SimpleNet_MsgHeader *) conn->oversizeBuffer.Ptr();
			*conn->oversizeMsg = *msg;			
			
			curr += sizeof(*msg);
			continue;
		}
		
		// Wait until we have received the entire message
		const byte * msgTerm = (const byte *) curr + msg->messageBytes;
		if (msgTerm > term)
			break;
		curr = msgTerm;

		// Dispatch msg
		if (!channel->onMsg(conn, msg)) {
			result = false;
			break;
		}
	}

	// Return count of bytes we processed
	read->bytesProcessed = curr - read->buffer;
	
	channel->DecRef();
	return result;
}

//============================================================================
static bool AsyncNotifySocketProc (
	AsyncSocket			sock,
	EAsyncNotifySocket	code,
	AsyncNotifySocket *	notify,
	void **				userState
) {
	bool result = true;
	SimpleNetConn * conn;

	switch (code) {
		case kNotifySocketListenSuccess: {

			AsyncNotifySocketListen * listen = (AsyncNotifySocketListen *) notify;

			const SimpleNet_ConnData & connect = *(const SimpleNet_ConnData *) listen->buffer;
			listen->bytesProcessed += sizeof(connect);

			SimpleNetChannel * channel;
			s_critsect.Enter();
			{
				if (nil != (channel = s_channels.Find(connect.channelId)))
					channel->IncRef();
			}
			s_critsect.Leave();
			
			if (!channel)
				break;
				
			conn = NEWZERO(SimpleNetConn);
			conn->channelId = channel->GetValue();
			conn->IncRef("Lifetime");
			conn->IncRef("Connected");
			conn->sock = sock;
			*userState = conn;
			
			bool accepted = s_queryAccept(
				s_queryAcceptParam,
				channel->GetValue(),
				conn,
				listen->remoteAddr
			);
			
			if (!accepted) {
				SimpleNetDisconnect(conn);
			}
			else {
				s_critsect.Enter();
				{
					channel->conns.Link(conn);
				}
				s_critsect.Leave();
			}
			
			channel->DecRef();
		}
		break;
		
		case kNotifySocketConnectSuccess: {
			conn = (SimpleNetConn *) notify->param;
			*userState = conn;
			bool abandoned;
			
			s_critsect.Enter();
			{
				conn->sock		= sock;
				conn->cancelId  = 0;
				abandoned		= conn->abandoned;
			}
			s_critsect.Leave();
			
			if (abandoned)
				AsyncSocketDisconnect(sock, true);
			else
				NotifyConnSocketConnect(conn);
		}
		break;

		case kNotifySocketConnectFailed:
			conn = (SimpleNetConn *) notify->param;
			NotifyConnSocketConnectFailed(conn);
		break;

		case kNotifySocketDisconnect:
			conn = (SimpleNetConn *) *userState;
			NotifyConnSocketDisconnect(conn);
		break;

		case kNotifySocketRead:
			conn = (SimpleNetConn *) *userState;
			result = NotifyConnSocketRead(conn, (AsyncNotifySocketRead *) notify);
		break;
	}

	return result;
}

//============================================================================
static void Connect (const NetAddress & addr, ConnectParam * cp) {

	SimpleNetConn * conn = NEWZERO(SimpleNetConn);
	conn->channelId = cp->channel->GetValue();
	conn->connectParam = cp;
	conn->IncRef("Lifetime");
	conn->IncRef("Connecting");

	s_critsect.Enter();
	{
		cp->channel->conns.Link(conn);
		
		SimpleNet_Connect connect;
		connect.hdr.connType	= kConnTypeSimpleNet;
		connect.hdr.hdrBytes	= sizeof(connect.hdr);
		connect.hdr.buildId		= BuildId();
		connect.hdr.buildType	= BuildType();
		connect.hdr.branchId	= BranchId();
		connect.hdr.productId	= ProductId();
		connect.data.channelId	= cp->channel->GetValue();
			
		AsyncSocketConnect(
			&conn->cancelId,
			addr,
			AsyncNotifySocketProc,
			conn,
			&connect,
			sizeof(connect)
		);
		
		conn = nil;	
		cp = nil;
	}
	s_critsect.Leave();
	
	DEL(conn);
	DEL(cp);
}

//============================================================================
static void AsyncLookupCallback (
	void *				param,
	const wchar			name[],
	unsigned			addrCount,
	const NetAddress	addrs[]
) {
	ref(name);
	
	ConnectParam * cp = (ConnectParam *)param;

	if (!addrCount) {
		if (cp->callback)
			cp->callback(cp->param, nil, kNetErrNameLookupFailed);
		DEL(cp);
		return;
	}

	Connect(addrs[0], (ConnectParam *)param);
}


/*****************************************************************************
*
*   Exported functions
*
***/

//============================================================================
void SimpleNetInitialize () {

	s_running = true;

	AsyncSocketRegisterNotifyProc(
		kConnTypeSimpleNet,
		AsyncNotifySocketProc
	);
}

//============================================================================
void SimpleNetShutdown () {

	s_running = false;

	ASSERT(!s_channels.Head());
	
	AsyncSocketUnregisterNotifyProc(
		kConnTypeSimpleNet,
		AsyncNotifySocketProc
	);
}

//============================================================================
void SimpleNetConnIncRef (SimpleNetConn * conn) {

	ASSERT(s_running);
	ASSERT(conn);
	
	conn->IncRef();
}

//============================================================================
void SimpleNetConnDecRef (SimpleNetConn * conn) {

	ASSERT(s_running);
	ASSERT(conn);
	
	conn->DecRef();
}

//============================================================================
bool SimpleNetStartListening (
	FSimpleNetQueryAccept	queryAccept,
	void *					param
) {
	ASSERT(s_running);
	ASSERT(queryAccept);
	ASSERT(!s_queryAccept);
	
	s_queryAccept		= queryAccept;
	s_queryAcceptParam	= param;
	
	NetAddress addr;
	NetAddressFromNode(0, kNetDefaultSimpleNetPort, &addr);
	return (0 != AsyncSocketStartListening(addr, nil));
}

//============================================================================
void SimpleNetStopListening () {

	ASSERT(s_running);

	NetAddress addr;
	NetAddressFromNode(0, kNetDefaultSimpleNetPort, &addr);
	AsyncSocketStopListening(addr, nil);

	s_queryAccept		= nil;
	s_queryAcceptParam	= nil;
}

//============================================================================
void SimpleNetCreateChannel (
	unsigned			channelId,
	FSimpleNetOnMsg		onMsg,
	FSimpleNetOnError	onError
) {
	ASSERT(s_running);

	SimpleNetChannel * channel = NEWZERO(SimpleNetChannel)(channelId);
	channel->IncRef();

	s_critsect.Enter();
	{
		#ifdef HS_DEBUGGING
		{
			SimpleNetChannel * existing = s_channels.Find(channelId);
			ASSERT(!existing);
		}
		#endif
		
		channel->onMsg		= onMsg;
		channel->onError	= onError;
		s_channels.Add(channel);
		channel->IncRef();
	}
	s_critsect.Leave();
	
	channel->DecRef();
}

//============================================================================
void SimpleNetDestroyChannel (unsigned channelId) {

	ASSERT(s_running);

	SimpleNetChannel * channel;
	s_critsect.Enter();
	{
		if (nil != (channel = s_channels.Find(channelId))) {
			s_channels.Unlink(channel);
			while (SimpleNetConn * conn = channel->conns.Head()) {
				SimpleNetDisconnect(conn);
				channel->conns.Unlink(conn);
			}
		}
	}
	s_critsect.Leave();

	if (channel)
		channel->DecRef();	
}

//============================================================================
void SimpleNetStartConnecting (
	unsigned			channelId,
	const wchar			addr[],
	FSimpleNetOnConnect	onConnect,
	void *				param
) {
	ASSERT(s_running);
	ASSERT(onConnect);
	
	ConnectParam * cp = NEW(ConnectParam);
	cp->callback	= onConnect;
	cp->param		= param;
	
	s_critsect.Enter();
	{
		if (nil != (cp->channel = s_channels.Find(channelId)))
			cp->channel->IncRef();
	}
	s_critsect.Leave();
	
	ASSERT(cp->channel);

	// Do we need to lookup the address?
	const wchar * name = addr;
	while (unsigned ch = *name) {
		++name;
		if (!(isdigit(ch) || ch == L'.' || ch == L':')) {

			AsyncCancelId cancelId;
			AsyncAddressLookupName(
				&cancelId,
				AsyncLookupCallback,
				addr,
				kNetDefaultSimpleNetPort,
				cp
			);
			break;
		}
	}
	if (!name[0]) {
		NetAddress netAddr;
		NetAddressFromString(&netAddr, addr, kNetDefaultSimpleNetPort);
		Connect(netAddr, cp);
	}
}

//============================================================================
void SimpleNetDisconnect (
	SimpleNetConn *	conn
) {
	ASSERT(s_running);
	ASSERT(conn);
	
	s_critsect.Enter();
	{
		conn->abandoned = true;
		if (conn->sock) {
			AsyncSocketDisconnect(conn->sock, true);
			conn->sock = nil;
		}
		else if (conn->cancelId) {
			AsyncSocketConnectCancel(AsyncNotifySocketProc, conn->cancelId);
			conn->cancelId = nil;
		}
	}
	s_critsect.Leave();

	conn->DecRef("Lifetime");
}

//============================================================================
void SimpleNetSend (
	SimpleNetConn *			conn,
	SimpleNet_MsgHeader *	msg
) {
	ASSERT(s_running);
	ASSERT(msg);
	ASSERT(msg->messageBytes != (dword)-1);
	ASSERT(conn);
	
	s_critsect.Enter();
	{
		if (conn->sock)
			AsyncSocketSend(conn->sock, msg, msg->messageBytes);
	}
	s_critsect.Leave();
}
