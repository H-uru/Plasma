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
//////////////////////////////////////////////////////////////////////
//
// pyVaultNodeRef   - a wrapper class to provide interface to the plVaultNodeRef
//
//////////////////////////////////////////////////////////////////////

#include "pyVaultNodeRef.h"
#ifndef BUILDING_PYPLASMA
#include "pyVault.h"
#endif
#include "pyVaultNode.h"
#include "pyVaultPlayerInfoNode.h"

#include "../plVault/plVault.h"


//////////////////////////////////////////////////////////////////////


// should only be created from C++ side
pyVaultNodeRef::pyVaultNodeRef(RelVaultNode * parent, RelVaultNode * child)
: fParent(parent)
, fChild(child)
{
	fParent->IncRef();
	fChild->IncRef();
}

pyVaultNodeRef::pyVaultNodeRef(int)
: fParent(nil)
, fChild(nil)
{
}

pyVaultNodeRef::~pyVaultNodeRef()
{
	if (fParent)
		fParent->DecRef();
	if (fChild)
		fChild->DecRef();
}


///////////////////////////////////////////////////////////////////////////

PyObject* pyVaultNodeRef::GetParent ( void )
{
	return pyVaultNode::New(fParent);
}

PyObject* pyVaultNodeRef::GetChild( void )
{
	return pyVaultNode::New(fChild);
}

unsigned pyVaultNodeRef::GetParentID () {
	if (!fParent)
		return 0;
	return fParent->nodeId;
}

unsigned pyVaultNodeRef::GetChildID () {
	if (!fChild)
		return 0;
	return fChild->nodeId;
}

unsigned pyVaultNodeRef::GetSaverID () {
	if (!fParent || !fChild)
		return 0;

	unsigned saverId = 0;		
	if (RelVaultNode * child = VaultGetNodeIncRef(fChild->nodeId)) {
		saverId = child->GetRefOwnerId(fParent->nodeId);
		child->DecRef();
	}
	return saverId;
}

PyObject * pyVaultNodeRef::GetSaver () {
	if (!fParent || !fChild)
		return 0;

	RelVaultNode * saver = nil;		
	if (RelVaultNode * child = VaultGetNodeIncRef(fChild->nodeId)) {
		if (unsigned saverId = child->GetRefOwnerId(fParent->nodeId)) {
			// Find the player info node representing the saver
			NetVaultNode * templateNode = NEWZERO(NetVaultNode);
			templateNode->IncRef();
			templateNode->SetNodeType(plVault::kNodeType_PlayerInfo);
			VaultPlayerInfoNode access(templateNode);
			access.SetPlayerId(saverId);
			saver = VaultGetNodeIncRef(templateNode);

			if (!saver) {
				ARRAY(unsigned) nodeIds;
				VaultFindNodesAndWait(templateNode, &nodeIds);
				if (nodeIds.Count() > 0) {
					VaultFetchNodesAndWait(nodeIds.Ptr(), nodeIds.Count());
					saver = VaultGetNodeIncRef(nodeIds[0]);
				}
			}

			templateNode->DecRef();
		}
		child->DecRef();
	}
	if (!saver)
		PYTHON_RETURN_NONE;
		
	PyObject * result = pyVaultPlayerInfoNode::New(saver);
	saver->DecRef();
	return result;
}

bool pyVaultNodeRef::BeenSeen () {
	return false;
}

void pyVaultNodeRef::SetSeen (bool v) {
	if (!fParent || !fChild)
		return;
		
	fParent->SetSeen(fChild->nodeId, v);
}
