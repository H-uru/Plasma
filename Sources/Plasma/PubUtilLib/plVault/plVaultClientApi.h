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

#ifdef PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLVAULT_PLVAULTCLIENTAPI_H
#error "Header $/Plasma20/Sources/Plasma/PubUtilLib/plVault/plVaultClientApi.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLVAULT_PLVAULTCLIENTAPI_H

#include <list>

/*****************************************************************************
*
*   VaultCallback
*
***/

struct RelVaultNode;
class plUUID;

struct VaultCallback {
    virtual ~VaultCallback() { }

    virtual void AddedChildNode (
        const hsRef<RelVaultNode>& parent,
        const hsRef<RelVaultNode>& child
    ) = 0;

    virtual void RemovingChildNode (
        const hsRef<RelVaultNode>& parent,
        const hsRef<RelVaultNode>& child
    ) = 0;

    virtual void ChangedNode (
        const hsRef<RelVaultNode>& changedNode
    ) = 0;
};

void VaultRegisterCallback (VaultCallback * cb);
void VaultUnregisterCallback (VaultCallback * cb);

void VaultSuppressCallbacks();
void VaultEnableCallbacks();

class VaultCallbackSuppressor
{
public:
    VaultCallbackSuppressor() { VaultSuppressCallbacks(); }
    VaultCallbackSuppressor(const VaultCallbackSuppressor&) = delete;
    VaultCallbackSuppressor(VaultCallbackSuppressor&&) = delete;
    ~VaultCallbackSuppressor() { VaultEnableCallbacks(); }
};

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
    typedef std::list<hsRef<RelVaultNode>> RefList;

    struct IRelVaultNode *  state;

    RelVaultNode ();
    ~RelVaultNode ();
    
    bool IsParentOf (unsigned nodeId, unsigned maxDepth);
    bool IsChildOf (unsigned nodeId, unsigned maxDepth);
    
    void GetRootIds (std::vector<unsigned> * nodeIds);
    
    unsigned RemoveChildNodes (unsigned maxDepth);  // returns # of nodes removed

    void GetChildNodeIds (
        std::vector<unsigned> * nodeIds,
        unsigned            maxDepth
    );
    void GetParentNodeIds (
        std::vector<unsigned> * nodeIds,
        unsigned            maxDepth
    );

    // returns first matching node found
    hsRef<RelVaultNode> GetParentNode (
        hsWeakRef<NetVaultNode> templateNode,
        unsigned                maxDepth
    );
    hsRef<RelVaultNode> GetChildNode (
        hsWeakRef<NetVaultNode> templateNode,
        unsigned                maxDepth
    );
    hsRef<RelVaultNode> GetChildNode (
        unsigned            nodeType,
        unsigned            maxDepth
    );
    hsRef<RelVaultNode> GetChildFolderNode (
        unsigned            folderType,
        unsigned            maxDepth
    );
    hsRef<RelVaultNode> GetChildPlayerInfoListNode (
        unsigned            folderType,
        unsigned            maxDepth
    );
    hsRef<RelVaultNode> GetChildAgeInfoListNode (
        unsigned            folderType,
        unsigned            maxDepth
    );

    // returns all matching nodes found 
    void GetChildNodes (
        unsigned                maxDepth,
        RefList *               nodes
    );
    void GetChildNodes (
        hsWeakRef<NetVaultNode> templateNode,
        unsigned                maxDepth,
        RefList *               nodes
    );
    void GetChildNodes (
        unsigned                nodeType,
        unsigned                maxDepth,
        RefList *               nodes
    );
    void GetChildFolderNodes (
        unsigned                folderType,
        unsigned                maxDepth,
        RefList *               nodes
    );
    
    unsigned GetRefOwnerId (unsigned parentId);

    bool BeenSeen (unsigned parentId) const;
    void SetSeen (unsigned parentId, bool seen);
    
    // logging
    void Print (const ST::string& tag, unsigned level);
    void PrintTree (unsigned level);
    
    // AgeInfoNode-specific (and it checks!)
    hsRef<RelVaultNode> GetParentAgeLink ();
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

hsRef<RelVaultNode> VaultGetNode(unsigned nodeId);
hsRef<RelVaultNode> VaultGetNode(hsWeakRef<NetVaultNode> templateNode);

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
void VaultSendNode (
    hsWeakRef<RelVaultNode> srcNode,
    unsigned                dstPlayerId
);

