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

#include "plNetTransport.h"
#include "plNetTransportMember.h"

#include "hsTimer.h"

#include "pnNetCommon/pnNetCommon.h"

#include "plNetMessage/plNetMessage.h"
#include "plNetClient/plNetClientMgr.h"

#include <algorithm>
#include <cfloat>

plNetTransport::~plNetTransport()
{
    ClearMembers();
}

//
// add a member to the master list if not already there
//
int plNetTransport::AddMember(plNetTransportMember* mbr)
{
    if (FindMember(mbr)==-1)
    {
        fMembers.push_back(mbr);
        hsLogEntry( plNetClientMgr::GetInstance()->DebugMsg("Adding member {}", mbr->AsString()) );
        plNetClientMgr::GetInstance()->GetListenList()->AddMember(mbr);
        plNetClientMgr::GetInstance()->GetTalkList()->AddMember(mbr);
        DumpState();
        return fMembers.size()-1;
    }
    return -1;
}

void plNetTransport::IUnSubscribeToAllChannelGrps(plNetTransportMember* mbr)
{
    int i;
    for( i=mbr->GetNumSubscriptions()-1; i>=0 ; i-- )
    {
        int chan=mbr->GetSubscription(i);
        bool ok=UnSubscribeToChannelGrp(mbr, chan);
        hsAssert(ok, "can't find supposed subscription to remove");
    } // for             
}

void plNetTransport::IRemoveMember(plNetTransportMember* mbr)
{
    if (!mbr)
        return;

    hsLogEntry( plNetClientMgr::GetInstance()->DebugMsg("Removing member {}", mbr->AsString()) );

//  plNetClientMgr::GetInstance()->GetNetCore()->RemovePeer(mbr->GetPeerID());
    plNetClientMgr::GetInstance()->GetTalkList()->RemoveMember(mbr);
    plNetClientMgr::GetInstance()->GetListenList()->RemoveMember(mbr);

    // remove member from subscription lists
    IUnSubscribeToAllChannelGrps(mbr);

    plMembersList::iterator it=std::find(fMembers.begin(),fMembers.end(),mbr);

    // remove member from master list
    fMembers.erase(it);

    hsLogEntry( plNetClientMgr::GetInstance()->DebugMsg("Done Removing member {}", mbr->AsString()) );
    DumpState();
    
    delete mbr;
}

//
// remove member from master list, and all subscription channels.
// return true on success.
//
bool plNetTransport::RemoveMember(int idx)
{
    if (idx>=0)
    {
        plNetTransportMember* mbr=GetMember(idx);
        IRemoveMember(mbr);
        return true;
    }
    return false;
}

//
// remove member from master list, and all subscription channels.
// return true on success.
//
bool plNetTransport::RemoveMember(plNetTransportMember* mbr)
{
    IRemoveMember(mbr);
    return true;
}

//
// return array index or -1
//
int plNetTransport::FindMember(const plNetTransportMember* mbr) 
{
    plMembersList::iterator it = std::find(fMembers.begin(), fMembers.end(), mbr);
    return (it==fMembers.end()) ? -1 : (it-fMembers.begin());
}


//
// add this member to the given channel grp
//
void plNetTransport::SubscribeToChannelGrp(plNetTransportMember* mbr, int channel)
{
//  hsAssert(FindMember(mbr) != -1, "must add member before subscribing to channel");
    if (mbr->AddSubscription(channel))
    {
        hsAssert(channel<fChannelGroups.size(), "invalid channel index");
        fChannelGroups[channel].push_back(mbr);
    }
}

//
// Remove the subscription to the given channel grp for a member.
//
bool plNetTransport::UnSubscribeToChannelGrp(plNetTransportMember* mbr, int chan)
{
    hsAssert(chan>=0 && chan<fChannelGroups.size(), "invalid channel idx");
    plMembersList* mList = &fChannelGroups[chan];
    plMembersList::iterator it=std::find(mList->begin(), mList->end(), mbr);
    if (it != mList->end())
    {
        mList->erase(it);
        bool ret=mbr->RemoveSubscription(chan);
        hsAssert(ret, "error removing subscription");
        return true;
    }
    return false;
}

//
// copy list of members channelGrp subscriptions
//
void plNetTransport::GetSubscriptions(plNetTransportMember* mbr, std::vector<int>* channels) const
{
    mbr->CopySubscriptions(channels);
}


