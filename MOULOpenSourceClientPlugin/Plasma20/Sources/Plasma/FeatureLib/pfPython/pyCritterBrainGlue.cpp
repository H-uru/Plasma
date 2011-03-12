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
#include "pyCritterBrain.h"
#include "pyEnum.h"
#include "pyGeometry3.h"

#include "..\plMessage\plAIMsg.h"

#include <python.h>

///////////////////////////////////////////////////////////////////////////////
//
// AddPlasmaConstantsClasses
//
void pyAIMsg::AddPlasmaConstantsClasses(PyObject *m)
{
	PYTHON_ENUM_START(PtAIMsgType);
	PYTHON_ENUM_ELEMENT(PtAIMsgType, kUnknown, plAIMsg::kAIMsg_Unknown);
	PYTHON_ENUM_ELEMENT(PtAIMsgType, kBrainCreated, plAIMsg::kAIMsg_BrainCreated);
	PYTHON_ENUM_ELEMENT(PtAIMsgType, kArrivedAtGoal, plAIMsg::kAIMsg_ArrivedAtGoal);
	PYTHON_ENUM_END(m, PtAIMsgType);
}

///////////////////////////////////////////////////////////////////////////////

// glue functions
PYTHON_CLASS_DEFINITION(ptCritterBrain, pyCritterBrain);

PYTHON_DEFAULT_NEW_DEFINITION(ptCritterBrain, pyCritterBrain)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptCritterBrain)

PYTHON_NO_INIT_DEFINITION(ptCritterBrain)

PYTHON_RICH_COMPARE_DEFINITION(ptCritterBrain, obj1, obj2, compareType)
{
	if ((obj1 == Py_None) || (obj2 == Py_None) || !pyCritterBrain::Check(obj1) || !pyCritterBrain::Check(obj2))
	{
		// if they aren't the same type, they don't match, obviously (we also never equal none)
		if (compareType == Py_EQ)
			PYTHON_RCOMPARE_FALSE;
		else if (compareType == Py_NE)
			PYTHON_RCOMPARE_TRUE;
		else
		{
			PyErr_SetString(PyExc_NotImplementedError, "invalid comparison for a ptCritterBrain object");
			PYTHON_RCOMPARE_ERROR;
		}
	}
	pyCritterBrain* brain1 = pyCritterBrain::ConvertFrom(obj1);
	pyCritterBrain* brain2 = pyCritterBrain::ConvertFrom(obj2);
	if (compareType == Py_EQ)
	{
		if ((*brain1) == (*brain2))
			PYTHON_RCOMPARE_TRUE;
		PYTHON_RCOMPARE_FALSE;
	}
	else if (compareType == Py_NE)
	{
		if ((*brain1) != (*brain2))
			PYTHON_RCOMPARE_TRUE;
		PYTHON_RCOMPARE_FALSE;
	}
	PyErr_SetString(PyExc_NotImplementedError, "invalid comparison for a ptCritterBrain object");
	PYTHON_RCOMPARE_ERROR;
}

