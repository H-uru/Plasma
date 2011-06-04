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
#include "../plNetMessage/plNetMessage.h"
#include "plCreatableIndex.h"
#include "plgDispatch.h"
#include "../plSDL/plSDL.h"
#include "../pnNetCommon/plNetApp.h"

#include "../plMessage/plLinkToAgeMsg.h"
#include "../plMessage/plLoadAvatarMsg.h"
#include "../plMessage/plLinkToAgeMsg.h"
#include "../pnMessage/plNotifyMsg.h"
#include "../plMessage/plAgeLoadedMsg.h"

#include "../plStatusLog/plStatusLog.h"


plNetClientStatsRecorder::plNetClientStatsRecorder(TimeWrapper* timeWrapper) :
plNetClientLoggingRecorder(timeWrapper)
{
	if (fLog)
		delete fLog;
	fLog = plStatusLogMgr::GetInstance().CreateStatusLog(30, "StatsRecorder.log", plStatusLog::kAlignToTop);
}

plNetClientStatsRecorder::~plNetClientStatsRecorder()
{
}

bool plNetClientStatsRecorder::BeginRecording(const char* recName)
{
	return true;
}

void plNetClientStatsRecorder::RecordMsg(plNetMessage* msg, double secs)
{
	if (IProcessRecordMsg(msg,secs))
	{
		char stats[256];
		sprintf(stats,"tm:%4.2f;sz:%u,plrid:%s%u : ",secs,msg->GetPackSize(),msg->GetHasPlayerID()?"":"XX",msg->GetHasPlayerID()?msg->GetPlayerID():0);
		// GetPackSize might compress the buffer on us, so uncompress it
		plNetMsgStreamedObject* so = plNetMsgStreamedObject::ConvertNoRef(msg);
		if (so)
			so->StreamInfo()->Uncompress();
		ILogMsg(msg,stats);
	}
}


void plNetClientStatsRecorder::ILogMsg(plNetMessage* msg, const char* preText)
{
	if (msg->ClassIndex() == CLASS_INDEX_SCOPED(plNetMsgGameMessage))
	{
		plNetMsgGameMessage* gameMsg = plNetMsgGameMessage::ConvertNoRef(msg);
		fLog->AddLineF("%s%s(%s)", preText, msg->ClassName(), plFactory::GetNameOfClass(gameMsg->StreamInfo()->GetStreamType()));

		if (gameMsg->StreamInfo()->GetStreamType() == CLASS_INDEX_SCOPED(plNotifyMsg))
		{
			plNotifyMsg* notifyMsg = plNotifyMsg::ConvertNoRef(gameMsg->GetContainedMsg());
			int numEvents = notifyMsg->GetEventCount();
			for (int i = 0; i < numEvents; i++)
			{
				const char* eventName = "";

				proEventData* event = notifyMsg->GetEventRecord(i);
				switch (event->fEventType)
				{
				case proEventData::kCollision:		eventName = "Collision";		break;
				case proEventData::kPicked:			eventName = "Picked";			break;
				case proEventData::kControlKey:		eventName = "ControlKey";		break;
				case proEventData::kVariable:		eventName = "Variable";			break;
				case proEventData::kFacing:			eventName = "Facing";			break;
				case proEventData::kContained:		eventName = "Contained";		break;
				case proEventData::kActivate:		eventName = "Activate";			break;
				case proEventData::kCallback:		eventName = "Callback";			break;
				case proEventData::kResponderState:	eventName = "ResponderState";	break;
				case proEventData::kMultiStage:		eventName = "MultiStage";		break;
				case proEventData::kSpawned:		eventName = "Spawned";			break;
				case proEventData::kClickDrag:		eventName = "ClickDrag";		break;
				}

				fLog->AddLineF("\t%s", eventName);
			}

			hsRefCnt_SafeUnRef(notifyMsg);
		}
	}
	else if (plNetMsgSDLState* sdlMsg = plNetMsgSDLState::ConvertNoRef(msg))
	{
		hsReadOnlyStream stream(sdlMsg->StreamInfo()->GetStreamLen(), sdlMsg->StreamInfo()->GetStreamBuf());		
		char* descName=nil;
		int ver;
		if (plStateDataRecord::ReadStreamHeader(&stream, &descName, &ver))
		{
			fLog->AddLineF("%s%s(%s)", preText, msg->ClassName(), descName);

			int i;

			plStateDataRecord sdRec(descName, ver);
			sdRec.Read(&stream, 0);
			plStateDataRecord::SimpleVarsList vars;
			sdRec.GetDirtyVars(&vars);
			for (i = 0; i < vars.size(); i++)
			{
				fLog->AddLineF("\t%s", vars[i]->GetVarDescriptor()->GetName());
			}

			plStateDataRecord::SDVarsList sdVars;
			sdRec.GetDirtySDVars(&sdVars);
			for (i = 0; i < sdVars.size(); i++)
			{
				fLog->AddLineF("\t%s", sdVars[i]->GetSDVarDescriptor()->GetName());
			}
		}
		delete [] descName;
	}
	else
		fLog->AddLineF("%s%s", preText, msg->ClassName());
}
