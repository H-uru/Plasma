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
    kSrv2Vault_PlayerCreateRequest		= 0,
    kSrv2Vault_PlayerDeleteRequest		= 1,
	kSrv2Vault_UpgradeVisitorRequest	= 2,
	kSrv2Vault_ChangePlayerNameRequest	= 3,
    
    // Account
    kSrv2Vault_AccountLoginRequest		= 20,
    kSrv2Vault_AccountLogout			= 21,
    
    // NodeRefs
    kSrv2Vault_FetchChildNodeRefs		= 40,
    kSrv2Vault_NodeAdd					= 41,
    kSrv2Vault_NodeRemove				= 42,
	kSrv2Vault_NodeAdd2					= 43,
    kSrv2Vault_NodeRemove2				= 44,
    
    // Nodes
    kSrv2Vault_NodeFetch				= 60,
    kSrv2Vault_CreateNodeRequest		= 61,
    kSrv2Vault_NodeSave					= 62,
    kSrv2Vault_DeleteNodeRequest		= 63,
    kSrv2Vault_NodeFindRequest			= 64,
    kSrv2Vault_SendNode					= 65,
	kSrv2Vault_NodeSave2				= 66,
    
    // Notification targets
    kSrv2Vault_RegisterPlayerVault		= 80,
    kSrv2Vault_UnregisterPlayerVault	= 81,
    kSrv2Vault_RegisterAgeVault			= 82,
    kSrv2Vault_UnregisterAgeVault		= 83,
    kSrv2Vault_AgeInitRequest			= 84,
    
    // Public ages
    kSrv2Vault_GetPublicAgeList			= 100,
    kSrv2Vault_SetAgePublic				= 101,
	kSrv2Vault_CurrentPopulationReply	= 102,
	// MCP Notifications
	kSrv2Vault_AccountOnline			= 140,
	kSrv2Vault_AccountOffline			= 141,
	kSrv2Vault_PlayerOnline				= 142,
	kSrv2Vault_PlayerOffline			= 143,
	kSrv2Vault_AgeOnline				= 144,
	kSrv2Vault_AgeOffline				= 145,
	kSrv2Vault_PlayerJoinedAge			= 146,
	kSrv2Vault_PlayerLeftAge			= 147,
};

enum {
	// Player creation
	kVault2Srv_PlayerCreateReply		= 0,
	kVault2Srv_PlayerDeleteReply		= 1,
	kVault2Srv_UpgradeVisitorReply		= 2,
	kVault2Srv_ChangePlayerNameReply	= 3,
	
	// Account
	kVault2Srv_AccountLoginReply		= 20,
	
	// NodeRefs
	kVault2Srv_NodeRefsFetched			= 40,
	
	// Nodes
	kVault2Srv_NodeFetched				= 60,
	kVault2Srv_NodeCreated				= 61,
	kVault2Srv_NodeFindReply			= 62,
	
	// Notification
	kVault2Srv_NodeChanged				= 80,
	kVault2Srv_NodeAdded				= 81,
	kVault2Srv_NodeRemoved				= 82,
	kVault2Srv_NodeDeleted				= 83,

    // Notification targets
    kVault2Srv_AgeInitReply				= 102,
    
    // Public ages
    kVault2Srv_PublicAgeList			= 120,
	kVault2Srv_CurrentPopulationRequest	= 121,

    // AgeSDL
    kVault2Srv_NotifyAgeSDLChanged		= 140,
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
    dword   dataBytes;
    dword   buildId;
    dword   srvType;
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
	Uuid	accountUuid;
	wchar	playerName[kMaxPlayerNameLength];
	wchar	avatarShape[MAX_PATH];
	wchar	friendInvite[MAX_PATH];
	byte	explorer;
};

struct Srv2Vault_PlayerDeleteRequest : SrvMsgHeader {
	Uuid	accountId;
	dword	playerId;
};

