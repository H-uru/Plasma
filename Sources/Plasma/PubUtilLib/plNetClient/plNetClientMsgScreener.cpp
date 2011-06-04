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
#include "plCreatableIndex.h"
#include "plNetClientMsgScreener.h"
#include "plNetLinkingMgr.h"

#include "pfMessage/pfKIMsg.h"
#include "pnNetCommon/plNetApp.h"
#include "pnMessage/plMessage.h"

#include "plStatusLog/plStatusLog.h"
#include "plAvatar/plAvatarMgr.h"
#include "plAvatar/plArmatureMod.h"

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
bool plNetClientMsgScreener::AllowOutgoingMessage(const plMessage* msg) const
{
    if (!msg)
        return false;

    Answer ans=IAllowMessageType(msg->ClassIndex());
    if (ans==kYes)
        return true;
    if (ans==kNo)
    {
        WarningMsg("Rejected: (Outgoing) %s [Illegal Message]", msg->ClassName());
        return false;
    }

    if (!IValidateMessage(msg))
    {
        WarningMsg("Rejected: (Outgoing) %s [Validation Failed]", msg->ClassName());
        return false;
    }
    return true;
}

//
// Return false for obvious hacked network messages
// This is all because we cannot trust Cyan's servers
//
bool plNetClientMsgScreener::AllowIncomingMessage(const plMessage* msg) const
{
    if (!msg)
        return false;

    bool result = IScreenIncoming(msg);
    if (!result)
        WarningMsg("Rejected: (Incoming) %s", msg->ClassName());

    return result;
}

bool plNetClientMsgScreener::IScreenIncoming(const plMessage* msg) const
{
    // Why would you EVER send a RefMsg accross the network???
    if (plFactory::DerivesFrom(CLASS_INDEX_SCOPED(plRefMsg), msg->ClassIndex()))
        return false;

    // Blacklist some obvious hacks here...
    switch (msg->ClassIndex())
    {
    case CLASS_INDEX_SCOPED(plAudioSysMsg):
        // This message has a flawed read/write
        return false;
    case CLASS_INDEX_SCOPED(plConsoleMsg):
        // Python remote code execution vunerability
        return false;
    case CLASS_INDEX_SCOPED(pfKIMsg):
        {
            // Only accept Chat Messages!
            const pfKIMsg* ki = pfKIMsg::ConvertNoRef(msg);
            if (ki->GetCommand() != pfKIMsg::kHACKChatMsg)
                return false;
            return true;
        }
    default:
        // Default allow everything else, otherweise we
        // might break something that we really shouldn't...
        return true;
    }
}
