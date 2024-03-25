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

#include "plSynchedObject.h"

#include "hsResMgr.h"
#include "hsStream.h"

#include "plNetApp.h"
#include "plNetGroup.h"

#include "pnKeyedObject/plKey.h"
#include "pnMessage/plSDLModifierMsg.h"
#include "pnMessage/plSetNetGroupIDMsg.h"

#ifdef USE_DIRTY_NOTIFIERS
#include <algorithm>
#endif

// statics
plSynchedObject* plSynchedObject::fStaticSynchedObj = nullptr;
std::vector<plSynchedObject::StateDefn> plSynchedObject::fDirtyStates;
std::vector<bool> plSynchedObject::fSynchStateStack;

plSynchedObject::plSynchedObject() : 
    fSynchFlags(0),
    fNetGroup(plNetGroup::kNetGroupUnknown)
{ 
    fStaticSynchedObj=this; 
}

plSynchedObject::~plSynchedObject()
{
}

bool plSynchedObject::MsgReceive(plMessage* msg)
{
    plSetNetGroupIDMsg* setNetGroupID = plSetNetGroupIDMsg::ConvertNoRef(msg);
    if (setNetGroupID)
    {
        SetNetGroupConstant(setNetGroupID->fId);
        return true;
    }

    return hsKeyedObject::MsgReceive(msg);
}

//
// send sdl state msg immediately
//
void plSynchedObject::SendSDLStateMsg(const ST::string& SDLStateName, uint32_t synchFlags /*SendSDLStateFlags*/)
{
    plSDLModifierMsg* sdlMsg = new plSDLModifierMsg(SDLStateName,
        (synchFlags & kBCastToClients) ? plSDLModifierMsg::kSendToServerAndClients : plSDLModifierMsg::kSendToServer /* action */);
    sdlMsg->SetFlags(synchFlags);
    hsAssert(GetKey(), "nil key on synchedObject?");
    sdlMsg->Send(GetKey());
}

