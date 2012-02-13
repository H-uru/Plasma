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

#include "pnAsyncCore/pnAsyncCore.h"
#include "pnProduct/pnProduct.h"
#include "pnNetCli/pnNetCli.h"
#include "plNetGameLib/plNetGameLib.h"
#include "pnIni/pnIni.h"
#include "pnEncryption/plChallengeHash.h"

#include "plMessage/plNetCommMsgs.h"
#include "plMessage/plNetClientMgrMsg.h"
#include "plNetMessage/plNetMessage.h"
#include "plNetCommon/plNetCommon.h"
#include "plVault/plVault.h"
#include "plMessage/plAccountUpdateMsg.h"
#include "plNetClient/plNetClientMgr.h"

#include "pfMessage/pfKIMsg.h"

#include "hsResMgr.h"

#include <malloc.h>

extern  hsBool  gDataServerLocal;

/*****************************************************************************
*
*   Exported data
*
***/

const unsigned          kNetCommAllMsgClasses   = (unsigned)-1;
FNetCommMsgHandler *    kNetCommAllMsgHandlers  = (FNetCommMsgHandler*)-1;
const void *            kNetCommAllUserStates   = (void*)-1;

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
static ARRAY(NetCommPlayer) s_players;
static NetCommPlayer *      s_player;
static NetCommAge           s_age;
static NetCommAge           s_startupAge;
static bool                 s_needAvatarLoad = true;
static bool                 s_loginComplete = false;
static bool                 s_hasAuthSrvIpAddress = false;
static bool                 s_hasFileSrvIpAddress = false;
static ENetError            s_authResult = kNetErrAuthenticationFailed;
static wchar_t                s_authSrvAddr[256];
static wchar_t                s_fileSrvAddr[256];

static wchar_t                s_iniServerAddr[256];
static wchar_t                s_iniFileServerAddr[256];
static wchar_t                s_iniAccountUsername[kMaxAccountNameLength];
static ShaDigest            s_namePassHash;
static wchar_t                s_iniAuthToken[kMaxPublisherAuthKeyLength];
static wchar_t                s_iniOS[kMaxGTOSIdLength];
static bool                 s_iniReadAccountInfo = true;
static wchar_t                s_iniStartupAgeName[kMaxAgeNameLength];
static Uuid                 s_iniStartupAgeInstId;
static wchar_t                s_iniStartupPlayerName[kMaxPlayerNameLength];
static bool                 s_netError = false;


struct NetCommMsgHandler : THashKeyVal<unsigned> {
    HASHLINK(NetCommMsgHandler) link;
    FNetCommMsgHandler  *       proc;
    void *                      state;

    NetCommMsgHandler (
        unsigned             msgId,
        FNetCommMsgHandler * proc,
        void *               state
    ) : THashKeyVal<unsigned>(msgId)
    ,   proc(proc)
    ,   state(state)
    { }
};

static HASHTABLEDECL(
    NetCommMsgHandler,
    THashKeyVal<unsigned>,
    link
) s_handlers;

static NetCommMsgHandler    s_defaultHandler(0, nil, nil);
static NetCommMsgHandler    s_preHandler(0, nil, nil);


//============================================================================
static void INetLogCallback (
    ELogSeverity    severity,
    const wchar_t     msg[]
) {
    // Use the async log facility
    AsyncLogWriteMsg(ProductShortName(), severity, msg);
}

