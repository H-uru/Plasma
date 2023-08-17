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
#include "plNetLinkingMgr.h"
#include "plNetClientMgr.h"
#include "plNetCliAgeJoiner.h"
#include "plNetCliAgeLeaver.h"

#include <list>
#include <string_theory/string>

#include "pnKeyedObject/plKey.h"
#include "pnMessage/plClientMsg.h"
#include "pnNetCommon/pnNetCommon.h"
#include "pnSceneObject/plSceneObject.h"

#include "plAvatar/plAvatarMgr.h"
#include "plAvatar/plArmatureMod.h"
#include "plMessage/plLinkToAgeMsg.h"
#include "plMessage/plVaultNotifyMsg.h"
#include "plNetTransport/plNetTransportMember.h"        // OfferLinkToPlayer()
#include "plVault/plVault.h"


/*****************************************************************************
*
*   Private
*
***/

enum ENlmOp {
    kNlmOpNoOp,
    kNlmOpWaitOp,
    kNlmOpJoinAgeOp,
    kNlmOpLeaveAgeOp,
    kNumNlmOps
};

struct NlmOp {
    ENlmOp  opcode;
    NlmOp (const ENlmOp & op)
        : opcode(op)
    { }

    virtual ~NlmOp() = default;
};

struct NlmNoOpOp : NlmOp {
    NlmNoOpOp ()
        : NlmOp(kNlmOpNoOp)
    { }
};

struct NlmOpWaitOp : NlmOp {
    NlmOpWaitOp ()
        : NlmOp(kNlmOpWaitOp)
    { }
};

struct NlmJoinAgeOp : NlmOp {
    NetCommAge  age;
    bool muteSfx;
    NlmJoinAgeOp ()
        : NlmOp(kNlmOpJoinAgeOp), muteSfx()
    { }
};

struct NlmLeaveAgeOp : NlmOp {
    bool    quitting;
    bool    muteSfx;
    NlmLeaveAgeOp ()
        : NlmOp(kNlmOpLeaveAgeOp), quitting(), muteSfx()
    { }
};


/*****************************************************************************
*
*   Private data
*
***/

static plNCAgeJoiner *              s_ageJoiner;
static plNCAgeLeaver *              s_ageLeaver;
static std::list<NlmOp*>            s_opqueue;


/*****************************************************************************
*
*   Local functions
*
***/

//============================================================================
static void QueueOp (NlmOp * op, bool front = false) {
    if (front) {
        s_opqueue.push_front(op);
    } else {
        s_opqueue.push_back(op);
    }
}


/*****************************************************************************
*
*   plNetLinkingMgr
*
***/

//============================================================================
void plNetLinkingMgr::NCAgeJoinerCallback (
    plNCAgeJoiner *         joiner,
    unsigned                type,
    void *                  notify,
    void *                  userState
) {
    NCAgeJoinerCompleteNotify* params = (NCAgeJoinerCompleteNotify*)notify;

    // Tell the user we failed to link.
    // In the future, we might want to try graceful recovery (link back to Relto?)
    if (!params->success) {
        plNetClientApp::GetInstance()->ErrorMsg(params->msg);
        plNetClientApp::GetInstance()->QueueDisableNet(true, params->msg);
    }
    
    plNetLinkingMgr * lm = plNetLinkingMgr::GetInstance();
    switch (type) {
        case kAgeJoinerComplete: {
            ASSERT(joiner == s_ageJoiner);
            s_ageJoiner = nullptr;
            
            lm->IPostProcessLink();

            lm->fLinkedIn = true;
            lm->SetEnabled(true);
            
            // Pull our wait op off exec queue
            if (NlmOpWaitOp * waitOp = (NlmOpWaitOp *) userState) {
                s_opqueue.remove(waitOp);
                delete waitOp;
            }
        }
        break;
        
        DEFAULT_FATAL(type);
    }
}

//============================================================================
void plNetLinkingMgr::NCAgeLeaverCallback (
    plNCAgeLeaver *         leaver,
    unsigned                type,
    void *                  notify,
    void *                  userState
) {
    switch (type) {
        case kAgeLeaveComplete: {
            ASSERT(leaver == s_ageLeaver);
            s_ageLeaver = nullptr;

            // Pull our wait op off exec queue
            if (NlmOpWaitOp * waitOp = (NlmOpWaitOp *) userState) {
                s_opqueue.remove(waitOp);
                delete waitOp;
            }
        }
        break;
        
        DEFAULT_FATAL(type);
    }
}

