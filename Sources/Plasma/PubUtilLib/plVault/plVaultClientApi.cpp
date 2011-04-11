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
	LINK(IVaultCallback)	link;
	VaultCallback *			cb;
};

struct INotifyAfterDownload : THashKeyVal<unsigned> {
	HASHLINK(INotifyAfterDownload)	link;
	unsigned						parentId;
	unsigned						childId;

	INotifyAfterDownload (unsigned parentId, unsigned childId)
	:	THashKeyVal<unsigned>(childId)
	,	parentId(parentId)
	,	childId(childId)
	{}
};

struct DeviceInbox : CHashKeyStr {
	HASHLINK(DeviceInbox)	link;
	wchar					inboxName[kMaxVaultNodeStringLength];

	DeviceInbox (const wchar device[], const wchar inbox[])
	:	CHashKeyStr(device)
	{
		StrCopy(inboxName, inbox, arrsize(inboxName));
	}
};

// A RelVaultNodeLink may be either stored in the global table,
// or stored in an IRelVaultNode's parents or children table.
struct RelVaultNodeLink : THashKeyVal<unsigned> {
	HASHLINK(RelVaultNodeLink)	link;
	RelVaultNode *				node;
	unsigned					ownerId;
	bool						seen;
	
	RelVaultNodeLink (bool seen, unsigned ownerId, unsigned nodeId, RelVaultNode * node)
	:	THashKeyVal<unsigned>(nodeId)
	,	seen(seen)
	,	ownerId(ownerId)
	,	node(node)
	{
		node->IncRef();
	}
	~RelVaultNodeLink () {
		node->DecRef();
	}
};


struct IRelVaultNode {
	RelVaultNode *	node;
	
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
	FVaultCreateNodeCallback	callback;
	void *						state;
	void *						param;
	
	unsigned					nodeId;
	RelVaultNode *				node;
	
	static void VaultNodeCreated (
		ENetError			result,
		void *				param,
		unsigned			nodeId
	);
	static void VaultNodeFetched (
		ENetError			result,
		void *				param,
		NetVaultNode *		node
	);
	
	void Complete (ENetError result);
};


struct VaultFindNodeTrans {
	FVaultFindNodeCallback		callback;
	void *						param;
	
	static void VaultNodeFound (
		ENetError			result,
		void *				param,
		unsigned			nodeIdCount,
		const unsigned		nodeIds[]
	);
};


struct VaultDownloadTrans {
	FVaultDownloadCallback		callback;
	void *						cbParam;
	FVaultProgressCallback		progressCallback;
	void *						cbProgressParam;
	
	wchar		tag[MAX_PATH];
	unsigned	nodeCount;
	unsigned	nodesLeft;
	unsigned	vaultId;
	ENetError	result;
	
	VaultDownloadTrans ();
	
	static void VaultNodeFetched (
		ENetError			result,
		void *				param,
		NetVaultNode *		node
	);
	static void VaultNodeRefsFetched (
		ENetError			result,
		void *				param,
		NetVaultNodeRef *	refs,
		unsigned			refCount
	);
};

struct VaultAgeInitTrans {
	FVaultInitAgeCallback	callback;
	void *					cbState;
	void *					cbParam;

	static void AgeInitCallback (
		ENetError		result,
		void *			param,
		unsigned		ageVaultId,
		unsigned		ageInfoVaultId
	);
};

struct AddChildNodeFetchTrans {
	FVaultAddChildNodeCallback	callback;
	void *						cbParam;
	ENetError					result;
	long						opCount;
	
	static void VaultNodeFetched (
		ENetError			result,
		void *				param,
		NetVaultNode *		node
	);
	static void VaultNodeRefsFetched (
		ENetError			result,
		void *				param,
		NetVaultNodeRef *	refs,
		unsigned			refCount
	);
};


/*****************************************************************************
*
*   Private data
*
***/

static bool	s_running;

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
	ENetError			result,
	void *				param,
	NetVaultNode *		node
);
static void VaultNodeFound (
	ENetError			result,
	void *				param,
	unsigned			nodeIdCount,
	const unsigned		nodeIds[]
);

//============================================================================
static void VaultNodeAddedDownloadCallback(ENetError result, void * param) {
	unsigned childId = (unsigned)param;

	INotifyAfterDownload* notify = s_notifyAfterDownload.Find(childId);

	if (notify) {
		if (IS_NET_SUCCESS(result)) {
			RelVaultNodeLink* parentLink	= s_nodes.Find(notify->parentId);
			RelVaultNodeLink* childLink		= s_nodes.Find(notify->childId);

			if (parentLink && childLink) {
				if (childLink->node->nodeType == plVault::kNodeType_TextNote) {
					VaultTextNoteNode textNote(childLink->node);
					if (textNote.noteType == plVault::kNoteType_Visit)
						VaultProcessVisitNote(childLink->node);
					else if (textNote.noteType == plVault::kNoteType_UnVisit)
						VaultProcessUnvisitNote(childLink->node);
				}

				for (IVaultCallback * cb = s_callbacks.Head(); cb; cb = s_callbacks.Next(cb))
					cb->cb->AddedChildNode(parentLink->node, childLink->node);
			}
		}

		DEL(notify);
	}
}

//============================================================================
static void __cdecl LogDumpProc (
	void *				,
	const wchar			fmt[],
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
	const NetVaultNodeRef	refs[],
	unsigned				refCount,
	ARRAY(unsigned) *		newNodeIds,
	ARRAY(unsigned) *		existingNodeIds,
	bool					notifyNow = true
) {
	for (unsigned i = 0; i < refCount; ++i) {
		// Find/Create global links
		RelVaultNodeLink * parentLink = s_nodes.Find(refs[i].parentId);
		if (!parentLink) {
			newNodeIds->Add(refs[i].parentId);
			parentLink = NEWZERO(RelVaultNodeLink)(false, 0, refs[i].parentId, NEWZERO(RelVaultNode));
			parentLink->node->nodeId = refs[i].parentId; // set directly so that the field's dirty flag isn't set
			s_nodes.Add(parentLink);
		}
		else {
			existingNodeIds->Add(refs[i].parentId);
		}
		RelVaultNodeLink * childLink = s_nodes.Find(refs[i].childId);
		if (!childLink) {
			newNodeIds->Add(refs[i].childId);
			childLink = NEWZERO(RelVaultNodeLink)(refs[i].seen, refs[i].ownerId, refs[i].childId, NEWZERO(RelVaultNode));
			childLink->node->nodeId = refs[i].childId; // set directly so that the field's dirty flag isn't set
			s_nodes.Add(childLink);
		}
		else {
			existingNodeIds->Add(refs[i].childId);
			if (unsigned ownerId = refs[i].ownerId)
				childLink->ownerId = ownerId;
		}

		RelVaultNode * parentNode = parentLink->node;
		RelVaultNode * childNode = childLink->node;
		
		bool isImmediateParent = parentNode->IsParentOf(refs[i].childId, 1);
		bool isImmediateChild = childNode->IsChildOf(refs[i].parentId, 1);
			
		if (!isImmediateParent) {
			// Add parent to child's parents table
			parentLink = NEWZERO(RelVaultNodeLink)(false, 0, parentNode->nodeId, parentNode);
			childNode->state->parents.Add(parentLink);
			LogMsg(kLogDebug, L"Added relationship: p:%u,c:%u", refs[i].parentId, refs[i].childId);
		}
		
		if (!isImmediateChild) {
			// Add child to parent's children table
			childLink = NEWZERO(RelVaultNodeLink)(refs[i].seen, refs[i].ownerId, childNode->nodeId, childNode);
			parentNode->state->children.Add(childLink);

			if (notifyNow || childNode->nodeType != 0) {
				// We made a new link, so make the callbacks
				for (IVaultCallback * cb = s_callbacks.Head(); cb; cb = s_callbacks.Next(cb))
					cb->cb->AddedChildNode(parentNode, childNode);
			}
			else {
				INotifyAfterDownload* notify = NEWZERO(INotifyAfterDownload)(parentNode->nodeId, childNode->nodeId);
				s_notifyAfterDownload.Add(notify);
			}
		}
	}
}

//============================================================================
static void InitFetchedNode (RelVaultNode * rvn) {

	switch (rvn->nodeType) {
		case plVault::kNodeType_SDL: {
			VaultSDLNode access(rvn);
			if (!access.sdlData || !access.sdlDataLen)
				access.InitStateDataRecord(access.sdlName);
		}
		break;
	}
}

//============================================================================
static void FetchRefOwners (
	NetVaultNodeRef *			refs,
	unsigned					refCount
) {
	ARRAY(unsigned)	ownerIds;
	{	for (unsigned i = 0; i < refCount; ++i)
			if (unsigned ownerId = refs[i].ownerId)
				ownerIds.Add(ownerId);
	}
	QSORT(unsigned, ownerIds.Ptr(), ownerIds.Count(), elem1 < elem2);
	RelVaultNode * templateNode = NEWZERO(RelVaultNode);
	templateNode->IncRef();
	templateNode->SetNodeType(plVault::kNodeType_PlayerInfo);
	{	unsigned prevId = 0;
		for (unsigned i = 0; i < ownerIds.Count(); ++i) {
			if (ownerIds[i] != prevId) {
				prevId = ownerIds[i];
				VaultPlayerInfoNode access(templateNode);
				access.SetPlayerId(refs[i].ownerId);
				if (RelVaultNode * rvn = VaultGetNodeIncRef(templateNode)) {
					rvn->DecRef();
					continue;
				}
				NetCliAuthVaultNodeFind(
					templateNode,
					VaultNodeFound,
					nil
				);
			}
		}
	}
	templateNode->DecRef();
}

//============================================================================
static void FetchNodesFromRefs (
	NetVaultNodeRef *			refs,
	unsigned					refCount,
	FNetCliAuthVaultNodeFetched	fetchCallback,
	void *						fetchParam,
	unsigned *					fetchCount
	
) {
	// On the side, start downloading PlayerInfo nodes of ref owners we don't already have locally
	FetchRefOwners(refs, refCount);

	*fetchCount = 0;
	
	ARRAY(unsigned) newNodeIds;
	ARRAY(unsigned) existingNodeIds;
	
	BuildNodeTree(refs, refCount, &newNodeIds, &existingNodeIds);

	ARRAY(unsigned)	nodeIds;
	nodeIds.Add(newNodeIds.Ptr(), newNodeIds.Count());
	nodeIds.Add(existingNodeIds.Ptr(), existingNodeIds.Count());
	QSORT(unsigned, nodeIds.Ptr(), nodeIds.Count(), elem1 < elem2);

	// Fetch the nodes that do not yet have a nodetype
	unsigned prevId = 0;
	{for (unsigned i = 0; i < nodeIds.Count(); ++i) {
		RelVaultNodeLink * link = s_nodes.Find(nodeIds[i]);
		if (link->node->nodeType != 0)
			continue;
		// filter duplicates
		if (link->node->nodeId == prevId)
			continue;
		prevId = link->node->nodeId;
		NetCliAuthVaultNodeFetch(
			nodeIds[i],
			fetchCallback,
			fetchParam
		);
		++(*fetchCount);
	}}
}

//============================================================================
static void VaultNodeFound (
	ENetError			result,
	void *				,
	unsigned			nodeIdCount,
	const unsigned		nodeIds[]
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
	ENetError			result,
	void *				,
	NetVaultNode *		node
) {
	if (IS_NET_ERROR(result)) {
		LogMsg(kLogDebug, L"VaultNodeFetched failed: %u (%s)", result, NetErrorToString(result));
		return;
	}

	// Add to global node table
	RelVaultNodeLink * link = s_nodes.Find(node->nodeId);
	if (!link) {
		link = NEWZERO(RelVaultNodeLink)(false, 0, node->nodeId, NEWZERO(RelVaultNode));
		link->node->nodeId = node->nodeId; // set directly so that the field's dirty flag isn't set
		s_nodes.Add(link);
	}
	link->node->CopyFrom(node, NetVaultNode::kCopyOverwrite);
	InitFetchedNode(link->node);
	
	link->node->Print(L"Fetched", LogDumpProc, 0);
}

//============================================================================
static void ChangedVaultNodeFetched (
	ENetError			result,
	void *				param,
	NetVaultNode *		node
) {
	if (IS_NET_ERROR(result)) {
		LogMsg(kLogDebug, L"ChangedVaultNodeFetched failed: %u (%s)", result, NetErrorToString(result));
		return;
	}

	VaultNodeFetched(result, param, node);

	RelVaultNodeLink* savedLink = s_nodes.Find(node->nodeId);

	if (savedLink) {
		for (IVaultCallback * cb = s_callbacks.Head(); cb; cb = s_callbacks.Next(cb))
			cb->cb->ChangedNode(savedLink->node);
	}
}

//============================================================================
static void VaultNodeChanged (
	unsigned		nodeId,
	const Uuid &	revisionId
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
	unsigned		parentId,
	unsigned		childId,
	unsigned		ownerId
) {
	LogMsg(kLogDebug, L"Notify: Node added: p:%u,c:%u", parentId, childId);

	unsigned inboxId = 0;	
	if (RelVaultNode * rvnInbox = VaultGetPlayerInboxFolderIncRef()) {
		inboxId = rvnInbox->nodeId;
		rvnInbox->DecRef();
	}

	// Build the relationship locally
	NetVaultNodeRef refs[] = {
		{ parentId, childId, ownerId }
	};
	ARRAY(unsigned) newNodeIds;
	ARRAY(unsigned) existingNodeIds;
	
	BuildNodeTree(refs, arrsize(refs), &newNodeIds, &existingNodeIds, false);

	ARRAY(unsigned)	nodeIds;
	nodeIds.Add(newNodeIds.Ptr(), newNodeIds.Count());
	nodeIds.Add(existingNodeIds.Ptr(), existingNodeIds.Count());
	QSORT(unsigned, nodeIds.Ptr(), nodeIds.Count(), elem1 < elem2);

	// Fetch the nodes that do not yet have a nodetype
	unsigned prevId = 0;
	unsigned i = 0;
	{for (; i < nodeIds.Count(); ++i) {
		RelVaultNodeLink * link = s_nodes.Find(nodeIds[i]);
		if (link->node->nodeType != 0)
			continue;
		// filter duplicates
		if (link->node->nodeId == prevId)
			continue;
		prevId = link->node->nodeId;
		VaultDownload(
			L"NodeAdded",
			nodeIds[i],
			VaultNodeAddedDownloadCallback,
			(void*)nodeIds[i],
			nil,
			nil
		);
	}}
	
	if (parentId == inboxId) {
		if (i > 0)
			s_processPlayerInbox = true;
		else
			VaultProcessPlayerInbox();
	}

	// if the added element is already downloaded then send the callbacks now
	RelVaultNodeLink* parentLink	= s_nodes.Find(parentId);
	RelVaultNodeLink* childLink		= s_nodes.Find(childId);

	if (childLink->node->nodeType != 0) {
		for (IVaultCallback * cb = s_callbacks.Head(); cb; cb = s_callbacks.Next(cb))
			cb->cb->AddedChildNode(parentLink->node, childLink->node);
	}
}

