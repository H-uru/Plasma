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
#include "plLightSpace.h"
#include "hsStream.h"
#include "hsResMgr.h"
#include "../plMessage/plLightRefMsg.h"
#include "../plMessage/plCollideMsg.h"
#include "plgDispatch.h"


hsBool plLightSpace::MsgReceive(plMessage* msg)
{
	plCollideMsg* collMsg = plCollideMsg::ConvertNoRef(msg);
	if( collMsg )
	{
		// HACK - CollideMsg doesn't have sufficient info yet. Need at least object
		// which is entering and leaving, and whether it is entering or leaving.
		plKey otherKey = nil;
		hsBool enter = true; 
		UInt8 ctx = enter ? plRefMsg::kOnRequest : plRefMsg::kOnRemove;
		plLightRefMsg* liMsg = TRACKED_NEW plLightRefMsg(GetKey(), otherKey, IGetLightInfo(), ctx);
		plgDispatch::MsgSend(liMsg);
		return true;
	}
	plLightRefMsg* liMsg = plLightRefMsg::ConvertNoRef(msg);
	if( liMsg )
	{
		if( liMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
			fLightInfo = liMsg->GetRef();
		else if( liMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
			fLightInfo = nil;

		return true;
	}
	return false;
}

void plLightSpace::Read(hsStream* s, hsResMgr* mgr)
{
	plMultiModifier::Read(s, mgr);

	mgr->ReadKeyNotifyMe(s, TRACKED_NEW plLightRefMsg(nil, GetKey(), nil, plRefMsg::kOnCreate), plRefFlags::kPassiveRef);
}

void plLightSpace::Write(hsStream* s, hsResMgr* mgr)
{
	plMultiModifier::Write(s, mgr);

	mgr->WriteKey(s, fLightInfo);
}

