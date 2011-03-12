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
#include "pyVaultFolderNode.h"

#include "../plVault/plVault.h"
#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptVaultFolderNode, pyVaultFolderNode);

PYTHON_DEFAULT_NEW_DEFINITION(ptVaultFolderNode, pyVaultFolderNode)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVaultFolderNode)

PYTHON_INIT_DEFINITION(ptVaultFolderNode, args, keywords)
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

PYTHON_METHOD_DEFINITION(ptVaultFolderNode, folderSetType, args)
{
	int folderType;
	if (!PyArg_ParseTuple(args, "i", &folderType))
	{
		PyErr_SetString(PyExc_TypeError, "folderSetType expects an int");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Folder_SetType(folderType);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultFolderNode, folderGetType)
{
	return PyInt_FromLong(self->fThis->Folder_GetType());
}

PYTHON_METHOD_DEFINITION(ptVaultFolderNode, folderSetName, args)
{
	char* name;
	if (!PyArg_ParseTuple(args, "s", &name))
	{
		PyErr_SetString(PyExc_TypeError, "folderSetName expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Folder_SetName(name);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultFolderNode, folderGetName)
{
	return PyString_FromString(self->fThis->Folder_GetName().c_str());
}

PYTHON_METHOD_DEFINITION(ptVaultFolderNode, setFolderType, args)
{
	int folderType;
	if (!PyArg_ParseTuple(args, "i", &folderType))
	{
		PyErr_SetString(PyExc_TypeError, "setFolderType expects an int");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Folder_SetType(folderType);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultFolderNode, getFolderType)
{
	return PyInt_FromLong(self->fThis->Folder_GetType());
}

PYTHON_METHOD_DEFINITION(ptVaultFolderNode, setFolderName, args)
{
	char* name;
	if (!PyArg_ParseTuple(args, "s", &name))
	{
		PyErr_SetString(PyExc_TypeError, "setFolderName expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Folder_SetName(name);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVaultFolderNode, setFolderNameW, args)
{
	PyObject* textObj;
	if (!PyArg_ParseTuple(args, "O", &textObj))
	{
		PyErr_SetString(PyExc_TypeError, "setFolderNameW expects a unicode string");
		PYTHON_RETURN_ERROR;
	}
	if (PyUnicode_Check(textObj))
	{
		int strLen = PyUnicode_GetSize(textObj);
		wchar_t* name = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)textObj, name, strLen);
		name[strLen] = L'\0';
		self->fThis->Folder_SetNameW(name);
		delete [] name;
		PYTHON_RETURN_NONE;
	}
	else if (PyString_Check(textObj))
	{
		// we'll allow this, just in case something goes weird
		char* name = PyString_AsString(textObj);
		self->fThis->Folder_SetName(name);
		PYTHON_RETURN_NONE;
	}
	PyErr_SetString(PyExc_TypeError, "setFolderNameW expects a unicode string");
	PYTHON_RETURN_ERROR;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultFolderNode, getFolderName)
{
	return PyString_FromString(self->fThis->Folder_GetName().c_str());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultFolderNode, getFolderNameW)
{
	std::wstring name = self->fThis->Folder_GetNameW();
	return PyUnicode_FromWideChar(name.c_str(), name.length());
}

PYTHON_START_METHODS_TABLE(ptVaultFolderNode)
	// legacy glue
	PYTHON_METHOD(ptVaultFolderNode, folderSetType, "Params: type\nLEGACY\nSet the folder type"),
	PYTHON_METHOD_NOARGS(ptVaultFolderNode, folderGetType, "LEGACY\nReturns the folder type (of the standard folder types)"),
	PYTHON_METHOD(ptVaultFolderNode, folderSetName, "Params: name\nLEGACY\nSet the folder name"),
	PYTHON_METHOD_NOARGS(ptVaultFolderNode, folderGetName, "LEGACY\nReturns the folder's name"),
	// new glue
	PYTHON_METHOD(ptVaultFolderNode, setFolderType, "Params: type\nSet the folder type"),
	PYTHON_METHOD_NOARGS(ptVaultFolderNode, getFolderType, "Returns the folder type (of the standard folder types)"),
	PYTHON_METHOD(ptVaultFolderNode, setFolderName, "Params: name\nSet the folder name"),
	PYTHON_METHOD(ptVaultFolderNode, setFolderNameW, "Params: name\nUnicode version of setFolderName"),
	PYTHON_METHOD_NOARGS(ptVaultFolderNode, getFolderName, "Returns the folder's name"),
	PYTHON_METHOD_NOARGS(ptVaultFolderNode, getFolderNameW, "Unicode version of getFolerName"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptVaultFolderNode, pyVaultNode, "Params: n=0\nPlasma vault folder node");
PYTHON_EXPOSE_TYPE_DEFINITION(ptVaultFolderNode, pyVaultFolderNode);

// required functions for PyObject interoperability
PyObject *pyVaultFolderNode::New(RelVaultNode* nfsNode)
{
	ptVaultFolderNode *newObj = (ptVaultFolderNode*)ptVaultFolderNode_type.tp_new(&ptVaultFolderNode_type, NULL, NULL);
	if (newObj->fThis->fNode)
		newObj->fThis->fNode->DecRef();
	newObj->fThis->fNode = nfsNode;
	if (newObj->fThis->fNode)
		newObj->fThis->fNode->IncRef();
	return (PyObject*)newObj;
}

PyObject *pyVaultFolderNode::New(int n /* =0 */)
{
	ptVaultFolderNode *newObj = (ptVaultFolderNode*)ptVaultFolderNode_type.tp_new(&ptVaultFolderNode_type, NULL, NULL);
	// oddly enough, nothing to do here
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptVaultFolderNode, pyVaultFolderNode)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptVaultFolderNode, pyVaultFolderNode)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyVaultFolderNode::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptVaultFolderNode);
	PYTHON_CLASS_IMPORT_END(m);
}