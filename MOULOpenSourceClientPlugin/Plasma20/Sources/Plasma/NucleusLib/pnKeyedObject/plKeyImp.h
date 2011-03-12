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
#ifndef plKeyImp_inc
#define plKeyImp_inc

#include "plKey.h"
#include "hsTemplates.h"
#include "plUoid.h"  
#include "hsBitVector.h"
#include "plRefFlags.h"

//------------------------------------
// plKey is a handle to a keyedObject
//------------------------------------
class plKeyImp : public plKeyData 
{
public:
	plKeyImp();
	plKeyImp(plUoid, UInt32 pos,UInt32 len);
	virtual ~plKeyImp();

	virtual const plUoid&	GetUoid() const { return fUoid; }
	virtual const char*		GetName() const;

	virtual hsKeyedObject*	GetObjectPtr();
	virtual hsKeyedObject*	ObjectIsLoaded() const;
	virtual hsKeyedObject*	VerifyLoaded();

	// called before writing to disk so that static keys can have faster lookups (int compare instead of string compare)
	void SetObjectID(UInt32 id) {fUoid.SetObjectID(id);}

	//----------------------
	// I/O
	// ResMgr performs read, so it can search for an existing instance....
	//----------------------
	void Read(hsStream* s);
	void Write(hsStream* s);
	void WriteObject(hsStream* s);
	// For when you need to skip over a key in a stream
	static void SkipRead(hsStream* s);

	UInt32 GetStartPos() const	{ return fStartPos; } // for ResMgr to read the Objects
	UInt32 GetDataLen() const	{ return fDataLen;  } // for ResMgr to read the Objects

	//----------------------
	// Allow a keyed object to behave as if it has an active ref when in fact the object
	// should only be active ref'ed by a non-keyed parent.  Essentially just bumps/decs
	// the active ref count to facilitate normal object creation/destruction
	//----------------------
	virtual hsKeyedObject*	RefObject(plRefFlags::Type flags = plRefFlags::kActiveRef);
	virtual void			UnRefObject(plRefFlags::Type flags = plRefFlags::kActiveRef);

	//----------------------
	// Release has two behaviors, depending on whether the ref is active or passive:
	// Active - Release decs the ActiveRefCnt. When it gets to zero, the object will be deleted.
	// Passive - Unregisters my interest in when the object is created or destroyed.
	//----------------------
	virtual void Release(plKey targetKey);

	void UnRegister();
	void SetUoid(const plUoid& uoid);
	void SetupNotify(plRefMsg* msg, plRefFlags::Type flags);

	// hsKeyedObject use only!
	hsKeyedObject* SetObjectPtr(hsKeyedObject* p);

	////////////////////////////////////////////////////////////////////////////
	// ResManager/Registry use only!
	//

	//----------------------
	// Clone access
	//----------------------
	void	AddClone(plKeyImp* c);
	void	RemoveClone(plKeyImp* c) const;
	plKey	GetClone(UInt32 playerID, UInt32 cloneID) const;
	void	CopyForClone(const plKeyImp* p, UInt32 playerID, UInt32 cloneID);    // Copy the contents of p for cloning process

	UInt32	GetNumClones();
	plKey	GetCloneByIdx(UInt32 idx);
	plKey	GetCloneOwner() { return fCloneOwner; }

	void NotifyCreated();
	void ISetupNotify(plRefMsg* msg, plRefFlags::Type flags); // Setup notifcations for reference, don't send anything.

	void		AddRef(plKeyImp* key) const;
	UInt16		GetNumRefs() const { return fRefs.GetCount(); }
	plKeyImp*	GetRef(int i) const { return fRefs[i]; }
	void		RemoveRef(plKeyImp *key) const;

	virtual UInt16      GetActiveRefs() const			{ return fNumActiveRefs; }
	virtual UInt16		GetNumNotifyCreated() const		{ return fNotifyCreated.GetCount(); }
	virtual plRefMsg*	GetNotifyCreated(int i) const	{ return fNotifyCreated[i]; }
	virtual const hsBitVector& GetActiveBits() const	{ return fActiveRefs; }

protected:
	void        AddNotifyCreated(plRefMsg* msg, plRefFlags::Type flags);
	void        ClearNotifyCreated();
	UInt16      GetNumNotifyCreated() { return fNotifyCreated.GetCount(); }
	plRefMsg*   GetNotifyCreated(int i) { return fNotifyCreated[i]; }
	void        RemoveNotifyCreated(int i);

	UInt16      IncActiveRefs() { return ++fNumActiveRefs; }
	UInt16      DecActiveRefs() { return fNumActiveRefs ? --fNumActiveRefs : 0; }

	hsBool	IsActiveRef(int i) const			{ return fActiveRefs.IsBitSet(i) != 0; }
	void	SetActiveRef(int i, hsBool on=true)	{ fActiveRefs.SetBit(i, on); }
	hsBool	IsNotified(int i) const				{ return fNotified.IsBitSet(i) != 0; }
	void	SetNotified(int i, hsBool on=true)	{ fNotified.SetBit(i, on); }

	void SatisfyPending(plRefMsg* msg) const;
	void SatisfyPending() const;

	void INotifySelf(hsKeyedObject* ko);
	void INotifyDestroyed();
	void IClearRefs();

	void IRelease(plKeyImp* keyImp);

	hsKeyedObject* fObjectPtr;

	// These fields are the ones actually saved to disk
	plUoid fUoid;
	UInt32 fStartPos;	// where I live in the Datafile  
	UInt32 fDataLen;	// Length in the Datafile

	// Following used by hsResMgr to notify on defered load or when a passive ref is destroyed.
	UInt16						fNumActiveRefs;	// num active refs on me
	hsBitVector					fActiveRefs;	// Which of notify created are active refs
	hsBitVector					fNotified;		// which of notifycreated i've already notified.
	hsTArray<plRefMsg*>			fNotifyCreated;	// people to notify when I'm created or destroyed
	mutable hsTArray<plKeyImp*>	fRefs;			// refs I've made (to be released when I'm unregistered).
	mutable Int16				fPendingRefs;	// Outstanding requests I have out.
	mutable hsTArray<plKeyImp*>	fClones;		// clones of me
	mutable plKey				fCloneOwner;	// pointer for clones back to the owning key
};

#endif // hsRegistry_inc
