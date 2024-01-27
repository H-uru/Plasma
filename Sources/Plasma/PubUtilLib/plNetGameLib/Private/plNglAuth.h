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
*   $/Plasma20/Sources/Plasma/PubUtilLib/plNetGameLib/Private/plNglAuth.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLNETGAMELIB_PRIVATE_PLNGLAUTH_H
#error "Header $/Plasma20/Sources/Plasma/PubUtilLib/plNetGameLib/Private/plNglAuth.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLNETGAMELIB_PRIVATE_PLNGLAUTH_H

#include <functional>
#include <vector>

#include "pnEncryption/plChecksum.h"

/*****************************************************************************
*
*   Client auth functions
*
***/

//============================================================================
// Connect
//============================================================================
void NetCliAuthStartConnect (
    const ST::string  authAddrList[],
    uint32_t        authAddrCount
);
bool NetCliAuthQueryConnected ();
void NetCliAuthAutoReconnectEnable (bool enable);   // is enabled by default

// Called after the auth/client connection is encrypted
typedef void (*FNetCliAuthConnectCallback)();
void NetCliAuthSetConnectCallback (
    FNetCliAuthConnectCallback callback
);


//============================================================================
// Server Capabilities
//============================================================================
bool NetCliAuthCheckCap(uint32_t cap);

//============================================================================
// Disconnect
//============================================================================
void NetCliAuthDisconnect ();
void NetCliAuthUnexpectedDisconnect ();

//============================================================================
// Ping
//============================================================================
typedef void (*FNetCliAuthPingRequestCallback)(
    ENetError   result,
    void *      param,
    unsigned    pingAtMs,
    unsigned    replyAtMs,
    unsigned    payloadBytes,
    const uint8_t  payload[]
);
void NetCliAuthPingRequest (
    unsigned                        pingTimeMs,
    unsigned                        payloadBytes,   // max 64k (pnNetCli enforced upon send)
    const void *                    payload,
    FNetCliAuthPingRequestCallback  callback,
    void *                          param
);

//============================================================================
// AccountExists
//============================================================================
typedef void (*FNetCliAuthAccountExistsRequestCallback)(
    ENetError   result,
    void *      param,
    bool        accountExists
);
void NetCliAuthAccountExistsRequest (
    const char16_t                              accountName[],
    FNetCliAuthAccountExistsRequestCallback     callback,
    void *                                      param
);  

//============================================================================
// Login
//============================================================================
struct NetCliAuthPlayerInfo {
    unsigned    playerInt;
    ST::string  playerName;
    ST::string  avatarShape;
    unsigned    playerFlags;
    unsigned    explorer;

    NetCliAuthPlayerInfo() : playerInt(), playerFlags(), explorer() { }
};

typedef void (*FNetCliAuthLoginRequestCallback)(
    ENetError                   result,
    void *                      param,
    const plUUID&               accountId,
    unsigned                    accountFlags,
    unsigned                    billingType,
    const NetCliAuthPlayerInfo  playerInfoArr[],
    unsigned                    playerCount
);
void NetCliAuthLoginRequest (
    const ST::string&               accountName,  // nil --> reuse previous acct name
    const ShaDigest *               accountNamePassHash,  // nil --> reuse previous acct pass
    const char16_t                  authToken[],  // nil --> reuse previous auth token
    const char16_t                  os[],  // nil --> reuse previous os
    FNetCliAuthLoginRequestCallback callback,
    void *                          param
);

//============================================================================
// Set Player
//============================================================================
typedef void (*FNetCliAuthSetPlayerRequestCallback)(
    ENetError       result,
    void *          param
);
void NetCliAuthSetPlayerRequest (
    unsigned                            playerInt,
    FNetCliAuthSetPlayerRequestCallback callback,
    void *                              param
);

