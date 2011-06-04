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
#include "plNetClientMgr.h"
#include "plNetClientMsgHandler.h"
#include "hsResMgr.h"
#include "plCreatableIndex.h"
#include "plgDispatch.h"
#include "plNetLinkingMgr.h"
#include "plCCRMgrBase.h"

#include "../pnKeyedObject/plKey.h"
#include "../pnKeyedObject/plFixedKey.h"
#include "../pnKeyedObject/hsKeyedObject.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plObjInterface.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../pnMessage/plObjRefMsg.h"
#include "../pnMessage/plNodeRefMsg.h"
#include "../pnMessage/plClientMsg.h"
//#include "../pnMessage/plWarpMsg.h"
#include "../pnMessage/plTimeMsg.h"
#include "../pnMessage/plCameraMsg.h"
#include "../pnMessage/plPlayerPageMsg.h"
#include "../pnFactory/plCreator.h"
#include "../pnSceneObject/plAudioInterface.h"
#include "../pnNetCommon/plSDLTypes.h"

#include "../plAudible/plWinAudible.h"
#include "../plAvatar/plAvatarMgr.h"
#include "../plNetTransport/plNetTransportMember.h"
#include "../plMessage/plMemberUpdateMsg.h"
#include "../plMessage/plNetOwnershipMsg.h"
#include "../plMessage/plCCRMsg.h"
#include "../plVault/plVault.h"
#include "../plSDL/plSDL.h"
#include "../plNetCommon/plNetCommonConstants.h"
#include "../plNetMessage/plNetMessage.h"
#include "../plNetMessage/plNetCommonMessage.h"

#include "../../FeatureLib/pfMessage/pfKIMsg.h"		// Should be moved to PubUtil level

////////////////////////////////////////////////////////////////////////

plNetClientMsgHandler::plNetClientMsgHandler(plNetClientMgr * mgr)
{
	SetNetApp(mgr);
}

plNetClientMsgHandler::~plNetClientMsgHandler()
{
}

plNetClientMgr * plNetClientMsgHandler::IGetNetClientMgr()
{
	return plNetClientMgr::ConvertNoRef(GetNetApp());
}

int plNetClientMsgHandler::PeekMsg(plNetMessage * netMsg)
{
	plNetClientMgr * nc = IGetNetClientMgr();
	int cnt = -1;
	if (netMsg->GetNetCoreMsg())	// && !netMsg->Peeked())	// not needed
	{
		cnt = netMsg->PeekBuffer(netMsg->GetNetCoreMsg()->GetData(), netMsg->GetNetCoreMsg()->GetLen());
		hsAssert(cnt,"0 length message");
	}
	return cnt;
}

void plNetClientMsgHandler::IFillInTransportMember(const plNetMsgMemberInfoHelper* mbi, plNetTransportMember* mbr)
{
	const plNetClientMgr* nc=IGetNetClientMgr();
	UInt16 port = mbi->GetClientGuid()->GetSrcPort();
	UInt32 addr = mbi->GetClientGuid()->GetSrcAddr();		
	UInt32 flags = mbi->GetFlags();
	UInt32 plrID = mbi->GetClientGuid()->GetPlayerID();
	plUoid avUoid = mbi->GetAvatarUoid();
	plKey avKey=hsgResMgr::ResMgr()->FindKey(avUoid);

	mbr->SetPlayerName(mbi->GetClientGuid()->GetPlayerName());
	mbr->SetFlags(flags);
	mbr->SetPlayerID(plrID);
	mbr->SetCCRLevel(mbi->GetClientGuid()->GetCCRLevel());
	if (avKey)
		mbr->SetAvatarKey(avKey);
}

