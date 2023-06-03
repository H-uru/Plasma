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
//////////////////////////////////////////////////////////////////////
//
// pyVaultNode   - a wrapper class to provide interface to the RelVaultNode
//
//////////////////////////////////////////////////////////////////////

#include <Python.h>
#include <string_theory/string>

#include "pyVaultNode.h"

#ifndef BUILDING_PYPLASMA
#   include "pyVault.h"
#   include "pyVaultSystemNode.h"
#   include "plNetClientComm/plNetClientComm.h"
#endif

#include "pyDniCoordinates.h"
#include "pyObjectRef.h"
#include "plPythonCallable.h"
#include "pyVaultNodeRef.h"
#include "pyVaultFolderNode.h"
#include "pyVaultPlayerInfoListNode.h"
#include "pyVaultImageNode.h"
#include "pyVaultTextNoteNode.h"
#include "pyVaultAgeLinkNode.h"
#include "pyVaultChronicleNode.h"
#include "pyVaultPlayerInfoNode.h"
#include "pyVaultMarkerGameNode.h"
#include "pyVaultAgeInfoNode.h"
#include "pyVaultAgeInfoListNode.h"
#include "pyVaultPlayerNode.h"
#include "pyVaultSDLNode.h"

#include "pnUUID/pnUUID.h"
#include "plVault/plVault.h"


///////////////////////////////////////////////////////////////////////////

pyVaultNode::pyVaultNodeOperationCallback::pyVaultNodeOperationCallback()
    : fContext()
{ }

/** Constructs a new operation callback from a borrowed reference */
pyVaultNode::pyVaultNodeOperationCallback::pyVaultNodeOperationCallback(PyObject* cbObject) noexcept
    : fCbObject(cbObject, pyObjectNewRef), fContext()
{ }

pyVaultNode::pyVaultNodeOperationCallback::pyVaultNodeOperationCallback(pyObjectRef cbObject) noexcept
    : fCbObject(std::move(cbObject)), fContext()
{ }

void pyVaultNode::pyVaultNodeOperationCallback::VaultOperationStarted( uint32_t context )
{
    fContext = context;
    if (fCbObject) {
        // Call the callback.
        if (PyObject_HasAttrString(fCbObject.Get(), "vaultOperationStarted")) {
            pyObjectRef func = PyObject_GetAttrString(fCbObject.Get(), "vaultOperationStarted");
            if (func && PyCallable_Check(func.Get()))
                plPython::CallObject(func, context);
        }
    }
}


void pyVaultNode::pyVaultNodeOperationCallback::VaultOperationComplete(uint32_t context, ENetError result)
{
    if (fCbObject) {
        // Call the callback.
        if (PyObject_HasAttrString(fCbObject.Get(), "vaultOperationComplete")) {
            pyObjectRef func = PyObject_GetAttrString(fCbObject.Get(), "vaultOperationComplete");
            if (func && PyCallable_Check(func.Get())) {
                pyObjectRef tup = PyTuple_New(2);
                PyTuple_SET_ITEM(tup.Get(), 0, pyVaultNode::New(fNode));
                PyTuple_SET_ITEM(tup.Get(), 1, fPyNodeRef.Release());
                plPython::CallObject(func, context, std::move(tup), result);
            }
        }
    }

    delete this;  // commit hara-kiri
}

void pyVaultNode::pyVaultNodeOperationCallback::SetNode(hsRef<RelVaultNode> rvn) {
    fNode = std::move(rvn);
}

hsRef<RelVaultNode> pyVaultNode::pyVaultNodeOperationCallback::GetNode() const {
    return fNode;
}

pyVaultNode::pyVaultNode()
    : fNode(new RelVaultNode, hsStealRef)
{
}

pyVaultNode::pyVaultNode(std::nullptr_t)
{
}

hsRef<RelVaultNode> pyVaultNode::GetNode() const
{
    return fNode;
}

// override the equals to operator
bool pyVaultNode::operator==(const pyVaultNode &vaultNode) const
{
    hsRef<RelVaultNode> ours = GetNode();
    hsRef<RelVaultNode> theirs = vaultNode.GetNode();
    if (ours == nullptr && theirs == nullptr)
        return true;
    if (ours == nullptr || theirs == nullptr)
        return false;
    if (ours->GetNodeId() == theirs->GetNodeId())
        return true;
    return ours->Matches(theirs.Get());
}

