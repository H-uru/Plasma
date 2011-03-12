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
#include "pyGUIControlButton.h"
#include "pyEnum.h"

#include "../pfGameGUIMgr/pfGUIButtonMod.h"
#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptGUIControlButton, pyGUIControlButton);

PYTHON_DEFAULT_NEW_DEFINITION(ptGUIControlButton, pyGUIControlButton)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGUIControlButton)

PYTHON_INIT_DEFINITION(ptGUIControlButton, args, keywords)
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

PYTHON_METHOD_DEFINITION(ptGUIControlButton, setNotifyType, args)
{
	long kind;
	if (!PyArg_ParseTuple(args, "l", &kind))
	{
		PyErr_SetString(PyExc_TypeError, "setNotifyType expects a long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetNotifyType(kind);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlButton, getNotifyType)
{
	return PyLong_FromLong(self->fThis->GetNotifyType());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlButton, isButtonDown)
{
	PYTHON_RETURN_BOOL(self->fThis->IsButtonDown());
}

PYTHON_START_METHODS_TABLE(ptGUIControlButton)
	PYTHON_METHOD(ptGUIControlButton, setNotifyType, "Params: kind\nSets this button's notify type. See PtButtonNotifyTypes"),
	PYTHON_METHOD_NOARGS(ptGUIControlButton, getNotifyType, "Returns this button's notify type. See PtButtonNotifyTypes"),
	PYTHON_METHOD_NOARGS(ptGUIControlButton, isButtonDown, "Is the button down? Returns 1 for true otherwise returns 0"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptGUIControlButton, pyGUIControl, "Params: ctrlKey\nPlasma GUI Control Button class");

// required functions for PyObject interoperability
PyObject *pyGUIControlButton::New(pyKey& gckey)
{
	ptGUIControlButton *newObj = (ptGUIControlButton*)ptGUIControlButton_type.tp_new(&ptGUIControlButton_type, NULL, NULL);
	newObj->fThis->fGCkey = gckey.getKey();
	return (PyObject*)newObj;
}

PyObject *pyGUIControlButton::New(plKey objkey)
{
	ptGUIControlButton *newObj = (ptGUIControlButton*)ptGUIControlButton_type.tp_new(&ptGUIControlButton_type, NULL, NULL);
	newObj->fThis->fGCkey = objkey;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGUIControlButton, pyGUIControlButton)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGUIControlButton, pyGUIControlButton)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyGUIControlButton::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptGUIControlButton);
	PYTHON_CLASS_IMPORT_END(m);
}

void pyGUIControlButton::AddPlasmaConstantsClasses(PyObject *m)
{
	PYTHON_ENUM_START(PtButtonNotifyTypes);
	PYTHON_ENUM_ELEMENT(PtButtonNotifyTypes, kNotifyOnUp,			pfGUIButtonMod::kNotifyOnUp);
	PYTHON_ENUM_ELEMENT(PtButtonNotifyTypes, kNotifyOnDown,			pfGUIButtonMod::kNotifyOnDown);
	PYTHON_ENUM_ELEMENT(PtButtonNotifyTypes, kNotifyOnUpAndDown,	pfGUIButtonMod::kNotifyOnUpAndDown);
	PYTHON_ENUM_END(m, PtButtonNotifyTypes);
}