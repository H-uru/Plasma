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
#pragma hdrstop

#ifdef CLIENT

/*****************************************************************************
*
*   Private
*
***/

struct IVaultCallback {
    LINK(IVaultCallback)    link;
    VaultCallback *         cb;
};

struct INotifyAfterDownload : THashKeyVal<unsigned> {
    HASHLINK(INotifyAfterDownload)  link;
    unsigned                        parentId;
    unsigned                        childId;

    INotifyAfterDownload (unsigned parentId, unsigned childId)
    :   THashKeyVal<unsigned>(childId)
    ,   parentId(parentId)
    ,   childId(childId)
    {}
};

struct DeviceInbox : CHashKeyStr {
    HASHLINK(DeviceInbox)   link;
    wchar_t                 inboxName[kMaxVaultNodeStringLength];

    DeviceInbox (const wchar_t device[], const wchar_t inbox[])
    :   CHashKeyStr(device)
    {
        StrCopy(inboxName, inbox, arrsize(inboxName));
    }
};

// A RelVaultNodeLink may be either stored in the global table,
// or stored in an IRelVaultNode's parents or children table.
struct RelVaultNodeLink : THashKeyVal<unsigned> {
    HASHLINK(RelVaultNodeLink)  link;
    hsRef<RelVaultNode>         node;
    unsigned                    ownerId;
    bool                        seen;
    
    RelVaultNodeLink (bool seen, unsigned ownerId, unsigned nodeId, RelVaultNode * node)
    :   THashKeyVal<unsigned>(nodeId)
    ,   node(node)
    ,   ownerId(ownerId)
    ,   seen(seen)
    {
    }
};


struct IRelVaultNode {
    RelVaultNode * node;        // MUST be a weak ref!
    
    HASHTABLEDECL(
        RelVaultNodeLink,
        THashKeyVal<unsigned>,
        link
    ) parents;

    HASHTABLEDECL(
        RelVaultNodeLink,
        THashKeyVal<unsigned>,
        link
    ) children;

    IRelVaultNode (RelVaultNode * node);
    ~IRelVaultNode ();

    // Unlink our node from all our parent and children
    void UnlinkFromRelatives ();
    
    // Unlink the node from our parent and children lists
    void Unlink (RelVaultNode * other);
};


struct VaultCreateNodeTrans {
    FVaultCreateNodeCallback    callback;
    void *                      state;
    void *                      param;

    unsigned                    nodeId;
    hsRef<RelVaultNode>         node;

    VaultCreateNodeTrans ()
        : callback(nil), state(nil), param(nil), nodeId(0), node(nil) { }

    VaultCreateNodeTrans (FVaultCreateNodeCallback _callback,
                          void * _state, void * _param)
        : callback(_callback), state(_state), param(_param),
          nodeId(0), node(nil) { }

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

    VaultFindNodeTrans () : callback(nil), param(nil) { }

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

    wchar_t     tag[MAX_PATH];
    unsigned    nodeCount;
    unsigned    nodesLeft;
    unsigned    vaultId;
    ENetError   result;

    VaultDownloadTrans ()
        : callback(nil), cbParam(nil), progressCallback(nil), cbProgressParam(nil),
          nodeCount(0), nodesLeft(0), vaultId(0), result(kNetSuccess)
    {
        memset(tag, 0, sizeof(tag));
    }

    VaultDownloadTrans (const wchar_t * _tag, FVaultDownloadCallback _callback,
                        void * _cbParam, FVaultProgressCallback _progressCallback,
                        void * _cbProgressParam, unsigned _vaultId)
        : callback(_callback), cbParam(_cbParam), progressCallback(_progressCallback),
          cbProgressParam(_cbProgressParam), nodeCount(0), nodesLeft(0),
          vaultId(_vaultId), result(kNetSuccess)
    {
        wcsncpy(tag, _tag, arrsize(tag));
        tag[arrsize(tag)-1] = 0;
    }


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

struct VaultAgeInitTrans {
    FVaultInitAgeCallback   callback;
    void *                  cbState;
    void *                  cbParam;

    VaultAgeInitTrans()
        : callback(nil), cbState(nil), cbParam(nil) { }

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
        : callback(nil), cbParam(nil), result(kNetSuccess), opCount(0) { }

    AddChildNodeFetchTrans(FVaultAddChildNodeCallback _callback, void * _param)
        : callback(_callback), cbParam(_param), result(kNetSuccess), opCount(0) { }

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

static bool s_running;

static HASHTABLEDECL(
    RelVaultNodeLink,
    THashKeyVal<unsigned>,
    link
) s_nodes;

static LISTDECL(
    IVaultCallback,
    link
) s_callbacks;

static HASHTABLEDECL(
    INotifyAfterDownload,
    THashKeyVal<unsigned>,
    link
) s_notifyAfterDownload;

static HASHTABLEDECL(
    DeviceInbox,
    CHashKeyStr,
    link
) s_ageDeviceInboxes;

static bool s_processPlayerInbox = false;

/*****************************************************************************
*
*   Local functions
*
***/

static void VaultProcessVisitNote(RelVaultNode * rvnVisit);
static void VaultProcessUnvisitNote(RelVaultNode * rvnUnVisit);

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

    INotifyAfterDownload* notify = s_notifyAfterDownload.Find(childId);