// public getters
uint32_t  pyVaultNode::GetID()
{
    if (fNode)
        return fNode->GetNodeId();
    return 0;
}

uint32_t  pyVaultNode::GetType()
{
    if (fNode)
        return fNode->GetNodeType();
    return 0;
}

uint32_t  pyVaultNode::GetOwnerNodeID()
{
    hsAssert(false, "eric, port?");
//  if (fNode)
//      return fNode->ownerId;
    return 0;
}

PyObject* pyVaultNode::GetOwnerNode()
{
    hsAssert(false, "eric, port?");
/*
    if (fNode)
    {
        const plVaultPlayerInfoNode* node = fNode->GetOwnerNode();
        if (node)
            return pyVaultPlayerInfoNode::New((plVaultPlayerInfoNode*)node);
    }
*/
    // just return a None object
    PYTHON_RETURN_NONE;
}

uint32_t pyVaultNode::GetModifyTime()
{
    if (fNode)
        return fNode->GetModifyTime();
    return 0;
}

uint32_t pyVaultNode::GetCreatorNodeID()
{
    if (fNode)
        return fNode->GetCreatorId();
    return 0;
}

PyObject* pyVaultNode::GetCreatorNode()
{
    if (fNode) {
        RelVaultNode templateNode;
        templateNode.SetNodeType(plVault::kNodeType_PlayerInfo);
        VaultPlayerInfoNode plrInfo(&templateNode);
        plrInfo.SetPlayerId(fNode->GetCreatorId());
        
        if (hsRef<RelVaultNode> rvn = VaultGetNode(&templateNode))
            return pyVaultPlayerInfoNode::New(rvn);
    }

    // just return a None object
    PYTHON_RETURN_NONE;
}

uint32_t pyVaultNode::GetCreateTime()
{
    if (fNode)
        return fNode->GetCreateTime();
    return 0;
}

uint32_t pyVaultNode::GetCreateAgeTime()
{
    hsAssert(false, "eric, port?");

    // for now, just return the earth-time the node was created
    return GetCreateTime();
}

ST::string pyVaultNode::GetCreateAgeName() const
{
    if (fNode)
        return fNode->GetCreateAgeName();
    return ST::string();
}

plUUID pyVaultNode::GetCreateAgeGuid() const
{
    if (fNode) {
        return fNode->GetCreateAgeUuid();
    }

    return kNilUuid;
}

PyObject* pyVaultNode::GetCreateAgeCoords () {
    if (!fNode)
        PYTHON_RETURN_NONE;

    return pyDniCoordinates::New(nullptr);
}

void pyVaultNode::SetID( uint32_t v )
{
    ASSERT(fNode);
    hsAssert(fNode->GetNodeId() == 0, "You may not change a node's ID");
    fNode->SetNodeId(v);
}

void pyVaultNode::SetType( int v )
{
    ASSERT(fNode);
    if (fNode->GetNodeId()) {
        FATAL("pyVaultNode: You may not change the type of a vault node");
        return;
    }
    
    fNode->SetNodeType(v);
}

void pyVaultNode::SetOwnerNodeID( uint32_t v )
{
    hsAssert(false, "eric, implement me.");
}

void pyVaultNode::SetCreatorNodeID( uint32_t v )
{
    ASSERT(fNode);
    if (fNode->GetNodeId()) {
        FATAL("pyVaultNode: You may not change the type of a vault node");
        return;
    }
    
    fNode->SetCreatorId(v);
}

void pyVaultNode::SetCreateAgeName(const ST::string& v)
{
    fNode->SetCreateAgeName(v);
}

void pyVaultNode::SetCreateAgeGuid(const ST::string& v)
{
    ASSERT(fNode);
    plUUID uuid(v);
    fNode->SetCreateAgeUuid(uuid);
}


/////////////////////////////////////////////////
// Vault Node API

// Add child node
void _AddNodeCallback(ENetError result, void* param) {
    pyVaultNode::pyVaultNodeOperationCallback* cb = (pyVaultNode::pyVaultNodeOperationCallback*)param;
    cb->VaultOperationComplete(result);
}

