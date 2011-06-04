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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/Protocols/pnNpCli2Auth.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PRIVATE_PROTOCOLS_PNNPCLI2AUTH_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/Protocols/pnNpCli2Auth.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PRIVATE_PROTOCOLS_PNNPCLI2AUTH_H


// kNetProtocolCli2Auth messages (must be <= (word)-1)
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

	kNumCli2AuthMessages
};
COMPILER_ASSERT_HEADER(Cli2Auth, kNumCli2AuthMessages <= (word)-1);

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

	kNumAuth2CliMessages
};
COMPILER_ASSERT_HEADER(Cli2Auth, kNumAuth2CliMessages <= (word)-1);


//============================================================================
// BEGIN PACKED DATA STRUCTURES
//============================================================================
#include <PshPack1.h>


/*****************************************************************************
*
*   Cli2Auth connect packet
*
***/

struct Cli2Auth_ConnData {
    dword		dataBytes;
    Uuid		token;
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
    dword       messageId;
    dword       pingTimeMs;
    dword		transId;
    dword		payloadBytes;
    byte		payload[1];	// [payloadBytes]
};

// ClientRegisterRequest
extern const NetMsg kNetMsg_Cli2Auth_ClientRegisterRequest;
struct Cli2Auth_ClientRegisterRequest {
    dword       messageId;
    dword       buildId;
};

// AccountExists
extern const NetMsg kNetMsg_Cli2Auth_AccountExistsRequest;
struct Cli2Auth_AccountExistsRequest {
	dword messageId;
	dword transId;
	wchar accountName[kMaxAccountNameLength];
};

// LoginRequest
extern const NetMsg kNetMsg_Cli2Auth_AcctLoginRequest;
struct Cli2Auth_AcctLoginRequest {
    dword       messageId;
    dword       transId;
	dword		clientChallenge;
    wchar       acctName[kMaxAccountNameLength];
    ShaDigest   challengeHash;
	wchar		authToken[kMaxPublisherAuthKeyLength];
	wchar		os[kMaxGTOSIdLength];
};

// AgeRequest
extern const NetMsg kNetMsg_Cli2Auth_AgeRequest;
struct Cli2Auth_AgeRequest {
    dword       messageId;
    dword       transId;
    wchar		ageName[kMaxAgeNameLength];
    Uuid        ageUuid;
};

// AcctCreateRequest
extern const NetMsg kNetMsg_Cli2Auth_AcctCreateRequest;
struct Cli2Auth_AcctCreateRequest {
	dword		messageId;
	dword		transId;
	wchar		accountName[kMaxAccountNameLength];
	ShaDigest   namePassHash;
	dword		accountFlags;
	dword		billingType;
};

// AcctCreateFromKeyRequest
extern const NetMsg kNetMsg_Cli2Auth_AcctCreateFromKeyRequest;
struct Cli2Auth_AcctCreateFromKeyRequest {
	dword		messageId;
	dword		transId;
	wchar		accountName[kMaxAccountNameLength];
	ShaDigest   namePassHash;
	Uuid		key;
	dword		billingType;
};

// CreatePlayerRequest
extern const NetMsg kNetMsg_Cli2Auth_PlayerCreateRequest;
struct Cli2Auth_PlayerCreateRequest {
	dword		messageId;
	dword		transId;
	wchar		playerName[kMaxPlayerNameLength];
	wchar		avatarShape[MAX_PATH];
	wchar		friendInvite[MAX_PATH];
};

extern const NetMsg kNetMsg_Cli2Auth_PlayerDeleteRequest;
struct Cli2Auth_PlayerDeleteRequest {
	dword		messageId;
	dword		transId;
	dword		playerId;
};

extern const NetMsg kNetMsg_Cli2Auth_UpgradeVisitorRequest;
struct Cli2Auth_UpgradeVisitorRequest {
	dword		messageId;
	dword		transId;
	dword		playerId;
};

// SetPlayerRequest
extern const NetMsg kNetMsg_Cli2Auth_AcctSetPlayerRequest;
struct Cli2Auth_AcctSetPlayerRequest {
    dword       messageId;
    dword       transId;
    dword       playerInt;
};

// ChangePasswordRequest
extern const NetMsg kNetMsg_Cli2Auth_AcctChangePasswordRequest;
struct Cli2Auth_AcctChangePasswordRequest {
	dword		messageId;
	dword		transId;
	wchar		accountName[kMaxAccountNameLength];
	ShaDigest	namePassHash;
};

