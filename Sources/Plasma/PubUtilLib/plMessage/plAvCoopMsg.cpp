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
#ifndef SERVER
#ifndef NO_AV_MSGS

// singular
#include "plAvCoopMsg.h"

// global
#include "hsStream.h"
#include "hsResMgr.h"

// other
#include "../plAvatar/plAvatarMgr.h"
#include "../plAvatar/plCoopCoordinator.h"

// plAvCoopMsg -----------
// ------------
plAvCoopMsg::plAvCoopMsg()
: plMessage(nil, nil, nil),
  fInitiatorID(0),
  fInitiatorSerial(0),
  fCommand(kNone),
  fCoordinator(nil)
{
}

// plAvCoopMsg ------------
// ------------
plAvCoopMsg::~plAvCoopMsg()
{
}

// plAvCoopMsg -----------------------------------
// ------------
plAvCoopMsg::plAvCoopMsg(Command cmd, UInt32 id, UInt16 serial)
: plMessage(nil, plAvatarMgr::GetInstance()->GetKey(), nil),
  fInitiatorID(id),
  fInitiatorSerial(serial),
  fCommand(cmd),
  fCoordinator(nil)
{

}

// plAvCoopMsg ----------------------------------
// ------------
plAvCoopMsg::plAvCoopMsg(plKey sender, plCoopCoordinator *coordinator)
: plMessage(sender, plAvatarMgr::GetInstance()->GetKey(), nil),
  fInitiatorID(coordinator->GetInitiatorID()),
  fInitiatorSerial(coordinator->GetInitiatorSerial()),
  fCommand(kStartNew),
  fCoordinator(coordinator)
{
}

// Read -----------------------------------------------
// -----
void plAvCoopMsg::Read(hsStream *stream, hsResMgr *mgr)
{
	plMessage::IMsgRead(stream, mgr);

	if(stream->Readbool())
		fCoordinator = reinterpret_cast<plCoopCoordinator *>(mgr->ReadCreatable(stream));

	fInitiatorID = stream->ReadSwap32();
	fInitiatorSerial = stream->ReadSwap16();

	fCommand = static_cast<Command>(stream->ReadSwap16());
}

// Write -----------------------------------------------
// ------
void plAvCoopMsg::Write(hsStream *stream, hsResMgr *mgr)
{
	plMessage::IMsgWrite(stream, mgr);

	stream->Writebool(fCoordinator != nil);
	if(fCoordinator)
		mgr->WriteCreatable(stream, fCoordinator);

	stream->WriteSwap32(fInitiatorID);
	stream->WriteSwap16(fInitiatorSerial);

	stream->WriteSwap16(fCommand);
}

#endif // ndef NO_AV_MSGS
#endif // ndef SERVER
