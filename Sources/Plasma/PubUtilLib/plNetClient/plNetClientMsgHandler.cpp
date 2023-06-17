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

#include "plNetClientMsgHandler.h"

#include "plNetClientMgr.h"
#include "plNetLinkingMgr.h"

#include "plCreatableIndex.h"
#include "plgDispatch.h"
#include "hsResMgr.h"
#include "hsTimer.h"

#include "pnKeyedObject/plKey.h"
#include "pnMessage/plTimeMsg.h"
#include "pnNetCommon/pnNetCommon.h"
#include "pnNetCommon/plSDLTypes.h"
#include "pnSceneObject/plAudioInterface.h"
#include "pnSceneObject/plSceneObject.h"

#include "plAudible/plWinAudible.h"
#include "plAvatar/plAvatarMgr.h"
#include "plMessage/plLoadAvatarMsg.h"
#include "plMessage/plLoadCloneMsg.h"
#include "plMessage/plMemberUpdateMsg.h"
#include "plMessage/plNetOwnershipMsg.h"
#include "plNetMessage/plNetMessage.h"
#include "plNetTransport/plNetTransportMember.h"
#include "plSDL/plSDL.h"
#include "plVault/plVault.h"

#include "pfMessage/pfKIMsg.h"      // Should be moved to PubUtil level

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

void plNetClientMsgHandler::IFillInTransportMember(const plNetMsgMemberInfoHelper* mbi, plNetTransportMember* mbr)
{
    const plNetClientMgr* nc=IGetNetClientMgr();
    uint16_t port = mbi->GetClientGuid()->GetSrcPort();
    uint32_t addr = mbi->GetClientGuid()->GetSrcAddr();       
    uint32_t flags = mbi->GetFlags();
    uint32_t plrID = mbi->GetClientGuid()->GetPlayerID();
    plUoid avUoid = mbi->GetAvatarUoid();
    plKey avKey=hsgResMgr::ResMgr()->FindKey(avUoid);

    mbr->SetPlayerName(mbi->GetClientGuid()->GetPlayerName());
    mbr->SetFlags(flags);
    mbr->SetPlayerID(plrID);
    mbr->SetCCRLevel(mbi->GetClientGuid()->GetCCRLevel());
    if (avKey)
        mbr->SetAvatarKey(avKey);
}