//============================================================================
void plNetLinkingMgr::ExecNextOp () {
    plNetLinkingMgr * lm = plNetLinkingMgr::GetInstance();

    if (!s_opqueue.size())
        return;

    NlmOp* op = s_opqueue.front();

    switch (op->opcode) {
        case kNlmOpNoOp:
            break;

        case kNlmOpWaitOp:
            return; // don't allow wait op to be unlinked/deleted from list

        case kNlmOpJoinAgeOp: {
            ASSERT(!s_ageJoiner);
            ASSERT(!s_ageLeaver);

            // Insert a wait operation into the exec queue
            NlmOpWaitOp * waitOp = new NlmOpWaitOp;
            QueueOp(waitOp, true);

            NlmJoinAgeOp * joinAgeOp = (NlmJoinAgeOp *)op;
            NCAgeJoinerCreate(
                &s_ageJoiner,
                joinAgeOp->age,
                joinAgeOp->muteSfx,
                NCAgeJoinerCallback,
                waitOp
            );
        }
        break;

        case kNlmOpLeaveAgeOp: {
            ASSERT(!s_ageJoiner);
            ASSERT(!s_ageLeaver);

            // Insert a wait operation into the exec queue
            NlmOpWaitOp * waitOp = new NlmOpWaitOp;
            QueueOp(waitOp, true);

            lm->SetEnabled(false);
            lm->fLinkedIn = false;

            NlmLeaveAgeOp * leaveAgeOp = (NlmLeaveAgeOp *)op;
            NCAgeLeaverCreate(
                &s_ageLeaver,
                leaveAgeOp->quitting,
                leaveAgeOp->muteSfx,
                NCAgeLeaverCallback,
                waitOp
            );
        }
        break;

        default:
            break;
    }

    s_opqueue.remove(op);
    delete op;
}


////////////////////////////////////////////////////////////////////

plNetLinkingMgr::~plNetLinkingMgr()
{
    std::for_each(s_opqueue.begin(), s_opqueue.end(),
        [](NlmOp * op) { delete op; }
    );
}

plNetLinkingMgr * plNetLinkingMgr::GetInstance()
{
    static plNetLinkingMgr Instance;
    return &Instance;
}


////////////////////////////////////////////////////////////////////

void plNetLinkingMgr::SetEnabled( bool b )
{
    plNetClientMgr * nc = plNetClientMgr::GetInstance();
    hsLogEntry( nc->DebugMsg( "plNetLinkingMgr: {} -> {}", fLinkingEnabled?"Enabled":"Disabled",b?"Enabled":"Disabled" ) );
    fLinkingEnabled = b;
}

////////////////////////////////////////////////////////////////////

// static
ST::string plNetLinkingMgr::GetProperAgeName( const ST::string & ageName )
{
    plNetClientMgr * nc = plNetClientMgr::GetInstance();
    std::vector<plFileName> files = plFileSystem::ListDir("dat", "*.age");
    for (auto iter = files.begin(); iter != files.end(); ++iter)
    {
        ST::string work = iter->GetFileNameNoExt();
        if (ageName.compare_i(work) == 0)
            return work;
    }
    return ageName;
}

////////////////////////////////////////////////////////////////////

bool plNetLinkingMgr::MsgReceive( plMessage *msg )
{
    if (s_ageLeaver && NCAgeLeaverMsgReceive(s_ageLeaver, msg))
        return true;

    if (s_ageJoiner && NCAgeJoinerMsgReceive(s_ageJoiner, msg))
        return true;

    if (plLinkToAgeMsg * pLinkMsg = plLinkToAgeMsg::ConvertNoRef(msg)) {
        if (!fLinkingEnabled)
            hsLogEntry(plNetClientMgr::GetInstance()->DebugMsg("Not linking. Linking is disabled."));
        else
            IProcessLinkToAgeMsg(pLinkMsg);
        return true;
    }

    if (plLinkingMgrMsg * pLinkingMgrMsg = plLinkingMgrMsg::ConvertNoRef(msg)) {
        IProcessLinkingMgrMsg( pLinkingMgrMsg );
        return true;
    }

    // If a link was deferred in order to register an owned age, we will
    // get a VaultNotify about the registration
    if (plVaultNotifyMsg* vaultMsg = plVaultNotifyMsg::ConvertNoRef(msg)) {
        IProcessVaultNotifyMsg(vaultMsg);
        return true;
    }

    return false;
}

////////////////////////////////////////////////////////////////////

void plNetLinkingMgr::Update()
{
    if (s_ageLeaver)
        NCAgeLeaverUpdate(s_ageLeaver);
    if (s_ageJoiner)
        NCAgeJoinerUpdate(s_ageJoiner);

    ExecNextOp();
}

////////////////////////////////////////////////////////////////////

