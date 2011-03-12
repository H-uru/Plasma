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
#include "hsResMgr.h"
#include "plgDispatch.h"
#include "../plSDL/plSDL.h"
#include "../pnNetCommon/plNetApp.h"

#include "../plMessage/plLinkToAgeMsg.h"
#include "../plMessage/plLoadAvatarMsg.h"
#include "../plMessage/plLinkToAgeMsg.h"
#include "../pnMessage/plNotifyMsg.h"
#include "../plMessage/plAgeLoadedMsg.h"

#include "../plStatusLog/plStatusLog.h"

plNetClientStreamRecorder::plNetClientStreamRecorder(TimeWrapper* timeWrapper) :
	plNetClientLoggingRecorder(timeWrapper),
	fRecordStream(nil),
	fResMgr(nil)
{
	if (fLog)
		delete fLog;
	fLog = plStatusLogMgr::GetInstance().CreateStatusLog(30, "StreamRecorder.log", plStatusLog::kAlignToTop);
}


plNetClientStreamRecorder::~plNetClientStreamRecorder()
{
	if (fRecordStream)
	{
		fRecordStream->Close();
		delete fRecordStream;
	}
}

hsResMgr* plNetClientStreamRecorder::GetResMgr()
{
	if (fResMgr)
		return fResMgr;
	else
		return hsgResMgr::ResMgr();
}

double plNetClientStreamRecorder::GetNextMessageTimeDelta()
{
	return fNextPlaybackTime - (GetTime() - fPlaybackTimeOffset);
}

enum NetClientRecFlags
{
	kNetClientRecSDLDesc,
};

bool plNetClientStreamRecorder::BeginRecording(const char* recName)
{
	if (!fRecordStream)
	{
		fRecordStream = TRACKED_NEW hsUNIXStream;
		char path[256];
		IMakeFilename(recName, path);

		if (!fRecordStream->Open(path, "wb"))
		{
			delete fRecordStream;
			return false;
		}

		hsBitVector contentFlags;
		contentFlags.SetBit(kNetClientRecSDLDesc);
		contentFlags.Write(fRecordStream);

		// kNetClientRecSDLDesc
		plSDLMgr::GetInstance()->Write(fRecordStream);

		return true;
	}

	return false;
}

bool plNetClientStreamRecorder::BeginPlayback(const char* recName)
{
	if (!fRecordStream)
	{
		fRecordStream = TRACKED_NEW hsUNIXStream;
		char path[256];
		IMakeFilename(recName, path);

		if (fRecordStream->Open(path, "rb"))
		{
			hsBitVector contentFlags;
			contentFlags.Read(fRecordStream);

			if (contentFlags.IsBitSet(kNetClientRecSDLDesc))
				plSDLMgr::GetInstance()->Read(fRecordStream);

			fPlaybackTimeOffset = GetTime();
			fNextPlaybackTime = fRecordStream->ReadSwapDouble();
			fBetweenAges = false;
		}
		else
		{
			delete fRecordStream;
			fRecordStream = nil;
			return false;
		}

		return true;
	}

	return false;
}

void plNetClientStreamRecorder::RecordMsg(plNetMessage* msg, double secs)
{
	if (!fRecordStream)
		return;

	if (IProcessRecordMsg(msg,secs))
	{
		fRecordStream->WriteSwapDouble(secs - fPlaybackTimeOffset);
		GetResMgr()->WriteCreatableVersion(fRecordStream, msg);

		ILogMsg(msg);
	}
}

void plNetClientStreamRecorder::RecordAgeLoadedMsg(plAgeLoadedMsg* ageLoadedMsg)
{
	fLog->AddLineF("Age %s", ageLoadedMsg->fLoaded ? "Loaded" : "Unloaded");

	if (ageLoadedMsg->fLoaded)
	{
		fBetweenAges = false;
		fPlaybackTimeOffset = GetTime();
	}
	else
	{
		fBetweenAges = true;
	}
}

bool plNetClientStreamRecorder::IsQueueEmpty()
{
	return (fRecordStream == nil);
}


plNetMessage* plNetClientStreamRecorder::GetNextMessage()
{
	plNetMessage* msg = nil;
	while (!fBetweenAges && (msg = IGetNextMessage()))
	{
		if (IIsValidMsg(msg))
			return msg;
		else
			hsRefCnt_SafeUnRef(msg);
	}

	return nil;
}

