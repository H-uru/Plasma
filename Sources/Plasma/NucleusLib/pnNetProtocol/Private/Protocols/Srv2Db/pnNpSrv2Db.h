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
    kSrv2Db_AccountCreateRequest            = 0,
    kSrv2Db_AccountLoginRequest             = 1,
    kSrv2Db_AccountLogout                   = 2,
    kSrv2Db_AccountChangePasswordRequest    = 3,
    kSrv2Db_AccountSetRolesRequest          = 4,
    kSrv2Db_AccountSetBillingTypeRequest    = 5,
    kSrv2Db_AccountActivateRequest          = 6,
    kSrv2Db_AccountCreateFromKeyRequest     = 7,
    kSrv2Db_AccountLockPlayerNameRequest    = 8,
    kSrv2Db_SetPlayerBanStatusRequest       = 9,
    kSrv2Db_AccountLoginRequest2            = 10,
    kSrv2Db_AccountExistsRequest            = 11,

    // VaultNodes
    kSrv2Db_VaultNodeCreateRequest      = 20,
    kSrv2Db_VaultNodeFetchRequest       = 21,
    kSrv2Db_VaultNodeSaveRequest        = 22,
    kSrv2Db_VaultNodeDeleteRequest      = 23,
    kSrv2Db_VaultNodeFindRequest        = 24,
    kSrv2Db_VaultSendNode               = 25,
    kSrv2Db_SetAgeSequenceNumRequest    = 26,
    kSrv2Db_VaultNodeChanged            = 27,
    kSrv2Db_FetchInviterInfo            = 28,
    
    // VaultNodeRefs
    kSrv2Db_VaultNodeAddRefs        = 40,
    kSrv2Db_VaultNodeDelRefs        = 41,
    kSrv2Db_VaultNodeGetChildRefs   = 42,
    kSrv2Db_VaultNodeGetParentRefs  = 43,

    // State Objects
    kSrv2Db_StateSaveObject         = 60,
    kSrv2Db_StateDeleteObject       = 61,
    kSrv2Db_StateFetchObject        = 62,
    
    // Public ages
    kSrv2Db_GetPublicAgeInfoIds     = 80,
    kSrv2Db_SetAgePublic            = 81,
    
    // Score
    kSrv2Db_ScoreCreate             = 100,
    kSrv2Db_ScoreFindScoreIds       = 101,
    kSrv2Db_ScoreFetchScores        = 102,
    kSrv2Db_ScoreSave               = 103,
    kSrv2Db_ScoreDelete             = 104,
    kSrv2Db_ScoreGetRanks           = 105,

    // Vault Notifications
    kSrv2Db_PlayerOnline            = 120,
    kSrv2Db_PlayerOffline           = 121,
    kSrv2Db_AgeOnline               = 122,
    kSrv2Db_AgeOffline              = 123,

    // CSR
    kSrv2Db_CsrAcctInfoRequest      = 140,
};

enum {
    // Account
    kDb2Srv_AccountCreateReply          = 0,
    kDb2Srv_AccountLoginReply           = 1,
    kDb2Srv_AccountCreateFromKeyReply   = 2,
    kDb2Srv_AccountExistsReply          = 3,

    // VaultNodes
    kDb2Srv_VaultNodeCreateReply    = 20,
    kDb2Srv_VaultNodeFetchReply     = 21,
    kDb2Srv_VaultNodeSaveReply      = 22,
    kDb2Srv_VaultNodeDeleteReply    = 23,
    kDb2Srv_VaultNodeFindReply      = 24,
    kDb2Srv_SetAgeSequenceNumReply  = 25,
    kDb2Srv_FetchInviterInfoReply   = 26,
    
    // VaultNodeRefs
    kDb2Srv_VaultNodeRefs           = 40,
    
    // State Objects
    kDb2Srv_StateObjectFetched      = 60,

    // Vault Notification
    kDb2Srv_NotifyVaultNodeChanged  = 80,
    kDb2Srv_NotifyVaultNodeAdded    = 81,
    kDb2Srv_NotifyVaultNodeRemoved  = 82,
    kDb2Srv_NotifyVaultNodeDeleted  = 83,

    // Public ages
    kDb2Srv_PublicAgeInfoIds        = 100,

