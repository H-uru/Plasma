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
*   $/Plasma20/Sources/Plasma/PubUtilLib/plNetClientComm/plNetClientComm.cpp
*   
***/

#include "plNetClientComm.h"

#include <chrono>
#include <string_view>
#include <thread>

#include "HeadSpin.h"
#include "plProduct.h"
#include "hsResMgr.h"

#include "pnAsyncCore/pnAsyncCore.h"
#include "pnEncryption/plChallengeHash.h"
#include "pnFactory/plFactory.h"
#include "pnNetBase/pnNbConst.h"
#include "pnNetCli/pnNetCli.h"
#include "pnNetCommon/plNetApp.h"
#include "pnUtils/pnUtStr.h"

#include "plFile/plStreamSource.h"
#include "plMessage/plAccountUpdateMsg.h"
#include "plMessage/plNetCommMsgs.h"
#include "plMessage/plNetClientMgrMsg.h"
#include "plNetCommon/plNetCommon.h"
#include "plNetCommon/plNetMsgHandler.h"
#include "plNetGameLib/plNetGameLib.h"
#include "plNetMessage/plNetMessage.h"
#include "plVault/plVault.h"

#include "pfMessage/pfKIMsg.h"

using namespace std::literals::string_view_literals;

extern  bool    gDataServerLocal;

struct NetCommParam {
    void *                          param;
    plNetCommReplyMsg::EParamType   type;
};


/*****************************************************************************
*
*   Private
*
***/


/*****************************************************************************
*
*   Private data
*
***/

static bool                 s_shutdown;

static NetCommAccount       s_account;
static std::vector<NetCommPlayer> s_players;
static NetCommPlayer *      s_player;
static NetCommAge           s_age;
static NetCommAge           s_startupAge;
static bool                 s_needAvatarLoad = true;
static bool                 s_loginComplete = false;
static bool                 s_hasAuthSrvIpAddress = false;
static bool                 s_hasFileSrvIpAddress = false;
static ENetError            s_authResult = kNetErrAuthenticationFailed;
static ST::string           s_authSrvAddr;
static ST::string           s_fileSrvAddr;

static ST::string          s_iniAccountUsername;
static ShaDigest           s_namePassHash;
static char16_t            s_iniAuthToken[kMaxPublisherAuthKeyLength];
static char16_t            s_iniOS[kMaxGTOSIdLength];
static ST::string          s_iniStartupAgeName = ST_LITERAL("StartUp");
static plUUID              s_iniStartupAgeInstId;
static unsigned            s_iniStartupPlayerId = 0;
static bool                s_netError = false;

static plNetMsgHandler* s_msgHandler = nullptr;


//============================================================================
static void INetErrorCallback (
    ENetProtocol    protocol,
    ENetError       error
) {
    NetClientDestroy(false);
    
    plNetClientMgrMsg * msg = new plNetClientMgrMsg(plNetClientMgrMsg::kCmdDisableNet,
                                                    true, nullptr);
    msg->AddReceiver(plNetClientApp::GetInstance()->GetKey());

    switch (error)
    {
    case kNetErrKickedByCCR:
        strncpy(msg->str, "You have been kicked by a CCR.", std::size(msg->str));
        break;

    default:
        // Until we get some real error handling, this'll ensure no errors
        // fall thru the cracks and we hang forever wondering what's up.
        snprintf(
            // buf
            msg->str,
            std::size(msg->str),
            // fmt
            "Network error %u, %S.\n"
            "protocol: %S\n"
            ,// values
            static_cast<unsigned>(error),
            NetErrorToString(error),
            NetProtocolToString(protocol)
        );
        s_netError = true;
    }
    
    msg->Send();
}

//============================================================================
static void IPreInitNetErrorCallback (
    ENetProtocol    protocol,
    ENetError       error
) {
    s_authResult = error;
    s_loginComplete = true;
}

