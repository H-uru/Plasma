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
#include "pyGUIControlValue.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptGUIControlValue, pyGUIControlValue);

PYTHON_DEFAULT_NEW_DEFINITION(ptGUIControlValue, pyGUIControlValue)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGUIControlValue)

PYTHON_INIT_DEFINITION(ptGUIControlValue, args, keywords)
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

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlValue, getValue)
{
	return PyFloat_FromDouble(self->fThis->GetValue());
}

PYTHON_METHOD_DEFINITION(ptGUIControlValue, setValue, args)
{
	float val;
	if (!PyArg_ParseTuple(args, "f", &val))
	{
		PyErr_SetString(PyExc_TypeError, "setValue expects a float");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetValue(val);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlValue, getMin)
{
	return PyFloat_FromDouble(self->fThis->GetMin());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlValue, getMax)
{
	return PyFloat_FromDouble(self->fThis->GetMax());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIControlValue, getStep)
{
	return PyFloat_FromDouble(self->fThis->GetStep());
}

PYTHON_METHOD_DEFINITION(ptGUIControlValue, setRange, args)
{
	float min, max;
	if (!PyArg_ParseTuple(args, "ff", &min, &max))
	{
		PyErr_SetString(PyExc_TypeError, "setRange expects two floats");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetRange(min, max);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIControlValue, setStep, args)
{
	float val;
	if (!PyArg_ParseTuple(args, "f", &val))
	{
		PyErr_SetString(PyExc_TypeError, "setStep expects a float");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetStep(val);
	PYTHON_RETURN_NONE;
}


PYTHON_START_METHODS_TABLE(ptGUIControlValue)
	PYTHON_METHOD_NOARGS(ptGUIControlValue, getValue, "Returns the current value of the control."),
	PYTHON_METHOD(ptGUIControlValue, setValue, "Params: value\nSets the current value of the control."),
	PYTHON_METHOD_NOARGS(ptGUIControlValue, getMin, "Returns the minimum of the control."),
	PYTHON_METHOD_NOARGS(ptGUIControlValue, getMax, "Returns the maximum of the control."),
	PYTHON_METHOD_NOARGS(ptGUIControlValue, getStep, "Returns the step increment of the control."),
	PYTHON_METHOD(ptGUIControlValue, setRange, "Params: minimum,maximum\nSets the minimum and maximum range of the control."),
	PYTHON_METHOD(ptGUIControlValue, setStep, "Params: step\nSets the step increment of the control."),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptGUIControlValue, pyGUIControl, "Params: ctrlKey\nPlasma GUI Control Value class  - knobs, spinners");
PYTHON_EXPOSE_TYPE_DEFINITION(ptGUIControlValue, pyGUIControlValue);

// required functions for PyObject interoperability
PyObject *pyGUIControlValue::New(pyKey& gckey)
{
	ptGUIControlValue *newObj = (ptGUIControlValue*)ptGUIControlValue_type.tp_new(&ptGUIControlValue_type, NULL, NULL);
	newObj->fThis->fGCkey = gckey.getKey();
	return (PyObject*)newObj;
}

PyObject *pyGUIControlValue::New(plKey objkey)
{
	ptGUIControlValue *newObj = (ptGUIControlValue*)ptGUIControlValue_type.tp_new(&ptGUIControlValue_type, NULL, NULL);
	newObj->fThis->fGCkey = objkey;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGUIControlValue, pyGUIControlValue)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGUIControlValue, pyGUIControlValue)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyGUIControlValue::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptGUIControlValue);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////

// glue functions
PYTHON_CLASS_DEFINITION(ptGUIControlKnob, pyGUIControlKnob);

PYTHON_DEFAULT_NEW_DEFINITION(ptGUIControlKnob, pyGUIControlKnob)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGUIControlKnob)

PYTHON_INIT_DEFINITION(ptGUIControlKnob, args, keywords)
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

PYTHON_START_METHODS_TABLE(ptGUIControlKnob)
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptGUIControlKnob, pyGUIControlValue, "Params: ctrlKey\nPlasma GUI control for knob");

