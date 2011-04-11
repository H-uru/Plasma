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
#include "plServerReplyMsg.h"
#include "hsStream.h"
#include "hsBitVector.h"

void plServerReplyMsg::Read(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgRead(stream, mgr);
	stream->ReadSwap(&fType);
}

void plServerReplyMsg::Write(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgWrite(stream, mgr);
	stream->WriteSwap(fType);
}

enum ServerReplyFlags
{
	kServerReplyType,
};

void plServerReplyMsg::ReadVersion(hsStream* s, hsResMgr* mgr)
{
	plMessage::IMsgReadVersion(s, mgr);

	hsBitVector contentFlags;
	contentFlags.Read(s);

	if (contentFlags.IsBitSet(kServerReplyType))
		s->ReadSwap(&fType);
}

void plServerReplyMsg::WriteVersion(hsStream* s, hsResMgr* mgr)
{
	plMessage::IMsgWriteVersion(s, mgr);

	hsBitVector contentFlags;
	contentFlags.SetBit(kServerReplyType);
	contentFlags.Write(s);

	// kServerReplyType
	s->WriteSwap(fType);
}
