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

#ifndef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PNNPCLI2AUTH_H
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PNNPCLI2AUTH_H


// kNetProtocolCli2Auth messages (must be <= (uint16_t)-1)
enum {
    // Global
    kCli2Auth_PingRequest,

    // Client
    kCli2Auth_ClientRegisterRequest,
    kCli2Auth_ClientSetCCRLevel,

    // Account
    kCli2Auth_AcctLoginRequest,
    kCli2Auth_AcctSetEulaVersion,
    kCli2Auth_AcctSetDataRequest,
    kCli2Auth_AcctSetPlayerRequest,
    kCli2Auth_AcctCreateRequest,
    kCli2Auth_AcctChangePasswordRequest,
    kCli2Auth_AcctSetRolesRequest,
    kCli2Auth_AcctSetBillingTypeRequest,
    kCli2Auth_AcctActivateRequest,
    kCli2Auth_AcctCreateFromKeyRequest,
    
    // Player
    kCli2Auth_PlayerDeleteRequest,
    kCli2Auth_PlayerUndeleteRequest,
    kCli2Auth_PlayerSelectRequest,
    kCli2Auth_PlayerRenameRequest,
    kCli2Auth_PlayerCreateRequest,
    kCli2Auth_PlayerSetStatus,
    kCli2Auth_PlayerChat,
    kCli2Auth_UpgradeVisitorRequest,
    kCli2Auth_SetPlayerBanStatusRequest,
    kCli2Auth_KickPlayer,
    kCli2Auth_ChangePlayerNameRequest,
    kCli2Auth_SendFriendInviteRequest,

    // Vault
    kCli2Auth_VaultNodeCreate,
    kCli2Auth_VaultNodeFetch,
    kCli2Auth_VaultNodeSave,
    kCli2Auth_VaultNodeDelete,
    kCli2Auth_VaultNodeAdd,
    kCli2Auth_VaultNodeRemove,
    kCli2Auth_VaultFetchNodeRefs,
    kCli2Auth_VaultInitAgeRequest,
    kCli2Auth_VaultNodeFind,
    kCli2Auth_VaultSetSeen,
    kCli2Auth_VaultSendNode,

    // Ages
    kCli2Auth_AgeRequest,

    // File-related
    kCli2Auth_FileListRequest,
    kCli2Auth_FileDownloadRequest,
    kCli2Auth_FileDownloadChunkAck,

    // Game
    kCli2Auth_PropagateBuffer,
    

    // Public ages    
    kCli2Auth_GetPublicAgeList,
    kCli2Auth_SetAgePublic,

    // Log Messages
    kCli2Auth_LogPythonTraceback,
    kCli2Auth_LogStackDump,
    kCli2Auth_LogClientDebuggerConnect,

    // Score
    kCli2Auth_ScoreCreate,
    kCli2Auth_ScoreDelete,
    kCli2Auth_ScoreGetScores,
    kCli2Auth_ScoreAddPoints,
    kCli2Auth_ScoreTransferPoints,
    kCli2Auth_ScoreSetPoints,
    kCli2Auth_ScoreGetRanks,

    kCli2Auth_AccountExistsRequest,

    // Extension messages
    kCli2Auth_AgeRequestEx = 0x1000,
    kCli2Auth_ScoreGetHighScores,

    kNumCli2AuthMessages
};
static_assert(kNumCli2AuthMessages <= 0xFFFF, "Cli2Auth message types overflow uint16");

enum {
    // Global
    kAuth2Cli_PingReply,
    kAuth2Cli_ServerAddr,
    kAuth2Cli_NotifyNewBuild,

    // Client
    kAuth2Cli_ClientRegisterReply,

    // Account
    kAuth2Cli_AcctLoginReply,
    kAuth2Cli_AcctData,
    kAuth2Cli_AcctPlayerInfo,
    kAuth2Cli_AcctSetPlayerReply,
    kAuth2Cli_AcctCreateReply,
    kAuth2Cli_AcctChangePasswordReply,
    kAuth2Cli_AcctSetRolesReply,
    kAuth2Cli_AcctSetBillingTypeReply,
    kAuth2Cli_AcctActivateReply,
    kAuth2Cli_AcctCreateFromKeyReply,
    