//============================================================================
static void INetErrorCallback (
    ENetProtocol    protocol,
    ENetError       error
) {
    NetClientDestroy(false);
    
    plNetClientMgrMsg * msg = NEWZERO(plNetClientMgrMsg);
    msg->type   = plNetClientMgrMsg::kCmdDisableNet;
    msg->yes    = true;
    msg->AddReceiver(plNetClientApp::GetInstance()->GetKey());

    switch (error)
    {
    case kNetErrKickedByCCR:
        StrPrintf(
            msg->str,
            arrsize(msg->str),
            "You have been kicked by a CCR."
        );
        break;

    default:
        // Until we get some real error handling, this'll ensure no errors
        // fall thru the cracks and we hang forever wondering what's up.
        StrPrintf(
            // buf
            msg->str,
            arrsize(msg->str),
            // fmt
            "Network error %u, %S.\n"
            "protocol: %S\n"
            ,// values
            error,
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
        LogMsg(kLogError, "NetComm: received junk propagated buffer");
        return;
    }
    plNetMessage * msg = plNetMessage::ConvertNoRef(plFactory::Create(type));
    if (!msg) {
        LogMsg(kLogError, "NetComm: could not convert plNetMessage to class %u", type);
        return;
    }

    if (!msg->PeekBuffer((const char *)buffer, bytes)) {
        LogMsg(kLogError, "NetComm: plNetMessage %u failed to peek buffer", type);
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
        
    plNetCommAuthConnectedMsg * msg = NEWZERO(plNetCommAuthConnectedMsg);
    msg->Send();
}

//============================================================================
static void PlayerInitCallback (
    ENetError   result,
    void *      param
) {
    if (IS_NET_ERROR(result) && (result != kNetErrVaultNodeNotFound)) {
        s_player = nil;
    }
    else {
        // Ensure the city link has the required spawn points
        plAgeInfoStruct info;
        info.SetAgeFilename(kCityAgeFilename);
        if (RelVaultNode * rvn = VaultGetOwnedAgeLinkIncRef(&info)) {
            VaultAgeLinkNode acc(rvn);
            acc.AddSpawnPoint(plSpawnPointInfo(kCityFerryTerminalLinkTitle, kCityFerryTerminalLinkSpawnPtName));
            rvn->DecRef();
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
        s_player = nil;
        PlayerInitCallback(result, param);
    }
    else {
        s_needAvatarLoad = true;

        VaultDownload(
            L"SetActivePlayer",
            s_player->playerInt,
            PlayerInitCallback,
            param,
            nil,
            nil
        );
    }
}

//============================================================================
static void LoginPlayerInitCallback (
    ENetError                   result,
    void *                      param
) {
    if (IS_NET_ERROR(result) && (result != kNetErrVaultNodeNotFound))
        s_player = nil;
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
        s_player = nil;
        
        plNetCommAuthMsg * msg  = new plNetCommAuthMsg;
        msg->result             = result;
        msg->param              = param;
        msg->Send();
    }
    else {
        VaultDownload(
            L"SetActivePlayer",
            s_player->playerInt,
            LoginPlayerInitCallback,
            param,
            nil,
            nil
        );
    }
}

//============================================================================
static void INetCliAuthLoginRequestCallback (
    ENetError                   result,
    void *                      param,
    const Uuid &                accountUuid,
    unsigned                    accountFlags,
    unsigned                    billingType,
    const NetCliAuthPlayerInfo  playerInfoArr[],
    unsigned                    playerCount
) {
    s_authResult = result;

    s_player = nil;
    s_players.Clear();
    
    bool wantsStartUpAge = (
        !StrLen(s_startupAge.ageDatasetName) ||
        0 == StrCmpI(s_startupAge.ageDatasetName, "StartUp")
    );

    s_loginComplete = true;

    if (!IS_NET_ERROR(result) || result == kNetErrVaultNodeNotFound) {
        s_account.accountUuid   = accountUuid;
        s_account.accountFlags  = accountFlags;
        s_account.billingType   = billingType;
        s_players.GrowToCount(playerCount, true);
        for (unsigned i = 0; i < playerCount; ++i) {
            LogMsg(kLogDebug, L"Player %u: %s explorer: %u", playerInfoArr[i].playerInt, playerInfoArr[i].playerName, playerInfoArr[i].explorer);
            s_players[i].playerInt  = playerInfoArr[i].playerInt;
            s_players[i].explorer   = playerInfoArr[i].explorer;
            StrCopy(s_players[i].playerName, playerInfoArr[i].playerName, arrsize(s_players[i].playerName));
            StrToAnsi(s_players[i].playerNameAnsi, playerInfoArr[i].playerName, arrsize(s_players[i].playerNameAnsi));
            StrToAnsi(s_players[i].avatarDatasetName, playerInfoArr[i].avatarShape, arrsize(s_players[i].avatarDatasetName));
            if (!wantsStartUpAge && 0 == StrCmpI(s_players[i].playerName, s_iniStartupPlayerName, (unsigned)-1))
                s_player = &s_players[i];
        }
    }
    else
        s_account.accountUuid = kNilGuid;

    // If they specified an alternate age, but we couldn't find the player, force
    // the StartUp age to load so that they may select/create a player first.    
    if (!wantsStartUpAge && !s_player)
        StrCopy(s_startupAge.ageDatasetName, "StartUp", arrsize(s_startupAge.ageDatasetName));

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
        LogMsg(kLogDebug, L"Create player failed: %s", NetErrorToString(result));
    }
    else {
        LogMsg(kLogDebug, L"Created player %s: %u", playerInfo.playerName, playerInfo.playerInt);

        unsigned currPlayer = s_player ? s_player->playerInt : 0;       
        NetCommPlayer * newPlayer = s_players.New();

        newPlayer->playerInt    = playerInfo.playerInt;
        newPlayer->explorer     = playerInfo.explorer;
        StrCopy(newPlayer->playerName, playerInfo.playerName, arrsize(newPlayer->playerName));
        StrToAnsi(newPlayer->playerNameAnsi, playerInfo.playerName, arrsize(newPlayer->playerNameAnsi));
        StrToAnsi(newPlayer->avatarDatasetName, playerInfo.avatarShape, arrsize(newPlayer->avatarDatasetName));

        { for (unsigned i = 0; i < s_players.Count(); ++i) {
            if (s_players[i].playerInt == currPlayer) {
                s_player = &s_players[i];
                break;
            }
        }}
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
        LogMsg(kLogDebug, L"Delete player failed: %d %s", playerInt, NetErrorToString(result));
    }
    else {
        LogMsg(kLogDebug, L"Player deleted: %d", playerInt);

        uint32_t currPlayer = s_player ? s_player->playerInt : 0;       

        {for (uint32_t i = 0; i < s_players.Count(); ++i) {
            if (s_players[i].playerInt == playerInt) {
                s_players.DeleteUnordered(i);
                break;
            }
        }}

        {for (uint32_t i = 0; i < s_players.Count(); ++i) {
            if (s_players[i].playerInt == currPlayer) {
                s_player = &s_players[i];
                break;
            }
        }}
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
        LogMsg(kLogDebug, L"Change password failed: %s", NetErrorToString(result));
    }
    else {
        LogMsg(kLogDebug, L"Password changed!");
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
    const ARRAY(NetAgeInfo) &   ages
) {
    NetCommParam * cp = (NetCommParam *) param;
    
    plNetCommPublicAgeListMsg * msg = NEWZERO(plNetCommPublicAgeListMsg);
    msg->result     = result;
    msg->param      = cp->param;
    msg->ptype      = cp->type;
    msg->ages.Set(ages.Ptr(), ages.Count());
    msg->Send();
    
    delete cp;
}

//============================================================================
static void INetAuthFileListRequestCallback (
    ENetError                   result,
    void *                      param,
    const NetCliAuthFileInfo    infoArr[],
    unsigned                    infoCount
) {
    plNetCommFileListMsg * msg = new plNetCommFileListMsg;
    msg->result = result;
    msg->param  = param;
    msg->fileInfoArr.Set(infoArr, infoCount);
    msg->Send();
}

//============================================================================
static void INetCliAuthFileRequestCallback (
    ENetError       result,
    void *          param,
    const wchar_t     filename[],
    hsStream *      writer
) {
    plNetCommFileDownloadMsg * msg = new plNetCommFileDownloadMsg;
    msg->result = result;
    msg->writer = writer;
    StrCopy(msg->filename, filename, arrsize(filename));
    msg->Send();
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
    const Uuid &    ageInstId,
    NetAddressNode  gameAddr
) {
    if (!IS_NET_ERROR(result) || result == kNetErrVaultNodeNotFound) {
        s_age.ageInstId = ageInstId;
        s_age.ageVaultId = ageVaultId;
        
        wchar_t gameAddrStr[64];
        wchar_t ageInstIdStr[64];
        NetAddressNodeToString(gameAddr, gameAddrStr, arrsize(gameAddrStr));
        LogMsg(
            kLogPerf,
            L"Connecting to game server %s, ageInstId %s",
            gameAddrStr,
            GuidToString(ageInstId, ageInstIdStr, arrsize(ageInstIdStr))
        );
        NetCliGameDisconnect();
        NetCliGameStartConnect(gameAddr);
        NetCliGameJoinAgeRequest(
            ageMcpId,
            s_account.accountUuid,
            s_player->playerInt,
            INetCliGameJoinAgeRequestCallback,
            param
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
        LogMsg(kLogDebug, L"Upgrade visitor failed: %d %s", playerInt, NetErrorToString(result));
    }
    else {
        LogMsg(kLogDebug, L"Upgrade visitor succeeded: %d", playerInt);

        {for (uint32_t i = 0; i < s_players.Count(); ++i) {
            if (s_players[i].playerInt == playerInt) {
                s_players[i].explorer = true;
                break;
            }
        }}
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
static void IReadNetIni() {
    wchar_t filename[MAX_PATH];
    StrPrintf(filename, arrsize(filename), L"%s.cfg", ProductCoreName());

    wchar_t pathAndName[MAX_PATH];
    PathGetInitDirectory(pathAndName, arrsize(pathAndName));
    PathAddFilename(pathAndName, pathAndName, filename, arrsize(pathAndName));

#ifndef PLASMA_EXTERNAL_RELEASE
    // internal dev build will override user-based setting with local folder if it's there
    wchar_t localPathAndName[MAX_PATH];
    PathAddFilename(localPathAndName, L"init", filename, arrsize(localPathAndName));
    if (PathDoesFileExist(localPathAndName))
            StrCopy(pathAndName, localPathAndName, arrsize(pathAndName));
#endif

    Ini * ini = IniOpen(pathAndName);

    wchar_t password[kMaxPasswordLength];

    if (ini) {
        // Read [Net.Server] section
        IniGetString(
            IniGetFirstValue(
                ini,
                L"Net.Server",
                L"Addr",
                nil
            ),
            s_iniServerAddr,
            arrsize(s_iniServerAddr),
            0,
            L""
        );

        // Read [Net.FileServer] section
        IniGetString(
            IniGetFirstValue(
                ini,
                L"Net.FileServer",
                L"Addr",
                nil
            ),
            s_iniFileServerAddr,
            arrsize(s_iniFileServerAddr),
            0,
            s_iniServerAddr
        );
        
        if (s_iniReadAccountInfo) {
            // Read [Net.Account] section
            IniGetString(
                IniGetFirstValue(
                    ini,
                    L"Net.Account",
                    L"Username",
                    nil
                ),
                s_iniAccountUsername,
                arrsize(s_iniAccountUsername),
                0,
                L""
            );
            IniGetString(
                IniGetFirstValue(
                    ini,
                    L"Net.Account",
                    L"Password",
                    nil
                ),
                password,
                arrsize(password),
                0,
                L""
            );

            // Read [Net.Startup] section
            IniGetString(
                IniGetFirstValue(
                    ini,
                    L"Net.Startup",
                    L"AgeName",
                    nil
                ),
                s_iniStartupAgeName,
                arrsize(s_iniStartupAgeName),
                0,
                L"StartUp"
            );
            IniGetString(
                IniGetFirstValue(
                    ini,
                    L"Net.Startup",
                    L"PlayerName",
                    nil
                ),
                s_iniStartupPlayerName,
                arrsize(s_iniStartupPlayerName),
                0,
                L""
            );

            CryptHashPassword(_TEMP_CONVERT_FROM_WCHAR_T(s_iniAccountUsername), _TEMP_CONVERT_FROM_WCHAR_T(password), s_namePassHash);
        }
        else {
            StrCopy(s_iniStartupAgeName, L"StartUp", arrsize(s_iniStartupAgeName));
        }
    }

#ifndef PLASMA_EXTERNAL_RELEASE
    // @@@: Internal build only: Drop a default version of the file if not found    
    if (!ini) {
        EFileError  fileError;
        uint64_t       fileSize;
        uint64_t       lastWrite;      
        AsyncFile file = AsyncFileOpen(
            pathAndName,
            nil,
            &fileError,
            kAsyncFileReadAccess|kAsyncFileWriteAccess,
            kAsyncFileModeCreateNew,
            0,
            nil,
            &fileSize,
            &lastWrite
        );
        
        if (file) {
            char line[2048];
            StrPrintf(
                line,
                arrsize(line),
                // format
                "[Net.Server]\r\n"
                "\tAddr=%S\r\n"
                "\r\n"
                "[Net.FileServer]\r\n"
                "\tAddr=%S\r\n"
                "\r\n"
                "[Net.Account]\r\n"
                "\tUsername=%S\r\n"
                "\tPassword=AccountPassword\r\n"
                "\r\n"
                "[Net.Startup]\r\n"
                "\tAgeName=%S\r\n"
                "\tPlayerName=%S\r\n"
                ,   // values
                L"shard",
                L"shard",
                L"AccountUserName",
                L"StartUp",
                L"PlayerName",
                nil
            );
            AsyncFileWrite(file, 0, line, StrLen(line), kAsyncFileRwSync, nil);
            AsyncFileClose(file, kAsyncFileDontTruncate);
        }
    }
#endif

    // Set startup age info
    memset(&s_startupAge, 0, sizeof(s_startupAge));

    if (StrLen(s_iniStartupAgeName) == 0)
        StrCopy(s_startupAge.ageDatasetName, "StartUp", arrsize(s_startupAge.ageDatasetName));
    else
        StrToAnsi(s_startupAge.ageDatasetName, s_iniStartupAgeName, arrsize(s_startupAge.ageDatasetName));

    s_startupAge.ageInstId = s_iniStartupAgeInstId;
    StrCopy(s_startupAge.spawnPtName, "LinkInPointDefault", arrsize(s_startupAge.spawnPtName));

    IniClose(ini);
}

//============================================================================
static void AuthSrvIpAddressCallback (
    ENetError       result,
    void *          param,
    const wchar_t     addr[]
) {
    StrCopy(s_authSrvAddr, addr, arrsize(s_authSrvAddr)); 
    s_hasAuthSrvIpAddress = true;
}

//============================================================================
static void FileSrvIpAddressCallback (
    ENetError       result,
    void *          param,
    const wchar_t     addr[]
) {
    StrCopy(s_fileSrvAddr, addr, arrsize(s_fileSrvAddr)); 
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
const ARRAY(NetCommPlayer)& NetCommGetPlayerList () {
    return s_players;
}

//============================================================================
unsigned NetCommGetPlayerCount () {
    return s_players.Count();
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
void NetCommChangeMyPassword (
    const wchar_t password[]
) {
    NetCliAuthAccountChangePasswordRequest(s_account.accountName, password, INetCliAuthChangePasswordCallback, nil);
}

//============================================================================
void NetCommStartup () {
    s_shutdown = false;

    LogRegisterHandler(INetLogCallback);
    AsyncCoreInitialize();
    AsyncLogInitialize(L"Log", false);
    wchar_t productString[256];
    ProductString(productString, arrsize(productString));
    LogMsg(kLogPerf, L"Client: %s", productString);

    NetClientInitialize();
    NetClientSetErrorHandler(IPreInitNetErrorCallback);
    NetCliGameSetRecvBufferHandler(INetBufferCallback);
//    NetCliAuthSetRecvBufferHandler(INetBufferCallback);
    NetCliAuthSetNotifyNewBuildHandler(INotifyNewBuildCallback);
    NetCliAuthSetConnectCallback(INotifyAuthConnectedCallback);

    IReadNetIni();
}

//============================================================================
void NetCommShutdown () {
    s_shutdown = true;

    NetCommSetDefaultMsgHandler(nil, nil);
    NetCommSetMsgPreHandler(nil, nil);
    NetCommRemoveMsgHandler(
        kNetCommAllMsgClasses,
        kNetCommAllMsgHandlers,
        kNetCommAllUserStates
    );
    
    NetCliGameDisconnect();
    NetCliAuthDisconnect();
    if (!gDataServerLocal)
        NetCliFileDisconnect();

    NetClientDestroy(false);
    AsyncLogDestroy();
    AsyncCoreDestroy(30 * 1000);
    LogUnregisterHandler(INetLogCallback);
}

//============================================================================
void NetCommEnableNet (
    bool            enabled,
    bool            wait
) {
    if (enabled) {
        NetClientInitialize();
        NetClientSetErrorHandler(INetErrorCallback);
        NetCliGameSetRecvBufferHandler(INetBufferCallback);
//      NetCliAuthSetRecvBufferHandler(INetBufferCallback);
    }
    else {
        NetClientDestroy(wait);
    }
}

//============================================================================
void NetCommActivatePostInitErrorHandler () {
    NetClientSetErrorHandler(INetErrorCallback);
}

//============================================================================
void NetCommUpdate () {
    // plClient likes to recursively call us on occasion; debounce that crap.
    static long s_updating;
    if (0 == AtomicSet(&s_updating, 1)) {
        NetClientUpdate();
        AtomicSet(&s_updating, 0);
    }
}

//============================================================================
void NetCommConnect () {

    const wchar_t ** addrs;
    unsigned count;
    hsBool connectedToKeeper = false;

    // if a console override was specified for a authserv, connect directly to the authserver rather than going through the gatekeeper
    if((count = GetAuthSrvHostnames(&addrs)) && wcslen(addrs[0]))
    {
        NetCliAuthStartConnect(addrs, count);
    }
    else
    {
        count = GetGateKeeperSrvHostnames(&addrs);
        NetCliGateKeeperStartConnect(addrs, count);
        connectedToKeeper = true;

        // request an auth server ip address
        NetCliGateKeeperAuthSrvIpAddressRequest(AuthSrvIpAddressCallback, nil);

        while(!s_hasAuthSrvIpAddress && !s_netError) {
            NetClientUpdate();
            AsyncSleep(10);
        }
            
        const wchar_t * authSrv[] = {
            s_authSrvAddr
        };
        NetCliAuthStartConnect(authSrv, 1);
    }

    if (!gDataServerLocal) {

        // if a console override was specified for a filesrv, connect directly to the fileserver rather than going through the gatekeeper
        if((count = GetFileSrvHostnames(&addrs)) && wcslen(addrs[0]))
        {
            NetCliFileStartConnect(addrs, count);
        }
        else
        {
            if (!connectedToKeeper) {
                count = GetGateKeeperSrvHostnames(&addrs);
                NetCliGateKeeperStartConnect(addrs, count);
                connectedToKeeper = true;
            }

            // request a file server ip address
            NetCliGateKeeperFileSrvIpAddressRequest(FileSrvIpAddressCallback, nil, false);

            while(!s_hasFileSrvIpAddress && !s_netError) {
                NetClientUpdate();
                AsyncSleep(10);
            }
            
            const wchar_t * fileSrv[] = {
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

    unsigned msgSize = msg->GetPackSize();
    uint8_t * buf = (uint8_t *)malloc(msgSize);
    msg->PokeBuffer((char *)buf, msgSize);

    switch (msg->GetNetProtocol()) {
        case kNetProtocolCli2Auth:
            NetCliAuthPropagateBuffer(
                msg->ClassIndex(),
                msgSize,
                buf
            );
        break;

        case kNetProtocolCli2Game:
            NetCliGamePropagateBuffer(
                msg->ClassIndex(),
                msgSize,
                buf
            );
        break;

        DEFAULT_FATAL(msg->GetNetProtocol());
    }

    free(buf);
}

//============================================================================
void NetCommRecvMsg (
    plNetMessage * msg
) {
    for (;;) {
        if (s_preHandler.proc && kOK_MsgConsumed == s_preHandler.proc(msg, s_preHandler.state))
            break;

        unsigned msgClassIdx = msg->ClassIndex();
        NetCommMsgHandler * handler = s_handlers.Find(msgClassIdx);

        if (!handler && s_defaultHandler.proc) {
            s_defaultHandler.proc(msg, s_defaultHandler.state);
            break;
        }        
        while (handler) {
            if (kOK_MsgConsumed == handler->proc(msg, handler->state))
                break;
            handler = s_handlers.FindNext(msgClassIdx, handler);
        }
        break;
    }
}

//============================================================================
void NetCommAddMsgHandlerForType (
    unsigned                msgClassIdx,
    FNetCommMsgHandler *    proc,
    void *                  state
) {
    for (unsigned i = 0; i < plFactory::GetNumClasses(); ++i) {
        if (plFactory::DerivesFrom(msgClassIdx, i))
            NetCommAddMsgHandlerForExactType(i, proc, state);
    }
}

//============================================================================
void NetCommAddMsgHandlerForExactType (
    unsigned                msgClassIdx,
    FNetCommMsgHandler *    proc,
    void *                  state
) {
    ASSERT(msgClassIdx != kNetCommAllMsgClasses);
    ASSERT(proc && proc != kNetCommAllMsgHandlers);
    ASSERT(!state || (state && state != kNetCommAllUserStates));

    NetCommRemoveMsgHandler(msgClassIdx, proc, state);
    NetCommMsgHandler * handler = new NetCommMsgHandler(msgClassIdx, proc, state);

    s_handlers.Add(handler);
}

//============================================================================
void NetCommRemoveMsgHandler (
    unsigned                msgClassIdx,
    FNetCommMsgHandler *    proc,
    const void *            state
) {
    NetCommMsgHandler * next, * handler = s_handlers.Head();
    for (; handler; handler = next) {
        next = handler->link.Next();
        if (handler->GetValue() != msgClassIdx)
            if (msgClassIdx != kNetCommAllMsgClasses)
                continue;
        if (handler->proc != proc)
            if (proc != kNetCommAllMsgHandlers)
                continue;
        if (handler->state != state)
            if (state != kNetCommAllUserStates)
                continue;

        // We found a matching handler, delete it
        delete handler;        
    }
}

//============================================================================
void NetCommSetDefaultMsgHandler (
    FNetCommMsgHandler *    proc,
    void *                  state
) {
    s_defaultHandler.proc  = proc;
    s_defaultHandler.state = state;
}

//============================================================================
void NetCommSetMsgPreHandler (
    FNetCommMsgHandler *    proc,
    void *                  state
) {
    s_preHandler.proc  = proc;
    s_preHandler.state = state;
}

//============================================================================
void NetCommSetAccountUsernamePassword (
    wchar_t               username[],
    const ShaDigest &   namePassHash
) {
    StrCopy(s_iniAccountUsername, username, arrsize(s_iniAccountUsername));
    memcpy(s_namePassHash, namePassHash, sizeof(ShaDigest));

    s_iniReadAccountInfo = false;
}

//============================================================================
void NetCommSetAuthTokenAndOS (
    wchar_t               authToken[],
    wchar_t               os[]
) {
    if (authToken)
        StrCopy(s_iniAuthToken, authToken, arrsize(s_iniAuthToken));
    if (os)
        StrCopy(s_iniOS, os, arrsize(s_iniOS));
}

//============================================================================
ENetError NetCommGetAuthResult () {
    return s_authResult;
}

//============================================================================
void NetCommSetReadIniAccountInfo(bool readFromIni) {
    s_iniReadAccountInfo = readFromIni;
}

//============================================================================
void NetCommAuthenticate (
    void *          param
) {
    s_loginComplete = false;

    StrCopy(
        s_account.accountName,
        s_iniAccountUsername,
        arrsize(s_account.accountName)
    );
    StrToAnsi(
        s_account.accountNameAnsi,
        s_iniAccountUsername,
        arrsize(s_account.accountNameAnsi)
    );
    memcpy(s_account.accountNamePassHash, s_namePassHash, sizeof(ShaDigest));

    NetCliAuthLoginRequest(
        s_account.accountName,
        &s_account.accountNamePassHash,
        s_iniAuthToken,
        s_iniOS,
        INetCliAuthLoginRequestCallback,
        nil
    );
}

//============================================================================
void NetCommLinkToAge (     // --> plNetCommLinkToAgeMsg
    const NetCommAge &      age,
    void *                  param
) {
    s_age = age;

    if (plNetClientMgr::GetInstance()->GetFlagsBit(plNetClientApp::kLinkingToOfflineAge)) {
        plNetCommLinkToAgeMsg * msg = new plNetCommLinkToAgeMsg;
        msg->result     = kNetSuccess;
        msg->param      = nil;
        msg->Send();

        return;
    }

    wchar_t wAgeName[kMaxAgeNameLength];
    StrToUnicode(wAgeName, s_age.ageDatasetName, arrsize(wAgeName));
    
    NetCliAuthAgeRequest(
        wAgeName,
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
        if (RelVaultNode* rvn = VaultGetPlayerInfoNodeIncRef()) {
            VaultPlayerInfoNode pInfo(rvn);
            pInfo.SetAgeInstName(nil);
            pInfo.SetAgeInstUuid(kNilGuid);
            pInfo.SetOnline(false);
            NetCliAuthVaultNodeSave(rvn, nil, nil);

            rvn->DecRef();
        }

        VaultCull(s_player->playerInt);
    }

    if (desiredPlayerInt == 0)
        s_player = nil;
    else {
        for (unsigned i = 0; i < s_players.Count(); ++i) {
            if (s_players[i].playerInt == desiredPlayerInt) {
                playerInt = desiredPlayerInt;
                s_player = &s_players[i];
                break;
            }
            else if (0 == StrCmpI(s_players[i].playerName, s_iniStartupPlayerName, arrsize(s_players[i].playerName))) {
                playerInt = s_players[i].playerInt;
                s_player = &s_players[i];
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
    const char              playerName[],
    const char              avatarShape[],
    const char              friendInvite[],
    unsigned                createFlags,
    void *                  param
) {
    wchar_t wplayerName[kMaxPlayerNameLength];
    wchar_t wavatarShape[MAX_PATH];
    wchar_t wfriendInvite[MAX_PATH];

    StrToUnicode(wplayerName, playerName, arrsize(wplayerName));
    StrToUnicode(wavatarShape, avatarShape, arrsize(wavatarShape));
    StrToUnicode(wfriendInvite, friendInvite, arrsize(wfriendInvite));

    NetCliAuthPlayerCreateRequest(
            wplayerName,
            wavatarShape,
            (friendInvite != NULL) ? wfriendInvite : NULL,
            INetCliAuthCreatePlayerRequestCallback,
            param
        );
}

//============================================================================
void NetCommCreatePlayer (  // --> plNetCommCreatePlayerMsg
    const wchar_t             playerName[],
    const wchar_t             avatarShape[],
    const wchar_t             friendInvite[],
    unsigned                createFlags,
    void *                  param
) {
    NetCliAuthPlayerCreateRequest(
        playerName,
        avatarShape,
        (friendInvite != NULL) ? friendInvite : NULL,
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
        (void*)playerInt
    );
}

//============================================================================
void NetCommGetPublicAgeList (//-> plNetCommPublicAgeListMsg
    const char                      ageName[],
    void *                          param,
    plNetCommReplyMsg::EParamType   ptype
) {
    NetCommParam * cp = new NetCommParam;
    cp->param   = param;
    cp->type    = ptype;
    
    wchar_t wStr[MAX_PATH];
    StrToUnicode(wStr, ageName, arrsize(wStr));
    NetCliAuthGetPublicAgeList(
        wStr,
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
void NetCommCreatePublicAge (// --> plNetCommPublicAgeMsg
    const char              ageName[],
    const Uuid &            ageInstId,
    void *                  param
) {
}

//============================================================================
void NetCommRemovePublicAge(// --> plNetCommPublicAgeMsg
    const Uuid &            ageInstId,
    void *                  param
) {
}

//============================================================================
void NetCommRegisterOwnedAge (
    const NetCommAge &      age,
    const char              ageInstDesc[],
    unsigned                playerInt,
    void *                  param
) {
}

//============================================================================
void NetCommUnregisterOwnedAge (
    const char              ageName[],
    unsigned                playerInt,
    void *                  param
) {
}

//============================================================================
void NetCommRegisterVisitAge (
    const NetCommAge &      age,
    const char              ageInstDesc[],
    unsigned                playerInt,
    void *                  param
) {
}

//============================================================================
void NetCommUnregisterVisitAge (
    const Uuid &            ageInstId,
    unsigned                playerInt,
    void *                  param
) {
}

//============================================================================
void NetCommConnectPlayerVault (
    void *                  param
) {
}

//============================================================================
void NetCommConnectAgeVault (
    const Uuid &            ageInstId,
    void *                  param
) {
}

//============================================================================
void NetCommUpgradeVisitorToExplorer (
    unsigned                playerInt,
    void *                  param
) {
    NetCliAuthUpgradeVisitorRequest(
        playerInt,
        INetCliAuthUpgradeVisitorRequestCallback,
        (void*)playerInt
    );
}

//============================================================================
void NetCommSetCCRLevel (
    unsigned                ccrLevel
) {
    if (RelVaultNode * rvnInfo = VaultGetPlayerInfoNodeIncRef()) {
        VaultPlayerInfoNode pInfo(rvnInfo);
        pInfo.SetCCRLevel(ccrLevel);
        rvnInfo->DecRef();
    }

    NetCliAuthSetCCRLevel(ccrLevel);
}

//============================================================================
void NetCommSendFriendInvite (
    const wchar_t     emailAddress[],
    const wchar_t     toName[],
    const Uuid&     inviteUuid
) {
    NetCliAuthSendFriendInvite(
        emailAddress,
        toName,
        inviteUuid,
        INetCliAuthSendFriendInviteCallback,
        nil
    );
}


/*****************************************************************************
*
*   Msg handler interface - compatibility layer with legacy code
*
***/


////////////////////////////////////////////////////////////////////

// plNetClientComm ----------------------------------------------
plNetClientComm::plNetClientComm()
{
}

// ~plNetClientComm ----------------------------------------------
plNetClientComm::~plNetClientComm()
{
    NetCommSetMsgPreHandler(nil, nil);
}

// AddMsgHandlerForType ----------------------------------------------
void plNetClientComm::AddMsgHandlerForType( uint16_t msgClassIdx, MsgHandler* handler )
{
    int i;
    for( i = 0; i < plFactory::GetNumClasses(); i++ )
    {
        if ( plFactory::DerivesFrom( msgClassIdx, i ) )
            AddMsgHandlerForExactType( i, handler );
    }
}

// AddMsgHandlerForExactType ----------------------------------------------
void plNetClientComm::AddMsgHandlerForExactType( uint16_t msgClassIdx, MsgHandler* handler )
{
    NetCommAddMsgHandlerForExactType(msgClassIdx, MsgHandler::StaticMsgHandler, handler);
}

// RemoveMsgHandler ----------------------------------------------
bool plNetClientComm::RemoveMsgHandler( MsgHandler* handler )
{
    NetCommRemoveMsgHandler(kNetCommAllMsgClasses, kNetCommAllMsgHandlers, handler);
    return true;
}

// SetDefaultHandler ----------------------------------------------
void plNetClientComm::SetDefaultHandler( MsgHandler* handler) {
    NetCommSetDefaultMsgHandler(MsgHandler::StaticMsgHandler, handler);
}

// MsgHandler::StaticMsgHandler ----------------------------------------------
int plNetClientComm::MsgHandler::StaticMsgHandler (plNetMessage * msg, void * userState) {
    plNetClientComm::MsgHandler * handler = (plNetClientComm::MsgHandler *) userState;
    return handler->HandleMessage(msg);
}
