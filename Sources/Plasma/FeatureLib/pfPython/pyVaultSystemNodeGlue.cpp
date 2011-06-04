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
#include "pyVaultSystemNode.h"

#include "../plVault/plVault.h"
#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptVaultSystemNode, pyVaultSystemNode);

PYTHON_DEFAULT_NEW_DEFINITION(ptVaultSystemNode, pyVaultSystemNode)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVaultSystemNode)

PYTHON_INIT_DEFINITION(ptVaultSystemNode, args, keywords)
{
	PYTHON_RETURN_INIT_OK;
}

PYTHON_START_METHODS_TABLE(ptVaultSystemNode)
	// no methods...
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptVaultSystemNode, pyVaultNode, "Plasma vault system node");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptVaultSystemNode, pyVaultSystemNode)

PyObject *pyVaultSystemNode::New(RelVaultNode* nfsNode)
{
	ptVaultSystemNode *newObj = (ptVaultSystemNode*)ptVaultSystemNode_type.tp_new(&ptVaultSystemNode_type, NULL, NULL);
	if (newObj->fThis->fNode)
		newObj->fThis->fNode->DecRef();
	newObj->fThis->fNode = nfsNode;
	if (newObj->fThis->fNode)
		newObj->fThis->fNode->IncRef();
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptVaultSystemNode, pyVaultSystemNode)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptVaultSystemNode, pyVaultSystemNode)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyVaultSystemNode::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptVaultSystemNode);
	PYTHON_CLASS_IMPORT_END(m);
}