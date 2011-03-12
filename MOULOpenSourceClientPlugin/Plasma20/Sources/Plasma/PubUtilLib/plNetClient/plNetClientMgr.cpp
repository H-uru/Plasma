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
#include "hsTimer.h"
#include "hsStream.h"
#include "plNetClientMgr.h"
#include "plgDispatch.h"
#include "plPhysical.h"
#include "plNetClientMsgHandler.h"
#include "plNetLinkingMgr.h"
#include "plNetObjectDebugger.h"

#include "../pnUtils/pnUtils.h"
#include "../pnProduct/pnProduct.h"
#include "../pnNetCommon/plSynchedObject.h"
#include "../pnNetCommon/plNetServers.h"
#include "../pnNetCommon/plSDLTypes.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnKeyedObject/plFixedKey.h"
#include "../pnKeyedObject/hsKeyedObject.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plObjInterface.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../pnMessage/plClientMsg.h"
#include "../pnMessage/plSDLModifierMsg.h"
#include "../pnMessage/plPlayerPageMsg.h"
#include "../pnMessage/plTimeMsg.h"
#include "../pnModifier/plModifier.h"
#include "../pnAsyncCore/pnAsyncCore.h"

#include "../plAgeLoader/plAgeLoader.h"
#include "../plAgeLoader/plResPatcher.h"
#include "../plNetClientRecorder/plNetClientRecorder.h"
#include "../plScene/plSceneNode.h"
#include "../plNetCommon/plNetCommonConstants.h"
#include "../plNetMessage/plNetMessage.h"
#include "../plMessage/plLoadAvatarMsg.h"
#include "../plMessage/plLoadCloneMsg.h"
#include "../plMessage/plSynchEnableMsg.h"
#include "../plMessage/plLinkToAgeMsg.h"
#include "../plMessage/plLoadAgeMsg.h"
#include "../plMessage/plAgeLoadedMsg.h"
#include "../plMessage/plCCRMsg.h"
#include "../plMessage/plAvatarMsg.h"
#include "../plMessage/plNetVoiceListMsg.h"
#include "../plMessage/plNetCommMsgs.h"
#include "../plMessage/plNetClientMgrMsg.h"
#include "../plResMgr/plKeyFinder.h"
#include "../plResMgr/plPageInfo.h"
#include "../plNetTransport/plNetTransportMember.h"
#include "../plAgeDescription/plAgeDescription.h"
#include "../plAvatar/plAvatarClothing.h"
#include "../plAvatar/plArmatureMod.h"
#include "../plAvatar/plAvatarMgr.h"
#include "../plSurface/plLayerInterface.h"
#include "../plStatusLog/plStatusLog.h"
#include "../plSDL/plSDL.h"
#include "../plUnifiedTime/plUnifiedTime.h"
#include "../plFile/plEncryptedStream.h"
#include "../plProgressMgr/plProgressMgr.h"
#include "../plVault/plVault.h"

#include "../../FeatureLib/pfMessage/pfKIMsg.h"	// Move this to PubUtil level

#if 1	// for debugging
#include "plCreatableIndex.h"	
#include "../plModifier/plResponderModifier.h"
#include "../plSurface/plLayerAnimation.h"
#endif

#include <algorithm>
#include <sstream>



////////////////////////////////////////////////////////////////////

plNetClientMgr::PendingLoad::~PendingLoad()
{
	delete fSDRec;
}

////////////////////////////////////////////////////////////////////

//
// CONSTRUCT
//
#pragma warning(disable:4355)	// this used in initializer list
plNetClientMgr::plNetClientMgr() : 
		fLocalPlayerKey(nil),
		fMsgHandler(this),
		fJoinOrder(0),
		// fProgressBar( nil ),
		fTaskProgBar( nil ),
		fMsgRecorder(nil),
		fServerTimeOffset(0),
		fTimeSamples(0),
		fLastTimeUpdate(0),
		fListenListMode(kListenList_Distance),
		fAgeSDLObjectKey(nil),
		fExperimentalLevel(0),
		fOverrideAgeTimeOfDayPercent(-1.f),
		fNumInitialSDLStates(0),
		fRequiredNumInitialSDLStates(0),
		fDisableMsg(nil),
		fIsOwner(true)
{	
#ifndef HS_DEBUGGING
	// release code will timeout inactive players on servers by default
	SetFlagsBit(kAllowTimeOut);
#endif
	SetFlagsBit(kAllowAuthTimeOut);

//	fPlayerVault.SetPlayerName("SinglePlayer"); // in a MP game, this will be replaced with a player name like 'Atrus'
	fTransport.SetNumChannels(kNetNumChannels);	
}
#pragma warning(default:4355)

//
// DESTRUCT
//
plNetClientMgr::~plNetClientMgr()
{
	plSynchedObject::PushSynchDisabled(true);		// disable dirty tracking

	IDeInitNetClientComm();	

	if (this==GetInstance())
		SetInstance(nil);		// we're going down boys
	
	IClearPendingLoads();
}

//
// App is exiting
//
void plNetClientMgr::Shutdown()
{
	ISendDirtyState(hsTimer::GetSysSeconds());

	plNetLinkingMgr::GetInstance()->LeaveAge(true);

	// release existing remote players
	int i;
	for (i=0;i<RemotePlayerKeys().size();i++)
	{
		plKey k=RemotePlayerKeys()[i];
		plAvatarMgr::GetInstance()->UnLoadRemotePlayer(k);
	}

	// Finally, pump the dispatch system so all the new refs get delivered.
	plgDispatch::Dispatch()->MsgQueueProcess();

	if (fMsgRecorder)
	{
		delete fMsgRecorder;
		fMsgRecorder = nil;
	}
	for (i = 0; i < fMsgPlayers.size(); i++)
		delete fMsgPlayers[i];
	fMsgPlayers.clear();

	IRemoveCloneRoom();

	// RATHER BAD DEBUG HACK: Clear the spawn override in armatureMod so there's no memory leak
	plArmatureMod::SetSpawnPointOverride( nil );

	VaultDestroy();
}

//
// Clear any pending state, probably joining a new age
//
void plNetClientMgr::IClearPendingLoads()
{
	std::for_each( fPendingLoads.begin(), fPendingLoads.end(), xtl::delete_ptr() );
	fPendingLoads.clear();
}

//
// manually add/remove the cloned object room in plClient
//
void plNetClientMgr::IAddCloneRoom()
{
	plSceneNode *cloneRoom = TRACKED_NEW plSceneNode();
	cloneRoom->RegisterAs(kNetClientCloneRoom_KEY);
	plKey clientKey=hsgResMgr::ResMgr()->FindKey(kClient_KEY);
	plClientRefMsg *pMsg1 = TRACKED_NEW plClientRefMsg(clientKey, plRefMsg::kOnCreate, -1, plClientRefMsg::kManualRoom);
	hsgResMgr::ResMgr()->AddViaNotify(cloneRoom->GetKey(), pMsg1, plRefFlags::kPassiveRef);
}

