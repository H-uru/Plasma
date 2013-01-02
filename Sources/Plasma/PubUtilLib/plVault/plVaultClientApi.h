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
*   $/Plasma20/Sources/Plasma/PubUtilLib/plVault/plVaultClientApi.h
*   
***/

#ifdef CLIENT

#ifdef PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLVAULT_PLVAULTCLIENTAPI_H
#error "Header $/Plasma20/Sources/Plasma/PubUtilLib/plVault/plVaultClientApi.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLVAULT_PLVAULTCLIENTAPI_H


/*****************************************************************************
*
*   VaultCallback
*
***/

struct RelVaultNode;
class plUUID;

struct VaultCallback {
    struct IVaultCallback * internal;
    
    virtual void AddedChildNode (
        RelVaultNode *  parent,
        RelVaultNode *  child
    ) = 0;

    virtual void RemovingChildNode (
        RelVaultNode *  parent,
        RelVaultNode *  child
    ) = 0;

    virtual void ChangedNode (
        RelVaultNode * changedNode
    ) = 0;
};

void VaultRegisterCallback (VaultCallback * cb);
void VaultUnregisterCallback (VaultCallback * cb);


/*****************************************************************************
*
*   RelVaultNode
*   A "relational" vault node (contains child and parent node linkage)
*
*   NOTE:   This API is not thread-safe and therefore not suitable for use
*           in server applications.
*
***/
struct RelVaultNode : NetVaultNode {
    struct IRelVaultNode *  state;

    RelVaultNode ();
    ~RelVaultNode ();
    
    bool IsParentOf (unsigned nodeId, unsigned maxDepth);
    bool IsChildOf (unsigned nodeId, unsigned maxDepth);
    
    void GetRootIds (ARRAY(unsigned) * nodeIds);
    
    unsigned RemoveChildNodes (unsigned maxDepth);  // returns # of nodes removed

    void GetChildNodeIds (
        ARRAY(unsigned) *   nodeIds,
        unsigned            maxDepth
    );
    void GetParentNodeIds (
        ARRAY(unsigned) *   nodeIds,
        unsigned            maxDepth
    );
    
    void GetMatchingChildNodeIds (
        NetVaultNode *      templateNode,
        ARRAY(unsigned) *   nodeIds,
        unsigned            maxDepth
    );
    void GetMatchingParentNodeIds (
        NetVaultNode *      templateNode,
        ARRAY(unsigned) *   nodeIds,
        unsigned            maxDepth
    );

    // returns first matching node found    
    RelVaultNode * GetParentNodeIncRef (
        NetVaultNode *      templateNode,
        unsigned            maxDepth
    );
    RelVaultNode * GetChildNodeIncRef (
        NetVaultNode *      templateNode,
        unsigned            maxDepth
    );
    RelVaultNode * GetChildNodeIncRef (
        unsigned            nodeType,
        unsigned            maxDepth
    );
    RelVaultNode * GetChildFolderNodeIncRef (
        unsigned            folderType,
        unsigned            maxDepth
    );
    RelVaultNode * GetChildPlayerInfoListNodeIncRef (
        unsigned            folderType,
        unsigned            maxDepth
    );
    RelVaultNode * GetChildAgeInfoListNodeIncRef (
        unsigned            folderType,
        unsigned            maxDepth
    );

    // returns all matching nodes found 
    void GetChildNodesIncRef (
        unsigned                maxDepth,
        ARRAY(RelVaultNode*) *  nodes
    );
    void GetChildNodesIncRef (
        NetVaultNode *          templateNode,
        unsigned                maxDepth,
        ARRAY(RelVaultNode*) *  nodes
    );
    void GetChildNodesIncRef (
        unsigned                nodeType,
        unsigned                maxDepth,
        ARRAY(RelVaultNode*) *  nodes
    );
    void GetChildFolderNodesIncRef (
        unsigned                folderType,
        unsigned                maxDepth,
        ARRAY(RelVaultNode*) *  nodes
    );
    
    unsigned GetRefOwnerId (unsigned parentId);

