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

#include <algorithm>
#include "hsMatrix44.h"
#include "hsGeometry3.h"
#include "plNetClientMgr.h"

#include "../plNetMessage/plNetMessage.h"
#include "../pnNetCommon/plNetServers.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../pnKeyedObject/plKey.h"

#include "../plNetTransport/plNetTransportMember.h"
#include "../plMessage/plMemberUpdateMsg.h"
#include "../plMessage/plNetVoiceListMsg.h"
#include "../plStatusLog/plStatusLog.h"
#include "../plVault/plVault.h"


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
hsBool plNetClientMgr::IApplyNewListenList(std::vector<DistSqInfo>& newListenList, hsBool forceSynch)
{
	hsBool changed=false;
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
		DebugMsg("ListenList changed, forceSynch=%d\n", forceSynch);

		plNetMsgListenListUpdate llu;
		llu.SetPlayerID(GetPlayerID());
		llu.SetNetProtocol(kNetProtocolCli2Game);
		
		//
		// for each client in the old list, if not in the new list, send a ListenList remove msg
		//
		llu.Receivers()->Clear();
		llu.SetAdding(false);
		for(i=0;i<GetListenList()->GetNumMembers(); i++)
		{
			hsBool found=false;
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
				DebugMsg("<SEND %s> ListenListUpdate msg, adding=%d\n", 
					mbr->AsStdString().c_str(), llu.GetAdding());
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
				DebugMsg("<SEND %s> ListenListUpdate msg, adding=%d, cNum=%d\n", 
					mbr->AsStdString().c_str(), llu.GetAdding(), cNum);
			}

			SendMsg(&llu);
		}
#endif

		//
		// set as new listen list
		//
		GetListenList()->Clear();
#ifdef HS_DEBUGGING
		DebugMsg("New ListenList, size=%d\n", newListenList.size());
#endif
		for(i=0;i<newListenList.size(); i++)
		{
			GetListenList()->AddMember(newListenList[i].fMbr);
#ifdef HS_DEBUGGING
			DebugMsg("\tLL Member %d, name=%s, cNum=%d, dist=%f\n", 
				i, newListenList[i].fMbr->AsStdString().c_str(), 
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
hsBool plNetClientMgr::IUpdateListenList(double secs)
{
	if (GetFlagsBit(kDisabled))
		return false;
	if (!fLocalPlayerKey || !fLocalPlayerKey->ObjectIsLoaded())
		return false;
	
	hsBool changed = false;
	
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

				int i;
				for(i=0;i<fTransport.GetNumMembers();i++)
				{
					fTransport.GetMember(i)->SetDistSq(hsScalarMax);

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
						DebugMsg("UpdateListenList: Nil avatar key on member %s\n", 
							fTransport.GetMember(i)->AsStdString().c_str());
					}
#endif
					plSceneObject* obj=plSceneObject::ConvertNoRef(k ? k->ObjectIsLoaded() : nil);
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
						// if we are p2p and member isn't, skip them.
						if ( IsPeerToPeer() && !fTransport.GetMember(i)->IsPeerToPeer() )
							continue;
						// otherwise, we aren't p2p so just update the listen list
						// normally so it will update in the gui as distance changes.
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
							if (plNetListenList::kMaxListenListSize!=-1)	// don't need to sort every time in this case
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
		plMemberUpdateMsg* mu = TRACKED_NEW plMemberUpdateMsg;
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
		int i;
		bool included = false;
		for (i = 0; i < msg->GetClientList()->Count(); i++)
		{	
			if (msg->GetClientList()->AcquireArray()[i] == NetCommGetPlayer()->playerInt)
			{	
				included = true;
				break;
			}
		}
		if (!included)
			return;
		SetListenListMode(kListenList_Forced);
		// add in the members we receive from python
		for (i = 0; i < msg->GetClientList()->Count(); i++)
		{
			plNetTransportMember **members = nil;
			plNetClientMgr::GetInstance()->TransportMgr().GetMemberListDistSorted( members );
					
			if( members != nil)
			{
				for(int j= 0; j < plNetClientMgr::GetInstance()->TransportMgr().GetNumMembers(); j++ )
				{
					plNetTransportMember *mbr = members[ j ];
					if( mbr != nil && mbr->GetAvatarKey() != nil && mbr->GetPlayerID() == msg->GetClientList()->AcquireArray()[i])
					{
						plNetClientMgr::GetInstance()->GetListenList()->AddMember(mbr);
					}
				}
			}
			delete [] members;
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