//
// manually add/remove the cloned object room in plClient
//
void plNetClientMgr::IRemoveCloneRoom()
{
	plKey cloneRoomKey = hsgResMgr::ResMgr()->FindKey(kNetClientCloneRoom_KEY);
	plSceneNode *cloneRoom = cloneRoomKey ? plSceneNode::ConvertNoRef(cloneRoomKey->ObjectIsLoaded()) : nil;
	if (cloneRoom)
	{
		cloneRoom->UnRegisterAs(kNetClientCloneRoom_KEY);
		cloneRoom = nil;
	}
}		

//
// turn null send on/off.  Null send does everything except actually send the msg out on the socket
//
void plNetClientMgr::SetNullSend(hsBool on)	
{
}

//
// returns server time in the form "[m/d/y h:m:s]"
//
const char* plNetClientMgr::GetServerLogTimeAsString(std::string& timestamp) const
{
	const plUnifiedTime st=GetServerTime();
	xtl::format(timestamp, "{%02d/%02d %02d:%02d:%02d}", 
		st.GetMonth(), st.GetDay(), st.GetHour(), st.GetMinute(), st.GetSecond());
	return timestamp.c_str();
}

//
// support for a single leading tab in statusLog strings
//
const char* ProcessTab(const char* fmt)
{
	static std::string s;
	if (fmt && *fmt=='\t')
	{
		s = xtl::format("  %s", fmt);
		return s.c_str();
	}
	return fmt;
}

//
// override for plLoggable
//
bool plNetClientMgr::Log(const char* str) const
{
	if (strlen(str)==nil)
		return true;

	// prepend raw time
	std::string buf2 = xtl::format("%.2f %s", hsTimer::GetSeconds(), ProcessTab(str));

	if ( GetConsoleOutput() )
		hsStatusMessage(buf2.c_str());

	// create status log if necessary
	if(fStatusLog==nil)
	{
		fStatusLog = plStatusLogMgr::GetInstance().CreateStatusLog(40, "network.log",
			plStatusLog::kTimestamp | plStatusLog::kFilledBackground | plStatusLog::kAlignToTop | 
			plStatusLog::kServerTimestamp);
		fWeCreatedLog = true;
	}

	plNetObjectDebugger::GetInstance()->LogMsgIfMatch(buf2.c_str());
	return fStatusLog->AddLine(buf2.c_str());
}

//
// Display OS version info for log
//
void plNetClientMgr::IDumpOSVersionInfo() const
{
	DebugMsg("*** OS Info");
	char** versionStrs = DisplaySystemVersion();
	int i=0;
	while(versionStrs && versionStrs[i])
	{
		DebugMsg(versionStrs[i]);
		delete [] versionStrs[i];
		i++;
	}
	delete [] versionStrs;
}

//
// initialize net client. returns hsFail on err.
//
int plNetClientMgr::Init()
{
	int ret=hsOK;
	hsLogEntry( DebugMsg("*** plNetClientMgr::Init GMT:%s", plUnifiedTime::GetCurrentTime().Print()) );
	
	IDumpOSVersionInfo();
	
	if (GetFlagsBit(kNullSend))
		SetNullSend(true);

	VaultInitialize();

	RegisterAs( kNetClientMgr_KEY );
	IAddCloneRoom();

	fNetGroups.Reset();

	plgDispatch::Dispatch()->RegisterForExactType(plNetClientMgrMsg::Index(), GetKey());	
	plgDispatch::Dispatch()->RegisterForExactType(plAgeLoadedMsg::Index(), GetKey());	
	plgDispatch::Dispatch()->RegisterForExactType(plAgeLoaded2Msg::Index(), GetKey());	
	plgDispatch::Dispatch()->RegisterForExactType(plCCRPetitionMsg::Index(), GetKey());	
	plgDispatch::Dispatch()->RegisterForExactType(plPlayerPageMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plInitialAgeStateLoadedMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plNetVoiceListMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plClientMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());

	plgDispatch::Dispatch()->RegisterForExactType(plNetCommAuthMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plNetCommActivePlayerMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plNetCommLinkToAgeMsg::Index(), GetKey());

    IInitNetClientComm();

	return ret;	
}

//
// Prepare to send.
// Update p2p transport groups and rcvrs list in some msgs
// Returns channel.
//
int plNetClientMgr::IPrepMsg(plNetMessage* msg)
{
	// pick channel, prepare msg
	int channel=kNetChanDefault;
	plNetMsgVoice* v=plNetMsgVoice::ConvertNoRef(msg);
	if (v)
	{		// VOICE MSG
		channel=kNetChanVoice;
		
		// compute new transport group (from talkList) if necessary
		GetTalkList()->UpdateTransportGroup(this);	
		
		// update receivers list in voice msg based on talk list
		v->Receivers()->Clear();
		int i;
		for(i=0;i<GetTalkList()->GetNumMembers(); i++)
		{
			v->Receivers()->AddReceiverPlayerID(GetTalkList()->GetMember(i)->GetPlayerID());
		}

		if (msg->IsBitSet(plNetMessage::kEchoBackToSender))
			v->Receivers()->AddReceiverPlayerID(GetPlayerID());
	}
	else if (plNetMsgListenListUpdate::ConvertNoRef(msg))
	{	// LISTEN LIST UPDATE MSG
		channel=kNetChanListenListUpdate;

		// update transport group from rcvrs list, add p2p mbrs to trasnport group
		fTransport.ClearChannelGrp(kNetChanListenListUpdate);
		if (IsPeerToPeer())
		{
			plNetMsgReceiversListHelper* rl = plNetMsgReceiversListHelper::ConvertNoRef(msg);
			hsAssert(rl, "error converting msg to rcvrs list?");
			int i;
			for(i=0;i<rl->GetNumReceivers();i++)
			{
				int idx=fTransport.FindMember(rl->GetReceiverPlayerID(i));
				hsAssert(idx!=-1, "error finding transport mbr");
				plNetTransportMember* tm = fTransport.GetMember(idx);
				if (tm->IsPeerToPeer())
					fTransport.SubscribeToChannelGrp(tm, kNetChanListenListUpdate);
			}
		}
	}
	else if( plNetMsgGameMessageDirected::ConvertNoRef( msg ) )
	{
		channel = kNetChanDirectedMsg;
	}
	return channel;
}

//
// unload other player since we're leaving the age
//
void plNetClientMgr::IUnloadRemotePlayers()
{
	for(int i=RemotePlayerKeys().size()-1;i>=0;i--)
		plAvatarMgr::GetInstance()->UnLoadRemotePlayer(RemotePlayerKeys()[i]);
	hsAssert(!RemotePlayerKeys().size(),"Still remote players left when linking out");
}

