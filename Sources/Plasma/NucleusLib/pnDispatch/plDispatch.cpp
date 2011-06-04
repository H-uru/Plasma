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

#include "hsTypes.h"
#include "hsResMgr.h"
#include "plDispatch.h"
#define PLMESSAGE_PRIVATE
#include "../pnMessage/plMessage.h"
#include "../pnKeyedObject/hsKeyedObject.h"
#include "hsTimer.h"
#include "../pnMessage/plTimeMsg.h"
#include "../pnKeyedObject/plKey.h"
#include "plDispatchLogBase.h"
#include "../pnNetCommon/plNetApp.h"
#include "../pnNetCommon/plSynchedObject.h"
#include "../pnNetCommon/pnNetCommon.h"
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
	plMsgWrap**						fBack;
	plMsgWrap*						fNext;
	hsTArray<plKey>					fReceivers;

	plMessage*						fMsg;

	plMsgWrap(plMessage* msg) : fMsg(msg) { hsRefCnt_SafeRef(msg); }
	virtual ~plMsgWrap() { hsRefCnt_SafeUnRef(fMsg); }

	plMsgWrap&						ClearReceivers() { fReceivers.SetCount(0); return *this; }
	plMsgWrap&						AddReceiver(const plKey& rcv) 
									{ 
										hsAssert(rcv, "Trying to send mail to nil receiver");
										fReceivers.Append(rcv); return *this;
									}
	const plKey&					GetReceiver(int i) const { return fReceivers[i]; }
	UInt32							GetNumReceivers() const { return fReceivers.GetCount(); }
};

Int32					plDispatch::fNumBufferReq = 0;
hsBool					plDispatch::fMsgActive = false;
plMsgWrap*				plDispatch::fMsgCurrent = nil;
plMsgWrap*				plDispatch::fMsgHead = nil;
plMsgWrap*				plDispatch::fMsgTail = nil;
hsTArray<plMessage*>	plDispatch::fMsgWatch;
MsgRecieveCallback		plDispatch::fMsgRecieveCallback = nil;

hsMutex		plDispatch::fMsgCurrentMutex; // mutex for fMsgCurrent
hsMutex		plDispatch::fMsgDispatchLock; // mutex for IMsgDispatch


plDispatch::plDispatch()
: fOwner(nil), fFutureMsgQueue(nil), fQueuedMsgOn(true)
{
}

plDispatch::~plDispatch()
{
	int i;
	for( i = 0; i < fRegisteredExactTypes.GetCount(); i++ )
		delete fRegisteredExactTypes[i];

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
			// hsRefCnt_SafeUnRef(nuke->fMsg);		// MOOSE - done in plMsgWrap dtor
			delete nuke;
		}

		// reset static members which we just deleted - MOOSE
		fMsgCurrent=fMsgHead=fMsgTail=nil;

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

hsBool plDispatch::ISortToDeferred(plMessage* msg)
{
	plMsgWrap* msgWrap = TRACKED_NEW plMsgWrap(msg);
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
		plMsgWrap* send = IDequeue(&fFutureMsgQueue, nil);
		MsgSend(send->fMsg);
		delete send;
	}

	int timeIdx = plTimeMsg::Index();
	if( IGetOwner()
		&& !fFutureMsgQueue 
		&& 
			( 
				(timeIdx >= fRegisteredExactTypes.GetCount()) 
				|| 
				!fRegisteredExactTypes[plTimeMsg::Index()]
			)
	  )
		plgDispatch::Dispatch()->UnRegisterForExactType(plTimeMsg::Index(), IGetOwnerKey());
}

hsBool plDispatch::IListeningForExactType(UInt16 hClass)
{
	if( (hClass == plTimeMsg::Index()) && fFutureMsgQueue )
		return true;

	return false;
}

