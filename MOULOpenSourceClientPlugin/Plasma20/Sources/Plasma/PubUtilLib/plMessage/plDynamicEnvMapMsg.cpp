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

#include "plDynamicEnvMapMsg.h"

#include "hsStream.h"

void plDynamicEnvMapMsg::Read(hsStream* s, hsResMgr* mgr)
{
	plMessage::IMsgRead(s, mgr);

	fCmd = s->ReadSwap32();
	
	fPos.Read(s);
	fHither = s->ReadSwapScalar();
	fYon = s->ReadSwapScalar();
	fFogStart = s->ReadSwapScalar();
	fColor.Read(s);
	fRefresh = s->ReadSwapScalar();
}

void plDynamicEnvMapMsg::Write(hsStream* s, hsResMgr* mgr)
{
	plMessage::IMsgWrite(s, mgr);

	s->WriteSwap32(fCmd);

	fPos.Write(s);
	s->WriteSwapScalar(fHither);
	s->WriteSwapScalar(fYon);
	s->WriteSwapScalar(fFogStart);
	fColor.Write(s);
	s->WriteSwapScalar(fRefresh);
}