//============================================================================
static void VaultNodeRemoved (
	unsigned		parentId,
	unsigned		childId
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
	unsigned		nodeId
) {
	LogMsg(kLogDebug, L"Notify: Node deleted: %u", nodeId);
	VaultCull(nodeId);
}

//============================================================================
static void SaveDirtyNodes () {
	// Save a max of 5Kb every quarter second
	static const unsigned kSaveUpdateIntervalMs		= 250;
	static const unsigned kMaxBytesPerSaveUpdate	= 5 * 1024;
	static unsigned s_nextSaveMs;
	unsigned currTimeMs = TimeGetMs() | 1;
	if (!s_nextSaveMs || signed(s_nextSaveMs - currTimeMs) <= 0) {
		s_nextSaveMs = (currTimeMs + kSaveUpdateIntervalMs) | 1;
		unsigned bytesWritten = 0;
		for (RelVaultNodeLink * link = s_nodes.Head(); link; link = s_nodes.Next(link)) {
			if (bytesWritten >= kMaxBytesPerSaveUpdate)
				break;
			if (link->node->dirtyFlags) {

				// Auth server needs the name of the sdl record
				if (link->node->nodeType == plVault::kNodeType_SDL)
					link->node->dirtyFlags |= VaultSDLNode::kSDLName;

				if (unsigned bytes = NetCliAuthVaultNodeSave(link->node, nil, nil)) {
					bytesWritten += bytes;
					link->node->Print(L"Saving", LogDumpProc, 0);
				}
			}
		}
	}
}

//============================================================================
static RelVaultNode * GetChildFolderNode (
	RelVaultNode *	parent,
	unsigned		folderType,
	unsigned		maxDepth
) {
	if (!parent)
		return nil;

	RelVaultNode * rvn = parent->GetChildFolderNodeIncRef(folderType, maxDepth);
	if (rvn)
		rvn->DecRef();

	return rvn;
}

//============================================================================
static RelVaultNode * GetChildPlayerInfoListNode (
	RelVaultNode *	parent,
	unsigned		folderType,
	unsigned		maxDepth
) {
	if (!parent)
		return nil;

	RelVaultNode * rvn = parent->GetChildPlayerInfoListNodeIncRef(folderType, maxDepth);
	if (rvn)
		rvn->DecRef();

	return rvn;
}


/*****************************************************************************
*
*   VaultCreateNodeTrans
*
***/

//============================================================================
void VaultCreateNodeTrans::VaultNodeCreated (
	ENetError			result,
	void *				param,
	unsigned			nodeId
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
	ENetError			result,
	void *				param,
	NetVaultNode *		node
) {
	::VaultNodeFetched(result, param, node);

	VaultCreateNodeTrans * trans = (VaultCreateNodeTrans *)param;
	
	if (IS_NET_SUCCESS(result))
		trans->node = s_nodes.Find(node->nodeId)->node;
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

	DEL(this);
}


/*****************************************************************************
*
*   VaultFindNodeTrans
*
***/

//============================================================================
void VaultFindNodeTrans::VaultNodeFound (
	ENetError			result,
	void *				param,
	unsigned			nodeIdCount,
	const unsigned		nodeIds[]
) {
	VaultFindNodeTrans * trans = (VaultFindNodeTrans*)param;
	if (trans->callback)
		trans->callback(
			result,
			trans->param,
			nodeIdCount,
			nodeIds
		);
	DEL(trans);
}


/*****************************************************************************
*
*   VaultDownloadTrans
*
***/

//============================================================================
VaultDownloadTrans::VaultDownloadTrans () {
	ASSERT(!nodeCount);	// must be alloced with
}

//============================================================================
void VaultDownloadTrans::VaultNodeFetched (
	ENetError			result,
	void *				param,
	NetVaultNode *		node
) {
	::VaultNodeFetched(result, param, node);

	VaultDownloadTrans * trans = (VaultDownloadTrans *)param;
	if (IS_NET_ERROR(result)) {
		trans->result = result;
		//LogMsg(kLogError, L"Error fetching node...most likely trying to fetch a nodeid of 0");
	}
	
	--trans->nodesLeft;
//	LogMsg(kLogDebug, L"(Download) %u of %u nodes fetched", trans->nodeCount - trans->nodesLeft, trans->nodeCount);
	
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

		DEL(trans);
	}
}

//============================================================================
void VaultDownloadTrans::VaultNodeRefsFetched (
	ENetError			result,
	void *				param,
	NetVaultNodeRef *	refs,
	unsigned			refCount
) {
	VaultDownloadTrans * trans = (VaultDownloadTrans *)param;
	
	if (IS_NET_ERROR(result)) {
		LogMsg(kLogDebug, L"VaultNodeRefsFetched failed: %u (%s)", result, NetErrorToString(result));
		trans->result		= result;
		trans->nodesLeft	= 0;
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
			if (!rootNodeLink || rootNodeLink->node->nodeType == 0) {
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

		DEL(trans);
	}
}


/*****************************************************************************
*
*   VaultAgeInitTrans
*
***/

//============================================================================
void VaultAgeInitTrans::AgeInitCallback (
	ENetError		result,
	void *			param,
	unsigned		ageVaultId,
	unsigned		ageInfoVaultId
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
	
	DEL(trans);
}


/*****************************************************************************
*
*   AddChildNodeFetchTrans
*
***/

//============================================================================
void AddChildNodeFetchTrans::VaultNodeRefsFetched (
	ENetError			result,
	void *				param,
	NetVaultNodeRef *	refs,
	unsigned			refCount
) {
	AddChildNodeFetchTrans * trans = (AddChildNodeFetchTrans *)param;

	if (IS_NET_ERROR(result)) {
		trans->result		= result;
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
		AtomicAdd(&trans->opCount, incFetchCount);
	}

	// Make the callback now if there are no nodes to fetch, or if error
	AtomicAdd(&trans->opCount, -1);	
	if (!trans->opCount) {
		if (trans->callback)
			trans->callback(
				trans->result,
				trans->cbParam
			);
		DEL(trans);
	}
}

//============================================================================
void AddChildNodeFetchTrans::VaultNodeFetched (
	ENetError			result,
	void *				param,
	NetVaultNode *		node
) {
	::VaultNodeFetched(result, param, node);
	
	AddChildNodeFetchTrans * trans = (AddChildNodeFetchTrans *)param;
	
	if (IS_NET_ERROR(result))
		trans->result = result;
		
	AtomicAdd(&trans->opCount, -1);	
	if (!trans->opCount) {
		if (trans->callback)
			trans->callback(
				trans->result,
				trans->cbParam
			);
		DEL(trans);
	}
}


/*****************************************************************************
*
*   IRelVaultNode
*
***/

//============================================================================
IRelVaultNode::IRelVaultNode (RelVaultNode * node)
:	node(node)
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
	if (nil != (link = parents.Find(other->nodeId))) {
		// make them non-findable in our parents table
		link->link.Unlink();
		// remove us from other's tables.
		link->node->state->Unlink(node);
		DEL(link);
	}
	if (nil != (link = children.Find(other->nodeId))) {
		// make them non-findable in our children table
		link->link.Unlink();
		// remove us from other's tables.
		link->node->state->Unlink(node);
		DEL(link);
	}
}

/*****************************************************************************
*
*   RelVaultNode
*
***/

//============================================================================
RelVaultNode::RelVaultNode () {
	state = NEWZERO(IRelVaultNode)(this);
}

//============================================================================
RelVaultNode::~RelVaultNode () {
	DEL(state);
}

//============================================================================
bool RelVaultNode::IsParentOf (unsigned childId, unsigned maxDepth) {
	if (nodeId == childId)
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
	if (nodeId == parentId)
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
		nodeIds->Add(nodeId);
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
	ARRAY(unsigned) *	nodeIds,
	unsigned			maxDepth
) {
	if (!maxDepth)
		return;
	RelVaultNodeLink * link = state->children.Head();
	for (; link; link = state->children.Next(link)) {
		nodeIds->Add(link->node->nodeId);
		link->node->GetChildNodeIds(nodeIds, maxDepth-1);
	}
}

//============================================================================
void RelVaultNode::GetParentNodeIds (
	ARRAY(unsigned) *	nodeIds,
	unsigned			maxDepth
) {
	if (!maxDepth)
		return;
	RelVaultNodeLink * link = state->parents.Head();
	for (; link; link = state->parents.Next(link)) {
		nodeIds->Add(link->node->nodeId);
		link->node->GetParentNodeIds(nodeIds, maxDepth-1);
	}
}


//============================================================================
RelVaultNode * RelVaultNode::GetParentNodeIncRef (
	NetVaultNode *		templateNode,
	unsigned			maxDepth
) {
	if (maxDepth == 0)
		return false;

	RelVaultNodeLink * link;
	link = state->parents.Head();
	for (; link; link = state->parents.Next(link)) {
		if (link->node->Matches(templateNode)) {
			link->node->IncRef("Found");
			return link->node;
		}
	}
	
	link = state->parents.Head();
	for (; link; link = state->parents.Next(link)) {
		if (RelVaultNode * node = link->node->GetParentNodeIncRef(templateNode, maxDepth-1))
			return node;
	}

	return nil;	
}

//============================================================================
RelVaultNode * RelVaultNode::GetChildNodeIncRef (
	NetVaultNode *		templateNode,
	unsigned			maxDepth
) {
	if (maxDepth == 0)
		return false;

	RelVaultNodeLink * link;
	link = state->children.Head();
	for (; link; link = state->children.Next(link)) {
		if (link->node->Matches(templateNode)) {
			link->node->IncRef("Found");
			return link->node;
		}
	}
	
	link = state->children.Head();
	for (; link; link = state->children.Next(link)) {
		if (RelVaultNode * node = link->node->GetChildNodeIncRef(templateNode, maxDepth-1))
			return node;
	}

	return nil;	
}

//============================================================================
RelVaultNode * RelVaultNode::GetChildNodeIncRef (
	unsigned			nodeType,
	unsigned			maxDepth
) {
	NetVaultNode * templateNode = NEWZERO(NetVaultNode);
	templateNode->IncRef();
	templateNode->SetNodeType(nodeType);
	RelVaultNode * result = GetChildNodeIncRef(templateNode, maxDepth);
	templateNode->DecRef();
	return result;
}

//============================================================================
RelVaultNode * RelVaultNode::GetChildFolderNodeIncRef (
	unsigned			folderType,
	unsigned			maxDepth
) {
	NetVaultNode * templateNode = NEWZERO(NetVaultNode);
	templateNode->IncRef();
	templateNode->SetNodeType(plVault::kNodeType_Folder);
	VaultFolderNode folder(templateNode);
	folder.SetFolderType(folderType);
	RelVaultNode * result = GetChildNodeIncRef(templateNode, maxDepth);
	templateNode->DecRef();
	return result;
}

//============================================================================
RelVaultNode * RelVaultNode::GetChildPlayerInfoListNodeIncRef (
	unsigned			folderType,
	unsigned			maxDepth
) {
	NetVaultNode * templateNode = NEWZERO(NetVaultNode);
	templateNode->IncRef();
	templateNode->SetNodeType(plVault::kNodeType_PlayerInfoList);
	VaultPlayerInfoListNode access(templateNode);
	access.SetFolderType(folderType);
	RelVaultNode * result = GetChildNodeIncRef(templateNode, maxDepth);
	templateNode->DecRef();
	return result;
}

//============================================================================
RelVaultNode * RelVaultNode::GetChildAgeInfoListNodeIncRef (
	unsigned			folderType,
	unsigned			maxDepth
) {
	NetVaultNode * templateNode = NEWZERO(NetVaultNode);
	templateNode->IncRef();
	templateNode->SetNodeType(plVault::kNodeType_AgeInfoList);
	VaultAgeInfoListNode access(templateNode);
	access.SetFolderType(folderType);
	RelVaultNode * result = GetChildNodeIncRef(templateNode, maxDepth);
	templateNode->DecRef();
	return result;
}

//============================================================================
void RelVaultNode::GetChildNodesIncRef (
	unsigned				maxDepth,
	ARRAY(RelVaultNode*) *	nodes
) {
	if (maxDepth == 0)
		return;

	RelVaultNodeLink * link;
	link = state->children.Head();
	for (; link; link = state->children.Next(link)) {
		nodes->Add(link->node);
		link->node->IncRef();
		link->node->GetChildNodesIncRef(
			maxDepth - 1,
			nodes
		);
	}
}

//============================================================================
void RelVaultNode::GetChildNodesIncRef (
	NetVaultNode *			templateNode,
	unsigned				maxDepth,
	ARRAY(RelVaultNode*) *	nodes
) {
	RelVaultNodeLink * link;
	link = state->children.Head();
	for (; link; link = state->children.Next(link)) {
		if (link->node->Matches(templateNode)) {
			nodes->Add(link->node);
			link->node->IncRef();
		}
		link->node->GetChildNodesIncRef(
			templateNode,
			maxDepth - 1,
			nodes
		);
	}
}