void plDispatch::IMsgEnqueue(plMsgWrap* msgWrap, hsBool async)
{
	fMsgCurrentMutex.Lock();

#ifdef HS_DEBUGGING
	if( msgWrap->fMsg->HasBCastFlag(plMessage::kMsgWatch) )
		fMsgWatch.Append(msgWrap->fMsg);
#endif // HS_DEBUGGING

	if( fMsgTail )
		fMsgTail = IInsertToQueue(&fMsgTail->fNext, msgWrap);
	else
		fMsgTail = IInsertToQueue(&fMsgHead, msgWrap);
	fMsgCurrentMutex.Unlock();

	if( !async )
		// Test for fMsgActive in IMsgDispatch(), properly wrapped inside a mutex -mcn
		IMsgDispatch();
}

// On starts deferring msg delivery until buffering is set to off again.
hsBool plDispatch::SetMsgBuffering(hsBool on)
{
	fMsgCurrentMutex.Lock();
	if( on )
	{
		hsAssert(fNumBufferReq || !fMsgActive, "Can't start deferring message delivery while delivering messages. See mf");
		if( !fNumBufferReq && fMsgActive )
		{
			fMsgCurrentMutex.Unlock();
			return false;
		}

		fNumBufferReq++;
		fMsgActive = true;
		fMsgCurrentMutex.Unlock();
	}
	else if( !--fNumBufferReq )
	{
		fMsgActive = false;
		fMsgCurrentMutex.Unlock();
		IMsgDispatch();
	}
	hsAssert(fNumBufferReq >= 0, "Mismatched number of on/off dispatch buffering requests");

	return true;
}

void plDispatch::IMsgDispatch()
{
	if( !fMsgDispatchLock.TryLock() )
		return;

	if( fMsgActive )
	{
		fMsgDispatchLock.Unlock();
		return;
	}

	fMsgActive = true;
	int responseLevel=0;

	fMsgCurrentMutex.Lock();

	plMsgWrap* origTail = fMsgTail;
	while( fMsgCurrent = fMsgHead )
	{
		IDequeue(&fMsgHead, &fMsgTail);
		fMsgCurrentMutex.Unlock();

		plMessage* msg = fMsgCurrent->fMsg;
		hsBool nonLocalMsg = msg && msg->HasBCastFlag(plMessage::kNetNonLocal);

#ifdef HS_DEBUGGING
		int watchIdx = fMsgWatch.Find(msg);
		if( fMsgWatch.kMissingIndex != watchIdx )
		{
			fMsgWatch.Remove(watchIdx);
#if HS_BUILD_FOR_WIN32
			__asm { int 3 }
#endif // HS_BUILD_FOR_WIN32
		}
#endif // HS_DEBUGGING

		static UInt64 startTicks = 0;
		if (plDispatchLogBase::IsLogging())
			startTicks = hsTimer::GetFullTickCount();

		int i, numReceivers=0;
		for( i = 0; fMsgCurrent && i < fMsgCurrent->GetNumReceivers(); i++ )
		{
			const plKey& rcvKey = fMsgCurrent->GetReceiver(i);
			plReceiver* rcv = rcvKey ? plReceiver::ConvertNoRef(rcvKey->ObjectIsLoaded()) : nil;
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
					{	// log net msg if this is a debug object
						hsKeyedObject* ko = hsKeyedObject::ConvertNoRef(rcv);
						if (plNetObjectDebuggerBase::GetInstance()->IsDebugObject(ko))
						{
							hsLogEntry(plNetObjectDebuggerBase::GetInstance()->LogMsg(
								xtl::format("<RCV> object:%s, GameMessage %s st=%.3f rt=%.3f", 
								ko->GetKeyName(), msg->ClassName(), hsTimer::GetSysSeconds(), hsTimer::GetSeconds()).c_str()));
						}
					}
				}

#ifndef PLASMA_EXTERNAL_RELEASE
				UInt32 rcvTicks = hsTimer::GetPrecTickCount();

				// Object could be deleted by this message, so we need to log this stuff now
				const char* keyname = "(unknown)";
				const char* className = "(unknown)";
				UInt32 clonePlayerID = 0;
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
					DebugBreakIfDebuggerPresent();
				#endif
					
				plProfile_BeginTiming(MsgReceive);
				rcv->MsgReceive(msg);
				plProfile_EndTiming(MsgReceive);

#ifndef PLASMA_EXTERNAL_RELEASE
				if (plDispatchLogBase::IsLoggingLong())
				{
					rcvTicks = hsTimer::GetPrecTickCount() - rcvTicks;

					float rcvTime = (float)(hsTimer::PrecTicksToSecs(rcvTicks) * 1000.f);
					// If the receiver takes more than 5 ms to process its message, log it
					if (rcvTime > 5.f)
						plDispatchLogBase::GetInstance()->LogLongReceive(keyname, className, clonePlayerID, msg, rcvTime);
				}
#endif // PLASMA_EXTERNAL_RELEASE

				numReceivers++;

				if (fMsgRecieveCallback != nil)
					fMsgRecieveCallback();
			}
		}

		// for message logging
