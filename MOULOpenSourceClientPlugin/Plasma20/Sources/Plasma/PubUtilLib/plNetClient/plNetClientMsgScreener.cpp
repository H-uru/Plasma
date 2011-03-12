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
#include "plNetClientMsgScreener.h"
#include "plNetLinkingMgr.h"

#include "../pnNetCommon/plNetApp.h"
#include "../pnMessage/plMessage.h"

#include "../plStatusLog/plStatusLog.h"
#include "../plAvatar/plAvatarMgr.h"
#include "../plAvatar/plArmatureMod.h"

///////////////////////////////////////////////////////////////
// CLIENT Version
///////////////////////////////////////////////////////////////

plNetClientMsgScreener::plNetClientMsgScreener()
{
	DebugMsg("created");
}

//
// For plLoggable base
//
void plNetClientMsgScreener::ICreateStatusLog() const
{
	fStatusLog = plStatusLogMgr::GetInstance().CreateStatusLog(40, "NetScreener.log",
			plStatusLog::kTimestamp | plStatusLog::kFilledBackground | plStatusLog::kAlignToTop);	
}

//
// return cur age name
//
const char* plNetClientMsgScreener::IGetAgeName() const
{
	plNetLinkingMgr *lm = plNetLinkingMgr::GetInstance();
	return lm && lm->GetAgeLink()->GetAgeInfo() ? lm->GetAgeLink()->GetAgeInfo()->GetAgeFilename() : "?";
}

//
// Check if key is local avatar
//
bool plNetClientMsgScreener::IIsLocalAvatarKey(plKey key, const plNetGameMember* gm) const
{
	return (!key || key==plNetClientApp::GetInstance()->GetLocalPlayerKey());
}

bool plNetClientMsgScreener::IIsLocalArmatureModKey(plKey key, const plNetGameMember* gm) const 
{
	plKey playerKey = plNetClientApp::GetInstance()->GetLocalPlayerKey();
	plArmatureMod* aMod = playerKey ? plAvatarMgr::GetInstance()->FindAvatar(playerKey) : nil; 
	return (!key || key==(aMod ? aMod->GetKey() : nil));
}

//
// Check if CCR
//
bool plNetClientMsgScreener::IIsSenderCCR(const plNetGameMember* gm) const
{
	return plNetClientApp::GetInstance()->AmCCR();
}

//
// return true if msg is allowed/accepted as a net msg
//
bool plNetClientMsgScreener::AllowMessage(const plMessage* msg) const
{
	if (!msg)
		return false;

	Answer ans=IAllowMessageType(msg->ClassIndex());
	if (ans==kYes)
		return true;
	if (ans==kNo)
	{
		// WarningMsg("Quick-reject net propagated msg %s", msg->ClassName());
		return false;
	}

	if (!IValidateMessage(msg))
	{
		// WarningMsg("Validation failed.  Blocking net propagated msg %s", msg->ClassName());
		return false;
	}
	return true;
}
