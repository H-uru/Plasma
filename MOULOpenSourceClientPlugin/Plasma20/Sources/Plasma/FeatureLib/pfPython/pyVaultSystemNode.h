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
#ifndef _pyVaultSystemNode_h_
#define _pyVaultSystemNode_h_

//////////////////////////////////////////////////////////////////////
//
// pyVaultSystemNode   - a wrapper class to provide interface to the RelVaultNode
//
//////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "hsStlUtils.h"

#include <python.h>
#include "pyGlueHelpers.h"

#include "pyVaultNode.h"

struct RelVaultNode;

class pyVaultSystemNode : public pyVaultNode
{
protected:
	// should only be created from C++ side
	pyVaultSystemNode(RelVaultNode* nfsNode);

	//create from the Python side
	pyVaultSystemNode();

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptVaultSystemNode);
	PYTHON_CLASS_NEW_DEFINITION;
	static PyObject *New(RelVaultNode* nfsNode);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyVaultSystemNode object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyVaultSystemNode); // converts a PyObject to a pyVaultSystemNode (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

};

#endif // _pyVaultSystemNode_h_
