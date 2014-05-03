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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/PubUtilLib/plNetClient/plNetCliAgeJoiner.cpp
*
*   Encapsulates all of the horrible ugliness that the age load process has become
*   
***/

#include "plNetCliAgeJoiner.h"
#include "plNetClientMgr.h"
#include "plNetLinkingMgr.h"
#include "plNetObjectDebugger.h"

#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plCoordinateInterface.h"

#include "pnMessage/plPlayerPageMsg.h"
#include "pnMessage/plTimeMsg.h"

#include "plNetClientComm/plNetClientComm.h"
#include "plAgeLoader/plAgeLoader.h"
#include "plAvatar/plArmatureMod.h"
#include "plAvatar/plAvatarMgr.h"
#include "plVault/plVault.h"

#include "plNetMessage/plNetMessage.h"

#include "plMessage/plNetCommMsgs.h"
#include "plMessage/plAgeLoadedMsg.h"
#include "plMessage/plInputIfaceMgrMsg.h"
#include "plMessage/plNetClientMgrMsg.h"
#include "plMessage/plResPatcherMsg.h"

#include "plProgressMgr/plProgressMgr.h"
#include "pnDispatch/plDispatch.h"
#include "plResMgr/plResManager.h"

#include "pnAsyncCore/pnAcLog.h"


/*****************************************************************************
*
*   Private
*
***/

struct plNCAgeJoiner {

    enum NextOp {
        kNoOp,
        kLoadAge,
        kLoadPlayer,
        kRequestAgeState,
        kPropagatePlayer,
        kDestroyProgressBar,
        kEnableClickables,
        kSimStateRcvd,
        kNotifyAgeLoaded,
    };

    NextOp                  nextOp;
    NetCommAge              age;
    FNCAgeJoinerCallback    callback;
    void *                  userState;
    bool                    complete;
    bool                    muteLinkSfx;

    plOperationProgress*    progressBar;

    plNCAgeJoiner (
        const NetCommAge &      age,
        bool                    muteSfx,
        FNCAgeJoinerCallback    callback,
        void *                  userState
    );
    ~plNCAgeJoiner ();

    void Start ();
    void Complete (bool success, const char msg[]);
    bool MsgReceive (plMessage * msg);
    void Update ();
    void ExecNextOp ();

    static void IDispatchMsgReceiveCallback ();
    static void IResMgrProgressBarCallback (plKey key);

    static plNCAgeJoiner* s_instance;
};

plNCAgeJoiner* plNCAgeJoiner::s_instance = nil;


/*****************************************************************************
*
*   Local functions
*
***/

//============================================================================
void AgeVaultDownloadCallback (
    ENetError           result,
    void *              param
) {
    plNCAgeJoiner * joiner = (plNCAgeJoiner *)param;
    if (IS_NET_ERROR(result)) {
        joiner->Complete(false, "Failed to download age vault");
    }
    else {
        // vault downloaded. start loading age data
        LogMsg(kLogPerf, L"AgeJoiner: Next:kLoadAge (vault downloaded)");
        joiner->nextOp = plNCAgeJoiner::kLoadAge;
    }
}


/*****************************************************************************
*
*   plNCAgeJoiner
*
***/

//============================================================================
plNCAgeJoiner::plNCAgeJoiner (
    const NetCommAge &      age,
    bool                    muteSfx,
    FNCAgeJoinerCallback    callback,
    void *                  userState
) : nextOp(kNoOp)
,   age(age)
,   muteLinkSfx(muteSfx)
,   callback(callback)
,   userState(userState)
,   complete(false)
,   progressBar(nil)
{
}

//============================================================================
plNCAgeJoiner::~plNCAgeJoiner () {
}

//============================================================================
void plNCAgeJoiner::IDispatchMsgReceiveCallback () {
    if (s_instance)
        s_instance->progressBar->Increment(1);
}

//============================================================================
void plNCAgeJoiner::IResMgrProgressBarCallback (plKey key) {
#ifndef PLASMA_EXTERNAL_RELEASE
    if (s_instance)
        s_instance->progressBar->SetStatusText(key->GetName());
#endif
    if (s_instance)
        s_instance->progressBar->Increment(1);
}

