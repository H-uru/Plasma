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

#include "HeadSpin.h"
#include "hsResMgr.h"
#include "plDispatch.h"
#include "pnFactory/plFactory.h"
#define PLMESSAGE_PRIVATE
#include "pnMessage/plMessage.h"
#include "pnKeyedObject/hsKeyedObject.h"
#include "hsTimer.h"
#include "pnMessage/plTimeMsg.h"
#include "pnKeyedObject/plKey.h"
#include "plDispatchLogBase.h"
#include "pnNetCommon/plNetApp.h"
#include "pnNetCommon/plSynchedObject.h"
#include "pnNetCommon/pnNetCommon.h"
#include "hsThread.h"
#include "plProfile.h"

plProfile_CreateTimer("MsgReceive", "Update", MsgReceive);
plProfile_CreateTimer("  TimeMsg", "Update", TimeMsg);
plProfile_CreateTimer("  EvalMsg", "Update", EvalMsg);
plProfile_CreateTimer("  TransformMsg", "Update", TransformMsg);
plProfile_CreateTimer("  CameraMsg", "Update", CameraMsg);

class plMsgWrap
{
public:
    plMsgWrap**                     fBack;
    plMsgWrap*                      fNext;
    std::vector<plKey>              fReceivers;

    plMessage*                      fMsg;

    plMsgWrap(plMessage* msg)
        : fMsg(msg), fNext(), fBack()
    { hsRefCnt_SafeRef(msg); }
    virtual ~plMsgWrap() { hsRefCnt_SafeUnRef(fMsg); }

    plMsgWrap&      ClearReceivers() { fReceivers.clear(); return *this; }
    plMsgWrap&      AddReceiver(plKey rcv)
                    {
                        hsAssert(rcv, "Trying to send mail to nil receiver");
                        fReceivers.emplace_back(std::move(rcv));
                        return *this;
                    }
    const plKey&    GetReceiver(size_t i) const { return fReceivers[i]; }
    size_t          GetNumReceivers() const { return fReceivers.size(); }
};

int32_t                 plDispatch::fNumBufferReq = 0;
bool                    plDispatch::fMsgActive = false;
plMsgWrap*              plDispatch::fMsgCurrent = nullptr;
plMsgWrap*              plDispatch::fMsgHead = nullptr;
plMsgWrap*              plDispatch::fMsgTail = nullptr;
std::vector<plMessage*> plDispatch::fMsgWatch;
MsgRecieveCallback      plDispatch::fMsgRecieveCallback = nullptr;

std::mutex              plDispatch::fMsgCurrentMutex; // mutex for fMsgCurrent
std::mutex              plDispatch::fMsgDispatchLock; // mutex for IMsgDispatch


plDispatch::plDispatch()
: fOwner(), fFutureMsgQueue(), fQueuedMsgOn(true)
{
}

plDispatch::~plDispatch()
{
    hsAssert(fRegisteredExactTypes.empty(), "registered type after Dispatch shutdown");
    ITrashUndelivered();
}

void plDispatch::BeginShutdown()
{
    for (plTypeFilter* type : fRegisteredExactTypes)
        delete type;
    fRegisteredExactTypes.clear();
    ITrashUndelivered();
}

void plDispatch::ITrashUndelivered()
{
    while( fFutureMsgQueue )
    {
        plMsgWrap* nuke = fFutureMsgQueue;
        fFutureMsgQueue = fFutureMsgQueue->fNext;
        hsRefCnt_SafeUnRef(nuke->fMsg);
        delete nuke;
    }

    // If we're the main dispatch, any unsent messages at this
    // point are just trashed. Slave dispatches just go away and
    // leave their messages to be delivered when the main dispatch
    // gets around to it.
    if( this == plgDispatch::Dispatch() )
    {
        while( fMsgHead )
        {
            plMsgWrap* nuke = fMsgHead;
            fMsgHead = fMsgHead->fNext;
            // hsRefCnt_SafeUnRef(nuke->fMsg);      // MOOSE - done in plMsgWrap dtor
            delete nuke;
        }

        // reset static members which we just deleted - MOOSE
        fMsgCurrent = fMsgHead = fMsgTail = nullptr;

        fMsgActive = false;
    }
}