PyObject* pyVaultNode::AddNode(pyVaultNode* pynode, PyObject* cbObject, uint32_t cbContext)
{
    pyVaultNodeOperationCallback * cb = new pyVaultNodeOperationCallback(cbObject);

    if ( fNode && pynode && pynode->GetNode() )
    {
        // Hack the callbacks until vault notification is in place
        cb->VaultOperationStarted(cbContext);

        if ( !pynode->GetID() )
        {
            // Block here until node is created and fetched =(
            ASSERT(pynode->GetNode()->GetNodeType());
            ENetError result;
            hsRef<RelVaultNode> newNode = VaultCreateNodeAndWait(
                pynode->GetNode(),
                &result
            );
            
            if (newNode)
                pynode->fNode = newNode;
        }

        pyObjectRef nodeRef = pyVaultNodeRef::New(fNode, pynode->fNode);
        cb->fPyNodeRef = nodeRef;
        cb->SetNode(pynode->fNode);

        VaultAddChildNode(fNode->GetNodeId(),
                          pynode->fNode->GetNodeId(),
                          NetCommGetPlayer()->playerInt,
                          (FVaultAddChildNodeCallback)_AddNodeCallback,
                          cb
        );

        // This return value is undocumented, but we maintain it for backwards compatibility.
        // Be sure to NOT decrement the reference count.
        return nodeRef.Release();
    }
    else
    {
        // manually make the callback
        cb->VaultOperationStarted( cbContext );
        cb->VaultOperationComplete(kNetErrInternalError);
    }

    // just return a None object
    PYTHON_RETURN_NONE;
}

// Link a node to this one
void pyVaultNode::LinkToNode(int nodeID, PyObject* cbObject, uint32_t cbContext)
{
    pyVaultNodeOperationCallback * cb = new pyVaultNodeOperationCallback( cbObject );

    if (fNode && nodeID)
    {
        // Hack the callbacks until vault notification is in place
        cb->VaultOperationStarted( cbContext );
        
        if (hsRef<RelVaultNode> rvn = VaultGetNode(nodeID)) {
            cb->SetNode(rvn);
            cb->fPyNodeRef = pyVaultNodeRef::New(fNode, rvn);
        }

        VaultAddChildNode(fNode->GetNodeId(),
                          nodeID,
                          NetCommGetPlayer()->playerInt,
                          (FVaultAddChildNodeCallback)_AddNodeCallback,
                          cb
        );
    }
    else
    {
        // manually make the callback
        cb->VaultOperationStarted( cbContext );
        cb->VaultOperationComplete(cbContext, kNetErrInternalError);
    }
}

static void _RemoveNodeCallback(ENetError result, void* param)
{
    auto cb = static_cast<pyVaultNode::pyVaultNodeOperationCallback*>(param);
    cb->VaultOperationComplete(result);
}

// Remove child node
bool pyVaultNode::RemoveNode( pyVaultNode& pynode, PyObject* cbObject, uint32_t cbContext )
{
    pyVaultNodeOperationCallback * cb = new pyVaultNodeOperationCallback( cbObject );

    if (fNode && pynode.fNode)
    {
        // Hack the callbacks until vault notification is in place
        cb->VaultOperationStarted( cbContext );
        cb->SetNode(pynode.fNode);
        cb->fPyNodeRef = pyVaultNodeRef::New(fNode, pynode.fNode);

        VaultRemoveChildNode(fNode->GetNodeId(), pynode.fNode->GetNodeId(), _RemoveNodeCallback, cb);

        return true;
    }
    else
    {
        // manually make the callback
        cb->VaultOperationStarted( cbContext );
        cb->VaultOperationComplete(cbContext, kNetErrInternalError);
    }

    return false;
}

// Remove all child nodes
void pyVaultNode::RemoveAllNodes()
{
    if (!fNode)
        return;
        
    std::vector<unsigned> nodeIds;
    fNode->GetChildNodeIds(&nodeIds, 1);
    for (unsigned id : nodeIds)
        VaultRemoveChildNode(fNode->GetNodeId(), id, nullptr, nullptr);
}

// Add/Save this node to vault
void pyVaultNode::Save(PyObject* cbObject, uint32_t cbContext)
{
    // If the node doesn't have an id, then use it as a template to create the node in the vault,
    // otherwise just ignore the save request since vault nodes are now auto-saved.
    ENetError result = kNetPending;
    if (!fNode->GetNodeId() && fNode->GetNodeType()) {
        if (hsRef<RelVaultNode> node = VaultCreateNodeAndWait(fNode, &result))
            fNode = node;
    } else {
        result = kNetSuccess;
    }
    pyVaultNodeOperationCallback * cb = new pyVaultNodeOperationCallback( cbObject );
    cb->SetNode(fNode);
    cb->VaultOperationStarted( cbContext );
    cb->VaultOperationComplete(cbContext, result);
}