//============================================================================
// Create Account
//============================================================================
typedef void (*FNetCliAuthAccountCreateRequestCallback)(
    ENetError                       result,
    void *                          param,
    const plUUID&                   accountId
);
void NetCliAuthAccountCreateRequest (
    const char16_t                          accountName[],
    const char16_t                          accountPass[],
    unsigned                                accountFlags,
    unsigned                                billingType,
    FNetCliAuthAccountCreateRequestCallback callback,
    void *                                  param
);

//============================================================================
// Create Account From Key
//============================================================================
typedef void (*FNetCliAuthAccountCreateFromKeyRequestCallback)(
    ENetError                       result,
    void *                          param,
    const plUUID&                   accountId,
    const plUUID&                   activationKey
);
void NetCliAuthAccountCreateFromKeyRequest (
    const char16_t                                  accountName[],
    const char16_t                                  accountPass[],
    plUUID                                          key,
    unsigned                                        billingType,
    FNetCliAuthAccountCreateFromKeyRequestCallback  callback,
    void *                                          param
);

//============================================================================
// Create Player
//============================================================================
typedef void (*FNetCliAuthPlayerCreateRequestCallback)(
    ENetError                       result,
    void *                          param,
    const NetCliAuthPlayerInfo &    playerInfo
);
void NetCliAuthPlayerCreateRequest (
    const ST::string&                       playerName,
    const ST::string&                       avatarShape,
    const ST::string&                       friendInvite,
    FNetCliAuthPlayerCreateRequestCallback  callback,
    void *                                  param
);

//============================================================================
// Delete Player
//============================================================================
typedef void (*FNetCliAuthPlayerDeleteRequestCallback)(
    ENetError                       result,
    void *                          param
);
void NetCliAuthPlayerDeleteRequest (
    unsigned                                playerId,
    FNetCliAuthPlayerDeleteRequestCallback  callback,
    void *                                  param
);

//============================================================================
// Upgrade Visitor
//============================================================================
typedef void (*FNetCliAuthUpgradeVisitorRequestCallback)(
    ENetError                       result,
    void *                          param
);
void NetCliAuthUpgradeVisitorRequest (
    unsigned                                    playerId,
    FNetCliAuthUpgradeVisitorRequestCallback    callback,
    void *                                      param
);

//============================================================================
// SetCCRLevel
//============================================================================
void NetCliAuthSetCCRLevel (
    unsigned                    ccrLevel
);

//============================================================================
// SetAgePublic
//============================================================================
void NetCliAuthSetAgePublic (
    unsigned                    ageInfoId,
    bool                        publicOrNot
);

//============================================================================
// GetPublicAgeList
//============================================================================
struct NetAgeInfo;
typedef void (*FNetCliAuthGetPublicAgeListCallback)(
    ENetError                   result,
    void *                      param,
    std::vector<NetAgeInfo>     ages
);
void NetCliAuthGetPublicAgeList (
    const ST::string&                   ageName,
    FNetCliAuthGetPublicAgeListCallback callback,
    void *                              param
);

//============================================================================
// Change Password
//============================================================================
typedef void (*FNetCliAuthAccountChangePasswordRequestCallback)(
    ENetError                       result,
    void *                          param
);
void NetCliAuthAccountChangePasswordRequest (
    const ST::string&                               accountName,
    const ST::string&                               accountPass,
    FNetCliAuthAccountChangePasswordRequestCallback callback,
    void *                                          param
);

//============================================================================
// Set Account Roles
//============================================================================
typedef void (*FNetCliAuthAccountSetRolesRequestCallback)(
    ENetError                       result,
    void *                          param
);
void NetCliAuthAccountSetRolesRequest (
    const char16_t                                  accountName[],
    unsigned                                        accountFlags,
    FNetCliAuthAccountSetRolesRequestCallback       callback,
    void *                                          param
);