    // Player
    kAuth2Cli_PlayerList,
    kAuth2Cli_PlayerChat,
    kAuth2Cli_PlayerCreateReply,
    kAuth2Cli_PlayerDeleteReply,
    kAuth2Cli_UpgradeVisitorReply,
    kAuth2Cli_SetPlayerBanStatusReply,
    kAuth2Cli_ChangePlayerNameReply,
    kAuth2Cli_SendFriendInviteReply,

    // Friends
    kAuth2Cli_FriendNotify,

    // Vault
    kAuth2Cli_VaultNodeCreated,
    kAuth2Cli_VaultNodeFetched,
    kAuth2Cli_VaultNodeChanged,
    kAuth2Cli_VaultNodeDeleted,
    kAuth2Cli_VaultNodeAdded,
    kAuth2Cli_VaultNodeRemoved,
    kAuth2Cli_VaultNodeRefsFetched,
    kAuth2Cli_VaultInitAgeReply,
    kAuth2Cli_VaultNodeFindReply,
    kAuth2Cli_VaultSaveNodeReply,
    kAuth2Cli_VaultAddNodeReply,
    kAuth2Cli_VaultRemoveNodeReply,

    // Ages
    kAuth2Cli_AgeReply,

    // File-related
    kAuth2Cli_FileListReply,
    kAuth2Cli_FileDownloadChunk,

    // Game
    kAuth2Cli_PropagateBuffer,
    
    // Admin
    kAuth2Cli_KickedOff,

    // Public ages    
    kAuth2Cli_PublicAgeList,

    // Score
    kAuth2Cli_ScoreCreateReply,
    kAuth2Cli_ScoreDeleteReply,
    kAuth2Cli_ScoreGetScoresReply,
    kAuth2Cli_ScoreAddPointsReply,
    kAuth2Cli_ScoreTransferPointsReply,
    kAuth2Cli_ScoreSetPointsReply,
    kAuth2Cli_ScoreGetRanksReply,

    kAuth2Cli_AccountExistsReply,

    // Extension messages
    kAuth2Cli_AgeReplyEx = 0x1000,
    kAuth2Cli_ScoreGetHighScoresReply,
    kAuth2Cli_ServerCaps,

    kNumAuth2CliMessages
};
static_assert(kNumAuth2CliMessages <= 0xFFFF, "Auth2Cli message types overflow uint16");


//============================================================================
// BEGIN PACKED DATA STRUCTURES
//============================================================================
#pragma pack(push,1)


/*****************************************************************************
*
*   Cli2Auth connect packet
*
***/

struct Cli2Auth_ConnData {
    uint32_t       dataBytes;
    plUUID      token;
};
struct Cli2Auth_Connect {
    AsyncSocketConnectPacket    hdr;
    Cli2Auth_ConnData           data;
};


/*****************************************************************************
*
*   Cli2Auth message structures
*
***/

// PingRequest
extern const NetMsg kNetMsg_Cli2Auth_PingRequest;
struct Cli2Auth_PingRequest {
    uint32_t       messageId;
    uint32_t       pingTimeMs;
    uint32_t       transId;
    uint32_t       payloadBytes;
    uint8_t        payload[1]; // [payloadBytes]
};

// ClientRegisterRequest
extern const NetMsg kNetMsg_Cli2Auth_ClientRegisterRequest;
struct Cli2Auth_ClientRegisterRequest {
    uint32_t       messageId;
    uint32_t       buildId;
};

// AccountExists
extern const NetMsg kNetMsg_Cli2Auth_AccountExistsRequest;
struct Cli2Auth_AccountExistsRequest {
    uint32_t messageId;
    uint32_t transId;
    char16_t accountName[kMaxAccountNameLength];
};

// LoginRequest
extern const NetMsg kNetMsg_Cli2Auth_AcctLoginRequest;
struct Cli2Auth_AcctLoginRequest {
    uint32_t       messageId;
    uint32_t       transId;
    uint32_t       clientChallenge;
    char16_t       acctName[kMaxAccountNameLength];
    ShaDigest   challengeHash;
    char16_t       authToken[kMaxPublisherAuthKeyLength];
    char16_t       os[kMaxGTOSIdLength];
};

// AgeRequest
extern const NetMsg kNetMsg_Cli2Auth_AgeRequest;
struct Cli2Auth_AgeRequest {
    uint32_t       messageId;
    uint32_t       transId;
    char16_t       ageName[kMaxAgeNameLength];
    plUUID      ageUuid;
};

