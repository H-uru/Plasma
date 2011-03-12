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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetLog/pnNlCli.cpp
*   
***/

#include "Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Private
*
***/

struct NetLogConn : SrvConn {
    NetAddress      netaddr;
	ESrvType		srvType;

    NetLogConn (const NetAddress & addr, ESrvType srvType);
    ~NetLogConn ();

    void Connect (
        AsyncCancelId *         cancelId,
        FAsyncNotifySocketProc  notifyProc,
        void *                  param
    );
};

struct LogMsgTrans : SrvTrans {
	Srv2Log_LogMsg *msgBuffer;

	LogMsgTrans();
	~LogMsgTrans();
	bool OnTransReply (
		ENetError *     error,
		SrvMsgHeader *  msg
	);
};

struct LogConnEventNode {
	LINK(LogConnEventNode)	link;
	Srv2Log_LogMsg *	msg;
	unsigned			sendTimeMs;
	
	LogConnEventNode(Srv2Log_LogMsg *msg, unsigned timeStampMs);
};


/*****************************************************************************
*
*   Private Data
*
***/

static NetLogConn *						s_conn;
static CCritSect						s_critsect;
static IniChangeReg *					s_change;
static ESrvType							s_srvType;
static LISTDECL(LogConnEventNode, link) s_eventQueue;
static AsyncTimer *						s_timer;
static long								s_perf[kNlCliNumPerf];
static bool								s_running;

static const unsigned					kIssueSaveMs				= 100;
static const unsigned					kMaxNumberOfTransactions	= 2000;


/*****************************************************************************
*
*   Private Functions
*
***/

//============================================================================
static NetLogConn * GetConnIncRef () {
	NetLogConn * conn;
	s_critsect.Enter();
	{
		if (nil != (conn = s_conn))
			conn->IncRef();
	}
	s_critsect.Leave();
	return conn;
}

//============================================================================
static void NetLogConnConnect (NetAddress & addr, ESrvType srvType) {
	NetLogConn *conn = SRV_CONN_ALLOC(NetLogConn)(addr, srvType);
  
    s_critsect.Enter();
    {
        SWAP(s_conn, conn);
    }
    s_critsect.Leave();

    if (conn)
        conn->Destroy();
}

//============================================================================
static void NetLogConnDisconnect () {
	NetLogConn *conn = nil;
  
    s_critsect.Enter();
    {
        SWAP(s_conn, conn);
    }
    s_critsect.Leave();

    if (conn)
        conn->Destroy();
}

//============================================================================
static void AddEventNode (Srv2Log_LogMsg *msg) {
	LogConnEventNode *node = NEW(LogConnEventNode)(msg, TimeGetMs() + kIssueSaveMs);
	s_critsect.Enter();
	{
		s_eventQueue.Link(node);
	}
	s_critsect.Leave();

	AsyncTimerUpdate(
		s_timer,
		kIssueSaveMs,
		kAsyncTimerUpdateSetPriorityHigher
	);
}

//============================================================================
static void ParseIni (Ini * ini) {
    unsigned iter;
    const IniValue * value = IniGetFirstValue(
        ini,
        L"Server Locations",
        L"LogSrv",
        &iter
    );

    if (value) {
        wchar addrStr[32];
        IniGetString(value, addrStr, arrsize(addrStr), 0, nil);
        NetAddress addr;
        NetAddressFromString(&addr, addrStr, kNetDefaultServerPort);
        NetLogConnConnect(addr, s_srvType);
    }
}

//============================================================================
static void IniChangeCallback (const wchar fullPath[]) {
    Ini * ini = IniOpen(fullPath);
    ParseIni(ini);
    IniClose(ini);
}

