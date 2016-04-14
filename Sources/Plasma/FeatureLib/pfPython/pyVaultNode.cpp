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
#include <exception>
#pragma hdrstop

#include "pyVaultNode.h"

#ifndef BUILDING_PYPLASMA
#   include "pyVault.h"
#   include "pyVaultSystemNode.h"
#   include "pnNetCommon/plNetApp.h"
#   include "plNetClientComm/plNetClientComm.h"
#endif

#include "pyImage.h"
#include "pyDniCoordinates.h"
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
#include "plGImage/plMipmap.h"
#include "plVault/plVault.h"


///////////////////////////////////////////////////////////////////////////

static void CDECL LogDumpProc (
    void *              ,
    const wchar_t       *fmt,
    ...
) {
    va_list args;
    va_start(args, fmt);
    LogMsgV(kLogDebug, fmt, args);
    va_end(args);
}

pyVaultNode::pyVaultNodeOperationCallback::pyVaultNodeOperationCallback(PyObject * cbObject)
: fCbObject( cbObject )
, fPyNodeRef(nil)
, fContext(0)
{
    Py_XINCREF( fCbObject );
}

pyVaultNode::pyVaultNodeOperationCallback::~pyVaultNodeOperationCallback()
{
    Py_XDECREF( fCbObject );
}

void pyVaultNode::pyVaultNodeOperationCallback::VaultOperationStarted( uint32_t context )
{
    fContext = context;
    if ( fCbObject )
    {
        // Call the callback.
        if (PyObject_HasAttrString( fCbObject, "vaultOperationStarted" ))
        {
            PyObject* func = nil;
            func = PyObject_GetAttrString( fCbObject, "vaultOperationStarted" );
            if ( func )
            {
                if ( PyCallable_Check(func)>0 )
                {
                    PyObject* retVal = PyObject_CallMethod(fCbObject, "vaultOperationStarted", "l", context);
                    Py_XDECREF(retVal);
                }
            }
        }
    }
}


void pyVaultNode::pyVaultNodeOperationCallback::VaultOperationComplete( uint32_t context, int resultCode )
{
    if ( fCbObject )
    {
        // Call the callback.
        if (PyObject_HasAttrString( fCbObject, "vaultOperationComplete" ))
        {
            PyObject* func = nil;
            func = PyObject_GetAttrString( fCbObject, "vaultOperationComplete" );
            if ( func )
            {
                if ( PyCallable_Check(func)>0 )
                {
                    PyObject * pyNode = pyVaultNode::New(fNode);
                    PyObject* t = PyTuple_New(2);
                    PyTuple_SetItem(t, 0, pyNode);
                    PyTuple_SetItem(t, 1, fPyNodeRef);
                    PyObject* retVal = PyObject_CallMethod(fCbObject, "vaultOperationComplete", "lOi", context, t, resultCode);
                    Py_XDECREF(retVal);
                    Py_DECREF(t);
                }
            }
        }
    }

    delete this;  // commit hara-kiri
}

void pyVaultNode::pyVaultNodeOperationCallback::SetNode (RelVaultNode * rvn) {
    fNode = rvn;
}

hsRef<RelVaultNode> pyVaultNode::pyVaultNodeOperationCallback::GetNode() const {
    return fNode;
}

pyVaultNode::pyVaultNode()
{
}

// should only be created from C++ side
pyVaultNode::pyVaultNode( RelVaultNode* nfsNode )
:   fNode(nfsNode)
{
}

pyVaultNode::~pyVaultNode() {}


hsRef<RelVaultNode> pyVaultNode::GetNode() const
{
    return fNode;
}


// override the equals to operator
bool pyVaultNode::operator==(const pyVaultNode &vaultNode) const
{
    hsRef<RelVaultNode> ours = GetNode();
    hsRef<RelVaultNode> theirs = vaultNode.GetNode();
    if (ours == nil && theirs == nil)
        return true;
    if (ours == nil || theirs == nil)
        return false;
    if (ours->GetNodeId() == theirs->GetNodeId())
        return true;
    return ours->Matches(theirs);
}

// public getters
uint32_t  pyVaultNode::GetID( void )
{
    if (fNode)
        return fNode->GetNodeId();
    return 0;
}

