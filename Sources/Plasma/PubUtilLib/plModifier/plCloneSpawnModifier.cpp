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
#include "plCloneSpawnModifier.h"

#include "hsResMgr.h"
#include "../plResMgr/plResManager.h"
#include "../plResMgr/plKeyFinder.h"
#include "../pnSceneObject/plSceneObject.h"

#include "../plScene/plSceneNode.h"
#include "../pnMessage/plClientMsg.h"
#include "plgDispatch.h"
#include "../pnMessage/plWarpMsg.h"
#include "../pnMessage/plNodeRefMsg.h"

plCloneSpawnModifier::plCloneSpawnModifier() : fTemplateName(nil), fExportTime(false)
{
}

plCloneSpawnModifier::~plCloneSpawnModifier()
{
	delete [] fTemplateName;
}

void plCloneSpawnModifier::Read(hsStream *s, hsResMgr *mgr)
{
	delete [] fTemplateName;
	fTemplateName = s->ReadSafeString();
	plSingleModifier::Read(s, mgr);
}

void plCloneSpawnModifier::Write(hsStream *s, hsResMgr *mgr)
{
	s->WriteSafeString(fTemplateName);
	plSingleModifier::Write(s, mgr);
}

void plCloneSpawnModifier::SetTemplateName(const char *templateName)
{
	delete [] fTemplateName;
	fTemplateName = hsStrcpy(templateName);
}

void plCloneSpawnModifier::SetTarget(plSceneObject* so)
{
	fTarget = so;
	// Spawning the clone here since at Read time fTarget isn't set.  Kind of a hack though.
	if (fTarget && !fExportTime)
	{
		// Assume the clone template is in the same age we are
		const plLocation& loc = GetKey()->GetUoid().GetLocation();
		char ageName[256];
		((plResManager*)hsgResMgr::ResMgr())->GetLocationStrings(loc, ageName, nil);

		// Spawn the clone
		plKey cloneKey = SpawnClone(fTemplateName, ageName, GetTarget()->GetLocalToWorld(), GetKey());
	}
}

#include "../plMessage/plLoadCloneMsg.h"

plKey plCloneSpawnModifier::SpawnClone(const char* cloneName, const char* cloneAge, const hsMatrix44& pos, plKey requestor)
{
	plResManager* resMgr = (plResManager*)hsgResMgr::ResMgr();

	// Find the clone room for this age
	const plLocation& loc = plKeyFinder::Instance().FindLocation(cloneAge, "BuiltIn");

	// Find the clone template key
	plUoid objUoid(loc, plSceneObject::Index(), cloneName);
	plKey key = resMgr->FindKey(objUoid);

	if (key)
	{
		plLoadCloneMsg* cloneMsg = TRACKED_NEW plLoadCloneMsg(objUoid, requestor, 0);
		cloneMsg->SetBCastFlag(plMessage::kMsgWatch);
		plKey cloneKey = cloneMsg->GetCloneKey();//resMgr->CloneKey(key);
		cloneMsg->Send();

		// Put the clone into the clone room, which also forces it to load.
		plKey cloneNodeKey = resMgr->FindKey(kNetClientCloneRoom_KEY);
		plNodeRefMsg* nodeRefCloneMsg = TRACKED_NEW plNodeRefMsg(cloneNodeKey, plNodeRefMsg::kOnRequest, -1, plNodeRefMsg::kObject);
		resMgr->AddViaNotify(cloneKey, nodeRefCloneMsg, plRefFlags::kActiveRef);

		// Warp it into position
		plWarpMsg *warpMsg = TRACKED_NEW plWarpMsg;
		warpMsg->AddReceiver(cloneKey);
		warpMsg->SetTransform(pos);
		plgDispatch::MsgSend(warpMsg);

		return cloneKey;
	}

	return nil;
}
