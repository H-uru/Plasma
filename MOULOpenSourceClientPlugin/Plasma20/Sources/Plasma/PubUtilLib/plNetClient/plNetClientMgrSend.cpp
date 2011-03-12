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
#include "hsTimer.h"
#include "hsResMgr.h"
#include "plNetClientMgr.h"
#include "plCreatableIndex.h"	
#include "plNetObjectDebugger.h"
#include "plNetClientMsgScreener.h"

#include "../pnNetCommon/plSynchedObject.h"
#include "../pnNetCommon/plSDLTypes.h"
#include "../pnMessage/plCameraMsg.h"

#include "../plNetClientRecorder/plNetClientRecorder.h"
#include "../plMessage/plLoadCloneMsg.h"
#include "../plMessage/plLoadAvatarMsg.h"
#include "../plAvatar/plAvatarClothing.h"
#include "../plAvatar/plArmatureMod.h"
#include "../plAvatar/plAvatarMgr.h"
#include "../plNetMessage/plNetMessage.h"
#include "../plMessage/plCCRMsg.h"
#include "../plVault/plVault.h"
#include "../plContainer/plConfigInfo.h"
#include "../plDrawable/plMorphSequence.h"
#include "../plParticleSystem/plParticleSystem.h"
#include "../plParticleSystem/plParticleSDLMod.h"
#include "../plResMgr/plLocalization.h"

#include "../../FeatureLib/pfMessage/pfKIMsg.h"	// TMP

#include "../plNetGameLib/plNetGameLib.h"
#include "../plSDL/plSDL.h"

//
// request members list from server
//
int plNetClientMgr::ISendMembersListRequest()
{
	plNetMsgMembersListReq	msg;
	msg.SetNetProtocol(kNetProtocolCli2Game);
	return SendMsg(&msg);
}

//
// reset paged in rooms list on server
//
int plNetClientMgr::ISendRoomsReset()
{
	plNetMsgPagingRoom msg;
	msg.SetPageFlags(plNetMsgPagingRoom::kResetList);
	msg.SetNetProtocol(kNetProtocolCli2Game);
	return SendMsg(&msg);
}

//
// Make sure all dirty objects save their state.
// Mark those objects as clean and clear the dirty list.
//
int plNetClientMgr::ISendDirtyState(double secs)
{
	std::vector<plSynchedObject::StateDefn> carryOvers;

	Int32 num=plSynchedObject::GetNumDirtyStates();
#if 0
	if (num)
	{
		DebugMsg("%d dirty sdl state msgs queued, t=%f", num, secs);
	}
#endif
	Int32 i;
	for(i=0;i<num;i++)
	{
		plSynchedObject::StateDefn* state=plSynchedObject::GetDirtyState(i);
		
		plSynchedObject* obj=state->GetObject();
		if (!obj)
			continue;	// could add to carryOvers

		if (!(state->fSendFlags & plSynchedObject::kSkipLocalOwnershipCheck))
		{
			int localOwned=obj->IsLocallyOwned();
			if (localOwned==plSynchedObject::kNo)
			{
				DebugMsg("Late rejection of queued SDL state, obj %s, sdl %s",
					state->fObjKey->GetName(), state->fSDLName.c_str());
				continue;
			}
		}

		obj->CallDirtyNotifiers();
		obj->SendSDLStateMsg(state->fSDLName.c_str(), state->fSendFlags);		
	}

	plSynchedObject::ClearDirtyState(carryOvers);

	return hsOK;
}

//
// Given a plasma petition msg, send a petition text node to the vault
// vault will detect and fwd to CCR system.
//
void plNetClientMgr::ISendCCRPetition(plCCRPetitionMsg* petMsg)
{
	// petition msg info
	UInt8 type = petMsg->GetType();
	const char* title = petMsg->GetTitle();
	const char* note = petMsg->GetNote();

	std::string work = note;
	std::replace( work.begin(), work.end(), '\n', '\t' );
	note = work.c_str();

	// stuff petition info fields into a config info object
	plConfigInfo info;
	info.AddValue( "Petition", "Type", type );
	info.AddValue( "Petition", "Content", note );
	info.AddValue( "Petition", "Title", title );
	info.AddValue( "Petition", "Language", plLocalization::GetLanguageName( plLocalization::GetLanguage() ) );
	info.AddValue( "Petition", "AcctName", NetCommGetAccount()->accountNameAnsi );
	char buffy[20];
	sprintf( buffy, "%lu", GetPlayerID() );
	info.AddValue( "Petition", "PlayerID", buffy );
	info.AddValue( "Petition", "PlayerName", GetPlayerName() );

	// write config info formatted like an ini file to a buffer
	hsRAMStream ram;
	info.WriteTo( &plIniStreamConfigSource( &ram ) );
	int size = ram.GetPosition();
	ram.Rewind();
	std::string buf;
	buf.resize( size );
	ram.CopyToMem( (void*)buf.data() );
	
	wchar * wStr = StrDupToUnicode(buf.c_str());
	NetCliAuthSendCCRPetition(wStr);
	FREE(wStr);
}

