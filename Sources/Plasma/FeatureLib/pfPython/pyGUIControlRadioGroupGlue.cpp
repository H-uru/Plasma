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
#include "pyGUIControlRadioGroup.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptGUIControlRadioGroup, pyGUIControlRadioGroup);

PYTHON_DEFAULT_NEW_DEFINITION(ptGUIControlRadioGroup, pyGUIControlRadioGroup)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGUIControlRadioGroup)

PYTHON_INIT_DEFINITION(ptGUIControlRadioGroup, args, keywords)
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

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlRadioGroup, getValue)
{
	return PyLong_FromLong(self->fThis->GetValue());
}

PYTHON_METHOD_DEFINITION(ptGUIControlRadioGroup, setValue, args)
{
	long val;
	if (!PyArg_ParseTuple(args, "l", &val))
	{
		PyErr_SetString(PyExc_TypeError, "setValue expects a long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetValue(val);
	PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptGUIControlRadioGroup)
	PYTHON_METHOD_NOARGS(ptGUIControlRadioGroup, getValue, "Returns the current selection of the radio group."),
	PYTHON_METHOD(ptGUIControlRadioGroup, setValue, "Params: value\nSets the current selection to 'value'"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptGUIControlRadioGroup, pyGUIControl, "Params: ctrlKey\nPlasma GUI Control Radio Group class");

// required functions for PyObject interoperability
PyObject *pyGUIControlRadioGroup::New(pyKey& gckey)
{
	ptGUIControlRadioGroup *newObj = (ptGUIControlRadioGroup*)ptGUIControlRadioGroup_type.tp_new(&ptGUIControlRadioGroup_type, NULL, NULL);
	newObj->fThis->fGCkey = gckey.getKey();
	return (PyObject*)newObj;
}

PyObject *pyGUIControlRadioGroup::New(plKey objkey)
{
	ptGUIControlRadioGroup *newObj = (ptGUIControlRadioGroup*)ptGUIControlRadioGroup_type.tp_new(&ptGUIControlRadioGroup_type, NULL, NULL);
	newObj->fThis->fGCkey = objkey;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGUIControlRadioGroup, pyGUIControlRadioGroup)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGUIControlRadioGroup, pyGUIControlRadioGroup)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyGUIControlRadioGroup::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptGUIControlRadioGroup);
	PYTHON_CLASS_IMPORT_END(m);
}