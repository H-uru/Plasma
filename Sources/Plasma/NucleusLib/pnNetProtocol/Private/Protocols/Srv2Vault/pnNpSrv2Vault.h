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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/Protocols/Srv2Vault/pnNpSrv2Vault.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PRIVATE_PROTOCOLS_SRV2VAULT_PNNPSRV2VAULT_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/Protocols/Srv2Vault/pnNpSrv2Vault.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PRIVATE_PROTOCOLS_SRV2VAULT_PNNPSRV2VAULT_H


// kNetProtocolSrv2Db messages
// Because SrvVault must remain compatible with older auth builds, these message ids
// must not change unless all front-end servers are synchronously replaced.
enum {
    // Player creation
    kSrv2Vault_PlayerCreateRequest      = 0,
    kSrv2Vault_PlayerDeleteRequest      = 1,
    kSrv2Vault_UpgradeVisitorRequest    = 2,
    kSrv2Vault_ChangePlayerNameRequest  = 3,
    
    // Account
    kSrv2Vault_AccountLoginRequest      = 20,
    kSrv2Vault_AccountLogout            = 21,
    
    // NodeRefs
    kSrv2Vault_FetchChildNodeRefs       = 40,
    kSrv2Vault_NodeAdd                  = 41,
    kSrv2Vault_NodeRemove               = 42,
    kSrv2Vault_NodeAdd2                 = 43,
    kSrv2Vault_NodeRemove2              = 44,
    
    // Nodes
    kSrv2Vault_NodeFetch                = 60,
    kSrv2Vault_CreateNodeRequest        = 61,
    kSrv2Vault_NodeSave                 = 62,
    kSrv2Vault_DeleteNodeRequest        = 63,
    kSrv2Vault_NodeFindRequest          = 64,
    kSrv2Vault_SendNode                 = 65,
    kSrv2Vault_NodeSave2                = 66,
    
    // Notification targets
    kSrv2Vault_RegisterPlayerVault      = 80,
    kSrv2Vault_UnregisterPlayerVault    = 81,
    kSrv2Vault_RegisterAgeVault         = 82,
    kSrv2Vault_UnregisterAgeVault       = 83,
    kSrv2Vault_AgeInitRequest           = 84,
    
    // Public ages
    kSrv2Vault_GetPublicAgeList         = 100,
    kSrv2Vault_SetAgePublic             = 101,
    kSrv2Vault_CurrentPopulationReply   = 102,
    // MCP Notifications
    kSrv2Vault_AccountOnline            = 140,
    kSrv2Vault_AccountOffline           = 141,
    kSrv2Vault_PlayerOnline             = 142,
    kSrv2Vault_PlayerOffline            = 143,
    kSrv2Vault_AgeOnline                = 144,
    kSrv2Vault_AgeOffline               = 145,
    kSrv2Vault_PlayerJoinedAge          = 146,
    kSrv2Vault_PlayerLeftAge            = 147,
};

enum {
    // Player creation
    kVault2Srv_PlayerCreateReply        = 0,
    kVault2Srv_PlayerDeleteReply        = 1,
    kVault2Srv_UpgradeVisitorReply      = 2,
    kVault2Srv_ChangePlayerNameReply    = 3,
    
    // Account
    kVault2Srv_AccountLoginReply        = 20,
    
    // NodeRefs
    kVault2Srv_NodeRefsFetched          = 40,
    
    // Nodes
    kVault2Srv_NodeFetched              = 60,
    kVault2Srv_NodeCreated              = 61,
    kVault2Srv_NodeFindReply            = 62,
    
    // Notification
    kVault2Srv_NodeChanged              = 80,
    kVault2Srv_NodeAdded                = 81,
    kVault2Srv_NodeRemoved              = 82,
    kVault2Srv_NodeDeleted              = 83,

    // Notification targets
    kVault2Srv_AgeInitReply             = 102,
    
    // Public ages
    kVault2Srv_PublicAgeList            = 120,
    kVault2Srv_CurrentPopulationRequest = 121,

    // AgeSDL
    kVault2Srv_NotifyAgeSDLChanged      = 140,
};


//============================================================================
// BEGIN PACKED DATA STRUCTURES
//============================================================================
#include <PshPack1.h>


/*****************************************************************************
*
*   Srv2Vault connect packet
*
***/

struct Srv2Vault_ConnData {
    uint32_t   dataBytes;
    uint32_t   buildId;
    uint32_t   srvType;
};
struct Srv2Vault_Connect {
    AsyncSocketConnectPacket    hdr;
    Srv2Vault_ConnData          data;
};