//============================================================================
static void INetBufferCallback (
    unsigned        type,
    unsigned        bytes,
    const uint8_t      buffer[]
) {
    if (!plFactory::IsValidClassIndex(type)) {
        plNetApp::StaticErrorMsg("NetComm: received junk propagated buffer");
        return;
    }
    plNetMessage * msg = plNetMessage::ConvertNoRef(plFactory::Create(type));
    if (!msg) {
        plNetApp::StaticErrorMsg("NetComm: could not convert plNetMessage to class {}", type);
        return;
    }

    hsReadOnlyStream stream(bytes, buffer);
    if (!msg->PeekBuffer(&stream)) {
        plNetApp::StaticErrorMsg("NetComm: plNetMessage %u failed to peek buffer", type);
        return;
    }

    NetCommRecvMsg(msg);

    msg->UnRef();
}

//============================================================================
static void INotifyNewBuildCallback () {

    if (!hsgResMgr::ResMgr())
        return;

    if (!NetCommGetPlayer())
        return;
    if (!NetCommGetPlayer()->playerInt)
        return;
    if (!NetCommGetAge())
        return;
    if (!NetCommGetAge()->ageInstId)
        return;

    pfKIMsg * msg = new pfKIMsg(pfKIMsg::kHACKChatMsg);
    msg->SetString("Uru has been updated. Please quit the game and log back in.");
    msg->SetUser("Updater Service", plNetClientApp::GetInstance()->GetPlayerID());
    msg->SetFlags(pfKIMsg::kAdminMsg);
    msg->SetBCastFlag(plMessage::kNetPropagate | plMessage::kNetForce, 0);
    msg->SetBCastFlag(plMessage::kLocalPropagate, 1);
    msg->Send();
}

//============================================================================
static void INotifyAuthConnectedCallback () {

    if (!hsgResMgr::ResMgr())
        return;
        
    plNetCommAuthConnectedMsg * msg = new plNetCommAuthConnectedMsg;
    msg->Send();
}

//============================================================================
static void PlayerInitCallback (
    ENetError   result,
    void *      param
) {
    if (IS_NET_ERROR(result) && (result != kNetErrVaultNodeNotFound)) {
        s_player = nullptr;
    }
    else {
        // Ensure the city link has the required spawn points
        plAgeInfoStruct info;
        info.SetAgeFilename(kCityAgeFilename);
        if (hsRef<RelVaultNode> rvn = VaultGetOwnedAgeLink(&info)) {
            VaultAgeLinkNode acc(rvn);
            acc.AddSpawnPoint(plSpawnPointInfo(kCityFerryTerminalLinkTitle, kCityFerryTerminalLinkSpawnPtName));
        }
        
        VaultProcessPlayerInbox();
    }

    plNetCommActivePlayerMsg * msg = new plNetCommActivePlayerMsg;
    msg->result     = result;
    msg->param      = param;
    msg->Send();
    
    plAccountUpdateMsg * updateMsg = new plAccountUpdateMsg(plAccountUpdateMsg::kActivePlayer);
    updateMsg->SetPlayerInt(NetCommGetPlayer()->playerInt);
    updateMsg->SetResult((unsigned)result);
    updateMsg->SetBCastFlag(plMessage::kBCastByExactType);
    updateMsg->Send();
}

//============================================================================
static void INetCliAuthSetPlayerRequestCallback (
    ENetError       result,
    void *          param
) {
    if (!s_player) {
        PlayerInitCallback(result, param);
    }
    else if (IS_NET_ERROR(result) && (result != kNetErrVaultNodeNotFound)) {
        s_player = nullptr;
        PlayerInitCallback(result, param);
    }
    else {
        s_needAvatarLoad = true;

        VaultDownloadNoCallbacks(
            "SetActivePlayer",
            s_player->playerInt,
            PlayerInitCallback,
            param,
            nullptr,
            nullptr
        );
    }
}

