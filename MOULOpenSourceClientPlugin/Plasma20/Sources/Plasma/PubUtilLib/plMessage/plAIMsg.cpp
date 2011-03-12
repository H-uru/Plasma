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
#ifndef SERVER // we use stuff the server doesn't link with
#ifndef NO_AV_MSGS

#include "plAIMsg.h"

#include "hsResMgr.h"
#include "hsStream.h"

#include "..\plAvatar\plArmatureMod.h"

///////////////////////////////////////////////////////////////////////////////

plAIMsg::plAIMsg(): plMessage(nil, nil, nil), fBrainUserStr("")
{}

plAIMsg::plAIMsg(const plKey& sender, const plKey& receiver): plMessage(sender, receiver, nil)
{
	// set up our user string from the sender, if it is the right type
	plArmatureMod* armMod = plArmatureMod::ConvertNoRef(sender->ObjectIsLoaded());
	if (armMod)
		fBrainUserStr = armMod->GetUserStr();
	else
		fBrainUserStr = "";
}

void plAIMsg::Read(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgRead(stream, mgr);

	char* temp = stream->ReadSafeString();
	if (temp)
		fBrainUserStr = temp;
	delete [] temp;
}

void plAIMsg::Write(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgWrite(stream, mgr);

	stream->WriteSafeString(fBrainUserStr.c_str());
}

///////////////////////////////////////////////////////////////////////////////

void plAIArrivedAtGoalMsg::Read(hsStream* stream, hsResMgr* mgr)
{
	plAIMsg::Read(stream, mgr);
	fGoal.Read(stream);
}

void plAIArrivedAtGoalMsg::Write(hsStream* stream, hsResMgr* mgr)
{
	plAIMsg::Write(stream, mgr);
	fGoal.Write(stream);
}

#endif // NO_AV_MSGS
#endif // SERVER