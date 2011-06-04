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
#include "pySceneObject.h"
#include "pyMatrix44.h"
#include "pyGeometry3.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptSceneobject, pySceneObject);

PYTHON_DEFAULT_NEW_DEFINITION(ptSceneobject, pySceneObject)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptSceneobject)

PYTHON_INIT_DEFINITION(ptSceneobject, args, keywords)
{
	PyObject *objKeyObject = NULL;
	PyObject *selfKeyObject = NULL;
	if (!PyArg_ParseTuple(args, "OO", &objKeyObject, &selfKeyObject))
	{
		PyErr_SetString(PyExc_TypeError, "init expects two ptKey objects");
		PYTHON_RETURN_INIT_ERROR;
	}
	if ((!pyKey::Check(objKeyObject))||(!pyKey::Check(selfKeyObject)))
	{
		PyErr_SetString(PyExc_TypeError, "init expects two ptKey objects");
		PYTHON_RETURN_INIT_ERROR;
	}

	pyKey *objKey = pyKey::ConvertFrom(objKeyObject);
	pyKey *selfKey = pyKey::ConvertFrom(selfKeyObject);
	self->fThis->addObjKey(objKey->getKey());
	self->fThis->setSenderKey(selfKey->getKey());
	self->fThis->setPyMod(*selfKey);

	PYTHON_RETURN_INIT_OK;
}

PYTHON_RICH_COMPARE_DEFINITION(ptSceneobject, obj1, obj2, compareType)
{
	if ((obj1 == Py_None) || (obj2 == Py_None) || !pySceneObject::Check(obj1) || !pySceneObject::Check(obj2))
	{
		// if they aren't the same type, they don't match, obviously (we also never equal none)
		if (compareType == Py_EQ)
			PYTHON_RCOMPARE_FALSE;
		else if (compareType == Py_NE)
			PYTHON_RCOMPARE_TRUE;
		else
		{
			PyErr_SetString(PyExc_NotImplementedError, "invalid comparison for a ptSceneobject object");
			PYTHON_RCOMPARE_ERROR;
		}
	}
	pySceneObject *scObj1 = pySceneObject::ConvertFrom(obj1);
	pySceneObject *scObj2 = pySceneObject::ConvertFrom(obj2);
	if (compareType == Py_EQ)
	{
		if ((*scObj1) == (*scObj2))
			PYTHON_RCOMPARE_TRUE;
		PYTHON_RCOMPARE_FALSE;
	}
	else if (compareType == Py_NE)
	{
		if ((*scObj1) != (*scObj2))
			PYTHON_RCOMPARE_TRUE;
		PYTHON_RCOMPARE_FALSE;
	}
	PyErr_SetString(PyExc_NotImplementedError, "invalid comparison for a ptSceneobject object");
	PYTHON_RCOMPARE_ERROR;
}

