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
#include "cyCamera.h"
#include "pyKey.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptCamera, cyCamera);

PYTHON_DEFAULT_NEW_DEFINITION(ptCamera, cyCamera)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptCamera)

PYTHON_INIT_DEFINITION(ptCamera, args, keywords)
{
	PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION(ptCamera, save, args)
{
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "save expects a ptKey object");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "save expects a ptKey object");
		PYTHON_RETURN_ERROR;
	}

	pyKey* key = pyKey::ConvertFrom(keyObj);
	self->fThis->Push(*key);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptCamera, restore, args)
{
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "restore expects a ptKey object");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "restore expects a ptKey object");
		PYTHON_RETURN_ERROR;
	}

	pyKey* key = pyKey::ConvertFrom(keyObj);
	self->fThis->Pop(*key);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptCamera, controlKey, args)
{
	long controlKey;
	char activateFlag;
	if (!PyArg_ParseTuple(args, "lb", &controlKey, &activateFlag))
	{
		PyErr_SetString(PyExc_TypeError, "controlKey expects a long and a boolean");
		PYTHON_RETURN_ERROR;
	}

	self->fThis->ControlKey(controlKey, activateFlag != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptCamera, set, args)
{
	PyObject* keyObj = NULL;
	double time;
	char save;
	if (!PyArg_ParseTuple(args, "Odb", &keyObj, &time, &save))
	{
		PyErr_SetString(PyExc_TypeError, "set expects a ptKey, double, and a boolean");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "set expects a ptKey, double, and a boolean");
		PYTHON_RETURN_ERROR;
	}

	pyKey* key = pyKey::ConvertFrom(keyObj);
	self->fThis->TransitionTo(*key, time, save != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptCamera, enableFirstPersonOverride, EnableFirstPersonOverride)
PYTHON_BASIC_METHOD_DEFINITION(ptCamera, disableFirstPersonOverride, DisableFirstPersonOverride)
PYTHON_BASIC_METHOD_DEFINITION(ptCamera, undoFirstPerson, UndoFirstPerson)

PYTHON_METHOD_DEFINITION_NOARGS(ptCamera, getFOV)
{
	return PyFloat_FromDouble(self->fThis->GetFOV());
}

PYTHON_METHOD_DEFINITION(ptCamera, setFOV, args)
{
	float fov;
	double time;
	if (!PyArg_ParseTuple(args, "fd", &fov, &time))
	{
		PyErr_SetString(PyExc_TypeError, "setFOV expects a float and a double");
		PYTHON_RETURN_ERROR;
	}

	self->fThis->SetFOV(fov, time);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptCamera, setSmootherCam, args)
{
	char state;
	if (!PyArg_ParseTuple(args, "b", &state))
	{
		PyErr_SetString(PyExc_TypeError, "setSmootherCam expects a boolean");
		PYTHON_RETURN_ERROR;
	}

	self->fThis->SetSmootherCam(state != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptCamera, isSmootherCam)
{
	PYTHON_RETURN_BOOL(self->fThis->IsSmootherCam());
}

PYTHON_METHOD_DEFINITION(ptCamera, setWalkAndVerticalPan, args)
{
	char state;
	if (!PyArg_ParseTuple(args, "b", &state))
	{
		PyErr_SetString(PyExc_TypeError, "setWalkAndVerticalPan expects a boolean");
		PYTHON_RETURN_ERROR;
	}

	self->fThis->SetWalkAndVerticalPan(state != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptCamera, isWalkAndVerticalPan)
{
	PYTHON_RETURN_BOOL(self->fThis->IsWalkAndVerticalPan());
}

PYTHON_METHOD_DEFINITION(ptCamera, setStayInFirstPerson, args)
{
	char state;
	if (!PyArg_ParseTuple(args, "b", &state))
	{
		PyErr_SetString(PyExc_TypeError, "setStayInFirstPerson expects a boolean");
		PYTHON_RETURN_ERROR;
	}

	self->fThis->SetStayInFirstPerson(state != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptCamera, isStayInFirstPerson)
{
	PYTHON_RETURN_BOOL(self->fThis->IsStayInFirstPerson());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptCamera, getAspectRatio)
{
	return PyFloat_FromDouble(self->fThis->GetAspectRatio());
}

PYTHON_METHOD_DEFINITION(ptCamera, setAspectRatio, args)
{
	float aspect;
	if (!PyArg_ParseTuple(args, "f", &aspect))
	{
		PyErr_SetString(PyExc_TypeError, "setAspectRatio expects a float");
		PYTHON_RETURN_ERROR;
	}

	self->fThis->SetAspectRatio(aspect);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptCamera, refreshFOV)
{
	self->fThis->RefreshFOV();
	PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptCamera)
	PYTHON_METHOD(ptCamera, save, "Params: cameraKey\nSaves the current camera and sets the camera to cameraKey"),
	PYTHON_METHOD(ptCamera, restore, "Params: cameraKey\nRestores camera to saved one"),
	
	PYTHON_METHOD(ptCamera, controlKey, "Params: controlKey,activateFlag\nSend a control key to the camera as if it was hit by the user.\n"
				"This is for sending things like pan-up, pan-down, zoom-in, etc."),

	PYTHON_METHOD(ptCamera, set, "Params: cameraKey,time,save\nDO NOT USE"),
	
	PYTHON_BASIC_METHOD(ptCamera, enableFirstPersonOverride, "Allows the user to override the camera and go to a first person camera."),
	PYTHON_BASIC_METHOD(ptCamera, disableFirstPersonOverride, "Does _not_ allow the user to override the camera to go to first person camera."),
	PYTHON_BASIC_METHOD(ptCamera, undoFirstPerson, "If the user has overridden the camera to be in first person, this will take them out of first person.\n"
				"If the user didn't override the camera, then this will do nothing."),
	 
	PYTHON_METHOD_NOARGS(ptCamera, getFOV, "Returns the current camera's FOV(h)"),
	PYTHON_METHOD(ptCamera, setFOV, "Params: fov, time\nSets the current cameras FOV (based on h)"),

	PYTHON_METHOD(ptCamera, setSmootherCam, "Params: state\nSet the faster cams thing"),
	PYTHON_METHOD_NOARGS(ptCamera, isSmootherCam, "Returns true if we are using the faster cams thing"),
	PYTHON_METHOD(ptCamera, setWalkAndVerticalPan, "Params: state\nSet Walk and chew gum"),
	PYTHON_METHOD_NOARGS(ptCamera, isWalkAndVerticalPan, "Returns true if we are walking and chewing gum"),
	PYTHON_METHOD(ptCamera, setStayInFirstPerson, "Params: state\nSet Stay In First Person Always"),
	PYTHON_METHOD_NOARGS(ptCamera, isStayInFirstPerson, "Are we staying in first person?"),
	PYTHON_METHOD_NOARGS(ptCamera, getAspectRatio, "Get the global aspect ratio"),
	PYTHON_METHOD(ptCamera, setAspectRatio, "Params: aspect\nSet the global aspect ratio"),
	PYTHON_METHOD_NOARGS(ptCamera, refreshFOV, "Refreshes the FOV"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptCamera, "Plasma camera class");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptCamera, cyCamera)
PYTHON_CLASS_CHECK_IMPL(ptCamera, cyCamera)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptCamera, cyCamera)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void cyCamera::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptCamera);
	PYTHON_CLASS_IMPORT_END(m);
}