//============================================================================
static unsigned TimerCallback (void *) {
	LISTDECL(LogConnEventNode, link) sendList;
	unsigned sleepMs = kAsyncTimeInfinite;
	
	s_critsect.Enter();
	{
		int allowedNumTrans = kMaxNumberOfTransactions - s_perf[kNlCliNumPendingSaves];	// find the number of transactions that we can send based on the number in transit, and the max number allowed
		if(allowedNumTrans < 0)		// this could be negative, if so set to zero
			allowedNumTrans = 0;

		dword currTime = TimeGetMs();
		LogConnEventNode *hash;
		int timeDiff;

		// Add pending saves, up to the max allowed per update
		for(;;) {
			if(!allowedNumTrans && s_running) {
				sleepMs = 5000;	// we are at our max number of transactions sleep for 5 seconds and try again
				break;
			}

			hash = s_eventQueue.Head();
			if(!hash)
				break;		// no messages left. We will wait until another message comes in before another timer update

			timeDiff = hash->sendTimeMs - currTime;		

			// nodes are naturally ordered by increasing sendTimeMs
			if(!s_running || (timeDiff <= 0)) {
				sendList.Link(hash);
				--allowedNumTrans;
			}
			else {
				sleepMs	= timeDiff;
				break;
			}
		}
	}
	s_critsect.Leave();

	while(LogConnEventNode *node = sendList.Head()) {
		LogMsgTrans * trans = SRV_TRANS_ALLOC(LogMsgTrans);
		trans->msgBuffer = node->msg;

		if (NetLogConn * conn  = GetConnIncRef()) {
			conn->SendRequest(trans, node->msg);
			conn->DecRef();
		}
		else {
			trans->TransCancel(kNetErrTimeout);
		}
		delete node;
	}
	return sleepMs;
}

//============================================================================
static unsigned CalcArgsLength (const NetLogEvent &event, va_list args) {
	unsigned length = 0;
	unsigned paramType = kNumLogParamTypes;	// invalidate
	unsigned field = 0;

	for(unsigned i = 0; i < event.numFields * 2; ++i) {
		if(!(i % 2)) {
			paramType = va_arg(args, unsigned);
			continue;
		}
		
		//validate parameter type
		if(paramType != (unsigned)event.fields[field].type) { 
			length = 0;
			LogMsg( kLogError, "Log parameter types do not match for event: %s parameter: %s?", event.eventName, event.fields[field].name);
			break;
		}

		switch(event.fields[field].type) {
			case kLogParamInt: {
				va_arg(args, int);
				length += sizeof(int);
			}
			break;

			case kLogParamUnsigned: {
				va_arg(args, unsigned);
				length += sizeof(unsigned);
			}
			break;

			case kLogParamFloat: {
				va_arg(args, float);
				length += sizeof(float);
			}
			break;

			case kLogParamLong: {
				va_arg(args, long);
				length += sizeof(long);
			}
			break;

			case kLogParamLongLong: {
				va_arg(args, long long);
				length += sizeof(long long);
			}
			break;

			case kLogParamUuid: {
				va_arg(args, Uuid);
				length += sizeof(Uuid);
			}
			break;
			
			case kLogParamStringW: {
				wchar *str = va_arg(args, wchar *);
				if(!str)
					str = L"";
				length += StrBytes(str);

			}
			break;

			default:
				hsAssert(false, "Unknown argument type in log statement");
			return 0;
		}
		++field;
	}
	return length;
}


/*****************************************************************************
*
*   NetLogConn
*
***/

//============================================================================
NetLogConn::NetLogConn (const NetAddress & addr, ESrvType srvType)
: netaddr(addr),
  srvType(srvType)
{
	AtomicAdd(&s_perf[kNlCliNumConn], 1);
	SetAutoPing();
    AutoReconnect();
}

//============================================================================
NetLogConn::~NetLogConn () {
	AtomicAdd(&s_perf[kNlCliNumConn], -1);
}

//============================================================================
void NetLogConn::Connect (
    AsyncCancelId *         cancelId,
    FAsyncNotifySocketProc  notifyProc,
    void *                  param
) { 
	// Connect to remote server
    Srv2Log_Connect          connect;
    connect.hdr.connType    = kConnTypeSrvToLog;
    connect.hdr.hdrBytes    = sizeof(connect.hdr);
    connect.hdr.buildId     = BuildId();
    connect.hdr.buildType   = BuildType();
    connect.hdr.branchId	= BranchId();
    connect.hdr.productId   = ProductId();
    connect.data.dataBytes  = sizeof(connect.data);
	connect.data.buildId    = BuildId();
    connect.data.srvType    = srvType;
	connect.data.buildType  = BuildType();
	connect.data.productId  = ProductId();
	
	AsyncSocketConnect(
        cancelId,
        netaddr,
        notifyProc,
        param,
        &connect,
        sizeof(connect)
    );
}