//============================================================================
// Set Billing Type
//============================================================================
typedef void (*FNetCliAuthAccountSetBillingTypeRequestCallback)(
    ENetError                       result,
    void *                          param
);
void NetCliAuthAccountSetBillingTypeRequest (
    const char16_t                                  accountName[],
    unsigned                                        billingType,
    FNetCliAuthAccountSetBillingTypeRequestCallback callback,
    void *                                          param
);

//============================================================================
// Account Activate
//============================================================================
typedef void (*FNetCliAuthAccountActivateRequestCallback)(
    ENetError                       result,
    void *                          param
);
void NetCliAuthAccountActivateRequest (
    const plUUID&                               activationKey,
    FNetCliAuthAccountActivateRequestCallback   callback,
    void *                                      param
);

//============================================================================
// Age
//============================================================================
typedef void (*FNetCliAuthAgeRequestCallback)(
    ENetError       result,
    void *          param,
    unsigned        ageMcpId,
    unsigned        ageVaultId,
    const plUUID&   ageInstId,
    plNetAddress    gameAddr
);
void NetCliAuthAgeRequest (
    const ST::string&                   ageName,      // "Teledahn"
    const plUUID&                       ageInstId,
    FNetCliAuthAgeRequestCallback       callback,
    void *                              param
);

//============================================================================
// Secure File Encryption Key
//============================================================================
void NetCliAuthGetEncryptionKey (
    uint32_t      key[],
    size_t        size
);

//============================================================================
// File List
//============================================================================
struct NetCliAuthFileInfo {
    char16_t    filename[kNetDefaultStringSize];
    unsigned    filesize;
};
using FNetCliAuthFileListRequestCallback = std::function<void(
    ENetError                   result,
    const std::vector<NetCliAuthFileInfo>& infos
)>;
void NetCliAuthFileListRequest (
    const char16_t                      dir[],
    const char16_t                      ext[],
    FNetCliAuthFileListRequestCallback  callback
);

//============================================================================
// File Download
//============================================================================
using FNetCliAuthFileRequestCallback = std::function<void(ENetError result)>;
void NetCliAuthFileRequest (
    const plFileName &              filename,
    hsStream *                      writer,
    FNetCliAuthFileRequestCallback  callback
);

//============================================================================
// Vault Operations
//============================================================================
class NetVaultNode;
struct NetVaultNodeRef;