PYTHON_METHOD_DEFINITION(ptCritterBrain, addReceiver, args)
{
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "addReceiver expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "addReceiver expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	self->fThis->AddReceiver(*key);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptCritterBrain, removeReceiver, args)
{
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "removeReceiver expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "removeReceiver expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	self->fThis->RemoveReceiver(*key);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptCritterBrain, setLocallyControlled, args)
{
	char local;
	if (!PyArg_ParseTuple(args, "b", &local))
	{
		PyErr_SetString(PyExc_TypeError, "setLocallyControlled expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->LocallyControlled(local != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptCritterBrain, getLocallyControlled)
{
	PYTHON_RETURN_BOOL(self->fThis->LocallyControlled());
}

PYTHON_METHOD_DEFINITION_WKEY(ptCritterBrain, addBehavior, args, keywords)
{
	char* kwlist[] = {"animationName", "behaviorName", "loop", "randomStartPos", "fadeInLen", "fadeOutLen", NULL};
	PyObject* animNameObj = NULL;
	PyObject* behNameObj = NULL;
	char loop = 1, randomStartPos = 1;
	float fadeInLen = 2.f, fadeOutLen = 2.f;
	if (!PyArg_ParseTupleAndKeywords(args, keywords, "OO|bbff", kwlist, &animNameObj, &behNameObj, &loop, &randomStartPos, &fadeInLen, &fadeOutLen))
	{
		PyErr_SetString(PyExc_TypeError, "addBehavior expects two strings, and optionally two booleans and two floats");
		PYTHON_RETURN_ERROR;
	}

	std::string animName = "";
	if (PyUnicode_Check(animNameObj))
	{
		int strLen = PyUnicode_GetSize(animNameObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)animNameObj, text, strLen);
		text[strLen] = L'\0';
		char* cText = hsWStringToString(text);
		animName = cText;
		delete [] cText;
		delete [] text;
	}
	else if (PyString_Check(animNameObj))
		animName = PyString_AsString(animNameObj);
	else
	{
		PyErr_SetString(PyExc_TypeError, "addBehavior expects two strings, and optionally two booleans and two floats");
		PYTHON_RETURN_ERROR;
	}

	std::string behName = "";
	if (PyUnicode_Check(behNameObj))
	{
		int strLen = PyUnicode_GetSize(behNameObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)behNameObj, text, strLen);
		text[strLen] = L'\0';
		char* cText = hsWStringToString(text);
		behName = cText;
		delete [] cText;
		delete [] text;
	}
	else if (PyString_Check(behNameObj))
		behName = PyString_AsString(behNameObj);
	else
	{
		PyErr_SetString(PyExc_TypeError, "addBehavior expects two strings, and optionally two booleans and two floats");
		PYTHON_RETURN_ERROR;
	}

	self->fThis->AddBehavior(animName, behName, loop != 0, randomStartPos != 0, fadeInLen, fadeOutLen);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_WKEY(ptCritterBrain, startBehavior, args, keywords)
{
	char* kwlist[] = {"behaviorName", "fade", NULL};
	PyObject* behNameObj = NULL;
	char fade = 1;
	if (!PyArg_ParseTupleAndKeywords(args, keywords, "O|b", kwlist, &behNameObj, &fade))
	{
		PyErr_SetString(PyExc_TypeError, "startBehavior expects a string, and an optional boolean");
		PYTHON_RETURN_NONE;
	}

	std::string behName = "";
	if (PyUnicode_Check(behNameObj))
	{
		int strLen = PyUnicode_GetSize(behNameObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)behNameObj, text, strLen);
		text[strLen] = L'\0';
		char* cText = hsWStringToString(text);
		behName = cText;
		delete [] cText;
		delete [] text;
	}
	else if (PyString_Check(behNameObj))
		behName = PyString_AsString(behNameObj);
	else
	{
		PyErr_SetString(PyExc_TypeError, "startBehavior expects a string, and an optional boolean");
		PYTHON_RETURN_ERROR;
	}

	self->fThis->StartBehavior(behName, fade != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptCritterBrain, runningBehavior, args)
{
	PyObject* behNameObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &behNameObj))
	{
		PyErr_SetString(PyExc_TypeError, "runningBehavior expects a string");
		PYTHON_RETURN_ERROR;
	}

	if (PyUnicode_Check(behNameObj))
	{
		int strLen = PyUnicode_GetSize(behNameObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)behNameObj, text, strLen);
		text[strLen] = L'\0';
		char* cText = hsWStringToString(text);
		bool retVal = self->fThis->RunningBehavior(cText);
		delete [] cText;
		delete [] text;
		PYTHON_RETURN_BOOL(retVal);
	}
	else if (PyString_Check(behNameObj))
	{
		bool retVal = self->fThis->RunningBehavior(PyString_AsString(behNameObj));
		PYTHON_RETURN_BOOL(retVal);
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "runningBehavior expects a string");
		PYTHON_RETURN_ERROR;
	}
}

