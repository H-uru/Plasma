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
*   $/Plasma20/Sources/Plasma/PubUtilLib/plVault/plVaultClientApi.cpp
*   
***/


#include "Pch.h"


/*****************************************************************************
*
*   Private
*
***/

// A RelVaultNodeLink is stored in an IRelVaultNode's children table.
struct RelVaultNodeLink {
    hsRef<RelVaultNode>         node;
    unsigned                    ownerId;
    bool                        seen;

    RelVaultNodeLink(bool seen, unsigned ownerId, hsRef<RelVaultNode> node)
        : node(std::move(node)), ownerId(ownerId), seen(seen)
    { }
};


struct IRelVaultNode {
    hsWeakRef<RelVaultNode> node;

    std::unordered_map<unsigned, hsRef<RelVaultNode>> parents;
    std::unordered_map<unsigned, RelVaultNodeLink> children;

    IRelVaultNode(hsWeakRef<RelVaultNode> node);
    ~IRelVaultNode ();

    // Unlink our node from all our parent and children
    void UnlinkFromRelatives ();
    
    // Unlink the node from our parent and children lists
    void Unlink(hsWeakRef<RelVaultNode> other);
};


struct VaultCreateNodeTrans {
    FVaultCreateNodeCallback    callback;
    void *                      state;
    void *                      param;

    unsigned                    nodeId;
    hsRef<RelVaultNode>         node;

    VaultCreateNodeTrans ()
        : callback(), state(), param(), nodeId() { }

    VaultCreateNodeTrans (FVaultCreateNodeCallback _callback,
                          void * _state, void * _param)
        : callback(_callback), state(_state), param(_param),
          nodeId() { }

    static void VaultNodeCreated (
        ENetError           result,
        void *              param,
        unsigned            nodeId
    );
    static void VaultNodeFetched (
        ENetError           result,
        void *              param,
        NetVaultNode *      node
    );

    void Complete (ENetError result);
};


struct VaultFindNodeTrans {
    FVaultFindNodeCallback      callback;
    void *                      param;

    VaultFindNodeTrans () : callback(), param() { }

    VaultFindNodeTrans (FVaultFindNodeCallback _callback, void * _param)
        : callback(_callback), param(_param) { }

    static void VaultNodeFound (
        ENetError           result,
        void *              param,
        unsigned            nodeIdCount,
        const unsigned      nodeIds[]
    );
};


struct VaultDownloadTrans {
    FVaultDownloadCallback      callback;
    void *                      cbParam;
    FVaultProgressCallback      progressCallback;
    void *                      cbProgressParam;

    ST::string  tag;
    unsigned    nodeCount;
    unsigned    nodesLeft;
    unsigned    vaultId;
    ENetError   result;

    VaultDownloadTrans ()
        : callback(), cbParam(), progressCallback(), cbProgressParam(),
          nodeCount(), nodesLeft(), vaultId(), result(kNetSuccess)
    { }

    VaultDownloadTrans (const ST::string& _tag, FVaultDownloadCallback _callback,
                        void * _cbParam, FVaultProgressCallback _progressCallback,
                        void * _cbProgressParam, unsigned _vaultId)
        : callback(_callback), cbParam(_cbParam), progressCallback(_progressCallback),
          cbProgressParam(_cbProgressParam), nodeCount(), nodesLeft(),
          vaultId(_vaultId), result(kNetSuccess), tag(_tag)
    { }

    virtual ~VaultDownloadTrans() = default;


    static void VaultNodeFetched (
        ENetError           result,
        void *              param,
        NetVaultNode *      node
    );
    static void VaultNodeRefsFetched (
        ENetError           result,
        void *              param,
        NetVaultNodeRef *   refs,
        unsigned            refCount
    );
};

struct VaultDownloadNoCallbacksTrans : VaultDownloadTrans {
    VaultDownloadNoCallbacksTrans()
        : VaultDownloadTrans()
    {
        VaultSuppressCallbacks();
    }

    VaultDownloadNoCallbacksTrans(const ST::string& _tag, FVaultDownloadCallback _callback,
                                  void* _cbParam, FVaultProgressCallback _progressCallback,
                                  void* _cbProgressParam, unsigned _vaultId)
        : VaultDownloadTrans(_tag, _callback, _cbParam, _progressCallback, _cbProgressParam, _vaultId)
    {
        VaultSuppressCallbacks();
    }

    ~VaultDownloadNoCallbacksTrans()
    {
        VaultEnableCallbacks();
    }
};

struct VaultAgeInitTrans {
    FVaultInitAgeCallback   callback;
    void *                  cbState;
    void *                  cbParam;

    VaultAgeInitTrans()
        : callback(), cbState(), cbParam() { }

    VaultAgeInitTrans(FVaultInitAgeCallback _callback,
                      void * state, void * param)
        : callback(_callback), cbState(state), cbParam(param) { }

    static void AgeInitCallback (
        ENetError       result,
        void *          param,
        unsigned        ageVaultId,
        unsigned        ageInfoVaultId
    );
};

struct AddChildNodeFetchTrans {
    FVaultAddChildNodeCallback  callback;
    void *                      cbParam;
    ENetError                   result;
    std::atomic<long>           opCount;

    AddChildNodeFetchTrans()
        : callback(), cbParam(), result(kNetSuccess), opCount() { }

    AddChildNodeFetchTrans(FVaultAddChildNodeCallback _callback, void * _param)
        : callback(_callback), cbParam(_param), result(kNetSuccess), opCount() { }

    static void VaultNodeFetched (
        ENetError           result,
        void *              param,
        NetVaultNode *      node
    );
    static void VaultNodeRefsFetched (
        ENetError           result,
        void *              param,
        NetVaultNodeRef *   refs,
        unsigned            refCount
    );
};


/*****************************************************************************
*
*   Private data
*
***/

std::unordered_map<unsigned, hsRef<RelVaultNode>> s_nodes;

std::list<VaultCallback*> s_callbacks;

// key: childId, value: parentId
std::unordered_map<unsigned, unsigned> s_notifyAfterDownload;

static std::unordered_map<ST::string, ST::string, ST::hash> s_ageDeviceInboxes;

static std::atomic<int> s_suppressCallbacks;

/*****************************************************************************
*
*   Local functions
*
***/

static void VaultProcessVisitNote(hsWeakRef<RelVaultNode> rvnVisit);
static void VaultProcessUnvisitNote(hsWeakRef<RelVaultNode> rvnUnVisit);

static void VaultNodeFetched (
    ENetError           result,
    void *              param,
    NetVaultNode *      node
);
static void VaultNodeFound (
    ENetError           result,
    void *              param,
    unsigned            nodeIdCount,
    const unsigned      nodeIds[]
);

//============================================================================
static void VaultNodeAddedDownloadCallback(ENetError result, void * param) {
    unsigned childId = (unsigned)((uintptr_t)param);

    auto it = s_notifyAfterDownload.find(childId);

    if (it != s_notifyAfterDownload.end()) {
        unsigned parentId = it->second;
        if (IS_NET_SUCCESS(result)) {
            auto parentIt = s_nodes.find(parentId);
            auto childIt = s_nodes.find(childId);

            if (parentIt != s_nodes.end() && childIt != s_nodes.end()) {
                const hsRef<RelVaultNode>& parentNode = parentIt->second;
                const hsRef<RelVaultNode>& childNode = childIt->second;
                if (childNode->GetNodeType() == plVault::kNodeType_TextNote) {
                    VaultTextNoteNode textNote(childNode);
                    if (textNote.GetNoteType() == plVault::kNoteType_Visit) {
                        VaultProcessVisitNote(childNode);
                    } else if (textNote.GetNoteType() == plVault::kNoteType_UnVisit) {
                        VaultProcessUnvisitNote(childNode);
                    }
                }

                if (s_suppressCallbacks == 0) {
                    for (auto cb : s_callbacks) {
                        cb->AddedChildNode(parentNode, childNode);
                    }
                }
            }
        }
    }
}

//============================================================================
// Returns ids of nodes that had to be created (so we can fetch them)
static void BuildNodeTree (
    const NetVaultNodeRef   refs[],
    unsigned                refCount,
    std::vector<unsigned> * newNodeIds,
    std::vector<unsigned> * existingNodeIds,
    bool                    notifyNow = true
) {
    for (unsigned i = 0; i < refCount; ++i) {
        // Find/Create global links
        auto parentIt = s_nodes.find(refs[i].parentId);
        if (parentIt == s_nodes.end()) {
            newNodeIds->emplace_back(refs[i].parentId);
            hsRef<RelVaultNode> newParentNode(new RelVaultNode(), hsStealRef);
            newParentNode->SetNodeId_NoDirty(refs[i].parentId);
            parentIt = s_nodes.emplace(refs[i].parentId, std::move(newParentNode)).first;
        } else {
            existingNodeIds->emplace_back(refs[i].parentId);
        }
        const hsRef<RelVaultNode>& parentNode = parentIt->second;

        auto childIt = s_nodes.find(refs[i].childId);
        if (childIt == s_nodes.end()) {
            newNodeIds->emplace_back(refs[i].childId);
            hsRef<RelVaultNode> newChildNode(new RelVaultNode(), hsStealRef);
            newChildNode->SetNodeId_NoDirty(refs[i].childId);
            childIt = s_nodes.emplace(refs[i].childId, std::move(newChildNode)).first;
        } else {
            existingNodeIds->emplace_back(refs[i].childId);
        }
        const hsRef<RelVaultNode>& childNode = childIt->second;

        bool isImmediateParent = parentNode->IsParentOf(refs[i].childId, 1);
        bool isImmediateChild = childNode->IsChildOf(refs[i].parentId, 1);
            
        if (!isImmediateParent) {
            // Add parent to child's parents table
            childNode->state->parents.emplace(parentNode->GetNodeId(), parentNode);
            LogMsg(kLogDebug, "Added relationship: p:{},c:{}", refs[i].parentId, refs[i].childId);
        }
        
        if (!isImmediateChild) {
            // Add child to parent's children table
            parentNode->state->children.emplace(childNode->GetNodeId(), RelVaultNodeLink(refs[i].seen, refs[i].ownerId, childNode));

            if (notifyNow || childNode->GetNodeType() != 0) {
                // We made a new link, so make the callbacks
                if (s_suppressCallbacks == 0) {
                    for (auto cb : s_callbacks) {
                        cb->AddedChildNode(parentNode, childNode);
                    }
                }
            }
            else {
                s_notifyAfterDownload.emplace(childNode->GetNodeId(), parentNode->GetNodeId());
            }
        }
    }
}

//============================================================================
static void InitFetchedNode(hsWeakRef<RelVaultNode> rvn) {

    switch (rvn->GetNodeType()) {
        case plVault::kNodeType_SDL: {
            VaultSDLNode access(rvn);
            if (access.GetSDLData().empty())
                access.InitStateDataRecord(access.GetSDLName());
        }
        break;
    }
}

//============================================================================
static void FetchRefOwners (
    NetVaultNodeRef *           refs,
    unsigned                    refCount
) {
    std::vector<unsigned> ownerIds;
    for (unsigned i = 0; i < refCount; ++i) {
        if (unsigned ownerId = refs[i].ownerId)
            ownerIds.emplace_back(ownerId);
    }
    std::sort(ownerIds.begin(), ownerIds.end());
    NetVaultNode templateNode;
    templateNode.SetNodeType(plVault::kNodeType_PlayerInfo);
    {
        unsigned prevId = 0;
        for (size_t i = 0; i < ownerIds.size(); ++i) {
            if (ownerIds[i] != prevId) {
                prevId = ownerIds[i];
                VaultPlayerInfoNode access(&templateNode);
                access.SetPlayerId(refs[i].ownerId);
                if (VaultGetNode(&templateNode))
                    continue;
                NetCliAuthVaultNodeFind(
                    &templateNode,
                    VaultNodeFound,
                    nullptr
                );
            }
        }
    }
}

//============================================================================
static void FetchNodesFromRefs (
    NetVaultNodeRef *           refs,
    unsigned                    refCount,
    FNetCliAuthVaultNodeFetched fetchCallback,
    void *                      fetchParam,
    unsigned *                  fetchCount
    
) {
    // On the side, start downloading PlayerInfo nodes of ref owners we don't already have locally
    FetchRefOwners(refs, refCount);

    *fetchCount = 0;
    
    std::vector<unsigned> newNodeIds;
    std::vector<unsigned> existingNodeIds;
    
    BuildNodeTree(refs, refCount, &newNodeIds, &existingNodeIds);

    std::vector<unsigned> nodeIds;
    nodeIds.insert(nodeIds.end(), newNodeIds.begin(), newNodeIds.end());
    nodeIds.insert(nodeIds.end(), existingNodeIds.begin(), existingNodeIds.end());
    std::sort(nodeIds.begin(), nodeIds.end());

    // Fetch the nodes that do not yet have a nodetype
    unsigned prevId = 0;
    for (unsigned nodeId : nodeIds) {
        const hsRef<RelVaultNode>& node = s_nodes.at(nodeId);
        if (node->GetNodeType() != 0) {
            continue;
        }
        // filter duplicates
        if (node->GetNodeId() == prevId) {
            continue;
        }
        prevId = node->GetNodeId();
        NetCliAuthVaultNodeFetch(
            nodeId,
            fetchCallback,
            fetchParam
        );
        ++(*fetchCount);
    }
}

//============================================================================
static void VaultNodeFound (
    ENetError           result,
    void *              ,
    unsigned            nodeIdCount,
    const unsigned      nodeIds[]
) {
    // TODO: Support some sort of optional transaction object/callback state
    
    // error?
    if (IS_NET_ERROR(result))
        return;

    for (unsigned i = 0; i < nodeIdCount; ++i) {
        
        // See if we already have this node
        if (s_nodes.find(nodeIds[i]) != s_nodes.end()) {
            return;
        }

        // Start fetching the node          
        NetCliAuthVaultNodeFetch(nodeIds[i], VaultNodeFetched, nullptr);
    }
}

//============================================================================
static void VaultNodeFetched (
    ENetError           result,
    void *              ,
    NetVaultNode *      node
) {
    if (IS_NET_ERROR(result)) {
        LogMsg(kLogDebug, "VaultNodeFetched failed: {} ({})", result, NetErrorToString(result));
        return;
    }

    // Add to global node table
    auto it = s_nodes.find(node->GetNodeId());
    if (it == s_nodes.end()) {
        hsRef<RelVaultNode> newGlobalNode(new RelVaultNode(), hsStealRef);
        newGlobalNode->SetNodeId_NoDirty(node->GetNodeId());
        it = s_nodes.emplace(node->GetNodeId(), std::move(newGlobalNode)).first;
    }
    const hsRef<RelVaultNode>& globalNode = it->second;
    globalNode->CopyFrom(node);
    InitFetchedNode(globalNode);

    globalNode->Print("Fetched", 0);
}

//============================================================================
static void ChangedVaultNodeFetched (
    ENetError           result,
    void *              param,
    NetVaultNode *      node
) {
    if (IS_NET_ERROR(result)) {
        LogMsg(kLogDebug, "ChangedVaultNodeFetched failed: {} ({})", result, NetErrorToString(result));
        return;
    }

    VaultNodeFetched(result, param, node);

    auto it = s_nodes.find(node->GetNodeId());

    // Yeah, we are purposefully allowing this global callback to go out,
    // even if callback suppression has been enabled. The intent behind
    // that is to suppress spurious callbacks, but node changes are
    // probably not spurious.
    if (it != s_nodes.end()) {
        const hsRef<RelVaultNode>& savedNode = it->second;
        for (auto cb : s_callbacks) {
            cb->ChangedNode(savedNode);
        }
    }
}

//============================================================================
static void VaultNodeChanged (
    unsigned        nodeId,
    const plUUID&   revisionId
) {
    LogMsg(kLogDebug, "Notify: Node changed: {}", nodeId);

    auto it = s_nodes.find(nodeId);

    // We don't have the node, so we don't care that it changed (we actually
    // shouldn't have been notified)
    if (it == s_nodes.end()) {
        LogMsg(kLogDebug, "rcvd change notification for node {}, but node doesn't exist locally.", nodeId);
        return;
    }
    const hsRef<RelVaultNode>& node = it->second;

    if (node->GetRevision() == revisionId) {
        // We are the party responsible for the change, so we already have the
        // latest version of the node; no need to fetch it. However, we do need to fire off
        // the "hey this was saved" callback.
        if (s_suppressCallbacks == 0) {
            for (auto cb : s_callbacks) {
                cb->ChangedNode(node);
            }
        }
    } else {
        // We have the node and we weren't the one that changed it, so fetch it.
        NetCliAuthVaultNodeFetch(
            nodeId,
            ChangedVaultNodeFetched,
            nullptr
        );
    }
}

//============================================================================
static void VaultNodeAdded (
    unsigned        parentId,
    unsigned        childId,
    unsigned        ownerId
) {
    LogMsg(kLogDebug, "Notify: Node added: p:{},c:{}", parentId, childId);

    unsigned inboxId = 0;
    if (hsRef<RelVaultNode> rvnInbox = VaultGetPlayerInboxFolder())
        inboxId = rvnInbox->GetNodeId();

    // Build the relationship locally
    NetVaultNodeRef refs[] = {
        { parentId, childId, ownerId }
    };
    std::vector<unsigned> newNodeIds;
    std::vector<unsigned> existingNodeIds;
    
    BuildNodeTree(refs, std::size(refs), &newNodeIds, &existingNodeIds, false);

    std::vector<unsigned> nodeIds;
    nodeIds.insert(nodeIds.end(), newNodeIds.begin(), newNodeIds.end());
    nodeIds.insert(nodeIds.end(), existingNodeIds.begin(), existingNodeIds.end());
    std::sort(nodeIds.begin(), nodeIds.end());

    // Fetch the nodes that do not yet have a nodetype
    unsigned prevId = 0;
    unsigned i = 0;
    for (; i < nodeIds.size(); ++i) {
        const hsRef<RelVaultNode>& node = s_nodes.at(nodeIds[i]);
        if (node->GetNodeType() != 0) {
            continue;
        }
        // filter duplicates
        if (node->GetNodeId() == prevId) {
            continue;
        }
        prevId = node->GetNodeId();
        VaultDownload(
            "NodeAdded",
            nodeIds[i],
            VaultNodeAddedDownloadCallback,
            (void*)(uintptr_t)nodeIds[i],
            nullptr,
            nullptr
        );
    }
    
    if (parentId == inboxId && i == 0) {
        VaultProcessPlayerInbox();
    }

    // if the added element is already downloaded then send the callbacks now
    const hsRef<RelVaultNode>& parentNode = s_nodes.at(parentId);
    const hsRef<RelVaultNode>& childNode = s_nodes.at(childId);

    if (childNode->GetNodeType() != 0 && s_suppressCallbacks == 0) {
        for (auto cb : s_callbacks) {
            cb->AddedChildNode(parentNode, childNode);
        }
    }
}

