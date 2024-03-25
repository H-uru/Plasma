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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#include "plNetClientRecorder.h"

#include "plCreatableIndex.h"
#include "plgDispatch.h"
#include "hsResMgr.h"
#include "hsStream.h"

#include "pnFactory/plFactory.h"
#include "pnMessage/plNotifyMsg.h"
#include "pnNetCommon/plNetApp.h"

#include "plMessage/plAgeLoadedMsg.h"
#include "plMessage/plLinkToAgeMsg.h"
#include "plMessage/plLoadAvatarMsg.h"
#include "plNetMessage/plNetMessage.h"
#include "plSDL/plSDL.h"
#include "plStatusLog/plStatusLog.h"

plNetClientStreamRecorder::plNetClientStreamRecorder(TimeWrapper* timeWrapper) :
    plNetClientLoggingRecorder(timeWrapper),
    fRecordStream(),
    fResMgr()
{
    if (fLog)
        delete fLog;
    fLog = plStatusLogMgr::GetInstance().CreateStatusLog(30, "StreamRecorder.log", plStatusLog::kAlignToTop);
}


plNetClientStreamRecorder::~plNetClientStreamRecorder()
{}

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
        char path[256];
        IMakeFilename(recName, path);

        auto newStream = std::make_unique<hsUNIXStream>();
        if (!newStream->Open(path, "wb")) {
            return false;
        }
        fRecordStream = std::move(newStream);

        hsBitVector contentFlags;
        contentFlags.SetBit(kNetClientRecSDLDesc);
        contentFlags.Write(fRecordStream.get());

        // kNetClientRecSDLDesc
        plSDLMgr::GetInstance()->Write(fRecordStream.get());

        return true;
    }

    return false;
}

bool plNetClientStreamRecorder::BeginPlayback(const char* recName)
{
    if (!fRecordStream)
    {
        char path[256];
        IMakeFilename(recName, path);

        auto newStream = std::make_unique<hsUNIXStream>();
        if (newStream->Open(path, "rb")) {
            fRecordStream = std::move(newStream);
            hsBitVector contentFlags;
            contentFlags.Read(fRecordStream.get());

            if (contentFlags.IsBitSet(kNetClientRecSDLDesc))
                plSDLMgr::GetInstance()->Read(fRecordStream.get());

            fPlaybackTimeOffset = GetTime();
            fNextPlaybackTime = fRecordStream->ReadLEDouble();
            fBetweenAges = false;
        } else {
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
        fRecordStream->WriteLEDouble(secs - fPlaybackTimeOffset);
        GetResMgr()->WriteCreatableVersion(fRecordStream.get(), msg);

        ILogMsg(msg);
    }
}

void plNetClientStreamRecorder::RecordAgeLoadedMsg(plAgeLoadedMsg* ageLoadedMsg)
{
    fLog->AddLineF("Age {}", ageLoadedMsg->fLoaded ? "Loaded" : "Unloaded");

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
    return (fRecordStream == nullptr);
}


plNetMessage* plNetClientStreamRecorder::GetNextMessage()
{
    plNetMessage* msg = nullptr;
    while (!fBetweenAges && (msg = IGetNextMessage()))
    {
        if (IIsValidMsg(msg))
            return msg;
        else
            hsRefCnt_SafeUnRef(msg);
    }

    return nullptr;
}

plNetMessage* plNetClientStreamRecorder::IGetNextMessage()
{
    plNetMessage* msg = nullptr;

    if (!IsQueueEmpty() && GetNextMessageTimeDelta() <= 0 )
    {
        msg = plNetMessage::ConvertNoRef(GetResMgr()->ReadCreatableVersion(fRecordStream.get()));
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
            // gameMsg->StreamInfo()->SetStreamType(plMsg->ClassIndex());       // type of game msg
        }

        double nextPlaybackTime = fRecordStream->ReadLEDouble();
        if (nextPlaybackTime < fNextPlaybackTime)
            fBetweenAges = true;

        fNextPlaybackTime = nextPlaybackTime;

        // If this was the last message, stop playing back
        if (fRecordStream->AtEnd())
        {
            fRecordStream.reset();
        }
    }
    
    return msg;
}