int plNetClientMsgHandler::ReceiveMsg(plNetMessage *& netMsg)
{
#ifdef HS_DEBUGGING
	//plNetClientMgr::GetInstance()->DebugMsg("<RCV> %s", netMsg->ClassName());
#endif

	plNetClientMgr::GetInstance()->UpdateServerTimeOffset(netMsg);
	
	switch(netMsg->ClassIndex())
	{
		default:
			plNetClientMgr::GetInstance()->ErrorMsg( "Unknown msg: %s", netMsg->ClassName() );
			return hsFail;

		MSG_HANDLER_CASE(plNetMsgTerminated)
		MSG_HANDLER_CASE(plNetMsgGroupOwner)

		case CLASS_INDEX_SCOPED(plNetMsgSDLStateBCast):
			MSG_HANDLER_CASE(plNetMsgSDLState)
			
		case CLASS_INDEX_SCOPED(plNetMsgGameMessageDirected):
		case CLASS_INDEX_SCOPED(plNetMsgLoadClone):
			MSG_HANDLER_CASE(plNetMsgGameMessage)

		MSG_HANDLER_CASE(plNetMsgVoice)
		MSG_HANDLER_CASE(plNetMsgMembersList)
		MSG_HANDLER_CASE(plNetMsgMemberUpdate)
		MSG_HANDLER_CASE(plNetMsgListenListUpdate)
		MSG_HANDLER_CASE(plNetMsgInitialAgeStateSent)
	}
}

////////////////////////////////////////////////////////////////////

MSG_HANDLER_DEFN(plNetClientMsgHandler,plNetMsgTerminated)
{
	return hsOK;
}

MSG_HANDLER_DEFN(plNetClientMsgHandler,plNetMsgGroupOwner)
{
	plNetClientMgr* nc = IGetNetClientMgr();
	plNetMsgGroupOwner* m = plNetMsgGroupOwner::ConvertNoRef(netMsg);
	PeekMsg(m);

/* !!! THIS LOG MSG CRASHES THE CLIENT SOMETIMES! -eap
	hsLogEntry( nc->DebugMsg("<RCV> %s, %s, sz=%d",
		m->ClassName(), m->AsStdString().c_str(), m->GetNetCoreMsgLen()) );
*/

	/*
	plNetOwnershipMsg* netOwnMsg = TRACKED_NEW plNetOwnershipMsg;

	int i;
	for(i=0;i<m->GetNumGroups();i++)
	{
		plNetMsgGroupOwner::GroupInfo gr=m->GetGroupInfo(i);
		netOwnMsg->AddGroupInfo(gr);
		nc->GetNetGroups()->SetGroup(gr.fGroupID, gr.fOwnIt!=0 ? true : false);
		hsLogEntry( nc->DebugMsg("\tGroup 0x%x, ownIt=%d\n", (const char*)gr.fGroupID.Room().GetSequenceNumber(), gr.fOwnIt) );
	}

	if (netOwnMsg->GetNumGroups())
		netOwnMsg->Send();
	else
		delete netOwnMsg;
	*/

	nc->SetObjectOwner(m->IsOwner());

	return hsOK;
}