// Save this node and all child nodes that need saving.
void pyVaultNode::SaveAll(PyObject* cbObject, uint32_t cbContext)
{
    // Nodes are now auto-saved
    pyVaultNodeOperationCallback * cb = new pyVaultNodeOperationCallback( cbObject );
    cb->VaultOperationStarted( cbContext );
    cb->VaultOperationComplete(cbContext, kNetSuccess);
}

void pyVaultNode::ForceSave()
{
    if (!fNode->GetNodeId() && fNode->GetNodeType()) {
        ENetError result;
        if (hsRef<RelVaultNode> node = VaultCreateNodeAndWait(fNode, &result))
            fNode = node;
    }
    else
        VaultForceSaveNodeAndWait(fNode);
}

// Send this node to the destination client node. will be received in it's inbox folder.
void pyVaultNode::SendTo(uint32_t destClientNodeID, PyObject* cbObject, uint32_t cbContext )
{
    pyVaultNodeOperationCallback * cb = new pyVaultNodeOperationCallback( cbObject );

    if (fNode)
    {
        // If the node doesn't have an id, then use it as a template to create the node in the vault,
        ENetError result = kNetPending;
        if (!fNode->GetNodeId() && fNode->GetNodeType()) {
            if (hsRef<RelVaultNode> node = VaultCreateNodeAndWait(fNode, &result))
                fNode = node;
        } else {
            result = kNetSuccess;
        }

        // Hack the callbacks until vault notification is in place
        cb->VaultOperationStarted( cbContext );

        VaultSendNode(fNode, destClientNodeID);

        cb->VaultOperationComplete(cbContext, result);
    }
    else
    {
        // manually make the callback
        cb->VaultOperationStarted( cbContext );
        cb->VaultOperationComplete(cbContext, kNetErrInternalError);
    }
}

// Get all child nodes, simulating a NodeRef list for legacy compatibility
PyObject* pyVaultNode::GetChildNodeRefList()
{
    // create the list
    PyObject* pyEL = PyList_New(0);

    // fill in the elements list of this folder
    if (fNode)
    {
        RelVaultNode::RefList nodes;
        fNode->GetChildNodes(1, &nodes);
        
        for (const hsRef<RelVaultNode> &node : nodes) {
            PyObject* elementObj = pyVaultNodeRef::New(fNode, node);
            PyList_Append(pyEL, elementObj);
            Py_DECREF(elementObj);
        }
    }

    return pyEL;
}

int pyVaultNode::GetChildNodeCount()
{
    if (!fNode)
        return 0;
        
    std::vector<unsigned> nodeIds;
    fNode->GetChildNodeIds(&nodeIds, 1);
    
    return int(nodeIds.size());
}

// Get the client ID from my Vault client.
uint32_t  pyVaultNode::GetClientID()
{
    hsAssert(false, "eric, port me");
    return 0;
}


bool pyVaultNode::HasNode( uint32_t nodeID )
{
    if ( fNode )
        return fNode->IsParentOf( nodeID, 1 );
    return false;
}

PyObject * pyVaultNode::GetNode2( uint32_t nodeID ) const
{
    PyObject * result = nullptr;
    if ( fNode )
    {
        RelVaultNode templateNode;
        templateNode.SetNodeId(nodeID);
        if (hsRef<RelVaultNode> rvn = fNode->GetChildNode(&templateNode, 1))
            result = pyVaultNodeRef::New(fNode, rvn);
    }
    
    if (result)
        return result;

    PYTHON_RETURN_NONE;
}

PyObject* pyVaultNode::FindNode( pyVaultNode * templateNode, unsigned int maxDepth )
{
    PyObject * result = nullptr;
    if ( fNode && templateNode->fNode )
    {
        hsWeakRef<NetVaultNode> node(templateNode->fNode);
        if (hsRef<RelVaultNode> rvn = fNode->GetChildNode(node, maxDepth))
            result = pyVaultNode::New(rvn);
    }
    
    if (result)
        return result;

    PYTHON_RETURN_NONE;
}


// all the upcasting...

