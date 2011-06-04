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
// pyVaultAgeLinkNode   - a wrapper class to provide interface to the RelVaultNode
//
//////////////////////////////////////////////////////////////////////

#include "pyVaultAgeLinkNode.h"
#include "pyVaultAgeInfoNode.h"
#include "pyNetLinkingMgr.h"
#include "pyAgeLinkStruct.h"
#include "pySpawnPointInfo.h"

#include "../plVault/plVault.h"

#include "../plNetCommon/plSpawnPointInfo.h"

// should only be created from C++ side
pyVaultAgeLinkNode::pyVaultAgeLinkNode(RelVaultNode* nfsNode)
: pyVaultNode(nfsNode)
{
}

//create from the Python side
pyVaultAgeLinkNode::pyVaultAgeLinkNode(int n)
: pyVaultNode(NEWZERO(RelVaultNode))
{
	fNode->SetNodeType(plVault::kNodeType_AgeLink);
}


//==================================================================
// class RelVaultNode : public plVaultNode
//

PyObject* pyVaultAgeLinkNode::GetAgeInfo() const
{
	if (!fNode)
		PYTHON_RETURN_NONE;

	PyObject * result = nil;		
	if (RelVaultNode * rvn = fNode->GetChildNodeIncRef(plVault::kNodeType_AgeInfo, 1)) {
		result = pyVaultAgeInfoNode::New(rvn);
		rvn->DecRef();
	}
	
	if (result)
		return result;
		
	PYTHON_RETURN_NONE;
}


void pyVaultAgeLinkNode::SetLocked( bool v )
{
	if (!fNode)
		return;
		
	VaultAgeLinkNode access(fNode);
	access.SetUnlocked(!v);
}

bool pyVaultAgeLinkNode::GetLocked() const
{
	if (!fNode)
		return false;

	VaultAgeLinkNode access(fNode);
	return !access.unlocked;
}

void pyVaultAgeLinkNode::SetVolatile( bool v )
{
	if (!fNode)
		return;

	VaultAgeLinkNode access(fNode);
	access.SetVolatile(v);
}

bool pyVaultAgeLinkNode::GetVolatile() const
{
	if (!fNode)
		return false;

	VaultAgeLinkNode access(fNode);
	return access.volat;
}

void pyVaultAgeLinkNode::AddSpawnPoint( pySpawnPointInfo & point )
{
	if (!fNode)
		return;

	VaultAgeLinkNode access(fNode);
	access.AddSpawnPoint(point.fInfo);
}

void pyVaultAgeLinkNode::AddSpawnPointRef( pySpawnPointInfoRef & point )
{
	if (!fNode)
		return;

	VaultAgeLinkNode access(fNode);
	access.AddSpawnPoint(point.fInfo);
}

void pyVaultAgeLinkNode::RemoveSpawnPoint( pySpawnPointInfo & point )
{
	if (!fNode)
		return;

	VaultAgeLinkNode access(fNode);
	access.RemoveSpawnPoint(point.GetName());
}

void pyVaultAgeLinkNode::RemoveSpawnPointRef( pySpawnPointInfoRef & point )
{
	if (!fNode)
		return;

	VaultAgeLinkNode access(fNode);
	access.RemoveSpawnPoint(point.GetName());
}

void pyVaultAgeLinkNode::RemoveSpawnPointByName( const char * spawnPtName )
{
	if (!fNode)
		return;

	VaultAgeLinkNode access(fNode);
	access.RemoveSpawnPoint(spawnPtName);
}

bool pyVaultAgeLinkNode::HasSpawnPoint( const char * spawnPtName ) const
{
	if (!fNode)
		return false;

	VaultAgeLinkNode access(fNode);
	return access.HasSpawnPoint(spawnPtName);
}

PyObject * pyVaultAgeLinkNode::GetSpawnPoints() const
{
	PyObject* pyEL = PyList_New(0);

	if (!fNode)
		return pyEL;

	plSpawnPointVec points;
	VaultAgeLinkNode access(fNode);
	access.GetSpawnPoints(&points);
	for (unsigned i = 0; i < points.size(); ++i) {
		PyObject* elementObj = pySpawnPointInfo::New(points[i]);
		PyList_Append(pyEL, elementObj);
		Py_DECREF(elementObj);
	}

	return pyEL;
}

PyObject * pyVaultAgeLinkNode::AsAgeLinkStruct() const
{
	if (!fNode)
		PYTHON_RETURN_NONE;

	VaultAgeLinkNode access(fNode);
	access.CopyTo(&fAgeLinkStruct);
	
	return pyAgeLinkStruct::New(&fAgeLinkStruct);
}