    // Score
    kDb2Srv_ScoreCreateReply        = 120,
    kDb2Srv_ScoreFindScoreIdsReply  = 121,
    kDb2Srv_ScoreFetchScoresReply   = 122,
    kDb2Srv_ScoreDeleteReply        = 123,
    kDb2Srv_ScoreGetRanksReply      = 124,
    
    // CSR
    kDb2Srv_CsrAcctInfoReply        = 140,
};


//============================================================================
// BEGIN PACKED DATA STRUCTURES
//============================================================================
#pragma pack(push,1)


/*****************************************************************************
*
*   Srv2Db connect packet
*
***/

struct Srv2Db_ConnData {
    uint32_t   dataBytes;
    uint32_t   srvType;
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
    wchar_t       accountName[kMaxAccountNameLength];
};

struct Srv2Db_AccountCreateRequest : SrvMsgHeader {
    wchar_t       accountName[kMaxAccountNameLength];
    ShaDigest   namePassHash;
    uint32_t       billingType;
    uint32_t       accountFlags;
    wchar_t       foreignAcctId[kMaxPublisherAuthKeyLength];
};

struct Srv2Db_AccountCreateFromKeyRequest : SrvMsgHeader {
    wchar_t       accountName[kMaxAccountNameLength];
    ShaDigest   namePassHash;
    plUUID      key;
    uint32_t       billingType;
};

struct Srv2Db_AccountLoginRequest : SrvMsgHeader {
    wchar_t       accountName[kMaxAccountNameLength];
};

struct Srv2Db_AccountLoginRequest2 : SrvMsgHeader {
    wchar_t       accountName[kMaxAccountNameLength];
    uint32_t       buildId;
};

struct Srv2Db_AccountLogout : SrvMsgHeader {
    plUUID      accountUuid;
    uint32_t       timeLoggedMins;
};

struct Srv2Db_AccountChangePasswordRequest : SrvMsgHeader {
    wchar_t       accountName[kMaxAccountNameLength];
    ShaDigest   namePassHash;
};

struct Srv2Db_AccountSetRolesRequest : SrvMsgHeader {
    wchar_t       accountName[kMaxAccountNameLength];
    uint32_t       accountFlags;
};

struct Srv2Db_AccountSetBillingTypeRequest : SrvMsgHeader {
    wchar_t       accountName[kMaxAccountNameLength];
    uint32_t       billingType;
};

struct Srv2Db_AccountActivateRequest : SrvMsgHeader {
    plUUID      activationKey;
};

struct Srv2Db_AccountLockPlayerNameRequest :SrvMsgHeader {
    wchar_t       playerName[kMaxPlayerNameLength];
    plUUID      accountUuid;
};

struct Srv2Db_VaultNodeCreateRequest : SrvMsgHeader {
    plUUID      accountUuid;
    uint32_t       creatorId;
    uint32_t       nodeBytes;
    uint8_t        nodeBuffer[1];
};

struct Srv2Db_VaultNodeFetchRequest : SrvMsgHeader {
    uint32_t       nodeId;
};

struct Srv2Db_VaultNodeChanged : SrvMsgHeader {
    uint32_t       nodeId;
    plUUID      revisionId;
};

struct Srv2Db_VaultNodeSaveRequest : SrvMsgHeader {
    plUUID      revisionId;
    uint32_t       nodeId;
    unsigned    playerCheckId;
    unsigned    isRequestFromAuth;
    uint32_t       nodeBytes;
    uint8_t        buffer[1];  // buffer[bytes], actually
    // no more fields after var length alloc
};

struct Srv2Db_VaultNodeDeleteRequest : SrvMsgHeader {
    uint32_t       nodeId;
    unsigned    playerCheckId;
    unsigned    isRequestFromAuth;
};


struct Srv2Db_VaultNodeFindRequest : SrvMsgHeader {
    // Template node to match
    uint32_t   nodeBytes;
    uint8_t    nodeBuffer[1];  // [nodeBytes], actually
    // no more fields after var length alloc
};

struct Srv2Db_VaultNodeAddRefs : SrvMsgHeader {
    uint32_t           refCount;
    NetVaultNodeRef refs[1];
    // no more fields after var length alloc
};

struct Srv2Db_VaultNodeDelRefs : SrvMsgHeader {
    uint32_t           refCount;
    unsigned        playerCheckId;
    unsigned        isRequestFromAuth;
    NetVaultNodeRef refs[1];
    // no more fields after var length alloc
};