//============================================================================
static void VaultNodeRemoved (
    unsigned        parentId,
    unsigned        childId
) {
    LogMsg(kLogDebug, "Notify: Node removed: p:{},c:{}", parentId, childId);
    for (;;) {
        // Unlink 'em locally, if we can
        auto parentIt = s_nodes.find(parentId);
        if (parentIt == s_nodes.end()) {
            break;
        }

        auto childIt = s_nodes.find(childId);
        if (childIt == s_nodes.end()) {
            break;
        }

        const hsRef<RelVaultNode>& parentNode = parentIt->second;
        const hsRef<RelVaultNode>& childNode = childIt->second;
        if (parentNode->IsParentOf(childId, 1) && s_suppressCallbacks == 0) {
            // We have the relationship, so make the callbacks
            for (auto cb : s_callbacks) {
                cb->RemovingChildNode(parentNode, childNode);
            }
        }

        parentNode->state->Unlink(childNode);
        childNode->state->Unlink(parentNode);
        break;
    }
}

//============================================================================
static void VaultNodeDeleted (
    unsigned        nodeId
) {
    LogMsg(kLogDebug, "Notify: Node deleted: {}", nodeId);
    VaultCull(nodeId);
}

//============================================================================
static void SaveDirtyNodes () {
    // Save a max of 5Kb every quarter second
    static const unsigned kSaveUpdateIntervalMs     = 250;
    static const unsigned kMaxBytesPerSaveUpdate    = 5 * 1024;
    static unsigned s_nextSaveMs;
    unsigned currTimeMs = hsTimer::GetMilliSeconds<uint32_t>() | 1;
    if (!s_nextSaveMs || signed(s_nextSaveMs - currTimeMs) <= 0) {
        s_nextSaveMs = (currTimeMs + kSaveUpdateIntervalMs) | 1;
        unsigned bytesWritten = 0;
        for (const auto& [nodeId, node] : s_nodes) {
            if (bytesWritten >= kMaxBytesPerSaveUpdate)
                break;
            if (node->IsDirty()) {
                if (unsigned bytes = NetCliAuthVaultNodeSave(node.Get(), nullptr, nullptr); bytes) {
                    bytesWritten += bytes;
                    node->Print("Saving", 0);
                }
            }
        }
    }
}

//============================================================================
static hsRef<RelVaultNode> GetChildFolderNode (
    hsWeakRef<RelVaultNode> parent,
    unsigned                folderType,
    unsigned                maxDepth
) {
    if (!parent)
        return nullptr;

    return parent->GetChildFolderNode(folderType, maxDepth);
}

//============================================================================
static hsRef<RelVaultNode> GetChildPlayerInfoListNode (
    hsWeakRef<RelVaultNode> parent,
    unsigned                folderType,
    unsigned                maxDepth
) {
    if (!parent)
        return nullptr;

    return parent->GetChildPlayerInfoListNode(folderType, maxDepth);
}


/*****************************************************************************
*
*   VaultCreateNodeTrans
*
***/

//============================================================================
void VaultCreateNodeTrans::VaultNodeCreated (
    ENetError           result,
    void *              param,
    unsigned            nodeId
) {
    VaultCreateNodeTrans * trans = (VaultCreateNodeTrans *)param;
    if (IS_NET_ERROR(result)) {
        trans->Complete(result);
    }
    else {
        trans->nodeId = nodeId;
        NetCliAuthVaultNodeFetch(
            nodeId,
            VaultCreateNodeTrans::VaultNodeFetched,
            trans
        );
    }
}

//============================================================================
void VaultCreateNodeTrans::VaultNodeFetched (
    ENetError           result,
    void *              param,
    NetVaultNode *      node
) {
    ::VaultNodeFetched(result, param, node);

    VaultCreateNodeTrans * trans = (VaultCreateNodeTrans *)param;
    
    if (IS_NET_SUCCESS(result)) {
        trans->node = s_nodes.at(node->GetNodeId());
    } else {
        trans->node = nullptr;
    }
    
    trans->Complete(result);
}

//============================================================================
void VaultCreateNodeTrans::Complete (ENetError result) {

    if (callback)
        callback(
            result,
            state,
            param,
            node
        );

    delete this;
}


/*****************************************************************************
*
*   VaultFindNodeTrans
*
***/

//============================================================================
void VaultFindNodeTrans::VaultNodeFound (
    ENetError           result,
    void *              param,
    unsigned            nodeIdCount,
    const unsigned      nodeIds[]
) {
    VaultFindNodeTrans * trans = (VaultFindNodeTrans*)param;
    if (trans->callback)
        trans->callback(
            result,
            trans->param,
            nodeIdCount,
            nodeIds
        );
    delete trans;
}


/*****************************************************************************
*
*   VaultDownloadTrans
*
***/

//============================================================================
void VaultDownloadTrans::VaultNodeFetched (
    ENetError           result,
    void *              param,
    NetVaultNode *      node
) {
    ::VaultNodeFetched(result, param, node);

    VaultDownloadTrans * trans = (VaultDownloadTrans *)param;
    if (IS_NET_ERROR(result)) {
        trans->result = result;
        //LogMsg(kLogError, "Error fetching node...most likely trying to fetch a nodeid of 0");
    }
    
    --trans->nodesLeft;
//  LogMsg(kLogDebug, "(Download) {} of {} nodes fetched", trans->nodeCount - trans->nodesLeft, trans->nodeCount);
    
    if (trans->progressCallback) {
        trans->progressCallback(
            trans->nodeCount,
            trans->nodeCount - trans->nodesLeft,
            trans->cbProgressParam
        );
    }
    
    if (!trans->nodesLeft) {
        VaultDump(trans->tag, trans->vaultId);

        if (trans->callback)
            trans->callback(
                trans->result,
                trans->cbParam
            );

        delete trans;
    }
}

//============================================================================
void VaultDownloadTrans::VaultNodeRefsFetched (
    ENetError           result,
    void *              param,
    NetVaultNodeRef *   refs,
    unsigned            refCount
) {
    VaultDownloadTrans * trans = (VaultDownloadTrans *)param;
    
    if (IS_NET_ERROR(result)) {
        LogMsg(kLogDebug, "VaultNodeRefsFetched failed: {} ({})", result, NetErrorToString(result));
        trans->result       = result;
        trans->nodesLeft    = 0;
    }
    else {
        if (refCount) {
            FetchNodesFromRefs(
                refs,
                refCount,
                VaultDownloadTrans::VaultNodeFetched,
                param,
                &trans->nodeCount
            );
            trans->nodesLeft = trans->nodeCount;
        }
        else {
            // root node has no child heirarchy? Make sure we still d/l the root node if necessary.
            auto rootNodeIt = s_nodes.find(trans->vaultId);
            if (rootNodeIt == s_nodes.end() || rootNodeIt->second->GetNodeType() == 0) {
                NetCliAuthVaultNodeFetch(
                    trans->vaultId,
                    VaultDownloadTrans::VaultNodeFetched,
                    trans
                );
                trans->nodesLeft = 1;
            }
        }
    }

    // Make the callback now if there are no nodes to fetch, or if error
    if (!trans->nodesLeft) {
        if (trans->callback)
            trans->callback(
                trans->result,
                trans->cbParam
            );

        delete trans;
    }
}


/*****************************************************************************
*
*   VaultAgeInitTrans
*
***/

//============================================================================
void VaultAgeInitTrans::AgeInitCallback (
    ENetError       result,
    void *          param,
    unsigned        ageVaultId,
    unsigned        ageInfoVaultId
) {
    VaultAgeInitTrans * trans = (VaultAgeInitTrans *)param;

    if (trans->callback)
        trans->callback(
            result,
            trans->cbState,
            trans->cbParam,
            ageVaultId,
            ageInfoVaultId
        );
    
    delete trans;
}


/*****************************************************************************
*
*   AddChildNodeFetchTrans
*
***/

//============================================================================
void AddChildNodeFetchTrans::VaultNodeRefsFetched (
    ENetError           result,
    void *              param,
    NetVaultNodeRef *   refs,
    unsigned            refCount
) {
    AddChildNodeFetchTrans * trans = (AddChildNodeFetchTrans *)param;

    if (IS_NET_ERROR(result)) {
        trans->result       = result;
    }
    else {
        unsigned incFetchCount = 0;
        FetchNodesFromRefs(
            refs,
            refCount,
            AddChildNodeFetchTrans::VaultNodeFetched,
            param,
            &incFetchCount
        );
        trans->opCount += incFetchCount;
    }

    // Make the callback now if there are no nodes to fetch, or if error
    if (!(--trans->opCount)) {
        if (trans->callback)
            trans->callback(
                trans->result,
                trans->cbParam
            );
        delete trans;
    }
}

//============================================================================
void AddChildNodeFetchTrans::VaultNodeFetched (
    ENetError           result,
    void *              param,
    NetVaultNode *      node
) {
    ::VaultNodeFetched(result, param, node);
    
    AddChildNodeFetchTrans * trans = (AddChildNodeFetchTrans *)param;
    
    if (IS_NET_ERROR(result))
        trans->result = result;

    if (!(--trans->opCount)) {
        if (trans->callback)
            trans->callback(
                trans->result,
                trans->cbParam
            );
        delete trans;
    }
}


/*****************************************************************************
*
*   IRelVaultNode
*
***/

//============================================================================
IRelVaultNode::IRelVaultNode(hsWeakRef<RelVaultNode> node)
    : node(node)
{ }

//============================================================================
IRelVaultNode::~IRelVaultNode () {
    ASSERT(parents.empty());
    ASSERT(children.empty());
}

//============================================================================
void IRelVaultNode::UnlinkFromRelatives () {
    for (auto it = parents.begin(); it != parents.end();) {
        // Advance the iterator before calling Unlink so that it doesn't get invalidated
        hsWeakRef<RelVaultNode> parentNode = it->second;
        ++it;

        // We have the relationship, so make the callbacks
        if (s_suppressCallbacks == 0) {
            for (auto cb : s_callbacks) {
                cb->RemovingChildNode(parentNode, this->node);
            }
        }

        parentNode->state->Unlink(node);
    }
    for (auto it = children.begin(); it != children.end();) {
        // Advance the iterator before calling Unlink so that it doesn't get invalidated
        hsWeakRef<RelVaultNode> childNode = it->second.node;
        ++it;
        childNode->state->Unlink(node);
    }

    ASSERT(parents.empty());
    ASSERT(children.empty());
}


//============================================================================
void IRelVaultNode::Unlink(hsWeakRef<RelVaultNode> other) {
    ASSERT(other != node);

    auto parentIt = parents.find(other->GetNodeId());
    if (parentIt != parents.end()) {
        // Grab the node ref - the erase call will invalidate the iterator!
        hsRef<RelVaultNode> parentNode = std::move(parentIt->second);
        // make them non-findable in our parents table
        parents.erase(parentIt);
        // remove us from other's tables.
        parentNode->state->Unlink(node);
    }

    auto childIt = children.find(other->GetNodeId());
    if (childIt != children.end()) {
        // Grab the node ref - the erase call will invalidate the iterator!
        hsRef<RelVaultNode> childNode = std::move(childIt->second.node);
        // make them non-findable in our children table
        children.erase(childIt);
        // remove us from other's tables.
        childNode->state->Unlink(node);
    }
}

/*****************************************************************************
*
*   RelVaultNode
*
***/

//============================================================================
RelVaultNode::RelVaultNode () {
    state = new IRelVaultNode(this);
}

//============================================================================
RelVaultNode::~RelVaultNode () {
    delete state;
}

//============================================================================
bool RelVaultNode::IsParentOf (unsigned childId, unsigned maxDepth) {
    if (GetNodeId() == childId)
        return false;
    if (maxDepth == 0)
        return false;
    if (state->children.find(childId) != state->children.end())
        return true;
    for (const auto& [nodeId, link] : state->children) {
        if (link.node->IsParentOf(childId, maxDepth - 1)) {
            return true;
        }
    }
    return false;
}

//============================================================================
bool RelVaultNode::IsChildOf (unsigned parentId, unsigned maxDepth) {
    if (GetNodeId() == parentId)
        return false;
    if (maxDepth == 0)
        return false;
    if (state->parents.find(parentId) != state->parents.end())
        return true;
    for (const auto& [nodeId, node] : state->parents) {
        if (node->IsChildOf(parentId, maxDepth - 1)) {
            return true;
        }
    }
    return false;
}

//============================================================================
void RelVaultNode::GetRootIds (std::vector<unsigned> * nodeIds) {
    if (state->parents.empty()) {
        nodeIds->emplace_back(GetNodeId());
    } else {
        for (const auto& [nodeId, node] : state->parents) {
            node->GetRootIds(nodeIds);
        }
    }
}

//============================================================================
unsigned RelVaultNode::RemoveChildNodes (unsigned maxDepth) {
    hsAssert(false, "eric, implement me.");
    return 0;
}

//============================================================================
void RelVaultNode::GetChildNodeIds (
    std::vector<unsigned> * nodeIds,
    unsigned            maxDepth
) {
    if (!maxDepth)
        return;
    for (const auto& [nodeId, link] : state->children) {
        nodeIds->emplace_back(link.node->GetNodeId());
        link.node->GetChildNodeIds(nodeIds, maxDepth-1);
    }
}

//============================================================================
void RelVaultNode::GetParentNodeIds (
    std::vector<unsigned> * nodeIds,
    unsigned            maxDepth
) {
    if (!maxDepth)
        return;
    for (const auto& [nodeId, node] : state->parents) {
        nodeIds->emplace_back(node->GetNodeId());
        node->GetParentNodeIds(nodeIds, maxDepth-1);
    }
}


//============================================================================
hsRef<RelVaultNode> RelVaultNode::GetParentNode (
    hsWeakRef<NetVaultNode> templateNode,
    unsigned                maxDepth
) {
    if (maxDepth == 0)
        return nullptr;

    for (const auto& [nodeId, node] : state->parents) {
        if (node->Matches(templateNode.Get())) {
            return node;
        }
    }

    for (const auto& [nodeId, node] : state->parents) {
        if (hsRef<RelVaultNode> parentNode = node->GetParentNode(templateNode, maxDepth - 1)) {
            return parentNode;
        }
    }

    return nullptr;
}

//============================================================================
hsRef<RelVaultNode> RelVaultNode::GetChildNode (
    hsWeakRef<NetVaultNode> templateNode,
    unsigned                maxDepth
) {
    if (maxDepth == 0)
        return nullptr;

    for (const auto& [nodeId, link] : state->children) {
        if (link.node->Matches(templateNode.Get())) {
            return link.node;
        }
    }

    for (const auto& [nodeId, link] : state->children) {
        if (hsRef<RelVaultNode> node = link.node->GetChildNode(templateNode, maxDepth - 1)) {
            return node;
        }
    }

    return nullptr;
}

//============================================================================
hsRef<RelVaultNode> RelVaultNode::GetChildNode (
    unsigned            nodeType,
    unsigned            maxDepth
) {
    NetVaultNode templateNode;
    templateNode.SetNodeType(nodeType);
    return GetChildNode(&templateNode, maxDepth);
}

//============================================================================
hsRef<RelVaultNode> RelVaultNode::GetChildFolderNode (
    unsigned            folderType,
    unsigned            maxDepth
) {
    NetVaultNode templateNode;
    templateNode.SetNodeType(plVault::kNodeType_Folder);
    VaultFolderNode folder(&templateNode);
    folder.SetFolderType(folderType);
    return GetChildNode(&templateNode, maxDepth);
}

//============================================================================
hsRef<RelVaultNode> RelVaultNode::GetChildPlayerInfoListNode (
    unsigned            folderType,
    unsigned            maxDepth
) {
    NetVaultNode templateNode;
    templateNode.SetNodeType(plVault::kNodeType_PlayerInfoList);
    VaultPlayerInfoListNode access(&templateNode);
    access.SetFolderType(folderType);
    return GetChildNode(&templateNode, maxDepth);
}

//============================================================================
hsRef<RelVaultNode> RelVaultNode::GetChildAgeInfoListNode (
    unsigned            folderType,
    unsigned            maxDepth
) {
    NetVaultNode templateNode;
    templateNode.SetNodeType(plVault::kNodeType_AgeInfoList);
    VaultAgeInfoListNode access(&templateNode);
    access.SetFolderType(folderType);
    return GetChildNode(&templateNode, maxDepth);
}

//============================================================================
void RelVaultNode::GetChildNodes (
    unsigned                maxDepth,
    RelVaultNode::RefList * nodes
) {
    if (maxDepth == 0)
        return;

    for (const auto& [nodeId, link] : state->children) {
        nodes->push_back(link.node);
        link.node->GetChildNodes(
            maxDepth - 1,
            nodes
        );
    }
}

//============================================================================
void RelVaultNode::GetChildNodes (
    hsWeakRef<NetVaultNode> templateNode,
    unsigned                maxDepth,
    RelVaultNode::RefList * nodes
) {
    for (const auto& [nodeId, link] : state->children) {
        if (link.node->Matches(templateNode.Get())) {
            nodes->push_back(link.node);
        }

        link.node->GetChildNodes(
            templateNode,
            maxDepth - 1,
            nodes
        );
    }
}

//============================================================================
void RelVaultNode::GetChildNodes (
    unsigned                nodeType,
    unsigned                maxDepth,
    RelVaultNode::RefList * nodes
) {
    NetVaultNode templateNode;
    templateNode.SetNodeType(nodeType);
    GetChildNodes(
        &templateNode,
        maxDepth,
        nodes
    );
}

//============================================================================
void RelVaultNode::GetChildFolderNodes (
    unsigned                folderType,
    unsigned                maxDepth,
    RelVaultNode::RefList * nodes
) {
    NetVaultNode templateNode;
    templateNode.SetNodeType(plVault::kNodeType_Folder);
    VaultFolderNode fldr(&templateNode);
    fldr.SetFolderType(folderType);
    GetChildNodes(
        &templateNode,
        maxDepth,
        nodes
    );
}