/*****************************************************************************
*
*   Srv2Vault packets
*
***/

struct Srv2Vault_PlayerCreateRequest : SrvMsgHeader {
    Uuid    accountUuid;
    wchar_t   playerName[kMaxPlayerNameLength];
    wchar_t   avatarShape[MAX_PATH];
    wchar_t   friendInvite[MAX_PATH];
    uint8_t    explorer;
};

struct Srv2Vault_PlayerDeleteRequest : SrvMsgHeader {
    Uuid    accountId;
    uint32_t   playerId;
};

struct Srv2Vault_UpgradeVisitorRequest : SrvMsgHeader {
    Uuid    accountId;
    uint32_t   playerId;
};

struct Srv2Vault_AccountLoginRequest : SrvMsgHeader {
    Uuid    accountUuid;
};

struct Srv2Vault_AccountLogout : SrvMsgHeader {
    Uuid    accountUuid;
};

struct Srv2Vault_FetchChildNodeRefs : SrvMsgHeader {
    uint32_t   parentId;
    uint32_t   maxDepth;
};

struct Srv2Vault_NodeFetch : SrvMsgHeader {
    uint32_t   nodeId;
};

struct Srv2Vault_CreateNodeRequest : SrvMsgHeader {
    Uuid    accountId;
    uint32_t   creatorId;
    uint32_t   nodeBytes;
    uint8_t    nodeBuffer[1];
};

struct Srv2Vault_DeleteNodeRequest : SrvMsgHeader {
    uint32_t       nodeId;
    unsigned    playerCheckId;
    unsigned    isRequestFromAuth;
};

struct Srv2Vault_NodeSave : SrvMsgHeader {
    uint32_t       nodeId;
    unsigned    playerCheckId;
    unsigned    isRequestFromAuth;
    Uuid        revisionId;
    uint32_t       nodeBytes;
    uint8_t        nodeBuffer[1];
};

struct Srv2Vault_NodeSave2 : SrvMsgHeader {
    uint32_t   nodeId;
    unsigned    playerCheckId;
    unsigned    isRequestFromAuth;
    Uuid    revisionId;
    uint32_t   nodeBytes;
    uint8_t    nodeBuffer[1];
};

struct Srv2Vault_NodeAdd : SrvMsgHeader {
    NetVaultNodeRef ref;
};

struct Srv2Vault_NodeAdd2 : SrvMsgHeader {
    NetVaultNodeRef ref;
};

struct Srv2Vault_NodeRemove : SrvMsgHeader {
    uint32_t   parentId;
    uint32_t   childId;
    unsigned    playerCheckId;
    unsigned    isRequestFromAuth;  
};

struct Srv2Vault_NodeRemove2 : SrvMsgHeader {
    uint32_t   parentId;
    uint32_t   childId;
    unsigned    playerCheckId;
    unsigned    isRequestFromAuth;
};

struct Srv2Vault_NodeFindRequest : SrvMsgHeader {
    // Template node to match
    uint32_t   nodeBytes;
    uint8_t    nodeBuffer[1];  // [nodeBytes], actually
    // no more fields after var length alloc
};

struct Srv2Vault_SendNode : SrvMsgHeader {
    uint32_t   srcPlayerId;    // sender
    uint32_t   srcNodeId;      // sent item
    uint32_t   dstPlayerId;    // recipient
};

struct Srv2Vault_RegisterPlayerVault : SrvMsgHeader {
    Uuid    accountId;
    uint32_t   playerId;
};

struct Srv2Vault_UnregisterPlayerVault : SrvMsgHeader {
    Uuid    accountId;
};

struct Srv2Vault_RegisterAgeVault : SrvMsgHeader {
    Uuid    accountId;
    uint32_t   ageId;  // age's vault node id
};

struct Srv2Vault_UnregisterAgeVault : SrvMsgHeader {
    Uuid    accountId;
};

struct Srv2Vault_AgeInitRequest : SrvMsgHeader {
    Uuid    accountId;
    uint32_t   playerId;
    Uuid    ageInstId;
    Uuid    parentAgeInstId;
    uint32_t   ageLanguage;
    uint32_t   ageSequenceNumber;
// packed fields:
    // wchar_t ageFilename[]
    // wchar_t ageInstName[]
    // wchar_t ageUserName[]
    // wchar_t ageDesc[]
};

struct Srv2Vault_GetPublicAgeList : SrvMsgHeader {
    wchar_t   ageName[kMaxAgeNameLength];
};

struct Srv2Vault_SetAgePublic : SrvMsgHeader {
    uint32_t   playerId;
    uint32_t   ageInfoId;
    uint8_t    publicOrNot;
};

