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
#include "cyMisc.h"
#include "pyGlueHelpers.h"
#include "pyKey.h"
#include "pySceneObject.h"
#include "pyAgeInfoStruct.h"
#include "pyGeometry3.h"
#include "../NucleusLib/inc/plPipeline.h"
#include "../pnNetBase/pnNetBase.h"

#include <python.h>

PYTHON_GLOBAL_METHOD_DEFINITION(PtRequestLOSScreen, args, "Params: selfKey,ID,xPos,yPos,distance,what,reportType\nRequest a LOS check from a point on the screen")
{
	PyObject* keyObj = NULL;
	long id;
	float xPos, yPos, distance;
	int what, reportType;
	if (!PyArg_ParseTuple(args, "Olfffii", &keyObj, &id, &xPos, &yPos, &distance, &what, &reportType))
	{
		PyErr_SetString(PyExc_TypeError, "PtRequestLOSScreen expects a ptKey, a long, three floats, and two ints");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtRequestLOSScreen expects a ptKey, a long, three floats, and two ints");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	PYTHON_RETURN_BOOL(cyMisc::RequestLOSScreen(*key, id, xPos, yPos, distance, what, reportType));
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtKillParticles, args, "Params: timeRemaining,pctToKill,particleSystem\nTells particleSystem to kill pctToKill percent of its particles")
{
	float timeRemaining, pctToKill;
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "ffO", &timeRemaining, &pctToKill, &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtKillParticles expects two floats and a ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtKillParticles expects two floats and a ptKey");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	cyMisc::KillParticles(timeRemaining, pctToKill, *key);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtGetNumParticles, args, "Params: key\nKey is the key of scene object host to particle system")
{
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtGetNumParticles expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtGetNumParticles expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	return PyInt_FromLong(cyMisc::GetNumParticles(*key));
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetParticleOffset, args, "Params: x,y,z,particlesys\nSets the particlesys particle system's offset")
{
	float x,y,z;
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "fffO", &x, &y, &z, &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtSetParticleOffset expects three floats and a ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtSetParticleOffset expects three floats and a ptKey");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	cyMisc::SetParticleOffset(x, y, z, *key);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetLightValue, args, "Params: key,name,r,g,b,a\n Key is the key of scene object host to light. Name is the name of the light to manipulate")
{
	PyObject* keyObj = NULL;
	PyObject* nameObj = NULL;
	float r,g,b,a;
	if (!PyArg_ParseTuple(args, "OOffff", &keyObj, &nameObj, &r, &g, &b, &a))
	{
		PyErr_SetString(PyExc_TypeError, "PtSetLightValue expects a ptKey, a string, and four floats");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtSetLightValue expects a ptKey, a string, and four floats");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	std::string name = "";
	if (PyUnicode_Check(nameObj))
	{
		int strLen = PyUnicode_GetSize(nameObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)nameObj, text, strLen);
		text[strLen] = L'\0';
		char* cText = hsWStringToString(text);
		name = cText;
		delete [] cText;
		delete [] text;
	}
	else if (PyString_Check(nameObj))
	{
		// we'll allow this, just in case something goes weird
		char* text = PyString_AsString(nameObj);
		name = text;
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "PtSetLightValue expects a ptKey, a string, and four floats");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::SetLightColorValue(*key, name, r, g, b, a);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetLightAnimStart, args, "Params: key,name,start\n Key is the key of scene object host to light, start is a bool. Name is the name of the light to manipulate")
{
	PyObject* keyObj = NULL;
	PyObject* nameObj = NULL;
	char start;
	if (!PyArg_ParseTuple(args, "OOb", &keyObj, &nameObj, &start))
	{
		PyErr_SetString(PyExc_TypeError, "PtSetLightAnimStart expects a ptKey, a string, and a boolean");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtSetLightAnimStart expects a ptKey, a string, and a boolean");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	std::string name = "";
	if (PyUnicode_Check(nameObj))
	{
		int strLen = PyUnicode_GetSize(nameObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)nameObj, text, strLen);
		text[strLen] = L'\0';
		char* cText = hsWStringToString(text);
		name = cText;
		delete [] cText;
		delete [] text;
	}
	else if (PyString_Check(nameObj))
	{
		// we'll allow this, just in case something goes weird
		char* text = PyString_AsString(nameObj);
		name = text;
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "PtSetLightAnimStart expects a ptKey, a string, and a boolean");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::SetLightAnimationOn(*key, name, start != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtIsSinglePlayerMode, "Returns whether the game is in single player mode or not")
{
	PYTHON_RETURN_BOOL(cyMisc::IsSinglePlayerMode());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtIsDemoMode, "Returns whether the game is in Demo mode or not")
{
	PYTHON_RETURN_BOOL(cyMisc::IsDemoMode());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtIsInternalRelease, "Returns whether the client is an internal build or not")
{
	PYTHON_RETURN_BOOL(cyMisc::IsInternalRelease());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtIsEnterChatModeKeyBound, "Returns whether the EnterChatMode is bound to a key")
{
	PYTHON_RETURN_BOOL(cyMisc::IsEnterChatModeKeyBound());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtShootBulletFromScreen, args, "Params: selfkey, xPos, yPos, radius, range\nShoots a bullet from a position on the screen")
{
	PyObject* keyObj = NULL;
	float xPos, yPos, radius, range;
	if (!PyArg_ParseTuple(args, "Offff", &keyObj, &xPos, &yPos, &radius, &range))
	{
		PyErr_SetString(PyExc_TypeError, "PtShootBulletFromScreen expects a ptKey and four floats");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtShootBulletFromScreen expects a ptKey and four floats");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	cyMisc::ShootBulletFromScreen(*key, xPos, yPos, radius, range);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtShootBulletFromObject, args, "Params: selfkey, gunObj, radius, range\nShoots a bullet from an object")
{
	PyObject* selfKeyObj = NULL;
	PyObject* gunSceneObj = NULL;
	float radius, range;
	if (!PyArg_ParseTuple(args, "OOff", &selfKeyObj, &gunSceneObj, &radius, &range))
	{
		PyErr_SetString(PyExc_TypeError, "PtShootBulletFromObject expects a ptKey, a ptSceneobject, and two floats");
		PYTHON_RETURN_ERROR;
	}
	if ((!pyKey::Check(selfKeyObj)) || (!pySceneObject::Check(gunSceneObj)))
	{
		PyErr_SetString(PyExc_TypeError, "PtShootBulletFromObject expects a ptKey, a ptSceneobject, and two floats");
		PYTHON_RETURN_ERROR;
	}
	pyKey* selfKey = pyKey::ConvertFrom(selfKeyObj);
	pySceneObject* gunObj = pySceneObject::ConvertFrom(gunSceneObj);
	cyMisc::ShootBulletFromObject(*selfKey, gunObj, radius, range);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtGetPublicAgeList, args, "Params: ageName, cbObject=None\nGet list of public ages for the given age name.\n"
			"cbObject, if supplied should have a method called gotPublicAgeList(self,ageList). ageList is a list of tuple(ptAgeInfoStruct,nPlayersInAge)")
{
	char* ageName;
	PyObject* cbObject = NULL;
	if (!PyArg_ParseTuple(args, "s|O", &ageName, &cbObject))
	{
		PyErr_SetString(PyExc_TypeError, "PtGetPublicAgeList expects a string and an optional object with a gotPublicAgeList() method");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::GetPublicAgeList(ageName, cbObject);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtCreatePublicAge, args, "Params: ageInfo, cbObject=None\nCreate a public instance of the given age.\n"
			"cbObject, if supplied should have a member called publicAgeCreated(self,ageInfo)")
{
	PyObject* ageInfoObj = NULL;
	PyObject* cbObject = NULL;
	if (!PyArg_ParseTuple(args, "O|O", &ageInfoObj, &cbObject))
	{
		PyErr_SetString(PyExc_TypeError, "PtCreatePublicAge expects a ptAgeInfoStruct object and an optional object with a publicAgeCreated() method");
		PYTHON_RETURN_ERROR;
	}
	if (!pyAgeInfoStruct::Check(ageInfoObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtCreatePublicAge expects a ptAgeInfoStruct object and an optional object with a publicAgeCreated() method");
		PYTHON_RETURN_ERROR;
	}
	pyAgeInfoStruct* ageInfo = pyAgeInfoStruct::ConvertFrom(ageInfoObj);
	cyMisc::CreatePublicAge(ageInfo, cbObject);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtRemovePublicAge, args, "Params: ageInstanceGuid, cbObject=None\nRemove a public instance of the given age.\n"
			"cbObject, if supplied should have a member called publicAgeRemoved(self,ageInstanceGuid)")
{
	char* ageInstanceGUID;
	PyObject* cbObject = NULL;
	if (!PyArg_ParseTuple(args, "s|O", &ageInstanceGUID, &cbObject))
	{
		PyErr_SetString(PyExc_TypeError, "PtRemovePublicAge expects a string and an optional object with a publicAgeRemoved() method");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::RemovePublicAge(ageInstanceGUID, cbObject);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetClearColor, args, "Params: red,green,blue\nSet the clear color")
{
	float red, green, blue;
	if (!PyArg_ParseTuple(args, "fff", &red, &green, &blue))
	{
		PyErr_SetString(PyExc_TypeError, "PtSetClearColor expects three floats");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::SetClearColor(red, green, blue);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetLocalKILevel, "returns local player's ki level")
{
	return PyInt_FromLong(cyMisc::GetKILevel());
}

PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtClearCameraStack, cyMisc::ClearCameraStack, "clears all cameras")

PYTHON_GLOBAL_METHOD_DEFINITION(PtGetCameraNumber, args, "Params: x\nReturns camera x's name from stack")
{
	int x;
	if (!PyArg_ParseTuple(args, "i", &x))
	{
		PyErr_SetString(PyExc_TypeError, "PtGetCameraNumber expects an int");
		PYTHON_RETURN_ERROR;
	}
	return PyString_FromString(cyMisc::GetCameraNumber(x));
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetNumCameras, "returns camera stack size")
{
	return PyInt_FromLong(cyMisc::GetNumCameras());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtRebuildCameraStack, args, "Params: name,ageName\nPush camera with this name on the stack")
{
	char* name;
	char* ageName;
	if (!PyArg_ParseTuple(args, "ss", &name, &ageName))
	{
		PyErr_SetString(PyExc_TypeError, "PtRebuildCameraStack expects two strings");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::RebuildCameraStack(name, ageName);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtRecenterCamera, cyMisc::RecenterCamera, "re-centers the camera")

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtFirstPerson, "is the local avatar in first person mode")
{
	PYTHON_RETURN_BOOL(cyMisc::IsFirstPerson());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtFadeIn, args, "Params: lenTime, holdFlag, noSound=0\nFades screen in for lenTime seconds")
{
	float lenTime;
	char holdFlag, noSound = 0;
	if (!PyArg_ParseTuple(args, "fb|b", &lenTime, &holdFlag, &noSound))
	{
		PyErr_SetString(PyExc_TypeError, "PtFadeIn expects a float, a boolean, and an optional boolean");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::FadeIn(lenTime, holdFlag != 0, noSound != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtFadeOut, args, "Params: lenTime, holdFlag, noSound=0\nFades screen out for lenTime seconds")
{
	float lenTime;
	char holdFlag, noSound = 0;
	if (!PyArg_ParseTuple(args, "fb|b", &lenTime, &holdFlag, &noSound))
	{
		PyErr_SetString(PyExc_TypeError, "PtFadeOut expects a float, a boolean, and an optional boolean");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::FadeOut(lenTime, holdFlag != 0, noSound != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetGlobalClickability, args, "Params: enable\nEnable or disable all clickables on the local client")
{
	char enable;
	if (!PyArg_ParseTuple(args, "b", &enable))
	{
		PyErr_SetString(PyExc_TypeError, "PtSetGlobalClickability expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::SetClickability(enable != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtDebugAssert, args, "Params: cond, msg\nDebug only: Assert if condition is false.")
{
	char cond;
	char* msg;
	if (!PyArg_ParseTuple(args, "bs", &cond, &msg))
	{
		PyErr_SetString(PyExc_TypeError, "PtDebugAssert expects a boolean and a string");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::DebugAssert(cond != 0, msg);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetAlarm, args, "Params: secs, cbObject, cbContext\nsecs is the amount of time before your alarm goes off.\n"
			"cbObject is a python object with the method onAlarm(int context)\ncbContext is an integer.")
{
	float secs;
	PyObject* cbObject = NULL;
	unsigned long cbContext;
	if (!PyArg_ParseTuple(args, "fOl", &secs, &cbObject, &cbContext))
	{
		PyErr_SetString(PyExc_TypeError, "PtSetAlarm expects a float, a object with a onAlarm() method, and an int");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::SetAlarm(secs, cbObject, cbContext);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSaveScreenShot, args, "Params: fileName,width=640,height=480,quality=75\nTakes a screenshot with the specified filename, size, and quality")
{
	char* fileName;
	int width = 640, height = 480, quality = 75;
	if (!PyArg_ParseTuple(args, "s|iii", &fileName, &width, &height, &quality))
	{
		PyErr_SetString(PyExc_TypeError, "PtSaveScreenShot expects a string, and three optional integers");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::SaveScreenShot(fileName, width, height, quality);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtStartScreenCapture, args, "Params: selfKey,width=800,height=600\nStarts a capture of the screen")
{
	PyObject* keyObj = NULL;
	unsigned short width = 800, height = 600;
	if (!PyArg_ParseTuple(args, "O|hh", &keyObj, &width, &height))
	{
		PyErr_SetString(PyExc_TypeError, "PtStartScreenCapture expects a ptKey, and two optional unsigned 16-bit ints");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtStartScreenCapture expects a ptKey, and two optional unsigned 16-bit ints");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	cyMisc::StartScreenCaptureWH(*key, width, height);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSendKIGZMarkerMsg, args, "Params: markerNumber,sender\nSame as PtSendKIMessageInt except 'sender' could get a notify message back\n")
{
	long markerNumber;
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "lO", &markerNumber, &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtSendKIGZMarkerMsg expects a long and a ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtSendKIGZMarkerMsg expects a long and a ptKey");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	cyMisc::SendKIGZMarkerMsg(markerNumber, *key);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSendKIRegisterImagerMsg, args, "Params: imagerName, sender\nSends a message to the KI to register the specified imager")
{
	char* name;
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "sO", &name, &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtSendKIRegisterImagerMsg expects a string and a ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtSendKIRegisterImagerMsg expects a string and a ptKey");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	cyMisc::SendKIRegisterImagerMsg(name, *key);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtWearMaintainerSuit, args, "Params: key,wearOrNot\nWears or removes the maintainer suit of clothes")
{
	PyObject* keyObj = NULL;
	char wearOrNot;
	if (!PyArg_ParseTuple(args, "Ob", &keyObj, &wearOrNot))
	{
		PyErr_SetString(PyExc_TypeError, "PtWearMaintainerSuit expects a ptKey and a boolean");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtWearMaintainerSuit expects a ptKey and a boolean");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	cyMisc::WearMaintainerSuit(*key, wearOrNot != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtWearDefaultClothing, args, "Params: key\nForces the avatar to wear the default clothing set")
{
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtWearDefaultClothing expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtWearDefaultClothing expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	cyMisc::WearDefaultClothing(*key);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetAgeTimeOfDayPercent, "Returns the current age time of day as a percent (0 to 1)")
{
	return PyFloat_FromDouble(cyMisc::GetAgeTimeOfDayPercent());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtCheckVisLOS, args, "Params: startPoint,endPoint\nDoes LOS check from start to end")
{
	PyObject* startPointObj = NULL;
	PyObject* endPointObj = NULL;
	if (!PyArg_ParseTuple(args, "OO", &startPointObj, &endPointObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtCheckVisLOS expects two ptPoint3 objects");
		PYTHON_RETURN_ERROR;
	}
	if ((!pyPoint3::Check(startPointObj)) || (!pyPoint3::Check(endPointObj)))
	{
		PyErr_SetString(PyExc_TypeError, "PtCheckVisLOS expects two ptPoint3 objects");
		PYTHON_RETURN_ERROR;
	}
	pyPoint3* startPoint = pyPoint3::ConvertFrom(startPointObj);
	pyPoint3* endPoint = pyPoint3::ConvertFrom(endPointObj);
	return cyMisc::CheckVisLOS(*startPoint, *endPoint);
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtCheckVisLOSFromCursor, "Does LOS check from where the mouse cursor is, into the screen")
{
	return cyMisc::CheckVisLOSFromCursor();
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtEnablePlanarReflections, args, "Params: on\nEnables/disables planar reflections")
{
	char on;
	if (!PyArg_ParseTuple(args, "b", &on))
	{
		PyErr_SetString(PyExc_TypeError, "PtEnablePlanarReflections expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::EnablePlanarReflections(on != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetSupportedDisplayModes, "Returns a list of supported resolutions")
{
	std::vector<plDisplayMode> res;
	cyMisc::GetSupportedDisplayModes(&res);
	PyObject *retVal = PyList_New(0);
	for (std::vector<plDisplayMode>::iterator curArg = res.begin(); curArg != res.end(); ++curArg)
	{
		PyObject* tup = PyTuple_New(2);
		PyTuple_SetItem(tup, 0, PyInt_FromLong((long)(*curArg).Width));
		PyTuple_SetItem(tup, 1, PyInt_FromLong((long)(*curArg).Height));

		PyList_Append(retVal, tup);
	}
	return retVal;
}
PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetDesktopWidth, "Returns desktop width")
{
	return PyInt_FromLong((long)cyMisc::GetDesktopWidth());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetDesktopHeight, "Returns desktop height")
{
	return PyInt_FromLong((long)cyMisc::GetDesktopHeight());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetDesktopColorDepth, "Returns desktop ColorDepth")
{
	return PyInt_FromLong((long)cyMisc::GetDesktopColorDepth());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetDefaultDisplayParams, "Returns the default resolution and display settings")
{
	PipelineParams *pp = cyMisc::GetDefaultDisplayParams();
	PyObject* tup = PyTuple_New(10);
	PyTuple_SetItem(tup, 0, PyInt_FromLong((long)pp->Width));
	PyTuple_SetItem(tup, 1, PyInt_FromLong((long)pp->Height));
	PyTuple_SetItem(tup, 2, PyInt_FromLong((long)pp->Windowed));
	PyTuple_SetItem(tup, 3, PyInt_FromLong((long)pp->ColorDepth));
	PyTuple_SetItem(tup, 4, PyInt_FromLong((long)pp->AntiAliasingAmount));
	PyTuple_SetItem(tup, 5, PyInt_FromLong((long)pp->AnisotropicLevel));
	PyTuple_SetItem(tup, 6, PyInt_FromLong((long)pp->TextureQuality));
	PyTuple_SetItem(tup, 7, PyInt_FromLong((long)pp->VideoQuality));
	PyTuple_SetItem(tup, 8, PyInt_FromLong((long)pp->Shadows));
	PyTuple_SetItem(tup, 9, PyInt_FromLong((long)pp->PlanarReflections));
	return tup;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetGraphicsOptions, args, "Params: width, height, colordepth, windowed, numAAsamples, numAnisoSamples, VSync\nSet the graphics options")
{
	int width = 800, height = 600, colordepth = 32, windowed = 0, numAAsamples = 0, numAnisoSamples = 0, vsync = 0;
	if (!PyArg_ParseTuple(args, "iiiiiii", &width, &height, &colordepth, &windowed, &numAAsamples, &numAnisoSamples, &vsync))
	{
		PyErr_SetString(PyExc_TypeError, "PtSetGraphicsOptions expects a ints for width, height, colordepth, windowed, numAAsamples, numAnisoSamples");
		PYTHON_RETURN_ERROR;
	}

	cyMisc::SetGraphicsOptions(width, height, colordepth, windowed != 0, numAAsamples, numAnisoSamples, vsync != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetBehaviorNetFlags, args, "Params: behKey, netForce, netProp\nSets net flags on the associated behavior")
{
	PyObject* keyObj = NULL;
	char netForce;
	char netProp;
	if (!PyArg_ParseTuple(args, "Obb", &keyObj, &netForce, &netProp))
	{
		PyErr_SetString(PyExc_TypeError, "PtSetBehaviorNetFlags expects a ptKey, a boolean and a boolean");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtSetBehaviorNetFlags expects a ptKey, a boolean and a boolean");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	cyMisc::SetBehaviorNetFlags(*key, netForce != 0, netProp != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSendFriendInvite, args, "Params: emailAddress, toName = \"Friend\"\nSends an email with invite code")
{
	PyObject* emailObj;
	PyObject* toNameObj = nil;
	if (!PyArg_ParseTuple(args, "O|O", &emailObj, &toNameObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtSendFriendInvite expects a string and optionally another string");
		PYTHON_RETURN_ERROR;
	}

	wchar emailAddr[kMaxEmailAddressLength];
	MemSet(emailAddr, 0, sizeof(emailAddr));

	wchar toName[kMaxPlayerNameLength];
	MemSet(toName, 0, sizeof(toName));

	// Check and see if the email address is ok
	int origStrLen = 0;
	if (PyUnicode_Check(emailObj))
	{
		origStrLen = PyUnicode_GET_SIZE(emailObj);
		PyUnicode_AsWideChar((PyUnicodeObject*)emailObj, emailAddr, arrsize(emailAddr) - 1);
	}
	else if (PyString_Check(emailObj))
	{
		char* cAddr = PyString_AsString(emailObj);
		origStrLen = StrLen(cAddr);
		StrToUnicode(emailAddr, cAddr, arrsize(emailAddr));
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "PtSendFriendInvite expects a string and optionally another string");
		PYTHON_RETURN_ERROR;
	}

	if (origStrLen >= kMaxEmailAddressLength)
	{
		PyErr_SetString(PyExc_TypeError, "PtSendFriendInvite: Email address too long");
		PYTHON_RETURN_ERROR;
	}

	// Check if the "to name" field is ok
	if (toNameObj)
	{
		if (PyUnicode_Check(toNameObj))
		{
			origStrLen = PyUnicode_GET_SIZE(toNameObj);
			PyUnicode_AsWideChar((PyUnicodeObject*)toNameObj, toName, arrsize(toName) - 1);
		}
		else if (PyString_Check(toNameObj))
		{
			char* cName = PyString_AsString(toNameObj);
			origStrLen = StrLen(cName);
			StrToUnicode(toName, cName, arrsize(toName));
		}
		else
			StrCopy(toName, L"Friend", arrsize(toName));
	}
	else
		StrCopy(toName, L"Friend", arrsize(toName));

	cyMisc::SendFriendInvite(emailAddr, toName);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGuidGenerate, "Returns string representation for a new guid")
{
	return cyMisc::PyGuidGenerate();
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtGetAIAvatarsByModelName, args, "Params: modelName\nReturns a list of tuples representing the matching ai avatars")
{
	char* modelName;
	if (!PyArg_ParseTuple(args, "s", &modelName))
	{
		PyErr_SetString(PyExc_TypeError, "PtGetAIAvatarsByModelName expects a string");
		PYTHON_RETURN_ERROR;
	}

	return cyMisc::GetAIAvatarsByModelName(modelName);
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtForceVaultNodeUpdate, args, "Params: nodeId\nForces a vault node to update")
{
	unsigned nodeId;
	if (!PyArg_ParseTuple(args, "I", &nodeId))
	{
		PyErr_SetString(PyExc_TypeError, "PtForceVaultNodeUpdate expects an unsigned int");
		PYTHON_RETURN_ERROR;
	}

	cyMisc::ForceVaultNodeUpdate(nodeId);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtVaultDownload, args, "Params: nodeId\nDownloads the vault tree of the given nodeid")
{
	unsigned nodeId;
	if (!PyArg_ParseTuple(args, "I", &nodeId))
	{
		PyErr_SetString(PyExc_TypeError, "PtVaultDownload expects an unsigned int");
		PYTHON_RETURN_ERROR;
	}

	cyMisc::VaultDownload(nodeId);
	PYTHON_RETURN_NONE;
}


///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaMethods - the python method definitions
//

void cyMisc::AddPlasmaMethods4(std::vector<PyMethodDef> &methods)
{
	PYTHON_GLOBAL_METHOD(methods, PtRequestLOSScreen);
	
	PYTHON_GLOBAL_METHOD(methods, PtKillParticles);
	PYTHON_GLOBAL_METHOD(methods, PtGetNumParticles);
	PYTHON_GLOBAL_METHOD(methods, PtSetParticleOffset);

	PYTHON_GLOBAL_METHOD(methods, PtSetLightValue);
	PYTHON_GLOBAL_METHOD(methods, PtSetLightAnimStart);
	
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtIsSinglePlayerMode);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtIsDemoMode);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtIsInternalRelease);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtIsEnterChatModeKeyBound);
	
	PYTHON_GLOBAL_METHOD(methods, PtShootBulletFromScreen);
	PYTHON_GLOBAL_METHOD(methods, PtShootBulletFromObject);
	
	PYTHON_GLOBAL_METHOD(methods, PtGetPublicAgeList);
	PYTHON_GLOBAL_METHOD(methods, PtCreatePublicAge);
	PYTHON_GLOBAL_METHOD(methods, PtRemovePublicAge);
	
	PYTHON_GLOBAL_METHOD(methods, PtSetClearColor);
	
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetLocalKILevel);
	
	PYTHON_BASIC_GLOBAL_METHOD(methods, PtClearCameraStack);
	PYTHON_GLOBAL_METHOD(methods, PtGetCameraNumber);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetNumCameras);
	PYTHON_GLOBAL_METHOD(methods, PtRebuildCameraStack);
	PYTHON_BASIC_GLOBAL_METHOD(methods, PtRecenterCamera);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtFirstPerson);
	
	PYTHON_GLOBAL_METHOD(methods, PtFadeIn);
	PYTHON_GLOBAL_METHOD(methods, PtFadeOut);
	
	PYTHON_GLOBAL_METHOD(methods, PtSetGlobalClickability);
	PYTHON_GLOBAL_METHOD(methods, PtDebugAssert);
	PYTHON_GLOBAL_METHOD(methods, PtSetAlarm);
	
	PYTHON_GLOBAL_METHOD(methods, PtSaveScreenShot);
	PYTHON_GLOBAL_METHOD(methods, PtStartScreenCapture);
	
	PYTHON_GLOBAL_METHOD(methods, PtSendKIGZMarkerMsg);
	PYTHON_GLOBAL_METHOD(methods, PtSendKIRegisterImagerMsg);
	
	PYTHON_GLOBAL_METHOD(methods, PtWearMaintainerSuit);
	PYTHON_GLOBAL_METHOD(methods, PtWearDefaultClothing);
	
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetAgeTimeOfDayPercent);
	
	PYTHON_GLOBAL_METHOD(methods, PtCheckVisLOS);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtCheckVisLOSFromCursor);

	PYTHON_GLOBAL_METHOD(methods, PtEnablePlanarReflections);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetSupportedDisplayModes);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetDesktopWidth);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetDesktopHeight);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetDesktopColorDepth);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetDefaultDisplayParams);
	PYTHON_GLOBAL_METHOD(methods, PtSetGraphicsOptions);

	PYTHON_GLOBAL_METHOD(methods, PtSetBehaviorNetFlags);
	PYTHON_GLOBAL_METHOD(methods, PtSendFriendInvite);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGuidGenerate);
	PYTHON_GLOBAL_METHOD(methods, PtGetAIAvatarsByModelName);
	PYTHON_GLOBAL_METHOD(methods, PtForceVaultNodeUpdate);
	PYTHON_GLOBAL_METHOD(methods, PtVaultDownload);
}