// VaultNodeChanged
using FNetCliAuthVaultNodeChanged = std::function<void(
    unsigned        nodeId,
    const plUUID&   revisionId
)>;
void NetCliAuthVaultSetRecvNodeChangedHandler (
    FNetCliAuthVaultNodeChanged handler
);
// VaultNodeAdded
using FNetCliAuthVaultNodeAdded = std::function<void(
    unsigned        parentId,
    unsigned        childId,
    unsigned        ownerId
)>;
void NetCliAuthVaultSetRecvNodeAddedHandler (
    FNetCliAuthVaultNodeAdded   handler
);
// VaultNodeRemoved
using FNetCliAuthVaultNodeRemoved = std::function<void(
    unsigned        parentId,
    unsigned        childId
)>;
void NetCliAuthVaultSetRecvNodeRemovedHandler (
    FNetCliAuthVaultNodeRemoved handler
);
// VaultNodeDeleted
using FNetCliAuthVaultNodeDeleted = std::function<void(
    unsigned        nodeId
)>;
void NetCliAuthVaultSetRecvNodeDeletedHandler (
    FNetCliAuthVaultNodeDeleted handler
);
// VaultNodeAdd
using FNetCliAuthVaultNodeAddCallback = std::function<void(ENetError result)>;
void NetCliAuthVaultNodeAdd (
    unsigned                        parentId,
    unsigned                        childId,
    unsigned                        ownerId,
    FNetCliAuthVaultNodeAddCallback callback
);
// VaultNodeRemove
using FNetCliAuthVaultNodeRemoveCallback = std::function<void(ENetError result)>;
void NetCliAuthVaultNodeRemove (
    unsigned                            parentId,
    unsigned                            childId,
    FNetCliAuthVaultNodeRemoveCallback  callback
);
// VaultNodeCreate
using FNetCliAuthVaultNodeCreated = std::function<void(
    ENetError           result,
    unsigned            nodeId
)>;
void NetCliAuthVaultNodeCreate (
    NetVaultNode *              templateNode,
    FNetCliAuthVaultNodeCreated callback
);
// VaultNodeFetch
using FNetCliAuthVaultNodeFetched = std::function<void(
    ENetError           result,
    NetVaultNode *      node
)>;
void NetCliAuthVaultNodeFetch (
    unsigned                    nodeId,
    FNetCliAuthVaultNodeFetched callback
);
// VaultNodeFind
using FNetCliAuthVaultNodeFind = std::function<void(
    ENetError           result,
    unsigned            nodeIdCount,
    const unsigned      nodeIds[]
)>;
void NetCliAuthVaultNodeFind (
    NetVaultNode *              templateNode,
    FNetCliAuthVaultNodeFind    callback
);
// VaultNodeSave
using FNetCliAuthVaultNodeSaveCallback = std::function<void(ENetError result)>;
unsigned NetCliAuthVaultNodeSave (  // returns number of bytes written
    NetVaultNode *                      node,
    FNetCliAuthVaultNodeSaveCallback    callback
);
void NetCliAuthVaultNodeDelete (
    unsigned                    nodeId
);
// FetchRefs (a vault structure only; no data)
using FNetCliAuthVaultNodeRefsFetched = std::function<void(
    ENetError           result,
    NetVaultNodeRef *   refs,
    unsigned            refCount
)>;
void NetCliAuthVaultFetchNodeRefs (
    unsigned                        nodeId,
    FNetCliAuthVaultNodeRefsFetched callback
);
void NetCliAuthVaultSetSeen (
    unsigned    parentId,
    unsigned    childId,
    bool        seen
);

void NetCliAuthVaultSendNode (
    unsigned    srcNodeId,
    unsigned    dstPlayerId
);

// Initialize an age vault. Will find existing match in db, or create a new age vault structure.
using FNetCliAuthAgeInitCallback = std::function<void(
    ENetError           result,
    unsigned            ageVaultId,
    unsigned            ageInfoVaultId
)>;
void NetCliAuthVaultInitAge (
    const plUUID&               ageInstId,          // optional. is used in match
    const plUUID&               parentAgeInstId,    // optional. is used in match
    const ST::string&           ageFilename,        // optional. is used in match
    const ST::string&           ageInstName,        // optional. not used in match
    const ST::string&           ageUserName,        // optional. not used in match
    const ST::string&           ageDesc,            // optional. not used in match
    unsigned                    ageSequenceNumber,  // optional. not used in match
    unsigned                    ageLanguage,        // optional. not used in match
    FNetCliAuthAgeInitCallback  callback
);

void NetCliAuthLogPythonTraceback (const char16_t traceback[]);

void NetCliAuthLogStackDump (const char16_t stackdump[]);
void NetCliAuthLogClientDebuggerConnect ();

//============================================================================
// SetPlayerBanStatusRequest
//============================================================================
typedef void (*FNetCliAuthSetPlayerBanStatusRequestCallback)(
    ENetError                       result,
    void *                          param
);
void NetCliAuthSetPlayerBanStatusRequest (
    unsigned                                        playerId,
    unsigned                                        banned,
    FNetCliAuthSetPlayerBanStatusRequestCallback    callback,
    void *                                          param
);

//============================================================================
// KickPlayerRequest
//============================================================================
void NetCliAuthKickPlayer (
    unsigned            playerId
);

//============================================================================
// ChangePlayerNameRequest
//============================================================================
typedef void (*FNetCliAuthChangePlayerNameRequestCallback)(
    ENetError                       result,
    void *                          param
);
void NetCliAuthChangePlayerNameRequest (
    unsigned                                    playerId,
    const char16_t                              newName[],
    FNetCliAuthChangePlayerNameRequestCallback  callback,
    void *                                      param
);