//
// begin linking out sounds and gfx
//
void plNetClientMgr::StartLinkOutFX()
{
	// send msg to trigger link out effect
	if (fLocalPlayerKey)
	{
		plNetLinkingMgr * lm = plNetLinkingMgr::GetInstance();

		plLinkEffectsTriggerMsg* lem = TRACKED_NEW plLinkEffectsTriggerMsg();
		lem->SetLeavingAge(true);
		lem->SetLinkKey(fLocalPlayerKey);
		lem->SetBCastFlag(plMessage::kNetPropagate);
		lem->SetBCastFlag(plMessage::kNetForce);	// Necessary?
		lem->AddReceiver(hsgResMgr::ResMgr()->FindKey(plUoid(kLinkEffectsMgr_KEY)));
		lem->Send();
	}
}

//
// beging linking in sounds snd gfx
//
void plNetClientMgr::StartLinkInFX()
{
	if (fLocalPlayerKey)
	{
		const plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();

		plLinkEffectsTriggerMsg* lem = TRACKED_NEW plLinkEffectsTriggerMsg();
		lem->SetLeavingAge(false);	// linking in
		lem->SetLinkKey(fLocalPlayerKey);
		lem->SetLinkInAnimKey(avMod->GetLinkInAnimKey());
		
		// indicate if we are invisible 
		if (avMod && avMod->IsInStealthMode() && avMod->GetTarget(0))
		{
			lem->SetInvisLevel(avMod->GetStealthLevel());
		}

		lem->SetBCastFlag(plMessage::kNetPropagate);

		lem->AddReceiver(hsgResMgr::ResMgr()->FindKey(plUoid(kLinkEffectsMgr_KEY)));
		lem->AddReceiver(hsgResMgr::ResMgr()->FindKey(plUoid(kClient_KEY)));
		plgDispatch::MsgSend(lem);
	}
}

//
// compute the difference between our clock and the server's in unified time
//
void plNetClientMgr::UpdateServerTimeOffset(plNetMessage* msg)
{
	if ((hsTimer::GetSysSeconds() - fLastTimeUpdate) > 5)
	{
		fLastTimeUpdate = hsTimer::GetSysSeconds();

		const plUnifiedTime& msgSentUT = msg->GetTimeSent();
		if (!msgSentUT.AtEpoch())
		{
			double diff = plUnifiedTime::GetTimeDifference(msgSentUT, plClientUnifiedTime::GetCurrentTime());

			if (fServerTimeOffset == 0)
			{
				fServerTimeOffset = diff;
			}
			else
			{
				fServerTimeOffset = fServerTimeOffset + ((diff - fServerTimeOffset) / ++fTimeSamples);
			}

			DebugMsg("Setting server time offset to %f", fServerTimeOffset);
		}
	}
}

void plNetClientMgr::ResetServerTimeOffset()
{
	fServerTimeOffset = 0;
	fTimeSamples = 0;
	fLastTimeUpdate = 0;
}

//
// return the gameservers time
//
plUnifiedTime plNetClientMgr::GetServerTime() const 
{ 
	if ( fServerTimeOffset==0 )		// offline mode or before connecting/calibrating to a server
		return plUnifiedTime::GetCurrentTime();
	
	plUnifiedTime serverUT;
	if (fServerTimeOffset<0)
		return plUnifiedTime::GetCurrentTime() - plUnifiedTime(fabs(fServerTimeOffset)); 
	else
		return  plUnifiedTime::GetCurrentTime() + plUnifiedTime(fServerTimeOffset); 
}

//
// How old is the age
//
double plNetClientMgr::GetCurrentAgeElapsedSeconds() const
{
	return plAgeLoader::GetInstance()->GetCurrAgeDesc().GetAgeElapsedSeconds(GetServerTime());
}

//
// Get the time in the current age (0-1), taking into account the age's day length
//
float plNetClientMgr::GetCurrentAgeTimeOfDayPercent() const
{
	if (fOverrideAgeTimeOfDayPercent>=0)
		return fOverrideAgeTimeOfDayPercent;

	return plAgeLoader::GetInstance()->GetCurrAgeDesc().GetAgeTimeOfDayPercent(GetServerTime());
}

//
// main update fxn for net client code, return hsFail on err
//
int plNetClientMgr::Update(double secs)
{
	int ret=hsOK;	// ret code is unchecked, but what the hay
	
	if (GetFlagsBit(kDisableOnNextUpdate)) {
		SetFlagsBit(kDisableOnNextUpdate, false);
		IDisableNet();
	}
	
	// Pump net messages
	NetCommUpdate();

	static double lastUpdateTime=0;
	double curTime=hsTimer::GetSeconds();
	if (curTime-lastUpdateTime > 1.f)
	{
		DebugMsg("NetClient hasn't updated for %f secs", curTime-lastUpdateTime);
	}
	lastUpdateTime=curTime;

	if (!GetFlagsBit(kDisabled))
	{
		fNetClientStats.UpdateAgeStats();
	}

	if (GetFlagsBit(kPlayingGame) )
	{
		MaybeSendPendingPagingRoomMsgs();
		ICheckPendingStateLoad(secs);
		ISendDirtyState(secs);
		IUpdateListenList(secs);
		if (GetFlagsBit(plNetClientApp::kShowLists))
			IShowLists();
		if (GetFlagsBit(plNetClientApp::kShowRooms))
			IShowRooms();
		if (GetFlagsBit(plNetClientApp::kShowAvatars))
			IShowAvatars();
		if (GetFlagsBit(plNetClientApp::kShowRelevanceRegions))
			IShowRelevanceRegions();
	}

	// Send dirty nodes, deliver vault callbacks, etc.
	VaultUpdate();

	plNetLinkingMgr::GetInstance()->Update();

	return ret;
}

