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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetLog/pnNlSrv.cpp
*   
***/

#include "Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Private
*
***/

struct EventHash {
	unsigned	eventType;
	ESrvType	srvType;

    inline EventHash (
		unsigned	eventType,
		ESrvType	srvType
    );

    inline dword GetHash () const;
    inline bool operator== (const EventHash & rhs) const;
};

struct NetLogEventHash : EventHash {
	const NetLogEvent *	event;

	NetLogEventHash(
		unsigned	eventType,
		ESrvType	srvType,
		const NetLogEvent *event
	);
	HASHLINK(NetLogEventHash)	link;	
};

struct LogConn : SrvConn {
    LINK(LogConn)       link;
	ESrvType			srvType;
	unsigned			buildId;
	NetAddressNode		addr;
	unsigned			buildType;
	unsigned			productId;

    // Ctor
    LogConn (
        AsyncSocket     sock,
        void **         userState,
		NetAddressNode  nodeNumber,
        unsigned        buildId,
        ESrvType        srvType,
		unsigned		buildType,
		unsigned		productId
    );
    ~LogConn ();

    bool OnSrvMsg (SrvMsgHeader * msg);
    void OnDisconnect ();

	bool Recv_Srv2Log_LogMsg( const Srv2Log_LogMsg & msg );
};


/*****************************************************************************
*
*   Private Data
*
***/

static IniChangeReg *			s_change;
static long						s_perf[kNlSrvNumPerf];
static CCritSect				s_critsect;
static bool						s_running;
static LISTDECL(LogConn, link)	s_conns;
void (*NetLogSrvCallback)(const NetLogEvent *event, const ARRAY(wchar) &, unsigned, NetAddressNode &, qword, unsigned, unsigned ) ;


/*****************************************************************************
*
*   Private Functions
*
***/

//============================================================================
static void ParseIni (Ini * ini) {
	unsigned iter;
	const IniValue *value;

	value = IniGetFirstValue(
		ini,
		L"",
		L"",
		&iter
	);
}

//============================================================================
static void IniChangeCallback (const wchar fullPath[]) {
	Ini * ini = IniOpen(fullPath);
	ParseIni(ini);
	IniClose(ini);
}

//===========================================================================
static bool SocketNotifyProc (
    AsyncSocket         sock,
    EAsyncNotifySocket  code,
    AsyncNotifySocket * notify,
    void **             userState
) {
 // If the socket is successfully connected then only
    // kNotifySocketListenSuccess will arrive here, as
    // the service routine will be changed by SrvConn.
    if (code != kNotifySocketListenSuccess) {
        if (code == kNotifySocketDisconnect)
            AsyncSocketDelete(sock);
        return false;
    }

    // TODO: Verify this is from a valid server
    AsyncNotifySocketListen * listen = (AsyncNotifySocketListen *) notify;
    EServerRights rights = SrvIniGetServerRights(listen->remoteAddr);
    if (rights < kSrvRightsServer) {
        LogMsgDebug("LogConn: insufficient server rights");
        AtomicAdd(&s_perf[kNlSrvPerfConnDenied], 1);
        return false;
    }

    NetAddressNode nodeNumber = NetAddressGetNode(listen->remoteAddr);

    // Parse the connect message
    Srv2Log_ConnData connect;
    if (!Srv2LogValidateConnect(listen, &connect)) {
        LogMsgDebug("LogConn: invalid connect packet");
        AtomicAdd(&s_perf[kNlSrvPerfConnDenied], 1);
        return false;
    }

    // Create connection record
    LogConn * conn = SRV_CONN_ALLOC(LogConn)(
        sock,
        userState,
		nodeNumber,
        connect.buildId,
        (ESrvType) connect.srvType,
		connect.buildType,
		connect.productId
    );

    // Add this connection
    bool result;
    s_critsect.Enter();
    {
        s_conns.Link(conn);
        result = s_running;
    }
    s_critsect.Leave();

    return result;
}


/*****************************************************************************
*
*   LogConn implementation
*
***/

//============================================================================
LogConn::LogConn (
    AsyncSocket     sock,
    void **         userState,
	NetAddressNode  nodeNumber,
    unsigned        buildId,
    ESrvType        srvType,
	unsigned		buildType,
	unsigned		productId
) : srvType(srvType),
	buildId(buildId),
	buildType(buildType),
	productId(productId)
{
	ref(nodeNumber);
    AtomicAdd(&s_perf[kNlSrvPerfConnCount], 1);
    SetAutoTimeout();
    NotifyListen(sock, userState);
	addr = nodeNumber;
}

//============================================================================
LogConn::~LogConn () {
    AtomicAdd(&s_perf[kNlSrvPerfConnCount], -1);
}

