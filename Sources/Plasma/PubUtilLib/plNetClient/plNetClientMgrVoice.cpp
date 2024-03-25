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

#include "plNetClientMgr.h"

#include "hsMatrix44.h"
#include "hsGeometry3.h"

#include <algorithm>

#include "pnKeyedObject/plKey.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "pnSceneObject/plSceneObject.h"

#include "plMessage/plMemberUpdateMsg.h"
#include "plMessage/plNetVoiceListMsg.h"
#include "plNetMessage/plNetMessage.h"
#include "plNetTransport/plNetTransportMember.h"
#include "plStatusLog/plStatusLog.h"
#include "plVault/plVault.h"


#define SAME_TALK_AND_LISTEN

struct DistSqInfo
{
    plNetTransportMember* fMbr;
    float fDistSq;
    DistSqInfo(plNetTransportMember* m, float d) : fMbr(m),fDistSq(d) {}
};

bool lessComp(DistSqInfo a, DistSqInfo b)
{
    return (a.fDistSq<b.fDistSq);
}

//
// see if new listen list differs from current one
// apply new list if there's a difference
// send listenList update msgs to old and new members in the list
// return true if the new list is different
//
bool plNetClientMgr::IApplyNewListenList(std::vector<DistSqInfo>& newListenList, bool forceSynch)
{
    bool changed=false;
    int i;

    // see if new listen list differs from current one
    if (forceSynch || newListenList.size() != GetListenList()->GetNumMembers())
        changed=true;
    else
    {
        for(i=0;i<newListenList.size(); i++)
        {           
            if (GetListenList()->FindMember(newListenList[i].fMbr)==-1)
            {
                changed=true;
                break;              
            }
        }
    }

    // set as new listen list
    if (changed)
    {   
        DebugMsg("ListenList changed, forceSynch={}\n", forceSynch);

        plNetMsgListenListUpdate llu;
        llu.SetPlayerID(GetPlayerID());
        
        //
        // for each client in the old list, if not in the new list, send a ListenList remove msg
        //
        llu.Receivers()->Clear();
        llu.SetAdding(false);
        for(i=0;i<GetListenList()->GetNumMembers(); i++)
        {
            bool found=false;
            if (!forceSynch)
            {           
                int j;
                for(j=0;j<newListenList.size(); j++)
                {
                    if (newListenList[j].fMbr==GetListenList()->GetMember(i))
                    {
                        found=true;
                        break;
                    }
                }
            }
            if (found==false)
            {
                llu.Receivers()->AddReceiverPlayerID(GetListenList()->GetMember(i)->GetPlayerID());
            }
        }

#ifndef SAME_TALK_AND_LISTEN
        if (llu.Receivers()->GetNumReceivers())
        {       
            // DEBUGGING
            int i;
            for(i=0;i<llu.Receivers()->GetNumReceivers(); i++)
            {
                int idx=fTransport.FindMember(llu.Receivers()->GetReceiverClientNum(i));
                plNetTransportMember* mbr=fTransport.GetMember(idx);
                DebugMsg("<SEND {}> ListenListUpdate msg, adding={}\n",
                    mbr->AsStdString(), llu.GetAdding());
            }
            
            SendMsg(&llu);
        }
#endif

        //
        // for each client in the new list, [if not in the old list,] send a ListenList add msg
        //
        llu.Receivers()->Clear();
        llu.SetAdding(true);
        for(i=0;i<newListenList.size(); i++)
        {
            if (forceSynch || GetListenList()->FindMember(newListenList[i].fMbr)==-1)
            {
                // if not in the old list, send a ListenList add msg
                llu.Receivers()->AddReceiverPlayerID(newListenList[i].fMbr->GetPlayerID());
            }
        }

#ifndef SAME_TALK_AND_LISTEN
        if (llu.Receivers()->GetNumReceivers())
        {
            // DEBUGGING
            int i;
            for(i=0;i<llu.Receivers()->GetNumReceivers(); i++)
            {
                int cNum=llu.Receivers()->GetReceiverClientNum(i);
                int idx=fTransport.FindMember(cNum);
                plNetTransportMember* mbr=fTransport.GetMember(idx);
                DebugMsg("<SEND {}> ListenListUpdate msg, adding={}, cNum={}\n",
                    mbr->AsStdString(), llu.GetAdding(), cNum);
            }

            SendMsg(&llu);
        }
#endif

        //
        // set as new listen list
        //
        GetListenList()->Clear();
#ifdef HS_DEBUGGING
        DebugMsg("New ListenList, size={}\n", newListenList.size());
#endif
        for(i=0;i<newListenList.size(); i++)
        {
            GetListenList()->AddMember(newListenList[i].fMbr);
#ifdef HS_DEBUGGING
            DebugMsg("\tLL Member {}, name={}, cNum={}, dist={f}\n",
                i, newListenList[i].fMbr->AsString(),
                newListenList[i].fMbr->GetPlayerID(), newListenList[i].fDistSq);
#endif
        }
    }       
    return changed;
}
//
// Periodically updates the list of what remote players I'm listening to.
// Used to filter voice streams.
// Returns true if the listenList was changed.
// Note: Updates distSq to each member. Other things rely on this so we must do it even if p2p is disabled.
//
bool plNetClientMgr::IUpdateListenList(double secs)
{
    if (GetFlagsBit(kDisabled))
        return false;
    if (!fLocalPlayerKey || !fLocalPlayerKey->ObjectIsLoaded())
        return false;
    
    bool changed = false;
    
    if (secs - GetListenList()->GetLastUpdateTime()>plNetListenList::kUpdateInterval)
    {
        GetListenList()->SetLastUpdateTime(secs);
        std::vector<DistSqInfo> newListenList;
        
        switch (fListenListMode)
        {
        case kListenList_Forced:
            {
#ifdef SAME_TALK_AND_LISTEN
                SynchTalkList();
#endif
            }
            return true;
        case kListenList_Distance:
            {
                // Finds the 3 closest players to our local player
                // Search is unoptimized for now...

                // compute our players pos
                plSceneObject* locPlayer = plSceneObject::ConvertNoRef(fLocalPlayerKey->ObjectIsLoaded());
                hsAssert(locPlayer, "local player is not a sceneObject?");
                hsAssert(locPlayer->GetCoordinateInterface(), "locPlayer has no coordInterface");
                hsMatrix44 l2w=locPlayer->GetCoordinateInterface()->GetLocalToWorld();
                hsPoint3 locPlayerPos=l2w.GetTranslate();

                for (size_t i = 0; i < fTransport.GetNumMembers(); i++)
                {
                    fTransport.GetMember(i)->SetDistSq(FLT_MAX);

                    if (fTransport.GetMember(i)->IsServer())
                        continue;
                    if(VaultAmIgnoringPlayer(fTransport.GetMember(i)->GetPlayerID()))
                    {           
                        continue;
                    }
                    plKey k=fTransport.GetMember(i)->GetAvatarKey();
#if 0
                    if (!k)
                    {
                        DebugMsg("UpdateListenList: Nil avatar key on member {}\n",
                            fTransport.GetMember(i)->AsStdString());
                    }
#endif
                    plSceneObject* obj=plSceneObject::ConvertNoRef(k ? k->ObjectIsLoaded() : nullptr);
                    if (obj && obj->GetCoordinateInterface())
                    {
#if 1
                        // compute distSq to me
                        l2w=obj->GetCoordinateInterface()->GetLocalToWorld();
                        hsPoint3 pos=l2w.GetTranslate();
                        float distSq = hsVector3(&pos, &locPlayerPos).MagnitudeSquared();

                        fTransport.GetMember(i)->SetDistSq(distSq);
                        
                        // I can't listen to players that are more than 50 ft away 
                        if (distSq>plNetListenList::kMaxListenDistSq)
                            continue;
#else
                        float distSq=1;
#endif
                        // if we have < 3 elements in the list, grow the list, or,
                        // if obj is closer than item 3, add it to the list.
                        // keep the list (3) elements sorted.
                        if (plNetListenList::kMaxListenListSize==-1 ||
                            newListenList.size()<plNetListenList::kMaxListenListSize ||
                            (distSq<newListenList[plNetListenList::kMaxListenListSize-1].fDistSq) )
                        {
                            DistSqInfo dsi(fTransport.GetMember(i), distSq);
                            if (plNetListenList::kMaxListenListSize==-1 ||
                                newListenList.size()<plNetListenList::kMaxListenListSize)
                            {
                                newListenList.push_back(dsi);   
                            }
                            else
                            {
                                newListenList[plNetListenList::kMaxListenListSize-1]=dsi;
                            }
                            if (plNetListenList::kMaxListenListSize!=-1)    // don't need to sort every time in this case
                            {
                                std::sort(newListenList.begin(), newListenList.end(), lessComp);
                            }
                        }
                    }
                    if (plNetListenList::kMaxListenListSize==-1 && newListenList.size())
                    {
                        std::sort(newListenList.begin(), newListenList.end(), lessComp);
                    }
                }
            }
            break;
        default:
            break;
        
        }

        hsAssert(plNetListenList::kMaxListenListSize==-1 || newListenList.size()<=plNetListenList::kMaxListenListSize, 
        "illegal new listenlist size");
    
        changed = IApplyNewListenList(newListenList, 
#ifdef SAME_TALK_AND_LISTEN
            false 
#else
            GetListenList()->CheckForceSynch()
#endif
        );
    }
    // update talkList based on listenList
    if (changed)
    {

#ifdef SAME_TALK_AND_LISTEN
        SynchTalkList();
#endif
        // notify KI, member distances have been updated
        plMemberUpdateMsg* mu = new plMemberUpdateMsg;
        mu->Send();
    }

    return changed;
}


