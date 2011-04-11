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
#include "pyGUIControlDraggable.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptGUIControlDraggable, pyGUIControlDraggable);

PYTHON_DEFAULT_NEW_DEFINITION(ptGUIControlDraggable, pyGUIControlDraggable)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGUIControlDraggable)

PYTHON_INIT_DEFINITION(ptGUIControlDraggable, args, keywords)
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

PYTHON_METHOD_DEFINITION(ptGUIControlDraggable, stopDragging, args)
{
	char cancelFlag;
	if (!PyArg_ParseTuple(args, "b", &cancelFlag))
	{
		PyErr_SetString(PyExc_TypeError, "stopDragging expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->StopDragging(cancelFlag != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlDraggable, getLastMousePoint)
{
	return self->fThis->GetLastMousePt();
}

PYTHON_START_METHODS_TABLE(ptGUIControlDraggable)
	PYTHON_METHOD(ptGUIControlDraggable, stopDragging, "Params: cancelFlag\nUNKNOWN"),
	PYTHON_METHOD_NOARGS(ptGUIControlDraggable, getLastMousePoint, "Returns the last point we were dragged to"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptGUIControlDraggable, pyGUIControl, "Params: ctrlKey\nPlasma GUI control for something draggable");

// required functions for PyObject interoperability
PyObject *pyGUIControlDraggable::New(pyKey& gckey)
{
	ptGUIControlDraggable *newObj = (ptGUIControlDraggable*)ptGUIControlDraggable_type.tp_new(&ptGUIControlDraggable_type, NULL, NULL);
	newObj->fThis->fGCkey = gckey.getKey();
	return (PyObject*)newObj;
}

PyObject *pyGUIControlDraggable::New(plKey objkey)
{
	ptGUIControlDraggable *newObj = (ptGUIControlDraggable*)ptGUIControlDraggable_type.tp_new(&ptGUIControlDraggable_type, NULL, NULL);
	newObj->fThis->fGCkey = objkey;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGUIControlDraggable, pyGUIControlDraggable)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGUIControlDraggable, pyGUIControlDraggable)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyGUIControlDraggable::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptGUIControlDraggable);
	PYTHON_CLASS_IMPORT_END(m);
}