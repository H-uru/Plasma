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
#include "pyAgeInfoStruct.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptAgeInfoStruct, pyAgeInfoStruct);

PYTHON_DEFAULT_NEW_DEFINITION(ptAgeInfoStruct, pyAgeInfoStruct)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptAgeInfoStruct)

PYTHON_INIT_DEFINITION(ptAgeInfoStruct, args, keywords)
{
	PYTHON_RETURN_INIT_OK;
}

PYTHON_RICH_COMPARE_DEFINITION(ptAgeInfoStruct, obj1, obj2, compareType)
{
	if ((obj1 == Py_None) || (obj2 == Py_None) || !pyAgeInfoStruct::Check(obj1) || !pyAgeInfoStruct::Check(obj2))
	{
		// if they aren't the same type, they don't match, obviously (we also never equal none)
		if (compareType == Py_EQ)
			PYTHON_RCOMPARE_FALSE;
		else if (compareType == Py_NE)
			PYTHON_RCOMPARE_TRUE;
		else
		{
			PyErr_SetString(PyExc_NotImplementedError, "invalid comparison for a ptAgeInfoStruct object");
			PYTHON_RCOMPARE_ERROR;
		}
	}
	pyAgeInfoStruct *struct1 = pyAgeInfoStruct::ConvertFrom(obj1);
	pyAgeInfoStruct *struct2 = pyAgeInfoStruct::ConvertFrom(obj2);
	if (compareType == Py_EQ)
	{
		if ((*struct1) == (*struct2))
			PYTHON_RCOMPARE_TRUE;
		PYTHON_RCOMPARE_FALSE;
	}
	else if (compareType == Py_NE)
	{
		if ((*struct1) != (*struct2))
			PYTHON_RCOMPARE_TRUE;
		PYTHON_RCOMPARE_FALSE;
	}
	PyErr_SetString(PyExc_NotImplementedError, "invalid comparison for a ptAgeInfoStruct object");
	PYTHON_RCOMPARE_ERROR;
}

PYTHON_METHOD_DEFINITION(ptAgeInfoStruct, copyFrom, args)
{
	PyObject* infoStructObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &infoStructObj))
	{
		PyErr_SetString(PyExc_TypeError, "copyFrom expects a ptAgeInfoStruct or ptAgeInfoStructRef");
		PYTHON_RETURN_ERROR;
	}
	if (pyAgeInfoStruct::Check(infoStructObj))
	{
		pyAgeInfoStruct* infoStruct = pyAgeInfoStruct::ConvertFrom(infoStructObj);
		self->fThis->CopyFrom(*infoStruct);
		PYTHON_RETURN_NONE;
	}
	else if (pyAgeInfoStructRef::Check(infoStructObj))
	{
		pyAgeInfoStructRef* infoStruct = pyAgeInfoStructRef::ConvertFrom(infoStructObj);
		self->fThis->CopyFromRef(*infoStruct);
		PYTHON_RETURN_NONE;
	}
	PyErr_SetString(PyExc_TypeError, "copyFrom expects a ptAgeInfoStruct or ptAgeInfoStructRef");
	PYTHON_RETURN_ERROR;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeInfoStruct, getAgeFilename)
{
	return PyString_FromString(self->fThis->GetAgeFilename());
}

PYTHON_METHOD_DEFINITION(ptAgeInfoStruct, setAgeFilename, args)
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

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeInfoStruct, getAgeInstanceName)
{
	return PyString_FromString(self->fThis->GetAgeInstanceName());
}

PYTHON_METHOD_DEFINITION(ptAgeInfoStruct, setAgeInstanceName, args)
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

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeInfoStruct, getAgeUserDefinedName)
{
	return PyString_FromString(self->fThis->GetAgeUserDefinedName());
}

