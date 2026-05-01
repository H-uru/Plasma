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

#include "plNetClientMsgScreener.h"

#include <string_theory/string>
#include <unordered_set>

#include "plCreatableIndex.h"
#include "plNetLinkingMgr.h"

#include "pnFactory/plFactory.h"
#include "pnMessage/plMessage.h"
#include "pnMessage/plMessageWithCallbacks.h"
#include "pnNetCommon/plNetApp.h"

#include "plAvatar/plAvBrain.h"
#include "plAvatar/plAvBrainCoop.h"
#include "plAvatar/plAvTaskBrain.h"
#include "plAvatar/plAvatarMgr.h"
#include "plAvatar/plArmatureMod.h"
#include "plAvatar/plCoopCoordinator.h"
#include "plMessage/plAvCoopMsg.h"
#include "plMessage/plAvatarMsg.h"
#include "plMessage/plInputEventMsg.h"
#include "plMessage/plLoadAvatarMsg.h"
#include "plMessage/plLoadCloneMsg.h"
#include "plStatusLog/plStatusLog.h"

#include "pfMessage/pfKIMsg.h"

using namespace ST::literals;

///////////////////////////////////////////////////////////////
// CLIENT Version
///////////////////////////////////////////////////////////////

plNetClientMsgScreener::plNetClientMsgScreener()
{}

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
ST::string plNetClientMsgScreener::IGetAgeName() const
{
    plNetLinkingMgr *lm = plNetLinkingMgr::GetInstance();
    return lm && lm->GetAgeLink()->GetAgeInfo() ? lm->GetAgeLink()->GetAgeInfo()->GetAgeFilename() : ST_LITERAL("?");
}

//
// Check if key is local avatar
//
bool plNetClientMsgScreener::IIsLocalAvatarKey(const plKey& key, const plNetGameMember* gm) const
{
    return (!key || key==plNetClientApp::GetInstance()->GetLocalPlayerKey());
}

bool plNetClientMsgScreener::IIsLocalArmatureModKey(const plKey& key, const plNetGameMember* gm) const 
{
    plKey playerKey = plNetClientApp::GetInstance()->GetLocalPlayerKey();
    plArmatureMod* aMod = playerKey ? plAvatarMgr::GetInstance()->FindAvatar(playerKey) : nullptr;
    return (!key || key == (aMod ? aMod->GetKey() : nullptr));
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
        WarningMsg("Rejected: (Outgoing) {} [Illegal Message]", msg->ClassName());
        return false;
    }

    if (!IValidateMessage(msg))
    {
        WarningMsg("Rejected: (Outgoing) {} [Validation Failed]", msg->ClassName());
        return false;
    }
    return true;
}

// The longest safe command name is 34 characters,
// so this maximum leaves at least 93 characters for the arguments,
// which is hopefully enough.
static constexpr size_t kMaxSafeCommandLength = 128;

/**
 * Console command names that are always safe to execute.
 * Currently includes all commands used by the TOC-MOUL shard and by Mir-o-Bot,
 * mostly for changing visual settings like fog color and distance.
 * We do not want to just allow all commands under Graphics.Renderer,
 * because some of them write files to disk (such as Graphics.Renderer.GrabCubeMap).
 */
static const std::unordered_set<ST::string, ST::hash_i, ST::equal_i> kSafeConsoleCommandNames = {
    "Avatar.Spawn.DontPanic"_st,
    "Graphics.Renderer.Fog.SetDefColor"_st,
    "Graphics.Renderer.Fog.SetDefExp"_st,
    "Graphics.Renderer.Fog.SetDefExp2"_st,
    "Graphics.Renderer.Fog.SetDefLinear"_st,
    "Graphics.Renderer.SetClearColor"_st,
    "Graphics.Renderer.SetYon"_st,
    "Graphics.SetDebugFlag"_st,
    "Nav.PageInNode"_st,
    "Nav.PageOutNode"_st,
};