// AcctCreateRequest
extern const NetMsg kNetMsg_Cli2Auth_AcctCreateRequest;
struct Cli2Auth_AcctCreateRequest {
    uint32_t       messageId;
    uint32_t       transId;
    char16_t       accountName[kMaxAccountNameLength];
    ShaDigest   namePassHash;
    uint32_t       accountFlags;
    uint32_t       billingType;
};

// AcctCreateFromKeyRequest
extern const NetMsg kNetMsg_Cli2Auth_AcctCreateFromKeyRequest;
struct Cli2Auth_AcctCreateFromKeyRequest {
    uint32_t       messageId;
    uint32_t       transId;
    char16_t       accountName[kMaxAccountNameLength];
    ShaDigest   namePassHash;
    plUUID      key;
    uint32_t       billingType;
};

// CreatePlayerRequest
extern const NetMsg kNetMsg_Cli2Auth_PlayerCreateRequest;
struct Cli2Auth_PlayerCreateRequest {
    uint32_t       messageId;
    uint32_t       transId;
    char16_t       playerName[kMaxPlayerNameLength];
    char16_t       avatarShape[kNetDefaultStringSize];
    char16_t       friendInvite[kNetDefaultStringSize];
};

extern const NetMsg kNetMsg_Cli2Auth_PlayerDeleteRequest;
struct Cli2Auth_PlayerDeleteRequest {
    uint32_t       messageId;
    uint32_t       transId;
    uint32_t       playerId;
};

extern const NetMsg kNetMsg_Cli2Auth_UpgradeVisitorRequest;
struct Cli2Auth_UpgradeVisitorRequest {
    uint32_t       messageId;
    uint32_t       transId;
    uint32_t       playerId;
};

// SetPlayerRequest
extern const NetMsg kNetMsg_Cli2Auth_AcctSetPlayerRequest;
struct Cli2Auth_AcctSetPlayerRequest {
    uint32_t       messageId;
    uint32_t       transId;
    uint32_t       playerInt;
};

// ChangePasswordRequest
extern const NetMsg kNetMsg_Cli2Auth_AcctChangePasswordRequest;
struct Cli2Auth_AcctChangePasswordRequest {
    uint32_t       messageId;
    uint32_t       transId;
    char16_t       accountName[kMaxAccountNameLength];
    ShaDigest   namePassHash;
};

// AcctSetRolesRequest
extern const NetMsg kNetMsg_Cli2Auth_AcctSetRolesRequest;
struct Cli2Auth_AcctSetRolesRequest {
    uint32_t       messageId;
    uint32_t       transId;
    char16_t       accountName[kMaxAccountNameLength];
    uint32_t       accountFlags;
};

// AcctSetBillingTypeRequest
extern const NetMsg kNetMsg_Cli2Auth_AcctSetBillingTypeRequest;
struct Cli2Auth_AcctSetBillingTypeRequest {
    uint32_t       messageId;
    uint32_t       transId;
    char16_t       accountName[kMaxAccountNameLength];
    uint32_t       billingType;
};

// AcctActivateRequest
extern const NetMsg kNetMsg_Cli2Auth_AcctActivateRequest;
struct Cli2Auth_AcctActivateRequest {
    uint32_t       messageId;
    uint32_t       transId;
    plUUID      activationKey;
};

// FileListRequest
extern const NetMsg kNetMsg_Cli2Auth_FileListRequest;
struct Cli2Auth_FileListRequest {
    uint32_t       messageId;
    uint32_t       transId;
    char16_t       directory[kNetDefaultStringSize];
    char16_t       ext[kMaxFileExtensionLength];
};

// FileDownloadRequest
extern const NetMsg kNetMsg_Cli2Auth_FileDownloadRequest;
struct Cli2Auth_FileDownloadRequest {
    uint32_t       messageId;
    uint32_t       transId;
    char16_t       filename[kNetDefaultStringSize];
};

// FileDownloadChunkAck
extern const NetMsg kNetMsg_Cli2Auth_FileDownloadChunkAck;
struct Cli2Auth_FileDownloadChunkAck {
    uint32_t       messageId;
    uint32_t       transId;
};

// VaultFetchNodeRefs
extern const NetMsg kNetMsg_Cli2Auth_VaultFetchNodeRefs;
struct Cli2Auth_VaultFetchNodeRefs {
    uint32_t       messageId;
    uint32_t       transId;
    uint32_t       nodeId;
};

// VaultNodeAdd
extern const NetMsg kNetMsg_Cli2Auth_VaultNodeAdd;
struct Cli2Auth_VaultNodeAdd {
    uint32_t       messageId;
    uint32_t       transId;
    uint32_t       parentId;
    uint32_t       childId;
    uint32_t       ownerId;
};