plNetMsgHandler::Status plNetClientMsgHandler::ReceiveMsg(plNetMessage *& netMsg)
{
#ifdef HS_DEBUGGING
    //plNetClientMgr::GetInstance()->DebugMsg("<RCV> {}", netMsg->ClassName());
#endif

    plNetClientMgr::GetInstance()->UpdateServerTimeOffset(netMsg);
    
    switch(netMsg->ClassIndex())
    {
        default:
            plNetClientMgr::GetInstance()->ErrorMsg( "Unknown msg: {}", netMsg->ClassName() );
            return plNetMsgHandler::Status::kError;

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
    return plNetMsgHandler::Status::kHandled;
}

MSG_HANDLER_DEFN(plNetClientMsgHandler,plNetMsgGroupOwner)
{
    plNetClientMgr* nc = IGetNetClientMgr();
    plNetMsgGroupOwner* m = plNetMsgGroupOwner::ConvertNoRef(netMsg);

/* !!! THIS LOG MSG CRASHES THE CLIENT SOMETIMES! -eap
    hsLogEntry( nc->DebugMsg("<RCV> {}, {}", m->ClassName(), m->AsStdString()));
*/

    /*
    plNetOwnershipMsg* netOwnMsg = new plNetOwnershipMsg;

    for(size_t i = 0; i < m->GetNumGroups(); i++)
    {
        plNetMsgGroupOwner::GroupInfo gr=m->GetGroupInfo(i);
        netOwnMsg->AddGroupInfo(gr);
        nc->GetNetGroups()->SetGroup(gr.fGroupID, gr.fOwnIt!=0 ? true : false);
        hsLogEntry( nc->DebugMsg("\tGroup 0x{x}, ownIt={}\n", gr.fGroupID.Room().GetSequenceNumber(), gr.fOwnIt) );
    }

    if (netOwnMsg->GetNumGroups())
        netOwnMsg->Send();
    else
        delete netOwnMsg;
    */

    // Simplified object ownership model means that one client owns everything.
    hsLogEntry(nc->DebugMsg("<RCV> plNetMsgGroupOwner, isOwner={}", m->IsOwner()));
    nc->SetObjectOwner(m->IsOwner());

    plNetOwnershipMsg* netOwnMsg = new plNetOwnershipMsg();
    netOwnMsg->Send();

    return plNetMsgHandler::Status::kHandled;
}



MSG_HANDLER_DEFN(plNetClientMsgHandler,plNetMsgSDLState)
{
    plNetClientMgr* nc = IGetNetClientMgr();
    plNetMsgSDLState* m = plNetMsgSDLState::ConvertNoRef(netMsg);

/* !!! THIS LOG MSG CRASHES THE CLIENT SOMETIMES! -eap
    hsLogEntry(nc->DebugMsg("<RCV> {}, {}, sz={}", m->ClassName(), m->AsStdString()));
*/

    uint32_t rwFlags = 0;

    if ( m->IsInitialState() )
    {
        nc->IncNumInitialSDLStates();
        rwFlags |= plSDL::kMakeDirty;   // if initial state, we want all vars.
    }
    else if ( nc->GetFlagsBit( plNetClientApp::kLoadingInitialAgeState ) )
    {
        if ( nc->GetFlagsBit( plNetClientApp::kNeedInitialAgeStateCount ) )
        {
            hsLogEntry( nc->DebugMsg( "Ignoring SDL state because we are still joining age and don't have initial age state count yet." ) );
            return plNetMsgHandler::Status::kHandled;
        }
        if ( nc->GetNumInitialSDLStates()<nc->GetRequiredNumInitialSDLStates() )
        {
            hsLogEntry( nc->DebugMsg( "Ignoring SDL state because we are still joining age and have not received all initial state yet." ) );
            return plNetMsgHandler::Status::kHandled;
        }
        hsLogEntry( nc->DebugMsg( "We are still joining age, but have all initial states. Accepting this state (risky?)." ) );
    }
    

    // extract stateDataRecord from msg
    hsReadOnlyStream stream(m->StreamInfo()->GetStreamLen(), m->StreamInfo()->GetStreamBuf());
    ST::string descName;
    int ver;
    plStateDataRecord::ReadStreamHeader(&stream, &descName, &ver);
    plStateDescriptor* des = plSDLMgr::GetInstance()->FindDescriptor(descName, ver);
    
    if (descName.compare_i(kSDLAvatarPhysical) == 0)
        rwFlags |= plSDL::kKeepDirty;

    //
    // ERROR CHECK SDL FILE
    //
    plStateDataRecord* sdRec  = des ? new plStateDataRecord(des) : nullptr;
    if (!sdRec || sdRec->GetDescriptor()->GetVersion()!=ver)
    {
        ST::string err;
        if (!sdRec)
            err = ST::format("SDL descriptor {} missing, v={}", descName, ver);
        else
            err = ST::format("SDL descriptor {}, version mismatch, server v={}, client v={}",
                descName, ver, sdRec->GetDescriptor()->GetVersion());

        hsAssert(false, err.c_str());
        nc->ErrorMsg(err);

        // Post Quit message
        nc->QueueDisableNet(true, "SDL Desc Problem");
        delete sdRec;
    }
    else if( sdRec->Read( &stream, 0, rwFlags ) )
    {
        plStateDataRecord* stateRec = nullptr;
        if (m->IsInitialState())
        {
            stateRec = new plStateDataRecord(des);
            stateRec->SetFromDefaults(false);
            stateRec->UpdateFrom(*sdRec, rwFlags);

            delete sdRec;
        }
        else
            stateRec = sdRec;

        plNetClientMgr::PendingLoad* pl = new plNetClientMgr::PendingLoad();
        pl->fSDRec = stateRec;      // will be deleted when PendingLoad is processed
        if (m->GetHasPlayerID())
            pl->fPlayerID = m->GetPlayerID();       // copy originating playerID if we have it
        pl->fUoid = m->ObjectInfo()->GetUoid();

        // queue up state
        nc->fPendingLoads.push_back(pl);
        hsLogEntry( nc->DebugMsg( "Added pending SDL delivery for {}:{}",
                                  m->ObjectInfo()->GetObjectName(), des->GetName() ) );
    }
    else
        delete sdRec;

    return plNetMsgHandler::Status::kHandled;
}

MSG_HANDLER_DEFN(plNetClientMsgHandler,plNetMsgGameMessage)
{
    plNetClientMgr* nc = IGetNetClientMgr();
    plNetMsgGameMessage* m = plNetMsgGameMessage::ConvertNoRef(netMsg);
    if (m)
    {
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
            hsLogEntry( nc->DebugMsg("<RCV> {}, {}, sndr {} rcvr {}",
                m->ClassName(), m->AsStdString(),
                gameMsg->GetSender() ? gameMsg->GetSender()->GetName() : "?",
                gameMsg->GetNumReceivers() ? gameMsg->GetReceiver(0)->GetName() : "?"));
        */

            if (lcMsg)
            {
                if (!lcMsg->GetIsLoading())
                {
                    plLoadAvatarMsg* unloadClone = plLoadAvatarMsg::ConvertNoRef(gameMsg);
                    if (unloadClone)
                    {
                        plLoadAvatarMsg* unloadMsg = new plLoadAvatarMsg(unloadClone->GetCloneKey(), unloadClone->GetRequestorKey(), unloadClone->GetUserData(), unloadClone->GetIsPlayer(), false);
                        unloadMsg->SetOriginatingPlayerID(unloadClone->GetOriginatingPlayerID());
                        gameMsg = unloadMsg;
                    }
                }
                else
                {
                    plLoadCloneMsg* loadClone = plLoadCloneMsg::ConvertNoRef(gameMsg);
                    if (loadClone)
                    {
                        hsSsize_t idx = nc->fTransport.FindMember(loadClone->GetOriginatingPlayerID());
                        if (idx == -1)
                        {
                            hsLogEntry( nc->DebugMsg( "Ignoring load clone because player isn't in our players list: {}", loadClone->GetOriginatingPlayerID()) );
                            return plNetMsgHandler::Status::kHandled;
                        }
                    }
                }
            }
            
            plNetClientApp::UnInheritNetMsgFlags(gameMsg);
            gameMsg->SetBCastFlag(plMessage::kNetCreatedRemotely);

            if (!m->GetDeliveryTime().AtEpoch())
            {
                double secs=hsTimer::GetSysSeconds();
                double timeStamp = m->GetDeliveryTime().ConvertToGameTime();
                hsAssert(timeStamp>=secs, "invalid future timeStamp");
                gameMsg->SetTimeStamp(timeStamp);
                nc->DebugMsg("Converting game msg future timeStamp, curT={f}, futT={f}", secs, timeStamp);
            }

            // Do some basic security checks on the incoming message because
            // we cannot nesecarily trust the server because the server trusts
            // the remote client WAY too much.
            if (!IGetNetClientMgr()->fScreener.AllowIncomingMessage(gameMsg))
                return plNetMsgHandler::Status::kHandled;

            plgDispatch::Dispatch()->MsgSend(gameMsg);
            
            // Debug
            if (m->GetHasPlayerID())
            {
                plNetTransportMember* mbr = nc->fTransport.GetMemberByID(m->GetPlayerID());
                if (mbr)
                    mbr->SetTransportFlags(mbr->GetTransportFlags() | plNetTransportMember::kSendingActions);
            }
            return plNetMsgHandler::Status::kHandled;
        }
    }
    return plNetMsgHandler::Status::kError;
}

