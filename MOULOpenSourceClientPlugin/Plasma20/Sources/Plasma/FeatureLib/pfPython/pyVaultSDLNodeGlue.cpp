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
#include "pyVaultSDLNode.h"
#include "pySDL.h"

#include "../plVault/plVault.h"
#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptVaultSDLNode, pyVaultSDLNode);

PYTHON_DEFAULT_NEW_DEFINITION(ptVaultSDLNode, pyVaultSDLNode)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVaultSDLNode)

PYTHON_INIT_DEFINITION(ptVaultSDLNode, args, keywords)
{
	PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultSDLNode, getIdent)
{
	return PyInt_FromLong(self->fThis->GetIdent());
}

PYTHON_METHOD_DEFINITION(ptVaultSDLNode, setIdent, args)
{
	int v;
	if (!PyArg_ParseTuple(args, "i", &v))
	{
		PyErr_SetString(PyExc_TypeError, "setIdent expects an integer");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetIdent(v);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVaultSDLNode, initStateDataRecord, args)
{
	char* fileName;
	int flags;
	if (!PyArg_ParseTuple(args, "si", &fileName, &flags))
	{
		PyErr_SetString(PyExc_TypeError, "initStateDataRecord expects a string and an int");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->InitStateDataRecord(fileName, flags);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultSDLNode, getStateDataRecord)
{
	return self->fThis->GetStateDataRecord();
}

PYTHON_METHOD_DEFINITION(ptVaultSDLNode, setStateDataRecord, args)
{
	PyObject* recObj = NULL;
	int writeOptions = 0;
	if (!PyArg_ParseTuple(args, "O|i", &recObj, &writeOptions))
	{
		PyErr_SetString(PyExc_TypeError, "setStateDataRecord expects a ptSDLStateDataRecord and an optional int");
		PYTHON_RETURN_ERROR;
	}
	if (!pySDLStateDataRecord::Check(recObj))
	{
		PyErr_SetString(PyExc_TypeError, "setStateDataRecord expects a ptSDLStateDataRecord and an optional int");
		PYTHON_RETURN_ERROR;
	}
	pySDLStateDataRecord* rec = pySDLStateDataRecord::ConvertFrom(recObj);
	self->fThis->SetStateDataRecord(*rec, writeOptions);
	PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptVaultSDLNode)
	PYTHON_METHOD_NOARGS(ptVaultSDLNode, getIdent, "UNKNOWN"),
	PYTHON_METHOD(ptVaultSDLNode, setIdent, "Params: v\nUNKNOWN"),
	PYTHON_METHOD(ptVaultSDLNode, initStateDataRecord, "Params: filename,flags\nRead the SDL Rec from File if needed"),
	PYTHON_METHOD_NOARGS(ptVaultSDLNode, getStateDataRecord, "Returns the ptSDLStateDataRecord associated with this node"),
	PYTHON_METHOD(ptVaultSDLNode, setStateDataRecord, "Params: rec,writeOptions=0\nSets the ptSDLStateDataRecord"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptVaultSDLNode, pyVaultNode, "Plasma vault SDL node");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptVaultSDLNode, pyVaultSDLNode)

PyObject *pyVaultSDLNode::New(RelVaultNode* nfsNode)
{
	ptVaultSDLNode *newObj = (ptVaultSDLNode*)ptVaultSDLNode_type.tp_new(&ptVaultSDLNode_type, NULL, NULL);
	if (newObj->fThis->fNode)
		newObj->fThis->fNode->DecRef();
	newObj->fThis->fNode = nfsNode;
	if (newObj->fThis->fNode)
		newObj->fThis->fNode->IncRef();
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptVaultSDLNode, pyVaultSDLNode)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptVaultSDLNode, pyVaultSDLNode)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyVaultSDLNode::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptVaultSDLNode);
	PYTHON_CLASS_IMPORT_END(m);
}