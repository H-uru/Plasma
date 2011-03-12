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
*   $/Plasma20/Sources/Plasma/PubUtilLib/plNetGameLib/Private/plNglAuth.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLNETGAMELIB_PRIVATE_PLNGLAUTH_H
#error "Header $/Plasma20/Sources/Plasma/PubUtilLib/plNetGameLib/Private/plNglAuth.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLNETGAMELIB_PRIVATE_PLNGLAUTH_H


/*****************************************************************************
*
*   Client auth functions
*
***/

//============================================================================
// Connect
//============================================================================
void NetCliAuthStartConnect (
    const wchar *   authAddrList[],
    unsigned        authAddrCount
);
bool NetCliAuthQueryConnected ();
void NetCliAuthAutoReconnectEnable (bool enable);	// is enabled by default

// Called after the auth/client connection is encrypted
typedef void (*FNetCliAuthConnectCallback)();
void NetCliAuthSetConnectCallback (
	FNetCliAuthConnectCallback callback
);


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
    unsigned	replyAtMs,
    unsigned	payloadbytes,
    const byte	payload[]
);
void NetCliAuthPingRequest (
    unsigned                        pingTimeMs,
    unsigned						payloadBytes,	// max 64k (pnNetCli enforced upon send)
    const void *					payload,
    FNetCliAuthPingRequestCallback  callback,
    void *                          param
);

//============================================================================
// AccountExists
//============================================================================
typedef void (*FNetCliAuthAccountExistsRequestCallback)(
    ENetError   result,
    void *      param,
	bool		accountExists
);
void NetCliAuthAccountExistsRequest (
    const wchar									accountName[],
    FNetCliAuthAccountExistsRequestCallback		callback,
    void *										param
);	

//============================================================================
// Login
//============================================================================
struct NetCliAuthPlayerInfo {
    unsigned    playerInt;
    wchar       playerName[kMaxPlayerNameLength];
    wchar		avatarShape[kMaxVaultNodeStringLength];
	unsigned	playerFlags;
	unsigned	explorer;
};

typedef void (*FNetCliAuthLoginRequestCallback)(
    ENetError                   result,
    void *                      param,
    const Uuid &                accountId,
	unsigned					accountFlags,
	unsigned					billingType,
    const NetCliAuthPlayerInfo  playerInfoArr[],
    unsigned                    playerCount
);
void NetCliAuthLoginRequest (
    const wchar                     accountName[],  // nil --> reuse previous acct name
    const ShaDigest *				accountNamePassHash,  // nil --> reuse previous acct pass
	const wchar                     authToken[],  // nil --> reuse previous auth token
	const wchar                     os[],  // nil --> reuse previous os
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
	ENetError						result,
	void *							param,
	const Uuid &					accountId
);
void NetCliAuthAccountCreateRequest (
	const wchar								accountName[],
	const wchar								accountPass[],
	unsigned								accountFlags,
	unsigned								billingType,
	FNetCliAuthAccountCreateRequestCallback	callback,
	void *									param
);

//============================================================================
// Create Account From Key
//============================================================================
typedef void (*FNetCliAuthAccountCreateFromKeyRequestCallback)(
	ENetError						result,
	void *							param,
	const Uuid &					accountId,
	const Uuid &					activationKey
);
void NetCliAuthAccountCreateFromKeyRequest (
	const wchar										accountName[],
	const wchar										accountPass[],
	Uuid											key,
	unsigned										billingType,
	FNetCliAuthAccountCreateFromKeyRequestCallback	callback,
	void *											param
);

//============================================================================
// Create Player
//============================================================================
typedef void (*FNetCliAuthPlayerCreateRequestCallback)(
	ENetError						result,
	void *							param,
    const NetCliAuthPlayerInfo &	playerInfo
);
void NetCliAuthPlayerCreateRequest (
	const wchar								playerName[],
	const wchar								avatarShape[],
	const wchar								friendInvite[],
	FNetCliAuthPlayerCreateRequestCallback	callback,
	void *									param
);

//============================================================================
// Delete Player
//============================================================================
typedef void (*FNetCliAuthPlayerDeleteRequestCallback)(
	ENetError						result,
	void *							param
);
void NetCliAuthPlayerDeleteRequest (
	unsigned								playerId,
	FNetCliAuthPlayerDeleteRequestCallback	callback,
	void *									param
);

