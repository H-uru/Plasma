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
    hsKeyedObject() : fpKey(nil) {}
    virtual ~hsKeyedObject();

	CLASSNAME_REGISTER(hsKeyedObject);
	GETINTERFACE_ANY(hsKeyedObject, plReceiver);

    const plKey&	GetKey() const { return fpKey; }
    const char*		GetKeyName() const;

    virtual void Validate();
	virtual hsBool	IsFinal() { return true; };		// experimental; currently "is ready to process Loads"

	virtual void Read(hsStream* s, hsResMgr* mgr);
	virtual void Write(hsStream* s, hsResMgr* mgr);

	virtual hsBool MsgReceive(plMessage* msg);

	//----------------------
	// Send a reference to GetKey() via enclosed message. See plKey::SendRef()
	//----------------------
	hsBool SendRef(plRefMsg* refMsg, plRefFlags::Type flags);

	//----------------------------------------
	// Fixed key functions
	// The following are to be matched pairs!
	//----------------------------------------
	plKey	RegisterAs(plFixedKeyId fixedKey);   // a fixed Key value from plFixedKey.h
	void	UnRegisterAs(plFixedKeyId fixedKey);

	// used when manually loading the player room
	plKey 	RegisterAsManual(plUoid& uoid, const char* p); 
	void	UnRegisterAsManual(plUoid& uoid);

	// If you want clone keys to share a type of object, override this function for it.
	// (You can also return a new object that shares only some of the original's data)
	virtual hsKeyedObject* GetSharedObject() { return nil; }

protected:
	friend class plResManager;

    virtual void	SetKey(plKey k);
	void			UnRegister();

private:
	plKey fpKey;
};

#endif // hsKeyedObject_h_inc
