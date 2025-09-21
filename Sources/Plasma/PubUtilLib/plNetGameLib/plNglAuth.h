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

#ifndef PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLNETGAMELIB_PLNGLAUTH_H
#define PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLNETGAMELIB_PLNGLAUTH_H

#include <functional>
#include <vector>

#include "pnEncryption/plChecksum.h"
#include "pnNetBase/pnNetBase.h"

class hsStream;
class plFileName;
class plNetAddress;
class plUUID;

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
using FNetCliAuthConnectCallback = std::function<void()>;
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
using FNetCliAuthPingRequestCallback = std::function<void(
    ENetError   result,
    unsigned    pingAtMs,
    unsigned    replyAtMs,
    unsigned    payloadBytes,
    const uint8_t  payload[]
)>;
void NetCliAuthPingRequest (
    unsigned                        pingTimeMs,
    unsigned                        payloadBytes,   // max 64k (pnNetCli enforced upon send)
    const void *                    payload,
    FNetCliAuthPingRequestCallback  callback
);

//============================================================================
// AccountExists
//============================================================================
using FNetCliAuthAccountExistsRequestCallback = std::function<void(
    ENetError   result,
    bool        accountExists
)>;
void NetCliAuthAccountExistsRequest (
    const char16_t                              accountName[],
    FNetCliAuthAccountExistsRequestCallback     callback
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

using FNetCliAuthLoginRequestCallback = std::function<void(
    ENetError                   result,
    const plUUID&               accountId,
    unsigned                    accountFlags,
    unsigned                    billingType,
    const NetCliAuthPlayerInfo  playerInfoArr[],
    unsigned                    playerCount
)>;
void NetCliAuthLoginRequest (
    const ST::string&               accountName,  // nil --> reuse previous acct name
    const ShaDigest *               accountNamePassHash,  // nil --> reuse previous acct pass
    const char16_t                  authToken[],  // nil --> reuse previous auth token
    const char16_t                  os[],  // nil --> reuse previous os
    FNetCliAuthLoginRequestCallback callback
);

//============================================================================
// Set Player
//============================================================================
using FNetCliAuthSetPlayerRequestCallback = std::function<void(ENetError result)>;
void NetCliAuthSetPlayerRequest (
    unsigned                            playerInt,
    FNetCliAuthSetPlayerRequestCallback callback
);

//============================================================================
// Create Account
//============================================================================
using FNetCliAuthAccountCreateRequestCallback = std::function<void(
    ENetError                       result,
    const plUUID&                   accountId
)>;
void NetCliAuthAccountCreateRequest (
    const char16_t                          accountName[],
    const char16_t                          accountPass[],
    unsigned                                accountFlags,
    unsigned                                billingType,
    FNetCliAuthAccountCreateRequestCallback callback
);

//============================================================================
// Create Account From Key
//============================================================================
using FNetCliAuthAccountCreateFromKeyRequestCallback = std::function<void(
    ENetError                       result,
    const plUUID&                   accountId,
    const plUUID&                   activationKey
)>;
void NetCliAuthAccountCreateFromKeyRequest (
    const char16_t                                  accountName[],
    const char16_t                                  accountPass[],
    plUUID                                          key,
    unsigned                                        billingType,
    FNetCliAuthAccountCreateFromKeyRequestCallback  callback
);

//============================================================================
// Create Player
//============================================================================
using FNetCliAuthPlayerCreateRequestCallback = std::function<void(
    ENetError                       result,
    const NetCliAuthPlayerInfo &    playerInfo
)>;
void NetCliAuthPlayerCreateRequest (
    const ST::string&                       playerName,
    const ST::string&                       avatarShape,
    const ST::string&                       friendInvite,
    FNetCliAuthPlayerCreateRequestCallback  callback
);

//============================================================================
// Delete Player
//============================================================================
using FNetCliAuthPlayerDeleteRequestCallback = std::function<void(ENetError result)>;
void NetCliAuthPlayerDeleteRequest (
    unsigned                                playerId,
    FNetCliAuthPlayerDeleteRequestCallback  callback
);

//============================================================================
// Upgrade Visitor
//============================================================================
using FNetCliAuthUpgradeVisitorRequestCallback = std::function<void(ENetError result)>;
void NetCliAuthUpgradeVisitorRequest (
    unsigned                                    playerId,
    FNetCliAuthUpgradeVisitorRequestCallback    callback
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
using FNetCliAuthGetPublicAgeListCallback = std::function<void(
    ENetError                   result,
    std::vector<NetAgeInfo>     ages
)>;
void NetCliAuthGetPublicAgeList (
    const ST::string&                   ageName,
    FNetCliAuthGetPublicAgeListCallback callback
);

//============================================================================
// Change Password
//============================================================================
using FNetCliAuthAccountChangePasswordRequestCallback = std::function<void(ENetError result)>;
void NetCliAuthAccountChangePasswordRequest (
    const ST::string&                               accountName,
    const ST::string&                               accountPass,
    FNetCliAuthAccountChangePasswordRequestCallback callback
);

//============================================================================
// Set Account Roles
//============================================================================
using FNetCliAuthAccountSetRolesRequestCallback = std::function<void(ENetError result)>;
void NetCliAuthAccountSetRolesRequest (
    const char16_t                                  accountName[],
    unsigned                                        accountFlags,
    FNetCliAuthAccountSetRolesRequestCallback       callback
);

//============================================================================
// Set Billing Type
//============================================================================
using FNetCliAuthAccountSetBillingTypeRequestCallback = std::function<void(ENetError result)>;
void NetCliAuthAccountSetBillingTypeRequest (
    const char16_t                                  accountName[],
    unsigned                                        billingType,
    FNetCliAuthAccountSetBillingTypeRequestCallback callback
);

//============================================================================
// Account Activate
//============================================================================
using FNetCliAuthAccountActivateRequestCallback = std::function<void(ENetError result)>;
void NetCliAuthAccountActivateRequest (
    const plUUID&                               activationKey,
    FNetCliAuthAccountActivateRequestCallback   callback
);

//============================================================================
// Age
//============================================================================
using FNetCliAuthAgeRequestCallback = std::function<void(
    ENetError       result,
    unsigned        ageMcpId,
    unsigned        ageVaultId,
    const plUUID&   ageInstId,
    plNetAddress    gameAddr
)>;
void NetCliAuthAgeRequest (
    const ST::string&                   ageName,      // "Teledahn"
    const plUUID&                       ageInstId,
    FNetCliAuthAgeRequestCallback       callback
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
using FNetCliAuthSetPlayerBanStatusRequestCallback = std::function<void(ENetError result)>;
void NetCliAuthSetPlayerBanStatusRequest (
    unsigned                                        playerId,
    unsigned                                        banned,
    FNetCliAuthSetPlayerBanStatusRequestCallback    callback
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
using FNetCliAuthChangePlayerNameRequestCallback = std::function<void(ENetError result)>;
void NetCliAuthChangePlayerNameRequest (
    unsigned                                    playerId,
    const char16_t                              newName[],
    FNetCliAuthChangePlayerNameRequestCallback  callback
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
using FNetCliAuthSendFriendInviteCallback = std::function<void(ENetError result)>;

void NetCliAuthSendFriendInvite (
    const ST::string&                   emailAddress,
    const ST::string&                   toName,
    const plUUID&                       inviteUuid,
    FNetCliAuthSendFriendInviteCallback callback
);

//============================================================================
// Propagate app-specific data
//============================================================================
using FNetCliAuthRecvBufferHandler = std::function<void(
    unsigned                        type,
    unsigned                        bytes,
    const uint8_t                      buffer[]
)>;
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
using FNotifyNewBuildHandler = std::function<void()>;
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
    const std::vector<NetGameScore>& scores
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

#endif // PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLNETGAMELIB_PLNGLAUTH_H