//============================================================================
// Upgrade Visitor
//============================================================================
typedef void (*FNetCliAuthUpgradeVisitorRequestCallback)(
	ENetError						result,
	void *							param
);
void NetCliAuthUpgradeVisitorRequest (
	unsigned									playerId,
	FNetCliAuthUpgradeVisitorRequestCallback	callback,
	void *										param
);

//============================================================================
// SetCCRLevel
//============================================================================
void NetCliAuthSetCCRLevel (
	unsigned					ccrLevel
);

//============================================================================
// SetAgePublic
//============================================================================
void NetCliAuthSetAgePublic (
	unsigned					ageInfoId,
	bool						publicOrNot
);

//============================================================================
// GetPublicAgeList
//============================================================================
struct NetAgeInfo;
typedef void (*FNetCliAuthGetPublicAgeListCallback)(
	ENetError					result,
	void *						param,
	const ARRAY(NetAgeInfo) &	ages
);
void NetCliAuthGetPublicAgeList (
	const wchar							ageName[],
	FNetCliAuthGetPublicAgeListCallback	callback,
	void *								param
);

//============================================================================
// Change Password
//============================================================================
typedef void (*FNetCliAuthAccountChangePasswordRequestCallback)(
	ENetError						result,
	void *							param
);
void NetCliAuthAccountChangePasswordRequest (
	const wchar										accountName[],
	const wchar										accountPass[],
	FNetCliAuthAccountChangePasswordRequestCallback	callback,
	void *											param
);

//============================================================================
// Set Account Roles
//============================================================================
typedef void (*FNetCliAuthAccountSetRolesRequestCallback)(
	ENetError						result,
	void *							param
);
void NetCliAuthAccountSetRolesRequest (
	const wchar										accountName[],
	unsigned										accountFlags,
	FNetCliAuthAccountSetRolesRequestCallback		callback,
	void *											param
);

//============================================================================
// Set Billing Type
//============================================================================
typedef void (*FNetCliAuthAccountSetBillingTypeRequestCallback)(
	ENetError						result,
	void *							param
);
void NetCliAuthAccountSetBillingTypeRequest (
	const wchar										accountName[],
	unsigned										billingType,
	FNetCliAuthAccountSetBillingTypeRequestCallback	callback,
	void *											param
);

//============================================================================
// Account Activate
//============================================================================
typedef void (*FNetCliAuthAccountActivateRequestCallback)(
	ENetError						result,
	void *							param
);
void NetCliAuthAccountActivateRequest (
	const Uuid &								activationKey,
	FNetCliAuthAccountActivateRequestCallback	callback,
	void *										param
);

//============================================================================
// Age
//============================================================================
typedef void (*FNetCliAuthAgeRequestCallback)(
    ENetError       result,
    void *          param,
    unsigned		ageMcpId,
    unsigned		ageVaultId,
    const Uuid &    ageInstId,
    NetAddressNode  gameAddr
);
void NetCliAuthAgeRequest (
    const wchar							ageName[],		// L"Teledahn"
    const Uuid &                        ageInstId,
    FNetCliAuthAgeRequestCallback       callback,
    void *                              param
);

//============================================================================
// Secure File Encryption Key
//============================================================================
void NetCliAuthGetEncryptionKey (
	UInt32		key[],
	unsigned	size
);

//============================================================================
// File List
//============================================================================
struct NetCliAuthFileInfo {
    wchar       filename[MAX_PATH];
    unsigned    filesize;
};
typedef void (*FNetCliAuthFileListRequestCallback)(
    ENetError                   result,
    void *                      param,
    const NetCliAuthFileInfo    infoArr[],
    unsigned                    infoCount
);
void NetCliAuthFileListRequest (
    const wchar                         dir[],
    const wchar                         ext[],
    FNetCliAuthFileListRequestCallback  callback,
    void *                              param
);

//============================================================================
// File Download
//============================================================================
typedef void (*FNetCliAuthFileRequestCallback)(
    ENetError       result,
    void *          param,
    const wchar     filename[],
    hsStream *      writer
);
void NetCliAuthFileRequest (
    const wchar                     filename[],
    hsStream *                      writer,
    FNetCliAuthFileRequestCallback  callback,
    void *                          param
);

//============================================================================
// Vault Operations
//============================================================================
struct NetVaultNode;
struct NetVaultNodeRef;