typedef void (*FVaultCreateNodeCallback)(
    ENetError       result,
    void *          state,
    void *          param,
    hsWeakRef<RelVaultNode> node
);
void VaultCreateNode (          // non-blocking
    plVault::NodeTypes          nodeType,
    FVaultCreateNodeCallback    callback,
    void *                      state,
    void *                      param
);
void VaultCreateNode (          // non-blocking
    hsWeakRef<NetVaultNode>     templateNode,
    FVaultCreateNodeCallback    callback,
    void *                      state,
    void *                      param
);
hsRef<RelVaultNode> VaultCreateNodeAndWait (   // block until completion. returns node. nullptr --> failure
    plVault::NodeTypes          nodeType,
    ENetError *                 result
);
hsRef<RelVaultNode> VaultCreateNodeAndWait (   // block until completion. returns node. nullptr --> failure
    hsWeakRef<NetVaultNode>     templateNode,
    ENetError *                 result
);
void VaultForceSaveNodeAndWait (
    hsWeakRef<NetVaultNode>     node
);

typedef void (*FVaultFindNodeCallback)(
    ENetError           result,
    void *              param,
    unsigned            nodeIdCount,
    const unsigned      nodeIds[]
);
void VaultFindNodes (
    hsWeakRef<NetVaultNode> templateNode,
    FVaultFindNodeCallback  callback,
    void *                  param
);
void VaultFindNodesAndWait (
    hsWeakRef<NetVaultNode> templateNode,
    std::vector<unsigned> * nodeIds
);
void VaultLocalFindNodes (
    hsWeakRef<NetVaultNode> templateNode,
    std::vector<unsigned> * nodeIds
);
void VaultFetchNodesAndWait (   // Use VaultGetNode to access the fetched nodes
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

unsigned            VaultGetPlayerId();
hsRef<RelVaultNode> VaultGetPlayerNode();
hsRef<RelVaultNode> VaultGetPlayerInfoNode();
hsRef<RelVaultNode> VaultGetAvatarOutfitFolder();
hsRef<RelVaultNode> VaultGetAvatarClosetFolder();
bool                VaultGetLinkToMyNeighborhood(plAgeLinkStruct * link);
bool                VaultGetLinkToMyPersonalAge(plAgeLinkStruct * link);
bool                VaultGetLinkToCity(plAgeLinkStruct * link);
hsRef<RelVaultNode> VaultGetAgesIOwnFolder();
hsRef<RelVaultNode> VaultGetAgesICanVisitFolder();
hsRef<RelVaultNode> VaultGetPlayerInboxFolder();
hsRef<RelVaultNode> VaultGetOwnedAgeLink(const plAgeInfoStruct * info);
hsRef<RelVaultNode> VaultGetOwnedAgeInfo(const plAgeInfoStruct * info);
bool                VaultGetOwnedAgeLink(const plAgeInfoStruct * info, plAgeLinkStruct * link);
bool                VaultAddOwnedAgeSpawnPoint(const plUUID& ageInstId, const plSpawnPointInfo & spawnPt);
bool                VaultSetOwnedAgePublic(const plAgeInfoStruct* info, bool publicOrNot);
bool                VaultSetAgePublic(hsWeakRef<NetVaultNode> ageInfoNode, bool publicOrNot);
hsRef<RelVaultNode> VaultGetVisitAgeLink(const plAgeInfoStruct * info);
bool                VaultGetVisitAgeLink(const plAgeInfoStruct * info, class plAgeLinkStruct * link);
bool                VaultRegisterOwnedAgeAndWait(const plAgeLinkStruct * link);
void                VaultRegisterOwnedAge(const plAgeLinkStruct* link);
bool                VaultRegisterVisitAgeAndWait(const plAgeLinkStruct * link);
void                VaultRegisterVisitAge(const plAgeLinkStruct* link);
bool                VaultUnregisterOwnedAge(const plAgeInfoStruct* info);
bool                VaultUnregisterVisitAge(const plAgeInfoStruct* info);
hsRef<RelVaultNode> VaultFindChronicleEntry(const ST::string& entryName, int entryType = -1);
bool                VaultHasChronicleEntry(const ST::string& entryName, int entryType = -1);
// if entry of same name and type already exists, value is updated
void            VaultAddChronicleEntryAndWait (
    const ST::string& entryName,
    int               entryType,
    const ST::string& entryValue
);
bool        VaultAmIgnoringPlayer (unsigned playerId);
unsigned    VaultGetKILevel ();
bool        VaultGetCCRStatus ();               // true=online, false=away
bool        VaultSetCCRStatus (bool online);    // true=online, false=away
void        VaultDump (const ST::string& tag, unsigned vaultId);

bool VaultAmInMyPersonalAge ();
bool VaultAmInMyNeighborhoodAge ();
bool VaultAmOwnerOfCurrentAge ();
bool VaultAmCzarOfCurrentAge ();
bool VaultAmOwnerOfAge (const plUUID& ageInstId);
bool VaultAmCzarOfAge (const plUUID& ageInstId);
bool VaultRegisterMTStation(
    const ST::string& stationName,
    const ST::string& linkBackSpawnPtObjName
);
void VaultProcessPlayerInbox ();


/*****************************************************************************
*
*   Age Vault Access
*
***/

#define DEFAULT_DEVICE_INBOX "DevInbox"

hsRef<RelVaultNode> VaultGetAgeNode();
hsRef<RelVaultNode> VaultGetAgeInfoNode();
hsRef<RelVaultNode> VaultGetAgeChronicleFolder();
hsRef<RelVaultNode> VaultGetAgeDevicesFolder();
hsRef<RelVaultNode> VaultGetAgeSubAgesFolder();
hsRef<RelVaultNode> VaultGetAgeChildAgesFolder();
hsRef<RelVaultNode> VaultGetAgeAgeOwnersFolder();
hsRef<RelVaultNode> VaultGetAgeCanVisitFolder();
hsRef<RelVaultNode> VaultGetAgePeopleIKnowAboutFolder();
hsRef<RelVaultNode> VaultGetAgePublicAgesFolder();
hsRef<RelVaultNode> VaultAgeGetBookshelfFolder();
hsRef<RelVaultNode> VaultFindAgeSubAgeLink(const plAgeInfoStruct * info);
hsRef<RelVaultNode> VaultFindAgeChildAgeLink(const plAgeInfoStruct * info);
hsRef<RelVaultNode> VaultFindAgeChronicleEntry(const ST::string& entryName, int entryType = -1);
// if entry of same name and type already exists, value is updated
void           VaultAddAgeChronicleEntry (
    const ST::string& entryName,
    int               entryType,
    const ST::string& entryValue
);
typedef void (*FVaultAgeAddDeviceCallback)(ENetError result, hsRef<RelVaultNode> device, void* param);
void VaultAgeAddDevice(const ST::string& deviceName, FVaultAgeAddDeviceCallback callback, void* param);
void VaultAgeRemoveDevice (const ST::string& deviceName);
bool VaultAgeHasDevice (const ST::string& deviceName);
hsRef<RelVaultNode> VaultAgeGetDevice(const ST::string& deviceName);
typedef void (*FVaultAgeSetDeviceInboxCallback)(ENetError result, hsRef<RelVaultNode> inbox, void* param);
void VaultAgeSetDeviceInbox(const ST::string& deviceName, const ST::string& inboxName, FVaultAgeSetDeviceInboxCallback callback, void* param);
hsRef<RelVaultNode> VaultAgeGetDeviceInbox(const ST::string& deviceName);
void VaultClearDeviceInboxMap ();

bool VaultAgeGetAgeSDL (class plStateDataRecord * out);
void VaultAgeUpdateAgeSDL (const class plStateDataRecord * rec);

unsigned VaultAgeGetAgeTime ();

hsRef<RelVaultNode> VaultGetSubAgeLink(const plAgeInfoStruct * info);
bool VaultAgeGetSubAgeLink (
    const plAgeInfoStruct * info,
    plAgeLinkStruct *       link
);
bool VaultAgeFindOrCreateSubAgeLink(const plAgeInfoStruct* info, plAgeLinkStruct* link, const plUUID& arentUuid);
enum class plVaultChildAgeLinkResult
{
    kFailed,
    kCreatingNew,
    kFoundExisting,
};
plVaultChildAgeLinkResult VaultAgeFindOrCreateChildAgeLink(const ST::string& parentAgeName, const plAgeInfoStruct* info, plAgeLinkStruct* link);



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
    const ST::string&           tag,
    unsigned                    vaultId,
    FVaultDownloadCallback      callback,
    void *                      cbParam,
    FVaultProgressCallback      progressCallback,
    void *                      cbProgressParam
);
void VaultDownloadNoCallbacks (
    const ST::string&           tag,
    unsigned                    vaultId,
    FVaultDownloadCallback      callback,
    void *                      cbParam,
    FVaultProgressCallback      progressCallback,
    void *                      cbProgressParam
);
void VaultDownloadAndWait (
    const ST::string&           tag,
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

hsRef<RelVaultNode> VaultGetSystemNode();
hsRef<RelVaultNode> VaultGetGlobalInbox();