//
// send a msg to reset the camera in a new age
//
void plNetClientMgr::ISendCameraReset(hsBool bEnteringAge)
{	
	plCameraMsg* pCamMsg = TRACKED_NEW plCameraMsg;
	if (bEnteringAge)
		pCamMsg->SetCmd(plCameraMsg::kResetOnEnter);
	else
		pCamMsg->SetCmd(plCameraMsg::kResetOnExit);
	pCamMsg->SetBCastFlag(plMessage::kBCastByExactType, false);		
	plUoid U2(kVirtualCamera1_KEY);
	plKey pCamKey = hsgResMgr::ResMgr()->FindKey(U2);
	if (pCamKey)
	pCamMsg->AddReceiver(pCamKey);
	pCamMsg->Send();
}

//
// When we link in to a new age, we need to send our avatar state up to the gameserver
//
void plNetClientMgr::SendLocalPlayerAvatarCustomizations()
{
	plSynchEnabler ps(true);	// make sure synching is enabled, since this happens during load

	const plArmatureMod * avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
	hsAssert(avMod,"Failed to get local avatar armature modifier.");
	avMod->GetClothingOutfit()->DirtySynchState(kSDLClothing, plSynchedObject::kBCastToClients | plSynchedObject::kForceFullSend);

	plSceneObject* pObj = (const_cast<plArmatureMod*>(avMod))->GetFollowerParticleSystemSO();
	if (pObj)
	{
		const plParticleSystem* sys = plParticleSystem::ConvertNoRef(pObj->GetModifierByType(plParticleSystem::Index()));
		if (sys)
			(const_cast<plParticleSystem*>(sys))->GetSDLMod()->SendState(plSynchedObject::kBCastToClients | plSynchedObject::kForceFullSend);

	}
	// may want to do this all the time, but for now stealthmode is the only extra avatar state we care about
	// don't bcast this to other clients, the invis level is contained in the linkIn msg which will synch other clients
	if (avMod->IsInStealthMode() && avMod->GetTarget(0))
		avMod->GetTarget(0)->DirtySynchState(kSDLAvatar, plSynchedObject::kForceFullSend);

	hsTArray<const plMorphSequence*> morphs;
	plMorphSequence::FindMorphMods(avMod->GetTarget(0), morphs);
	int i;
	for (i = 0; i < morphs.GetCount(); i++)
		if (morphs[i]->GetTarget(0))
			morphs[i]->GetTarget(0)->DirtySynchState(kSDLMorphSequence, plSynchedObject::kBCastToClients);

}