// AcctSetRolesRequest
extern const NetMsg kNetMsg_Cli2Auth_AcctSetRolesRequest;
struct Cli2Auth_AcctSetRolesRequest {
	dword		messageId;
	dword		transId;
	wchar		accountName[kMaxAccountNameLength];
	dword		accountFlags;
};

// AcctSetBillingTypeRequest
extern const NetMsg kNetMsg_Cli2Auth_AcctSetBillingTypeRequest;
struct Cli2Auth_AcctSetBillingTypeRequest {
	dword		messageId;
	dword		transId;
	wchar		accountName[kMaxAccountNameLength];
	dword		billingType;
};

// AcctActivateRequest
extern const NetMsg kNetMsg_Cli2Auth_AcctActivateRequest;
struct Cli2Auth_AcctActivateRequest {
	dword		messageId;
	dword		transId;
	Uuid		activationKey;
};

// FileListRequest
extern const NetMsg kNetMsg_Cli2Auth_FileListRequest;
struct Cli2Auth_FileListRequest {
	dword		messageId;
	dword		transId;
	wchar		directory[MAX_PATH];
	wchar		ext[MAX_EXT];
};

// FileDownloadRequest
extern const NetMsg kNetMsg_Cli2Auth_FileDownloadRequest;
struct Cli2Auth_FileDownloadRequest {
	dword		messageId;
	dword		transId;
	wchar		filename[MAX_PATH];
};

// FileDownloadChunkAck
extern const NetMsg kNetMsg_Cli2Auth_FileDownloadChunkAck;
struct Cli2Auth_FileDownloadChunkAck {
	dword		messageId;
	dword		transId;
};

// VaultFetchNodeRefs
extern const NetMsg kNetMsg_Cli2Auth_VaultFetchNodeRefs;
struct Cli2Auth_VaultFetchNodeRefs {
	dword		messageId;
	dword		transId;
	dword		nodeId;
};

// VaultNodeAdd
extern const NetMsg kNetMsg_Cli2Auth_VaultNodeAdd;
struct Cli2Auth_VaultNodeAdd {
	dword		messageId;
	dword		transId;
	dword		parentId;
	dword		childId;
	dword		ownerId;
};

// VaultNodeRemove
extern const NetMsg kNetMsg_Cli2Auth_VaultNodeRemove;
struct Cli2Auth_VaultNodeRemove {
	dword		messageId;
	dword		transId;
	dword		parentId;
	dword		childId;
};

// VaultNodeSave
extern const NetMsg kNetMsg_Cli2Auth_VaultNodeSave;
struct Cli2Auth_VaultNodeSave {
	dword		messageId;
	dword		transId;
	dword		nodeId;
	Uuid		revisionId;
	dword		nodeBytes;
	byte		nodeBuffer[1];
};

// VaultNodeCreate
extern const NetMsg kNetMsg_Cli2Auth_VaultNodeCreate;
struct Cli2Auth_VaultNodeCreate {
	dword		messageId;
	dword		transId;
	dword		nodeBytes;
	byte		nodeBuffer[1];
};

// VaultNodeFetch
extern const NetMsg kNetMsg_Cli2Auth_VaultNodeFetch;
struct Cli2Auth_VaultNodeFetch {
	dword		messageId;
	dword		transId;
	dword		nodeId;
};

// VaultInitAgeRequest
extern const NetMsg kNetMsg_Cli2Auth_VaultInitAgeRequest;
struct Cli2Auth_VaultInitAgeRequest {
	dword		messageId;
	dword		transId;
	Uuid		ageInstId;
	Uuid		parentAgeInstId;
	wchar		ageFilename[MAX_PATH];
	wchar		ageInstName[MAX_PATH];
	wchar		ageUserName[MAX_PATH];
	wchar		ageDesc[1024];
	dword		ageSequenceNumber;
	dword		ageLanguage;
};

// VaultNodeFind
extern const NetMsg kNetMsg_Cli2Auth_VaultNodeFind;
struct Cli2Auth_VaultNodeFind {
	dword		messageId;
	dword		transId;
	dword		nodeBytes;
	byte		nodeBuffer[1];
};