//
// See if there is state recvd from the network that needsto be delivered
//
void plNetClientMgr::ICheckPendingStateLoad(double secs)
{
	if (!fPendingLoads.empty() && GetFlagsBit( kPlayingGame ) || (GetFlagsBit(kLoadingInitialAgeState) && !GetFlagsBit(kNeedInitialAgeStateCount)))
	{
		PendingLoadsList::iterator it = fPendingLoads.begin();
		while ( it!=fPendingLoads.end() )
		{
			PendingLoad * pl = (*it);

			// cache rcvr key
			if (!pl->fKey)
			{
				// check for existence of key in dataset, excluding clone info.
				plUoid tmpUoid = pl->fUoid;
				tmpUoid.SetClone(0,0);
				if ( !hsgResMgr::ResMgr()->FindKey( tmpUoid ) )
				{
					// discard the state if object not found in dataset.
					char tmp[256];
					hsLogEntry( DebugMsg( "Failed to find object %s in dataset. Discarding pending state '%s'",
						tmpUoid.StringIze(tmp),
						pl->fSDRec->GetDescriptor()->GetName() ) );
					delete pl;
					it = fPendingLoads.erase(it);
					continue;
				}
				// find and cache the real key.
				pl->fKey = hsgResMgr::ResMgr()->FindKey(pl->fUoid);
			}
						
			// deliver state if possible
			plSynchedObject*so = pl->fKey ? plSynchedObject::ConvertNoRef(pl->fKey->ObjectIsLoaded()) : nil;
			if (so && so->IsFinal())
			{				
				plSDLModifierMsg* sdlMsg = TRACKED_NEW plSDLModifierMsg(pl->fSDRec->GetDescriptor()->GetName(), 
					plSDLModifierMsg::kRecv);
				sdlMsg->SetState( pl->fSDRec, true/*delete pl->fSDRec for us*/ );
				sdlMsg->SetPlayerID( pl->fPlayerID );

#ifdef HS_DEBUGGING
				if (plNetObjectDebugger::GetInstance()->IsDebugObject(so))
				{
					hsLogEntry( DebugMsg( "Delivering SDL state %s:%s", pl->fKey->GetName(), pl->fSDRec->GetDescriptor()->GetName() ) );
//					hsLogEntry(plNetObjectDebugger::GetInstance()->LogMsg(xtl::format("Dispatching SDL state, type %s to object:%s, locallyOwned=%d, st=%.3f rt=%.3f", 
//						pl->fSDRec->GetDescriptor()->GetName(), pl->fKey->GetName(), 
//						so->IsLocallyOwned()==plSynchedObject::kYes, secs, hsTimer::GetSeconds()).c_str()));
//					hsLogEntry( pl->fSDRec->DumpToObjectDebugger( "Delivering SDL state", false, 0 ) );
				}
#endif

				sdlMsg->Send(pl->fKey);

				pl->fSDRec = nil;	// so it won't be deleted in the PendingLoad dtor.
				delete pl;
				it = fPendingLoads.erase(it);
			}
			else
			{
				// report old pending state
				double rawSecs = hsTimer::GetSeconds();
				if ((rawSecs - pl->fQueuedTime) > 60.f /*secs*/)
				{
					char tmp[256], tmp2[256];
					if (pl->fQueueTimeResets >= 5)
					{
						// if this is our fifth time in here then we've been queued
						// for around 5 minutes and its time to go

						WarningMsg( "Pending state '%s' for object [uoid:%s,key:%s] has been queued for about %f secs. Removing...",
							pl->fSDRec && pl->fSDRec->GetDescriptor() ? pl->fSDRec->GetDescriptor()->GetName() : "?",
							pl->fUoid.StringIze(tmp2), pl->fKey ? pl->fKey->GetUoid().StringIze(tmp) : "?",
							( rawSecs - pl->fQueuedTime ) * pl->fQueueTimeResets);

						delete pl;
						it = fPendingLoads.erase(it);
						continue;
					}

					WarningMsg( "Pending state '%s' for object [uoid:%s,key:%s] has been queued for about %f secs. %s",
						pl->fSDRec && pl->fSDRec->GetDescriptor() ? pl->fSDRec->GetDescriptor()->GetName() : "?",
						pl->fUoid.StringIze(tmp2), pl->fKey ? pl->fKey->GetUoid().StringIze(tmp) : "?",
						( rawSecs - pl->fQueuedTime ) * pl->fQueueTimeResets,
						so ? "(not loaded)" : "(not final)" );
					// reset queue time so we don't spew too many log msgs.
					pl->fQueuedTime = rawSecs;
					pl->fQueueTimeResets += 1;
				}

				it++;
			}
		}
	}
}

//
// Determine the net group for a given object (based on paging unit)
//
plNetGroupId plNetClientMgr::SelectNetGroup(plSynchedObject* objIn, plKey roomKey)
{
	if( objIn->GetSynchFlags() & plSynchedObject::kHasConstantNetGroup )
		return objIn->GetNetGroup();

	return plNetGroupId(roomKey->GetUoid().GetLocation(), 0);
}

//
// Get the net group that an object belongs to, taking into acct
// that objects may derive their net group from another object they belong to.
//
plNetGroupId plNetClientMgr::GetEffectiveNetGroup(const plSynchedObject*& obj) const
{
   plNetGroupId netGroup = plNetGroup::kNetGroupUnknown;
	// the object is an interface, so consider the owner instead 
	const plObjInterface* oInt=plObjInterface::ConvertNoRef(obj);
	if (oInt)
	{
		// the object is an interface, so consider the owner instead 
		if (oInt->GetOwner())
			obj = oInt->GetOwner();
	}	
	else
	{
		const plModifier* mod=plModifier::ConvertNoRef(obj);
		if (mod)
		{
			// the object is a modifier, so consider the first target instead 
			if (mod->GetNumTargets() && mod->GetTarget(0))
				obj = mod->GetTarget(0);
		}
		else
		{
			const plLayerInterface* li = plLayerInterface::ConvertNoRef(obj);
			const plClothingOutfit* cl = plClothingOutfit::ConvertNoRef(obj);
			if (li || cl)
			{
				// the object is a layer interface or clothing object
				plUoid u = obj->GetKey()->GetUoid();
				if (u.GetLocation().IsReserved())
				{
					// layer interface is associated with an avatar, find which one
					int cloneID = u.GetCloneID();
					int clonePlayerID = u.GetClonePlayerID();
					for(int i = -1; i < RemotePlayerKeys().size(); i++)
					{
						plKey pKey = (i==-1) ? GetLocalPlayerKey() : RemotePlayerKeys()[i];
						if (pKey && pKey->GetUoid().GetCloneID()==cloneID && pKey->GetUoid().GetClonePlayerID()==clonePlayerID)
						{
							obj = plSynchedObject::ConvertNoRef(pKey->ObjectIsLoaded());
							break;
						}
					}
				}
				else
				{
					// layer interface is on a non-avatar object
					netGroup = u.GetLocation();
				}
			}
			else
			{
				const plPhysical* ph = plPhysical::ConvertNoRef(obj);
				if (ph)
				{
					// the object is a physical
					obj = plSynchedObject::ConvertNoRef(ph->GetObjectKey()->ObjectIsLoaded());
				}
			}
		}
	}
	
	if (obj)
		netGroup = obj->GetNetGroup();	
	
	return netGroup;
}

//
// If we don't know for sure if something is locallyOwned, check if it' related to the remote or 
// local avatar.  if not, fallback to join order.
//
int plNetClientMgr::IDeduceLocallyOwned(const plUoid& uoid) const
{
	// check against local/remote avatars
	if (fLocalPlayerKey)
		if (uoid.GetLocation() == fLocalPlayerKey->GetUoid().GetLocation() &&
			uoid.GetClonePlayerID() == fLocalPlayerKey->GetUoid().GetClonePlayerID())
			return plSynchedObject::kYes;	// this object's location is the same as the local player's

	if (!fRemotePlayerKeys.empty())
		if (uoid.GetLocation() == fRemotePlayerKeys.back()->GetUoid().GetLocation() &&
			uoid.GetClonePlayerID() == fRemotePlayerKeys.back()->GetUoid().GetClonePlayerID())
			return plSynchedObject::kNo;	// this object's location is the same as the currently loading remote player's

	return fIsOwner;
}