// VaultNodeRemove
extern const NetMsg kNetMsg_Cli2Auth_VaultNodeRemove;
struct Cli2Auth_VaultNodeRemove {
    uint32_t       messageId;
    uint32_t       transId;
    uint32_t       parentId;
    uint32_t       childId;
};

// VaultNodeSave
extern const NetMsg kNetMsg_Cli2Auth_VaultNodeSave;
struct Cli2Auth_VaultNodeSave {
    uint32_t       messageId;
    uint32_t       transId;
    uint32_t       nodeId;
    plUUID      revisionId;
    uint32_t       nodeBytes;
    uint8_t        nodeBuffer[1];
};

// VaultNodeCreate
extern const NetMsg kNetMsg_Cli2Auth_VaultNodeCreate;
struct Cli2Auth_VaultNodeCreate {
    uint32_t       messageId;
    uint32_t       transId;
    uint32_t       nodeBytes;
    uint8_t        nodeBuffer[1];
};

// VaultNodeFetch
extern const NetMsg kNetMsg_Cli2Auth_VaultNodeFetch;
struct Cli2Auth_VaultNodeFetch {
    uint32_t       messageId;
    uint32_t       transId;
    uint32_t       nodeId;
};

// VaultInitAgeRequest
extern const NetMsg kNetMsg_Cli2Auth_VaultInitAgeRequest;
struct Cli2Auth_VaultInitAgeRequest {
    uint32_t       messageId;
    uint32_t       transId;
    plUUID      ageInstId;
    plUUID      parentAgeInstId;
    char16_t       ageFilename[kNetDefaultStringSize];
    char16_t       ageInstName[kNetDefaultStringSize];
    char16_t       ageUserName[kNetDefaultStringSize];
    char16_t       ageDesc[1024];
    uint32_t       ageSequenceNumber;
    uint32_t       ageLanguage;
};

// VaultNodeFind
extern const NetMsg kNetMsg_Cli2Auth_VaultNodeFind;
struct Cli2Auth_VaultNodeFind {
    uint32_t       messageId;
    uint32_t       transId;
    uint32_t       nodeBytes;
    uint8_t        nodeBuffer[1];
};

// VaultSetSeen
extern const NetMsg kNetMsg_Cli2Auth_VaultSetSeen;
struct Cli2Auth_VaultSetSeen {
    uint32_t       messageId;
    uint32_t       parentId;
    uint32_t       childId;
    uint8_t        seen;
};

// VaultSendNode
extern const NetMsg kNetMsg_Cli2Auth_VaultSendNode;
struct Cli2Auth_VaultSendNode {
    uint32_t       messageId;
    uint32_t       srcNodeId;
    uint32_t       dstPlayerId;
};

// GetPublicAgeList
extern const NetMsg kNetMsg_Cli2Auth_GetPublicAgeList;
struct Cli2Auth_GetPublicAgeList {
    uint32_t       messageId;
    uint32_t       transId;
    char16_t       ageFilename[kMaxAgeNameLength];
};

extern const NetMsg kNetMsg_Cli2Auth_SetAgePublic;
struct Cli2Auth_SetAgePublic {
    uint32_t       messageId;
    uint32_t       ageInfoId;
    uint8_t        publicOrNot;
};

// PropagateBuffer
extern const NetMsg kNetMsg_Cli2Auth_PropagateBuffer;
struct Cli2Auth_PropagateBuffer {
    uint32_t       messageId;
    uint32_t       type;
    uint32_t       bytes;
    uint8_t        buffer[1];  // [bytes], actually
    // no more fields
};

extern const NetMsg kNetMsg_Cli2Auth_ClientSetCCRLevel;
struct Cli2Auth_ClientSetCCRLevel {
    uint32_t       messageId;
    uint32_t       ccrLevel;
};

extern const NetMsg kNetMsg_Cli2Auth_LogPythonTraceback;
struct Cli2Auth_LogPythonTraceback {
    uint32_t messageId;
    char16_t traceback[1024];
};

extern const NetMsg kNetMsg_Cli2Auth_LogStackDump;
struct Cli2Auth_LogStackDump {
    uint32_t messageId;
    char16_t stackdump[1024];
};

extern const NetMsg kNetMsg_Cli2Auth_LogClientDebuggerConnect;
struct Cli2Auth_LogClientDebuggerConnect {
    uint32_t   messageId;
};