// VaultNodeChanged
typedef void (*FNetCliAuthVaultNodeChanged)(
	unsigned		nodeId,
	const Uuid &	revisionId
);
void NetCliAuthVaultSetRecvNodeChangedHandler (
	FNetCliAuthVaultNodeChanged	handler
);
// VaultNodeAdded
typedef void (*FNetCliAuthVaultNodeAdded)(
	unsigned		parentId,
	unsigned		childId,
	unsigned		ownerId
);
void NetCliAuthVaultSetRecvNodeAddedHandler (
	FNetCliAuthVaultNodeAdded	handler
);
// VaultNodeRemoved
typedef void (*FNetCliAuthVaultNodeRemoved)(
	unsigned		parentId,
	unsigned		childId
);
void NetCliAuthVaultSetRecvNodeRemovedHandler (
	FNetCliAuthVaultNodeRemoved	handler
);
// VaultNodeDeleted
typedef void (*FNetCliAuthVaultNodeDeleted)(
	unsigned		nodeId
);
void NetCliAuthVaultSetRecvNodeDeletedHandler (
	FNetCliAuthVaultNodeDeleted	handler
);
// VaultNodeAdd
typedef void (*FNetCliAuthVaultNodeAddCallback)(
	ENetError			result,
	void *				param
);
void NetCliAuthVaultNodeAdd (
	unsigned						parentId,
	unsigned						childId,
	unsigned						ownerId,
	FNetCliAuthVaultNodeAddCallback	callback,
	void *							param
);
// VaultNodeRemove
typedef void (*FNetCliAuthVaultNodeRemoveCallback)(
	ENetError			result,
	void *				param
);
void NetCliAuthVaultNodeRemove (
	unsigned							parentId,
	unsigned							childId,
	FNetCliAuthVaultNodeRemoveCallback	callback,
	void *								param
);
// VaultNodeCreate
typedef void (*FNetCliAuthVaultNodeCreated)(
	ENetError			result,
	void *				param,
	unsigned			nodeId
);
void NetCliAuthVaultNodeCreate (
	NetVaultNode *				templateNode,
	FNetCliAuthVaultNodeCreated	callback,
	void *						param
);
// VaultNodeFetch
typedef void (*FNetCliAuthVaultNodeFetched)(
	ENetError			result,
	void *				param,
	NetVaultNode *		node
);
void NetCliAuthVaultNodeFetch (
	unsigned					nodeId,
	FNetCliAuthVaultNodeFetched	callback,
	void *						param
);
// VaultNodeFind
typedef void (*FNetCliAuthVaultNodeFind)(
	ENetError			result,
	void *				param,
	unsigned			nodeIdCount,
	const unsigned		nodeIds[]
);
void NetCliAuthVaultNodeFind (
	NetVaultNode *				templateNode,
	FNetCliAuthVaultNodeFind	callback,
	void *						param
);
// VaultNodeSave
typedef void (*FNetCliAuthVaultNodeSaveCallback)(
	ENetError			result,
	void *				param
);
unsigned NetCliAuthVaultNodeSave (	// returns number of bytes written
	NetVaultNode *						node,
	FNetCliAuthVaultNodeSaveCallback	callback,
	void *								param
);
void NetCliAuthVaultNodeDelete (
	unsigned					nodeId
);
// FetchRefs (a vault structure only; no data)
typedef void (*FNetCliAuthVaultNodeRefsFetched)(
	ENetError			result,
	void *				param,
	NetVaultNodeRef *	refs,
	unsigned			refCount
);
void NetCliAuthVaultFetchNodeRefs (
	unsigned						nodeId,
	FNetCliAuthVaultNodeRefsFetched	callback,
	void *							param
);
void NetCliAuthVaultSetSeen (
	unsigned	parentId,
	unsigned	childId,
	bool		seen
);

void NetCliAuthVaultSendNode (
	unsigned	srcNodeId,
	unsigned	dstPlayerId
);

// Initialize an age vault. Will find existing match in db, or create a new age vault structure.
typedef void (*FNetCliAuthAgeInitCallback) (
	ENetError			result,
	void *				param,
	unsigned			ageVaultId,
	unsigned			ageInfoVaultId
);
void NetCliAuthVaultInitAge (
	const Uuid &				ageInstId,			// optional. is used in match
	const Uuid &				parentAgeInstId,	// optional. is used in match
	const wchar					ageFilename[],		// optional. is used in match
	const wchar					ageInstName[],		// optional. not used in match
	const wchar					ageUserName[],		// optional. not used in match
	const wchar					ageDesc[],			// optional. not used in match
	unsigned					ageSequenceNumber,	// optional. not used in match
	unsigned					ageLanguage,		// optional. not used in match
	FNetCliAuthAgeInitCallback	callback,			// optional
	void *						param				// optional
);