uint32_t  pyVaultNode::GetType( void )
{
    if (fNode)
        return fNode->GetNodeType();
    return 0;
}

uint32_t  pyVaultNode::GetOwnerNodeID( void )
{
    hsAssert(false, "eric, port?");
//  if (fNode)
//      return fNode->ownerId;
    return 0;
}

PyObject* pyVaultNode::GetOwnerNode( void )
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

uint32_t pyVaultNode::GetModifyTime( void )
{
    if (fNode)
        return fNode->GetModifyTime();
    return 0;
}

uint32_t pyVaultNode::GetCreatorNodeID( void )
{
    if (fNode)
        return fNode->GetCreatorId();
    return 0;
}

PyObject* pyVaultNode::GetCreatorNode( void )
{
    PyObject * result = nil;
    if (fNode)
    {
        hsRef<RelVaultNode> templateNode = new RelVaultNode;
        templateNode->SetNodeType(plVault::kNodeType_PlayerInfo);
        VaultPlayerInfoNode plrInfo(templateNode);
        plrInfo.SetPlayerId(fNode->GetCreatorId());
        
        if (hsRef<RelVaultNode> rvn = VaultGetNode(templateNode))
            result = pyVaultPlayerInfoNode::New(rvn);
    }
    
    if (result)
        return result;
        
    // just return a None object
    PYTHON_RETURN_NONE;
}

uint32_t pyVaultNode::GetCreateTime( void )
{
    if (fNode)
        return fNode->GetCreateTime();
    return 0;
}

uint32_t pyVaultNode::GetCreateAgeTime( void )
{
    hsAssert(false, "eric, port?");

    // for now, just return the earth-time the node was created
    return GetCreateTime();
}

ST::string pyVaultNode::GetCreateAgeName() const
{
    if (fNode)
        return fNode->GetCreateAgeName();
    return ST::null;
}

plUUID pyVaultNode::GetCreateAgeGuid(void) const
{
    if (fNode) {
        return fNode->GetCreateAgeUuid();
    }

    return kNilUuid;
}

PyObject* pyVaultNode::GetCreateAgeCoords () {
    if (!fNode)
        PYTHON_RETURN_NONE;

    return pyDniCoordinates::New(nil);
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

void pyVaultNode::SetCreateAgeName( const char * v )
{
    fNode->SetCreateAgeName(v);
}

void pyVaultNode::SetCreateAgeGuid( const char * v )
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
    if (IS_NET_SUCCESS(result))
        cb->VaultOperationComplete(hsOK);
    else
        cb->VaultOperationComplete(hsFail);
}

PyObject* pyVaultNode::AddNode(pyVaultNode* pynode, PyObject* cbObject, uint32_t cbContext)
{
    pyVaultNodeOperationCallback * cb = new pyVaultNodeOperationCallback(cbObject);

    if ( fNode && pynode && pynode->GetNode() )
    {
        // Hack the callbacks until vault notification is in place
        cb->VaultOperationStarted(cbContext);

        int hsResult = hsOK;
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
            else
                hsResult = hsFail;
        }

        PyObject* nodeRef = cb->fPyNodeRef = pyVaultNodeRef::New(fNode, pynode->fNode);
        Py_INCREF(nodeRef); // The callback steals the ref, according to Eric...
        cb->SetNode(pynode->fNode);

        VaultAddChildNode(fNode->GetNodeId(),
                          pynode->fNode->GetNodeId(),
                          NetCommGetPlayer()->playerInt,
                          (FVaultAddChildNodeCallback)_AddNodeCallback,
                          cb
        );

        // Evil undocumented functionality that some fool
        // decided to use in xKI.py. Really???
        return nodeRef;
    }
    else
    {
        // manually make the callback
        cb->VaultOperationStarted( cbContext );
        cb->VaultOperationComplete(hsFail);
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
        cb->VaultOperationComplete( cbContext, hsFail );
    }
}

