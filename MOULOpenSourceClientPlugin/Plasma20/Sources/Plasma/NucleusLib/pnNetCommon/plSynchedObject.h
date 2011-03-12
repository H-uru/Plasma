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
#ifndef PLSYNCHOBJ_inc
#define PLSYNCHOBJ_inc

#include "hsTypes.h"
#include "../pnKeyedObject/hsKeyedObject.h"
#include "../pnKeyedObject/plKey.h"
#include "hsStlUtils.h"
#include "plNetGroup.h"

/////////////////////////////////////
// plSynchedObject
// one per savable object
/////////////////////////////////////
class hsStream;
class plDirtyNotifier;
class plSynchedObject : public hsKeyedObject
{
public:
	enum LocallyOwnedAnswer
	{
		kNo=false,
		kYes=true
	};

	enum Flags	
	{
		kDontDirty				= 0x1,
		kSendReliably			= 0x2,		// object wants reliable send
		kHasConstantNetGroup	= 0x4,		// has a constant net group.
		kDontSynchGameMessages	= 0x8,		// don't send/recv game actions
		kExcludePersistentState = 0x10,		// don't send SDL state msgs to server, check exclude list
		kExcludeAllPersistentState=0x20,	// don't send ANY type of SDL state
		kLocalOnly				= (kExcludeAllPersistentState | kDontSynchGameMessages),	// localOnly in all respects
		kHasVolatileState		= 0x40,		// server won't save this state on shutdown
		kAllStateIsVolatile		 = 0x80
	};

	enum SDLSendFlags	
	{
		kBCastToClients			= 0x1,
		kForceFullSend			= 0x2,
		kSkipLocalOwnershipCheck= 0x4,
		kSendImmediately		= 0x8,
		kDontPersistOnServer	= 0x10,		// for an SDL bcast msg which is used for synching only, not persisting
		kUseRelevanceRegions	= 0x20,
		kNewState				= 0x40,
		kIsAvatarState			= 0x80,
	};

	struct StateDefn
	{
		plKey	fObjKey;
		UInt32	fSendFlags;
		std::string fSDLName;

		plSynchedObject* GetObject() const { return plSynchedObject::ConvertNoRef(fObjKey->ObjectIsLoaded()); }
		StateDefn() : fObjKey(nil),fSendFlags(0) {}
		StateDefn(plKey k, UInt32 f, const char* sdlName) : fObjKey(k),fSendFlags(f) { fSDLName=sdlName; }
	};

private:
	typedef std::vector<std::string> SDLStateList;
	SDLStateList fSDLExcludeList;
	SDLStateList fSDLVolatileList;
	UInt32 fSynchFlags;

	plNetGroupId fNetGroup;

	static std::vector<hsBool> fSynchStateStack;
	static plSynchedObject*	fStaticSynchedObj;		// static which temporarily holds address of each object's synchMgr
	static std::vector<StateDefn>	fDirtyStates;

	static void IRemoveDirtyState(plKey o, const char* sdlName);
	static void IAddDirtyState(plKey o, const char* sdlName, UInt32 sendFlags);
	bool IOKToDirty(const char* SDLStateName) const;
	SDLStateList::const_iterator IFindInSDLStateList(const SDLStateList& list, const char* sdlName) const;
protected:
	bool IOKToNetwork(const char* sdlName, UInt32* synchFlags) const;	
public:
	plSynchedObject();
	virtual ~plSynchedObject();

	CLASSNAME_REGISTER( plSynchedObject );
	GETINTERFACE_ANY( plSynchedObject, hsKeyedObject);

	virtual hsBool MsgReceive(plMessage* msg);
	
	// getters
	int GetSynchFlags() const { return fSynchFlags; }
	plNetGroupId GetNetGroup() const { return fNetGroup; };
	plNetGroupId GetEffectiveNetGroup() const;
	
	// setters
	void SetSynchFlagsBit(UInt32 f) { fSynchFlags |= f; }
    virtual void SetNetGroupConstant(plNetGroupId netGroup);
    virtual void SetNetGroup(plNetGroupId netGroup) { fNetGroup = netGroup; }	
	plNetGroupId SelectNetGroup(plKey groupKey);

	virtual hsBool DirtySynchState(const char* sdlName, UInt32 sendFlags);		
	void SendSDLStateMsg(const char* SDLStateName, UInt32 synchFlags);	// don't use, only for net code

	void ClearSynchFlagsBit(UInt32 f) { fSynchFlags &= ~f; }

	// static 
	static hsBool GetSynchDisabled() { return fSynchStateStack.size() ? fSynchStateStack.back() : true; }
	static void PushSynchDisabled(hsBool b) { fSynchStateStack.push_back(b); }
	static hsBool PopSynchDisabled();
	static plSynchedObject* GetStaticSynchedObject() { return fStaticSynchedObj; }
	static Int32 GetNumDirtyStates() { return fDirtyStates.size(); }
	static plSynchedObject::StateDefn* GetDirtyState(Int32 i) { return &fDirtyStates[i]; }
	static void ClearDirtyState(std::vector<StateDefn>& carryOver) { fDirtyStates=carryOver; } 

	// IO 
//	void SendCreationMsg(double secs);
//	void SendDestructionMsg(double secs) ;