bool plNetLinkingMgr::IProcessLinkToAgeMsg( plLinkToAgeMsg * msg )
{
    if (!fLinkingEnabled) {
        hsLogEntry( plNetClientMgr::GetInstance()->DebugMsg( "Not linking. Linking is disabled." ) );
        return false;
    }

    plNetClientMgr * nc = plNetClientMgr::GetInstance();

    bool result = true;

    plAgeLinkStruct save;
    save.CopyFrom( GetPrevAgeLink() );
    GetPrevAgeLink()->CopyFrom( GetAgeLink() );
    GetAgeLink()->CopyFrom( msg->GetAgeLink() );

    // Actually do stuff...
    uint8_t pre = IPreProcessLink();
    if (pre == kLinkImmediately)
    {
        msg->Ref();
        IDoLink(msg);
    }
    else if (pre == kLinkDeferred)
    {
        msg->Ref();
        fDeferredLink = msg;
    }
    else if (pre == kLinkFailed)
    {
        hsLogEntry( nc->ErrorMsg( "IPreProcessLink failed. Not linking." ) );
        // Restore previous age info state.
        GetAgeLink()->CopyFrom( GetPrevAgeLink() );
        GetPrevAgeLink()->CopyFrom( &save );
        result = false;
    }
    
    return result;
}

////////////////////////////////////////////////////////////////////

void plNetLinkingMgr::IDoLink(plLinkToAgeMsg* msg)
{
    plNetClientMgr* nc = plNetClientMgr::GetInstance();
    GetAgeLink()->SetSpawnPoint(msg->GetAgeLink()->SpawnPoint());

    if (fLinkedIn) {
        // Set the link out animation we should use
        if (plSceneObject *localSO = plSceneObject::ConvertNoRef(nc->GetLocalPlayer())) {
            plArmatureMod *avMod = const_cast<plArmatureMod*>(plArmatureMod::ConvertNoRef(localSO->GetModifierByType(plArmatureMod::Index())));
            avMod->SetLinkInAnim(msg->GetLinkInAnimName());
        }
        // Queue leave op
        NlmLeaveAgeOp * leaveAgeOp = new NlmLeaveAgeOp;
        leaveAgeOp->muteSfx = !msg->PlayLinkOutSfx();
        QueueOp(leaveAgeOp);
    }

    // Queue join op
    NlmJoinAgeOp * joinAgeOp = new NlmJoinAgeOp;
    joinAgeOp->age.ageInstId = *GetAgeLink()->GetAgeInfo()->GetAgeInstanceGuid();
    joinAgeOp->muteSfx = !msg->PlayLinkInSfx();
    joinAgeOp->age.ageDatasetName = GetAgeLink()->GetAgeInfo()->GetAgeFilename();
    StrCopy(
        joinAgeOp->age.spawnPtName,
        GetAgeLink()->SpawnPoint().GetName().c_str(),
        std::size(joinAgeOp->age.spawnPtName)
        );
    QueueOp(joinAgeOp);

    // UnRef
    msg->UnRef();
}

////////////////////////////////////////////////////////////////////

bool plNetLinkingMgr::IProcessLinkingMgrMsg( plLinkingMgrMsg * msg )
{
    plNetClientMgr * nc = plNetClientMgr::GetInstance();

    bool result = true;

    switch ( msg->GetCmd() )
    {
    /////////////////////
    case kLinkPlayerHere:
        {
            // player wants to link to our age
            uint32_t playerID = msg->GetArgs()->GetInt( 0 );
            hsLogEntry( nc->DebugMsg( "Linking player {} to this age.", playerID ) );
            LinkPlayerHere( playerID );
        }
        break;
        
    /////////////////////
    case kLinkPlayerToPrevAge:
        {
            // link myself back to my last age
            hsLogEntry( nc->DebugMsg( "Linking back to my last age.") );
            LinkToPrevAge();
        }
        break;
    /////////////////////
    case kOfferLinkToPlayer:
        {
//          // Notify the KI that we received an offer.
//          plVaultNotifyMsg * notify = new plVaultNotifyMsg();
//          notify->SetType( plVaultNotifyMsg::kPlayerOfferedLink );
//          notify->GetArgs()->AddItem( 0, msg->GetArgs()->GetItem( 0 ), true );    // add to notify and have notify take over memory management of the item.
//          msg->GetArgs()->RemoveItem( 0, true );  // msg to stop memory managing item, notify msg will delete it.
//          notify->Send();

            plAgeLinkStruct *myLink = plAgeLinkStruct::ConvertNoRef(msg->GetArgs()->GetItem( 0 ));
            LinkToAge(myLink);
        }
        break;

    /////////////////////
    default:
        hsAssert( false, "IProcessLinkingMgrMsg: Unknown linking mgr cmd." );
        result = false;
        break;
    }

    return result;
}


////////////////////////////////////////////////////////////////////