plMsgWrap* plDispatch::IInsertToQueue(plMsgWrap** curr, plMsgWrap* isert)
{
    isert->fNext = *curr;
    isert->fBack = curr;
    if( *curr )
        (*curr)->fBack = &isert->fNext;
    *curr = isert;
    return isert;
}

plMsgWrap* plDispatch::IDequeue(plMsgWrap** head, plMsgWrap** tail)
{
    plMsgWrap* retVal = *head;
    if( *head )
    {
        *head = (*head)->fNext;
        if( *head )
            (*head)->fBack = head;
    }
    if( tail && (*tail == retVal) )
        *tail = *head;
    return retVal;
}

bool plDispatch::ISortToDeferred(plMessage* msg)
{
    plMsgWrap* msgWrap = new plMsgWrap(msg);
    if( !fFutureMsgQueue )
    {
        if( IGetOwner() )
            plgDispatch::Dispatch()->RegisterForExactType(plTimeMsg::Index(), IGetOwnerKey());

        IInsertToQueue(&fFutureMsgQueue, msgWrap);
        return false;
    }
    if( fFutureMsgQueue->fMsg->fTimeStamp > msgWrap->fMsg->fTimeStamp )
    {
        IInsertToQueue(&fFutureMsgQueue, msgWrap);
        return false;
    }
    plMsgWrap* after = fFutureMsgQueue;
    while( after->fNext && (after->fNext->fMsg->fTimeStamp < msgWrap->fMsg->fTimeStamp) )
        after = after->fNext;

    IInsertToQueue(&after->fNext, msgWrap);

    return false;
}

void plDispatch::ICheckDeferred(double secs)
{
    while( fFutureMsgQueue && (fFutureMsgQueue->fMsg->fTimeStamp < secs) )
    {
        plMsgWrap* send = IDequeue(&fFutureMsgQueue, nullptr);
        MsgSend(send->fMsg);
        delete send;
    }

    uint16_t timeIdx = plTimeMsg::Index();
    if( IGetOwner()
        && !fFutureMsgQueue 
        && 
            ( 
                (timeIdx >= fRegisteredExactTypes.size())
                || 
                !fRegisteredExactTypes[plTimeMsg::Index()]
            )
      )
        plgDispatch::Dispatch()->UnRegisterForExactType(plTimeMsg::Index(), IGetOwnerKey());
}

bool plDispatch::IListeningForExactType(uint16_t hClass)
{
    if( (hClass == plTimeMsg::Index()) && fFutureMsgQueue )
        return true;

    return false;
}

void plDispatch::IMsgEnqueue(plMsgWrap* msgWrap, bool async)
{
    {
        hsLockGuard(fMsgCurrentMutex);

#ifdef HS_DEBUGGING
        if (msgWrap->fMsg->HasBCastFlag(plMessage::kMsgWatch))
            fMsgWatch.emplace_back(msgWrap->fMsg);
#endif // HS_DEBUGGING

        if (fMsgTail)
            fMsgTail = IInsertToQueue(&fMsgTail->fNext, msgWrap);
        else
            fMsgTail = IInsertToQueue(&fMsgHead, msgWrap);
    }

    if( !async )
        // Test for fMsgActive in IMsgDispatch(), properly wrapped inside a mutex -mcn
        IMsgDispatch();
}

// On starts deferring msg delivery until buffering is set to off again.
bool plDispatch::SetMsgBuffering(bool on)
{
    std::unique_lock<std::mutex> lock(fMsgCurrentMutex);
    if (on)
    {
        hsAssert(fNumBufferReq || !fMsgActive, "Can't start deferring message delivery while delivering messages. See mf");
        if (!fNumBufferReq && fMsgActive)
            return false;

        fNumBufferReq++;
        fMsgActive = true;
    }
    else if (!--fNumBufferReq)
    {
        fMsgActive = false;
        lock.unlock();
        IMsgDispatch();
    }
    hsAssert(fNumBufferReq >= 0, "Mismatched number of on/off dispatch buffering requests");

    return true;
}

