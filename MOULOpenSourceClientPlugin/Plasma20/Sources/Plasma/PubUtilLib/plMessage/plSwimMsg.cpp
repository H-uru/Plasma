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
#include "plSwimMsg.h"

// global
#include "hsResMgr.h"
#include "hsStream.h"

// ctor default ------
// -------------
plSwimMsg::plSwimMsg()
: fIsEntering(false),
  fSwimRegionKey(nil)
{
}

// ctor sender receiver entering ----------------------------------------------
// ------------------------------
plSwimMsg::plSwimMsg(const plKey &sender, const plKey &receiver, bool entering, plKey regionKey)
: plMessage(sender, receiver, nil),
  fIsEntering(entering)
{
	fSwimRegionKey = regionKey;
}

// GetIsEntering --------------
// --------------
bool plSwimMsg::GetIsEntering()
{
	return fIsEntering;
}

// GetIsLeaving --------------
// -------------
bool plSwimMsg::GetIsLeaving()
{
	return !fIsEntering;
}

// Read ---------------------------------------------
// -----
void plSwimMsg::Read(hsStream *stream, hsResMgr *mgr)
{
	plMessage::IMsgRead(stream, mgr);

	fIsEntering = stream->Readbool();
	fSwimRegionKey = mgr->ReadKey(stream);
}

// Write ---------------------------------------------
// ------
void plSwimMsg::Write(hsStream *stream, hsResMgr *mgr)
{
	plMessage::IMsgWrite(stream, mgr);

	stream->Writebool(fIsEntering);
	mgr->WriteKey(stream, fSwimRegionKey);
}