struct Srv2Db_VaultNodeGetChildRefs : SrvMsgHeader {
    uint32_t           nodeId;
    uint32_t           maxDepth;
};

struct Srv2Db_VaultNodeGetParentRefs : SrvMsgHeader {
    uint32_t           nodeId;
    uint32_t           maxDepth;
};

struct Srv2Db_VaultSendNode : SrvMsgHeader {
    uint32_t   srcPlayerId;    // sender
    uint32_t   srcNodeId;      // sent item
    uint32_t   dstPlayerId;    // recipient
};

struct Srv2Db_SetAgeSequenceNumRequest : SrvMsgHeader {
    uint32_t       nodeId;
    wchar_t       ageInstName[kMaxAgeNameLength];
    wchar_t       ageUserName[kMaxAgeNameLength];
};

struct Srv2Db_StateSaveObject : SrvMsgHeader {
    uint32_t   buildId;
    plUUID  ownerId;
    wchar_t   objectName[kMaxStateObjectName];
    uint32_t   objectDataBytes;
    uint8_t    objectData[1];
    // no more fields after var length alloc
};

struct Srv2Db_StateDeleteObject : SrvMsgHeader {
    plUUID  ownerId;
    wchar_t   objectName[kMaxStateObjectName];
};

struct Srv2Db_StateFetchObject : SrvMsgHeader {
    plUUID  ownerId;
    wchar_t   objectName[kMaxStateObjectName];
};

struct Srv2Db_GetPublicAgeInfoIds : SrvMsgHeader {
    wchar_t   ageName[kMaxAgeNameLength];
};

struct Srv2Db_SetAgePublic : SrvMsgHeader {
    uint32_t   playerId;
    uint32_t   ageInfoId;
    uint8_t    publicOrNot;
};

struct Srv2Db_SetPlayerBanStatusRequest : SrvMsgHeader {
    uint32_t   playerId;
    uint32_t   banned;
};

struct Srv2Db_ScoreCreate : SrvMsgHeader {
    uint32_t   ownerId;
    wchar_t   gameName[kMaxGameScoreNameLength];
    uint32_t   gameType;
    uint32_t   value;
};

struct Srv2Db_ScoreDelete : SrvMsgHeader {
    uint32_t   scoreId;
};

struct Srv2Db_ScoreFindScoreIds : SrvMsgHeader {
    uint32_t   ownerId;
    wchar_t   gameName[kMaxGameScoreNameLength];
};

struct Srv2Db_ScoreFetchScores : SrvMsgHeader {
    uint32_t           scoreCount;
    uint32_t           scoreIds[1];    // [scoreCount], actually
    // no more fields after var length alloc
};

struct Srv2Db_ScoreSave : SrvMsgHeader {
    uint32_t   scoreId;
    uint32_t   value;
};

struct Srv2Db_ScoreGetRanks : SrvMsgHeader {
    uint32_t ownerId;
    uint32_t scoreGroup;
    uint32_t parentFolderId;
    wchar_t gameName[kMaxGameScoreNameLength];
    uint32_t timePeriod;
    uint32_t numResults;
    uint32_t pageNumber;
    uint32_t sortDesc;
};

struct Srv2Db_PlayerOnline : SrvMsgHeader {
    uint32_t   playerId;
};

struct Srv2Db_PlayerOffline : SrvMsgHeader {
    uint32_t   playerId;
};

struct Srv2Db_AgeOnline : SrvMsgHeader {
    plUUID  ageInstId;
};

struct Srv2Db_AgeOffline : SrvMsgHeader {
    plUUID  ageInstId;
};

struct Srv2Db_CsrAcctInfoRequest : SrvMsgHeader {
    wchar_t   csrName[kMaxAccountNameLength];
};

struct Srv2Db_FetchInviterInfo : SrvMsgHeader {
    plUUID  inviteUuid;
};


/*****************************************************************************
*
*   Db2Srv packets
*
***/

struct Db2Srv_AccountExistsReply : SrvMsgHeader {
    uint8_t            exists;
};

struct Db2Srv_AccountCreateReply : SrvMsgHeader {
    plUUID          accountUuid;
};