void plDispatch::IMsgDispatch()
{
    std::unique_lock<std::mutex> dispatchLock(fMsgDispatchLock, std::try_to_lock);
    if (!dispatchLock.owns_lock())
        return;

    if (fMsgActive)
        return;

    fMsgActive = true;
    int responseLevel=0;

    std::unique_lock<std::mutex> msgCurrentLock(fMsgCurrentMutex);

    plMsgWrap* origTail = fMsgTail;
    while((fMsgCurrent = fMsgHead))
    {
        IDequeue(&fMsgHead, &fMsgTail);
        msgCurrentLock.unlock();

        plMessage* msg = fMsgCurrent->fMsg;
        bool nonLocalMsg = msg && msg->HasBCastFlag(plMessage::kNetNonLocal);

#ifdef HS_DEBUGGING
        auto watchIdx = std::find(fMsgWatch.cbegin(), fMsgWatch.cend(), msg);
        if (fMsgWatch.cend() != watchIdx)
        {
            fMsgWatch.erase(watchIdx);
#if HS_BUILD_FOR_WIN32
            __debugbreak();
#endif // HS_BUILD_FOR_WIN32
        }
#endif // HS_DEBUGGING

        static uint64_t startTicks = 0;
        if (plDispatchLogBase::IsLogging())
            startTicks = hsTimer::GetTicks();

        int numReceivers=0;
        for (size_t i = 0; fMsgCurrent && i < fMsgCurrent->GetNumReceivers(); i++)
        {
            const plKey& rcvKey = fMsgCurrent->GetReceiver(i);
            plReceiver* rcv = rcvKey ? plReceiver::ConvertNoRef(rcvKey->ObjectIsLoaded()) : nullptr;
            if( rcv )
            {
                if (nonLocalMsg)
                {
                    // localOnly objects should not get remote messages
                    plSynchedObject* synchedObj = plSynchedObject::ConvertNoRef(rcv);
                    if (synchedObj && !synchedObj->IsNetSynched() )
                    {
                        continue;
                    }

                    if (plNetObjectDebuggerBase::GetInstance())
                    {   // log net msg if this is a debug object
                        hsKeyedObject* ko = hsKeyedObject::ConvertNoRef(rcv);
                        if (plNetObjectDebuggerBase::GetInstance()->IsDebugObject(ko))
                        {
                            hsLogEntry(plNetObjectDebuggerBase::GetInstance()->LogMsg(
                                ST::format("<RCV> object:{}, GameMessage {} st={.3f} rt={.3f}",
                                ko->GetKeyName(), msg->ClassName(), hsTimer::GetSysSeconds(),
                                hsTimer::GetSeconds())));
                        }
                    }
                }

#ifndef PLASMA_EXTERNAL_RELEASE
                uint64_t rcvTicks = hsTimer::GetTicks();

                // Object could be deleted by this message, so we need to log this stuff now
                ST::string keyname = ST_LITERAL("(unknown)");
                const char* className = "(unknown)";
                uint32_t clonePlayerID = 0;
                if (plDispatchLogBase::IsLoggingLong())
                {
                    hsKeyedObject* ko = hsKeyedObject::ConvertNoRef(rcv);
                    if (ko)
                    {
                        keyname = ko->GetKeyName();
                        clonePlayerID = ko->GetKey()->GetUoid().GetClonePlayerID();
                        className = ko->ClassName();
                    }
                }
#endif // PLASMA_EXTERNAL_RELEASE

                #ifdef HS_DEBUGGING
                if (msg->GetBreakBeforeDispatch())
                    hsDebugBreakIfDebuggerPresent();
                #endif
                    
                plProfile_BeginTiming(MsgReceive);
                rcv->MsgReceive(msg);
                plProfile_EndTiming(MsgReceive);

#ifndef PLASMA_EXTERNAL_RELEASE
                if (plDispatchLogBase::IsLoggingLong())
                {
                    rcvTicks = hsTimer::GetTicks() - rcvTicks;

                    float rcvTime = hsTimer::GetMilliSeconds<float>(rcvTicks);
                    // If the receiver takes more than 5 ms to process its message, log it
                    if (rcvTime > 5.f)
                        plDispatchLogBase::GetInstance()->LogLongReceive(keyname.c_str(), className, clonePlayerID, msg, rcvTime);
                }
#endif // PLASMA_EXTERNAL_RELEASE

                numReceivers++;

                if (fMsgRecieveCallback != nullptr)
                    fMsgRecieveCallback();
            }
        }

        // for message logging
//      if (plDispatchLogBase::IsLogging())
//      {
//          float sendTime = hsTimer::GetMilliSeconds<float>(hsTimer::GetTicks() - startTicks);
//
//          plDispatchLogBase::GetInstance()->DumpMsg(msg, numReceivers, (int)sendTime, responseLevel*2 /* indent */);
//          if (origTail==fMsgCurrent)
//          {   // if we deliver more msgs after this, they must be response msgs
//              responseLevel++;
//              origTail = fMsgTail;
//          }
//      }

        msgCurrentLock.lock();

        delete fMsgCurrent;
        // TEMP
        fMsgCurrent = (class plMsgWrap *)(uintptr_t)0xdeadc0de;
    }

    fMsgActive = false;
}

