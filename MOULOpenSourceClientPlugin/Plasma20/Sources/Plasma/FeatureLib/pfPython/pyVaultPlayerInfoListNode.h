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
#ifndef _pyVaultPlayerInfoListNode_h_
#define _pyVaultPlayerInfoListNode_h_

//////////////////////////////////////////////////////////////////////
//
// pyVaultPlayerInfoListNode   - a wrapper class to provide interface to the RelVaultNode
//
//////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "hsStlUtils.h"

#include <python.h>
#include "pyGlueHelpers.h"

#include "pyVaultFolderNode.h"

struct RelVaultNode;
class pyVaultPlayerInfoNode;

	
class pyVaultPlayerInfoListNode : public pyVaultFolderNode
{
protected:
	// should only be created from C++ side
	pyVaultPlayerInfoListNode(RelVaultNode* nfsNode);

	//create from the Python side
	pyVaultPlayerInfoListNode(int n=0);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptVaultPlayerInfoListNode);
	static PyObject *New(RelVaultNode* nfsNode);
	static PyObject *New(int n=0);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyVaultPlayerInfoListNode object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyVaultPlayerInfoListNode); // converts a PyObject to a pyVaultPlayerInfoListNode (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

//==================================================================
// class RelVaultNode : public plVaultFolderNode
//
	virtual hsBool	HasPlayer( UInt32 playerID );
	hsBool	AddPlayer( UInt32 playerID );
	void	RemovePlayer( UInt32 playerID );
	PyObject * GetPlayer( UInt32 playerID ); // returns pyVaultPlayerInfoNode

	void	Sort();

};


#endif // _pyVaultPlayerInfoListNode_h_