//============================================================================
static void LoginPlayerInitCallback (
    ENetError                   result,
    void *                      param
) {
    if (IS_NET_ERROR(result) && (result != kNetErrVaultNodeNotFound))
        s_player = nullptr;
    else
        VaultProcessPlayerInbox();

    {
        plNetCommAuthMsg * msg  = new plNetCommAuthMsg;
        msg->result             = result;
        msg->param              = param;
        msg->Send();
    }
    {   
        plNetCommActivePlayerMsg * msg = new plNetCommActivePlayerMsg;
        msg->result     = result;
        msg->param      = param;
        msg->Send();
    }
    {   
        plAccountUpdateMsg * msg = new plAccountUpdateMsg(plAccountUpdateMsg::kActivePlayer);
        msg->SetPlayerInt(NetCommGetPlayer()->playerInt);
        msg->SetResult((unsigned)result);
        msg->SetBCastFlag(plMessage::kBCastByExactType);
        msg->Send();
    }
}

//============================================================================
static void INetCliAuthLoginSetPlayerRequestCallback (
    ENetError       result,
    void *          param
) {
    if (IS_NET_ERROR(result) && (result != kNetErrVaultNodeNotFound)) {
        s_player = nullptr;
        
        plNetCommAuthMsg * msg  = new plNetCommAuthMsg;
        msg->result             = result;
        msg->param              = param;
        msg->Send();
    }
    else {
        VaultDownloadNoCallbacks(
            "SetActivePlayer",
            s_player->playerInt,
            LoginPlayerInitCallback,
            param,
            nullptr,
            nullptr
        );
    }
}

//============================================================================
static void INetCliAuthLoginRequestCallback (
    ENetError                   result,
    void *                      param,
    const plUUID&               accountUuid,
    unsigned                    accountFlags,
    unsigned                    billingType,
    const NetCliAuthPlayerInfo  playerInfoArr[],
    unsigned                    playerCount
) {
    s_authResult = result;

    s_player = nullptr;
    s_players.clear();
    
    bool wantsStartUpAge = (s_startupAge.ageDatasetName.empty() ||
                            s_startupAge.ageDatasetName.compare_i("StartUp") == 0);

    s_loginComplete = true;

    if (!IS_NET_ERROR(result) || result == kNetErrVaultNodeNotFound) {
        s_account.accountUuid   = accountUuid;
        s_account.accountFlags  = accountFlags;
        s_account.billingType   = billingType;
        s_players.resize(playerCount);
        for (unsigned i = 0; i < playerCount; ++i) {
            plNetApp::StaticDebugMsg("Player {}: {} explorer: {}", playerInfoArr[i].playerInt, playerInfoArr[i].playerName, playerInfoArr[i].explorer);
            s_players[i].playerInt         = playerInfoArr[i].playerInt;
            s_players[i].explorer          = playerInfoArr[i].explorer;
            s_players[i].playerName        = playerInfoArr[i].playerName;
            s_players[i].avatarDatasetName = playerInfoArr[i].avatarShape;
            if (!wantsStartUpAge && s_players[i].playerInt == s_iniStartupPlayerId)
                s_player = &s_players[i];
        }

        // store this server's encryption key for posterity
        NetCliAuthGetEncryptionKey(plStreamSource::GetInstance()->GetEncryptionKey(), 4);
    }
    else
        s_account.accountUuid = kNilUuid;

    // If they specified an alternate age, but we couldn't find the player, force
    // the StartUp age to load so that they may select/create a player first.    
    if (!wantsStartUpAge && !s_player)
        s_startupAge.ageDatasetName = "StartUp";

    // If they specified an alternate age, and we found the player, set the active player now
    // so that the link operation will be successful once the client is finished initializing.
    if (!wantsStartUpAge && s_player) {
        NetCliAuthSetPlayerRequest(
            s_player->playerInt,
            INetCliAuthLoginSetPlayerRequestCallback,
            param
        );
    }
}