extern const NetMsg kNetMsg_Cli2Auth_SetPlayerBanStatusRequest;
struct Cli2Auth_SetPlayerBanStatusRequest {
    uint32_t messageId;
    uint32_t transId;
    uint32_t playerId;
    uint32_t banned;
};

extern const NetMsg kNetMsg_Cli2Auth_KickPlayer;
struct Cli2Auth_KickPlayer {
    uint32_t messageId;
    uint32_t playerId;
};

extern const NetMsg kNetMsg_Cli2Auth_ChangePlayerNameRequest;
struct Cli2Auth_ChangePlayerNameRequest {
    uint32_t messageId;
    uint32_t transId;
    uint32_t playerId;
    char16_t newName[kMaxPlayerNameLength];
};

extern const NetMsg kNetMsg_Cli2Auth_SendFriendInviteRequest;
struct Cli2Auth_SendFriendInviteRequest {
    uint32_t   messageId;
    uint32_t   transId;
    plUUID  inviteUuid;
    char16_t   emailAddress[kMaxEmailAddressLength];
    char16_t   toName[kMaxPlayerNameLength];
};

extern const NetMsg kNetMsg_Cli2Auth_ScoreCreate;
struct Cli2Auth_ScoreCreate {
    uint32_t messageId;
    uint32_t transId;
    uint32_t ownerId;
    char16_t gameName[kMaxGameScoreNameLength];
    uint32_t gameType;
    uint32_t scoreValue;
};

extern const NetMsg kNetMsg_Cli2Auth_ScoreDelete;
struct Cli2Auth_ScoreDelete {
    uint32_t messageId;
    uint32_t transId;
    uint32_t scoreId;
};

extern const NetMsg kNetMsg_Cli2Auth_ScoreGetScores;
struct Cli2Auth_ScoreGetScores {
    uint32_t messageId;
    uint32_t transId;
    uint32_t ownerId;
    char16_t gameName[kMaxGameScoreNameLength];
};

extern const NetMsg kNetMsg_Cli2Auth_ScoreAddPoints;
struct Cli2Auth_ScoreAddPoints {
    uint32_t messageId;
    uint32_t transId;
    uint32_t scoreId;
    uint32_t numPoints;
};

extern const NetMsg kNetMsg_Cli2Auth_ScoreTransferPoints;
struct Cli2Auth_ScoreTransferPoints {
    uint32_t messageId;
    uint32_t transId;
    uint32_t srcScoreId;
    uint32_t destScoreId;
    uint32_t numPoints;
};

extern const NetMsg kNetMsg_Cli2Auth_ScoreSetPoints;
struct Cli2Auth_ScoreSetPoints {
    uint32_t messageId;
    uint32_t transId;
    uint32_t scoreId;
    uint32_t numPoints;
};

extern const NetMsg kNetMsg_Cli2Auth_ScoreGetRanks;
struct Cli2Auth_ScoreGetRanks {
    uint32_t messageId;
    uint32_t transId;
    uint32_t ownerId;
    uint32_t scoreGroup;
    uint32_t parentFolderId;
    char16_t gameName[kMaxGameScoreNameLength];
    uint32_t timePeriod;
    uint32_t numResults;
    uint32_t pageNumber;
    uint32_t sortDesc;
};

extern const NetMsg kNetMsg_Cli2Auth_ScoreGetHighScores;
struct Cli2Auth_ScoreGetHighScores {
    uint32_t messageId;
    uint32_t transId;
    uint32_t ageId;
    uint32_t maxScores;
    char16_t gameName[kMaxGameScoreNameLength];
};


/*****************************************************************************
*
*   Auth2Cli message structures
*
***/

// PingReply
extern const NetMsg kNetMsg_Auth2Cli_PingReply;
struct Auth2Cli_PingReply {
    uint32_t       messageId;
    uint32_t       pingTimeMs;
    uint32_t       transId;
    uint32_t       payloadBytes;
    uint8_t        payload[1]; // [payloadBytes]
};

// ClientRegisterReply
extern const NetMsg kNetMsg_Auth2Cli_ClientRegisterReply;
struct Auth2Cli_ClientRegisterReply {
    uint32_t       messageId;
    uint32_t       serverChallenge;
};

// AccountExists
extern const NetMsg kNetMsg_Auth2Cli_AccountExistsReply;
struct Auth2Cli_AccountExistsReply {
    uint32_t       messageId;
    uint32_t       transId;
    ENetError   result;
    uint8_t        exists;
};