bool plNetLinkingMgr::IProcessVaultNotifyMsg(plVaultNotifyMsg* msg) 
{
    // No deferred link? Bye bye.
    if (fDeferredLink == nullptr)
        return false;

    plAgeLinkStruct* cur = GetAgeLink();
    hsRef<RelVaultNode> cVaultLink;
    switch (msg->GetType())
    {
        case plVaultNotifyMsg::kRegisteredChildAgeLink:
        case plVaultNotifyMsg::kRegisteredOwnedAge:
        case plVaultNotifyMsg::kRegisteredSubAgeLink:
            cVaultLink = VaultGetNode(msg->GetArgs()->GetInt(plNetCommon::VaultTaskArgs::kAgeLinkNode));
            break;
        default:
            return false;
    }

    if (cVaultLink != nullptr) {
        // Verify that if the Age vault already exists that it matches the requested
        // deferred link. If it doesn't exist, well, I hope you're happy with the
        // result.
        VaultAgeLinkNode accLink(cVaultLink);
        if (hsRef<RelVaultNode> rvnInfo = cVaultLink->GetChildNode(plVault::kNodeType_AgeInfo, 1)) {
            plAgeInfoStruct dest;
            VaultAgeInfoNode accInfo(rvnInfo);
            accInfo.CopyTo(&dest);
            if (!dest.IsEqualTo(fDeferredLink->GetAgeLink()->GetAgeInfo())) {
                hsLogEntry(
                    plNetClientMgr::GetInstance()->DebugMsg(
                        "Waiting for a deferred link to '{}' but got AgeInfo for '{}' instead.",
                        fDeferredLink->GetAgeLink()->AsString(),
                        dest.AsString()
                    );
                );
                return false;
            }
        }

        // If we're still here, that means the links match. Set the vault copy as our current
        // AgeLink and AgeInfo. Note this mainly copies the AgeInfo.
        accLink.CopyTo(cur);
        hsLogEntry(
            plNetClientMgr::GetInstance()->DebugMsg(
                "Performing deferred link to '{}'",
                cur->AsString()
            );
        );

        // Steals fDeferredLink
        IDoLink(fDeferredLink);
        fDeferredLink = nullptr;
        return true;
    } else {
        hsLogEntry(
            plNetClientMgr::GetInstance()->ErrorMsg(
                "Waiting for a deferred link to '{}' but got a garbage link?",
                fDeferredLink->GetAgeLink()->AsString()
            )
        );
    }

    return false;
}

////////////////////////////////////////////////////////////////////

bool plNetLinkingMgr::IDispatchMsg( plMessage* msg, uint32_t playerID )
{
    plNetClientMgr * nc = plNetClientMgr::GetInstance();
    msg->AddReceiver( plNetClientMgr::GetInstance()->GetKey() );

    plLinkToAgeMsg* linkToAge = plLinkToAgeMsg::ConvertNoRef(msg);
    if ( playerID!=kInvalidPlayerID && playerID!=nc->GetPlayerID() )
    {
        msg->SetBCastFlag( plMessage::kNetAllowInterAge );
        msg->SetBCastFlag( plMessage::kNetPropagate );
        msg->SetBCastFlag( plMessage::kNetForce );
        msg->SetBCastFlag( plMessage::kLocalPropagate, 0 );
        // send msg to other player (maybe in different age than us)
        msg->AddNetReceiver( playerID );
    }

    return msg->Send();
}

////////////////////////////////////////////////////////////////////

void plNetLinkingMgr::LinkToAge( plAgeLinkStruct * link, bool linkInSfx, bool linkOutSfx, uint32_t playerID )
{
    LinkToAge(link, {}, linkInSfx, linkOutSfx, playerID);
}

void plNetLinkingMgr::LinkToAge( plAgeLinkStruct * link, const ST::string& linkAnim, bool linkInSfx, bool linkOutSfx, uint32_t playerID )
{
    if ( !fLinkingEnabled )
    {
        hsLogEntry( plNetClientMgr::GetInstance()->DebugMsg( "Not linking. Linking is disabled." ) );
        return;
    }

    plLinkToAgeMsg* pMsg = new plLinkToAgeMsg( link );
    pMsg->SetLinkInAnimName(linkAnim);
    pMsg->PlayLinkSfx(linkInSfx, linkOutSfx);
    IDispatchMsg( pMsg, playerID );
}

// link myself back to my last age
void plNetLinkingMgr::LinkToPrevAge( uint32_t playerID )
{
    if ( !fLinkingEnabled )
    {
        hsLogEntry( plNetClientMgr::GetInstance()->DebugMsg( "Not linking. Linking is disabled." ) );
        return;
    }

    if (GetPrevAgeLink()->GetAgeInfo()->HasAgeFilename())
    {
        plLinkToAgeMsg* pMsg = new plLinkToAgeMsg( GetPrevAgeLink() );
        IDispatchMsg( pMsg, playerID );
    }
    else
    {
        hsLogEntry( plNetClientMgr::GetInstance()->DebugMsg( "Not linking. No prev age fileName." ) );
    }
}