//
// returns yes/no
// for special cases, like sceneNodes.
//
int plNetClientMgr::IsLocallyOwned(const plUoid& uoid) const
{
	// we're non-networked
	if ( GetFlagsBit(kDisabled))
		return plSynchedObject::kYes;

	plNetGroupId netGroup = uoid.GetLocation();
	int answer=GetNetGroups()->IsGroupLocal(netGroup);
	
	if (answer==-1)
		answer = IDeduceLocallyOwned(uoid);

	hsAssert(answer==plSynchedObject::kYes || answer==plSynchedObject::kNo, "invalid answer to IsLocallyOwned()");
	return answer;
}


//
// check if this object is in a group which is owned (mastered) by this client.
// returns yes/no
//
int plNetClientMgr::IsLocallyOwned(const plSynchedObject* obj) const
{
	// we're non-networked
	if ( GetFlagsBit(kDisabled) )
		return plSynchedObject::kYes;

	if( !obj )
		return plSynchedObject::kNo;

	// object is flagged as local only
	if (!obj->IsNetSynched())
		return plSynchedObject::kYes;

	plNetGroupId netGroup = GetEffectiveNetGroup(obj);
	int answer=GetNetGroups()->IsGroupLocal(netGroup);

	if (answer==-1)		// not sure
		answer = IDeduceLocallyOwned(obj->GetKey()->GetUoid());

	hsAssert(answer==plSynchedObject::kYes || answer==plSynchedObject::kNo, "invalid answer to IsLocallyOwned()");
	return answer;
}

//
// return localPlayer ptr
//
plSynchedObject* plNetClientMgr::GetLocalPlayer(hsBool forceLoad) const
{ 
	if (forceLoad)
		return fLocalPlayerKey ? plSynchedObject::ConvertNoRef(fLocalPlayerKey->GetObjectPtr()) : nil; 
	else
		return fLocalPlayerKey ?
			plSynchedObject::ConvertNoRef(fLocalPlayerKey->ObjectIsLoaded()) : nil; 
}

//
// return a ptr to a remote player
//
plSynchedObject* plNetClientMgr::GetRemotePlayer(int i) const
{ 
	return fRemotePlayerKeys[i] ? plSynchedObject::ConvertNoRef(fRemotePlayerKeys[i]->ObjectIsLoaded()) : nil; 
}

//
// check if a key si a remote player
//
hsBool plNetClientMgr::IsRemotePlayerKey(const plKey pKey, int *idx)
{
	if (pKey)
	{
		plKeyVec::iterator result=std::find(fRemotePlayerKeys.begin(), fRemotePlayerKeys.end(), pKey);
		hsBool found = result!=fRemotePlayerKeys.end();
		if (idx)
			*idx = found ? result-fRemotePlayerKeys.begin() : -1;
		return found;
	}
	return false;
}

//
// add remote char if not already there
//
void plNetClientMgr::AddRemotePlayerKey(plKey pKey)
{
	hsAssert(pKey, "adding nil remote player key");
	if (!IsRemotePlayerKey(pKey))
	{
		fRemotePlayerKeys.push_back(pKey);
		hsAssert(pKey!=fLocalPlayerKey, "setting remote player to the local player?");
	}
}

//
// MsgReceive handler for plasma messages
//
hsBool plNetClientMgr::MsgReceive( plMessage* msg )
{
	if (plNetLinkingMgr::GetInstance()->MsgReceive( msg ))
		return true;

	plEvalMsg* evalMsg = plEvalMsg::ConvertNoRef(msg);
	if (evalMsg)
	{
		IPlaybackMsgs();

		if ( GetFlagsBit( kNeedToSendAgeLoadedMsg ) )
		{
			SetFlagsBit( kNeedToSendAgeLoadedMsg, false );
			plAgeLoader::GetInstance()->NotifyAgeLoaded( true );
		}

		if ( GetFlagsBit( kNeedToSendInitialAgeStateLoadedMsg ) )
		{
			SetFlagsBit(kNeedToSendInitialAgeStateLoadedMsg, false);
			plInitialAgeStateLoadedMsg* m = TRACKED_NEW plInitialAgeStateLoadedMsg;
			m->Send();
		}

		return true;
	}

	plGenRefMsg* ref = plGenRefMsg::ConvertNoRef(msg);
	if (ref)
	{
		if( ref->fType == kVaultImage )
		{
			// Ignore, we just use it for reffing, don't care about the actual pointer
			return true;
		}

		hsAssert(ref->fType==kAgeSDLHook, "unknown ref msg context");
		if (ref->GetContext()==plRefMsg::kOnCreate)
		{
			hsAssert(fAgeSDLObjectKey==nil, "already have a ref to age sdl hook");
			fAgeSDLObjectKey = ref->GetRef()->GetKey();
			char tmp[256];
			DebugMsg("Age SDL hook object created, uoid=%s", fAgeSDLObjectKey->GetUoid().StringIze(tmp));
		}
		else
		{
			fAgeSDLObjectKey=nil;
			DebugMsg("Age SDL hook object destroyed");
		}
		return true;
	}

	if (plNetClientMgrMsg * ncmMsg = plNetClientMgrMsg::ConvertNoRef(msg)) {
		if (ncmMsg->type == plNetClientMgrMsg::kCmdDisableNet) {
			SetFlagsBit(kDisableOnNextUpdate);
			hsRefCnt_SafeUnRef(fDisableMsg);
			fDisableMsg = ncmMsg;
			fDisableMsg->Ref();
		}
		return true;
	}
	
    if (plNetCommAuthMsg * authMsg = plNetCommAuthMsg::ConvertNoRef(msg)) {
        if (IS_NET_ERROR(authMsg->result)) {
			char str[256];
			StrPrintf(str, arrsize(str), "Authentication failed: %S", NetErrorToString(authMsg->result));
			QueueDisableNet(true, str);
            return false;	// @@@ TODO: Handle this failure better
        }

        return true;
    }

    if (plNetCommActivePlayerMsg * activePlrMsg = plNetCommActivePlayerMsg::ConvertNoRef(msg)) {
        if (IS_NET_ERROR(activePlrMsg->result)) {
			char str[256];
			StrPrintf(str, arrsize(str), "SetActivePlayer failed: %S", NetErrorToString(activePlrMsg->result));
			QueueDisableNet(true, str);
            return false;	// @@@ TODO: Handle this failure better.
        }
            
        return true;
    }

	plPlayerPageMsg *playerPageMsg = plPlayerPageMsg::ConvertNoRef(msg);
	if(playerPageMsg)
	{
		IHandlePlayerPageMsg(playerPageMsg);
		return true;	// handled
	}

	plLoadCloneMsg* pCloneMsg = plLoadCloneMsg::ConvertNoRef(msg);
	if(pCloneMsg)
	{
		ILoadClone(pCloneMsg);
		return true;	// handled
	}

	// player is petitioning a CCR
	plCCRPetitionMsg* petMsg=plCCRPetitionMsg::ConvertNoRef(msg);
	if (petMsg)
	{
		ISendCCRPetition(petMsg);
		return true;
	}

	// a remote CCR is turning invisible
	plCCRInvisibleMsg* invisMsg=plCCRInvisibleMsg::ConvertNoRef(msg);
	if (invisMsg)
	{
		LogMsg(kLogDebug, "plNetClientMgr::MsgReceive - Got plCCRInvisibleMsg");
		MakeCCRInvisible(invisMsg->fAvKey, invisMsg->fInvisLevel);
		return true;
	}
	
	plCCRBanLinkingMsg* banLinking = plCCRBanLinkingMsg::ConvertNoRef(msg);
	if (banLinking)
	{
		DebugMsg("Setting BanLinking to %d", banLinking->fBan);
		SetFlagsBit(kBanLinking, banLinking->fBan);
		return true;
	}

	plCCRSilencePlayerMsg* silence = plCCRSilencePlayerMsg::ConvertNoRef(msg);
	if (silence)
	{
		DebugMsg("Setting Silence to %d", silence->fSilence);
		SetFlagsBit(kSilencePlayer, silence->fSilence);
		return true;
	}


	plInitialAgeStateLoadedMsg *stateMsg = plInitialAgeStateLoadedMsg::ConvertNoRef( msg );
	if( stateMsg != nil )
	{
		// done receiving the initial state of the age from the server
		plNetObjectDebugger::GetInstance()->LogMsg("OnServerInitComplete");

		// delete fProgressBar;
		// fProgressBar=nil;

		SetFlagsBit(kLoadingInitialAgeState, false);
		StartLinkInFX();
	}

	plNetVoiceListMsg* voxList = plNetVoiceListMsg::ConvertNoRef(msg);
	if (voxList)
	{
		IHandleNetVoiceListMsg(voxList);
		return true;
	}

	plSynchEnableMsg* synchEnable = plSynchEnableMsg::ConvertNoRef(msg);
	if (synchEnable)
	{
		if (synchEnable->fPush)
		{
			plSynchedObject::PushSynchDisabled(!synchEnable->fEnable);
		}
		else
		{
			plSynchedObject::PopSynchDisabled();
		}
		return true;
	}

	plClientMsg* clientMsg = plClientMsg::ConvertNoRef(msg);
	if (clientMsg && clientMsg->GetClientMsgFlag()==plClientMsg::kInitComplete)
	{
		// add 1 debug object for age sdl
		if (plNetObjectDebugger::GetInstance())
		{
			plNetObjectDebugger::GetInstance()->RemoveDebugObject("AgeSDLHook");	
			plNetObjectDebugger::GetInstance()->AddDebugObject("AgeSDLHook");
		}

		// if we're linking to startup we don't need (or want) a player set
		char ageName[kMaxAgeNameLength];
		StrCopy(ageName, NetCommGetStartupAge()->ageDatasetName, arrsize(ageName));
		if (!StrLen(ageName))
			StrCopy(ageName, "StartUp", arrsize(ageName));
		if (0 == StrCmpI(ageName, "StartUp"))
			NetCommSetActivePlayer(0, nil);

		plAgeLinkStruct link;
		link.GetAgeInfo()->SetAgeFilename(NetCommGetStartupAge()->ageDatasetName);
		link.SetLinkingRules(plNetCommon::LinkingRules::kOriginalBook);
		plNetLinkingMgr::GetInstance()->LinkToAge(&link);

		return true;
	}
	
	return plNetClientApp::MsgReceive(msg);
}

