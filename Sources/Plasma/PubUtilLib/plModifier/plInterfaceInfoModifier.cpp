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
#include "plInterfaceInfoModifier.h"
#include "hsResMgr.h"
#include "../pnKeyedObject/plKey.h"


plInterfaceInfoModifier::plInterfaceInfoModifier() 
{
}
plInterfaceInfoModifier::~plInterfaceInfoModifier()
{
	fKeyList.Reset();
}


void plInterfaceInfoModifier::Read(hsStream* s, hsResMgr* mgr)
{
	plSingleModifier::Read(s, mgr);
	int i = s->ReadSwap32();
	for (int x = 0; x < i; x++)
		fKeyList.Append(mgr->ReadKey(s));
}

void plInterfaceInfoModifier::Write(hsStream* s, hsResMgr* mgr)
{
	plSingleModifier::Write(s, mgr);
	s->WriteSwap32(fKeyList.Count());
	for (int i = 0; i < fKeyList.Count(); i++)
		mgr->WriteKey(s, fKeyList[i]);
}
