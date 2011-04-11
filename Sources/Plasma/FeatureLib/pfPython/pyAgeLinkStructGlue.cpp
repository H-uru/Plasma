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
#include "pyAgeLinkStruct.h"
#include "pySpawnPointInfo.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptAgeLinkStruct, pyAgeLinkStruct);

PYTHON_DEFAULT_NEW_DEFINITION(ptAgeLinkStruct, pyAgeLinkStruct)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptAgeLinkStruct)

PYTHON_INIT_DEFINITION(ptAgeLinkStruct, args, keywords)
{
	PYTHON_RETURN_INIT_OK;
}

PYTHON_RICH_COMPARE_DEFINITION(ptAgeLinkStruct, obj1, obj2, compareType)
{
	if ((obj1 == Py_None) || (obj2 == Py_None) || !pyAgeLinkStruct::Check(obj1) || !pyAgeLinkStruct::Check(obj2))
	{
		// if they aren't the same type, they don't match, obviously (we also never equal none)
		if (compareType == Py_EQ)
			PYTHON_RCOMPARE_FALSE;
		else if (compareType == Py_NE)
			PYTHON_RCOMPARE_TRUE;
		else
		{
			PyErr_SetString(PyExc_NotImplementedError, "invalid comparison for a ptAgeLinkStruct object");
			PYTHON_RCOMPARE_ERROR;
		}
	}
	pyAgeLinkStruct *struct1 = pyAgeLinkStruct::ConvertFrom(obj1);
	pyAgeLinkStruct *struct2 = pyAgeLinkStruct::ConvertFrom(obj2);
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
	PyErr_SetString(PyExc_NotImplementedError, "invalid comparison for a ptAgeLinkStruct object");
	PYTHON_RCOMPARE_ERROR;
}

PYTHON_METHOD_DEFINITION(ptAgeLinkStruct, copyFrom, args)
{
	PyObject* linkStructObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &linkStructObj))
	{
		PyErr_SetString(PyExc_TypeError, "copyFrom expects a ptAgeLinkStruct or ptAgeLinkStructRef");
		PYTHON_RETURN_ERROR;
	}
	if (pyAgeLinkStruct::Check(linkStructObj))
	{
		pyAgeLinkStruct* linkStruct = pyAgeLinkStruct::ConvertFrom(linkStructObj);
		self->fThis->CopyFrom(*linkStruct);
		PYTHON_RETURN_NONE;
	}
	else if (pyAgeLinkStructRef::Check(linkStructObj))
	{
		pyAgeLinkStructRef* linkStruct = pyAgeLinkStructRef::ConvertFrom(linkStructObj);
		self->fThis->CopyFromRef(*linkStruct);
		PYTHON_RETURN_NONE;
	}
	PyErr_SetString(PyExc_TypeError, "copyFrom expects a ptAgeLinkStruct or ptAgeLinkStructRef");
	PYTHON_RETURN_ERROR;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeLinkStruct, getAgeInfo)
{
	return self->fThis->GetAgeInfo();
}

PYTHON_METHOD_DEFINITION(ptAgeLinkStruct, setAgeInfo, args)
{
	PyObject* ageInfoObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &ageInfoObj))
	{
		PyErr_SetString(PyExc_TypeError, "setAgeInfo expects a ptAgeInfoStruct");
		PYTHON_RETURN_ERROR;
	}
	if (!pyAgeInfoStruct::Check(ageInfoObj))
	{
		PyErr_SetString(PyExc_TypeError, "setAgeInfo expects a ptAgeInfoStruct");
		PYTHON_RETURN_ERROR;
	}
	pyAgeInfoStruct* ageInfo = pyAgeInfoStruct::ConvertFrom(ageInfoObj);
	self->fThis->SetAgeInfo(*ageInfo);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeLinkStruct, getParentAgeFilename)
{
	return PyString_FromString(self->fThis->GetParentAgeFilename());
}