//============================================================================
void plNCAgeJoiner::Complete (bool success, const char msg[]) {

    if (!complete) {
        complete = true;
        
        s_instance = nil;
        
        NCAgeJoinerCompleteNotify   notify;
        notify.success  = success;
        notify.msg      = msg;
        
        callback(this, kAgeJoinerComplete, &notify, userState);
        delete this;
    }

}

//============================================================================
void plNCAgeJoiner::Start () {
    s_instance = this;

    plNetClientMgr * nc = plNetClientMgr::GetInstance();
    nc->SetFlagsBit(plNetClientMgr::kPlayingGame, false);
    nc->fServerTimeOffset               = 0;    // reset since we're connecting to a new server
    nc->fRequiredNumInitialSDLStates    = 0;
    nc->fNumInitialSDLStates            = 0;
    nc->SetFlagsBit(plNetClientApp::kNeedInitialAgeStateCount);
    nc->SetFlagsBit(plNetClientApp::kLoadingInitialAgeState);

    // if we're linking to startup then set the OfflineAge flag
    // so we by-pass the game server
    if (StrLen(age.ageDatasetName) == 0 || StrCmpI(age.ageDatasetName, "StartUp") == 0) {
        nc->SetFlagsBit(plNetClientApp::kLinkingToOfflineAge);

        // no need to update if we're not using a GameSrv
        plgDispatch::MsgSend(new plResPatcherMsg());
    } else {
        nc->SetFlagsBit(plNetClientApp::kLinkingToOfflineAge, false);

        // we only need to update the age if we're using a GameSrv
        plAgeLoader* al = plAgeLoader::GetInstance();
        al->UpdateAge(age.ageDatasetName);
    }
}

