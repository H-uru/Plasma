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
#include "plNetClientRecorder.h"
#include "hsStream.h"
#include "hsTimer.h"

#include "../plNetMessage/plNetMessage.h"
#include "plCreatableIndex.h"
#include "hsResMgr.h"
#include "plgDispatch.h"
#include "../plSDL/plSDL.h"
#include "../pnNetCommon/plNetApp.h"

#include "../plMessage/plLinkToAgeMsg.h"
#include "../plMessage/plLoadAvatarMsg.h"
#include "../pnMessage/plNotifyMsg.h"
#include "../plMessage/plAgeLoadedMsg.h"

#include "../plStatusLog/plStatusLog.h"
#include "../plFile/hsFiles.h"

plNetClientRecorder::plNetClientRecorder(TimeWrapper* timeWrapper) :
fTimeWrapper(timeWrapper)
{
}

plNetClientRecorder::~plNetClientRecorder()
{
}

double plNetClientRecorder::GetTime()
{
	if (fTimeWrapper)
		return fTimeWrapper->GetWrappedTime();
	else
		return hsTimer::GetSysSeconds();
}

void plNetClientRecorder::IMakeFilename(const char* recName, char* path)
{
	strcpy(path, "Recordings" PATH_SEPARATOR_STR);
#if HS_BUILD_FOR_WIN32
	CreateDirectory(path, NULL);
#endif

	char* lastDot = strrchr(recName, '.');
	if (lastDot)
		strncat(path, recName, lastDot-recName);
	else
		strcat(path, recName);

	strcat(path, ".rec");
}

bool plNetClientRecorder::IsRecordableMsg(plNetMessage* msg) const
{
	UInt16 idx = msg->ClassIndex();

	return (
		idx == CLASS_INDEX_SCOPED(plNetMsgLoadClone) ||
		idx == CLASS_INDEX_SCOPED(plNetMsgSDLStateBCast) ||
		idx == CLASS_INDEX_SCOPED(plNetMsgSDLState) ||
		idx == CLASS_INDEX_SCOPED(plNetMsgGameMessage)
		);
}

plNetClientLoggingRecorder::plNetClientLoggingRecorder(TimeWrapper* timeWrapper) :
	plNetClientRecorder(timeWrapper),
	fPlaybackTimeOffset(0),
	fNextPlaybackTime(0),
	fLog(nil),
	fBetweenAges(true)
{
}

plNetClientLoggingRecorder::~plNetClientLoggingRecorder()
{
	delete fLog;
	fLog = nil;
}

bool plNetClientLoggingRecorder::IProcessRecordMsg(plNetMessage* msg, double secs)
{
	if (msg->ClassIndex() == CLASS_INDEX_SCOPED(plNetMsgGameMessage))
	{
		plNetMsgGameMessage* gameMsg = plNetMsgGameMessage::ConvertNoRef(msg);
		UInt16 gameMsgIdx = gameMsg->StreamInfo()->GetStreamType();

		if (gameMsgIdx == CLASS_INDEX_SCOPED(plServerReplyMsg))
			return false;

		// Throw out any notify messages that don't involve picking (running into
		// detectors and that sort of thing should be recreated automatically
		// during playback)
		if (gameMsgIdx == CLASS_INDEX_SCOPED(plNotifyMsg))
		{
			bool hasPick = false;

			plNotifyMsg* notifyMsg = plNotifyMsg::ConvertNoRef(gameMsg->GetContainedMsg());
			int numEvents = notifyMsg->GetEventCount();
			for (int i = 0; i < numEvents; i++)
			{
				proEventData* event = notifyMsg->GetEventRecord(i);
				if (event->fEventType == proEventData::kPicked)
					hasPick = true;
			}

			hsRefCnt_SafeUnRef(notifyMsg);

			if (!hasPick)
				return false;
		}
	}
	
	if (fPlaybackTimeOffset == 0)
		fPlaybackTimeOffset = secs;

	return true;
}

void plNetClientLoggingRecorder::RecordLinkMsg(plLinkToAgeMsg* linkMsg, double secs)
{
	if (!linkMsg->HasBCastFlag(plMessage::kNetSent))
	{
		plNetMsgGameMessage netMsgWrap;

		// write message (and label) to ram stream
		hsRAMStream stream;
		hsgResMgr::ResMgr()->WriteCreatable(&stream, linkMsg);

		// put stream in net msg wrapper
		netMsgWrap.StreamInfo()->CopyStream(&stream);
		// netMsgWrap.StreamInfo()->SetStreamType(linkMsg->ClassIndex());		// type of game msg

		RecordMsg(&netMsgWrap, secs);
	}
}
