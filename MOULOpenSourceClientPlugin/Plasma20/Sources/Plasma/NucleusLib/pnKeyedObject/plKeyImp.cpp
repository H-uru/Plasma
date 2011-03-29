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
#include "plKeyImp.h"
#include "hsStream.h"
#include "hsKeyedObject.h"
#include "hsResMgr.h"
#include "hsTypes.h"
#include "../pnMessage/plRefMsg.h"
#include "../pnMessage/plSelfDestructMsg.h"
#include "hsTimer.h"
#include "plProfile.h"
#include "plgDispatch.h"

plProfile_CreateMemCounter("Keys", "Memory", KeyMem);

static UInt32 CalcKeySize(plKeyImp* key)
{
	UInt32 nameLen = 0;
	if (key->GetUoid().GetObjectName())
		nameLen = strlen(key->GetUoid().GetObjectName()) + 1;
	return sizeof(plKeyImp) + nameLen;
}

//#define LOG_ACTIVE_REFS
#ifdef LOG_ACTIVE_REFS
#include "plCreatableIndex.h"
static const char* kObjName = "GUI_District_OptionsMenuGUI";
static UInt16 kClassType = CLASS_INDEX_SCOPED(plSceneNode);
static UInt32 kCloneID = 0;
hsBool IsTrackedKey(const plKeyImp* key)
{
	return hsStrEQ(key->GetName(), kObjName) && key->GetUoid().GetClassType() == kClassType && key->GetUoid().GetCloneID() == kCloneID;
}
#endif

plKeyImp::plKeyImp() :
	fObjectPtr(nil), 
	fStartPos(-1),
	fDataLen(-1),
	fNumActiveRefs(0),
	fPendingRefs(1),
	fCloneOwner(nil)
{
#ifdef HS_DEBUGGING
	fIDName = nil;
	fClassType = nil;
#endif
}

plKeyImp::plKeyImp(plUoid u, UInt32 pos,UInt32 len):
	fUoid(u),
	fObjectPtr(nil), 
	fStartPos(pos),
	fDataLen(len),
	fNumActiveRefs(0),
	fPendingRefs(1),
	fCloneOwner(nil)
{
	plProfile_NewMem(KeyMem, CalcKeySize(this));

#ifdef HS_DEBUGGING
	fIDName = fUoid.GetObjectName();
	fClassType = plFactory::GetNameOfClass( fUoid.GetClassType() );
#endif
}

plKeyImp::~plKeyImp() 
{
	plProfile_DelMem(KeyMem, CalcKeySize(this));

#if defined(HS_DEBUGGING) && 0
	// Colin debugging
	char buf[512];
	sprintf(buf, "0x%x %s %s\n", this, fIDName, fClassType);
	hsStatusMessage(buf);
#endif

	hsAssert(fObjectPtr == nil, "Deleting non-nil key!  Bad idea!");

	if (fCloneOwner != nil)
	{
		// Must be a clone, remove us from our parent list
		((plKeyImp*)fCloneOwner)->RemoveClone(this);
	}

	for (int i = 0; i < fClones.GetCount(); i++)
	{
		if (fClones[i])
			fClones[i]->UnRegister();
	}
	fClones.Reset();

	// This is normally empty by now, but if we never got loaded,
	// there will be unsent ref messages in the NotifyCreated list
	ClearNotifyCreated();
}

void plKeyImp::SetUoid(const plUoid& uoid)
{
	fUoid = uoid; 
#ifdef HS_DEBUGGING
	fIDName = fUoid.GetObjectName();
	fClassType = plFactory::GetNameOfClass(fUoid.GetClassType());
#endif
}

const char* plKeyImp::GetName() const	
{ 
	return fUoid.GetObjectName(); 
}

hsKeyedObject* plKeyImp::GetObjectPtr()
{	
	return ObjectIsLoaded();
}

hsKeyedObject* plKeyImp::ObjectIsLoaded() const
{
	return this ? fObjectPtr : nil;
}

