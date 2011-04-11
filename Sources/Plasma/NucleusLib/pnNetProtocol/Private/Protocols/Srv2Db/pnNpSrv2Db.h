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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/Protocols/Srv2Db/pnNpSrv2Db.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PRIVATE_PROTOCOLS_SRV2DB_PNNPSRV2DB_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/Protocols/Srv2Db/pnNpSrv2Db.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PRIVATE_PROTOCOLS_SRV2DB_PNNPSRV2DB_H



// kNetProtocolSrv2Db messages
// Because SrvDb must remain compatible with older auth builds, these message ids
// must not change unless all front-end servers are synchronously replaced.
enum {
    // Account
    kSrv2Db_AccountCreateRequest			= 0,
    kSrv2Db_AccountLoginRequest				= 1,
    kSrv2Db_AccountLogout					= 2,
	kSrv2Db_AccountChangePasswordRequest	= 3,
	kSrv2Db_AccountSetRolesRequest			= 4,
	kSrv2Db_AccountSetBillingTypeRequest	= 5,
	kSrv2Db_AccountActivateRequest			= 6,
	kSrv2Db_AccountCreateFromKeyRequest		= 7,
	kSrv2Db_AccountLockPlayerNameRequest	= 8,
	kSrv2Db_SetPlayerBanStatusRequest		= 9,
	kSrv2Db_AccountLoginRequest2			= 10,
	kSrv2Db_AccountExistsRequest			= 11,

	// VaultNodes
	kSrv2Db_VaultNodeCreateRequest		= 20,
	kSrv2Db_VaultNodeFetchRequest		= 21,
	kSrv2Db_VaultNodeSaveRequest		= 22,
	kSrv2Db_VaultNodeDeleteRequest		= 23,
	kSrv2Db_VaultNodeFindRequest		= 24,
	kSrv2Db_VaultSendNode				= 25,
	kSrv2Db_SetAgeSequenceNumRequest	= 26,
	kSrv2Db_VaultNodeChanged			= 27,
	kSrv2Db_FetchInviterInfo			= 28,
	
	// VaultNodeRefs
	kSrv2Db_VaultNodeAddRefs		= 40,
	kSrv2Db_VaultNodeDelRefs		= 41,
	kSrv2Db_VaultNodeGetChildRefs	= 42,
	kSrv2Db_VaultNodeGetParentRefs	= 43,

	// State Objects
	kSrv2Db_StateSaveObject			= 60,
	kSrv2Db_StateDeleteObject		= 61,
	kSrv2Db_StateFetchObject		= 62,
	
	// Public ages
	kSrv2Db_GetPublicAgeInfoIds		= 80,
	kSrv2Db_SetAgePublic			= 81,
	
	// Score
	kSrv2Db_ScoreCreate				= 100,
	kSrv2Db_ScoreFindScoreIds		= 101,
	kSrv2Db_ScoreFetchScores		= 102,
	kSrv2Db_ScoreSave				= 103,
	kSrv2Db_ScoreDelete				= 104,
	kSrv2Db_ScoreGetRanks			= 105,

	// Vault Notifications
	kSrv2Db_PlayerOnline			= 120,
	kSrv2Db_PlayerOffline			= 121,
	kSrv2Db_AgeOnline				= 122,
	kSrv2Db_AgeOffline				= 123,

	// CSR
	kSrv2Db_CsrAcctInfoRequest		= 140,
};

enum {
    // Account
    kDb2Srv_AccountCreateReply			= 0,
    kDb2Srv_AccountLoginReply		    = 1,
	kDb2Srv_AccountCreateFromKeyReply	= 2,
	kDb2Srv_AccountExistsReply			= 3,

	// VaultNodes
	kDb2Srv_VaultNodeCreateReply	= 20,
	kDb2Srv_VaultNodeFetchReply		= 21,
	kDb2Srv_VaultNodeSaveReply		= 22,
	kDb2Srv_VaultNodeDeleteReply	= 23,
	kDb2Srv_VaultNodeFindReply		= 24,
	kDb2Srv_SetAgeSequenceNumReply	= 25,
	kDb2Srv_FetchInviterInfoReply	= 26,
	
	// VaultNodeRefs
	kDb2Srv_VaultNodeRefs			= 40,
	
