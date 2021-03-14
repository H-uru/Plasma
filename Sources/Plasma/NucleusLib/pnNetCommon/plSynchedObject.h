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
#ifndef PLSYNCHOBJ_inc
#define PLSYNCHOBJ_inc

#include "HeadSpin.h"
#include "pnKeyedObject/hsKeyedObject.h"
#include "pnKeyedObject/plKey.h"
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
        kDontDirty              = 0x1,
        kSendReliably           = 0x2,      // object wants reliable send
        kHasConstantNetGroup    = 0x4,      // has a constant net group.
        kDontSynchGameMessages  = 0x8,      // don't send/recv game actions
        kExcludePersistentState = 0x10,     // don't send SDL state msgs to server, check exclude list
        kExcludeAllPersistentState=0x20,    // don't send ANY type of SDL state
        kLocalOnly              = (kExcludeAllPersistentState | kDontSynchGameMessages),    // localOnly in all respects
        kHasVolatileState       = 0x40,     // server won't save this state on shutdown
        kAllStateIsVolatile      = 0x80
    };

    enum SDLSendFlags   
    {
        kBCastToClients         = 0x1,
        kForceFullSend          = 0x2,
        kSkipLocalOwnershipCheck= 0x4,
        kSendImmediately        = 0x8,
        kDontPersistOnServer    = 0x10,     // for an SDL bcast msg which is used for synching only, not persisting
        kUseRelevanceRegions    = 0x20,
        kNewState               = 0x40,
        kIsAvatarState          = 0x80,
    };

    struct StateDefn
    {
        plKey     fObjKey;
        uint32_t  fSendFlags;
        ST::string  fSDLName;

        plSynchedObject* GetObject() const { return plSynchedObject::ConvertNoRef(fObjKey->ObjectIsLoaded()); }
        StateDefn() : fSendFlags() { }
        StateDefn(plKey k, uint32_t f, const ST::string& sdlName)
            : fObjKey(k), fSendFlags(f), fSDLName(sdlName) { }
    };

private:
    typedef std::vector<ST::string> SDLStateList;
    SDLStateList fSDLExcludeList;
    SDLStateList fSDLVolatileList;
    uint32_t fSynchFlags;

    plNetGroupId fNetGroup;

    static std::vector<bool> fSynchStateStack;
    static plSynchedObject* fStaticSynchedObj;      // static which temporarily holds address of each object's synchMgr
    static std::vector<StateDefn>   fDirtyStates;

    static void IRemoveDirtyState(plKey o, const ST::string& sdlName);
    static void IAddDirtyState(plKey o, const ST::string& sdlName, uint32_t sendFlags);
    bool IOKToDirty(const ST::string& SDLStateName) const;
    SDLStateList::const_iterator IFindInSDLStateList(const SDLStateList& list, const ST::string& sdlName) const;
protected:
    bool IOKToNetwork(const ST::string& sdlName, uint32_t* synchFlags) const;
public:
    plSynchedObject();
    virtual ~plSynchedObject();

    CLASSNAME_REGISTER(plSynchedObject);
    GETINTERFACE_ANY(plSynchedObject, hsKeyedObject);

    bool MsgReceive(plMessage* msg) override;

    // getters
    int GetSynchFlags() const { return fSynchFlags; }
    plNetGroupId GetNetGroup() const { return fNetGroup; };
    plNetGroupId GetEffectiveNetGroup() const;
    
    // setters
    void SetSynchFlagsBit(uint32_t f) { fSynchFlags |= f; }
    virtual void SetNetGroupConstant(plNetGroupId netGroup);
    virtual void SetNetGroup(plNetGroupId netGroup) { fNetGroup = netGroup; }   
    plNetGroupId SelectNetGroup(plKey groupKey);

    virtual bool DirtySynchState(const ST::string& sdlName, uint32_t sendFlags);
    void SendSDLStateMsg(const ST::string& SDLStateName, uint32_t synchFlags);  // don't use, only for net code

    void ClearSynchFlagsBit(uint32_t f) { fSynchFlags &= ~f; }

    // static 
    static bool GetSynchDisabled() { return fSynchStateStack.size() ? fSynchStateStack.back() : true; }
    static void PushSynchDisabled(bool b) { fSynchStateStack.push_back(b); }
    static bool PopSynchDisabled();
    static plSynchedObject* GetStaticSynchedObject() { return fStaticSynchedObj; }
    static size_t GetNumDirtyStates() { return fDirtyStates.size(); }
    static plSynchedObject::StateDefn* GetDirtyState(size_t i) { return &fDirtyStates[i]; }
    static void ClearDirtyState(std::vector<StateDefn>& carryOver) { fDirtyStates=carryOver; } 

    // IO 
//  void SendCreationMsg(double secs);
//  void SendDestructionMsg(double secs) ;

    void Read(hsStream* s, hsResMgr* mgr) override;
    void Write(hsStream* s, hsResMgr* mgr) override;

    int IsLocallyOwned() const;     // returns yes/no/maybe

    // disable net synching only
    bool IsNetSynched() const { return (fSynchFlags & kDontSynchGameMessages)==0; }
    void SetNetSynched(bool b) { if (!b) fSynchFlags |= kDontSynchGameMessages; else fSynchFlags &= ~kDontSynchGameMessages;    }

    // disable net synching AND persisting
    bool IsLocalOnly() const { return (fSynchFlags & kLocalOnly)==0;    }
    void SetLocalOnly(bool b) { if (b) fSynchFlags |= kLocalOnly; else fSynchFlags &= ~kLocalOnly;  }

    // disable particular types of persistence
    void AddToSDLExcludeList(const ST::string&);
    void RemoveFromSDLExcludeList(const ST::string&);
    bool IsInSDLExcludeList(const ST::string&) const;

    // make volatile particular types of state
    void AddToSDLVolatileList(const ST::string&);
    void RemoveFromSDLVolatileList(const ST::string&);
    bool IsInSDLVolatileList(const ST::string&) const;

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
    plSynchEnabler(bool enable) { plSynchedObject::PushSynchDisabled(!enable); }
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
    plDirtyNotifier() : fUserData() { }
    virtual ~plDirtyNotifier()
    {
        if (fSynchedObjKey)
        {
            plSynchedObject* so = plSynchedObject::ConvertNoRef(fSynchedObjKey->ObjectIsLoaded());
            if (so)
                so->RemoveDirtyNotifier(this);
        }
    }
    
    void SetSynchedObjKey(plKey k) { fSynchedObjKey=k; }    // should be set
    void SetUserData(void* v) { fUserData=v;}               // optional 

    plKey   GetSynchedObjKey() { return fSynchedObjKey; }
    void*   GetUserData() { return fUserData;}

    // override
    virtual void Callback() = 0;
};
#endif

#endif  // PLSYNCHOBJ_inc
