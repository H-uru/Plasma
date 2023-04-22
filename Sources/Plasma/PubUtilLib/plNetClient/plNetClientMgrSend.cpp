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

#include "plCreatableIndex.h"
#include "hsResMgr.h"
#include "hsTimer.h"

#include <string_theory/char_buffer>

#include "pnMessage/plCameraMsg.h"
#include "pnNetCommon/plSDLTypes.h"
#include "pnNetCommon/plSynchedObject.h"
#include "pnNetCommon/pnNetCommon.h"
#include "pnSceneObject/plSceneObject.h"

#include "plAvatar/plArmatureMod.h"
#include "plAvatar/plAvatarClothing.h"
#include "plAvatar/plAvatarMgr.h"
#include "plContainer/plConfigInfo.h"
#include "plDrawable/plMorphSequence.h"
#include "plMessage/plCCRMsg.h"
#include "plMessage/plLoadAvatarMsg.h"
#include "plMessage/plLoadCloneMsg.h"
#include "plNetClientRecorder/plNetClientRecorder.h"
#include "plNetCommon/plNetObjectDebugger.h"
#include "plNetGameLib/plNetGameLib.h"
#include "plNetMessage/plNetMessage.h"
#include "plParticleSystem/plParticleSDLMod.h"
#include "plParticleSystem/plParticleSystem.h"
#include "plResMgr/plLocalization.h"
#include "plSDL/plSDL.h"
#include "plVault/plVault.h"

#include "pfMessage/pfKIMsg.h"  // TMP

//
// request members list from server
//
void plNetClientMgr::ISendMembersListRequest()
{
    plNetMsgMembersListReq  msg;
    msg.SetNetProtocol(kNetProtocolCli2Game);
    SendMsg(&msg);
}

//
// reset paged in rooms list on server
//
void plNetClientMgr::ISendRoomsReset()
{
    plNetMsgPagingRoom msg;
    msg.SetPageFlags(plNetMsgPagingRoom::kResetList);
    msg.SetNetProtocol(kNetProtocolCli2Game);
    SendMsg(&msg);
}

//
// Make sure all dirty objects save their state.
// Mark those objects as clean and clear the dirty list.
//
void plNetClientMgr::ISendDirtyState(double secs)
{
    std::vector<plSynchedObject::StateDefn> carryOvers;

    size_t num = plSynchedObject::GetNumDirtyStates();
#if 0
    if (num)
    {
        DebugMsg("{} dirty sdl state msgs queued, t={f}", num, secs);
    }
#endif

    for (size_t i = 0; i < num; i++) {
        plSynchedObject::StateDefn* state=plSynchedObject::GetDirtyState(i);

        plSynchedObject* obj=state->GetObject();
        if (!obj)
            continue;   // could add to carryOvers

        if (!(state->fSendFlags & plSynchedObject::kSkipLocalOwnershipCheck))
        {
            int localOwned=obj->IsLocallyOwned();
            if (localOwned==plSynchedObject::kNo)
            {
                DebugMsg("Late rejection of queued SDL state, obj {}, sdl {}",
                    state->fObjKey->GetName(), state->fSDLName);
                continue;
            }
        }

        obj->CallDirtyNotifiers();
        obj->SendSDLStateMsg(state->fSDLName.c_str(), state->fSendFlags);
    }

    plSynchedObject::ClearDirtyState(carryOvers);
}

