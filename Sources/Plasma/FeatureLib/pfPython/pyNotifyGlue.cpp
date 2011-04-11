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
#include "pyNotify.h"
#include "pyEnum.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptNotify, pyNotify);

PYTHON_DEFAULT_NEW_DEFINITION(ptNotify, pyNotify)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptNotify)

PYTHON_INIT_DEFINITION(ptNotify, args, keywords)
{
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "__init__ expects a ptKey");
		PYTHON_RETURN_INIT_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "__init__ expects a ptKey");
		PYTHON_RETURN_INIT_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	self->fThis->SetSender(*key);
	PYTHON_RETURN_INIT_OK;
}

PYTHON_BASIC_METHOD_DEFINITION(ptNotify, clearReceivers, ClearReceivers)

PYTHON_METHOD_DEFINITION(ptNotify, addReceiver, args)
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
	self->fThis->AddReceiver(key);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptNotify, netPropagate, args)
{
	char netFlag;
	if (!PyArg_ParseTuple(args, "b", &netFlag))
	{
		PyErr_SetString(PyExc_TypeError, "netPropagate expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetNetPropagate(netFlag != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptNotify, netForce, args)
{
	char netFlag;
	if (!PyArg_ParseTuple(args, "b", &netFlag))
	{
		PyErr_SetString(PyExc_TypeError, "netForce expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetNetForce(netFlag != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptNotify, setActivate, args)
{
	float actState;
	if (!PyArg_ParseTuple(args, "f", &actState))
	{
		PyErr_SetString(PyExc_TypeError, "setActivate expects a float");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetActivateState(actState);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptNotify, setType, args)
{
	long msgType;
	if (!PyArg_ParseTuple(args, "l", &msgType))
	{
		PyErr_SetString(PyExc_TypeError, "setType expects a long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetType(msgType);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptNotify, addCollisionEvent, args)
{
	char enterFlag;
	PyObject* hitterKey = NULL;
	PyObject* hitteeKey = NULL;
	if (!PyArg_ParseTuple(args, "bOO", &enterFlag, &hitterKey, &hitteeKey))
	{
		PyErr_SetString(PyExc_TypeError, "addCollisionEvent expects a boolean, and two ptKeys");
		PYTHON_RETURN_ERROR;
	}
	if ((!pyKey::Check(hitterKey)) || (!pyKey::Check(hitteeKey)))
	{
		PyErr_SetString(PyExc_TypeError, "addCollisionEvent expects a boolean, and two ptKeys");
		PYTHON_RETURN_ERROR;
	}
	pyKey* hitter = pyKey::ConvertFrom(hitterKey);
	pyKey* hittee = pyKey::ConvertFrom(hitteeKey);
	self->fThis->AddCollisionEvent(enterFlag != 0, hitter, hittee);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptNotify, addPickEvent, args)
{
	char enabledFlag;
	PyObject* pickerKey = NULL;
	PyObject* pickeeKey = NULL;
	PyObject* hitPointObj = NULL;
	if (!PyArg_ParseTuple(args, "bOOO", &enabledFlag, &pickerKey, &pickeeKey, &hitPointObj))
	{
		PyErr_SetString(PyExc_TypeError, "addPickEvent expects a boolean, two ptKeys and a ptPoint3");
		PYTHON_RETURN_ERROR;
	}
	if ((!pyKey::Check(pickerKey)) || (!pyKey::Check(pickeeKey)) || (!pyPoint3::Check(hitPointObj)))
	{
		PyErr_SetString(PyExc_TypeError, "addPickEvent expects a boolean, two ptKeys and a ptPoint3");
		PYTHON_RETURN_ERROR;
	}
	pyKey* picker = pyKey::ConvertFrom(pickerKey);
	pyKey* pickee = pyKey::ConvertFrom(pickeeKey);
	pyPoint3* hitPoint = pyPoint3::ConvertFrom(hitPointObj);
	self->fThis->AddPickEvent(enabledFlag != 0, picker, pickee, *hitPoint);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptNotify, addControlKeyEvent, args)
{
	long key;
	char down;
	if (!PyArg_ParseTuple(args, "lb", &key, &down))
	{
		PyErr_SetString(PyExc_TypeError, "addControlKeyEvent expects a long and a boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->AddControlKeyEvent(key, down != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptNotify, addVarNumber, args)
{
	char* name;
	float number;
	if (!PyArg_ParseTuple(args, "sf", &name, &number))
	{
		PyErr_SetString(PyExc_TypeError, "addVarNumber expects a string and a float");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->AddVarNumber(name, number);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptNotify, addVarKey, args)
{
	char* name;
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "sO", &name, &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "addVarKey expects a string and a ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "addVarKey expects a string and a ptKey");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	self->fThis->AddVarKey(name, key);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptNotify, addFacingEvent, args)
{
	char enabledFlag;
	PyObject* facerKey = NULL;
	PyObject* faceeKey = NULL;
	float dot;
	if (!PyArg_ParseTuple(args, "bOOf", &enabledFlag, &facerKey, &faceeKey, &dot))
	{
		PyErr_SetString(PyExc_TypeError, "addFacingEvent expects a boolean, two ptKeys, and a float");
		PYTHON_RETURN_ERROR;
	}
	if ((!pyKey::Check(facerKey)) || (!pyKey::Check(faceeKey)))
	{
		PyErr_SetString(PyExc_TypeError, "addFacingEvent expects a boolean, two ptKeys, and a float");
		PYTHON_RETURN_ERROR;
	}
	pyKey* facer = pyKey::ConvertFrom(facerKey);
	pyKey* facee = pyKey::ConvertFrom(faceeKey);
	self->fThis->AddFacingEvent(enabledFlag != 0, facer, facee, dot);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptNotify, addContainerEvent, args)
{
	char enterFlag;
	PyObject* containerKey = NULL;
	PyObject* containedKey = NULL;
	if (!PyArg_ParseTuple(args, "bOO", &enterFlag, &containerKey, &containedKey))
	{
		PyErr_SetString(PyExc_TypeError, "addContainerEvent expects a boolean, and two ptKeys");
		PYTHON_RETURN_ERROR;
	}

	pyKey* container = NULL;
	pyKey* contained = NULL;
	
	if (containerKey != Py_None)
	{
		if (!pyKey::Check(containerKey))
		{
			PyErr_SetString(PyExc_TypeError, "addContainerEvent expects a boolean, and two ptKeys");
			PYTHON_RETURN_ERROR;
		}
		container = pyKey::ConvertFrom(containerKey);
	}

	if (containedKey != Py_None)
	{
		if (!pyKey::Check(containedKey))
		{
			PyErr_SetString(PyExc_TypeError, "addContainerEvent expects a boolean, and two ptKeys");
			PYTHON_RETURN_ERROR;
		}
		contained = pyKey::ConvertFrom(containedKey);
	}

	self->fThis->AddContainerEvent(enterFlag != 0, container, contained);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptNotify, addActivateEvent, args)
{
	char activeFlag, activateFlag;
	if (!PyArg_ParseTuple(args, "bb", &activeFlag, &activateFlag))
	{
		PyErr_SetString(PyExc_TypeError, "addActivateEvent expects two booleans");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->AddActivateEvent(activeFlag != 0, activateFlag != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptNotify, addCallbackEvent, args)
{
	long eventNumber;
	if (!PyArg_ParseTuple(args, "l", &eventNumber))
	{
		PyErr_SetString(PyExc_TypeError, "addCallbackEvent expects a long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->AddCallbackEvent(eventNumber);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptNotify, addResponderState, args)
{
	long respState;
	if (!PyArg_ParseTuple(args, "l", &respState))
	{
		PyErr_SetString(PyExc_TypeError, "addResponderState expects a long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->AddResponderState(respState);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptNotify, send, Send)

PYTHON_START_METHODS_TABLE(ptNotify)
	PYTHON_BASIC_METHOD(ptNotify, clearReceivers, "Remove all the receivers that this Notify message has\n"
				"- receivers are automatically added if from a ptAttribActivator"),
	PYTHON_METHOD(ptNotify, addReceiver, "Params: key\nAdd a receivers key to receive this Notify message"),
	PYTHON_METHOD(ptNotify, netPropagate, "Params: netFlag\nSets the net propagate flag - default to set"),
	PYTHON_METHOD(ptNotify, netForce, "Params: forceFlag\nSpecify whether this object needs to use messages that are forced to the network\n"
				"- This is to be used if your Python program is running on only one client\n"
				"Such as a game master, only running on the client that owns a particular object"),
	PYTHON_METHOD(ptNotify, setActivate, "Params: state\nSet the activate state to true(1.0) or false(0.0)"),
	PYTHON_METHOD(ptNotify, setType, "Params: type\nSets the message type"),
	PYTHON_METHOD(ptNotify, addCollisionEvent, "Params: enterFlag,hitterKey,hitteeKey\nAdd a collision event record to the Notify message"),
	PYTHON_METHOD(ptNotify, addPickEvent, "Params: enabledFlag,pickerKey,pickeeKey,hitPoint\nAdd a pick event record to the Notify message"),
	PYTHON_METHOD(ptNotify, addControlKeyEvent, "Params: keynumber,downFlag\nAdd a keyboard event record to the Notify message"),
	PYTHON_METHOD(ptNotify, addVarNumber, "Params: name,number\nAdd a number variable event record to the Notify message\n"
				"This event record is used to pass a number variable to another python program"),
	PYTHON_METHOD(ptNotify, addVarKey, "Params: name,key\nAdd a ptKey variable event record to the Notify message\n"
				"This event record is used to pass a ptKey variable to another python program"),
	PYTHON_METHOD(ptNotify, addFacingEvent, "Params: enabledFlag,facerKey, faceeKey, dotProduct\nAdd a facing event record to the Notify message"),
	PYTHON_METHOD(ptNotify, addContainerEvent, "Params: enteringFlag,containerKey,containedKey\nAdd a container event record to the notify message"),
	PYTHON_METHOD(ptNotify, addActivateEvent, "Params: activeFlag,activateFlag\nAdd an activate event record to the notify message"),
	PYTHON_METHOD(ptNotify, addCallbackEvent, "Params: eventNumber\nAdd a callback event record to the notify message"),
	PYTHON_METHOD(ptNotify, addResponderState, "Params: state\nAdd a responder state event record to the notify message"),
	PYTHON_BASIC_METHOD(ptNotify, send, "Send the notify message"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptNotify, "Params: selfKey\nCreates a Notify message\n"
			"- selfKey is ptKey of your PythonFile modifier");

// required functions for PyObject interoperability
PyObject *pyNotify::New(pyKey& selfkey)
{
	ptNotify *newObj = (ptNotify*)ptNotify_type.tp_new(&ptNotify_type, NULL, NULL);
	newObj->fThis->SetSender(selfkey);
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptNotify, pyNotify)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptNotify, pyNotify)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyNotify::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptNotify);
	PYTHON_CLASS_IMPORT_END(m);
}

void pyNotify::AddPlasmaConstantsClasses(PyObject *m)
{
	PYTHON_ENUM_START(PtNotificationType);
	PYTHON_ENUM_ELEMENT(PtNotificationType, kActivator,				plNotifyMsg::kActivator);
	PYTHON_ENUM_ELEMENT(PtNotificationType, kVarNotification,		plNotifyMsg::kVarNotification);
	PYTHON_ENUM_ELEMENT(PtNotificationType, kNotifySelf,			plNotifyMsg::kNotifySelf);
	PYTHON_ENUM_ELEMENT(PtNotificationType, kResponderFF,			plNotifyMsg::kResponderFF);
	PYTHON_ENUM_ELEMENT(PtNotificationType, kResponderChangeState,	plNotifyMsg::kResponderChangeState);
	PYTHON_ENUM_END(m, PtNotificationType);

	PYTHON_ENUM_START(PtEventType);
	PYTHON_ENUM_ELEMENT(PtEventType, kCollision,		proEventData::kCollision);
	PYTHON_ENUM_ELEMENT(PtEventType, kPicked,			proEventData::kPicked);
	PYTHON_ENUM_ELEMENT(PtEventType, kControlKey,		proEventData::kControlKey);
	PYTHON_ENUM_ELEMENT(PtEventType, kVariable,			proEventData::kVariable);
	PYTHON_ENUM_ELEMENT(PtEventType, kFacing,			proEventData::kFacing);
	PYTHON_ENUM_ELEMENT(PtEventType, kContained,		proEventData::kContained);
	PYTHON_ENUM_ELEMENT(PtEventType, kActivate,			proEventData::kActivate);
	PYTHON_ENUM_ELEMENT(PtEventType, kCallback,			proEventData::kCallback);
	PYTHON_ENUM_ELEMENT(PtEventType, kResponderState,	proEventData::kResponderState);
	PYTHON_ENUM_ELEMENT(PtEventType, kMultiStage,		proEventData::kMultiStage);
	PYTHON_ENUM_ELEMENT(PtEventType, kSpawned,			proEventData::kSpawned);
	PYTHON_ENUM_ELEMENT(PtEventType, kClickDrag,		proEventData::kClickDrag);
	PYTHON_ENUM_ELEMENT(PtEventType, kOfferLinkingBook,	proEventData::kOfferLinkingBook);
	PYTHON_ENUM_ELEMENT(PtEventType, kBook,				proEventData::kBook);
	PYTHON_ENUM_END(m, PtEventType);

	PYTHON_ENUM_START(PtNotifyDataType);
	PYTHON_ENUM_ELEMENT(PtNotifyDataType, kNumber,	proEventData::kNumber);
	PYTHON_ENUM_ELEMENT(PtNotifyDataType, kKey,		proEventData::kKey);
	PYTHON_ENUM_END(m, PtNotifyDataType);

	PYTHON_ENUM_START(PtMultiStageEventType);
	PYTHON_ENUM_ELEMENT(PtMultiStageEventType, kEnterStage,			proEventData::kEnterStage);
	PYTHON_ENUM_ELEMENT(PtMultiStageEventType, kBeginingOfLoop,		proEventData::kBeginingOfLoop);
	PYTHON_ENUM_ELEMENT(PtMultiStageEventType, kAdvanceNextStage,	proEventData::kAdvanceNextStage);
	PYTHON_ENUM_ELEMENT(PtMultiStageEventType, kRegressPrevStage,	proEventData::kRegressPrevStage);
	PYTHON_ENUM_END(m, PtMultiStageEventType);
}