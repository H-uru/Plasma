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
#include "cyAvatar.h"
#include "pyKey.h"
#include "pyEnum.h"
#include "pyColor.h"
#include "pySceneObject.h"

#include "../plAvatar/plAvBrainHuman.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptAvatar, cyAvatar);

PYTHON_DEFAULT_NEW_DEFINITION(ptAvatar, cyAvatar);
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptAvatar);

PYTHON_NO_INIT_DEFINITION(ptAvatar);

PYTHON_METHOD_DEFINITION(ptAvatar, netForce, args)
{
	char forceFlag;
	if (!PyArg_ParseTuple(args, "b", &forceFlag))
	{
		PyErr_SetString(PyExc_TypeError, "netForce expects a boolean");
		PYTHON_RETURN_ERROR;
	}

	self->fThis->SetNetForce(forceFlag != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAvatar, oneShot, args)
{
	PyObject* keyObj = NULL;
	float duration;
	char usePhysics;
	char* animName = NULL;
	char drivable, reversable;
	if (!PyArg_ParseTuple(args, "Ofbsbb", &keyObj, &duration, &usePhysics, &animName, &drivable, &reversable))
	{
		PyErr_SetString(PyExc_TypeError, "oneShot expects a ptKey, float, boolean, string, and two booleans");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "oneShot expects a ptKey, float, boolean, string, and two booleans");
		PYTHON_RETURN_ERROR;
	}

	pyKey* key = pyKey::ConvertFrom(keyObj);
	std::string animNameStr = animName; // convert to string (for safety)
	self->fThis->OneShot(*key, duration, usePhysics != 0, animNameStr.c_str(), drivable != 0, reversable != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAvatar, runBehavior, args)
{
	PyObject* keyObj = NULL;
	char netForce;
	char netProp = 1;
	if (!PyArg_ParseTuple(args, "Ob|b", &keyObj, &netForce, &netProp))
	{
		PyErr_SetString(PyExc_TypeError, "runBehavior expects a ptKey and a boolean and an optional boolean");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "runBehavior expects a ptKey and a boolean and an optional boolean");
		PYTHON_RETURN_ERROR;
	}

	pyKey* key = pyKey::ConvertFrom(keyObj);
	self->fThis->RunBehavior(*key, netForce != 0, netProp != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAvatar, runBehaviorSetNotify, args)
{
	PyObject* behKeyObj = NULL;
	PyObject* replyKeyObj = NULL;
	char netForce;
	char netProp = 1;
	if (!PyArg_ParseTuple(args, "OOb|b", &behKeyObj, &replyKeyObj, &netForce, &netProp))
	{
		PyErr_SetString(PyExc_TypeError, "runBehaviorSetNotify expects two ptKeys and a boolean and an optional boolean");
		PYTHON_RETURN_ERROR;
	}
	if ((!pyKey::Check(behKeyObj)) || (!pyKey::Check(replyKeyObj)))
	{
		PyErr_SetString(PyExc_TypeError, "runBehaviorSetNotify expects two ptKeys and a boolean and an optional boolean");
		PYTHON_RETURN_ERROR;
	}

	pyKey* behKey = pyKey::ConvertFrom(behKeyObj);
	pyKey* replyKey = pyKey::ConvertFrom(replyKeyObj);
	self->fThis->RunBehaviorAndReply(*behKey, *replyKey, netForce != 0, netProp != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAvatar, nextStage, args)
{
	PyObject* keyObj = NULL;
	float transTime;
	char setTime;
	float newTime;
	char setDirection, isForward, netForce;
	if (!PyArg_ParseTuple(args, "Ofbfbbb", &keyObj, &transTime, &setTime, &newTime, &setDirection, &isForward, &netForce))
	{
		PyErr_SetString(PyExc_TypeError, "nextStage expects a ptkey, float, bool, float, and three booleans");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "nextStage expects a ptkey, float, bool, float, and three booleans");
		PYTHON_RETURN_ERROR;
	}

	pyKey* key = pyKey::ConvertFrom(keyObj);
	self->fThis->NextStage(*key, transTime, setTime != 0, newTime, setDirection != 0, isForward != 0, netForce != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAvatar, previousStage, args)
{
	PyObject* keyObj = NULL;
	float transTime;
	char setTime;
	float newTime;
	char setDirection, isForward, netForce;
	if (!PyArg_ParseTuple(args, "Ofbfbbb", &keyObj, &transTime, &setTime, &newTime, &setDirection, &isForward, &netForce))
	{
		PyErr_SetString(PyExc_TypeError, "previousStage expects a ptkey, float, bool, float, and three booleans");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "previousStage expects a ptkey, float, bool, float, and three booleans");
		PYTHON_RETURN_ERROR;
	}

	pyKey* key = pyKey::ConvertFrom(keyObj);
	self->fThis->PreviousStage(*key, transTime, setTime != 0, newTime, setDirection != 0, isForward != 0, netForce != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAvatar, gotoStage, args)
{
	PyObject* keyObj = NULL;
	long stage;
	float transTime;
	char setTime;
	float newTime;
	char setDirection, isForward, netForce;
	if (!PyArg_ParseTuple(args, "Olfbfbbb", &keyObj, &stage, &transTime, &setTime, &newTime, &setDirection, &isForward, &netForce))
	{
		PyErr_SetString(PyExc_TypeError, "previousStage expects a ptkey, long, float, bool, float, and three booleans");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "previousStage expects a ptkey, long, float, bool, float, and three booleans");
		PYTHON_RETURN_ERROR;
	}

	pyKey* key = pyKey::ConvertFrom(keyObj);
	self->fThis->GoToStage(*key, stage, transTime, setTime != 0, newTime, setDirection != 0, isForward != 0, netForce != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAvatar, getAvatarClothingGroup)
{
	return PyLong_FromLong(self->fThis->GetAvatarClothingGroup());
}

PYTHON_METHOD_DEFINITION(ptAvatar, getEntireClothingList, args)
{
	long clothingType;
	if (!PyArg_ParseTuple(args, "l", &clothingType))
	{
		PyErr_SetString(PyExc_TypeError, "getEntireClothingList expects a long");
		PYTHON_RETURN_ERROR;
	}

	std::vector<std::string> clothingList = self->fThis->GetEntireClothingList(clothingType);
	PyObject* retVal = PyList_New(clothingList.size());
	for (int i = 0; i < clothingList.size(); i++)
		PyList_SetItem(retVal, i, PyString_FromString(clothingList[i].c_str()));
	return retVal;
}

PYTHON_METHOD_DEFINITION(ptAvatar, getClosetClothingList, args)
{
	long clothingType;
	if (!PyArg_ParseTuple(args, "l", &clothingType))
	{
		PyErr_SetString(PyExc_TypeError, "getClosetCothingList expects a long");
		PYTHON_RETURN_ERROR;
	}

	std::vector<PyObject*> clothingList = self->fThis->GetClosetClothingList(clothingType);
	PyObject* retVal = PyList_New(clothingList.size());
	for (int i = 0; i < clothingList.size(); i++)
		PyList_SetItem(retVal, i, clothingList[i]); // steals the ref, so no need to decref
	return retVal;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAvatar, getAvatarClothingList)
{
	std::vector<PyObject*> clothingList = self->fThis->GetAvatarClothingList();
	PyObject* retVal = PyList_New(clothingList.size());
	for (int i = 0; i < clothingList.size(); i++)
		PyList_SetItem(retVal, i, clothingList[i]); // steals the ref, so no need to decref
	return retVal;
}

PYTHON_METHOD_DEFINITION(ptAvatar, getMatchingClothingItem, args)
{
	char* clothingName = NULL;
	if (!PyArg_ParseTuple(args, "s", &clothingName))
	{
		PyErr_SetString(PyExc_TypeError, "getMatchingClothingItem expects a string");
		PYTHON_RETURN_ERROR;
	}

	std::string clothingNameStr = clothingName; // convert to string (for safety)
	return self->fThis->GetMatchingClothingItem(clothingNameStr.c_str());
}

PYTHON_METHOD_DEFINITION(ptAvatar, wearClothingItem, args)
{
	char* clothingName = NULL;
	char update = 1;
	if (!PyArg_ParseTuple(args, "s|b", &clothingName, &update))
	{
		PyErr_SetString(PyExc_TypeError, "wearClothingItem expects a string and an optional boolean");
		PYTHON_RETURN_ERROR;
	}

	std::string clothingNameStr = clothingName; // convert to string (for safety)
	PYTHON_RETURN_BOOL(self->fThis->WearClothingItemU(clothingNameStr.c_str(), update != 0));
}

PYTHON_METHOD_DEFINITION(ptAvatar, removeClothingItem, args)
{
	char* clothingName = NULL;
	char update = 1;
	if (!PyArg_ParseTuple(args, "s|b", &clothingName, &update))
	{
		PyErr_SetString(PyExc_TypeError, "removeClothingItem expects a string and an optional boolean");
		PYTHON_RETURN_ERROR;
	}

	std::string clothingNameStr = clothingName; // convert to string (for safety)
	PYTHON_RETURN_BOOL(self->fThis->RemoveClothingItemU(clothingNameStr.c_str(), update != 0));
}

PYTHON_METHOD_DEFINITION(ptAvatar, tintClothingItem, args)
{
	char* clothingName = NULL;
	PyObject* tintObj = NULL;
	char update = 1;
	if (!PyArg_ParseTuple(args, "sO|b", &clothingName, &tintObj, &update))
	{
		PyErr_SetString(PyExc_TypeError, "tintClothingItem expects a string, a ptColor, and an optional boolean");
		PYTHON_RETURN_ERROR;
	}
	if (!pyColor::Check(tintObj))
	{
		PyErr_SetString(PyExc_TypeError, "tintClothingItem expects a string, a ptColor, and an optional boolean");
		PYTHON_RETURN_ERROR;
	}

	std::string clothingNameStr = clothingName; // convert to string (for safety)
	pyColor* tint = pyColor::ConvertFrom(tintObj);
	PYTHON_RETURN_BOOL(self->fThis->TintClothingItemU(clothingNameStr.c_str(), *tint, update != 0));
}

PYTHON_METHOD_DEFINITION(ptAvatar, tintClothingItemLayer, args)
{
	char* clothingName = NULL;
	PyObject* tintObj = NULL;
	unsigned char layer;
	char update = 1;
	if (!PyArg_ParseTuple(args, "sOB|b", &clothingName, &tintObj, &layer, &update))
	{
		PyErr_SetString(PyExc_TypeError, "tintClothingItemLayer expects a string, a ptColor, an unsigned 8-bit int, and an optional boolean");
		PYTHON_RETURN_ERROR;
	}
	if (!pyColor::Check(tintObj))
	{
		PyErr_SetString(PyExc_TypeError, "tintClothingItemLayer expects a string, a ptColor, an unsigned 8-bit int, and an optional boolean");
		PYTHON_RETURN_ERROR;
	}

	std::string clothingNameStr = clothingName; // convert to string (for safety)
	pyColor* tint = pyColor::ConvertFrom(tintObj);
	PYTHON_RETURN_BOOL(self->fThis->TintClothingItemLayerU(clothingNameStr.c_str(), *tint, layer, update != 0));
}

PYTHON_METHOD_DEFINITION(ptAvatar, getTintClothingItem, args)
{
	char* clothingName = NULL;
	unsigned char layer = 1;
	if (!PyArg_ParseTuple(args, "s|B", &clothingName, &layer))
	{
		PyErr_SetString(PyExc_TypeError, "getTintClothingItem expects a string and an optional unsigned 8-bit int");
		PYTHON_RETURN_NONE;
	}
	
	std::string clothingNameStr = clothingName; // convert to string (for safety)
	return self->fThis->GetTintClothingItemL(clothingNameStr.c_str(), layer);
}

PYTHON_METHOD_DEFINITION(ptAvatar, tintSkin, args)
{
	PyObject* tintObj = NULL;
	char update = 1;
	if (!PyArg_ParseTuple(args, "O|b", &tintObj, &update))
	{
		PyErr_SetString(PyExc_TypeError, "tintSkin expects a ptColor and an optional boolean");
		PYTHON_RETURN_NONE;
	}
	if (!pyColor::Check(tintObj))
	{
		PyErr_SetString(PyExc_TypeError, "tintSkin expects a ptColor and an optional boolean");
		PYTHON_RETURN_NONE;
	}

	pyColor* tint = pyColor::ConvertFrom(tintObj);
	self->fThis->TintSkinU(*tint, update != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAvatar, getTintSkin)
{
	return self->fThis->GetTintSkin();
}

PYTHON_METHOD_DEFINITION(ptAvatar, enterSubWorld, args)
{
	PyObject* sceneObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &sceneObj))
	{
		PyErr_SetString(PyExc_TypeError, "enterSubWorld expects a ptSceneObject");
		PYTHON_RETURN_ERROR;
	}
	if (!pySceneObject::Check(sceneObj))
	{
		PyErr_SetString(PyExc_TypeError, "enterSubWorld expects a ptSceneObject");
		PYTHON_RETURN_ERROR;
	}

	pySceneObject* target = pySceneObject::ConvertFrom(sceneObj);
	self->fThis->EnterSubWorld(*target);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptAvatar, exitSubWorld, ExitSubWorld)

PYTHON_METHOD_DEFINITION(ptAvatar, setMorph, args)
{
	char* clothingName = NULL;
	unsigned char layer;
	float value;
	if (!PyArg_ParseTuple(args, "sBf", &clothingName, &layer, &value))
	{
		PyErr_SetString(PyExc_TypeError, "setMorph expects a string, unsigned 8-bit int, and a float");
		PYTHON_RETURN_ERROR;
	}

	std::string clothingNameStr = clothingName; // convert to string (for safety)
	self->fThis->SetMorph(clothingNameStr.c_str(), layer, value);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAvatar, getMorph, args)
{
	char* clothingName = NULL;
	unsigned char layer;
	if (!PyArg_ParseTuple(args, "sB", &clothingName, &layer))
	{
		PyErr_SetString(PyExc_TypeError, "getMorph expects a string, and an unsignd 8-bit int");
		PYTHON_RETURN_ERROR;
	}

	std::string clothingNameStr = clothingName; // convert to string (for safety)
	return PyFloat_FromDouble(self->fThis->GetMorph(clothingNameStr.c_str(), layer));
}

PYTHON_METHOD_DEFINITION(ptAvatar, setSkinBlend, args)
{
	unsigned char layer;
	float value;
	if (!PyArg_ParseTuple(args, "Bf", &layer, &value))
	{
		PyErr_SetString(PyExc_TypeError, "setSkinBlend expects an unsigned 8-bit int and a float");
		PYTHON_RETURN_ERROR;
	}

	self->fThis->SetSkinBlend(layer, value);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAvatar, getSkinBlend, args)
{
	unsigned char layer;
	if (!PyArg_ParseTuple(args, "B", &layer))
	{
		PyErr_SetString(PyExc_TypeError, "getSkinBlend expects an unsigned 8-bit int");
		PYTHON_RETURN_ERROR;
	}

	return PyFloat_FromDouble(self->fThis->GetSkinBlend(layer));
}

PYTHON_BASIC_METHOD_DEFINITION(ptAvatar, saveClothing, SaveClothing)

PYTHON_METHOD_DEFINITION(ptAvatar, getUniqueMeshList, args)
{
	long clothingType;
	if (!PyArg_ParseTuple(args, "l", &clothingType))
	{
		PyErr_SetString(PyExc_TypeError, "getUniqueMeshList expects a long");
		PYTHON_RETURN_ERROR;
	}

	std::vector<PyObject*> clothingList = self->fThis->GetUniqueMeshList(clothingType);
	PyObject* retVal = PyList_New(clothingList.size());
	for (int i = 0; i < clothingList.size(); i++)
		PyList_SetItem(retVal, i, clothingList[i]); // steals the ref, so no need to decref
	return retVal;
}

PYTHON_METHOD_DEFINITION(ptAvatar, getAllWithSameMesh, args)
{
	char* clothingName = NULL;
	if (!PyArg_ParseTuple(args, "s", &clothingName))
	{
		PyErr_SetString(PyExc_TypeError, "getAllWithSameMesh expects a string");
		PYTHON_RETURN_ERROR;
	}

	std::string clothingNameStr = clothingName; // convert to string (for safety)
	std::vector<PyObject*> clothingList = self->fThis->GetAllWithSameMesh(clothingNameStr.c_str());
	PyObject* retVal = PyList_New(clothingList.size());
	for (int i = 0; i < clothingList.size(); i++)
		PyList_SetItem(retVal, i, clothingList[i]); // steals the ref, so no need to decref
	return retVal;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAvatar, getWardrobeClothingList)
{
	std::vector<PyObject*> clothingList = self->fThis->GetWardrobeClothingList();
	PyObject* retVal = PyList_New(clothingList.size());
	for (int i = 0; i < clothingList.size(); i++)
		PyList_SetItem(retVal, i, clothingList[i]); // steals the ref, so no need to decref
	return retVal;
}

PYTHON_METHOD_DEFINITION(ptAvatar, addWardrobeClothingItem, args)
{
	char* clothingName = NULL;
	PyObject* tint1Obj = NULL;
	PyObject* tint2Obj = NULL;
	if (!PyArg_ParseTuple(args, "sOO", &clothingName, &tint1Obj, &tint2Obj))
	{
		PyErr_SetString(PyExc_TypeError, "addWardrobeClothingItem expects a string and two ptColor objects");
		PYTHON_RETURN_ERROR;
	}
	if ((!pyColor::Check(tint1Obj)) || (!pyColor::Check(tint2Obj)))
	{
		PyErr_SetString(PyExc_TypeError, "addWardrobeClothingItem expects a string and two ptColor objects");
		PYTHON_RETURN_ERROR;
	}

	std::string clothingNameStr = clothingName; // convert to string (for safety)
	pyColor* tint1 = pyColor::ConvertFrom(tint1Obj);
	pyColor* tint2 = pyColor::ConvertFrom(tint2Obj);
	self->fThis->AddWardrobeClothingItem(clothingNameStr.c_str(), *tint1, *tint2);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAvatar, setReplyKey, args)
{
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "setReplyKey expects a ptKey object");
		PYTHON_RETURN_NONE;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "setReplyKey expects a ptKey object");
		PYTHON_RETURN_NONE;
	}

	pyKey* key = pyKey::ConvertFrom(keyObj);
	self->fThis->SetSenderKey(*key);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAvatar, getCurrentMode)
{
	return PyInt_FromLong(self->fThis->GetCurrentMode());
}