    bool BeenSeen (unsigned parentId) const;
    void SetSeen (unsigned parentId, bool seen);
    
    // logging
    void Print (const wchar_t tag[], FStateDump dumpProc, unsigned level);
    void PrintTree (FStateDump dumpProc, unsigned level);
    
    // AgeInfoNode-specific (and it checks!)
    RelVaultNode * GetParentAgeLinkIncRef ();
};


/*****************************************************************************
*
*   Vault Initialize
*
***/

void VaultInitialize ();
void VaultDestroy ();
void VaultUpdate ();


/*****************************************************************************
*
*   Generic vault access
*
***/

RelVaultNode * VaultGetNodeIncRef (
    unsigned    nodeId
);
RelVaultNode * VaultGetNodeIncRef (
    NetVaultNode *  templateNode
);
// VaultAddChildNode will download the child node if necessary
// the parent exists locally before making the callback.
typedef void (*FVaultAddChildNodeCallback)(
    ENetError       result,
    void *          param
);
void VaultAddChildNode (
    unsigned                    parentId,
    unsigned                    childId,
    unsigned                    ownerId,
    FVaultAddChildNodeCallback  callback,   // optional
    void *                      param       // optional
);
void VaultAddChildNodeAndWait (
    unsigned                    parentId,
    unsigned                    childId,
    unsigned                    ownerId
);
typedef void (*FVaultRemoveChildNodeCallback)(
    ENetError       result,
    void *          param
);
void VaultRemoveChildNode (
    unsigned                        parentId,
    unsigned                        childId,
    FVaultRemoveChildNodeCallback   callback,
    void *                          param
);
void VaultSetNodeSeen (
    unsigned    nodeId,
    bool        seen
);
void VaultDeleteNode (
    unsigned    nodeId
);
void VaultPublishNode (
    unsigned        nodeId,
    const wchar_t     deviceName[]
);
void VaultSendNode (
    RelVaultNode*   srcNode,
    unsigned        dstPlayerId
);

typedef void (*FVaultCreateNodeCallback)(
    ENetError       result,
    void *          state,
    void *          param,
    RelVaultNode *  node
);
void VaultCreateNode (          // non-blocking
    plVault::NodeTypes          nodeType,
    FVaultCreateNodeCallback    callback,
    void *                      state,
    void *                      param
);
void VaultCreateNode (          // non-blocking
    NetVaultNode *              templateNode,
    FVaultCreateNodeCallback    callback,
    void *                      state,
    void *                      param
);
RelVaultNode * VaultCreateNodeAndWaitIncRef (   // block until completion. returns node. nil --> failure
    plVault::NodeTypes          nodeType,
    ENetError *                 result
);
RelVaultNode * VaultCreateNodeAndWaitIncRef (   // block until completion. returns node. nil --> failure
    NetVaultNode *              templateNode,
    ENetError *                 result
);
void VaultForceSaveNodeAndWait (
    NetVaultNode *      node
);

typedef void (*FVaultFindNodeCallback)(
    ENetError           result,
    void *              param,
    unsigned            nodeIdCount,
    const unsigned      nodeIds[]
);
void VaultFindNodes (
    NetVaultNode *          templateNode,
    FVaultFindNodeCallback  callback,
    void *                  param
);
void VaultFindNodesAndWait (
    NetVaultNode *          templateNode,
    ARRAY(unsigned) *       nodeIds
);
void VaultLocalFindNodes (
    NetVaultNode *          templateNode,
    ARRAY(unsigned) *       nodeIds
);
void VaultFetchNodesAndWait (   // Use VaultGetNodeIncRef to access the fetched nodes
    const unsigned          nodeIds[],
    unsigned                count,
    bool                    force = false
);
typedef void (*FVaultInitAgeCallback)(
    ENetError       result,
    void *          state,
    void *          param,
    unsigned        ageVaultId,
    unsigned        ageInfoVaultId
);
void VaultInitAge (
    const class plAgeInfoStruct *   info,
    const plUUID                    parentAgeInstId,
    FVaultInitAgeCallback           callback,
    void *                          state,
    void *                          param
);