// Remove child node
bool pyVaultNode::RemoveNode( pyVaultNode& pynode, PyObject* cbObject, uint32_t cbContext )
{
    pyVaultNodeOperationCallback * cb = new pyVaultNodeOperationCallback( cbObject );

    if (fNode && pynode.fNode)
    {
        // Hack the callbacks until vault notification is in place
        cb->VaultOperationStarted( cbContext );

        VaultRemoveChildNode(fNode->GetNodeId(), pynode.fNode->GetNodeId(), nil, nil);

        cb->SetNode(pynode.fNode);
        cb->fPyNodeRef = pyVaultNodeRef::New(fNode, pynode.fNode);
        cb->VaultOperationComplete( cbContext, hsOK );
        return true;
    }
    else
    {
        // manually make the callback
        cb->VaultOperationStarted( cbContext );
        cb->VaultOperationComplete( cbContext, hsFail );
    }

    return false;
}

// Remove all child nodes
void pyVaultNode::RemoveAllNodes( void )
{
    if (!fNode)
        return;
        
    ARRAY(unsigned) nodeIds;
    fNode->GetChildNodeIds(&nodeIds, 1);
    for (unsigned i = 0; i < nodeIds.Count(); ++i)
        VaultRemoveChildNode(fNode->GetNodeId(), nodeIds[i], nil, nil);
}

// Add/Save this node to vault
void pyVaultNode::Save(PyObject* cbObject, uint32_t cbContext)
{
    // If the node doesn't have an id, then use it as a template to create the node in the vault,
    // otherwise just ignore the save request since vault nodes are now auto-saved.
    if (!fNode->GetNodeId() && fNode->GetNodeType()) {
        ENetError result;
        if (hsRef<RelVaultNode> node = VaultCreateNodeAndWait(fNode, &result))
            fNode = node;
    }
    pyVaultNodeOperationCallback * cb = new pyVaultNodeOperationCallback( cbObject );
    cb->SetNode(fNode);
    cb->VaultOperationStarted( cbContext );
    cb->VaultOperationComplete( cbContext, hsOK );
}

// Save this node and all child nodes that need saving.
void pyVaultNode::SaveAll(PyObject* cbObject, uint32_t cbContext)
{
    // Nodes are now auto-saved
    pyVaultNodeOperationCallback * cb = new pyVaultNodeOperationCallback( cbObject );
    cb->VaultOperationStarted( cbContext );
    cb->VaultOperationComplete( cbContext, hsOK );
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
        if (!fNode->GetNodeId() && fNode->GetNodeType()) {
            ENetError result;
            if (hsRef<RelVaultNode> node = VaultCreateNodeAndWait(fNode, &result))
                fNode = node;
        }   

        // Hack the callbacks until vault notification is in place
        cb->VaultOperationStarted( cbContext );

        VaultSendNode(fNode, destClientNodeID);

        cb->VaultOperationComplete( cbContext, hsOK );
    }
    else
    {
        // manually make the callback
        cb->VaultOperationStarted( cbContext );
        cb->VaultOperationComplete( cbContext, hsFail );
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
        
    ARRAY(unsigned) nodeIds;
    fNode->GetChildNodeIds(&nodeIds, 1);
    
    return nodeIds.Count();
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
    PyObject * result = nil;
    if ( fNode )
    {
        hsRef<RelVaultNode> templateNode = new RelVaultNode;
        templateNode->SetNodeId(nodeID);
        if (hsRef<RelVaultNode> rvn = fNode->GetChildNode(templateNode, 1))
            result = pyVaultNodeRef::New(fNode, rvn);
    }
    
    if (result)
        return result;

    PYTHON_RETURN_NONE;
}

PyObject* pyVaultNode::FindNode( pyVaultNode * templateNode )
{
    PyObject * result = nil;
    if ( fNode && templateNode->fNode )
    {
        if (hsRef<RelVaultNode> rvn = fNode->GetChildNode(templateNode->fNode, 1))
            result = pyVaultNode::New(rvn);
    }
    
    if (result)
        return result;

    PYTHON_RETURN_NONE;
}

PyObject * pyVaultNode::GetChildNode (unsigned nodeId) {

    if (!fNode)
        PYTHON_RETURN_NONE;
        
    hsRef<RelVaultNode> templateNode = new RelVaultNode;
    templateNode->SetNodeId(nodeId);
    hsRef<RelVaultNode> rvn = fNode->GetChildNode(templateNode, 1);
    
    if (rvn)
        return pyVaultNode::New(rvn);

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
