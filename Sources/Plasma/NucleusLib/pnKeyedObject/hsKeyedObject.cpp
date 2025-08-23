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
#include "plgDispatch.h"
#include "pnMessage/plSelfDestructMsg.h"

#include <string_theory/format>

void hsKeyedObject::SetKey(plKey k)              
{
    if (fpKey != nullptr)
    {
        hsAssert(k == nullptr || k == fpKey, "Changing an object's key is not allowed");
        plKeyImp::GetFromKey(fpKey)->SetObjectPtr(nullptr);
    }

    fpKey = std::move(k);

    if (fpKey != nullptr)
        plKeyImp::GetFromKey(fpKey)->SetObjectPtr(this); 
}   

bool hsKeyedObject::SendRef(plRefMsg* refMsg, plRefFlags::Type flags)
{
    plKey key = GetKey();       // for linux build
    return hsgResMgr::SendRef(key, refMsg, flags);
}

ST::string hsKeyedObject::GetKeyName() const
{
    if (fpKey)
        return fpKey->GetName();
    else
        return ST_LITERAL("(unknown)");
}

hsKeyedObject::~hsKeyedObject()
{ 
    if( fpKey && fpKey->ObjectIsLoaded() )
    {
        // If our key is pointing to an object (presumably back to us),
        // then UnRegister will call SetObjectPtr(nullptr) will unregister the key (and us), which will
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
        
        plKeyImp::GetFromKey(fpKey)->SetObjectPtr(nullptr);
    }
}

plKey hsKeyedObject::RegisterAs(plFixedKeyId fixedKey)
{
    plUoid meUoid(fixedKey);

    hsAssert(meUoid.GetClassType() == ClassIndex(), "Registering as wrong type!");
    plKey key = hsgResMgr::ResMgr()->FindKey(meUoid);
    if (key == nullptr)
    {
        key = hsgResMgr::ResMgr()->NewKey(meUoid, this);

        //  the key list "helpfully" assigns us an object id.
        // we don't want one for fixed keys however (initialization order might bite us in the ass)
        plKeyImp::GetFromKey(key)->SetObjectID(0);
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

plKey hsKeyedObject::RegisterAsManual(plUoid& meUoid, const ST::string& p)
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
            hsAssert(false,
                ST::format("Request to Unregister wrong FixedKey, keyName={}, inUoid={}, myUoid={}",
                           fpKey->GetName(), inUoid, myUoid).c_str());
        }
        plKeyImp::GetFromKey(fpKey)->UnRegister();
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

bool hsKeyedObject::MsgReceive(plMessage* msg)
{
    plSelfDestructMsg* nuke = plSelfDestructMsg::ConvertNoRef(msg);
    if (nuke)
    {
        hsAssert(RefCnt() == 1, "Trying to selfdestruct with bogus refcnt");
        UnRef();

        return true;
    }
    return plReceiver::MsgReceive(msg);
}