void plNetClientMgr::SynchTalkList()
{
    GetTalkList()->Clear();
    int i;
    for(i=0;i<GetListenList()->GetNumMembers(); i++)
        GetTalkList()->AddMember(GetListenList()->GetMember(i));
}

void plNetClientMgr::SetListenListMode(int i)
{
    // set new mode, clear list and force update
    fListenListMode = i;
    GetListenList()->Clear();
    GetListenList()->SetLastUpdateTime(0.f);
}

void plNetClientMgr::IHandleNetVoiceListMsg(plNetVoiceListMsg* msg)
{
    if (msg->GetCmd() == plNetVoiceListMsg::kForcedListenerMode)
    {
        // first make sure this message applies to us:
        const std::vector<uint32_t>& clientList = msg->GetClientList();
        if (std::find(clientList.cbegin(), clientList.cend(), NetCommGetPlayer()->playerInt) == clientList.cend())
            return;
        SetListenListMode(kListenList_Forced);
        // add in the members we receive from python
        for (uint32_t clientId : clientList)
        {
            std::vector<plNetTransportMember*> members = plNetClientMgr::GetInstance()->TransportMgr().GetMemberListDistSorted();

            for (plNetTransportMember* mbr : members)
            {
                if (mbr != nullptr && mbr->GetAvatarKey() != nullptr && mbr->GetPlayerID() == clientId)
                {
                    plNetClientMgr::GetInstance()->GetListenList()->AddMember(mbr);
                }
            }
        }
    }
    else
    if (msg->GetCmd() == plNetVoiceListMsg::kDistanceMode)
    {
        // again, see that it is us that we care about:
        if (msg->GetRemovedKey() == GetLocalPlayerKey())
            SetListenListMode(kListenList_Distance);
    }   
}   