//
// Tell an object to send an sdl state update.
// The request will get queued (returns true)
//
bool plSynchedObject::DirtySynchState(const ST::string& SDLStateName, uint32_t synchFlags /*SendSDLStateFlags*/)
{
    if (!IOKToDirty(SDLStateName))
    {
#if 0
        if (plNetClientApp::GetInstance())
            plNetClientApp::GetInstance()->DebugMsg("NotOKToDirty - Not queueing SDL state, obj {}, sdl {}",
                    GetKeyName(), SDLStateName);
#endif
        return false;
    }
    
    if (!IOKToNetwork(SDLStateName, &synchFlags))
    {
#if 0
        if (plNetClientApp::GetInstance())
            plNetClientApp::GetInstance()->DebugMsg("LocalOnly Object - Not queueing SDL msg, obj {}, sdl {}",
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
            synchFlags |= kSkipLocalOwnershipCheck;     // don't have to check again
        else
        {
            if (plNetClientApp::GetInstance())
                plNetClientApp::GetInstance()->DebugMsg("Queueing SDL state with 'maybe' ownership, obj {}, sdl {}",
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
void plSynchedObject::IAddDirtyState(plKey objKey, const ST::string& sdlName, uint32_t sendFlags)
{
    bool found=false;
    std::vector<StateDefn>::iterator it=fDirtyStates.begin();
    for( ; it != fDirtyStates.end(); it++)
    {
        if (it->fObjKey == objKey && it->fSDLName.compare_i(sdlName) == 0)
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
        fDirtyStates.emplace_back(std::move(objKey), sendFlags, sdlName);
    }   
    else
    {
#if 0
        plNetClientApp::GetInstance()->DebugMsg("Not queueing diplicate request for SDL state, obj {}, sdl {}",
                    objKey->GetName(), sdlName);
#endif
    }
}

//
// STATIC
//
void plSynchedObject::IRemoveDirtyState(const plKey& objKey, const ST::string& sdlName)
{ 
    std::vector<StateDefn>::iterator it=fDirtyStates.begin();
    for( ; it != fDirtyStates.end(); it++)
    {
        if (it->fObjKey == objKey && it->fSDLName.compare_i(sdlName) == 0)
        {
            fDirtyStates.erase(it);
            break;
        }
    }
}

void plSynchedObject::SetNetGroupConstant(plNetGroupId netGroup)
{
   ClearSynchFlagsBit(kHasConstantNetGroup);
   SetNetGroup(std::move(netGroup));   // may recurse
   SetSynchFlagsBit(kHasConstantNetGroup);
}

plNetGroupId plSynchedObject::SelectNetGroup(const plKey& rmKey)
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

void    plSynchedObject::Read(hsStream* stream, hsResMgr* mgr)
{
    hsKeyedObject::Read(stream, mgr);
    fNetGroup = GetKey()->GetUoid().GetLocation();

    stream->ReadLE32(&fSynchFlags);
    if (fSynchFlags & kExcludePersistentState)
    {
        uint16_t num = stream->ReadLE16();
        fSDLExcludeList.clear();
        for (uint16_t i = 0; i < num; i++)
        {
            ST::string s;
            plMsgStdStringHelper::Peek(s, stream);
            fSDLExcludeList.push_back(s);
        }
    }

    if (fSynchFlags & kHasVolatileState)
    {
        uint16_t num = stream->ReadLE16();
        fSDLVolatileList.clear();
        for (uint16_t i = 0; i < num; i++)
        {
            ST::string s;
            plMsgStdStringHelper::Peek(s, stream);
            fSDLVolatileList.push_back(s);
        }
    }
}

void    plSynchedObject::Write(hsStream* stream, hsResMgr* mgr)
{
    hsKeyedObject::Write(stream, mgr);
    stream->WriteLE32(fSynchFlags);

    if (fSynchFlags & kExcludePersistentState)
    {
        stream->WriteLE16((uint16_t)fSDLExcludeList.size());

        SDLStateList::iterator it=fSDLExcludeList.begin();
        for(; it != fSDLExcludeList.end(); it++)
        {
            plMsgStdStringHelper::Poke(*it, stream);
        }
    }

    if (fSynchFlags & kHasVolatileState)
    {
        stream->WriteLE16((uint16_t)fSDLVolatileList.size());

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
bool plSynchedObject::PopSynchDisabled() 
{ 
    if (fSynchStateStack.size())
    {
        bool ret=fSynchStateStack.back(); 
        fSynchStateStack.pop_back(); 
        return ret;
    }
    else
    {
        hsAssert(false, "invalid stack size?");
    }
    return true;    // disabled
}

#ifdef USE_DIRTY_NOTIFIERS
void plSynchedObject::AddDirtyNotifier(plDirtyNotifier* dn)
{
    if (dn)
    {
        std::vector<plDirtyNotifier*>::iterator it=std::find(fDirtyNotifiers.begin(), fDirtyNotifiers.end(), dn);
        if (it == fDirtyNotifiers.end())    // not there
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
        if (it != fDirtyNotifiers.end())    // its there
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
bool plSynchedObject::IOKToDirty(const ST::string& SDLStateName) const
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

    return true;    // OK to dirty
}

//
// return true if this object should send his SDL msg (for persistence or synch) over the net
//
bool plSynchedObject::IOKToNetwork(const ST::string& sdlName, uint32_t* synchFlags) const
{
    // determine destination
    bool dstServerOnly=false, dstClientsOnly=false, dstClientsAndServer=false;  

    if ((*synchFlags) & kBCastToClients)
    {   // bcasting to clients and server
        if ((*synchFlags) & kDontPersistOnServer)
            dstClientsOnly=true;
        else
            dstClientsAndServer=true;
    }
    else
    {   // not bcasting, must be sending to server only
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
            *synchFlags &= ~kBCastToClients;        // don't send to clients
        }
        if ( inExcludeList )
        {
            *synchFlags |= kDontPersistOnServer;    // don't store on server
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
 
plSynchedObject::SDLStateList::const_iterator plSynchedObject::IFindInSDLStateList(const SDLStateList& list, const ST::string& sdlName) const
{
    if (sdlName.empty())
        return list.end();  // false

    SDLStateList::const_iterator it = list.begin();
    for(; it != list.end(); it++)
        if (it->compare_i(sdlName) == 0)
            return it;

    return it;  // .end(), false
}

///////////////////////////
// EXCLUDE LIST
///////////////////////////

void plSynchedObject::AddToSDLExcludeList(const ST::string& sdlName)
{
    if (!sdlName.empty())
    {
        if (IFindInSDLStateList(fSDLExcludeList, sdlName)==fSDLExcludeList.end())
        {
            fSDLExcludeList.push_back(sdlName);
            fSynchFlags |= kExcludePersistentState;
        }
    }
}

void plSynchedObject::RemoveFromSDLExcludeList(const ST::string& sdlName)
{
    SDLStateList::const_iterator it=IFindInSDLStateList(fSDLExcludeList, sdlName);
    if (it != fSDLExcludeList.end())
    {
        fSDLExcludeList.erase(fSDLExcludeList.begin()+(it-fSDLExcludeList.begin()));
        if (fSDLExcludeList.size()==0)
            fSynchFlags &= ~kExcludePersistentState;
    }
}

bool plSynchedObject::IsInSDLExcludeList(const ST::string& sdlName) const
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

void plSynchedObject::AddToSDLVolatileList(const ST::string& sdlName)
{
    if (!sdlName.empty())
    {
        if (IFindInSDLStateList(fSDLVolatileList,sdlName)==fSDLVolatileList.end())
        {
            fSDLVolatileList.push_back(sdlName); // Don't dupe sdlName, std::string will copy
            fSynchFlags |= kHasVolatileState;
        }
    }
}

void plSynchedObject::RemoveFromSDLVolatileList(const ST::string& sdlName)
{
    SDLStateList::const_iterator it=IFindInSDLStateList(fSDLVolatileList,sdlName);
    if (it != fSDLVolatileList.end())
    {
        fSDLVolatileList.erase(fSDLVolatileList.begin()+(it-fSDLVolatileList.begin()));
        if (fSDLVolatileList.size()==0)
            fSynchFlags &= ~kHasVolatileState;
    }
}

bool plSynchedObject::IsInSDLVolatileList(const ST::string& sdlName) const
{
    if ((fSynchFlags & kAllStateIsVolatile) != 0)
        return true;
    
    if ((fSynchFlags & kHasVolatileState) == 0)
        return false;

    SDLStateList::const_iterator it=IFindInSDLStateList(fSDLVolatileList,sdlName);
    return (it != fSDLVolatileList.end());
}