PYTHON_METHOD_DEFINITION(ptCritterBrain, behaviorName, args)
{
	int behavior;
	if (!PyArg_ParseTuple(args, "i", &behavior))
	{
		PyErr_SetString(PyExc_TypeError, "behaviorName expects an integer");
		PYTHON_RETURN_ERROR;
	}
	return PyString_FromString(self->fThis->BehaviorName(behavior).c_str());
}

PYTHON_METHOD_DEFINITION(ptCritterBrain, animationName, args)
{
	int behavior;
	if (!PyArg_ParseTuple(args, "i", &behavior))
	{
		PyErr_SetString(PyExc_TypeError, "animationName expects an integer");
		PYTHON_RETURN_ERROR;
	}
	return PyString_FromString(self->fThis->AnimationName(behavior).c_str());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptCritterBrain, curBehavior)
{
	return PyInt_FromLong(self->fThis->CurBehavior());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptCritterBrain, nextBehavior)
{
	return PyInt_FromLong(self->fThis->NextBehavior());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptCritterBrain, idleBehaviorName)
{
	return PyString_FromString(self->fThis->IdleBehaviorName().c_str());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptCritterBrain, runBehaviorName)
{
	return PyString_FromString(self->fThis->RunBehaviorName().c_str());
}