//
// Send Msg to all members in the given channelGrp.
// Here's where multicasting would be used.
// Returns neg number (NetCore::RetCode) on send error, 1, if not sent, and 0 if sent
//
int plNetTransport::SendMsg(int chan, plNetMessage* netMsg) const
{
    NetCommSendMsg(netMsg);
    return hsOK;
    
    plNetClientMgr* nc=plNetClientMgr::GetInstance();
    int ret=1; // didn't send

    if (chan < fChannelGroups.size())
    {
        const plMembersList* mList = &fChannelGroups[chan];
                
        // does this msg have a list of receivers
        plNetMsgReceiversListHelper* rl = plNetMsgReceiversListHelper::ConvertNoRef(netMsg);

#if 0               
        // send msg to all subscribers to this channel
        int size=mList->size();
        for( int i=0 ; i<size; i++  )
        {
            hsAssert(false, "eric, port me");

            plNetTransportMember* tm=(*mList)[i];
            hsAssert(tm, "nil mbr in sendMsg");
//          int peerID=tm->GetPeerID();
//          hsAssert(peerID>=0, "queing message to invalid peer");

//          if ((ncRet=nc->GetNetClientComm().SendMsg(netMsg, peerID, sendFlags, msgSize)) != plNetCore::kNetOK)

            NetCommSendMsg(netMsg);
            if (rl)
            {
                bool ok=rl->RemoveReceiverPlayerID(tm->GetPlayerID());
                hsAssert(ok, "couldn't find rcvr to remove?");
            }
            ret=0; // sent ok   
        } // for      
#endif

        // if there are rcvrs left that we couldn't send to, send via server
        if (rl && rl->GetNumReceivers())
        {           
//          if ((ncRet=nc->GetNetClientComm().SendMsg(netMsg, nc->GetServerPeerID(), sendFlags, msgSize)) != plNetCore::kNetOK)
            NetCommSendMsg(netMsg);
            ret=0;  // sent
        }
    }
    else
    {
        hsStatusMessage("EMPTY TRANSPORT GROUP\n");
    }
    return ret;
}


void plNetTransport::ClearMembers()
{
    int i;
    for( i=0 ;i<GetNumMembers() ;i++  )
    {
        plNetTransportMember* mbr = GetMember(i);
        hsAssert(mbr, "nil member?");
        IUnSubscribeToAllChannelGrps(mbr);
        delete mbr;
    } // for         
    
    fMembers.clear();
}


//
// return array index or -1
//
int plNetTransport::FindMember(uint32_t playerID) const
{
    int i;
    for( i=0 ;i<GetNumMembers() ;i++  )
    {
        plNetTransportMember* mbr = GetMember(i);
        if (mbr->GetPlayerID()==playerID)
            return i;
    }
    return -1;
}

//
// return array index or -1
//
int plNetTransport::FindMember(const plKey avKey) const
{
    int i;
    for( i=0 ;i<GetNumMembers() ;i++  )
    {
        plNetTransportMember* mbr = GetMember(i);
        if (mbr->GetAvatarKey()==avKey)
            return i;
    }
    return -1;
}

//
// clear channel and unsubscribe all members to that channel
//
void plNetTransport::ClearChannelGrp(int channel)
{
    const plMembersList* mList = &fChannelGroups[channel];
    int i, size=mList->size();
    for( i=0 ; i<size; i++  )
    {
        plNetTransportMember* tm=(*mList)[i];
        bool ok=tm->RemoveSubscription(channel);
        hsAssert(ok, "error removing subscription");
    }

    fChannelGroups[channel].clear();
}

void plNetTransport::DumpState()
{
    plNetClientMgr* nc=plNetClientMgr::GetInstance();
    
    hsLogEntry( nc->DebugMsg("-------------------\n") );
    hsLogEntry( nc->DebugMsg("Num Channels={}\n", fChannelGroups.size()) );

    int i;
    for(i=0;i<fChannelGroups.size();i++)
    {
        plMembersList* mList = &fChannelGroups[i];
        hsLogEntry( nc->DebugMsg("\tChannel {}, num mbrs={}\n", i, mList->size()) );
        int j;
        for(j=0; j<mList->size();j++)
        {
            hsLogEntry( nc->DebugMsg("\t\tMbr {}\n",(*mList)[j]->AsString()) );
        }
    }

    nc->DebugMsg("Num Mbrs={}\n", GetNumMembers());
    for(i=0;i<GetNumMembers();i++)
    {
        plNetTransportMember * mbr = GetMember(i);
        hsLogEntry (nc->DebugMsg("\tMbr {}, name={}, plyrID={}, subs={}",
            i,mbr->AsString(),mbr->GetPlayerID(),mbr->GetNumSubscriptions()) );
        int j;
        for(j=0;j<mbr->GetNumSubscriptions();j++)
        {
            hsLogEntry( nc->DebugMsg("\t\tSub {}, chan={}\n", j, mbr->GetSubscription(j)) );
        }
    }
    hsLogEntry( nc->DebugMsg("\n") );
}

void plNetTransport::SetNumChannels(int n)
{
    if (n>fChannelGroups.size())
        fChannelGroups.resize(n);
}

//
// create a members list sorted by dist.
//
std::vector<plNetTransportMember*> plNetTransport::GetMemberListDistSorted() const
{
    std::vector<plNetTransportMember*> sortedList = fMembers;

    std::sort(sortedList.begin(), sortedList.end(),
              [](plNetTransportMember* l, plNetTransportMember* r) {
                  const float d1 = l ? l->GetDistSq() : FLT_MAX;
                  const float d2 = r ? r->GetDistSq() : FLT_MAX;
                  return d1 < d2;
              });
    return sortedList;
}
