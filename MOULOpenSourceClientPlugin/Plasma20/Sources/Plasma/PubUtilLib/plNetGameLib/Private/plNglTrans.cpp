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
*   $/Plasma20/Sources/Plasma/PubUtilLib/plNetGameLib/Private/plNglTrans.cpp
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

enum {
	kPerfCurrTransactions,
	kNumPerf
};

static const unsigned kDefaultTimeoutMs	= 5 * 60 * 1000;

static bool							s_running;
static CCritSect                    s_critsect;
static LISTDECL(NetTrans, m_link)   s_transactions;
static long							s_perf[kNumPerf];
static unsigned						s_timeoutMs = kDefaultTimeoutMs;


/*****************************************************************************
*
*   Internal functions
*
***/

//============================================================================
static NetTrans * FindTransIncRef_CS (unsigned transId, const char tag[]) {
    // There shouldn't be more than a few transactions; just do a linear scan
    for (NetTrans * trans = s_transactions.Head(); trans; trans = s_transactions.Next(trans))
        if (trans->m_transId == transId) {
            trans->IncRef(tag);
            return trans;
        }

    return nil;
}

//============================================================================
static NetTrans * FindTransIncRef (unsigned transId, const char tag[]) {
    NetTrans * trans;
    s_critsect.Enter();
    {
        trans = FindTransIncRef_CS(transId, tag);
    }
    s_critsect.Leave();
    return trans;
}

//============================================================================
static void CancelTrans_CS (NetTrans * trans, ENetError error) {
	ASSERT(IS_NET_ERROR(error));
    if (trans->m_state != kTransStateComplete) {
        trans->m_result = error;
        trans->m_state  = kTransStateComplete;
    }
}


/*****************************************************************************
*
*   NetTrans implementation
*
***/

//============================================================================
NetTrans::NetTrans (ENetProtocol protocol, ETransType transType)
:   m_state(kTransStateWaitServerConnect)
,   m_result(kNetPending)
,   m_transId(0)
,   m_connId(0)
,   m_protocol(protocol)
,	m_hasSubTrans(false)
,	m_transType(transType)
{
	AtomicAdd(&s_perf[kPerfCurrTransactions], 1);
	AtomicAdd(&s_perfTransCount[m_transType], 1);
//	DebugMsg("%s@%p created", s_transTypes[m_transType], this);
}

//============================================================================
NetTrans::~NetTrans () {
    ASSERT(!m_link.IsLinked());
	AtomicAdd(&s_perfTransCount[m_transType], -1);
	AtomicAdd(&s_perf[kPerfCurrTransactions], -1);
//	DebugMsg("%s@%p destroyed", s_transTypes[m_transType], this);
}

//============================================================================
bool NetTrans::CanStart () const {
    switch (m_protocol) {
        case kNetProtocolCli2Auth: return AuthQueryConnected();
        case kNetProtocolCli2Game: return GameQueryConnected();
		case kNetProtocolCli2File: return FileQueryConnected();
		case kNetProtocolCli2Csr:  return CsrQueryConnected();
		case kNetProtocolCli2GateKeeper: return GateKeeperQueryConnected();
        DEFAULT_FATAL(m_protocol);
    }
}


/*****************************************************************************
*
*   Module functions
*
***/

//============================================================================
void NetTransInitialize () {
    s_critsect.Enter();
    {
		s_running = true;
    }
    s_critsect.Leave();
}

//============================================================================
void NetTransDestroy (bool wait) {
    s_critsect.Enter();
    {
		s_running = false;
    }
    s_critsect.Leave();

	NetTransCancelAll(kNetErrRemoteShutdown);
	
	if (!wait)
		return;
		
	while (s_perf[kPerfCurrTransactions]) {
		NetTransUpdate();
		AsyncSleep(10);
	}
}

//============================================================================
void NetTransSetTimeoutMs (unsigned ms) {
	s_timeoutMs = ms ? ms : kDefaultTimeoutMs;
}

//============================================================================
unsigned NetTransGetTimeoutMs () {
	return s_timeoutMs;
}

//============================================================================
void NetTransSend (NetTrans * trans) {
    trans->IncRef("Lifetime");
    s_critsect.Enter();
    {
        static unsigned s_transId;
        while (!trans->m_transId)
            trans->m_transId = ++s_transId;
        s_transactions.Link(trans, kListTail);
        if (!s_running)
			CancelTrans_CS(trans, kNetErrRemoteShutdown);
    }
    s_critsect.Leave();
}

