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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/Protocols/pnNpCli2Auth.cpp
*   
***/

#define USES_PROTOCOL_CLI2AUTH
#include "../../../Pch.h"
#pragma hdrstop


namespace Cli2Auth {
/*****************************************************************************
*
*   Cli2Auth message definitions
*
***/

static const NetMsgField kPingRequestFields[] = {
    kNetMsgFieldTimeMs,						// pingTimeMs
    NET_MSG_FIELD_DWORD(),					// transId
    NET_MSG_FIELD_VAR_COUNT(1, 64 * 1024),	// payloadBytes
    NET_MSG_FIELD_VAR_PTR(),				// payload
};

static const NetMsgField kClientRegisterRequestFields[] = {
    kNetMsgFieldBuildId,        // buildId
};

static const NetMsgField kAccountExistsRequestFields[] = {
	kNetMsgFieldTransId,							// transId
	kNetMsgFieldAccountName,						// accountName
};

static const NetMsgField kAcctLoginRequestFields[] = {
    kNetMsgFieldTransId,							// transId
	NET_MSG_FIELD_DWORD(),							// clientChallenge
    kNetMsgFieldAccountName,						// accountName
    kNetMsgFieldShaDigest,							// challenge
	NET_MSG_FIELD_STRING(kMaxPublisherAuthKeyLength),	// authToken
	NET_MSG_FIELD_STRING(kMaxGTOSIdLength),			// os
};

static const NetMsgField kAgeRequestFields[] = {
    kNetMsgFieldTransId,						// transId
    NET_MSG_FIELD_STRING(kMaxAgeNameLength),	// ageName
    kNetMsgFieldUuid,							// ageInstId
};

static const NetMsgField kAcctCreateRequestFields[] = {
	kNetMsgFieldTransId,							// transId
	NET_MSG_FIELD_STRING(kMaxAccountNameLength),	// accountName
	kNetMsgFieldShaDigest,							// namePassHash
	NET_MSG_FIELD_DWORD(),							// accountFlags
	NET_MSG_FIELD_DWORD(),							// billingType
};

static const NetMsgField kAcctCreateFromKeyRequestFields[] = {
	kNetMsgFieldTransId,							// transId
	NET_MSG_FIELD_STRING(kMaxAccountNameLength),	// accountName
	kNetMsgFieldShaDigest,							// namePassHash
	kNetMsgFieldUuid,								// key
	NET_MSG_FIELD_DWORD(),							// billingType
};

static const NetMsgField kPlayerCreateRequestFields[] = {
	kNetMsgFieldTransId,							// transId
	NET_MSG_FIELD_STRING(kMaxPlayerNameLength),		// playerName
	NET_MSG_FIELD_STRING(MAX_PATH),					// avatarShape
	NET_MSG_FIELD_STRING(MAX_PATH),					// friendInvite
};

static const NetMsgField kPlayerDeleteRequestFields[] = {
	kNetMsgFieldTransId,		// transId
    NET_MSG_FIELD_DWORD(),      // playerInt
};

static const NetMsgField kUpgradeVisitorRequestFields[] = {
	kNetMsgFieldTransId,		// transId
    NET_MSG_FIELD_DWORD(),      // playerInt
};

static const NetMsgField kAcctSetPlayerRequestFields[] = {
    kNetMsgFieldTransId,        // transId
    NET_MSG_FIELD_DWORD(),      // playerInt
};

static const NetMsgField kAcctChangePasswordRequestFields[] = {
	kNetMsgFieldTransId,							// transId
	NET_MSG_FIELD_STRING(kMaxAccountNameLength),	// accountName
	kNetMsgFieldShaDigest,							// namePassHash
};

static const NetMsgField kAcctSetRolesRequestFields[] = {
	kNetMsgFieldTransId,							// transId
	NET_MSG_FIELD_STRING(kMaxAccountNameLength),	// accountName
	NET_MSG_FIELD_DWORD(),							// accountFlags
};

static const NetMsgField kAcctSetBillingTypeRequestFields[] = {
	kNetMsgFieldTransId,							// transId
	NET_MSG_FIELD_STRING(kMaxAccountNameLength),	// accountName
	NET_MSG_FIELD_DWORD(),							// billingType
};

static const NetMsgField kAcctActivateRequestFields[] = {
	kNetMsgFieldTransId,							// transId
	kNetMsgFieldUuid,								// activationKey
};

static const NetMsgField kFileListRequestFields[] = {
	kNetMsgFieldTransId,				// transId
	NET_MSG_FIELD_STRING(MAX_PATH),		// directory
	NET_MSG_FIELD_STRING(MAX_EXT),		// ext
};

static const NetMsgField kFileDownloadRequestFields[] = {
	kNetMsgFieldTransId,				// transId
	NET_MSG_FIELD_STRING(MAX_PATH),		// filename
};

static const NetMsgField kFileDownloadChunkAckFields[] = {
	kNetMsgFieldTransId,				// transId
};

static const NetMsgField kVaultFetchNodeRefsFields[] = {
	kNetMsgFieldTransId,				// transId
	NET_MSG_FIELD_DWORD(),				// nodeId
};

static const NetMsgField kVaultNodeAddFields[] = {
	kNetMsgFieldTransId,				// transId
	NET_MSG_FIELD_DWORD(),				// parentId
	NET_MSG_FIELD_DWORD(),				// childId
	NET_MSG_FIELD_DWORD(),				// ownerId
};

static const NetMsgField kVaultNodeRemoveFields[] = {
	kNetMsgFieldTransId,				// transId
	NET_MSG_FIELD_DWORD(),				// parentId
	NET_MSG_FIELD_DWORD(),				// childId
};

static const NetMsgField kVaultNodeSaveFields[] = {
	kNetMsgFieldTransId,						// transId
	NET_MSG_FIELD_DWORD(),						// nodeId
	kNetMsgFieldUuid,							// revId
    NET_MSG_FIELD_VAR_COUNT(1, 1024 * 1024),	// nodeBytes
    NET_MSG_FIELD_VAR_PTR(),                    // nodeBuffer
};

static const NetMsgField kVaultNodeCreateFields[] = {
	kNetMsgFieldTransId,						// transId
    NET_MSG_FIELD_VAR_COUNT(1, 1024 * 1024),	// nodeBytes
    NET_MSG_FIELD_VAR_PTR(),                    // nodeBuffer
};

static const NetMsgField kVaultNodeFetchFields[] = {
	kNetMsgFieldTransId,				// transId
	NET_MSG_FIELD_DWORD(),				// nodeId
};

static const NetMsgField kVaultInitAgeRequestFields[] = {
	kNetMsgFieldTransId,						// transId
	kNetMsgFieldUuid,							// ageInstId
	kNetMsgFieldUuid,							// parentAgeInstId
    NET_MSG_FIELD_STRING(MAX_PATH),				// ageFilename
    NET_MSG_FIELD_STRING(MAX_PATH),				// ageInstName
    NET_MSG_FIELD_STRING(MAX_PATH),				// ageUserName
    NET_MSG_FIELD_STRING(1024),					// ageDesc
	NET_MSG_FIELD_DWORD(),						// ageSequenceNumber
	NET_MSG_FIELD_DWORD(),						// ageLanguage
};

static const NetMsgField kVaultNodeFindFields[] = {
	kNetMsgFieldTransId,						// transId
    NET_MSG_FIELD_VAR_COUNT(1, 1024 * 1024),	// nodeBytes
    NET_MSG_FIELD_VAR_PTR(),                    // nodeBuffer
};

static const NetMsgField kVaultSetSeenFields[] = {
	NET_MSG_FIELD_DWORD(),						// parentId
	NET_MSG_FIELD_DWORD(),						// childId
	NET_MSG_FIELD_BYTE(),						// seen
};

static const NetMsgField kVaultSendNodeFields[] = {
	NET_MSG_FIELD_DWORD(),						// srcNodeId
	NET_MSG_FIELD_DWORD(),						// dstPlayerId
};

static const NetMsgField kGetPublicAgeListFields[] = {
	kNetMsgFieldTransId,						// transId
    NET_MSG_FIELD_STRING(kMaxAgeNameLength),	// ageFilename
};

static const NetMsgField kSetAgePublicFields[] = {
	NET_MSG_FIELD_DWORD(),						// ageInfoId
	NET_MSG_FIELD_BYTE(),						// publicOrNot
};

static const NetMsgField kPropagateBufferFields[] = {
    NET_MSG_FIELD_DWORD(),                      // type
    NET_MSG_FIELD_VAR_COUNT(1, 1024 * 1024),	// bytes
    NET_MSG_FIELD_VAR_PTR(),                    // buffer
};

static const NetMsgField kClientSetCCRLevelFields[] = {
	NET_MSG_FIELD_DWORD(),						// ccrLevel
};

static const NetMsgField kLogPythonTracebackFields[] = {
    NET_MSG_FIELD_STRING(1024),					// traceback text
};

static const NetMsgField kLogStackDumpFields[] = {
	NET_MSG_FIELD_STRING(1024),					// stackdump text
};

static const NetMsgField kLogClientDebuggerConnectFields[] = {
	NET_MSG_FIELD_DWORD(),						// nothing
};

static const NetMsgField kSetPlayerBanStatusRequestFields[] = {
	kNetMsgFieldTransId,						// transId
	NET_MSG_FIELD_DWORD(),						// playerId
	NET_MSG_FIELD_DWORD(),						// banned
};

static const NetMsgField kKickPlayerFields[] = {
	NET_MSG_FIELD_DWORD(),						// playerId
};

static const NetMsgField kChangePlayerNameRequestFields[] = {
	kNetMsgFieldTransId,						// transId
	NET_MSG_FIELD_DWORD(),						// playerId
	NET_MSG_FIELD_STRING(kMaxPlayerNameLength),	// newName
};

static const NetMsgField kSendFriendInviteRequestFields[] = {
	kNetMsgFieldTransId,							// transId
	kNetMsgFieldUuid,								// inviteUuid
	NET_MSG_FIELD_STRING(kMaxEmailAddressLength),	// emailAddress
	NET_MSG_FIELD_STRING(kMaxPlayerNameLength),		// toPlayer
};

static const NetMsgField kScoreCreateFields[] = {
	kNetMsgFieldTransId,							// transId
	NET_MSG_FIELD_DWORD(),							// ownerId
	NET_MSG_FIELD_STRING(kMaxGameScoreNameLength),	// gameName
	NET_MSG_FIELD_DWORD(),							// gameType
	NET_MSG_FIELD_DWORD(),							// value
};

static const NetMsgField kScoreDeleteFields[] = {
	kNetMsgFieldTransId,							// transId
	NET_MSG_FIELD_DWORD(),							// scoreId
};

static const NetMsgField kScoreGetScoresFields[] = {
	kNetMsgFieldTransId,							// transId
	NET_MSG_FIELD_DWORD(),							// ownerId
	NET_MSG_FIELD_STRING(kMaxGameScoreNameLength),	// gameName
};

static const NetMsgField kScoreAddPointsFields[] = {
	kNetMsgFieldTransId,							// transId
	NET_MSG_FIELD_DWORD(),							// scoreId
	NET_MSG_FIELD_DWORD(),							// numPoints
};

static const NetMsgField kScoreTransferPointsFields[] = {
	kNetMsgFieldTransId,							// transId
	NET_MSG_FIELD_DWORD(),							// srcScoreId
	NET_MSG_FIELD_DWORD(),							// destScoreId
	NET_MSG_FIELD_DWORD(),							// numPoints
};

static const NetMsgField kScoreSetPointsFields[] = {
	kNetMsgFieldTransId,							// transId
	NET_MSG_FIELD_DWORD(),							// scoreId
	NET_MSG_FIELD_DWORD(),							// numPoints
};

static const NetMsgField kScoreGetRanksFields[] = {
	kNetMsgFieldTransId,							// transId
	NET_MSG_FIELD_DWORD(),							// ownerId
	NET_MSG_FIELD_DWORD(),							// scoreGroup
	NET_MSG_FIELD_DWORD(),							// parentFolderId
	NET_MSG_FIELD_STRING(kMaxGameScoreNameLength),	// gameName
	NET_MSG_FIELD_DWORD(),							// timePeriod
	NET_MSG_FIELD_DWORD(),							// numResults
	NET_MSG_FIELD_DWORD(),							// pageNumber
	NET_MSG_FIELD_DWORD(),							// sortDesc
};


/*****************************************************************************
*
*   Auth2Cli message fields
*
***/

static const NetMsgField kPingReplyFields[] = {
    kNetMsgFieldTimeMs,						// pingTimeMs
    NET_MSG_FIELD_DWORD(),					// transId
    NET_MSG_FIELD_VAR_COUNT(1, 64 * 1024),	// payloadBytes
    NET_MSG_FIELD_VAR_PTR(),				// payload
};

static const NetMsgField kClientRegisterReplyFields[] = {
    NET_MSG_FIELD_DWORD(),      // serverChallenge
};

static const NetMsgField kAccountExistsReplyFields[] = {
	kNetMsgFieldTransId,			// transId
    kNetMsgFieldENetError,			// result
	NET_MSG_FIELD_BYTE(),			// account exists
};

static const NetMsgField kServerAddrFields[] = {
	kNetMsgFieldNetNode,		// srvAddr
	kNetMsgFieldUuid,			// token
};

static const NetMsgField kNotifyNewBuildFields[] = {
    NET_MSG_FIELD_DWORD(),								// foo
};

static const NetMsgField kAcctPlayerInfoFields[] = {
    kNetMsgFieldTransId,								// transId
    NET_MSG_FIELD_DWORD(),								// playerInt
    NET_MSG_FIELD_STRING(kMaxPlayerNameLength),			// playerName
    NET_MSG_FIELD_STRING(kMaxVaultNodeStringLength),	// avatarShape
	NET_MSG_FIELD_DWORD(),								// explorer
};

static const NetMsgField kAcctLoginReplyFields[] = {
    kNetMsgFieldTransId,			// transId
    kNetMsgFieldENetError,			// result
    kNetMsgFieldUuid,				// accountId
	NET_MSG_FIELD_DWORD(),			// accountFlags
	NET_MSG_FIELD_DWORD(),			// billingType
	NET_MSG_FIELD_DWORD_ARRAY(4),	// encryptionKey
};

static const NetMsgField kAgeReplyFields[] = {
    kNetMsgFieldTransId,        // transId
    kNetMsgFieldENetError,      // result
    NET_MSG_FIELD_DWORD(),		// ageMcpId
    kNetMsgFieldUuid,			// ageInstId
    NET_MSG_FIELD_DWORD(),		// ageVaultId
    kNetMsgFieldNetNode,        // gameSrvNode
};

static const NetMsgField kAcctCreateReplyFields[] = {
	kNetMsgFieldTransId,								// transId
	kNetMsgFieldENetError,								// result
	kNetMsgFieldUuid									// accountId
};

static const NetMsgField kAcctCreateFromKeyReplyFields[] = {
	kNetMsgFieldTransId,								// transId
	kNetMsgFieldENetError,								// result
	kNetMsgFieldUuid,									// accountId
	kNetMsgFieldUuid									// activationKey
};

static const NetMsgField kPlayerCreateReplyFields[] = {
	kNetMsgFieldTransId,								// transId
	kNetMsgFieldENetError,								// result
    NET_MSG_FIELD_DWORD(),								// playerInt
	NET_MSG_FIELD_DWORD(),								// explorer
    NET_MSG_FIELD_STRING(kMaxPlayerNameLength),			// playerName
    NET_MSG_FIELD_STRING(kMaxVaultNodeStringLength),	// avatarShape
};

static const NetMsgField kPlayerDeleteReplyFields[] = {
	kNetMsgFieldTransId,								// transId
	kNetMsgFieldENetError,								// result
};

static const NetMsgField kUpgradeVisitorReplyFields[] = {
	kNetMsgFieldTransId,								// transId
	kNetMsgFieldENetError,								// result
};

static const NetMsgField kAcctSetPlayerReplyFields[] = {
    kNetMsgFieldTransId,        // transId
    kNetMsgFieldENetError,      // result
};

static const NetMsgField kAcctChangePasswordReplyFields[] = {
    kNetMsgFieldTransId,        // transId
    kNetMsgFieldENetError,      // result
};

static const NetMsgField kAcctSetRolesReplyFields[] = {
    kNetMsgFieldTransId,        // transId
    kNetMsgFieldENetError,      // result
};

static const NetMsgField kAcctSetBillingTypeReplyFields[] = {
    kNetMsgFieldTransId,        // transId
    kNetMsgFieldENetError,      // result
};

static const NetMsgField kAcctActivateReplyFields[] = {
    kNetMsgFieldTransId,        // transId
    kNetMsgFieldENetError,      // result
};

static const NetMsgField kFileListReplyFields[] = {
	kNetMsgFieldTransId,									// transId
	kNetMsgFieldENetError,									// result
	NET_MSG_FIELD_VAR_COUNT(sizeof(wchar), 1024 * 1024),	// wcharCount
	NET_MSG_FIELD_VAR_PTR(),								// fileData
};

static const NetMsgField kFileDownloadChunkFields[] = {
	kNetMsgFieldTransId,							// transId
	kNetMsgFieldENetError,							// result
	NET_MSG_FIELD_DWORD(),							// totalFileSize
	NET_MSG_FIELD_DWORD(),							// chunkOffset
	NET_MSG_FIELD_VAR_COUNT(1, kMaxTcpPacketSize),	// chunkSize
	NET_MSG_FIELD_VAR_PTR(),						// chunkData
};

static const NetMsgField kKickedOffFields[] = {
	kNetMsgFieldENetError,		// reason
};

static const NetMsgField kVaultNodeFields[] = {
    kNetMsgFieldTransId,								// transId
	NET_MSG_FIELD_VAR_COUNT(1, 1024 * 1024),			// nodeBytes
	NET_MSG_FIELD_VAR_PTR(),							// nodeBuffer
};

static const NetMsgField kVaultNodeRefsFields[] = {
    kNetMsgFieldTransId,											// transId
	NET_MSG_FIELD_VAR_COUNT(sizeof(NetVaultNodeRef), 1024 * 1024),	// refCount
	NET_MSG_FIELD_VAR_PTR(),										// refs
};

static const NetMsgField kVaultNodeCreatedFields[] = {
    kNetMsgFieldTransId,										// transId
	kNetMsgFieldENetError,										// result
	NET_MSG_FIELD_DWORD(),										// nodeId
};

static const NetMsgField kVaultNodeRefsFetchedFields[] = {
    kNetMsgFieldTransId,											// transId
	kNetMsgFieldENetError,											// result
	NET_MSG_FIELD_VAR_COUNT(sizeof(NetVaultNodeRef), 1024 * 1024),	// refCount
	NET_MSG_FIELD_VAR_PTR(),										// refs
};

static const NetMsgField kVaultNodeFetchedFields[] = {
    kNetMsgFieldTransId,									// transId
	kNetMsgFieldENetError,									// result
	NET_MSG_FIELD_VAR_COUNT(1, 1024 * 1024),				// nodeBytes
	NET_MSG_FIELD_VAR_PTR(),								// nodeBuffer
};

static const NetMsgField kVaultNodeChangedFields[] = {
	NET_MSG_FIELD_DWORD(),									// nodeId
    kNetMsgFieldUuid,										// revisionId
};

static const NetMsgField kVaultNodeAddedFields[] = {
	NET_MSG_FIELD_DWORD(),									// parentId
	NET_MSG_FIELD_DWORD(),									// childId
	NET_MSG_FIELD_DWORD(),									// ownerId
};

static const NetMsgField kVaultNodeRemovedFields[] = {
	NET_MSG_FIELD_DWORD(),									// parentId
	NET_MSG_FIELD_DWORD(),									// childId
};

static const NetMsgField kVaultNodeDeletedFields[] = {
	NET_MSG_FIELD_DWORD(),									// nodeId
};

static const NetMsgField kVaultSaveNodeReplyFields[] = {
    kNetMsgFieldTransId,        // transId
    kNetMsgFieldENetError,      // result
};

static const NetMsgField kVaultAddNodeReplyFields[] = {
    kNetMsgFieldTransId,        // transId
    kNetMsgFieldENetError,      // result
};

static const NetMsgField kVaultRemoveNodeReplyFields[] = {
    kNetMsgFieldTransId,        // transId
    kNetMsgFieldENetError,      // result
};

static const NetMsgField kVaultInitAgeReplyFields[] = {
    kNetMsgFieldTransId,									// transId
	kNetMsgFieldENetError,									// result
	NET_MSG_FIELD_DWORD(),									// ageVaultId
	NET_MSG_FIELD_DWORD(),									// ageInfoVaultId
};

static const NetMsgField kVaultNodeFindReplyFields[] = {
    kNetMsgFieldTransId,									// transId
	kNetMsgFieldENetError,									// result
	NET_MSG_FIELD_VAR_COUNT(sizeof(dword), 512),			// nodeIdCount
	NET_MSG_FIELD_VAR_PTR(),								// nodeIds
};

static const NetMsgField kPublicAgeListFields[] = {
    kNetMsgFieldTransId,									// transId
	kNetMsgFieldENetError,									// result
	NET_MSG_FIELD_VAR_COUNT(sizeof(NetAgeInfo), 512),		// ageCount
	NET_MSG_FIELD_VAR_PTR(),								// ages
};

static const NetMsgField kSetPlayerBanStatusReplyFields[] = {
    kNetMsgFieldTransId,        // transId
    kNetMsgFieldENetError,      // result
};

static const NetMsgField kChangePlayerNameReplyFields[] = {
    kNetMsgFieldTransId,        // transId
    kNetMsgFieldENetError,      // result
};

static const NetMsgField kSendFriendInviteReplyFields[] = {
    kNetMsgFieldTransId,        // transId
    kNetMsgFieldENetError,      // result
};

static const NetMsgField kScoreCreateReplyFields[] = {
    kNetMsgFieldTransId,        // transId
    kNetMsgFieldENetError,      // result
	NET_MSG_FIELD_DWORD(),		// scoreId
	NET_MSG_FIELD_DWORD(),		// createdTime
};

static const NetMsgField kScoreDeleteReplyFields[] = {
    kNetMsgFieldTransId,									// transId
	kNetMsgFieldENetError,									// result
};

static const NetMsgField kScoreGetScoresReplyFields[] = {
    kNetMsgFieldTransId,									// transId
	kNetMsgFieldENetError,									// result
	NET_MSG_FIELD_DWORD(),									// scoreCount
	NET_MSG_FIELD_VAR_COUNT(1, 1024 * 1024),				// nodeBytes
	NET_MSG_FIELD_VAR_PTR(),								// nodeBuffer
};

static const NetMsgField kScoreAddPointsReplyFields[] = {
    kNetMsgFieldTransId,									// transId
	kNetMsgFieldENetError,									// result
};

static const NetMsgField kScoreTransferPointsReplyFields[] = {
    kNetMsgFieldTransId,									// transId
	kNetMsgFieldENetError,									// result
};

static const NetMsgField kScoreSetPointsReplyFields[] = {
    kNetMsgFieldTransId,									// transId
	kNetMsgFieldENetError,									// result
};

static const NetMsgField kScoreGetRanksReplyFields[] = {
    kNetMsgFieldTransId,									// transId
	kNetMsgFieldENetError,									// result
	NET_MSG_FIELD_DWORD(),									// rankCount
	NET_MSG_FIELD_VAR_COUNT(1, 1024 * 1024),				// nodeBytes
	NET_MSG_FIELD_VAR_PTR(),								// nodeBuffer
};

} using namespace Cli2Auth;


