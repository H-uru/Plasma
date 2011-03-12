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
#ifndef _pyVaultNode_h_
#define _pyVaultNode_h_

//////////////////////////////////////////////////////////////////////
//
// pyVaultNode   - a wrapper class to provide interface to the RelVaultNode
//
//////////////////////////////////////////////////////////////////////
#include "hsTypes.h"
#include "hsStlUtils.h"

#include <python.h>
#include "pyGlueHelpers.h"

struct RelVaultNode;
class plMipmap;
class pyImage;

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
		PyObject *			fCbObject;
		RelVaultNode *		fNode;
		PyObject *			fPyNodeRef;

		pyVaultNodeOperationCallback(PyObject * cbObject);
		~pyVaultNodeOperationCallback();

		void VaultOperationStarted(UInt32 context);
		void VaultOperationComplete(UInt32 context, int resultCode);
		
		void SetNode (RelVaultNode * rvn);
		RelVaultNode * GetNode ();
	};

	RelVaultNode *		fNode;
	mutable char *		fCreateAgeGuid;
	mutable char *		fCreateAgeName;

protected:
	// only for python glue, do NOT call
	pyVaultNode();
	// should only be created from C++ side
	pyVaultNode( RelVaultNode* node );

public:
	virtual ~pyVaultNode();

	// required functions for PyObject interoperability
	PYTHON_EXPOSE_TYPE; // so we can subclass
	PYTHON_CLASS_NEW_FRIEND(ptVaultNode);
	static PyObject *New(RelVaultNode* node);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyVaultNode object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyVaultNode); // converts a PyObject to a pyVaultNode (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	RelVaultNode * GetNode() const;

	// override the equals to operator
	bool operator==(const pyVaultNode &vaultNode) const;
	bool operator!=(const pyVaultNode &vaultNode) const { return !(vaultNode == *this);	}

	// public getters
	UInt32	GetID( void );
	virtual UInt32	GetType( void );
	UInt32	GetPermissions( void );
	UInt32	GetOwnerNodeID( void );
	PyObject* GetOwnerNode( void ); // returns pyVaultPlayerInfoNode
	UInt32	GetGroupNodeID( void );
	PyObject* GetGroupNode( void ); // returns pyVaultNode
	UInt32 GetModifyTime( void );
	UInt32 GetCreatorNodeID( void );
	PyObject* GetCreatorNode( void ); // returns pyVaultPlayerInfoNode
	UInt32 GetCreateTime( void );
	UInt32 GetCreateAgeTime( void );
	const char * GetCreateAgeName( void );
	const char * GetCreateAgeGuid( void );
	PyObject* GetCreateAgeCoords ();

	// public setters
	void SetID( UInt32 v );
	void SetType( int v );
	void SetOwnerNodeID( UInt32 v );
	void SetCreatorNodeID( UInt32 v );
	void SetCreateAgeName( const char * v );
	void SetCreateAgeGuid( const char * v );


	/////////////////////////////////////////////////
	// Vault Node API

	// Add child node
	PyObject* AddNode(pyVaultNode* pynode, PyObject* cbObject=nil, UInt32 cbContext=0 );
	// Link node to this one
	void LinkToNode(int nodeID, PyObject* cbObject=nil, UInt32 cbContext=0 );
	// Remove child node
	hsBool RemoveNode( pyVaultNode& pynode, PyObject* cbObject=nil, UInt32 cbContext=0 );
	// Remove all child nodes
	void RemoveAllNodes( void );
	// Add/Save this node to vault
	void Save( PyObject* cbObject=nil, UInt32 cbContext=0 );
	// Save this node and all child nodes that need saving.
	// NOTE: Currently, the cb object is called back for
	// each node saved.
	void SaveAll( PyObject* cbObject=nil, UInt32 cbContext=0 );
	// Force a save on this node because currently Save doesn't do anything because dirty
	// nodes are periodically saved automatically - call this to force a save immediately
	void ForceSave();
	// Send this node to the destination client node. will be received in it's inbox folder.
	void SendTo(UInt32 destClientNodeID, PyObject* cbObject=nil, UInt32 cbContext=0 );
	// Returns true if is a child node of ours.
	bool HasNode( UInt32 nodeID );
	//  Returns a ptVaultNodeRef or nil
	PyObject* GetNode2( UInt32 nodeID ) const;			// returns pyVaultNodeRef, for legacy compatibility
	// Get child node matching template node
	PyObject* FindNode( pyVaultNode * templateNode );	// returns pyVaultNode
	
	PyObject * GetChildNode (unsigned nodeId);	// returns pyVaultNode, or None

	// Get all child nodes.
	virtual PyObject* GetChildNodeRefList(); // for legacy compatibility
	virtual int GetChildNodeCount();

	// Get the client ID from my Vault client.
	UInt32	GetClientID( void );

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