//============================================================================
void plNCAgeJoiner::ExecNextOp () {
    plNetClientMgr *    nc = plNetClientMgr::GetInstance();
    plAvatarMgr *       am = plAvatarMgr::GetInstance();
    plAgeLoader *       al = plAgeLoader::GetInstance();

    NextOp next = nextOp;
    nextOp      = kNoOp;
    switch (next) {
        //====================================================================
        case kNoOp: {
        }
        break;
            
        //====================================================================
        case kLoadAge: {
            LogMsg(kLogPerf, L"AgeJoiner: Exec:kLoadAge");

            // Start progress bar
            plString str;
#ifndef PLASMA_EXTERNAL_RELEASE
            str = plString::Format("Loading age... %s", age.ageDatasetName);
#endif
            progressBar = plProgressMgr::GetInstance()->RegisterOperation(0, str.c_str(), plProgressMgr::kNone, false, true);
            plDispatch::SetMsgRecieveCallback(IDispatchMsgReceiveCallback);
            ((plResManager*)hsgResMgr::ResMgr())->SetProgressBarProc(IResMgrProgressBarCallback);

            // Start loading age data
            al->LoadAge(age.ageDatasetName);
        }
        break;

        //====================================================================
        case kLoadPlayer: {
            LogMsg(kLogPerf, L"AgeJoiner: Exec:kLoadPlayer");
            // Start loading local player
            const char * avatarName;
            if (NetCommNeedToLoadAvatar()) {
                if (nc->GetFlagsBit(plNetClientApp::kLinkingToOfflineAge))
                    avatarName = "Male";
                else
                    avatarName = NetCommGetPlayer()->avatarDatasetName;
                plString linkInName = plNetLinkingMgr::GetInstance()->GetAgeLink()->SpawnPoint().GetName();
                am->LoadPlayer(avatarName, "", linkInName);
            }
            else {
                LogMsg(kLogPerf, L"AgeJoiner: Next:kPropagatePlayer");
                nextOp = kPropagatePlayer;
            }
        }
        break;

        //====================================================================
        case kPropagatePlayer: {
            LogMsg(kLogPerf, L"AgeJoiner: Exec:kPropagatePlayer");
            // Add our avatar to the scene
            int spawnPt = am->FindSpawnPoint(age.spawnPtName);
            nc->IPlayerChangeAge(false /*not exiting*/, spawnPt);

            if (!nc->GetFlagsBit(plNetClientApp::kLinkingToOfflineAge))
                // Add our avatar to the game state
                am->PropagateLocalPlayer(spawnPt);
            
            LogMsg(kLogPerf, L"AgeJoiner: Next:kRequestAgeState");
            nextOp = kRequestAgeState;
        }
        break;

        //============================================================================
        case kRequestAgeState: {
            LogMsg(kLogPerf, L"AgeJoiner: Exec:kRequestAgeState");
            if (nc->GetFlagsBit(plNetClientApp::kLinkingToOfflineAge)) {
                LogMsg(kLogPerf, L"AgeJoiner: Next:kSimStateRcvd");
                nextOp = kSimStateRcvd;
            }
            else {
                // Request age player list
                nc->ISendMembersListRequest();

                // Request initial SDL state
                plNetMsgGameStateRequest gsmsg;
                gsmsg.SetNetProtocol(kNetProtocolCli2Game);
                gsmsg.SetBit(plNetMessage::kInitialAgeStateRequest);
                nc->SendMsg(&gsmsg);
                
                // Send our avatar settings
                nc->SendLocalPlayerAvatarCustomizations();
            }
        }
        break;

        //====================================================================
        case kSimStateRcvd: {
            nc->NotifyRcvdAllSDLStates();
        }
        break;

        //============================================================================
        case kDestroyProgressBar: {
            plDispatch::SetMsgRecieveCallback(nil);
            ((plResManager*)hsgResMgr::ResMgr())->SetProgressBarProc(nil);
            delete progressBar;
            progressBar = nil;
            nc->EndTask();

            nextOp = kEnableClickables;
        }
        break;

        //====================================================================
        case kEnableClickables: {
            LogMsg(kLogPerf, L"AgeJoiner: Exec:kEnableClickables");
            // Enable scene clickables
            (void)(new plInputIfaceMgrMsg(plInputIfaceMgrMsg::kEnableClickables))->Send();

            LogMsg(kLogPerf, L"AgeJoiner: Next:kNotifyAgeLoaded");
            nextOp = kNotifyAgeLoaded;
        }
        break;
        
        //====================================================================
        case kNotifyAgeLoaded: {
            LogMsg(kLogPerf, L"AgeJoiner: Exec:kNotifyAgeLoaded");
            nc->SetFlagsBit(plNetClientApp::kPlayingGame);
            nc->SetFlagsBit(plNetClientApp::kNeedToSendInitialAgeStateLoadedMsg);
            plAgeLoader::GetInstance()->NotifyAgeLoaded(true);
            nextOp = kNoOp;
        }
        break;

        DEFAULT_FATAL(nextOp);
    }
}

