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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnCsrCli/pnCsrCli.cpp
*   
***/

#include "Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Local types
*
***/


/*****************************************************************************
*
*   Local data
*
***/

static bool						s_running;
static CCritSect				s_critsect;
static SimpleNetConn *			s_conn;
static FCsrCliOnError			s_onError;


/*****************************************************************************
*
*   Local functions
*
***/

//============================================================================
static SimpleNetConn * GetConnIncRef () {

	SimpleNetConn * conn;
	s_critsect.Enter();
	{
		if (nil != (conn = s_conn))
			SimpleNetConnIncRef(conn);
	}
	s_critsect.Leave();
	return conn;
}

//============================================================================
static bool SimpleNetOnMsg (
	SimpleNetConn *			,
	SimpleNet_MsgHeader *	
) {
	LogMsg(kLogPerf, L"pnCsrCli: Rcvd unexpected message from peer");
	return false;
}

//============================================================================
static void SimpleNetOnError (
	SimpleNetConn *	,
	ENetError		error
) {
	if (!s_running)
		return;
		
	LogMsg(kLogPerf, L"pnCsrCli: NetError: %s", NetErrorToString(error));

	if (error == kNetErrDisconnected)
		CsrCliDisconnect();
		
	s_onError(error);
}

//============================================================================
static void SimpleNetOnConnect (
	void *			param,
	SimpleNetConn *	conn,
	ENetError		result
) {
	FCsrCliOnConnect onConnect = (FCsrCliOnConnect)param;
	
	LogMsg(kLogPerf, L"pnCsrCli: OnConnect: %s", NetErrorToString(result));
	
	if (s_conn)
		CsrCliDisconnect();
		
	if (IS_NET_SUCCESS(result)) {
		s_critsect.Enter();
		{
			s_conn = conn;
		}
		s_critsect.Leave();
	}
	
	if (onConnect)
		onConnect(result);
}

//============================================================================
static void Send_ExecConsoleCmd (const char cmd[]) {

	SimpleNetConn * conn = GetConnIncRef();
	if (!conn)
		return;
		
	unsigned cmdBytes = StrBytes(cmd);
	
	CsrNet_ExecConsoleCmd * msg;

	unsigned msgBytes
		= sizeof(*msg)
		- sizeof(msg->cmd)
		+ cmdBytes
		+ sizeof(cmd[0])
		;
		
	msg = new(_alloca(msgBytes)) CsrNet_ExecConsoleCmd();
	msg->messageBytes = msgBytes;
	
	StrCopy(msg->cmd, cmd, cmdBytes / sizeof(cmd[0]));
	msg->cmd[cmdBytes] = 0;
	
	SimpleNetSend(conn, msg);
	SimpleNetConnDecRef(conn);	
}


/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
void CsrCliInitialize (FCsrCliOnError onError) {

	ASSERT(!s_running);
	ASSERT(onError);
	
	s_running = true;
	s_onError = onError;
	
	SimpleNetInitialize();
	SimpleNetCreateChannel(kSimpleNetChannelCsr, SimpleNetOnMsg, SimpleNetOnError);
};

//============================================================================
void CsrCliShutdown () {

	s_running = false;
	s_onError = nil;

	CsrCliDisconnect();	
	SimpleNetDestroyChannel(kSimpleNetChannelCsr);
	SimpleNetShutdown();
}

//============================================================================
void CsrCliStartConnecting (
	const wchar addr[],
	FCsrCliOnConnect onConnect
) {
	ASSERT(s_running);

	CsrCliDisconnect();	
	SimpleNetStartConnecting(kSimpleNetChannelCsr, addr, SimpleNetOnConnect, onConnect);
}

//============================================================================
void CsrCliDisconnect () {

	SimpleNetConn * conn = nil;
	s_critsect.Enter();
	{
		SWAP(conn, s_conn);
	}
	s_critsect.Leave();
	if (conn)
		SimpleNetDisconnect(conn);
}

//============================================================================
void CsrCliToggleAvatarPhysical () {

	ASSERT(s_running);
	
	Send_ExecConsoleCmd("Avatar.Physics.TogglePhysical");
}

//============================================================================
void CsrCliWarpPlayerHere (unsigned playerId) {

	ASSERT(s_running);
	
	char cmd[1024];
	StrPrintf(cmd, arrsize(cmd), "CCR.WarpPlayerHere %u", playerId);
	Send_ExecConsoleCmd(cmd);
}

//============================================================================
void CsrCliWarpToPlayer (unsigned playerId) {

	ASSERT(s_running);

	char cmd[1024];
	StrPrintf(cmd, arrsize(cmd), "CCR.WarpToPlayer %u", playerId);
	Send_ExecConsoleCmd(cmd);
}

//============================================================================
void CsrCliLinkPlayerHere (unsigned playerId) {

	ASSERT(s_running);

	char cmd[1024];
	StrPrintf(cmd, arrsize(cmd), "CCR.LinkPlayerHere %u", playerId);
	Send_ExecConsoleCmd(cmd);
}

//============================================================================
void CsrCliLinkToPlayer (unsigned playerId) {

	ASSERT(s_running);

	char cmd[1024];
	StrPrintf(cmd, arrsize(cmd), "CCR.LinkToPlayer %u", playerId);
	Send_ExecConsoleCmd(cmd);
}
