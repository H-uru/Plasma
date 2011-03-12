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

#include "../pfGameGUIMgr/pfGUIDragBarCtrl.h"

#include "pyGUIControlDragBar.h"

pyGUIControlDragBar::pyGUIControlDragBar(pyKey& gckey) : pyGUIControl(gckey)
{
}

pyGUIControlDragBar::pyGUIControlDragBar(plKey objkey) : pyGUIControl(objkey)
{
}

pyGUIControlDragBar::~pyGUIControlDragBar()
{
}


hsBool pyGUIControlDragBar::IsGUIControlDragBar(pyKey& gckey)
{
	if ( gckey.getKey() && pfGUIDragBarCtrl::ConvertNoRef(gckey.getKey()->ObjectIsLoaded()) )
		return true;
	return false;
}


void pyGUIControlDragBar::Anchor( void )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIDragBarCtrl* pbmod = pfGUIDragBarCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
			pbmod->SetAnchored(true);
	}
}

void pyGUIControlDragBar::Unanchor( void )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIDragBarCtrl* pbmod = pfGUIDragBarCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
			pbmod->SetAnchored(false);
	}
}

hsBool pyGUIControlDragBar::IsAnchored()
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIDragBarCtrl* pbmod = pfGUIDragBarCtrl::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( pbmod )
			return pbmod->IsAnchored();
	}
	return false;
}