PYTHON_METHOD_DEFINITION(ptAgeLinkStruct, setParentAgeFilename, args)
{
	char* filename;
	if (!PyArg_ParseTuple(args, "s", &filename))
	{
		PyErr_SetString(PyExc_TypeError, "setParentAgeFilename expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetParentAgeFilename(filename);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeLinkStruct, getLinkingRules)
{
	return PyInt_FromLong(self->fThis->GetLinkingRules());
}

PYTHON_METHOD_DEFINITION(ptAgeLinkStruct, setLinkingRules, args)
{
	int rules;
	if (!PyArg_ParseTuple(args, "i", &rules))
	{
		PyErr_SetString(PyExc_TypeError, "setLinkingRules expects an int");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetLinkingRules(rules);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeLinkStruct, getSpawnPoint)
{
	return self->fThis->GetSpawnPoint();
}

PYTHON_METHOD_DEFINITION(ptAgeLinkStruct, setSpawnPoint, args)
{
	PyObject* spawnPtInfoObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &spawnPtInfoObj))
	{
		PyErr_SetString(PyExc_TypeError, "setSpawnPoint expects a ptSpawnPointInfo or a ptSpawnPointInfoRef");
		PYTHON_RETURN_ERROR;
	}
	if (pySpawnPointInfo::Check(spawnPtInfoObj))
	{
		pySpawnPointInfo* spawnPt = pySpawnPointInfo::ConvertFrom(spawnPtInfoObj);
		self->fThis->SetSpawnPoint(*spawnPt);
		PYTHON_RETURN_NONE;
	}
	else if (pySpawnPointInfoRef::Check(spawnPtInfoObj))
	{
		pySpawnPointInfoRef* spawnPt = pySpawnPointInfoRef::ConvertFrom(spawnPtInfoObj);
		self->fThis->SetSpawnPointRef(*spawnPt);
		PYTHON_RETURN_NONE;
	}
	PyErr_SetString(PyExc_TypeError, "setSpawnPoint expects a ptSpawnPointInfo or a ptSpawnPointInfoRef");
	PYTHON_RETURN_ERROR;
}

PYTHON_START_METHODS_TABLE(ptAgeLinkStruct)
	PYTHON_METHOD(ptAgeLinkStruct, copyFrom, "Params: other\nCopies data from one ptAgeLinkStruct or ptAgeLinkStructRef to this one"),
	PYTHON_METHOD_NOARGS(ptAgeLinkStruct, getAgeInfo, "Returns a ptAgeInfoStructRef of the AgeInfo for this link"),
	PYTHON_METHOD(ptAgeLinkStruct, setAgeInfo, "Params: ageInfo\nSets the AgeInfoStruct from the data in ageInfo (a ptAgeInfoStruct)"),
	PYTHON_METHOD_NOARGS(ptAgeLinkStruct, getParentAgeFilename, "Returns a string of the parent age filename"),
	PYTHON_METHOD(ptAgeLinkStruct, setParentAgeFilename, "Params: filename\nSets the parent age filename for child age links"),
	PYTHON_METHOD_NOARGS(ptAgeLinkStruct, getLinkingRules, "Returns the linking rules of this link"),
	PYTHON_METHOD(ptAgeLinkStruct, setLinkingRules, "Params: rule\nSets the linking rules for this link"),
	PYTHON_METHOD_NOARGS(ptAgeLinkStruct, getSpawnPoint, "Gets the spawn point ptSpawnPointInfoRef of this link"),
	PYTHON_METHOD(ptAgeLinkStruct, setSpawnPoint, "Params: spawnPtInfo\nSets the spawn point of this link (a ptSpawnPointInfo or ptSpawnPointInfoRef)"),
PYTHON_END_METHODS_TABLE;

// type structure definition
#define ptAgeLinkStruct_COMPARE			PYTHON_NO_COMPARE
#define ptAgeLinkStruct_AS_NUMBER		PYTHON_NO_AS_NUMBER
#define ptAgeLinkStruct_AS_SEQUENCE		PYTHON_NO_AS_SEQUENCE
#define ptAgeLinkStruct_AS_MAPPING		PYTHON_NO_AS_MAPPING
#define ptAgeLinkStruct_STR				PYTHON_NO_STR
#define ptAgeLinkStruct_RICH_COMPARE	PYTHON_DEFAULT_RICH_COMPARE(ptAgeLinkStruct)
#define ptAgeLinkStruct_GETSET			PYTHON_NO_GETSET
#define ptAgeLinkStruct_BASE			PYTHON_NO_BASE
PLASMA_CUSTOM_TYPE(ptAgeLinkStruct, "Class to hold the data of the AgeLink structure");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptAgeLinkStruct, pyAgeLinkStruct)

PyObject *pyAgeLinkStruct::New(plAgeLinkStruct *link)
{
	ptAgeLinkStruct *newObj = (ptAgeLinkStruct*)ptAgeLinkStruct_type.tp_new(&ptAgeLinkStruct_type, NULL, NULL);
	newObj->fThis->fAgeLink.CopyFrom(link);
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptAgeLinkStruct, pyAgeLinkStruct)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptAgeLinkStruct, pyAgeLinkStruct)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyAgeLinkStruct::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptAgeLinkStruct);
	PYTHON_CLASS_IMPORT_END(m);
}