struct Srv2Vault_UpgradeVisitorRequest : SrvMsgHeader {
	Uuid	accountId;
	dword	playerId;
};

struct Srv2Vault_AccountLoginRequest : SrvMsgHeader {
	Uuid	accountUuid;
};

struct Srv2Vault_AccountLogout : SrvMsgHeader {
	Uuid	accountUuid;
};

struct Srv2Vault_FetchChildNodeRefs : SrvMsgHeader {
	dword	parentId;
	dword	maxDepth;
};

struct Srv2Vault_NodeFetch : SrvMsgHeader {
	dword	nodeId;
};

struct Srv2Vault_CreateNodeRequest : SrvMsgHeader {
    Uuid    accountId;
	dword	creatorId;
	dword	nodeBytes;
	byte	nodeBuffer[1];
};

struct Srv2Vault_DeleteNodeRequest : SrvMsgHeader {
	dword		nodeId;
	unsigned	playerCheckId;
	unsigned	isRequestFromAuth;
};

struct Srv2Vault_NodeSave : SrvMsgHeader {
	dword		nodeId;
	unsigned	playerCheckId;
	unsigned	isRequestFromAuth;
	Uuid		revisionId;
	dword		nodeBytes;
	byte		nodeBuffer[1];
};

struct Srv2Vault_NodeSave2 : SrvMsgHeader {
	dword	nodeId;
	unsigned	playerCheckId;
	unsigned	isRequestFromAuth;
	Uuid	revisionId;
	dword	nodeBytes;
	byte	nodeBuffer[1];
};

struct Srv2Vault_NodeAdd : SrvMsgHeader {
	NetVaultNodeRef	ref;
};

struct Srv2Vault_NodeAdd2 : SrvMsgHeader {
	NetVaultNodeRef	ref;
};

struct Srv2Vault_NodeRemove : SrvMsgHeader {
	dword	parentId;
	dword	childId;
	unsigned	playerCheckId;
	unsigned	isRequestFromAuth;	
};

struct Srv2Vault_NodeRemove2 : SrvMsgHeader {
	dword	parentId;
	dword	childId;
	unsigned	playerCheckId;
	unsigned	isRequestFromAuth;
};

struct Srv2Vault_NodeFindRequest : SrvMsgHeader {
	// Template node to match
	dword	nodeBytes;
	byte	nodeBuffer[1];	// [nodeBytes], actually
	// no more fields after var length alloc
};

struct Srv2Vault_SendNode : SrvMsgHeader {
	dword	srcPlayerId;	// sender
	dword	srcNodeId;		// sent item
	dword	dstPlayerId;	// recipient
};

struct Srv2Vault_RegisterPlayerVault : SrvMsgHeader {
	Uuid	accountId;
	dword	playerId;
};

struct Srv2Vault_UnregisterPlayerVault : SrvMsgHeader {
	Uuid	accountId;
};

struct Srv2Vault_RegisterAgeVault : SrvMsgHeader {
	Uuid	accountId;
	dword	ageId;	// age's vault node id
};

struct Srv2Vault_UnregisterAgeVault : SrvMsgHeader {
	Uuid	accountId;
};

struct Srv2Vault_AgeInitRequest : SrvMsgHeader {
	Uuid	accountId;
	dword	playerId;
	Uuid	ageInstId;
	Uuid	parentAgeInstId;
	dword	ageLanguage;
	dword	ageSequenceNumber;
// packed fields:
	// wchar ageFilename[]
	// wchar ageInstName[]
	// wchar ageUserName[]
	// wchar ageDesc[]
};

struct Srv2Vault_GetPublicAgeList : SrvMsgHeader {
	wchar	ageName[kMaxAgeNameLength];
};

struct Srv2Vault_SetAgePublic : SrvMsgHeader {
	dword	playerId;
	dword	ageInfoId;
	byte	publicOrNot;
};

