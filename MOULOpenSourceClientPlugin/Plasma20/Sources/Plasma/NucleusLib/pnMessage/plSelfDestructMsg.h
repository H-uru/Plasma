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

#ifndef plSelfDestructMsg_inc
#define plSelfDestructMsg_inc

#include "plMessage.h"

class plSelfDestructMsg : public plMessage
{
protected:
	plSelfDestructMsg(plKey &victim) : plMessage(victim, victim, nil) {}

	friend class plKeyImp;
public:

	plSelfDestructMsg() { hsAssert(false, "For system use only"); }

	CLASSNAME_REGISTER( plSelfDestructMsg );
	GETINTERFACE_ANY( plSelfDestructMsg, plMessage );

	virtual void Read(hsStream* stream, hsResMgr* mgr) { IMsgRead(stream, mgr); }
	virtual void Write(hsStream* stream, hsResMgr* mgr) { IMsgWrite(stream, mgr); }
	
};

#endif // plSelfDestructMsg_inc
