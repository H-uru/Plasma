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
#include "pyVaultChronicleNode.h"

#include "../plVault/plVault.h"
#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptVaultChronicleNode, pyVaultChronicleNode);

PYTHON_DEFAULT_NEW_DEFINITION(ptVaultChronicleNode, pyVaultChronicleNode)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVaultChronicleNode)

PYTHON_INIT_DEFINITION(ptVaultChronicleNode, args, keywords)
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

PYTHON_METHOD_DEFINITION(ptVaultChronicleNode, chronicleSetName, args)
{
	char* name;
	if (!PyArg_ParseTuple(args, "s", &name))
	{
		PyErr_SetString(PyExc_TypeError, "chronicleSetName expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Chronicle_SetName(name);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultChronicleNode, chronicleGetName)
{
	return PyString_FromString(self->fThis->Chronicle_GetName());
}

PYTHON_METHOD_DEFINITION(ptVaultChronicleNode, chronicleSetValue, args)
{
	char* val;
	if (!PyArg_ParseTuple(args, "s", &val))
	{
		PyErr_SetString(PyExc_TypeError, "chronicleSetValue expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Chronicle_SetValue(val);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultChronicleNode, chronicleGetValue)
{
	return PyString_FromString(self->fThis->Chronicle_GetValue());
}

PYTHON_METHOD_DEFINITION(ptVaultChronicleNode, chronicleSetType, args)
{
	unsigned long chronType;
	if (!PyArg_ParseTuple(args, "l", &chronType))
	{
		PyErr_SetString(PyExc_TypeError, "chronicleSetType expects an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Chronicle_SetType(chronType);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultChronicleNode, chronicleGetType)
{
	return PyLong_FromUnsignedLong(self->fThis->Chronicle_GetType());
}

PYTHON_METHOD_DEFINITION(ptVaultChronicleNode, setName, args)
{
	char* name;
	if (!PyArg_ParseTuple(args, "s", &name))
	{
		PyErr_SetString(PyExc_TypeError, "setName expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Chronicle_SetName(name);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultChronicleNode, getName)
{
	return PyString_FromString(self->fThis->Chronicle_GetName());
}

PYTHON_METHOD_DEFINITION(ptVaultChronicleNode, setValue, args)
{
	char* val;
	if (!PyArg_ParseTuple(args, "s", &val))
	{
		PyErr_SetString(PyExc_TypeError, "setValue expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Chronicle_SetValue(val);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultChronicleNode, getValue)
{
	return PyString_FromString(self->fThis->Chronicle_GetValue());
}

PYTHON_METHOD_DEFINITION(ptVaultChronicleNode, setEntryType, args)
{
	unsigned long chronType;
	if (!PyArg_ParseTuple(args, "l", &chronType))
	{
		PyErr_SetString(PyExc_TypeError, "setEntryType expects an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Chronicle_SetType(chronType);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultChronicleNode, getEntryType)
{
	return PyLong_FromUnsignedLong(self->fThis->Chronicle_GetType());
}

PYTHON_START_METHODS_TABLE(ptVaultChronicleNode)
	// legacy glue
	PYTHON_METHOD(ptVaultChronicleNode, chronicleSetName, "Params: name\nLEGACY: Sets the name of the chronicle node."),
	PYTHON_METHOD_NOARGS(ptVaultChronicleNode, chronicleGetName, "LEGACY: Returns the name of the chronicle node."),
	PYTHON_METHOD(ptVaultChronicleNode, chronicleSetValue, "Params: value\nLEGACY: Sets the chronicle to a value that is a string"),
	PYTHON_METHOD_NOARGS(ptVaultChronicleNode, chronicleGetValue, "LEGACY: Returns the value as a string of this chronicle node."),
	PYTHON_METHOD(ptVaultChronicleNode, chronicleSetType, "Params: type\nLEGACY: Sets this chronicle node to a user defined type."),
	PYTHON_METHOD_NOARGS(ptVaultChronicleNode, chronicleGetType, "LEGACY: Returns the user defined type of the chronicle node."),
	// new glue
	PYTHON_METHOD(ptVaultChronicleNode, setName, "Params: name\nSets the name of the chronicle node."),
	PYTHON_METHOD_NOARGS(ptVaultChronicleNode, getName, "Returns the name of the chronicle node."),
	PYTHON_METHOD(ptVaultChronicleNode, setValue, "Params: value\nSets the chronicle to a value that is a string"),
	PYTHON_METHOD_NOARGS(ptVaultChronicleNode, getValue, "Returns the value as a string of this chronicle node."),
	PYTHON_METHOD(ptVaultChronicleNode, setEntryType, "Params: type\nSets this chronicle node to a user defined type."),
	PYTHON_METHOD_NOARGS(ptVaultChronicleNode, getEntryType, "Returns the user defined type of the chronicle node."),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptVaultChronicleNode, pyVaultNode, "Params: n=0\nPlasma vault chronicle node");

// required functions for PyObject interoperability
PyObject *pyVaultChronicleNode::New(RelVaultNode* nfsNode)
{
	ptVaultChronicleNode *newObj = (ptVaultChronicleNode*)ptVaultChronicleNode_type.tp_new(&ptVaultChronicleNode_type, NULL, NULL);
	if (newObj->fThis->fNode)
		newObj->fThis->fNode->DecRef();
	newObj->fThis->fNode = nfsNode;
	if (newObj->fThis->fNode)
		newObj->fThis->fNode->IncRef();
	return (PyObject*)newObj;
}

PyObject *pyVaultChronicleNode::New(int n /* =0 */)
{
	ptVaultChronicleNode *newObj = (ptVaultChronicleNode*)ptVaultChronicleNode_type.tp_new(&ptVaultChronicleNode_type, NULL, NULL);
	// oddly enough, nothing to do here
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptVaultChronicleNode, pyVaultChronicleNode)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptVaultChronicleNode, pyVaultChronicleNode)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyVaultChronicleNode::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptVaultChronicleNode);
	PYTHON_CLASS_IMPORT_END(m);
}