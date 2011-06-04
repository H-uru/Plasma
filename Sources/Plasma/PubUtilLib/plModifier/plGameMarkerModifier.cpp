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
#include "plGameMarkerModifier.h"
#include "../plMessage/plCollideMsg.h"

#include "../pnMessage/plNotifyMsg.h"
#include "../pnSceneObject/plSceneObject.h"

hsBool plGameMarkerModifier::MsgReceive(plMessage* msg)
{
	plCollideMsg *collideMsg = plCollideMsg::ConvertNoRef(msg);
	if (collideMsg)
	{
		if (collideMsg->fEntering)
		{
			plNotifyMsg* notify = TRACKED_NEW plNotifyMsg;
			notify->AddCollisionEvent(true, collideMsg->fOtherKey, GetTarget()->GetKey());
			notify->Send(hsgResMgr::ResMgr()->FindKey(kMarkerMgr_KEY));
		}
	}

	return plSingleModifier::MsgReceive(msg);
}

plKey plGameMarkerModifier::IFindCloneKey(plKey baseKey)
{
	const plUoid& myUoid = GetKey()->GetUoid();
	plUoid cloneUoid = baseKey->GetUoid();
	cloneUoid.SetClone(myUoid.GetClonePlayerID(), myUoid.GetCloneID());
	return hsgResMgr::ResMgr()->FindKey(cloneUoid);
}

void plGameMarkerModifier::FixupAnimKeys()
{
	fGreenAnimKey	= IFindCloneKey(fGreenAnimKey);
	fRedAnimKey		= IFindCloneKey(fRedAnimKey);
	fOpenAnimKey	= IFindCloneKey(fOpenAnimKey);
	fBounceAnimKey	= IFindCloneKey(fBounceAnimKey);
}

void plGameMarkerModifier::Read(hsStream* stream, hsResMgr* mgr)
{
	plSingleModifier::Read(stream, mgr);

	fGreenAnimKey	= mgr->ReadKey(stream);
	fRedAnimKey		= mgr->ReadKey(stream);
	fOpenAnimKey	= mgr->ReadKey(stream);
	fBounceAnimKey	= mgr->ReadKey(stream);
	fPlaceSndIdx	= stream->ReadSwap16();
	fHitSndIdx		= stream->ReadSwap16();
}

void plGameMarkerModifier::Write(hsStream* stream, hsResMgr* mgr)
{
	plSingleModifier::Write(stream, mgr);

	mgr->WriteKey(stream, fGreenAnimKey);
	mgr->WriteKey(stream, fRedAnimKey);
	mgr->WriteKey(stream, fOpenAnimKey);
	mgr->WriteKey(stream, fBounceAnimKey);
	stream->WriteSwap16(fPlaceSndIdx);
	stream->WriteSwap16(fHitSndIdx);
}
