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
*   $/Plasma20/Sources/Plasma/PubUtilLib/plNetGameLib/Private/plNglTrans.cpp
*   
***/

#include "../Pch.h"

#include <list>

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

static const unsigned kDefaultTimeoutMs = 5 * 60 * 1000;

static bool                         s_running;
static std::recursive_mutex         s_critsect;
static std::list<NetTrans*> s_transactions;
static std::atomic<long>            s_perf[kNumPerf];
static unsigned                     s_timeoutMs = kDefaultTimeoutMs;


/*****************************************************************************
*
*   Internal functions
*
***/

//============================================================================
static NetTrans * FindTransIncRef_CS (unsigned transId, const char tag[]) {
    // There shouldn't be more than a few transactions; just do a linear scan
    for (NetTrans* trans : s_transactions) {
        if (trans->m_transId == transId) {
            trans->Ref(tag);
            return trans;
        }
    }

    return nullptr;
}

//============================================================================
static NetTrans * FindTransIncRef (unsigned transId, const char tag[]) {
    hsLockGuard(s_critsect);
    return FindTransIncRef_CS(transId, tag);
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
:   hsRefCnt(0)
,   m_state(kTransStateWaitServerConnect)
,   m_result(kNetPending)
,   m_transId(0)
,   m_connId(0)
,   m_protocol(protocol)
,   m_hasSubTrans(false)
,   m_timeoutAtMs(0)
,   m_transType(transType)
{
    ++s_perf[kPerfCurrTransactions];
    ++s_perfTransCount[m_transType];
//  hsDebugPrintToTerminal("%s@%p created", s_transTypes[m_transType], this);
}

//============================================================================
NetTrans::~NetTrans () {
#if defined(HS_DEBUGGING)
    {
        hsLockGuard(s_critsect);
        hsAssert(
            std::find(s_transactions.begin(), s_transactions.end(), this) == s_transactions.end(),
            "Destroying a transaction that's still in progress!"
        );
    }
#endif
    --s_perfTransCount[m_transType];
    --s_perf[kPerfCurrTransactions];
//  hsDebugPrintToTerminal("%s@%p destroyed", s_transTypes[m_transType], this);
}

//============================================================================
bool NetTrans::CanStart () const {
    switch (m_protocol) {
        case kNetProtocolCli2Auth: return AuthQueryConnected();
        case kNetProtocolCli2Game: return GameQueryConnected();
        case kNetProtocolCli2File: return FileQueryConnected();
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
    hsLockGuard(s_critsect);
    s_running = true;
}

//============================================================================
void NetTransDestroy (bool wait) {
    {
        hsLockGuard(s_critsect);
        s_running = false;
    }

    NetTransCancelAll(kNetErrRemoteShutdown);
    
    if (!wait)
        return;
        
    while (s_perf[kPerfCurrTransactions]) {
        NetTransUpdate();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
    trans->Ref("Lifetime");
    hsLockGuard(s_critsect);
    static unsigned s_transId;
    while (!trans->m_transId)
        trans->m_transId = ++s_transId;
    s_transactions.push_back(trans);
    if (!s_running)
        CancelTrans_CS(trans, kNetErrRemoteShutdown);
}

//============================================================================
bool NetTransRecv (unsigned transId, const uint8_t msg[], unsigned bytes) {
    NetTrans * trans = FindTransIncRef(transId, "Recv");

    if (!trans)
        return true;    // transaction was canceled.

    // Update the timeout time
    trans->m_timeoutAtMs = hsTimer::GetMilliSeconds<uint32_t>() + s_timeoutMs;

    bool result = trans->Recv(msg, bytes);

    if (!result)
        NetTransCancel(transId, kNetErrInternalError);

    trans->UnRef("Recv");
    return result;
}

//============================================================================
void NetTransCancel (unsigned transId, ENetError error) {
    hsLockGuard(s_critsect);
    for (NetTrans* trans : s_transactions) {
        if (trans->m_transId == transId) {
            CancelTrans_CS(trans, error);
            break;
        }
    }
}

//============================================================================
void NetTransCancelByProtocol (ENetProtocol protocol, ENetError error) {
    hsLockGuard(s_critsect);
    for (NetTrans* trans : s_transactions) {
        if (trans->m_protocol == protocol)
            CancelTrans_CS(trans, error);
    }
}

//============================================================================
void NetTransCancelByConnId (unsigned connId, ENetError error) {
    hsLockGuard(s_critsect);
    for (NetTrans* trans : s_transactions) {
        if (trans->m_connId == connId)
            CancelTrans_CS(trans, error);
    }
}

//============================================================================
void NetTransCancelAll (ENetError error) {
    hsLockGuard(s_critsect);
    for (NetTrans* trans : s_transactions) {
        CancelTrans_CS(trans, error);
    }
}

//============================================================================
void NetTransUpdate () {
    std::list<NetTrans*> completed;
    std::list<NetTrans*> parentCompleted;

    {
        hsLockGuard(s_critsect);

        for (auto it = s_transactions.begin(); it != s_transactions.end();) {
            NetTrans* trans = *it;
            // Increment a copy of the iterator here already,
            // because the original iterator's meaning may be changed by a splice call below.
            auto next = it;
            next++;

            bool done = false;
            while (!done) {
                switch (trans->m_state) {
                    case kTransStateComplete:
                        // Move the completed transaction out of s_transactions.
                        if (trans->m_hasSubTrans) {
                            parentCompleted.splice(parentCompleted.end(), s_transactions, it);
                        } else {
                            completed.splice(completed.end(), s_transactions, it);
                        }
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
                        trans->m_timeoutAtMs = hsTimer::GetMilliSeconds<uint32_t>() + s_timeoutMs;
                        if (!trans->Send()) {
                            // Revert back to current state so that we'll attempt to send again
                            trans->m_state = kTransStateWaitServerConnect;
                            done = true;
                            break;
                        }
                    break;

                    case kTransStateWaitServerResponse:
                        // Check for timeout
                        if ((int)(hsTimer::GetMilliSeconds<uint32_t>() - trans->m_timeoutAtMs) > 0) {
                            // Check to see if the transaction wants to "abort" the timeout
                            if (trans->TimedOut())
                                CancelTrans_CS(trans, kNetErrTimeout);
                            else
                                trans->m_timeoutAtMs = hsTimer::GetMilliSeconds<uint32_t>() + s_timeoutMs; // Reset the timeout counter
                        }
                        done = true;
                    break;

                    DEFAULT_FATAL(trans->m_state);
                }
            }

            it = next;
        }
    }

    // Post completed transactions
    for (NetTrans* trans : completed) {
        trans->Post();
        trans->UnRef("Lifetime");
    }
    // Post completed parent transactions
    for (NetTrans* trans : parentCompleted) {
        trans->Post();
        trans->UnRef("Lifetime");
    }
}


} // namespace Ngl
