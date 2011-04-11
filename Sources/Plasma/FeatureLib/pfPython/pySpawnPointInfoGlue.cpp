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
#include "pySpawnPointInfo.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptSpawnPointInfo, pySpawnPointInfo);

PYTHON_DEFAULT_NEW_DEFINITION(ptSpawnPointInfo, pySpawnPointInfo)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptSpawnPointInfo)

PYTHON_INIT_DEFINITION(ptSpawnPointInfo, args, keywords)
{
	char* title = NULL;
	char* spawnPt = NULL;
	if (!PyArg_ParseTuple(args, "|ss", &title, &spawnPt))
	{
		PyErr_SetString(PyExc_TypeError, "__init__ expects two optional strings, or no parameters");
		PYTHON_RETURN_INIT_ERROR;
	}
	if (!title && !spawnPt)
	{
		// default init
		PYTHON_RETURN_INIT_OK;
	}
	else if (title && spawnPt)
	{
		self->fThis->SetTitle(title);
		self->fThis->SetName(spawnPt);
		PYTHON_RETURN_INIT_OK;
	}
	// only one param existed
	PyErr_SetString(PyExc_TypeError, "__init__ expects two optional strings, or no parameters");
	PYTHON_RETURN_INIT_ERROR;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSpawnPointInfo, getTitle)
{
	return PyString_FromString(self->fThis->GetTitle());
}