// required functions for PyObject interoperability
PyObject *pyGUIControlKnob::New(pyKey& gckey)
{
	ptGUIControlKnob *newObj = (ptGUIControlKnob*)ptGUIControlKnob_type.tp_new(&ptGUIControlKnob_type, NULL, NULL);
	newObj->fThis->fGCkey = gckey.getKey();
	return (PyObject*)newObj;
}

PyObject *pyGUIControlKnob::New(plKey objkey)
{
	ptGUIControlKnob *newObj = (ptGUIControlKnob*)ptGUIControlKnob_type.tp_new(&ptGUIControlKnob_type, NULL, NULL);
	newObj->fThis->fGCkey = objkey;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGUIControlKnob, pyGUIControlKnob)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGUIControlKnob, pyGUIControlKnob)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyGUIControlKnob::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptGUIControlKnob);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////

// glue functions
PYTHON_CLASS_DEFINITION(ptGUIControlUpDownPair, pyGUIControlUpDownPair);

PYTHON_DEFAULT_NEW_DEFINITION(ptGUIControlUpDownPair, pyGUIControlUpDownPair)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGUIControlUpDownPair)

PYTHON_INIT_DEFINITION(ptGUIControlUpDownPair, args, keywords)
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

PYTHON_START_METHODS_TABLE(ptGUIControlUpDownPair)
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptGUIControlUpDownPair, pyGUIControlValue, "Params: ctrlKey\nPlasma GUI control for up/down pair");

// required functions for PyObject interoperability
PyObject *pyGUIControlUpDownPair::New(pyKey& gckey)
{
	ptGUIControlUpDownPair *newObj = (ptGUIControlUpDownPair*)ptGUIControlUpDownPair_type.tp_new(&ptGUIControlUpDownPair_type, NULL, NULL);
	newObj->fThis->fGCkey = gckey.getKey();
	return (PyObject*)newObj;
}

PyObject *pyGUIControlUpDownPair::New(plKey objkey)
{
	ptGUIControlUpDownPair *newObj = (ptGUIControlUpDownPair*)ptGUIControlUpDownPair_type.tp_new(&ptGUIControlUpDownPair_type, NULL, NULL);
	newObj->fThis->fGCkey = objkey;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGUIControlUpDownPair, pyGUIControlUpDownPair)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGUIControlUpDownPair, pyGUIControlUpDownPair)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyGUIControlUpDownPair::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptGUIControlUpDownPair);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////

// glue functions
PYTHON_CLASS_DEFINITION(ptGUIControlProgress, pyGUIControlProgress);

PYTHON_DEFAULT_NEW_DEFINITION(ptGUIControlProgress, pyGUIControlProgress)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGUIControlProgress)

PYTHON_INIT_DEFINITION(ptGUIControlProgress, args, keywords)
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

PYTHON_METHOD_DEFINITION(ptGUIControlProgress, animateToPercent, args)
{
	float percent;
	if (!PyArg_ParseTuple(args, "f", &percent))
	{
		PyErr_SetString(PyExc_TypeError, "animateToPercent expects a float");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->AnimateToPercentage(percent);
	PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptGUIControlProgress)
	PYTHON_METHOD(ptGUIControlProgress, animateToPercent, "Params: percent\nSets the value of the control and animates to that point."),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptGUIControlProgress, pyGUIControlValue, "Params: ctrlKey\nPlasma GUI control for progress bar");

// required functions for PyObject interoperability
PyObject *pyGUIControlProgress::New(pyKey& gckey)
{
	ptGUIControlProgress *newObj = (ptGUIControlProgress*)ptGUIControlProgress_type.tp_new(&ptGUIControlProgress_type, NULL, NULL);
	newObj->fThis->fGCkey = gckey.getKey();
	return (PyObject*)newObj;
}

PyObject *pyGUIControlProgress::New(plKey objkey)
{
	ptGUIControlProgress *newObj = (ptGUIControlProgress*)ptGUIControlProgress_type.tp_new(&ptGUIControlProgress_type, NULL, NULL);
	newObj->fThis->fGCkey = objkey;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGUIControlProgress, pyGUIControlProgress)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGUIControlProgress, pyGUIControlProgress)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyGUIControlProgress::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptGUIControlProgress);
	PYTHON_CLASS_IMPORT_END(m);
}