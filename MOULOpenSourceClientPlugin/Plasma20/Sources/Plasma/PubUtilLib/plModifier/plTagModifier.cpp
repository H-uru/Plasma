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
#include "plTagModifier.h"

#include "hsResMgr.h"
#include "../pnMessage/plRemoteAvatarInfoMsg.h"
#include "../plMessage/plAvatarMsg.h"
//#include "../pfMessage/plClothingMsg.h"
#include "../plMessage/plCollideMsg.h"
#include "../plMessage/plSimInfluenceMsg.h"

#include "../plStatusLog/plStatusLog.h"

static plStatusLog* gLog = nil;

plTagModifier::plTagModifier()
{
	if (!gLog)
		gLog = plStatusLogMgr::GetInstance().CreateStatusLog(15, "Tag", plStatusLog::kFilledBackground | plStatusLog::kDeleteForMe | plStatusLog::kDontWriteFile | plStatusLog::kAlignToTop);
}

plTagModifier::~plTagModifier()
{
}

hsBool plTagModifier::MsgReceive(plMessage* msg)
{
	plCollideMsg* collideMsg = plCollideMsg::ConvertNoRef(msg);
	if (collideMsg)
	{
		gLog->AddLineF("Kicked by %s", collideMsg->fOtherKey->GetName());
		return true;
	}

	plRemoteAvatarInfoMsg* avInfoMsg = plRemoteAvatarInfoMsg::ConvertNoRef(msg);
	if (avInfoMsg)
	{
		// TODO
		// Check if the local av is frozen
//		plKey localAvKey = plNetClientMgr::GetInstance()->GetLocalPlayerKey();

		// Freeze clicked av
		plKey clickedAvKey = avInfoMsg->GetAvatarKey();
		if (clickedAvKey)
		{
			static hsBool tempHack = true;
			tempHack = !tempHack;

			plAvEnableMsg* avEnableMsg = new plAvEnableMsg(GetKey(), clickedAvKey, tempHack);
			avEnableMsg->SetBCastFlag(plMessage::kNetPropagate | plMessage::kPropagateToModifiers);
			avEnableMsg->Send();

			gLog->AddLineF("Tagged %s", clickedAvKey->GetName());
		}

		return true;
	}
	
	return plSingleModifier::MsgReceive(msg);
}

void plTagModifier::Read(hsStream* stream, hsResMgr* mgr)
{
	plSingleModifier::Read(stream, mgr);
}

void plTagModifier::Write(hsStream* stream, hsResMgr* mgr)
{
	plSingleModifier::Write(stream, mgr);
}

#include "plgDispatch.h"

void plTagModifier::SetTarget(plSceneObject* so)
{
	if (so)
		plgDispatch::Dispatch()->RegisterForExactType(plRemoteAvatarInfoMsg::Index(), GetKey());
	else
		plgDispatch::Dispatch()->UnRegisterForExactType(plRemoteAvatarInfoMsg::Index(), GetKey());
}