//
// Called to send a plasma msg out over the network.  Called by the dispatcher.
// return hsOK if ok
//
int plNetClientMgr::ISendGameMessage(plMessage* msg)
{
	if (GetFlagsBit(kDisabled))
		return hsOK;

	static plNetClientMsgScreener screener;		// make static so that there's only 1 log per session
	if (!screener.AllowMessage(msg))
	{
		if (GetFlagsBit(kScreenMessages))
			return hsOK;		// filter out illegal messages
	}
	
	// TEMP
	if (GetFlagsBit(kSilencePlayer))
	{
		pfKIMsg* kiMsg = pfKIMsg::ConvertNoRef(msg);
		if (kiMsg && kiMsg->GetCommand()==pfKIMsg::kHACKChatMsg)
			return hsOK;
	}

	plNetPlayerIDList* dstIDs = msg->GetNetReceivers();

#ifdef HS_DEBUGGING
	if ( dstIDs )
	{
		DebugMsg( "Preparing to send %s to specific players.", msg->ClassName() );
	}
#endif

	// get sender object
	plSynchedObject* synchedObj = msg->GetSender() ? plSynchedObject::ConvertNoRef(msg->GetSender()->ObjectIsLoaded()) : nil;

	// if sender is flagged as localOnly, he shouldn't talk to the network
	if (synchedObj && !synchedObj->IsNetSynched() )
		return hsOK;

	// choose appropriate type of net game msg wrapper
	plNetMsgGameMessage* netMsgWrap=nil;
	plLoadCloneMsg* loadClone = plLoadCloneMsg::ConvertNoRef(msg);
	if (loadClone)
	{
		plLoadAvatarMsg* lam=plLoadAvatarMsg::ConvertNoRef(msg);

		netMsgWrap = TRACKED_NEW plNetMsgLoadClone;
		plNetMsgLoadClone* netLoadClone=plNetMsgLoadClone::ConvertNoRef(netMsgWrap);
		
		netLoadClone->SetIsPlayer(lam && lam->GetIsPlayer());
		netLoadClone->SetIsLoading(loadClone->GetIsLoading()!=0);
		netLoadClone->ObjectInfo()->SetFromKey(loadClone->GetCloneKey());
	}
	else
	if (dstIDs)
	{
		netMsgWrap = TRACKED_NEW plNetMsgGameMessageDirected;
		int i;
		for(i=0;i<dstIDs->size();i++)
		{
			UInt32 playerID = (*dstIDs)[i];
			if (playerID == NetCommGetPlayer()->playerInt)
				continue;
			hsLogEntry( DebugMsg( "\tAdding receiver: %lu" , playerID ) );
			((plNetMsgGameMessageDirected*)netMsgWrap)->Receivers()->AddReceiverPlayerID( playerID );
		}
	}
	else
		netMsgWrap = TRACKED_NEW plNetMsgGameMessage;

	// check delivery timestamp
	if (msg->GetTimeStamp()<=hsTimer::GetSysSeconds())
		msg->SetTimeStamp(0);	
	else
		netMsgWrap->GetDeliveryTime().SetFromGameTime(msg->GetTimeStamp(), hsTimer::GetSysSeconds());	

	// write message (and label) to ram stream
	hsRAMStream stream;
	hsgResMgr::ResMgr()->WriteCreatable(&stream, msg);

	// put stream in net msg wrapper
	netMsgWrap->StreamInfo()->CopyStream(&stream);
	
	// hsLogEntry( DebugMsg(plDispatchLog::GetInstance()->MakeMsgInfoString(msg, "\tActionMsg:",0)) );

	// check if this msg uses direct communication (sent to specific rcvrs)
	// if so the server can filter it
	hsBool bCast = msg->HasBCastFlag(plMessage::kBCastByExactType) ||
		msg->HasBCastFlag(plMessage::kBCastByType);
	hsBool directCom = msg->GetNumReceivers()>0;
	if( directCom )
	{
		// It's direct if we have receivers AND any of them are in non-virtual locations
		int		i;
		for( i = 0, directCom = false; i < msg->GetNumReceivers(); i++ )
		{
			if( !msg->GetReceiver( i )->GetUoid().GetLocation().IsVirtual() &&
				!msg->GetReceiver( i )->GetUoid().GetLocation().IsReserved()
				// && !IsBuiltIn
				)
			{
				directCom = true;
				break;
			}
		}
		if (!directCom)
			bCast = true;
	}
	if (!directCom && !bCast && !dstIDs)
		WarningMsg("Msg %s has no rcvrs or bcast instructions?", msg->ClassName());

	hsAssert(!(directCom && bCast), "msg has both rcvrs and bcast instructions, rcvrs ignored");
	if (directCom && !bCast)
	{
		netMsgWrap->SetBit(plNetMessage::kHasGameMsgRcvrs);	// for quick server filtering
		netMsgWrap->StreamInfo()->SetCompressionType(plNetMessage::kCompressionDont);
	}

	//
	// check for net propagated plasma msgs which should be filtered by relevance regions.
	// currently only avatar control messages.
	// 
	if (msg->HasBCastFlag(plMessage::kNetUseRelevanceRegions))
	{
		netMsgWrap->SetBit(plNetMessage::kUseRelevanceRegions);
	}

	//
	// CCRs can route a plMessage to all online players.
	//
	hsBool ccrSendToAllPlayers = false;
#ifndef PLASMA_EXTERNAL_RELEASE
	if ( AmCCR() )
	{
		ccrSendToAllPlayers = msg->HasBCastFlag( plMessage::kCCRSendToAllPlayers );
		if ( ccrSendToAllPlayers )
			netMsgWrap->SetBit( plNetMessage::kRouteToAllPlayers );
	}
#endif

	//
	// check for inter-age routing. if set, online rcvrs not in current age will receive
	// this msg courtesy of pls routing.
	//
	if ( !ccrSendToAllPlayers )
	{
		hsBool allowInterAge = msg->HasBCastFlag( plMessage::kNetAllowInterAge );
		if ( allowInterAge )
			netMsgWrap->SetBit(plNetMessage::kInterAgeRouting);
	}

	// check for reliable send
	if (msg->HasBCastFlag(plMessage::kNetSendUnreliable) && 
		!(synchedObj && (synchedObj->GetSynchFlags() & plSynchedObject::kSendReliably)) )
		netMsgWrap->SetBit(plNetMessage::kNeedsReliableSend, 0);	// clear reliable net send bit
	
#ifdef HS_DEBUGGING
	Int16 type=*(Int16*)netMsgWrap->StreamInfo()->GetStreamBuf();
	hsAssert(type>=0 && type<plCreatableIndex::plNumClassIndices, "garbage type out");
#endif
								
	netMsgWrap->SetPlayerID(GetPlayerID());	
	netMsgWrap->SetNetProtocol(kNetProtocolCli2Game);
	int ret = SendMsg(netMsgWrap);

	if (plNetObjectDebugger::GetInstance()->IsDebugObject(msg->GetSender() ? msg->GetSender()->ObjectIsLoaded() : nil))
	{
	#if 0
		hsLogEntry(plNetObjectDebugger::GetInstance()->LogMsg(
			xtl::format("<SND> object:%s, rcvr %s %s",
			msg->GetSender(), 
			msg->GetNumReceivers() ? msg->GetReceiver(0)->GetName() : "?", 
			netMsgWrap->AsStdString().c_str()).c_str()));
	#endif
	}

	delete netMsgWrap;	
	return ret;
}

