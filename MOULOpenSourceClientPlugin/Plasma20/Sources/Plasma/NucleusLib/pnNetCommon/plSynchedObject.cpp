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
#include "plSynchedObject.h"
#include "plSynchedValue.h"
#include "plNetApp.h"
#include "plNetGroup.h"
#include "hsResMgr.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnMessage/plSDLModifierMsg.h"
#include "../pnMessage/plSetNetGroupIDMsg.h"

#include <algorithm>

// statics
plSynchedObject* plSynchedObject::fStaticSynchedObj=nil;
std::vector<plSynchedObject::StateDefn>	plSynchedObject::fDirtyStates;
std::vector<hsBool> plSynchedObject::fSynchStateStack;

plSynchedObject::plSynchedObject() : 
	fSynchFlags(0),
#ifdef USE_SYNCHED_VALUES
	fSynchedValueAddrOffsets(nil),
	fNumSynchedValues(0),
	fSynchedValueFriends(nil),
	fNumSynchedValueFriends(0),
#endif
	fNetGroup(plNetGroup::kNetGroupUnknown)
{ 
	fStaticSynchedObj=this; 
}

plSynchedObject::~plSynchedObject()
{
#ifdef USE_SYNCHED_VALUES
	delete [] fSynchedValueAddrOffsets;
	delete [] fSynchedValueFriends;
#endif
}

hsBool plSynchedObject::MsgReceive(plMessage* msg)
{
	plSetNetGroupIDMsg* setNetGroupID = plSetNetGroupIDMsg::ConvertNoRef(msg);
	if (setNetGroupID)
	{
		SetNetGroupConstant(setNetGroupID->fId);
		return true;
	}

	return hsKeyedObject::MsgReceive(msg);
}

#ifdef USE_SYNCHED_VALUES
plSynchedValueBase* plSynchedObject::GetSynchedValue(int i) const
{ 
	if (i<fNumSynchedValues)
		return IGetSynchedValue((NumSynchedValuesType)i);
	return IGetSynchedValueFriend((NumSynchedValuesType)(i-fNumSynchedValues));
}

// realloc and add 
void plSynchedObject::IAppendSynchedValueAddrOffset(AddrOffsetType synchedValueAddrOffset)
{
	// copy to new larger array
	AddrOffsetType* tmp = TRACKED_NEW AddrOffsetType[fNumSynchedValues+1];
	Int32 i;
	for(i=0;i<fNumSynchedValues;i++)
		tmp[i] = fSynchedValueAddrOffsets[i];

	// delete old one
	delete [] fSynchedValueAddrOffsets;

	// point to new array and append value
	fSynchedValueAddrOffsets=tmp;
	tmp[fNumSynchedValues++]=synchedValueAddrOffset;
}

void plSynchedObject::IAppendSynchedValueFriend(plSynchedValueBase* v)
{
	// copy to new larger array
	plSynchedValueBase** tmp = TRACKED_NEW plSynchedValueBase*[fNumSynchedValueFriends+1];
	Int32 i;
	for(i=0;i<fNumSynchedValueFriends;i++)
		tmp[i] = fSynchedValueFriends[i];

	// delete old one
	delete [] fSynchedValueFriends;

	// point to new array and append value
	fSynchedValueFriends=tmp;
	tmp[fNumSynchedValueFriends++]=v;
}

// adds synchedValue and returns index
UInt8 plSynchedObject::RegisterSynchedValue(plSynchedValueBase* v) 
{ 
	Int32 addrOff = ((Int32)v - (Int32)this)>>2;	
	hsAssert(hsABS(addrOff) < (UInt32)(1<<(sizeof(AddrOffsetType)<<3)), "address offset overflow");
	IAppendSynchedValueAddrOffset((AddrOffsetType)addrOff); 
	Int32 idx = fNumSynchedValues-1; 
	hsAssert(idx<256, "index too big");
	return (UInt8)idx;
}

hsBool plSynchedObject::RemoveSynchedValue(plSynchedValueBase* v) 
{
	int i;
	for(i=0;i<GetNumSynchedValues(); i++)
		if (GetSynchedValue(i)==v)
			break;
	
	// couldn't find it
	if (i==GetNumSynchedValues())
		return false;

	int idx=i;
	if (idx<fNumSynchedValues)
	{
		AddrOffsetType* tmp = TRACKED_NEW AddrOffsetType[fNumSynchedValues-1];		
		for(i=0;i<idx;i++)
			tmp[i] = fSynchedValueAddrOffsets[i];
		for(i=idx+1;i<fNumSynchedValues;i++)
			tmp[i-1] = fSynchedValueAddrOffsets[i];
		delete [] fSynchedValueAddrOffsets;
		fSynchedValueAddrOffsets=tmp;		
		fNumSynchedValues--;
	}
	else
	{
		idx -= fNumSynchedValues;
		plSynchedValueBase** tmp = TRACKED_NEW plSynchedValueBase*[fNumSynchedValueFriends-1];		
		for(i=0;i<idx;i++)
			tmp[i] = fSynchedValueFriends[i];
		for(i=idx+1;i<fNumSynchedValueFriends;i++)
			tmp[i-1] = fSynchedValueFriends[i];
		delete [] fSynchedValueFriends;
		fSynchedValueFriends=tmp;		
		fNumSynchedValueFriends--;
	}

	return true;
}

