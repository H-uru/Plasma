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
#include "plProxyDrawMsg.h"
#include "hsStream.h"

plProxyDrawMsg::plProxyDrawMsg()
:	plMessage(nil, nil, nil), 
	fProxyFlags(0)
{
	SetBCastFlag(plMessage::kBCastByExactType);
}

plProxyDrawMsg::plProxyDrawMsg(UInt16 flags)
:	plMessage(nil, nil, nil),
	fProxyFlags(flags)
{
	SetBCastFlag(plMessage::kBCastByExactType);
}

plProxyDrawMsg::plProxyDrawMsg(plKey &rcv, UInt16 flags)
:	plMessage(rcv, rcv, nil),
	fProxyFlags(flags)
{
}

plProxyDrawMsg::~plProxyDrawMsg()
{
}

void plProxyDrawMsg::Read(hsStream* s, hsResMgr* mgr)
{
	plMessage::IMsgRead(s, mgr);
	fProxyFlags = s->ReadSwap16();
}

void plProxyDrawMsg::Write(hsStream* s, hsResMgr* mgr)
{
	plMessage::IMsgWrite(s, mgr);
	s->WriteSwap16(fProxyFlags);
}