MSG_HANDLER_DEFN(plNetClientMsgHandler,plNetMsgSDLState)
{
	plNetClientMgr* nc = IGetNetClientMgr();
	plNetMsgSDLState* m = plNetMsgSDLState::ConvertNoRef(netMsg);
	PeekMsg(m);

/* !!! THIS LOG MSG CRASHES THE CLIENT SOMETIMES! -eap
	hsLogEntry( nc->DebugMsg("<RCV> %s, %s, sz=%d",
		m->ClassName(), m->AsStdString().c_str(), m->GetNetCoreMsgLen()) );
*/

	UInt32 rwFlags = 0;

	if ( m->IsInitialState() )
	{
		nc->IncNumInitialSDLStates();
		rwFlags |= plSDL::kMakeDirty;	// if initial state, we want all vars.
	}
	else if ( nc->GetFlagsBit( plNetClientApp::kLoadingInitialAgeState ) )
	{
		if ( nc->GetFlagsBit( plNetClientApp::kNeedInitialAgeStateCount ) )
		{
			hsLogEntry( nc->DebugMsg( "Ignoring SDL state because we are still joining age and don't have initial age state count yet." ) );
			return hsOK;
		}
		if ( nc->GetNumInitialSDLStates()<nc->GetRequiredNumInitialSDLStates() )
		{
			hsLogEntry( nc->DebugMsg( "Ignoring SDL state because we are still joining age and have not received all initial state yet." ) );
			return hsOK;
		}
		hsLogEntry( nc->DebugMsg( "We are still joining age, but have all initial states. Accepting this state (risky?)." ) );
	}
	

	// extract stateDataRecord from msg
	hsReadOnlyStream stream(m->StreamInfo()->GetStreamLen(), m->StreamInfo()->GetStreamBuf());
	char* descName = nil;
	int ver;
	plStateDataRecord::ReadStreamHeader(&stream, &descName, &ver);
	plStateDescriptor* des = plSDLMgr::GetInstance()->FindDescriptor(descName, ver);
	
	if (strcmpi(descName, kSDLAvatarPhysical) == 0)
		rwFlags |= plSDL::kKeepDirty;

	//
	// ERROR CHECK SDL FILE
	//
	plStateDataRecord* sdRec  = des ? TRACKED_NEW plStateDataRecord(des) : nil;
	if (!sdRec || sdRec->GetDescriptor()->GetVersion()!=ver)
	{
		std::string err;
		if (!sdRec)
			err = xtl::format( "SDL descriptor %s missing, v=%d", descName, ver);
		else
			err = xtl::format( "SDL descriptor %s, version mismatch, server v=%d, client v=%d",
				descName, ver, sdRec->GetDescriptor()->GetVersion());

		hsAssert(false, err.c_str());
		nc->ErrorMsg(const_cast<char*>(err.c_str()));

		// Post Quit message
		nc->QueueDisableNet(true, "SDL Desc Problem");		
		delete sdRec;
	}
	else if( sdRec->Read( &stream, 0, rwFlags ) )
	{
		plStateDataRecord* stateRec = nil;
		if (m->IsInitialState())
		{
			stateRec = TRACKED_NEW plStateDataRecord(des);
			stateRec->SetFromDefaults(false);
			stateRec->UpdateFrom(*sdRec, rwFlags);

			delete sdRec;
		}
		else
			stateRec = sdRec;

		plNetClientMgr::PendingLoad* pl = TRACKED_NEW plNetClientMgr::PendingLoad();
		pl->fSDRec = stateRec;		// will be deleted when PendingLoad is processed
		if (m->GetHasPlayerID())
			pl->fPlayerID = m->GetPlayerID();		// copy originating playerID if we have it
		pl->fUoid = m->ObjectInfo()->GetUoid();
		
		// queue up state
		nc->fPendingLoads.push_back(pl);
		hsLogEntry( nc->DebugMsg( "Added pending SDL delivery for %s:%s", m->ObjectInfo()->GetObjectName(), des->GetName() ) );
	}
	else
		delete sdRec;

	delete [] descName; // We've only used descName for a lookup (via SDR, and some error strings. Must delete now.
	
	return hsOK;
}