// VaultSetSeen
extern const NetMsg kNetMsg_Cli2Auth_VaultSetSeen;
struct Cli2Auth_VaultSetSeen {
	dword		messageId;
	dword		parentId;
	dword		childId;
	byte		seen;
};

// VaultSendNode
extern const NetMsg kNetMsg_Cli2Auth_VaultSendNode;
struct Cli2Auth_VaultSendNode {
	dword		messageId;
	dword		srcNodeId;
	dword		dstPlayerId;
};

// GetPublicAgeList
extern const NetMsg kNetMsg_Cli2Auth_GetPublicAgeList;
struct Cli2Auth_GetPublicAgeList {
	dword		messageId;
	dword		transId;
	wchar		ageFilename[kMaxAgeNameLength];
};

extern const NetMsg kNetMsg_Cli2Auth_SetAgePublic;
struct Cli2Auth_SetAgePublic {
	dword		messageId;
	dword		ageInfoId;
	byte		publicOrNot;
};

// PropagateBuffer
extern const NetMsg kNetMsg_Cli2Auth_PropagateBuffer;
struct Cli2Auth_PropagateBuffer {
    dword       messageId;
    dword       type;
    dword       bytes;
    byte        buffer[1];  // [bytes], actually
    // no more fields
};

extern const NetMsg kNetMsg_Cli2Auth_ClientSetCCRLevel;
struct Cli2Auth_ClientSetCCRLevel {
	dword		messageId;
	dword		ccrLevel;
};

extern const NetMsg kNetMsg_Cli2Auth_LogPythonTraceback;
struct Cli2Auth_LogPythonTraceback {
	dword messageId;
	wchar traceback[1024];
};

extern const NetMsg kNetMsg_Cli2Auth_LogStackDump;
struct Cli2Auth_LogStackDump {
	dword messageId;
	wchar stackdump[1024];
};

extern const NetMsg kNetMsg_Cli2Auth_LogClientDebuggerConnect;
struct Cli2Auth_LogClientDebuggerConnect {
	dword	messageId;
};

extern const NetMsg kNetMsg_Cli2Auth_SetPlayerBanStatusRequest;
struct Cli2Auth_SetPlayerBanStatusRequest {
	dword messageId;
	dword transId;
	dword playerId;
	dword banned;
};

extern const NetMsg kNetMsg_Cli2Auth_KickPlayer;
struct Cli2Auth_KickPlayer {
	dword messageId;
	dword playerId;
};

extern const NetMsg kNetMsg_Cli2Auth_ChangePlayerNameRequest;
struct Cli2Auth_ChangePlayerNameRequest {
	dword messageId;
	dword transId;
	dword playerId;
	wchar newName[kMaxPlayerNameLength];
};

extern const NetMsg kNetMsg_Cli2Auth_SendFriendInviteRequest;
struct Cli2Auth_SendFriendInviteRequest {
	dword	messageId;
	dword	transId;
	Uuid	inviteUuid;
	wchar	emailAddress[kMaxEmailAddressLength];
	wchar	toName[kMaxPlayerNameLength];
};

extern const NetMsg kNetMsg_Cli2Auth_ScoreCreate;
struct Cli2Auth_ScoreCreate {
	dword messageId;
	dword transId;
	dword ownerId;
	wchar gameName[kMaxGameScoreNameLength];
	dword gameType;
	dword scoreValue;
};

extern const NetMsg kNetMsg_Cli2Auth_ScoreDelete;
struct Cli2Auth_ScoreDelete {
	dword messageId;
	dword transId;
	dword scoreId;
};

extern const NetMsg kNetMsg_Cli2Auth_ScoreGetScores;
struct Cli2Auth_ScoreGetScores {
	dword messageId;
	dword transId;
	dword ownerId;
	wchar gameName[kMaxGameScoreNameLength];
};

extern const NetMsg kNetMsg_Cli2Auth_ScoreAddPoints;
struct Cli2Auth_ScoreAddPoints {
	dword messageId;
	dword transId;
	dword scoreId;
	dword numPoints;
};

extern const NetMsg kNetMsg_Cli2Auth_ScoreTransferPoints;
struct Cli2Auth_ScoreTransferPoints {
	dword messageId;
	dword transId;
	dword srcScoreId;
	dword destScoreId;
	dword numPoints;
};