/**
 * Checks whether the given command is definitely safe to execute
 * and thus accepted when received over the network.
 * Currently, a command is only considered safe
 * if its command name is listed in \ref kSafeConsoleCommandNames
 * and its arguments only consist of ASCII letters, digits, "-", ".", and "_".
 *
 * This check is very restrictive - it rejects many commands that are not actually dangerous
 * and disallows alternative spellings of otherwise allowed commands
 * (such as "_" instead of "." as the command group separator).
 * This is intentional, to avoid overly complex parsing of untrusted inputs.
 *
 * @param command the complete command (name and arguments) to check
 * @return whether the command is definitely safe
 */
bool plNetClientMsgScreener::IIsConsoleCommandSafe(const ST::string& command)
{
    // Overly long commands are considered unsafe.
    if (command.size() > kMaxSafeCommandLength) {
        return false;
    }

    // Split name and arguments.
    auto parts = command.split(' ', 1);
    if (parts.empty()) {
        hsAssert(false, "ST::string::split returned an empty vector??");
        return false;
    }

    // Is the command name safe?
    const ST::string& name = parts[0];
    if (kSafeConsoleCommandNames.find(name) == kSafeConsoleCommandNames.end()) {
        return false;
    }

    // If there are arguments, are they all "simple" characters?
    if (parts.size() > 1) {
        const ST::string& args = parts[1];
        for (char c : args) {
            if (c < 0 || !(isalnum(c) || c == ' ' || c == '-' || c == '.' || c == '_')) {
                return false;
            }
        }
    }

    // Everything seems to be in order.
    return true;
}

bool plNetClientMsgScreener::IScreenIncomingBrain(const plArmatureBrain* brain)
{
    if (!brain) {
        return true;
    }

    const plAvBrainGeneric* brainGeneric;
    switch (brain->ClassIndex()) {
        case CLASS_INDEX_SCOPED(plAvBrainCoop):
        case CLASS_INDEX_SCOPED(plAvBrainGeneric):
            // These brains can contain nested messages, which need to be recursively screened.
            brainGeneric = plAvBrainGeneric::ConvertNoRef(brain);
            ASSERT(brainGeneric);
            if (!IScreenIncoming(brainGeneric->GetStartMessage()) || !IScreenIncoming(brainGeneric->GetEndMessage())) {
                return false;
            }
            return true;

        case CLASS_INDEX_SCOPED(plAvBrainClimb):
        case CLASS_INDEX_SCOPED(plAvBrainCritter):
        case CLASS_INDEX_SCOPED(plAvBrainDrive):
        case CLASS_INDEX_SCOPED(plAvBrainHuman):
        case CLASS_INDEX_SCOPED(plAvBrainSwim):
            // The data for these brains can't contain anything scary.
            return true;

        default:
            // Don't know this type of brain!
            return false;
    }
}

bool plNetClientMsgScreener::IScreenIncomingTask(const plAvTask* task)
{
    if (!task) {
        return true;
    }

    const plAvTaskBrain* taskBrain;
    switch (task->ClassIndex()) {
        case CLASS_INDEX_SCOPED(plAvTaskBrain):
            // This task contains a brain, which needs to be recursively screened.
            taskBrain = plAvTaskBrain::ConvertNoRef(task);
            ASSERT(taskBrain);
            if (!IScreenIncomingBrain(taskBrain->GetBrain())) {
                return false;
            }
            return true;

        case CLASS_INDEX_SCOPED(plAvAnimTask):
        case CLASS_INDEX_SCOPED(plAvOneShotTask):
        case CLASS_INDEX_SCOPED(plAvOneShotLinkTask):
        case CLASS_INDEX_SCOPED(plAvSeekTask):
        case CLASS_INDEX_SCOPED(plAvTaskSeek):
            // The data for these tasks can't contain anything scary.
            return true;

        default:
            // Don't know this type of task!
            return false;
    }
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
        WarningMsg("Rejected: (Incoming) {}", msg->ClassName());

    return result;
}

