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
*   $/Plasma20/Sources/Plasma/FeatureLib/pfCsrSrv/pfCsrSrv.cpp
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

static bool s_running;


/*****************************************************************************
*
*   Local functions
*
***/

//============================================================================
static bool Recv_ExecConsoleCmd (
	SimpleNetConn * ,
	CsrNet_ExecConsoleCmd * msg
) {
	LogMsg(kLogPerf, L"pfCsrSrv: ExecConsoleCmd: %S", msg->cmd);

	pfConsole::RunCommandAsync(msg->cmd);
	
	return true;
}

//============================================================================
static bool OnMsg (
	SimpleNetConn * conn,
	SimpleNet_MsgHeader * msg
) {
	bool result;
	
	#define DISPATCH(a) case kCsrNet_##a: result = Recv_##a(conn, (CsrNet_##a *) msg); break
	switch (msg->messageId) {
		DISPATCH(ExecConsoleCmd);
		default:
			result = false;
	}
	#undef DISPATCH
	
	return result;
}

//============================================================================
static void OnError (
	SimpleNetConn *	,
	ENetError		error
) {
	LogMsg(kLogPerf, L"pfCsrSrv NetError: %s", NetErrorToString(error));
}

//============================================================================
static bool QueryAccept (
	void *				,
	unsigned			channel,
	SimpleNetConn *		,
	const NetAddress &	addr
) {
	wchar str[64];
	NetAddressToString(addr, str, arrsize(str), kNetAddressFormatAll);
	LogMsg(kLogPerf, L"pfCsrSrv: Accepted connection from %s", str);
	return channel == kSimpleNetChannelCsr;
}


/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
void CsrSrvInitialize () {

#ifdef PLASMA_ENABLE_CSR_EXTERNAL

	LogMsg(kLogPerf, L"pfCsrSrv: Initializing");
	
	s_running = true;

	SimpleNetInitialize();
	SimpleNetCreateChannel(kSimpleNetChannelCsr, OnMsg, OnError);	
	SimpleNetStartListening(QueryAccept, nil);

#endif
}

//============================================================================
void CsrSrvShutdown () {

#ifdef PLASMA_ENABLE_CSR_EXTERNAL

	LogMsg(kLogPerf, L"pfCsrSrv: Shutting down");

	s_running = false;

	SimpleNetStopListening();
	SimpleNetDestroyChannel(kSimpleNetChannelCsr);
	SimpleNetShutdown();

#endif
}