	// State Objects
	kDb2Srv_StateObjectFetched		= 60,

	// Vault Notification
	kDb2Srv_NotifyVaultNodeChanged	= 80,
	kDb2Srv_NotifyVaultNodeAdded	= 81,
	kDb2Srv_NotifyVaultNodeRemoved	= 82,
	kDb2Srv_NotifyVaultNodeDeleted	= 83,

	// Public ages
	kDb2Srv_PublicAgeInfoIds		= 100,

	// Score
	kDb2Srv_ScoreCreateReply		= 120,
	kDb2Srv_ScoreFindScoreIdsReply	= 121,
	kDb2Srv_ScoreFetchScoresReply	= 122,
	kDb2Srv_ScoreDeleteReply		= 123,
	kDb2Srv_ScoreGetRanksReply		= 124,
	
	// CSR
	kDb2Srv_CsrAcctInfoReply		= 140,
};


//============================================================================
// BEGIN PACKED DATA STRUCTURES
//============================================================================
#include <PshPack1.h>


/*****************************************************************************
*
*   Srv2Db connect packet
*
***/

struct Srv2Db_ConnData {
    dword   dataBytes;
    dword   srvType;
};
struct Srv2Db_Connect {
    AsyncSocketConnectPacket    hdr;
    Srv2Db_ConnData             data;
};


/*****************************************************************************
*
*   Srv2Db packets
*
***/


struct Srv2Db_AccountExistsRequest : SrvMsgHeader {
    wchar       accountName[kMaxAccountNameLength];
};

struct Srv2Db_AccountCreateRequest : SrvMsgHeader {
    wchar       accountName[kMaxAccountNameLength];
    ShaDigest   namePassHash;
    dword       billingType;
	dword		accountFlags;
	wchar		foreignAcctId[kMaxPublisherAuthKeyLength];
};

struct Srv2Db_AccountCreateFromKeyRequest : SrvMsgHeader {
    wchar       accountName[kMaxAccountNameLength];
    ShaDigest   namePassHash;
    Uuid		key;
	dword		billingType;
};

struct Srv2Db_AccountLoginRequest : SrvMsgHeader {
    wchar       accountName[kMaxAccountNameLength];
};

struct Srv2Db_AccountLoginRequest2 : SrvMsgHeader {
    wchar       accountName[kMaxAccountNameLength];
    dword		buildId;
};

struct Srv2Db_AccountLogout : SrvMsgHeader {
    Uuid        accountUuid;
    dword       timeLoggedMins;
};

struct Srv2Db_AccountChangePasswordRequest : SrvMsgHeader {
    wchar       accountName[kMaxAccountNameLength];
    ShaDigest   namePassHash;
};

struct Srv2Db_AccountSetRolesRequest : SrvMsgHeader {
    wchar       accountName[kMaxAccountNameLength];
    dword		accountFlags;
};

struct Srv2Db_AccountSetBillingTypeRequest : SrvMsgHeader {
    wchar       accountName[kMaxAccountNameLength];
    dword		billingType;
};

struct Srv2Db_AccountActivateRequest : SrvMsgHeader {
    Uuid		activationKey;
};

struct Srv2Db_AccountLockPlayerNameRequest :SrvMsgHeader {
	wchar		playerName[kMaxPlayerNameLength];
	Uuid		accountUuid;
};

struct Srv2Db_VaultNodeCreateRequest : SrvMsgHeader {
    Uuid        accountUuid;
	dword		creatorId;
	dword		nodeBytes;
	byte		nodeBuffer[1];
};

struct Srv2Db_VaultNodeFetchRequest : SrvMsgHeader {
	dword		nodeId;
};

struct Srv2Db_VaultNodeChanged : SrvMsgHeader {
	dword		nodeId;
	Uuid		revisionId;
};

struct Srv2Db_VaultNodeSaveRequest : SrvMsgHeader {
	Uuid		revisionId;
	dword		nodeId;
	unsigned	playerCheckId;
	unsigned	isRequestFromAuth;
	dword		nodeBytes;
	byte		buffer[1];	// buffer[bytes], actually
	// no more fields after var length alloc
};

struct Srv2Db_VaultNodeDeleteRequest : SrvMsgHeader {
	dword		nodeId;
	unsigned	playerCheckId;
	unsigned	isRequestFromAuth;
};