/*****************************************************************************
*
*   Exported data
*
***/


const NetMsg kNetMsg_Cli2Auth_PingRequest				= NET_MSG(kCli2Auth_PingRequest,				kPingRequestFields);
const NetMsg kNetMsg_Cli2Auth_ClientRegisterRequest		= NET_MSG(kCli2Auth_ClientRegisterRequest,		kClientRegisterRequestFields);
const NetMsg kNetMsg_Cli2Auth_AccountExistsRequest		= NET_MSG(kCli2Auth_AccountExistsRequest,		kAccountExistsRequestFields);
const NetMsg kNetMsg_Cli2Auth_AcctLoginRequest			= NET_MSG(kCli2Auth_AcctLoginRequest,			kAcctLoginRequestFields);
const NetMsg kNetMsg_Cli2Auth_AcctCreateRequest			= NET_MSG(kCli2Auth_AcctCreateRequest,			kAcctCreateRequestFields);
const NetMsg kNetMsg_Cli2Auth_AcctCreateFromKeyRequest	= NET_MSG(kCli2Auth_AcctCreateFromKeyRequest,	kAcctCreateFromKeyRequestFields);
const NetMsg kNetMsg_Cli2Auth_PlayerCreateRequest		= NET_MSG(kCli2Auth_PlayerCreateRequest,		kPlayerCreateRequestFields);
const NetMsg kNetMsg_Cli2Auth_PlayerDeleteRequest		= NET_MSG(kCli2Auth_PlayerDeleteRequest,		kPlayerDeleteRequestFields);
const NetMsg kNetMsg_Cli2Auth_UpgradeVisitorRequest		= NET_MSG(kCli2Auth_UpgradeVisitorRequest,		kUpgradeVisitorRequestFields);
const NetMsg kNetMsg_Cli2Auth_AcctSetPlayerRequest		= NET_MSG(kCli2Auth_AcctSetPlayerRequest,		kAcctSetPlayerRequestFields);
const NetMsg kNetMsg_Cli2Auth_AcctChangePasswordRequest	= NET_MSG(kCli2Auth_AcctChangePasswordRequest,	kAcctChangePasswordRequestFields);
const NetMsg kNetMsg_Cli2Auth_AcctSetRolesRequest		= NET_MSG(kCli2Auth_AcctSetRolesRequest,		kAcctSetRolesRequestFields);
const NetMsg kNetMsg_Cli2Auth_AcctSetBillingTypeRequest	= NET_MSG(kCli2Auth_AcctSetBillingTypeRequest,	kAcctSetBillingTypeRequestFields);
const NetMsg kNetMsg_Cli2Auth_AcctActivateRequest		= NET_MSG(kCli2Auth_AcctActivateRequest,		kAcctActivateRequestFields);
const NetMsg kNetMsg_Cli2Auth_AgeRequest				= NET_MSG(kCli2Auth_AgeRequest,					kAgeRequestFields);
const NetMsg kNetMsg_Cli2Auth_FileListRequest			= NET_MSG(kCli2Auth_FileListRequest,			kFileListRequestFields);
const NetMsg kNetMsg_Cli2Auth_FileDownloadRequest		= NET_MSG(kCli2Auth_FileDownloadRequest,		kFileDownloadRequestFields);
const NetMsg kNetMsg_Cli2Auth_FileDownloadChunkAck		= NET_MSG(kCli2Auth_FileDownloadChunkAck,		kFileDownloadChunkAckFields);
const NetMsg kNetMsg_Cli2Auth_VaultFetchNodeRefs		= NET_MSG(kCli2Auth_VaultFetchNodeRefs,			kVaultFetchNodeRefsFields);
const NetMsg kNetMsg_Cli2Auth_VaultNodeAdd				= NET_MSG(kCli2Auth_VaultNodeAdd,				kVaultNodeAddFields);
const NetMsg kNetMsg_Cli2Auth_VaultNodeRemove			= NET_MSG(kCli2Auth_VaultNodeRemove,			kVaultNodeRemoveFields);
const NetMsg kNetMsg_Cli2Auth_VaultNodeCreate			= NET_MSG(kCli2Auth_VaultNodeCreate,			kVaultNodeCreateFields);
const NetMsg kNetMsg_Cli2Auth_VaultNodeSave				= NET_MSG(kCli2Auth_VaultNodeSave,				kVaultNodeSaveFields);
const NetMsg kNetMsg_Cli2Auth_VaultNodeFetch			= NET_MSG(kCli2Auth_VaultNodeFetch,				kVaultNodeFetchFields);
const NetMsg kNetMsg_Cli2Auth_VaultInitAgeRequest		= NET_MSG(kCli2Auth_VaultInitAgeRequest,		kVaultInitAgeRequestFields);
const NetMsg kNetMsg_Cli2Auth_VaultNodeFind				= NET_MSG(kCli2Auth_VaultNodeFind,				kVaultNodeFindFields);
const NetMsg kNetMsg_Cli2Auth_VaultSetSeen				= NET_MSG(kCli2Auth_VaultSetSeen,				kVaultSetSeenFields);
const NetMsg kNetMsg_Cli2Auth_VaultSendNode				= NET_MSG(kCli2Auth_VaultSendNode,				kVaultSendNodeFields);
const NetMsg kNetMsg_Cli2Auth_GetPublicAgeList			= NET_MSG(kCli2Auth_GetPublicAgeList,			kGetPublicAgeListFields);
const NetMsg kNetMsg_Cli2Auth_SetAgePublic				= NET_MSG(kCli2Auth_SetAgePublic,				kSetAgePublicFields);
const NetMsg kNetMsg_Cli2Auth_PropagateBuffer			= NET_MSG(kCli2Auth_PropagateBuffer,			kPropagateBufferFields);
const NetMsg kNetMsg_Cli2Auth_ClientSetCCRLevel			= NET_MSG(kCli2Auth_ClientSetCCRLevel,			kClientSetCCRLevelFields);
const NetMsg kNetMsg_Cli2Auth_LogPythonTraceback		= NET_MSG(kCli2Auth_LogPythonTraceback,			kLogPythonTracebackFields);
const NetMsg kNetMsg_Cli2Auth_LogStackDump				= NET_MSG(kCli2Auth_LogStackDump,				kLogStackDumpFields);
const NetMsg kNetMsg_Cli2Auth_LogClientDebuggerConnect	= NET_MSG(kCli2Auth_LogClientDebuggerConnect,	kLogClientDebuggerConnectFields);
const NetMsg kNetMsg_Cli2Auth_SetPlayerBanStatusRequest	= NET_MSG(kCli2Auth_SetPlayerBanStatusRequest,	kSetPlayerBanStatusRequestFields);
const NetMsg kNetMsg_Cli2Auth_KickPlayer				= NET_MSG(kCli2Auth_KickPlayer,					kKickPlayerFields);
const NetMsg kNetMsg_Cli2Auth_ChangePlayerNameRequest	= NET_MSG(kCli2Auth_ChangePlayerNameRequest,	kChangePlayerNameRequestFields);
const NetMsg kNetMsg_Cli2Auth_SendFriendInviteRequest	= NET_MSG(kCli2Auth_SendFriendInviteRequest,	kSendFriendInviteRequestFields);
const NetMsg kNetMsg_Cli2Auth_ScoreCreate				= NET_MSG(kCli2Auth_ScoreCreate,				kScoreCreateFields);
const NetMsg kNetMsg_Cli2Auth_ScoreDelete				= NET_MSG(kCli2Auth_ScoreDelete,				kScoreDeleteFields);
const NetMsg kNetMsg_Cli2Auth_ScoreGetScores			= NET_MSG(kCli2Auth_ScoreGetScores,				kScoreGetScoresFields);
const NetMsg kNetMsg_Cli2Auth_ScoreAddPoints			= NET_MSG(kCli2Auth_ScoreAddPoints,				kScoreAddPointsFields);
const NetMsg kNetMsg_Cli2Auth_ScoreTransferPoints		= NET_MSG(kCli2Auth_ScoreTransferPoints,		kScoreTransferPointsFields);
const NetMsg kNetMsg_Cli2Auth_ScoreSetPoints			= NET_MSG(kCli2Auth_ScoreSetPoints,				kScoreSetPointsFields);
const NetMsg kNetMsg_Cli2Auth_ScoreGetRanks				= NET_MSG(kCli2Auth_ScoreGetRanks,				kScoreGetRanksFields);