PYTHON_METHOD_DEFINITION(ptAgeInfoStruct, setAgeUserDefinedName, args)
{
	char* userName;
	if (!PyArg_ParseTuple(args, "s", &userName))
	{
		PyErr_SetString(PyExc_TypeError, "setAgeUserDefinedName expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetAgeUserDefinedName(userName);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeInfoStruct, getAgeDescription)
{
	return PyString_FromString(self->fThis->GetAgeDescription());
}

PYTHON_METHOD_DEFINITION(ptAgeInfoStruct, setAgeDescription, args)
{
	char* desc;
	if (!PyArg_ParseTuple(args, "s", &desc))
	{
		PyErr_SetString(PyExc_TypeError, "setAgeDescription expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetAgeDescription(desc);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeInfoStruct, getAgeInstanceGuid)
{
	return PyString_FromString(self->fThis->GetAgeInstanceGuid());
}

PYTHON_METHOD_DEFINITION(ptAgeInfoStruct, setAgeInstanceGuid, args)
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

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeInfoStruct, getAgeSequenceNumber)
{
	return PyInt_FromLong(self->fThis->GetAgeSequenceNumber());
}

PYTHON_METHOD_DEFINITION(ptAgeInfoStruct, setAgeSequenceNumber, args)
{
	long sequenceNum;
	if (!PyArg_ParseTuple(args, "l", &sequenceNum))
	{
		PyErr_SetString(PyExc_TypeError, "setAgeSequenceNumber expects a long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetAgeSequenceNumber(sequenceNum);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeInfoStruct, getAgeLanguage)
{
	return PyInt_FromLong(self->fThis->GetAgeLanguage());
}

PYTHON_METHOD_DEFINITION(ptAgeInfoStruct, setAgeLanguage, args)
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

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeInfoStruct, getDisplayName)
{
	return PyString_FromString(self->fThis->GetDisplayName());
}

PYTHON_START_METHODS_TABLE(ptAgeInfoStruct)
	PYTHON_METHOD(ptAgeInfoStruct, copyFrom, "Params: other\nCopies data from one ptAgeInfoStruct or ptAgeInfoStructRef to this one"),
	PYTHON_METHOD_NOARGS(ptAgeInfoStruct, getAgeFilename, "Gets the Age's filename"),
	PYTHON_METHOD(ptAgeInfoStruct, setAgeFilename, "Params: filename\nSets the filename of the Age"),
	PYTHON_METHOD_NOARGS(ptAgeInfoStruct, getAgeInstanceName, "Get the instance name of the Age"),
	PYTHON_METHOD(ptAgeInfoStruct, setAgeInstanceName, "Params: instanceName\nSets the instance name of the Age"),
	PYTHON_METHOD_NOARGS(ptAgeInfoStruct, getAgeUserDefinedName, "Gets the user defined part of the Age name"),
	PYTHON_METHOD(ptAgeInfoStruct, setAgeUserDefinedName, "Params: udName\nSets the user defined part of the Age"),
	PYTHON_METHOD_NOARGS(ptAgeInfoStruct, getAgeDescription, "Gets the description part of the Age name"),
	PYTHON_METHOD(ptAgeInfoStruct, setAgeDescription, "Params: udName\nSets the description part of the Age"),
	PYTHON_METHOD_NOARGS(ptAgeInfoStruct, getAgeInstanceGuid, "Get the Age's instance GUID"),
	PYTHON_METHOD(ptAgeInfoStruct, setAgeInstanceGuid, "Params: guid\nSets the Age instance's GUID"),
	PYTHON_METHOD_NOARGS(ptAgeInfoStruct, getAgeSequenceNumber, "Gets the unique sequence number"),
	PYTHON_METHOD(ptAgeInfoStruct, setAgeSequenceNumber, "Params: seqNumber\nSets the unique sequence number"),
	PYTHON_METHOD_NOARGS(ptAgeInfoStruct, getAgeLanguage, "Gets the age's language (integer)"),
	PYTHON_METHOD(ptAgeInfoStruct, setAgeLanguage, "Params: lang\nSets the age's language (integer)"),
	PYTHON_METHOD_NOARGS(ptAgeInfoStruct, getDisplayName, "Returns a string that is the displayable name of the age instance"),
PYTHON_END_METHODS_TABLE;

// type structure definition
#define ptAgeInfoStruct_COMPARE			PYTHON_NO_COMPARE
#define ptAgeInfoStruct_AS_NUMBER		PYTHON_NO_AS_NUMBER
#define ptAgeInfoStruct_AS_SEQUENCE		PYTHON_NO_AS_SEQUENCE
#define ptAgeInfoStruct_AS_MAPPING		PYTHON_NO_AS_MAPPING
#define ptAgeInfoStruct_STR				PYTHON_NO_STR
#define ptAgeInfoStruct_RICH_COMPARE	PYTHON_DEFAULT_RICH_COMPARE(ptAgeInfoStruct)
#define ptAgeInfoStruct_GETSET			PYTHON_NO_GETSET
#define ptAgeInfoStruct_BASE			PYTHON_NO_BASE
PLASMA_CUSTOM_TYPE(ptAgeInfoStruct, "Class to hold AgeInfo struct data");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptAgeInfoStruct, pyAgeInfoStruct)

PyObject *pyAgeInfoStruct::New(plAgeInfoStruct *info)
{
	ptAgeInfoStruct *newObj = (ptAgeInfoStruct*)ptAgeInfoStruct_type.tp_new(&ptAgeInfoStruct_type, NULL, NULL);
	newObj->fThis->fAgeInfo.CopyFrom(info);
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptAgeInfoStruct, pyAgeInfoStruct)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptAgeInfoStruct, pyAgeInfoStruct)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyAgeInfoStruct::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptAgeInfoStruct);
	PYTHON_CLASS_IMPORT_END(m);
}

// glue functions
PYTHON_CLASS_DEFINITION(ptAgeInfoStructRef, pyAgeInfoStructRef);

PYTHON_DEFAULT_NEW_DEFINITION(ptAgeInfoStructRef, pyAgeInfoStructRef)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptAgeInfoStructRef)

PYTHON_NO_INIT_DEFINITION(ptAgeInfoStructRef)

PYTHON_METHOD_DEFINITION(ptAgeInfoStructRef, copyFrom, args)
{
	PyObject* infoStructObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &infoStructObj))
	{
		PyErr_SetString(PyExc_TypeError, "copyFrom expects a ptAgeInfoStruct or ptAgeInfoStructRef");
		PYTHON_RETURN_ERROR;
	}
	if (pyAgeInfoStruct::Check(infoStructObj))
	{
		pyAgeInfoStruct* infoStruct = pyAgeInfoStruct::ConvertFrom(infoStructObj);
		self->fThis->CopyFrom(*infoStruct);
		PYTHON_RETURN_NONE;
	}
	else if (pyAgeInfoStructRef::Check(infoStructObj))
	{
		pyAgeInfoStructRef* infoStruct = pyAgeInfoStructRef::ConvertFrom(infoStructObj);
		self->fThis->CopyFromRef(*infoStruct);
		PYTHON_RETURN_NONE;
	}
	PyErr_SetString(PyExc_TypeError, "copyFrom expects a ptAgeInfoStruct or ptAgeInfoStructRef");
	PYTHON_RETURN_ERROR;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeInfoStructRef, getAgeFilename)
{
	return PyString_FromString(self->fThis->GetAgeFilename());
}