// ServerAddr
extern const NetMsg kNetMsg_Auth2Cli_ServerAddr;
struct Auth2Cli_ServerAddr {
    uint32_t messageId;
    uint32_t srvAddr;
    plUUID          token;
};

extern const NetMsg kNetMsg_Auth2Cli_NotifyNewBuild;
struct Auth2Cli_NotifyNewBuild {
    uint32_t           foo;        // msgs must have at least one field
};

// AcctPlayerInfo
extern const NetMsg kNetMsg_Auth2Cli_AcctPlayerInfo;
struct Auth2Cli_AcctPlayerInfo {
    uint32_t       messageId;
    uint32_t       transId;
    uint32_t       playerInt;
    char16_t       playerName[kMaxPlayerNameLength];
    char16_t       avatarShape[kMaxVaultNodeStringLength];
    uint32_t       explorer;
};

// LoginReply
extern const NetMsg kNetMsg_Auth2Cli_AcctLoginReply;
struct Auth2Cli_AcctLoginReply {
    uint32_t       messageId;
    uint32_t       transId;
    ENetError   result;
    plUUID      accountId;
    uint32_t       accountFlags;
    uint32_t       billingType;
    uint32_t       encryptionKey[4];
};

// AgeReply
extern const NetMsg kNetMsg_Auth2Cli_AgeReply;
struct Auth2Cli_AgeReply {
    uint32_t  messageId;
    uint32_t  transId;
    ENetError result;
    uint32_t  ageMcpId;
    plUUID    ageInstId;
    uint32_t  ageVaultId;
    uint32_t  gameSrvNode;
};

// AcctCreateReply
extern const NetMsg kNetMsg_Auth2Cli_AcctCreateReply;
struct Auth2Cli_AcctCreateReply {
    uint32_t           messageId;
    uint32_t           transId;
    ENetError       result;
    plUUID          accountId;
};

// AcctCreateFromKeyReply
extern const NetMsg kNetMsg_Auth2Cli_AcctCreateFromKeyReply;
struct Auth2Cli_AcctCreateFromKeyReply {
    uint32_t           messageId;
    uint32_t           transId;
    ENetError       result;
    plUUID          accountId;
    plUUID          activationKey;
};

// CreatePlayerReply
extern const NetMsg kNetMsg_Auth2Cli_PlayerCreateReply;
struct Auth2Cli_PlayerCreateReply {
    uint32_t           messageId;
    uint32_t           transId;
    ENetError       result;
    uint32_t           playerInt;
    uint32_t           explorer;
    char16_t           playerName[kMaxPlayerNameLength];
    char16_t           avatarShape[kMaxVaultNodeStringLength];
};

// DeletePlayerReply
extern const NetMsg kNetMsg_Auth2Cli_PlayerDeleteReply;
struct Auth2Cli_PlayerDeleteReply {
    uint32_t           messageId;
    uint32_t           transId;
    ENetError       result;
};

// DeletePlayerReply
extern const NetMsg kNetMsg_Auth2Cli_UpgradeVisitorReply;
struct Auth2Cli_UpgradeVisitorReply {
    uint32_t           messageId;
    uint32_t           transId;
    ENetError       result;
};

// SetPlayerReply
extern const NetMsg kNetMsg_Auth2Cli_AcctSetPlayerReply;
struct Auth2Cli_AcctSetPlayerReply {
    uint32_t           messageId;
    uint32_t           transId;
    ENetError       result;
};

// AcctChangePasswordReply
extern const NetMsg kNetMsg_Auth2Cli_AcctChangePasswordReply;
struct Auth2Cli_AcctChangePasswordReply {
    uint32_t           messageId;
    uint32_t           transId;
    ENetError       result;
};

// AcctSetRolesReply
extern const NetMsg kNetMsg_Auth2Cli_AcctSetRolesReply;
struct Auth2Cli_AcctSetRolesReply {
    uint32_t           messageId;
    uint32_t           transId;
    ENetError       result;
};

// AcctSetBillingTypeReply
extern const NetMsg kNetMsg_Auth2Cli_AcctSetBillingTypeReply;
struct Auth2Cli_AcctSetBillingTypeReply {
    uint32_t           messageId;
    uint32_t           transId;
    ENetError       result;
};

