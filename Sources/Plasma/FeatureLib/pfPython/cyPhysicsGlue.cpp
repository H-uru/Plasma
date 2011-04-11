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
#include "cyPhysics.h"
#include "pyKey.h"
#include "pyGeometry3.h"
#include "pyMatrix44.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptPhysics, cyPhysics);

PYTHON_DEFAULT_NEW_DEFINITION(ptPhysics, cyPhysics)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptPhysics)

PYTHON_INIT_DEFINITION(ptPhysics, args, keywords)
{
	PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION(ptPhysics, netForce, args)
{
	char forceFlag;
	if (!PyArg_ParseTuple(args, "b", &forceFlag))
	{
		PyErr_SetString(PyExc_TypeError, "netForce requires a boolean argument");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetNetForce(forceFlag != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptPhysics, enable, args)
{
	char stateFlag = 1;
	if (!PyArg_ParseTuple(args, "|b", &stateFlag))
	{
		PyErr_SetString(PyExc_TypeError, "enable expects an optional boolean argument");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->EnableT(stateFlag != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptPhysics, disable, Disable)

PYTHON_BASIC_METHOD_DEFINITION(ptPhysics, disableCollision, DisableCollision)
PYTHON_BASIC_METHOD_DEFINITION(ptPhysics, enableCollision, EnableCollision)

PYTHON_METHOD_DEFINITION(ptPhysics, warp, args)
{
	PyObject *positionObject = NULL;
	if (!PyArg_ParseTuple(args, "O", &positionObject))
	{
		PyErr_SetString(PyExc_TypeError, "warp expects a ptPoint3 or ptMatrix44 object");
		PYTHON_RETURN_ERROR;
	}
	if (pyPoint3::Check(positionObject))
	{
		pyPoint3 *pos = pyPoint3::ConvertFrom(positionObject);
		self->fThis->Warp(*pos);
		PYTHON_RETURN_NONE;
	}
	else if (pyMatrix44::Check(positionObject))
	{
		pyMatrix44 *mat = pyMatrix44::ConvertFrom(positionObject);
		self->fThis->WarpMat(*mat);
		PYTHON_RETURN_NONE;
	}
	PyErr_SetString(PyExc_TypeError, "warp expects a ptPoint3 or ptMatrix44 object");
	PYTHON_RETURN_ERROR;
}

PYTHON_METHOD_DEFINITION(ptPhysics, warpObj, args)
{
	PyObject *keyObject = NULL;
	if (!PyArg_ParseTuple(args, "O", &keyObject))
	{
		PyErr_SetString(PyExc_TypeError, "warpObj expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObject))
	{
		PyErr_SetString(PyExc_TypeError, "warpObj expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	pyKey *key = pyKey::ConvertFrom(keyObject);
	self->fThis->WarpObj(*key);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptPhysics, move, args)
{
	PyObject *directionObject = NULL;
	float distance;
	if (!PyArg_ParseTuple(args, "Of", &directionObject, &distance))
	{
		PyErr_SetString(PyExc_TypeError, "move expects a ptVector3 and float");
		PYTHON_RETURN_ERROR;
	}
	if (!pyVector3::Check(directionObject))
	{
		PyErr_SetString(PyExc_TypeError, "move expects a ptVector3 and float");
		PYTHON_RETURN_ERROR;
	}
	pyVector3 *direction = pyVector3::ConvertFrom(directionObject);
	self->fThis->Move(*direction, distance);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptPhysics, rotate, args)
{
	float radians;
	PyObject *axisObject = NULL;
	if (!PyArg_ParseTuple(args, "fO", &radians, &axisObject))
	{
		PyErr_SetString(PyExc_TypeError, "rotate expects a float and ptVector3");
		PYTHON_RETURN_ERROR;
	}
	if (!pyVector3::Check(axisObject))
	{
		PyErr_SetString(PyExc_TypeError, "rotate expects a float and ptVector3");
		PYTHON_RETURN_ERROR;
	}
	pyVector3 *axis = pyVector3::ConvertFrom(axisObject);
	self->fThis->Rotate(radians, *axis);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptPhysics, force, args)
{
	PyObject *forceObject = NULL;
	if (!PyArg_ParseTuple(args, "O", &forceObject))
	{
		PyErr_SetString(PyExc_TypeError, "force expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	if (!pyVector3::Check(forceObject))
	{
		PyErr_SetString(PyExc_TypeError, "force expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	pyVector3 *force = pyVector3::ConvertFrom(forceObject);
	self->fThis->Force(*force);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptPhysics, forceWithOffset, args)
{
	PyObject *forceObject = NULL;
	PyObject *offsetObject = NULL;
	if (!PyArg_ParseTuple(args, "OO", &forceObject, &offsetObject))
	{
		PyErr_SetString(PyExc_TypeError, "forceWithOffset expects a ptVector3 and a ptPoint3");
		PYTHON_RETURN_ERROR;
	}
	if ((!pyVector3::Check(forceObject)) || (!pyPoint3::Check(offsetObject)))
	{
		PyErr_SetString(PyExc_TypeError, "forceWithOffset expects a ptVector3 and a ptPoint3");
		PYTHON_RETURN_ERROR;
	}
	pyVector3 *force = pyVector3::ConvertFrom(forceObject);
	pyPoint3 *offset = pyPoint3::ConvertFrom(offsetObject);
	self->fThis->ForceWithOffset(*force, *offset);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptPhysics, torque, args)
{
	PyObject *torqueObject = NULL;
	if (!PyArg_ParseTuple(args, "O", &torqueObject))
	{
		PyErr_SetString(PyExc_TypeError, "torque expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	if (!pyVector3::Check(torqueObject))
	{
		PyErr_SetString(PyExc_TypeError, "torque expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	pyVector3 *torque = pyVector3::ConvertFrom(torqueObject);
	self->fThis->Torque(*torque);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptPhysics, impulse, args)
{
	PyObject *forceObject = NULL;
	if (!PyArg_ParseTuple(args, "O", &forceObject))
	{
		PyErr_SetString(PyExc_TypeError, "impulse expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	if (!pyVector3::Check(forceObject))
	{
		PyErr_SetString(PyExc_TypeError, "impulse expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	pyVector3 *force = pyVector3::ConvertFrom(forceObject);
	self->fThis->Impulse(*force);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptPhysics, impulseWithOffset, args)
{
	PyObject *forceObject = NULL;
	PyObject *offsetObject = NULL;
	if (!PyArg_ParseTuple(args, "OO", &forceObject, &offsetObject))
	{
		PyErr_SetString(PyExc_TypeError, "impulseWithOffset expects a ptVector3 and a ptPoint3");
		PYTHON_RETURN_ERROR;
	}
	if ((!pyVector3::Check(forceObject)) || (!pyPoint3::Check(offsetObject)))
	{
		PyErr_SetString(PyExc_TypeError, "impulseWithOffset expects a ptVector3 and a ptPoint3");
		PYTHON_RETURN_ERROR;
	}
	pyVector3 *force = pyVector3::ConvertFrom(forceObject);
	pyPoint3 *offset = pyPoint3::ConvertFrom(offsetObject);
	self->fThis->ImpulseWithOffset(*force, *offset);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptPhysics, angularImpulse, args)
{
	PyObject *forceObject = NULL;
	if (!PyArg_ParseTuple(args, "O", &forceObject))
	{
		PyErr_SetString(PyExc_TypeError, "angularImpulse expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	if (!pyVector3::Check(forceObject))
	{
		PyErr_SetString(PyExc_TypeError, "angularImpulse expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	pyVector3 *force = pyVector3::ConvertFrom(forceObject);
	self->fThis->AngularImpulse(*force);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptPhysics, damp, args)
{
	float damp;
	if (!PyArg_ParseTuple(args, "f", &damp))
	{
		PyErr_SetString(PyExc_TypeError, "damp expects a float");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Damp(damp);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptPhysics, shiftMass, args)
{
	PyObject *offestObject = NULL;
	if (!PyArg_ParseTuple(args, "O", &offestObject))
	{
		PyErr_SetString(PyExc_TypeError, "shiftMass expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	if (!pyVector3::Check(offestObject))
	{
		PyErr_SetString(PyExc_TypeError, "shiftMass expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	pyVector3 *offset = pyVector3::ConvertFrom(offestObject);
	self->fThis->ShiftMass(*offset);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptPhysics, suppress, args)
{
	char doSuppress;
	if (!PyArg_ParseTuple(args, "b", &doSuppress))
	{
		PyErr_SetString(PyExc_TypeError, "suppress expects a boolean");
		PYTHON_RETURN_NONE;
	}
	self->fThis->Suppress(doSuppress != 0);
	PYTHON_RETURN_NONE;
}
PYTHON_METHOD_DEFINITION(ptPhysics, setLinearVelocity, args)
{
	PyObject *velocity = NULL;
	if (!PyArg_ParseTuple(args, "O", &velocity))
	{
		PyErr_SetString(PyExc_TypeError, "setVelocity expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	if (!pyVector3::Check(velocity))
	{
		PyErr_SetString(PyExc_TypeError, "setVelocity expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	pyVector3 *velocityVec = pyVector3::ConvertFrom(velocity);
	self->fThis->SetLinearVelocity(*velocityVec);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptPhysics, setAngularVelocity, args)
{
	PyObject *velocity = NULL;
	if (!PyArg_ParseTuple(args, "O", &velocity))
	{
		PyErr_SetString(PyExc_TypeError, "setAngularVelocity expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	if (!pyVector3::Check(velocity))
	{
		PyErr_SetString(PyExc_TypeError, "setAngularVelocity expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	pyVector3 *velocityVec = pyVector3::ConvertFrom(velocity);
	self->fThis->SetAngularVelocity(*velocityVec);
	PYTHON_RETURN_NONE;
}
PYTHON_START_METHODS_TABLE(ptPhysics)
	PYTHON_METHOD(ptPhysics, netForce, "Params: forceFlag\nSpecify whether this object needs to use messages that are forced to the network\n"
				"- This is to be used if your Python program is running on only one client\n"
				"Such as a game master, only running on the client that owns a particular object"),

	PYTHON_METHOD(ptPhysics, enable, "Params: state=1\nSets the physics enable state for the sceneobject attached"),
	PYTHON_BASIC_METHOD(ptPhysics, disable, "Disables physics on the sceneobject attached"),

	PYTHON_BASIC_METHOD(ptPhysics, disableCollision, "Disables collision detection on the attached sceneobject"),
	PYTHON_BASIC_METHOD(ptPhysics, enableCollision, "Enables collision detection on the attached sceneobject"),

	PYTHON_METHOD(ptPhysics, warp, "Params: position\nWarps the sceneobject to a specified location.\n"
				"'position' can be a ptPoint3 or a ptMatrix44"),
	PYTHON_METHOD(ptPhysics, warpObj, "Params: objkey\nWarps the sceneobject to match the location and orientation of the specified object"),

	PYTHON_METHOD(ptPhysics, move, "Params: direction,distance\nMoves the attached sceneobject the specified distance in the specified direction"),
	PYTHON_METHOD(ptPhysics, rotate, "Params: radians,axis\nRotates the attached sceneobject the specified radians around the specified axis"),
	PYTHON_METHOD(ptPhysics, force, "Params: forceVector\nApplies the specified force to the attached sceneobject"),
	PYTHON_METHOD(ptPhysics, forceWithOffset, "Params: forceVector,offsetPt\nApplies the specified offsetted force to the attached sceneobject"),
	PYTHON_METHOD(ptPhysics, torque, "Params: torqueVector\nApplies the specified torque to the attached sceneobject"),
	PYTHON_METHOD(ptPhysics, impulse, "Params: impulseVector\nAdds the given vector to the attached sceneobject's velocity"),
	PYTHON_METHOD(ptPhysics, impulseWithOffset, "Params: impulseVector,offsetPt\nAdds the given vector to the attached sceneobject's velocity\n"
				"with the specified offset"),
	PYTHON_METHOD(ptPhysics, angularImpulse, "Params: impulseVector\nAdd the given vector (representing a rotation axis and magnitude) to\n"
				"the attached sceneobject's velocity"),
	PYTHON_METHOD(ptPhysics, damp, "Params: damp\nReduce all velocities on the object (0 = all stop, 1 = no effect)"),
	PYTHON_METHOD(ptPhysics, shiftMass, "Params: offsetVector\nShifts the attached sceneobject's center to mass in the specified direction and distance"),
	PYTHON_METHOD(ptPhysics, suppress, "Params: doSuppress\nCompletely remove the physical, but keep it around so it\n"
				"can be added back later."),
	PYTHON_METHOD(ptPhysics, setLinearVelocity, "Params: velocityVector\nSets the objects LinearVelocity to the specified vector"),
	PYTHON_METHOD(ptPhysics, setAngularVelocity, "Params: velocityVector\nSets the objects AngularVelocity to the specified vector"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptPhysics, "Plasma physics class");

// required functions for PyObject interoperability
PyObject *cyPhysics::New(PyObject *sender, PyObject *recvr)
{
	ptPhysics *newObj = (ptPhysics*)ptPhysics_type.tp_new(&ptPhysics_type, NULL, NULL);
	if (sender != NULL)
	{
		pyKey *senderKey = pyKey::ConvertFrom(sender);
		newObj->fThis->SetSender(senderKey->getKey());
	}
	if (recvr != NULL)
	{
		pyKey *recvrKey = pyKey::ConvertFrom(recvr);
		newObj->fThis->AddRecvr(recvrKey->getKey());
	}
	newObj->fThis->SetNetForce(false);
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptPhysics, cyPhysics)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptPhysics, cyPhysics)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void cyPhysics::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptPhysics);
	PYTHON_CLASS_IMPORT_END(m);
}