PYTHON_METHOD_DEFINITION(ptAgeInfoStructRef, setAgeFilename, args)
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

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeInfoStructRef, getAgeInstanceName)
{
	return PyString_FromString(self->fThis->GetAgeInstanceName());
}

PYTHON_METHOD_DEFINITION(ptAgeInfoStructRef, setAgeInstanceName, args)
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

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeInfoStructRef, getAgeUserDefinedName)
{
	return PyString_FromString(self->fThis->GetAgeUserDefinedName());
}

PYTHON_METHOD_DEFINITION(ptAgeInfoStructRef, setAgeUserDefinedName, args)
{
	char* userName;
	if (!PyArg_ParseTuple(args, "s", &userName))
	{
		PyErr_SetString(PyExc_TypeError, "setAgeUserDefinedName expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetAgeUserDefinedName(userName);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeInfoStructRef, getAgeInstanceGuid)
{
	return PyString_FromString(self->fThis->GetAgeInstanceGuid());
}

PYTHON_METHOD_DEFINITION(ptAgeInfoStructRef, setAgeInstanceGuid, args)
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

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeInfoStructRef, getAgeSequenceNumber)
{
	return PyInt_FromLong(self->fThis->GetAgeSequenceNumber());
}

PYTHON_METHOD_DEFINITION(ptAgeInfoStructRef, setAgeSequenceNumber, args)
{
	long sequenceNum;
	if (!PyArg_ParseTuple(args, "l", &sequenceNum))
	{
		PyErr_SetString(PyExc_TypeError, "setAgeSequenceNumber expects a long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetAgeSequenceNumber(sequenceNum);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeInfoStructRef, getDisplayName)
{
	return PyString_FromString(self->fThis->GetDisplayName());
}

PYTHON_START_METHODS_TABLE(ptAgeInfoStructRef)
	PYTHON_METHOD(ptAgeInfoStructRef, copyFrom, "Params: other\nCopies data from one ptAgeInfoStruct or ptAgeInfoStructRef to this one"),
	PYTHON_METHOD_NOARGS(ptAgeInfoStructRef, getAgeFilename, "Gets the Age's filename"),
	PYTHON_METHOD(ptAgeInfoStructRef, setAgeFilename, "Params: filename\nSets the filename of the Age"),
	PYTHON_METHOD_NOARGS(ptAgeInfoStructRef, getAgeInstanceName, "Get the instance name of the Age"),
	PYTHON_METHOD(ptAgeInfoStructRef, setAgeInstanceName, "Params: instanceName\nSets the instance name of the Age"),
	PYTHON_METHOD_NOARGS(ptAgeInfoStructRef, getAgeUserDefinedName, "Gets the user defined part of the Age name"),
	PYTHON_METHOD(ptAgeInfoStructRef, setAgeUserDefinedName, "Params: udName\nSets the user defined part of the Age"),
	PYTHON_METHOD_NOARGS(ptAgeInfoStructRef, getAgeInstanceGuid, "Get the Age's instance GUID"),
	PYTHON_METHOD(ptAgeInfoStructRef, setAgeInstanceGuid, "Params: guid\nSets the Age instance's GUID"),
	PYTHON_METHOD_NOARGS(ptAgeInfoStructRef, getAgeSequenceNumber, "Gets the unique sequence number"),
	PYTHON_METHOD(ptAgeInfoStructRef, setAgeSequenceNumber, "Params: seqNumber\nSets the unique sequence number"),
	PYTHON_METHOD_NOARGS(ptAgeInfoStructRef, getDisplayName, "Returns a string that is the displayable name of the age instance"),
PYTHON_END_METHODS_TABLE;

// type structure definition
PLASMA_DEFAULT_TYPE(ptAgeInfoStructRef, "Class to hold AgeInfo struct data");

// required functions for PyObject interoperability
PyObject *pyAgeInfoStructRef::New(plAgeInfoStruct &info)
{
	ptAgeInfoStructRef *newObj = (ptAgeInfoStructRef*)ptAgeInfoStructRef_type.tp_new(&ptAgeInfoStructRef_type, NULL, NULL);
	newObj->fThis->fAgeInfo = info;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptAgeInfoStructRef, pyAgeInfoStructRef)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptAgeInfoStructRef, pyAgeInfoStructRef)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyAgeInfoStructRef::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptAgeInfoStructRef);
	PYTHON_CLASS_IMPORT_END(m);
}