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
//////////////////////////////////////////////
//
//
///////////////////////////////////////////////

#include "pyKey.h"

#include "pyGUIControlButton.h"
#include "../pfGameGUIMgr/pfGUIButtonMod.h"

pyGUIControlButton::pyGUIControlButton(pyKey& gckey) : pyGUIControl(gckey)
{
}

pyGUIControlButton::pyGUIControlButton(plKey objkey) : pyGUIControl(objkey)
{
}

hsBool pyGUIControlButton::IsGUIControlButton(pyKey& gckey)
{
	if ( gckey.getKey() && pfGUIButtonMod::ConvertNoRef(gckey.getKey()->ObjectIsLoaded()) )
		return true;
	return false;
}

void	pyGUIControlButton::SetNotifyType(Int32 kind)
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIButtonMod* butnmod = pfGUIButtonMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( butnmod )
			butnmod->SetNotifyType(kind);
	}
}

Int32	pyGUIControlButton::GetNotifyType()
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIButtonMod* butnmod = pfGUIButtonMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( butnmod )
			return butnmod->GetNotifyType();
	}
	return false;
}

hsBool	pyGUIControlButton::IsButtonDown()
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIButtonMod* butnmod = pfGUIButtonMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( butnmod )
			return butnmod->IsButtonDown();
	}
	return false;
}
