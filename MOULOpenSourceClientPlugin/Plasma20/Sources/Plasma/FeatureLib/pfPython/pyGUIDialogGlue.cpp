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
#include "pyGUIDialog.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptGUIDialog, pyGUIDialog);

PYTHON_DEFAULT_NEW_DEFINITION(ptGUIDialog, pyGUIDialog)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGUIDialog)

PYTHON_INIT_DEFINITION(ptGUIDialog, args, keywords)
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

PYTHON_RICH_COMPARE_DEFINITION(ptGUIDialog, obj1, obj2, compareType)
{
	if ((obj1 == Py_None) || (obj2 == Py_None) || !pyGUIDialog::Check(obj1) || !pyGUIDialog::Check(obj2))
	{
		// if they aren't the same type, they don't match, obviously (we also never equal none)
		if (compareType == Py_EQ)
			PYTHON_RCOMPARE_FALSE;
		else if (compareType == Py_NE)
			PYTHON_RCOMPARE_TRUE;
		else
		{
			PyErr_SetString(PyExc_NotImplementedError, "invalid comparison for a ptGUIDialog object");
			PYTHON_RCOMPARE_ERROR;
		}
	}
	pyGUIDialog *dlg1 = pyGUIDialog::ConvertFrom(obj1);
	pyGUIDialog *dlg2 = pyGUIDialog::ConvertFrom(obj2);
	if (compareType == Py_EQ)
	{
		if ((*dlg1) == (*dlg2))
			PYTHON_RCOMPARE_TRUE;
		PYTHON_RCOMPARE_FALSE;
	}
	else if (compareType == Py_NE)
	{
		if ((*dlg1) != (*dlg2))
			PYTHON_RCOMPARE_TRUE;
		PYTHON_RCOMPARE_FALSE;
	}
	PyErr_SetString(PyExc_NotImplementedError, "invalid comparison for a ptGUIDialog object");
	PYTHON_RCOMPARE_ERROR;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIDialog, getKey)
{
	return self->fThis->getObjPyKey();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIDialog, getTagID)
{
	return PyLong_FromUnsignedLong(self->fThis->GetTagID());
}