extern const NetMsg kNetMsg_Cli2Auth_ScoreSetPoints;
struct Cli2Auth_ScoreSetPoints {
	dword messageId;
	dword transId;
	dword scoreId;
	dword numPoints;
};

extern const NetMsg kNetMsg_Cli2Auth_ScoreGetRanks;
struct Cli2Auth_ScoreGetRanks {
	dword messageId;
	dword transId;
	dword ownerId;
	dword scoreGroup;
	dword parentFolderId;
	wchar gameName[kMaxGameScoreNameLength];
	dword timePeriod;
	dword numResults;
	dword pageNumber;
	dword sortDesc;
};


/*****************************************************************************
*
*   Auth2Cli message structures
*
***/

// PingReply
extern const NetMsg kNetMsg_Auth2Cli_PingReply;
struct Auth2Cli_PingReply {
    dword       messageId;
    dword       pingTimeMs;
    dword		transId;
    dword		payloadBytes;
    byte		payload[1];	// [payloadBytes]
};

// ClientRegisterReply
extern const NetMsg kNetMsg_Auth2Cli_ClientRegisterReply;
struct Auth2Cli_ClientRegisterReply {
    dword       messageId;
    dword       serverChallenge;
};

// AccountExists
extern const NetMsg kNetMsg_Auth2Cli_AccountExistsReply;
struct Auth2Cli_AccountExistsReply {
	dword		messageId;
	dword		transId;
	ENetError	result;
	byte		exists;
};

// ServerAddr
extern const NetMsg kNetMsg_Auth2Cli_ServerAddr;
struct Auth2Cli_ServerAddr {
	dword			messageId;
	NetAddressNode	srvAddr;
	Uuid			token;
};

extern const NetMsg kNetMsg_Auth2Cli_NotifyNewBuild;
struct Auth2Cli_NotifyNewBuild {
	dword			foo;		// msgs must have at least one field
};

// AcctPlayerInfo
extern const NetMsg kNetMsg_Auth2Cli_AcctPlayerInfo;
struct Auth2Cli_AcctPlayerInfo {
    dword       messageId;
    dword       transId;
    dword       playerInt;
    wchar       playerName[kMaxPlayerNameLength];
    wchar		avatarShape[kMaxVaultNodeStringLength];
	dword		explorer;
};

// LoginReply
extern const NetMsg kNetMsg_Auth2Cli_AcctLoginReply;
struct Auth2Cli_AcctLoginReply {
    dword       messageId;
    dword       transId;
    ENetError   result;
    Uuid        accountId;
	dword		accountFlags;
	dword		billingType;
	dword		encryptionKey[4];
};

// AgeReply
extern const NetMsg kNetMsg_Auth2Cli_AgeReply;
struct Auth2Cli_AgeReply {
    dword           messageId;
    dword           transId;
    ENetError       result;
    dword			ageMcpId;
    Uuid			ageInstId;
    dword			ageVaultId;
    NetAddressNode  gameSrvNode;
};

// AcctCreateReply
extern const NetMsg kNetMsg_Auth2Cli_AcctCreateReply;
struct Auth2Cli_AcctCreateReply {
	dword			messageId;
	dword			transId;
	ENetError		result;
	Uuid			accountId;
};

// AcctCreateFromKeyReply
extern const NetMsg kNetMsg_Auth2Cli_AcctCreateFromKeyReply;
struct Auth2Cli_AcctCreateFromKeyReply {
	dword			messageId;
	dword			transId;
	ENetError		result;
	Uuid			accountId;
	Uuid			activationKey;
};

// CreatePlayerReply
extern const NetMsg kNetMsg_Auth2Cli_PlayerCreateReply;
struct Auth2Cli_PlayerCreateReply {
    dword           messageId;
    dword           transId;
    ENetError       result;
    dword			playerInt;
	dword			explorer;
    wchar			playerName[kMaxPlayerNameLength];
    wchar			avatarShape[kMaxVaultNodeStringLength];
};

// DeletePlayerReply
extern const NetMsg kNetMsg_Auth2Cli_PlayerDeleteReply;
struct Auth2Cli_PlayerDeleteReply {
    dword           messageId;
    dword           transId;
    ENetError       result;
};

// DeletePlayerReply
extern const NetMsg kNetMsg_Auth2Cli_UpgradeVisitorReply;
struct Auth2Cli_UpgradeVisitorReply {
    dword           messageId;
    dword           transId;
    ENetError       result;
};

