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
#include "pyGUISkin.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptGUISkin, pyGUISkin);

PYTHON_DEFAULT_NEW_DEFINITION(ptGUISkin, pyGUISkin)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGUISkin)

PYTHON_INIT_DEFINITION(ptGUISkin, args, keywords)
{
	PyObject *keyObject = NULL;
	if (!PyArg_ParseTuple(args, "O", &keyObject))
	{
		PyErr_SetString(PyExc_TypeError, "__init__ expects a ptKey");
		PYTHON_RETURN_INIT_ERROR;
	}
	if (!pyKey::Check(keyObject))
	{
		PyErr_SetString(PyExc_TypeError, "__init__ expects a ptKey");
		PYTHON_RETURN_INIT_ERROR;
	}

	pyKey *key = pyKey::ConvertFrom(keyObject);
	self->fThis->setKey(key->getKey());

	PYTHON_RETURN_INIT_OK;
}

PYTHON_RICH_COMPARE_DEFINITION(ptGUISkin, obj1, obj2, compareType)
{
	if ((obj1 == Py_None) || (obj2 == Py_None) || !pyGUISkin::Check(obj1) || !pyGUISkin::Check(obj2))
	{
		// if they aren't the same type, they don't match, obviously (we also never equal none)
		if (compareType == Py_EQ)
			PYTHON_RCOMPARE_FALSE;
		else if (compareType == Py_NE)
			PYTHON_RCOMPARE_TRUE;
		else
		{
			PyErr_SetString(PyExc_NotImplementedError, "invalid comparison for a ptGUISkin object");
			PYTHON_RCOMPARE_ERROR;
		}
	}
	pyGUISkin *skin1 = pyGUISkin::ConvertFrom(obj1);
	pyGUISkin *skin2 = pyGUISkin::ConvertFrom(obj2);
	if (compareType == Py_EQ)
	{
		if ((*skin1) == (*skin2))
			PYTHON_RCOMPARE_TRUE;
		PYTHON_RCOMPARE_FALSE;
	}
	else if (compareType == Py_NE)
	{
		if ((*skin1) != (*skin2))
			PYTHON_RCOMPARE_TRUE;
		PYTHON_RCOMPARE_FALSE;
	}
	PyErr_SetString(PyExc_NotImplementedError, "invalid comparison for a ptGUISkin object");
	PYTHON_RCOMPARE_ERROR;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUISkin, getKey)
{
	return self->fThis->getObjPyKey();
}

PYTHON_START_METHODS_TABLE(ptGUISkin)
	PYTHON_METHOD_NOARGS(ptGUISkin, getKey, "Returns this object's ptKey"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
#define ptGUISkin_COMPARE		PYTHON_NO_COMPARE
#define ptGUISkin_AS_NUMBER		PYTHON_NO_AS_NUMBER
#define ptGUISkin_AS_SEQUENCE	PYTHON_NO_AS_SEQUENCE
#define ptGUISkin_AS_MAPPING	PYTHON_NO_AS_MAPPING
#define ptGUISkin_STR			PYTHON_NO_STR
#define ptGUISkin_RICH_COMPARE	PYTHON_DEFAULT_RICH_COMPARE(ptGUISkin)
#define ptGUISkin_GETSET		PYTHON_NO_GETSET
#define ptGUISkin_BASE			PYTHON_NO_BASE
PLASMA_CUSTOM_TYPE(ptGUISkin, "Params: key\nPlasma GUI Skin object");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptGUISkin, pyGUISkin)

PyObject *pyGUISkin::New(pyKey& gckey)
{
	ptGUISkin *newObj = (ptGUISkin*)ptGUISkin_type.tp_new(&ptGUISkin_type, NULL, NULL);
	newObj->fThis->fGCkey = gckey.getKey();
	return (PyObject*)newObj;
}

PyObject *pyGUISkin::New(plKey objkey)
{
	ptGUISkin *newObj = (ptGUISkin*)ptGUISkin_type.tp_new(&ptGUISkin_type, NULL, NULL);
	newObj->fThis->fGCkey = objkey;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGUISkin, pyGUISkin)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGUISkin, pyGUISkin)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyGUISkin::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptGUISkin);
	PYTHON_CLASS_IMPORT_END(m);
}