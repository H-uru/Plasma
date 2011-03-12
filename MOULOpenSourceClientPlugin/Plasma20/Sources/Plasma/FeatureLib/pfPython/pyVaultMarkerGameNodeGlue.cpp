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
#include "pyVaultMarkerGameNode.h"

#include "../plVault/plVault.h"
#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptVaultMarkerGameNode, pyVaultMarkerGameNode);

PYTHON_DEFAULT_NEW_DEFINITION(ptVaultMarkerGameNode, pyVaultMarkerGameNode)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVaultMarkerGameNode)

PYTHON_INIT_DEFINITION(ptVaultMarkerGameNode, args, keywords)
{
	int n = 0;
	if (!PyArg_ParseTuple(args, "|i", &n))
	{
		PyErr_SetString(PyExc_TypeError, "__init__ expects an optional int");
		PYTHON_RETURN_INIT_ERROR;
	}
	// we don't really do anything? Not according to the associated constructor. Odd...
	PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultMarkerGameNode, getGameName)
{
	return PyString_FromString(self->fThis->GetGameName());
}

PYTHON_METHOD_DEFINITION(ptVaultMarkerGameNode, setGameName, args)
{
	char * name;
	if (!PyArg_ParseTuple(args, "s", &name))
	{
		PyErr_SetString(PyExc_TypeError, "setGameName expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetGameName(name);
	
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultMarkerGameNode, getGameGuid)
{
	return PyString_FromString(self->fThis->GetGameGuid());
}

PYTHON_METHOD_DEFINITION(ptVaultMarkerGameNode, setGameGuid, args)
{
	char * guid;
	if (!PyArg_ParseTuple(args, "s", &guid))
	{
		PyErr_SetString(PyExc_TypeError, "setGameGuid expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetGameGuid(guid);
	PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptVaultMarkerGameNode)
	PYTHON_METHOD_NOARGS(ptVaultMarkerGameNode, getGameName, "Returns the marker game's name"),
	PYTHON_METHOD(ptVaultMarkerGameNode, setGameName, "Params: name\nSets marker game's name"),
	PYTHON_METHOD_NOARGS(ptVaultMarkerGameNode, getGameGuid, "Returns the marker game's guid"),
	PYTHON_METHOD(ptVaultMarkerGameNode, setGameGuid, "Params: guid\nSets the marker game's guid"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptVaultMarkerGameNode, pyVaultNode, "Params: n=0\nPlasma vault age info node");

// required functions for PyObject interoperability
PyObject *pyVaultMarkerGameNode::New(RelVaultNode* nfsNode)
{
	ptVaultMarkerGameNode *newObj = (ptVaultMarkerGameNode*)ptVaultMarkerGameNode_type.tp_new(&ptVaultMarkerGameNode_type, NULL, NULL);
	if (newObj->fThis->fNode)
		newObj->fThis->fNode->DecRef();
	newObj->fThis->fNode = nfsNode;
	if (newObj->fThis->fNode)
		newObj->fThis->fNode->IncRef();
	return (PyObject*)newObj;
}

PyObject *pyVaultMarkerGameNode::New(int n /* =0 */)
{
	ptVaultMarkerGameNode *newObj = (ptVaultMarkerGameNode*)ptVaultMarkerGameNode_type.tp_new(&ptVaultMarkerGameNode_type, NULL, NULL);
	// oddly enough, nothing to do here
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptVaultMarkerGameNode, pyVaultMarkerGameNode)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptVaultMarkerGameNode, pyVaultMarkerGameNode)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyVaultMarkerGameNode::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptVaultMarkerGameNode);
	PYTHON_CLASS_IMPORT_END(m);
}