PYTHON_METHOD_DEFINITION(ptAvatar, registerForBehaviorNotify, args)
{
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "registerForBehaviorNotify expects a ptKey object");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "registerForBehaviorNotify expects a ptKey object");
		PYTHON_RETURN_ERROR;
	}

	pyKey* key = pyKey::ConvertFrom(keyObj);
	self->fThis->RegisterForBehaviorNotify(*key);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAvatar, unRegisterForBehaviorNotify, args)
{
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "unRegisterForBehaviorNotify expects a ptKey object");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "unRegisterForBehaviorNotify expects a ptKey object");
		PYTHON_RETURN_ERROR;
	}

	pyKey* key = pyKey::ConvertFrom(keyObj);
	self->fThis->UnRegisterForBehaviorNotify(*key);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAvatar, playSimpleAnimation, args)
{
	char* animName = NULL;
	if (!PyArg_ParseTuple(args, "s", &animName))
	{
		PyErr_SetString(PyExc_TypeError, "playSimpleAnimation expects a string object");
		PYTHON_RETURN_ERROR;
	}

	std::string animNameStr = animName; // convert to a string (for safety)
	self->fThis->PlaySimpleAnimation(animNameStr.c_str());
	PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptAvatar)
	PYTHON_METHOD(ptAvatar, netForce, "Params: forceFlag\nSpecify whether this object needs to use messages that are forced to the network\n"
				"- This is to be used if your Python program is running on only one client\n"
				"Such as a game master, only running on the client that owns a particular object"),
	
	PYTHON_METHOD(ptAvatar, oneShot, "Params: seekKey,duration,usePhysicsFlag,animationName,drivableFlag,reversibleFlag\nPlays a one-shot animation on the avatar"),
	PYTHON_METHOD(ptAvatar, runBehavior, "Params: behaviorKey,netForceFlag\nRuns a behavior on the avatar. Can be a single or multi-stage behavior."),
	PYTHON_METHOD(ptAvatar, runBehaviorSetNotify, "Params: behaviorKey,replyKey,netForceFlag\nSame as runBehavior, except send notifications to specified keyed object"),
	PYTHON_METHOD(ptAvatar, nextStage, "Params: behaviorKey,transitionTime,setTimeFlag,newTime,SetDirectionFlag,isForward,netForce\nTells a multistage behavior to go to the next stage (Why does Matt like so many parameters?)"),
	PYTHON_METHOD(ptAvatar, previousStage, "Params: behaviorKey,transitionTime,setTimeFlag,newTime,SetDirectionFlag,isForward,netForce\nTells a multistage behavior to go to the previous stage"),
	PYTHON_METHOD(ptAvatar, gotoStage, "Params: behaviorKey,stage,transitionTime,setTimeFlag,newTime,SetDirectionFlag,isForward,netForce\nTells a multistage behavior to go to a particular stage"),
	
	PYTHON_METHOD_NOARGS(ptAvatar, getAvatarClothingGroup, "Returns what clothing group the avatar belongs to.\n"
				"It is also a means to determine if avatar is male or female"),
	PYTHON_METHOD(ptAvatar, getEntireClothingList, "Params: clothing_type\nGets the entire list of clothing available. 'clothing_type' not used\n"
				"NOTE: should use getClosetClothingList"),
	PYTHON_METHOD(ptAvatar, getClosetClothingList, "Params: clothing_type\nReturns a list of clothes for the avatar that are in specified clothing group."),
	PYTHON_METHOD_NOARGS(ptAvatar, getAvatarClothingList, "Returns a list of clothes that the avatar is currently wearing."),
	PYTHON_METHOD(ptAvatar, getMatchingClothingItem, "Params: clothingName\nFinds the matching clothing item that goes with 'clothingName'\n"
				"Used to find matching left and right gloves and shoes."),
	PYTHON_METHOD(ptAvatar, wearClothingItem, "Params: clothing_name,update=1\nTells the avatar to wear a particular item of clothing.\n"
				"And optionally hold update until later (for applying tinting before wearing)."),
	PYTHON_METHOD(ptAvatar, removeClothingItem, "Params: clothing_name,update=1\nTells the avatar to remove a particular item of clothing."),
	PYTHON_METHOD(ptAvatar, tintClothingItem, "Params: clothing_name,tint,update=1\nTells the avatar to tint(color) a particular item of clothing that they are already wearing.\n"
				"'tint' is a ptColor object"),
	PYTHON_METHOD(ptAvatar, tintClothingItemLayer, "Params: clothing_name,tint,layer,update=1\nTells the avatar to tint(color) a particular layer of a particular item of clothing."),
	PYTHON_METHOD(ptAvatar, getTintClothingItem, "Params: clothing_name,layer=1\nReturns a ptColor of a particular item of clothing that the avatar is wearing.\n"
				"The color will be a ptColor object."),
	PYTHON_METHOD(ptAvatar, tintSkin, "Params: tint,update=1\nTints all of the skin on the avatar, with the ptColor tint"),
	PYTHON_METHOD_NOARGS(ptAvatar, getTintSkin, "Returns a ptColor of the current skin tint for the avatar"),

	PYTHON_METHOD(ptAvatar, enterSubWorld, "Params: sceneobject\nPlaces the avatar into the subworld of the ptSceneObject specified"),
	PYTHON_BASIC_METHOD(ptAvatar, exitSubWorld, "Exits the avatar from the subWorld where it was"),

	PYTHON_METHOD(ptAvatar, setMorph, "Params: clothing_name,layer,value\nSet the morph value (clipped between -1 and 1)"),
	PYTHON_METHOD(ptAvatar, getMorph, "Params: clothing_name,layer\nGet the current morph value"),
	PYTHON_METHOD(ptAvatar, setSkinBlend, "Params: layer,value\nSet the skin blend (value between 0 and 1)"),
	PYTHON_METHOD(ptAvatar, getSkinBlend, "Params: layer\nGet the current skin blend value"),

	PYTHON_BASIC_METHOD(ptAvatar, saveClothing, "Saves the current clothing options (including morphs) to the vault"),

	PYTHON_METHOD(ptAvatar, getUniqueMeshList, "Params: clothing_type\nReturns a list of unique clothing items of the desired type (different meshes)"),
	PYTHON_METHOD(ptAvatar, getAllWithSameMesh, "Params: clothing_name\nReturns a lilst of all clothing items that use the same mesh as the specified one"),
	PYTHON_METHOD_NOARGS(ptAvatar, getWardrobeClothingList, "Return a list of items that are in the avatars closet"),
	PYTHON_METHOD(ptAvatar, addWardrobeClothingItem, "Params: clothing_name,tint1,tint2\nTo add a clothing item to the avatar's wardrobe (closet)"),

	PYTHON_METHOD(ptAvatar, setReplyKey, "Params: key\nSets the sender's key"),

	PYTHON_METHOD_NOARGS(ptAvatar, getCurrentMode, "Returns current brain mode for avatar"),
	
	PYTHON_METHOD(ptAvatar, registerForBehaviorNotify, "Params: selfKey\nThis will register for behavior notifies from the avatar"),
	PYTHON_METHOD(ptAvatar, unRegisterForBehaviorNotify, "Params: selfKey\nThis will unregister behavior notifications"),

	PYTHON_METHOD(ptAvatar, playSimpleAnimation, "Params: animName\nPlay simple animation on avatar"),
