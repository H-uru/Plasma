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
#ifndef _pyVaultNode_h_
#define _pyVaultNode_h_

//////////////////////////////////////////////////////////////////////
//
// pyVaultNode   - a wrapper class to provide interface to the RelVaultNode
//
//////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "pyGlueHelpers.h"
#include "pyObjectRef.h"
#include "hsRefCnt.h"

#include "pnNetBase/pnNbError.h"

struct RelVaultNode;
class plMipmap;
class pyImage;
class plUUID;

class pyDniCoordinates;

class pyVaultNodeRef;
class pyVaultFolderNode;
class pyVaultPlayerInfoListNode;
class pyVaultImageNode;
class pyVaultTextNoteNode;
class pyVaultAgeLinkNode;
class pyVaultChronicleNode;
class pyVaultPlayerInfoNode;
class pyVaultMarkerNode;
class pyVaultAgeInfoNode;
class pyVaultAgeInfoListNode;
class pyVaultSDLNode;
class pyVaultPlayerNode;
class pyVaultMarkerListNode;
#ifndef BUILDING_PYPLASMA
class pyVaultSystemNode;
#endif

class pyVaultNode
{
public:
    struct pyVaultNodeOperationCallback
    {
        pyObjectRef         fCbObject;
        hsRef<RelVaultNode> fNode;
        pyObjectRef         fPyNodeRef;
        uint32_t            fContext;

        pyVaultNodeOperationCallback();

        /** Constructs a new operation callback from a borrowed reference */
        explicit pyVaultNodeOperationCallback(PyObject* cbObject) noexcept;

        explicit pyVaultNodeOperationCallback(pyObjectRef cbObject) noexcept;

        void VaultOperationStarted(uint32_t context);
        void VaultOperationComplete(uint32_t context, ENetError result);
        void VaultOperationComplete(ENetError result) { VaultOperationComplete(fContext, result); }
        
        void SetNode(hsRef<RelVaultNode> rvn);
        hsRef<RelVaultNode> GetNode() const;
    };

    hsRef<RelVaultNode> fNode;

protected:
    pyVaultNode();

    pyVaultNode(std::nullptr_t);

public:
    virtual ~pyVaultNode() = default;

    // required functions for PyObject interoperability
    PYTHON_EXPOSE_TYPE; // so we can subclass
    PYTHON_CLASS_NEW_FRIEND(ptVaultNode);
    PYTHON_CLASS_VAULT_NODE_NEW_DEFINITION;
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyVaultNode object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyVaultNode); // converts a PyObject to a pyVaultNode (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);

    hsRef<RelVaultNode> GetNode() const;

    // override the equals to operator
    bool operator==(const pyVaultNode &vaultNode) const;
    bool operator!=(const pyVaultNode &vaultNode) const { return !(vaultNode == *this); }

    // public getters
    uint32_t  GetID();
    uint32_t GetType();
    uint32_t  GetPermissions();
    uint32_t  GetOwnerNodeID();
    PyObject* GetOwnerNode(); // returns pyVaultPlayerInfoNode
    uint32_t  GetGroupNodeID();
    PyObject* GetGroupNode(); // returns pyVaultNode
    uint32_t GetModifyTime();
    uint32_t GetCreatorNodeID();
    PyObject* GetCreatorNode(); // returns pyVaultPlayerInfoNode
    uint32_t GetCreateTime();
    uint32_t GetCreateAgeTime();
    ST::string GetCreateAgeName() const;
    plUUID    GetCreateAgeGuid() const;
    PyObject* GetCreateAgeCoords ();

    // public setters
    void SetID( uint32_t v );
    void SetType( int v );
    void SetOwnerNodeID( uint32_t v );
    void SetCreatorNodeID( uint32_t v );
    void SetCreateAgeName(const ST::string& v);
    void SetCreateAgeGuid(const ST::string& v);


    /////////////////////////////////////////////////
    // Vault Node API

    // Add child node
    PyObject* AddNode(pyVaultNode* pynode, PyObject* cbObject=nullptr, uint32_t cbContext=0);
    // Link node to this one
    void LinkToNode(int nodeID, PyObject* cbObject=nullptr, uint32_t cbContext=0);
    // Remove child node
    bool RemoveNode(pyVaultNode& pynode, PyObject* cbObject=nullptr, uint32_t cbContext=0);
    // Remove all child nodes
    void RemoveAllNodes();
    // Add/Save this node to vault
    void Save(PyObject* cbObject=nullptr, uint32_t cbContext=0);
    // Save this node and all child nodes that need saving.
    // NOTE: Currently, the cb object is called back for
    // each node saved.
    void SaveAll(PyObject* cbObject=nullptr, uint32_t cbContext=0);
    // Force a save on this node because currently Save doesn't do anything because dirty
    // nodes are periodically saved automatically - call this to force a save immediately
    void ForceSave();
    // Send this node to the destination client node. will be received in it's inbox folder.
    void SendTo(uint32_t destClientNodeID, PyObject* cbObject=nullptr, uint32_t cbContext=0);
    // Returns true if is a child node of ours.
    bool HasNode( uint32_t nodeID );
    //  Returns a ptVaultNodeRef or nullptr
    PyObject* GetNode2( uint32_t nodeID ) const;          // returns pyVaultNodeRef, for legacy compatibility
    // Get child node matching template node
    PyObject* FindNode( pyVaultNode * templateNode, unsigned int maxDepth = 1 );   // returns pyVaultNode

    // Get all child nodes.
    PyObject* GetChildNodeRefList(); // for legacy compatibility
    int GetChildNodeCount();

    // Get the client ID from my Vault client.
    uint32_t  GetClientID();

    // all the upcasting stuff...
    PyObject* UpcastToFolderNode(); // returns pyVaultFolderNode
    PyObject* UpcastToPlayerInfoListNode(); // returns pyVaultPlayerInfoListNode
    PyObject* UpcastToImageNode(); // returns pyVaultImageNode
    PyObject* UpcastToTextNoteNode(); // returns pyVaultTextNoteNode
    PyObject* UpcastToAgeLinkNode(); // returns pyVaultAgeLinkNode
    PyObject* UpcastToChronicleNode(); // returns pyVaultChronicleNode
    PyObject* UpcastToPlayerInfoNode(); // returns pyVaultPlayerInfoNode
    PyObject* UpcastToMarkerGameNode(); // returns pyVaultMarkerNode
    PyObject* UpcastToAgeInfoNode(); // returns pyVaultAgeInfoNode
    PyObject* UpcastToAgeInfoListNode(); // returns pyVaultAgeInfoListNode
    PyObject* UpcastToSDLNode(); // returns pyVaultSDLNode
    PyObject* UpcastToPlayerNode(); // returns pyVaultPlayerNode
#ifndef BUILDING_PYPLASMA
    PyObject* UpcastToSystemNode(); // returns pyVaultSystemNode
#endif

};

#endif // _pyVaultNode_h_