// adds synchedValueFriend
void plSynchedObject::RegisterSynchedValueFriend(plSynchedValueBase* v) 
{ 
	IAppendSynchedValueFriend(v);
}
#endif

//
// send sdl state msg immediately
//
void plSynchedObject::SendSDLStateMsg(const char* SDLStateName, UInt32 synchFlags /*SendSDLStateFlags*/)
{
	plSDLModifierMsg* sdlMsg = TRACKED_NEW plSDLModifierMsg(SDLStateName,
		(synchFlags & kBCastToClients) ? plSDLModifierMsg::kSendToServerAndClients : plSDLModifierMsg::kSendToServer /* action */);
	sdlMsg->SetFlags(synchFlags);
	hsAssert(GetKey(), "nil key on synchedObject?");
	sdlMsg->Send(GetKey());
}

//
// Tell an object to send an sdl state update.
// The request will get queued (returns true)
//
hsBool plSynchedObject::DirtySynchState(const char* SDLStateName, UInt32 synchFlags /*SendSDLStateFlags*/)
{
	if (!IOKToDirty(SDLStateName))
	{
#if 0
		if (plNetClientApp::GetInstance())
			plNetClientApp::GetInstance()->DebugMsg("NotOKToDirty - Not queueing SDL state, obj %s, sdl %s",
					GetKeyName(), SDLStateName);
#endif
		return false;
	}
	
	if (!IOKToNetwork(SDLStateName, &synchFlags))
	{
#if 0
		if (plNetClientApp::GetInstance())
			plNetClientApp::GetInstance()->DebugMsg("LocalOnly Object - Not queueing SDL msg, obj %s, sdl %s",
					GetKeyName(), SDLStateName);
#endif
		return false;
	}

	if (!(synchFlags & kSkipLocalOwnershipCheck))
	{
		int localOwned=IsLocallyOwned();
		if (localOwned==plSynchedObject::kNo)
			return false;
		if (localOwned==plSynchedObject::kYes)
			synchFlags |= kSkipLocalOwnershipCheck;		// don't have to check again
		else
		{
			if (plNetClientApp::GetInstance())
				plNetClientApp::GetInstance()->DebugMsg("Queueing SDL state with 'maybe' ownership, obj %s, sdl %s",
					GetKeyName(), SDLStateName);
		}
	}
	
	if (synchFlags & kSendImmediately)
	{
		SendSDLStateMsg(SDLStateName, synchFlags);
	}
	else
	{
		IAddDirtyState(GetKey(), SDLStateName, synchFlags);
	}
	return true;
}

//
// STATIC
// add state defn if not already there.
// if there adjust flags if necessary
//
void plSynchedObject::IAddDirtyState(plKey objKey, const char* sdlName, UInt32 sendFlags)
{
	bool found=false;
	std::vector<StateDefn>::iterator it=fDirtyStates.begin();
	for( ; it != fDirtyStates.end(); it++)
	{
		if ((*it).fObjKey==objKey && !stricmp((*it).fSDLName.c_str(), sdlName))
		{
			if (sendFlags & kForceFullSend)
				(*it).fSendFlags |= kForceFullSend;
			if (sendFlags & kBCastToClients)
				(*it).fSendFlags |= kBCastToClients;			
			found=true;
			break;
		}
	}

	if (!found)
	{
		StateDefn state(objKey, sendFlags, sdlName);
		fDirtyStates.push_back(state);
	}	
	else
	{
#if 0
		plNetClientApp::GetInstance()->DebugMsg("Not queueing diplicate request for SDL state, obj %s, sdl %s",
					objKey->GetName(), sdlName);
#endif
	}
}

//
// STATIC
//
void plSynchedObject::IRemoveDirtyState(plKey objKey, const char* sdlName)
{ 
	std::vector<StateDefn>::iterator it=fDirtyStates.begin();
	for( ; it != fDirtyStates.end(); it++)
	{
		if ((*it).fObjKey==objKey && !stricmp((*it).fSDLName.c_str(), sdlName))
		{
			fDirtyStates.erase(it);
			break;
		}
	}
}

