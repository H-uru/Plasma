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
#include "hsStream.h"
#include "plArmatureEffectMsg.h"
#include "../plAvatar/plArmatureEffects.h"

plArmatureEffectStateMsg::plArmatureEffectStateMsg() : 
	fSurface(plArmatureEffectsMgr::kFootNoSurface), fAddSurface(false) {}

plArmatureEffectStateMsg::~plArmatureEffectStateMsg() {}

void plArmatureEffectStateMsg::Read(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgRead(stream, mgr);
	
	fSurface = stream->ReadByte();
	fAddSurface = stream->ReadBool();
}

void plArmatureEffectStateMsg::Write(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgWrite(stream, mgr);

	stream->WriteByte(fSurface);
	stream->WriteBool(fAddSurface);
}