// SetPlayerReply
extern const NetMsg kNetMsg_Auth2Cli_AcctSetPlayerReply;
struct Auth2Cli_AcctSetPlayerReply {
    dword           messageId;
    dword           transId;
    ENetError       result;
};

// AcctChangePasswordReply
extern const NetMsg kNetMsg_Auth2Cli_AcctChangePasswordReply;
struct Auth2Cli_AcctChangePasswordReply {
	dword			messageId;
	dword			transId;
	ENetError		result;
};

// AcctSetRolesReply
extern const NetMsg kNetMsg_Auth2Cli_AcctSetRolesReply;
struct Auth2Cli_AcctSetRolesReply {
	dword			messageId;
	dword			transId;
	ENetError		result;
};

// AcctSetBillingTypeReply
extern const NetMsg kNetMsg_Auth2Cli_AcctSetBillingTypeReply;
struct Auth2Cli_AcctSetBillingTypeReply {
	dword			messageId;
	dword			transId;
	ENetError		result;
};

// AcctActivateReply
extern const NetMsg kNetMsg_Auth2Cli_AcctActivateReply;
struct Auth2Cli_AcctActivateReply {
	dword			messageId;
	dword			transId;
	ENetError		result;
};

// FileListReply
extern const NetMsg kNetMsg_Auth2Cli_FileListReply;
struct Auth2Cli_FileListReply {
	dword			messageId;
	dword			transId;
	ENetError		result;
	dword			wcharCount;
	wchar			fileData[1];		// [wcharCount], actually
	// no more fields
};

// FileDownloadChunk
extern const NetMsg kNetMsg_Auth2Cli_FileDownloadChunk;
struct Auth2Cli_FileDownloadChunk {
	dword			messageId;
	dword			transId;
	ENetError		result;
	dword			fileSize;
	dword			chunkOffset;
	dword			chunkSize;
	byte			chunkData[1];		// [chunkSize], actually
	// no more fields
};

// KickedOff
extern const NetMsg kNetMsg_Auth2Cli_KickedOff;
struct Auth2Cli_KickedOff {
	dword			messageId;
	ENetError		reason;
};

extern const NetMsg kNetMsg_Auth2Cli_VaultNodeRefsFetched;
struct Auth2Cli_VaultNodeRefsFetched {
	dword					messageId;
	dword					transId;
	ENetError				result;
	dword					refCount;
	NetVaultNodeRef			refs[1];
};

extern const NetMsg kNetMsg_Auth2Cli_VaultNodeCreated;
struct Auth2Cli_VaultNodeCreated {
	dword					messageId;
	dword					transId;
	ENetError				result;
	dword					nodeId;
};

extern const NetMsg kNetMsg_Auth2Cli_VaultNodeFetched;
struct Auth2Cli_VaultNodeFetched {
	dword					messageId;
	dword					transId;
	ENetError				result;
	dword					nodeBytes;
	byte					nodeBuffer[1];
};

extern const NetMsg kNetMsg_Auth2Cli_VaultNodeChanged;
struct Auth2Cli_VaultNodeChanged {
	dword					messageId;
	dword					nodeId;
	Uuid					revisionId;
};

extern const NetMsg kNetMsg_Auth2Cli_VaultNodeAdded;
struct Auth2Cli_VaultNodeAdded {
	dword					messageId;
	dword					parentId;
	dword					childId;
	dword					ownerId;
};

extern const NetMsg kNetMsg_Auth2Cli_VaultNodeRemoved;
struct Auth2Cli_VaultNodeRemoved {
	dword					messageId;
	dword					parentId;
	dword					childId;
};

extern const NetMsg kNetMsg_Auth2Cli_VaultNodeDeleted;
struct Auth2Cli_VaultNodeDeleted {
	dword					messageId;
	dword					nodeId;
};

extern const NetMsg kNetMsg_Auth2Cli_VaultSaveNodeReply;
struct Auth2Cli_VaultSaveNodeReply {
	dword			messageId;
	dword			transId;
	ENetError		result;
};

extern const NetMsg kNetMsg_Auth2Cli_VaultAddNodeReply;
struct Auth2Cli_VaultAddNodeReply {
	dword			messageId;
	dword			transId;
	ENetError		result;
};

