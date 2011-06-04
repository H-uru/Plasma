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
#include "plgDispatch.h"
#include "plNetClientMgr.h"

#include "../pnNetCommon/pnNetCommon.h"
#include "../pnMessage/plTimeMsg.h"

#include "../plNetClientRecorder/plNetClientRecorder.h"

//
// make a recording of current play
//
bool plNetClientMgr::RecordMsgs(const char* recType, const char* recName)
{
	if (!fMsgRecorder)
	{
		if (stricmp(recType,"stream") == 0)
			fMsgRecorder = TRACKED_NEW plNetClientStreamRecorder;
		if (stricmp(recType,"stressstream") == 0)
			fMsgRecorder = TRACKED_NEW plNetClientStressStreamRecorder;
		if (stricmp(recType,"stats") == 0)
			fMsgRecorder = TRACKED_NEW plNetClientStatsRecorder;
		if (stricmp(recType,"streamandstats") == 0)
			fMsgRecorder = TRACKED_NEW plNetClientStreamAndStatsRecorder(TRACKED_NEW plNetClientStreamRecorder(), TRACKED_NEW plNetClientStatsRecorder());
		if (stricmp(recType,"stressstreamandstats") == 0)
			fMsgRecorder = TRACKED_NEW plNetClientStreamAndStatsRecorder(TRACKED_NEW plNetClientStressStreamRecorder(), TRACKED_NEW plNetClientStatsRecorder());

		if (!fMsgRecorder || !fMsgRecorder->BeginRecording(recName))
		{
			delete fMsgRecorder;
			fMsgRecorder = nil;
			return false;
		}

		return true;
	}

	return false;
}

//
// play a recording
//
bool plNetClientMgr::PlaybackMsgs(const char* recName)
{
	hsLogEntry(DebugMsg("DEMO: Beginning Playback"));

	plNetClientRecorder* player = TRACKED_NEW plNetClientStreamRecorder;

	if (player->BeginPlayback(recName))
	{
		fMsgPlayers.push_back(player);
//		plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
		return true;
	}
	else
	{
		delete player;
		return false;
	}
}

//
//
//
void plNetClientMgr::IPlaybackMsgs()
{
	for (int i = 0; i < fMsgPlayers.size(); i++)
	{
		plNetClientRecorder* recorder = fMsgPlayers[i];
		if (recorder->IsQueueEmpty())
		{
			delete recorder;
			fMsgPlayers.erase(fMsgPlayers.begin()+i);
			i--;

			if (fMsgPlayers.empty())
			{
//				plgDispatch::Dispatch()->UnRegisterForExactType(plEvalMsg::Index(), GetKey());
			}
		}
		else
		{
			while (plNetMessage* msg = recorder->GetNextMessage())
			{
				hsLogEntry(DebugMsg("<Recorded Msg>"));
				fMsgHandler.ReceiveMsg(msg);
			}
		}
	}
}