//
// returns true if msg has been consumed and deleted
//
bool plDispatch::IMsgNetPropagate(plMessage* msg)
{
    {
        hsLockGuard(fMsgCurrentMutex);

        // Make sure cascaded messages all have the same net flags
        plNetClientApp::InheritNetMsgFlags(fMsgCurrent ? fMsgCurrent->fMsg : nullptr, msg, false);
    }

    // Decide if msg should go out over the network.
    // If kNetForce is used, this message should always go out over the network, even if it's already 
    // part of a net cascade. We still want to inherit net status flags (but ignore them)
    // so that response messages obey cascading rules.  In other words, we are not 
    // halting the cascade, just overriding the send rule for this message.
    if( msg->HasBCastFlag(plMessage::kNetPropagate) && 
        (!msg->HasBCastFlag(plMessage::kNetSent) || 
        msg->HasBCastFlag(plMessage::kNetForce) ||
        msg->HasBCastFlag(plMessage::kNetNonDeterministic) ||
        msg->HasBCastFlag(plMessage::kCCRSendToAllPlayers )) )
    {
        // send it off...
        hsAssert(!msg->HasBCastFlag(plMessage::kNetStartCascade), "initial net cascade msg getting sent over the net again?");
        if (plNetClientApp::GetInstance()) {
            plNetClientApp::GetInstance()->ISendGameMessage(msg);
            msg->SetBCastFlag(plMessage::kNetSent);
        }
    }

    // Decide if msg should get sent locally
    if (!msg->HasBCastFlag(plMessage::kLocalPropagate))
    {   
        hsRefCnt_SafeUnRef(msg);
        return true;
    }

    // since we've already checked this property, and the msg will be dispatched locally,
    // it should not start any more net cascades.
    msg->SetBCastFlag(plMessage::kNetStartCascade, false);

    return false;
}

bool plDispatch::MsgSend(plMessage* msg, bool async)
{
    if( IMsgNetPropagate(msg) )
        return true;

    plTimeMsg* timeMsg;
    if( msg->GetTimeStamp() > hsTimer::GetSysSeconds() )
        return ISortToDeferred(msg);
    else if((timeMsg = plTimeMsg::ConvertNoRef(msg)))
        ICheckDeferred(timeMsg->DSeconds());

    plMsgWrap* msgWrap = new plMsgWrap(msg);
    hsRefCnt_SafeUnRef(msg);

    // broadcast
    if( msg->HasBCastFlag(plMessage::kBCastByExactType) | msg->HasBCastFlag(plMessage::kBCastByType) )
    {
        uint16_t idx = msg->ClassIndex();
        if (idx < fRegisteredExactTypes.size())
        {
            plTypeFilter* filt = fRegisteredExactTypes[idx];
            if( filt )
            {
                for (const plKey& rcvr : filt->fReceivers)
                    msgWrap->AddReceiver(rcvr);

                if( msg->HasBCastFlag(plMessage::kClearAfterBCast) )
                {
                    delete filt;
                    fRegisteredExactTypes[idx] = nullptr;
                }
            }
        }
    }
    // Direct communique
    else
    if( msg->GetNumReceivers() )
    {
        msgWrap->fReceivers = msg->fReceivers;
    }
    IMsgEnqueue(msgWrap, async);

    return true;
}
void plDispatch::MsgQueueOnOff(bool sw)
{
    fQueuedMsgOn = sw;
}
void plDispatch::MsgQueue(plMessage* msg)
{
    if (fQueuedMsgOn)
    {
        hsLockGuard(fQueuedMsgListMutex);
        hsAssert(msg,"Message missing");
        fQueuedMsgList.push_back(msg);
    }
    else
        MsgSend(msg, false);
}