MSG_HANDLER_DEFN(plNetClientMsgHandler,plNetMsgVoice)
{
    plNetClientMgr* nc = IGetNetClientMgr();
    plNetMsgVoice* m = plNetMsgVoice::ConvertNoRef(netMsg);

/* !!! THIS LOG MSG CRASHES THE CLIENT SOMETIMES! -eap
    hsLogEntry(nc->DebugMsg("<RCV> {}, {}", m->ClassName(), m->AsStdString()));
*/

    size_t bufLen = m->GetVoiceDataLen();
    const char* buf = m->GetVoiceData();
    uint8_t flags = m->GetFlags();
    uint8_t numFrames = m->GetNumFrames();
    plKey key;

    // Filter ignored sender
    if (VaultAmIgnoringPlayer(m->GetPlayerID())) {
        hsLogEntry(nc->DebugMsg("Ignoring voice chat from ignored player {}", m->GetPlayerID()));
        return plNetMsgHandler::Status::kHandled;
    }

    plNetTransportMember* mbr = nc->fTransport.GetMemberByID(m->GetPlayerID());

    if (mbr) {
        key = mbr->GetAvatarKey();
        // filter based on listen/talk list (for forced mode)
        if (nc->GetListenListMode() == plNetClientMgr::kListenList_Forced) {
            if (nc->GetListenList()->FindMember(mbr)) {
                hsLogEntry(nc->DebugMsg("Ignoring voice chat from ignored player {}", m->GetPlayerID()));
                return plNetMsgHandler::Status::kHandled;
            }
        }
        mbr->SetTransportFlags(mbr->GetTransportFlags() | plNetTransportMember::kSendingVoice);
    } else if (m->GetPlayerID() == nc->GetPlayerID()) {
        key = nc->GetLocalPlayerKey();
    }


    plSceneObject* avObj = key ? plSceneObject::ConvertNoRef(key->ObjectIsLoaded()) : nullptr;
    if (avObj) {
        plAudible * aud = avObj->GetAudioInterface()->GetAudible();
        pl2WayWinAudible* pAud = pl2WayWinAudible::ConvertNoRef(aud);
        if (pAud)
            pAud->PlayNetworkedSpeech(buf, bufLen,  numFrames, flags);
        else
            nc->ErrorMsg("\tObject doesn't have audible");
    } else {
        nc->DebugMsg("\tCan't find loaded object\n");
    }
    return plNetMsgHandler::Status::kHandled;
}