// Copy the contents of p for cloning process
void plKeyImp::CopyForClone(const plKeyImp *p, UInt32 playerID, UInt32 cloneID)
{
	fObjectPtr = nil;				// the clone object start as nil
	fUoid = p->GetUoid();			// we will set the UOID the same to start

#ifdef HS_DEBUGGING
	fIDName = fUoid.GetObjectName();
	fClassType = plFactory::GetNameOfClass( fUoid.GetClassType() );
#endif

	fStartPos = p->GetStartPos();	
	fDataLen = p->GetDataLen();		
	fUoid.SetClone(playerID, cloneID);
}

hsKeyedObject* plKeyImp::VerifyLoaded()
{
	if (!fObjectPtr)
		hsgResMgr::ResMgr()->ReadObject(this);

	return fObjectPtr;
}

//// Read/Write //////////////////////////////////////////////////////////////
//	The actual key read/writes for the index file, the only time the whole
//	key is ever actually stored.

void plKeyImp::Read(hsStream* s)
{
	fUoid.Read(s);
	s->ReadSwap(&fStartPos);
	s->ReadSwap(&fDataLen);

	plProfile_NewMem(KeyMem, CalcKeySize(this));

#ifdef HS_DEBUGGING
	fIDName = fUoid.GetObjectName();
	fClassType = plFactory::GetNameOfClass(fUoid.GetClassType());
#endif
}

void plKeyImp::SkipRead(hsStream* s)
{
	plUoid tempUoid;
	tempUoid.Read(s);
	s->ReadSwap32();
	s->ReadSwap32();
}

void plKeyImp::Write(hsStream* s)
{
	fUoid.Write(s);
	s->WriteSwap(fStartPos);
	s->WriteSwap(fDataLen);
	if (fStartPos == (UInt32)-1)
		int foo = 0;
}

//// WriteObject /////////////////////////////////////////////////////////////
//	Writes the key's object to the already opened stream

void plKeyImp::WriteObject(hsStream* stream)
{
	hsKeyedObject* ko = ObjectIsLoaded();
	if (ko == nil)
	{
		// Mark the key as not written
		fStartPos = (UInt32)-1;
		fDataLen = (UInt32)-1;
		return;
	}

	fStartPos = stream->GetPosition();
	hsgResMgr::ResMgr()->WriteCreatable(stream, ko);
	fDataLen = stream->GetPosition() - fStartPos;
}

void plKeyImp::UnRegister()		// called from plRegistry
{
	plKey safeRefUntilWereDone = plKey::Make(this);

	hsKeyedObject* ko = ObjectIsLoaded();
	if (ko)
	{
		INotifyDestroyed();
		fObjectPtr = nil;
		fNumActiveRefs = 0;

		hsRefCnt_SafeUnRef(ko);
	}
	IClearRefs();
	ClearNotifyCreated();
};

hsKeyedObject* plKeyImp::RefObject(plRefFlags::Type flags)
{
	if ((flags == plRefFlags::kPassiveRef) && !ObjectIsLoaded())
		return nil;

#ifdef LOG_ACTIVE_REFS
	if (IsTrackedKey(this))
		hsStatusMessageF("@@@ RefObject adding active ref to %s (%d total)", kObjName, fNumActiveRefs+1);
#endif // LOG_ACTIVE_REFS

	IncActiveRefs();

	return VerifyLoaded();	// load object on demand
}

void plKeyImp::UnRefObject(plRefFlags::Type flags)		
{
	// Rather than using hsRefCnt's, make Ref and 
	// UnRef work with ActiveRef system
	if ( (flags == plRefFlags::kPassiveRef) && !ObjectIsLoaded())
		return;

#ifdef LOG_ACTIVE_REFS
	if (IsTrackedKey(this))
		hsStatusMessageF("@@@ UnRefObject releasing active ref to %s (%d total)", kObjName, fNumActiveRefs-1);
#endif // LOG_ACTIVE_REFS
	DecActiveRefs();

	if( !GetActiveRefs() )
	{
		INotifyDestroyed();

		IClearRefs();
		ClearNotifyCreated();
		
		plKey key=plKey::Make( this );	// for linux build
		plSelfDestructMsg* nuke = TRACKED_NEW plSelfDestructMsg( key );
		plgDispatch::Dispatch()->MsgSend(nuke);
	}
}