PYTHON_END_METHODS_TABLE;

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetBehaviorLoopCount, args, "Params: behaviorKey,stage,loopCount,netForce\nThis will set the loop count for a particular stage in a multistage behavior")
{
	PyObject* keyObj = NULL;
	long stage, loopCount;
	char netForce;
	if (!PyArg_ParseTuple(args, "Ollb", &keyObj, &stage, &loopCount, &netForce))
	{
		PyErr_SetString(PyExc_TypeError, "PtSetBehaviorLoopCount expects a ptKey, two longs, and a boolean");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtSetBehaviorLoopCount expects a ptKey, two longs, and a boolean");
		PYTHON_RETURN_ERROR;
	}

	pyKey* key = pyKey::ConvertFrom(keyObj);
	cyAvatar::SetLoopCount(*key, stage, loopCount, netForce != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtChangeAvatar, args, "Params: gender\nChange the local avatar's gender (or clothing type)")
{
	char* gender = NULL;
	if (!PyArg_ParseTuple(args, "s", &gender))
	{
		PyErr_SetString(PyExc_TypeError, "PtChangeAvatar expects a string");
		PYTHON_RETURN_ERROR;
	}

	std::string genderStr = gender; // convert to string (for safety)
	cyAvatar::ChangeAvatar(genderStr.c_str());
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtChangePlayerName, args, "Params: name\nChange the local avatar's name")
{
	char* name = NULL;
	if (!PyArg_ParseTuple(args, "s", &name))
	{
		PyErr_SetString(PyExc_TypeError, "PtChangePlayerName expects a string");
		PYTHON_RETURN_ERROR;
	}

	std::string nameStr = name; // convert to string (for safety)
	cyAvatar::ChangePlayerName(nameStr.c_str());
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtEmoteAvatar, args, "Params: emote\nPlay an emote on the local avatar (netpropagated)")
{
	char* emote = NULL;
	if (!PyArg_ParseTuple(args, "s", &emote))
	{
		PyErr_SetString(PyExc_TypeError, "PtEmoteAvatar expects a string");
		PYTHON_RETURN_ERROR;
	}

	std::string emoteStr = emote; // convert to string (for safety)
	PYTHON_RETURN_BOOL(cyAvatar::Emote(emoteStr.c_str()));
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtAvatarSitOnGround, "Tells the local avatar to sit on ground and enter sit idle loop (netpropagated)")
{
	PYTHON_RETURN_BOOL(cyAvatar::Sit());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtAvatarEnterLookingAtKI, "Tells the local avatar to enter looking at KI idle loop (netpropagated)")
{
	PYTHON_RETURN_BOOL(cyAvatar::EnterKiMode());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtAvatarExitLookingAtKI, "Tells the local avatar to exit looking at KI idle loop (netpropagated)")
{
	PYTHON_RETURN_BOOL(cyAvatar::ExitKiMode());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtAvatarEnterUsePersBook, "Tells the local avatar to enter using their personal book idle loop (netpropagated)")
{
	PYTHON_RETURN_BOOL(cyAvatar::EnterPBMode());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtAvatarExitUsePersBook, "Tells the local avatar to exit using their personal book idle loop (netpropagated)")
{
	PYTHON_RETURN_BOOL(cyAvatar::ExitPBMode());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtAvatarEnterAFK, "Tells the local avatar to enter AwayFromKeyboard idle loop (netpropagated)")
{
	PYTHON_RETURN_BOOL(cyAvatar::EnterAFKMode());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtAvatarExitAFK, "Tells the local avatar to exit AwayFromKeyboard idle loop (netpropagated)")
{
	PYTHON_RETURN_BOOL(cyAvatar::ExitAFKMode());
}

PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtDisableMovementKeys, cyAvatar::DisableMovementControls, "Disable avatar movement input")
PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtEnableMovementKeys, cyAvatar::EnableMovementControls, "Enable avatar movement input")
PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtDisableMouseMovement, cyAvatar::DisableMouseMovement, "Disable avatar mouse movement input")
PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtEnableMouseMovement, cyAvatar::EnableMouseMovement, "Enable avatar mouse movement input")
PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtDisableAvatarJump, cyAvatar::DisableAvatarJump, "Disable the ability of the avatar to jump")
PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtEnableAvatarJump, cyAvatar::EnableAvatarJump, "Enable the ability of the avatar to jump")
PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtDisableForwardMovement, cyAvatar::DisableForwardMovement, "Disable the ability of the avatar to move forward")
PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtEnableForwardMovement, cyAvatar::EnableForwardMovement, "Enable the ability of the avatar to move forward")

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtLocalAvatarRunKeyDown, "Returns true if the run key is being held down for the local avatar")
{
	PYTHON_RETURN_BOOL(cyAvatar::LocalAvatarRunKeyDown());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtLocalAvatarIsMoving, "Returns true if the local avatar is moving (a movement key is held down)")
{
	PYTHON_RETURN_BOOL(cyAvatar::LocalAvatarIsMoving());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetMouseTurnSensitivity, args, "Params: sensitivity\nSet the mouse sensitivity")
{
	float sensitivity;
	if (!PyArg_ParseTuple(args, "f", &sensitivity))
	{
		PyErr_SetString(PyExc_TypeError, "PtSetMouseTurnSensitivity expects a floating point value");
		PYTHON_RETURN_ERROR;
	}

	cyAvatar::SetMouseTurnSensitivity(sensitivity);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetMouseTurnSensitivity, "Returns the sensitivity")
{
	return PyFloat_FromDouble(cyAvatar::GetMouseTurnSensitivity());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtIsCurrentBrainHuman, "Returns whether the local avatar current brain is the human brain")
{
	PYTHON_RETURN_BOOL(cyAvatar::IsCurrentBrainHuman());
}

PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtAvatarSpawnNext, cyAvatar::SpawnNext, "Send the avatar to the next spawn point")

// Type structure definition
PLASMA_DEFAULT_TYPE(ptAvatar, "Plasma avatar class");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptAvatar, cyAvatar)

static PyObject* New(PyObject* sender, PyObject* recvr = nil)
{
	ptAvatar* newObj = (ptAvatar*)ptAvatar_type.tp_new(&ptAvatar_type, NULL, NULL);
	pyKey* senderKey = pyKey::ConvertFrom(sender);
	pyKey* recvrKey = pyKey::ConvertFrom(recvr);
	newObj->fThis->SetSender(senderKey->getKey());
	newObj->fThis->AddRecvr(recvrKey->getKey());
	newObj->fThis->SetNetForce(false);
	return (PyObject*) newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptAvatar, cyAvatar)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptAvatar, cyAvatar)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void cyAvatar::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptAvatar);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaMethods - the python method definitions
//
void cyAvatar::AddPlasmaMethods(std::vector<PyMethodDef> &methods)
{
	// static/global functions (to the local avatar)
	PYTHON_GLOBAL_METHOD(methods, PtSetBehaviorLoopCount);
	PYTHON_GLOBAL_METHOD(methods, PtChangeAvatar);
	PYTHON_GLOBAL_METHOD(methods, PtChangePlayerName);
	PYTHON_GLOBAL_METHOD(methods, PtEmoteAvatar);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtAvatarSitOnGround);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtAvatarEnterLookingAtKI);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtAvatarExitLookingAtKI);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtAvatarEnterUsePersBook);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtAvatarExitUsePersBook);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtAvatarEnterAFK);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtAvatarExitAFK);

	// Suspend avatar input
	PYTHON_BASIC_GLOBAL_METHOD(methods, PtDisableMovementKeys);
	PYTHON_BASIC_GLOBAL_METHOD(methods, PtEnableMovementKeys);
	PYTHON_BASIC_GLOBAL_METHOD(methods, PtDisableMouseMovement);
	PYTHON_BASIC_GLOBAL_METHOD(methods, PtEnableMouseMovement);
	PYTHON_BASIC_GLOBAL_METHOD(methods, PtDisableAvatarJump);
	PYTHON_BASIC_GLOBAL_METHOD(methods, PtEnableAvatarJump);
	PYTHON_BASIC_GLOBAL_METHOD(methods, PtDisableForwardMovement);
	PYTHON_BASIC_GLOBAL_METHOD(methods, PtEnableForwardMovement);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtLocalAvatarRunKeyDown);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtLocalAvatarIsMoving);
	PYTHON_GLOBAL_METHOD(methods, PtSetMouseTurnSensitivity);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetMouseTurnSensitivity);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtIsCurrentBrainHuman);
	PYTHON_BASIC_GLOBAL_METHOD(methods, PtAvatarSpawnNext);
}

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaConstantsClasses - the python constants definitions
//
void cyAvatar::AddPlasmaConstantsClasses(PyObject *m)
{
	PYTHON_ENUM_START(PtBrainModes);
	PYTHON_ENUM_ELEMENT(PtBrainModes, kGeneric,		plAvBrainGeneric::kGeneric);
	PYTHON_ENUM_ELEMENT(PtBrainModes, kLadder,		plAvBrainGeneric::kLadder);
	PYTHON_ENUM_ELEMENT(PtBrainModes, kSit,			plAvBrainGeneric::kSit);
	PYTHON_ENUM_ELEMENT(PtBrainModes, kSitOnGround,	plAvBrainGeneric::kSitOnGround);
	PYTHON_ENUM_ELEMENT(PtBrainModes, kEmote,		plAvBrainGeneric::kEmote);
	PYTHON_ENUM_ELEMENT(PtBrainModes, kAFK,			plAvBrainGeneric::kAFK);
	PYTHON_ENUM_ELEMENT(PtBrainModes, kNonGeneric,	plAvBrainGeneric::kNonGeneric);
	PYTHON_ENUM_END(m, PtBrainModes);

	PYTHON_ENUM_START(PtBehaviorTypes);
	PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeStandingJump,		plHBehavior::kBehaviorTypeStandingJump);
	PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeWalkingJump,		plHBehavior::kBehaviorTypeWalkingJump);
	PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeRunningJump,		plHBehavior::kBehaviorTypeRunningJump);
	PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeAnyJump,			plHBehavior::kBehaviorTypeAnyJump);
	PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeRunningImpact,	plHBehavior::kBehaviorTypeRunningImpact);
	PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeGroundImpact,		plHBehavior::kBehaviorTypeGroundImpact);
	PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeAnyImpact,		plHBehavior::kBehaviorTypeAnyImpact);
	PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeIdle,				plHBehavior::kBehaviorTypeIdle);
	PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeWalk,				plHBehavior::kBehaviorTypeWalk);
	PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeRun,				plHBehavior::kBehaviorTypeRun);
	PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeWalkBack,			plHBehavior::kBehaviorTypeWalkBack);
	PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeTurnLeft,			plHBehavior::kBehaviorTypeTurnLeft);
	PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeTurnRight,		plHBehavior::kBehaviorTypeTurnRight);
	PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeSidestepLeft,		plHBehavior::kBehaviorTypeSidestepLeft);
	PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeSidestepRight,	plHBehavior::kBehaviorTypeSidestepRight);
	PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeFall,				plHBehavior::kBehaviorTypeFall);
	PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeMovingTurnLeft,	plHBehavior::kBehaviorTypeMovingTurnLeft);
	PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeMovingTurnRight,	plHBehavior::kBehaviorTypeMovingTurnRight);
	PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeLinkIn,			plHBehavior::kBehaviorTypeLinkIn);
	PYTHON_ENUM_ELEMENT(PtBehaviorTypes, kBehaviorTypeLinkOut,			plHBehavior::kBehaviorTypeLinkOut);
	PYTHON_ENUM_END(m, PtBehaviorTypes);
}