void plNetClientMgr::IncNumInitialSDLStates()
{
	fNumInitialSDLStates++;
	DebugMsg( "Received %d initial SDL states", fNumInitialSDLStates );
	if ( GetFlagsBit( plNetClientApp::kNeedInitialAgeStateCount ) )
	{
		DebugMsg( "Need initial SDL state count" );
		return;
	}

	if ( GetNumInitialSDLStates()>=GetRequiredNumInitialSDLStates() )
	{
		ICheckPendingStateLoad(hsTimer::GetSysSeconds());
		NotifyRcvdAllSDLStates();
	}
}

void plNetClientMgr::NotifyRcvdAllSDLStates() {
	DebugMsg( "Got all initial SDL states" );
	plNetClientMgrMsg * msg = TRACKED_NEW plNetClientMgrMsg();
	msg->type = plNetClientMgrMsg::kNotifyRcvdAllSDLStates;
	msg->SetBCastFlag(plMessage::kBCastByType);
	msg->Send();
}

//
// return true if we should send this msg
//
bool plNetClientMgr::CanSendMsg(plNetMessage * msg)
{
	// TEMP
	if (GetFlagsBit(kSilencePlayer))
	{
		if (plNetMsgVoice::ConvertNoRef(msg))
			return false;

		plNetMsgGameMessage* gm=plNetMsgGameMessage::ConvertNoRef(msg);
		if (gm && gm->StreamInfo()->GetStreamType()==pfKIMsg::Index())
		{
			hsReadOnlyStream stream(gm->StreamInfo()->GetStreamLen(), gm->StreamInfo()->GetStreamBuf());
			pfKIMsg* kiMsg = pfKIMsg::ConvertNoRef(hsgResMgr::ResMgr()->ReadCreatable(&stream));
			if (kiMsg && kiMsg->GetCommand()==pfKIMsg::kHACKChatMsg)
			{
				delete kiMsg;
				return false;
			}
			delete kiMsg;
		}
	}

	return true;
}

//
// Return the net client (account) name of the player whose avatar
// key is provided.  If avKey is nil, returns local client name.
//
const char* plNetClientMgr::GetPlayerName(const plKey avKey) const
{
	// local case
	if (!avKey || avKey==GetLocalPlayerKey())
		return NetCommGetPlayer()->playerNameAnsi;
	
	plNetTransportMember* mbr=TransportMgr().GetMember(TransportMgr().FindMember(avKey));
	return mbr ? mbr->GetPlayerName() : nil;
}

const char * plNetClientMgr::GetPlayerNameById (unsigned playerId) const {
	// local case
	if (NetCommGetPlayer()->playerInt == playerId)
		return NetCommGetPlayer()->playerNameAnsi;
		
	plNetTransportMember * mbr = TransportMgr().GetMember(TransportMgr().FindMember(playerId));
	return mbr ? mbr->GetPlayerName() : nil;
}

unsigned plNetClientMgr::GetPlayerIdByName (const char name[]) const {
	// local case
	if (0 == StrCmp(name, NetCommGetPlayer()->playerNameAnsi, (unsigned)-1))
		NetCommGetPlayer()->playerInt;
	
	unsigned n = TransportMgr().GetNumMembers();
	for (unsigned i = 0; i < n; ++i)
		if (plNetTransportMember * member = TransportMgr().GetMember(i))
			if (0 == StrCmp(name, member->GetPlayerName(), (unsigned)-1))
				return member->GetPlayerID();
	return 0;
}