//		if (plDispatchLogBase::IsLogging())
//		{
//			float sendTime = hsTimer::FullTicksToMs(hsTimer::GetFullTickCount() - startTicks);
//
//			plDispatchLogBase::GetInstance()->DumpMsg(msg, numReceivers, (int)sendTime, responseLevel*2 /* indent */);
//			if (origTail==fMsgCurrent)
//			{	// if we deliver more msgs after this, they must be response msgs
//				responseLevel++;
//				origTail = fMsgTail;
//			}
//		}

		fMsgCurrentMutex.Lock();

		delete fMsgCurrent;
		// TEMP
		fMsgCurrent = (class plMsgWrap *)0xdeadc0de;
	}
	fMsgCurrentMutex.Unlock();

	fMsgActive = false;
	fMsgDispatchLock.Unlock();
}

//
// returns true if msg has been consumed and deleted
//
hsBool plDispatch::IMsgNetPropagate(plMessage* msg)
{
	fMsgCurrentMutex.Lock();

	// Make sure cascaded messages all have the same net flags
	plNetClientApp::InheritNetMsgFlags(fMsgCurrent ? fMsgCurrent->fMsg : nil, msg, false);

	fMsgCurrentMutex.Unlock();

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
		if (plNetClientApp::GetInstance() && plNetClientApp::GetInstance()->ISendGameMessage(msg)>=0)
			msg->SetBCastFlag(plMessage::kNetSent);
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

hsBool plDispatch::MsgSend(plMessage* msg, hsBool async)
{
	if( IMsgNetPropagate(msg) )
		return true;

	plTimeMsg* timeMsg;
	if( msg->GetTimeStamp() > hsTimer::GetSysSeconds() )
		return ISortToDeferred(msg);
	else if( timeMsg = plTimeMsg::ConvertNoRef(msg) )
		ICheckDeferred(timeMsg->DSeconds());

	plMsgWrap* msgWrap = TRACKED_NEW plMsgWrap(msg);
	hsRefCnt_SafeUnRef(msg);

	// broadcast
	if( msg->HasBCastFlag(plMessage::kBCastByExactType) | msg->HasBCastFlag(plMessage::kBCastByType) )
	{
		int idx = msg->ClassIndex();
		if( idx < fRegisteredExactTypes.GetCount() )
		{
			plTypeFilter* filt = fRegisteredExactTypes[idx];
			if( filt )
			{
				int j;
				for( j = 0; j < filt->fReceivers.GetCount(); j++ )
				{
					msgWrap->AddReceiver(filt->fReceivers[j]);
				}
				if( msg->HasBCastFlag(plMessage::kClearAfterBCast) )
				{
					delete filt;
					fRegisteredExactTypes[idx] = nil;
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
void plDispatch::MsgQueueOnOff(hsBool sw)
{
	fQueuedMsgOn = sw;
}
void plDispatch::MsgQueue(plMessage* msg)
{
	if (fQueuedMsgOn)
	{
		fQueuedMsgListMutex.Lock();
		hsAssert(msg,"Message missing");
		fQueuedMsgList.push_back(msg);
		fQueuedMsgListMutex.Unlock();
	}
	else
		MsgSend(msg, false);
}

void plDispatch::MsgQueueProcess()
{
		// Process all messages on Queue, unlock while sending them
		// this would allow other threads to put new messages on the list while we send()
	while (1)
	{	
		plMessage * pMsg = nil;
		fQueuedMsgListMutex.Lock();
		int size = fQueuedMsgList.size();
		if (size)
		{	pMsg = fQueuedMsgList.front();
			fQueuedMsgList.pop_front();
		}
		fQueuedMsgListMutex.Unlock();
		if (pMsg)
		{	MsgSend(pMsg, false);
		}
		if (!size)
			break;
	}
}

void plDispatch::RegisterForType(UInt16 hClass, const plKey& receiver)
{
	int i;
	for( i = 0; i < plFactory::GetNumClasses(); i++ )
	{
		if( plFactory::DerivesFrom(hClass, i) )
			RegisterForExactType(i, receiver);
	}
}

void plDispatch::RegisterForExactType(UInt16 hClass, const plKey& receiver)
{
	int idx = hClass;
	fRegisteredExactTypes.ExpandAndZero(idx+1);
	plTypeFilter* filt = fRegisteredExactTypes[idx];
	if( !filt )
	{
		filt = TRACKED_NEW plTypeFilter;
		fRegisteredExactTypes[idx] = filt;
		filt->fHClass = hClass;
	}

	if( filt->fReceivers.kMissingIndex == filt->fReceivers.Find(receiver) )
		filt->fReceivers.Append(receiver);
}

void plDispatch::UnRegisterForType(UInt16 hClass, const plKey& receiver)
{
	int i;
	for( i = 0; i < fRegisteredExactTypes.GetCount(); i++ )
	{
		if( plFactory::DerivesFrom(hClass, i) )
			IUnRegisterForExactType(i , receiver);
	}

}

hsBool plDispatch::IUnRegisterForExactType(int idx, const plKey& receiver)
{
	hsAssert(idx < fRegisteredExactTypes.GetCount(), "Out of range should be filtered before call to internal");
	plTypeFilter* filt = fRegisteredExactTypes[idx];
	if (!filt)
		return false;
	int j;
	for( j = 0; j < filt->fReceivers.GetCount(); j++ )
	{
		if( receiver == filt->fReceivers[j] )
		{
			if( filt->fReceivers.GetCount() > 1 )
			{
				if( j < filt->fReceivers.GetCount() - 1 )
					filt->fReceivers[j] = filt->fReceivers[filt->fReceivers.GetCount() - 1];
				filt->fReceivers[filt->fReceivers.GetCount()-1] = nil;
				filt->fReceivers.SetCount(filt->fReceivers.GetCount()-1);
			}
			else
			{
				delete filt;
				fRegisteredExactTypes[idx] = nil;
			}

			break;
		}
	}
	return false;
}

void plDispatch::UnRegisterAll(const plKey& receiver)
{
	int i;
	for( i = 0; i < fRegisteredExactTypes.GetCount(); i++ )
	{
		plTypeFilter* filt = fRegisteredExactTypes[i];
		if( filt )
		{
			int idx = filt->fReceivers.Find(receiver);
			if( idx != filt->fReceivers.kMissingIndex )
			{
				if( filt->fReceivers.GetCount() > 1 )
				{
					if( idx < filt->fReceivers.GetCount() - 1 )
						filt->fReceivers[idx] = filt->fReceivers[filt->fReceivers.GetCount() - 1];
					filt->fReceivers[filt->fReceivers.GetCount()-1] = nil;
					filt->fReceivers.SetCount(filt->fReceivers.GetCount()-1);
				}
				else
				{
					delete filt;
					fRegisteredExactTypes[i] = nil;
				}
			}
		}
	}
}

void plDispatch::UnRegisterForExactType(UInt16 hClass, const plKey& receiver)
{
	int idx = hClass;
	if( idx >= fRegisteredExactTypes.GetCount() )
		return;
	plTypeFilter* filt = fRegisteredExactTypes[idx];
	if( !filt )
		return;

	IUnRegisterForExactType(idx, receiver);
}