//============================================================================
static void INetCliAuthCreatePlayerRequestCallback (
    ENetError                       result,
    void *                          param,
    const NetCliAuthPlayerInfo &    playerInfo
) {
    if (IS_NET_ERROR(result)) {
        plNetApp::StaticErrorMsg("Create player failed: {}", NetErrorToString(result));
    }
    else {
        plNetApp::StaticDebugMsg("Created player {}: {}", playerInfo.playerName, playerInfo.playerInt);

        unsigned currPlayer = s_player ? s_player->playerInt : 0;
        s_players.emplace_back(playerInfo.playerInt, playerInfo.playerName,
                               playerInfo.avatarShape, playerInfo.explorer);

        for (NetCommPlayer& player : s_players) {
            if (player.playerInt == currPlayer) {
                s_player = &player;
                break;
            }
        }
    }

    plAccountUpdateMsg* updateMsg = new plAccountUpdateMsg(plAccountUpdateMsg::kCreatePlayer);
    updateMsg->SetPlayerInt(playerInfo.playerInt);
    updateMsg->SetResult((unsigned)result);
    updateMsg->SetBCastFlag(plMessage::kBCastByExactType);
    updateMsg->Send();
}

//============================================================================
static void INetCliAuthDeletePlayerCallback (
    ENetError                       result,
    void *                          param
) {
    uint32_t playerInt = (uint32_t)((uintptr_t)param);

    if (IS_NET_ERROR(result)) {
        plNetApp::StaticErrorMsg("Delete player failed: {} {}", playerInt, NetErrorToString(result));
    }
    else {
        plNetApp::StaticDebugMsg("Player deleted: {}", playerInt);

        uint32_t currPlayer = s_player ? s_player->playerInt : 0;

        for (auto it = s_players.begin(); it != s_players.end(); ++it) {
            if (it->playerInt == playerInt) {
                s_players.erase(it);
                break;
            }
        }

        for (NetCommPlayer& player : s_players) {
            if (player.playerInt == currPlayer) {
                s_player = &player;
                break;
            }
        }
    }

    plAccountUpdateMsg* updateMsg = new plAccountUpdateMsg(plAccountUpdateMsg::kDeletePlayer);
    updateMsg->SetPlayerInt(playerInt);
    updateMsg->SetResult((uint32_t)result);
    updateMsg->SetBCastFlag(plMessage::kBCastByExactType);
    updateMsg->Send();
}

//============================================================================
static void INetCliAuthChangePasswordCallback (
    ENetError       result,
    void *          param
) {
    if (IS_NET_ERROR(result)) {
        plNetApp::StaticErrorMsg("Change password failed: {}", NetErrorToString(result));
    }
    else {
        plNetApp::StaticDebugMsg("Password changed!");
    }

    plAccountUpdateMsg* updateMsg = new plAccountUpdateMsg(plAccountUpdateMsg::kChangePassword);
    updateMsg->SetPlayerInt(0);
    updateMsg->SetResult((unsigned)result);
    updateMsg->SetBCastFlag(plMessage::kBCastByExactType);
    updateMsg->Send();
}

//============================================================================
static void INetCliAuthGetPublicAgeListCallback (
    ENetError                   result,
    void *                      param,
    std::vector<NetAgeInfo>     ages
) {
    NetCommParam * cp = (NetCommParam *) param;
    
    plNetCommPublicAgeListMsg * msg = new plNetCommPublicAgeListMsg;
    msg->result     = result;
    msg->param      = cp->param;
    msg->ptype      = cp->type;
    msg->ages       = std::move(ages);
    msg->Send();
    
    delete cp;
}

//============================================================================
static void INetCliGameJoinAgeRequestCallback (
    ENetError       result,
    void *          param
) {
    plNetCommLinkToAgeMsg * msg = new plNetCommLinkToAgeMsg;
    msg->result     = result;
    msg->param      = param;
    msg->Send();
}