//============================================================================
unsigned RelVaultNode::GetRefOwnerId (unsigned parentId) {
    // find our parents' link to us and return its ownerId
    auto parentIt = state->parents.find(parentId);
    if (parentIt != state->parents.end()) {
        const hsRef<RelVaultNode>& parentNode = parentIt->second;
        auto childIt = parentNode->state->children.find(GetNodeId());
        if (childIt != parentNode->state->children.end()) {
            return childIt->second.ownerId;
        }
    }
    return 0;
}

//============================================================================
bool RelVaultNode::BeenSeen (unsigned parentId) const {
    // find our parents' link to us and return its seen flag
    auto parentIt = state->parents.find(parentId);
    if (parentIt != state->parents.end()) {
        const hsRef<RelVaultNode>& parentNode = parentIt->second;
        auto childIt = parentNode->state->children.find(GetNodeId());
        if (childIt != parentNode->state->children.end()) {
            return childIt->second.seen;
        }
    }
    return true;
}

//============================================================================
void RelVaultNode::SetSeen (unsigned parentId, bool seen) {
    // find our parents' link to us and set its seen flag
    auto parentIt = state->parents.find(parentId);
    if (parentIt != state->parents.end()) {
        const hsRef<RelVaultNode>& parentNode = parentIt->second;
        auto childIt = parentNode->state->children.find(GetNodeId());
        if (childIt != parentNode->state->children.end()) {
            RelVaultNodeLink& childLink = childIt->second;
            if (childLink.seen != seen) {
                childLink.seen = seen;
                NetCliAuthVaultSetSeen(parentId, GetNodeId(), seen);
            }
        }
    }
}

//============================================================================
void RelVaultNode::Print (const ST::string& tag, unsigned level) {
    ST::string_stream ss;
    ss << tag;
    ss << ST::string::fill(level * 2, ' ');
    ss << " " << GetNodeId();
    ss << " " << plVault::NodeTypeStr(GetNodeType());

    for (uint64_t bit = 1; bit; bit <<= 1) {
        if (!(GetFieldFlags() & bit))
            continue;
        if (bit > GetFieldFlags())
            break;

#define STPRINT(flag) \
    case k##flag: \
        ss << ", " #flag "=\"" << Get##flag() << "\""; \
        break;
#define STPRINT_UUID(flag) \
    case k##flag: \
        ss << ", " #flag "=\"" << Get##flag().AsString() << "\""; \
        break;
#define STPRINT_ESCAPE(flag) \
    case k##flag: \
        ss << ", " #flag "=\"" << Get##flag().replace("\"", "\\\"") << "\""; \
        break;
#define STNAME(flag) \
    case k##flag: \
        ss << ", " << #flag; \
        break;

        switch (bit) {
            STPRINT(NodeId);
            STPRINT(CreateTime);
            STPRINT(ModifyTime);
            STPRINT(CreateAgeName);
            STPRINT_UUID(CreateAgeUuid);
            STPRINT_UUID(CreatorAcct);
            STPRINT(CreatorId);
            STPRINT(NodeType);
            STPRINT(Int32_1);
            STPRINT(Int32_2);
            STPRINT(Int32_3);
            STPRINT(Int32_4);
            STPRINT(UInt32_1);
            STPRINT(UInt32_2);
            STPRINT(UInt32_3);
            STPRINT(UInt32_4);
            STPRINT_UUID(Uuid_1);
            STPRINT_UUID(Uuid_2);
            STPRINT_UUID(Uuid_3);
            STPRINT_UUID(Uuid_4);
            STPRINT_ESCAPE(String64_1);
            STPRINT_ESCAPE(String64_2);
            STPRINT_ESCAPE(String64_3);
            STPRINT_ESCAPE(String64_4);
            STPRINT_ESCAPE(String64_5);
            STPRINT_ESCAPE(String64_6);
            STPRINT_ESCAPE(IString64_1);
            STPRINT_ESCAPE(IString64_2);
            STNAME(Text_1);
            STNAME(Text_2);
            STNAME(Blob_1);
            STNAME(Blob_2);
            DEFAULT_FATAL(bit);
        }
#undef STPRINT
#undef STNAME
    }

    plStatusLog::AddLineS("VaultClient.log", ss.to_string());
}

//============================================================================
void RelVaultNode::PrintTree (unsigned level) {
    Print("", level);
    for (const auto& [nodeId, link] : state->children) {
        link.node->PrintTree(level + 1);
    }
}

//============================================================================
hsRef<RelVaultNode> RelVaultNode::GetParentAgeLink () {

    // this function only makes sense when called on age info nodes
    ASSERT(GetNodeType() == plVault::kNodeType_AgeInfo);

    hsRef<RelVaultNode> result;
    NetVaultNode templateNode;
    templateNode.SetNodeType(plVault::kNodeType_AgeLink);

    // Get our parent AgeLink node  
    if (hsRef<RelVaultNode> rvnLink = GetParentNode(&templateNode, 1)) {
        // Get the next AgeLink node in our parent tree
        result = rvnLink->GetParentNode(&templateNode, 3);
    }

    return result;
}


/*****************************************************************************
*
*   Exports - Callbacks
*
***/

//============================================================================
void VaultRegisterCallback (VaultCallback * cb) {
    s_callbacks.emplace_back(cb);
}

//============================================================================
void VaultUnregisterCallback (VaultCallback * cb) {
    auto it = std::find(s_callbacks.begin(), s_callbacks.end(), cb);
    ASSERT(it != s_callbacks.end());
    s_callbacks.erase(it);
}

//============================================================================
void VaultSuppressCallbacks() {
    ++s_suppressCallbacks;
}

//============================================================================
void VaultEnableCallbacks() {
    --s_suppressCallbacks;
    hsAssert(s_suppressCallbacks >= 0, "Hmm... A negative vault callback suppression count?");
}

/*****************************************************************************
*
*   Exports - Initialize
*
***/

//============================================================================
void VaultInitialize () {
    NetCliAuthVaultSetRecvNodeChangedHandler(VaultNodeChanged);
    NetCliAuthVaultSetRecvNodeAddedHandler(VaultNodeAdded);
    NetCliAuthVaultSetRecvNodeRemovedHandler(VaultNodeRemoved);
    NetCliAuthVaultSetRecvNodeDeletedHandler(VaultNodeDeleted);
}

//============================================================================
void VaultDestroy () {
    NetCliAuthVaultSetRecvNodeChangedHandler(nullptr);
    NetCliAuthVaultSetRecvNodeAddedHandler(nullptr);
    NetCliAuthVaultSetRecvNodeRemovedHandler(nullptr);
    NetCliAuthVaultSetRecvNodeDeletedHandler(nullptr);

    VaultClearDeviceInboxMap();

    for (auto it = s_nodes.begin(); it != s_nodes.end();) {
        it->second->state->UnlinkFromRelatives();
        it = s_nodes.erase(it);
    }
}

//============================================================================
void VaultUpdate () {
    SaveDirtyNodes();
}


/*****************************************************************************
*
*   Exports - Generic Vault Access
*
***/

//============================================================================
hsRef<RelVaultNode> VaultGetNode (
    hsWeakRef<NetVaultNode> templateNode
) {
    ASSERT(templateNode);
    for (const auto& [nodeId, node] : s_nodes) {
        if (node->Matches(templateNode.Get())) {
            return node;
        }
    }
    return nullptr;
}

//============================================================================
hsRef<RelVaultNode> VaultGetNode (
    unsigned nodeId
) {
    auto it = s_nodes.find(nodeId);
    return it == s_nodes.end() ? nullptr : it->second;
}

//============================================================================
void VaultAddChildNode (
    unsigned                    parentId,
    unsigned                    childId,
    unsigned                    ownerId,
    FVaultAddChildNodeCallback  callback,
    void *                      param
) {
    // Make sure we only do the callback once
    bool madeCallback = false;

    // Too much of the client relies on the assumption that the node will be immediately
    // associated with its parent.  THIS SUCKS, because there's no way to guarantee the
    // association won't be circular (the db checks this in a comprehensive way).
    // Because the client depends on this so much, we just link 'em together here if 
    // we have both of them present locally.
    // This directly affects: New clothing items added to the avatar outfit folder,
    // new chronicle entries in some ages, and I'm sure several other situations.

    auto parentIt = s_nodes.find(parentId);
    if (parentIt != s_nodes.end()) {
        const hsRef<RelVaultNode>& parentNode = parentIt->second;
        auto childIt = s_nodes.find(childId);
        if (childIt == s_nodes.end()) {
            hsRef<RelVaultNode> newChildNode(new RelVaultNode(), hsStealRef);
            newChildNode->SetNodeId_NoDirty(childId);
            childIt = s_nodes.emplace(childId, std::move(newChildNode)).first;
        }
        const hsRef<RelVaultNode>& childNode = childIt->second;

        // We can do a sanity check for a would-be circular link, but it isn't
        // authoritative.  The db will prevent circular links from entering into
        // the persistent state, but because we are hacking in the association
        // before the authoritative check, we're risking the local client operating
        // on bad, possibly harmful vault state.  Not harmful in a national security
        // kinda way, but still harmful.
        if (parentNode->IsChildOf(childId, 255)) {
            LogMsg(kLogDebug, "Node relationship would be circular: p:{}, c:{}", parentId, childId);
            // callback now with error code
            if (callback)
                callback(kNetErrCircularReference, param);
        }
        else if (childNode->IsParentOf(parentId, 255)) {
            LogMsg(kLogDebug, "Node relationship would be circular: p:{}, c:{}", parentId, childId);
            // callback now with error code
            if (callback)
                callback(kNetErrCircularReference, param);
        }
        else {
            NetVaultNodeRef refs[] = {
                { parentId, childId, ownerId }
            };

            std::vector<unsigned> newNodeIds;
            std::vector<unsigned> existingNodeIds;

            BuildNodeTree(refs, std::size(refs), &newNodeIds, &existingNodeIds);
        
            if (!childNode->GetNodeType() || !parentNode->GetNodeType()) {
                // One or more nodes need to be fetched before the callback is made
                AddChildNodeFetchTrans * trans = new AddChildNodeFetchTrans(callback, param);
                if (!childNode->GetNodeType()) {
                    ++trans->opCount;
                    NetCliAuthVaultNodeFetch(
                        childId,
                        AddChildNodeFetchTrans::VaultNodeFetched,
                        trans
                    );
                    ++trans->opCount;
                    NetCliAuthVaultFetchNodeRefs(
                        childId,
                        AddChildNodeFetchTrans::VaultNodeRefsFetched,
                        trans
                    );
                }
                if (!parentNode->GetNodeType()) {
                    ++trans->opCount;
                    NetCliAuthVaultNodeFetch(
                        parentId,
                        AddChildNodeFetchTrans::VaultNodeFetched,
                        trans
                    );
                    ++trans->opCount;
                    NetCliAuthVaultFetchNodeRefs(
                        parentId,
                        AddChildNodeFetchTrans::VaultNodeRefsFetched,
                        trans
                    );
                }
            }
            else {
                // We have both nodes already, so make the callback now.
                if (callback) {
                    callback(kNetSuccess, param);
                    madeCallback = true;
                }
            }
        }
    }
    else {
        // Parent doesn't exist locally (and we may not want it to), just make the callback now.
        if (callback) {
            callback(kNetSuccess, param);
            madeCallback = true;
        }
    }

    // Send it on up to the vault. The db server filters out duplicate and
    // circular node relationships. We send the request up even if we think
    // the relationship would be circular since the db does a universal
    // check and is the only real authority in this matter.
    NetCliAuthVaultNodeAdd(
        parentId,
        childId,
        ownerId,
        madeCallback ? nullptr : callback,
        madeCallback ? nullptr : param
    );
}

//============================================================================
namespace _VaultAddChildNodeAndWait {

struct _AddChildNodeParam {
    ENetError       result;
    bool            complete;
};
static void _AddChildNodeCallback (
    ENetError       result,
    void *          vparam
) {
    _AddChildNodeParam * param = (_AddChildNodeParam *)vparam;
    param->result       = result;
    param->complete     = true;
}

} // namespace _VaultAddChildNodeAndWait