PYTHON_METHOD_DEFINITION_WKEY(ptCritterBrain, goToGoal, args, keywords)
{
	char* kwlist[] = {"newGoal", "avoidingAvatars", NULL};
	PyObject* goalObj = NULL;
	char avoidingAvatars = 0;
	if (!PyArg_ParseTupleAndKeywords(args, keywords, "O|b", kwlist, &goalObj, &avoidingAvatars))
	{
		PyErr_SetString(PyExc_TypeError, "goToGoal expects a ptPoint and an optional boolean.");
		PYTHON_RETURN_ERROR;
	}

	if (!pyPoint3::Check(goalObj))
	{
		PyErr_SetString(PyExc_TypeError, "goToGoal expects a ptPoint and an optional boolean.");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->GoToGoal(pyPoint3::ConvertFrom(goalObj)->fPoint, avoidingAvatars != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptCritterBrain, currentGoal)
{
	return self->fThis->CurrentGoal();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptCritterBrain, avoidingAvatars)
{
	PYTHON_RETURN_BOOL(self->fThis->AvoidingAvatars());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptCritterBrain, atGoal)
{
	PYTHON_RETURN_BOOL(self->fThis->AtGoal());
}

PYTHON_METHOD_DEFINITION(ptCritterBrain, setStopDistance, args)
{
	float dist;
	if (!PyArg_ParseTuple(args, "f", &dist))
	{
		PyErr_SetString(PyExc_TypeError, "setStopDistance expects a float");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->StopDistance(dist);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptCritterBrain, getStopDistance)
{
	return PyFloat_FromDouble(self->fThis->StopDistance());
}

PYTHON_METHOD_DEFINITION(ptCritterBrain, setSightCone, args)
{
	float dist;
	if (!PyArg_ParseTuple(args, "f", &dist))
	{
		PyErr_SetString(PyExc_TypeError, "setSightCone expects a float");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SightCone(dist);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptCritterBrain, getSightCone)
{
	return PyFloat_FromDouble(self->fThis->SightCone());
}

PYTHON_METHOD_DEFINITION(ptCritterBrain, setSightDistance, args)
{
	float dist;
	if (!PyArg_ParseTuple(args, "f", &dist))
	{
		PyErr_SetString(PyExc_TypeError, "setSightDistance expects a float");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SightDistance(dist);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptCritterBrain, getSightDistance)
{
	return PyFloat_FromDouble(self->fThis->SightDistance());
}

PYTHON_METHOD_DEFINITION(ptCritterBrain, setHearingDistance, args)
{
	float dist;
	if (!PyArg_ParseTuple(args, "f", &dist))
	{
		PyErr_SetString(PyExc_TypeError, "setHearingDistance expects a float");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->HearingDistance(dist);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptCritterBrain, getHearingDistance)
{
	return PyFloat_FromDouble(self->fThis->HearingDistance());
}

PYTHON_METHOD_DEFINITION(ptCritterBrain, canSeeAvatar, args)
{
	unsigned long id;
	if (!PyArg_ParseTuple(args, "k", &id))
	{
		PyErr_SetString(PyExc_TypeError, "canSeeAvatar expects an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	PYTHON_RETURN_BOOL(self->fThis->CanSeeAvatar(id));
}

PYTHON_METHOD_DEFINITION(ptCritterBrain, canHearAvatar, args)
{
	unsigned long id;
	if (!PyArg_ParseTuple(args, "k", &id))
	{
		PyErr_SetString(PyExc_TypeError, "canHearAvatar expects an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	PYTHON_RETURN_BOOL(self->fThis->CanHearAvatar(id));
}

PYTHON_METHOD_DEFINITION_NOARGS(ptCritterBrain, playersICanSee)
{
	return self->fThis->PlayersICanSee();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptCritterBrain, playersICanHear)
{
	return self->fThis->PlayersICanHear();
}

PYTHON_METHOD_DEFINITION(ptCritterBrain, vectorToPlayer, args)
{
	unsigned long id;
	if (!PyArg_ParseTuple(args, "k", &id))
	{
		PyErr_SetString(PyExc_TypeError, "vectorToPlayer expects an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	return self->fThis->VectorToPlayer(id);
}

PYTHON_START_METHODS_TABLE(ptCritterBrain)
	PYTHON_METHOD(ptCritterBrain, addReceiver, "Params: key\nTells the brain that the specified key wants AI messages"),
	PYTHON_METHOD(ptCritterBrain, removeReceiver, "Params: key\nTells the brain that the specified key no longer wants AI messages"),
	PYTHON_METHOD(ptCritterBrain, setLocallyControlled, "Params: local\nTells the brain that we are the ones making all the AI decisions, and to prop location "
		"and other information to the server."),
	PYTHON_METHOD_NOARGS(ptCritterBrain, getLocallyControlled, "Are we the one making AI decisions? NOTE: Not set automatically, some python script needs to "
		"tell the brain this using setLocallyControlled()."),
	PYTHON_METHOD_WKEY(ptCritterBrain, addBehavior, "Params: animName, behaviorName, loop = 1, randomStartPos = 1, fadeInLen = 2.0, fadeOutLen = 2.0\n"
		"Adds a new animation to the brain as a behavior with the specified name and parameters. If multiple animations are assigned to the same behavior, "
		"they will be randomly picked from when started."),
	PYTHON_METHOD_WKEY(ptCritterBrain, startBehavior, "Params: behaviorName, fade = 1\nStarts playing the named behavior. If fade is true, it will fade out "
		"the previous behavior and fade in the new one. If false, they will immediately switch."),
	PYTHON_METHOD(ptCritterBrain, runningBehavior, "Params: behaviorName\nReturns true if the named behavior is running."),
	PYTHON_METHOD(ptCritterBrain, behaviorName, "Params: behavior\nReturns the behavior name associated with the specified integral behavior."),
	PYTHON_METHOD(ptCritterBrain, animationName, "Params: behavior\nReturns the animation name associated with the specified integral behavior."),
	PYTHON_METHOD_NOARGS(ptCritterBrain, curBehavior, "Returns the current integral behavior the brain is running."),
	PYTHON_METHOD_NOARGS(ptCritterBrain, nextBehavior, "Returns the behavior the brain will be switching to next frame. (-1 if no change)"),
	PYTHON_METHOD_NOARGS(ptCritterBrain, idleBehaviorName, "Returns the name of the brain's idle behavior."),
	PYTHON_METHOD_NOARGS(ptCritterBrain, runBehaviorName, "Returns the name of the brain's run behavior."),
	PYTHON_METHOD_WKEY(ptCritterBrain, goToGoal, "Params: newGoal, avoidingAvatars = 0\nTells the brain to start running towards the specified location, "
		"avoiding avatars it can see or hear if told to."),
	PYTHON_METHOD_NOARGS(ptCritterBrain, currentGoal, "Returns the current ptPoint that the brain is running towards."),
	PYTHON_METHOD_NOARGS(ptCritterBrain, avoidingAvatars, "Are we currently avoiding avatars while pathfinding?"),
	PYTHON_METHOD_NOARGS(ptCritterBrain, atGoal, "Are we currently are our final destination?"),
	PYTHON_METHOD(ptCritterBrain, setStopDistance, "Params: dist\nSet how far away from the goal we should be when we are considered there and stop running."),
	PYTHON_METHOD_NOARGS(ptCritterBrain, getStopDistance, "Returns how far away from the goal we could be and still be considered there."),
	PYTHON_METHOD(ptCritterBrain, setSightCone, "Params: radians\nSet how wide the brain's field of view is in radians. Note that it is the total angle of the "
		"cone, half on one side of the brain's line of sight, half on the other."),
	PYTHON_METHOD_NOARGS(ptCritterBrain, getSightCone, "Returns the width of the brain's field of view in radians."),
	PYTHON_METHOD(ptCritterBrain, setSightDistance, "Params: dist\nSet how far away the brain can see."),
	PYTHON_METHOD_NOARGS(ptCritterBrain, getSightDistance, "Returns how far the brain can see."),
	PYTHON_METHOD(ptCritterBrain, setHearingDistance, "Params: dist\nSet how far away the brain can hear (360 degree field of hearing)."),
	PYTHON_METHOD_NOARGS(ptCritterBrain, getHearingDistance, "Returns how far away the brain can hear."),
	PYTHON_METHOD(ptCritterBrain, canSeeAvatar, "Params: avatarID\nReturns whether this brain can see the avatar with the specified id."),
	PYTHON_METHOD(ptCritterBrain, canHearAvatar, "Params: avatarID\nReturns whether this brain can hear the avatar with the specified id."),
	PYTHON_METHOD_NOARGS(ptCritterBrain, playersICanSee, "Returns a list of player ids which this brain can see."),
	PYTHON_METHOD_NOARGS(ptCritterBrain, playersICanHear, "Returns a list of player ids which this brain can hear."),
	PYTHON_METHOD(ptCritterBrain, vectorToPlayer, "Params: avatarID\nReturns the vector between us and the specified player."),
PYTHON_END_METHODS_TABLE;

// Type structure definition
#define ptCritterBrain_COMPARE		PYTHON_NO_COMPARE
#define ptCritterBrain_AS_NUMBER	PYTHON_NO_AS_NUMBER
#define ptCritterBrain_AS_SEQUENCE	PYTHON_NO_AS_SEQUENCE
#define ptCritterBrain_AS_MAPPING	PYTHON_NO_AS_MAPPING
#define ptCritterBrain_STR			PYTHON_NO_STR
#define ptCritterBrain_RICH_COMPARE	PYTHON_DEFAULT_RICH_COMPARE(ptCritterBrain)
#define ptCritterBrain_GETSET		PYTHON_NO_GETSET
#define ptCritterBrain_BASE			PYTHON_NO_BASE
PLASMA_CUSTOM_TYPE(ptCritterBrain, "Object to manipulate critter brains");

// required functions for PyObject interoperability
PyObject* pyCritterBrain::New(plAvBrainCritter* brain)
{
	ptCritterBrain *newObj = (ptCritterBrain*)ptCritterBrain_type.tp_new(&ptCritterBrain_type, NULL, NULL);
	newObj->fThis->fBrain = brain;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptCritterBrain, pyCritterBrain)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptCritterBrain, pyCritterBrain)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyCritterBrain::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptCritterBrain);
	PYTHON_CLASS_IMPORT_END(m);
}