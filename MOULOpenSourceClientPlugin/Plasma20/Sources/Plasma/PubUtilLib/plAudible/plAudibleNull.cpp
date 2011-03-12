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
#include "hsGeometry3.h"
#include "plAudibleNull.h"

#include "plgDispatch.h"
#include "hsResMgr.h"
#include "../pnMessage/plNodeRefMsg.h"
#include "../pnKeyedObject/plKey.h"

hsVector3	plAudibleNull::GetVelocity(int index) const
{
	hsVector3 ret(0,0,0);
	return(ret);
}
hsPoint3	plAudibleNull::GetPosition(int index)
{
	hsPoint3 ret(0,0,0);
	return(ret);
}

void plAudibleNull::SetSceneNode(plKey newNode)
{
	plKey oldNode = GetSceneNode();
	if( oldNode == newNode )
		return;

	if( newNode )
	{
		plNodeRefMsg* refMsg = TRACKED_NEW plNodeRefMsg(newNode, plNodeRefMsg::kOnRequest, -1, plNodeRefMsg::kAudible);
		hsgResMgr::ResMgr()->AddViaNotify(GetKey(), refMsg, plRefFlags::kActiveRef);
	}
	if( oldNode )
	{
		oldNode->Release(GetKey());
	}
}





