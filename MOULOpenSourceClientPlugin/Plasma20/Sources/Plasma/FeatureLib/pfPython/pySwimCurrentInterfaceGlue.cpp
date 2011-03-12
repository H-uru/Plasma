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
#include "pySwimCurrentInterface.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptSwimCurrentInterface, pySwimCurrentInterface);

PYTHON_DEFAULT_NEW_DEFINITION(ptSwimCurrentInterface, pySwimCurrentInterface)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptSwimCurrentInterface)

PYTHON_INIT_DEFINITION(ptSwimCurrentInterface, args, keywords)
{
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "__init__ expects a ptKey");
		PYTHON_RETURN_INIT_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "__init__ expects a ptKey");
		PYTHON_RETURN_INIT_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	self->fThis->setKey(*key);
	PYTHON_RETURN_INIT_OK;
}

PYTHON_BASIC_METHOD_DEFINITION(ptSwimCurrentInterface, enable, enable)
PYTHON_BASIC_METHOD_DEFINITION(ptSwimCurrentInterface, disable, disable)

PYTHON_START_METHODS_TABLE(ptSwimCurrentInterface)
	PYTHON_BASIC_METHOD(ptSwimCurrentInterface, enable, "UNKNOWN"),
	PYTHON_BASIC_METHOD(ptSwimCurrentInterface, disable, "UNKNOWN"),
PYTHON_END_METHODS_TABLE;

PYTHON_GET_DEFINITION(ptSwimCurrentInterface, nearDistance)
{
	return PyFloat_FromDouble(self->fThis->getNearDist());
}

PYTHON_SET_DEFINITION(ptSwimCurrentInterface, nearDistance, newValue)
{
	if (!newValue)
	{
		PyErr_SetString(PyExc_TypeError, "nearDistance cannot be deleted");
		PYTHON_RETURN_SET_ERROR;
	}
	if (!PyFloat_Check(newValue))
	{
		PyErr_SetString(PyExc_TypeError, "nearDistance expects a floating point value");
		PYTHON_RETURN_SET_ERROR;
	}
	float val = (float)PyFloat_AsDouble(newValue);
	self->fThis->setNearDist(val);
	PYTHON_RETURN_SET_OK;
}

PYTHON_GET_DEFINITION(ptSwimCurrentInterface, farDistance)
{
	return PyFloat_FromDouble(self->fThis->getFarDist());
}

PYTHON_SET_DEFINITION(ptSwimCurrentInterface, farDistance, newValue)
{
	if (!newValue)
	{
		PyErr_SetString(PyExc_TypeError, "farDistance cannot be deleted");
		PYTHON_RETURN_SET_ERROR;
	}
	if (!PyFloat_Check(newValue))
	{
		PyErr_SetString(PyExc_TypeError, "farDistance expects a floating point value");
		PYTHON_RETURN_SET_ERROR;
	}
	float val = (float)PyFloat_AsDouble(newValue);
	self->fThis->setFarDist(val);
	PYTHON_RETURN_SET_OK;
}

PYTHON_GET_DEFINITION(ptSwimCurrentInterface, nearVelocity)
{
	return PyFloat_FromDouble(self->fThis->getNearVel());
}

PYTHON_SET_DEFINITION(ptSwimCurrentInterface, nearVelocity, newValue)
{
	if (!newValue)
	{
		PyErr_SetString(PyExc_TypeError, "nearVelocity cannot be deleted");
		PYTHON_RETURN_SET_ERROR;
	}
	if (!PyFloat_Check(newValue))
	{
		PyErr_SetString(PyExc_TypeError, "nearVelocity expects a floating point value");
		PYTHON_RETURN_SET_ERROR;
	}
	float val = (float)PyFloat_AsDouble(newValue);
	self->fThis->setNearVel(val);
	PYTHON_RETURN_SET_OK;
}

PYTHON_GET_DEFINITION(ptSwimCurrentInterface, farVelocity)
{
	return PyFloat_FromDouble(self->fThis->getFarVel());
}