struct Db2Srv_AccountCreateFromKeyReply : SrvMsgHeader {
    plUUID          accountUuid;
    plUUID          activationKey;
};

struct Db2Srv_AccountLoginReply : SrvMsgHeader {
    plUUID          accountUuid;
    uint32_t           accountFlags;
    uint32_t           billingType;
    ShaDigest       namePassHash;
};

struct Db2Srv_VaultNodeCreateReply : SrvMsgHeader {
    uint32_t           nodeId;
};

struct Db2Srv_VaultNodeFetchReply : SrvMsgHeader {
    uint32_t           nodeBytes;
    uint8_t            buffer[1];
    // no more fields after var length alloc
};

struct Db2Srv_VaultNodeFindReply : SrvMsgHeader {
    // out: ids of matching nodes
    uint32_t           nodeIdCount;
    uint32_t           nodeIds[1]; // [nodeIdCount], actually
    // no more fields after var length alloc
};

struct Db2Srv_VaultNodeRefs : SrvMsgHeader {
    uint32_t           refCount;
    NetVaultNodeRef refs[1];
    // no more fields after var length alloc
};

struct Db2Srv_SetAgeSequenceNumReply : SrvMsgHeader {
    uint32_t       sequenceNum;
};

struct Db2Srv_FetchInviterInfoReply : SrvMsgHeader {
    plUUID      hoodInstance;
};

struct Db2Srv_StateObjectFetched : SrvMsgHeader {
    uint32_t       buildId;
    plUUID      ownerId;
    wchar_t       objectName[kMaxStateObjectName];
    uint32_t       objectDataBytes;
    uint8_t        objectData[1];
    // no more fields after var length alloc
};

struct Db2Srv_NotifyVaultNodeChanged : SrvMsgHeader {
    uint32_t           nodeId;
    plUUID          revId;
    uint32_t           notifyIdCount;
    uint32_t           notifyIds[1];
};

struct Db2Srv_NotifyVaultNodeAdded : SrvMsgHeader {
    NetVaultNodeRef ref;
    uint32_t           notifyIdCount;
    uint32_t           notifyIds[1];
};

struct Db2Srv_NotifyVaultNodeRemoved : SrvMsgHeader {
    uint32_t           parentId;
    uint32_t           childId;
    uint32_t           notifyIdCount;
    uint32_t           notifyIds[1];
};

struct Db2Srv_NotifyVaultNodeDeleted : SrvMsgHeader {
    uint32_t           nodeId;
    uint32_t           notifyIdCount;
    uint32_t           notifyIds[1];
};

struct Db2Srv_PublicAgeInfoIds : SrvMsgHeader {
    uint32_t           idCount;
    uint32_t           ids[1]; // [idCount], actually
    // no more fields after var length alloc
};

struct Db2Srv_ScoreCreateReply : SrvMsgHeader {
    uint32_t           scoreId;
    uint32_t           createdTime;
};

struct Db2Srv_ScoreDeleteReply : SrvMsgHeader {
    uint32_t           ownerId;
    wchar_t           gameName[kMaxGameScoreNameLength];
};

struct Db2Srv_ScoreFindScoreIdsReply : SrvMsgHeader {
    uint32_t           idCount;
    uint32_t           scoreIds[1];    // [idCount], actually
    // no more fields after var length alloc
};

struct Db2Srv_ScoreFetchScoresReply : SrvMsgHeader {
    uint32_t           scoreCount;
    uint32_t           byteCount;
    uint8_t            buffer[1];  // [byteCount], actually
    // no more fields after var length alloc
};

struct Db2Srv_ScoreGetRanksReply : SrvMsgHeader {
    uint32_t           rankCount;
    uint32_t           byteCount;
    uint8_t            buffer[1];  // [byteCount], actually
    // no more fields after var length alloc
};

struct Db2Srv_CsrAcctInfoReply : SrvMsgHeader {
    plUUID          csrId;
    uint32_t           csrFlags;
    ShaDigest       namePassHash;
};


//============================================================================
// END PACKED DATA STRUCTURES
//============================================================================
#pragma pack(pop)


/*****************************************************************************
*
*   Srv2Db functions
*
***/

bool Srv2DbValidateConnect (
    AsyncNotifySocketListen *   listen,
    Srv2Db_ConnData *           connectPtr
);