//============================================================================
static void INetCliAuthAgeRequestCallback (
    ENetError       result,
    void *          param,
    unsigned        ageMcpId,
    unsigned        ageVaultId,
    const plUUID&   ageInstId,
    plNetAddress    gameAddr
) {
    if (!IS_NET_ERROR(result) || result == kNetErrVaultNodeNotFound) {
        s_age.ageInstId = ageInstId;
        s_age.ageVaultId = ageVaultId;

        ST::string gameAddrStr = gameAddr.GetHostString();
        ST::string ageInstIdStr = ageInstId.AsString();

        plNetApp::StaticDebugMsg(
            "Connecting to game server {}, ageInstId {}",
            gameAddrStr,
            ageInstIdStr
        );

        NetCliGameDisconnect();
        NetCliGameStartConnect(gameAddr.GetHost());
        NetCliGameJoinAgeRequest(
            ageMcpId,
            s_account.accountUuid,
            s_player->playerInt,
            [param](auto result) {
                INetCliGameJoinAgeRequestCallback(result, param);
            }
        );
    }
    else {
        INetCliGameJoinAgeRequestCallback(
            result,
            param
        );
    }
}

//============================================================================
static void INetCliAuthUpgradeVisitorRequestCallback (
    ENetError       result,
    void *          param
) {
    uint32_t playerInt = (uint32_t)((uintptr_t)param);

    if (IS_NET_ERROR(result)) {
        plNetApp::StaticErrorMsg("Upgrade visitor failed: {} {}", playerInt, NetErrorToString(result));
    }
    else {
        plNetApp::StaticDebugMsg("Upgrade visitor succeeded: {}", playerInt);

        for (NetCommPlayer& player : s_players) {
            if (player.playerInt == playerInt) {
                player.explorer = true;
                break;
            }
        }
    }

    plAccountUpdateMsg* updateMsg = new plAccountUpdateMsg(plAccountUpdateMsg::kUpgradePlayer);
    updateMsg->SetPlayerInt(playerInt);
    updateMsg->SetResult((uint32_t)result);
    updateMsg->SetBCastFlag(plMessage::kBCastByExactType);
    updateMsg->Send();
}

//============================================================================
static void INetCliAuthSendFriendInviteCallback (
    ENetError       result,
    void *          param
) {
    pfKIMsg* kiMsg = new pfKIMsg(pfKIMsg::kFriendInviteSent);
    kiMsg->SetIntValue((int32_t)result);
    kiMsg->Send();
}

//============================================================================
static void AuthSrvIpAddressCallback (
    ENetError       result,
    const ST::string& addr
) {
    s_authSrvAddr = addr;
    s_hasAuthSrvIpAddress = true;
}

//============================================================================
static void FileSrvIpAddressCallback (
    ENetError       result,
    const ST::string& addr
) {
    s_fileSrvAddr = addr;
    s_hasFileSrvIpAddress = true;
}


/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
const NetCommPlayer * NetCommGetPlayer () {
    static NetCommPlayer s_nilPlayer;
    return s_player ? s_player : &s_nilPlayer;
}

//============================================================================
const std::vector<NetCommPlayer>& NetCommGetPlayerList () {
    return s_players;
}

//============================================================================
unsigned NetCommGetPlayerCount () {
    return s_players.size();
}

//============================================================================
const NetCommAccount * NetCommGetAccount () {
    return &s_account;
}

//============================================================================
bool NetCommIsLoginComplete() {
    return s_loginComplete;
}

//============================================================================
void NetCommSetIniPlayerId(unsigned playerId) {
    s_iniStartupPlayerId = playerId;
}

//============================================================================
void NetCommSetIniStartUpAge(const ST::string& ageName) {
    s_iniStartupAgeName = ageName;
}

//============================================================================
const NetCommAge * NetCommGetAge () {
    return &s_age;
}

//============================================================================
const NetCommAge * NetCommGetStartupAge () {
    return &s_startupAge;
}

//============================================================================
bool NetCommNeedToLoadAvatar () {
    return s_needAvatarLoad;
}

//============================================================================
void NetCommSetAvatarLoaded (bool loaded /* = true */) {
    s_needAvatarLoad = !loaded;
}

//============================================================================
void NetCommChangeMyPassword (const ST::string& password) {
    NetCliAuthAccountChangePasswordRequest(s_account.accountName, password, INetCliAuthChangePasswordCallback, nullptr);
}