//============================================================================
bool NetTransRecv (unsigned transId, const byte msg[], unsigned bytes) {
    NetTrans * trans = FindTransIncRef(transId, "Recv");

    if (!trans)
        return true;    // transaction was canceled.

	// Update the timeout time
	trans->m_timeoutAtMs = TimeGetMs() + s_timeoutMs;

    bool result = trans->Recv(msg, bytes);

    if (!result)
        NetTransCancel(transId, kNetErrInternalError);

    trans->DecRef("Recv");
    return result;
}

//============================================================================
void NetTransCancel (unsigned transId, ENetError error) {
    s_critsect.Enter();
    {
        NetTrans * trans = s_transactions.Head();
        for (; trans; trans = trans->m_link.Next()) {
            if (trans->m_transId == transId) {
                CancelTrans_CS(trans, error);
                break;
            }
        }
    }
    s_critsect.Leave();
}

//============================================================================
void NetTransCancelByProtocol (ENetProtocol protocol, ENetError error) {
    s_critsect.Enter();
    {
        NetTrans * trans = s_transactions.Head();
        for (; trans; trans = trans->m_link.Next()) {
            if (trans->m_protocol == protocol)
                CancelTrans_CS(trans, error);
        }
    }
    s_critsect.Leave();
}

//============================================================================
void NetTransCancelByConnId (unsigned connId, ENetError error) {
    s_critsect.Enter();
    {
        NetTrans * trans = s_transactions.Head();
        for (; trans; trans = trans->m_link.Next()) {
            if (trans->m_connId == connId)
                CancelTrans_CS(trans, error);
        }
    }
    s_critsect.Leave();
}

//============================================================================
void NetTransCancelAll (ENetError error) {
    s_critsect.Enter();
    {
        NetTrans * trans = s_transactions.Head();
        for (; trans; trans = trans->m_link.Next())
            CancelTrans_CS(trans, error);
    }
    s_critsect.Leave();
}

//============================================================================
void NetTransUpdate () {
	LISTDECL(NetTrans, m_link) completed;
	LISTDECL(NetTrans, m_link) parentCompleted;

    s_critsect.Enter();
    NetTrans * next, * trans = s_transactions.Head();
    for (; trans; trans = next) {
        next = s_transactions.Next(trans);

		bool done = false;
		while (!done) {
			switch (trans->m_state) {
				case kTransStateComplete:
					if (trans->m_hasSubTrans)
						parentCompleted.Link(trans);
					else
						completed.Link(trans);
					done = true;
				break;

				case kTransStateWaitServerConnect:
					if (!trans->CanStart()) {
						done = true;
						break;
					}
					if (trans->m_protocol && 0 == (trans->m_connId = ConnGetId(trans->m_protocol))) {
						done = true;
						break;
					}
					// This is the default "next state", trans->Send() can override this
					trans->m_state = kTransStateWaitServerResponse;
					// Set timeout time before calling Send(), allowing Send() to change it if it wants to.
					trans->m_timeoutAtMs = TimeGetMs() + s_timeoutMs;
					if (!trans->Send()) {
						// Revert back to current state so that we'll attempt to send again
						trans->m_state = kTransStateWaitServerConnect;
						done = true;
						break;
					}
				break;
				
				case kTransStateWaitServerResponse:
					// Check for timeout
					if ((int)(TimeGetMs() - trans->m_timeoutAtMs) > 0) {
						// Check to see if the transaction wants to "abort" the timeout
						if (trans->TimedOut())
							CancelTrans_CS(trans, kNetErrTimeout);
						else
							trans->m_timeoutAtMs = TimeGetMs() + s_timeoutMs; // Reset the timeout counter
					}
					done = true;
				break;

				DEFAULT_FATAL(trans->m_state);
			}
		}
    }
    s_critsect.Leave();

	// Post completed transactions    
    while (NetTrans * trans = completed.Head()) {
		completed.Unlink(trans);
		trans->Post();
		trans->DecRef("Lifetime");
	}
	// Post completed parent transactions
    while (NetTrans * trans = parentCompleted.Head()) {
		parentCompleted.Unlink(trans);
		trans->Post();
		trans->DecRef("Lifetime");
	}
}


} // namespace Ngl
