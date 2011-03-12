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
#ifndef plMsgForwarder_h_inc
#define plMsgForwarder_h_inc

#include "hsKeyedObject.h"
#include "hsTemplates.h"
#include "hsStlUtils.h"


class plMessageWithCallbacks;
class plForwardCallback;

class plMsgForwarder : public hsKeyedObject
{
protected:
	hsTArray<plKey> fForwardKeys;

	typedef std::map<plMessage*, plForwardCallback*> CallbackMap;
	CallbackMap fCallbacks;

	void IForwardMsg(plMessage *msg);
	hsBool IForwardCallbackMsg(plMessage *msg);

public:
    plMsgForwarder();
    ~plMsgForwarder();

	CLASSNAME_REGISTER(plMsgForwarder);
	GETINTERFACE_ANY(plMsgForwarder, hsKeyedObject);

	void Read(hsStream* s, hsResMgr* mgr);
	void Write(hsStream* s, hsResMgr* mgr);

	hsBool MsgReceive(plMessage* msg);

	void AddForwardKey(plKey key);
};

#endif // plMsgForwarder_h_inc