struct Srv2Db_VaultNodeFindRequest : SrvMsgHeader {
	// Template node to match
	dword	nodeBytes;
	byte	nodeBuffer[1];	// [nodeBytes], actually
	// no more fields after var length alloc
};

struct Srv2Db_VaultNodeAddRefs : SrvMsgHeader {
	dword			refCount;
	NetVaultNodeRef	refs[1];
	// no more fields after var length alloc
};

struct Srv2Db_VaultNodeDelRefs : SrvMsgHeader {
	dword			refCount;
	unsigned		playerCheckId;
	unsigned		isRequestFromAuth;
	NetVaultNodeRef	refs[1];
	// no more fields after var length alloc
};

struct Srv2Db_VaultNodeGetChildRefs : SrvMsgHeader {
	dword			nodeId;
	dword			maxDepth;
};

struct Srv2Db_VaultNodeGetParentRefs : SrvMsgHeader {
	dword			nodeId;
	dword			maxDepth;
};

struct Srv2Db_VaultSendNode : SrvMsgHeader {
	dword	srcPlayerId;	// sender
	dword	srcNodeId;		// sent item
	dword	dstPlayerId;	// recipient
};

struct Srv2Db_SetAgeSequenceNumRequest : SrvMsgHeader {
	dword		nodeId;
	wchar		ageInstName[kMaxAgeNameLength];
	wchar		ageUserName[kMaxAgeNameLength];
};

struct Srv2Db_StateSaveObject : SrvMsgHeader {
	dword	buildId;
	Uuid	ownerId;		
	wchar	objectName[kMaxStateObjectName];
	dword	objectDataBytes;
	byte	objectData[1];
	// no more fields after var length alloc
};

struct Srv2Db_StateDeleteObject : SrvMsgHeader {
	Uuid	ownerId;
	wchar	objectName[kMaxStateObjectName];
};

struct Srv2Db_StateFetchObject : SrvMsgHeader {
	Uuid	ownerId;
	wchar	objectName[kMaxStateObjectName];
};

struct Srv2Db_GetPublicAgeInfoIds : SrvMsgHeader {
	wchar	ageName[kMaxAgeNameLength];
};

struct Srv2Db_SetAgePublic : SrvMsgHeader {
	dword	playerId;
	dword	ageInfoId;
	byte	publicOrNot;
};

struct Srv2Db_SetPlayerBanStatusRequest : SrvMsgHeader {
    dword	playerId;
	dword	banned;
};

struct Srv2Db_ScoreCreate : SrvMsgHeader {
    dword	ownerId;
	wchar	gameName[kMaxGameScoreNameLength];
	dword	gameType;
	dword	value;
};

struct Srv2Db_ScoreDelete : SrvMsgHeader {
    dword	scoreId;
};

struct Srv2Db_ScoreFindScoreIds : SrvMsgHeader {
    dword	ownerId;
	wchar	gameName[kMaxGameScoreNameLength];
};

struct Srv2Db_ScoreFetchScores : SrvMsgHeader {
	dword			scoreCount;
	dword			scoreIds[1];	// [scoreCount], actually
	// no more fields after var length alloc
};

struct Srv2Db_ScoreSave : SrvMsgHeader {
    dword	scoreId;
	dword	value;
};

struct Srv2Db_ScoreGetRanks : SrvMsgHeader {
	dword ownerId;
	dword scoreGroup;
	dword parentFolderId;
	wchar gameName[kMaxGameScoreNameLength];
	dword timePeriod;
	dword numResults;
	dword pageNumber;
	dword sortDesc;
};

struct Srv2Db_PlayerOnline : SrvMsgHeader {
	dword	playerId;
};

struct Srv2Db_PlayerOffline : SrvMsgHeader {
	dword	playerId;
};

struct Srv2Db_AgeOnline : SrvMsgHeader {
	Uuid	ageInstId;
};

struct Srv2Db_AgeOffline : SrvMsgHeader {
	Uuid	ageInstId;
};

struct Srv2Db_CsrAcctInfoRequest : SrvMsgHeader {
	wchar	csrName[kMaxAccountNameLength];
};

struct Srv2Db_FetchInviterInfo : SrvMsgHeader {
	Uuid	inviteUuid;
};