UInt32 plNetClientMgr::GetPlayerID() const
{
	return NetCommGetPlayer()->playerInt;
}

//
// return true if the age is the set of local ages
//
static const char* gLocalAges[]={"GUI", "AvatarCustomization", ""};
static bool IsLocalAge(const char* ageName)
{
	int i=0;
	while(strlen(gLocalAges[i]))
	{
		if (!stricmp(gLocalAges[i], ageName))
			return true;
		i++;
	}
	return false;
}

//
// is the object in a local age?
//
bool plNetClientMgr::ObjectInLocalAge(const plSynchedObject* obj) const
{
	plLocation loc = obj->GetKey()->GetUoid().GetLocation();
	const plPageInfo* pageInfo = plKeyFinder::Instance().GetLocationInfo(loc);
	bool ret= IsLocalAge(pageInfo->GetAge());
	return ret;
}

//
// the next age we are going to
//
const char* plNetClientMgr::GetNextAgeFilename() 
{ 
	// set when we start linking to an age.
	plNetLinkingMgr * lm = plNetLinkingMgr::GetInstance();
	return lm->GetAgeLink()->GetAgeInfo()->GetAgeFilename();
}

//
// a remote ccr is turning [in]visible
//
void plNetClientMgr::MakeCCRInvisible(plKey avKey, int level)
{
	if (!avKey)
	{
		ErrorMsg("Failed to make remote CCR Invisible, nil avatar key");
		return;
	}
		
	if (level<0)
	{
		ErrorMsg("Failed to make remote CCR Invisible, negative level");
		return;
	}
	
	if (!avKey->ObjectIsLoaded() || !avKey->ObjectIsLoaded()->IsFinal())
	{
		hsAssert(false, "avatar not loaded or final");
		return;
	}
	
	plAvatarStealthModeMsg *msg = TRACKED_NEW plAvatarStealthModeMsg();
	msg->SetSender(avKey);
	msg->fLevel = level;

	if (GetCCRLevel()<level)	// I'm a lower level than him, so he's invisible to me
		msg->fMode = plAvatarStealthModeMsg::kStealthCloaked;		
	else
	{
		// he's visible to me
		if (AmCCR() && level > 0)
			msg->fMode = plAvatarStealthModeMsg::kStealthCloakedButSeen;	// draw as semi-invis
		else
			msg->fMode = plAvatarStealthModeMsg::kStealthVisible;
	}

	DebugMsg("Handled MakeCCRInvisible - sending stealth msg");;

	// This fxn is called when avatar SDL state is received from the server,
	// so this msg will inherit the 'non-local' status of the parent sdl msg.
	// That means that the avatar linkSound won't receive it, since the linkSound
	// is set as localOnly.
	// So, terminate the remote cascade and start a new (local) cascade.
	msg->SetBCastFlag(plMessage::kNetStartCascade);	
	
	msg->Send();
}

void plNetClientMgr::QueueDisableNet (bool showDlg, const char str[]) {

    plNetClientMgrMsg * msg = NEWZERO(plNetClientMgrMsg);
    msg->type	= plNetClientMgrMsg::kCmdDisableNet;
    msg->yes	= showDlg;
    if (str)
		StrCopy(msg->str, str, arrsize(msg->str));

#if 0
	msg->Send(GetKey());
#else
	msg->Ref();
	fDisableMsg = msg;
	IDisableNet();
#endif;
}

void plNetClientMgr::IDisableNet () {
	ASSERT(fDisableMsg);
	
	if (!GetFlagsBit(kDisabled)) {
		SetFlagsBit(kDisabled);
		// cause subsequent net operations to fail immediately, but don't block
		// waiting for net subsystem to shutdown (we'll do that later)
		NetCommEnableNet(false, false);

		// display a msg to the player
		if ( fDisableMsg->yes )
		{
			if (!GetFlagsBit(plNetClientApp::kPlayingGame))
			{
				// KI may not be loaded
				char title[256];
				StrPrintf(title, arrsize(title), "%S Error", ProductCoreName());
				hsMessageBox(fDisableMsg->str, title, hsMessageBoxNormal, hsMessageBoxIconError );
				plClientMsg *quitMsg = NEW(plClientMsg)(plClientMsg::kQuit);
				quitMsg->Send(hsgResMgr::ResMgr()->FindKey(kClient_KEY));
			}
			else
			{
				pfKIMsg *msg = TRACKED_NEW pfKIMsg(pfKIMsg::kKIOKDialog);
				msg->SetString(fDisableMsg->str);
				msg->Send();
			}
		}
	}

	hsRefCnt_SafeUnRef(fDisableMsg);
	fDisableMsg = nil;
}

bool plNetClientMgr::IHandlePlayerPageMsg(plPlayerPageMsg *playerMsg)
{
	bool result = false;
	plKey playerKey = playerMsg->fPlayer;
	int idx;

	if(playerMsg->fUnload)
	{
		if (GetLocalPlayerKey() == playerKey)
		{
			fLocalPlayerKey = nil;
			DebugMsg("Net: Unloading local player %s", playerKey->GetName());
			
			// notify server - NOTE: he might not still be around to get this...
			plNetMsgPlayerPage npp (playerKey->GetUoid(), playerMsg->fUnload);
			npp.SetNetProtocol(kNetProtocolCli2Game);
			SendMsg(&npp);
		}
		else if (IsRemotePlayerKey(playerKey, &idx))
		{
			fRemotePlayerKeys.erase(fRemotePlayerKeys.begin()+idx);	// remove key from list
			DebugMsg("Net: Unloading remote player %s", playerKey->GetName());
		}
	}
	else
	{
		plSceneObject *playerSO = plSceneObject::ConvertNoRef(playerKey->ObjectIsLoaded());
		if (!playerSO)
		{
			hsStatusMessageF("Ignoring player page message for non-existant player.");
		}
		else
		if(playerMsg->fPlayer)
		{
			if (playerMsg->fLocallyOriginated)
			{
				hsAssert(!GetLocalPlayerKey() ||
					GetLocalPlayerKey() == playerKey,
					"Different local player already loaded");

				hsLogEntry(DebugMsg("Adding LOCAL player %s\n", playerKey->GetName()));
				playerSO->SetNetGroupConstant(plNetGroup::kNetGroupLocalPlayer);
				
				// don't save avatar state permanently on server
				playerSO->SetSynchFlagsBit(plSynchedObject::kAllStateIsVolatile);				
				const plCoordinateInterface* co = playerSO->GetCoordinateInterface();
				if (co)
				{
					int i;
					for(i=0;i<co->GetNumChildren();i++)
					{
						if (co->GetChild(i) && co->GetChild(i)->GetOwner())
								const_cast<plSceneObject*>(co->GetChild(i)->GetOwner())->SetSynchFlagsBit(plSynchedObject::kAllStateIsVolatile);
					}
				}

				// notify server
				plNetMsgPlayerPage npp (playerKey->GetUoid(), playerMsg->fUnload);
				npp.SetNetProtocol(kNetProtocolCli2Game);
				SendMsg(&npp);
			}
			else
			{
				hsLogEntry(DebugMsg("Adding REMOTE player %s\n", playerKey->GetName()));
				playerSO->SetNetGroupConstant(plNetGroup::kNetGroupRemotePlayer);
				idx=fTransport.FindMember(playerMsg->fClientID);
				if( idx != -1 )
				{
					hsAssert(playerKey, "NIL KEY?");
					hsAssert(playerKey->GetName(), "UNNAMED KEY");
					fTransport.GetMember(idx)->SetAvatarKey(playerKey);
				}
				else
				{
					hsLogEntry(DebugMsg("Ignoring player page msg (player not found in member list) : %s\n", playerKey->GetName()));
				}
			}

			hsAssert(IFindModifier(playerSO, CLASS_INDEX_SCOPED(plAvatarSDLModifier)), "avatar missing avatar SDL modifier");
			hsAssert(IFindModifier(playerSO, CLASS_INDEX_SCOPED(plClothingSDLModifier)), "avatar missing clothing SDL modifier");
			hsAssert(IFindModifier(playerSO, CLASS_INDEX_SCOPED(plAGMasterSDLModifier)), "avatar missing AGMaster SDL modifier");
			result = true;
		}
	}
	return result;
}