// glue functions
PYTHON_CLASS_DEFINITION(ptAgeLinkStructRef, pyAgeLinkStructRef);

PYTHON_DEFAULT_NEW_DEFINITION(ptAgeLinkStructRef, pyAgeLinkStructRef)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptAgeLinkStructRef)

PYTHON_NO_INIT_DEFINITION(ptAgeLinkStructRef)

PYTHON_METHOD_DEFINITION(ptAgeLinkStructRef, copyFrom, args)
{
	PyObject* linkStructObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &linkStructObj))
	{
		PyErr_SetString(PyExc_TypeError, "copyFrom expects a ptAgeLinkStruct or ptAgeLinkStructRef");
		PYTHON_RETURN_ERROR;
	}
	if (pyAgeLinkStruct::Check(linkStructObj))
	{
		pyAgeLinkStruct* linkStruct = pyAgeLinkStruct::ConvertFrom(linkStructObj);
		self->fThis->CopyFrom(*linkStruct);
		PYTHON_RETURN_NONE;
	}
	else if (pyAgeLinkStructRef::Check(linkStructObj))
	{
		pyAgeLinkStructRef* linkStruct = pyAgeLinkStructRef::ConvertFrom(linkStructObj);
		self->fThis->CopyFromRef(*linkStruct);
		PYTHON_RETURN_NONE;
	}
	PyErr_SetString(PyExc_TypeError, "copyFrom expects a ptAgeLinkStruct or ptAgeLinkStructRef");
	PYTHON_RETURN_ERROR;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeLinkStructRef, getAgeInfo)
{
	return self->fThis->GetAgeInfo();
}

PYTHON_METHOD_DEFINITION(ptAgeLinkStructRef, setAgeInfo, args)
{
	PyObject* ageInfoObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &ageInfoObj))
	{
		PyErr_SetString(PyExc_TypeError, "setAgeInfo expects a ptAgeInfoStruct");
		PYTHON_RETURN_ERROR;
	}
	if (!pyAgeInfoStruct::Check(ageInfoObj))
	{
		PyErr_SetString(PyExc_TypeError, "setAgeInfo expects a ptAgeInfoStruct");
		PYTHON_RETURN_ERROR;
	}
	pyAgeInfoStruct* ageInfo = pyAgeInfoStruct::ConvertFrom(ageInfoObj);
	self->fThis->SetAgeInfo(*ageInfo);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeLinkStructRef, getLinkingRules)
{
	return PyInt_FromLong(self->fThis->GetLinkingRules());
}