//============================================================================
void NetCommStartup () {
    s_shutdown = false;

    AsyncCoreInitialize();
    plNetApp::StaticDebugMsg("Client: {}", plProduct::ProductString());

    NetClientInitialize();
    NetClientSetErrorHandler(IPreInitNetErrorCallback);

    // Set startup age info
    s_startupAge.ageDatasetName = s_iniStartupAgeName;

    s_startupAge.ageInstId = s_iniStartupAgeInstId;
    StrCopy(s_startupAge.spawnPtName, "LinkInPointDefault", std::size(s_startupAge.spawnPtName));
}

//============================================================================
void NetCommShutdown () {
    s_shutdown = true;

    NetCliGameDisconnect();
    NetCliAuthDisconnect();
    if (!gDataServerLocal)
        NetCliFileDisconnect();

    NetClientDestroy();
    AsyncCoreDestroy(30 * 1000);
}

//============================================================================
void NetCommEnableNet (
    bool            enabled,
    bool            wait
) {
    if (enabled)
        NetClientInitialize();
    else
        NetClientDestroy(wait);
}

//============================================================================
void NetCommActivatePostInitErrorHandler () {
    NetClientSetErrorHandler(INetErrorCallback);
}

//============================================================================
void NetCommActivateMsgDispatchers() {
    NetClientSetErrorHandler(INetErrorCallback);
    NetCliGameSetRecvBufferHandler(INetBufferCallback);
//  NetCliAuthSetRecvBufferHandler(INetBufferCallback);
    NetCliAuthSetNotifyNewBuildHandler(INotifyNewBuildCallback);
    NetCliAuthSetConnectCallback(INotifyAuthConnectedCallback);
}

//============================================================================
void NetCommUpdate () {
    // plClient likes to recursively call us on occasion; debounce that crap.
    static std::atomic_flag s_updating = ATOMIC_FLAG_INIT;
    if (!s_updating.test_and_set()) {
        NetClientUpdate();
        s_updating.clear();
    }
}