struct Srv2Vault_CurrentPopulationReply : SrvMsgHeader {
	dword			ageCount;
	unsigned		agePopulations[1];	// [ageCount], actually
	// no more fields after var length alloc
};

struct Srv2Vault_ChangePlayerNameRequest : SrvMsgHeader {
	Uuid	accountId;
	dword	playerId;
	wchar	newName[kMaxPlayerNameLength];
};

struct Srv2Vault_AccountOnline : SrvMsgHeader {
	Uuid	acctId;
	dword	buildId;
	dword	authNode;
};

struct Srv2Vault_AccountOffline : SrvMsgHeader {
	Uuid	acctId;
	dword	buildId;
};

struct Srv2Vault_PlayerOnline : SrvMsgHeader {
	Uuid	acctId;
	dword	buildId;
	dword	playerId;
};

struct Srv2Vault_PlayerOffline : SrvMsgHeader {
	dword	playerId;
	dword	buildId;
};

struct Srv2Vault_AgeOnline : SrvMsgHeader {
	Uuid	ageInstId;
	dword	buildId;
	dword	gameNode;
};

struct Srv2Vault_AgeOffline : SrvMsgHeader {
	Uuid	ageInstId;
	dword	buildId;
};

struct Srv2Vault_PlayerJoinedAge : SrvMsgHeader {
	dword	playerId;
	Uuid	ageInstId;
	dword	buildId;
};

struct Srv2Vault_PlayerLeftAge : SrvMsgHeader {
	dword	playerId;
	dword	buildId;
};



/*****************************************************************************
*
*   Vault2Srv packets
*
***/

struct Vault2Srv_PlayerCreateReply : SrvMsgHeader {
	dword	playerId;
};

struct Vault2Srv_AccountLoginReply : SrvMsgHeader {
	dword			playerInfoCount;
	SrvPlayerInfo	playerInfos[1];
};

struct Vault2Srv_NodeRefsFetched : SrvMsgHeader {
	dword			refCount;
	NetVaultNodeRef	refs[1];
};

struct Vault2Srv_NodeFetched : SrvMsgHeader {
	dword			nodeBytes;
	byte			nodeBuffer[1];
};

struct Vault2Srv_NodeCreated : SrvMsgHeader {
	dword			nodeId;
};

struct Vault2Srv_NodeChanged : SrvMsgHeader {
	dword			nodeId;
	Uuid			revisionId;
	Uuid			accountId;	// the notify target
};

struct Vault2Srv_NodeAdded : SrvMsgHeader {
	NetVaultNodeRef	ref;
	Uuid			accountId;	// the notify target
};

struct Vault2Srv_NodeRemoved : SrvMsgHeader {
	dword			parentId;
	dword			childId;
	Uuid			accountId;	// the notify target
};

struct Vault2Srv_NodeDeleted : SrvMsgHeader {
	dword			nodeId;
	Uuid			accountId;	// the notify target
};

struct Vault2Srv_NodeFindReply : SrvMsgHeader {
	// out: ids of matching nodes
	dword			nodeIdCount;
	dword			nodeIds[1];	// [nodeIdCount], actually
	// no more fields after var length alloc
};

struct Vault2Srv_AgeInitReply : SrvMsgHeader {
	dword			ageNodeId;
	dword			ageInfoNodeId;
	Uuid			accountId;	// the requestor
};

struct Vault2Srv_PublicAgeList : SrvMsgHeader {
	dword			ageCount;
	NetAgeInfo		ages[1];	// [ageCount], actually
	// no more fields after var length alloc
};

struct Vault2Srv_NotifyAgeSDLChanged : SrvMsgHeader {
	wchar		ageName[kMaxAgeNameLength];
	Uuid		ageInstId;
};

struct Vault2Srv_CurrentPopulationRequest : SrvMsgHeader {
	dword		ageCount;
	Uuid		ageInstIds[1];	// [ageCount], actually
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