	virtual void	Read(hsStream* s, hsResMgr* mgr);
	virtual void	Write(hsStream* s, hsResMgr* mgr);

	int IsLocallyOwned() const;		// returns yes/no/maybe
	
	// disable net synching only
	bool IsNetSynched() const { return (fSynchFlags & kDontSynchGameMessages)==0; }
	void SetNetSynched(bool b) { if (!b) fSynchFlags |= kDontSynchGameMessages; else fSynchFlags &= ~kDontSynchGameMessages;	}

	// disable net synching AND persisting
	bool IsLocalOnly() const { return (fSynchFlags & kLocalOnly)==0; 	}
	void SetLocalOnly(bool b) { if (b) fSynchFlags |= kLocalOnly; else fSynchFlags &= ~kLocalOnly;	}

	// disable particular types of persistence
	void AddToSDLExcludeList(const char*);
	void RemoveFromSDLExcludeList(const char*);
	bool IsInSDLExcludeList(const char*) const;

	// make volatile particular types of state
	void AddToSDLVolatileList(const char*);
	void RemoveFromSDLVolatileList(const char*);
	bool IsInSDLVolatileList(const char*) const;

	//
	// synched value stuff, currently unused
	// current size is 16 + numValue bytes*2 + numFriends*4 bytes
	//
#ifdef USE_SYNCHED_VALUES
public:
	typedef UInt16 AddrOffsetType;
	typedef UInt8 NumSynchedValuesType;
	typedef UInt16 FlagsType;
	friend class plSynchedValueBase;

private:
	AddrOffsetType* fSynchedValueAddrOffsets;	// represent dwords offsets
	NumSynchedValuesType fNumSynchedValues;

	// array of friends
	plSynchedValueBase** fSynchedValueFriends;	
	NumSynchedValuesType fNumSynchedValueFriends;

	// dirty callback notifiers
	std::vector<plDirtyNotifier*> fDirtyNotifiers;

	void IAppendSynchedValueAddrOffset(AddrOffsetType synchedValueAddrOffset);
	void IAppendSynchedValueFriend(plSynchedValueBase* v);
	plSynchedValueBase* IGetSynchedValue(NumSynchedValuesType i) const
		{ return (plSynchedValueBase*)((Int32)this + (fSynchedValueAddrOffsets[i]<<2)); }
	plSynchedValueBase* IGetSynchedValueFriend(NumSynchedValuesType i) const
		{ return fSynchedValueFriends[i]; }

public:
	Int32 GetNumSynchedValues() const { return fNumSynchedValues+fNumSynchedValueFriends; }
	plSynchedValueBase* GetSynchedValue(int i) const;

	UInt8 RegisterSynchedValue(plSynchedValueBase* v); 
	hsBool RemoveSynchedValue(plSynchedValueBase* v);		// handles SVFriends too
	void RegisterSynchedValueFriend(plSynchedValueBase* v); 
#endif

#ifdef USE_DIRTY_NOTIFIERS
	// dirty CB notifiers
	void AddDirtyNotifier(plDirtyNotifier* dn);
	void RemoveDirtyNotifier(plDirtyNotifier* dn);
#endif
	void CallDirtyNotifiers();
};

//
// helper class to set dirty tracking on/off within scope
//
class plSynchEnabler
{
public:
	plSynchEnabler(hsBool enable) { plSynchedObject::PushSynchDisabled(!enable); }
	~plSynchEnabler() { plSynchedObject::PopSynchDisabled(); }
};

#ifdef USE_DIRTY_NOTIFIERS
///////////////////////////////////
// plDirtyNotifier - When a synchedObj 
// gets dirty, this callback will be called.
///////////////////////////////////
class plDirtyNotifier
{
protected:
	plKey fSynchedObjKey;
	void* fUserData;
public:	
	plDirtyNotifier() : fSynchedObjKey(nil),fUserData(nil) {}
	virtual ~plDirtyNotifier()
	{
		if (fSynchedObjKey)
		{
			plSynchedObject* so = plSynchedObject::ConvertNoRef(fSynchedObjKey->ObjectIsLoaded());
			if (so)
				so->RemoveDirtyNotifier(this);
		}
	}
	
	void SetSynchedObjKey(plKey k) { fSynchedObjKey=k; }	// should be set
	void SetUserData(void* v) { fUserData=v;}				// optional 

	plKey	GetSynchedObjKey() { return fSynchedObjKey; }
	void*	GetUserData() { return fUserData;}

	// override
	virtual void Callback() = 0;
};
#endif

//
// MACROS
//

#ifdef USE_SYNCHED_VALUES
#define SYNCHED_VALUE(type)				plSynchedValue<type>	
#define SYNCHED_TARRAY(type)			plSynchedTArray<type>	
#define SYNCHED_VALUE_FRIEND(type)		plSynchedValueFriend<type>	
#define SYNCHED_TARRAY_FRIEND(type)		plSynchedTArrayFriend<type>	
#else
#define SYNCHED_VALUE(type)				type
#define SYNCHED_TARRAY(type)			hsTArray<type>
#define SYNCHED_VALUE_FRIEND(type)		type
#define SYNCHED_TARRAY_FRIEND(type)		hsTArray<type>
#endif

#endif	// PLSYNCHOBJ_inc
