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
#include "cyAnimation.h"
#include "hsUtils.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptAnimation, cyAnimation);

PYTHON_DEFAULT_NEW_DEFINITION(ptAnimation, cyAnimation)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptAnimation)

PYTHON_INIT_DEFINITION(ptAnimation, args, keywords)
{
	PyObject *keyObject = NULL;
	if (!PyArg_ParseTuple(args, "|O", &keyObject))
	{
		PyErr_SetString(PyExc_TypeError, "__init__ expects an optional ptKey");
		PYTHON_RETURN_INIT_ERROR;
	}
	if (keyObject == NULL) // no parameter was passed
		PYTHON_RETURN_INIT_OK; // nothing to init

	if (!pyKey::Check(keyObject))
	{
		PyErr_SetString(PyExc_TypeError, "__init__ expects an optional ptKey");
		PYTHON_RETURN_INIT_ERROR;
	}

	pyKey *key = pyKey::ConvertFrom(keyObject);
	self->fThis->SetSender(*key);

	PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION(ptAnimation, sender, args)
{
	PyObject *keyObject = NULL;
	if (!PyArg_ParseTuple(args, "O", &keyObject))
	{
		PyErr_SetString(PyExc_TypeError, "sender requires a ptKey argument");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObject))
	{
		PyErr_SetString(PyExc_TypeError, "sender requires a ptKey argument");
		PYTHON_RETURN_ERROR;
	}

	pyKey *key = pyKey::ConvertFrom(keyObject);
	self->fThis->SetSender(*key);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAnimation, addKey, args)
{
	PyObject *keyObject = NULL;
	if (!PyArg_ParseTuple(args, "O", &keyObject))
	{
		PyErr_SetString(PyExc_TypeError, "addKey requires a ptKey argument");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObject))
	{
		PyErr_SetString(PyExc_TypeError, "addKey requires a ptKey argument");
		PYTHON_RETURN_ERROR;
	}

	pyKey *key = pyKey::ConvertFrom(keyObject);
	self->fThis->AddRecvr(*key);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAnimation, netForce, args)
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