PYTHON_SET_DEFINITION(ptSwimCurrentInterface, farVelocity, newValue)
{
	if (!newValue)
	{
		PyErr_SetString(PyExc_TypeError, "farVelocity cannot be deleted");
		PYTHON_RETURN_SET_ERROR;
	}
	if (!PyFloat_Check(newValue))
	{
		PyErr_SetString(PyExc_TypeError, "farVelocity expects a floating point value");
		PYTHON_RETURN_SET_ERROR;
	}
	float val = (float)PyFloat_AsDouble(newValue);
	self->fThis->setFarVel(val);
	PYTHON_RETURN_SET_OK;
}

PYTHON_GET_DEFINITION(ptSwimCurrentInterface, rotation)
{
	return PyFloat_FromDouble(self->fThis->getRotation());
}

PYTHON_SET_DEFINITION(ptSwimCurrentInterface, rotation, newValue)
{
	if (!newValue)
	{
		PyErr_SetString(PyExc_TypeError, "rotation cannot be deleted");
		PYTHON_RETURN_SET_ERROR;
	}
	if (!PyFloat_Check(newValue))
	{
		PyErr_SetString(PyExc_TypeError, "rotation expects a floating point value");
		PYTHON_RETURN_SET_ERROR;
	}
	float val = (float)PyFloat_AsDouble(newValue);
	self->fThis->setRotation(val);
	PYTHON_RETURN_SET_OK;
}

PYTHON_START_GETSET_TABLE(ptSwimCurrentInterface)
	PYTHON_GETSET(ptSwimCurrentInterface, nearDistance, "UNKNOWN"),
	PYTHON_GETSET(ptSwimCurrentInterface, farDistance, "UNKNOWN"),
	PYTHON_GETSET(ptSwimCurrentInterface, nearVelocity, "UNKNOWN"),
	PYTHON_GETSET(ptSwimCurrentInterface, farVelocity, "UNKNOWN"),
	PYTHON_GETSET(ptSwimCurrentInterface, rotation, "UNKNOWN"),
PYTHON_END_GETSET_TABLE;

// Type structure definition
#define ptSwimCurrentInterface_COMPARE			PYTHON_NO_COMPARE
#define ptSwimCurrentInterface_AS_NUMBER		PYTHON_NO_AS_NUMBER
#define ptSwimCurrentInterface_AS_SEQUENCE		PYTHON_NO_AS_SEQUENCE
#define ptSwimCurrentInterface_AS_MAPPING		PYTHON_NO_AS_MAPPING
#define ptSwimCurrentInterface_STR				PYTHON_NO_STR
#define ptSwimCurrentInterface_RICH_COMPARE		PYTHON_NO_RICH_COMPARE
#define ptSwimCurrentInterface_GETSET			PYTHON_DEFAULT_GETSET(ptSwimCurrentInterface)
#define ptSwimCurrentInterface_BASE				PYTHON_NO_BASE
PLASMA_CUSTOM_TYPE(ptSwimCurrentInterface, "Params: key\nCreates a new ptSwimCurrentInterface");

// required functions for PyObject interoperability
PyObject *pySwimCurrentInterface::New(plKey key)
{
	ptSwimCurrentInterface *newObj = (ptSwimCurrentInterface*)ptSwimCurrentInterface_type.tp_new(&ptSwimCurrentInterface_type, NULL, NULL);
	newObj->fThis->fSwimCurrentKey = key;
	return (PyObject*)newObj;
}

PyObject *pySwimCurrentInterface::New(pyKey& key)
{
	ptSwimCurrentInterface *newObj = (ptSwimCurrentInterface*)ptSwimCurrentInterface_type.tp_new(&ptSwimCurrentInterface_type, NULL, NULL);
	newObj->fThis->fSwimCurrentKey = key.getKey();
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptSwimCurrentInterface, pySwimCurrentInterface)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptSwimCurrentInterface, pySwimCurrentInterface)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pySwimCurrentInterface::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptSwimCurrentInterface);
	PYTHON_CLASS_IMPORT_END(m);
}