hsKeyedObject* plKeyImp::SetObjectPtr(hsKeyedObject* p)	
{ 
	hsKeyedObject* retVal = nil;

	// If our object is the only one with a ref to us, this function will crash, so we 
	// make sure we have an extra ref, just like in UnRegister().
	plKey safeRefUntilWereDone = plKey::Make(this);

	if (p)
	{
#ifdef HS_DEBUGGING
		if (fClassType)
		{
			char str[2048];
			sprintf(str, "Mismatch of class (we are a %s, given a %s)", fClassType, p->ClassName());
			hsAssert(fClassType == p->ClassName() || strcmp(fClassType, p->ClassName()) == 0, str); // points to static
		}
		else
			fClassType = p->ClassName();
#endif

		hsAssert(!fObjectPtr, "Setting an ObjectPtr thats already Set!");
		
		retVal = fObjectPtr = p;
	}
	else
	{
		if (fObjectPtr)
			UnRegister();

		fObjectPtr = nil;
		retVal = nil;
	}

	return retVal; 
}

void plKeyImp::ClearNotifyCreated() 
{ 
	for (int i = 0; i < fNotifyCreated.GetCount(); i++)
		hsRefCnt_SafeUnRef(fNotifyCreated[i]);
	fNotifyCreated.Reset(); 
	fNotified.Reset();
	fActiveRefs.Reset();
}

void plKeyImp::AddNotifyCreated(plRefMsg* msg, plRefFlags::Type flags) 
{ 
	if (!(flags == plRefFlags::kPassiveRef))
	{
#ifdef LOG_ACTIVE_REFS
		if (IsTrackedKey(this))
		{
			hsStatusMessageF("@@@ %s(%s) adding active ref to %s (%d total)", msg->GetReceiver(0)->GetName(),
				plFactory::GetNameOfClass(msg->GetReceiver(0)->GetUoid().GetClassType()), kObjName, fNumActiveRefs+1);
		}
#endif // LOG_ACTIVE_REFS

		IncActiveRefs();
		SetActiveRef(GetNumNotifyCreated());
	}

	hsRefCnt_SafeRef(msg);
	fNotifyCreated.Append(msg); 
}

void plKeyImp::RemoveNotifyCreated(int i) 
{ 
	hsRefCnt_SafeUnRef(fNotifyCreated[i]);
	fNotifyCreated.Remove(i); 

	fNotified.RemoveBit(i);
	fActiveRefs.RemoveBit(i);
}

void plKeyImp::AddRef(plKeyImp* key) const
{
	fPendingRefs++;
	fRefs.Append(key);
}


void plKeyImp::RemoveRef(plKeyImp* key) const
{
	int idx = fRefs.Find(key);
	if (fRefs.kMissingIndex != idx)
		fRefs.Remove(idx);
}

void plKeyImp::AddClone(plKeyImp* key)
{
	hsAssert(!GetClone(key->GetUoid().GetClonePlayerID(), key->GetUoid().GetCloneID()),
				"Adding a clone which is already there?");

	key->fCloneOwner = plKey::Make(this);
	fClones.Append(key);
}

void plKeyImp::RemoveClone(plKeyImp* key) const
{
	if (key->GetUoid().IsClone())
	{
		int idx = fClones.Find(key);
		if (idx != -1)
		{
			fClones.Remove(idx);
			key->fCloneOwner = nil;
		}
	}
}

plKey plKeyImp::GetClone(UInt32 playerID, UInt32 cloneID) const
{
	for (int i = 0; i < fClones.GetCount(); i++)
	{
		plKeyImp* cloneKey = fClones[i];
		if (cloneKey
			&& cloneKey->GetUoid().GetCloneID() == cloneID
			&& cloneKey->GetUoid().GetClonePlayerID() == playerID)
			return plKey::Make(cloneKey);
	}

	return plKey();
}

UInt32 plKeyImp::GetNumClones()
{
	return fClones.GetCount();
}

plKey plKeyImp::GetCloneByIdx(UInt32 idx)
{
	if (idx < fClones.GetCount())
		return plKey::Make(fClones[idx]);

	return nil;
}

void plKeyImp::SatisfyPending(plRefMsg* msg) const
{
	for (int i = 0; i < msg->GetNumReceivers(); i++)
		((plKeyImp*)msg->GetReceiver(i))->SatisfyPending();
}