/*****************************************************************************
*
*   Player Vault Access
*
***/

unsigned        VaultGetPlayerId ();
RelVaultNode *  VaultGetPlayerNodeIncRef ();
RelVaultNode *  VaultGetPlayerInfoNodeIncRef ();
RelVaultNode *  VaultGetAvatarOutfitFolderIncRef ();
RelVaultNode *  VaultGetAvatarClosetFolderIncRef ();
bool            VaultGetLinkToMyNeighborhood (plAgeLinkStruct * link);
bool            VaultGetLinkToMyPersonalAge (plAgeLinkStruct * link);
bool            VaultGetLinkToCity (plAgeLinkStruct * link);
RelVaultNode *  VaultGetAgesIOwnFolderIncRef ();
RelVaultNode *  VaultGetAgesICanVisitFolderIncRef ();
RelVaultNode *  VaultGetPlayerInboxFolderIncRef ();
RelVaultNode *  VaultGetOwnedAgeLinkIncRef (const plAgeInfoStruct * info);
RelVaultNode *  VaultGetOwnedAgeInfoIncRef (const plAgeInfoStruct * info);
bool            VaultGetOwnedAgeLink (const plAgeInfoStruct * info, plAgeLinkStruct * link);
bool            VaultAddOwnedAgeSpawnPoint (const plUUID& ageInstId, const plSpawnPointInfo & spawnPt);
bool            VaultSetOwnedAgePublicAndWait (const plAgeInfoStruct * info, bool publicOrNot);
RelVaultNode *  VaultGetVisitAgeLinkIncRef (const plAgeInfoStruct * info);
bool            VaultGetVisitAgeLink (const plAgeInfoStruct * info, class plAgeLinkStruct * link);
bool            VaultRegisterOwnedAgeAndWait (const plAgeLinkStruct * link);
void            VaultRegisterOwnedAge(const plAgeLinkStruct* link);
bool            VaultRegisterVisitAgeAndWait (const plAgeLinkStruct * link);
void            VaultRegisterVisitAge (const plAgeLinkStruct* link);
bool            VaultUnregisterOwnedAgeAndWait (const plAgeInfoStruct * info);
bool            VaultUnregisterVisitAgeAndWait (const plAgeInfoStruct * info);
RelVaultNode *  VaultFindChronicleEntryIncRef (const wchar_t entryName[], int entryType = -1);
bool            VaultHasChronicleEntry (const wchar_t entryName[], int entryType = -1);
// if entry of same name and type already exists, value is updated
void            VaultAddChronicleEntryAndWait (
    const wchar_t entryName[],
    int         entryType,
    const wchar_t entryValue[]
);
bool        VaultAmIgnoringPlayer (unsigned playerId);
unsigned    VaultGetKILevel ();
bool        VaultGetCCRStatus ();               // true=online, false=away
bool        VaultSetCCRStatus (bool online);    // true=online, false=away
void        VaultDump (const wchar_t tag[], unsigned vaultId, FStateDump dumpProc);
void        VaultDump (const wchar_t tag[], unsigned vaultId);

bool VaultAmInMyPersonalAge ();
bool VaultAmInMyNeighborhoodAge ();
bool VaultAmOwnerOfCurrentAge ();
bool VaultAmCzarOfCurrentAge ();
bool VaultAmOwnerOfAge (const plUUID& ageInstId);
bool VaultAmCzarOfAge (const plUUID& ageInstId);
bool VaultRegisterMTStationAndWait (
    const wchar_t stationName[],
    const wchar_t linkBackSpawnPtObjName[]
);
void VaultProcessPlayerInbox ();


/*****************************************************************************
*
*   Age Vault Access
*
***/

#define DEFAULT_DEVICE_INBOX L"DevInbox"

