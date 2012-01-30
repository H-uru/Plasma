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
#include "hsKeyedObject.h"
#include "plKeyImp.h"
#include "hsResMgr.h"
#include "pnDispatch/plDispatch.h"
#include "pnMessage/plSelfDestructMsg.h"

void hsKeyedObject::SetKey(plKey k)              
{
    if (fpKey != nil)
    {
        hsAssert(k == nil || k == fpKey, "Changing an object's key is not allowed");
        ((plKeyImp*)fpKey)->SetObjectPtr(nil); 
    }

    fpKey = k;

    if (fpKey != nil)
        ((plKeyImp*)fpKey)->SetObjectPtr(this); 
}   

hsBool hsKeyedObject::SendRef(plRefMsg* refMsg, plRefFlags::Type flags)
{
    plKey key = GetKey();       // for linux build
    return hsgResMgr::SendRef(key, refMsg, flags);
}

plString hsKeyedObject::GetKeyName() const
{
    if (fpKey)
        return fpKey->GetName();
    else
        return _TEMP_CONVERT_FROM_LITERAL("(unknown)");
}

hsKeyedObject::~hsKeyedObject()
{ 
    if( fpKey && fpKey->ObjectIsLoaded() )
    {
        // If our key is pointing to an object (presumably back to us),
        // then UnRegister will call SetObjectPtr(nil) will unregister the key (and us), which will
        // decrement our RefCnt. Unfortunately, we are here because of a call
        // to our destructor, in which case we don't want to go back into our
        // destructor again. So we'll just up the RefCnt, plKey::UnRegister will dec it back to 1.
        hsRefCnt_SafeRef(fpKey->ObjectIsLoaded());
    }
    UnRegister(); 
}

void hsKeyedObject::UnRegister()
{
    if (fpKey)
    {
        if (plgDispatch::Dispatch())
            plgDispatch::Dispatch()->UnRegisterAll(fpKey);
        
        ((plKeyImp *)fpKey)->SetObjectPtr(nil);
    }
}

plKey hsKeyedObject::RegisterAs(plFixedKeyId fixedKey)
{
    plUoid meUoid(fixedKey);

    hsAssert(meUoid.GetClassType() == ClassIndex(), "Registering as wrong type!");
    plKey key = hsgResMgr::ResMgr()->FindKey(meUoid);
    if (key == nil)
    {
        key = hsgResMgr::ResMgr()->NewKey(meUoid, this);
    }
    else
    {
        SetKey(key);
    }

    return key;
}


void hsKeyedObject::UnRegisterAs(plFixedKeyId fixedKey)
{
    plUoid uoid(fixedKey);
    UnRegisterAsManual(uoid);
}

plKey hsKeyedObject::RegisterAsManual(plUoid& meUoid, const plString& p)
{
    hsAssert(meUoid.GetClassType() == ClassIndex(),"Registering as wrong type!");
    // Really should be a NewKey() call just for fixed keys, so change this once player rooms behave
    plKey pkey = hsgResMgr::ResMgr()->ReRegister(p,meUoid);

    if (pkey)
        SetKey(pkey);
    return pkey;
}


void hsKeyedObject::UnRegisterAsManual(plUoid& inUoid)
{
    if (fpKey)
    {
        plUoid myUoid = fpKey->GetUoid();
        if (!(inUoid == myUoid))
        {
#if !HS_BUILD_FOR_UNIX      // disable for unix servers
            hsAssert(false,
                plString::Format("Request to Unregister wrong FixedKey, keyName=%s, inUoid=%s, myUoid=%s",
                    fpKey->GetName().s_str("?"), inUoid.StringIze().c_str(), myUoid.StringIze().c_str()).c_str());
#endif
        }
        ((plKeyImp*)fpKey)->UnRegister();
    }
}

void hsKeyedObject::Validate()
{
    const char* msg = "KeyedObject invalid!";

    if (fpKey)
    {
        hsAssert(fpKey->GetObjectPtr() == this, msg);
    }
}

void hsKeyedObject::Read(hsStream* s, hsResMgr* mgr)
{
    plKey pK = mgr->ReadKey(s);
    SetKey(pK);
}

void hsKeyedObject::Write(hsStream* s, hsResMgr* mgr)
{
    hsAssert(GetKey(),"hsKeyedObject:Must have a key!");
    mgr->WriteKey(s, fpKey);
}

hsBool hsKeyedObject::MsgReceive(plMessage* msg)
{
    plSelfDestructMsg* nuke = plSelfDestructMsg::ConvertNoRef(msg);
    if (nuke)
    {
        hsAssert(RefCnt() == 1, "Trying to selfdestruct with bogus refcnt");
        hsRefCnt_SafeUnRef(this); 

        return true;
    }
    return plReceiver::MsgReceive(msg);
}