MSG_HANDLER_DEFN(plNetClientMsgHandler,plNetMsgGameMessage)
{
	plNetClientMgr* nc = IGetNetClientMgr();
	plNetMsgGameMessage* m = plNetMsgGameMessage::ConvertNoRef(netMsg);
	if (m)
	{
		PeekMsg(m);

		plNetMsgLoadClone * lcMsg = plNetMsgLoadClone::ConvertNoRef( m );
		if ( lcMsg )
		{
			if ( lcMsg->GetIsInitialState() )
			{
				nc->IncNumInitialSDLStates();
			}
		}

		hsReadOnlyStream stream(m->StreamInfo()->GetStreamLen(), m->StreamInfo()->GetStreamBuf());
		plMessage* gameMsg = plMessage::ConvertNoRef(hsgResMgr::ResMgr()->ReadCreatable(&stream));
		hsAssert(gameMsg, "nil game msg?");

		if (gameMsg)
		{
		/* !!! THIS LOG MSG CRASHES THE CLIENT SOMETIMES!!! -eap
			hsLogEntry( nc->DebugMsg("<RCV> %s, %s, sndr %s rcvr %s sz=%d",
				m->ClassName(), m->AsStdString().c_str(), 
				gameMsg->GetSender() ? gameMsg->GetSender()->GetName() : "?",
				gameMsg->GetNumReceivers() ? gameMsg->GetReceiver(0)->GetName() : "?",
				m->GetNetCoreMsgLen()) );
		*/

			if (lcMsg)
			{
				if (!lcMsg->GetIsLoading())
				{
					plLoadAvatarMsg* unloadClone = plLoadAvatarMsg::ConvertNoRef(gameMsg);
					if (unloadClone)
					{
						plLoadAvatarMsg* unloadMsg = TRACKED_NEW plLoadAvatarMsg(unloadClone->GetCloneKey(), unloadClone->GetRequestorKey(), unloadClone->GetUserData(), unloadClone->GetIsPlayer(), false);
						unloadMsg->SetOriginatingPlayerID(unloadClone->GetOriginatingPlayerID());
						gameMsg = unloadMsg;
					}
				}
				else
				{
					plLoadCloneMsg* loadClone = plLoadCloneMsg::ConvertNoRef(gameMsg);
					if (loadClone)
					{
						int idx = nc->fTransport.FindMember(loadClone->GetOriginatingPlayerID());
						if (idx == -1)
						{
							hsLogEntry( nc->DebugMsg( "Ignoring load clone because player isn't in our players list: %d", loadClone->GetOriginatingPlayerID()) );
							return hsOK;
						}
					}
				}
			}
			
			plNetClientApp::UnInheritNetMsgFlags(gameMsg);
			gameMsg->SetBCastFlag(plMessage::kNetCreatedRemotely);

			if (!m->GetDeliveryTime().AtEpoch())
			{
				double timeStamp;
				double secs=hsTimer::GetSysSeconds();
				m->GetDeliveryTime().ConvertToGameTime(&timeStamp, secs);
				hsAssert(timeStamp>=secs, "invalid future timeStamp");
				gameMsg->SetTimeStamp(timeStamp);
				nc->DebugMsg("Converting game msg future timeStamp, curT=%f, futT=%f", secs, timeStamp);
			}

			plgDispatch::Dispatch()->MsgSend(gameMsg);
			
			// Debug
			if (m->GetHasPlayerID())
			{
				int idx=nc->fTransport.FindMember(m->GetPlayerID());
				plNetTransportMember* mbr = idx != -1 ? nc->fTransport.GetMember(idx) : nil;
				if (mbr)
					mbr->SetTransportFlags(mbr->GetTransportFlags() | plNetTransportMember::kSendingActions);
			}
			return hsOK;
		}
	}
	return hsFail;
}

