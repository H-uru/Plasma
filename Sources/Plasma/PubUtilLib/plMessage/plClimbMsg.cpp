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
// singular
#include "plClimbMsg.h"

// global
#include "hsResMgr.h"
#include "hsStream.h"

plClimbMsg::plClimbMsg()
: fCommand(kNoCommand),
  fDirection(plClimbMsg::kCenter),
  fStatus(false),
  fTarget(nil)
{
	// nothing
}

plClimbMsg::plClimbMsg(const plKey &sender, const plKey &receiver, Command command, Direction direction, hsBool status, plKey target)
: plMessage(sender, receiver, nil),
  fCommand(command), fDirection(direction),
  fStatus(status),
  fTarget(target)
{
	// not here
}

void plClimbMsg::Read(hsStream *stream, hsResMgr *mgr)
{
	plMessage::IMsgRead(stream, mgr);

	fCommand = static_cast<Command>(stream->ReadSwap32());
	fDirection = static_cast<Direction>(stream->ReadSwap32());
	fStatus = stream->ReadBool();
	fTarget = mgr->ReadKey(stream);
}

void plClimbMsg::Write(hsStream *stream, hsResMgr *mgr)
{
	plMessage::IMsgWrite(stream, mgr);
	stream->WriteSwap32(static_cast<UInt32>(fCommand));
	stream->WriteSwap32(static_cast<UInt32>(fDirection));
	stream->WriteBool(fStatus);
	mgr->WriteKey(stream, fTarget);
}