void plNetLinkingMgr::LinkToMyPersonalAge( uint32_t playerID )
{
    if ( !fLinkingEnabled )
    {
        hsLogEntry( plNetClientMgr::GetInstance()->DebugMsg( "Not linking. Linking is disabled." ) );
        return;
    }
    plNetClientMgr* nc = plNetClientMgr::GetInstance();

    plAgeLinkStruct link;
    link.GetAgeInfo()->SetAgeFilename( kPersonalAgeFilename );
    link.SetLinkingRules( plNetCommon::LinkingRules::kOwnedBook );

    plSpawnPointInfo hutSpawnPoint;
    hutSpawnPoint.SetName(kPersonalAgeLinkInPointCloset);
    link.SetSpawnPoint(hutSpawnPoint);

    plLinkToAgeMsg* pMsg = new plLinkToAgeMsg( &link );
    IDispatchMsg( pMsg, playerID );
}

void plNetLinkingMgr::LinkToMyNeighborhoodAge( uint32_t playerID )
{
    if ( !fLinkingEnabled )
    {
        hsLogEntry( plNetClientMgr::GetInstance()->DebugMsg( "Not linking. Linking is disabled." ) );
        return;
    }
    plNetClientMgr * nc = plNetClientMgr::GetInstance();

    plAgeLinkStruct link;
    if (!VaultGetLinkToMyNeighborhood(&link))
        return;
        
    link.SetLinkingRules( plNetCommon::LinkingRules::kOwnedBook );

    plLinkToAgeMsg* pMsg = new plLinkToAgeMsg( &link );
    IDispatchMsg( pMsg, playerID );
}

void plNetLinkingMgr::LinkPlayerHere( uint32_t playerID )
{
    if ( !fLinkingEnabled )
    {
        hsLogEntry( plNetClientMgr::GetInstance()->DebugMsg( "Not linking. Linking is disabled." ) );
        return;
    }
    plNetClientMgr * nc = plNetClientMgr::GetInstance();

    // send the player our current age info so they can link here.
    plAgeLinkStruct link;
    link.GetAgeInfo()->CopyFrom( GetAgeLink()->GetAgeInfo() );
    LinkPlayerToAge( &link, playerID );
}

void plNetLinkingMgr::LinkPlayerToAge( plAgeLinkStruct * link, uint32_t playerID )
{
    if ( !fLinkingEnabled )
    {
        hsLogEntry( plNetClientMgr::GetInstance()->DebugMsg( "Not linking. Linking is disabled." ) );
        return;
    }
    plNetClientMgr * nc = plNetClientMgr::GetInstance();

    // send the player the age link so they can link there.
    link->SetLinkingRules( plNetCommon::LinkingRules::kBasicLink );
    plLinkToAgeMsg* pMsg = new plLinkToAgeMsg( link );
    IDispatchMsg( pMsg, playerID );
}

//
// link the player back to his previous age
//
void plNetLinkingMgr::LinkPlayerToPrevAge( uint32_t playerID )
{
    if ( !fLinkingEnabled )
    {
        hsLogEntry( plNetClientMgr::GetInstance()->DebugMsg( "Not linking. Linking is disabled." ) );
        return;
    }

    // Send the player a msg telling them to link to their last age
    plNetClientMgr * nc = plNetClientMgr::GetInstance();

    plLinkingMgrMsg* pMsg = new plLinkingMgrMsg();
    pMsg->SetCmd( kLinkPlayerToPrevAge);
    IDispatchMsg( pMsg, playerID );
}

void plNetLinkingMgr::LinkToPlayersAge( uint32_t playerID )
{
    if ( !fLinkingEnabled )
    {
        hsLogEntry( plNetClientMgr::GetInstance()->DebugMsg( "Not linking. Linking is disabled." ) );
        return;
    }
    // Send the player a msg telling them to send us a msg to link to them. isn't that fun? :)
    plNetClientMgr * nc = plNetClientMgr::GetInstance();

    plLinkingMgrMsg* pMsg = new plLinkingMgrMsg();
    pMsg->SetCmd( kLinkPlayerHere );
    pMsg->GetArgs()->AddInt( 0, NetCommGetPlayer()->playerInt );    // send them our id.
    IDispatchMsg( pMsg, playerID );
}

////////////////////////////////////////////////////////////////////


void plNetLinkingMgr::OfferLinkToPlayer( const plAgeLinkStruct * inInfo, uint32_t playerID )
{

    plNetClientMgr* mgr = plNetClientMgr::GetInstance();
    plNetTransport& transport = mgr->TransportMgr();
    plNetTransportMember* guestMem = transport.GetMemberByID(playerID);

    if (guestMem) {
        plLinkToAgeMsg* linkM = new plLinkToAgeMsg(inInfo);
        linkM->AddReceiver(mgr->GetKey());

        plKey host = mgr->GetLocalPlayerKey();
        plKey guest = guestMem->GetAvatarKey();
        plAvatarMgr::OfferLinkingBook(host, guest, linkM, host);
    }
}
// my special version - cjp
void plNetLinkingMgr::OfferLinkToPlayer( const plAgeLinkStruct * inInfo, uint32_t playerID, const plKey& replyKey )
{
    plNetClientMgr *mgr = plNetClientMgr::GetInstance();

    plNetTransport &transport = mgr->TransportMgr();
    plNetTransportMember* guestMem = transport.GetMemberByID(playerID);
    if (guestMem)
    {
        plLinkToAgeMsg* linkM = new plLinkToAgeMsg(inInfo);
        linkM->AddReceiver(mgr->GetKey());

        plKey guest = guestMem->GetAvatarKey();
        plKey host = mgr->GetLocalPlayerKey();
        plAvatarMgr::OfferLinkingBook(host, guest, linkM, replyKey);
    }
}