MSG_HANDLER_DEFN(plNetClientMsgHandler,plNetMsgVoice)
{
	plNetClientMgr* nc = IGetNetClientMgr();
	plNetMsgVoice* m = plNetMsgVoice::ConvertNoRef(netMsg);
	PeekMsg(m);

/* !!! THIS LOG MSG CRASHES THE CLIENT SOMETIMES! -eap
	hsLogEntry( nc->DebugMsg("<RCV> %s, %s, sz=%d",
		m->ClassName(), m->AsStdString().c_str(), m->GetNetCoreMsgLen()) );
*/

	int bufLen = m->GetVoiceDataLen();
	const char* buf = m->GetVoiceData();
	BYTE flags = m->GetFlags();
	BYTE numFrames = m->GetNumFrames();
	plKey key = NULL;

	// plKey key=hsgResMgr::ResMgr()->FindKey(m->ObjectInfo()->GetUoid());

	

	// Filter ignored sender
	if ( VaultAmIgnoringPlayer( m->GetPlayerID() ) )
	{
		hsLogEntry( nc->DebugMsg( "Ignoring voice chat from ignored player %lu", m->GetPlayerID() ) );
		return hsOK;
	}

	int idx=nc->fTransport.FindMember(m->GetPlayerID());
	plNetTransportMember* mbr = idx != -1 ? nc->fTransport.GetMember(idx) : nil;
	
	if (mbr)
	{
		key = mbr->GetAvatarKey();
		// filter based on listen/talk list (for forced mode)
		if (nc->GetListenListMode() == plNetClientMgr::kListenList_Forced)
		{
			if (nc->GetListenList()->FindMember( mbr ))
			{		
				hsLogEntry( nc->DebugMsg( "Ignoring voice chat from ignored player %lu", m->GetPlayerID() ) );
				return hsOK;
			}
		}
		mbr->SetTransportFlags(mbr->GetTransportFlags() | plNetTransportMember::kSendingVoice);
	}


//	hsKeyedObject* obj = key ? key->ObjectIsLoaded() : nil;
	plSceneObject* avObj = key ? plSceneObject::ConvertNoRef( key->ObjectIsLoaded() ) : nil;

//	if (obj)
	if (avObj)
	{
		plAudible * aud = avObj->GetAudioInterface()->GetAudible();
		pl2WayWinAudible* pAud = pl2WayWinAudible::ConvertNoRef(aud);
		if (pAud)
			pAud->PlayNetworkedSpeech(buf, bufLen,  numFrames, flags);
		else
		{
			nc->ErrorMsg("\tObject doesn't have audible");
		}
	}
	else
	{
		nc->DebugMsg("\tCan't find loaded object\n");
	}
	return hsOK;
}


MSG_HANDLER_DEFN(plNetClientMsgHandler,plNetMsgMembersList)
{
	plNetClientMgr* nc = IGetNetClientMgr();
	plNetMsgMembersList* m = plNetMsgMembersList::ConvertNoRef(netMsg);
	PeekMsg(m);

/* !!! THIS LOG MSG CRASHES THE CLIENT SOMETIMES! -eap
	hsLogEntry( nc->DebugMsg("<RCV> %s, %s, sz=%d",
		m->ClassName(), m->AsStdString().c_str(), m->GetNetCoreMsgLen()) );
*/

	int i;

	// remove existing members, except server
	for( i=nc->fTransport.GetNumMembers()-1 ; i>=0; i--  )
	{
		if (!nc->fTransport.GetMember(i)->IsServer())
		{			
			nc->fTransport.RemoveMember(i);			
		}
	} // for	     

	// update the members list from the msg.
	// this app is not one of the members in the msg
	for( i=0 ;i<m->MemberListInfo()->GetNumMembers() ;i++  )
	{
		plNetTransportMember* mbr = TRACKED_NEW plNetTransportMember(nc);
		IFillInTransportMember(m->MemberListInfo()->GetMember(i), mbr);
		hsLogEntry(nc->DebugMsg("\tAdding transport member, name=%s, p2p=%d, plrID=%d\n", mbr->AsStdString().c_str(), mbr->IsPeerToPeer(), mbr->GetPlayerID()));
		int idx=nc->fTransport.AddMember(mbr);
		hsAssert(idx>=0, "Failed adding member?");
			
	} // for	     

	// new player has been aded send local MembersUpdate msg
	plMemberUpdateMsg* mu = TRACKED_NEW plMemberUpdateMsg;
	mu->Send();

	return hsOK;
}

