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
#include "pyGUIControlDragBar.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptGUIControlDragBar, pyGUIControlDragBar);

PYTHON_DEFAULT_NEW_DEFINITION(ptGUIControlDragBar, pyGUIControlDragBar)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGUIControlDragBar)

PYTHON_INIT_DEFINITION(ptGUIControlDragBar, args, keywords)
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

PYTHON_BASIC_METHOD_DEFINITION(ptGUIControlDragBar, anchor, Anchor)
PYTHON_BASIC_METHOD_DEFINITION(ptGUIControlDragBar, unanchor, Unanchor)

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlDragBar, isAnchored)
{
	PYTHON_RETURN_BOOL(self->fThis->IsAnchored());
}

PYTHON_START_METHODS_TABLE(ptGUIControlDragBar)
	PYTHON_BASIC_METHOD(ptGUIControlDragBar, anchor, "Don't allow this dragbar object to be moved by the user.\nDrop anchor!"),
	PYTHON_BASIC_METHOD(ptGUIControlDragBar, unanchor, "Allow the user to drag this control around the screen.\nRaise anchor."),
	PYTHON_METHOD_NOARGS(ptGUIControlDragBar, isAnchored, "Is this dragbar control anchored? Returns 1 if true otherwise returns 0"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptGUIControlDragBar, pyGUIControl, "Params: ctrlKey\nPlasma GUI Control DragBar class");

// required functions for PyObject interoperability
PyObject *pyGUIControlDragBar::New(pyKey& gckey)
{
	ptGUIControlDragBar *newObj = (ptGUIControlDragBar*)ptGUIControlDragBar_type.tp_new(&ptGUIControlDragBar_type, NULL, NULL);
	newObj->fThis->fGCkey = gckey.getKey();
	return (PyObject*)newObj;
}

PyObject *pyGUIControlDragBar::New(plKey objkey)
{
	ptGUIControlDragBar *newObj = (ptGUIControlDragBar*)ptGUIControlDragBar_type.tp_new(&ptGUIControlDragBar_type, NULL, NULL);
	newObj->fThis->fGCkey = objkey;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGUIControlDragBar, pyGUIControlDragBar)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGUIControlDragBar, pyGUIControlDragBar)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyGUIControlDragBar::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptGUIControlDragBar);
	PYTHON_CLASS_IMPORT_END(m);
}