void plSynchedObject::SetNetGroupConstant(plNetGroupId netGroup)
{
   ClearSynchFlagsBit(kHasConstantNetGroup);
   SetNetGroup(netGroup);	// may recurse
   SetSynchFlagsBit(kHasConstantNetGroup);
}

plNetGroupId plSynchedObject::SelectNetGroup(plKey rmKey)
{
	return plNetClientApp::GetInstance() ? 
      plNetClientApp::GetInstance()->SelectNetGroup(this, rmKey) : plNetGroup::kNetGroupUnknown;
}

plNetGroupId plSynchedObject::GetEffectiveNetGroup() const
{
	return plNetClientApp::GetInstance() ? 
		plNetClientApp::GetInstance()->GetEffectiveNetGroup(this) : plNetGroup::kNetGroupLocalPlayer;
}

int plSynchedObject::IsLocallyOwned() const
{ 
	return plNetClientApp::GetInstance() ? 
		plNetClientApp::GetInstance()->IsLocallyOwned(this) : kYes;
}

void	plSynchedObject::Read(hsStream* stream, hsResMgr* mgr)
{
	hsKeyedObject::Read(stream, mgr);
	fNetGroup = GetKey()->GetUoid().GetLocation();

	stream->ReadSwap(&fSynchFlags);
	if (fSynchFlags & kExcludePersistentState)
	{
		Int16 num;
		stream->ReadSwap(&num);
		fSDLExcludeList.clear();
		int i;
		for(i=0;i<num;i++)
		{
			std::string s;
			plMsgStdStringHelper::Peek(s, stream);
			fSDLExcludeList.push_back(s);
		}
	}

	if (fSynchFlags & kHasVolatileState)
	{
		Int16 num;
		stream->ReadSwap(&num);
		fSDLVolatileList.clear();
		int i;
		for(i=0;i<num;i++)
		{
			std::string s;
			plMsgStdStringHelper::Peek(s, stream);
			fSDLVolatileList.push_back(s);
		}
	}
}

void	plSynchedObject::Write(hsStream* stream, hsResMgr* mgr)
{
	hsKeyedObject::Write(stream, mgr);
	stream->WriteSwap(fSynchFlags);

	if (fSynchFlags & kExcludePersistentState)
	{
		Int16 num=fSDLExcludeList.size();
		stream->WriteSwap(num);

		SDLStateList::iterator it=fSDLExcludeList.begin();
		for(; it != fSDLExcludeList.end(); it++)
		{
			plMsgStdStringHelper::Poke(*it, stream);
		}
	}

	if (fSynchFlags & kHasVolatileState)
	{
		Int16 num=fSDLVolatileList.size();
		stream->WriteSwap(num);

		SDLStateList::iterator it=fSDLVolatileList.begin();
		for(; it != fSDLVolatileList.end(); it++)
		{
			plMsgStdStringHelper::Poke(*it, stream);
		}
	}
}


//
// static
//
hsBool plSynchedObject::PopSynchDisabled() 
{ 
	if (fSynchStateStack.size())
	{
		hsBool ret=fSynchStateStack.back(); 
		fSynchStateStack.pop_back(); 
		return ret;
	}
	else
	{
		hsAssert(false, "invalid stack size?");
	}
	return true;	// disabled
}

#ifdef USE_DIRTY_NOTIFIERS
void plSynchedObject::AddDirtyNotifier(plDirtyNotifier* dn)
{
	if (dn)
	{
		std::vector<plDirtyNotifier*>::iterator it=std::find(fDirtyNotifiers.begin(), fDirtyNotifiers.end(), dn);
		if (it == fDirtyNotifiers.end())	// not there
		{
			dn->SetSynchedObjKey(GetKey());
			fDirtyNotifiers.push_back(dn);	
		}
	}
}

void plSynchedObject::RemoveDirtyNotifier(plDirtyNotifier* dn)
{
	if (dn)
	{
		std::vector<plDirtyNotifier*>::iterator it=std::find(fDirtyNotifiers.begin(), fDirtyNotifiers.end(), dn);
		if (it != fDirtyNotifiers.end())	// its there
			fDirtyNotifiers.erase(it);	
	}
}

void plSynchedObject::CallDirtyNotifiers()
{
	int i;
	for(i=0;i<fDirtyNotifiers.size();i++)
		fDirtyNotifiers[i]->Callback();
}
#else
void plSynchedObject::CallDirtyNotifiers() {}
#endif

//
// return true if it's ok to dirty this object
//
bool plSynchedObject::IOKToDirty(const char* SDLStateName) const
{ 	
	// is synching disabled?
	bool synchDisabled = (GetSynchDisabled()!=0);
	if (synchDisabled)
		return false;
	
	// is object dirtyAble?
	bool dontDirty = (fSynchFlags & kDontDirty) != 0;
	if (dontDirty)
		return false;

	// is object in a LocalOnly age?
	if (plNetClientApp::GetConstInstance() && plNetClientApp::GetConstInstance()->ObjectInLocalAge(this))
		return false;

	return true;	// OK to dirty
}

