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

#ifndef plAttachMsg_inc
#define plAttachMsg_inc

#include "plRefMsg.h"
#include "hsStream.h"

class hsResMgr;

class plAttachMsg : public plRefMsg
{
public:
	plAttachMsg() {}
	// child is the plSceneObject being added as a child to the receiver. If rcv is not loaded, this will have no effect.
	// flags should be either:
	//		plRefMsg::kOnRequest - I'm adding this child to the receiver
	//		plRefMsg::kOnRemove - I'm detaching this child from the receiver
	plAttachMsg(const plKey &rcv, hsKeyedObject* child, UInt8 context, const plKey snd=nil) : plRefMsg(rcv, context) { SetSender(snd); SetRef(child); }

	CLASSNAME_REGISTER( plAttachMsg );
	GETINTERFACE_ANY( plAttachMsg, plRefMsg );

};

#endif // plAttachMsg_inc