PYTHON_METHOD_DEFINITION(ptSceneobject, addKey, args)
{
	PyObject *keyObject = NULL;
	if (!PyArg_ParseTuple(args, "O", &keyObject))
	{
		PyErr_SetString(PyExc_TypeError, "addKey expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObject))
	{
		PyErr_SetString(PyExc_TypeError, "addKey expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	pyKey *key = pyKey::ConvertFrom(keyObject);
	self->fThis->addObjPyKey(*key);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSceneobject, getKey)
{
	return self->fThis->getObjPyKey();
}

PYTHON_METHOD_DEFINITION(ptSceneobject, netForce, args)
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

PYTHON_METHOD_DEFINITION(ptSceneobject, findObject, args)
{
	char *name = NULL;
	if (!PyArg_ParseTuple(args, "s", &name))
	{
		PyErr_SetString(PyExc_TypeError, "findObject expects a string");
		PYTHON_RETURN_ERROR;
	}
	return self->fThis->findObj(name);
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSceneobject, getName)
{
	return PyString_FromString(self->fThis->GetName());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSceneobject, getResponders)
{
	std::vector<PyObject*> vecList = self->fThis->GetResponders();
	PyObject *retVal = PyList_New(vecList.size());
	for (int curKey = 0; curKey < vecList.size(); curKey++)
		PyList_SetItem(retVal, curKey, vecList[curKey]); // steals the vecList ref
	return retVal;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSceneobject, getPythonMods)
{
	std::vector<PyObject*> vecList = self->fThis->GetPythonMods();
	PyObject *retVal = PyList_New(vecList.size());
	for (int curKey = 0; curKey < vecList.size(); curKey++)
		PyList_SetItem(retVal, curKey, vecList[curKey]); // steals the vecList ref
	return retVal;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSceneobject, isLocallyOwned)
{
	PYTHON_RETURN_BOOL(self->fThis->IsLocallyOwned());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSceneobject, getLocalToWorld)
{
	return self->fThis->GetLocalToWorld();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSceneobject, getWorldToLocal)
{
	return self->fThis->GetWorldToLocal();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSceneobject, getLocalToParent)
{
	return self->fThis->GetLocalToParent();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSceneobject, getParentToLocal)
{
	return self->fThis->GetParentToLocal();
}

PYTHON_METHOD_DEFINITION(ptSceneobject, setTransform, args)
{
	PyObject *local2WorldObj = NULL;
	PyObject *world2LocalObj = NULL;
	if (!PyArg_ParseTuple(args, "OO", &local2WorldObj, &world2LocalObj))
	{
		PyErr_SetString(PyExc_TypeError, "setTransform expects two ptMatrix44 objects");
		PYTHON_RETURN_ERROR;
	}
	if ((!pyMatrix44::Check(local2WorldObj))||(!pyMatrix44::Check(world2LocalObj)))
	{
		PyErr_SetString(PyExc_TypeError, "setTransform expects two ptMatrix44 objects");
		PYTHON_RETURN_ERROR;
	}
	pyMatrix44 *local2World = pyMatrix44::ConvertFrom(local2WorldObj);
	pyMatrix44 *world2Local = pyMatrix44::ConvertFrom(world2LocalObj);
	self->fThis->SetTransform(*local2World, *world2Local);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSceneobject, position)
{
	return self->fThis->GetWorldPosition();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSceneobject, view)
{
	return self->fThis->GetViewVector();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSceneobject, up)
{
	return self->fThis->GetUpVector();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSceneobject, right)
{
	return self->fThis->GetRightVector();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSceneobject, isAvatar)
{
	PYTHON_RETURN_BOOL(self->fThis->IsAvatar());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSceneobject, avatarVelocity)
{
	return self->fThis->GetAvatarVelocity();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSceneobject, isHuman)
{
	PYTHON_RETURN_BOOL(self->fThis->IsHumanAvatar());
}

PYTHON_METHOD_DEFINITION(ptSceneobject, pushCutsceneCamera, args)
{
	char cutFlag;
	PyObject *avKeyObject = NULL;
	if (!PyArg_ParseTuple(args, "bO", &cutFlag, &avKeyObject))
	{
		PyErr_SetString(PyExc_TypeError, "pushCutseneCamera expects a boolean and a ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(avKeyObject))
	{
		PyErr_SetString(PyExc_TypeError, "pushCutseneCamera expects a boolean and a ptKey");
		PYTHON_RETURN_ERROR;
	}
	pyKey *avKey = pyKey::ConvertFrom(avKeyObject);
	self->fThis->PushCutsceneCamera((cutFlag != 0), *avKey);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptSceneobject, popCutsceneCamera, args)
{
	PyObject *avKeyObject = NULL;
	if (!PyArg_ParseTuple(args, "O", &avKeyObject))
	{
		PyErr_SetString(PyExc_TypeError, "popCutsceneCamera expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(avKeyObject))
	{
		PyErr_SetString(PyExc_TypeError, "popCutsceneCamera expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	pyKey *avKey = pyKey::ConvertFrom(avKeyObject);
	self->fThis->PopCutsceneCamera(*avKey);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptSceneobject, pushCamera, args)
{
	PyObject *avKeyObject = NULL;
	if (!PyArg_ParseTuple(args, "O", &avKeyObject))
	{
		PyErr_SetString(PyExc_TypeError, "pushCamera expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(avKeyObject))
	{
		PyErr_SetString(PyExc_TypeError, "pushCamera expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	pyKey *avKey = pyKey::ConvertFrom(avKeyObject);
	self->fThis->PushCamera(*avKey);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptSceneobject, pushCameraCut, args)
{
	PyObject *avKeyObject = NULL;
	if (!PyArg_ParseTuple(args, "O", &avKeyObject))
	{
		PyErr_SetString(PyExc_TypeError, "pushCameraCut expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(avKeyObject))
	{
		PyErr_SetString(PyExc_TypeError, "pushCameraCut expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	pyKey *avKey = pyKey::ConvertFrom(avKeyObject);
	self->fThis->PushCameraCut(*avKey);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptSceneobject, popCamera, args)
{
	PyObject *avKeyObject = NULL;
	if (!PyArg_ParseTuple(args, "O", &avKeyObject))
	{
		PyErr_SetString(PyExc_TypeError, "popCamera expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(avKeyObject))
	{
		PyErr_SetString(PyExc_TypeError, "popCamera expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	pyKey *avKey = pyKey::ConvertFrom(avKeyObject);
	self->fThis->PopCamera(*avKey);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSceneobject, getResponderState)
{
	return PyInt_FromLong((long)self->fThis->GetResponderState());
}

PYTHON_BASIC_METHOD_DEFINITION(ptSceneobject, animate, Animate)

PYTHON_METHOD_DEFINITION(ptSceneobject, rewindAnimNamed, args)
{
	char *name = NULL;
	if (!PyArg_ParseTuple(args, "s", &name))
	{
		PyErr_SetString(PyExc_TypeError, "rewindAnimNamed expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->RewindAnim(name);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptSceneobject, playAnimNamed, args)
{
	char *name = NULL;
	if (!PyArg_ParseTuple(args, "s", &name))
	{
		PyErr_SetString(PyExc_TypeError, "playAnimNamed expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->PlayAnim(name);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptSceneobject, stopAnimNamed, args)
{
	char *name = NULL;
	if (!PyArg_ParseTuple(args, "s", &name))
	{
		PyErr_SetString(PyExc_TypeError, "stopAnimNamed expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->StopAnim(name);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptSceneobject, runAttachedResponder, args)
{
	int state;
	if (!PyArg_ParseTuple(args, "i", &state))
	{
		PyErr_SetString(PyExc_TypeError, "runAttachedResponder expects an integer");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->RunResponder(state);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptSceneobject, fastForwardAttachedResponder, args)
{
	int state;
	if (!PyArg_ParseTuple(args, "i", &state))
	{
		PyErr_SetString(PyExc_TypeError, "fastForwardAttachedResponder expects an integer");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->FFResponder(state);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptSceneobject, setSoundFilename, args)
{
	int index;
	char *filename = NULL;
	char isCompressed;
	if (!PyArg_ParseTuple(args, "isb", &index, &filename, &isCompressed))
	{
		PyErr_SetString(PyExc_TypeError, "setSoundFilename expects an integer, string and boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetSoundFilename(index, filename, (isCompressed != 0));
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptSceneobject, getSoundIndex, args)
{
	char *sndComponentName = NULL;
	if (!PyArg_ParseTuple(args, "s", &sndComponentName))
	{
		PyErr_SetString(PyExc_TypeError, "getSoundIndex expects a string");
		PYTHON_RETURN_ERROR;
	}
	return PyInt_FromLong((long)self->fThis->GetSoundObjectIndex(sndComponentName));
}

PYTHON_METHOD_DEFINITION(ptSceneobject, volumeSensorIgnoreExtraEnters, args)
{
	char ignore;
	if (!PyArg_ParseTuple(args, "b", &ignore))
	{
		PyErr_SetString(PyExc_TypeError, "volumeSensorIgnoreExtraEnters expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->VolumeSensorIgnoreExtraEnters(ignore != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptSceneobject)
	PYTHON_METHOD(ptSceneobject, addKey, "Params: key\nMostly used internally.\n"
				"Add another sceneobject ptKey"),
	PYTHON_METHOD_NOARGS(ptSceneobject, getKey, "Get the ptKey of this sceneobject\n"
				"If there are more then one attached, get the first one"),

	PYTHON_METHOD(ptSceneobject, netForce, "Params: forceFlag\nSpecify whether this object needs to use messages that are forced to the network\n"
				"- This is to be used if your Python program is running on only one client\n"
				"Such as a game master, only running on the client that owns a particular object\n"
				"- Setting the netForce flag on a sceneobject will also set the netForce flag on\n"
				"its draw, physics, avatar, particle objects"),
	
	PYTHON_METHOD(ptSceneobject, findObject, "Params: name\nFind a particular object in just the sceneobjects that are attached"),
	PYTHON_METHOD_NOARGS(ptSceneobject, getName, "Returns the name of the sceneobject (Max name)\n"
				"- If there are more than one sceneobject attached, return just the first one"),
	PYTHON_METHOD_NOARGS(ptSceneobject, getResponders, "Returns list of ptKeys of the responders attached to this sceneobject"),
	PYTHON_METHOD_NOARGS(ptSceneobject, getPythonMods, "Returns list of ptKeys of the python modifiers attached to this sceneobject"),
	PYTHON_METHOD_NOARGS(ptSceneobject, isLocallyOwned, "Returns true(1) if this object is locally owned by this client\n"
				"or returns false(0) if it is not or don't know"),

	PYTHON_METHOD_NOARGS(ptSceneobject, getLocalToWorld, "Returns ptMatrix44 of the local to world transform for this sceneobject\n"
				"- If there is more than one sceneobject attached, returns just the first one"),
	PYTHON_METHOD_NOARGS(ptSceneobject, getWorldToLocal, "Returns ptMatrix44 of the world to local transform for this sceneobject\n"
				"- If there is more than one sceneobject attached, returns just the first one"),
	PYTHON_METHOD_NOARGS(ptSceneobject, getLocalToParent, "Returns ptMatrix44 of the local to parent transform for this sceneobject\n"
				"- If there is more than one sceneobject attached, returns just the first one"),
	PYTHON_METHOD_NOARGS(ptSceneobject, getParentToLocal, "Returns ptMatrix44 of the parent to local transform for this sceneobject\n"
				"- If there is more than one sceneobject attached, returns just the first one"),
	PYTHON_METHOD(ptSceneobject, setTransform, "Params: local2world,world2local\nSet our current transforms"),

	PYTHON_METHOD_NOARGS(ptSceneobject, position, "Returns the scene object's current position"),
	PYTHON_METHOD_NOARGS(ptSceneobject, view, "Returns the scene object's current view vector"),
	PYTHON_METHOD_NOARGS(ptSceneobject, up, "Returns the scene object's current up vector"),
	PYTHON_METHOD_NOARGS(ptSceneobject, right, "Returns the scene object's current right vector"),
	PYTHON_METHOD_NOARGS(ptSceneobject, isAvatar, "Returns true if the scene object is an avatar"),
	PYTHON_METHOD_NOARGS(ptSceneobject, avatarVelocity, "Returns the velocity of the first attached avatar scene object"),
	PYTHON_METHOD_NOARGS(ptSceneobject, isHuman, "Returns true if the scene object is a human avatar"),

	PYTHON_METHOD(ptSceneobject, pushCutsceneCamera, "Params: cutFlag,avKey\nSwitch to this object (assuming that it is actually a camera)"),
	PYTHON_METHOD(ptSceneobject, popCutsceneCamera, "Params: avKey\nPop the camera stack and go back to previous camera."),
	PYTHON_METHOD(ptSceneobject, pushCamera, "Params: avKey\nSwitch to this object (if it is a camera)"),
	PYTHON_METHOD(ptSceneobject, pushCameraCut, "Params: avKey\nSwitch to this object, cutting the view (if it is a camera)"),
	PYTHON_METHOD(ptSceneobject, popCamera, "Params: avKey\nPop the camera stack and go back to the previous camera"),

	PYTHON_METHOD_NOARGS(ptSceneobject, getResponderState, "Return the responder state (if we are a responder)"),
	
	PYTHON_BASIC_METHOD(ptSceneobject, animate, "If we can animate, start animating"),
	PYTHON_METHOD(ptSceneobject, rewindAnimNamed, "Params: animName\nRewind the attached named animation"),
	PYTHON_METHOD(ptSceneobject, playAnimNamed, "Params: animName\nPlay the attached named animation"),
	PYTHON_METHOD(ptSceneobject, stopAnimNamed, "Params: animName\nStop the attached named animation"),

	PYTHON_METHOD(ptSceneobject, runAttachedResponder, "Params: state\nRun the attached responder to the specified state"),
	PYTHON_METHOD(ptSceneobject, fastForwardAttachedResponder, "Params: state\nFast forward the attached responder to the specified state"),

	PYTHON_METHOD(ptSceneobject, setSoundFilename, "Params: index, filename, isCompressed\nSets the sound attached to this sceneobject to use the specified sound file."),
	PYTHON_METHOD(ptSceneobject, getSoundIndex, "Params: sndComponentName\nGet the index of the requested sound component"),

	PYTHON_METHOD(ptSceneobject, volumeSensorIgnoreExtraEnters, "Params: ignore\nTells the volume sensor attached to this object to ignore extra enters (default), or not (hack for garrison)."),
PYTHON_END_METHODS_TABLE;

PYTHON_GET_DEFINITION(ptSceneobject, draw)
{
	Py_INCREF(self->fThis->fDraw); // we need to return a new ref
	return self->fThis->fDraw;
}

PYTHON_SET_DEFINITION_READONLY(ptSceneobject, draw)

PYTHON_GET_DEFINITION(ptSceneobject, physics)
{
	Py_INCREF(self->fThis->fPhysics); // we need to return a new ref
	return self->fThis->fPhysics;
}

PYTHON_SET_DEFINITION_READONLY(ptSceneobject, physics)

PYTHON_GET_DEFINITION(ptSceneobject, avatar)
{
	Py_INCREF(self->fThis->fAvatar); // we need to return a new ref
	return self->fThis->fAvatar;
}

PYTHON_SET_DEFINITION_READONLY(ptSceneobject, avatar)

PYTHON_GET_DEFINITION(ptSceneobject, particle)
{
	Py_INCREF(self->fThis->fParticle); // we need to return a new ref
	return self->fThis->fParticle;
}

PYTHON_SET_DEFINITION_READONLY(ptSceneobject, particle)

PYTHON_START_GETSET_TABLE(ptSceneobject)
	PYTHON_GETSET(ptSceneobject, draw, ""),
	PYTHON_GETSET(ptSceneobject, physics, ""),
	PYTHON_GETSET(ptSceneobject, avatar, ""),
	PYTHON_GETSET(ptSceneobject, particle, ""),
PYTHON_END_GETSET_TABLE;

// Type structure definition
#define ptSceneobject_COMPARE			PYTHON_NO_COMPARE
#define ptSceneobject_AS_NUMBER			PYTHON_NO_AS_NUMBER
#define ptSceneobject_AS_SEQUENCE		PYTHON_NO_AS_SEQUENCE
#define ptSceneobject_AS_MAPPING		PYTHON_NO_AS_MAPPING
#define ptSceneobject_STR				PYTHON_NO_STR
#define ptSceneobject_RICH_COMPARE		PYTHON_DEFAULT_RICH_COMPARE(ptSceneobject)
#define ptSceneobject_GETSET			PYTHON_DEFAULT_GETSET(ptSceneobject)
#define ptSceneobject_BASE				PYTHON_NO_BASE
PLASMA_CUSTOM_TYPE(ptSceneobject, "Params: objKey, selfKey\nPlasma Sceneobject class");

// required functions for PyObject interoperability
PyObject *pySceneObject::New(plKey objKey, PyObject *selfKeyObj)
{
	if (!pyKey::Check(selfKeyObj))
		return NULL;
	pyKey *selfKey = pyKey::ConvertFrom(selfKeyObj);

	ptSceneobject *newObj = (ptSceneobject*)ptSceneobject_type.tp_new(&ptSceneobject_type, NULL, NULL);
	newObj->fThis->addObjKey(objKey);
	newObj->fThis->setSenderKey(selfKey->getKey());
	newObj->fThis->setPyMod(*selfKey);
	newObj->fThis->SetNetForce(false);
	return (PyObject*)newObj;
}

PyObject *pySceneObject::New(plKey objKey, pyKey &selfKey)
{
	ptSceneobject *newObj = (ptSceneobject*)ptSceneobject_type.tp_new(&ptSceneobject_type, NULL, NULL);
	newObj->fThis->addObjKey(objKey);
	newObj->fThis->setSenderKey(selfKey.getKey());
	newObj->fThis->setPyMod(selfKey);
	newObj->fThis->SetNetForce(false);
	return (PyObject*)newObj;
}

PyObject *pySceneObject::New(plKey objKey, plKey selfKey)
{
	ptSceneobject *newObj = (ptSceneobject*)ptSceneobject_type.tp_new(&ptSceneobject_type, NULL, NULL);
	newObj->fThis->addObjKey(objKey);
	newObj->fThis->setSenderKey(selfKey);
	newObj->fThis->setPyMod(selfKey);
	newObj->fThis->SetNetForce(false);
	return (PyObject*)newObj;
}

PyObject *pySceneObject::New(plKey objKey)
{
	ptSceneobject *newObj = (ptSceneobject*)ptSceneobject_type.tp_new(&ptSceneobject_type, NULL, NULL);
	newObj->fThis->addObjKey(objKey);
	newObj->fThis->setSenderKey(objKey);
	newObj->fThis->SetNetForce(false);
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptSceneobject, pySceneObject)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptSceneobject, pySceneObject)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pySceneObject::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptSceneobject);
	PYTHON_CLASS_IMPORT_END(m);
}