void NetCliAuthLogPythonTraceback (const wchar traceback[]);

void NetCliAuthLogStackDump (const wchar stackdump[]);
void NetCliAuthLogClientDebuggerConnect ();

//============================================================================
// SetPlayerBanStatusRequest
//============================================================================
typedef void (*FNetCliAuthSetPlayerBanStatusRequestCallback)(
	ENetError						result,
	void *							param
);
void NetCliAuthSetPlayerBanStatusRequest (
	unsigned										playerId,
	unsigned										banned,
	FNetCliAuthSetPlayerBanStatusRequestCallback	callback,
	void *											param
);

//============================================================================
// KickPlayerRequest
//============================================================================
void NetCliAuthKickPlayer (
	unsigned			playerId
);

//============================================================================
// ChangePlayerNameRequest
//============================================================================
typedef void (*FNetCliAuthChangePlayerNameRequestCallback)(
	ENetError						result,
	void *							param
);
void NetCliAuthChangePlayerNameRequest (
	unsigned									playerId,
	const wchar									newName[],
	FNetCliAuthChangePlayerNameRequestCallback	callback,
	void *										param
);

//============================================================================
// CCRPetition
//============================================================================
void NetCliAuthSendCCRPetition (
	const wchar *		petitionText
);

//============================================================================
// SendFriendInvite
//============================================================================
typedef void (*FNetCliAuthSendFriendInviteCallback)(
	ENetError							result,
	void *								param
);

void NetCliAuthSendFriendInvite (
	const wchar							emailAddress[],
	const wchar							toName[],
	const Uuid&							inviteUuid,
	FNetCliAuthSendFriendInviteCallback	callback,
	void *								param
);

//============================================================================
// Propagate app-specific data
//============================================================================
typedef void (*FNetCliAuthRecvBufferHandler)(
    unsigned                        type,
    unsigned                        bytes,
    const byte                      buffer[]
);
void NetCliAuthSetRecvBufferHandler (
    FNetCliAuthRecvBufferHandler    handler
);
void NetCliAuthPropagateBuffer (
    unsigned                        type,
    unsigned                        bytes,
    const byte                      buffer[]
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

typedef void (*FNetCliAuthScoreUpdateCallback)(
	ENetError			result,
	void *				param
);

//============================================================================
typedef void (*FNetCliAuthCreateScoreCallback)(
	ENetError	result,
	void *		param,
	unsigned	scoreId,
	UInt32		createdTime,
	unsigned	ownerId,
	const char*	gameName,
	unsigned	gameType,
	int			value
);
void NetCliAuthScoreCreate(
	unsigned						ownerId,
	const char*						gameName,
	unsigned						gameType,
	int								value,
	FNetCliAuthCreateScoreCallback	callback,
	void *							param
);

//============================================================================
void NetCliAuthScoreDelete(
	unsigned						scoreId,
	FNetCliAuthScoreUpdateCallback	callback,
	void *							param
);

//============================================================================
typedef void (*FNetCliAuthGetScoresCallback)(
	ENetError			result,
	void *				param,
	const NetGameScore	scores[],
	unsigned			scoreCount
);

void NetCliAuthScoreGetScores(
	unsigned						ownerId,
	const char*						gameName,
	FNetCliAuthGetScoresCallback	callback,
	void *							param
);

//============================================================================
void NetCliAuthScoreAddPoints(
	unsigned						scoreId,
	int								numPoints,
	FNetCliAuthScoreUpdateCallback	callback,
	void *							param
);

//============================================================================
void NetCliAuthScoreTransferPoints(
	unsigned						srcScoreId,
	unsigned						destScoreId,
	int								numPoints,
	FNetCliAuthScoreUpdateCallback	callback,
	void *							param
);

//============================================================================
void NetCliAuthScoreSetPoints(
	unsigned						scoreId,
	int								numPoints,
	FNetCliAuthScoreUpdateCallback	callback,
	void *							param
);

//============================================================================
struct NetGameRank;
typedef void (*FNetCliAuthGetRanksCallback)(
	ENetError			result,
	void *				param,
	const NetGameRank	ranks[],
	unsigned			rankCount
);

void NetCliAuthScoreGetRankList(
	unsigned					ownerId,
	unsigned					scoreGroup,
	unsigned					parentFolderId,
	const char *				gameName,
	unsigned					timePeriod,
	unsigned					numResults,
	unsigned					pageNumber,
	bool						sortDesc,
	FNetCliAuthGetRanksCallback	callback,
	void *						param
);
