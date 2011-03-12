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
#include "pyGUIControlCheckBox.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptGUIControlCheckBox, pyGUIControlCheckBox);

PYTHON_DEFAULT_NEW_DEFINITION(ptGUIControlCheckBox, pyGUIControlCheckBox)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGUIControlCheckBox)

PYTHON_INIT_DEFINITION(ptGUIControlCheckBox, args, keywords)
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

PYTHON_METHOD_DEFINITION(ptGUIControlCheckBox, setChecked, args)
{
	char checkedState;
	if (!PyArg_ParseTuple(args, "b", &checkedState))
	{
		PyErr_SetString(PyExc_TypeError, "setChecked expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetChecked(checkedState != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlCheckBox, isChecked)
{
	PYTHON_RETURN_BOOL(self->fThis->IsChecked());
}

PYTHON_START_METHODS_TABLE(ptGUIControlCheckBox)
	PYTHON_METHOD(ptGUIControlCheckBox, setChecked, "Params: checkedState\nSets this checkbox to the 'checkedState'"),
	PYTHON_METHOD_NOARGS(ptGUIControlCheckBox, isChecked, "Is this checkbox checked? Returns 1 for true otherwise returns 0"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptGUIControlCheckBox, pyGUIControl, "Params: ctrlKey\nPlasma GUI Control Checkbox class");

// required functions for PyObject interoperability
PyObject *pyGUIControlCheckBox::New(pyKey& gckey)
{
	ptGUIControlCheckBox *newObj = (ptGUIControlCheckBox*)ptGUIControlCheckBox_type.tp_new(&ptGUIControlCheckBox_type, NULL, NULL);
	newObj->fThis->fGCkey = gckey.getKey();
	return (PyObject*)newObj;
}

PyObject *pyGUIControlCheckBox::New(plKey objkey)
{
	ptGUIControlCheckBox *newObj = (ptGUIControlCheckBox*)ptGUIControlCheckBox_type.tp_new(&ptGUIControlCheckBox_type, NULL, NULL);
	newObj->fThis->fGCkey = objkey;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGUIControlCheckBox, pyGUIControlCheckBox)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGUIControlCheckBox, pyGUIControlCheckBox)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyGUIControlCheckBox::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptGUIControlCheckBox);
	PYTHON_CLASS_IMPORT_END(m);
}