// for debugging purposes
bool plNetClientMgr::IFindModifier(plSynchedObject* obj, Int16 classIdx)
{
	plLayerAnimation* layer = plLayerAnimation::ConvertNoRef(obj);
	if (layer)
		return (layer->GetSDLModifier() != nil);

	plResponderModifier* resp = plResponderModifier::ConvertNoRef(obj);
	if (resp)
		return (resp->GetSDLModifier() != nil);

	int cnt=0;
	plSceneObject* sceneObj=plSceneObject::ConvertNoRef(obj);
	if (sceneObj)
	{
		int i;
		for(i=0; i<sceneObj->GetNumModifiers(); i++)
			if (sceneObj->GetModifier(i) && sceneObj->GetModifier(i)->ClassIndex()==classIdx)
				cnt++;
	}

	hsAssert(cnt<2, xtl::format("Object %s has multiple SDL modifiers of the same kind (%s)?", 
		obj->GetKeyName(), plFactory::GetNameOfClass(classIdx)).c_str());
	return cnt==0 ? false : true;
}

plUoid plNetClientMgr::GetAgeSDLObjectUoid(const char* ageName) const
{
	hsAssert(ageName, "nil ageName");

	// if age sdl hook is loaded
	if (fAgeSDLObjectKey)
		return fAgeSDLObjectKey->GetUoid();

	// if age is loaded
	plLocation loc = plKeyFinder::Instance().FindLocation(ageName,plAgeDescription::GetCommonPage(plAgeDescription::kGlobal));
	if (!loc.IsValid())
	{
		// check current age des
		if (plAgeLoader::GetInstance()->GetCurrAgeDesc().GetAgeName() &&
			!strcmp(plAgeLoader::GetInstance()->GetCurrAgeDesc().GetAgeName(), ageName))
			loc=plAgeLoader::GetInstance()->GetCurrAgeDesc().CalcPageLocation("BuiltIn");

		if (!loc.IsValid())
		{
			// try to load age desc
			hsStream* stream=plAgeLoader::GetAgeDescFileStream(ageName);
			if (stream)
			{
				plAgeDescription ad;
				ad.Read(stream);
				loc=ad.CalcPageLocation("BuiltIn");
				stream->Close();
			}			
			delete stream;
		}
	}

	return plUoid(loc, plSceneObject::Index(), plSDL::kAgeSDLObjectName);
}

//
// Add a state update to the pending queue
//
void plNetClientMgr::AddPendingLoad(PendingLoad *pl) 
{ 
	pl->fQueuedTime = hsTimer::GetSeconds();	// timestamp

	// find corresponding key
	pl->fKey = hsgResMgr::ResMgr()->FindKey(pl->fUoid);

	// check for age SDL state
	char tmp[256];
	if (pl->fUoid.GetObjectName() && !strcmp(pl->fUoid.GetObjectName(), plSDL::kAgeSDLObjectName))
	{
		DebugMsg("Recv SDL state for age hook object, uoid=%s", pl->fUoid.StringIze(tmp));
		if (!pl->fKey)
			WarningMsg("Can't find age hook object, nil key!");
		else
			DebugMsg("Found age hook object");
	}

	// check if object is ready
	if (pl->fKey)
	{
		if (!pl->fKey->ObjectIsLoaded())
		{
			WarningMsg("Object %s not loaded, withholding SDL state", 
				pl->fKey->GetUoid().StringIze(tmp));
		}
		else if (!pl->fKey->ObjectIsLoaded()->IsFinal())
		{
			WarningMsg("Object %s is not FINAL, withholding SDL state", 
				pl->fKey->GetUoid().StringIze(tmp));			
		}
	}

	// add entry
	fPendingLoads.push_back(pl);	
}

void plNetClientMgr::AddPendingPagingRoomMsg( plNetMsgPagingRoom * msg )
{
	fPendingPagingRoomMsgs.push_back( msg );
}

void plNetClientMgr::MaybeSendPendingPagingRoomMsgs()
{
	if ( GetFlagsBit( kPlayingGame ) )
		SendPendingPagingRoomMsgs();
}

void plNetClientMgr::SendPendingPagingRoomMsgs()
{
	for ( int i=0; i<fPendingPagingRoomMsgs.size(); i++ )
		SendMsg( fPendingPagingRoomMsgs[i] );
	ClearPendingPagingRoomMsgs();
}

void plNetClientMgr::ClearPendingPagingRoomMsgs()
{
	std::for_each( fPendingPagingRoomMsgs.begin(), fPendingPagingRoomMsgs.end(), xtl::delete_ptr() );
	fPendingPagingRoomMsgs.clear();
}


bool plNetClientMgr::DebugMsgV(const char* fmt, va_list args) const 
{
	LogMsgV(kLogDebug, fmt, args);
	return true;
}

bool plNetClientMgr::ErrorMsgV(const char* fmt, va_list args) const 
{
	LogMsgV(kLogError, fmt, args);
	return true;
}

bool plNetClientMgr::WarningMsgV(const char* fmt, va_list args) const 
{
	LogMsgV(kLogError, fmt, args);
	return true;
}

bool plNetClientMgr::AppMsgV(const char* fmt, va_list args) const 
{
	LogMsgV(kLogPerf, fmt, args);
	return true;
}

bool plNetClientMgr::IsObjectOwner()
{
	return fIsOwner;
}

void plNetClientMgr::SetObjectOwner(bool own)
{
	fIsOwner = own;
}