MSG_HANDLER_DEFN(plNetClientMsgHandler,plNetMsgMembersList)
{
    plNetClientMgr* nc = IGetNetClientMgr();
    plNetMsgMembersList* m = plNetMsgMembersList::ConvertNoRef(netMsg);

/* !!! THIS LOG MSG CRASHES THE CLIENT SOMETIMES! -eap
    hsLogEntry(nc->DebugMsg("<RCV> {}, {}", m->ClassName(), m->AsStdString()));
*/

    // remove existing members, except server
    for (hsSsize_t i = nc->fTransport.GetNumMembers() - 1; i >= 0; i--)
    {
        if (!nc->fTransport.GetMember(i)->IsServer())
        {           
            nc->fTransport.RemoveMember(i);
        }
    }

    // update the members list from the msg.
    // this app is not one of the members in the msg
    for (size_t i = 0; i < m->MemberListInfo()->GetNumMembers(); i++)
    {
        plNetTransportMember* mbr = new plNetTransportMember(nc);
        IFillInTransportMember(m->MemberListInfo()->GetMember(i), mbr);
        hsLogEntry(nc->DebugMsg("\tAdding transport member, name={}, plrID={}\n", mbr->AsString(), mbr->GetPlayerID()));
        hsSsize_t idx = nc->fTransport.AddMember(mbr);
        hsAssert(idx>=0, "Failed adding member?");
            
    }

    // new player has been aded send local MembersUpdate msg
    plMemberUpdateMsg* mu = new plMemberUpdateMsg;
    mu->Send();

    return plNetMsgHandler::Status::kHandled;
}

