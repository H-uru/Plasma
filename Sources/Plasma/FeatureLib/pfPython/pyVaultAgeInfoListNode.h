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
#ifndef _pyVaultAgeInfoListNode_h_
#define _pyVaultAgeInfoListNode_h_

//////////////////////////////////////////////////////////////////////
//
// pyVaultAgeInfoListNode   - a wrapper class to provide interface to the RelVaultNode
//
//////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "hsStlUtils.h"

#include <python.h>
#include "pyGlueHelpers.h"

#include "pyVaultFolderNode.h"

struct RelVaultNode;

	
class pyVaultAgeInfoListNode : public pyVaultFolderNode
{
protected:
	// should only be created from C++ side
	pyVaultAgeInfoListNode(RelVaultNode* nfsNode);

	// python-side ctor
	pyVaultAgeInfoListNode(int n=0);
public:
	
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptVaultAgeInfoListNode);
	static PyObject *New(RelVaultNode* nfsNode);
	static PyObject *New(int n=0);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyVaultAgeInfoListNode object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyVaultAgeInfoListNode); // converts a PyObject to a pyVaultAgeInfoListNode (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

//==================================================================
// class RelVaultNode : public plVaultFolderNode
//
	hsBool	HasAge( UInt32 ageID );
	hsBool	AddAge( UInt32 ageID );
	void	RemoveAge( UInt32 ageID );
};

#endif // _pyVaultAgeInfoListNode_h_