//============================================================================
void LogConn::OnDisconnect () {
    // Accepting side just destroys the connection.  Connecting side
    // will reconnect if that's what should happen.
    Destroy();
}

//============================================================================
bool LogConn::OnSrvMsg (SrvMsgHeader * msg) {
    // Pass along messages not intended for us
    if (msg->protocolId != kNetProtocolSrv2Log)
        return SrvConn::OnSrvMsg(msg);

	bool status = false;
	#define DISPATCH(a) case k##a: status = Recv_##a(*(const a*)msg); break
	switch (msg->messageId) {
		DISPATCH(Srv2Log_LogMsg);

		default:
			status = false;
		break;
	}
	#undef DISPATCH

    return status;
}

//============================================================================
bool LogConn::Recv_Srv2Log_LogMsg(const Srv2Log_LogMsg & msg ) {
	CSrvUnpackBuffer unpack(&msg, msg.messageBytes);
	(void)unpack.GetData(sizeof(msg));
	unsigned length = 0;
	ARRAY(wchar) databuf;
	wchar data[256];
	const void *pData = 0;

	SendReply(msg.transId, kNetSuccess);	
	if(!s_running)
		return true;

	// WARNING: each parameter type needs to be able to handle the case where GetData returns a nil pointer for backward compatability
		
	const NetLogEvent *event = NetLogFindEvent(msg.eventType, srvType);
	if(event) {
		for(unsigned i = 0; i < event->numFields; ++i) {
			if(!event->fields[i].name)
			{
				LogMsg(kLogError, "Failed logging event because of nil param name: %s", event->eventName);
				return true;
			}
			
			switch(event->fields[i].type) {
				case kLogParamInt: {
					int i = 0;
					pData = unpack.GetData(sizeof(int));
					if(!pData)
						continue;

					i = *(int *)pData;
					StrPrintf(data, arrsize(data), L"%d", i);
					length += StrLen(data) + 1;
				}
				break;

				case kLogParamUnsigned: {
					unsigned u = 0;
					pData = unpack.GetData(sizeof(unsigned));
					if(!pData)
						continue;

					u = *(unsigned *)pData;
					StrPrintf(data, arrsize(data), L"%d", u);
					length += StrLen(data) + 1;
				}
				break;

				case kLogParamFloat: {
					float f = 0;
					pData = unpack.GetData(sizeof(float));
					if(!pData)
						continue;

					f = *(float *)pData;
					StrPrintf(data, arrsize(data), L"%f", f);
					length += StrLen(data) + 1;
				}
				break;

				case kLogParamLong: {
					long l = 0;
					pData = unpack.GetData(sizeof(long));
					if(!pData)
						continue;
					
					l = *(long *)pData;
					StrPrintf(data, arrsize(data), L"%ld", l);
					length += StrLen(data) + 1;
				}
				break;

				case kLogParamLongLong: {
					long long ll = 0;
					pData = unpack.GetData(sizeof(long));
					if(!pData)
						continue;
					ll = *(long *)pData;
					StrPrintf(data, arrsize(data), L"%lld", ll);
					length += StrLen(data) + 1;
				}
				break;

				case kLogParamUuid: {
					Uuid uuid = 0;
					pData = unpack.GetData(sizeof(Uuid));
					if(!pData)
						continue;
					uuid = *(Uuid *)pData;
					StrPrintf(data, arrsize(data), L"%s", uuid.data);
					length += StrLen(data) + 1;
				}
				break;
				
				case kLogParamStringW: {
					const wchar *str = unpack.GetString();
					if(!str) {
						continue;
					}
					length += StrLen(str) + 1;
				}
				break;
			}
			length += StrLen(event->fields[i].name) + 1;	// this must happen after the parameter check so we can opt out of saving a non existant parameter
		}
		
		databuf.Reserve(length + 1);

		{
			CSrvUnpackBuffer unpack(&msg, msg.messageBytes);
			(void)unpack.GetData(sizeof(msg));
			for(unsigned i = 0; i < event->numFields; ++i){
				
				// the parameter name needs to be written before the data. Also, we need to be able to opt out of writing a parameter.
				switch(event->fields[i].type) {
					case kLogParamInt: {
						pData = unpack.GetData(sizeof(int));
						if(!pData)
							continue;

						int val = *(int *)pData;

						// log event name
						databuf.Add(event->fields[i].name, StrLen(event->fields[i].name));
						databuf.Add(0);

						// log event data
						StrPrintf(data, arrsize(data), L"%d", val);
						databuf.Add(data, StrLen(data));
						databuf.Add(0);
					}
					break;

					case kLogParamUnsigned: {
						pData = unpack.GetData(sizeof(unsigned));
						if(!pData)
							continue;
						
						unsigned u = *(unsigned *)pData;

						// log event name
						databuf.Add(event->fields[i].name, StrLen(event->fields[i].name));
						databuf.Add(0);

						// log event data
						StrPrintf(data, arrsize(data), L"%d", u);
						databuf.Add(data, StrLen(data));
						databuf.Add(0);
					}
					break;

					case kLogParamFloat: {
						pData = unpack.GetData(sizeof(float));
						if(!pData)
							continue;

						float f = *(float *)pData;
							
						// log event name
						databuf.Add(event->fields[i].name, StrLen(event->fields[i].name));
						databuf.Add(0);

						// log event data
						StrPrintf(data, arrsize(data), L"%f", f);
						databuf.Add(data, StrLen(data));
						databuf.Add(0);
					}
					break;

					case kLogParamLong: {
						pData = unpack.GetData(sizeof(long));
						if(!pData)
							continue;

						long l = *(long *)pData;
							
						// log event name
						databuf.Add(event->fields[i].name, StrLen(event->fields[i].name));
						databuf.Add(0);

						// log event data
						StrPrintf(data, arrsize(data), L"%ld", l);
						databuf.Add(data, StrLen(data));
						databuf.Add(0);
					}
					break;

					case kLogParamLongLong: {
						pData = unpack.GetData(sizeof(long long));
						if(!pData)
							continue;

						long long ll = *(long long *)pData;

						// log event name
						databuf.Add(event->fields[i].name, StrLen(event->fields[i].name));
						databuf.Add(0);

						// log event data
						StrPrintf(data, arrsize(data), L"%lld", ll);
						databuf.Add(data, StrLen(data));
						databuf.Add(0);
					}
					break;

					case kLogParamUuid: {
						pData = unpack.GetData(sizeof(Uuid));
						if(!pData)
							continue;
						
						Uuid uuid = *(Uuid *)pData;

						// log event name
						databuf.Add(event->fields[i].name, StrLen(event->fields[i].name));
						databuf.Add(0);

						// log event data
						GuidToString(uuid, data, arrsize(data));
						databuf.Add(data, StrLen(data));
						databuf.Add(0);
					}
					break;
					
					case kLogParamStringW: {
						const wchar *str = unpack.GetString();
						if(!str) {
							continue;
						}

						// log event name
						databuf.Add(event->fields[i].name, StrLen(event->fields[i].name));
						databuf.Add(0);

						// log event data
						databuf.Add(str, StrLen(str));
						databuf.Add(0);
					}
					break;
				}
			}
		}
		databuf.Add(0);

		if(NetLogSrvCallback) {
			NetLogSrvCallback(
				event,
				databuf,
				buildId,
				addr, 
				msg.timestamp,
				productId,
				buildType
			);
		}
	}
	else
	{
		LogMsg(kLogError, "Unable to log event - event not found. type: %d from server: %d. If it is a new event the log server needs to be updated.", msg.eventType, srvType);
	}
	
	return true;
}