void plKeyImp::SatisfyPending() const
{
	hsAssert(fPendingRefs > 0, "Have more requests satisfied than we made");
	if (!--fPendingRefs)
	{
#ifdef PL_SEND_SATISFIED
		plSatisfiedMsg* msg = TRACKED_NEW plSatisfiedMsg(this);
		plgDispatch::MsgSend(msg);
#endif // PL_SEND_SATISFIED
	}
}

void plKeyImp::ISetupNotify(plRefMsg* msg, plRefFlags::Type flags)
{
	msg->SetSender(nil);

	AddNotifyCreated(msg, flags);

	hsAssert(msg->GetNumReceivers(), "nil object getting a reference");
	for (int i = 0; i < msg->GetNumReceivers(); i++)
		((plKeyImp*)msg->GetReceiver(i))->AddRef(plKey::Make(this));
}

void plKeyImp::SetupNotify(plRefMsg* msg, plRefFlags::Type flags)
{
	hsKeyedObject* ko = ObjectIsLoaded();

	ISetupNotify(msg, flags);

	// the KeyedObject is already Loaded, so we better go ahead and send the notify message just added.
	if (ko)
	{
		hsRefCnt_SafeRef(ko);
		msg->SetRef(ko);
		msg->SetTimeStamp(hsTimer::GetSysSeconds());

		// Add ref, since Dispatch will unref this msg but we want to keep using it.
		hsRefCnt_SafeRef(msg);

		SetNotified(GetNumNotifyCreated()-1);
		SatisfyPending(msg);

#ifdef LOAD_IN_THREAD		// test for resLoader
		plgDispatch::Dispatch()->MsgQueue(msg);
#else
		plgDispatch::Dispatch()->MsgSend(msg);
#endif

		hsRefCnt_SafeUnRef(ko);
	}
	hsRefCnt_SafeUnRef(msg);
}

// We could just call NotifyCreated() on all our fRefs, and then fix
// up fNotified to only get set when the message actually was delivered (i.e.
// refMsg->GetReceiver(0)->GetObjectPtr() != nil. But that only really works
// if we guarantee the refMsg->GetNumReceivers() == 1.
// This looks like it'll take forever to run, but this is only called right
// when our object has just been loaded, at which time normally fRefs.GetCount() == 0.
void plKeyImp::INotifySelf(hsKeyedObject* ko)
{
	for (int i = 0; i < fRefs.GetCount(); i++)
	{
		hsKeyedObject* rcv = fRefs[i]->GetObjectPtr();
		if (rcv)
		{
			for (int j = 0; j < fRefs[i]->fNotifyCreated.GetCount(); j++)
			{
				plRefMsg* refMsg = fRefs[i]->fNotifyCreated[j];
				if (refMsg && refMsg->GetRef() && !fRefs[i]->IsNotified(j))
				{
					hsAssert(refMsg->GetRef() == rcv, "Ref message out of sync with its ref");

					// GetNumReceivers() should always be 1 for a refMsg.
					for (int k = 0; k < refMsg->GetNumReceivers(); k++)
					{
						if (&(*refMsg->GetReceiver(k)) == (plKeyData*)this)
						{
							fRefs[i]->SetNotified(j);
							fRefs[i]->SatisfyPending(refMsg);

							hsRefCnt_SafeRef(refMsg);
							plgDispatch::MsgSend(refMsg);
							break;
						}
					}
				}
			}
		}
	}
}

void plKeyImp::NotifyCreated()
{
	hsKeyedObject* ko = GetObjectPtr();
	hsRefCnt_SafeRef(ko);
	hsAssert(ko, "Notifying of created before on nil object");
	
	INotifySelf(ko);
	
	for (int i = 0; i < GetNumNotifyCreated(); i++)
	{
		if (!IsNotified(i) && GetNotifyCreated(i)->GetReceiver(0)->GetObjectPtr())
		{
			plRefMsg* msg = GetNotifyCreated(i);
			msg->SetRef(ko);
			msg->SetTimeStamp(hsTimer::GetSysSeconds());
			msg->SetContext(plRefMsg::kOnCreate);
			hsRefCnt_SafeRef(msg);

			SetNotified(i);
			SatisfyPending(msg);

			plgDispatch::MsgSend(msg);		
		}
	}
	hsRefCnt_SafeUnRef(ko);
}