//============================================================================
// CCRPetition
//============================================================================
void NetCliAuthSendCCRPetition (
    const ST::string&     petitionText
);

//============================================================================
// SendFriendInvite
//============================================================================
typedef void (*FNetCliAuthSendFriendInviteCallback)(
    ENetError                           result,
    void *                              param
);

void NetCliAuthSendFriendInvite (
    const ST::string&                   emailAddress,
    const ST::string&                   toName,
    const plUUID&                       inviteUuid,
    FNetCliAuthSendFriendInviteCallback callback,
    void *                              param
);

//============================================================================
// Propagate app-specific data
//============================================================================
typedef void (*FNetCliAuthRecvBufferHandler)(
    unsigned                        type,
    unsigned                        bytes,
    const uint8_t                      buffer[]
);
void NetCliAuthSetRecvBufferHandler (
    FNetCliAuthRecvBufferHandler    handler
);
void NetCliAuthPropagateBuffer (
    unsigned                        type,
    unsigned                        bytes,
    const uint8_t                      buffer[]
);

//============================================================================
// New build notifications
//============================================================================
typedef void (*FNotifyNewBuildHandler)();
void NetCliAuthSetNotifyNewBuildHandler (FNotifyNewBuildHandler handler);

//============================================================================
// Score handling
//============================================================================
struct NetGameScore;

using FNetCliAuthScoreUpdateCallback = std::function<void(ENetError result)>;

//============================================================================
using FNetCliAuthCreateScoreCallback = std::function<void(
    ENetError       result,
    unsigned        scoreId,
    uint32_t        createdTime
)>;
void NetCliAuthScoreCreate(
    unsigned                        ownerId,
    const ST::string&               gameName,
    unsigned                        gameType,
    int                             value,
    FNetCliAuthCreateScoreCallback  callback
);

//============================================================================
void NetCliAuthScoreDelete(
    unsigned                        scoreId,
    FNetCliAuthScoreUpdateCallback  callback
);

//============================================================================
using FNetCliAuthGetScoresCallback = std::function<void(
    ENetError           result,
    const NetGameScore  scores[],
    unsigned            scoreCount
)>;

void NetCliAuthScoreGetScores(
    unsigned                        ownerId,
    const ST::string&               gameName,
    FNetCliAuthGetScoresCallback    callback
);

//============================================================================
void NetCliAuthScoreAddPoints(
    unsigned                        scoreId,
    int                             numPoints,
    FNetCliAuthScoreUpdateCallback  callback
);

//============================================================================
void NetCliAuthScoreTransferPoints(
    unsigned                        srcScoreId,
    unsigned                        destScoreId,
    int                             numPoints,
    FNetCliAuthScoreUpdateCallback  callback
);

//============================================================================
void NetCliAuthScoreSetPoints(
    unsigned                        scoreId,
    int                             numPoints,
    FNetCliAuthScoreUpdateCallback  callback
);

//============================================================================
struct NetGameRank;
using FNetCliAuthGetRanksCallback = std::function<void(
    ENetError           result,
    const NetGameRank   ranks[],
    unsigned            rankCount
)>;

void NetCliAuthScoreGetRankList(
    unsigned                    ownerId,
    unsigned                    scoreGroup,
    unsigned                    parentFolderId,
    const ST::string&           gameName,
    unsigned                    timePeriod,
    unsigned                    numResults,
    unsigned                    pageNumber,
    bool                        sortDesc,
    FNetCliAuthGetRanksCallback callback
);

//============================================================================
void NetCliAuthScoreGetHighScores(
    unsigned                        ageId,
    unsigned                        maxScores,
    const ST::string&               gameName,
    FNetCliAuthGetScoresCallback    callback
);