/*****************************************************************************
*
*   Db2Srv packets
*
***/

struct Db2Srv_AccountExistsReply : SrvMsgHeader {
	byte			exists;
};

struct Db2Srv_AccountCreateReply : SrvMsgHeader {
    Uuid            accountUuid;
};

struct Db2Srv_AccountCreateFromKeyReply : SrvMsgHeader {
    Uuid            accountUuid;
	Uuid			activationKey;
};

struct Db2Srv_AccountLoginReply : SrvMsgHeader {
	Uuid			accountUuid;
	dword			accountFlags;
	dword			billingType;
	ShaDigest		namePassHash;
};

struct Db2Srv_VaultNodeCreateReply : SrvMsgHeader {
	dword			nodeId;
};

struct Db2Srv_VaultNodeFetchReply : SrvMsgHeader {
	dword			nodeBytes;
	byte			buffer[1];
	// no more fields after var length alloc
};

struct Db2Srv_VaultNodeFindReply : SrvMsgHeader {
	// out: ids of matching nodes
	dword			nodeIdCount;
	dword			nodeIds[1];	// [nodeIdCount], actually
	// no more fields after var length alloc
};

struct Db2Srv_VaultNodeRefs : SrvMsgHeader {
	dword			refCount;
	NetVaultNodeRef refs[1];
	// no more fields after var length alloc
};

struct Db2Srv_SetAgeSequenceNumReply : SrvMsgHeader {
	dword		sequenceNum;
};

struct Db2Srv_FetchInviterInfoReply : SrvMsgHeader {
	Uuid		hoodInstance;
};

struct Db2Srv_StateObjectFetched : SrvMsgHeader {
	dword		buildId;
	Uuid		ownerId;		
	wchar		objectName[kMaxStateObjectName];
	dword		objectDataBytes;
	byte		objectData[1];
	// no more fields after var length alloc
};

struct Db2Srv_NotifyVaultNodeChanged : SrvMsgHeader {
	dword			nodeId;
	Uuid			revId;
	dword			notifyIdCount;
	dword			notifyIds[1];
};

struct Db2Srv_NotifyVaultNodeAdded : SrvMsgHeader {
	NetVaultNodeRef	ref;
	dword			notifyIdCount;
	dword			notifyIds[1];
};

struct Db2Srv_NotifyVaultNodeRemoved : SrvMsgHeader {
	dword			parentId;
	dword			childId;
	dword			notifyIdCount;
	dword			notifyIds[1];
};

struct Db2Srv_NotifyVaultNodeDeleted : SrvMsgHeader {
	dword			nodeId;
	dword			notifyIdCount;
	dword			notifyIds[1];
};

struct Db2Srv_PublicAgeInfoIds : SrvMsgHeader {
	dword			idCount;
	dword			ids[1];	// [idCount], actually
	// no more fields after var length alloc
};

struct Db2Srv_ScoreCreateReply : SrvMsgHeader {
	dword			scoreId;
	dword			createdTime;
};

struct Db2Srv_ScoreDeleteReply : SrvMsgHeader {
	dword			ownerId;
	wchar			gameName[kMaxGameScoreNameLength];
};

struct Db2Srv_ScoreFindScoreIdsReply : SrvMsgHeader {
	dword			idCount;
	dword			scoreIds[1];	// [idCount], actually
	// no more fields after var length alloc
};

struct Db2Srv_ScoreFetchScoresReply : SrvMsgHeader {
	dword			scoreCount;
	dword			byteCount;
	byte			buffer[1];	// [byteCount], actually
	// no more fields after var length alloc
};

struct Db2Srv_ScoreGetRanksReply : SrvMsgHeader {
	dword			rankCount;
	dword			byteCount;
	byte			buffer[1];	// [byteCount], actually
	// no more fields after var length alloc
};

struct Db2Srv_CsrAcctInfoReply : SrvMsgHeader {
	Uuid			csrId;
	dword			csrFlags;
	ShaDigest		namePassHash;
};


//============================================================================
// END PACKED DATA STRUCTURES
//============================================================================
#include <PopPack.h>


/*****************************************************************************
*
*   Srv2Db functions
*
***/

bool Srv2DbValidateConnect (
    AsyncNotifySocketListen *   listen,
    Srv2Db_ConnData *           connectPtr
);