MSG_HANDLER_DEFN(plNetClientMsgHandler,plNetMsgMemberUpdate)
{
    plNetClientMgr* nc = IGetNetClientMgr();
    plNetMsgMemberUpdate* m = plNetMsgMemberUpdate::ConvertNoRef(netMsg);

/* !!! THIS LOG MSG CRASHES THE CLIENT SOMETIMES! -eap
    hsLogEntry(nc->DebugMsg("<RCV> {}, {}", m->ClassName(), m->AsStdString()));
*/
    
    if (m->AddingMember())
    {
        plNetTransportMember* mbr = nullptr;
        hsSsize_t idx = nc->fTransport.FindMember(m->MemberInfo()->GetClientGuid()->GetPlayerID());
        if ( idx>=0 )
            mbr = nc->fTransport.GetMember(idx);
        else
            mbr = new plNetTransportMember(nc);
        hsAssert(mbr, "nil xport member");
        IFillInTransportMember(m->MemberInfo(), mbr);
        
        if ( idx<0 )
        {   // didn't find him
            if ( nc->fTransport.AddMember(mbr)<0 )
                delete mbr;     // delete newly created member
        }
    }
    else
    {
        hsSsize_t idx = nc->fTransport.FindMember(m->MemberInfo()->GetClientGuid()->GetPlayerID());
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
    plMemberUpdateMsg* mu = new plMemberUpdateMsg;
    mu->Send();

    return plNetMsgHandler::Status::kHandled;
}

MSG_HANDLER_DEFN(plNetClientMsgHandler,plNetMsgListenListUpdate)
{
    plNetClientMgr* nc = IGetNetClientMgr();
    plNetMsgListenListUpdate* m = plNetMsgListenListUpdate::ConvertNoRef(netMsg);

/* !!! THIS LOG MSG CRASHES THE CLIENT SOMETIMES! -eap
    hsLogEntry(nc->DebugMsg("<RCV> {}, {}", m->ClassName(), m->AsStdString()));
*/

    plNetTransportMember* tm = nc->fTransport.GetMemberByID(m->GetPlayerID());
    if(!tm)
    {
#if 0
      tm = new plNetTransportMember(nc);
      tm->SetClientNum(m->GetSenderClientNum());
      int idx=nc->fTransport.AddMember(tm);
      hsAssert(idx>=0, "Failed adding member?");
      nc->DebugMsg("ListenListUpdate msg: Adding member on the fly\n");
#endif
      return plNetMsgHandler::Status::kHandled;
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
   
   return plNetMsgHandler::Status::kHandled;
}

MSG_HANDLER_DEFN(plNetClientMsgHandler,plNetMsgInitialAgeStateSent)
{
    plNetClientMgr * nc = IGetNetClientMgr();
    plNetMsgInitialAgeStateSent* msg = plNetMsgInitialAgeStateSent::ConvertNoRef(netMsg);

/* !!! THIS LOG MSG CRASHES THE CLIENT SOMETIMES! -eap
    hsLogEntry(nc->DebugMsg("<RCV> {}, {}", netMsg->ClassName(), netMsg->AsStdString()));
*/

    nc->DebugMsg( "Initial age SDL count: {}", msg->GetNumInitialSDLStates( ) );

    nc->SetRequiredNumInitialSDLStates( msg->GetNumInitialSDLStates() );
    nc->SetFlagsBit( plNetClientApp::kNeedInitialAgeStateCount, false );

    if (nc->GetNumInitialSDLStates() >= nc->GetRequiredNumInitialSDLStates()) {
        nc->ICheckPendingStateLoad(hsTimer::GetSysSeconds());
        nc->NotifyRcvdAllSDLStates();
    }

    return plNetMsgHandler::Status::kHandled;
}

////////////////////////////////////////////////////////////////////
// End.