//============================================================================
void RelVaultNode::GetChildNodesIncRef (
	unsigned				nodeType,
	unsigned				maxDepth,
	ARRAY(RelVaultNode*) *	nodes
) {
	NetVaultNode * templateNode = NEWZERO(NetVaultNode);
	templateNode->IncRef();
	templateNode->SetNodeType(nodeType);
	GetChildNodesIncRef(
		templateNode,
		maxDepth,
		nodes
	);
	templateNode->DecRef();
}

//============================================================================
void RelVaultNode::GetChildFolderNodesIncRef (
	unsigned				folderType,
	unsigned				maxDepth,
	ARRAY(RelVaultNode*) *	nodes
) {
	NetVaultNode * templateNode = NEWZERO(NetVaultNode);
	templateNode->IncRef();
	templateNode->SetNodeType(plVault::kNodeType_Folder);
	VaultFolderNode fldr(templateNode);
	fldr.SetFolderType(folderType);
	GetChildNodesIncRef(
		templateNode,
		maxDepth,
		nodes
	);
	templateNode->DecRef();
}

//============================================================================
unsigned RelVaultNode::GetRefOwnerId (unsigned parentId) {
	// find our parents' link to us and return its ownerId
	if (RelVaultNodeLink * parentLink = state->parents.Find(parentId))
		if (RelVaultNodeLink * childLink = parentLink->node->state->children.Find(nodeId))
			return childLink->ownerId;
	return 0;
}

//============================================================================
bool RelVaultNode::BeenSeen (unsigned parentId) const {
	// find our parents' link to us and return its seen flag
	if (RelVaultNodeLink * parentLink = state->parents.Find(parentId))
		if (RelVaultNodeLink * childLink = parentLink->node->state->children.Find(nodeId))
			return childLink->seen;
	return true;
}

//============================================================================
void RelVaultNode::SetSeen (unsigned parentId, bool seen) {
	// find our parents' link to us and set its seen flag
	if (RelVaultNodeLink * parentLink = state->parents.Find(parentId))
		if (RelVaultNodeLink * childLink = parentLink->node->state->children.Find(nodeId))
			if (childLink->seen != seen) {
				childLink->seen = seen;
				NetCliAuthVaultSetSeen(parentId, nodeId, seen);
			}
}

