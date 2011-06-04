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
#ifndef hsResMgr_inc
#define hsResMgr_inc

#include "hsTypes.h"
#include "hsRefCnt.h"
#include "plLoadMask.h"
#include "plRefFlags.h"
#include "../pnKeyedObject/plKey.h"

class hsStream;
class plKeyImp;
class hsKeyedObject;
class plRefMsg;
class plUoid;
class plLocation;
class plCreatable;
class plDispatchBase;

class hsResMgr : public hsRefCnt
{
public:
	//---------------------------
	//  Load and Unload
	//---------------------------
	virtual void Load  (const plKey& objKey)=0;		// places on list to be loaded
	virtual hsBool Unload(const plKey& objKey)=0;		// Unregisters (deletes) an object, Return true if successful
	virtual plKey CloneKey(const plKey& objKey)=0;

	//---------------------------
	//  Finding Functions
	//---------------------------
	virtual plKey FindKey(const plUoid& uoid)=0;	

	//---------------------------
	//  Establish reference linkage	
	//---------------------------
	virtual hsBool AddViaNotify(const plKey& sentKey, plRefMsg* msg, plRefFlags::Type flags)=0;
	virtual hsBool AddViaNotify(plRefMsg* msg, plRefFlags::Type flags)=0; // msg->fRef->GetKey() == sentKey

	virtual hsBool SendRef(const plKey& key, plRefMsg* refMsg, plRefFlags::Type flags)=0;
	virtual hsBool SendRef(hsKeyedObject* ko, plRefMsg* refMsg, plRefFlags::Type flags)=0;

	//---------------------------
	//  Reading and Writing keys
	//---------------------------
	// Read a Key in, and Notify me when the Object is loaded
	virtual plKey ReadKeyNotifyMe(hsStream* s, plRefMsg* retMsg, plRefFlags::Type flags)=0; 
	// Just read the Key data in and find a match in the registry and return it.
	virtual plKey ReadKey(hsStream* s)=0; 

	// For convenience you can write a key using the KeyedObject or the Key...same result
	virtual void WriteKey(hsStream* s, hsKeyedObject* obj)=0; 
	virtual void WriteKey(hsStream* s, const plKey& key)=0; 

	//---------------------------
	//  Reading and Writing Objects directly
	//---------------------------
	virtual plCreatable*	ReadCreatable(hsStream* s)=0;
	virtual void			WriteCreatable(hsStream* s, plCreatable* cre)=0;

	virtual plCreatable*	ReadCreatableVersion(hsStream* s)=0;
	virtual void			WriteCreatableVersion(hsStream* s, plCreatable* cre)=0;
	
	//---------------------------
	// Registry Modification Functions
	//---------------------------
	virtual plKey NewKey(const char* name, hsKeyedObject* object, const plLocation& loc, const plLoadMask& m = plLoadMask::kAlways)=0;
	virtual plKey NewKey(plUoid& newUoid, hsKeyedObject* object)=0;

	virtual plDispatchBase* Dispatch()=0;

	virtual void BeginShutdown() {}

protected:
	friend class hsKeyedObject;
	friend class plKey;
	friend class plKeyImp;
	friend class plArmatureMod; // Temp hack until a findkey/clone issue is fixed. -Bob

	virtual plKey	ReRegister(const char *nm, const plUoid& oid)=0;
	virtual hsBool	ReadObject(plKeyImp* key)=0;  // plKeys call this when needed

	// Sets a key as used or unused in the registry.  When all keys in a page of a
	// particular type in an page are unused, we can free the memory associated with them.
	// Only called by plKeyImp
	virtual void IKeyReffed(plKeyImp* key)=0;
	virtual void IKeyUnreffed(plKeyImp* key)=0;

protected:	// hsgResMgr only
	friend class hsgResMgr;
	virtual hsBool IReset()=0;
	virtual hsBool IInit()=0;
	virtual void IShutdown()=0;
};


class hsgResMgr
{
private:
	static hsResMgr* fResMgr;

public:
	static hsResMgr* ResMgr() { return (hsResMgr*)fResMgr; }

	static plDispatchBase* Dispatch() { hsAssert(fResMgr, "No resmgr"); return fResMgr->Dispatch(); }

	static hsBool Init(hsResMgr* m);
	static hsBool Reset() { return fResMgr->IReset(); }
	static void Shutdown();
	
	static hsBool SendRef(plKey& key, plRefMsg* refMsg, plRefFlags::Type flags) { return fResMgr->SendRef(key, refMsg, flags); }
	static hsBool SendRef(hsKeyedObject* ko, plRefMsg* refMsg, plRefFlags::Type flags) { return fResMgr->SendRef(ko, refMsg, flags); }
};

#endif // hsResMgr_inc