PyObject* pyVaultNode::UpcastToFolderNode()
{
    if (!fNode)
        PYTHON_RETURN_NONE;
    if (
        fNode->GetNodeType() != plVault::kNodeType_Folder &&
        fNode->GetNodeType() != plVault::kNodeType_AgeInfoList &&
        fNode->GetNodeType() != plVault::kNodeType_PlayerInfoList
        )
        PYTHON_RETURN_NONE;

    return pyVaultFolderNode::New(fNode);
}

PyObject* pyVaultNode::UpcastToPlayerInfoListNode()
{
    if (!fNode)
        PYTHON_RETURN_NONE;
    if (fNode->GetNodeType() != plVault::kNodeType_PlayerInfoList)
        PYTHON_RETURN_NONE;

    return pyVaultPlayerInfoListNode::New(fNode);
}

PyObject* pyVaultNode::UpcastToImageNode()
{
    if (!fNode)
        PYTHON_RETURN_NONE;
    if (fNode->GetNodeType() != plVault::kNodeType_Image)
        PYTHON_RETURN_NONE;

    return pyVaultImageNode::New(fNode);
}

PyObject* pyVaultNode::UpcastToTextNoteNode()
{
    if (!fNode)
        PYTHON_RETURN_NONE;
    if (fNode->GetNodeType() != plVault::kNodeType_TextNote)
        PYTHON_RETURN_NONE;

    return pyVaultTextNoteNode::New(fNode);
}

PyObject* pyVaultNode::UpcastToAgeLinkNode()
{
    if (!fNode)
        PYTHON_RETURN_NONE;
    if (fNode->GetNodeType() != plVault::kNodeType_AgeLink)
        PYTHON_RETURN_NONE;

    return pyVaultAgeLinkNode::New(fNode);
}

PyObject* pyVaultNode::UpcastToChronicleNode()
{
    if (!fNode)
        PYTHON_RETURN_NONE;
    if (fNode->GetNodeType() != plVault::kNodeType_Chronicle)
        PYTHON_RETURN_NONE;

    return pyVaultChronicleNode::New(fNode);
}

PyObject* pyVaultNode::UpcastToPlayerInfoNode()
{
    if (!fNode)
        PYTHON_RETURN_NONE;
    if (fNode->GetNodeType() != plVault::kNodeType_PlayerInfo)
        PYTHON_RETURN_NONE;

    return pyVaultPlayerInfoNode::New(fNode);
}

PyObject* pyVaultNode::UpcastToMarkerGameNode()
{
    if (!fNode)
        PYTHON_RETURN_NONE;
    if (fNode->GetNodeType() != plVault::kNodeType_MarkerGame)
        PYTHON_RETURN_NONE;

    return pyVaultMarkerGameNode::New(fNode);
}

PyObject* pyVaultNode::UpcastToAgeInfoNode()
{
    if (!fNode)
        PYTHON_RETURN_NONE;
    if (fNode->GetNodeType() != plVault::kNodeType_AgeInfo)
        PYTHON_RETURN_NONE;

    return pyVaultAgeInfoNode::New(fNode);
}

PyObject* pyVaultNode::UpcastToAgeInfoListNode()
{
    if (!fNode)
        PYTHON_RETURN_NONE;
    if (fNode->GetNodeType() != plVault::kNodeType_AgeInfoList)
        PYTHON_RETURN_NONE;

    return pyVaultAgeInfoListNode::New(fNode);
}

PyObject* pyVaultNode::UpcastToSDLNode()
{
    if (!fNode)
        PYTHON_RETURN_NONE;
    if (fNode->GetNodeType() != plVault::kNodeType_SDL)
        PYTHON_RETURN_NONE;

    return pyVaultSDLNode::New(fNode);
}

PyObject* pyVaultNode::UpcastToPlayerNode()
{
    if (!fNode)
        PYTHON_RETURN_NONE;
    if (fNode->GetNodeType() != plVault::kNodeType_VNodeMgrPlayer)
        PYTHON_RETURN_NONE;

    return pyVaultPlayerNode::New(fNode);
}

#ifndef BUILDING_PYPLASMA
PyObject* pyVaultNode::UpcastToSystemNode()
{
    if (!fNode)
        PYTHON_RETURN_NONE;
    if (fNode->GetNodeType() != plVault::kNodeType_System)
        PYTHON_RETURN_NONE;

    return pyVaultSystemNode::New(fNode);
}
#endif