//
// Given a plasma petition msg, send a petition text node to the vault
// vault will detect and fwd to CCR system.
//
void plNetClientMgr::ISendCCRPetition(plCCRPetitionMsg* petMsg)
{
    // petition msg info
    uint8_t type = petMsg->GetType();
    ST::string title = petMsg->GetTitle();
    ST::string note = petMsg->GetNote().replace("\n", "\t");

    // stuff petition info fields into a config info object
    plConfigInfo info;
    info.AddValue( "Petition", "Type", type );
    info.AddValue( "Petition", "Content", note );
    info.AddValue( "Petition", "Title", title );
    info.AddValue( "Petition", "Language", plLocalization::GetLanguageName( plLocalization::GetLanguage() ) );
    info.AddValue("Petition", "AcctName", NetCommGetAccount()->accountName);
    info.AddValue("Petition", "PlayerID", ST::string::from_uint(GetPlayerID()));
    info.AddValue( "Petition", "PlayerName", GetPlayerName() );

    // write config info formatted like an ini file to a buffer
    hsRAMStream ram;
    plIniStreamConfigSource src(&ram);
    info.WriteTo(&src);
    int size = ram.GetPosition();
    ram.Rewind();
    ST::char_buffer buf;
    buf.allocate(size);
    ram.CopyToMem(buf.data());

    NetCliAuthSendCCRPetition(buf);
}

//
// send a msg to reset the camera in a new age
//
void plNetClientMgr::ISendCameraReset(bool bEnteringAge)
{   
    plCameraMsg* pCamMsg = new plCameraMsg;
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
    plSynchEnabler ps(true);    // make sure synching is enabled, since this happens during load

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

    std::vector<const plMorphSequence*> morphs;
    plMorphSequence::FindMorphMods(avMod->GetTarget(0), morphs);
    for (const plMorphSequence* morphSeq : morphs)
        if (morphSeq->GetTarget(0))
            morphSeq->GetTarget(0)->DirtySynchState(kSDLMorphSequence, plSynchedObject::kBCastToClients);
}

