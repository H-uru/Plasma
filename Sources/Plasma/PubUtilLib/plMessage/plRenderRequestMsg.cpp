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

#include "hsTypes.h"
#include "plRenderRequestMsg.h"
#include "../pnKeyedObject/plUoid.h"
#include "../pnKeyedObject/plFixedKey.h"
#include "hsResMgr.h"


plRenderRequestMsg::plRenderRequestMsg(plKey sender, plRenderRequestBase* req)
:	plMessage(sender, nil, nil),
	fReq(req)
{
	plUoid oid( kClient_KEY );		// from plFixedKey.h
	plKey key = hsgResMgr::ResMgr()->FindKey(oid);
	AddReceiver(key);

	hsRefCnt_SafeRef(fReq);
}

plRenderRequestMsg::~plRenderRequestMsg()
{
	hsRefCnt_SafeUnRef(fReq);
}


plRenderRequestMsg::plRenderRequestMsg()
{
	hsAssert(false, "Improper usage, use argumented constructor");
}

void plRenderRequestMsg::Read(hsStream* s, hsResMgr* mgr)
{
	hsAssert(false, "Transmission/read/write of render requests not currently supported");
	plMessage::IMsgRead(s, mgr);

	fReq = nil;
}

void plRenderRequestMsg::Write(hsStream* s, hsResMgr* mgr)
{
	hsAssert(false, "Transmission/read/write of render requests not currently supported");
	plMessage::IMsgWrite(s, mgr);
}

plRenderRequestAck::plRenderRequestAck()
{
	hsAssert(false, "Improper usage, use argumented constructor");
}

plRenderRequestAck::plRenderRequestAck(plKey r, UInt32 userData)
:	plMessage(nil, r, nil),
	fUserData(userData)
{
}

void plRenderRequestAck::Read(hsStream* s, hsResMgr* mgr)
{
	hsAssert(false, "Transmission/read/write of render requests not currently supported");
	plMessage::IMsgRead(s, mgr);

}

void plRenderRequestAck::Write(hsStream* s, hsResMgr* mgr)
{
	hsAssert(false, "Transmission/read/write of render requests not currently supported");
	plMessage::IMsgWrite(s, mgr);
}

