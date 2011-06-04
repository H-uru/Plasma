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
#include "hsResMgr.h"
#include "plNetClientGroup.h"
#include "../plResMgr/plKeyFinder.h"
#include "../plResMgr/plPageInfo.h"

//
// cache room desc string, from fID
//
void plNetClientGroups::ISetGroupDesc(plNetGroupId& grpId)
{
	if (grpId.Room() == plNetGroup::kNetGroupUnknown.Room())
		grpId.SetDesc("Unknown");
	else
	if (grpId.Room()== plNetGroup::kNetGroupLocalPlayer.Room())
		grpId.SetDesc("LocalPlayer");
	else
	if (grpId.Room()== plNetGroup::kNetGroupRemotePlayer.Room())
		grpId.SetDesc("RemotePlayer");
	else
	if (grpId.Room()== plNetGroup::kNetGroupLocalPhysicals.Room())
		grpId.SetDesc("LocalPhysicals");
	else
	if (grpId.Room()== plNetGroup::kNetGroupRemotePhysicals.Room())
		grpId.SetDesc("RemotePhysicals");
	else
	{
		const plPageInfo* pageInfo=plKeyFinder::Instance().GetLocationInfo(grpId.Room());
		grpId.SetDesc(pageInfo->GetPage());	
	}
}