RelVaultNode * VaultGetAgeNodeIncRef ();
RelVaultNode * VaultGetAgeInfoNodeIncRef ();
RelVaultNode * VaultGetAgeChronicleFolderIncRef ();
RelVaultNode * VaultGetAgeDevicesFolderIncRef ();
RelVaultNode * VaultGetAgeSubAgesFolderIncRef ();
RelVaultNode * VaultGetAgeChildAgesFolderIncRef ();
RelVaultNode * VaultGetAgeAgeOwnersFolderIncRef ();
RelVaultNode * VaultGetAgeCanVisitFolderIncRef ();
RelVaultNode * VaultGetAgePeopleIKnowAboutFolderIncRef ();
RelVaultNode * VaultGetAgePublicAgesFolderIncRef ();
RelVaultNode * VaultAgeGetBookshelfFolderIncRef ();
RelVaultNode * VaultFindAgeSubAgeLinkIncRef (const plAgeInfoStruct * info);
RelVaultNode * VaultFindAgeChildAgeLinkIncRef (const plAgeInfoStruct * info);
RelVaultNode * VaultFindAgeChronicleEntryIncRef (const wchar_t entryName[], int entryType = -1);
// if entry of same name and type already exists, value is updated
void           VaultAddAgeChronicleEntry (
    const wchar_t entryName[],
    int         entryType,
    const wchar_t entryValue[]
);
RelVaultNode * VaultAgeAddDeviceAndWaitIncRef (const wchar_t deviceName[]);   // blocks until completion
void VaultAgeRemoveDevice (const wchar_t deviceName[]);
bool VaultAgeHasDevice (const wchar_t deviceName[]);
RelVaultNode * VaultAgeGetDeviceIncRef (const wchar_t deviceName[]);
RelVaultNode * VaultAgeSetDeviceInboxAndWaitIncRef (const wchar_t deviceName[], const wchar_t inboxName[]); // blocks until completion
RelVaultNode * VaultAgeGetDeviceInboxIncRef (const wchar_t deviceName[]);
void VaultClearDeviceInboxMap ();

bool VaultAgeGetAgeSDL (class plStateDataRecord * out);
void VaultAgeUpdateAgeSDL (const class plStateDataRecord * rec);

unsigned VaultAgeGetAgeTime ();

RelVaultNode * VaultGetSubAgeLinkIncRef (const plAgeInfoStruct * info);
bool VaultAgeGetSubAgeLink (
    const plAgeInfoStruct * info,
    plAgeLinkStruct *       link
);
bool VaultAgeFindOrCreateSubAgeLinkAndWait (
    const plAgeInfoStruct * info,
    plAgeLinkStruct *       link,
    const plUUID&           parentAgeInstId
);
bool VaultAgeFindOrCreateSubAgeLink(const plAgeInfoStruct* info, plAgeLinkStruct* link, const plUUID& arentUuid);
bool VaultAgeFindOrCreateChildAgeLinkAndWait (
    const wchar_t             parentAgeName[],    // nil --> current age, non-nil --> owned age by given name
    const plAgeInfoStruct * info,
    plAgeLinkStruct *       link
);
uint8_t VaultAgeFindOrCreateChildAgeLink(const wchar_t parentAgeName[], const plAgeInfoStruct* info, plAgeLinkStruct* link);



/*****************************************************************************
*
*   CCR Vault Access
*
***/

void VaultCCRDumpPlayers();



/*****************************************************************************
*
*   Vault download
*
***/

typedef void (*FVaultDownloadCallback)(
    ENetError                   result,
    void *                      param
);
typedef void (*FVaultProgressCallback)(
    unsigned                    total,
    unsigned                    curr,
    void *                      param
);

void VaultDownload (
    const wchar_t                 tag[],
    unsigned                    vaultId,
    FVaultDownloadCallback      callback,
    void *                      cbParam,
    FVaultProgressCallback      progressCallback,
    void *                      cbProgressParam
);
void VaultDownloadAndWait (
    const wchar_t                 tag[],
    unsigned                    vaultId,
    FVaultProgressCallback      progressCallback,
    void *                      cbProgressParam
);

void VaultCull (
    unsigned                    vaultId
);

/*****************************************************************************
*
*   Vault global node handling
*
***/

RelVaultNode * VaultGetSystemNodeIncRef ();
RelVaultNode * VaultGetGlobalInboxIncRef ();

#endif // def CLIENT