plNetMessage* plNetClientStreamRecorder::IGetNextMessage()
{
	plNetMessage* msg = nil;

	if (!IsQueueEmpty() && GetNextMessageTimeDelta() <= 0 )
	{
		msg = plNetMessage::ConvertNoRef(GetResMgr()->ReadCreatableVersion(fRecordStream));
		// msg->SetPeeked(true);

		// Fix the flags on game messages, so we won't get an assert later
		plNetMsgGameMessage* gameMsg = plNetMsgGameMessage::ConvertNoRef(msg);
		if (gameMsg)
		{
			plMessage* plMsg = gameMsg->GetContainedMsg(GetResMgr());
			plNetClientApp::GetInstance()->UnInheritNetMsgFlags(plMsg);

			// write message (and label) to ram stream
			hsRAMStream stream;
			GetResMgr()->WriteCreatable(&stream, plMsg);

			// put stream in net msg wrapper
			gameMsg->StreamInfo()->CopyStream(&stream);
			// gameMsg->StreamInfo()->SetStreamType(plMsg->ClassIndex());		// type of game msg
		}

		double nextPlaybackTime = fRecordStream->ReadSwapDouble();
		if (nextPlaybackTime < fNextPlaybackTime)
			fBetweenAges = true;

		fNextPlaybackTime = nextPlaybackTime;

		// If this was the last message, stop playing back
		if (fRecordStream->AtEnd())
		{
			fRecordStream->Close();
			delete fRecordStream;
			fRecordStream = nil;
		}
	}
	
	return msg;
}

bool plNetClientStreamRecorder::IIsValidMsg(plNetMessage* msg)
{
	if (plNetMsgGameMessage* gameMsg = plNetMsgGameMessage::ConvertNoRef(msg))
	{
		Int16 type = gameMsg->StreamInfo()->GetStreamType();

		//
		// These messages will be regenerated if they are for the local avatar,
		// so don't dispatch them in that case.
		//

		if (type == CLASS_INDEX_SCOPED(plLinkEffectsTriggerMsg))
		{
			plLinkEffectsTriggerMsg* linkMsg = plLinkEffectsTriggerMsg::ConvertNoRef(gameMsg->GetContainedMsg(GetResMgr()));
			if (plNetClientApp::GetInstance())
			{
				bool isLocal = (linkMsg->GetLinkKey() == plNetClientApp::GetInstance()->GetLocalPlayerKey());
				hsRefCnt_SafeUnRef(linkMsg);

				if (isLocal)
				{
					ILogMsg(msg, "IGNORING ");
					return false;
				}
			}
		}
		else if (type == CLASS_INDEX_SCOPED(plLoadAvatarMsg))
		{
			plLoadAvatarMsg* loadAvMsg = plLoadAvatarMsg::ConvertNoRef(gameMsg->GetContainedMsg(GetResMgr()));
			if (plNetClientApp::GetInstance())
			{
				bool isLocal = (loadAvMsg->GetCloneKey() == plNetClientApp::GetInstance()->GetLocalPlayerKey());
				hsRefCnt_SafeUnRef(loadAvMsg);

				if (isLocal)
				{
					ILogMsg(msg, "IGNORING ");
					return false;
				}
			}
		}
	}

	ILogMsg(msg);
	return true;
}

void plNetClientStreamRecorder::ILogMsg(plNetMessage* msg, const char* preText)
{
	if (msg->ClassIndex() == CLASS_INDEX_SCOPED(plNetMsgGameMessage))
	{
		plNetMsgGameMessage* gameMsg = plNetMsgGameMessage::ConvertNoRef(msg);
		fLog->AddLineF("%s%s(%s)", preText, msg->ClassName(), plFactory::GetNameOfClass(gameMsg->StreamInfo()->GetStreamType()));

		if (gameMsg->StreamInfo()->GetStreamType() == CLASS_INDEX_SCOPED(plNotifyMsg))
		{
			plNotifyMsg* notifyMsg = plNotifyMsg::ConvertNoRef(gameMsg->GetContainedMsg(GetResMgr()));
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


bool plNetClientStressStreamRecorder::IsRecordableMsg(plNetMessage* msg) const
{
	UInt16 idx = msg->ClassIndex();

	return (
		plNetClientStreamRecorder::IsRecordableMsg(msg)
		|| idx == CLASS_INDEX_SCOPED(plNetMsgTestAndSet)
		|| idx == CLASS_INDEX_SCOPED(plNetMsgGameMessageDirected)
		|| idx == CLASS_INDEX_SCOPED(plNetMsgVoice)
		);
}