//
// Send a net msg.  Delivers to transport mgr who sends p2p or to server
//
int plNetClientMgr::SendMsg(plNetMessage* msg)
{
	if (GetFlagsBit(kDisabled))
		return hsOK;

	if (!CanSendMsg(msg))
		return hsOK;

	// If we're recording messages, set an identifying flag and echo the message back to ourselves
	if (fMsgRecorder && fMsgRecorder->IsRecordableMsg(msg))
	{
		msg->SetBit(plNetMessage::kEchoBackToSender, true);
	}
	
	msg->SetTimeSent(plUnifiedTime::GetCurrentTime());
	int channel = IPrepMsg(msg);
	
//	hsLogEntry( DebugMsg( "<SND> %s %s", msg->ClassName(), msg->AsStdString().c_str()) );
	
	int ret=fTransport.SendMsg(channel, msg);

	// Debug
	if (plNetMsgVoice::ConvertNoRef(msg))
		SetFlagsBit(kSendingVoice);
	if (plNetMsgGameMessage::ConvertNoRef(msg))
		SetFlagsBit(kSendingActions);
	
	plCheckNetMgrResult_ValReturn(ret,(char*)xtl::format("Failed to send %s, NC ret=%d",
		msg->ClassName(), ret).c_str());

	return ret;
}


void plNetClientMgr::StoreSDLState(const plStateDataRecord* sdRec, const plUoid& uoid, 
									UInt32 sendFlags, UInt32 writeOptions)
{
	// send to server
	plNetMsgSDLState* msg = sdRec->PrepNetMsg(0, writeOptions);
	msg->SetNetProtocol(kNetProtocolCli2Game);
	msg->ObjectInfo()->SetUoid(uoid);

	if (sendFlags & plSynchedObject::kNewState)
		msg->SetBit(plNetMessage::kNewSDLState);

	if (sendFlags & plSynchedObject::kUseRelevanceRegions)
		msg->SetBit(plNetMessage::kUseRelevanceRegions);

	if (sendFlags & plSynchedObject::kDontPersistOnServer)
		msg->SetPersistOnServer(false);

	if (sendFlags & plSynchedObject::kIsAvatarState)
		msg->SetIsAvatarState(true);

	bool broadcast = (sendFlags & plSynchedObject::kBCastToClients) != 0;
	if (broadcast && plNetClientApp::GetInstance())
	{
		msg->SetPlayerID(plNetClientApp::GetInstance()->GetPlayerID());
	}

	SendMsg(msg);
	DEL(msg);
}