// for backwards compatibility
void plNetLinkingMgr::OfferLinkToPlayer( const plAgeInfoStruct * inInfo, uint32_t playerID )
{
    plAgeLinkStruct ageLink;

    ageLink.GetAgeInfo()->CopyFrom(inInfo);
    ageLink.SetLinkingRules(plNetCommon::LinkingRules::kBasicLink);
    OfferLinkToPlayer(&ageLink, playerID);
}

////////////////////////////////////////////////////////////////////

void plNetLinkingMgr::IPostProcessLink()
{
    // Grab some useful things...
    plAgeLinkStruct* link = GetAgeLink();
    plAgeInfoStruct* info = link->GetAgeInfo();

    bool city = (info->GetAgeFilename().compare_i(kCityAgeFilename) == 0);

    // Update our online status 
    if (hsRef<RelVaultNode> rvnInfo = VaultGetPlayerInfoNode()) {
        VaultPlayerInfoNode accInfo(rvnInfo);
        plUUID ageInstGuid = *info->GetAgeInstanceGuid();
        accInfo.SetAgeInstName(info->GetAgeInstanceName());
        accInfo.SetAgeInstUuid(ageInstGuid);
        accInfo.SetOnline(true);
    }
    
    switch (link->GetLinkingRules()) {

        case plNetCommon::LinkingRules::kOwnedBook: {
            // SPECIAL CASE: City: Every player ever created would be in the list; avoid that.
            if (city)
                break;
                
            {   // Ensure we're in the AgeOwners folder
                hsRef<RelVaultNode> fldr = VaultGetAgeAgeOwnersFolder();
                hsRef<RelVaultNode> info = VaultGetPlayerInfoNode();
                
                if (fldr && info)
                    if (!fldr->IsParentOf(info->GetNodeId(), 1))
                        VaultAddChildNode(
                            fldr->GetNodeId(),
                            info->GetNodeId(),
                            NetCommGetPlayer()->playerInt,
                            nullptr,
                            nullptr
                        );
            }
        }
        break;  

        case plNetCommon::LinkingRules::kVisitBook: {
            // SPECIAL CASE: City: Every player ever created would be in the list; avoid that.
            if (city)
                break;
                
            {   // Ensure we're in the CanVisit folder
                hsRef<RelVaultNode> fldr = VaultGetAgeCanVisitFolder();
                hsRef<RelVaultNode> info = VaultGetPlayerInfoNode();
                
                if (fldr && info)
                    if (!fldr->IsParentOf(info->GetNodeId(), 1))
                        VaultAddChildNode(
                            fldr->GetNodeId(),
                            info->GetNodeId(),
                            NetCommGetPlayer()->playerInt,
                            nullptr,
                            nullptr
                        );
            }
        }
        break;
        
        case plNetCommon::LinkingRules::kSubAgeBook: {
            // Register the previous age as a sub age of the current one so that we can link back to that instance
            plAgeLinkStruct subAgeLink;
            VaultAgeFindOrCreateSubAgeLink(GetPrevAgeLink()->GetAgeInfo(), &subAgeLink, NetCommGetAge()->ageInstId);
        }
        break;
    }
}

////////////////////////////////////////////////////////////////////