//============================================================================
void RelVaultNode::Print (const wchar tag[], FStateDump dumpProc, unsigned level) {
	wchar str[1024];
	StrPrintf(
		str,
		arrsize(str),
		L"%s%*s%*s%u, %S",
		tag ? tag : L"",
		tag ? 1 : 0,
		" ",
		level * 2,
		" ",
		nodeId,
		plVault::NodeTypeStr(nodeType, false)
	);

	NetVaultNodeFieldArray fields(this);
	for (qword bit = 1; bit; bit <<= 1) {
		if (!(fieldFlags & bit))
			continue;
		if (bit > fieldFlags)
			break;
		
		StrPack(str, L", ", arrsize(str));
		StrPack(str, fields.GetFieldName(bit), arrsize(str));
		if (fields.GetFieldAddress(bit)) {
			StrPack(str, L"=", arrsize(str));
			const unsigned chars = StrLen(str);
			fields.GetFieldValueString_LCS(bit, str + chars, arrsize(str) - chars * sizeof(str[0]));
		}
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
RelVaultNode * RelVaultNode::GetParentAgeLinkIncRef () {

	// this function only makes sense when called on age info nodes
	ASSERT(nodeType == plVault::kNodeType_AgeInfo);
	
	RelVaultNode * result = nil;
	
	NetVaultNode * templateNode = NEWZERO(NetVaultNode);
	templateNode->IncRef();
	templateNode->SetNodeType(plVault::kNodeType_AgeLink);

	// Get our parent AgeLink node	
	if (RelVaultNode * rvnLink = GetParentNodeIncRef(templateNode, 1)) {
		// Get the next AgeLink node in our parent tree
		result = rvnLink->GetParentNodeIncRef(templateNode, 3);
	}
	
	templateNode->DecRef();
	return result;		
}


/*****************************************************************************
*
*   Exports - Callbacks
*
***/

//============================================================================
void VaultRegisterCallback (VaultCallback * cb) {
	IVaultCallback * internal = NEW(IVaultCallback);
	internal->cb = cb;
	cb->internal = internal;
	s_callbacks.Link(internal);
}

//============================================================================
void VaultUnregisterCallback (VaultCallback * cb) {
	ASSERT(cb->internal);
	DEL(cb->internal);
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
		DEL(link);
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
static RelVaultNode * GetNode (
	unsigned id
) {
	RelVaultNodeLink * link = s_nodes.Find(id);
	if (link)
		return link->node;
	return nil;
}

//============================================================================
static RelVaultNode * GetNode (
	NetVaultNode *	templateNode
) {
	ASSERT(templateNode);
	RelVaultNodeLink * link = s_nodes.Head();
	while (link) {
		if (link->node->Matches(templateNode))
			return link->node;
		link = s_nodes.Next(link);
	}
	return nil;
}

//============================================================================
RelVaultNode * VaultGetNodeIncRef (
	NetVaultNode *	templateNode
) {
	if (RelVaultNode * node = GetNode(templateNode)) {
		node->IncRef();
		return node;
	}
	return nil;
}

//============================================================================
RelVaultNode * VaultGetNodeIncRef (
	unsigned		nodeId
) {
	if (RelVaultNode * node = GetNode(nodeId)) {
		node->IncRef();
		return node;
	}
	return nil;
}

//============================================================================
RelVaultNode * VaultGetNodeIncRef (
	unsigned	nodeId,
	const char	reftag[]
) {
	if (RelVaultNodeLink * link = s_nodes.Find(nodeId)) {
		link->node->IncRef(reftag);
		return link->node;
	}
	return nil;
}

//============================================================================
void VaultAddChildNode (
	unsigned					parentId,
	unsigned					childId,
	unsigned					ownerId,
	FVaultAddChildNodeCallback	callback,
	void *						param
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
			childLink = NEWZERO(RelVaultNodeLink)(false, ownerId, childId, NEWZERO(RelVaultNode));
			childLink->node->nodeId = childId;	// set directly so that the field's dirty flag isn't set
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

			ARRAY(unsigned)	newNodeIds;
			ARRAY(unsigned)	existingNodeIds;

			BuildNodeTree(refs, arrsize(refs), &newNodeIds, &existingNodeIds);
		
			if (!childLink->node->nodeType || !parentLink->node->nodeType) {
				// One or more nodes need to be fetched before the callback is made
				AddChildNodeFetchTrans * trans = NEWZERO(AddChildNodeFetchTrans);
				trans->callback = callback;
				trans->cbParam = param;
				if (!childLink->node->nodeType) {
					AtomicAdd(&trans->opCount, 1);
					NetCliAuthVaultNodeFetch(
						childId,
						AddChildNodeFetchTrans::VaultNodeFetched,
						trans
					);
					AtomicAdd(&trans->opCount, 1);
					NetCliAuthVaultFetchNodeRefs(
						childId,
						AddChildNodeFetchTrans::VaultNodeRefsFetched,
						trans
					);
				}
				if (!parentLink->node->nodeType) {
					AtomicAdd(&trans->opCount, 1);
					NetCliAuthVaultNodeFetch(
						parentId,
						AddChildNodeFetchTrans::VaultNodeFetched,
						trans
					);
					AtomicAdd(&trans->opCount, 1);
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
	ENetError		result;
	bool			complete;
};
static void _AddChildNodeCallback (
	ENetError		result,
	void *			vparam
) {
	_AddChildNodeParam * param = (_AddChildNodeParam *)vparam;
	param->result		= result;
	param->complete		= true;
}

} // namespace _VaultAddChildNodeAndWait

//============================================================================
void VaultAddChildNodeAndWait (
	unsigned					parentId,
	unsigned					childId,
	unsigned					ownerId
) {
	using namespace _VaultAddChildNodeAndWait;
	
	_AddChildNodeParam param;
	ZERO(param);
	
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
	unsigned						parentId,
	unsigned						childId,
	FVaultRemoveChildNodeCallback	callback,
	void *							param
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
	unsigned	nodeId,
	bool		seen
) {
	hsAssert(false, "eric, implement me");
}

//============================================================================
void VaultDeleteNode (
	unsigned	nodeId
) {
	// Send request up to vault.  We will remove it locally upon notification of deletion.
	NetCliAuthVaultNodeDelete(nodeId);	
}

//============================================================================
void VaultPublishNode (
	unsigned		nodeId,
	const wchar		deviceName[]
) {
	RelVaultNode * rvn;
	
	rvn = VaultAgeGetDeviceInboxIncRef(deviceName);
	if (!rvn) {
		LogMsg(kLogDebug, L"Failed to find inbox for device %s, adding it on-the-fly", deviceName);
		VaultAgeSetDeviceInboxAndWaitIncRef(deviceName, DEFAULT_DEVICE_INBOX);

		rvn = VaultAgeGetDeviceInboxIncRef(deviceName);
		if (!rvn) {
			LogMsg(kLogDebug, L"Failed to add inbox to device %s on-the-fly", deviceName);
			return;
		}
	}
	
	VaultAddChildNode(rvn->nodeId, nodeId, VaultGetPlayerId(), nil, nil);
	rvn->DecRef();
}

//============================================================================
void VaultSendNode (
	RelVaultNode*	srcNode,
	unsigned		dstPlayerId
) {
	NetCliAuthVaultNodeSave(srcNode, nil, nil);
	NetCliAuthVaultSendNode(srcNode->nodeId, dstPlayerId);
}

//============================================================================
void VaultCreateNode (
	NetVaultNode *				templateNode,
	FVaultCreateNodeCallback	callback,
	void *						state,
	void *						param
) {
	VaultCreateNodeTrans * trans = NEWZERO(VaultCreateNodeTrans);
	trans->callback	= callback;
	trans->state	= state;
	trans->param	= param;

	if (RelVaultNode * age = VaultGetAgeNodeIncRef()) {
		VaultAgeNode access(age);
		if (!(templateNode->fieldFlags & NetVaultNode::kCreateAgeName))
			templateNode->SetCreateAgeName(access.ageName);
		if (!(templateNode->fieldFlags & NetVaultNode::kCreateAgeUuid))
			templateNode->SetCreateAgeUuid(access.ageInstUuid);
		age->DecRef();
	}
	
	NetCliAuthVaultNodeCreate(
		templateNode,
		VaultCreateNodeTrans::VaultNodeCreated,
		trans
	);
}

//============================================================================
void VaultCreateNode (
	plVault::NodeTypes			nodeType,
	FVaultCreateNodeCallback	callback,
	void *						state,
	void *						param
) {
	RelVaultNode * templateNode = NEWZERO(RelVaultNode);
	templateNode->IncRef();
	templateNode->SetNodeType(nodeType);
	
	VaultCreateNode(
		templateNode,
		callback,
		state,
		param
	);

	templateNode->DecRef();
}

//============================================================================
namespace _VaultCreateNodeAndWaitIncRef {

struct _CreateNodeParam {
	RelVaultNode *	node;
	ENetError		result;
	bool			complete;
};
static void _CreateNodeCallback (
	ENetError		result,
	void *			,
	void *			vparam,
	RelVaultNode *	node
) {
	_CreateNodeParam * param = (_CreateNodeParam *)vparam;
	param->node		= node;
	param->result	= result;
	param->complete	= true;
}

} // namespace _VaultCreateNodeAndWaitIncRef

RelVaultNode * VaultCreateNodeAndWaitIncRef (
	NetVaultNode *				templateNode,
	ENetError *					result
) {
	using namespace _VaultCreateNodeAndWaitIncRef;
	
	_CreateNodeParam param;
	ZERO(param);
	
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
	if (IS_NET_SUCCESS(param.result))
		param.node->IncRef();
	return param.node;
}

//============================================================================
RelVaultNode * VaultCreateNodeAndWaitIncRef (
	plVault::NodeTypes			nodeType,
	ENetError *					result
) {
	RelVaultNode * node;
	RelVaultNode * templateNode = NEWZERO(RelVaultNode);
	templateNode->IncRef();
	templateNode->SetNodeType(nodeType);
	
	node = VaultCreateNodeAndWaitIncRef(templateNode, result);

	templateNode->DecRef();
	return node;
}

//============================================================================
namespace _VaultForceSaveNodeAndWait {

struct _SaveNodeParam {
	ENetError		result;
	bool			complete;
};
static void _SaveNodeCallback (
	ENetError		result,
	void *			vparam
) {
	_SaveNodeParam * param = (_SaveNodeParam *)vparam;
	param->result	= result;
	param->complete	= true;
}

} // namespace _VaultForceSaveNodeAndWait

void VaultForceSaveNodeAndWait (
	NetVaultNode *		node
) {
	using namespace _VaultForceSaveNodeAndWait;
	
	_SaveNodeParam param;
	ZERO(param);
	
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
	NetVaultNode *			templateNode,
	FVaultFindNodeCallback	callback,
	void *					param
) {
	VaultFindNodeTrans * trans = NEWZERO(VaultFindNodeTrans);
	trans->callback	= callback;
	trans->param	= param;

	NetCliAuthVaultNodeFind(
		templateNode,
		VaultFindNodeTrans::VaultNodeFound,
		trans
	);	
}

//============================================================================
namespace _VaultFindNodesAndWait {
	struct _FindNodeParam {
		ARRAY(unsigned)		nodeIds;
		ENetError			result;
		bool				complete;
	};
	static void _FindNodeCallback (
		ENetError			result,
		void *				vparam,
		unsigned			nodeIdCount,
		const unsigned		nodeIds[]
	) {
		_FindNodeParam * param = (_FindNodeParam *)vparam;
		param->nodeIds.Set(nodeIds, nodeIdCount);
		param->result	= result;
		param->complete	= true;
	}

} // namespace _VaultFindNodesAndWait

void VaultFindNodesAndWait (
	NetVaultNode *			templateNode,
	ARRAY(unsigned) *		nodeIds
) {
	using namespace _VaultFindNodesAndWait;
	
	_FindNodeParam	param;
	ZERO(param);
	
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
	NetVaultNode *			templateNode,
	ARRAY(unsigned) *		nodeIds
) {
	for (RelVaultNodeLink * link = s_nodes.Head(); link != nil; link = s_nodes.Next(link)) {
		if (link->node->Matches(templateNode))
			nodeIds->Add(link->node->nodeId);
	}
}

//============================================================================
namespace _VaultFetchNodesAndWait {

	static void _VaultNodeFetched (
		ENetError			result,
		void *				param,
		NetVaultNode *		node
	) {
		::VaultNodeFetched(result, nil, node);
		
		long * nodeCount = (long *)param;
		AtomicAdd(nodeCount, -1);
	}

} // namespace _VaultFetchNodesAndWait

void VaultFetchNodesAndWait (
	const unsigned	nodeIds[],
	unsigned		count,
	bool			force
) {
	using namespace _VaultFetchNodesAndWait;
	
	long nodeCount = (long)count;
	
	for (unsigned i = 0; i < count; ++i) {
		
		if (!force) {
			// See if we already have this node
			if (RelVaultNodeLink * link = s_nodes.Find(nodeIds[i])) {
				AtomicAdd(&nodeCount, -1);
				continue;
			}
		}

		// Start fetching the node			
		NetCliAuthVaultNodeFetch(nodeIds[i], _VaultNodeFetched, (void *)&nodeCount);
	}

	while (nodeCount) {
		NetClientUpdate();
		AsyncSleep(10);
	}	
}


//============================================================================
void VaultInitAge (
	const plAgeInfoStruct * info,
	const Uuid &			parentAgeInstId,	// optional
	FVaultInitAgeCallback	callback,
	void *					state,
	void *					param
) {
	VaultAgeInitTrans * trans = NEWZERO(VaultAgeInitTrans);
	trans->callback			= callback;
	trans->cbState			= state;
	trans->cbParam			= param;
	
	wchar ageFilename[MAX_PATH];
	wchar ageInstName[MAX_PATH];
	wchar ageUserName[MAX_PATH];
	wchar ageDesc[1024];

	StrToUnicode(ageFilename, info->GetAgeFilename(), arrsize(ageFilename));
	StrToUnicode(ageInstName, info->GetAgeInstanceName(), arrsize(ageInstName));
	StrToUnicode(ageUserName, info->GetAgeUserDefinedName(), arrsize(ageUserName));
	StrToUnicode(ageDesc, info->GetAgeDescription(), arrsize(ageDesc));
	
	NetCliAuthVaultInitAge(
		(Uuid)*info->GetAgeInstanceGuid(),
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
static RelVaultNode * GetPlayerNode () {
	NetVaultNode * templateNode = NEWZERO(NetVaultNode);
	templateNode->IncRef();
	templateNode->SetNodeType(plVault::kNodeType_VNodeMgrPlayer);
	if (NetCommGetPlayer())
		templateNode->SetNodeId(NetCommGetPlayer()->playerInt);
	RelVaultNode * result = GetNode(templateNode);
	templateNode->DecRef();
	return result;
}

//============================================================================
unsigned VaultGetPlayerId () {
	if (RelVaultNode * rvn = GetPlayerNode())
		return rvn->nodeId;
	return 0;
}

//============================================================================
RelVaultNode * VaultGetPlayerNodeIncRef () {
	if (RelVaultNode * rvnPlr = GetPlayerNode()) {
		rvnPlr->IncRef();
		return rvnPlr;
	}
	return nil;
}

//============================================================================
RelVaultNode * VaultGetPlayerInfoNodeIncRef () {
	RelVaultNode * rvnPlr = VaultGetPlayerNodeIncRef();
	if (!rvnPlr)
		return nil;

	NetVaultNode * templateNode = NEWZERO(NetVaultNode);
	templateNode->IncRef();
	templateNode->SetNodeType(plVault::kNodeType_PlayerInfo);
	VaultPlayerInfoNode plrInfo(templateNode);
	plrInfo.SetPlayerId(rvnPlr->nodeId);
			
	rvnPlr->DecRef();

	RelVaultNode * result = nil;
	if (RelVaultNode * rvnPlrInfo = rvnPlr->GetChildNodeIncRef(templateNode, 1))
		result = rvnPlrInfo;

	templateNode->DecRef();
	
	return result;
}

//============================================================================
RelVaultNode * VaultGetAvatarOutfitFolderIncRef () {
	if (RelVaultNode * rvn = GetPlayerNode())
		return rvn->GetChildFolderNodeIncRef(plVault::kAvatarOutfitFolder, 1);
	return nil;
}

//============================================================================
RelVaultNode * VaultGetAvatarClosetFolderIncRef () {
	if (RelVaultNode * rvn = GetPlayerNode())
		return rvn->GetChildFolderNodeIncRef(plVault::kAvatarClosetFolder, 1);
	return nil;
}

//============================================================================
RelVaultNode * VaultGetChronicleFolderIncRef () {
	if (RelVaultNode * rvn = GetPlayerNode())
		return rvn->GetChildFolderNodeIncRef(plVault::kChronicleFolder, 1);
	return nil;
}

//============================================================================
RelVaultNode * VaultGetAgesIOwnFolderIncRef () {
	if (RelVaultNode * rvn = GetPlayerNode())
		return rvn->GetChildAgeInfoListNodeIncRef(plVault::kAgesIOwnFolder, 1);
	return nil;
}

//============================================================================
RelVaultNode * VaultGetAgesICanVisitFolderIncRef () {
	if (RelVaultNode * rvn = GetPlayerNode())
		return rvn->GetChildAgeInfoListNodeIncRef(plVault::kAgesICanVisitFolder, 1);
	return nil;
}

//============================================================================
RelVaultNode * VaultGetPlayerInboxFolderIncRef () {
	if (RelVaultNode * rvn = GetPlayerNode())
		return rvn->GetChildFolderNodeIncRef(plVault::kInboxFolder, 1);
	return nil;
}

//============================================================================
bool VaultGetLinkToMyNeighborhood (plAgeLinkStruct * link) {
	RelVaultNode * rvnFldr = VaultGetAgesIOwnFolderIncRef();
	if (!rvnFldr)
		return false;

	NetVaultNode * templateNode = NEWZERO(NetVaultNode);
	templateNode->IncRef();
	
	templateNode->SetNodeType(plVault::kNodeType_AgeInfo);
	VaultAgeInfoNode ageInfo(templateNode);
	wchar str[MAX_PATH];
	StrToUnicode(str, kNeighborhoodAgeFilename, arrsize(str));
	ageInfo.SetAgeFilename(str);

	RelVaultNode * node;
	if (nil != (node = rvnFldr->GetChildNodeIncRef(templateNode, 2))) {
		VaultAgeInfoNode info(node);
		info.CopyTo(link->GetAgeInfo());
		node->DecRef();
	}
	templateNode->DecRef();
	rvnFldr->DecRef();

	return node != nil;
}

//============================================================================
bool VaultGetLinkToMyPersonalAge (plAgeLinkStruct * link) {
	RelVaultNode * rvnFldr = VaultGetAgesIOwnFolderIncRef();
	if (!rvnFldr)
		return false;

	NetVaultNode * templateNode = NEWZERO(NetVaultNode);
	templateNode->IncRef();

	templateNode->SetNodeType(plVault::kNodeType_AgeInfo);
	VaultAgeInfoNode ageInfo(templateNode);
	wchar str[MAX_PATH];
	StrToUnicode(str, kPersonalAgeFilename, arrsize(str));
	ageInfo.SetAgeFilename(str);

	RelVaultNode * node;
	if (nil != (node = rvnFldr->GetChildNodeIncRef(templateNode, 2))) {
		VaultAgeInfoNode info(node);
		info.CopyTo(link->GetAgeInfo());
		node->DecRef();
	}
	templateNode->DecRef();
	rvnFldr->DecRef();

	return node != nil;
}

//============================================================================
bool VaultGetLinkToCity (plAgeLinkStruct * link) {
	RelVaultNode * rvnFldr = VaultGetAgesIOwnFolderIncRef();
	if (!rvnFldr)
		return false;

	NetVaultNode * templateNode = NEWZERO(NetVaultNode);
	templateNode->IncRef();
	templateNode->SetNodeType(plVault::kNodeType_AgeInfo);

	VaultAgeInfoNode ageInfo(templateNode);
	wchar str[MAX_PATH];
	StrToUnicode(str, kCityAgeFilename, arrsize(str));
	ageInfo.SetAgeFilename(str);

	RelVaultNode * node;
	if (nil != (node = rvnFldr->GetChildNodeIncRef(templateNode, 2))) {
		VaultAgeInfoNode info(node);
		info.CopyTo(link->GetAgeInfo());
		node->DecRef();
	}
	templateNode->DecRef();
	rvnFldr->DecRef();

	return node != nil;
}

//============================================================================
RelVaultNode * VaultGetOwnedAgeLinkIncRef (const plAgeInfoStruct * info) {
	
	RelVaultNode * rvnLink = nil;
	
	if (RelVaultNode * rvnFldr = VaultGetAgesIOwnFolderIncRef()) {

		NetVaultNode * templateNode = NEWZERO(NetVaultNode);
		templateNode->IncRef();
		templateNode->SetNodeType(plVault::kNodeType_AgeInfo);

		VaultAgeInfoNode ageInfo(templateNode);
		if (info->HasAgeFilename()) {
			wchar str[MAX_PATH];
			StrToUnicode(str, info->GetAgeFilename(), arrsize(str));
			ageInfo.SetAgeFilename(str);
		}
		if (info->HasAgeInstanceGuid()) {
			ageInfo.SetAgeInstGuid(*info->GetAgeInstanceGuid());
		}

		if (RelVaultNode * rvnInfo = rvnFldr->GetChildNodeIncRef(templateNode, 2)) {
			templateNode->fieldFlags = 0;
			templateNode->SetNodeType(plVault::kNodeType_AgeLink);	
			rvnLink = rvnInfo->GetParentNodeIncRef(templateNode, 1);
			rvnInfo->DecRef();
		}

		templateNode->DecRef();
		rvnFldr->DecRef();
	}
	
	return rvnLink;
}

//============================================================================
RelVaultNode * VaultGetOwnedAgeInfoIncRef (const plAgeInfoStruct * info) {
	
	RelVaultNode * rvnInfo = nil;
	
	if (RelVaultNode * rvnFldr = VaultGetAgesIOwnFolderIncRef()) {

		NetVaultNode * templateNode = NEWZERO(NetVaultNode);
		templateNode->IncRef();
		templateNode->SetNodeType(plVault::kNodeType_AgeInfo);

		VaultAgeInfoNode ageInfo(templateNode);
		if (info->HasAgeFilename()) {
			wchar str[MAX_PATH];
			StrToUnicode(str, info->GetAgeFilename(), arrsize(str));
			ageInfo.SetAgeFilename(str);
		}
		if (info->HasAgeInstanceGuid()) {
			ageInfo.SetAgeInstGuid(*info->GetAgeInstanceGuid());
		}

		rvnInfo = rvnFldr->GetChildNodeIncRef(templateNode, 2);

		templateNode->DecRef();
		rvnFldr->DecRef();
	}
	
	return rvnInfo;
}

//============================================================================
bool VaultGetOwnedAgeLink (const plAgeInfoStruct * info, plAgeLinkStruct * link) {
	bool result = false;
	if (RelVaultNode * rvnLink = VaultGetOwnedAgeLinkIncRef(info)) {
		if (RelVaultNode * rvnInfo = rvnLink->GetChildNodeIncRef(plVault::kNodeType_AgeInfo, 1)) {
			VaultAgeInfoNode ageInfo(rvnInfo);
			ageInfo.CopyTo(link->GetAgeInfo());
			rvnInfo->DecRef();
			result = true;
		}

		rvnLink->DecRef();
	}
	
	return result;
}

//============================================================================
bool VaultFindOrCreateChildAgeLinkAndWait (const wchar ownedAgeName[], const plAgeInfoStruct * info, plAgeLinkStruct * link) {
	hsAssert(false, "eric, implement me");
	return false;
}

//============================================================================
bool VaultAddOwnedAgeSpawnPoint (const Uuid & ageInstId, const plSpawnPointInfo & spawnPt) {
	
	RelVaultNode * fldr = nil;
	RelVaultNode * link = nil;
	
	for (;;) {
		if (!spawnPt.GetName())
			break;
		if (!spawnPt.GetTitle())
			break;
		if (!StrLen(spawnPt.GetName()))
			break;
		if (!StrLen(spawnPt.GetTitle()))
			break;

		fldr = VaultGetAgesIOwnFolderIncRef();
		if (!fldr)
			break;

		ARRAY(unsigned) nodeIds;
		fldr->GetChildNodeIds(&nodeIds, 1);
		
		NetVaultNode * templateNode = NEWZERO(NetVaultNode);
		templateNode->IncRef();
		templateNode->SetNodeType(plVault::kNodeType_AgeInfo);
		VaultAgeInfoNode access(templateNode);
		access.SetAgeInstGuid(ageInstId);
		
		for (unsigned i = 0; i < nodeIds.Count(); ++i) {
			link = VaultGetNodeIncRef(nodeIds[i]);
			if (!link)
				continue;
			if (RelVaultNode * info = link->GetChildNodeIncRef(templateNode, 1)) {
				VaultAgeLinkNode access(link);
				access.AddSpawnPoint(spawnPt);
				info->DecRef();
				link->DecRef();
				link = nil;
				break;
			}
		}
		templateNode->DecRef();

		break;	
	}		

	if (fldr)		
		fldr->DecRef();
	if (link)
		link->DecRef();
		
	return true;
}

//============================================================================
bool VaultSetOwnedAgePublicAndWait (const plAgeInfoStruct * info, bool publicOrNot) {
	if (RelVaultNode * rvnLink = VaultGetOwnedAgeLinkIncRef(info)) {
		if (RelVaultNode * rvnInfo = rvnLink->GetChildNodeIncRef(plVault::kNodeType_AgeInfo, 1)) {
			NetCliAuthSetAgePublic(rvnInfo->nodeId, publicOrNot);

			VaultAgeInfoNode access(rvnInfo);
			char ageName[MAX_PATH];
			StrToAnsi(ageName, access.ageFilename, arrsize(ageName));
			
			plVaultNotifyMsg * msg = NEWZERO(plVaultNotifyMsg);
			if (publicOrNot)
				msg->SetType(plVaultNotifyMsg::kPublicAgeCreated);
			else
				msg->SetType(plVaultNotifyMsg::kPublicAgeRemoved);
			msg->SetResultCode(true);
			msg->GetArgs()->AddString(plNetCommon::VaultTaskArgs::kAgeFilename, ageName);
			msg->Send();
			
			rvnInfo->DecRef();
		}
		rvnLink->DecRef();
	}
	return true;
}

//============================================================================
RelVaultNode * VaultGetVisitAgeLinkIncRef (const plAgeInfoStruct * info) {
	RelVaultNode * rvnLink = nil;
	
	if (RelVaultNode * rvnFldr = VaultGetAgesICanVisitFolderIncRef()) {

		NetVaultNode * templateNode = NEWZERO(NetVaultNode);
		templateNode->IncRef();
		templateNode->SetNodeType(plVault::kNodeType_AgeInfo);

		VaultAgeInfoNode ageInfo(templateNode);
		if (info->HasAgeFilename()) {
			wchar str[MAX_PATH];
			StrToUnicode(str, info->GetAgeFilename(), arrsize(str));
			ageInfo.SetAgeFilename(str);
		}
		if (info->HasAgeInstanceGuid()) {
			ageInfo.SetAgeInstGuid(*info->GetAgeInstanceGuid());
		}

		if (RelVaultNode * rvnInfo = rvnFldr->GetChildNodeIncRef(templateNode, 2)) {
			templateNode->fieldFlags = 0;
			templateNode->SetNodeType(plVault::kNodeType_AgeLink);	
			rvnLink = rvnInfo->GetParentNodeIncRef(templateNode, 1);
			rvnInfo->DecRef();
		}

		templateNode->DecRef();
		rvnFldr->DecRef();
	}
	
	return rvnLink;
}

//============================================================================
bool VaultGetVisitAgeLink (const plAgeInfoStruct * info, class plAgeLinkStruct * link) {
	RelVaultNode * rvn = VaultGetVisitAgeLinkIncRef(info);
	if (!rvn)
		return false;
		
	VaultAgeLinkNode ageLink(rvn);
	ageLink.CopyTo(link);
		
	rvn->DecRef();
	return true;
}

//============================================================================
namespace _VaultRegisterOwnedAgeAndWait {

struct _InitAgeParam {
	ENetError		result;
	bool			complete;
	unsigned		ageInfoId;
};
static void _InitAgeCallback (
	ENetError		result,
	void *			,
	void *			vparam,
	unsigned		ageVaultId,
	unsigned		ageInfoVaultId
) {
	_InitAgeParam * param = (_InitAgeParam *)vparam;
	param->ageInfoId	= ageInfoVaultId;
	param->result		= result;
	param->complete		= true;
}
struct _FetchVaultParam {
	ENetError		result;
	bool			complete;
};
static void _FetchVaultCallback (
	ENetError		result,
	void *			vparam
) {
	_FetchVaultParam * param = (_FetchVaultParam *)vparam;
	param->result		= result;
	param->complete		= true;
}
struct _CreateNodeParam {
	ENetError		result;
	bool			complete;
	unsigned		nodeId;
};
static void _CreateNodeCallback (
	ENetError		result,
	void *			,
	void *			vparam,
	RelVaultNode *	node
) {
	_CreateNodeParam * param = (_CreateNodeParam *)vparam;
	if (IS_NET_SUCCESS(result))
		param->nodeId = node->nodeId;
	param->result		= result;
	param->complete		= true;
}
struct _AddChildNodeParam {
	ENetError		result;
	bool			complete;
};
static void _AddChildNodeCallback (
	ENetError		result,
	void *			vparam
) {
	_AddChildNodeParam * param = (_AddChildNodeParam *)vparam;
	param->result		= result;
	param->complete		= true;
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
		if (RelVaultNode * rvn = VaultGetAgesIOwnFolderIncRef()) {
			agesIOwnId = rvn->nodeId;
			rvn->DecRef();
		}
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
		
		{	// Init age vault
			_InitAgeParam	param;
			ZERO(param);

			VaultInitAge(
				link->GetAgeInfo(),
				kNilGuid,
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
				LogMsg(kLogError, L"RegisterOwnedAge: Failed to init age %S", link->GetAgeInfo()->GetAgeFilename());
				break;
			}
				
			ageInfoId = param.ageInfoId;
		}		
		
		{	// Create age link
			_CreateNodeParam	param;
			ZERO(param);

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

		{	// Fetch age info node tree
			_FetchVaultParam	param;
			ZERO(param);
			
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
			_AddChildNodeParam	param1;
			_AddChildNodeParam	param2;
			_AddChildNodeParam	param3;
			ZERO(param1);
			ZERO(param2);
			ZERO(param3);

			unsigned ageOwnersId = 0;		
			if (RelVaultNode * rvnAgeInfo = VaultGetNodeIncRef(ageInfoId)) {
				if (RelVaultNode * rvnAgeOwners = rvnAgeInfo->GetChildPlayerInfoListNodeIncRef(plVault::kAgeOwnersFolder, 1)) {
					ageOwnersId = rvnAgeOwners->nodeId;
					rvnAgeOwners->DecRef();
				}
				rvnAgeInfo->DecRef();
			}
			
			unsigned playerInfoId = 0;
			if (RelVaultNode * rvnPlayerInfo = VaultGetPlayerInfoNodeIncRef()) {
				playerInfoId = rvnPlayerInfo->nodeId;
				rvnPlayerInfo->DecRef();
			}

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
		if (RelVaultNode * node = VaultGetNodeIncRef(ageLinkId)) {
			VaultAgeLinkNode access(node);
			access.AddSpawnPoint(link->SpawnPoint());
			node->DecRef();
		}
		
		result = true;
		break;
	}
		
	plVaultNotifyMsg * msg = NEWZERO(plVaultNotifyMsg);
	msg->SetType(plVaultNotifyMsg::kRegisteredOwnedAge);
	msg->SetResultCode(result);
	msg->GetArgs()->AddInt(plNetCommon::VaultTaskArgs::kAgeLinkNode, ageLinkId);
	msg->Send();

	return result;
}

//============================================================================
namespace _VaultRegisterVisitAgeAndWait {

struct _InitAgeParam {
	ENetError		result;
	bool			complete;
	unsigned		ageInfoId;
};
static void _InitAgeCallback (
	ENetError		result,
	void *			,
	void *			vparam,
	unsigned		ageVaultId,
	unsigned		ageInfoVaultId
) {
	_InitAgeParam * param = (_InitAgeParam *)vparam;
	param->ageInfoId	= ageInfoVaultId;
	param->result		= result;
	param->complete		= true;
}
struct _FetchVaultParam {
	ENetError		result;
	bool			complete;
};
static void _FetchVaultCallback (
	ENetError		result,
	void *			vparam
) {
	_FetchVaultParam * param = (_FetchVaultParam *)vparam;
	param->result		= result;
	param->complete		= true;
}
struct _CreateNodeParam {
	ENetError		result;
	bool			complete;
	unsigned		nodeId;
};
static void _CreateNodeCallback (
	ENetError		result,
	void *			,
	void *			vparam,
	RelVaultNode *	node
) {
	_CreateNodeParam * param = (_CreateNodeParam *)vparam;
	if (IS_NET_SUCCESS(result))
		param->nodeId = node->nodeId;
	param->result		= result;
	param->complete		= true;
}
struct _AddChildNodeParam {
	ENetError		result;
	bool			complete;
};
static void _AddChildNodeCallback (
	ENetError		result,
	void *			vparam
) {
	_AddChildNodeParam * param = (_AddChildNodeParam *)vparam;
	param->result		= result;
	param->complete		= true;
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
		if (RelVaultNode * rvn = VaultGetAgesICanVisitFolderIncRef()) {
			agesICanVisitId = rvn->nodeId;
			rvn->DecRef();
		}
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
		
		
		{	// Init age vault
			_InitAgeParam	param;
			ZERO(param);

			VaultInitAge(
				link->GetAgeInfo(),
				kNilGuid,
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
				LogMsg(kLogError, L"RegisterVisitAge: Failed to init age %S", link->GetAgeInfo()->GetAgeFilename());
				break;
			}
				
			ageInfoId = param.ageInfoId;
		}		
		
		{	// Create age link
			_CreateNodeParam	param;
			ZERO(param);

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

		{	// Fetch age info node tree
			_FetchVaultParam	param;
			ZERO(param);
			
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
			_AddChildNodeParam	param1;
			_AddChildNodeParam	param2;
			_AddChildNodeParam	param3;
			ZERO(param1);
			ZERO(param2);
			ZERO(param3);

			unsigned ageVisitorsId = 0;		
			if (RelVaultNode * rvnAgeInfo = VaultGetNodeIncRef(ageInfoId)) {
				if (RelVaultNode * rvnAgeVisitors = rvnAgeInfo->GetChildPlayerInfoListNodeIncRef(plVault::kCanVisitFolder, 1)) {
					ageVisitorsId = rvnAgeVisitors->nodeId;
					rvnAgeVisitors->DecRef();
				}
				rvnAgeInfo->DecRef();
			}
			
			unsigned playerInfoId = 0;
			if (RelVaultNode * rvnPlayerInfo = VaultGetPlayerInfoNodeIncRef()) {
				playerInfoId = rvnPlayerInfo->nodeId;
				rvnPlayerInfo->DecRef();
			}

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
		if (RelVaultNode * node = VaultGetNodeIncRef(ageLinkId)) {
			VaultAgeLinkNode access(node);
			access.AddSpawnPoint(link->SpawnPoint());
			node->DecRef();
		}

		result = true;
		break;
	}

	plVaultNotifyMsg * msg = NEWZERO(plVaultNotifyMsg);
	msg->SetType(plVaultNotifyMsg::kRegisteredVisitAge);
	msg->SetResultCode(result);
	msg->GetArgs()->AddInt(plNetCommon::VaultTaskArgs::kAgeLinkNode, ageLinkId);
	msg->Send();

	return result;
}


//============================================================================
bool VaultUnregisterOwnedAgeAndWait (const plAgeInfoStruct * info) {

	unsigned ageLinkId = 0;
	unsigned agesIOwnId;

	bool result = false;
	for (;;) {	
		RelVaultNode * rvnLink = VaultGetOwnedAgeLinkIncRef(info);
		if (!rvnLink) {
			result = true;
			break;	// we aren't an owner of the age, just return true
		}

		if (RelVaultNode * rvn = VaultGetAgesIOwnFolderIncRef()) {
			agesIOwnId = rvn->nodeId;
			rvn->DecRef();
		}
		else {
			LogMsg(kLogError, L"UnregisterOwnedAge: Failed to get player's AgesIOwnFolder");
			break;	// something's wrong with the player vault, it doesn't have a required folder node
		}
			
		ageLinkId = rvnLink->nodeId;
		
		unsigned ageOwnersId = 0;
		if (RelVaultNode * rvnAgeInfo = rvnLink->GetChildNodeIncRef(plVault::kNodeType_AgeInfo, 1)) {
			if (RelVaultNode * rvnAgeOwners = rvnAgeInfo->GetChildPlayerInfoListNodeIncRef(plVault::kAgeOwnersFolder, 1)) {
				ageOwnersId = rvnAgeOwners->nodeId;
				rvnAgeOwners->DecRef();
			}
			rvnAgeInfo->DecRef();
		}
		
		unsigned playerInfoId = 0;
		if (RelVaultNode * rvnPlayerInfo = VaultGetPlayerInfoNodeIncRef()) {
			playerInfoId = rvnPlayerInfo->nodeId;
			rvnPlayerInfo->DecRef();
		}
		
		rvnLink->DecRef();

		// remove our playerInfo from the ageOwners folder
		VaultRemoveChildNode(ageOwnersId, playerInfoId, nil, nil);
		
		// remove the link from AgesIOwn folder	
		VaultRemoveChildNode(agesIOwnId, ageLinkId, nil, nil);

		// delete the link node since link nodes aren't shared with anyone else
	//	VaultDeleteNode(ageLinkId);

		result = true;
		break;
	}
	
	plVaultNotifyMsg * msg = NEWZERO(plVaultNotifyMsg);
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
		RelVaultNode * rvnLink = VaultGetVisitAgeLinkIncRef(info);
		if (!rvnLink) {
			result = true;
			break;	// we aren't an owner of the age, just return true
		}

		if (RelVaultNode * rvn = VaultGetAgesICanVisitFolderIncRef()) {
			agesICanVisitId = rvn->nodeId;
			rvn->DecRef();
		}
		else {
			LogMsg(kLogError, L"UnregisterOwnedAge: Failed to get player's AgesICanVisitFolder");
			break;	// something's wrong with the player vault, it doesn't have a required folder node
		}
			
		ageLinkId = rvnLink->nodeId;

		unsigned ageVisitorsId = 0;
		if (RelVaultNode * rvnAgeInfo = rvnLink->GetChildNodeIncRef(plVault::kNodeType_AgeInfo, 1)) {
			if (RelVaultNode * rvnAgeVisitors = rvnAgeInfo->GetChildPlayerInfoListNodeIncRef(plVault::kCanVisitFolder, 1)) {
				ageVisitorsId = rvnAgeVisitors->nodeId;
				rvnAgeVisitors->DecRef();
			}
			rvnAgeInfo->DecRef();
		}
		
		unsigned playerInfoId = 0;
		if (RelVaultNode * rvnPlayerInfo = VaultGetPlayerInfoNodeIncRef()) {
			playerInfoId = rvnPlayerInfo->nodeId;
			rvnPlayerInfo->DecRef();
		}

		rvnLink->DecRef();

		// remove our playerInfo from the ageVisitors folder
		VaultRemoveChildNode(ageVisitorsId, playerInfoId, nil, nil);

		// remove the link from AgesICanVisit folder	
		VaultRemoveChildNode(agesICanVisitId, ageLinkId, nil, nil);
		
		// delete the link node since link nodes aren't shared with anyone else
	//	VaultDeleteNode(ageLinkId);
	
		result = true;
		break;
	}
	
	plVaultNotifyMsg * msg = NEWZERO(plVaultNotifyMsg);
	msg->SetType(plVaultNotifyMsg::kUnRegisteredVisitAge);
	msg->SetResultCode(result);
	msg->GetArgs()->AddInt(plNetCommon::VaultTaskArgs::kAgeLinkNode, ageLinkId);
	msg->Send();

	return result;
}

//============================================================================
RelVaultNode * VaultFindChronicleEntryIncRef (const wchar entryName[], int entryType) {

	RelVaultNode * result = nil;
	if (RelVaultNode * rvnFldr = GetChildFolderNode(GetPlayerNode(), plVault::kChronicleFolder, 1)) {
		NetVaultNode * templateNode = NEWZERO(NetVaultNode);
		templateNode->IncRef();
		templateNode->SetNodeType(plVault::kNodeType_Chronicle);
		VaultChronicleNode chrn(templateNode);
		chrn.SetEntryName(entryName);
		if (entryType >= 0)
			chrn.SetEntryType(entryType);
		if (RelVaultNode * rvnChrn = rvnFldr->GetChildNodeIncRef(templateNode, 255))
			result = rvnChrn;
		templateNode->DecRef();
	}		
	return result;
}

//============================================================================
bool VaultHasChronicleEntry (const wchar entryName[], int entryType) {
	if (RelVaultNode * rvn = VaultFindChronicleEntryIncRef(entryName, entryType)) {
		rvn->DecRef();
		return true;
	}
	return false;
}

//============================================================================
void VaultAddChronicleEntryAndWait (
	const wchar entryName[],
	int			entryType,
	const wchar entryValue[]
) {
	if (RelVaultNode * rvnChrn = VaultFindChronicleEntryIncRef(entryName, entryType)) {
		VaultChronicleNode chrnNode(rvnChrn);
		chrnNode.SetEntryValue(entryValue);
	}
	else if (RelVaultNode * rvnFldr = GetChildFolderNode(GetPlayerNode(), plVault::kChronicleFolder, 1)) {
		NetVaultNode * templateNode = NEWZERO(NetVaultNode);
		templateNode->IncRef();
		templateNode->SetNodeType(plVault::kNodeType_Chronicle);
		VaultChronicleNode chrnNode(templateNode);
		chrnNode.SetEntryName(entryName);
		chrnNode.SetEntryType(entryType);
		chrnNode.SetEntryValue(entryValue);
		ENetError result;
		if (RelVaultNode * rvnChrn = VaultCreateNodeAndWaitIncRef(templateNode, &result)) {
			VaultAddChildNode(rvnFldr->nodeId, rvnChrn->nodeId, 0, nil, nil);
			rvnChrn->DecRef();
		}
		templateNode->DecRef();
	}
}

//============================================================================
bool VaultAmIgnoringPlayer (unsigned playerId) {
	bool retval = false;
	if (RelVaultNode * rvnFldr = GetChildPlayerInfoListNode(GetPlayerNode(), plVault::kIgnoreListFolder, 1)) {
		rvnFldr->IncRef();

		NetVaultNode * templateNode = NEWZERO(NetVaultNode);
		templateNode->IncRef();
		templateNode->SetNodeType(plVault::kNodeType_PlayerInfo);
		VaultPlayerInfoNode pinfoNode(templateNode);
		pinfoNode.SetPlayerId(playerId);

		if (RelVaultNode * rvnPlayerInfo = rvnFldr->GetChildNodeIncRef(templateNode, 1)) {
			retval = true;
			rvnPlayerInfo->DecRef();
		}

		templateNode->DecRef();
		rvnFldr->DecRef();
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
	if (RelVaultNode * rvnSystem = VaultGetSystemNodeIncRef()) {
		VaultSystemNode sysNode(rvnSystem);
		retval = (sysNode.ccrStatus != 0);

		rvnSystem->DecRef();
	}

	return retval;
}

//============================================================================
bool VaultSetCCRStatus (bool online) {
	bool retval = false;
	if (RelVaultNode * rvnSystem = VaultGetSystemNodeIncRef()) {
		VaultSystemNode sysNode(rvnSystem);
		sysNode.SetCCRStatus(online ? 1 : 0);

		rvnSystem->DecRef();
		retval = true;
	}

	return retval;
}

//============================================================================
void VaultDump (const wchar tag[], unsigned vaultId, FStateDump dumpProc) {
	LogMsg(kLogDebug, L"<---- ID:%u, Begin Vault%*s%s ---->", vaultId, tag ? 1 : 0, L" ", tag);

	if (RelVaultNode * rvn = GetNode(vaultId))
		rvn->PrintTree(dumpProc, 0);

	LogMsg(kLogDebug, L"<---- ID:%u, End Vault%*s%s ---->", vaultId, tag ? 1 : 0, L" ", tag);
}

//============================================================================
void VaultDump (const wchar tag[], unsigned vaultId) {
	VaultDump (tag, vaultId, LogDumpProc);
}

//============================================================================
bool VaultAmInMyPersonalAge () {
	bool result = false;

	plAgeInfoStruct info;
	info.SetAgeFilename(kPersonalAgeFilename);

	if (RelVaultNode * rvnLink = VaultGetOwnedAgeLinkIncRef(&info)) {
		if (RelVaultNode * rvnInfo = rvnLink->GetChildNodeIncRef(plVault::kNodeType_AgeInfo, 1)) {
			VaultAgeInfoNode ageInfo(rvnInfo);

			if (RelVaultNode* currentAgeInfoNode = VaultGetAgeInfoNodeIncRef()) {
				VaultAgeInfoNode curAgeInfo(currentAgeInfoNode);

				if (ageInfo.ageInstUuid == curAgeInfo.ageInstUuid)
					result = true;

				currentAgeInfoNode->DecRef();
			}

			rvnInfo->DecRef();
		}

		rvnLink->DecRef();
	}

	return result;
}

//============================================================================
bool VaultAmInMyNeighborhoodAge () {
	bool result = false;

	plAgeInfoStruct info;
	info.SetAgeFilename(kNeighborhoodAgeFilename);

	if (RelVaultNode * rvnLink = VaultGetOwnedAgeLinkIncRef(&info)) {
		if (RelVaultNode * rvnInfo = rvnLink->GetChildNodeIncRef(plVault::kNodeType_AgeInfo, 1)) {
			VaultAgeInfoNode ageInfo(rvnInfo);

			if (RelVaultNode* currentAgeInfoNode = VaultGetAgeInfoNodeIncRef()) {
				VaultAgeInfoNode curAgeInfo(currentAgeInfoNode);

				if (ageInfo.ageInstUuid == curAgeInfo.ageInstUuid)
					result = true;

				currentAgeInfoNode->DecRef();
			}

			rvnInfo->DecRef();
		}

		rvnLink->DecRef();
	}

	return result;
}

//============================================================================
bool VaultAmOwnerOfCurrentAge () {
	bool result = false;

	if (RelVaultNode* currentAgeInfoNode = VaultGetAgeInfoNodeIncRef()) {
		VaultAgeInfoNode curAgeInfo(currentAgeInfoNode);

		char* ageFilename = StrDupToAnsi(curAgeInfo.ageFilename);

		plAgeInfoStruct info;
		info.SetAgeFilename(ageFilename);

		FREE(ageFilename);
		
		if (RelVaultNode * rvnLink = VaultGetOwnedAgeLinkIncRef(&info)) {

			if (RelVaultNode * rvnInfo = rvnLink->GetChildNodeIncRef(plVault::kNodeType_AgeInfo, 1)) {
				VaultAgeInfoNode ageInfo(rvnInfo);

				if (ageInfo.ageInstUuid == curAgeInfo.ageInstUuid)
					result = true;

				rvnInfo->DecRef();
			}

			rvnLink->DecRef();
		}

		currentAgeInfoNode->DecRef();
	}

	return result;
}

//============================================================================
bool VaultAmCzarOfCurrentAge () {
	hsAssert(false, "eric, implement me");
	return true;
}

//============================================================================
bool VaultAmOwnerOfAge (const Uuid & ageInstId) {
	hsAssert(false, "eric, implement me");
	return true;
}

//============================================================================
bool VaultAmCzarOfAge (const Uuid & ageInstId) {
//	hsAssert(false, "eric, implement me");
	return false;
}

//============================================================================
bool VaultRegisterMTStationAndWait (
	const wchar	stationName[],
	const wchar	linkBackSpawnPtObjName[]
) {
	plAgeInfoStruct info;
	info.SetAgeFilename(kCityAgeFilename);
	if (RelVaultNode * rvn = VaultGetOwnedAgeLinkIncRef(&info)) {
		char title[MAX_PATH], spawnPt[MAX_PATH];
		StrToAnsi(title, stationName, arrsize(title));
		StrToAnsi(spawnPt, linkBackSpawnPtObjName, arrsize(spawnPt));
		VaultAgeLinkNode link(rvn);
		link.AddSpawnPoint(plSpawnPointInfo(title, spawnPt));
		rvn->DecRef();
		return true;
	}
	return false;
}

//============================================================================
void VaultProcessVisitNote(RelVaultNode * rvnVisit) {
	if (RelVaultNode * rvnInbox = VaultGetPlayerInboxFolderIncRef()) {
		rvnVisit->IncRef();

		VaultTextNoteNode visitAcc(rvnVisit);
		plAgeLinkStruct link;
		if (visitAcc.GetVisitInfo(link.GetAgeInfo())) {
			// Add it to our "ages i can visit" folder
			VaultRegisterVisitAgeAndWait(&link);
		}
		// remove it from the inbox
		VaultRemoveChildNode(rvnInbox->nodeId, rvnVisit->nodeId, nil, nil);
		rvnVisit->DecRef();

		rvnInbox->DecRef();
	}
}

//============================================================================
void VaultProcessUnvisitNote(RelVaultNode * rvnUnVisit) {
	if (RelVaultNode * rvnInbox = VaultGetPlayerInboxFolderIncRef()) {
		rvnUnVisit->IncRef();
		VaultTextNoteNode unvisitAcc(rvnUnVisit);
		plAgeInfoStruct info;
		if (unvisitAcc.GetVisitInfo(&info)) {
			// Remove it from our "ages i can visit" folder
			VaultUnregisterVisitAgeAndWait(&info);
		}
		// remove it from the inbox
		VaultRemoveChildNode(rvnInbox->nodeId, rvnUnVisit->nodeId, nil, nil);
		rvnUnVisit->DecRef();

		rvnInbox->DecRef();
	}
}

//============================================================================
void VaultProcessPlayerInbox () {
	if (RelVaultNode * rvnInbox = VaultGetPlayerInboxFolderIncRef()) {
		{	// Process new visit requests
			ARRAY(RelVaultNode*) visits;
			RelVaultNode * templateNode = NEWZERO(RelVaultNode);
			templateNode->IncRef();
			templateNode->SetNodeType(plVault::kNodeType_TextNote);
			VaultTextNoteNode tmpAcc(templateNode);
			tmpAcc.SetNoteType(plVault::kNoteType_Visit);
			rvnInbox->GetChildNodesIncRef(templateNode, 1, &visits);
			templateNode->DecRef();
			
			for (unsigned i = 0; i < visits.Count(); ++i) {
				RelVaultNode * rvnVisit = visits[i];
				VaultTextNoteNode visitAcc(rvnVisit);
				plAgeLinkStruct link;
				if (visitAcc.GetVisitInfo(link.GetAgeInfo())) {
					// Add it to our "ages i can visit" folder
					VaultRegisterVisitAgeAndWait(&link);
				}
				// remove it from the inbox
				VaultRemoveChildNode(rvnInbox->nodeId, rvnVisit->nodeId, nil, nil);
				rvnVisit->DecRef();
			}
		}
		{	// Process new unvisit requests
			ARRAY(RelVaultNode*) unvisits;
			RelVaultNode * templateNode = NEWZERO(RelVaultNode);
			templateNode->IncRef();
			templateNode->SetNodeType(plVault::kNodeType_TextNote);
			VaultTextNoteNode tmpAcc(templateNode);
			tmpAcc.SetNoteType(plVault::kNoteType_UnVisit);
			rvnInbox->GetChildNodesIncRef(templateNode, 1, &unvisits);
			templateNode->DecRef();
			
			for (unsigned i = 0; i < unvisits.Count(); ++i) {
				RelVaultNode * rvnUnVisit = unvisits[i];
				VaultTextNoteNode unvisitAcc(rvnUnVisit);
				plAgeInfoStruct info;
				if (unvisitAcc.GetVisitInfo(&info)) {
					// Remove it from our "ages i can visit" folder
					VaultUnregisterVisitAgeAndWait(&info);
				}
				// remove it from the inbox
				VaultRemoveChildNode(rvnInbox->nodeId, rvnUnVisit->nodeId, nil, nil);
				rvnUnVisit->DecRef();
			}
		}
		
		rvnInbox->DecRef();
	}
}


/*****************************************************************************
*
*   Exports - Age Vault Access
*
***/

//============================================================================
static RelVaultNode * GetAgeNode () {
	NetVaultNode * templateNode = NEWZERO(NetVaultNode);
	templateNode->IncRef();
	templateNode->SetNodeType(plVault::kNodeType_VNodeMgrAge);
	if (NetCommGetAge())
		templateNode->SetNodeId(NetCommGetAge()->ageVaultId);
	RelVaultNode * result = GetNode(templateNode);
	templateNode->DecRef();
	return result;
}

//============================================================================
RelVaultNode * VaultGetAgeNodeIncRef () {
	RelVaultNode * result = nil;
	NetVaultNode * templateNode = NEWZERO(NetVaultNode);
	templateNode->IncRef();
	templateNode->SetNodeType(plVault::kNodeType_VNodeMgrAge);
	if (NetCommGetAge())
		templateNode->SetNodeId(NetCommGetAge()->ageVaultId);
	if (RelVaultNode * rvnAge = VaultGetNodeIncRef(templateNode))
		result = rvnAge;
	templateNode->DecRef();
	return result;
}

//============================================================================
static RelVaultNode * GetAgeInfoNode () {
	RelVaultNode * rvnAge = VaultGetAgeNodeIncRef();
	if (!rvnAge)
		return nil;

	RelVaultNode * result = nil;
	NetVaultNode * templateNode = NEWZERO(NetVaultNode);
	templateNode->IncRef();
	
	templateNode->SetNodeType(plVault::kNodeType_AgeInfo);
	templateNode->SetCreatorId(rvnAge->nodeId);
			
	if (RelVaultNode * rvnAgeInfo = rvnAge->GetChildNodeIncRef(templateNode, 1)) {
		rvnAgeInfo->DecRef();
		result = rvnAgeInfo;
	}
	
	templateNode->DecRef();
	rvnAge->DecRef();
	
	return result;
}

//============================================================================
RelVaultNode * VaultGetAgeInfoNodeIncRef () {
	RelVaultNode * rvnAge = VaultGetAgeNodeIncRef();
	if (!rvnAge)
		return nil;

	RelVaultNode * result = nil;
	NetVaultNode * templateNode = NEWZERO(NetVaultNode);
	templateNode->IncRef();
	templateNode->SetNodeType(plVault::kNodeType_AgeInfo);
	templateNode->SetCreatorId(rvnAge->nodeId);
			
	if (RelVaultNode * rvnAgeInfo = rvnAge->GetChildNodeIncRef(templateNode, 1))
		result = rvnAgeInfo;
	
	templateNode->DecRef();
	rvnAge->DecRef();

	return result;
}

//============================================================================
RelVaultNode * VaultGetAgeChronicleFolderIncRef () {
	if (RelVaultNode * rvn = GetAgeNode())
		return rvn->GetChildFolderNodeIncRef(plVault::kChronicleFolder, 1);
	return nil;
}

//============================================================================
RelVaultNode * VaultGetAgeDevicesFolderIncRef () {
	if (RelVaultNode * rvn = GetAgeNode())
		return rvn->GetChildFolderNodeIncRef(plVault::kAgeDevicesFolder, 1);
	return nil;
}

//============================================================================
RelVaultNode * VaultGetAgeAgeOwnersFolderIncRef () {
	if (RelVaultNode * rvn = GetAgeInfoNode())
		return rvn->GetChildPlayerInfoListNodeIncRef(plVault::kAgeOwnersFolder, 1);
	return nil;
}

//============================================================================
RelVaultNode * VaultGetAgeCanVisitFolderIncRef () {
	if (RelVaultNode * rvn = GetAgeInfoNode())
		return rvn->GetChildPlayerInfoListNodeIncRef(plVault::kCanVisitFolder, 1);
	return nil;
}

//============================================================================
RelVaultNode * VaultGetAgePeopleIKnowAboutFolderIncRef () {
	if (RelVaultNode * rvn = GetAgeNode())
		return rvn->GetChildPlayerInfoListNodeIncRef(plVault::kPeopleIKnowAboutFolder, 1);
	return nil;
}

//============================================================================
RelVaultNode * VaultGetAgeSubAgesFolderIncRef () {
	if (RelVaultNode * rvn = GetAgeNode())
		return rvn->GetChildAgeInfoListNodeIncRef(plVault::kSubAgesFolder, 1);
	return nil;
}

//============================================================================
RelVaultNode * VaultGetAgePublicAgesFolderIncRef () {
	hsAssert(false, "eric, implement me");
	return nil;
}

//============================================================================
RelVaultNode * VaultAgeGetBookshelfFolderIncRef () {
	if (RelVaultNode * rvn = GetAgeNode())
		return rvn->GetChildAgeInfoListNodeIncRef(plVault::kAgesIOwnFolder, 1);
	return nil;
}

//============================================================================
RelVaultNode * VaultFindAgeSubAgeLinkIncRef (const plAgeInfoStruct * info) {
	RelVaultNode * rvnLink = nil;
	
	if (RelVaultNode * rvnFldr = VaultGetAgeSubAgesFolderIncRef()) {

		NetVaultNode * templateNode = NEWZERO(NetVaultNode);
		templateNode->IncRef();
		templateNode->SetNodeType(plVault::kNodeType_AgeInfo);

		VaultAgeInfoNode ageInfo(templateNode);
		wchar str[MAX_PATH];
		StrToUnicode(str, info->GetAgeFilename(), arrsize(str));
		ageInfo.SetAgeFilename(str);

		if (RelVaultNode * rvnInfo = rvnFldr->GetChildNodeIncRef(templateNode, 2)) {
			templateNode->fieldFlags = 0;
			templateNode->SetNodeType(plVault::kNodeType_AgeLink);	
			rvnLink = rvnInfo->GetParentNodeIncRef(templateNode, 1);
			rvnInfo->DecRef();
		}

		templateNode->DecRef();
		rvnFldr->DecRef();
	}
	
	return rvnLink;
}

//============================================================================
RelVaultNode * VaultFindAgeChronicleEntryIncRef (const wchar entryName[], int entryType) {
	hsAssert(false, "eric, implement me");
	return nil;
}

//============================================================================
void VaultAddAgeChronicleEntry (
	const wchar entryName[],
	int			entryType,
	const wchar entryValue[]
) {
	hsAssert(false, "eric, implement me");
}

//============================================================================
RelVaultNode * VaultAgeAddDeviceAndWaitIncRef (const wchar deviceName[]) {
	if (RelVaultNode * existing = VaultAgeGetDeviceIncRef(deviceName))
		return existing;
		
	RelVaultNode * device = nil;
	RelVaultNode * folder = nil;
	
	for (;;) {
		folder = VaultGetAgeDevicesFolderIncRef();
		if (!folder)
			break;

		ENetError result;
		device = VaultCreateNodeAndWaitIncRef(plVault::kNodeType_TextNote, &result);
		if (!device)
			break;

		VaultTextNoteNode access(device);
		access.SetNoteType(plVault::kNoteType_Device);			
		access.SetNoteTitle(deviceName);
			
		VaultAddChildNodeAndWait(folder->nodeId, device->nodeId, 0);
		break;
	}

	if (folder)
		folder->DecRef();
		
	return device;
}

//============================================================================
void VaultAgeRemoveDevice (const wchar deviceName[]) {
	if (RelVaultNode * folder = VaultGetAgeDevicesFolderIncRef()) {
		NetVaultNode * templateNode = NEWZERO(NetVaultNode);
		templateNode->IncRef();
		templateNode->SetNodeType(plVault::kNodeType_TextNote);
		VaultTextNoteNode access(templateNode);
		access.SetNoteTitle(deviceName);
		if (RelVaultNode * device = folder->GetChildNodeIncRef(templateNode, 1)) {
			VaultRemoveChildNode(folder->nodeId, device->nodeId, nil, nil);
			device->DecRef();

			if (DeviceInbox * deviceInbox = s_ageDeviceInboxes.Find(CHashKeyStr(deviceName)))
				DEL(device);
		}
		templateNode->DecRef();
		folder->DecRef();
	}
}

//============================================================================
bool VaultAgeHasDevice (const wchar deviceName[]) {
	bool found = false;
	if (RelVaultNode * folder = VaultGetAgeDevicesFolderIncRef()) {
		NetVaultNode * templateNode = NEWZERO(NetVaultNode);
		templateNode->IncRef();
		templateNode->SetNodeType(plVault::kNodeType_TextNote);
		VaultTextNoteNode access(templateNode);
		access.SetNoteTitle(deviceName);
		if (RelVaultNode * device = folder->GetChildNodeIncRef(templateNode, 1)) {
			found = true;
			device->DecRef();
		}
		templateNode->DecRef();
		folder->DecRef();
	}
	return found;
}

//============================================================================
RelVaultNode * VaultAgeGetDeviceIncRef (const wchar deviceName[]) {
	RelVaultNode * result = nil;
	if (RelVaultNode * folder = VaultGetAgeDevicesFolderIncRef()) {
		NetVaultNode * templateNode = NEWZERO(NetVaultNode);
		templateNode->IncRef();
		templateNode->SetNodeType(plVault::kNodeType_TextNote);
		VaultTextNoteNode access(templateNode);
		access.SetNoteTitle(deviceName);
		if (RelVaultNode * device = folder->GetChildNodeIncRef(templateNode, 1))
			result = device;
		templateNode->DecRef();
		folder->DecRef();
	}
	return result;
}

//============================================================================
RelVaultNode * VaultAgeSetDeviceInboxAndWaitIncRef (const wchar deviceName[], const wchar inboxName[]) {
	DeviceInbox * devInbox = s_ageDeviceInboxes.Find(CHashKeyStr(deviceName));
	if (devInbox) {
		StrCopy(devInbox->inboxName, inboxName, arrsize(devInbox->inboxName));
	}
	else {
		devInbox = NEWZERO(DeviceInbox)(deviceName, inboxName);
		s_ageDeviceInboxes.Add(devInbox);
	}

	// if we found the inbox or its a global inbox then return here, otherwise if its the default inbox and
	// it wasn't found then continue on and create the inbox
	RelVaultNode * existing = VaultAgeGetDeviceInboxIncRef(deviceName);
	if (existing || StrCmp(inboxName, DEFAULT_DEVICE_INBOX) != 0)
		return existing;

	RelVaultNode * device = nil;
	RelVaultNode * inbox  = nil;
	
	for (;;) {
		device = VaultAgeGetDeviceIncRef(deviceName);
		if (!device)
			break;

		ENetError result;
		inbox = VaultCreateNodeAndWaitIncRef(plVault::kNodeType_Folder, &result);
		if (!inbox)
			break;
			
		VaultFolderNode access(inbox);
		access.SetFolderName(inboxName);
		access.SetFolderType(plVault::kDeviceInboxFolder);
					
		VaultAddChildNodeAndWait(device->nodeId, inbox->nodeId, 0);
		break;
	}

	return inbox;	 		
}

//============================================================================
RelVaultNode * VaultAgeGetDeviceInboxIncRef (const wchar deviceName[]) {
	RelVaultNode * result = nil;
	DeviceInbox * devInbox = s_ageDeviceInboxes.Find(CHashKeyStr(deviceName));

	if (devInbox)
	{
		RelVaultNode * parentNode = nil;
		const wchar * inboxName = nil;

		if (StrCmp(devInbox->inboxName, DEFAULT_DEVICE_INBOX) == 0) {
			parentNode = VaultAgeGetDeviceIncRef(deviceName);
		}
		else {
			parentNode = VaultGetGlobalInboxIncRef();
		}

		if (parentNode) {
			NetVaultNode * templateNode = NEWZERO(NetVaultNode);
			templateNode->IncRef();
			templateNode->SetNodeType(plVault::kNodeType_Folder);
			VaultFolderNode access(templateNode);
			access.SetFolderType(plVault::kDeviceInboxFolder);
			access.SetFolderName(devInbox->inboxName);
			result = parentNode->GetChildNodeIncRef(templateNode, 1);
			templateNode->DecRef();
			parentNode->DecRef();
		}
	}
	return result;
}

//============================================================================
void VaultClearDeviceInboxMap () {
	while (DeviceInbox * inbox = s_ageDeviceInboxes.Head()) {
		DEL(inbox);
	}
}

//============================================================================
bool VaultAgeGetAgeSDL (plStateDataRecord * out) {
	bool result = false;
	if (RelVaultNode * rvn = VaultGetAgeInfoNodeIncRef()) {
		if (RelVaultNode * rvnSdl = rvn->GetChildNodeIncRef(plVault::kNodeType_SDL, 1)) {
			VaultSDLNode sdl(rvnSdl);
			result = sdl.GetStateDataRecord(out, plSDL::kKeepDirty);
			if (!result) {
				sdl.InitStateDataRecord(sdl.sdlName);
				result = sdl.GetStateDataRecord(out, plSDL::kKeepDirty);
			}
			rvnSdl->DecRef();
		}
		rvn->DecRef();
	}
	return result;
}

//============================================================================
void VaultAgeUpdateAgeSDL (const plStateDataRecord * rec) {
	if (RelVaultNode * rvn = VaultGetAgeInfoNodeIncRef()) {
		if (RelVaultNode * rvnSdl = rvn->GetChildNodeIncRef(plVault::kNodeType_SDL, 1)) {
			VaultSDLNode sdl(rvnSdl);
			sdl.SetStateDataRecord(rec, plSDL::kDirtyOnly | plSDL::kTimeStampOnRead);
			rvnSdl->DecRef();
		}
		rvn->DecRef();
	}
}

//============================================================================
unsigned VaultAgeGetAgeTime () {
	hsAssert(false, "eric, implement me");
	return 0;
}

//============================================================================
RelVaultNode * VaultGetSubAgeLinkIncRef (const plAgeInfoStruct * info) {
	
	RelVaultNode * rvnLink = nil;
	
	if (RelVaultNode * rvnFldr = VaultGetAgeSubAgesFolderIncRef()) {

		NetVaultNode * templateNode = NEWZERO(NetVaultNode);
		templateNode->IncRef();
		templateNode->SetNodeType(plVault::kNodeType_AgeInfo);

		VaultAgeInfoNode ageInfo(templateNode);
		wchar str[MAX_PATH];
		StrToUnicode(str, info->GetAgeFilename(), arrsize(str));
		ageInfo.SetAgeFilename(str);

		if (RelVaultNode * rvnInfo = rvnFldr->GetChildNodeIncRef(templateNode, 2)) {
			templateNode->fieldFlags = 0;
			templateNode->SetNodeType(plVault::kNodeType_AgeLink);	
			rvnLink = rvnInfo->GetParentNodeIncRef(templateNode, 1);
			rvnInfo->DecRef();
		}

		templateNode->DecRef();
		rvnFldr->DecRef();
	}
	
	return rvnLink;
}

//============================================================================
bool VaultAgeGetSubAgeLink (const plAgeInfoStruct * info, plAgeLinkStruct * link) {
	bool result = false;
	if (RelVaultNode * rvnLink = VaultGetSubAgeLinkIncRef(info)) {
		if (RelVaultNode * rvnInfo = rvnLink->GetChildNodeIncRef(plVault::kNodeType_AgeInfo, 1)) {
			VaultAgeInfoNode ageInfo(rvnInfo);
			ageInfo.CopyTo(link->GetAgeInfo());
			rvnInfo->DecRef();
			result = true;
		}

		rvnLink->DecRef();
	}
	
	return result;
}

//============================================================================
namespace _VaultCreateSubAgeAndWait {

struct _InitAgeParam {
	ENetError		result;
	bool			complete;
	unsigned		ageInfoId;
};
static void _InitAgeCallback (
	ENetError		result,
	void *			,
	void *			vparam,
	unsigned		ageVaultId,
	unsigned		ageInfoVaultId
) {
	_InitAgeParam * param = (_InitAgeParam *)vparam;
	param->ageInfoId	= ageInfoVaultId;
	param->result		= result;
	param->complete		= true;
}
struct _FetchVaultParam {
	ENetError		result;
	bool			complete;
};
static void _FetchVaultCallback (
	ENetError		result,
	void *			vparam
) {
	_FetchVaultParam * param = (_FetchVaultParam *)vparam;
	param->result		= result;
	param->complete		= true;
}
struct _CreateNodeParam {
	ENetError		result;
	bool			complete;
	unsigned		nodeId;
};
static void _CreateNodeCallback (
	ENetError		result,
	void *			,
	void *			vparam,
	RelVaultNode *	node
) {
	_CreateNodeParam * param = (_CreateNodeParam *)vparam;
	if (IS_NET_SUCCESS(result))
		param->nodeId = node->nodeId;
	param->result		= result;
	param->complete		= true;
}
struct _AddChildNodeParam {
	ENetError		result;
	bool			complete;
};
static void _AddChildNodeCallback (
	ENetError		result,
	void *			vparam
) {
	_AddChildNodeParam * param = (_AddChildNodeParam *)vparam;
	param->result		= result;
	param->complete		= true;
}

} // namespace _VaultCreateSubAgeAndWait

//============================================================================
bool VaultAgeFindOrCreateSubAgeLinkAndWait (
	const plAgeInfoStruct * info,
	plAgeLinkStruct *		link,
	const Uuid &			parentAgeInstId
) {
	if (RelVaultNode * rvnLink = VaultFindAgeSubAgeLinkIncRef(info)) {
		VaultAgeLinkNode linkAcc(rvnLink);
		linkAcc.CopyTo(link);
		if (RelVaultNode * rvnInfo = rvnLink->GetChildNodeIncRef(plVault::kNodeType_AgeInfo, 1)) {
			VaultAgeInfoNode infoAcc(rvnInfo);
			infoAcc.CopyTo(link->GetAgeInfo());
			rvnInfo->DecRef();
			rvnLink->DecRef();
			return true;
		}
	}

	using namespace _VaultCreateSubAgeAndWait;

	unsigned subAgesId;
	unsigned ageInfoId;
	unsigned ageLinkId;
	
	if (RelVaultNode * rvnSubAges = VaultGetAgeSubAgesFolderIncRef()) {
		subAgesId = rvnSubAges->nodeId;
		rvnSubAges->DecRef();
	}
	else {
		LogMsg(kLogError, L"CreateSubAge: Failed to get ages's SubAges folder");
		return false;
	}
	
	{	// Init age vault
		_InitAgeParam	param;
		ZERO(param);
		
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
			LogMsg(kLogError, L"CreateSubAge: Failed to init age %S", link->GetAgeInfo()->GetAgeFilename());
			return false;
		}
			
		ageInfoId = param.ageInfoId;
	}		
	
	{	// Create age link
		_CreateNodeParam	param;
		ZERO(param);

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

	{	// Fetch age info node tree
		_FetchVaultParam	param;
		ZERO(param);
		
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
		_AddChildNodeParam	param1;
		_AddChildNodeParam	param2;
		ZERO(param1);
		ZERO(param2);

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
		
	if (RelVaultNode * rvnLink = VaultGetNodeIncRef(ageLinkId)) {
		VaultAgeLinkNode linkAcc(rvnLink);
		linkAcc.CopyTo(link);
		rvnLink->DecRef();
	}

	if (RelVaultNode * rvnInfo = VaultGetNodeIncRef(ageInfoId)) {
		VaultAgeInfoNode infoAcc(rvnInfo);
		infoAcc.CopyTo(link->GetAgeInfo());
		rvnInfo->DecRef();
	}

	return true;
}


//============================================================================
namespace _VaultCreateChildAgeAndWait {

struct _InitAgeParam {
	ENetError		result;
	bool			complete;
	unsigned		ageInfoId;
};
static void _InitAgeCallback (
	ENetError		result,
	void *			,
	void *			vparam,
	unsigned		ageVaultId,
	unsigned		ageInfoVaultId
) {
	_InitAgeParam * param = (_InitAgeParam *)vparam;
	param->ageInfoId	= ageInfoVaultId;
	param->result		= result;
	param->complete		= true;
}
struct _FetchVaultParam {
	ENetError		result;
	bool			complete;
};
static void _FetchVaultCallback (
	ENetError		result,
	void *			vparam
) {
	_FetchVaultParam * param = (_FetchVaultParam *)vparam;
	param->result		= result;
	param->complete		= true;
}
struct _CreateNodeParam {
	ENetError		result;
	bool			complete;
	unsigned		nodeId;
};
static void _CreateNodeCallback (
	ENetError		result,
	void *			,
	void *			vparam,
	RelVaultNode *	node
) {
	_CreateNodeParam * param = (_CreateNodeParam *)vparam;
	if (IS_NET_SUCCESS(result))
		param->nodeId = node->nodeId;
	param->result		= result;
	param->complete		= true;
}
struct _AddChildNodeParam {
	ENetError		result;
	bool			complete;
};
static void _AddChildNodeCallback (
	ENetError		result,
	void *			vparam
) {
	_AddChildNodeParam * param = (_AddChildNodeParam *)vparam;
	param->result		= result;
	param->complete		= true;
}

} // namespace _VaultCreateChildAgeAndWait

//============================================================================
bool VaultAgeFindOrCreateChildAgeLinkAndWait (
	const wchar				parentAgeName[],
	const plAgeInfoStruct * info,
	plAgeLinkStruct *		link
) {
	using namespace _VaultCreateChildAgeAndWait;

	unsigned childAgesId;
	unsigned ageInfoId;
	unsigned ageLinkId;

	{	// Get id of child ages folder
		RelVaultNode * rvnAgeInfo = nil;
		if (parentAgeName) {
			char ansi[MAX_PATH];
			StrToAnsi(ansi, parentAgeName, arrsize(ansi));
			plAgeInfoStruct pinfo;
			pinfo.SetAgeFilename(ansi);
			if (RelVaultNode * rvnAgeLink = VaultGetOwnedAgeLinkIncRef(&pinfo)) {
				rvnAgeInfo = rvnAgeLink->GetChildNodeIncRef(plVault::kNodeType_AgeInfo, 1);
				rvnAgeLink->DecRef();
			}
		}
		else {
			rvnAgeInfo = VaultGetAgeInfoNodeIncRef();
		}
		
		if (!rvnAgeInfo) {
			LogMsg(kLogError, L"CreateChildAge: Failed to get ages's AgeInfo node");
			return false;
		}

		RelVaultNode * rvnChildAges;
		if (nil != (rvnChildAges = rvnAgeInfo->GetChildAgeInfoListNodeIncRef(plVault::kChildAgesFolder, 1))) {
			childAgesId = rvnChildAges->nodeId;
		}		
		else {
			rvnAgeInfo->DecRef();
			LogMsg(kLogError, L"CreateChildAge: Failed to get ages's ChildAges folder");
			return false;
		}
		rvnAgeInfo->DecRef();
		
		// Check for existing child age in folder
		RelVaultNode * rvnLink = nil;
		NetVaultNode * templateNode = NEWZERO(NetVaultNode);
		templateNode->IncRef();
		templateNode->SetNodeType(plVault::kNodeType_AgeInfo);

		VaultAgeInfoNode ageInfo(templateNode);
		wchar str[MAX_PATH];
		StrToUnicode(str, info->GetAgeFilename(), arrsize(str));
		ageInfo.SetAgeFilename(str);

		if (RelVaultNode * rvnInfo = rvnChildAges->GetChildNodeIncRef(templateNode, 2)) {
			templateNode->fieldFlags = 0;
			templateNode->SetNodeType(plVault::kNodeType_AgeLink);	
			rvnLink = rvnInfo->GetParentNodeIncRef(templateNode, 1);
			rvnInfo->DecRef();
		}

		templateNode->DecRef();
		rvnChildAges->DecRef();
		
		if (rvnLink) {
			VaultAgeLinkNode access(rvnLink);
			access.CopyTo(link);
			rvnLink->DecRef();
			return true;
		}
	}	

	{	// Init age vault
		_InitAgeParam	param;
		ZERO(param);

		Uuid parentAgeInstId;
		ZERO(parentAgeInstId);
		if (RelVaultNode * rvnAge = VaultGetAgeNodeIncRef()) {
			VaultAgeNode access(rvnAge);
			parentAgeInstId = access.ageInstUuid;
			rvnAge->DecRef();
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
			LogMsg(kLogError, L"CreateChildAge: Failed to init age %S", link->GetAgeInfo()->GetAgeFilename());
			return false;
		}
			
		ageInfoId = param.ageInfoId;
	}		
	
	{	// Create age link
		_CreateNodeParam	param;
		ZERO(param);

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

	{	// Fetch age info node tree
		_FetchVaultParam	param;
		ZERO(param);
		
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
		_AddChildNodeParam	param1;
		_AddChildNodeParam	param2;
		ZERO(param1);
		ZERO(param2);

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
		
	if (RelVaultNode * rvnLink = VaultGetNodeIncRef(ageLinkId)) {
		VaultAgeLinkNode linkAcc(rvnLink);
		linkAcc.CopyTo(link);
		rvnLink->DecRef();
	}

	if (RelVaultNode * rvnInfo = VaultGetNodeIncRef(ageInfoId)) {
		VaultAgeInfoNode infoAcc(rvnInfo);
		infoAcc.CopyTo(link->GetAgeInfo());
		rvnInfo->DecRef();
	}

	return true;
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
	const wchar					tag[],
	unsigned					vaultId,
	FVaultDownloadCallback		callback,
	void *						cbParam,
	FVaultProgressCallback		progressCallback,
	void *						cbProgressParam
) {
	VaultDownloadTrans * trans = NEWZERO(VaultDownloadTrans);
	StrCopy(trans->tag, tag, arrsize(trans->tag));
	trans->callback			= callback;
	trans->cbParam			= cbParam;
	trans->progressCallback	= progressCallback;
	trans->cbProgressParam	= cbProgressParam;
	trans->vaultId			= vaultId;

	NetCliAuthVaultFetchNodeRefs(
		vaultId,
		VaultDownloadTrans::VaultNodeRefsFetched,
		trans
	);
}

//============================================================================
struct _DownloadVaultParam {
	ENetError		result;
	bool			complete;
};
static void _DownloadVaultCallback (
	ENetError		result,
	void *			vparam
) {
	_DownloadVaultParam * param = (_DownloadVaultParam *)vparam;
	param->result		= result;
	param->complete		= true;
}

void VaultDownloadAndWait (
	const wchar					tag[],
	unsigned					vaultId,
	FVaultProgressCallback		progressCallback,
	void *						cbProgressParam
) {
	_DownloadVaultParam	param;
	ZERO(param);
	
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
		LogMsg(kLogDebug, L"Vault: Culling node %u", link->node->nodeId);
		link->node->state->UnlinkFromRelatives();
		DEL(link);
	}

	// Remove all orphaned nodes from the global table
	for (RelVaultNodeLink * next, * link = s_nodes.Head(); link; link = next) {
		next = s_nodes.Next(link);

		if (link->node->nodeType > plVault::kNodeType_VNodeMgrLow && link->node->nodeType < plVault::kNodeType_VNodeMgrHigh)
			continue;

		ARRAY(unsigned)	nodeIds;
		link->node->GetRootIds(&nodeIds);
		bool foundRoot = false;
		for (unsigned i = 0; i < nodeIds.Count(); ++i) {
			RelVaultNodeLink * root = s_nodes.Find(nodeIds[i]);
			if (root && root->node->nodeType > plVault::kNodeType_VNodeMgrLow && root->node->nodeType < plVault::kNodeType_VNodeMgrHigh) {
				foundRoot = true;
				break;
			}
		}
		if (!foundRoot) {
			LogMsg(kLogDebug, L"Vault: Culling node %u", link->node->nodeId);
			link->node->state->UnlinkFromRelatives();
			DEL(link);
		}
	}	
}

/*****************************************************************************
*
*   Exports - Vault global node handling
*
***/

//============================================================================
RelVaultNode * VaultGetSystemNodeIncRef () {
	RelVaultNode * result = nil;
	if (RelVaultNode * player = VaultGetPlayerNodeIncRef()) {
		NetVaultNode * templateNode = NEWZERO(NetVaultNode);
		templateNode->IncRef();
		templateNode->SetNodeType(plVault::kNodeType_System);
		if (RelVaultNode * systemNode = player->GetChildNodeIncRef(templateNode, 1))
			result = systemNode;
		templateNode->DecRef();
		player->DecRef();
	}
	return result;
}

//============================================================================
RelVaultNode * VaultGetGlobalInboxIncRef () {
	RelVaultNode * result = nil;
	if (RelVaultNode * system = VaultGetSystemNodeIncRef()) {
		NetVaultNode * templateNode = NEWZERO(NetVaultNode);
		templateNode->IncRef();
		templateNode->SetNodeType(plVault::kNodeType_Folder);
		VaultFolderNode folder(templateNode);
		folder.SetFolderType(plVault::kGlobalInboxFolder);
		if (RelVaultNode * inbox = system->GetChildNodeIncRef(templateNode, 1))
			result = inbox;
		templateNode->DecRef();
		system->DecRef();
	}
	return result;
}

#endif // def CLIENT