void plKeyImp::INotifyDestroyed()
{
	hsKeyedObject* ko = GetObjectPtr();
	hsAssert(ko, "Notifying of destroy on already destroyed");
	int i;
	for( i = 0; i < GetNumNotifyCreated(); i++ )
	{
		hsAssert(ko, "Notifying of destroy on already destroyed");
		plRefMsg* msg = GetNotifyCreated(i);
		msg->SetRef(ko);
		msg->SetTimeStamp(hsTimer::GetSysSeconds());
		msg->SetContext(plRefMsg::kOnDestroy);
		hsRefCnt_SafeRef(msg);
		msg->Send();
	}
	fNotified.Clear();
}

void plKeyImp::IClearRefs()
{
	while (GetNumRefs())
		IRelease(GetRef(0));
	fRefs.Reset();

	for (int i = 0; i < GetNumNotifyCreated(); i++)
	{
		plRefMsg* msg = GetNotifyCreated(i);
		for (int j = 0; j < msg->GetNumReceivers(); j++)
			((plKeyImp*)msg->GetReceiver(j))->RemoveRef(this);
	}
}

void plKeyImp::Release(plKey targetKey)
{
	IRelease((plKeyImp*)targetKey);
}

void plKeyImp::IRelease(plKeyImp* iTargetKey)
{
	// Remove the target key from my ref list
	RemoveRef(iTargetKey);

	// Inspect the target key to find whether it is supposed to send a message
	// to me on destruction, and to find out if I have an active of passive 
	// ref on this key.  Not sure why I don't track my own active/passive ref states
	hsBool isActive = false;
	int iTarg = -1;
	for (int i = 0; (iTarg < 0) && (i < iTargetKey->GetNumNotifyCreated()); i++)
	{
		plMessage* rcvMsg = iTargetKey->GetNotifyCreated(i);
		for (int j = 0; j < rcvMsg->GetNumReceivers(); j++)
		{
			if (&(*rcvMsg->GetReceiver(j)) == (plKeyData*)this)
			{
				isActive = iTargetKey->IsActiveRef(iTarg = i);
				break;
			}
		}
	}

	if (iTarg < 0)
	{
		// If it doesn't send me a message on destruction,  I am assuming I don't have an
		// active ref on the key (seems to be always true, but should we depend on it?)
		// Since it doesn't send me a message, and won't be destroyed by the release, no 
		// need to do anything more here
		return;
	}

	// If releasing this target causes its destruction, we'll notify
	// everyone that target is dead. Otherwise, we'll remove
	// releaser from target's notify list and notify releaser
	// it's been removed. Object doesn't actually get deleted until
	// it receives the SelfDestruct msg, by which time everyone referencing
	// it has been notified it is going away.
#ifdef LOG_ACTIVE_REFS
	if (isActive && IsTrackedKey(iTargetKey))
		hsStatusMessageF("@@@ %s(%s) releasing active ref on %s (%d total)", GetName(), plFactory::GetNameOfClass(GetUoid().GetClassType()), kObjName, iTargetKey->fNumActiveRefs-1);
#endif // LOG_ACTIVE_REFS

	if (isActive && iTargetKey->GetActiveRefs() && !iTargetKey->DecActiveRefs())
	{
		iTargetKey->INotifyDestroyed();

		iTargetKey->IClearRefs();
		iTargetKey->ClearNotifyCreated();
		
		plKey key = plKey::Make(iTargetKey);
		plSelfDestructMsg* nuke = TRACKED_NEW plSelfDestructMsg(key);
		plgDispatch::Dispatch()->MsgSend(nuke);
	}
	else
	{
		plRefMsg* refMsg = iTargetKey->GetNotifyCreated(iTarg);
		hsRefCnt_SafeRef(refMsg);
		iTargetKey->RemoveNotifyCreated(iTarg);
		if (refMsg)
		{
			refMsg->SetRef(iTargetKey->ObjectIsLoaded());
			refMsg->SetTimeStamp(hsTimer::GetSysSeconds());
			refMsg->SetContext(plRefMsg::kOnRemove);
			hsRefCnt_SafeRef(refMsg);
			plgDispatch::MsgSend(refMsg);
		}
		hsRefCnt_SafeUnRef(refMsg);
	}
}
