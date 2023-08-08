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

#include "plNetMsgScreener.h"
#include "plCreatableIndex.h"

#include "pnFactory/plFactory.h"
#include "pnNetCommon/plNetApp.h"
#include "pnKeyedObject/plKey.h"

#include "plMessage/plAnimCmdMsg.h"
#include "plMessage/plInputIfaceMgrMsg.h"
#include "plMessage/plLinkToAgeMsg.h"

#include "pfMessage/pfKIMsg.h"

//
// say why the msg got rejected
//
void plNetMsgScreener::IRejectLogMsg(int16_t classIndex, const char* desc, const plNetGameMember* gm) const
{
    DebugMsg("Message {} was rejected, reason:{}, age:{}, client:{}",
        plFactory::GetNameOfClass(classIndex), desc, IGetAgeName(), IGetSenderName(gm));
}

//
// say why the msg got rejected
//
void plNetMsgScreener::IRejectLogMsg(const plMessage* msg, const char* desc, const plNetGameMember* gm) const
{
    ST::string senderName = msg->GetSender() ? msg->GetSender()->GetUoid().GetObjectName() : ST_LITERAL("?");
    ST::string rcvrName = msg->GetNumReceivers() && msg->GetReceiver(0) ? msg->GetReceiver(0)->GetUoid().GetObjectName() : ST_LITERAL("?");

    DebugMsg("Message {} was rejected, reason:{}, age:{}, client:{}, msgSndr:{}, msgRcvr:{}",
        msg->ClassName(), desc, IGetAgeName(), IGetSenderName(gm),
        senderName, rcvrName);
}

//
// Try to accept/reject quickly
// the netMsg arg has been peeked except for the stream
//
plNetMsgScreener::Answer plNetMsgScreener::IAllowMessageType(int16_t classIndex, const plNetGameMember* gm) const
{
    // Check based on exact type
    switch(classIndex)
    {
        // these are wrapped in their own net msg, so the client will see them this way, but not the server
        // that's why they check IAmClient() - this is a special case
    case CLASS_INDEX_SCOPED(plLoadAvatarMsg):
    case CLASS_INDEX_SCOPED(plLoadCloneMsg):
        {
            Answer ans=IAmClient() ? kYes : kNo;
            if (ans==kNo)
            {
                IRejectLogMsg(classIndex, "Only seen in native form on client", gm);
            }
            return ans;
        }       
        
        // definitely yes
    case CLASS_INDEX_SCOPED(pfMarkerMsg):
    case CLASS_INDEX_SCOPED(plBulletMsg):
    case CLASS_INDEX_SCOPED(plNotifyMsg):
    case CLASS_INDEX_SCOPED(plSetNetGroupIDMsg):
    case CLASS_INDEX_SCOPED(plAvCoopMsg):
    case CLASS_INDEX_SCOPED(plClothingMsg):
    case CLASS_INDEX_SCOPED(plEnableMsg):
    case CLASS_INDEX_SCOPED(plLinkToAgeMsg):
    case CLASS_INDEX_SCOPED(plSubWorldMsg):
    case CLASS_INDEX_SCOPED(plDynamicTextMsg):
        return kYes;

    // conditionally yes, requires further validation of msg contents
    case CLASS_INDEX_SCOPED(plAnimCmdMsg):
    case CLASS_INDEX_SCOPED(pfKIMsg):
    case CLASS_INDEX_SCOPED(plAvTaskMsg):
    case CLASS_INDEX_SCOPED(plLinkEffectsTriggerMsg):
    case CLASS_INDEX_SCOPED(plInputIfaceMgrMsg):
    case CLASS_INDEX_SCOPED(plParticleKillMsg):
    case CLASS_INDEX_SCOPED(plParticleTransferMsg):
    case CLASS_INDEX_SCOPED(plAvatarInputStateMsg):
    case CLASS_INDEX_SCOPED(plAvBrainGenericMsg):
    case CLASS_INDEX_SCOPED(plMultistageModMsg):
        return kMaybe;
        
        // definitely no
    default:
        IRejectLogMsg(classIndex, "Illegal msg class", gm);
        return kNo;
    }
}

//
// Message may be allowed if contents or conditions are met
//
bool plNetMsgScreener::IValidateMessage(const plMessage* msg, const plNetGameMember* gm) const
{
    if (!msg)
        return true;

    switch(msg->ClassIndex())
    {
        // Only chat KI msgs are allowed.
        // Admin/system-wide chat msgs are only allowed by CCRs
    case CLASS_INDEX_SCOPED(pfKIMsg):
        {
            const pfKIMsg* km = pfKIMsg::ConvertNoRef(msg);
            if (km->GetCommand() != pfKIMsg::kHACKChatMsg)
            {
                IRejectLogMsg(msg, "Non-chat KI msg", gm);
                return false;
            }

            ILogChatMessage(msg, gm);
            if (km->GetFlags() & pfKIMsg::kAdminMsg)
            {
                if (!IIsSenderCCR(gm))
                {
                    IRejectLogMsg(msg, "Must be a CCR to send an Admin KI msg", gm);
                    return false;
                }
            }
            return true;
        }
        break;
        
        // Allowed for local avatar
    case CLASS_INDEX_SCOPED(plAvTaskMsg):
    case CLASS_INDEX_SCOPED(plAvatarInputStateMsg):
    case CLASS_INDEX_SCOPED(plAvBrainGenericMsg):
    case CLASS_INDEX_SCOPED(plMultistageModMsg):
        {
            bool ret=IIsLocalArmatureModKey(msg->GetReceiver(0), gm);
            if (!ret)
            {
                IRejectLogMsg(msg, "msg must refer to local avatar", gm);
            }
            return ret;
        }

        // Allowed for local avatar
    case CLASS_INDEX_SCOPED(plLinkEffectsTriggerMsg):
        {
            const plLinkEffectsTriggerMsg* linkMsg = plLinkEffectsTriggerMsg::ConvertNoRef(msg);
            bool ret=IIsLocalAvatarKey(linkMsg->GetLinkKey(), gm);
            if (!ret)
            {
                IRejectLogMsg(msg, "msg must refer to local avatar", gm);
            }
            return ret;
        }

        // Allowed for local avatar
    case CLASS_INDEX_SCOPED(plInputIfaceMgrMsg):
        {
            const plInputIfaceMgrMsg* iMsg = plInputIfaceMgrMsg::ConvertNoRef(msg);
            bool ret=IIsLocalAvatarKey(iMsg->GetAvKey(), gm);
            if (!ret)
            {
                IRejectLogMsg(msg, "msg must refer to local avatar", gm);
            }
            return ret;
        }
        break;
    
    case CLASS_INDEX_SCOPED(plParticleKillMsg):
    case CLASS_INDEX_SCOPED(plParticleTransferMsg):
        {
            bool ret = IIsLocalAvatarKey(msg->GetReceiver(0), gm);
            if (!ret)
            {
                IRejectLogMsg(msg, "msg must refer to local avatar", gm);
            }
            return ret;
        }   
        break;

    case CLASS_INDEX_SCOPED(plAnimCmdMsg):
        {
            const plAnimCmdMsg *animMsg = plAnimCmdMsg::ConvertNoRef(msg);
            bool ret = (animMsg->GetNumCallbacks() == 0);
            if (!ret)
            {
                IRejectLogMsg(msg, "msg has callbacks", gm);
            }
            return ret;
        }
        break;
    default:
        return false;
    }

}