PYTHON_METHOD_DEFINITION(ptAgeLinkStructRef, setLinkingRules, args)
{
	int rules;
	if (!PyArg_ParseTuple(args, "i", &rules))
	{
		PyErr_SetString(PyExc_TypeError, "setLinkingRules expects an int");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetLinkingRules(rules);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeLinkStructRef, getSpawnPoint)
{
	return self->fThis->GetSpawnPoint();
}

PYTHON_METHOD_DEFINITION(ptAgeLinkStructRef, setSpawnPoint, args)
{
	PyObject* spawnPtInfoObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &spawnPtInfoObj))
	{
		PyErr_SetString(PyExc_TypeError, "setSpawnPoint expects a ptSpawnPointInfo or a ptSpawnPointInfoRef");
		PYTHON_RETURN_ERROR;
	}
	if (pySpawnPointInfo::Check(spawnPtInfoObj))
	{
		pySpawnPointInfo* spawnPt = pySpawnPointInfo::ConvertFrom(spawnPtInfoObj);
		self->fThis->SetSpawnPoint(*spawnPt);
		PYTHON_RETURN_NONE;
	}
	else if (pySpawnPointInfoRef::Check(spawnPtInfoObj))
	{
		pySpawnPointInfoRef* spawnPt = pySpawnPointInfoRef::ConvertFrom(spawnPtInfoObj);
		self->fThis->SetSpawnPointRef(*spawnPt);
		PYTHON_RETURN_NONE;
	}
	PyErr_SetString(PyExc_TypeError, "setSpawnPoint expects a ptSpawnPointInfo or a ptSpawnPointInfoRef");
	PYTHON_RETURN_ERROR;
}

PYTHON_START_METHODS_TABLE(ptAgeLinkStructRef)
	PYTHON_METHOD(ptAgeLinkStructRef, copyFrom, "Params: other\nCopies data from one ptAgeLinkStruct or ptAgeLinkStructRef to this one"),
	PYTHON_METHOD_NOARGS(ptAgeLinkStructRef, getAgeInfo, "Returns a ptAgeInfoStructRef of the AgeInfo for this link"),
	PYTHON_METHOD(ptAgeLinkStructRef, setAgeInfo, "Params: ageInfo\nSets the AgeInfoStruct from the data in ageInfo (a ptAgeInfoStruct)"),
	PYTHON_METHOD_NOARGS(ptAgeLinkStructRef, getLinkingRules, "Returns the linking rules of this link"),
	PYTHON_METHOD(ptAgeLinkStructRef, setLinkingRules, "Params: rule\nSets the linking rules for this link"),
	PYTHON_METHOD_NOARGS(ptAgeLinkStructRef, getSpawnPoint, "Gets the spawn point ptSpawnPointInfoRef of this link"),
	PYTHON_METHOD(ptAgeLinkStructRef, setSpawnPoint, "Params: spawnPtInfo\nSets the spawn point of this link (a ptSpawnPointInfo or ptSpawnPointInfoRef)"),
PYTHON_END_METHODS_TABLE;

// type structure definition
PLASMA_DEFAULT_TYPE(ptAgeLinkStructRef, "Class to hold the data of the AgeLink structure");

// required functions for PyObject interoperability
PyObject *pyAgeLinkStructRef::New(plAgeLinkStruct &link)
{
	ptAgeLinkStructRef *newObj = (ptAgeLinkStructRef*)ptAgeLinkStructRef_type.tp_new(&ptAgeLinkStructRef_type, NULL, NULL);
	newObj->fThis->fAgeLink = link;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptAgeLinkStructRef, pyAgeLinkStructRef)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptAgeLinkStructRef, pyAgeLinkStructRef)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyAgeLinkStructRef::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptAgeLinkStructRef);
	PYTHON_CLASS_IMPORT_END(m);
}