MSG_HANDLER_DEFN(plNetClientMsgHandler,plNetMsgMemberUpdate)
{
	plNetClientMgr* nc = IGetNetClientMgr();
	plNetMsgMemberUpdate* m = plNetMsgMemberUpdate::ConvertNoRef(netMsg);
	PeekMsg(m);

/* !!! THIS LOG MSG CRASHES THE CLIENT SOMETIMES! -eap
	hsLogEntry( nc->DebugMsg("<RCV> %s, %s, sz=%d",
		m->ClassName(), m->AsStdString().c_str(), m->GetNetCoreMsgLen()) );
*/
	
	if (m->AddingMember())
	{
		plNetTransportMember* mbr=nil;
		int idx = nc->fTransport.FindMember(m->MemberInfo()->GetClientGuid()->GetPlayerID());
		if ( idx>=0 )
			mbr = nc->fTransport.GetMember(idx);
		else
			mbr = TRACKED_NEW plNetTransportMember(nc);
		hsAssert(mbr, "nil xport member");
		IFillInTransportMember(m->MemberInfo(), mbr);
		
		if ( idx<0 )
		{	// didn't find him
			if ( nc->fTransport.AddMember(mbr)<0 )
				delete mbr;		// delete newly created member
		}
	}
	else
	{
		int idx=nc->fTransport.FindMember(m->MemberInfo()->GetClientGuid()->GetPlayerID());
		if (idx<0)
		{
			hsLogEntry( nc->DebugMsg("\tCan't find member to remove.") );
		}
		else
		{
			nc->fTransport.RemoveMember(idx);
		}
	}

	// new player has been aded send local MembersUpdate msg
	plMemberUpdateMsg* mu = TRACKED_NEW plMemberUpdateMsg;
	mu->Send();

	return hsOK;
}

MSG_HANDLER_DEFN(plNetClientMsgHandler,plNetMsgListenListUpdate)
{
	plNetClientMgr* nc = IGetNetClientMgr();
	plNetMsgListenListUpdate* m = plNetMsgListenListUpdate::ConvertNoRef(netMsg);
	PeekMsg(m);

/* !!! THIS LOG MSG CRASHES THE CLIENT SOMETIMES! -eap
	hsLogEntry( nc->DebugMsg("<RCV> %s, %s, sz=%d",
		m->ClassName(), m->AsStdString().c_str(), m->GetNetCoreMsgLen()) );
*/

	int idx=nc->fTransport.FindMember(m->GetPlayerID());
	plNetTransportMember* tm = (idx==-1 ? nil : nc->fTransport.GetMember(idx));
	if(!tm)
	{
#if 0
      tm = TRACKED_NEW plNetTransportMember(nc);
      tm->SetClientNum(m->GetSenderClientNum());
      int idx=nc->fTransport.AddMember(tm);
      hsAssert(idx>=0, "Failed adding member?");
      nc->DebugMsg("ListenListUpdate msg: Adding member on the fly\n");
#endif
      return hsOK;
   }
      
   if (m->GetAdding())
   {
      // add the sender to my talk list
      nc->GetTalkList()->AddMember(tm);
   }
   else
   {
      // remove the sender from my talk list
      nc->GetTalkList()->RemoveMember(tm);
   }
   
   return hsOK;
}

MSG_HANDLER_DEFN(plNetClientMsgHandler,plNetMsgInitialAgeStateSent)
{
	plNetClientMgr * nc = IGetNetClientMgr();
	plNetMsgInitialAgeStateSent* msg = plNetMsgInitialAgeStateSent::ConvertNoRef(netMsg);
	PeekMsg(msg);

/* !!! THIS LOG MSG CRASHES THE CLIENT SOMETIMES! -eap
	hsLogEntry( nc->DebugMsg("<RCV> %s, %s, sz=%d",
		netMsg->ClassName(), netMsg->AsStdString().c_str(), netMsg->GetNetCoreMsgLen()) );
*/

	nc->DebugMsg( "Initial age SDL count: %d", msg->GetNumInitialSDLStates( ) );

	nc->SetRequiredNumInitialSDLStates( msg->GetNumInitialSDLStates() );
	nc->SetFlagsBit( plNetClientApp::kNeedInitialAgeStateCount, false );

	if (nc->GetNumInitialSDLStates() >= nc->GetRequiredNumInitialSDLStates()) {
		nc->ICheckPendingStateLoad(hsTimer::GetSysSeconds());
		nc->NotifyRcvdAllSDLStates();
	}

	return hsOK;
}

////////////////////////////////////////////////////////////////////
// End.
