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
#include "pyVaultAgeInfoNode.h"

#include "../plVault/plVault.h"
#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptVaultAgeInfoNode, pyVaultAgeInfoNode);

PYTHON_DEFAULT_NEW_DEFINITION(ptVaultAgeInfoNode, pyVaultAgeInfoNode)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVaultAgeInfoNode)

PYTHON_INIT_DEFINITION(ptVaultAgeInfoNode, args, keywords)
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

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultAgeInfoNode, getCanVisitFolder)
{
	return self->fThis->GetCanVisitFolder();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultAgeInfoNode, getAgeOwnersFolder)
{
	return self->fThis->GetAgeOwnersFolder();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultAgeInfoNode, getChildAgesFolder)
{
	return self->fThis->GetChildAgesFolder();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultAgeInfoNode, getAgeSDL)
{
	return self->fThis->GetAgeSDL();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultAgeInfoNode, getCzar)
{
	return self->fThis->GetCzar();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultAgeInfoNode, getParentAgeLink)
{
	return self->fThis->GetParentAgeLink();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultAgeInfoNode, getAgeFilename)
{
	return PyString_FromString(self->fThis->GetAgeFilename());
}

PYTHON_METHOD_DEFINITION(ptVaultAgeInfoNode, setAgeFilename, args)
{
	char* filename;
	if (!PyArg_ParseTuple(args, "s", &filename))
	{
		PyErr_SetString(PyExc_TypeError, "setAgeFilename expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetAgeFilename(filename);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultAgeInfoNode, getAgeInstanceName)
{
	return PyString_FromString(self->fThis->GetAgeInstanceName());
}

PYTHON_METHOD_DEFINITION(ptVaultAgeInfoNode, setAgeInstanceName, args)
{
	char* instanceName;
	if (!PyArg_ParseTuple(args, "s", &instanceName))
	{
		PyErr_SetString(PyExc_TypeError, "setAgeInstanceName expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetAgeInstanceName(instanceName);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultAgeInfoNode, getAgeUserDefinedName)
{
	return PyString_FromString(self->fThis->GetAgeUserDefinedName());
}

PYTHON_METHOD_DEFINITION(ptVaultAgeInfoNode, setAgeUserDefinedName, args)
{
	char* userDefName;
	if (!PyArg_ParseTuple(args, "s", &userDefName))
	{
		PyErr_SetString(PyExc_TypeError, "setAgeUserDefinedName expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetAgeUserDefinedName(userDefName);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultAgeInfoNode, getAgeInstanceGuid)
{
	return PyString_FromString(self->fThis->GetAgeInstanceGuid());
}

PYTHON_METHOD_DEFINITION(ptVaultAgeInfoNode, setAgeInstanceGuid, args)
{
	char* guid;
	if (!PyArg_ParseTuple(args, "s", &guid))
	{
		PyErr_SetString(PyExc_TypeError, "setAgeInstanceGuid expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetAgeInstanceGuid(guid);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultAgeInfoNode, getAgeDescription)
{
	return PyString_FromString(self->fThis->GetAgeDescription());
}

PYTHON_METHOD_DEFINITION(ptVaultAgeInfoNode, setAgeDescription, args)
{
	char* descr;
	if (!PyArg_ParseTuple(args, "s", &descr))
	{
		PyErr_SetString(PyExc_TypeError, "setAgeDescription expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetAgeDescription(descr);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultAgeInfoNode, getAgeSequenceNumber)
{
	return PyLong_FromLong(self->fThis->GetSequenceNumber());
}

PYTHON_METHOD_DEFINITION(ptVaultAgeInfoNode, setAgeSequenceNumber, args)
{
	long seqNum;
	if (!PyArg_ParseTuple(args, "l", &seqNum))
	{
		PyErr_SetString(PyExc_TypeError, "setAgeSequenceNumber expects a long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetSequenceNumber(seqNum);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultAgeInfoNode, getAgeLanguage)
{
	return PyLong_FromLong(self->fThis->GetAgeLanguage());
}

PYTHON_METHOD_DEFINITION(ptVaultAgeInfoNode, setAgeLanguage, args)
{
	long lang;
	if (!PyArg_ParseTuple(args, "l", &lang))
	{
		PyErr_SetString(PyExc_TypeError, "setAgeLanguage expects a long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetAgeLanguage(lang);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultAgeInfoNode, getAgeID)
{
	return PyLong_FromUnsignedLong(self->fThis->GetAgeID());
}

PYTHON_METHOD_DEFINITION(ptVaultAgeInfoNode, setAgeID, args)
{
	unsigned long ageID;
	if (!PyArg_ParseTuple(args, "l", &ageID))
	{
		PyErr_SetString(PyExc_TypeError, "setAgeID expects an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetAgeID(ageID);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultAgeInfoNode, getCzarID)
{
	return PyLong_FromUnsignedLong(self->fThis->GetCzarID());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultAgeInfoNode, isPublic)
{
	PYTHON_RETURN_BOOL(self->fThis->IsPublic());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultAgeInfoNode, getDisplayName)
{
	return PyString_FromString(self->fThis->GetDisplayName());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultAgeInfoNode, asAgeInfoStruct)
{
	return self->fThis->AsAgeInfoStruct();
}

PYTHON_START_METHODS_TABLE(ptVaultAgeInfoNode)
	PYTHON_METHOD_NOARGS(ptVaultAgeInfoNode, getCanVisitFolder, "Returns a ptVaultPlayerInfoList of the players that can visit this age"),
	PYTHON_METHOD_NOARGS(ptVaultAgeInfoNode, getAgeOwnersFolder, "Returns a ptVaultPlayerInfoList of the players that own this age"),
	PYTHON_METHOD_NOARGS(ptVaultAgeInfoNode, getChildAgesFolder, "Returns a ptVaultFolderNode of the child ages of this age"),
	PYTHON_METHOD_NOARGS(ptVaultAgeInfoNode, getAgeSDL, "Returns a ptVaultSDLNode of the age's SDL"),
	PYTHON_METHOD_NOARGS(ptVaultAgeInfoNode, getCzar, "Returns ptVaultPlayerInfoNode of the player that is the Czar"),
	PYTHON_METHOD_NOARGS(ptVaultAgeInfoNode, getParentAgeLink, "Returns ptVaultAgeLinkNode of the age's parent age, or None if not a child age"),
	PYTHON_METHOD_NOARGS(ptVaultAgeInfoNode, getAgeFilename, "Returns the age filename"),
	PYTHON_METHOD(ptVaultAgeInfoNode, setAgeFilename, "Params: fileName\nSets the filename"),
	PYTHON_METHOD_NOARGS(ptVaultAgeInfoNode, getAgeInstanceName, "Returns the instance name of the age"),
	PYTHON_METHOD(ptVaultAgeInfoNode, setAgeInstanceName, "Params: instanceName\nSets the instance name"),
	PYTHON_METHOD_NOARGS(ptVaultAgeInfoNode, getAgeUserDefinedName, "Returns the user define part of the age name"),
	PYTHON_METHOD(ptVaultAgeInfoNode, setAgeUserDefinedName, "Params: udname\nSets the user defined part of the name"),
	PYTHON_METHOD_NOARGS(ptVaultAgeInfoNode, getAgeInstanceGuid, "Returns the age instance guid"),
	PYTHON_METHOD(ptVaultAgeInfoNode, setAgeInstanceGuid, "Params: guid\nSets the age instance GUID"),
	PYTHON_METHOD_NOARGS(ptVaultAgeInfoNode, getAgeDescription, "Returns the description of the age"),
	PYTHON_METHOD(ptVaultAgeInfoNode, setAgeDescription, "Params: description\nSets the description of the age"),
	PYTHON_METHOD_NOARGS(ptVaultAgeInfoNode, getAgeSequenceNumber, "Returns the sequence number of this instance of the age"),
	PYTHON_METHOD(ptVaultAgeInfoNode, setAgeSequenceNumber, "Params: seqNumber\nSets the sequence number"),
	PYTHON_METHOD_NOARGS(ptVaultAgeInfoNode, getAgeLanguage, "Returns the age's language (integer)"),
	PYTHON_METHOD(ptVaultAgeInfoNode, setAgeLanguage, "Params: lang\nSets the age's language (integer)"),
	PYTHON_METHOD_NOARGS(ptVaultAgeInfoNode, getAgeID, "Returns the age ID"),
	PYTHON_METHOD(ptVaultAgeInfoNode, setAgeID, "Params: ageID\nSets the age ID"),
	PYTHON_METHOD_NOARGS(ptVaultAgeInfoNode, getCzarID, "Returns the ID of the age's czar"),
	PYTHON_METHOD_NOARGS(ptVaultAgeInfoNode, isPublic, "Returns whether the age is Public or Not"),
	PYTHON_METHOD_NOARGS(ptVaultAgeInfoNode, getDisplayName, "Returns the displayable version of the age name"),
	PYTHON_METHOD_NOARGS(ptVaultAgeInfoNode, asAgeInfoStruct, "Returns this ptVaultAgeInfoNode as a ptAgeInfoStruct"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptVaultAgeInfoNode, pyVaultNode, "Params: n=0\nPlasma vault age info node");

// required functions for PyObject interoperability
PyObject *pyVaultAgeInfoNode::New(RelVaultNode* nfsNode)
{
	ptVaultAgeInfoNode *newObj = (ptVaultAgeInfoNode*)ptVaultAgeInfoNode_type.tp_new(&ptVaultAgeInfoNode_type, NULL, NULL);
	if (newObj->fThis->fNode)
		newObj->fThis->fNode->DecRef();
	newObj->fThis->fNode = nfsNode;
	if (newObj->fThis->fNode)
		newObj->fThis->fNode->IncRef();
	return (PyObject*)newObj;
}

PyObject *pyVaultAgeInfoNode::New(int n /* =0 */)
{
	ptVaultAgeInfoNode *newObj = (ptVaultAgeInfoNode*)ptVaultAgeInfoNode_type.tp_new(&ptVaultAgeInfoNode_type, NULL, NULL);
	// oddly enough, nothing to do here
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptVaultAgeInfoNode, pyVaultAgeInfoNode)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptVaultAgeInfoNode, pyVaultAgeInfoNode)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyVaultAgeInfoNode::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptVaultAgeInfoNode);
	PYTHON_CLASS_IMPORT_END(m);
}