PYTHON_METHOD_DEFINITION(ptSpawnPointInfo, setTitle, args)
{
	char* title;
	if (!PyArg_ParseTuple(args, "s", &title))
	{
		PyErr_SetString(PyExc_TypeError, "setTitle expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetTitle(title);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSpawnPointInfo, getName)
{
	return PyString_FromString(self->fThis->GetName());
}

PYTHON_METHOD_DEFINITION(ptSpawnPointInfo, setName, args)
{
	char* name;
	if (!PyArg_ParseTuple(args, "s", &name))
	{
		PyErr_SetString(PyExc_TypeError, "setName expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetName(name);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSpawnPointInfo, getCameraStack)
{
	return PyString_FromString(self->fThis->GetCameraStack());
}

PYTHON_METHOD_DEFINITION(ptSpawnPointInfo, setCameraStack, args)
{
	char* camStack;
	if (!PyArg_ParseTuple(args, "s", &camStack))
	{
		PyErr_SetString(PyExc_TypeError, "setCameraStack expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetCameraStack(camStack);
	PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptSpawnPointInfo)
	PYTHON_METHOD_NOARGS(ptSpawnPointInfo, getTitle, "Returns the spawnpoint's title"),
	PYTHON_METHOD(ptSpawnPointInfo, setTitle, "Params: title\nSets the spawnpoint's title"),
	PYTHON_METHOD_NOARGS(ptSpawnPointInfo, getName, "Returns the spawnpoint's name"),
	PYTHON_METHOD(ptSpawnPointInfo, setName, "Params: name\nSets the spawnpoint's name"),
	PYTHON_METHOD_NOARGS(ptSpawnPointInfo, getCameraStack, "Returns the camera stack for this spawnpoint as a string"),
	PYTHON_METHOD(ptSpawnPointInfo, setCameraStack, "Params: stack\nSets the spawnpoint's camera stack (as a string)"),
PYTHON_END_METHODS_TABLE;

// type structure definition
PLASMA_DEFAULT_TYPE(ptSpawnPointInfo, "Params: title=None,spawnPt=None\nClass to hold spawn point data");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptSpawnPointInfo, pySpawnPointInfo)

PyObject *pySpawnPointInfo::New(const plSpawnPointInfo& info)
{
	ptSpawnPointInfo *newObj = (ptSpawnPointInfo*)ptSpawnPointInfo_type.tp_new(&ptSpawnPointInfo_type, NULL, NULL);
	newObj->fThis->fInfo = info;
	return (PyObject*)newObj;
}

PyObject *pySpawnPointInfo::New(const char* title, const char* spawnPt)
{
	ptSpawnPointInfo *newObj = (ptSpawnPointInfo*)ptSpawnPointInfo_type.tp_new(&ptSpawnPointInfo_type, NULL, NULL);
	newObj->fThis->fInfo.fTitle = title;
	newObj->fThis->fInfo.fSpawnPt = spawnPt;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptSpawnPointInfo, pySpawnPointInfo)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptSpawnPointInfo, pySpawnPointInfo)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pySpawnPointInfo::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptSpawnPointInfo);
	PYTHON_CLASS_IMPORT_END(m);
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetDefaultSpawnPoint, "Returns the default spawnpoint definition (as a ptSpawnPointInfo)")
{
	return pySpawnPointInfo::GetDefaultSpawnPoint();
}

void pySpawnPointInfo::AddPlasmaMethods(std::vector<PyMethodDef> &methods)
{
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetDefaultSpawnPoint);
}

// glue functions
PYTHON_CLASS_DEFINITION(ptSpawnPointInfoRef, pySpawnPointInfoRef);

PYTHON_DEFAULT_NEW_DEFINITION(ptSpawnPointInfoRef, pySpawnPointInfoRef)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptSpawnPointInfoRef)

PYTHON_NO_INIT_DEFINITION(ptSpawnPointInfoRef)

PYTHON_METHOD_DEFINITION_NOARGS(ptSpawnPointInfoRef, getTitle)
{
	return PyString_FromString(self->fThis->GetTitle());
}

PYTHON_METHOD_DEFINITION(ptSpawnPointInfoRef, setTitle, args)
{
	char* title;
	if (!PyArg_ParseTuple(args, "s", &title))
	{
		PyErr_SetString(PyExc_TypeError, "setTitle expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetTitle(title);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSpawnPointInfoRef, getName)
{
	return PyString_FromString(self->fThis->GetName());
}

PYTHON_METHOD_DEFINITION(ptSpawnPointInfoRef, setName, args)
{
	char* name;
	if (!PyArg_ParseTuple(args, "s", &name))
	{
		PyErr_SetString(PyExc_TypeError, "setName expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetName(name);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSpawnPointInfoRef, getCameraStack)
{
	return PyString_FromString(self->fThis->GetCameraStack());
}

PYTHON_METHOD_DEFINITION(ptSpawnPointInfoRef, setCameraStack, args)
{
	char* camStack;
	if (!PyArg_ParseTuple(args, "s", &camStack))
	{
		PyErr_SetString(PyExc_TypeError, "setCameraStack expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetCameraStack(camStack);
	PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptSpawnPointInfoRef)
	PYTHON_METHOD_NOARGS(ptSpawnPointInfoRef, getTitle, "Returns the spawnpoint's title"),
	PYTHON_METHOD(ptSpawnPointInfoRef, setTitle, "Params: title\nSets the spawnpoint's title"),
	PYTHON_METHOD_NOARGS(ptSpawnPointInfoRef, getName, "Returns the spawnpoint's name"),
	PYTHON_METHOD(ptSpawnPointInfoRef, setName, "Params: name\nSets the spawnpoint's name"),
	PYTHON_METHOD_NOARGS(ptSpawnPointInfoRef, getCameraStack, "Returns the camera stack for this spawnpoint as a string"),
	PYTHON_METHOD(ptSpawnPointInfoRef, setCameraStack, "Params: stack\nSets the spawnpoint's camera stack (as a string)"),
PYTHON_END_METHODS_TABLE;

// type structure definition
PLASMA_DEFAULT_TYPE(ptSpawnPointInfoRef, "Class to hold spawn point data");

// required functions for PyObject interoperability
PyObject *pySpawnPointInfoRef::New(plSpawnPointInfo& info)
{
	ptSpawnPointInfoRef *newObj = (ptSpawnPointInfoRef*)ptSpawnPointInfoRef_type.tp_new(&ptSpawnPointInfoRef_type, NULL, NULL);
	newObj->fThis->fInfo = info;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptSpawnPointInfoRef, pySpawnPointInfoRef)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptSpawnPointInfoRef, pySpawnPointInfoRef)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pySpawnPointInfoRef::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptSpawnPointInfoRef);
	PYTHON_CLASS_IMPORT_END(m);
}