//
// Called to send a plasma msg out over the network.  Called by the dispatcher.
//
void plNetClientMgr::ISendGameMessage(plMessage* msg)
{
    if (GetFlagsBit(kDisabled))
        return;

    if (!fScreener.AllowOutgoingMessage(msg))
    {
        if (GetFlagsBit(kScreenMessages))
            return; // filter out illegal messages
    }
    
    // TEMP
    if (GetFlagsBit(kSilencePlayer))
    {
        pfKIMsg* kiMsg = pfKIMsg::ConvertNoRef(msg);
        if (kiMsg && kiMsg->GetCommand()==pfKIMsg::kHACKChatMsg)
            return;
    }

    plNetPlayerIDList* dstIDs = msg->GetNetReceivers();

#ifdef HS_DEBUGGING
    if ( dstIDs )
    {
        DebugMsg( "Preparing to send {} to specific players.", msg->ClassName() );
    }
#endif

    // get sender object
    plSynchedObject* synchedObj = msg->GetSender() ? plSynchedObject::ConvertNoRef(msg->GetSender()->ObjectIsLoaded()) : nullptr;

    // if sender is flagged as localOnly, he shouldn't talk to the network
    if (synchedObj && !synchedObj->IsNetSynched() )
        return;

    // choose appropriate type of net game msg wrapper
    plNetMsgGameMessage* netMsgWrap = nullptr;
    plLoadCloneMsg* loadClone = plLoadCloneMsg::ConvertNoRef(msg);
    if (loadClone)
    {
        plLoadAvatarMsg* lam=plLoadAvatarMsg::ConvertNoRef(msg);

        netMsgWrap = new plNetMsgLoadClone;
        plNetMsgLoadClone* netLoadClone=plNetMsgLoadClone::ConvertNoRef(netMsgWrap);
        
        netLoadClone->SetIsPlayer(lam && lam->GetIsPlayer());
        netLoadClone->SetIsLoading(loadClone->GetIsLoading()!=0);
        netLoadClone->ObjectInfo()->SetFromKey(loadClone->GetCloneKey());
    }
    else
    if (dstIDs)
    {
        netMsgWrap = new plNetMsgGameMessageDirected;
        int i;
        for(i=0;i<dstIDs->size();i++)
        {
            uint32_t playerID = (*dstIDs)[i];
            if (playerID == NetCommGetPlayer()->playerInt)
                continue;
            hsLogEntry( DebugMsg( "\tAdding receiver: {}" , playerID ) );
            ((plNetMsgGameMessageDirected*)netMsgWrap)->Receivers()->AddReceiverPlayerID( playerID );
        }
    }
    else
        netMsgWrap = new plNetMsgGameMessage;

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
    bool bCast = msg->HasBCastFlag(plMessage::kBCastByExactType) ||
        msg->HasBCastFlag(plMessage::kBCastByType);
    bool directCom = msg->GetNumReceivers()>0;
    if( directCom )
    {
        // It's direct if we have receivers AND any of them are in non-virtual locations
        int     i;
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
        WarningMsg("Msg {} has no rcvrs or bcast instructions?", msg->ClassName());

    hsAssert(!(directCom && bCast), "msg has both rcvrs and bcast instructions, rcvrs ignored");
    if (directCom && !bCast)
    {
        netMsgWrap->SetBit(plNetMessage::kHasGameMsgRcvrs); // for quick server filtering
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
    bool ccrSendToAllPlayers = false;
#ifndef PLASMA_EXTERNAL_RELEASE
    ccrSendToAllPlayers = msg->HasBCastFlag( plMessage::kCCRSendToAllPlayers );
    if ( ccrSendToAllPlayers )
        netMsgWrap->SetBit( plNetMessage::kRouteToAllPlayers );
#endif

    //
    // check for inter-age routing. if set, online rcvrs not in current age will receive
    // this msg courtesy of pls routing.
    //
    if ( !ccrSendToAllPlayers )
    {
        bool allowInterAge = msg->HasBCastFlag( plMessage::kNetAllowInterAge );
        if ( allowInterAge )
            netMsgWrap->SetBit(plNetMessage::kInterAgeRouting);
    }

    // check for reliable send
    if (msg->HasBCastFlag(plMessage::kNetSendUnreliable) && 
        !(synchedObj && (synchedObj->GetSynchFlags() & plSynchedObject::kSendReliably)) )
        netMsgWrap->SetBit(plNetMessage::kNeedsReliableSend, 0);    // clear reliable net send bit

#ifdef HS_DEBUGGING
    int16_t type=*(int16_t*)netMsgWrap->StreamInfo()->GetStreamBuf();
    hsAssert(type>=0 && type<plCreatableIndex::plNumClassIndices, "garbage type out");
#endif

    netMsgWrap->SetPlayerID(GetPlayerID());
    netMsgWrap->SetNetProtocol(kNetProtocolCli2Game);
    SendMsg(netMsgWrap);

    if (plNetObjectDebugger::GetInstance()->IsDebugObject(msg->GetSender() ? msg->GetSender()->ObjectIsLoaded() : nullptr))
    {
    #if 0
        hsLogEntry(plNetObjectDebugger::GetInstance()->LogMsg(
            ST::format("<SND> object:{}, rcvr {} {}",
            msg->GetSender().GetKeyName(),
            msg->GetNumReceivers() ? msg->GetReceiver(0)->GetName() : "?",
            netMsgWrap->AsStdString())));
    #endif
    }

    delete netMsgWrap;
}

//
// Send a net msg.  Delivers to transport mgr who sends p2p or to server
//
void plNetClientMgr::SendMsg(plNetMessage* msg)
{
    if (GetFlagsBit(kDisabled))
        return;

    if (!CanSendMsg(msg))
        return;

    // If we're recording messages, set an identifying flag and echo the message back to ourselves
    if (fMsgRecorder && fMsgRecorder->IsRecordableMsg(msg))
    {
        msg->SetBit(plNetMessage::kEchoBackToSender, true);
    }
    
    msg->SetTimeSent(plUnifiedTime::GetCurrent());
    int channel = IPrepMsg(msg);
    
//  hsLogEntry( DebugMsg( "<SND> {} {}", msg->ClassName(), msg->AsStdString()) );
    
    fTransport.SendMsg(channel, msg);

    // Debug
    if (plNetMsgVoice::ConvertNoRef(msg))
        SetFlagsBit(kSendingVoice);
    if (plNetMsgGameMessage::ConvertNoRef(msg))
        SetFlagsBit(kSendingActions);
}