const NetMsg kNetMsg_Auth2Cli_PingReply					= NET_MSG(kAuth2Cli_PingReply,					kPingReplyFields);
const NetMsg kNetMsg_Auth2Cli_ClientRegisterReply		= NET_MSG(kAuth2Cli_ClientRegisterReply,		kClientRegisterReplyFields);
const NetMsg kNetMsg_Auth2Cli_AccountExistsReply		= NET_MSG(kAuth2Cli_AccountExistsReply,			kAccountExistsReplyFields);
const NetMsg kNetMsg_Auth2Cli_ServerAddr				= NET_MSG(kAuth2Cli_ServerAddr,					kServerAddrFields);
const NetMsg kNetMsg_Auth2Cli_NotifyNewBuild			= NET_MSG(kAuth2Cli_NotifyNewBuild,				kNotifyNewBuildFields);
const NetMsg kNetMsg_Auth2Cli_AcctPlayerInfo			= NET_MSG(kAuth2Cli_AcctPlayerInfo,				kAcctPlayerInfoFields);
const NetMsg kNetMsg_Auth2Cli_AcctLoginReply			= NET_MSG(kAuth2Cli_AcctLoginReply,				kAcctLoginReplyFields);
const NetMsg kNetMsg_Auth2Cli_AcctCreateReply			= NET_MSG(kAuth2Cli_AcctCreateReply,			kAcctCreateReplyFields);
const NetMsg kNetMsg_Auth2Cli_AcctCreateFromKeyReply	= NET_MSG(kAuth2Cli_AcctCreateFromKeyReply,		kAcctCreateFromKeyReplyFields);
const NetMsg kNetMsg_Auth2Cli_PlayerCreateReply			= NET_MSG(kAuth2Cli_PlayerCreateReply,			kPlayerCreateReplyFields);
const NetMsg kNetMsg_Auth2Cli_PlayerDeleteReply			= NET_MSG(kAuth2Cli_PlayerDeleteReply,			kPlayerDeleteReplyFields);
const NetMsg kNetMsg_Auth2Cli_UpgradeVisitorReply		= NET_MSG(kAuth2Cli_UpgradeVisitorReply,		kUpgradeVisitorReplyFields);
const NetMsg kNetMsg_Auth2Cli_AcctSetPlayerReply		= NET_MSG(kAuth2Cli_AcctSetPlayerReply,			kAcctSetPlayerReplyFields);
const NetMsg kNetMsg_Auth2Cli_AcctChangePasswordReply	= NET_MSG(kAuth2Cli_AcctChangePasswordReply,	kAcctChangePasswordReplyFields);
const NetMsg kNetMsg_Auth2Cli_AcctSetRolesReply			= NET_MSG(kAuth2Cli_AcctSetRolesReply,			kAcctSetRolesReplyFields);
const NetMsg kNetMsg_Auth2Cli_AcctSetBillingTypeReply	= NET_MSG(kAuth2Cli_AcctSetBillingTypeReply,	kAcctSetBillingTypeReplyFields);
const NetMsg kNetMsg_Auth2Cli_AcctActivateReply			= NET_MSG(kAuth2Cli_AcctActivateReply,			kAcctActivateReplyFields);
const NetMsg kNetMsg_Auth2Cli_AgeReply					= NET_MSG(kAuth2Cli_AgeReply,					kAgeReplyFields);
const NetMsg kNetMsg_Auth2Cli_FileListReply				= NET_MSG(kAuth2Cli_FileListReply,				kFileListReplyFields);
const NetMsg kNetMsg_Auth2Cli_FileDownloadChunk			= NET_MSG(kAuth2Cli_FileDownloadChunk,			kFileDownloadChunkFields);
const NetMsg kNetMsg_Auth2Cli_KickedOff					= NET_MSG(kAuth2Cli_KickedOff,					kKickedOffFields);
const NetMsg kNetMsg_Auth2Cli_VaultNodeRefsFetched		= NET_MSG(kAuth2Cli_VaultNodeRefsFetched,		kVaultNodeRefsFetchedFields);
const NetMsg kNetMsg_Auth2Cli_VaultNodeCreated			= NET_MSG(kAuth2Cli_VaultNodeCreated,			kVaultNodeCreatedFields);
const NetMsg kNetMsg_Auth2Cli_VaultNodeFetched			= NET_MSG(kAuth2Cli_VaultNodeFetched,			kVaultNodeFetchedFields);
const NetMsg kNetMsg_Auth2Cli_VaultNodeChanged			= NET_MSG(kAuth2Cli_VaultNodeChanged,			kVaultNodeChangedFields);
const NetMsg kNetMsg_Auth2Cli_VaultNodeAdded			= NET_MSG(kAuth2Cli_VaultNodeAdded,				kVaultNodeAddedFields);
const NetMsg kNetMsg_Auth2Cli_VaultNodeRemoved			= NET_MSG(kAuth2Cli_VaultNodeRemoved,			kVaultNodeRemovedFields);
const NetMsg kNetMsg_Auth2Cli_VaultNodeDeleted			= NET_MSG(kAuth2Cli_VaultNodeDeleted,			kVaultNodeDeletedFields);
const NetMsg kNetMsg_Auth2Cli_VaultSaveNodeReply		= NET_MSG(kAuth2Cli_VaultSaveNodeReply,			kVaultSaveNodeReplyFields);
const NetMsg kNetMsg_Auth2Cli_VaultAddNodeReply			= NET_MSG(kAuth2Cli_VaultAddNodeReply,			kVaultAddNodeReplyFields);
const NetMsg kNetMsg_Auth2Cli_VaultRemoveNodeReply		= NET_MSG(kAuth2Cli_VaultRemoveNodeReply,		kVaultRemoveNodeReplyFields);
const NetMsg kNetMsg_Auth2Cli_VaultInitAgeReply			= NET_MSG(kAuth2Cli_VaultInitAgeReply,			kVaultInitAgeReplyFields);
const NetMsg kNetMsg_Auth2Cli_VaultNodeFindReply		= NET_MSG(kAuth2Cli_VaultNodeFindReply,			kVaultNodeFindReplyFields);
const NetMsg kNetMsg_Auth2Cli_PublicAgeList				= NET_MSG(kAuth2Cli_PublicAgeList,				kPublicAgeListFields);
const NetMsg kNetMsg_Auth2Cli_PropagateBuffer			= NET_MSG(kAuth2Cli_PropagateBuffer,			kPropagateBufferFields);
const NetMsg kNetMsg_Auth2Cli_SetPlayerBanStatusReply	= NET_MSG(kAuth2Cli_SetPlayerBanStatusReply,	kSetPlayerBanStatusReplyFields);
const NetMsg kNetMsg_Auth2Cli_ChangePlayerNameReply		= NET_MSG(kAuth2Cli_ChangePlayerNameReply,		kChangePlayerNameReplyFields);
const NetMsg kNetMsg_Auth2Cli_SendFriendInviteReply		= NET_MSG(kAuth2Cli_SendFriendInviteReply,		kSendFriendInviteReplyFields);
const NetMsg kNetMsg_Auth2Cli_ScoreCreateReply			= NET_MSG(kAuth2Cli_ScoreCreateReply,			kScoreCreateReplyFields);
const NetMsg kNetMsg_Auth2Cli_ScoreDeleteReply			= NET_MSG(kAuth2Cli_ScoreDeleteReply,			kScoreDeleteReplyFields);
const NetMsg kNetMsg_Auth2Cli_ScoreGetScoresReply		= NET_MSG(kAuth2Cli_ScoreGetScoresReply,		kScoreGetScoresReplyFields);
const NetMsg kNetMsg_Auth2Cli_ScoreAddPointsReply		= NET_MSG(kAuth2Cli_ScoreAddPointsReply,		kScoreAddPointsReplyFields);
const NetMsg kNetMsg_Auth2Cli_ScoreTransferPointsReply	= NET_MSG(kAuth2Cli_ScoreTransferPointsReply,	kScoreTransferPointsReplyFields);
const NetMsg kNetMsg_Auth2Cli_ScoreSetPointsReply		= NET_MSG(kAuth2Cli_ScoreSetPointsReply,		kScoreSetPointsReplyFields);
const NetMsg kNetMsg_Auth2Cli_ScoreGetRanksReply		= NET_MSG(kAuth2Cli_ScoreGetRanksReply,			kScoreGetRanksReplyFields);