/*****************************************************************************
*
*   LogMsgTrans
*
***/

//============================================================================
LogMsgTrans::LogMsgTrans () 
: msgBuffer(nil)
{
	AtomicAdd(&s_perf[kNlCliNumTrans], 1);
}

//============================================================================
LogMsgTrans::~LogMsgTrans () {
	AtomicAdd(&s_perf[kNlCliNumTrans], -1);
}

//============================================================================
bool LogMsgTrans::OnTransReply (
    ENetError *     error,
    SrvMsgHeader *  msg
) {
	ref(msg);
	bool result;
	if (msg->protocolId != kNetProtocolSrv2Log) {
        result = SrvTrans::OnTransReply(error, msg);
    }
	else {
		result = true;
	}

	if(IS_NET_ERROR(*error) && s_running) {
		AddEventNode(msgBuffer);
	}
	else {
		FREE(msgBuffer);
	}
	return true;
}


/*****************************************************************************
*
*   LogConnEventNode
*
***/

//============================================================================
LogConnEventNode::LogConnEventNode (Srv2Log_LogMsg *msg, unsigned sendTimeMs)
:	msg(msg),
	sendTimeMs(sendTimeMs)
{
}


/*****************************************************************************
*
*   Protected
*
***/

//============================================================================
void NetLogCliInitialize (ESrvType srvType) {
	s_running = true;
	s_srvType = srvType;
	AsyncTimerCreate(
		&s_timer,
		TimerCallback,
		kAsyncTimeInfinite,
		nil
	);
	IniChangeAdd(L"plServer", IniChangeCallback, &s_change);
}

//============================================================================
void NetLogCliShutdown () {
	s_running = false;
	if(s_change) {
		IniChangeRemove(s_change, true);
		s_change = false;
	}
	if(s_timer) {
		AsyncTimerDeleteCallback(s_timer, TimerCallback);
		s_timer = nil;
	}
	NetLogConnDisconnect();
}

//============================================================================
void NetLogCliDestroy () {
	while(s_perf[kNlCliNumTrans]) 
		AsyncSleep(10);
	while(s_perf[kNlCliNumConn])
        AsyncSleep(10);
}

//============================================================================
void NetLogCliSendEvent (const NetLogEvent &event, va_list args) {
	Srv2Log_LogMsg *msg;
	unsigned length = CalcArgsLength(event, args);
	if(!length)
		return;

	CSrvPackBuffer pack(
		sizeof(*msg) +
		length
	);

	msg = (Srv2Log_LogMsg *) pack.Alloc(sizeof(*msg));
	msg->transId			= 0;
	msg->protocolId			= kNetProtocolSrv2Log;
	msg->messageId			= kSrv2Log_LogMsg;
	msg->eventType			= event.logEventType;
	msg->timestamp			= TimeGetLocalTime();
	
	unsigned field = 0;
	unsigned paramType = kNumLogParamTypes;

	// if we get here the template parameters have already been validated by CalcLength, no need to do that again.
	for(unsigned i = 0; i < event.numFields * 2; ++i) {
		if(!(i % 2)) {
			paramType = va_arg(args, unsigned);
			continue;
		}
		switch(event.fields[field].type) {
			case kLogParamInt: {
				int i = va_arg(args, int);
				pack.AddData(&i, sizeof(int));
			}
			break;

			case kLogParamUnsigned: {
				unsigned u = va_arg(args, unsigned);
				pack.AddData(&u, sizeof(unsigned));
			}
			break;

			case kLogParamFloat: {
				float f = va_arg(args, float);
				pack.AddData(&f, sizeof(float));
			}
			break;

			case kLogParamLong: {
				long l = va_arg(args, long);
				pack.AddData(&l, sizeof(long));
			}
			break;

			case kLogParamLongLong: {
				long long ll = va_arg(args, long long);
				pack.AddData(&ll, sizeof(long long));
			}
			break;

			case kLogParamUuid: {
				Uuid uuid = va_arg(args, Uuid);
				pack.AddData(&uuid, sizeof(Uuid));
			}
			break;
			
			case kLogParamStringW: {
				wchar *str = va_arg(args, wchar *);
				if(!str)
					str = L"";
				pack.AddString(str);
			}
			break;
		}
		++field;
	}

	msg->messageBytes	= pack.Size();
	AddEventNode(msg);
}
