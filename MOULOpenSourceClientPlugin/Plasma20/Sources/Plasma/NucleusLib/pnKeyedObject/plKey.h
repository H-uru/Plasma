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
#ifndef plKey_h_inc
#define plKey_h_inc

#include "hsTypes.h"
#include "plRefFlags.h"

class hsKeyedObject;
class plRefMsg;
class plUoid;
class hsBitVector;

//// plKey ///////////////////////////////////////////////////////////////////
//	Pointer to a plKeyData struct, which is a handle to a keyedObject

class plKeyData;
class plKeyImp;

class plKey 
{
public:
	// Constructors and destructors and copy stuff
	plKey();
	plKey(const plKey& rhs);
	plKey(void* ptr);	// For constructing a nil key
	~plKey();
	plKey& operator=(const plKey& rhs);

	hsBool operator==(const plKey& rhs) const;
	hsBool operator==(const plKeyData* rhs) const;
	hsBool operator!=(const plKey& rhs) const { return !(*this == rhs); }
	hsBool operator!=(const plKeyData* rhs) const { return !(*this == rhs); }

	plKeyData* operator->() const;
	plKeyData& operator*() const;

	operator plKeyImp*() const { return (plKeyImp*)fKeyData; }

	static plKey Make(plKeyData* data) { return plKey(data, false); }

protected:
	// Pointer to our real key
	plKeyData* fKeyData;
	void IIncRef();
	void IDecRef();

	// Internal constructor, extra param is to distinguish it from the void* constructor
	plKey(plKeyData* data, hsBool ignore);
};

//// plKeyData ///////////////////////////////////////////////////////////////
//	Base virtual class that provides the essential functionality you would
//	want from a plKey-ish thing.

class plKeyData
{
public:
	virtual const plUoid&	GetUoid() const=0;
	virtual const char*		GetName() const=0;

	virtual hsKeyedObject*	GetObjectPtr()=0;
	virtual hsKeyedObject*	ObjectIsLoaded() const=0;
	virtual hsKeyedObject*	VerifyLoaded() = 0;

	//----------------------
	// Allow a keyed object to behave as if it has an active ref when in fact the object
	// should only be active ref'ed by a non-keyed parent.  Essentially just bumps/decs
	// the active ref count to facilitate normal object creation/destruction
	//----------------------
	virtual hsKeyedObject*	RefObject(plRefFlags::Type flags = plRefFlags::kActiveRef)=0;
	virtual void			UnRefObject(plRefFlags::Type flags = plRefFlags::kActiveRef)=0;

	//----------------------
	// Release has two behaviors, depending on whether the ref is active or passive:
	// Active - Release decs the ActiveRefCnt. When it gets to zero, the object will be deleted.
	// Passive - Unregisters my interest in when the object is created or destroyed.
	//----------------------
	virtual void Release(plKey targetKey)=0;

	virtual UInt16      GetActiveRefs() const = 0;

	virtual UInt16		GetNumNotifyCreated() const = 0;
	virtual plRefMsg*	GetNotifyCreated(int i) const = 0;
	virtual const hsBitVector& GetActiveBits() const = 0;

protected:
	// Protected so only the registry can create it
	plKeyData();
	virtual ~plKeyData();

#ifdef HS_DEBUGGING
	// Debugging info fields
	const char* fIDName;
	const char* fClassType;
#endif

	//// RefCount Stuff //////////////////////////////////////////////////////////
	//	The refcounts on plKeyData/plKeyImps are zero-based. When you first create
	//	a new plKeyImp (which should ONLY ever be done inside the resMgr), it gets
	//  a refcount of zero. Assigning a new plKey to represent it bumps it to 1,
	//	and when that key goes away, the refcount drops to zero and the ResManager
	//  is notified of the fact and may delete the keyImp.
	//  So the only refcounts on keys outside of the resMgr should be one or more;
	//  only inside the resMgr should there EVER exist keys with a refcount of 0.
	//
	//	Using our own refCount system instead of hsRefCnt allows us to make it all
	//	protected, so that only plKey can actually ref/unref, which is as it should
	//	be.

	// Only one class should ever touch this...
	friend class plKey;

	// Refcount--the number of plKeys that have pointers to us.
	UInt16 fRefCount;
};

#endif // plKey_h_inc