//============================================================================
bool plNCAgeJoiner::MsgReceive (plMessage * msg) {
    plNetClientMgr *    nc = plNetClientMgr::GetInstance();
    plAvatarMgr *       am = plAvatarMgr::GetInstance();
    plAgeLoader *       al = plAgeLoader::GetInstance();

    //========================================================================
    // Finished updating the age from FileSrv
    //========================================================================
    if (plResPatcherMsg * resMsg = plResPatcherMsg::ConvertNoRef(msg)) {

        if (resMsg->Success())
        {
            nc->ResetServerTimeOffset();
            NetCommLinkToAge(
                age,
                this
            );
            LogMsg(kLogPerf, L"AgeJoiner: Next:kNoOp (age updated)");
        } else
            Complete(false, resMsg->GetError().c_str());
        return true;
    }

    //========================================================================
    // Connected to age instance
    //========================================================================
    if (plNetCommLinkToAgeMsg * linkToAgeMsg = plNetCommLinkToAgeMsg::ConvertNoRef(msg)) {
        if (IS_NET_ERROR(linkToAgeMsg->result)) {
            Complete(false, "LinkToAge failed");
        }
        else if (unsigned ageVaultId = NetCommGetAge()->ageVaultId) {
            // Download the age vault
            VaultDownload(
                L"AgeJoin",
                ageVaultId,
                AgeVaultDownloadCallback,
                this,
                nil, // FVaultDownloadProgressCallback
                this
            );
        }
        else {
            // not vault to downloaded, just start loading age data
            LogMsg(kLogPerf, L"AgeJoiner: Next:kLoadAge (no vault)");
            nextOp = kLoadAge;
        }
        return true;
    }

    //========================================================================
    // All age data paged in
    //========================================================================
    if (plAgeLoaded2Msg * ageLoaded2Msg = plAgeLoaded2Msg::ConvertNoRef(msg)) {
        // Exec custom age settings
        al->ExecPendingAgeFniFiles();
        al->ExecPendingAgeCsvFiles();

        LogMsg(kLogPerf, L"AgeJoiner: Next:kLoadPlayer");
        nextOp = kLoadPlayer;
        return true;
    }

    //========================================================================
    // Local avatar loaded
    //========================================================================
    plPlayerPageMsg * playerPageMsg = plPlayerPageMsg::ConvertNoRef(msg);
    if (playerPageMsg && !playerPageMsg->fUnload && playerPageMsg->fPlayer && playerPageMsg->fLocallyOriginated) {

        if (NetCommNeedToLoadAvatar())
            NetCommSetAvatarLoaded();
        
        LogMsg(kLogPerf, L"AgeJoiner: Next:kPropagatePlayer");
        nextOp = kPropagatePlayer;
        return false;   // NetClientMgr must also handle this message
    }
    
    //========================================================================
    // Received all SDL states
    //========================================================================
    plNetClientMgrMsg * netClientMgrMsg = plNetClientMgrMsg::ConvertNoRef(msg);
    if (netClientMgrMsg && netClientMgrMsg->type == plNetClientMgrMsg::kNotifyRcvdAllSDLStates) {
        LogMsg(kLogPerf, L"AgeJoiner: Next:kEnableClickables");
        nextOp = kDestroyProgressBar;
        return true;
    }

    //========================================================================
    // Done loading all states. Time to link in!
    //========================================================================
    plInitialAgeStateLoadedMsg * stateMsg = plInitialAgeStateLoadedMsg::ConvertNoRef(msg);
    if(stateMsg) {
        plNetObjectDebugger::GetInstance()->LogMsg("OnServerInitComplete");
        nc->SetFlagsBit(plNetClientApp::kLoadingInitialAgeState, false);

        const plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();

        plLinkEffectsTriggerMsg* lem = new plLinkEffectsTriggerMsg();
        lem->SetLeavingAge(false);  // linking in
        lem->SetLinkKey(nc->GetLocalPlayerKey());
        plKey animKey = avMod->GetLinkInAnimKey();
        lem->SetLinkInAnimKey(animKey);

        // indicate if we are invisible
        if (avMod && avMod->IsInStealthMode() && avMod->GetTarget(0))
            lem->SetInvisLevel(avMod->GetStealthLevel());

        lem->SetBCastFlag(plMessage::kNetPropagate);
        lem->MuteLinkSfx(muteLinkSfx);
        lem->AddReceiver(hsgResMgr::ResMgr()->FindKey(plUoid(kLinkEffectsMgr_KEY)));
        lem->AddReceiver(hsgResMgr::ResMgr()->FindKey(plUoid(kClient_KEY)));
        lem->Send();

        Complete(true, "Age joined");
        return true;
    }
    
    return false;
}

//============================================================================
void plNCAgeJoiner::Update () {
    ExecNextOp();
}



/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
void NCAgeJoinerCreate (
    plNCAgeJoiner **        pjoiner,
    const NetCommAge &      age,
    bool                    muteSfx,
    FNCAgeJoinerCallback    callback,
    void *                  userState
) {
    ASSERT(pjoiner);
    ASSERT(callback);
    
    plNCAgeJoiner * joiner;
    *pjoiner = joiner = new plNCAgeJoiner(
        age,
        muteSfx,
        callback,
        userState
    );
    joiner->Start();
}

//============================================================================
bool NCAgeJoinerMsgReceive (
    plNCAgeJoiner *         joiner,
    plMessage *             msg
) {
    ASSERT(joiner);
    return joiner->MsgReceive(msg);
}

//============================================================================
void NCAgeJoinerUpdate (
    plNCAgeJoiner *     joiner
) {
    ASSERT(joiner);
    joiner->Update();
}