    if (notify) {
        if (IS_NET_SUCCESS(result)) {
            RelVaultNodeLink* parentLink    = s_nodes.Find(notify->parentId);
            RelVaultNodeLink* childLink     = s_nodes.Find(notify->childId);

            if (parentLink && childLink) {
                if (childLink->node->GetNodeType() == plVault::kNodeType_TextNote) {
                    VaultTextNoteNode textNote(childLink->node);
                    if (textNote.GetNoteType() == plVault::kNoteType_Visit)
                        VaultProcessVisitNote(childLink->node);
                    else if (textNote.GetNoteType() == plVault::kNoteType_UnVisit)
                        VaultProcessUnvisitNote(childLink->node);
                }

                for (IVaultCallback * cb = s_callbacks.Head(); cb; cb = s_callbacks.Next(cb))
                    cb->cb->AddedChildNode(parentLink->node, childLink->node);
            }
        }

        delete notify;
    }
}

//============================================================================
static void CDECL LogDumpProc (
    void *              ,
    const wchar_t         fmt[],
    ...
) {
    va_list args;
    va_start(args, fmt);
    LogMsgV(kLogDebug, fmt, args);
    va_end(args);
}

//============================================================================
// Returns ids of nodes that had to be created (so we can fetch them)
static void BuildNodeTree (
    const NetVaultNodeRef   refs[],
    unsigned                refCount,
    ARRAY(unsigned) *       newNodeIds,
    ARRAY(unsigned) *       existingNodeIds,
    bool                    notifyNow = true
) {
    for (unsigned i = 0; i < refCount; ++i) {
        // Find/Create global links
        RelVaultNodeLink * parentLink = s_nodes.Find(refs[i].parentId);
        if (!parentLink) {
            newNodeIds->Add(refs[i].parentId);
            parentLink = new RelVaultNodeLink(false, 0, refs[i].parentId, new RelVaultNode);
            parentLink->node->SetNodeId_NoDirty(refs[i].parentId);
            s_nodes.Add(parentLink);
        }
        else {
            existingNodeIds->Add(refs[i].parentId);
        }
        RelVaultNodeLink * childLink = s_nodes.Find(refs[i].childId);
        if (!childLink) {
            newNodeIds->Add(refs[i].childId);
            childLink = new RelVaultNodeLink(refs[i].seen, refs[i].ownerId, refs[i].childId, new RelVaultNode);
            childLink->node->SetNodeId_NoDirty(refs[i].childId);
            s_nodes.Add(childLink);
        }
        else {
            existingNodeIds->Add(refs[i].childId);
            if (unsigned ownerId = refs[i].ownerId)
                childLink->ownerId = ownerId;
        }

        hsRef<RelVaultNode> parentNode = parentLink->node;
        hsRef<RelVaultNode> childNode = childLink->node;
        
        bool isImmediateParent = parentNode->IsParentOf(refs[i].childId, 1);
        bool isImmediateChild = childNode->IsChildOf(refs[i].parentId, 1);
            
        if (!isImmediateParent) {
            // Add parent to child's parents table
            parentLink = new RelVaultNodeLink(false, 0, parentNode->GetNodeId(), parentNode);
            childNode->state->parents.Add(parentLink);
            LogMsg(kLogDebug, L"Added relationship: p:%u,c:%u", refs[i].parentId, refs[i].childId);
        }
        
        if (!isImmediateChild) {
            // Add child to parent's children table
            childLink = new RelVaultNodeLink(refs[i].seen, refs[i].ownerId, childNode->GetNodeId(), childNode);
            parentNode->state->children.Add(childLink);

            if (notifyNow || childNode->GetNodeType() != 0) {
                // We made a new link, so make the callbacks
                for (IVaultCallback * cb = s_callbacks.Head(); cb; cb = s_callbacks.Next(cb))
                    cb->cb->AddedChildNode(parentNode, childNode);
            }
            else {
                INotifyAfterDownload* notify = new INotifyAfterDownload(parentNode->GetNodeId(), childNode->GetNodeId());
                s_notifyAfterDownload.Add(notify);
            }
        }
    }
}

//============================================================================
static void InitFetchedNode (RelVaultNode * rvn) {

    switch (rvn->GetNodeType()) {
        case plVault::kNodeType_SDL: {
            VaultSDLNode access(rvn);
            if (!access.GetSDLData() || !access.GetSDLDataLength())
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
    ARRAY(unsigned) ownerIds;
    {   for (unsigned i = 0; i < refCount; ++i)
            if (unsigned ownerId = refs[i].ownerId)
                ownerIds.Add(ownerId);
    }
    QSORT(unsigned, ownerIds.Ptr(), ownerIds.Count(), elem1 < elem2);
    hsRef<RelVaultNode> templateNode = new RelVaultNode;
    templateNode->SetNodeType(plVault::kNodeType_PlayerInfo);
    {   unsigned prevId = 0;
        for (unsigned i = 0; i < ownerIds.Count(); ++i) {
            if (ownerIds[i] != prevId) {
                prevId = ownerIds[i];
                VaultPlayerInfoNode access(templateNode);
                access.SetPlayerId(refs[i].ownerId);
                if (VaultGetNode(templateNode))
                    continue;
                NetCliAuthVaultNodeFind(
                    templateNode,
                    VaultNodeFound,
                    nil
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
    
    ARRAY(unsigned) newNodeIds;
    ARRAY(unsigned) existingNodeIds;
    
    BuildNodeTree(refs, refCount, &newNodeIds, &existingNodeIds);

    ARRAY(unsigned) nodeIds;
    nodeIds.Add(newNodeIds.Ptr(), newNodeIds.Count());
    nodeIds.Add(existingNodeIds.Ptr(), existingNodeIds.Count());
    QSORT(unsigned, nodeIds.Ptr(), nodeIds.Count(), elem1 < elem2);

    // Fetch the nodes that do not yet have a nodetype
    unsigned prevId = 0;
    for (unsigned i = 0; i < nodeIds.Count(); ++i) {
        RelVaultNodeLink * link = s_nodes.Find(nodeIds[i]);
        if (link->node->GetNodeType() != 0)
            continue;
        // filter duplicates
        if (link->node->GetNodeId() == prevId)
            continue;
        prevId = link->node->GetNodeId();
        NetCliAuthVaultNodeFetch(
            nodeIds[i],
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
        if (RelVaultNodeLink * link = s_nodes.Find(nodeIds[i]))
            return;

        // Start fetching the node          
        NetCliAuthVaultNodeFetch(nodeIds[i], VaultNodeFetched, nil);
    }
}

//============================================================================
static void VaultNodeFetched (
    ENetError           result,
    void *              ,
    NetVaultNode *      node
) {
    if (IS_NET_ERROR(result)) {
        LogMsg(kLogDebug, L"VaultNodeFetched failed: %u (%s)", result, NetErrorToString(result));
        return;
    }

    // Add to global node table
    RelVaultNodeLink * link = s_nodes.Find(node->GetNodeId());
    if (!link) {
        link = new RelVaultNodeLink(false, 0, node->GetNodeId(), new RelVaultNode);
        link->node->SetNodeId_NoDirty(node->GetNodeId());
        s_nodes.Add(link);
    }
    link->node->CopyFrom(node, NetVaultNode::kCopyOverwrite);
    InitFetchedNode(link->node);
    
    link->node->Print(L"Fetched", LogDumpProc, 0);
}

//============================================================================
static void ChangedVaultNodeFetched (
    ENetError           result,
    void *              param,
    NetVaultNode *      node
) {
    if (IS_NET_ERROR(result)) {
        LogMsg(kLogDebug, L"ChangedVaultNodeFetched failed: %u (%s)", result, NetErrorToString(result));
        return;
    }

    VaultNodeFetched(result, param, node);

    RelVaultNodeLink* savedLink = s_nodes.Find(node->GetNodeId());

    if (savedLink) {
        for (IVaultCallback * cb = s_callbacks.Head(); cb; cb = s_callbacks.Next(cb))
            cb->cb->ChangedNode(savedLink->node);
    }
}

//============================================================================
static void VaultNodeChanged (
    unsigned        nodeId,
    const plUUID&   revisionId
) {
    LogMsg(kLogDebug, L"Notify: Node changed: %u", nodeId);

    RelVaultNodeLink * link = s_nodes.Find(nodeId);

    // We don't have the node, so we don't care that it changed (we actually
    // shouldn't have been notified)
    if (!link) {
        LogMsg(kLogDebug, L"rcvd change notification for node %u, but node doesn't exist locally.", nodeId);
        return;
    }

    // We are the party responsible for the change, so we already have the
    // latest version of the node; no need to fetch it.
    if (link->node->revisionId == revisionId)
        return;

    // We have the node and we weren't the one that changed it, so fetch it.
    NetCliAuthVaultNodeFetch(
        nodeId,
        ChangedVaultNodeFetched,
        nil
    );
}

//============================================================================
static void VaultNodeAdded (
    unsigned        parentId,
    unsigned        childId,
    unsigned        ownerId
) {
    LogMsg(kLogDebug, L"Notify: Node added: p:%u,c:%u", parentId, childId);

    unsigned inboxId = 0;
    if (hsRef<RelVaultNode> rvnInbox = VaultGetPlayerInboxFolder())
        inboxId = rvnInbox->GetNodeId();

    // Build the relationship locally
    NetVaultNodeRef refs[] = {
        { parentId, childId, ownerId }
    };
    ARRAY(unsigned) newNodeIds;
    ARRAY(unsigned) existingNodeIds;
    
    BuildNodeTree(refs, arrsize(refs), &newNodeIds, &existingNodeIds, false);

    ARRAY(unsigned) nodeIds;
    nodeIds.Add(newNodeIds.Ptr(), newNodeIds.Count());
    nodeIds.Add(existingNodeIds.Ptr(), existingNodeIds.Count());
    QSORT(unsigned, nodeIds.Ptr(), nodeIds.Count(), elem1 < elem2);

    // Fetch the nodes that do not yet have a nodetype
    unsigned prevId = 0;
    unsigned i = 0;
    for (; i < nodeIds.Count(); ++i) {
        RelVaultNodeLink * link = s_nodes.Find(nodeIds[i]);
        if (link->node->GetNodeType() != 0)
            continue;
        // filter duplicates
        if (link->node->GetNodeId() == prevId)
            continue;
        prevId = link->node->GetNodeId();
        VaultDownload(
            L"NodeAdded",
            nodeIds[i],
            VaultNodeAddedDownloadCallback,
            (void*)nodeIds[i],
            nil,
            nil
        );
    }
    
    if (parentId == inboxId) {
        if (i > 0)
            s_processPlayerInbox = true;
        else
            VaultProcessPlayerInbox();
    }

    // if the added element is already downloaded then send the callbacks now
    RelVaultNodeLink* parentLink    = s_nodes.Find(parentId);
    RelVaultNodeLink* childLink     = s_nodes.Find(childId);

    if (childLink->node->GetNodeType() != 0) {
        for (IVaultCallback * cb = s_callbacks.Head(); cb; cb = s_callbacks.Next(cb))
            cb->cb->AddedChildNode(parentLink->node, childLink->node);
    }
}

//============================================================================
static void VaultNodeRemoved (
    unsigned        parentId,
    unsigned        childId
) {
    LogMsg(kLogDebug, L"Notify: Node removed: p:%u,c:%u", parentId, childId);
    for (;;) {
        // Unlink 'em locally, if we can
        RelVaultNodeLink * parentLink = s_nodes.Find(parentId);
        if (!parentLink)
            break;

        RelVaultNodeLink * childLink = s_nodes.Find(childId);
        if (!childLink)
            break;
            
        if (parentLink->node->IsParentOf(childId, 1)) {
            // We have the relationship, so make the callbacks
            for (IVaultCallback * cb = s_callbacks.Head(); cb; cb = s_callbacks.Next(cb))
                cb->cb->RemovingChildNode(parentLink->node, childLink->node);
        }
            
        parentLink->node->state->Unlink(childLink->node);
        childLink->node->state->Unlink(parentLink->node);
        break;
    }
}

//============================================================================
static void VaultNodeDeleted (
    unsigned        nodeId
) {
    LogMsg(kLogDebug, L"Notify: Node deleted: %u", nodeId);
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
        for (RelVaultNodeLink * link = s_nodes.Head(); link; link = s_nodes.Next(link)) {
            if (bytesWritten >= kMaxBytesPerSaveUpdate)
                break;
            if (link->node->GetDirtyFlags()) {

                // Auth server needs the name of the sdl record
                if (link->node->GetNodeType() == plVault::kNodeType_SDL)
                    link->node->SetDirtyFlags(VaultSDLNode::kSDLName);

                if (unsigned bytes = NetCliAuthVaultNodeSave(link->node, nil, nil)) {
                    bytesWritten += bytes;
                    link->node->Print(L"Saving", LogDumpProc, 0);
                }
            }
        }
    }
}

//============================================================================
static hsRef<RelVaultNode> GetChildFolderNode (
    RelVaultNode *  parent,
    unsigned        folderType,
    unsigned        maxDepth
) {
    if (!parent)
        return nullptr;

    return parent->GetChildFolderNode(folderType, maxDepth);
}

//============================================================================
static hsRef<RelVaultNode> GetChildPlayerInfoListNode (
    RelVaultNode *  parent,
    unsigned        folderType,
    unsigned        maxDepth
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
    
    if (IS_NET_SUCCESS(result))
        trans->node = s_nodes.Find(node->GetNodeId())->node;
    else
        trans->node = nil;
    
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
        //LogMsg(kLogError, L"Error fetching node...most likely trying to fetch a nodeid of 0");
    }
    
    --trans->nodesLeft;
//  LogMsg(kLogDebug, L"(Download) %u of %u nodes fetched", trans->nodeCount - trans->nodesLeft, trans->nodeCount);
    
    if (trans->progressCallback) {
        trans->progressCallback(
            trans->nodeCount,
            trans->nodeCount - trans->nodesLeft,
            trans->cbProgressParam
        );
    }
    
    if (!trans->nodesLeft) {
        VaultDump(trans->tag, trans->vaultId, LogDumpProc);

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
        LogMsg(kLogDebug, L"VaultNodeRefsFetched failed: %u (%s)", result, NetErrorToString(result));
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
            RelVaultNodeLink* rootNodeLink = s_nodes.Find(trans->vaultId);
            if (!rootNodeLink || rootNodeLink->node->GetNodeType() == 0) {
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
IRelVaultNode::IRelVaultNode (RelVaultNode * node)
:   node(node)
{
}

//============================================================================
IRelVaultNode::~IRelVaultNode () {
    ASSERT(!parents.Head());
    ASSERT(!children.Head());
}

//============================================================================
void IRelVaultNode::UnlinkFromRelatives () {

    RelVaultNodeLink * link, * next;
    for (link = parents.Head(); link; link = next) {
        next = parents.Next(link);

        // We have the relationship, so make the callbacks
        for (IVaultCallback * cb = s_callbacks.Head(); cb; cb = s_callbacks.Next(cb))
            cb->cb->RemovingChildNode(link->node, this->node);

        link->node->state->Unlink(node);
    }
    for (link = children.Head(); link; link = next) {
        next = children.Next(link);
        link->node->state->Unlink(node);
    }
    
    ASSERT(!parents.Head());
    ASSERT(!children.Head());
}


//============================================================================
void IRelVaultNode::Unlink (RelVaultNode * other) {
    ASSERT(other != node);
    
    RelVaultNodeLink * link;
    if (nil != (link = parents.Find(other->GetNodeId()))) {
        // make them non-findable in our parents table
        link->link.Unlink();
        // remove us from other's tables.
        link->node->state->Unlink(node);
        delete link;
    }
    if (nil != (link = children.Find(other->GetNodeId()))) {
        // make them non-findable in our children table
        link->link.Unlink();
        // remove us from other's tables.
        link->node->state->Unlink(node);
        delete link;
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
    if (state->children.Find(childId))
        return true;
    RelVaultNodeLink * link = state->children.Head();
    for (; link; link = state->children.Next(link))
        if (link->node->IsParentOf(childId, maxDepth - 1))
            return true;
    return false;
}

//============================================================================
bool RelVaultNode::IsChildOf (unsigned parentId, unsigned maxDepth) {
    if (GetNodeId() == parentId)
        return false;
    if (maxDepth == 0)
        return false;
    if (state->parents.Find(parentId))
        return true;
    RelVaultNodeLink * link = state->parents.Head();
    for (; link; link = state->parents.Next(link))
        if (link->node->IsChildOf(parentId, maxDepth - 1))
            return true;
    return false;
}

//============================================================================
void RelVaultNode::GetRootIds (ARRAY(unsigned) * nodeIds) {
    RelVaultNodeLink * link = state->parents.Head();
    if (!link) {
        nodeIds->Add(GetNodeId());
    }
    else {
        for (; link; link = state->parents.Next(link))
            link->node->GetRootIds(nodeIds);
    }
}

//============================================================================
unsigned RelVaultNode::RemoveChildNodes (unsigned maxDepth) {
    hsAssert(false, "eric, implement me.");
    return 0;
}

//============================================================================
void RelVaultNode::GetChildNodeIds (
    ARRAY(unsigned) *   nodeIds,
    unsigned            maxDepth
) {
    if (!maxDepth)
        return;
    RelVaultNodeLink * link = state->children.Head();
    for (; link; link = state->children.Next(link)) {
        nodeIds->Add(link->node->GetNodeId());
        link->node->GetChildNodeIds(nodeIds, maxDepth-1);
    }
}

//============================================================================
void RelVaultNode::GetParentNodeIds (
    ARRAY(unsigned) *   nodeIds,
    unsigned            maxDepth
) {
    if (!maxDepth)
        return;
    RelVaultNodeLink * link = state->parents.Head();
    for (; link; link = state->parents.Next(link)) {
        nodeIds->Add(link->node->GetNodeId());
        link->node->GetParentNodeIds(nodeIds, maxDepth-1);
    }
}


//============================================================================
hsRef<RelVaultNode> RelVaultNode::GetParentNode (
    NetVaultNode *      templateNode,
    unsigned            maxDepth
) {
    if (maxDepth == 0)
        return nullptr;

    RelVaultNodeLink * link;
    link = state->parents.Head();
    for (; link; link = state->parents.Next(link)) {
        if (link->node->Matches(templateNode))
            return link->node;
    }
    
    link = state->parents.Head();
    for (; link; link = state->parents.Next(link)) {
        if (hsRef<RelVaultNode> node = link->node->GetParentNode(templateNode, maxDepth - 1))
            return node;
    }

    return nullptr;
}

//============================================================================
hsRef<RelVaultNode> RelVaultNode::GetChildNode (
    NetVaultNode *      templateNode,
    unsigned            maxDepth
) {
    if (maxDepth == 0)
        return nullptr;

    RelVaultNodeLink * link;
    link = state->children.Head();
    for (; link; link = state->children.Next(link)) {
        if (link->node->Matches(templateNode))
            return link->node;
    }
    
    link = state->children.Head();
    for (; link; link = state->children.Next(link)) {
        if (hsRef<RelVaultNode> node = link->node->GetChildNode(templateNode, maxDepth-1))
            return node;
    }

    return nullptr;
}

//============================================================================
hsRef<RelVaultNode> RelVaultNode::GetChildNode (
    unsigned            nodeType,
    unsigned            maxDepth
) {
    hsRef<NetVaultNode> templateNode = new NetVaultNode;
    templateNode->SetNodeType(nodeType);
    return GetChildNode(templateNode, maxDepth);
}

//============================================================================
hsRef<RelVaultNode> RelVaultNode::GetChildFolderNode (
    unsigned            folderType,
    unsigned            maxDepth
) {
    hsRef<NetVaultNode> templateNode = new NetVaultNode;
    templateNode->SetNodeType(plVault::kNodeType_Folder);
    VaultFolderNode folder(templateNode);
    folder.SetFolderType(folderType);
    return GetChildNode(templateNode, maxDepth);
}

//============================================================================
hsRef<RelVaultNode> RelVaultNode::GetChildPlayerInfoListNode (
    unsigned            folderType,
    unsigned            maxDepth
) {
    hsRef<NetVaultNode> templateNode = new NetVaultNode;
    templateNode->SetNodeType(plVault::kNodeType_PlayerInfoList);
    VaultPlayerInfoListNode access(templateNode);
    access.SetFolderType(folderType);
    return GetChildNode(templateNode, maxDepth);
}

//============================================================================
hsRef<RelVaultNode> RelVaultNode::GetChildAgeInfoListNode (
    unsigned            folderType,
    unsigned            maxDepth
) {
    hsRef<NetVaultNode> templateNode = new NetVaultNode;
    templateNode->SetNodeType(plVault::kNodeType_AgeInfoList);
    VaultAgeInfoListNode access(templateNode);
    access.SetFolderType(folderType);
    return GetChildNode(templateNode, maxDepth);
}

//============================================================================
void RelVaultNode::GetChildNodes (
    unsigned                maxDepth,
    RelVaultNode::RefList * nodes
) {
    if (maxDepth == 0)
        return;

    RelVaultNodeLink * link;
    link = state->children.Head();
    for (; link; link = state->children.Next(link)) {
        nodes->push_back(link->node);
        link->node->GetChildNodes(
            maxDepth - 1,
            nodes
        );
    }
}

//============================================================================
void RelVaultNode::GetChildNodes (
    NetVaultNode *          templateNode,
    unsigned                maxDepth,
    RelVaultNode::RefList * nodes
) {
    RelVaultNodeLink * link;
    link = state->children.Head();
    for (; link; link = state->children.Next(link)) {
        if (link->node->Matches(templateNode))
            nodes->push_back(link->node);

        link->node->GetChildNodes(
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
    hsRef<NetVaultNode> templateNode = new NetVaultNode;
    templateNode->SetNodeType(nodeType);
    GetChildNodes(
        templateNode,
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
    hsRef<NetVaultNode> templateNode = new NetVaultNode;
    templateNode->SetNodeType(plVault::kNodeType_Folder);
    VaultFolderNode fldr(templateNode);
    fldr.SetFolderType(folderType);
    GetChildNodes(
        templateNode,
        maxDepth,
        nodes
    );
}

//============================================================================
unsigned RelVaultNode::GetRefOwnerId (unsigned parentId) {
    // find our parents' link to us and return its ownerId
    if (RelVaultNodeLink * parentLink = state->parents.Find(parentId))
        if (RelVaultNodeLink * childLink = parentLink->node->state->children.Find(GetNodeId()))
            return childLink->ownerId;
    return 0;
}

//============================================================================
bool RelVaultNode::BeenSeen (unsigned parentId) const {
    // find our parents' link to us and return its seen flag
    if (RelVaultNodeLink * parentLink = state->parents.Find(parentId))
        if (RelVaultNodeLink * childLink = parentLink->node->state->children.Find(GetNodeId()))
            return childLink->seen;
    return true;
}

//============================================================================
void RelVaultNode::SetSeen (unsigned parentId, bool seen) {
    // find our parents' link to us and set its seen flag
    if (RelVaultNodeLink * parentLink = state->parents.Find(parentId))
        if (RelVaultNodeLink * childLink = parentLink->node->state->children.Find(GetNodeId()))
            if (childLink->seen != seen) {
                childLink->seen = seen;
                NetCliAuthVaultSetSeen(parentId, GetNodeId(), seen);
            }
}

//============================================================================
template <typename T>
static bool IStrSqlEscape (const T src[], T * dst, unsigned dstChars) {
    // count the number of ' chars
    unsigned ticks = 0;
    {
        const T * cur = src;
        while (*cur) {
            if (*cur == L'\'')
                ++ticks;
            cur++;
        }
    }

    unsigned reqChars = StrLen(src) + ticks + 1;

    if (dstChars < reqChars)
        // failure!
        return false;

    T * cur = dst;

    // copy src to dst, escaping ' chars
    while (*src) {
        if (*src == L'\'') {
            *cur++ = L'\'';
            *cur++ = *src++;
            continue;
        }
        *cur++ = *src++;
    }

    // null-terminate dst string
    *cur = 0;

    // success!
    return true;
}

static void IGetStringFieldValue (
    const wchar_t * value,
    wchar_t *       dst,
    size_t          dstChars
) {
    wchar_t * tmp = (wchar_t*)malloc(sizeof(wchar_t) * dstChars);
    IStrSqlEscape(value, tmp, dstChars);
    swprintf(dst, dstChars, L"'%s'", tmp);
    free(tmp);
}

static void IGetUuidFieldValue (
    const plUUID &  value,
    wchar_t *       dst,
    size_t          dstChars
) {
    swprintf(dst, dstChars, L"hextoraw('%S')", value.AsString().c_str());
}

static void IGetUintFieldValue (
    uint32_t    value,
    wchar_t *   dst,
    size_t      dstChars
) {
    swprintf(dst, dstChars, L"%u", value);
}

static void IGetIntFieldValue (
    int32_t     value,
    wchar_t *   dst,
    size_t      dstChars
) {
    swprintf(dst, dstChars, L"%d", value);
}

void RelVaultNode::Print (const wchar_t tag[], FStateDump dumpProc, unsigned level) {
    wchar_t str[1024];
    StrPrintf(
        str,
        arrsize(str),
        L"%s%*s%*s%u, %S",
        tag ? tag : L"",
        tag ? 1 : 0,
        " ",
        level * 2,
        " ",
        GetNodeId(),
        plVault::NodeTypeStr(GetNodeType(), false)
    );

    for (uint64_t bit = 1; bit; bit <<= 1) {
        if (!(GetFieldFlags() & bit))
            continue;
        if (bit > GetFieldFlags())
            break;

        #define STPRINT(flag, func) case k##flag: { \
                wcsncat(str, L", " L ## #flag L"=", arrsize(str)); \
                const size_t chars = wcslen(str); \
                func(Get##flag(), str + chars, arrsize(str) - chars * sizeof(str[0])); \
            }; break
        #define STNAME(flag) case k##flag: { \
                wcsncat(str, L", " L ## #flag, arrsize(str)); \
            }; break
        switch (bit) {
            STPRINT(NodeId,         IGetUintFieldValue);
            STPRINT(CreateTime,     IGetUintFieldValue);
            STPRINT(ModifyTime,     IGetUintFieldValue);
            STPRINT(CreateAgeName,  IGetStringFieldValue);
            STPRINT(CreateAgeUuid,  IGetUuidFieldValue);
            STPRINT(CreatorAcct,    IGetUuidFieldValue);
            STPRINT(CreatorId,      IGetUintFieldValue);
            STPRINT(NodeType,       IGetUintFieldValue);
            STPRINT(Int32_1,        IGetIntFieldValue);
            STPRINT(Int32_2,        IGetIntFieldValue);
            STPRINT(Int32_3,        IGetIntFieldValue);
            STPRINT(Int32_4,        IGetIntFieldValue);
            STPRINT(UInt32_1,       IGetUintFieldValue);
            STPRINT(UInt32_2,       IGetUintFieldValue);
            STPRINT(UInt32_3,       IGetUintFieldValue);
            STPRINT(UInt32_4,       IGetUintFieldValue);
            STPRINT(Uuid_1,         IGetUuidFieldValue);
            STPRINT(Uuid_2,         IGetUuidFieldValue);
            STPRINT(Uuid_3,         IGetUuidFieldValue);
            STPRINT(Uuid_4,         IGetUuidFieldValue);
            STPRINT(String64_1,     IGetStringFieldValue);
            STPRINT(String64_2,     IGetStringFieldValue);
            STPRINT(String64_3,     IGetStringFieldValue);
            STPRINT(String64_4,     IGetStringFieldValue);
            STPRINT(String64_5,     IGetStringFieldValue);
            STPRINT(String64_6,     IGetStringFieldValue);
            STPRINT(IString64_1,    IGetStringFieldValue);
            STPRINT(IString64_2,    IGetStringFieldValue);
            STNAME(Text_1);
            STNAME(Text_2);
            STNAME(Blob_1);
            STNAME(Blob_2);
            DEFAULT_FATAL(bit);
        }
        #undef STPRINT
    }

    dumpProc(nil, str);
}

//============================================================================
void RelVaultNode::PrintTree (FStateDump dumpProc, unsigned level) {
    Print(L"", dumpProc, level);
    for (RelVaultNodeLink * link = state->children.Head(); link; link = state->children.Next(link))
        link->node->PrintTree(dumpProc, level + 1);
}

//============================================================================
hsRef<RelVaultNode> RelVaultNode::GetParentAgeLink () {

    // this function only makes sense when called on age info nodes
    ASSERT(GetNodeType() == plVault::kNodeType_AgeInfo);

    hsRef<RelVaultNode> result;

    hsRef<NetVaultNode> templateNode = new NetVaultNode;
    templateNode->SetNodeType(plVault::kNodeType_AgeLink);

    // Get our parent AgeLink node  
    if (hsRef<RelVaultNode> rvnLink = GetParentNode(templateNode, 1)) {
        // Get the next AgeLink node in our parent tree
        result = rvnLink->GetParentNode(templateNode, 3);
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
    IVaultCallback * internal = new IVaultCallback;
    internal->cb = cb;
    cb->internal = internal;
    s_callbacks.Link(internal);
}

//============================================================================
void VaultUnregisterCallback (VaultCallback * cb) {
    ASSERT(cb->internal);
    delete cb->internal;
    cb->internal = nil;
}


/*****************************************************************************
*
*   Exports - Initialize
*
***/

//============================================================================
void VaultInitialize () {
    s_running = true;
    
    NetCliAuthVaultSetRecvNodeChangedHandler(VaultNodeChanged);
    NetCliAuthVaultSetRecvNodeAddedHandler(VaultNodeAdded);
    NetCliAuthVaultSetRecvNodeRemovedHandler(VaultNodeRemoved);
    NetCliAuthVaultSetRecvNodeDeletedHandler(VaultNodeDeleted);
}

//============================================================================
void VaultDestroy () {
    s_running = false;

    NetCliAuthVaultSetRecvNodeChangedHandler(nil);
    NetCliAuthVaultSetRecvNodeAddedHandler(nil);
    NetCliAuthVaultSetRecvNodeRemovedHandler(nil);
    NetCliAuthVaultSetRecvNodeDeletedHandler(nil);

    VaultClearDeviceInboxMap();
    
    RelVaultNodeLink * next, * link = s_nodes.Head();
    for (; link; link = next) {
        next = s_nodes.Next(link);
        link->node->state->UnlinkFromRelatives();
        delete link;
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
    NetVaultNode * templateNode
) {
    ASSERT(templateNode);
    RelVaultNodeLink * link = s_nodes.Head();
    while (link) {
        if (link->node->Matches(templateNode))
            return link->node;
        link = s_nodes.Next(link);
    }
    return nullptr;
}

//============================================================================
hsRef<RelVaultNode> VaultGetNode (
    unsigned nodeId
) {
    if (RelVaultNodeLink * link = s_nodes.Find(nodeId))
        return link->node;
    return nullptr;
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

    if (RelVaultNodeLink * parentLink = s_nodes.Find(parentId)) {
        RelVaultNodeLink * childLink = s_nodes.Find(childId);
        if (!childLink) {
            childLink = new RelVaultNodeLink(false, ownerId, childId, new RelVaultNode);
            childLink->node->SetNodeId_NoDirty(childId);
            s_nodes.Add(childLink);
        }
        else if (ownerId) {
            childLink->ownerId = ownerId;
        }

        // We can do a sanity check for a would-be circular link, but it isn't
        // authoritative.  The db will prevent circular links from entering into
        // the persistent state, but because we are hacking in the association
        // before the authoritative check, we're risking the local client operating
        // on bad, possibly harmful vault state.  Not harmful in a national security
        // kinda way, but still harmful.
        if (parentLink->node->IsChildOf(childId, 255)) {
            LogMsg(kLogDebug, L"Node relationship would be circular: p:%u, c:%u", parentId, childId);
            // callback now with error code
            if (callback)
                callback(kNetErrCircularReference, param);
        }
        else if (childLink->node->IsParentOf(parentId, 255)) {
            LogMsg(kLogDebug, L"Node relationship would be circular: p:%u, c:%u", parentId, childId);
            // callback now with error code
            if (callback)
                callback(kNetErrCircularReference, param);
        }
        else {
            NetVaultNodeRef refs[] = {
                { parentId, childId, ownerId }
            };

            ARRAY(unsigned) newNodeIds;
            ARRAY(unsigned) existingNodeIds;

            BuildNodeTree(refs, arrsize(refs), &newNodeIds, &existingNodeIds);
        
            if (!childLink->node->GetNodeType() || !parentLink->node->GetNodeType()) {
                // One or more nodes need to be fetched before the callback is made
                AddChildNodeFetchTrans * trans = new AddChildNodeFetchTrans(callback, param);
                if (!childLink->node->GetNodeType()) {
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
                if (!parentLink->node->GetNodeType()) {
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
        madeCallback ? nil : callback,
        madeCallback ? nil : param
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
        AsyncSleep(10);
    }
    
    if (IS_NET_ERROR(param.result))
        LogMsg(kLogError, L"VaultAddChildNodeAndWait: Failed to add child node: p:%u,c:%u. %s", parentId, childId, NetErrorToString(param.result));
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
        RelVaultNodeLink * parentLink = s_nodes.Find(parentId);
        if (!parentLink)
            break;

        RelVaultNodeLink * childLink = s_nodes.Find(childId);
        if (!childLink)
            break;
            
        if (parentLink->node->IsParentOf(childId, 1)) {
            // We have the relationship, so make the callbacks
            for (IVaultCallback * cb = s_callbacks.Head(); cb; cb = s_callbacks.Next(cb))
                cb->cb->RemovingChildNode(parentLink->node, childLink->node);
        }
            
        parentLink->node->state->Unlink(childLink->node);
        childLink->node->state->Unlink(parentLink->node);
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
void VaultPublishNode (
    unsigned        nodeId,
    const wchar_t     deviceName[]
) {
    hsRef<RelVaultNode> rvn;
    
    rvn = VaultAgeGetDeviceInbox(deviceName);
    if (!rvn) {
        LogMsg(kLogDebug, L"Failed to find inbox for device %s, adding it on-the-fly", deviceName);
        VaultAgeSetDeviceInboxAndWait(deviceName, DEFAULT_DEVICE_INBOX);

        rvn = VaultAgeGetDeviceInbox(deviceName);
        if (!rvn) {
            LogMsg(kLogDebug, L"Failed to add inbox to device %s on-the-fly", deviceName);
            return;
        }
    }
    
    VaultAddChildNode(rvn->GetNodeId(), nodeId, VaultGetPlayerId(), nil, nil);
}

//============================================================================
void VaultSendNode (
    RelVaultNode*   srcNode,
    unsigned        dstPlayerId
) {
    NetCliAuthVaultNodeSave(srcNode, nil, nil);
    NetCliAuthVaultSendNode(srcNode->GetNodeId(), dstPlayerId);
}

//============================================================================
void VaultCreateNode (
    NetVaultNode *              templateNode,
    FVaultCreateNodeCallback    callback,
    void *                      state,
    void *                      param
) {
    VaultCreateNodeTrans * trans = new VaultCreateNodeTrans(callback, state, param);

    if (hsRef<RelVaultNode> age = VaultGetAgeNode()) {
        VaultAgeNode access(age);
        if (!(templateNode->GetFieldFlags() & NetVaultNode::kCreateAgeName))
            templateNode->SetCreateAgeName(access.GetAgeName());
        if (!(templateNode->GetFieldFlags() & NetVaultNode::kCreateAgeUuid))
            templateNode->SetCreateAgeUuid(access.GetAgeInstanceGuid());
    }
    
    NetCliAuthVaultNodeCreate(
        templateNode,
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
    hsRef<RelVaultNode> templateNode = new RelVaultNode;
    templateNode->SetNodeType(nodeType);
    
    VaultCreateNode(
        templateNode,
        callback,
        state,
        param
    );
}

//============================================================================
namespace _VaultCreateNodeAndWait {

struct _CreateNodeParam {
    RelVaultNode *  node;
    ENetError       result;
    bool            complete;
};
static void _CreateNodeCallback (
    ENetError       result,
    void *          ,
    void *          vparam,
    RelVaultNode *  node
) {
    _CreateNodeParam * param = (_CreateNodeParam *)vparam;
    param->node     = node;
    param->result   = result;
    param->complete = true;
}

} // namespace _VaultCreateNodeAndWait

hsRef<RelVaultNode> VaultCreateNodeAndWait (
    NetVaultNode *              templateNode,
    ENetError *                 result
) {
    using namespace _VaultCreateNodeAndWait;
    
    _CreateNodeParam param;
    memset(&param, 0, sizeof(param));
    
    VaultCreateNode(
        templateNode,
        _CreateNodeCallback,
        nil,
        &param
    );
    
    while (!param.complete) {
        NetClientUpdate();
        plgDispatch::Dispatch()->MsgQueueProcess();
        AsyncSleep(10);
    }
    
    *result = param.result;
    return param.node;
}

//============================================================================
hsRef<RelVaultNode> VaultCreateNodeAndWait (
    plVault::NodeTypes          nodeType,
    ENetError *                 result
) {
    hsRef<RelVaultNode> node;
    hsRef<RelVaultNode> templateNode = new RelVaultNode;
    templateNode->SetNodeType(nodeType);

    return VaultCreateNodeAndWait(templateNode, result);
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
    NetVaultNode *      node
) {
    using namespace _VaultForceSaveNodeAndWait;
    
    _SaveNodeParam param;
    memset(&param, 0, sizeof(param));
    
    NetCliAuthVaultNodeSave(
        node,
        _SaveNodeCallback,
        &param
    );
    
    while (!param.complete) {
        NetClientUpdate();
        plgDispatch::Dispatch()->MsgQueueProcess();
        AsyncSleep(10);
    }
}

//============================================================================
void VaultFindNodes (
    NetVaultNode *          templateNode,
    FVaultFindNodeCallback  callback,
    void *                  param
) {
    VaultFindNodeTrans * trans = new VaultFindNodeTrans(callback, param);

    NetCliAuthVaultNodeFind(
        templateNode,
        VaultFindNodeTrans::VaultNodeFound,
        trans
    );  
}

//============================================================================
namespace _VaultFindNodesAndWait {
    struct _FindNodeParam {
        ARRAY(unsigned)     nodeIds;
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
        param->nodeIds.Set(nodeIds, nodeIdCount);
        param->result   = result;
        param->complete = true;
    }

} // namespace _VaultFindNodesAndWait

void VaultFindNodesAndWait (
    NetVaultNode *          templateNode,
    ARRAY(unsigned) *       nodeIds
) {
    using namespace _VaultFindNodesAndWait;

    _FindNodeParam  param;
    NetCliAuthVaultNodeFind(
        templateNode,
        _FindNodeCallback,
        &param
    );

    while (!param.complete) {
        NetClientUpdate();
        plgDispatch::Dispatch()->MsgQueueProcess();
        AsyncSleep(10);
    }

    if (IS_NET_SUCCESS(param.result))
        nodeIds->Add(param.nodeIds.Ptr(), param.nodeIds.Count());
}

//============================================================================
void VaultLocalFindNodes (
    NetVaultNode *          templateNode,
    ARRAY(unsigned) *       nodeIds
) {
    for (RelVaultNodeLink * link = s_nodes.Head(); link != nil; link = s_nodes.Next(link)) {
        if (link->node->Matches(templateNode))
            nodeIds->Add(link->node->GetNodeId());
    }
}

//============================================================================
namespace _VaultFetchNodesAndWait {

    static void _VaultNodeFetched (
        ENetError           result,
        void *              param,
        NetVaultNode *      node
    ) {
        ::VaultNodeFetched(result, nil, node);
        
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
            if (RelVaultNodeLink * link = s_nodes.Find(nodeIds[i])) {
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
        AsyncSleep(10);
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

    wchar_t ageFilename[MAX_PATH];
    wchar_t ageInstName[MAX_PATH];
    wchar_t ageUserName[MAX_PATH];
    wchar_t ageDesc[1024];

    StrToUnicode(ageFilename, info->GetAgeFilename(), arrsize(ageFilename));
    StrToUnicode(ageInstName, info->GetAgeInstanceName(), arrsize(ageInstName));
    StrToUnicode(ageUserName, info->GetAgeUserDefinedName(), arrsize(ageUserName));
    StrToUnicode(ageDesc, info->GetAgeDescription(), arrsize(ageDesc));
    
    NetCliAuthVaultInitAge(
        *info->GetAgeInstanceGuid(),
        parentAgeInstId,
        ageFilename,
        ageInstName,
        ageUserName,
        ageDesc,
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
    hsRef<NetVaultNode> templateNode = new NetVaultNode;
    templateNode->SetNodeType(plVault::kNodeType_VNodeMgrPlayer);
    if (NetCommGetPlayer())
        templateNode->SetNodeId(NetCommGetPlayer()->playerInt);
    return VaultGetNode(templateNode);
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

    hsRef<NetVaultNode> templateNode = new NetVaultNode;
    templateNode->SetNodeType(plVault::kNodeType_PlayerInfo);
    VaultPlayerInfoNode plrInfo(templateNode);
    plrInfo.SetPlayerId(rvnPlr->GetNodeId());

    hsRef<RelVaultNode> result;
    if (hsRef<RelVaultNode> rvnPlrInfo = rvnPlr->GetChildNode(templateNode, 1))
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

    hsRef<NetVaultNode> templateNode = new NetVaultNode;
    
    templateNode->SetNodeType(plVault::kNodeType_AgeInfo);
    VaultAgeInfoNode ageInfo(templateNode);
    wchar_t str[MAX_PATH];
    StrToUnicode(str, kNeighborhoodAgeFilename, arrsize(str));
    ageInfo.SetAgeFilename(str);

    hsRef<RelVaultNode> node;
    if (node = rvnFldr->GetChildNode(templateNode, 2)) {
        VaultAgeInfoNode info(node);
        info.CopyTo(link->GetAgeInfo());
    }

    return node != nil;
}

//============================================================================
bool VaultGetLinkToMyPersonalAge (plAgeLinkStruct * link) {
    hsRef<RelVaultNode> rvnFldr = VaultGetAgesIOwnFolder();
    if (!rvnFldr)
        return false;

    hsRef<NetVaultNode> templateNode = new NetVaultNode;

    templateNode->SetNodeType(plVault::kNodeType_AgeInfo);
    VaultAgeInfoNode ageInfo(templateNode);
    wchar_t str[MAX_PATH];
    StrToUnicode(str, kPersonalAgeFilename, arrsize(str));
    ageInfo.SetAgeFilename(str);

    hsRef<RelVaultNode> node;
    if (node = rvnFldr->GetChildNode(templateNode, 2)) {
        VaultAgeInfoNode info(node);
        info.CopyTo(link->GetAgeInfo());
    }

    return node != nil;
}

//============================================================================
bool VaultGetLinkToCity (plAgeLinkStruct * link) {
    hsRef<RelVaultNode> rvnFldr = VaultGetAgesIOwnFolder();
    if (!rvnFldr)
        return false;

    hsRef<NetVaultNode> templateNode = new NetVaultNode;
    templateNode->SetNodeType(plVault::kNodeType_AgeInfo);

    VaultAgeInfoNode ageInfo(templateNode);
    wchar_t str[MAX_PATH];
    StrToUnicode(str, kCityAgeFilename, arrsize(str));
    ageInfo.SetAgeFilename(str);

    hsRef<RelVaultNode> node;
    if (node = rvnFldr->GetChildNode(templateNode, 2)) {
        VaultAgeInfoNode info(node);
        info.CopyTo(link->GetAgeInfo());
    }

    return node != nil;
}

//============================================================================
hsRef<RelVaultNode> VaultGetOwnedAgeLink (const plAgeInfoStruct * info) {
    
    hsRef<RelVaultNode> rvnLink;
    
    if (hsRef<RelVaultNode> rvnFldr = VaultGetAgesIOwnFolder()) {

        hsRef<NetVaultNode> templateNode = new NetVaultNode;
        templateNode->SetNodeType(plVault::kNodeType_AgeInfo);

        VaultAgeInfoNode ageInfo(templateNode);
        if (info->HasAgeFilename()) {
            wchar_t str[MAX_PATH];
            StrToUnicode(str, info->GetAgeFilename(), arrsize(str));
            ageInfo.SetAgeFilename(str);
        }
        if (info->HasAgeInstanceGuid()) {
            ageInfo.SetAgeInstanceGuid(*info->GetAgeInstanceGuid());
        }

        if (hsRef<RelVaultNode> rvnInfo = rvnFldr->GetChildNode(templateNode, 2)) {
            templateNode->ClearFieldFlags();
            templateNode->SetNodeType(plVault::kNodeType_AgeLink);
            rvnLink = rvnInfo->GetParentNode(templateNode, 1);
        }
    }
    
    return rvnLink;
}

//============================================================================
hsRef<RelVaultNode> VaultGetOwnedAgeInfo (const plAgeInfoStruct * info) {
    
    hsRef<RelVaultNode> rvnInfo;
    
    if (hsRef<RelVaultNode> rvnFldr = VaultGetAgesIOwnFolder()) {

        hsRef<NetVaultNode> templateNode = new NetVaultNode;
        templateNode->SetNodeType(plVault::kNodeType_AgeInfo);

        VaultAgeInfoNode ageInfo(templateNode);
        if (info->HasAgeFilename()) {
            wchar_t str[MAX_PATH];
            StrToUnicode(str, info->GetAgeFilename(), arrsize(str));
            ageInfo.SetAgeFilename(str);
        }
        if (info->HasAgeInstanceGuid()) {
            ageInfo.SetAgeInstanceGuid(*info->GetAgeInstanceGuid());
        }

        rvnInfo = rvnFldr->GetChildNode(templateNode, 2);
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
        if (spawnPt.GetName().IsEmpty())
            break;
        if (spawnPt.GetTitle().IsEmpty())
            break;

        fldr = VaultGetAgesIOwnFolder();
        if (!fldr)
            break;

        ARRAY(unsigned) nodeIds;
        fldr->GetChildNodeIds(&nodeIds, 1);
        
        hsRef<NetVaultNode> templateNode = new NetVaultNode;
        templateNode->SetNodeType(plVault::kNodeType_AgeInfo);
        VaultAgeInfoNode access(templateNode);
        access.SetAgeInstanceGuid(ageInstId);
        
        for (unsigned i = 0; i < nodeIds.Count(); ++i) {
            link = VaultGetNode(nodeIds[i]);
            if (!link)
                continue;
            if (link->GetChildNode(templateNode, 1)) {
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
            NetCliAuthSetAgePublic(rvnInfo->GetNodeId(), publicOrNot);

            VaultAgeInfoNode access(rvnInfo);
            char ageName[MAX_PATH];
            StrToAnsi(ageName, access.GetAgeFilename(), arrsize(ageName));
            
            plVaultNotifyMsg * msg = new plVaultNotifyMsg;
            if (publicOrNot)
                msg->SetType(plVaultNotifyMsg::kPublicAgeCreated);
            else
                msg->SetType(plVaultNotifyMsg::kPublicAgeRemoved);
            msg->SetResultCode(true);
            msg->GetArgs()->AddString(plNetCommon::VaultTaskArgs::kAgeFilename, ageName);
            msg->Send();
        }
    }
    return true;
}

//============================================================================
hsRef<RelVaultNode> VaultGetVisitAgeLink (const plAgeInfoStruct * info) {
    hsRef<RelVaultNode> rvnLink;
    
    if (hsRef<RelVaultNode> rvnFldr = VaultGetAgesICanVisitFolder()) {

        hsRef<NetVaultNode> templateNode = new NetVaultNode;
        templateNode->SetNodeType(plVault::kNodeType_AgeInfo);

        VaultAgeInfoNode ageInfo(templateNode);
        if (info->HasAgeFilename()) {
            wchar_t str[MAX_PATH];
            StrToUnicode(str, info->GetAgeFilename(), arrsize(str));
            ageInfo.SetAgeFilename(str);
        }
        if (info->HasAgeInstanceGuid()) {
            ageInfo.SetAgeInstanceGuid(*info->GetAgeInstanceGuid());
        }

        if (hsRef<RelVaultNode> rvnInfo = rvnFldr->GetChildNode(templateNode, 2)) {
            templateNode->ClearFieldFlags();
            templateNode->SetNodeType(plVault::kNodeType_AgeLink);  
            rvnLink = rvnInfo->GetParentNode(templateNode, 1);
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
    RelVaultNode *  node
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
    
    bool result = false;
    
    for (;;) {
        if (hsRef<RelVaultNode> rvn = VaultGetAgesIOwnFolder())
            agesIOwnId = rvn->GetNodeId();
        else {
            LogMsg(kLogError, L"RegisterOwnedAge: Failed to get player's AgesIOwnFolder");
            break;
        }

        // Check for existing link to this age  
        plAgeLinkStruct existing;
        if (VaultGetOwnedAgeLink(link->GetAgeInfo(), &existing)) {
            result = true;
            break;
        }
        
        {   // Init age vault
            _InitAgeParam   param;
            memset(&param, 0, sizeof(param));

            VaultInitAge(
                link->GetAgeInfo(),
                kNilUuid,
                _InitAgeCallback,
                nil,
                &param
            );

            while (!param.complete) {
                NetClientUpdate();
                plgDispatch::Dispatch()->MsgQueueProcess();
                AsyncSleep(10);
            }
            
            if (IS_NET_ERROR(param.result)) {
                LogMsg(kLogError, "RegisterOwnedAge: Failed to init age %s", link->GetAgeInfo()->GetAgeFilename().c_str());
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
                nil,
                &param
            );

            while (!param.complete) {
                NetClientUpdate();
                plgDispatch::Dispatch()->MsgQueueProcess();
                AsyncSleep(10);
            }
            
            if (IS_NET_ERROR(param.result)) {
                LogMsg(kLogError, L"RegisterOwnedAge: Failed create age link node");
                break;
            }
                
            ageLinkId = param.nodeId;
        }       

        {   // Fetch age info node tree
            _FetchVaultParam    param;
            memset(&param, 0, sizeof(param));
            
            VaultDownload(
                L"RegisterOwnedAge",
                ageInfoId,
                _FetchVaultCallback,
                &param,
                nil,
                nil
            );
            
            while (!param.complete) {
                NetClientUpdate();
                plgDispatch::Dispatch()->MsgQueueProcess();
                AsyncSleep(10);
            }
            
            if (IS_NET_ERROR(param.result)) {
                LogMsg(kLogError, L"RegisterOwnedAge: Failed to download age info vault");
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
                AsyncSleep(10);
            }
            
            if (IS_NET_ERROR(param1.result)) {
                LogMsg(kLogError, L"RegisterOwnedAge: Failed to add link to player's bookshelf");
                break;
            }
            if (IS_NET_ERROR(param2.result)) {
                LogMsg(kLogError, L"RegisterOwnedAge: Failed to add info to link");
                break;
            }
            if (IS_NET_ERROR(param3.result)) {
                LogMsg(kLogError, L"RegisterOwnedAge: Failed to add playerInfo to ageOwners");
                break;
            }
        }

        // Copy the link spawn point to the link node       
        if (hsRef<RelVaultNode> node = VaultGetNode(ageLinkId)) {
            VaultAgeLinkNode access(node);
            access.AddSpawnPoint(link->SpawnPoint());
        }
        
        result = true;
        break;
    }
        
    plVaultNotifyMsg * msg = new plVaultNotifyMsg;
    msg->SetType(plVaultNotifyMsg::kRegisteredOwnedAge);
    msg->SetResultCode(result);
    msg->GetArgs()->AddInt(plNetCommon::VaultTaskArgs::kAgeLinkNode, ageLinkId);
    msg->Send();

    return result;
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

    void _CreateAgeLinkNode(ENetError result, void* state, void* param, RelVaultNode* node) {
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
        VaultAddChildNode(agesIOwn->GetNodeId(), node->GetNodeId(), 0, (FVaultAddChildNodeCallback)_AddAgeLinkNode, nil); 
        VaultAddChildNode(node->GetNodeId(), (uint32_t)((uintptr_t)p->fAgeInfoId), 0, (FVaultAddChildNodeCallback)_AddAgeInfoNode, nil);

        // Add our PlayerInfo to important places
        if (hsRef<RelVaultNode> rvnAgeInfo = VaultGetNode((uint32_t)((uintptr_t)p->fAgeInfoId))) {
            if (hsRef<RelVaultNode> rvnAgeOwners = rvnAgeInfo->GetChildPlayerInfoListNode(plVault::kAgeOwnersFolder, 1))
                VaultAddChildNode(rvnAgeOwners->GetNodeId(), plyrInfo->GetNodeId(), 0, (FVaultAddChildNodeCallback)_AddPlayerInfoNode, nil);
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
            VaultCreateNode(plVault::kNodeType_AgeLink, (FVaultCreateNodeCallback)_CreateAgeLinkNode, nil, param);
    }

    void _InitAgeCallback(ENetError result, void* state, void* param, uint32_t ageVaultId, uint32_t ageInfoVaultId) {
        if (IS_NET_SUCCESS(result)) {
            _Params* p = new _Params();
            p->fAgeInfoId = (void*)ageInfoVaultId;
            p->fSpawn = (plSpawnPointInfo*)param;

            VaultDownload(
                L"RegisterOwnedAge",
                ageInfoVaultId,
                (FVaultDownloadCallback)_DownloadCallback,
                p,
                nil,
                nil);
        } else
            LogMsg(kLogError, "VaultRegisterOwnedAge: Failed to init age (async)");
    }
}; // namespace _VaultRegisterOwnedAge

void VaultRegisterOwnedAge(const plAgeLinkStruct* link) {
    using namespace _VaultRegisterOwnedAge;

    hsRef<RelVaultNode> agesIOwn = VaultGetAgesIOwnFolder();
    if (agesIOwn == nil) {
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
        nil,
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
    RelVaultNode *  node
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
    
    bool result = false;
    for (;;) {
        if (hsRef<RelVaultNode> rvn = VaultGetAgesICanVisitFolder())
            agesICanVisitId = rvn->GetNodeId();
        else {
            LogMsg(kLogError, L"RegisterVisitAge: Failed to get player's AgesICanVisitFolder");
            break;
        }

        // Check for existing link to this age  
        plAgeLinkStruct existing;
        if (VaultGetVisitAgeLink(link->GetAgeInfo(), &existing)) {
            result = true;
            break;
        }
        
        
        {   // Init age vault
            _InitAgeParam   param;
            memset(&param, 0, sizeof(param));

            VaultInitAge(
                link->GetAgeInfo(),
                kNilUuid,
                _InitAgeCallback,
                nil,
                &param
            );

            while (!param.complete) {
                NetClientUpdate();
                plgDispatch::Dispatch()->MsgQueueProcess();
                AsyncSleep(10);
            }
            
            if (IS_NET_ERROR(param.result)) {
                LogMsg(kLogError, "RegisterVisitAge: Failed to init age %s", link->GetAgeInfo()->GetAgeFilename().c_str());
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
                nil,
                &param
            );

            while (!param.complete) {
                NetClientUpdate();
                plgDispatch::Dispatch()->MsgQueueProcess();
                AsyncSleep(10);
            }
            
            if (IS_NET_ERROR(param.result)) {
                LogMsg(kLogError, L"RegisterVisitAge: Failed create age link node");
                break;
            }
                
            ageLinkId = param.nodeId;
        }       

        {   // Fetch age info node tree
            _FetchVaultParam    param;
            memset(&param, 0, sizeof(param));
            
            VaultDownload(
                L"RegisterVisitAge",
                ageInfoId,
                _FetchVaultCallback,
                &param,
                nil,
                nil
            );
            
            while (!param.complete) {
                NetClientUpdate();
                plgDispatch::Dispatch()->MsgQueueProcess();
                AsyncSleep(10);
            }
            
            if (IS_NET_ERROR(param.result)) {
                LogMsg(kLogError, L"RegisterVisitAge: Failed to download age info vault");
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
                AsyncSleep(10);
            }
            
            if (IS_NET_ERROR(param1.result)) {
                LogMsg(kLogError, L"RegisterVisitAge: Failed to add link to folder");
                break;
            }
            if (IS_NET_ERROR(param2.result)) {
                LogMsg(kLogError, L"RegisterVisitAge: Failed to add info to link");
                break;
            }
            if (IS_NET_ERROR(param3.result)) {
                LogMsg(kLogError, L"RegisterVisitAge: Failed to add playerInfo to canVisit folder");
                break;
            }
        }

        // Copy the link spawn point to the link node       
        if (hsRef<RelVaultNode> node = VaultGetNode(ageLinkId)) {
            VaultAgeLinkNode access(node);
            access.AddSpawnPoint(link->SpawnPoint());
        }

        result = true;
        break;
    }

    plVaultNotifyMsg * msg = new plVaultNotifyMsg;
    msg->SetType(plVaultNotifyMsg::kRegisteredVisitAge);
    msg->SetResultCode(result);
    msg->GetArgs()->AddInt(plNetCommon::VaultTaskArgs::kAgeLinkNode, ageLinkId);
    msg->Send();

    return result;
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

    void _CreateAgeLinkNode(ENetError result, void* state, void* param, RelVaultNode* node) {
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
                VaultAddChildNode(canVisit->GetNodeId(), playerInfo->GetNodeId(), 0, nil, nil);
        }

        // Get our AgesICanVisit folder
        if (hsRef<RelVaultNode> iCanVisit = VaultGetAgesICanVisitFolder()) {
            VaultAddChildNode(node->GetNodeId(), ageInfo->GetNodeId(), 0, nil, nil);
            VaultAddChildNode(iCanVisit->GetNodeId(), node->GetNodeId(), 0, nil, nil);
        }

        // Update the AgeLink with a spawn point
        VaultAgeLinkNode access(node);
        access.AddSpawnPoint(*p->fSpawn);

        // Send out the VaultNotify msg
        plVaultNotifyMsg * msg = new plVaultNotifyMsg;
        msg->SetType(plVaultNotifyMsg::kRegisteredVisitAge);
        msg->SetResultCode(true);
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
        VaultCreateNode(plVault::kNodeType_AgeLink, (FVaultCreateNodeCallback)_CreateAgeLinkNode, nil, param);
    }
    
    void _InitAgeCallback(ENetError result, void* state, void* param, uint32_t ageVaultId, uint32_t ageInfoId) {
        if (IS_NET_ERROR(result)) {
            LogMsg(kLogError, "RegisterVisitAge: Failed to init age vault (async)");
            delete (_Params*)param;
            return;
        }

        // Save the AgeInfo nodeID, then download the age vault
        _Params* p = (_Params*)param;
        p->fAgeInfoId = (void*)ageInfoId;
        
        VaultDownload(L"RegisterVisitAge",
                      ageInfoId,
                      (FVaultDownloadCallback)_DownloadCallback,
                      param,
                      nil,
                      nil
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
                 nil,
                 p
    );
}

//============================================================================
bool VaultUnregisterOwnedAgeAndWait (const plAgeInfoStruct * info) {

    unsigned ageLinkId = 0;
    unsigned agesIOwnId;

    bool result = false;
    for (;;) {  
        hsRef<RelVaultNode> rvnLink = VaultGetOwnedAgeLink(info);
        if (!rvnLink) {
            result = true;
            break;  // we aren't an owner of the age, just return true
        }

        if (hsRef<RelVaultNode> rvn = VaultGetAgesIOwnFolder())
            agesIOwnId = rvn->GetNodeId();
        else {
            LogMsg(kLogError, L"UnregisterOwnedAge: Failed to get player's AgesIOwnFolder");
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
        VaultRemoveChildNode(ageOwnersId, playerInfoId, nil, nil);
        
        // remove the link from AgesIOwn folder 
        VaultRemoveChildNode(agesIOwnId, ageLinkId, nil, nil);

        // delete the link node since link nodes aren't shared with anyone else
    //  VaultDeleteNode(ageLinkId);

        result = true;
        break;
    }
    
    plVaultNotifyMsg * msg = new plVaultNotifyMsg;
    msg->SetType(plVaultNotifyMsg::kUnRegisteredOwnedAge);
    msg->SetResultCode(result);
    msg->GetArgs()->AddInt(plNetCommon::VaultTaskArgs::kAgeLinkNode, ageLinkId);
    msg->Send();
    
    return result;
}

//============================================================================
bool VaultUnregisterVisitAgeAndWait (const plAgeInfoStruct * info) {

    unsigned ageLinkId = 0;
    unsigned agesICanVisitId;
    
    bool result = false;
    for (;;) {
        hsRef<RelVaultNode> rvnLink = VaultGetVisitAgeLink(info);
        if (!rvnLink) {
            result = true;
            break;  // we aren't an owner of the age, just return true
        }

        if (hsRef<RelVaultNode> rvn = VaultGetAgesICanVisitFolder())
            agesICanVisitId = rvn->GetNodeId();
        else {
            LogMsg(kLogError, L"UnregisterOwnedAge: Failed to get player's AgesICanVisitFolder");
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
        VaultRemoveChildNode(ageVisitorsId, playerInfoId, nil, nil);

        // remove the link from AgesICanVisit folder    
        VaultRemoveChildNode(agesICanVisitId, ageLinkId, nil, nil);
        
        // delete the link node since link nodes aren't shared with anyone else
    //  VaultDeleteNode(ageLinkId);
    
        result = true;
        break;
    }
    
    plVaultNotifyMsg * msg = new plVaultNotifyMsg;
    msg->SetType(plVaultNotifyMsg::kUnRegisteredVisitAge);
    msg->SetResultCode(result);
    msg->GetArgs()->AddInt(plNetCommon::VaultTaskArgs::kAgeLinkNode, ageLinkId);
    msg->Send();

    return result;
}

//============================================================================
hsRef<RelVaultNode> VaultFindChronicleEntry (const wchar_t entryName[], int entryType) {

    hsRef<RelVaultNode> result;
    if (hsRef<RelVaultNode> rvnFldr = GetChildFolderNode(GetPlayerNode(), plVault::kChronicleFolder, 1)) {
        hsRef<NetVaultNode> templateNode = new NetVaultNode;
        templateNode->SetNodeType(plVault::kNodeType_Chronicle);
        VaultChronicleNode chrn(templateNode);
        chrn.SetEntryName(entryName);
        if (entryType >= 0)
            chrn.SetEntryType(entryType);
        if (hsRef<RelVaultNode> rvnChrn = rvnFldr->GetChildNode(templateNode, 255))
            result = rvnChrn;
    }
    return result;
}

//============================================================================
bool VaultHasChronicleEntry (const wchar_t entryName[], int entryType) {
    if (VaultFindChronicleEntry(entryName, entryType))
        return true;
    return false;
}

//============================================================================
void VaultAddChronicleEntryAndWait (
    const wchar_t entryName[],
    int         entryType,
    const wchar_t entryValue[]
) {
    if (hsRef<RelVaultNode> rvnChrn = VaultFindChronicleEntry(entryName, entryType)) {
        VaultChronicleNode chrnNode(rvnChrn);
        chrnNode.SetEntryValue(entryValue);
    }
    else if (hsRef<RelVaultNode> rvnFldr = GetChildFolderNode(GetPlayerNode(), plVault::kChronicleFolder, 1)) {
        hsRef<NetVaultNode> templateNode = new NetVaultNode;
        templateNode->SetNodeType(plVault::kNodeType_Chronicle);
        VaultChronicleNode chrnNode(templateNode);
        chrnNode.SetEntryName(entryName);
        chrnNode.SetEntryType(entryType);
        chrnNode.SetEntryValue(entryValue);
        ENetError result;
        if (hsRef<RelVaultNode> rvnChrn = VaultCreateNodeAndWait(templateNode, &result))
            VaultAddChildNode(rvnFldr->GetNodeId(), rvnChrn->GetNodeId(), 0, nil, nil);
    }
}

//============================================================================
bool VaultAmIgnoringPlayer (unsigned playerId) {
    bool retval = false;
    if (hsRef<RelVaultNode> rvnFldr = GetChildPlayerInfoListNode(GetPlayerNode(), plVault::kIgnoreListFolder, 1)) {
        hsRef<NetVaultNode> templateNode = new NetVaultNode;
        templateNode->SetNodeType(plVault::kNodeType_PlayerInfo);
        VaultPlayerInfoNode pinfoNode(templateNode);
        pinfoNode.SetPlayerId(playerId);

        if (rvnFldr->GetChildNode(templateNode, 1))
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
void VaultDump (const wchar_t tag[], unsigned vaultId, FStateDump dumpProc) {
    LogMsg(kLogDebug, L"<---- ID:%u, Begin Vault%*s%s ---->", vaultId, tag ? 1 : 0, L" ", tag);

    if (hsRef<RelVaultNode> rvn = VaultGetNode(vaultId))
        rvn->PrintTree(dumpProc, 0);

    LogMsg(kLogDebug, L"<---- ID:%u, End Vault%*s%s ---->", vaultId, tag ? 1 : 0, L" ", tag);
}

//============================================================================
void VaultDump (const wchar_t tag[], unsigned vaultId) {
    VaultDump (tag, vaultId, LogDumpProc);
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
        info.SetAgeFilename(plString::FromWchar(curAgeInfo.GetAgeFilename()));

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
    const wchar_t stationName[],
    const wchar_t linkBackSpawnPtObjName[]
) {
    plAgeInfoStruct info;
    info.SetAgeFilename(kCityAgeFilename);
    if (hsRef<RelVaultNode> rvn = VaultGetOwnedAgeLink(&info)) {
        char title[MAX_PATH], spawnPt[MAX_PATH];
        StrToAnsi(title, stationName, arrsize(title));
        StrToAnsi(spawnPt, linkBackSpawnPtObjName, arrsize(spawnPt));
        VaultAgeLinkNode link(rvn);
        link.AddSpawnPoint(plSpawnPointInfo(title, spawnPt));
        return true;
    }
    return false;
}

//============================================================================
void VaultProcessVisitNote(RelVaultNode * rvnVisit) {
    if (hsRef<RelVaultNode> rvnInbox = VaultGetPlayerInboxFolder()) {
        VaultTextNoteNode visitAcc(rvnVisit);
        plAgeLinkStruct link;
        if (visitAcc.GetVisitInfo(link.GetAgeInfo())) {
            // Add it to our "ages i can visit" folder
            VaultRegisterVisitAge(&link);
        }
        // remove it from the inbox
        VaultRemoveChildNode(rvnInbox->GetNodeId(), rvnVisit->GetNodeId(), nil, nil);
    }
}

//============================================================================
void VaultProcessUnvisitNote(RelVaultNode * rvnUnVisit) {
    if (hsRef<RelVaultNode> rvnInbox = VaultGetPlayerInboxFolder()) {
        VaultTextNoteNode unvisitAcc(rvnUnVisit);
        plAgeInfoStruct info;
        if (unvisitAcc.GetVisitInfo(&info)) {
            // Remove it from our "ages i can visit" folder
            VaultUnregisterVisitAgeAndWait(&info);
        }
        // remove it from the inbox
        VaultRemoveChildNode(rvnInbox->GetNodeId(), rvnUnVisit->GetNodeId(), nil, nil);
    }
}

//============================================================================
void VaultProcessPlayerInbox () {
    if (hsRef<RelVaultNode> rvnInbox = VaultGetPlayerInboxFolder()) {
        {   // Process new visit requests
            RelVaultNode::RefList visits;
            hsRef<RelVaultNode> templateNode = new RelVaultNode;
            templateNode->SetNodeType(plVault::kNodeType_TextNote);
            VaultTextNoteNode tmpAcc(templateNode);
            tmpAcc.SetNoteType(plVault::kNoteType_Visit);
            rvnInbox->GetChildNodes(templateNode, 1, &visits);

            for (const hsRef<RelVaultNode> &rvnVisit : visits) {
                VaultTextNoteNode visitAcc(rvnVisit);
                plAgeLinkStruct link;
                if (visitAcc.GetVisitInfo(link.GetAgeInfo())) {
                    // Add it to our "ages i can visit" folder
                    VaultRegisterVisitAge(&link);
                }
                // remove it from the inbox
                VaultRemoveChildNode(rvnInbox->GetNodeId(), rvnVisit->GetNodeId(), nil, nil);
            }
        }
        {   // Process new unvisit requests
            RelVaultNode::RefList unvisits;
            hsRef<RelVaultNode> templateNode = new RelVaultNode;
            templateNode->SetNodeType(plVault::kNodeType_TextNote);
            VaultTextNoteNode tmpAcc(templateNode);
            tmpAcc.SetNoteType(plVault::kNoteType_UnVisit);
            rvnInbox->GetChildNodes(templateNode, 1, &unvisits);

            for (const hsRef<RelVaultNode> &rvnUnVisit : unvisits) {
                VaultTextNoteNode unvisitAcc(rvnUnVisit);
                plAgeInfoStruct info;
                if (unvisitAcc.GetVisitInfo(&info)) {
                    // Remove it from our "ages i can visit" folder
                    VaultUnregisterVisitAgeAndWait(&info);
                }
                // remove it from the inbox
                VaultRemoveChildNode(rvnInbox->GetNodeId(), rvnUnVisit->GetNodeId(), nil, nil);
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
    hsRef<NetVaultNode> templateNode = new NetVaultNode;
    templateNode->SetNodeType(plVault::kNodeType_VNodeMgrAge);
    if (NetCommGetAge())
        templateNode->SetNodeId(NetCommGetAge()->ageVaultId);
    return VaultGetNode(templateNode);
}

//============================================================================
hsRef<RelVaultNode> VaultGetAgeNode () {
    hsRef<RelVaultNode> result;
    hsRef<NetVaultNode> templateNode = new NetVaultNode;
    templateNode->SetNodeType(plVault::kNodeType_VNodeMgrAge);
    if (NetCommGetAge())
        templateNode->SetNodeId(NetCommGetAge()->ageVaultId);
    if (hsRef<RelVaultNode> rvnAge = VaultGetNode(templateNode))
        result = rvnAge;
    return result;
}

//============================================================================
static hsRef<RelVaultNode> GetAgeInfoNode () {
    hsRef<RelVaultNode> rvnAge = VaultGetAgeNode();
    if (!rvnAge)
        return nullptr;

    hsRef<RelVaultNode> result;
    hsRef<NetVaultNode> templateNode = new NetVaultNode;
    
    templateNode->SetNodeType(plVault::kNodeType_AgeInfo);
    templateNode->SetCreatorId(rvnAge->GetNodeId());
            
    if (hsRef<RelVaultNode> rvnAgeInfo = rvnAge->GetChildNode(templateNode, 1))
        result = rvnAgeInfo;
    
    return result;
}

//============================================================================
hsRef<RelVaultNode> VaultGetAgeInfoNode () {
    hsRef<RelVaultNode> rvnAge = VaultGetAgeNode();
    if (!rvnAge)
        return nullptr;

    hsRef<RelVaultNode> result;
    hsRef<NetVaultNode> templateNode = new NetVaultNode;
    templateNode->SetNodeType(plVault::kNodeType_AgeInfo);
    templateNode->SetCreatorId(rvnAge->GetNodeId());
            
    if (hsRef<RelVaultNode> rvnAgeInfo = rvnAge->GetChildNode(templateNode, 1))
        result = rvnAgeInfo;

    return result;
}

//============================================================================
hsRef<RelVaultNode> VaultGetAgeChronicleFolder () {
    if (hsRef<RelVaultNode> rvn = GetAgeNode())
        return rvn->GetChildFolderNode(plVault::kChronicleFolder, 1);
    return nil;
}

//============================================================================
hsRef<RelVaultNode> VaultGetAgeDevicesFolder () {
    if (hsRef<RelVaultNode> rvn = GetAgeNode())
        return rvn->GetChildFolderNode(plVault::kAgeDevicesFolder, 1);
    return nil;
}

//============================================================================
hsRef<RelVaultNode> VaultGetAgeAgeOwnersFolder () {
    if (hsRef<RelVaultNode> rvn = GetAgeInfoNode())
        return rvn->GetChildPlayerInfoListNode(plVault::kAgeOwnersFolder, 1);
    return nil;
}

//============================================================================
hsRef<RelVaultNode> VaultGetAgeCanVisitFolder () {
    if (hsRef<RelVaultNode> rvn = GetAgeInfoNode())
        return rvn->GetChildPlayerInfoListNode(plVault::kCanVisitFolder, 1);
    return nil;
}

//============================================================================
hsRef<RelVaultNode> VaultGetAgePeopleIKnowAboutFolder () {
    if (hsRef<RelVaultNode> rvn = GetAgeNode())
        return rvn->GetChildPlayerInfoListNode(plVault::kPeopleIKnowAboutFolder, 1);
    return nil;
}

//============================================================================
hsRef<RelVaultNode> VaultGetAgeSubAgesFolder () {
    if (hsRef<RelVaultNode> rvn = GetAgeNode())
        return rvn->GetChildAgeInfoListNode(plVault::kSubAgesFolder, 1);
    return nil;
}

//============================================================================
hsRef<RelVaultNode> VaultGetAgePublicAgesFolder () {
    hsAssert(false, "eric, implement me");
    return nil;
}

//============================================================================
hsRef<RelVaultNode> VaultAgeGetBookshelfFolder () {
    if (hsRef<RelVaultNode> rvn = GetAgeNode())
        return rvn->GetChildAgeInfoListNode(plVault::kAgesIOwnFolder, 1);
    return nil;
}

//============================================================================
hsRef<RelVaultNode> VaultFindAgeSubAgeLink (const plAgeInfoStruct * info) {
    hsRef<RelVaultNode> rvnLink;
    
    if (hsRef<RelVaultNode> rvnFldr = VaultGetAgeSubAgesFolder()) {

        hsRef<NetVaultNode> templateNode = new NetVaultNode;
        templateNode->SetNodeType(plVault::kNodeType_AgeInfo);

        VaultAgeInfoNode ageInfo(templateNode);
        wchar_t str[MAX_PATH];
        StrToUnicode(str, info->GetAgeFilename(), arrsize(str));
        ageInfo.SetAgeFilename(str);

        if (hsRef<RelVaultNode> rvnInfo = rvnFldr->GetChildNode(templateNode, 2)) {
            templateNode->ClearFieldFlags();
            templateNode->SetNodeType(plVault::kNodeType_AgeLink);  
            rvnLink = rvnInfo->GetParentNode(templateNode, 1);
        }
    }
    
    return rvnLink;
}

//============================================================================
hsRef<RelVaultNode> VaultFindAgeChronicleEntry (const wchar_t entryName[], int entryType) {
    hsAssert(false, "eric, implement me");
    return nil;
}

//============================================================================
void VaultAddAgeChronicleEntry (
    const wchar_t entryName[],
    int         entryType,
    const wchar_t entryValue[]
) {
    hsAssert(false, "eric, implement me");
}

//============================================================================
hsRef<RelVaultNode> VaultAgeAddDeviceAndWait (const wchar_t deviceName[]) {
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
void VaultAgeRemoveDevice (const wchar_t deviceName[]) {
    if (hsRef<RelVaultNode> folder = VaultGetAgeDevicesFolder()) {
        hsRef<NetVaultNode> templateNode = new NetVaultNode;
        templateNode->SetNodeType(plVault::kNodeType_TextNote);
        VaultTextNoteNode access(templateNode);
        access.SetNoteTitle(deviceName);
        if (hsRef<RelVaultNode> device = folder->GetChildNode(templateNode, 1)) {
            VaultRemoveChildNode(folder->GetNodeId(), device->GetNodeId(), nil, nil);

            if (DeviceInbox * deviceInbox = s_ageDeviceInboxes.Find(CHashKeyStr(deviceName)))
                delete device;
        }
    }
}

//============================================================================
bool VaultAgeHasDevice (const wchar_t deviceName[]) {
    bool found = false;
    if (hsRef<RelVaultNode> folder = VaultGetAgeDevicesFolder()) {
        hsRef<NetVaultNode> templateNode = new NetVaultNode;
        templateNode->SetNodeType(plVault::kNodeType_TextNote);
        VaultTextNoteNode access(templateNode);
        access.SetNoteTitle(deviceName);
        if (folder->GetChildNode(templateNode, 1))
            found = true;
    }
    return found;
}

//============================================================================
hsRef<RelVaultNode> VaultAgeGetDevice (const wchar_t deviceName[]) {
    hsRef<RelVaultNode> result;
    if (hsRef<RelVaultNode> folder = VaultGetAgeDevicesFolder()) {
        hsRef<NetVaultNode> templateNode = new NetVaultNode;
        templateNode->SetNodeType(plVault::kNodeType_TextNote);
        VaultTextNoteNode access(templateNode);
        access.SetNoteTitle(deviceName);
        if (hsRef<RelVaultNode> device = folder->GetChildNode(templateNode, 1))
            result = device;
    }
    return result;
}

//============================================================================
hsRef<RelVaultNode> VaultAgeSetDeviceInboxAndWait (const wchar_t deviceName[], const wchar_t inboxName[]) {
    DeviceInbox * devInbox = s_ageDeviceInboxes.Find(CHashKeyStr(deviceName));
    if (devInbox) {
        StrCopy(devInbox->inboxName, inboxName, arrsize(devInbox->inboxName));
    }
    else {
        devInbox = new DeviceInbox(deviceName, inboxName);
        s_ageDeviceInboxes.Add(devInbox);
    }

    // if we found the inbox or its a global inbox then return here, otherwise if its the default inbox and
    // it wasn't found then continue on and create the inbox
    hsRef<RelVaultNode> existing = VaultAgeGetDeviceInbox(deviceName);
    if (existing || StrCmp(inboxName, DEFAULT_DEVICE_INBOX) != 0)
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
hsRef<RelVaultNode> VaultAgeGetDeviceInbox (const wchar_t deviceName[]) {
    hsRef<RelVaultNode> result;
    DeviceInbox * devInbox = s_ageDeviceInboxes.Find(CHashKeyStr(deviceName));

    if (devInbox)
    {
        hsRef<RelVaultNode> parentNode;
        const wchar_t * inboxName = nil;

        if (StrCmp(devInbox->inboxName, DEFAULT_DEVICE_INBOX) == 0) {
            parentNode = VaultAgeGetDevice(deviceName);
        }
        else {
            parentNode = VaultGetGlobalInbox();
        }

        if (parentNode) {
            hsRef<NetVaultNode> templateNode = new NetVaultNode;
            templateNode->SetNodeType(plVault::kNodeType_Folder);
            VaultFolderNode access(templateNode);
            access.SetFolderType(plVault::kDeviceInboxFolder);
            access.SetFolderName(devInbox->inboxName);
            result = parentNode->GetChildNode(templateNode, 1);
        }
    }
    return result;
}

//============================================================================
void VaultClearDeviceInboxMap () {
    while (DeviceInbox * inbox = s_ageDeviceInboxes.Head()) {
        delete inbox;
    }
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

        hsRef<NetVaultNode> templateNode = new NetVaultNode;
        templateNode->SetNodeType(plVault::kNodeType_AgeInfo);

        VaultAgeInfoNode ageInfo(templateNode);
        wchar_t str[MAX_PATH];
        StrToUnicode(str, info->GetAgeFilename(), arrsize(str));
        ageInfo.SetAgeFilename(str);

        if (hsRef<RelVaultNode> rvnInfo = rvnFldr->GetChildNode(templateNode, 2)) {
            templateNode->ClearFieldFlags();
            templateNode->SetNodeType(plVault::kNodeType_AgeLink);  
            rvnLink = rvnInfo->GetParentNode(templateNode, 1);
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
    RelVaultNode *  node
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
        LogMsg(kLogError, L"CreateSubAge: Failed to get ages's SubAges folder");
        return false;
    }
    
    {   // Init age vault
        _InitAgeParam   param;
        memset(&param, 0, sizeof(param));
        
        VaultInitAge(
            info,
            parentAgeInstId,
            _InitAgeCallback,
            nil,
            &param
        );

        while (!param.complete) {
            NetClientUpdate();
            plgDispatch::Dispatch()->MsgQueueProcess();
            AsyncSleep(10);
        }
        
        if (IS_NET_ERROR(param.result)) {
            LogMsg(kLogError, "CreateSubAge: Failed to init age %s", link->GetAgeInfo()->GetAgeFilename().c_str());
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
            nil,
            &param
        );

        while (!param.complete) {
            NetClientUpdate();
            plgDispatch::Dispatch()->MsgQueueProcess();
            AsyncSleep(10);
        }
        
        if (IS_NET_ERROR(param.result)) {
            LogMsg(kLogError, L"CreateSubAge: Failed create age link node");
            return false;
        }
            
        ageLinkId = param.nodeId;
    }       

    {   // Fetch age info node tree
        _FetchVaultParam    param;
        memset(&param, 0, sizeof(param));
        
        VaultDownload(
            L"CreateSubAge",
            ageInfoId,
            _FetchVaultCallback,
            &param,
            nil,
            nil
        );
        
        while (!param.complete) {
            NetClientUpdate();
            plgDispatch::Dispatch()->MsgQueueProcess();
            AsyncSleep(10);
        }
        
        if (IS_NET_ERROR(param.result)) {
            LogMsg(kLogError, L"CreateSubAge: Failed to download age info vault");
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
            AsyncSleep(10);
        }
        
        if (IS_NET_ERROR(param1.result)) {
            LogMsg(kLogError, L"CreateSubAge: Failed to add link to ages's subages");
            return false;
        }
        if (IS_NET_ERROR(param2.result)) {
            LogMsg(kLogError, L"CreateSubAge: Failed to add info to link");
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
    void _CreateNodeCallback(ENetError result, void* state, void* param, RelVaultNode* node) {
        if (IS_NET_ERROR(result)) {
            LogMsg(kLogError, "CreateSubAge: Failed to create AgeLink (async)");
            return;
        }

        // Add the children to the right places
        VaultAddChildNode(node->GetNodeId(), (uint32_t)((uintptr_t)param), 0, nil, nil);
        if (hsRef<RelVaultNode> saFldr = VaultGetAgeSubAgesFolder())
            VaultAddChildNode(saFldr->GetNodeId(), node->GetNodeId(), 0, nil, nil);
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
                        nil,
                        param
        );
    }

    void _InitAgeCallback(ENetError result, void* state, void* param, uint32_t ageVaultId, uint32_t ageInfoId) {
        if (IS_NET_ERROR(result)) {
            LogMsg(kLogError, "CreateSubAge: Failed to init age (async)");
            return;
        }

        // Download age vault
        VaultDownload(L"CreateSubAge",
                      ageInfoId,
                      (FVaultDownloadCallback)_DownloadCallback,
                      (void*)ageInfoId,
                      nil,
                      nil
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
                 nil,
                 nil
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
    RelVaultNode *  node
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
    const wchar_t             parentAgeName[],
    const plAgeInfoStruct * info,
    plAgeLinkStruct *       link
) {
    using namespace _VaultCreateChildAgeAndWait;

    unsigned childAgesId;
    unsigned ageInfoId;
    unsigned ageLinkId;

    {   // Get id of child ages folder
        hsRef<RelVaultNode> rvnAgeInfo;
        if (parentAgeName) {
            char ansi[MAX_PATH];
            StrToAnsi(ansi, parentAgeName, arrsize(ansi));
            plAgeInfoStruct pinfo;
            pinfo.SetAgeFilename(ansi);
            if (hsRef<RelVaultNode> rvnAgeLink = VaultGetOwnedAgeLink(&pinfo))
                rvnAgeInfo = rvnAgeLink->GetChildNode(plVault::kNodeType_AgeInfo, 1);
        }
        else {
            rvnAgeInfo = VaultGetAgeInfoNode();
        }
        
        if (!rvnAgeInfo) {
            LogMsg(kLogError, L"CreateChildAge: Failed to get ages's AgeInfo node");
            return false;
        }

        hsRef<RelVaultNode> rvnChildAges;
        if (rvnChildAges = rvnAgeInfo->GetChildAgeInfoListNode(plVault::kChildAgesFolder, 1)) {
            childAgesId = rvnChildAges->GetNodeId();
        }       
        else {
            LogMsg(kLogError, L"CreateChildAge: Failed to get ages's ChildAges folder");
            return false;
        }
        
        // Check for existing child age in folder
        hsRef<RelVaultNode> rvnLink;
        hsRef<NetVaultNode> templateNode = new NetVaultNode;
        templateNode->SetNodeType(plVault::kNodeType_AgeInfo);

        VaultAgeInfoNode ageInfo(templateNode);
        wchar_t str[MAX_PATH];
        StrToUnicode(str, info->GetAgeFilename(), arrsize(str));
        ageInfo.SetAgeFilename(str);

        if (hsRef<RelVaultNode> rvnInfo = rvnChildAges->GetChildNode(templateNode, 2)) {
            templateNode->ClearFieldFlags();
            templateNode->SetNodeType(plVault::kNodeType_AgeLink);  
            rvnLink = rvnInfo->GetParentNode(templateNode, 1);
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
            nil,
            &param
        );

        while (!param.complete) {
            NetClientUpdate();
            plgDispatch::Dispatch()->MsgQueueProcess();
            AsyncSleep(10);
        }
        
        if (IS_NET_ERROR(param.result)) {
            LogMsg(kLogError, "CreateChildAge: Failed to init age %s", link->GetAgeInfo()->GetAgeFilename().c_str());
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
            nil,
            &param
        );

        while (!param.complete) {
            NetClientUpdate();
            plgDispatch::Dispatch()->MsgQueueProcess();
            AsyncSleep(10);
        }
        
        if (IS_NET_ERROR(param.result)) {
            LogMsg(kLogError, L"CreateChildAge: Failed create age link node");
            return false;
        }
            
        ageLinkId = param.nodeId;
    }       

    {   // Fetch age info node tree
        _FetchVaultParam    param;
        memset(&param, 0, sizeof(param));
        
        VaultDownload(
            L"CreateChildAge",
            ageInfoId,
            _FetchVaultCallback,
            &param,
            nil,
            nil
        );
        
        while (!param.complete) {
            NetClientUpdate();
            plgDispatch::Dispatch()->MsgQueueProcess();
            AsyncSleep(10);
        }
        
        if (IS_NET_ERROR(param.result)) {
            LogMsg(kLogError, L"CreateChildAge: Failed to download age info vault");
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
            AsyncSleep(10);
        }
        
        if (IS_NET_ERROR(param1.result)) {
            LogMsg(kLogError, L"CreateChildAge: Failed to add link to ages's subages");
            return false;
        }
        if (IS_NET_ERROR(param2.result)) {
            LogMsg(kLogError, L"CreateChildAge: Failed to add info to link");
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

    void _CreateNodeCallback(ENetError result, void* state, void* param, RelVaultNode* node) {
        if (IS_NET_ERROR(result)) {
            LogMsg(kLogError, "CreateChildAge: Failed to create AgeLink (async)");
            delete (_Params*)param;
            return;
        }

        _Params* p = (_Params*)param;

        // Add the children to the right places
        VaultAddChildNode(node->GetNodeId(), (uint32_t)((uintptr_t)p->fAgeInfoId), 0, nil, nil);
        VaultAddChildNode((uint32_t)((uintptr_t)p->fChildAgesFldr), node->GetNodeId(), 0, nil, nil);

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
                        (FVaultCreateNodeCallback)_CreateNodeCallback,
                        nil,
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
        p->fAgeInfoId = (void*)ageInfoId;

        // Download age vault
        VaultDownload(L"CreateChildAge",
                      ageInfoId,
                      (FVaultDownloadCallback)_DownloadCallback,
                      param,
                      nil,
                      nil
        );
    }
}; // namespace _VaultCreateAge

uint8_t VaultAgeFindOrCreateChildAgeLink(
    const wchar_t            parentAgeName[], 
    const plAgeInfoStruct* info,
    plAgeLinkStruct*       link) 
{
    using namespace _VaultCreateChildAge;

    // First, try to find an already existing ChildAge
    char name[MAX_PATH];
    StrToAnsi(name, parentAgeName, arrsize(name));
    plAgeInfoStruct search;
    search.SetAgeFilename(name);

    hsRef<RelVaultNode> rvnParentInfo;
    if (hsRef<RelVaultNode> rvnParentLink = VaultGetOwnedAgeLink(&search))
        rvnParentInfo = rvnParentLink->GetChildNode(plVault::kNodeType_AgeInfo, 1);
    else // Fallback to current age
        rvnParentInfo = VaultGetAgeInfoNode();

    // Test to make sure nothing went horribly wrong...
    if (rvnParentInfo == nil) {
        LogMsg(kLogError, "CreateChildAge: Couldn't find the parent ageinfo (async)");
        return hsFail;
    }

    // Still here? Try to find the Child Ages folder
    uint8_t retval = hsFail;
    if (hsRef<RelVaultNode> rvnChildAges = rvnParentInfo->GetChildAgeInfoListNode(plVault::kChildAgesFolder, 1)) {
        wchar_t hack[MAX_PATH];
        StrToUnicode(hack, info->GetAgeFilename(), arrsize(hack));

        // Search for our age
        hsRef<NetVaultNode> temp = new NetVaultNode;
        temp->SetNodeType(plVault::kNodeType_AgeInfo);
        VaultAgeInfoNode theAge(temp);
        theAge.SetAgeFilename(hack);

        if (hsRef<RelVaultNode> rvnAgeInfo = rvnChildAges->GetChildNode(temp, 2)) {
            hsRef<RelVaultNode> rvnAgeLink = rvnAgeInfo->GetParentAgeLink();

            VaultAgeLinkNode accAgeLink(rvnAgeLink);
            accAgeLink.CopyTo(link);
            VaultAgeInfoNode accAgeInfo(rvnAgeInfo);
            accAgeInfo.CopyTo(link->GetAgeInfo());

            retval = true;
        } else {
            _Params* p = new _Params;
            p->fChildAgesFldr = (void*)rvnChildAges->GetNodeId();

            VaultAgeInfoNode accParentInfo(rvnParentInfo);
            VaultInitAge(info,
                         accParentInfo.GetAgeInstanceGuid(),
                         (FVaultInitAgeCallback)_InitAgeCallback,
                         nil,
                         p
            );
            retval = false;
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
    const wchar_t                 tag[],
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
    const wchar_t                 tag[],
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
        AsyncSleep(10);
    }
}

//============================================================================
void VaultCull (unsigned vaultId) {
    // Remove the node from the global table
    if (RelVaultNodeLink * link = s_nodes.Find(vaultId)) {
        LogMsg(kLogDebug, L"Vault: Culling node %u", link->node->GetNodeId());
        link->node->state->UnlinkFromRelatives();
        delete link;
    }

    // Remove all orphaned nodes from the global table
    for (RelVaultNodeLink * next, * link = s_nodes.Head(); link; link = next) {
        next = s_nodes.Next(link);

        if (link->node->GetNodeType() > plVault::kNodeType_VNodeMgrLow && link->node->GetNodeType() < plVault::kNodeType_VNodeMgrHigh)
            continue;

        ARRAY(unsigned) nodeIds;
        link->node->GetRootIds(&nodeIds);
        bool foundRoot = false;
        for (unsigned i = 0; i < nodeIds.Count(); ++i) {
            RelVaultNodeLink * root = s_nodes.Find(nodeIds[i]);
            if (root && root->node->GetNodeType() > plVault::kNodeType_VNodeMgrLow && root->node->GetNodeType() < plVault::kNodeType_VNodeMgrHigh) {
                foundRoot = true;
                break;
            }
        }
        if (!foundRoot) {
            LogMsg(kLogDebug, L"Vault: Culling node %u", link->node->GetNodeId());
            link->node->state->UnlinkFromRelatives();
            delete link;
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
        hsRef<NetVaultNode> templateNode = new NetVaultNode;
        templateNode->SetNodeType(plVault::kNodeType_System);
        if (hsRef<RelVaultNode> systemNode = player->GetChildNode(templateNode, 1))
            result = systemNode;
    }
    return result;
}

//============================================================================
hsRef<RelVaultNode> VaultGetGlobalInbox () {
    hsRef<RelVaultNode> result;
    if (hsRef<RelVaultNode> system = VaultGetSystemNode()) {
        hsRef<NetVaultNode> templateNode = new NetVaultNode;
        templateNode->SetNodeType(plVault::kNodeType_Folder);
        VaultFolderNode folder(templateNode);
        folder.SetFolderType(plVault::kGlobalInboxFolder);
        if (hsRef<RelVaultNode> inbox = system->GetChildNode(templateNode, 1))
            result = inbox;
    }
    return result;
}

#endif // def CLIENT
