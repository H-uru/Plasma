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
#include "plRelevanceRegion.h"
#include "plRelevanceMgr.h"
#include "../plIntersect/plRegionBase.h"

void plRelevanceRegion::Read(hsStream* s, hsResMgr* mgr)
{
	plObjInterface::Read(s, mgr);
	
	mgr->ReadKeyNotifyMe(s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, 0), plRefFlags::kActiveRef);
	
	// Added to the manager when read in.
	// Removed when paged out, due to passive ref.
	if (plRelevanceMgr::Instance())
	{
		plGenRefMsg *msg = TRACKED_NEW plGenRefMsg(plRelevanceMgr::Instance()->GetKey(), plRefMsg::kOnCreate, -1, -1);
		hsgResMgr::ResMgr()->AddViaNotify(GetKey(), msg, plRefFlags::kPassiveRef);
	}
}

void plRelevanceRegion::Write(hsStream* s, hsResMgr* mgr)
{
	plObjInterface::Write(s, mgr);
	
	mgr->WriteKey(s, fRegion);
}

hsBool plRelevanceRegion::MsgReceive(plMessage* msg)
{
	plGenRefMsg *genMsg = plGenRefMsg::ConvertNoRef(msg);
	if (genMsg)
	{
		plRegionBase *base = plRegionBase::ConvertNoRef(genMsg->GetRef());
		if( genMsg->GetContext() & (plRefMsg::kOnCreate) )
		{
			fRegion = base;
		}
		else if( genMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
		{
			fRegion = nil;
		}
		return true;
	}			

	return plObjInterface::MsgReceive(msg);
}


void plRelevanceRegion::SetMgrIndex(UInt32 index)
{
	if (fMgrIdx != (UInt32)-1)
		fRegionsICareAbout.SetBit(fMgrIdx, false);

	fMgrIdx = index;
	fRegionsICareAbout.SetBit(index, true); // I care about myself. Awww...
}