//
// return true if this object should send his SDL msg (for persistence or synch) over the net
//
bool plSynchedObject::IOKToNetwork(const char* sdlName, UInt32* synchFlags) const
{
	// determine destination
	bool dstServerOnly=false, dstClientsOnly=false, dstClientsAndServer=false;	

	if ((*synchFlags) & kBCastToClients)
	{	// bcasting to clients and server
		if ((*synchFlags) & kDontPersistOnServer)
			dstClientsOnly=true;
		else
			dstClientsAndServer=true;
	}
	else
	{	// not bcasting, must be sending to server only
		hsAssert( ((*synchFlags) & kDontPersistOnServer)==0, "invalid synchedObject msg flag");
		dstServerOnly=true;
	}

	bool netSynched = IsNetSynched();
	bool inExcludeList = IsInSDLExcludeList(sdlName);

	//
	// check if ok to network based on destination
	//
	if (dstClientsOnly)
	{
		return netSynched;
	}

	if (dstClientsAndServer)
	{
		if ( !netSynched )
		{
			*synchFlags &= ~kBCastToClients;		// don't send to clients
		}
		if ( inExcludeList )
		{
			*synchFlags |= kDontPersistOnServer;	// don't store on server
		}

		return !inExcludeList || netSynched;
	}

	if (dstServerOnly)
	{
		return !inExcludeList;
	}

	hsAssert(false, "how did I get here");
	return false;
}
 
plSynchedObject::SDLStateList::const_iterator plSynchedObject::IFindInSDLStateList(const SDLStateList& list, const char* sdlName) const
{
	if (!sdlName)
		return list.end();	// false

	SDLStateList::const_iterator it = list.begin();
	for(; it != list.end(); it++)
		if (!_stricmp((*it).c_str(), sdlName))
			return it;

	return it;	// .end(), false
}

///////////////////////////
// EXCLUDE LIST
///////////////////////////

void plSynchedObject::AddToSDLExcludeList(const char* sdlName)
{
	if (sdlName)
	{
		if (IFindInSDLStateList(fSDLExcludeList, sdlName)==fSDLExcludeList.end())
		{
			fSDLExcludeList.push_back(sdlName); // Don't dupe sdlName, std::string will copy
			fSynchFlags |= kExcludePersistentState;
		}
	}
}

void plSynchedObject::RemoveFromSDLExcludeList(const char* sdlName)
{
	SDLStateList::const_iterator it=IFindInSDLStateList(fSDLExcludeList, sdlName);
	if (it != fSDLExcludeList.end())
	{
		fSDLExcludeList.erase(fSDLExcludeList.begin()+(it-fSDLExcludeList.begin()));
		if (fSDLExcludeList.size()==0)
			fSynchFlags &= ~kExcludePersistentState;
	}
}

bool plSynchedObject::IsInSDLExcludeList(const char* sdlName) const
{
	if ((fSynchFlags & kExcludeAllPersistentState) != 0)
		return true;
	
	if ((fSynchFlags & kExcludePersistentState) == 0)
		return false;

	SDLStateList::const_iterator it=IFindInSDLStateList(fSDLExcludeList, sdlName);
	return (it != fSDLExcludeList.end());
}

///////////////////////////
// VOLATILE LIST
///////////////////////////

void plSynchedObject::AddToSDLVolatileList(const char* sdlName)
{
	if (sdlName)
	{
		if (IFindInSDLStateList(fSDLVolatileList,sdlName)==fSDLVolatileList.end())
		{
			fSDLVolatileList.push_back(sdlName); // Don't dupe sdlName, std::string will copy
			fSynchFlags |= kHasVolatileState;
		}
	}
}

void plSynchedObject::RemoveFromSDLVolatileList(const char* sdlName)
{
	SDLStateList::const_iterator it=IFindInSDLStateList(fSDLVolatileList,sdlName);
	if (it != fSDLVolatileList.end())
	{
		fSDLVolatileList.erase(fSDLVolatileList.begin()+(it-fSDLVolatileList.begin()));
		if (fSDLVolatileList.size()==0)
			fSynchFlags &= ~kHasVolatileState;
	}
}

bool plSynchedObject::IsInSDLVolatileList(const char* sdlName) const
{
	if ((fSynchFlags & kAllStateIsVolatile) != 0)
		return true;
	
	if ((fSynchFlags & kHasVolatileState) == 0)
		return false;

	SDLStateList::const_iterator it=IFindInSDLStateList(fSDLVolatileList,sdlName);
	return (it != fSDLVolatileList.end());
}

