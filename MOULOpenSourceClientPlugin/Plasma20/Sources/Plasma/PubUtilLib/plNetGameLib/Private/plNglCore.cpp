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
*   $/Plasma20/Sources/Plasma/PubUtilLib/plNetGameLib/Private/plNglCore.cpp
*   
***/

#include "../Pch.h"
#pragma hdrstop


namespace Ngl {
/*****************************************************************************
*
*   Private
*
***/


struct ReportNetErrorTrans : NetNotifyTrans {
    ENetProtocol        m_errProtocol;
    ENetError           m_errError;

    ReportNetErrorTrans (
        ENetProtocol    errProtocol,
        ENetError       errError
    );

    void Post ();
};


/*****************************************************************************
*
*   Private data
*
***/

static FNetClientErrorProc	s_errorProc;
static long					s_initCount;


/*****************************************************************************
*
*   Local functions
*
***/


/*****************************************************************************
*
*   Transactions
*
***/

//============================================================================
// NetNotifyTrans
//============================================================================
NetNotifyTrans::NetNotifyTrans (ETransType transType)
:   NetTrans(kNetProtocolNil, transType)
{
}

//============================================================================
// ReportNetErrorTrans
//============================================================================
ReportNetErrorTrans::ReportNetErrorTrans (
    ENetProtocol    errProtocol,
    ENetError       errError
) : NetNotifyTrans(kReportNetErrorTrans)
,	m_errProtocol(errProtocol)
,   m_errError(errError)
{ }

//============================================================================
void ReportNetErrorTrans::Post () {
    if (s_errorProc)
        s_errorProc(m_errProtocol, m_errError);
}


/*****************************************************************************
*
*   Module functions
*
***/

//============================================================================
void ReportNetError (ENetProtocol protocol, ENetError error) {
    ReportNetErrorTrans * trans = NEW(ReportNetErrorTrans)(protocol, error);
    NetTransSend(trans);
}

} using namespace Ngl;


/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
void NetClientInitialize () {
    
    if (0 == AtomicAdd(&s_initCount, 1)) {
		NetTransInitialize();
		AuthInitialize();
		GameInitialize();
		FileInitialize();
		CsrInitialize();
		GateKeeperInitialize();
	}
}

//============================================================================
void NetClientCancelAllTrans () {
	NetTransCancelAll(kNetErrTimeout);
}

//============================================================================
void NetClientDestroy (bool wait) {

    if (1 == AtomicAdd(&s_initCount, -1)) {
		s_errorProc = nil;

		GateKeeperDestroy(false);
		CsrDestroy(false);
		FileDestroy(false);
		GameDestroy(false);
		AuthDestroy(false);
		NetTransDestroy(false);
		if (wait) {
			GateKeeperDestroy(true);
			CsrDestroy(true);
			FileDestroy(true);
			GameDestroy(true);
			AuthDestroy(true);
			NetTransDestroy(true);
		}
	}
}

//============================================================================
void NetClientUpdate () {
    NetTransUpdate();
}

//============================================================================
void NetClientSetTransTimeoutMs (unsigned ms) {
	NetTransSetTimeoutMs(ms);
}

//============================================================================
void NetClientPingEnable (bool enable) {
	AuthPingEnable(enable);
	GamePingEnable(enable);
}

//============================================================================
void NetClientSetErrorHandler (FNetClientErrorProc errorProc) {
	s_errorProc = errorProc;
}