bool plNetClientStreamRecorder::IIsValidMsg(plNetMessage* msg)
{
    if (plNetMsgGameMessage* gameMsg = plNetMsgGameMessage::ConvertNoRef(msg))
    {
        int16_t type = gameMsg->StreamInfo()->GetStreamType();

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
        fLog->AddLineF("{}{}({})", preText, msg->ClassName(), plFactory::GetNameOfClass(gameMsg->StreamInfo()->GetStreamType()));

        if (gameMsg->StreamInfo()->GetStreamType() == CLASS_INDEX_SCOPED(plNotifyMsg))
        {
            plNotifyMsg* notifyMsg = plNotifyMsg::ConvertNoRef(gameMsg->GetContainedMsg(GetResMgr()));
            size_t numEvents = notifyMsg->GetEventCount();
            for (size_t i = 0; i < numEvents; i++)
            {
                const char* eventName = "";

                proEventData* event = notifyMsg->GetEventRecord(i);
                switch (event->fEventType)
                {
                case proEventData::kCollision:      eventName = "Collision";        break;
                case proEventData::kPicked:         eventName = "Picked";           break;
                case proEventData::kControlKey:     eventName = "ControlKey";       break;
                case proEventData::kVariable:       eventName = "Variable";         break;
                case proEventData::kFacing:         eventName = "Facing";           break;
                case proEventData::kContained:      eventName = "Contained";        break;
                case proEventData::kActivate:       eventName = "Activate";         break;
                case proEventData::kCallback:       eventName = "Callback";         break;
                case proEventData::kResponderState: eventName = "ResponderState";   break;
                case proEventData::kMultiStage:     eventName = "MultiStage";       break;
                case proEventData::kSpawned:        eventName = "Spawned";          break;
                case proEventData::kClickDrag:      eventName = "ClickDrag";        break;
                }

                fLog->AddLineF("\t{}", eventName);
            }

            hsRefCnt_SafeUnRef(notifyMsg);
        }
    }
    else if (plNetMsgSDLState* sdlMsg = plNetMsgSDLState::ConvertNoRef(msg))
    {
        hsReadOnlyStream stream(sdlMsg->StreamInfo()->GetStreamLen(), sdlMsg->StreamInfo()->GetStreamBuf());
        ST::string descName;
        int ver;
        if (plStateDataRecord::ReadStreamHeader(&stream, &descName, &ver))
        {
            fLog->AddLineF("{}{}({})", preText, msg->ClassName(), descName);

            int i;

            plStateDataRecord sdRec(descName, ver);
            sdRec.Read(&stream, 0);
            plStateDataRecord::SimpleVarsList vars;
            sdRec.GetDirtyVars(&vars);
            for (i = 0; i < vars.size(); i++)
            {
                fLog->AddLineF("\t{}", vars[i]->GetVarDescriptor()->GetName());
            }

            plStateDataRecord::SDVarsList sdVars;
            sdRec.GetDirtySDVars(&sdVars);
            for (i = 0; i < sdVars.size(); i++)
            {
                fLog->AddLineF("\t{}", sdVars[i]->GetSDVarDescriptor()->GetName());
            }
        }
    }
    else
        fLog->AddLineF("{}{}", preText, msg->ClassName());
}


bool plNetClientStressStreamRecorder::IsRecordableMsg(plNetMessage* msg) const
{
    uint16_t idx = msg->ClassIndex();

    return (
        plNetClientStreamRecorder::IsRecordableMsg(msg)
        || idx == CLASS_INDEX_SCOPED(plNetMsgTestAndSet)
        || idx == CLASS_INDEX_SCOPED(plNetMsgGameMessageDirected)
        || idx == CLASS_INDEX_SCOPED(plNetMsgVoice)
        );
}