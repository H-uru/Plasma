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
hsSsize_t plNetTransport::AddMember(plNetTransportMember* mbr)
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

void plNetTransport::IRemoveMember(plNetTransportMember* mbr)
{
    if (!mbr)
        return;

    hsLogEntry( plNetClientMgr::GetInstance()->DebugMsg("Removing member {}", mbr->AsString()) );

//  plNetClientMgr::GetInstance()->GetNetCore()->RemovePeer(mbr->GetPeerID());
    plNetClientMgr::GetInstance()->GetTalkList()->RemoveMember(mbr);
    plNetClientMgr::GetInstance()->GetListenList()->RemoveMember(mbr);

    auto it = std::find(fMembers.cbegin(), fMembers.cend(), mbr);

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
void plNetTransport::RemoveMember(size_t idx)
{
    plNetTransportMember* mbr=GetMember(idx);
    IRemoveMember(mbr);
}

//
// remove member from master list, and all subscription channels.
// return true on success.
//
void plNetTransport::RemoveMember(plNetTransportMember* mbr)
{
    IRemoveMember(mbr);
}

plNetTransportMember* plNetTransport::GetMemberByID(uint32_t playerID) const
{
    hsSsize_t memberIdx = FindMember(playerID);
    return (memberIdx < 0) ? nullptr : fMembers[memberIdx];
}

plNetTransportMember* plNetTransport::GetMemberByKey(const plKey& avKey) const
{
    hsSsize_t memberIdx = FindMember(avKey);
    return (memberIdx < 0) ? nullptr : fMembers[memberIdx];
}

//
// return array index or -1
//
hsSsize_t plNetTransport::FindMember(const plNetTransportMember* mbr)
{
    auto it = std::find(fMembers.cbegin(), fMembers.cend(), mbr);
    return (it == fMembers.cend()) ? -1 : (it-fMembers.cbegin());
}


//
// Send Msg to the server.
//
void plNetTransport::SendMsg(plNetMessage* netMsg) const
{
    NetCommSendMsg(netMsg);
}


void plNetTransport::ClearMembers()
{
    for (plNetTransportMember* mbr : fMembers)
    {
        hsAssert(mbr, "nil member?");
        delete mbr;
    } // for         
    
    fMembers.clear();
}


//
// return array index or -1
//
hsSsize_t plNetTransport::FindMember(uint32_t playerID) const
{
    for (size_t i = 0; i < GetNumMembers(); i++)
    {
        plNetTransportMember* mbr = GetMember(i);
        if (mbr->GetPlayerID()==playerID)
            return hsSsize_t(i);
    }
    return -1;
}

//
// return array index or -1
//
hsSsize_t plNetTransport::FindMember(const plKey& avKey) const
{
    for (size_t i = 0; i < GetNumMembers(); i++)
    {
        plNetTransportMember* mbr = GetMember(i);
        if (mbr->GetAvatarKey()==avKey)
            return hsSsize_t(i);
    }
    return -1;
}

void plNetTransport::DumpState()
{
    plNetClientMgr* nc=plNetClientMgr::GetInstance();
    
    hsLogEntry( nc->DebugMsg("-------------------\n") );

    nc->DebugMsg("Num Mbrs={}\n", GetNumMembers());
    for (size_t i = 0; i < GetNumMembers(); i++)
    {
        plNetTransportMember * mbr = GetMember(i);
        hsLogEntry(nc->DebugMsg("\tMbr {}, name={}, plyrID={}", i, mbr->AsString(), mbr->GetPlayerID()));
    }
    hsLogEntry( nc->DebugMsg("\n") );
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