// AcctActivateReply
extern const NetMsg kNetMsg_Auth2Cli_AcctActivateReply;
struct Auth2Cli_AcctActivateReply {
    uint32_t           messageId;
    uint32_t           transId;
    ENetError       result;
};

// FileListReply
extern const NetMsg kNetMsg_Auth2Cli_FileListReply;
struct Auth2Cli_FileListReply {
    uint32_t           messageId;
    uint32_t           transId;
    ENetError       result;
    uint32_t           wcharCount;
    char16_t           fileData[1];        // [wcharCount], actually
    // no more fields
};

// FileDownloadChunk
extern const NetMsg kNetMsg_Auth2Cli_FileDownloadChunk;
struct Auth2Cli_FileDownloadChunk {
    uint32_t           messageId;
    uint32_t           transId;
    ENetError       result;
    uint32_t           fileSize;
    uint32_t           chunkOffset;
    uint32_t           chunkSize;
    uint8_t            chunkData[1];       // [chunkSize], actually
    // no more fields
};

// KickedOff
extern const NetMsg kNetMsg_Auth2Cli_KickedOff;
struct Auth2Cli_KickedOff {
    uint32_t           messageId;
    ENetError       reason;
};

extern const NetMsg kNetMsg_Auth2Cli_VaultNodeRefsFetched;
struct Auth2Cli_VaultNodeRefsFetched {
    uint32_t                   messageId;
    uint32_t                   transId;
    ENetError               result;
    uint32_t                   refCount;
    NetVaultNodeRef         refs[1];
};

extern const NetMsg kNetMsg_Auth2Cli_VaultNodeCreated;
struct Auth2Cli_VaultNodeCreated {
    uint32_t                   messageId;
    uint32_t                   transId;
    ENetError               result;
    uint32_t                   nodeId;
};

extern const NetMsg kNetMsg_Auth2Cli_VaultNodeFetched;
struct Auth2Cli_VaultNodeFetched {
    uint32_t                   messageId;
    uint32_t                   transId;
    ENetError               result;
    uint32_t                   nodeBytes;
    uint8_t                    nodeBuffer[1];
};

extern const NetMsg kNetMsg_Auth2Cli_VaultNodeChanged;
struct Auth2Cli_VaultNodeChanged {
    uint32_t                   messageId;
    uint32_t                   nodeId;
    plUUID                  revisionId;
};

extern const NetMsg kNetMsg_Auth2Cli_VaultNodeAdded;
struct Auth2Cli_VaultNodeAdded {
    uint32_t                   messageId;
    uint32_t                   parentId;
    uint32_t                   childId;
    uint32_t                   ownerId;
};

extern const NetMsg kNetMsg_Auth2Cli_VaultNodeRemoved;
struct Auth2Cli_VaultNodeRemoved {
    uint32_t                   messageId;
    uint32_t                   parentId;
    uint32_t                   childId;
};

extern const NetMsg kNetMsg_Auth2Cli_VaultNodeDeleted;
struct Auth2Cli_VaultNodeDeleted {
    uint32_t                   messageId;
    uint32_t                   nodeId;
};

extern const NetMsg kNetMsg_Auth2Cli_VaultSaveNodeReply;
struct Auth2Cli_VaultSaveNodeReply {
    uint32_t           messageId;
    uint32_t           transId;
    ENetError       result;
};

extern const NetMsg kNetMsg_Auth2Cli_VaultAddNodeReply;
struct Auth2Cli_VaultAddNodeReply {
    uint32_t           messageId;
    uint32_t           transId;
    ENetError       result;
};

extern const NetMsg kNetMsg_Auth2Cli_VaultRemoveNodeReply;
struct Auth2Cli_VaultRemoveNodeReply {
    uint32_t           messageId;
    uint32_t           transId;
    ENetError       result;
};

extern const NetMsg kNetMsg_Auth2Cli_VaultInitAgeReply;
struct Auth2Cli_VaultInitAgeReply {
    uint32_t                   messageId;
    uint32_t                   transId;
    ENetError               result;
    uint32_t                   ageVaultId;
    uint32_t                   ageInfoVaultId;
};

extern const NetMsg kNetMsg_Auth2Cli_VaultNodeFindReply;
struct Auth2Cli_VaultNodeFindReply {
    uint32_t                   messageId;
    uint32_t                   transId;
    ENetError               result;
    uint32_t                   nodeIdCount;
    uint32_t                   nodeIds[1];
};

