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
#include "pyKey.h"
#ifndef BUILDING_PYPLASMA
#include "pySceneObject.h"
#endif

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptKey, pyKey);

PYTHON_DEFAULT_NEW_DEFINITION(ptKey, pyKey)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptKey)

PYTHON_NO_INIT_DEFINITION(ptKey)

PYTHON_RICH_COMPARE_DEFINITION(ptKey, obj1, obj2, compareType)
{
	if ((obj1 == Py_None) || (obj2 == Py_None) || !pyKey::Check(obj1) || !pyKey::Check(obj2))
	{
		// if they aren't the same type, they don't match, obviously (we also never equal none)
		if (compareType == Py_EQ)
			PYTHON_RCOMPARE_FALSE;
		else if (compareType == Py_NE)
			PYTHON_RCOMPARE_TRUE;
		else
		{
			PyErr_SetString(PyExc_NotImplementedError, "invalid comparison for a ptKey object");
			PYTHON_RCOMPARE_ERROR;
		}
	}
	pyKey *key1 = pyKey::ConvertFrom(obj1);
	pyKey *key2 = pyKey::ConvertFrom(obj2);
	if (compareType == Py_EQ)
	{
		if ((*key1) == (*key2))
			PYTHON_RCOMPARE_TRUE;
		PYTHON_RCOMPARE_FALSE;
	}
	else if (compareType == Py_NE)
	{
		if ((*key1) != (*key2))
			PYTHON_RCOMPARE_TRUE;
		PYTHON_RCOMPARE_FALSE;
	}
	PyErr_SetString(PyExc_NotImplementedError, "invalid comparison for a ptKey object");
	PYTHON_RCOMPARE_ERROR;
}

#ifndef BUILDING_PYPLASMA
PYTHON_METHOD_DEFINITION(ptKey, netForce, args)
{
	char forceFlag;
	if (!PyArg_ParseTuple(args, "b", &forceFlag))
	{
		PyErr_SetString(PyExc_TypeError, "netForce requires a boolean argument");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetNetForce(forceFlag != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptKey, getName)
{
	return PyString_FromString(self->fThis->getName());
}

PYTHON_BASIC_METHOD_DEFINITION(ptKey, enable, Enable)
PYTHON_BASIC_METHOD_DEFINITION(ptKey, disable, Disable)

PYTHON_METHOD_DEFINITION_NOARGS(ptKey, getSceneObject)
{
	return self->fThis->GetPySceneObject();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptKey, getParentKey)
{
	PyObject* retVal = self->fThis->GetParentObject();
	if (!retVal)
		PYTHON_RETURN_NONE;
	return retVal;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptKey, isAttachedToClone)
{
	PYTHON_RETURN_BOOL(self->fThis->IsAttachedToClone());
}

#endif // BUILDING_PYPLASMA

PYTHON_START_METHODS_TABLE(ptKey)
#ifndef BUILDING_PYPLASMA
	PYTHON_METHOD(ptKey, netForce, "Params: forceFlag\nSpecify whether this object needs to use messages that are forced to the network\n"
				"- This is to be used if your Python program is running on only one client\n"
				"Such as a game master, only running on the client that owns a particular object"),
	PYTHON_METHOD_NOARGS(ptKey, getName, "Get the name of the object that this ptKey is pointing to"),
	PYTHON_BASIC_METHOD(ptKey, enable, "Sends an enable message to whatever this ptKey is pointing to"),
	PYTHON_BASIC_METHOD(ptKey, disable, "Sends a disable message to whatever this ptKey is pointing to"),
	PYTHON_METHOD_NOARGS(ptKey, getSceneObject, "This will return a ptSceneobject object that is associated with this ptKey\n"
				"However, if this ptKey is _not_ a sceneobject, then unpredicatable results will ensue"),
	PYTHON_METHOD_NOARGS(ptKey, getParentKey, "This will return a ptKey object that is the parent of this modifer\n"
				"However, if the parent is not a modifier or not loaded, then None is returned."),
	PYTHON_METHOD_NOARGS(ptKey, isAttachedToClone, "Returns whether the python file mod is attached to a clone"),
#endif // BUILDING_PYPLASMA
PYTHON_END_METHODS_TABLE;

// type structure definition
#define ptKey_COMPARE		PYTHON_NO_COMPARE
#define ptKey_AS_NUMBER		PYTHON_NO_AS_NUMBER
#define ptKey_AS_SEQUENCE	PYTHON_NO_AS_SEQUENCE
#define ptKey_AS_MAPPING	PYTHON_NO_AS_MAPPING
#define ptKey_STR			PYTHON_NO_STR
#define ptKey_RICH_COMPARE	PYTHON_DEFAULT_RICH_COMPARE(ptKey)
#define ptKey_GETSET		PYTHON_NO_GETSET
#define ptKey_BASE			PYTHON_NO_BASE
PLASMA_CUSTOM_TYPE(ptKey, "Plasma Key class");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptKey, pyKey)

PyObject *pyKey::New(plKey key)
{
	ptKey *newObj = (ptKey*)ptKey_type.tp_new(&ptKey_type, NULL, NULL);
	newObj->fThis->fKey = key;
#ifndef BUILDING_PYPLASMA
	newObj->fThis->fPyFileMod = nil;
	newObj->fThis->fNetForce = false;
#endif
	return (PyObject*)newObj;
}

PyObject *pyKey::New(pyKey *key)
{
	ptKey *newObj = (ptKey*)ptKey_type.tp_new(&ptKey_type, NULL, NULL);
	newObj->fThis->fKey = key->getKey();
#ifndef BUILDING_PYPLASMA
	newObj->fThis->fPyFileMod = nil;
	newObj->fThis->fNetForce = false;
#endif
	return (PyObject*)newObj;
}
#ifndef BUILDING_PYPLASMA
PyObject *pyKey::New(plKey key, plPythonFileMod* pymod)
{
	ptKey *newObj = (ptKey*)ptKey_type.tp_new(&ptKey_type, NULL, NULL);
	newObj->fThis->fKey = key;
	newObj->fThis->fPyFileMod = pymod;
	newObj->fThis->fNetForce = false;
	return (PyObject*)newObj;
}

PyObject *pyKey::New(pyKey *key, plPythonFileMod* pymod)
{
	ptKey *newObj = (ptKey*)ptKey_type.tp_new(&ptKey_type, NULL, NULL);
	newObj->fThis->fKey = key->getKey();
	newObj->fThis->fPyFileMod = pymod;
	newObj->fThis->fNetForce = false;
	return (PyObject*)newObj;
}
#endif // BUILDING_PYPLASMA

PYTHON_CLASS_CHECK_IMPL(ptKey, pyKey)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptKey, pyKey)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyKey::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptKey);
	PYTHON_CLASS_IMPORT_END(m);
}