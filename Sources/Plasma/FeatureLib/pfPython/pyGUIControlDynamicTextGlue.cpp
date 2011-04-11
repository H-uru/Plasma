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
#include "pyGUIControlDynamicText.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptGUIControlDynamicText, pyGUIControlDynamicText);

PYTHON_DEFAULT_NEW_DEFINITION(ptGUIControlDynamicText, pyGUIControlDynamicText)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGUIControlDynamicText)

PYTHON_INIT_DEFINITION(ptGUIControlDynamicText, args, keywords)
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

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlDynamicText, getNumMaps)
{
	return PyLong_FromUnsignedLong(self->fThis->GetNumMaps());
}

PYTHON_METHOD_DEFINITION(ptGUIControlDynamicText, getMap, args)
{
	unsigned long i;
	if (!PyArg_ParseTuple(args, "l", &i))
	{
		PyErr_SetString(PyExc_KeyError, "getMap expects an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	return self->fThis->GetMap(i);
}

PYTHON_START_METHODS_TABLE(ptGUIControlDynamicText)
	PYTHON_METHOD_NOARGS(ptGUIControlDynamicText, getNumMaps, "Returns the number of ptDynamicText maps attached"),
	PYTHON_METHOD(ptGUIControlDynamicText, getMap, "Params: index\nReturns a specific ptDynamicText attached to this contol\n"
				"If there is no map at 'index' then a KeyError exception will be raised"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptGUIControlDynamicText, pyGUIControl, "Params: ctrlKey\nPlasma GUI Control DynamicText class");

// required functions for PyObject interoperability
PyObject *pyGUIControlDynamicText::New(pyKey& gckey)
{
	ptGUIControlDynamicText *newObj = (ptGUIControlDynamicText*)ptGUIControlDynamicText_type.tp_new(&ptGUIControlDynamicText_type, NULL, NULL);
	newObj->fThis->fGCkey = gckey.getKey();
	return (PyObject*)newObj;
}

PyObject *pyGUIControlDynamicText::New(plKey objkey)
{
	ptGUIControlDynamicText *newObj = (ptGUIControlDynamicText*)ptGUIControlDynamicText_type.tp_new(&ptGUIControlDynamicText_type, NULL, NULL);
	newObj->fThis->fGCkey = objkey;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGUIControlDynamicText, pyGUIControlDynamicText)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGUIControlDynamicText, pyGUIControlDynamicText)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyGUIControlDynamicText::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptGUIControlDynamicText);
	PYTHON_CLASS_IMPORT_END(m);
}