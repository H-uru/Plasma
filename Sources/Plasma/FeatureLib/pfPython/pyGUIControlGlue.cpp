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
#include "pyGUIControl.h"
#include "pyKey.h"
#include "pyGeometry3.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptGUIControl, pyGUIControl);

PYTHON_DEFAULT_NEW_DEFINITION(ptGUIControl, pyGUIControl)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGUIControl)

PYTHON_INIT_DEFINITION(ptGUIControl, args, keywords)
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

PYTHON_RICH_COMPARE_DEFINITION(ptGUIControl, obj1, obj2, compareType)
{
	if ((obj1 == Py_None) || (obj2 == Py_None) || !pyGUIControl::Check(obj1) || !pyGUIControl::Check(obj2))
	{
		// if they aren't the same type, they don't match, obviously (we also never equal none)
		if (compareType == Py_EQ)
			PYTHON_RCOMPARE_FALSE;
		else if (compareType == Py_NE)
			PYTHON_RCOMPARE_TRUE;
		else
		{
			PyErr_SetString(PyExc_NotImplementedError, "invalid comparison for a ptGUIControl object");
			PYTHON_RCOMPARE_ERROR;
		}
	}
	pyGUIControl *ctrl1 = pyGUIControl::ConvertFrom(obj1);
	pyGUIControl *ctrl2 = pyGUIControl::ConvertFrom(obj2);
	if (compareType == Py_EQ)
	{
		if ((*ctrl1) == (*ctrl2))
			PYTHON_RCOMPARE_TRUE;
		PYTHON_RCOMPARE_FALSE;
	}
	else if (compareType == Py_NE)
	{
		if ((*ctrl1) != (*ctrl2))
			PYTHON_RCOMPARE_TRUE;
		PYTHON_RCOMPARE_FALSE;
	}
	PyErr_SetString(PyExc_NotImplementedError, "invalid comparison for a ptGUIControl object");
	PYTHON_RCOMPARE_ERROR;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControl, getKey)
{
	return self->fThis->getObjPyKey();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControl, getTagID)
{
	return PyLong_FromUnsignedLong(self->fThis->GetTagID());
}

PYTHON_METHOD_DEFINITION(ptGUIControl, enable, args)
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

PYTHON_BASIC_METHOD_DEFINITION(ptGUIControl, disable, Disable)

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControl, isEnabled)
{
	PYTHON_RETURN_BOOL(self->fThis->IsEnabled());
}