void plDispatch::MsgQueueProcess()
{
    // Process all messages on Queue, unlock while sending them
    // this would allow other threads to put new messages on the list while we send()
    bool empty = false;
    while (!empty)
    {
        plMessage * pMsg = nullptr;
        {
            hsLockGuard(fQueuedMsgListMutex);
            empty = fQueuedMsgList.empty();
            if (!empty)
            {
                pMsg = fQueuedMsgList.front();
                fQueuedMsgList.pop_front();
            }
        }
        if (pMsg)
            MsgSend(pMsg, false);
    }
}

void plDispatch::RegisterForType(uint16_t hClass, const plKey& receiver)
{
    int i;
    for( i = 0; i < plFactory::GetNumClasses(); i++ )
    {
        if( plFactory::DerivesFrom(hClass, i) )
            RegisterForExactType(i, receiver);
    }
}

void plDispatch::RegisterForExactType(uint16_t hClass, const plKey& receiver)
{
    if (hClass + 1 > fRegisteredExactTypes.size())
        fRegisteredExactTypes.resize(hClass + 1);
    plTypeFilter* filt = fRegisteredExactTypes[hClass];
    if( !filt )
    {
        filt = new plTypeFilter;
        fRegisteredExactTypes[hClass] = filt;
        filt->fHClass = hClass;
    }

    const auto iter = std::find(filt->fReceivers.begin(), filt->fReceivers.end(), receiver);
    if (iter == filt->fReceivers.end())
        filt->fReceivers.emplace_back(receiver);
}

void plDispatch::UnRegisterForType(uint16_t hClass, const plKey& receiver)
{
    for (size_t i = 0; i < fRegisteredExactTypes.size(); i++)
    {
        if (plFactory::DerivesFrom(hClass, uint16_t(i)))
            IUnRegisterForExactType(uint16_t(i), receiver);
    }
}

bool plDispatch::IUnRegisterForExactType(uint16_t idx, const plKey& receiver)
{
    hsAssert(idx < fRegisteredExactTypes.size(), "Out of range should be filtered before call to internal");
    plTypeFilter* filt = fRegisteredExactTypes[idx];
    if (!filt)
        return false;

    for (size_t i = 0; i < filt->fReceivers.size(); i++)
    {
        if (receiver == filt->fReceivers[i])
        {
            if (filt->fReceivers.size() > 1)
            {
                if (i < filt->fReceivers.size() - 1)
                    filt->fReceivers[i] = filt->fReceivers.back();
                filt->fReceivers.pop_back();
            }
            else
            {
                delete filt;
                fRegisteredExactTypes[idx] = nullptr;
            }

            break;
        }
    }
    return false;
}

void plDispatch::UnRegisterAll(const plKey& receiver)
{
    for (size_t i = 0; i < fRegisteredExactTypes.size(); i++)
    {
        plTypeFilter* filt = fRegisteredExactTypes[i];
        if( filt )
        {
            auto idx = std::find(filt->fReceivers.begin(), filt->fReceivers.end(), receiver);
            if (idx != filt->fReceivers.end())
            {
                if (filt->fReceivers.size() > 1)
                {
                    if (idx < filt->fReceivers.end() - 1)
                        *idx = filt->fReceivers.back();
                    filt->fReceivers.pop_back();
                }
                else
                {
                    delete filt;
                    fRegisteredExactTypes[i] = nullptr;
                }
            }
        }
    }
}

void plDispatch::UnRegisterForExactType(uint16_t hClass, const plKey& receiver)
{
    if (hClass >= fRegisteredExactTypes.size())
        return;
    plTypeFilter* filt = fRegisteredExactTypes[hClass];
    if( !filt )
        return;

    IUnRegisterForExactType(hClass, receiver);
}

