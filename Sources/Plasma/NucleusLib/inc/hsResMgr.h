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
#ifndef hsResMgr_inc
#define hsResMgr_inc

#include "HeadSpin.h"
#include "hsRefCnt.h"
#include "plLoadMask.h"
#include "plRefFlags.h"

class hsStream;
class plKey;
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
    virtual void  Load  (const plKey& objKey)=0;     // places on list to be loaded
    virtual bool  Unload(const plKey& objKey)=0;       // Unregisters (deletes) an object, Return true if successful
    virtual plKey CloneKey(const plKey& objKey)=0;

    //---------------------------
    //  Finding Functions
    //---------------------------
    virtual plKey FindKey(const plUoid& uoid)=0;    

    //---------------------------
    //  Establish reference linkage 
    //---------------------------
    virtual bool  AddViaNotify(const plKey& sentKey, plRefMsg* msg, plRefFlags::Type flags)=0;
    virtual bool  AddViaNotify(plRefMsg* msg, plRefFlags::Type flags)=0; // msg->fRef->GetKey() == sentKey

    virtual bool  SendRef(const plKey& key, plRefMsg* refMsg, plRefFlags::Type flags)=0;
    virtual bool  SendRef(hsKeyedObject* ko, plRefMsg* refMsg, plRefFlags::Type flags)=0;

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
    virtual plCreatable*    ReadCreatable(hsStream* s)=0;
    virtual void            WriteCreatable(hsStream* s, plCreatable* cre)=0;

    virtual plCreatable*    ReadCreatableVersion(hsStream* s)=0;
    virtual void            WriteCreatableVersion(hsStream* s, plCreatable* cre)=0;
    
    //---------------------------
    // Registry Modification Functions
    //---------------------------
    virtual plKey NewKey(const ST::string& name, hsKeyedObject* object, const plLocation& loc, const plLoadMask& m = plLoadMask::kAlways)=0;
    virtual plKey NewKey(plUoid& newUoid, hsKeyedObject* object)=0;

    virtual plDispatchBase* Dispatch()=0;

    virtual void BeginShutdown() {}

protected:
    friend class hsKeyedObject;
    friend class plKey;
    friend class plKeyImp;
    friend class plArmatureMod; // Temp hack until a findkey/clone issue is fixed. -Bob

    virtual plKey   ReRegister(const ST::string& nm, const plUoid& oid)=0;
    virtual bool    ReadObject(plKeyImp* key)=0;  // plKeys call this when needed

    // Sets a key as used or unused in the registry.  When all keys in a page of a
    // particular type in an page are unused, we can free the memory associated with them.
    // Only called by plKeyImp
    virtual void IKeyReffed(plKeyImp* key)=0;
    virtual void IKeyUnreffed(plKeyImp* key)=0;

protected:  // hsgResMgr only
    friend class hsgResMgr;
    virtual bool IReset()=0;
    virtual bool IInit()=0;
    virtual void IShutdown()=0;
};


class hsgResMgr
{
private:
    static hsResMgr* fResMgr;

public:
    static hsResMgr* ResMgr() { return (hsResMgr*)fResMgr; }

    // External modifier use only
    static void SetTheResMgr(hsResMgr* mgr) { fResMgr = mgr; }

    static plDispatchBase* Dispatch() { hsAssert(fResMgr, "No resmgr"); return fResMgr->Dispatch(); }

    static bool Init(hsResMgr* m);
    static bool Reset() { return fResMgr->IReset(); }
    static void Shutdown();
    
    static bool SendRef(plKey& key, plRefMsg* refMsg, plRefFlags::Type flags) { return fResMgr->SendRef(key, refMsg, flags); }
    static bool SendRef(hsKeyedObject* ko, plRefMsg* refMsg, plRefFlags::Type flags) { return fResMgr->SendRef(ko, refMsg, flags); }
};

#endif // hsResMgr_inc

