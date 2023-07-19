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
#ifndef plKeyImp_inc
#define plKeyImp_inc

#include "plKey.h"
#include "plUoid.h"
#include "hsBitVector.h"
#include "plRefFlags.h"

//------------------------------------
// plKey is a handle to a keyedObject
//------------------------------------
class plKeyImp : public plKeyData 
{
private:
    static hsKeyedObject* SafeGetObject(const plKeyImp* key);

public:
    plKeyImp();
    plKeyImp(plUoid, uint32_t pos,uint32_t len);
    virtual ~plKeyImp();

    static plKeyImp* GetFromKey(const plKey& key)
    {
        return static_cast<plKeyImp*>(&*key);
    }

    const plUoid&           GetUoid() const override { return fUoid; }
    ST::string              GetName() const override;

    hsKeyedObject*  GetObjectPtr() override;
    hsKeyedObject*  ObjectIsLoaded() const override;
    hsKeyedObject*  VerifyLoaded() override;

    // called before writing to disk so that static keys can have faster lookups (int compare instead of string compare)
    void SetObjectID(uint32_t id) {fUoid.SetObjectID(id);}

    //----------------------
    // I/O
    // ResMgr performs read, so it can search for an existing instance....
    //----------------------
    void Read(hsStream* s);
    void Write(hsStream* s);
    void WriteObject(hsStream* s);
    // For when you need to skip over a key in a stream
    static void SkipRead(hsStream* s);

    uint32_t GetStartPos() const  { return fStartPos; } // for ResMgr to read the Objects
    uint32_t GetDataLen() const   { return fDataLen;  } // for ResMgr to read the Objects

    //----------------------
    // Allow a keyed object to behave as if it has an active ref when in fact the object
    // should only be active ref'ed by a non-keyed parent.  Essentially just bumps/decs
    // the active ref count to facilitate normal object creation/destruction
    //----------------------
    hsKeyedObject*  RefObject(plRefFlags::Type flags = plRefFlags::kActiveRef) override;
    void            UnRefObject(plRefFlags::Type flags = plRefFlags::kActiveRef) override;

    //----------------------
    // Release has two behaviors, depending on whether the ref is active or passive:
    // Active - Release decs the ActiveRefCnt. When it gets to zero, the object will be deleted.
    // Passive - Unregisters my interest in when the object is created or destroyed.
    //----------------------
    void Release(plKey targetKey) override;

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
    void    AddClone(plKeyImp* c);
    void    RemoveClone(plKeyImp* c) const;
    plKey   GetClone(uint32_t playerID, uint32_t cloneID) const;
    void    CopyForClone(const plKeyImp* p, uint32_t playerID, uint32_t cloneID);    // Copy the contents of p for cloning process

    size_t  GetNumClones();
    plKey   GetCloneByIdx(size_t idx);
    plKey   GetCloneOwner() const { return fCloneOwner; }

    void NotifyCreated();
    void ISetupNotify(plRefMsg* msg, plRefFlags::Type flags); // Setup notifcations for reference, don't send anything.

    void        AddRef(plKeyImp* key) const;
    size_t      GetNumRefs() const { return fRefs.size(); }
    plKeyImp*   GetRef(size_t i) const { return fRefs[i]; }
    void        RemoveRef(plKeyImp *key) const;

    uint16_t    GetActiveRefs() const override          { return fNumActiveRefs; }
    size_t      GetNumNotifyCreated() const override    { return fNotifyCreated.size(); }
    plRefMsg*   GetNotifyCreated(size_t i) const override { return fNotifyCreated[i]; }
    const hsBitVector& GetActiveBits() const override   { return fActiveRefs; }

protected:
    void        AddNotifyCreated(plRefMsg* msg, plRefFlags::Type flags);
    void        ClearNotifyCreated();
    void        RemoveNotifyCreated(size_t i);

    uint16_t      IncActiveRefs() { return ++fNumActiveRefs; }
    uint16_t      DecActiveRefs() { return fNumActiveRefs ? --fNumActiveRefs : 0; }

    bool    IsActiveRef(size_t i) const          { return fActiveRefs.IsBitSet(i) != 0; }
    void    SetActiveRef(size_t i, bool on=true) { fActiveRefs.SetBit(i, on); }
    bool    IsNotified(size_t i) const           { return fNotified.IsBitSet(i) != 0; }
    void    SetNotified(size_t i, bool on=true)  { fNotified.SetBit(i, on); }

    void SatisfyPending(plRefMsg* msg) const;
    void SatisfyPending() const;

    void INotifySelf(hsKeyedObject* ko);
    void INotifyDestroyed();
    void IClearRefs();

    void IRelease(plKeyImp* keyImp);

    hsKeyedObject* fObjectPtr;

    // These fields are the ones actually saved to disk
    plUoid fUoid;
    uint32_t fStartPos;   // where I live in the Datafile  
    uint32_t fDataLen;    // Length in the Datafile

    // Following used by hsResMgr to notify on defered load or when a passive ref is destroyed.
    uint16_t                    fNumActiveRefs; // num active refs on me
    hsBitVector                 fActiveRefs;    // Which of notify created are active refs
    hsBitVector                 fNotified;      // which of notifycreated i've already notified.
    std::vector<plRefMsg*>      fNotifyCreated; // people to notify when I'm created or destroyed
    mutable std::vector<plKeyImp*>  fRefs;      // refs I've made (to be released when I'm unregistered).
    mutable int16_t             fPendingRefs;   // Outstanding requests I have out.
    mutable std::vector<plKeyImp*>  fClones;    // clones of me
    mutable plKey               fCloneOwner;    // pointer for clones back to the owning key

    friend class pfConsoleActiveRefPeeker;
    friend class plPageOptimizer; // needs to update fStartPos
};

#endif // hsRegistry_inc