struct Srv2Vault_CurrentPopulationReply : SrvMsgHeader {
    uint32_t           ageCount;
    unsigned        agePopulations[1];  // [ageCount], actually
    // no more fields after var length alloc
};

struct Srv2Vault_ChangePlayerNameRequest : SrvMsgHeader {
    Uuid    accountId;
    uint32_t   playerId;
    wchar_t   newName[kMaxPlayerNameLength];
};

struct Srv2Vault_AccountOnline : SrvMsgHeader {
    Uuid    acctId;
    uint32_t   buildId;
    uint32_t   authNode;
};

struct Srv2Vault_AccountOffline : SrvMsgHeader {
    Uuid    acctId;
    uint32_t   buildId;
};

struct Srv2Vault_PlayerOnline : SrvMsgHeader {
    Uuid    acctId;
    uint32_t   buildId;
    uint32_t   playerId;
};

struct Srv2Vault_PlayerOffline : SrvMsgHeader {
    uint32_t   playerId;
    uint32_t   buildId;
};

struct Srv2Vault_AgeOnline : SrvMsgHeader {
    Uuid    ageInstId;
    uint32_t   buildId;
    uint32_t   gameNode;
};

struct Srv2Vault_AgeOffline : SrvMsgHeader {
    Uuid    ageInstId;
    uint32_t   buildId;
};

struct Srv2Vault_PlayerJoinedAge : SrvMsgHeader {
    uint32_t   playerId;
    Uuid    ageInstId;
    uint32_t   buildId;
};

struct Srv2Vault_PlayerLeftAge : SrvMsgHeader {
    uint32_t   playerId;
    uint32_t   buildId;
};



/*****************************************************************************
*
*   Vault2Srv packets
*
***/

struct Vault2Srv_PlayerCreateReply : SrvMsgHeader {
    uint32_t   playerId;
};

struct Vault2Srv_AccountLoginReply : SrvMsgHeader {
    uint32_t           playerInfoCount;
    SrvPlayerInfo   playerInfos[1];
};

struct Vault2Srv_NodeRefsFetched : SrvMsgHeader {
    uint32_t           refCount;
    NetVaultNodeRef refs[1];
};

struct Vault2Srv_NodeFetched : SrvMsgHeader {
    uint32_t           nodeBytes;
    uint8_t            nodeBuffer[1];
};

struct Vault2Srv_NodeCreated : SrvMsgHeader {
    uint32_t           nodeId;
};

struct Vault2Srv_NodeChanged : SrvMsgHeader {
    uint32_t           nodeId;
    Uuid            revisionId;
    Uuid            accountId;  // the notify target
};

struct Vault2Srv_NodeAdded : SrvMsgHeader {
    NetVaultNodeRef ref;
    Uuid            accountId;  // the notify target
};

struct Vault2Srv_NodeRemoved : SrvMsgHeader {
    uint32_t           parentId;
    uint32_t           childId;
    Uuid            accountId;  // the notify target
};

struct Vault2Srv_NodeDeleted : SrvMsgHeader {
    uint32_t           nodeId;
    Uuid            accountId;  // the notify target
};

struct Vault2Srv_NodeFindReply : SrvMsgHeader {
    // out: ids of matching nodes
    uint32_t           nodeIdCount;
    uint32_t           nodeIds[1]; // [nodeIdCount], actually
    // no more fields after var length alloc
};

struct Vault2Srv_AgeInitReply : SrvMsgHeader {
    uint32_t           ageNodeId;
    uint32_t           ageInfoNodeId;
    Uuid            accountId;  // the requestor
};

struct Vault2Srv_PublicAgeList : SrvMsgHeader {
    uint32_t           ageCount;
    NetAgeInfo      ages[1];    // [ageCount], actually
    // no more fields after var length alloc
};

struct Vault2Srv_NotifyAgeSDLChanged : SrvMsgHeader {
    wchar_t       ageName[kMaxAgeNameLength];
    Uuid        ageInstId;
};

struct Vault2Srv_CurrentPopulationRequest : SrvMsgHeader {
    uint32_t       ageCount;
    Uuid        ageInstIds[1];  // [ageCount], actually
    // no more fields after var length alloc
};

//============================================================================
// END PACKED DATA STRUCTURES
//============================================================================
#include <PopPack.h>


/*****************************************************************************
*
*   Srv2Vault functions
*
***/

bool Srv2VaultValidateConnect (
    AsyncNotifySocketListen *   listen,
    Srv2Vault_ConnData *        connectPtr
);