PYTHON_METHOD_DEFINITION(ptGUIControl, setFocus, args)
{
	char focusFlag;
	if (!PyArg_ParseTuple(args, "b", &focusFlag))
	{
		PyErr_SetString(PyExc_TypeError, "setFocus expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetFocused(focusFlag != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptGUIControl, focus, Focus)
PYTHON_BASIC_METHOD_DEFINITION(ptGUIControl, unFocus, UnFocus)

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControl, isFocused)
{
	PYTHON_RETURN_BOOL(self->fThis->IsFocused());
}

PYTHON_METHOD_DEFINITION(ptGUIControl, setVisible, args)
{
	char visible;
	if (!PyArg_ParseTuple(args, "b", &visible))
	{
		PyErr_SetString(PyExc_TypeError, "setVisible expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetVisible(visible != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptGUIControl, show, Show)
PYTHON_BASIC_METHOD_DEFINITION(ptGUIControl, hide, Hide)

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControl, isVisible)
{
	PYTHON_RETURN_BOOL(self->fThis->IsVisible());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControl, isInteresting)
{
	PYTHON_RETURN_BOOL(self->fThis->IsInteresting());
}

PYTHON_METHOD_DEFINITION(ptGUIControl, setNotifyOnInteresting, args)
{
	char notify;
	if (!PyArg_ParseTuple(args, "b", &notify))
	{
		PyErr_SetString(PyExc_TypeError, "setNotifyOnInteresting expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetNotifyOnInteresting(notify != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptGUIControl, refresh, Refresh)

PYTHON_METHOD_DEFINITION(ptGUIControl, setObjectCenter, args)
{
	PyObject* pointObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &pointObj))
	{
		PyErr_SetString(PyExc_TypeError, "setObjectCenter expects a ptPoint3");
		PYTHON_RETURN_ERROR;
	}
	if (!pyPoint3::Check(pointObj))
	{
		PyErr_SetString(PyExc_TypeError, "setObjectCenter expects a ptPoint3");
		PYTHON_RETURN_ERROR;
	}
	pyPoint3* point = pyPoint3::ConvertFrom(pointObj);
	self->fThis->SetObjectCenter(*point);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControl, getObjectCenter)
{
	return self->fThis->GetObjectCenter();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControl, getOwnerDialog)
{
	return self->fThis->GetOwnerDlg();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControl, getForeColor)
{
	return self->fThis->GetForeColor();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControl, getSelectColor)
{
	return self->fThis->GetSelColor();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControl, getBackColor)
{
	return self->fThis->GetBackColor();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControl, getBackSelectColor)
{
	return self->fThis->GetBackSelColor();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControl, getFontSize)
{
	return PyLong_FromUnsignedLong(self->fThis->GetFontSize());
}

PYTHON_METHOD_DEFINITION(ptGUIControl, setForeColor, args)
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

PYTHON_METHOD_DEFINITION(ptGUIControl, setSelectColor, args)
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

PYTHON_METHOD_DEFINITION(ptGUIControl, setBackColor, args)
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

PYTHON_METHOD_DEFINITION(ptGUIControl, setBackSelectColor, args)
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

PYTHON_METHOD_DEFINITION(ptGUIControl, setFontSize, args)
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

PYTHON_START_METHODS_TABLE(ptGUIControl)
	PYTHON_METHOD_NOARGS(ptGUIControl, getKey, "Returns the ptKey for this GUI control"),
	PYTHON_METHOD_NOARGS(ptGUIControl, getTagID, "Returns the Tag ID for this GUI control"),
	PYTHON_METHOD(ptGUIControl, enable, "Params: flag=1\nEnables this GUI control"),
	PYTHON_BASIC_METHOD(ptGUIControl, disable, "Disables this GUI control"),
	PYTHON_METHOD_NOARGS(ptGUIControl, isEnabled, "Returns whether this GUI control is enabled"),
	PYTHON_METHOD(ptGUIControl, setFocus, "Params: state\nSets the state of the focus of this GUI control"),
	PYTHON_BASIC_METHOD(ptGUIControl, focus, "Gets focus for this GUI control"),
	PYTHON_BASIC_METHOD(ptGUIControl, unFocus, "Releases focus for this GUI control"),
	PYTHON_METHOD_NOARGS(ptGUIControl, isFocused, "Returns whether this GUI control has focus"),
	PYTHON_METHOD(ptGUIControl, setVisible, "Params: state\nSets the state of visibility of this GUI control"),
	PYTHON_BASIC_METHOD(ptGUIControl, show, "Shows this GUI control"),
	PYTHON_BASIC_METHOD(ptGUIControl, hide, "Hides this GUI control"),
	PYTHON_METHOD(ptGUIControl, isVisible, "Returns whether this GUI control is visible"),
	PYTHON_METHOD_NOARGS(ptGUIControl, isInteresting, "Returns whether this GUI control is interesting at the moment"),
	PYTHON_METHOD(ptGUIControl, setNotifyOnInteresting, "Params: state\nSets whether this control should send interesting events or not"),
	PYTHON_BASIC_METHOD(ptGUIControl, refresh, "UNKNOWN"),
	PYTHON_METHOD(ptGUIControl, setObjectCenter, "Params: point\nSets the GUI controls object center to 'point'"),
	PYTHON_METHOD_NOARGS(ptGUIControl, getObjectCenter, "Returns ptPoint3 of the center of the GUI control object"),
	PYTHON_METHOD_NOARGS(ptGUIControl, getOwnerDialog, "Returns a ptGUIDialog of the dialog that owns this GUI control"),
	PYTHON_METHOD_NOARGS(ptGUIControl, getForeColor, "Returns the foreground color"),
	PYTHON_METHOD_NOARGS(ptGUIControl, getSelectColor, "Returns the selection color"),
	PYTHON_METHOD_NOARGS(ptGUIControl, getBackColor, "Returns the background color"),
	PYTHON_METHOD_NOARGS(ptGUIControl, getBackSelectColor, "Returns the background selection color"),
	PYTHON_METHOD_NOARGS(ptGUIControl, getFontSize, "Returns the font size"),
	PYTHON_METHOD(ptGUIControl, setForeColor, "Params: r,g,b,a\nSets the foreground color"),
	PYTHON_METHOD(ptGUIControl, setSelectColor, "Params: r,g,b,a\nSets the selection color"),
	PYTHON_METHOD(ptGUIControl, setBackColor, "Params: r,g,b,a\nSets the background color"),
	PYTHON_METHOD(ptGUIControl, setBackSelectColor, "Params: r,g,b,a\nSets the selection background color"),
	PYTHON_METHOD(ptGUIControl, setFontSize, "Params: fontSize\nSets the font size"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
#define ptGUIControl_COMPARE		PYTHON_NO_COMPARE
#define ptGUIControl_AS_NUMBER		PYTHON_NO_AS_NUMBER
#define ptGUIControl_AS_SEQUENCE	PYTHON_NO_AS_SEQUENCE
#define ptGUIControl_AS_MAPPING		PYTHON_NO_AS_MAPPING
#define ptGUIControl_STR			PYTHON_NO_STR
#define ptGUIControl_RICH_COMPARE	PYTHON_DEFAULT_RICH_COMPARE(ptGUIControl)
#define ptGUIControl_GETSET			PYTHON_NO_GETSET
#define ptGUIControl_BASE			PYTHON_NO_BASE
PLASMA_CUSTOM_TYPE(ptGUIControl, "Params: controlKey\nBase class for all GUI controls");
PYTHON_EXPOSE_TYPE_DEFINITION(ptGUIControl, pyGUIControl);

// required functions for PyObject interoperability
PyObject *pyGUIControl::New(pyKey& gckey)
{
	ptGUIControl *newObj = (ptGUIControl*)ptGUIControl_type.tp_new(&ptGUIControl_type, NULL, NULL);
	newObj->fThis->fGCkey = gckey.getKey();
	return (PyObject*)newObj;
}

PyObject *pyGUIControl::New(plKey objkey)
{
	ptGUIControl *newObj = (ptGUIControl*)ptGUIControl_type.tp_new(&ptGUIControl_type, NULL, NULL);
	newObj->fThis->fGCkey = objkey;
	return (PyObject*)newObj;
}

PyObject *pyGUIControl::New(const pyGUIControl& other)
{
	ptGUIControl *newObj = (ptGUIControl*)ptGUIControl_type.tp_new(&ptGUIControl_type, NULL, NULL);
	newObj->fThis->fGCkey = other.fGCkey;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGUIControl, pyGUIControl)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGUIControl, pyGUIControl)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyGUIControl::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptGUIControl);
	PYTHON_CLASS_IMPORT_END(m);
}