//============================================================================
void VaultAddChildNodeAndWait (
    unsigned                    parentId,
    unsigned                    childId,
    unsigned                    ownerId
) {
    using namespace _VaultAddChildNodeAndWait;
    
    _AddChildNodeParam param;
    memset(&param, 0, sizeof(param));
    
    VaultAddChildNode(
        parentId,
        childId,
        ownerId,
        _AddChildNodeCallback,
        &param
    );

    while (!param.complete) {
        NetClientUpdate();
        plgDispatch::Dispatch()->MsgQueueProcess();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    if (IS_NET_ERROR(param.result))
        LogMsg(kLogError, "VaultAddChildNodeAndWait: Failed to add child node: p:{},c:{}. {}", parentId, childId, NetErrorToString(param.result));
}

//============================================================================
void VaultRemoveChildNode (
    unsigned                        parentId,
    unsigned                        childId,
    FVaultRemoveChildNodeCallback   callback,
    void *                          param
) {
    for (;;) {
        // Unlink 'em locally, if we can
        auto parentIt = s_nodes.find(parentId);
        if (parentIt == s_nodes.end()) {
            break;
        }

        auto childIt = s_nodes.find(childId);
        if (childIt == s_nodes.end()) {
            break;
        }

        const hsRef<RelVaultNode>& parentNode = parentIt->second;
        const hsRef<RelVaultNode>& childNode = childIt->second;
        if (parentNode->IsParentOf(childId, 1)) {
            // We have the relationship, so make the callbacks
            if (s_suppressCallbacks == 0) {
                for (auto cb : s_callbacks) {
                    cb->RemovingChildNode(parentNode, childNode);
                }
            }
        }

        parentNode->state->Unlink(childNode);
        childNode->state->Unlink(parentNode);
        break;
    }
    
    // Send it on up to the vault
    NetCliAuthVaultNodeRemove(
        parentId,
        childId,
        callback,
        param
    );
}

//============================================================================
void VaultSetNodeSeen (
    unsigned    nodeId,
    bool        seen
) {
    hsAssert(false, "eric, implement me");
}

//============================================================================
void VaultDeleteNode (
    unsigned    nodeId
) {
    // Send request up to vault.  We will remove it locally upon notification of deletion.
    NetCliAuthVaultNodeDelete(nodeId);  
}

//============================================================================
void VaultSendNode (
    hsWeakRef<RelVaultNode> srcNode,
    unsigned                dstPlayerId
) {
    NetCliAuthVaultNodeSave(srcNode.Get(), nullptr, nullptr);
    NetCliAuthVaultSendNode(srcNode->GetNodeId(), dstPlayerId);
}

//============================================================================
void VaultCreateNode (
    hsWeakRef<NetVaultNode>     templateNode,
    FVaultCreateNodeCallback    callback,
    void *                      state,
    void *                      param
) {
    VaultCreateNodeTrans * trans = new VaultCreateNodeTrans(callback, state, param);

    if (hsRef<RelVaultNode> age = VaultGetAgeNode()) {
        VaultAgeNode access(age);
        templateNode->SetCreateAgeName(access.GetAgeName());
        templateNode->SetCreateAgeUuid(access.GetAgeInstanceGuid());
    }
    
    NetCliAuthVaultNodeCreate(
        templateNode.Get(),
        VaultCreateNodeTrans::VaultNodeCreated,
        trans
    );
}

//============================================================================
void VaultCreateNode (
    plVault::NodeTypes          nodeType,
    FVaultCreateNodeCallback    callback,
    void *                      state,
    void *                      param
) {
    NetVaultNode templateNode;
    templateNode.SetNodeType(nodeType);

    VaultCreateNode(
        &templateNode,
        callback,
        state,
        param
    );
}

//============================================================================
namespace _VaultCreateNodeAndWait {

struct _CreateNodeParam {
    hsWeakRef<RelVaultNode> node;
    ENetError       result;
    bool            complete;
};
static void _CreateNodeCallback (
    ENetError       result,
    void *          ,
    void *          vparam,
    hsWeakRef<RelVaultNode>  node
) {
    _CreateNodeParam * param = (_CreateNodeParam *)vparam;
    param->node     = node;
    param->result   = result;
    param->complete = true;
}

} // namespace _VaultCreateNodeAndWait

hsRef<RelVaultNode> VaultCreateNodeAndWait (
    hsWeakRef<NetVaultNode>     templateNode,
    ENetError *                 result
) {
    using namespace _VaultCreateNodeAndWait;
    
    _CreateNodeParam param;
    memset(&param, 0, sizeof(param));
    
    VaultCreateNode(
        templateNode,
        _CreateNodeCallback,
        nullptr,
        &param
    );
    
    while (!param.complete) {
        NetClientUpdate();
        plgDispatch::Dispatch()->MsgQueueProcess();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    *result = param.result;
    return param.node;
}

//============================================================================
hsRef<RelVaultNode> VaultCreateNodeAndWait (
    plVault::NodeTypes          nodeType,
    ENetError *                 result
) {
    NetVaultNode templateNode;
    templateNode.SetNodeType(nodeType);

    return VaultCreateNodeAndWait(&templateNode, result);
}

//============================================================================
namespace _VaultForceSaveNodeAndWait {

struct _SaveNodeParam {
    ENetError       result;
    bool            complete;
};
static void _SaveNodeCallback (
    ENetError       result,
    void *          vparam
) {
    _SaveNodeParam * param = (_SaveNodeParam *)vparam;
    param->result   = result;
    param->complete = true;
}

} // namespace _VaultForceSaveNodeAndWait

void VaultForceSaveNodeAndWait (
    hsWeakRef<NetVaultNode> node
) {
    using namespace _VaultForceSaveNodeAndWait;
    
    _SaveNodeParam param;
    memset(&param, 0, sizeof(param));
    
    NetCliAuthVaultNodeSave(
        node.Get(),
        _SaveNodeCallback,
        &param
    );
    
    while (!param.complete) {
        NetClientUpdate();
        plgDispatch::Dispatch()->MsgQueueProcess();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

//============================================================================
void VaultFindNodes (
    hsWeakRef<NetVaultNode> templateNode,
    FVaultFindNodeCallback  callback,
    void *                  param
) {
    VaultFindNodeTrans * trans = new VaultFindNodeTrans(callback, param);

    NetCliAuthVaultNodeFind(
        templateNode.Get(),
        VaultFindNodeTrans::VaultNodeFound,
        trans
    );  
}

//============================================================================
namespace _VaultFindNodesAndWait {
    struct _FindNodeParam {
        std::vector<unsigned> nodeIds;
        ENetError           result;
        bool                complete;

        _FindNodeParam()
            : result(kNetPending), complete(false)
        { }
    };
    static void _FindNodeCallback (
        ENetError           result,
        void *              vparam,
        unsigned            nodeIdCount,
        const unsigned      nodeIds[]
    ) {
        _FindNodeParam * param = (_FindNodeParam *)vparam;
        param->nodeIds.assign(nodeIds, nodeIds + nodeIdCount);
        param->result   = result;
        param->complete = true;
    }

} // namespace _VaultFindNodesAndWait

void VaultFindNodesAndWait (
    hsWeakRef<NetVaultNode> templateNode,
    std::vector<unsigned> * nodeIds
) {
    using namespace _VaultFindNodesAndWait;

    _FindNodeParam  param;
    NetCliAuthVaultNodeFind(
        templateNode.Get(),
        _FindNodeCallback,
        &param
    );

    while (!param.complete) {
        NetClientUpdate();
        plgDispatch::Dispatch()->MsgQueueProcess();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (IS_NET_SUCCESS(param.result))
        nodeIds->insert(nodeIds->end(), param.nodeIds.begin(), param.nodeIds.end());
}

//============================================================================
void VaultLocalFindNodes (
    hsWeakRef<NetVaultNode> templateNode,
    std::vector<unsigned> * nodeIds
) {
    for (const auto& [nodeId, node] : s_nodes) {
        if (node->Matches(templateNode.Get())) {
            nodeIds->emplace_back(node->GetNodeId());
        }
    }
}

//============================================================================
namespace _VaultFetchNodesAndWait {

    static void _VaultNodeFetched (
        ENetError           result,
        void *              param,
        NetVaultNode *      node
    ) {
        ::VaultNodeFetched(result, nullptr, node);
        
        --(*reinterpret_cast<std::atomic<unsigned>*>(param));
    }

} // namespace _VaultFetchNodesAndWait

void VaultFetchNodesAndWait (
    const unsigned  nodeIds[],
    unsigned        count,
    bool            force
) {
    using namespace _VaultFetchNodesAndWait;
    
    std::atomic<unsigned> nodeCount(count);
    
    for (unsigned i = 0; i < count; ++i) {
        
        if (!force) {
            // See if we already have this node
            if (s_nodes.find(nodeIds[i]) != s_nodes.end()) {
                --nodeCount;
                continue;
            }
        }

        // Start fetching the node
        NetCliAuthVaultNodeFetch(nodeIds[i], _VaultNodeFetched,
                                 reinterpret_cast<void *>(&nodeCount));
    }

    while (nodeCount) {
        NetClientUpdate();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }   
}


//============================================================================
void VaultInitAge (
    const plAgeInfoStruct * info,
    const plUUID            parentAgeInstId,    // optional
    FVaultInitAgeCallback   callback,
    void *                  state,
    void *                  param
) {
    VaultAgeInitTrans * trans = new VaultAgeInitTrans(callback, state, param);

    NetCliAuthVaultInitAge(
        *info->GetAgeInstanceGuid(),
        parentAgeInstId,
        info->GetAgeFilename(),
        info->GetAgeInstanceName(),
        info->GetAgeUserDefinedName(),
        info->GetAgeDescription(),
        info->GetAgeSequenceNumber(),
        info->GetAgeLanguage(),
        VaultAgeInitTrans::AgeInitCallback,
        trans
    );
}

/*****************************************************************************
*
*   Exports - Player Vault Access
*
***/

//============================================================================
static hsRef<RelVaultNode> GetPlayerNode () {
    NetVaultNode templateNode;
    templateNode.SetNodeType(plVault::kNodeType_VNodeMgrPlayer);
    if (NetCommGetPlayer())
        templateNode.SetNodeId(NetCommGetPlayer()->playerInt);
    return VaultGetNode(&templateNode);
}

//============================================================================
unsigned VaultGetPlayerId () {
    if (hsRef<RelVaultNode> rvn = GetPlayerNode())
        return rvn->GetNodeId();
    return 0;
}

//============================================================================
hsRef<RelVaultNode> VaultGetPlayerNode () {
    if (hsRef<RelVaultNode> rvnPlr = GetPlayerNode())
        return rvnPlr;
    return nullptr;
}

//============================================================================
hsRef<RelVaultNode> VaultGetPlayerInfoNode () {
    hsRef<RelVaultNode> rvnPlr = VaultGetPlayerNode();
    if (!rvnPlr)
        return nullptr;

    NetVaultNode templateNode;
    templateNode.SetNodeType(plVault::kNodeType_PlayerInfo);
    VaultPlayerInfoNode plrInfo(&templateNode);
    plrInfo.SetPlayerId(rvnPlr->GetNodeId());

    hsRef<RelVaultNode> result;
    if (hsRef<RelVaultNode> rvnPlrInfo = rvnPlr->GetChildNode(&templateNode, 1))
        result = rvnPlrInfo;

    return result;
}

//============================================================================
hsRef<RelVaultNode> VaultGetAvatarOutfitFolder () {
    if (hsRef<RelVaultNode> rvn = GetPlayerNode())
        return rvn->GetChildFolderNode(plVault::kAvatarOutfitFolder, 1);
    return nullptr;
}

//============================================================================
hsRef<RelVaultNode> VaultGetAvatarClosetFolder () {
    if (hsRef<RelVaultNode> rvn = GetPlayerNode())
        return rvn->GetChildFolderNode(plVault::kAvatarClosetFolder, 1);
    return nullptr;
}

//============================================================================
hsRef<RelVaultNode> VaultGetChronicleFolder () {
    if (hsRef<RelVaultNode> rvn = GetPlayerNode())
        return rvn->GetChildFolderNode(plVault::kChronicleFolder, 1);
    return nullptr;
}

//============================================================================
hsRef<RelVaultNode> VaultGetAgesIOwnFolder () {
    if (hsRef<RelVaultNode> rvn = GetPlayerNode())
        return rvn->GetChildAgeInfoListNode(plVault::kAgesIOwnFolder, 1);
    return nullptr;
}

//============================================================================
hsRef<RelVaultNode> VaultGetAgesICanVisitFolder () {
    if (hsRef<RelVaultNode> rvn = GetPlayerNode())
        return rvn->GetChildAgeInfoListNode(plVault::kAgesICanVisitFolder, 1);
    return nullptr;
}

//============================================================================
hsRef<RelVaultNode> VaultGetPlayerInboxFolder () {
    if (hsRef<RelVaultNode> rvn = GetPlayerNode())
        return rvn->GetChildFolderNode(plVault::kInboxFolder, 1);
    return nullptr;
}

//============================================================================
bool VaultGetLinkToMyNeighborhood (plAgeLinkStruct * link) {
    hsRef<RelVaultNode> rvnFldr = VaultGetAgesIOwnFolder();
    if (!rvnFldr)
        return false;

    NetVaultNode templateNode;

    templateNode.SetNodeType(plVault::kNodeType_AgeInfo);
    VaultAgeInfoNode ageInfo(&templateNode);
    ageInfo.SetAgeFilename(kNeighborhoodAgeFilename);

    hsRef<RelVaultNode> node;
    if (node = rvnFldr->GetChildNode(&templateNode, 2)) {
        VaultAgeInfoNode info(node);
        info.CopyTo(link->GetAgeInfo());
    }

    return node != nullptr;
}

//============================================================================
bool VaultGetLinkToMyPersonalAge (plAgeLinkStruct * link) {
    hsRef<RelVaultNode> rvnFldr = VaultGetAgesIOwnFolder();
    if (!rvnFldr)
        return false;

    NetVaultNode templateNode;

    templateNode.SetNodeType(plVault::kNodeType_AgeInfo);
    VaultAgeInfoNode ageInfo(&templateNode);
    ageInfo.SetAgeFilename(kPersonalAgeFilename);

    hsRef<RelVaultNode> node;
    if (node = rvnFldr->GetChildNode(&templateNode, 2)) {
        VaultAgeInfoNode info(node);
        info.CopyTo(link->GetAgeInfo());
    }

    return node != nullptr;
}

//============================================================================
bool VaultGetLinkToCity (plAgeLinkStruct * link) {
    hsRef<RelVaultNode> rvnFldr = VaultGetAgesIOwnFolder();
    if (!rvnFldr)
        return false;

    NetVaultNode templateNode;
    templateNode.SetNodeType(plVault::kNodeType_AgeInfo);

    VaultAgeInfoNode ageInfo(&templateNode);
    ageInfo.SetAgeFilename(kCityAgeFilename);

    hsRef<RelVaultNode> node;
    if (node = rvnFldr->GetChildNode(&templateNode, 2)) {
        VaultAgeInfoNode info(node);
        info.CopyTo(link->GetAgeInfo());
    }

    return node != nullptr;
}

//============================================================================
hsRef<RelVaultNode> VaultGetOwnedAgeLink (const plAgeInfoStruct * info) {
    hsRef<RelVaultNode> rvnLink;

    if (hsRef<RelVaultNode> rvnFldr = VaultGetAgesIOwnFolder()) {

        NetVaultNode templateNode;
        templateNode.SetNodeType(plVault::kNodeType_AgeInfo);

        VaultAgeInfoNode ageInfo(&templateNode);
        if (info->HasAgeFilename()) {
            ageInfo.SetAgeFilename(info->GetAgeFilename());
        }
        if (info->HasAgeInstanceGuid()) {
            ageInfo.SetAgeInstanceGuid(*info->GetAgeInstanceGuid());
        }

        if (hsRef<RelVaultNode> rvnInfo = rvnFldr->GetChildNode(&templateNode, 2)) {
            templateNode.Clear();
            templateNode.SetNodeType(plVault::kNodeType_AgeLink);
            rvnLink = rvnInfo->GetParentNode(&templateNode, 1);
        }
    }
    
    return rvnLink;
}

//============================================================================
hsRef<RelVaultNode> VaultGetOwnedAgeInfo (const plAgeInfoStruct * info) {
    hsRef<RelVaultNode> rvnInfo;
    if (hsRef<RelVaultNode> rvnFldr = VaultGetAgesIOwnFolder()) {
        NetVaultNode templateNode;
        templateNode.SetNodeType(plVault::kNodeType_AgeInfo);

        VaultAgeInfoNode ageInfo(&templateNode);
        if (info->HasAgeFilename()) {
            ageInfo.SetAgeFilename(info->GetAgeFilename());
        }
        if (info->HasAgeInstanceGuid()) {
            ageInfo.SetAgeInstanceGuid(*info->GetAgeInstanceGuid());
        }

        rvnInfo = rvnFldr->GetChildNode(&templateNode, 2);
    }
    return rvnInfo;
}

//============================================================================
bool VaultGetOwnedAgeLink (const plAgeInfoStruct * info, plAgeLinkStruct * link) {
    bool result = false;
    if (hsRef<RelVaultNode> rvnLink = VaultGetOwnedAgeLink(info)) {
        if (hsRef<RelVaultNode> rvnInfo = rvnLink->GetChildNode(plVault::kNodeType_AgeInfo, 1)) {
            VaultAgeInfoNode ageInfo(rvnInfo);
            ageInfo.CopyTo(link->GetAgeInfo());
            result = true;
        }
    }
    
    return result;
}

//============================================================================
bool VaultFindOrCreateChildAgeLinkAndWait (const wchar_t ownedAgeName[], const plAgeInfoStruct * info, plAgeLinkStruct * link) {
    hsAssert(false, "eric, implement me");
    return false;
}

//============================================================================
bool VaultAddOwnedAgeSpawnPoint (const plUUID& ageInstId, const plSpawnPointInfo & spawnPt) {
    
    hsRef<RelVaultNode> fldr, link;
    
    for (;;) {
        if (spawnPt.GetName().empty())
            break;
        if (spawnPt.GetTitle().empty())
            break;

        fldr = VaultGetAgesIOwnFolder();
        if (!fldr)
            break;

        std::vector<unsigned> nodeIds;
        fldr->GetChildNodeIds(&nodeIds, 1);

        NetVaultNode templateNode;
        templateNode.SetNodeType(plVault::kNodeType_AgeInfo);
        VaultAgeInfoNode access(&templateNode);
        access.SetAgeInstanceGuid(ageInstId);
        
        for (unsigned nodeId : nodeIds) {
            link = VaultGetNode(nodeId);
            if (!link)
                continue;
            if (link->GetChildNode(&templateNode, 1)) {
                VaultAgeLinkNode access(link);
                access.AddSpawnPoint(spawnPt);
                link = nullptr;
                break;
            }
        }

        break;  
    }       

    return true;
}

//============================================================================
bool VaultSetOwnedAgePublicAndWait (const plAgeInfoStruct * info, bool publicOrNot) {
    if (hsRef<RelVaultNode> rvnLink = VaultGetOwnedAgeLink(info)) {
        if (hsRef<RelVaultNode> rvnInfo = rvnLink->GetChildNode(plVault::kNodeType_AgeInfo, 1)) {
            VaultSetAgePublicAndWait(rvnInfo, publicOrNot);
        }
    }
    return true;
}

//============================================================================
bool VaultSetAgePublicAndWait(hsWeakRef<NetVaultNode> ageInfoNode, bool publicOrNot) {
    NetCliAuthSetAgePublic(ageInfoNode->GetNodeId(), publicOrNot);

    VaultAgeInfoNode access(ageInfoNode);

    plVaultNotifyMsg * msg = new plVaultNotifyMsg;
    if (publicOrNot)
        msg->SetType(plVaultNotifyMsg::kPublicAgeCreated);
    else
        msg->SetType(plVaultNotifyMsg::kPublicAgeRemoved);
    msg->SetResultCode(kNetSuccess);
    msg->GetArgs()->AddString(plNetCommon::VaultTaskArgs::kAgeFilename, access.GetAgeFilename().c_str());
    msg->Send();
    return true;
}

//============================================================================
hsRef<RelVaultNode> VaultGetVisitAgeLink (const plAgeInfoStruct * info) {
    hsRef<RelVaultNode> rvnLink;
    if (hsRef<RelVaultNode> rvnFldr = VaultGetAgesICanVisitFolder()) {
        NetVaultNode templateNode;
        templateNode.SetNodeType(plVault::kNodeType_AgeInfo);

        VaultAgeInfoNode ageInfo(&templateNode);
        if (info->HasAgeFilename()) {
            ageInfo.SetAgeFilename(info->GetAgeFilename());
        }
        if (info->HasAgeInstanceGuid()) {
            ageInfo.SetAgeInstanceGuid(*info->GetAgeInstanceGuid());
        }

        if (hsRef<RelVaultNode> rvnInfo = rvnFldr->GetChildNode(&templateNode, 2)) {
            templateNode.Clear();
            templateNode.SetNodeType(plVault::kNodeType_AgeLink);
            rvnLink = rvnInfo->GetParentNode(&templateNode, 1);
        }
    }
    
    return rvnLink;
}

//============================================================================
bool VaultGetVisitAgeLink (const plAgeInfoStruct * info, class plAgeLinkStruct * link) {
    hsRef<RelVaultNode> rvn = VaultGetVisitAgeLink(info);
    if (!rvn)
        return false;
        
    VaultAgeLinkNode ageLink(rvn);
    ageLink.CopyTo(link);
    return true;
}

//============================================================================
namespace _VaultRegisterOwnedAgeAndWait {

struct _InitAgeParam {
    ENetError       result;
    bool            complete;
    unsigned        ageInfoId;
};
static void _InitAgeCallback (
    ENetError       result,
    void *          ,
    void *          vparam,
    unsigned        ageVaultId,
    unsigned        ageInfoVaultId
) {
    _InitAgeParam * param = (_InitAgeParam *)vparam;
    param->ageInfoId    = ageInfoVaultId;
    param->result       = result;
    param->complete     = true;
}
struct _FetchVaultParam {
    ENetError       result;
    bool            complete;
};
static void _FetchVaultCallback (
    ENetError       result,
    void *          vparam
) {
    _FetchVaultParam * param = (_FetchVaultParam *)vparam;
    param->result       = result;
    param->complete     = true;
}
struct _CreateNodeParam {
    ENetError       result;
    bool            complete;
    unsigned        nodeId;
};
static void _CreateNodeCallback (
    ENetError       result,
    void *          ,
    void *          vparam,
    hsWeakRef<RelVaultNode> node
) {
    _CreateNodeParam * param = (_CreateNodeParam *)vparam;
    if (IS_NET_SUCCESS(result))
        param->nodeId = node->GetNodeId();
    param->result       = result;
    param->complete     = true;
}
struct _AddChildNodeParam {
    ENetError       result;
    bool            complete;
};
static void _AddChildNodeCallback (
    ENetError       result,
    void *          vparam
) {
    _AddChildNodeParam * param = (_AddChildNodeParam *)vparam;
    param->result       = result;
    param->complete     = true;
}

} // namespace _VaultRegisterOwnedAgeAndWait

//============================================================================
bool VaultRegisterOwnedAgeAndWait (const plAgeLinkStruct * link) {
    using namespace _VaultRegisterOwnedAgeAndWait;

    unsigned ageLinkId = 0;
    unsigned ageInfoId;
    unsigned agesIOwnId;
    
    ENetError result = kNetPending;
    
    for (;;) {
        if (hsRef<RelVaultNode> rvn = VaultGetAgesIOwnFolder())
            agesIOwnId = rvn->GetNodeId();
        else {
            LogMsg(kLogError, "RegisterOwnedAge: Failed to get player's AgesIOwnFolder");
            result = kNetErrVaultNodeNotFound;
            break;
        }

        // Check for existing link to this age  
        plAgeLinkStruct existing;
        if (VaultGetOwnedAgeLink(link->GetAgeInfo(), &existing)) {
            result = kNetSuccess;
            break;
        }
        
        {   // Init age vault
            _InitAgeParam   param;
            memset(&param, 0, sizeof(param));

            VaultInitAge(
                link->GetAgeInfo(),
                kNilUuid,
                _InitAgeCallback,
                nullptr,
                &param
            );

            while (!param.complete) {
                NetClientUpdate();
                plgDispatch::Dispatch()->MsgQueueProcess();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            
            if (IS_NET_ERROR(param.result)) {
                LogMsg(kLogError, "RegisterOwnedAge: Failed to init age {}", link->GetAgeInfo()->GetAgeFilename());
                result = param.result;
                break;
            }
                
            ageInfoId = param.ageInfoId;
        }       
        
        {   // Create age link
            _CreateNodeParam    param;
            memset(&param, 0, sizeof(param));

            VaultCreateNode(
                plVault::kNodeType_AgeLink,
                _CreateNodeCallback,
                nullptr,
                &param
            );

            while (!param.complete) {
                NetClientUpdate();
                plgDispatch::Dispatch()->MsgQueueProcess();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            
            if (IS_NET_ERROR(param.result)) {
                LogMsg(kLogError, "RegisterOwnedAge: Failed create age link node");
                result = param.result;
                break;
            }
                
            ageLinkId = param.nodeId;
        }       

        {   // Fetch age info node tree
            _FetchVaultParam    param;
            memset(&param, 0, sizeof(param));
            
            VaultDownload(
                "RegisterOwnedAge",
                ageInfoId,
                _FetchVaultCallback,
                &param,
                nullptr,
                nullptr
            );
            
            while (!param.complete) {
                NetClientUpdate();
                plgDispatch::Dispatch()->MsgQueueProcess();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            
            if (IS_NET_ERROR(param.result)) {
                LogMsg(kLogError, "RegisterOwnedAge: Failed to download age info vault");
                result = param.result;
                break;
            }
        }

        { // Link:
            // ageLink to player's bookshelf folder
            // ageInfo to ageLink
            // playerInfo to ageOwners
            _AddChildNodeParam  param1;
            _AddChildNodeParam  param2;
            _AddChildNodeParam  param3;
            memset(&param1, 0, sizeof(param1));
            memset(&param2, 0, sizeof(param2));
            memset(&param3, 0, sizeof(param3));

            unsigned ageOwnersId = 0;       
            if (hsRef<RelVaultNode> rvnAgeInfo = VaultGetNode(ageInfoId)) {
                if (hsRef<RelVaultNode> rvnAgeOwners = rvnAgeInfo->GetChildPlayerInfoListNode(plVault::kAgeOwnersFolder, 1))
                    ageOwnersId = rvnAgeOwners->GetNodeId();
            }
            
            unsigned playerInfoId = 0;
            if (hsRef<RelVaultNode> rvnPlayerInfo = VaultGetPlayerInfoNode())
                playerInfoId = rvnPlayerInfo->GetNodeId();

            VaultAddChildNode(
                agesIOwnId,
                ageLinkId,
                0,
                _AddChildNodeCallback,
                &param1
            );

            VaultAddChildNode(
                ageLinkId,
                ageInfoId,
                0,
                _AddChildNodeCallback,
                &param2
            );

            VaultAddChildNode(
                ageOwnersId,
                playerInfoId,
                0,
                _AddChildNodeCallback,
                &param3
            );

            while (!param1.complete && !param2.complete && !param3.complete) {
                NetClientUpdate();
                plgDispatch::Dispatch()->MsgQueueProcess();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            
            if (IS_NET_ERROR(param1.result)) {
                LogMsg(kLogError, "RegisterOwnedAge: Failed to add link to player's bookshelf");
                result = param1.result;
                break;
            }
            if (IS_NET_ERROR(param2.result)) {
                LogMsg(kLogError, "RegisterOwnedAge: Failed to add info to link");
                result = param2.result;
                break;
            }
            if (IS_NET_ERROR(param3.result)) {
                LogMsg(kLogError, "RegisterOwnedAge: Failed to add playerInfo to ageOwners");
                result = param3.result;
                break;
            }
        }

        // Copy the link spawn point to the link node       
        if (hsRef<RelVaultNode> node = VaultGetNode(ageLinkId)) {
            VaultAgeLinkNode access(node);
            access.AddSpawnPoint(link->SpawnPoint());
        }
        
        result = kNetSuccess;
        break;
    }

    hsAssert(result != kNetPending, "Result code was not set");

    plVaultNotifyMsg * msg = new plVaultNotifyMsg;
    msg->SetType(plVaultNotifyMsg::kRegisteredOwnedAge);
    msg->SetResultCode(result);
    msg->GetArgs()->AddInt(plNetCommon::VaultTaskArgs::kAgeLinkNode, ageLinkId);
    msg->Send();

    return IS_NET_SUCCESS(result);
}

//============================================================================
namespace _VaultRegisterOwnedAge {
    struct _Params {
        plSpawnPointInfo* fSpawn;
        void*           fAgeInfoId;

        ~_Params() {
            delete fSpawn;
        }
    };

    void _AddAgeInfoNode(ENetError result, void* param) {
        if (IS_NET_ERROR(result))
            LogMsg(kLogError, "VaultRegisterOwnedAge: Failed to add info to link (async)");
    }

    void _AddAgeLinkNode(ENetError result, void* param) {
        if (IS_NET_ERROR(result))
            LogMsg(kLogError, "VaultRegisterOwnedAge: Failed to add age to bookshelf (async)");
    }

    void _AddPlayerInfoNode(ENetError result, void* param) {
        if (IS_NET_ERROR(result))
            LogMsg(kLogError, "VaultRegisterOwnedAge: Failed to add playerInfo to ageOwners (async)");
    }

    void _CreateAgeLinkNode(ENetError result, void* state, void* param, hsWeakRef<RelVaultNode> node) {
        if (IS_NET_ERROR(result)) {
            LogMsg(kLogError, "VaultRegisterOwnedAge: Failed to create AgeLink (async)");
            delete (_Params*)param;
            return;
        }

        // Grab our params
        _Params* p = (_Params*)param;

        // Set swpoint
        VaultAgeLinkNode aln(node);
        aln.AddSpawnPoint(*(p->fSpawn));

        // Make some refs
        hsRef<RelVaultNode> agesIOwn = VaultGetAgesIOwnFolder();
        hsRef<RelVaultNode> plyrInfo = VaultGetPlayerInfoNode();
        VaultAddChildNode(agesIOwn->GetNodeId(), node->GetNodeId(), 0, (FVaultAddChildNodeCallback)_AddAgeLinkNode, nullptr);
        VaultAddChildNode(node->GetNodeId(), (uint32_t)((uintptr_t)p->fAgeInfoId), 0, (FVaultAddChildNodeCallback)_AddAgeInfoNode, nullptr);

        // Add our PlayerInfo to important places
        if (hsRef<RelVaultNode> rvnAgeInfo = VaultGetNode((uint32_t)((uintptr_t)p->fAgeInfoId))) {
            if (hsRef<RelVaultNode> rvnAgeOwners = rvnAgeInfo->GetChildPlayerInfoListNode(plVault::kAgeOwnersFolder, 1))
                VaultAddChildNode(rvnAgeOwners->GetNodeId(), plyrInfo->GetNodeId(), 0, (FVaultAddChildNodeCallback)_AddPlayerInfoNode, nullptr);
        }

        // Fire off vault callbacks
        plVaultNotifyMsg* msg = new plVaultNotifyMsg;
        msg->SetType(plVaultNotifyMsg::kRegisteredOwnedAge);
        msg->SetResultCode(result);
        msg->GetArgs()->AddInt(plNetCommon::VaultTaskArgs::kAgeLinkNode, node->GetNodeId());
        msg->Send();

        // Don't leak memory
        delete p;
    }

    void _DownloadCallback(ENetError result, void* param) {
        if (IS_NET_ERROR(result)) {
            LogMsg(kLogError, "VaultRegisterOwnedAge: Failed to download age vault (async)");
            delete (_Params*)param;
        } else
            VaultCreateNode(plVault::kNodeType_AgeLink, (FVaultCreateNodeCallback)_CreateAgeLinkNode, nullptr, param);
    }

    void _InitAgeCallback(ENetError result, void* state, void* param, uint32_t ageVaultId, uint32_t ageInfoVaultId) {
        if (IS_NET_SUCCESS(result)) {
            _Params* p = new _Params();
            p->fAgeInfoId = (void*)(uintptr_t)ageInfoVaultId;
            p->fSpawn = (plSpawnPointInfo*)param;

            VaultDownload(
                "RegisterOwnedAge",
                ageInfoVaultId,
                (FVaultDownloadCallback)_DownloadCallback,
                p,
                nullptr,
                nullptr);
        } else
            LogMsg(kLogError, "VaultRegisterOwnedAge: Failed to init age (async)");
    }
}; // namespace _VaultRegisterOwnedAge

void VaultRegisterOwnedAge(const plAgeLinkStruct* link) {
    using namespace _VaultRegisterOwnedAge;

    hsRef<RelVaultNode> agesIOwn = VaultGetAgesIOwnFolder();
    if (agesIOwn == nullptr) {
        LogMsg(kLogError, "VaultRegisterOwnedAge: Couldn't find the stupid AgesIOwnfolder!");
        return;
    }

    // Make sure we don't already have the age
    plAgeLinkStruct existing;
    if (VaultGetOwnedAgeLink(link->GetAgeInfo(), &existing))
        return;

    // Let's go async, my friend :)
    VaultInitAge(link->GetAgeInfo(), 
        kNilUuid, 
        (FVaultInitAgeCallback)_InitAgeCallback, 
        nullptr,
        new plSpawnPointInfo(link->SpawnPoint()));
}

//============================================================================
namespace _VaultRegisterVisitAgeAndWait {

struct _InitAgeParam {
    ENetError       result;
    bool            complete;
    unsigned        ageInfoId;
};
static void _InitAgeCallback (
    ENetError       result,
    void *          ,
    void *          vparam,
    unsigned        ageVaultId,
    unsigned        ageInfoVaultId
) {
    _InitAgeParam * param = (_InitAgeParam *)vparam;
    param->ageInfoId    = ageInfoVaultId;
    param->result       = result;
    param->complete     = true;
}
struct _FetchVaultParam {
    ENetError       result;
    bool            complete;
};
static void _FetchVaultCallback (
    ENetError       result,
    void *          vparam
) {
    _FetchVaultParam * param = (_FetchVaultParam *)vparam;
    param->result       = result;
    param->complete     = true;
}
struct _CreateNodeParam {
    ENetError       result;
    bool            complete;
    unsigned        nodeId;
};
static void _CreateNodeCallback (
    ENetError       result,
    void *          ,
    void *          vparam,
    hsWeakRef<RelVaultNode> node
) {
    _CreateNodeParam * param = (_CreateNodeParam *)vparam;
    if (IS_NET_SUCCESS(result))
        param->nodeId = node->GetNodeId();
    param->result       = result;
    param->complete     = true;
}
struct _AddChildNodeParam {
    ENetError       result;
    bool            complete;
};
static void _AddChildNodeCallback (
    ENetError       result,
    void *          vparam
) {
    _AddChildNodeParam * param = (_AddChildNodeParam *)vparam;
    param->result       = result;
    param->complete     = true;
}

} // namespace _VaultRegisterVisitAgeAndWait

//============================================================================
bool VaultRegisterVisitAgeAndWait (const plAgeLinkStruct * link) {
    using namespace _VaultRegisterVisitAgeAndWait;

    unsigned ageLinkId = 0;
    unsigned ageInfoId;
    unsigned agesICanVisitId;
    
    ENetError result = kNetPending;
    for (;;) {
        if (hsRef<RelVaultNode> rvn = VaultGetAgesICanVisitFolder())
            agesICanVisitId = rvn->GetNodeId();
        else {
            LogMsg(kLogError, "RegisterVisitAge: Failed to get player's AgesICanVisitFolder");
            result = kNetErrVaultNodeNotFound;
            break;
        }

        // Check for existing link to this age  
        plAgeLinkStruct existing;
        if (VaultGetVisitAgeLink(link->GetAgeInfo(), &existing)) {
            result = kNetSuccess;
            break;
        }
        
        
        {   // Init age vault
            _InitAgeParam   param;
            memset(&param, 0, sizeof(param));

            VaultInitAge(
                link->GetAgeInfo(),
                kNilUuid,
                _InitAgeCallback,
                nullptr,
                &param
            );

            while (!param.complete) {
                NetClientUpdate();
                plgDispatch::Dispatch()->MsgQueueProcess();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            
            if (IS_NET_ERROR(param.result)) {
                LogMsg(kLogError, "RegisterVisitAge: Failed to init age {}", link->GetAgeInfo()->GetAgeFilename());
                result = param.result;
                break;
            }
                
            ageInfoId = param.ageInfoId;
        }       
        
        {   // Create age link
            _CreateNodeParam    param;
            memset(&param, 0, sizeof(param));

            VaultCreateNode(
                plVault::kNodeType_AgeLink,
                _CreateNodeCallback,
                nullptr,
                &param
            );

            while (!param.complete) {
                NetClientUpdate();
                plgDispatch::Dispatch()->MsgQueueProcess();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            
            if (IS_NET_ERROR(param.result)) {
                LogMsg(kLogError, "RegisterVisitAge: Failed create age link node");
                result = param.result;
                break;
            }
                
            ageLinkId = param.nodeId;
        }       

        {   // Fetch age info node tree
            _FetchVaultParam    param;
            memset(&param, 0, sizeof(param));
            
            VaultDownload(
                "RegisterVisitAge",
                ageInfoId,
                _FetchVaultCallback,
                &param,
                nullptr,
                nullptr
            );
            
            while (!param.complete) {
                NetClientUpdate();
                plgDispatch::Dispatch()->MsgQueueProcess();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            
            if (IS_NET_ERROR(param.result)) {
                LogMsg(kLogError, "RegisterVisitAge: Failed to download age info vault");
                result = param.result;
                break;
            }
        }

        { // Link:
            // ageLink to player's "can visit" folder
            // ageInfo to ageLink
            _AddChildNodeParam  param1;
            _AddChildNodeParam  param2;
            _AddChildNodeParam  param3;
            memset(&param1, 0, sizeof(param1));
            memset(&param2, 0, sizeof(param2));
            memset(&param3, 0, sizeof(param3));

            unsigned ageVisitorsId = 0;     
            if (hsRef<RelVaultNode> rvnAgeInfo = VaultGetNode(ageInfoId)) {
                if (hsRef<RelVaultNode> rvnAgeVisitors = rvnAgeInfo->GetChildPlayerInfoListNode(plVault::kCanVisitFolder, 1))
                    ageVisitorsId = rvnAgeVisitors->GetNodeId();
            }
            
            unsigned playerInfoId = 0;
            if (hsRef<RelVaultNode> rvnPlayerInfo = VaultGetPlayerInfoNode())
                playerInfoId = rvnPlayerInfo->GetNodeId();

            VaultAddChildNode(
                agesICanVisitId,
                ageLinkId,
                0,
                _AddChildNodeCallback,
                &param1
            );

            VaultAddChildNode(
                ageLinkId,
                ageInfoId,
                0,
                _AddChildNodeCallback,
                &param2
            );

            VaultAddChildNode(
                ageVisitorsId,
                playerInfoId,
                0,
                _AddChildNodeCallback,
                &param3
            );

            while (!param1.complete && !param2.complete && !param3.complete) {
                NetClientUpdate();
                plgDispatch::Dispatch()->MsgQueueProcess();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            
            if (IS_NET_ERROR(param1.result)) {
                LogMsg(kLogError, "RegisterVisitAge: Failed to add link to folder");
                result = param1.result;
                break;
            }
            if (IS_NET_ERROR(param2.result)) {
                LogMsg(kLogError, "RegisterVisitAge: Failed to add info to link");
                result = param2.result;
                break;
            }
            if (IS_NET_ERROR(param3.result)) {
                LogMsg(kLogError, "RegisterVisitAge: Failed to add playerInfo to canVisit folder");
                result = param3.result;
                break;
            }
        }

        // Copy the link spawn point to the link node       
        if (hsRef<RelVaultNode> node = VaultGetNode(ageLinkId)) {
            VaultAgeLinkNode access(node);
            access.AddSpawnPoint(link->SpawnPoint());
        }

        result = kNetSuccess;
        break;
    }

    hsAssert(result != kNetPending, "Result code was not set");

    plVaultNotifyMsg * msg = new plVaultNotifyMsg;
    msg->SetType(plVaultNotifyMsg::kRegisteredVisitAge);
    msg->SetResultCode(result);
    msg->GetArgs()->AddInt(plNetCommon::VaultTaskArgs::kAgeLinkNode, ageLinkId);
    msg->Send();

    return IS_NET_SUCCESS(result);
}

//============================================================================
namespace _VaultRegisterVisitAge {
    struct _Params {

        plSpawnPointInfo* fSpawn;
        void*             fAgeInfoId;

        ~_Params() {
            delete fSpawn;
        }
    };

    void _CreateAgeLinkNode(ENetError result, void* state, void* param, hsWeakRef<RelVaultNode> node) {
        if (IS_NET_ERROR(result)) {
            LogMsg(kLogError, "RegisterVisitAge: Failed to create AgeLink (async)");
            delete (_Params*)param;
            return;
        }

        _Params* p = (_Params*)param;
        hsRef<RelVaultNode> ageInfo = VaultGetNode((uint32_t)((uintptr_t)p->fAgeInfoId));

        // Add ourselves to the Can Visit folder of the age
        if (hsRef<RelVaultNode> playerInfo = VaultGetPlayerInfoNode()) {
            if (hsRef<RelVaultNode> canVisit = ageInfo->GetChildPlayerInfoListNode(plVault::kCanVisitFolder, 1))
                VaultAddChildNode(canVisit->GetNodeId(), playerInfo->GetNodeId(), 0, nullptr, nullptr);
        }

        // Get our AgesICanVisit folder
        if (hsRef<RelVaultNode> iCanVisit = VaultGetAgesICanVisitFolder()) {
            VaultAddChildNode(node->GetNodeId(), ageInfo->GetNodeId(), 0, nullptr, nullptr);
            VaultAddChildNode(iCanVisit->GetNodeId(), node->GetNodeId(), 0, nullptr, nullptr);
        }

        // Update the AgeLink with a spawn point
        VaultAgeLinkNode access(node);
        access.AddSpawnPoint(*p->fSpawn);

        // Send out the VaultNotify msg
        plVaultNotifyMsg * msg = new plVaultNotifyMsg;
        msg->SetType(plVaultNotifyMsg::kRegisteredVisitAge);
        msg->SetResultCode(result);
        msg->GetArgs()->AddInt(plNetCommon::VaultTaskArgs::kAgeLinkNode, node->GetNodeId());
        msg->Send();

        //Don't leak memory
        delete (_Params*)param;
    }

    void _DownloadCallback(ENetError result, void* param) {
        if (IS_NET_ERROR(result)) {
            LogMsg(kLogError, "RegisterVisitAge: Failed to download age vault (async)");
            delete (_Params*)param;
            return;
        }

        // Create the AgeLink node 
        VaultCreateNode(plVault::kNodeType_AgeLink, (FVaultCreateNodeCallback)_CreateAgeLinkNode, nullptr, param);
    }
    
    void _InitAgeCallback(ENetError result, void* state, void* param, uint32_t ageVaultId, uint32_t ageInfoId) {
        if (IS_NET_ERROR(result)) {
            LogMsg(kLogError, "RegisterVisitAge: Failed to init age vault (async)");
            delete (_Params*)param;
            return;
        }

        // Save the AgeInfo nodeID, then download the age vault
        _Params* p = (_Params*)param;
        p->fAgeInfoId = (void*)(uintptr_t)ageInfoId;
        
        VaultDownload("RegisterVisitAge",
                      ageInfoId,
                      (FVaultDownloadCallback)_DownloadCallback,
                      param,
                      nullptr,
                      nullptr
        );
    }
};

void VaultRegisterVisitAge(const plAgeLinkStruct* link) {
    using namespace _VaultRegisterVisitAge;

    // Test to see if we already have this visit age...
    plAgeLinkStruct existing;
    if (VaultGetVisitAgeLink(link->GetAgeInfo(), &existing))
        return;

    // Still here? We need to actually do some work, then.
    _Params* p = new _Params;
    p->fSpawn = new plSpawnPointInfo(link->SpawnPoint());

    // This doesn't actually *create* a new age but rather fetches the
    // already existing age vault. Weird? Yes...
    VaultInitAge(link->GetAgeInfo(),
                 kNilUuid,
                 (FVaultInitAgeCallback)_InitAgeCallback,
                 nullptr,
                 p
    );
}

//============================================================================
bool VaultUnregisterOwnedAgeAndWait (const plAgeInfoStruct * info) {

    unsigned ageLinkId = 0;
    unsigned agesIOwnId;

    ENetError result = kNetPending;
    for (;;) {  
        hsRef<RelVaultNode> rvnLink = VaultGetOwnedAgeLink(info);
        if (!rvnLink) {
            result = kNetSuccess;
            break;  // we aren't an owner of the age, just return true
        }

        if (hsRef<RelVaultNode> rvn = VaultGetAgesIOwnFolder())
            agesIOwnId = rvn->GetNodeId();
        else {
            LogMsg(kLogError, "UnregisterOwnedAge: Failed to get player's AgesIOwnFolder");
            result = kNetErrVaultNodeNotFound;
            break;  // something's wrong with the player vault, it doesn't have a required folder node
        }

        ageLinkId = rvnLink->GetNodeId();

        unsigned ageOwnersId = 0;
        if (hsRef<RelVaultNode> rvnAgeInfo = rvnLink->GetChildNode(plVault::kNodeType_AgeInfo, 1)) {
            if (hsRef<RelVaultNode> rvnAgeOwners = rvnAgeInfo->GetChildPlayerInfoListNode(plVault::kAgeOwnersFolder, 1))
                ageOwnersId = rvnAgeOwners->GetNodeId();
        }

        unsigned playerInfoId = 0;
        if (hsRef<RelVaultNode> rvnPlayerInfo = VaultGetPlayerInfoNode())
            playerInfoId = rvnPlayerInfo->GetNodeId();

        // remove our playerInfo from the ageOwners folder
        VaultRemoveChildNode(ageOwnersId, playerInfoId, nullptr, nullptr);
        
        // remove the link from AgesIOwn folder 
        VaultRemoveChildNode(agesIOwnId, ageLinkId, nullptr, nullptr);

        // delete the link node since link nodes aren't shared with anyone else
    //  VaultDeleteNode(ageLinkId);

        result = kNetSuccess;
        break;
    }

    hsAssert(result != kNetPending, "Result code was not set");

    plVaultNotifyMsg * msg = new plVaultNotifyMsg;
    msg->SetType(plVaultNotifyMsg::kUnRegisteredOwnedAge);
    msg->SetResultCode(result);
    msg->GetArgs()->AddInt(plNetCommon::VaultTaskArgs::kAgeLinkNode, ageLinkId);
    msg->Send();
    
    return IS_NET_SUCCESS(result);
}

//============================================================================
bool VaultUnregisterVisitAgeAndWait (const plAgeInfoStruct * info) {

    unsigned ageLinkId = 0;
    unsigned agesICanVisitId;
    
    ENetError result = kNetPending;
    for (;;) {
        hsRef<RelVaultNode> rvnLink = VaultGetVisitAgeLink(info);
        if (!rvnLink) {
            result = kNetSuccess;
            break;  // we aren't an owner of the age, just return true
        }

        if (hsRef<RelVaultNode> rvn = VaultGetAgesICanVisitFolder())
            agesICanVisitId = rvn->GetNodeId();
        else {
            LogMsg(kLogError, "UnregisterOwnedAge: Failed to get player's AgesICanVisitFolder");
            result = kNetErrVaultNodeNotFound;
            break;  // something's wrong with the player vault, it doesn't have a required folder node
        }

        ageLinkId = rvnLink->GetNodeId();

        unsigned ageVisitorsId = 0;
        if (hsRef<RelVaultNode> rvnAgeInfo = rvnLink->GetChildNode(plVault::kNodeType_AgeInfo, 1)) {
            if (hsRef<RelVaultNode> rvnAgeVisitors = rvnAgeInfo->GetChildPlayerInfoListNode(plVault::kCanVisitFolder, 1))
                ageVisitorsId = rvnAgeVisitors->GetNodeId();
        }
        
        unsigned playerInfoId = 0;
        if (hsRef<RelVaultNode> rvnPlayerInfo = VaultGetPlayerInfoNode())
            playerInfoId = rvnPlayerInfo->GetNodeId();

        // remove our playerInfo from the ageVisitors folder
        VaultRemoveChildNode(ageVisitorsId, playerInfoId, nullptr, nullptr);

        // remove the link from AgesICanVisit folder    
        VaultRemoveChildNode(agesICanVisitId, ageLinkId, nullptr, nullptr);
        
        // delete the link node since link nodes aren't shared with anyone else
    //  VaultDeleteNode(ageLinkId);
    
        result = kNetSuccess;
        break;
    }

    hsAssert(result != kNetPending, "Result code was not set");

    plVaultNotifyMsg * msg = new plVaultNotifyMsg;
    msg->SetType(plVaultNotifyMsg::kUnRegisteredVisitAge);
    msg->SetResultCode(result);
    msg->GetArgs()->AddInt(plNetCommon::VaultTaskArgs::kAgeLinkNode, ageLinkId);
    msg->Send();

    return IS_NET_SUCCESS(result);
}

//============================================================================
hsRef<RelVaultNode> VaultFindChronicleEntry (const ST::string& entryName, int entryType) {

    hsRef<RelVaultNode> result;
    if (hsRef<RelVaultNode> rvnFldr = GetChildFolderNode(GetPlayerNode(), plVault::kChronicleFolder, 1)) {
        NetVaultNode templateNode;
        templateNode.SetNodeType(plVault::kNodeType_Chronicle);
        VaultChronicleNode chrn(&templateNode);
        chrn.SetEntryName(entryName);
        if (entryType >= 0)
            chrn.SetEntryType(entryType);
        if (hsRef<RelVaultNode> rvnChrn = rvnFldr->GetChildNode(&templateNode, 255))
            result = rvnChrn;
    }
    return result;
}

//============================================================================
bool VaultHasChronicleEntry (const ST::string& entryName, int entryType) {
    if (VaultFindChronicleEntry(entryName, entryType))
        return true;
    return false;
}

//============================================================================
void VaultAddChronicleEntryAndWait (
    const ST::string& entryName,
    int               entryType,
    const ST::string& entryValue
) {
    if (hsRef<RelVaultNode> rvnChrn = VaultFindChronicleEntry(entryName, entryType)) {
        VaultChronicleNode chrnNode(rvnChrn);
        chrnNode.SetEntryValue(entryValue);
    }
    else if (hsRef<RelVaultNode> rvnFldr = GetChildFolderNode(GetPlayerNode(), plVault::kChronicleFolder, 1)) {
        NetVaultNode templateNode;
        templateNode.SetNodeType(plVault::kNodeType_Chronicle);
        VaultChronicleNode chrnNode(&templateNode);
        chrnNode.SetEntryName(entryName);
        chrnNode.SetEntryType(entryType);
        chrnNode.SetEntryValue(entryValue);
        ENetError result;
        if (hsRef<RelVaultNode> rvnChrn = VaultCreateNodeAndWait(&templateNode, &result))
            VaultAddChildNode(rvnFldr->GetNodeId(), rvnChrn->GetNodeId(), 0, nullptr, nullptr);
    }
}

//============================================================================
bool VaultAmIgnoringPlayer (unsigned playerId) {
    bool retval = false;
    if (hsRef<RelVaultNode> rvnFldr = GetChildPlayerInfoListNode(GetPlayerNode(), plVault::kIgnoreListFolder, 1)) {
        NetVaultNode templateNode;
        templateNode.SetNodeType(plVault::kNodeType_PlayerInfo);
        VaultPlayerInfoNode pinfoNode(&templateNode);
        pinfoNode.SetPlayerId(playerId);

        if (rvnFldr->GetChildNode(&templateNode, 1))
            retval = true;
    }

    return retval;
}

//============================================================================
unsigned VaultGetKILevel () {
    hsAssert(false, "eric, implement me");
    return pfKIMsg::kNanoKI;
}

//============================================================================
bool VaultGetCCRStatus () {
    bool retval = false;
    if (hsRef<RelVaultNode> rvnSystem = VaultGetSystemNode()) {
        VaultSystemNode sysNode(rvnSystem);
        retval = (sysNode.GetCCRStatus() != 0);
    }

    return retval;
}

//============================================================================
bool VaultSetCCRStatus (bool online) {
    bool retval = false;
    if (hsRef<RelVaultNode> rvnSystem = VaultGetSystemNode()) {
        VaultSystemNode sysNode(rvnSystem);
        sysNode.SetCCRStatus(online ? 1 : 0);

        retval = true;
    }

    return retval;
}

//============================================================================
void VaultDump (const ST::string& tag, unsigned vaultId) {
    plStatusLog::AddLineSF("VaultClient.log", "<---- ID:{}, Begin Vault {} ---->", vaultId, tag);

    if (hsRef<RelVaultNode> rvn = VaultGetNode(vaultId))
        rvn->PrintTree(0);

    plStatusLog::AddLineSF("VaultClient.log", "<---- ID:{}, End Vault {} ---->", vaultId, tag);
}

//============================================================================
bool VaultAmInMyPersonalAge () {
    bool result = false;

    plAgeInfoStruct info;
    info.SetAgeFilename(kPersonalAgeFilename);

    if (hsRef<RelVaultNode> rvnLink = VaultGetOwnedAgeLink(&info)) {
        if (hsRef<RelVaultNode> rvnInfo = rvnLink->GetChildNode(plVault::kNodeType_AgeInfo, 1)) {
            VaultAgeInfoNode ageInfo(rvnInfo);

            if (hsRef<RelVaultNode> currentAgeInfoNode = VaultGetAgeInfoNode()) {
                VaultAgeInfoNode curAgeInfo(currentAgeInfoNode);

                if (ageInfo.GetAgeInstanceGuid() == curAgeInfo.GetAgeInstanceGuid())
                    result = true;
            }
        }
    }

    return result;
}

//============================================================================
bool VaultAmInMyNeighborhoodAge () {
    bool result = false;

    plAgeInfoStruct info;
    info.SetAgeFilename(kNeighborhoodAgeFilename);

    if (hsRef<RelVaultNode> rvnLink = VaultGetOwnedAgeLink(&info)) {
        if (hsRef<RelVaultNode> rvnInfo = rvnLink->GetChildNode(plVault::kNodeType_AgeInfo, 1)) {
            VaultAgeInfoNode ageInfo(rvnInfo);

            if (hsRef<RelVaultNode> currentAgeInfoNode = VaultGetAgeInfoNode()) {
                VaultAgeInfoNode curAgeInfo(currentAgeInfoNode);

                if (ageInfo.GetAgeInstanceGuid() == curAgeInfo.GetAgeInstanceGuid())
                    result = true;
            }
        }
    }

    return result;
}

//============================================================================
bool VaultAmOwnerOfCurrentAge () {
    bool result = false;

    if (hsRef<RelVaultNode> currentAgeInfoNode = VaultGetAgeInfoNode()) {
        VaultAgeInfoNode curAgeInfo(currentAgeInfoNode);

        plAgeInfoStruct info;
        info.SetAgeFilename(curAgeInfo.GetAgeFilename());

        if (hsRef<RelVaultNode> rvnLink = VaultGetOwnedAgeLink(&info)) {

            if (hsRef<RelVaultNode> rvnInfo = rvnLink->GetChildNode(plVault::kNodeType_AgeInfo, 1)) {
                VaultAgeInfoNode ageInfo(rvnInfo);

                if (ageInfo.GetAgeInstanceGuid() == curAgeInfo.GetAgeInstanceGuid())
                    result = true;
            }
        }
    }

    return result;
}

//============================================================================
bool VaultAmCzarOfCurrentAge () {
    hsAssert(false, "eric, implement me");
    return true;
}

//============================================================================
bool VaultAmOwnerOfAge (const plUUID& ageInstId) {
    hsAssert(false, "eric, implement me");
    return true;
}

//============================================================================
bool VaultAmCzarOfAge (const plUUID& ageInstId) {
//  hsAssert(false, "eric, implement me");
    return false;
}

//============================================================================
bool VaultRegisterMTStationAndWait (
    const ST::string& stationName,
    const ST::string& linkBackSpawnPtObjName
) {
    plAgeInfoStruct info;
    info.SetAgeFilename(kCityAgeFilename);
    if (hsRef<RelVaultNode> rvn = VaultGetOwnedAgeLink(&info)) {
        VaultAgeLinkNode link(rvn);
        link.AddSpawnPoint({ stationName, linkBackSpawnPtObjName });
        return true;
    }
    return false;
}

//============================================================================
void VaultProcessVisitNote(hsWeakRef<RelVaultNode> rvnVisit) {
    if (hsRef<RelVaultNode> rvnInbox = VaultGetPlayerInboxFolder()) {
        VaultTextNoteNode visitAcc(rvnVisit);
        plAgeLinkStruct link;
        if (visitAcc.GetVisitInfo(link.GetAgeInfo())) {
            // Add it to our "ages i can visit" folder
            VaultRegisterVisitAge(&link);
        }
        // remove it from the inbox
        VaultRemoveChildNode(rvnInbox->GetNodeId(), rvnVisit->GetNodeId(), nullptr, nullptr);
    }
}

//============================================================================
void VaultProcessUnvisitNote(hsWeakRef<RelVaultNode> rvnUnVisit) {
    if (hsRef<RelVaultNode> rvnInbox = VaultGetPlayerInboxFolder()) {
        VaultTextNoteNode unvisitAcc(rvnUnVisit);
        plAgeInfoStruct info;
        if (unvisitAcc.GetVisitInfo(&info)) {
            // Remove it from our "ages i can visit" folder
            VaultUnregisterVisitAgeAndWait(&info);
        }
        // remove it from the inbox
        VaultRemoveChildNode(rvnInbox->GetNodeId(), rvnUnVisit->GetNodeId(), nullptr, nullptr);
    }
}

//============================================================================
void VaultProcessPlayerInbox () {
    if (hsRef<RelVaultNode> rvnInbox = VaultGetPlayerInboxFolder()) {
        {   // Process new visit requests
            RelVaultNode::RefList visits;
            RelVaultNode templateNode;
            templateNode.SetNodeType(plVault::kNodeType_TextNote);
            VaultTextNoteNode tmpAcc(&templateNode);
            tmpAcc.SetNoteType(plVault::kNoteType_Visit);
            rvnInbox->GetChildNodes(&templateNode, 1, &visits);

            for (const hsRef<RelVaultNode> &rvnVisit : visits) {
                VaultTextNoteNode visitAcc(rvnVisit);
                plAgeLinkStruct link;
                if (visitAcc.GetVisitInfo(link.GetAgeInfo())) {
                    // Add it to our "ages i can visit" folder
                    VaultRegisterVisitAge(&link);
                }
                // remove it from the inbox
                VaultRemoveChildNode(rvnInbox->GetNodeId(), rvnVisit->GetNodeId(), nullptr, nullptr);
            }
        }
        {   // Process new unvisit requests
            RelVaultNode::RefList unvisits;
            NetVaultNode templateNode;
            templateNode.SetNodeType(plVault::kNodeType_TextNote);
            VaultTextNoteNode tmpAcc(&templateNode);
            tmpAcc.SetNoteType(plVault::kNoteType_UnVisit);
            rvnInbox->GetChildNodes(&templateNode, 1, &unvisits);

            for (const hsRef<RelVaultNode> &rvnUnVisit : unvisits) {
                VaultTextNoteNode unvisitAcc(rvnUnVisit);
                plAgeInfoStruct info;
                if (unvisitAcc.GetVisitInfo(&info)) {
                    // Remove it from our "ages i can visit" folder
                    VaultUnregisterVisitAgeAndWait(&info);
                }
                // remove it from the inbox
                VaultRemoveChildNode(rvnInbox->GetNodeId(), rvnUnVisit->GetNodeId(), nullptr, nullptr);
            }
        }
    }
}


/*****************************************************************************
*
*   Exports - Age Vault Access
*
***/

//============================================================================
static hsRef<RelVaultNode> GetAgeNode () {
    NetVaultNode templateNode;
    templateNode.SetNodeType(plVault::kNodeType_VNodeMgrAge);
    if (NetCommGetAge())
        templateNode.SetNodeId(NetCommGetAge()->ageVaultId);
    return VaultGetNode(&templateNode);
}

//============================================================================
hsRef<RelVaultNode> VaultGetAgeNode () {
    NetVaultNode templateNode;
    templateNode.SetNodeType(plVault::kNodeType_VNodeMgrAge);
    if (NetCommGetAge())
        templateNode.SetNodeId(NetCommGetAge()->ageVaultId);
    return VaultGetNode(&templateNode);
}

//============================================================================
static hsRef<RelVaultNode> GetAgeInfoNode () {
    hsRef<RelVaultNode> rvnAge = VaultGetAgeNode();
    if (!rvnAge)
        return nullptr;

    NetVaultNode templateNode;
    templateNode.SetNodeType(plVault::kNodeType_AgeInfo);
    templateNode.SetCreatorId(rvnAge->GetNodeId());
    return rvnAge->GetChildNode(&templateNode, 1);
}

//============================================================================
hsRef<RelVaultNode> VaultGetAgeInfoNode () {
    hsRef<RelVaultNode> rvnAge = VaultGetAgeNode();
    if (!rvnAge)
        return nullptr;

    NetVaultNode templateNode;
    templateNode.SetNodeType(plVault::kNodeType_AgeInfo);
    templateNode.SetCreatorId(rvnAge->GetNodeId());
    return rvnAge->GetChildNode(&templateNode, 1);
}

//============================================================================
hsRef<RelVaultNode> VaultGetAgeChronicleFolder () {
    if (hsRef<RelVaultNode> rvn = GetAgeNode())
        return rvn->GetChildFolderNode(plVault::kChronicleFolder, 1);
    return nullptr;
}

//============================================================================
hsRef<RelVaultNode> VaultGetAgeDevicesFolder () {
    if (hsRef<RelVaultNode> rvn = GetAgeNode())
        return rvn->GetChildFolderNode(plVault::kAgeDevicesFolder, 1);
    return nullptr;
}

//============================================================================
hsRef<RelVaultNode> VaultGetAgeAgeOwnersFolder () {
    if (hsRef<RelVaultNode> rvn = GetAgeInfoNode())
        return rvn->GetChildPlayerInfoListNode(plVault::kAgeOwnersFolder, 1);
    return nullptr;
}

//============================================================================
hsRef<RelVaultNode> VaultGetAgeCanVisitFolder () {
    if (hsRef<RelVaultNode> rvn = GetAgeInfoNode())
        return rvn->GetChildPlayerInfoListNode(plVault::kCanVisitFolder, 1);
    return nullptr;
}

//============================================================================
hsRef<RelVaultNode> VaultGetAgePeopleIKnowAboutFolder () {
    if (hsRef<RelVaultNode> rvn = GetAgeNode())
        return rvn->GetChildPlayerInfoListNode(plVault::kPeopleIKnowAboutFolder, 1);
    return nullptr;
}

//============================================================================
hsRef<RelVaultNode> VaultGetAgeSubAgesFolder () {
    if (hsRef<RelVaultNode> rvn = GetAgeNode())
        return rvn->GetChildAgeInfoListNode(plVault::kSubAgesFolder, 1);
    return nullptr;
}

//============================================================================
hsRef<RelVaultNode> VaultGetAgePublicAgesFolder () {
    hsAssert(false, "eric, implement me");
    return nullptr;
}

//============================================================================
hsRef<RelVaultNode> VaultAgeGetBookshelfFolder () {
    if (hsRef<RelVaultNode> rvn = GetAgeNode())
        return rvn->GetChildAgeInfoListNode(plVault::kAgesIOwnFolder, 1);
    return nullptr;
}

//============================================================================
hsRef<RelVaultNode> VaultFindAgeSubAgeLink (const plAgeInfoStruct * info) {
    hsRef<RelVaultNode> rvnLink;
    if (hsRef<RelVaultNode> rvnFldr = VaultGetAgeSubAgesFolder()) {
        NetVaultNode templateNode;
        templateNode.SetNodeType(plVault::kNodeType_AgeInfo);

        VaultAgeInfoNode ageInfo(&templateNode);
        ageInfo.SetAgeFilename(info->GetAgeFilename());

        if (hsRef<RelVaultNode> rvnInfo = rvnFldr->GetChildNode(&templateNode, 2)) {
            templateNode.Clear();
            templateNode.SetNodeType(plVault::kNodeType_AgeLink);
            rvnLink = rvnInfo->GetParentNode(&templateNode, 1);
        }
    }
    
    return rvnLink;
}

//============================================================================
hsRef<RelVaultNode> VaultFindAgeChronicleEntry(const ST::string& entryName, int entryType) {
    hsAssert(false, "eric, implement me");
    return nullptr;
}

//============================================================================
void VaultAddAgeChronicleEntry (
    const ST::string& entryName,
    int               entryType,
    const ST::string& entryValue
) {
    hsAssert(false, "eric, implement me");
}

//============================================================================
hsRef<RelVaultNode> VaultAgeAddDeviceAndWait (const ST::string& deviceName) {
    if (hsRef<RelVaultNode> existing = VaultAgeGetDevice(deviceName))
        return existing;
        
    hsRef<RelVaultNode> device, folder;
    
    for (;;) {
        folder = VaultGetAgeDevicesFolder();
        if (!folder)
            break;

        ENetError result;
        device = VaultCreateNodeAndWait(plVault::kNodeType_TextNote, &result);
        if (!device)
            break;

        VaultTextNoteNode access(device);
        access.SetNoteType(plVault::kNoteType_Device);
        access.SetNoteTitle(deviceName);

        VaultAddChildNodeAndWait(folder->GetNodeId(), device->GetNodeId(), 0);
        break;
    }

    return device;
}

//============================================================================
void VaultAgeRemoveDevice (const ST::string& deviceName) {
    if (hsRef<RelVaultNode> folder = VaultGetAgeDevicesFolder()) {
        NetVaultNode templateNode;
        templateNode.SetNodeType(plVault::kNodeType_TextNote);
        VaultTextNoteNode access(&templateNode);
        access.SetNoteTitle(deviceName);
        if (hsRef<RelVaultNode> device = folder->GetChildNode(&templateNode, 1)) {
            VaultRemoveChildNode(folder->GetNodeId(), device->GetNodeId(), nullptr, nullptr);

            auto it = s_ageDeviceInboxes.find(deviceName);
            if (it != s_ageDeviceInboxes.end())
                s_ageDeviceInboxes.erase(it);
        }
    }
}

//============================================================================
bool VaultAgeHasDevice (const ST::string& deviceName) {
    if (hsRef<RelVaultNode> folder = VaultGetAgeDevicesFolder()) {
        NetVaultNode templateNode;
        templateNode.SetNodeType(plVault::kNodeType_TextNote);
        VaultTextNoteNode access(&templateNode);
        access.SetNoteTitle(deviceName);
        if (folder->GetChildNode(&templateNode, 1))
            return true;
    }
    return false;
}

//============================================================================
hsRef<RelVaultNode> VaultAgeGetDevice (const ST::string& deviceName) {
    hsRef<RelVaultNode> result;
    if (hsRef<RelVaultNode> folder = VaultGetAgeDevicesFolder()) {
        NetVaultNode templateNode;
        templateNode.SetNodeType(plVault::kNodeType_TextNote);
        VaultTextNoteNode access(&templateNode);
        access.SetNoteTitle(deviceName);
        if (hsRef<RelVaultNode> device = folder->GetChildNode(&templateNode, 1))
            result = device;
    }
    return result;
}

//============================================================================
hsRef<RelVaultNode> VaultAgeSetDeviceInboxAndWait (const ST::string& deviceName, const ST::string& inboxName) {
    s_ageDeviceInboxes[deviceName] = inboxName;

    // if we found the inbox or its a global inbox then return here, otherwise if its the default inbox and
    // it wasn't found then continue on and create the inbox
    hsRef<RelVaultNode> existing = VaultAgeGetDeviceInbox(deviceName);
    if (existing || inboxName != DEFAULT_DEVICE_INBOX)
        return existing;

    hsRef<RelVaultNode> device, inbox;

    for (;;) {
        device = VaultAgeGetDevice(deviceName);
        if (!device)
            break;

        ENetError result;
        inbox = VaultCreateNodeAndWait(plVault::kNodeType_Folder, &result);
        if (!inbox)
            break;
            
        VaultFolderNode access(inbox);
        access.SetFolderName(inboxName);
        access.SetFolderType(plVault::kDeviceInboxFolder);

        VaultAddChildNodeAndWait(device->GetNodeId(), inbox->GetNodeId(), 0);
        break;
    }

    return inbox;
}

//============================================================================
hsRef<RelVaultNode> VaultAgeGetDeviceInbox (const ST::string& deviceName) {
    hsRef<RelVaultNode> result;
    auto it = s_ageDeviceInboxes.find(deviceName);

    if (it != s_ageDeviceInboxes.end()) {
        hsRef<RelVaultNode> parentNode;

        if (it->second == DEFAULT_DEVICE_INBOX)
            parentNode = VaultAgeGetDevice(deviceName);
        else
            parentNode = VaultGetGlobalInbox();

        if (parentNode) {
            NetVaultNode templateNode;
            templateNode.SetNodeType(plVault::kNodeType_Folder);
            VaultFolderNode access(&templateNode);
            access.SetFolderType(plVault::kDeviceInboxFolder);
            access.SetFolderName(it->second);
            result = parentNode->GetChildNode(&templateNode, 1);
        }
    }
    return result;
}

//============================================================================
void VaultClearDeviceInboxMap () {
    s_ageDeviceInboxes.clear();
}

//============================================================================
bool VaultAgeGetAgeSDL (plStateDataRecord * out) {
    bool result = false;
    if (hsRef<RelVaultNode> rvn = VaultGetAgeInfoNode()) {
        if (hsRef<RelVaultNode> rvnSdl = rvn->GetChildNode(plVault::kNodeType_SDL, 1)) {
            VaultSDLNode sdl(rvnSdl);
            result = sdl.GetStateDataRecord(out, plSDL::kKeepDirty);
            if (!result) {
                sdl.InitStateDataRecord(sdl.GetSDLName());
                result = sdl.GetStateDataRecord(out, plSDL::kKeepDirty);
            }
        }
    }
    return result;
}

//============================================================================
void VaultAgeUpdateAgeSDL (const plStateDataRecord * rec) {
    if (hsRef<RelVaultNode> rvn = VaultGetAgeInfoNode()) {
        if (hsRef<RelVaultNode> rvnSdl = rvn->GetChildNode(plVault::kNodeType_SDL, 1)) {
            VaultSDLNode sdl(rvnSdl);
            sdl.SetStateDataRecord(rec, plSDL::kDirtyOnly | plSDL::kTimeStampOnRead);
        }
    }
}

//============================================================================
unsigned VaultAgeGetAgeTime () {
    hsAssert(false, "eric, implement me");
    return 0;
}

//============================================================================
hsRef<RelVaultNode> VaultGetSubAgeLink (const plAgeInfoStruct * info) {
    hsRef<RelVaultNode> rvnLink;
    if (hsRef<RelVaultNode> rvnFldr = VaultGetAgeSubAgesFolder()) {
        NetVaultNode templateNode;
        templateNode.SetNodeType(plVault::kNodeType_AgeInfo);

        VaultAgeInfoNode ageInfo(&templateNode);
        ageInfo.SetAgeFilename(info->GetAgeFilename());

        if (hsRef<RelVaultNode> rvnInfo = rvnFldr->GetChildNode(&templateNode, 2)) {
            templateNode.Clear();
            templateNode.SetNodeType(plVault::kNodeType_AgeLink);
            rvnLink = rvnInfo->GetParentNode(&templateNode, 1);
        }
    }
    
    return rvnLink;
}

//============================================================================
bool VaultAgeGetSubAgeLink (const plAgeInfoStruct * info, plAgeLinkStruct * link) {
    bool result = false;
    if (hsRef<RelVaultNode> rvnLink = VaultGetSubAgeLink(info)) {
        if (hsRef<RelVaultNode> rvnInfo = rvnLink->GetChildNode(plVault::kNodeType_AgeInfo, 1)) {
            VaultAgeInfoNode ageInfo(rvnInfo);
            ageInfo.CopyTo(link->GetAgeInfo());
            result = true;
        }
    }
    
    return result;
}

//============================================================================
namespace _VaultCreateSubAgeAndWait {

struct _InitAgeParam {
    ENetError       result;
    bool            complete;
    unsigned        ageInfoId;
};
static void _InitAgeCallback (
    ENetError       result,
    void *          ,
    void *          vparam,
    unsigned        ageVaultId,
    unsigned        ageInfoVaultId
) {
    _InitAgeParam * param = (_InitAgeParam *)vparam;
    param->ageInfoId    = ageInfoVaultId;
    param->result       = result;
    param->complete     = true;
}
struct _FetchVaultParam {
    ENetError       result;
    bool            complete;
};
static void _FetchVaultCallback (
    ENetError       result,
    void *          vparam
) {
    _FetchVaultParam * param = (_FetchVaultParam *)vparam;
    param->result       = result;
    param->complete     = true;
}
struct _CreateNodeParam {
    ENetError       result;
    bool            complete;
    unsigned        nodeId;
};
static void _CreateNodeCallback (
    ENetError       result,
    void *          ,
    void *          vparam,
    hsWeakRef<RelVaultNode> node
) {
    _CreateNodeParam * param = (_CreateNodeParam *)vparam;
    if (IS_NET_SUCCESS(result))
        param->nodeId = node->GetNodeId();
    param->result       = result;
    param->complete     = true;
}
struct _AddChildNodeParam {
    ENetError       result;
    bool            complete;
};
static void _AddChildNodeCallback (
    ENetError       result,
    void *          vparam
) {
    _AddChildNodeParam * param = (_AddChildNodeParam *)vparam;
    param->result       = result;
    param->complete     = true;
}

} // namespace _VaultCreateSubAgeAndWait

//============================================================================
bool VaultAgeFindOrCreateSubAgeLinkAndWait (
    const plAgeInfoStruct * info,
    plAgeLinkStruct *       link,
    const plUUID&           parentAgeInstId
) {
    if (hsRef<RelVaultNode> rvnLink = VaultFindAgeSubAgeLink(info)) {
        VaultAgeLinkNode linkAcc(rvnLink);
        linkAcc.CopyTo(link);
        if (hsRef<RelVaultNode> rvnInfo = rvnLink->GetChildNode(plVault::kNodeType_AgeInfo, 1)) {
            VaultAgeInfoNode infoAcc(rvnInfo);
            infoAcc.CopyTo(link->GetAgeInfo());
            return true;
        }
    }

    using namespace _VaultCreateSubAgeAndWait;

    unsigned subAgesId;
    unsigned ageInfoId;
    unsigned ageLinkId;
    
    if (hsRef<RelVaultNode> rvnSubAges = VaultGetAgeSubAgesFolder())
        subAgesId = rvnSubAges->GetNodeId();
    else {
        LogMsg(kLogError, "CreateSubAge: Failed to get ages's SubAges folder");
        return false;
    }
    
    {   // Init age vault
        _InitAgeParam   param;
        memset(&param, 0, sizeof(param));
        
        VaultInitAge(
            info,
            parentAgeInstId,
            _InitAgeCallback,
            nullptr,
            &param
        );

        while (!param.complete) {
            NetClientUpdate();
            plgDispatch::Dispatch()->MsgQueueProcess();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        if (IS_NET_ERROR(param.result)) {
            LogMsg(kLogError, "CreateSubAge: Failed to init age {}", link->GetAgeInfo()->GetAgeFilename());
            return false;
        }
            
        ageInfoId = param.ageInfoId;
    }       
    
    {   // Create age link
        _CreateNodeParam    param;
        memset(&param, 0, sizeof(param));

        VaultCreateNode(
            plVault::kNodeType_AgeLink,
            _CreateNodeCallback,
            nullptr,
            &param
        );

        while (!param.complete) {
            NetClientUpdate();
            plgDispatch::Dispatch()->MsgQueueProcess();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        if (IS_NET_ERROR(param.result)) {
            LogMsg(kLogError, "CreateSubAge: Failed create age link node");
            return false;
        }
            
        ageLinkId = param.nodeId;
    }       

    {   // Fetch age info node tree
        _FetchVaultParam    param;
        memset(&param, 0, sizeof(param));
        
        VaultDownload(
            "CreateSubAge",
            ageInfoId,
            _FetchVaultCallback,
            &param,
            nullptr,
            nullptr
        );
        
        while (!param.complete) {
            NetClientUpdate();
            plgDispatch::Dispatch()->MsgQueueProcess();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        if (IS_NET_ERROR(param.result)) {
            LogMsg(kLogError, "CreateSubAge: Failed to download age info vault");
            return false;
        }
    }

    { // Link:
        // ageLink to ages's subages folder
        // ageInfo to ageLink
        _AddChildNodeParam  param1;
        _AddChildNodeParam  param2;
        memset(&param1, 0, sizeof(param1));
        memset(&param2, 0, sizeof(param2));

        VaultAddChildNode(
            subAgesId,
            ageLinkId,
            0,
            _AddChildNodeCallback,
            &param1
        );

        VaultAddChildNode(
            ageLinkId,
            ageInfoId,
            0,
            _AddChildNodeCallback,
            &param2
        );

        while (!param1.complete && !param2.complete) {
            NetClientUpdate();
            plgDispatch::Dispatch()->MsgQueueProcess();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        if (IS_NET_ERROR(param1.result)) {
            LogMsg(kLogError, "CreateSubAge: Failed to add link to ages's subages");
            return false;
        }
        if (IS_NET_ERROR(param2.result)) {
            LogMsg(kLogError, "CreateSubAge: Failed to add info to link");
            return false;
        }
    }
        
    if (hsRef<RelVaultNode> rvnLink = VaultGetNode(ageLinkId)) {
        VaultAgeLinkNode linkAcc(rvnLink);
        linkAcc.CopyTo(link);
    }

    if (hsRef<RelVaultNode> rvnInfo = VaultGetNode(ageInfoId)) {
        VaultAgeInfoNode infoAcc(rvnInfo);
        infoAcc.CopyTo(link->GetAgeInfo());
    }

    return true;
}

//============================================================================
namespace _VaultCreateSubAge {
    void _CreateNodeCallback(ENetError result, void* state, void* param, hsWeakRef<RelVaultNode> node) {
        if (IS_NET_ERROR(result)) {
            LogMsg(kLogError, "CreateSubAge: Failed to create AgeLink (async)");
            return;
        }

        // Add the children to the right places
        VaultAddChildNode(node->GetNodeId(), (uint32_t)((uintptr_t)param), 0, nullptr, nullptr);
        if (hsRef<RelVaultNode> saFldr = VaultGetAgeSubAgesFolder())
            VaultAddChildNode(saFldr->GetNodeId(), node->GetNodeId(), 0, nullptr, nullptr);
        else
            LogMsg(kLogError, "CreateSubAge: Couldn't find SubAges folder (async)");

        // Send the VaultNotify that the plNetLinkingMgr wants...
        plVaultNotifyMsg * msg = new plVaultNotifyMsg;
        msg->SetType(plVaultNotifyMsg::kRegisteredSubAgeLink);
        msg->SetResultCode(result);
        msg->GetArgs()->AddInt(plNetCommon::VaultTaskArgs::kAgeLinkNode, node->GetNodeId());
        msg->Send();
    }

    void _DownloadCallback(ENetError result, void* param) {
        if (IS_NET_ERROR(result)) {
            LogMsg(kLogError, "CreateSubAge: Failed to download age vault (async)");
            return;
        }

        // Create the AgeLink node
        VaultCreateNode(plVault::kNodeType_AgeLink,
                        (FVaultCreateNodeCallback)_CreateNodeCallback,
                        nullptr,
                        param
        );
    }

    void _InitAgeCallback(ENetError result, void* state, void* param, uint32_t ageVaultId, uint32_t ageInfoId) {
        if (IS_NET_ERROR(result)) {
            LogMsg(kLogError, "CreateSubAge: Failed to init age (async)");
            return;
        }

        // Download age vault
        VaultDownload("CreateSubAge",
                      ageInfoId,
                      (FVaultDownloadCallback)_DownloadCallback,
                      (void*)(uintptr_t)ageInfoId,
                      nullptr,
                      nullptr
        );
    }
}; // namespace _VaultCreateSubAge

bool VaultAgeFindOrCreateSubAgeLink(const plAgeInfoStruct* info, plAgeLinkStruct* link, const plUUID& parentUuid) {
    using namespace _VaultCreateSubAge;

    // First, try to find an already existing subage
    if (hsRef<RelVaultNode> rvnLink = VaultGetSubAgeLink(info)) {
        VaultAgeLinkNode accLink(rvnLink);
        accLink.CopyTo(link);

        if (hsRef<RelVaultNode> rvnInfo = rvnLink->GetChildNode(plVault::kNodeType_AgeInfo, 1)) {
            VaultAgeInfoNode accInfo(rvnInfo);
            accInfo.CopyTo(link->GetAgeInfo());
        }

        return true;
    }
    
    VaultInitAge(info,
                 parentUuid,
                 (FVaultInitAgeCallback)_InitAgeCallback,
                 nullptr,
                 nullptr
    );

    return false;
}

//============================================================================
namespace _VaultCreateChildAgeAndWait {

struct _InitAgeParam {
    ENetError       result;
    bool            complete;
    unsigned        ageInfoId;
};
static void _InitAgeCallback (
    ENetError       result,
    void *          ,
    void *          vparam,
    unsigned        ageVaultId,
    unsigned        ageInfoVaultId
) {
    _InitAgeParam * param = (_InitAgeParam *)vparam;
    param->ageInfoId    = ageInfoVaultId;
    param->result       = result;
    param->complete     = true;
}
struct _FetchVaultParam {
    ENetError       result;
    bool            complete;
};
static void _FetchVaultCallback (
    ENetError       result,
    void *          vparam
) {
    _FetchVaultParam * param = (_FetchVaultParam *)vparam;
    param->result       = result;
    param->complete     = true;
}
struct _CreateNodeParam {
    ENetError       result;
    bool            complete;
    unsigned        nodeId;
};
static void _CreateNodeCallback (
    ENetError       result,
    void *          ,
    void *          vparam,
    hsWeakRef<RelVaultNode> node
) {
    _CreateNodeParam * param = (_CreateNodeParam *)vparam;
    if (IS_NET_SUCCESS(result))
        param->nodeId = node->GetNodeId();
    param->result       = result;
    param->complete     = true;
}
struct _AddChildNodeParam {
    ENetError       result;
    bool            complete;
};
static void _AddChildNodeCallback (
    ENetError       result,
    void *          vparam
) {
    _AddChildNodeParam * param = (_AddChildNodeParam *)vparam;
    param->result       = result;
    param->complete     = true;
}

} // namespace _VaultCreateChildAgeAndWait

//============================================================================
bool VaultAgeFindOrCreateChildAgeLinkAndWait (
    const ST::string&       parentAgeName,
    const plAgeInfoStruct * info,
    plAgeLinkStruct *       link
) {
    using namespace _VaultCreateChildAgeAndWait;

    unsigned childAgesId;
    unsigned ageInfoId;
    unsigned ageLinkId;

    {   // Get id of child ages folder
        hsRef<RelVaultNode> rvnAgeInfo;
        if (!parentAgeName.empty()) {
            plAgeInfoStruct pinfo;
            pinfo.SetAgeFilename(parentAgeName);
            if (hsRef<RelVaultNode> rvnAgeLink = VaultGetOwnedAgeLink(&pinfo))
                rvnAgeInfo = rvnAgeLink->GetChildNode(plVault::kNodeType_AgeInfo, 1);
        } else {
            rvnAgeInfo = VaultGetAgeInfoNode();
        }
        
        if (!rvnAgeInfo) {
            LogMsg(kLogError, "CreateChildAge: Failed to get ages's AgeInfo node");
            return false;
        }

        hsRef<RelVaultNode> rvnChildAges;
        if (rvnChildAges = rvnAgeInfo->GetChildAgeInfoListNode(plVault::kChildAgesFolder, 1)) {
            childAgesId = rvnChildAges->GetNodeId();
        }       
        else {
            LogMsg(kLogError, "CreateChildAge: Failed to get ages's ChildAges folder");
            return false;
        }
        
        // Check for existing child age in folder
        hsRef<RelVaultNode> rvnLink;
        NetVaultNode templateNode;
        templateNode.SetNodeType(plVault::kNodeType_AgeInfo);

        VaultAgeInfoNode ageInfo(&templateNode);
        ageInfo.SetAgeFilename(info->GetAgeFilename());

        if (hsRef<RelVaultNode> rvnInfo = rvnChildAges->GetChildNode(&templateNode, 2)) {
            templateNode.Clear();
            templateNode.SetNodeType(plVault::kNodeType_AgeLink);
            rvnLink = rvnInfo->GetParentNode(&templateNode, 1);
        }

        if (rvnLink) {
            VaultAgeLinkNode access(rvnLink);
            access.CopyTo(link);
            return true;
        }
    }   

    {   // Init age vault
        _InitAgeParam   param;
        memset(&param, 0, sizeof(param));

        plUUID parentAgeInstId;
        if (hsRef<RelVaultNode> rvnAge = VaultGetAgeNode()) {
            VaultAgeNode access(rvnAge);
            parentAgeInstId = access.GetAgeInstanceGuid();
        }
        
        VaultInitAge(
            info,
            parentAgeInstId,
            _InitAgeCallback,
            nullptr,
            &param
        );

        while (!param.complete) {
            NetClientUpdate();
            plgDispatch::Dispatch()->MsgQueueProcess();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        if (IS_NET_ERROR(param.result)) {
            LogMsg(kLogError, "CreateChildAge: Failed to init age {}", link->GetAgeInfo()->GetAgeFilename());
            return false;
        }
            
        ageInfoId = param.ageInfoId;
    }       
    
    {   // Create age link
        _CreateNodeParam    param;
        memset(&param, 0, sizeof(param));

        VaultCreateNode(
            plVault::kNodeType_AgeLink,
            _CreateNodeCallback,
            nullptr,
            &param
        );

        while (!param.complete) {
            NetClientUpdate();
            plgDispatch::Dispatch()->MsgQueueProcess();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        if (IS_NET_ERROR(param.result)) {
            LogMsg(kLogError, "CreateChildAge: Failed create age link node");
            return false;
        }
            
        ageLinkId = param.nodeId;
    }       

    {   // Fetch age info node tree
        _FetchVaultParam    param;
        memset(&param, 0, sizeof(param));
        
        VaultDownload(
            "CreateChildAge",
            ageInfoId,
            _FetchVaultCallback,
            &param,
            nullptr,
            nullptr
        );
        
        while (!param.complete) {
            NetClientUpdate();
            plgDispatch::Dispatch()->MsgQueueProcess();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        if (IS_NET_ERROR(param.result)) {
            LogMsg(kLogError, "CreateChildAge: Failed to download age info vault");
            return false;
        }
    }

    { // Link:
        // ageLink to ages's subages folder
        // ageInfo to ageLink
        _AddChildNodeParam  param1;
        _AddChildNodeParam  param2;
        memset(&param1, 0, sizeof(param1));
        memset(&param2, 0, sizeof(param2));

        VaultAddChildNode(
            childAgesId,
            ageLinkId,
            0,
            _AddChildNodeCallback,
            &param1
        );

        VaultAddChildNode(
            ageLinkId,
            ageInfoId,
            0,
            _AddChildNodeCallback,
            &param2
        );

        while (!param1.complete && !param2.complete) {
            NetClientUpdate();
            plgDispatch::Dispatch()->MsgQueueProcess();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        if (IS_NET_ERROR(param1.result)) {
            LogMsg(kLogError, "CreateChildAge: Failed to add link to ages's subages");
            return false;
        }
        if (IS_NET_ERROR(param2.result)) {
            LogMsg(kLogError, "CreateChildAge: Failed to add info to link");
            return false;
        }
    }
        
    if (hsRef<RelVaultNode> rvnLink = VaultGetNode(ageLinkId)) {
        VaultAgeLinkNode linkAcc(rvnLink);
        linkAcc.CopyTo(link);
    }

    if (hsRef<RelVaultNode> rvnInfo = VaultGetNode(ageInfoId)) {
        VaultAgeInfoNode infoAcc(rvnInfo);
        infoAcc.CopyTo(link->GetAgeInfo());
    }

    return true;
}

//============================================================================
namespace _VaultCreateChildAge {
    struct _Params {
        void* fChildAgesFldr;
        void* fAgeInfoId;
    };

    void _CreateNodeCallback(ENetError result, void* state, void* param, hsWeakRef<RelVaultNode> node) {
        if (IS_NET_ERROR(result)) {
            LogMsg(kLogError, "CreateChildAge: Failed to create AgeLink (async)");
            delete (_Params*)param;
            return;
        }

        _Params* p = (_Params*)param;

        // Add the children to the right places
        VaultAddChildNode(node->GetNodeId(), (uint32_t)((uintptr_t)p->fAgeInfoId), 0, nullptr, nullptr);
        VaultAddChildNode((uint32_t)((uintptr_t)p->fChildAgesFldr), node->GetNodeId(), 0, nullptr, nullptr);

        // Send the VaultNotify that the plNetLinkingMgr wants...
        plVaultNotifyMsg * msg = new plVaultNotifyMsg;
        msg->SetType(plVaultNotifyMsg::kRegisteredChildAgeLink);
        msg->SetResultCode(result);
        msg->GetArgs()->AddInt(plNetCommon::VaultTaskArgs::kAgeLinkNode, node->GetNodeId());
        msg->Send();

        delete (_Params*)param;
    }

    void _DownloadCallback(ENetError result, void* param) {
        if (IS_NET_ERROR(result)) {
            LogMsg(kLogError, "CreateChildAge: Failed to download age vault (async)");
            delete (_Params*)param;
            return;
        }

        // Create the AgeLink node
        VaultCreateNode(plVault::kNodeType_AgeLink,
                        _CreateNodeCallback,
                        nullptr,
                        param
        );
    }

    void _InitAgeCallback(ENetError result, void* state, void* param, uint32_t ageVaultId, uint32_t ageInfoId) {
        if (IS_NET_ERROR(result)) {
            LogMsg(kLogError, "CreateChildAge: Failed to init age (async)");
            delete (_Params*)param;
            return;
        }

        _Params* p = (_Params*)param;
        p->fAgeInfoId = (void*)(uintptr_t)ageInfoId;

        // Download age vault
        VaultDownload("CreateChildAge",
                      ageInfoId,
                      (FVaultDownloadCallback)_DownloadCallback,
                      param,
                      nullptr,
                      nullptr
        );
    }
}; // namespace _VaultCreateAge

plVaultChildAgeLinkResult VaultAgeFindOrCreateChildAgeLink(
    const ST::string&      parentAgeName,
    const plAgeInfoStruct* info,
    plAgeLinkStruct*       link) 
{
    using namespace _VaultCreateChildAge;

    // First, try to find an already existing ChildAge
    plAgeInfoStruct search;
    search.SetAgeFilename(parentAgeName);

    hsRef<RelVaultNode> rvnParentInfo;
    if (hsRef<RelVaultNode> rvnParentLink = VaultGetOwnedAgeLink(&search))
        rvnParentInfo = rvnParentLink->GetChildNode(plVault::kNodeType_AgeInfo, 1);
    else // Fallback to current age
        rvnParentInfo = VaultGetAgeInfoNode();

    // Test to make sure nothing went horribly wrong...
    if (rvnParentInfo == nullptr) {
        LogMsg(kLogError, "CreateChildAge: Couldn't find the parent ageinfo (async)");
        return plVaultChildAgeLinkResult::kFailed;
    }

    // Still here? Try to find the Child Ages folder
    plVaultChildAgeLinkResult retval = plVaultChildAgeLinkResult::kFailed;
    if (hsRef<RelVaultNode> rvnChildAges = rvnParentInfo->GetChildAgeInfoListNode(plVault::kChildAgesFolder, 1)) {
        // Search for our age
        NetVaultNode temp;
        temp.SetNodeType(plVault::kNodeType_AgeInfo);
        VaultAgeInfoNode theAge(&temp);
        theAge.SetAgeFilename(info->GetAgeFilename());

        if (hsRef<RelVaultNode> rvnAgeInfo = rvnChildAges->GetChildNode(&temp, 2)) {
            hsRef<RelVaultNode> rvnAgeLink = rvnAgeInfo->GetParentAgeLink();

            VaultAgeLinkNode accAgeLink(rvnAgeLink);
            accAgeLink.CopyTo(link);
            VaultAgeInfoNode accAgeInfo(rvnAgeInfo);
            accAgeInfo.CopyTo(link->GetAgeInfo());

            retval = plVaultChildAgeLinkResult::kFoundExisting;
        } else {
            _Params* p = new _Params;
            p->fChildAgesFldr = (void*)(uintptr_t)rvnChildAges->GetNodeId();

            VaultAgeInfoNode accParentInfo(rvnParentInfo);
            VaultInitAge(info,
                         accParentInfo.GetAgeInstanceGuid(),
                         (FVaultInitAgeCallback)_InitAgeCallback,
                         nullptr,
                         p
            );
            retval = plVaultChildAgeLinkResult::kCreatingNew;
        }
    }

    return retval;
}


/*****************************************************************************
*
*   CCR Vault Access
*
***/

//============================================================================
void VaultCCRDumpPlayers() {
    hsAssert(false, "eric, implement me");
}


/*****************************************************************************
*
*   Exports - Vault download
*
***/

//============================================================================
void VaultDownload (
    const ST::string&           tag,
    unsigned                    vaultId,
    FVaultDownloadCallback      callback,
    void *                      cbParam,
    FVaultProgressCallback      progressCallback,
    void *                      cbProgressParam
) {
    VaultDownloadTrans * trans = new VaultDownloadTrans(tag, callback, cbParam,
        progressCallback, cbProgressParam, vaultId);

    NetCliAuthVaultFetchNodeRefs(
        vaultId,
        VaultDownloadTrans::VaultNodeRefsFetched,
        trans
    );
}

//============================================================================
void VaultDownloadNoCallbacks (
    const ST::string&           tag,
    unsigned                    vaultId,
    FVaultDownloadCallback      callback,
    void *                      cbParam,
    FVaultProgressCallback      progressCallback,
    void *                      cbProgressParam
) {
    VaultDownloadNoCallbacksTrans * trans = new VaultDownloadNoCallbacksTrans(tag,
        callback, cbParam, progressCallback, cbProgressParam, vaultId);

    NetCliAuthVaultFetchNodeRefs(
        vaultId,
        VaultDownloadTrans::VaultNodeRefsFetched,
        trans
    );
}

//============================================================================
struct _DownloadVaultParam {
    ENetError       result;
    bool            complete;
};
static void _DownloadVaultCallback (
    ENetError       result,
    void *          vparam
) {
    _DownloadVaultParam * param = (_DownloadVaultParam *)vparam;
    param->result       = result;
    param->complete     = true;
}

void VaultDownloadAndWait (
    const ST::string&           tag,
    unsigned                    vaultId,
    FVaultProgressCallback      progressCallback,
    void *                      cbProgressParam
) {
    _DownloadVaultParam param;
    memset(&param, 0, sizeof(param));
    
    VaultDownload(
        tag,
        vaultId,
        _DownloadVaultCallback,
        &param,
        progressCallback,
        cbProgressParam
    );
    
    while (!param.complete) {
        NetClientUpdate();
        plgDispatch::Dispatch()->MsgQueueProcess();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

//============================================================================
void VaultCull (unsigned vaultId) {
    VaultCallbackSuppressor suppress;

    // Remove the node from the global table
    auto nodeIt = s_nodes.find(vaultId);
    if (nodeIt != s_nodes.end()) {
        LogMsg(kLogDebug, "Vault: Culling node {}", nodeIt->first);
        nodeIt->second->state->UnlinkFromRelatives();
        s_nodes.erase(nodeIt);
    }

    // Remove all orphaned nodes from the global table
    for (auto it = s_nodes.begin(); it != s_nodes.end();) {
        const hsRef<RelVaultNode>& node = it->second;

        if (node->GetNodeType() > plVault::kNodeType_VNodeMgrLow && node->GetNodeType() < plVault::kNodeType_VNodeMgrHigh) {
            ++it;
            continue;
        }

        std::vector<unsigned> nodeIds;
        node->GetRootIds(&nodeIds);
        bool foundRoot = false;
        for (unsigned nodeId : nodeIds) {
            auto rootIt = s_nodes.find(nodeId);
            if (rootIt != s_nodes.end()) {
                const hsRef<RelVaultNode>& root = rootIt->second;
                if (root->GetNodeType() > plVault::kNodeType_VNodeMgrLow && root->GetNodeType() < plVault::kNodeType_VNodeMgrHigh) {
                    foundRoot = true;
                    break;
                }
            }
        }
        if (!foundRoot) {
            LogMsg(kLogDebug, "Vault: Culling node {}", it->first);
            node->state->UnlinkFromRelatives();
            it = s_nodes.erase(it);
        } else {
            ++it;
        }
    }   
}

/*****************************************************************************
*
*   Exports - Vault global node handling
*
***/

//============================================================================
hsRef<RelVaultNode> VaultGetSystemNode () {
    hsRef<RelVaultNode> result;
    if (hsRef<RelVaultNode> player = VaultGetPlayerNode()) {
        NetVaultNode templateNode;
        templateNode.SetNodeType(plVault::kNodeType_System);
        if (hsRef<RelVaultNode> systemNode = player->GetChildNode(&templateNode, 1))
            result = systemNode;
    }
    return result;
}

//============================================================================
hsRef<RelVaultNode> VaultGetGlobalInbox () {
    hsRef<RelVaultNode> result;
    if (hsRef<RelVaultNode> system = VaultGetSystemNode()) {
        NetVaultNode templateNode;
        templateNode.SetNodeType(plVault::kNodeType_Folder);
        VaultFolderNode folder(&templateNode);
        folder.SetFolderType(plVault::kGlobalInboxFolder);
        if (hsRef<RelVaultNode> inbox = system->GetChildNode(&templateNode, 1))
            result = inbox;
    }
    return result;
}