PYTHON_METHOD_DEFINITION(ptGUIDialog, enable, args)
{
	char enableFlag = 1;
	if (!PyArg_ParseTuple(args, "|b", &enableFlag))
	{
		PyErr_SetString(PyExc_TypeError, "enable expects an optional boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetEnabled(enableFlag != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptGUIDialog, disable, Disable)

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIDialog, isEnabled)
{
	PYTHON_RETURN_BOOL(self->fThis->IsEnabled());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIDialog, getName)
{
	return PyString_FromString(self->fThis->GetName());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIDialog, getVersion)
{
	return PyLong_FromUnsignedLong(self->fThis->GetVersion());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIDialog, getNumControls)
{
	return PyLong_FromUnsignedLong(self->fThis->GetNumControls());
}

PYTHON_METHOD_DEFINITION(ptGUIDialog, getControlFromIndex, args)
{
	unsigned long index;
	if (!PyArg_ParseTuple(args, "l", &index))
	{
		PyErr_SetString(PyExc_TypeError, "getControlFromIndex expects an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	return self->fThis->GetControl(index);
}

PYTHON_METHOD_DEFINITION(ptGUIDialog, setFocus, args)
{
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "setFocus expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "setFocus expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	self->fThis->SetFocus(*key);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptGUIDialog, noFocus, NoFocus)

PYTHON_BASIC_METHOD_DEFINITION(ptGUIDialog, show, Show)
PYTHON_BASIC_METHOD_DEFINITION(ptGUIDialog, showNoReset, ShowNoReset)
PYTHON_BASIC_METHOD_DEFINITION(ptGUIDialog, hide, Hide)

PYTHON_METHOD_DEFINITION(ptGUIDialog, getControlFromTag, args)
{
	unsigned long tagID;
	if (!PyArg_ParseTuple(args, "l", &tagID))
	{
		PyErr_SetString(PyExc_TypeError, "getControlFromTag expects an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	return self->fThis->GetControlFromTag(tagID);
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIDialog, getForeColor)
{
	return self->fThis->GetForeColor();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIDialog, getSelectColor)
{
	return self->fThis->GetSelColor();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIDialog, getBackColor)
{
	return self->fThis->GetBackColor();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIDialog, getBackSelectColor)
{
	return self->fThis->GetBackSelColor();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIDialog, getFontSize)
{
	return PyLong_FromUnsignedLong(self->fThis->GetFontSize());
}

PYTHON_METHOD_DEFINITION(ptGUIDialog, setForeColor, args)
{
	float r, g, b, a;
	if (!PyArg_ParseTuple(args, "ffff", &r, &g, &b, &a))
	{
		PyErr_SetString(PyExc_TypeError, "setForeColor expects four floats");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetForeColor(r, g, b, a);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIDialog, setSelectColor, args)
{
	float r, g, b, a;
	if (!PyArg_ParseTuple(args, "ffff", &r, &g, &b, &a))
	{
		PyErr_SetString(PyExc_TypeError, "setSelectColor expects four floats");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetSelColor(r, g, b, a);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIDialog, setBackColor, args)
{
	float r, g, b, a;
	if (!PyArg_ParseTuple(args, "ffff", &r, &g, &b, &a))
	{
		PyErr_SetString(PyExc_TypeError, "setBackColor expects four floats");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetBackColor(r, g, b, a);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIDialog, setBackSelectColor, args)
{
	float r, g, b, a;
	if (!PyArg_ParseTuple(args, "ffff", &r, &g, &b, &a))
	{
		PyErr_SetString(PyExc_TypeError, "setBackSelectColor expects four floats");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetBackSelColor(r, g, b, a);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIDialog, setFontSize, args)
{
	unsigned long fontSize;
	if (!PyArg_ParseTuple(args, "l", &fontSize))
	{
		PyErr_SetString(PyExc_TypeError, "setFontSize expects an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetFontSize(fontSize);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptGUIDialog, updateAllBounds, UpdateAllBounds)
PYTHON_BASIC_METHOD_DEFINITION(ptGUIDialog, refreshAllControls, RefreshAllControls)

PYTHON_START_METHODS_TABLE(ptGUIDialog)
	PYTHON_METHOD_NOARGS(ptGUIDialog, getKey, "Returns this dialog's ptKey"),
	PYTHON_METHOD_NOARGS(ptGUIDialog, getTagID, "Returns this dialog's tag ID"),
	PYTHON_METHOD(ptGUIDialog, enable, "Params: enableFlag=1\nEnable this dialog"),
	PYTHON_BASIC_METHOD(ptGUIDialog, disable, "Disables this dialog"),
	PYTHON_METHOD_NOARGS(ptGUIDialog, isEnabled, "Is this dialog currently enabled?"),
	PYTHON_METHOD_NOARGS(ptGUIDialog, getName, "Returns the dialog's name"),
	PYTHON_METHOD_NOARGS(ptGUIDialog, getVersion, "UNKNOWN"),
	PYTHON_METHOD_NOARGS(ptGUIDialog, getNumControls, "Returns the number of controls in this dialog"),
	PYTHON_METHOD(ptGUIDialog, getControlFromIndex, "Params: index\nReturns the ptKey of the control with the specified index (not tag ID!)"),
	PYTHON_METHOD(ptGUIDialog, setFocus, "Params: ctrlKey\nSets the control that has input focus"),
	PYTHON_BASIC_METHOD(ptGUIDialog, noFocus, "Makes sure no control has input focus"),
	PYTHON_BASIC_METHOD(ptGUIDialog, show, "Shows the dialog"),
	PYTHON_BASIC_METHOD(ptGUIDialog, showNoReset, "Show dialog without resetting clickables"),
	PYTHON_BASIC_METHOD(ptGUIDialog, hide, "Hides the dialog"),
	PYTHON_METHOD(ptGUIDialog, getControlFromTag, "Params: tagID\nReturns the ptKey of the control with the specified tag ID"),
	PYTHON_METHOD_NOARGS(ptGUIDialog, getForeColor, "Returns the fore color as a ptColor object"),
	PYTHON_METHOD_NOARGS(ptGUIDialog, getSelectColor, "Returns the select color as a ptColor object"),
	PYTHON_METHOD_NOARGS(ptGUIDialog, getBackColor, "Returns the back color as a ptColor object"),
	PYTHON_METHOD_NOARGS(ptGUIDialog, getBackSelectColor, "Returns the select back color as a ptColor object"),
	PYTHON_METHOD_NOARGS(ptGUIDialog, getFontSize, "Returns the font size"),
	PYTHON_METHOD(ptGUIDialog, setForeColor, "Params: red,green,blue,alpha\nSets the fore color, -1 means don't change"),
	PYTHON_METHOD(ptGUIDialog, setSelectColor, "Params: red,green,blue,alpha\nSets the select color, -1 means don't change"),
	PYTHON_METHOD(ptGUIDialog, setBackColor, "Params: red,green,blue,alpha\nSets the back color, -1 means don't change"),
	PYTHON_METHOD(ptGUIDialog, setBackSelectColor, "Params: red,green,blue,alpha\nSets the select back color, -1 means don't change"),
	PYTHON_METHOD(ptGUIDialog, setFontSize, "Params: fontSize\nSets the font size"),
	PYTHON_BASIC_METHOD(ptGUIDialog, updateAllBounds, "Tells the dialog to recompute all the bounds for its controls"),
	PYTHON_BASIC_METHOD(ptGUIDialog, refreshAllControls, "Tells the dialog to redraw all its controls"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
#define ptGUIDialog_COMPARE			PYTHON_NO_COMPARE
#define ptGUIDialog_AS_NUMBER		PYTHON_NO_AS_NUMBER
#define ptGUIDialog_AS_SEQUENCE		PYTHON_NO_AS_SEQUENCE
#define ptGUIDialog_AS_MAPPING		PYTHON_NO_AS_MAPPING
#define ptGUIDialog_STR				PYTHON_NO_STR
#define ptGUIDialog_RICH_COMPARE	PYTHON_DEFAULT_RICH_COMPARE(ptGUIDialog)
#define ptGUIDialog_GETSET			PYTHON_NO_GETSET
#define ptGUIDialog_BASE			PYTHON_NO_BASE
PLASMA_CUSTOM_TYPE(ptGUIDialog, "Params: dialogKey\nPlasma GUI dialog class");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptGUIDialog, pyGUIDialog)

PyObject *pyGUIDialog::New(pyKey& gckey)
{
	ptGUIDialog *newObj = (ptGUIDialog*)ptGUIDialog_type.tp_new(&ptGUIDialog_type, NULL, NULL);
	newObj->fThis->fGCkey = gckey.getKey();
	return (PyObject*)newObj;
}

PyObject *pyGUIDialog::New(plKey objkey)
{
	ptGUIDialog *newObj = (ptGUIDialog*)ptGUIDialog_type.tp_new(&ptGUIDialog_type, NULL, NULL);
	newObj->fThis->fGCkey = objkey;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGUIDialog, pyGUIDialog)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGUIDialog, pyGUIDialog)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyGUIDialog::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptGUIDialog);
	PYTHON_CLASS_IMPORT_END(m);
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtWhatGUIControlType, args, "Params: guiKey\nReturns the control type of the key passed in")
{
	PyObject* guiKeyObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &guiKeyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtWhatGUIControlType expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(guiKeyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtWhatGUIControlType expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	pyKey* guiKey = pyKey::ConvertFrom(guiKeyObj);
	return PyLong_FromUnsignedLong(pyGUIDialog::WhatControlType(*guiKey));
}

PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtGUICursorOff, pyGUIDialog::GUICursorOff, "Turns the GUI cursor off")
PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtGUICursorOn, pyGUIDialog::GUICursorOn, "Turns the GUI cursor on")
PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtGUICursorDimmed, pyGUIDialog::GUICursorDimmed, "Dimms the GUI cursor")

void pyGUIDialog::AddPlasmaMethods(std::vector<PyMethodDef> &methods)
{
	PYTHON_GLOBAL_METHOD(methods, PtWhatGUIControlType);
	PYTHON_BASIC_GLOBAL_METHOD(methods, PtGUICursorOff);
	PYTHON_BASIC_GLOBAL_METHOD(methods, PtGUICursorOn);
	PYTHON_BASIC_GLOBAL_METHOD(methods, PtGUICursorDimmed);
}