/*****************************************************************************
*
*   Module exports
*
***/

//============================================================================
void NetLogSrvInitialize () {
	s_running = true;
    AsyncSocketRegisterNotifyProc(
        kConnTypeSrvToLog,
        SocketNotifyProc,
        0,  // Accept all buildIds
        0,  // Accept all buildTypes
        0,	// Accept all branchIds
        0   // Accept all product Ids
    );
	IniChangeAdd(L"Log", IniChangeCallback, &s_change);
}

//============================================================================
void NetLogSrvShutdown () {
	if(s_change) {
		IniChangeRemove(s_change, true);
		s_change = false;
	}

	s_running = false;
    AsyncSocketUnregisterNotifyProc(
        kConnTypeSrvToLog,
        SocketNotifyProc,
        0,  // Accept all buildIds
        0,
        0,	// Accept all branchIds
        0
    );
}

//============================================================================
void NetLogSrvDestroy () {
 
	while(s_perf[kNlSrvPerfConnCount])
		AsyncSleep(10);
}

//============================================================================
void NetLogSrvRegisterCallback( void (*NlSrvCallback)(const NetLogEvent *, const ARRAY(wchar) &, unsigned, NetAddressNode &, qword, unsigned, unsigned )) {
	NetLogSrvCallback = NlSrvCallback;
}

//============================================================================
void LogConnIncRef (LogConn * conn) {
    conn->IncRef();
}

//============================================================================
void LogConnDecRef (LogConn * conn) {
    conn->DecRef();
}


/*****************************************************************************
*
*   Public exports
*
***/

//============================================================================
long NetLogSrvGetPerf (unsigned index) {
    ASSERT(index < kNlSrvNumPerf);
    return s_perf[index];
}