//============================================================================
void NetCommConnect () {

    const ST::string* addrs;
    unsigned count;
    bool connectedToKeeper = false;

    // if a console override was specified for a authserv, connect directly to the authserver rather than going through the gatekeeper
    if((count = GetAuthSrvHostnames(addrs)) && !addrs[0].empty())
    {
        NetCliAuthStartConnect(addrs, count);
    }
    else
    {
        count = GetGateKeeperSrvHostnames(addrs);
        NetCliGateKeeperStartConnect(addrs, count);
        connectedToKeeper = true;

        // request an auth server ip address
        NetCliGateKeeperAuthSrvIpAddressRequest(AuthSrvIpAddressCallback);

        while(!s_hasAuthSrvIpAddress && !s_netError) {
            NetClientUpdate();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
            
        const ST::string authSrv[] = {
            s_authSrvAddr
        };
        NetCliAuthStartConnect(authSrv, 1);
    }

    if (!gDataServerLocal) {

        // if a console override was specified for a filesrv, connect directly to the fileserver rather than going through the gatekeeper
        if((count = GetFileSrvHostnames(addrs)) && !addrs[0].empty())
        {
            NetCliFileStartConnect(addrs, count);
        }
        else
        {
            if (!connectedToKeeper) {
                count = GetGateKeeperSrvHostnames(addrs);
                NetCliGateKeeperStartConnect(addrs, count);
                connectedToKeeper = true;
            }

            // request a file server ip address
            NetCliGateKeeperFileSrvIpAddressRequest(FileSrvIpAddressCallback, false);

            while(!s_hasFileSrvIpAddress && !s_netError) {
                NetClientUpdate();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            
            const ST::string fileSrv[] = {
                s_fileSrvAddr
            };
            NetCliFileStartConnect(fileSrv, 1);
        }
    }

    if (connectedToKeeper)
        NetCliGateKeeperDisconnect();
}

//============================================================================
void NetCommDisconnect () {
    NetCliAuthDisconnect();

    if (!gDataServerLocal) {
        NetCliFileDisconnect();
    }
}

//============================================================================
void NetCommSendMsg (
    plNetMessage *  msg
) {
    msg->SetPlayerID(NetCommGetPlayer()->playerInt);

    hsRAMStream stream;
    msg->PokeBuffer(&stream);

    NetCliGamePropagateBuffer(
        msg->ClassIndex(),
        stream.GetEOF(),
        static_cast<const uint8_t *>(stream.GetData())
    );
}

//============================================================================
void NetCommRecvMsg (
    plNetMessage * msg
) {
    if (s_msgHandler != nullptr) {
        s_msgHandler->ReceiveMsg(msg);
    }
}

//============================================================================
void NetCommSetAccountUsernamePassword (
    const ST::string&   username,
    const ShaDigest &   namePassHash
) {
    s_iniAccountUsername = username;
    memcpy(s_namePassHash, namePassHash, sizeof(ShaDigest));
}

//============================================================================
void NetCommSetAuthTokenAndOS (
    const char16_t*       authToken,
    const char16_t*       os
) {
    if (authToken)
        StrCopy(s_iniAuthToken, authToken, std::size(s_iniAuthToken));
    if (os)
        StrCopy(s_iniOS, os, std::size(s_iniOS));
}

//============================================================================
ENetError NetCommGetAuthResult () {
    return s_authResult;
}

//============================================================================
void NetCommAuthenticate (
    void *          param
) {
    s_loginComplete = false;

    s_account.accountName = s_iniAccountUsername;
    memcpy(s_account.accountNamePassHash, s_namePassHash, sizeof(ShaDigest));

    NetCliAuthLoginRequest(
        s_account.accountName,
        &s_account.accountNamePassHash,
        s_iniAuthToken,
        s_iniOS,
        INetCliAuthLoginRequestCallback,
        nullptr
    );
}

//============================================================================
void NetCommLinkToAge (     // --> plNetCommLinkToAgeMsg
    const NetCommAge &      age,
    void *                  param
) {
    s_age = age;

    if (plNetClientApp::GetInstance()->GetFlagsBit(plNetClientApp::kLinkingToOfflineAge)) {
        plNetCommLinkToAgeMsg * msg = new plNetCommLinkToAgeMsg;
        msg->result     = kNetSuccess;
        msg->param      = nullptr;
        msg->Send();

        return;
    }

    NetCliAuthAgeRequest(
        s_age.ageDatasetName,
        s_age.ageInstId,
        INetCliAuthAgeRequestCallback,
        param
    );
}

//============================================================================
void NetCommSetActivePlayer (//--> plNetCommActivePlayerMsg
    unsigned                desiredPlayerInt,
    void *                  param
) {
    unsigned playerInt = 0;

    if (s_player) {
        if (hsRef<RelVaultNode> rvn = VaultGetPlayerInfoNode()) {
            VaultPlayerInfoNode pInfo(rvn);
            pInfo.SetAgeInstUuid(kNilUuid);
            pInfo.SetOnline(false);
            NetCliAuthVaultNodeSave(rvn.Get(), nullptr, nullptr);
        }

        VaultCull(s_player->playerInt);
    }

    if (desiredPlayerInt == 0)
        s_player = nullptr;
    else {
        for (NetCommPlayer& player : s_players) {
            if (player.playerInt == desiredPlayerInt) {
                playerInt = desiredPlayerInt;
                s_player = &player;
                break;
            }
        }
        ASSERT(s_player);
    }

    NetCliAuthSetPlayerRequest(
        playerInt,
        INetCliAuthSetPlayerRequestCallback,
        param
    );
}

//============================================================================
void NetCommCreatePlayer (  // --> plNetCommCreatePlayerMsg
    const ST::string&       playerName,
    const ST::string&       avatarShape,
    const ST::string&       friendInvite,
    unsigned                createFlags,
    void *                  param
) {
    NetCliAuthPlayerCreateRequest(
        playerName,
        avatarShape,
        friendInvite,
        INetCliAuthCreatePlayerRequestCallback,
        param
    );
}

//============================================================================
void NetCommDeletePlayer (  // --> plNetCommDeletePlayerMsg
    unsigned                playerInt,
    void *                  param
) {
    ASSERTMSG(!param, "'param' will not be propagated to your callback function, you may modify the code to support this");
    ASSERT(NetCommGetPlayer()->playerInt != playerInt);

    NetCliAuthPlayerDeleteRequest(
        playerInt,
        INetCliAuthDeletePlayerCallback,
        (void*)(uintptr_t)playerInt
    );
}

//============================================================================
void NetCommGetPublicAgeList (//-> plNetCommPublicAgeListMsg
    const ST::string&               ageName,
    void *                          param,
    plNetCommReplyMsg::EParamType   ptype
) {
    NetCommParam * cp = new NetCommParam;
    cp->param   = param;
    cp->type    = ptype;

    NetCliAuthGetPublicAgeList(
        ageName,
        INetCliAuthGetPublicAgeListCallback,
        cp
    );
}

//============================================================================
void NetCommSetAgePublic (  // --> no msg
    unsigned                ageInfoId,
    bool                    makePublic
) {
    NetCliAuthSetAgePublic(
        ageInfoId,
        makePublic
    );
}

//============================================================================
void NetCommUpgradeVisitorToExplorer (
    unsigned                playerInt,
    void *                  param
) {
    NetCliAuthUpgradeVisitorRequest(
        playerInt,
        INetCliAuthUpgradeVisitorRequestCallback,
        (void*)(uintptr_t)playerInt
    );
}

//============================================================================
void NetCommSetCCRLevel (
    unsigned                ccrLevel
) {
    if (hsRef<RelVaultNode> rvnInfo = VaultGetPlayerInfoNode()) {
        VaultPlayerInfoNode pInfo(rvnInfo);
        pInfo.SetCCRLevel(ccrLevel);
    }

    NetCliAuthSetCCRLevel(ccrLevel);
}

//============================================================================
void NetCommSendFriendInvite (
    const ST::string& emailAddress,
    const ST::string& toName,
    const plUUID&     inviteUuid
) {
    NetCliAuthSendFriendInvite(
        emailAddress,
        toName,
        inviteUuid,
        INetCliAuthSendFriendInviteCallback,
        nullptr
    );
}

static constexpr std::u16string_view kTracebackTruncatedMarker = u"[...]"sv;
// Maximum length of a traceback chunk to still leave room for the truncation marker.
// Like kMaxTracebackLength, includes one char16_t extra for the zero terminator.
static constexpr unsigned int kTracebackChunkLength = kMaxTracebackLength - std::size(kTracebackTruncatedMarker);

typedef void LogErrorFunc(const char16_t* errorText);

template<LogErrorFunc logErrorFunc>
static void NetCommLogChunkedError(const ST::string& errorText)
{
    ST::utf16_buffer wdata = errorText.to_utf16();
    if (wdata.size() < kMaxTracebackLength - 1) {
        logErrorFunc(wdata.data());
    } else {
        // The traceback is too long for a single message,
        // so split it up into multiple small messages
        // and add a truncation marker at the end of every message but the last.
        auto pos = wdata.begin();
        for (; wdata.end() - pos >= kMaxTracebackLength - 1; pos += kTracebackChunkLength - 1) {
            ST::utf16_buffer chunk = ST::utf16_buffer(pos, kMaxTracebackLength - 1);
            kTracebackTruncatedMarker.copy(&chunk[kTracebackChunkLength - 1], kTracebackTruncatedMarker.size());
            logErrorFunc(chunk.data());
        }
        logErrorFunc(pos);
    }
}

void NetCommLogPythonTraceback(const ST::string& traceback)
{
    NetCommLogChunkedError<NetCliAuthLogPythonTraceback>(traceback);
}

void NetCommLogStackDump(const ST::string& stackDump)
{
    NetCommLogChunkedError<NetCliAuthLogStackDump>(stackDump);
}


/*****************************************************************************
*
*   Msg handler interface
*
***/

void plNetClientComm::SetMsgHandler(plNetMsgHandler* msgHandler)
{
    s_msgHandler = msgHandler;
}