bool plNetClientMsgScreener::IScreenIncoming(const plMessage* msg)
{
    if (!msg) {
        // The top-level message cannot be nullptr (this is checked by AllowIncomingMessage).
        // plMessage* fields within other messages are allowed to be nullptr though.
        return true;
    }

    // Blacklist some obvious hacks here...
    switch (msg->ClassIndex())
    {
    case CLASS_INDEX_SCOPED(plAttachMsg):
        return true;
    case CLASS_INDEX_SCOPED(plAudioSysMsg):
        // This message has a flawed read/write
        return false;
    case CLASS_INDEX_SCOPED(plConsoleMsg):
        // Python remote code execution vulnerability
        return false;
    case CLASS_INDEX_SCOPED(plControlEventMsg):
        // Other clients have no business sending us inputs,
        // but plControlEventMsg is also used by PtConsoleNet
        // to send console commands to other players.
        // This is another remote code execution risk, like plConsoleMsg.
        // Unfortunately, PtConsoleNet has some safe, legitimate uses (see kSafeConsoleCommandNames),
        // so allow plControlEventMsgs containing such known-safe console commands.
        {
            const plControlEventMsg* controlEventMsg = plControlEventMsg::ConvertNoRef(msg);
            return controlEventMsg->GetControlCode() == B_CONTROL_CONSOLE_COMMAND
                && controlEventMsg->ControlActivated()
                && IIsConsoleCommandSafe(controlEventMsg->GetCmdString());
        }
    case CLASS_INDEX_SCOPED(pfKIMsg):
        {
            // Only accept Chat Messages!
            const pfKIMsg* ki = pfKIMsg::ConvertNoRef(msg);
            if (ki->GetCommand() != pfKIMsg::kHACKChatMsg)
                return false;
            return true;
        }
    default:
        // Toss non-attach plRefMsgs
        if (plFactory::DerivesFrom(CLASS_INDEX_SCOPED(plRefMsg), msg->ClassIndex()))
            return false;

        // Other clients have no business sending us inputs.
        // The only exception is plControlEventMsg,
        // which is handled separately above.
        if (plFactory::DerivesFrom(CLASS_INDEX_SCOPED(plInputEventMsg), msg->ClassIndex())) {
            return false;
        }

        // Recursively screen messages contained within other messages (directly or indirectly).

        if (auto msgWithCallbacks = plMessageWithCallbacks::ConvertNoRef(msg)) {
            size_t numCallbacks = msgWithCallbacks->GetNumCallbacks();
            for (size_t i = 0; i < numCallbacks; i++) {
                if (!IScreenIncoming(msgWithCallbacks->GetCallback(i))) {
                    return false;
                }
            }
        }

        // Some avatar brains can contain messages and some avatar tasks contain brains,
        // so we have to recursively screen all of those as well...

        if (auto loadCloneMsg = plLoadCloneMsg::ConvertNoRef(msg)) {
            if (!IScreenIncoming(loadCloneMsg->GetTriggerMsg())) {
                return false;
            }

            if (auto loadAvatarMsg = plLoadAvatarMsg::ConvertNoRef(loadCloneMsg)) {
                if (!IScreenIncomingTask(loadAvatarMsg->GetInitialTask())) {
                    return false;
                }
            }
        }

        if (auto avCoopMsg = plAvCoopMsg::ConvertNoRef(msg)) {
            if (avCoopMsg->fCoordinator && (
                !IScreenIncomingBrain(avCoopMsg->fCoordinator->fHostBrain)
                || !IScreenIncomingBrain(avCoopMsg->fCoordinator->fGuestBrain)
                || !IScreenIncoming(avCoopMsg->fCoordinator->fGuestAcceptMsg)
            )) {
                return false;
            }
        }

        if (auto avTaskMsg = plAvTaskMsg::ConvertNoRef(msg)) {
            if (!IScreenIncomingTask(avTaskMsg->GetTask())) {
                return false;
            }
            
            if (auto avPushBrainMsg = plAvPushBrainMsg::ConvertNoRef(avTaskMsg)) {
                if (!IScreenIncomingBrain(avPushBrainMsg->fBrain)) {
                    return false;
                }
            }
        }

        // Default allow everything else, otherwise we
        // might break something that we really shouldn't...
        return true;
    }
}
