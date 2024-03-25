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
#ifndef hsKeyedObject_h_inc
#define hsKeyedObject_h_inc

#include "plReceiver.h"
#include "plRefFlags.h"

#include "plKey.h"
#include "plFixedKey.h"

class hsStream;
class plRefMsg;
class plUoid;

class hsKeyedObject : public plReceiver
{
public:
    hsKeyedObject() = default;
    virtual ~hsKeyedObject();

    CLASSNAME_REGISTER(hsKeyedObject);
    GETINTERFACE_ANY(hsKeyedObject, plReceiver);

    const plKey&    GetKey() const { return fpKey; }
    ST::string      GetKeyName() const;

    virtual void Validate();

    // experimental; currently "is ready to process Loads"
    virtual bool IsFinal() { return true; };

    void Read(hsStream* s, hsResMgr* mgr) override;
    void Write(hsStream* s, hsResMgr* mgr) override;

    bool MsgReceive(plMessage* msg) override;

    // Send a reference to GetKey() via enclosed message. See plKey::SendRef()
    bool SendRef(plRefMsg* refMsg, plRefFlags::Type flags);

    //----------------------------------------
    // Fixed key functions
    // The following are to be matched pairs!
    //----------------------------------------
    plKey   RegisterAs(plFixedKeyId fixedKey);   // a fixed Key value from plFixedKey.h
    void    UnRegisterAs(plFixedKeyId fixedKey);

    // used when manually loading the player room
    plKey   RegisterAsManual(plUoid& uoid, const ST::string& p);
    void    UnRegisterAsManual(plUoid& uoid);

    // If you want clone keys to share a type of object, override this function
    // for it. (You can also return a new object that shares only some of the
    // original's data)
    virtual hsKeyedObject* GetSharedObject() { return nullptr; }

protected:
    friend class plResManager;

    virtual void    SetKey(plKey k);
    void            UnRegister();

private:
    plKey fpKey;
};

#endif // hsKeyedObject_h_inc
