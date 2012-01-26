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
#include <algorithm>
#include "plgDispatch.h"
#include "plNetClientMgr.h"
#include "plNetVoiceList.h"
#include "plNetTransport/plNetTransportMember.h"
#include "pnMessage/plSoundMsg.h"
#include "pnKeyedObject/plKey.h"
#include "plStatusLog/plStatusLog.h"

// statics
float plNetListenList::kUpdateInterval=0.5f;
int plNetListenList::kMaxListenListSize=-1;     // -1 is unlimited
float plNetListenList::kMaxListenDistSq=75.0f*75.0f;

int plNetVoiceList::FindMember(plNetTransportMember* e) 
{
    VoiceListType::iterator result = std::find(fMembers.begin(), fMembers.end(), e);
    return result!=fMembers.end() ? result-fMembers.begin() : -1;
}


/*****************************************************************************
*
*   plNetTalkList
*
***/

void plNetTalkList::UpdateTransportGroup(plNetClientMgr* nc)
{
    if (fFlags & kDirty)
    {
        nc->fTransport.ClearChannelGrp(plNetClientMgr::kNetChanVoice);
        if (nc->IsPeerToPeer())
        {
            int i;
            for(i=0;i<GetNumMembers();i++)
            {
                if (GetMember(i)->IsPeerToPeer())
                    nc->fTransport.SubscribeToChannelGrp(GetMember(i), plNetClientMgr::kNetChanVoice);
            }
        }
        fFlags &= ~kDirty;
    }
}

void plNetTalkList::AddMember(plNetTransportMember* e) 
{ 
    if (FindMember(e)==-1)
    {
        plStatusLog::AddLineS("voice.log", "Adding %s to talk list", e->AsStdString().c_str());
        fMembers.push_back(e);
    }
    fFlags |= kDirty;   
}
    
void plNetTalkList::RemoveMember(plNetTransportMember* e) 
{ 
    int idx=FindMember(e);
    if (idx!=-1)
    {
        plStatusLog::AddLineS("voice.log", "Removing %s from talklist", e->AsStdString().c_str());
        fMembers.erase(fMembers.begin()+idx);
    }
    fFlags |= kDirty; 
}
    
void plNetTalkList::Clear() 
{ 
    plNetVoiceList::Clear(); 
    fFlags |= kDirty; 
}


/*****************************************************************************
*
*   plNetListenList
*
***/

void plNetListenList::AddMember(plNetTransportMember* e)
{
    if (FindMember(e)==-1)
    {
        plStatusLog::AddLineS("voice.log", "Adding %s to listen list ", e->AsStdString().c_str());
        fMembers.push_back(e);
    
#if 0   
        // call the new member's win audible and set talk icon parameters

        plSoundMsg* pMsg = new plSoundMsg;
        plArmatureMod* pMod = plArmatureMod::ConvertNoRef(e->GetAvatarKey()->GetObjectPtr());
        if (pMod)
            pMsg->AddReceiver(pMod->GetTarget(0)->GetKey());
        pMsg->SetCmd(plSoundMsg::kSetTalkIcon);
        pMsg->fIndex = GetNumMembers();
        pMsg->fNameStr = (uint32_t)e->GetName();
        plgDispatch::MsgSend(pMsg);
#endif  
    }
}

void plNetListenList::RemoveMember(plNetTransportMember* e)
{
    int idx=FindMember(e);
    if (idx!=-1)
    {
        fMembers.erase(fMembers.begin()+idx);
        plStatusLog::AddLineS("voice.log", "Removing %s from listen list", e->AsStdString().c_str());
#if 0
        // call the new member's win audible and set talk icon parameters

        plSoundMsg* pMsg = new plSoundMsg;
        plArmatureMod* pMod = plArmatureMod::ConvertNoRef(e->GetAvatarKey()->GetObjectPtr());
        if (pMod)
            pMsg->AddReceiver(pMod->GetTarget(0)->GetKey());
        pMsg->SetCmd(plSoundMsg::kClearTalkIcon);
        plgDispatch::MsgSend(pMsg);
#endif  
    }
}