// PublicAgeList
extern const NetMsg kNetMsg_Auth2Cli_PublicAgeList;
struct Auth2Cli_PublicAgeList {
    uint32_t               messageId;
    uint32_t               transId;
    ENetError           result;
    uint32_t               ageCount;
    NetAgeInfo          ages[1];    // [ageCount], actually
};

// PropagateBuffer
extern const NetMsg kNetMsg_Auth2Cli_PropagateBuffer;
struct Auth2Cli_PropagateBuffer {
    uint32_t       messageId;
    uint32_t       type;
    uint32_t       bytes;
    uint8_t        buffer[1];  // [bytes], actually
    // no more fields
};

// SetPlayerBanStatusReply
extern const NetMsg kNetMsg_Auth2Cli_SetPlayerBanStatusReply;
struct Auth2Cli_SetPlayerBanStatusReply {
    uint32_t           messageId;
    uint32_t           transId;
    ENetError       result;
};

// ChangePlayerNameReply
extern const NetMsg kNetMsg_Auth2Cli_ChangePlayerNameReply;
struct Auth2Cli_ChangePlayerNameReply {
    uint32_t           messageId;
    uint32_t           transId;
    ENetError       result;
};

// SendFriendInviteReply
extern const NetMsg kNetMsg_Auth2Cli_SendFriendInviteReply;
struct Auth2Cli_SendFriendInviteReply {
    uint32_t           messageId;
    uint32_t           transId;
    ENetError       result;
};

// ScoreCreateReply
extern const NetMsg kNetMsg_Auth2Cli_ScoreCreateReply;
struct Auth2Cli_ScoreCreateReply {
    uint32_t           messageId;
    uint32_t           transId;
    ENetError       result;
    uint32_t           scoreId;
    uint32_t           createdTime;
};

// ScoreDeleteReply
extern const NetMsg kNetMsg_Auth2Cli_ScoreDeleteReply;
struct Auth2Cli_ScoreDeleteReply {
    uint32_t           messageId;
    uint32_t           transId;
    ENetError       result;
};

// ScoreGetScoresReply
extern const NetMsg kNetMsg_Auth2Cli_ScoreGetScoresReply;
struct Auth2Cli_ScoreGetScoresReply {
    uint32_t           messageId;
    uint32_t           transId;
    ENetError       result;
    uint32_t           scoreCount;
    uint32_t           byteCount;
    uint8_t            buffer[1];  // [byteCount], actually
    // no more fields
};

// ScoreAddPoints
extern const NetMsg kNetMsg_Auth2Cli_ScoreAddPointsReply;
struct Auth2Cli_ScoreAddPointsReply {
    uint32_t           messageId;
    uint32_t           transId;
    ENetError       result;
};

// ScoreTransferPoints
extern const NetMsg kNetMsg_Auth2Cli_ScoreTransferPointsReply;
struct Auth2Cli_ScoreTransferPointsReply {
    uint32_t           messageId;
    uint32_t           transId;
    ENetError       result;
};

// ScoreSetPoints
extern const NetMsg kNetMsg_Auth2Cli_ScoreSetPointsReply;
struct Auth2Cli_ScoreSetPointsReply {
    uint32_t           messageId;
    uint32_t           transId;
    ENetError       result;
};

// ScoreGetRanksReply
extern const NetMsg kNetMsg_Auth2Cli_ScoreGetRanksReply;
struct Auth2Cli_ScoreGetRanksReply {
    uint32_t           messageId;
    uint32_t           transId;
    ENetError       result;
    uint32_t           rankCount;
    uint32_t           byteCount;
    uint8_t            buffer[1];  // [byteCount], actually
    // no more fields
};

// ScoreGetHighScoresReply
extern const NetMsg kNetMsg_Auth2Cli_ScoreGetHighScoresReply;
struct Auth2Cli_ScoreGetHighScoresReply {
    uint32_t           messageId;
    uint32_t           transId;
    ENetError          result;
    uint32_t           scoreCount;
    uint32_t           byteCount;
    uint8_t            buffer[1];  // [byteCount], actually
    // no more fields
};

// ServerCaps
extern const NetMsg kNetMsg_Auth2Cli_ServerCaps;
struct Auth2Cli_ServerCaps {
    uint32_t           messageId;
    uint32_t           byteCount;
    uint8_t            buffer[1];  // [byteCount], actually
};

//============================================================================
// END PACKED DATA STRUCTURES
//============================================================================
#pragma pack(pop)

#endif // PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PNNPCLI2AUTH_H