uint8_t plNetLinkingMgr::IPreProcessLink()
{
    // Grab some stuff we're gonna use extensively
    plNetClientMgr* nc = plNetClientMgr::GetInstance();
    plAgeLinkStruct* link = GetAgeLink();
    plAgeInfoStruct* info = link->GetAgeInfo();

    PreProcessResult success = kLinkImmediately;

    if ( nc->GetFlagsBit( plNetClientMgr::kNullSend ) )
    {
        hsLogEntry( nc->DebugMsg( "NetClientMgr nullsend. Not linking." ) );
        return kLinkFailed;
    }

#if 0
    // Appear offline until we're done linking
    if (hsRef<RelVaultNode> rvnInfo = VaultGetPlayerInfoNode()) {
        VaultPlayerInfoNode accInfo(rvnInfo);
        accInfo.SetAgeInstName(nullptr);
        accInfo.SetAgeInstUuid(kNilUuid);
        accInfo.SetOnline(false);
    }
#else
    // Update our online status 
    if (hsRef<RelVaultNode> rvnInfo = VaultGetPlayerInfoNode()) {
        VaultPlayerInfoNode accInfo(rvnInfo);
        const plUUID* ageInstGuid = info->GetAgeInstanceGuid();
        accInfo.SetAgeInstName(info->GetAgeInstanceName());
        accInfo.SetAgeInstUuid(*ageInstGuid);
        accInfo.SetOnline(true);
    }
#endif

    //------------------------------------------------------------------------
    // Fixup empty fields
    if (info->HasAgeFilename())
    {
        info->SetAgeFilename(plNetLinkingMgr::GetProperAgeName(info->GetAgeFilename()));

        if (!info->HasAgeInstanceName())
        {
            info->SetAgeInstanceName(info->GetAgeFilename());
        }
    }

    hsLogEntry(nc->DebugMsg( "plNetLinkingMgr: Pre-Process: Linking with {} rules...",
        plNetCommon::LinkingRules::LinkingRuleStr(link->GetLinkingRules())));

    //------------------------------------------------------------------------
    // SPECIAL CASE: StartUp: force basic link
    if (info->GetAgeFilename().compare_i(kStartUpAgeFilename) == 0)
        link->SetLinkingRules( plNetCommon::LinkingRules::kBasicLink );

    //------------------------------------------------------------------------
    // SPECIAL CASE: Nexus: force original link
    if (info->GetAgeFilename().compare_i(kNexusAgeFilename) == 0)
        link->SetLinkingRules(plNetCommon::LinkingRules::kOriginalBook);

    //------------------------------------------------------------------------
    // SPECIAL CASE: ACA: force original link
    if (info->GetAgeFilename().compare_i(kAvCustomizationFilename) == 0)
        link->SetLinkingRules(plNetCommon::LinkingRules::kOriginalBook);

    hsLogEntry(nc->DebugMsg("plNetLinkingMgr: Process: Linking with {} rules...",
        plNetCommon::LinkingRules::LinkingRuleStr(link->GetLinkingRules())));

    switch (link->GetLinkingRules())
    {
        //--------------------------------------------------------------------
        // BASIC LINK. Link to a unique instance of the age, if no instance specified.
        case plNetCommon::LinkingRules::kBasicLink:
            if (!info->HasAgeInstanceGuid()) {
                plUUID newuuid = plUUID::Generate();
                info->SetAgeInstanceGuid(&newuuid);
            }
        break;

        //--------------------------------------------------------------------
        // ORIGINAL BOOK.  Become an owner of the age, if not already one.
        case plNetCommon::LinkingRules::kOriginalBook:
            {
                // create a new ageinfo struct with the filename of the age because
                // we just want to find out if we own *any* link to the age, not the specific
                // link that we're linking through
                plAgeInfoStruct ageInfo;
                ageInfo.SetAgeFilename(info->GetAgeFilename());

                plAgeLinkStruct ownedLink;
                if (!VaultGetOwnedAgeLink(&ageInfo, &ownedLink)) {
                    // Fill in fields for new age create.
                    if (!info->HasAgeUserDefinedName())
                    {
                        // set user-defined name
                        ST::string title;
                        if (nc->GetPlayerName().to_lower().back() == 's')
                            title = ST::format("{}'", nc->GetPlayerName());
                        else
                            title = ST::format("{}'s", nc->GetPlayerName());
                        info->SetAgeUserDefinedName(title.c_str());
                    }
                    if (!info->HasAgeDescription())
                    {
                        // set description
                        ST::string desc;
                        if (nc->GetPlayerName().to_lower().back() == 's')
                            desc = ST::format("{}' {}", nc->GetPlayerName(), info->GetAgeInstanceName());
                        else
                            desc = ST::format("{}'s {}", nc->GetPlayerName(), info->GetAgeInstanceName());
                        info->SetAgeDescription(desc.c_str());
                    }
                    if (!info->HasAgeInstanceGuid()) {
                        plUUID newuuid = plUUID::Generate();
                        info->SetAgeInstanceGuid(&newuuid);
                    }
                    
                    // register this as an owned age now before we link to it.
                    // Note: We MUST break or the OwnedBook code will fail!
                    VaultRegisterOwnedAge(link);
                    success = kLinkDeferred;
                    break;
                }
                else if (hsRef<RelVaultNode> linkNode = VaultGetOwnedAgeLink(&ageInfo)) {
                    // We have the age in our AgesIOwnFolder. If its volatile, dump it for the new one.
                    VaultAgeLinkNode linkAcc(linkNode);
                    if (linkAcc.GetVolatile()) {
                        if (VaultUnregisterOwnedAge(&ageInfo)) {
                            // Fill in fields for new age create.
                            if (!info->HasAgeUserDefinedName())
                            {
                                // set user-defined name
                                ST::string title;
                                if (nc->GetPlayerName().to_lower().back() == 's')
                                    title = ST::format("{}'", nc->GetPlayerName());
                                else
                                    title = ST::format("{}'s", nc->GetPlayerName());
                                info->SetAgeUserDefinedName(title.c_str());
                            }

                            if (!info->HasAgeDescription())
                            {
                                // set description
                                ST::string desc;
                                if (nc->GetPlayerName().to_lower().back() == 's')
                                    desc = ST::format("{}' {}", nc->GetPlayerName(), info->GetAgeInstanceName());
                                else
                                    desc = ST::format("{}'s {}", nc->GetPlayerName(), info->GetAgeInstanceName());
                                info->SetAgeDescription( desc.c_str() );
                            }

                            if (!info->HasAgeInstanceGuid()) {
                                plUUID newuuid = plUUID::Generate();
                                info->SetAgeInstanceGuid(&newuuid);
                            }

                            VaultRegisterOwnedAge(link);

                            // Note: We MUST break or the OwnedBook code will fail!
                            success = kLinkDeferred;
                            break;
                        }
                    }
                    else {
                        if (info->GetAgeFilename().compare_i(kNeighborhoodAgeFilename) == 0) {
                            // if we get here then it's because we're linking to a neighborhood that we don't belong to
                            // and our own neighborhood book is not volatile, so really we want to basic link
                            link->SetLinkingRules(plNetCommon::LinkingRules::kBasicLink);
                            success = kLinkImmediately;
                            break;
                        }
                    }
                }
            }

            link->SetLinkingRules(plNetCommon::LinkingRules::kOwnedBook);
            // falls through
            // to OWNED BOOK case...

        //--------------------------------------------------------------------
        // OWNED BOOK. Look for the book in our AgesIOwn folder
        case plNetCommon::LinkingRules::kOwnedBook:
            {
                plAgeLinkStruct ownedLink;
                if (VaultGetOwnedAgeLink(info, &ownedLink)) {
                    info->CopyFrom(ownedLink.GetAgeInfo());
                    // Remember spawn point (treasure book support)                     
                    plSpawnPointInfo theSpawnPt = link->SpawnPoint();
                    VaultAddOwnedAgeSpawnPoint(*info->GetAgeInstanceGuid(), theSpawnPt);
                }
                else {
                    success = kLinkFailed;
                }
            }
            break;

        //--------------------------------------------------------------------
        // VISIT BOOK. Look for the book in our AgesICanVisit folder
        case plNetCommon::LinkingRules::kVisitBook:
            {
                plAgeLinkStruct visitLink;
                if (VaultGetVisitAgeLink(info, &visitLink))
                    info->CopyFrom(visitLink.GetAgeInfo());
                else
                    success = kLinkFailed;
            }
            break;

        //--------------------------------------------------------------------
        // SUB-AGE BOOK: Look for an existing sub-age in this age's SubAges folder and use it if found.
        //  plNetClientTaskHandler will add a SubAge back link to our current age once we arrive there.
        case plNetCommon::LinkingRules::kSubAgeBook:
            {
                plAgeLinkStruct subAgeLink;
                if (VaultAgeFindOrCreateSubAgeLink(info, &subAgeLink, NetCommGetAge()->ageInstId))
                    info->CopyFrom(subAgeLink.GetAgeInfo());
                else
                    success = kLinkDeferred;
            }
            break;

        //--------------------------------------------------------------------
        // CHILD-AGE BOOK: Look for an existing child-age in this parent ageinfo ChildAges folder and use it if found.
        //  plNetClientTaskHandler will add a ChildAge back link to our current age once we arrive there.
        case plNetCommon::LinkingRules::kChildAgeBook:
            {
                plAgeLinkStruct childLink;
                switch(VaultAgeFindOrCreateChildAgeLink(
                          link->GetParentAgeFilename(),
                          info,
                          &childLink))
                {
                    case plVaultChildAgeLinkResult::kFailed:
                        success = kLinkFailed;
                        break;
                    case plVaultChildAgeLinkResult::kCreatingNew:
                        success = kLinkDeferred;
                        break;
                    case plVaultChildAgeLinkResult::kFoundExisting:
                        success = kLinkImmediately;
                }

                if (success == kLinkImmediately)
                    info->CopyFrom(childLink.GetAgeInfo());
            }
            break;

        //--------------------------------------------------------------------
        // ???
        DEFAULT_FATAL(link->GetLinkingRules());
    }

    hsLogEntry(nc->DebugMsg( "plNetLinkingMgr: Post-Process: Linking with {} rules...",
        plNetCommon::LinkingRules::LinkingRuleStr(link->GetLinkingRules())));

    hsAssert(info->HasAgeFilename(), "AgeLink has no AgeFilename. Link will fail.");

    return success;
}


////////////////////////////////////////////////////////////////////
void plNetLinkingMgr::LeaveAge (bool quitting) {
    // Queue leave op
    NlmLeaveAgeOp * leaveAgeOp = new NlmLeaveAgeOp;
    leaveAgeOp->quitting = quitting;
    QueueOp(leaveAgeOp);

}
 
 


////////////////////////////////////////////////////////////////////
// End.