extern const NetMsg kNetMsg_Auth2Cli_VaultRemoveNodeReply;
struct Auth2Cli_VaultRemoveNodeReply {
	dword			messageId;
	dword			transId;
	ENetError		result;
};

extern const NetMsg kNetMsg_Auth2Cli_VaultInitAgeReply;
struct Auth2Cli_VaultInitAgeReply {
	dword					messageId;
	dword					transId;
	ENetError				result;
	dword					ageVaultId;
	dword					ageInfoVaultId;
};

extern const NetMsg kNetMsg_Auth2Cli_VaultNodeFindReply;
struct Auth2Cli_VaultNodeFindReply {
	dword					messageId;
	dword					transId;
	ENetError				result;
	dword					nodeIdCount;
	dword					nodeIds[1];
};

// PublicAgeList
extern const NetMsg kNetMsg_Auth2Cli_PublicAgeList;
struct Auth2Cli_PublicAgeList {
	dword				messageId;
	dword				transId;
	ENetError			result;
	dword				ageCount;
	NetAgeInfo			ages[1];	// [ageCount], actually
};

// PropagateBuffer
extern const NetMsg kNetMsg_Auth2Cli_PropagateBuffer;
struct Auth2Cli_PropagateBuffer {
    dword       messageId;
    dword       type;
    dword       bytes;
    byte        buffer[1];  // [bytes], actually
    // no more fields
};

// SetPlayerBanStatusReply
extern const NetMsg kNetMsg_Auth2Cli_SetPlayerBanStatusReply;
struct Auth2Cli_SetPlayerBanStatusReply {
	dword			messageId;
	dword			transId;
	ENetError		result;
};

// ChangePlayerNameReply
extern const NetMsg kNetMsg_Auth2Cli_ChangePlayerNameReply;
struct Auth2Cli_ChangePlayerNameReply {
	dword			messageId;
	dword			transId;
	ENetError		result;
};

// SendFriendInviteReply
extern const NetMsg kNetMsg_Auth2Cli_SendFriendInviteReply;
struct Auth2Cli_SendFriendInviteReply {
	dword			messageId;
	dword			transId;
	ENetError		result;
};

// ScoreCreateReply
extern const NetMsg kNetMsg_Auth2Cli_ScoreCreateReply;
struct Auth2Cli_ScoreCreateReply {
	dword			messageId;
	dword			transId;
	ENetError		result;
	dword			scoreId;
	dword			createdTime;
};

// ScoreDeleteReply
extern const NetMsg kNetMsg_Auth2Cli_ScoreDeleteReply;
struct Auth2Cli_ScoreDeleteReply {
	dword			messageId;
	dword			transId;
	ENetError		result;
};

// ScoreGetScoresReply
extern const NetMsg kNetMsg_Auth2Cli_ScoreGetScoresReply;
struct Auth2Cli_ScoreGetScoresReply {
	dword			messageId;
	dword			transId;
	ENetError		result;
	dword			scoreCount;
	dword			byteCount;
	byte			buffer[1];  // [byteCount], actually
    // no more fields
};

// ScoreAddPoints
extern const NetMsg kNetMsg_Auth2Cli_ScoreAddPointsReply;
struct Auth2Cli_ScoreAddPointsReply {
	dword			messageId;
	dword			transId;
	ENetError		result;
};

// ScoreTransferPoints
extern const NetMsg kNetMsg_Auth2Cli_ScoreTransferPointsReply;
struct Auth2Cli_ScoreTransferPointsReply {
	dword			messageId;
	dword			transId;
	ENetError		result;
};

// ScoreSetPoints
extern const NetMsg kNetMsg_Auth2Cli_ScoreSetPointsReply;
struct Auth2Cli_ScoreSetPointsReply {
	dword			messageId;
	dword			transId;
	ENetError		result;
};

// ScoreGetRanksReply
extern const NetMsg kNetMsg_Auth2Cli_ScoreGetRanksReply;
struct Auth2Cli_ScoreGetRanksReply {
	dword			messageId;
	dword			transId;
	ENetError		result;
	dword			rankCount;
	dword			byteCount;
	byte			buffer[1];  // [byteCount], actually
    // no more fields
};

//============================================================================
// END PACKED DATA STRUCTURES
//============================================================================
#include <PopPack.h>