PYTHON_METHOD_DEFINITION(ptAnimation, setAnimName, args)
{
	char *name = NULL;
	if (!PyArg_ParseTuple(args, "s", &name)) // name points at the internal buffer SO DON'T DELETE IT
	{
		PyErr_SetString(PyExc_TypeError, "setAnimName requires a string argument");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetAnimName(name);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptAnimation, play, Play)
PYTHON_BASIC_METHOD_DEFINITION(ptAnimation, stop, Stop)
PYTHON_BASIC_METHOD_DEFINITION(ptAnimation, resume, Resume)

PYTHON_METHOD_DEFINITION(ptAnimation, playRange, args)
{
	float start, end;
	if (!PyArg_ParseTuple(args, "ff", &start, &end))
	{
		PyErr_SetString(PyExc_TypeError, "playRange requires two floating-point arguments");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->PlayRange(start, end);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAnimation, playToTime, args)
{
	float time;
	if (!PyArg_ParseTuple(args, "f", &time))
	{
		PyErr_SetString(PyExc_TypeError, "playToTime requires one floating-point argument");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->PlayToTime(time);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAnimation, playToPercentage, args)
{
	float percent;
	if (!PyArg_ParseTuple(args, "f", &percent))
	{
		PyErr_SetString(PyExc_TypeError, "playToPercentage requires one floating-point argument");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->PlayToPercentage(percent);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAnimation, skipToTime, args)
{
	float time;
	if (!PyArg_ParseTuple(args, "f", &time))
	{
		PyErr_SetString(PyExc_TypeError, "skipToTime requires one floating-point argument");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SkipToTime(time);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAnimation, looped, args)
{
	char looped;
	if (!PyArg_ParseTuple(args, "b", &looped))
	{
		PyErr_SetString(PyExc_TypeError, "looped requires a boolean argument");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Looped(looped != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAnimation, backwards, args)
{
	char backwards;
	if (!PyArg_ParseTuple(args, "b", &backwards))
	{
		PyErr_SetString(PyExc_TypeError, "backwards requires a boolean argument");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Backwards(backwards != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAnimation, setLoopStart, args)
{
	float time;
	if (!PyArg_ParseTuple(args, "f", &time))
	{
		PyErr_SetString(PyExc_TypeError, "setLoopStart requires one floating-point argument");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetLoopStart(time);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAnimation, setLoopEnd, args)
{
	float time;
	if (!PyArg_ParseTuple(args, "f", &time))
	{
		PyErr_SetString(PyExc_TypeError, "setLoopEnd requires one floating-point argument");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetLoopEnd(time);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAnimation, speed, args)
{
	float speed;
	if (!PyArg_ParseTuple(args, "f", &speed))
	{
		PyErr_SetString(PyExc_TypeError, "speed requires one floating-point argument");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Speed(speed);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptAnimation, skipToBegin, SkipToBegin)
PYTHON_BASIC_METHOD_DEFINITION(ptAnimation, skipToEnd, SkipToEnd)
PYTHON_BASIC_METHOD_DEFINITION(ptAnimation, skipToLoopBegin, SkipToLoopBegin)
PYTHON_BASIC_METHOD_DEFINITION(ptAnimation, skipToLoopEnd, SkipToLoopEnd)
PYTHON_BASIC_METHOD_DEFINITION(ptAnimation, incrementForward, IncrementForward)
PYTHON_BASIC_METHOD_DEFINITION(ptAnimation, incrementBackward, IncrementBackward)

PYTHON_METHOD_DEFINITION_NOARGS(ptAnimation, getFirstKey)
{
	PyObject *key = self->fThis->GetFirstRecvr();
	if (key == NULL)
		PYTHON_RETURN_NONE;
	return key;
}

PYTHON_START_METHODS_TABLE(ptAnimation)
	PYTHON_METHOD(ptAnimation, sender, "Params: selfKey\nSets the sender of the messages being sent to the animation modifier"),
	PYTHON_METHOD(ptAnimation, addKey, "Params: key\nAdds an animation modifier to the list of receiver keys"),
	PYTHON_METHOD(ptAnimation, netForce, "Params: forceFlag\nSpecify whether this object needs to use messages that are forced to the network\n"
				"- This is to be used if your Python program is running on only one client\n"
				"Such as a game master, only running on the client that owns a particular object"),
	PYTHON_METHOD(ptAnimation, setAnimName, "Params: name\nSets the animation notetrack name (or (Entire Animation))"),
	PYTHON_BASIC_METHOD(ptAnimation, play, "Plays the animation"),
	PYTHON_BASIC_METHOD(ptAnimation, stop, "Stops the animation"),
	PYTHON_BASIC_METHOD(ptAnimation, resume, "Resumes the animation from where it was stopped last"),
	PYTHON_METHOD(ptAnimation, playRange, "Params: start,end\nPlay the animation from start to end"),
	PYTHON_METHOD(ptAnimation, playToTime, "Params: time\nPlay the animation to the specified time"),
	PYTHON_METHOD(ptAnimation, playToPercentage, "Params: zeroToOne\nPlay the animation to the specified percentage (0 to 1)"),
	PYTHON_METHOD(ptAnimation, skipToTime, "Params: time\nSkip the animation to time (don't play)"),
	PYTHON_METHOD(ptAnimation, looped, "Params: loopedFlag\nTurn on and off looping of the animation"),
	PYTHON_METHOD(ptAnimation, backwards, "Params: backwardsFlag\nTurn on and off playing the animation backwards"),
	PYTHON_METHOD(ptAnimation, setLoopStart, "Params: loopStart\nSets the loop starting position\n"
				"- 'loopStart' is the number of seconds from the absolute beginning of the animation"),
	PYTHON_METHOD(ptAnimation, setLoopEnd, "Params: loopEnd\nSets the loop ending position\n"
				"- 'loopEnd' is the number of seconds from the absolute beginning of the animation"),
	PYTHON_METHOD(ptAnimation, speed, "Params: speed\nSets the animation playback speed"),
	PYTHON_BASIC_METHOD(ptAnimation, skipToBegin, "Skip to the beginning of the animation (don't play)"),
	PYTHON_BASIC_METHOD(ptAnimation, skipToEnd, "Skip to the end of the animation (don't play)"),
	PYTHON_BASIC_METHOD(ptAnimation, skipToLoopBegin, "Skip to the beginning of the animation loop (don't play)"),
	PYTHON_BASIC_METHOD(ptAnimation, skipToLoopEnd, "Skip to the end of the animation loop (don't play)"),
	PYTHON_BASIC_METHOD(ptAnimation, incrementForward, "Step the animation forward a frame"),
	PYTHON_BASIC_METHOD(ptAnimation, incrementBackward, "Step the animation backward a frame"),
	PYTHON_METHOD_NOARGS(ptAnimation, getFirstKey, "This will return a ptKey object that is the first receiver (target)\n"
				"However, if the parent is not a modifier or not loaded, then None is returned."),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptAnimation, "Params: key=None\nPlasma animation class");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptAnimation, cyAnimation)

PyObject *cyAnimation::New(PyObject *sender)
{
	ptAnimation *newObj = (ptAnimation*)ptAnimation_type.tp_new(&ptAnimation_type, NULL, NULL);
	pyKey *key = pyKey::ConvertFrom(sender);
	newObj->fThis->SetSender(*key);
	newObj->fThis->fAnimName = nil;
	newObj->fThis->fNetForce = false;
	return (PyObject*)newObj;
}

PyObject *cyAnimation::New(cyAnimation &obj)
{
	ptAnimation *newObj = (ptAnimation*)ptAnimation_type.tp_new(&ptAnimation_type, NULL, NULL);
	newObj->fThis->fSender = obj.fSender;
	newObj->fThis->fRecvr = obj.fRecvr;
	newObj->fThis->fAnimName = hsStrcpy(obj.fAnimName);
	newObj->fThis->fNetForce = obj.fNetForce;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptAnimation, cyAnimation)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptAnimation, cyAnimation)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void cyAnimation::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptAnimation);
	PYTHON_CLASS_IMPORT_END(m);
}