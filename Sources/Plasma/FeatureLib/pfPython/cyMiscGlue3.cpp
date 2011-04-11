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

#include <python.h>

PYTHON_GLOBAL_METHOD_DEFINITION(PtSendPetitionToCCR, args, "Params: message,reason=0,title=\"\"\nSends a petition with a message to the CCR group")
{
	char* message;
	unsigned char reason = 0;
	char* title = nil;
	if (!PyArg_ParseTuple(args, "s|bs", &message, &reason, &title))
	{
		PyErr_SetString(PyExc_TypeError, "PtSendPetitionToCCR expects a string, and an optional unsigned 8-bit int and optional string");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::SendPetitionToCCRI(message, reason, title);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSendChatToCCR, args, "Params: message,CCRPlayerID\nSends a chat message to a CCR that has contacted this player")
{
	char* message;
	long CCRPlayerID;
	if (!PyArg_ParseTuple(args, "sl", &message, &CCRPlayerID))
	{
		PyErr_SetString(PyExc_TypeError, "PtSendChatToCCR expects a string and a long");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::SendChatToCCR(message, CCRPlayerID);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetPythonLoggingLevel, "Returns the current level of python logging")
{
	return PyLong_FromUnsignedLong(cyMisc::GetPythonLoggingLevel());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetPythonLoggingLevel, args, "Params: level\nSets the current level of python logging")
{
	unsigned long level;
	if (!PyArg_ParseTuple(args, "l", &level))
	{
		PyErr_SetString(PyExc_TypeError, "PtSetPythonLoggingLevel expects an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::SetPythonLoggingLevel(level);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtConsole, args, "Params: command\nThis will execute 'command' as if it were typed into the Plasma console.")
{
	char* command;
	if (!PyArg_ParseTuple(args, "s", &command))
	{
		PyErr_SetString(PyExc_TypeError, "PtConsole expects a string");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::Console(command);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtConsoleNet, args, "Params: command,netForce\nThis will execute 'command' on the console, over the network, on all clients.\n"
			"If 'netForce' is true then force command to be sent over the network.")
{
	char* command;
	char netForce;
	if (!PyArg_ParseTuple(args, "sb", &command, &netForce))
	{
		PyErr_SetString(PyExc_TypeError, "PtConsoleNet expects a string and a boolean");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::ConsoleNet(command, netForce != 0);
	PYTHON_RETURN_NONE;
}

#if 1
// TEMP
PYTHON_GLOBAL_METHOD_DEFINITION(PtPrintToScreen, args, "Params: message\nPrints 'message' to the status log, for debug only.")
{
	char* message;
	if (!PyArg_ParseTuple(args, "s", &message))
	{
		PyErr_SetString(PyExc_TypeError, "PtPrintToScreen expects a string");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::PrintToScreen(message);
	PYTHON_RETURN_NONE;
}
#endif

PYTHON_GLOBAL_METHOD_DEFINITION(PtAtTimeCallback, args, "Params: selfkey,time,id\nThis will create a timer callback that will call OnTimer when complete\n"
			"- 'selfkey' is the ptKey of the PythonFile component\n"
			"- 'time' is how much time from now (in seconds) to call back\n"
			"- 'id' is an integer id that will be returned in the OnTimer call")
{
	PyObject* keyObj = NULL;
	float time;
	unsigned long id;
	if (!PyArg_ParseTuple(args, "Ofl", &keyObj, &time, &id))
	{
		PyErr_SetString(PyExc_TypeError, "PtAtTimeCallback expects a ptKey, a float, and an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtAtTimeCallback expects a ptKey, a float, and an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	cyMisc::TimerCallback(*key, time, id);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtClearTimerCallbacks, args, "Params: key\nThis will remove timer callbacks to the specified key")
{
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtClearTimerCallbacks expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtClearTimerCallbacks expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	cyMisc::ClearTimerCallbacks(*key);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtFindSceneobject, args, "Params: name,ageName\nThis will try to find a sceneobject based on its name and what age its in\n"
			"- it will return a ptSceneObject if found"
			"- if not found then a NameError exception will happen")
{
	char* name;
	char* ageName;
	if (!PyArg_ParseTuple(args, "ss", &name, &ageName))
	{
		PyErr_SetString(PyExc_TypeError, "PtFindSceneobject expects two strings");
		PYTHON_RETURN_ERROR;
	}
	return cyMisc::FindSceneObject(name, ageName);
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtFindActivator, args, "Params: name\nThis will try to find an activator based on its name\n"
			"- it will return a ptKey if found"
			"- it will return None if not found")
{
	char* name;
	if (!PyArg_ParseTuple(args, "s", &name))
	{
		PyErr_SetString(PyExc_TypeError, "PtFindActivator expects a string");
		PYTHON_RETURN_ERROR;
	}

	return cyMisc::FindActivator(name);
}

PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtClearCameraStack, cyMisc::ClearCameraStack, "Clears the camera stack")

PYTHON_GLOBAL_METHOD_DEFINITION(PtWasLocallyNotified, args, "Params: selfKey\nReturns 1 if the last notify was local or 0 if the notify originated on the network")
{
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtWasLocallyNotified expects a ptKey");
		PYTHON_RETURN_NONE;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtWasLocallyNotified expects a ptKey");
		PYTHON_RETURN_NONE;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	PYTHON_RETURN_BOOL(cyMisc::WasLocallyNotified(*key));
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtAttachObject, args, "Params: child,parent\nAttach child to parent based on ptKey or ptSceneobject\n"
			"- childKey is the ptKey or ptSceneobject of the one being attached\n"
			"- parentKey is the ptKey or ptSceneobject of the one being attached to\n"
			"(both arguments must be ptKeys or ptSceneobjects, you cannot mix types)")
{
	PyObject* childObj = NULL;
	PyObject* parentObj = NULL;
	if (!PyArg_ParseTuple(args, "OO", &childObj, &parentObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtAttachObject expects either two ptKeys or two ptSceneobjects");
		PYTHON_RETURN_ERROR;
	}
	if ((pyKey::Check(childObj)) && (pyKey::Check(parentObj)))
	{
		pyKey* child = pyKey::ConvertFrom(childObj);
		pyKey* parent = pyKey::ConvertFrom(parentObj);
		cyMisc::AttachObject(*child, *parent);
	}
	else if ((pySceneObject::Check(childObj)) && (pySceneObject::Check(parentObj)))
	{
		pySceneObject* child = pySceneObject::ConvertFrom(childObj);
		pySceneObject* parent = pySceneObject::ConvertFrom(parentObj);
		cyMisc::AttachObjectSO(*child, *parent);
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "PtAttachObject expects either two ptKeys or two ptSceneobjects");
		PYTHON_RETURN_ERROR;
	}
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtDetachObject, args, "Params: child,parent\nDetach child from parent based on ptKey or ptSceneobject\n"
			"- child is the ptKey or ptSceneobject of the one being detached\n"
			"- parent is the ptKey or ptSceneobject of the one being detached from\n"
			"(both arguments must be ptKeys or ptSceneobjects, you cannot mix types)")
{
	PyObject* childObj = NULL;
	PyObject* parentObj = NULL;
	if (!PyArg_ParseTuple(args, "OO", &childObj, &parentObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtDetachObject expects either two ptKeys or two ptSceneobjects");
		PYTHON_RETURN_ERROR;
	}
	if ((pyKey::Check(childObj)) && (pyKey::Check(parentObj)))
	{
		pyKey* child = pyKey::ConvertFrom(childObj);
		pyKey* parent = pyKey::ConvertFrom(parentObj);
		cyMisc::DetachObject(*child, *parent);
	}
	else if ((pySceneObject::Check(childObj)) && (pySceneObject::Check(parentObj)))
	{
		pySceneObject* child = pySceneObject::ConvertFrom(childObj);
		pySceneObject* parent = pySceneObject::ConvertFrom(parentObj);
		cyMisc::DetachObjectSO(*child, *parent);
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "PtDetachObject expects either two ptKeys or two ptSceneobjects");
		PYTHON_RETURN_ERROR;
	}
	PYTHON_RETURN_NONE;
}

/*PYTHON_GLOBAL_METHOD_DEFINITION(PtLinkToAge, args, "Params: selfKey,ageName,spawnPointName\nDEPRECIATED: Links you to the specified age and spawnpoint")
{
	PyObject* keyObj = NULL;
	char* ageName;
	char* spawnPointName;
	if (!PyArg_ParseTuple(args, "Oss", &keyObj, &ageName, &spawnPointName))
	{
		PyErr_SetString(PyExc_TypeError, "PtLinkToAge expects a ptKey, and two strings");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtLinkToAge expects a ptKey, and two strings");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	cyMisc::LinkToAge(*key, ageName, spawnPointName);
	PYTHON_RETURN_NONE;
}*/

PYTHON_GLOBAL_METHOD_DEFINITION(PtDirtySynchState, args, "Params: selfKey,SDLStateName,flags\nDO NOT USE - handled by ptSDL")
{
	PyObject* keyObj = NULL;
	char* SDLStateName;
	unsigned long flags;
	if (!PyArg_ParseTuple(args, "Osl", &keyObj, &SDLStateName, &flags))
	{
		PyErr_SetString(PyExc_TypeError, "PtDirtySynchState expects a ptKey, a string, and an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtDirtySynchState expects a ptKey, a string, and an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	cyMisc::SetDirtySyncState(*key, SDLStateName, flags);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtDirtySynchClients, args, "Params: selfKey,SDLStateName,flags\nDO NOT USE - handled by ptSDL")
{
	PyObject* keyObj = NULL;
	char* SDLStateName;
	unsigned long flags;
	if (!PyArg_ParseTuple(args, "Osl", &keyObj, &SDLStateName, &flags))
	{
		PyErr_SetString(PyExc_TypeError, "PtDirtySynchClients expects a ptKey, a string, and an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtDirtySynchClients expects a ptKey, a string, and an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	cyMisc::SetDirtySyncStateWithClients(*key, SDLStateName, flags);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtEnableControlKeyEvents, args, "Params: selfKey\nEnable control key events to call OnControlKeyEvent(controlKey,activateFlag)")
{
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtEnableControlKeyEvents expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtEnableControlKeyEvents expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	cyMisc::EnableControlKeyEvents(*key);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtDisableControlKeyEvents, args, "Params: selfKey\nDisable the control key events from calling OnControlKeyEvent")
{
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtDisableControlKeyEvents expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtDisableControlKeyEvents expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	cyMisc::DisableControlKeyEvents(*key);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtEnableAvatarCursorFade, cyMisc::EnableAvatarCursorFade, "Enable the avatar cursor fade")
PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtDisableAvatarCursorFade, cyMisc::DisableAvatarCursorFade, "Disable the avatar cursor fade")

PYTHON_GLOBAL_METHOD_DEFINITION(PtFadeLocalAvatar, args, "Params: fade\nFade (or unfade) the local avatar")
{
	char fade;
	if (!PyArg_ParseTuple(args, "b", &fade))
	{
		PyErr_SetString(PyExc_TypeError, "PtFadeLocalAvatar expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::FadeLocalPlayer(fade != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetOfferBookMode, args, "Params: selfkey,ageFilename,ageInstanceName\nPut us into the offer book interface")
{
	PyObject* keyObj = NULL;
	char* ageFilename;
	char* ageInstanceName;
	if (!PyArg_ParseTuple(args, "Oss", &keyObj, &ageFilename, &ageInstanceName))
	{
		PyErr_SetString(PyExc_TypeError, "PtSetOfferBookMode expects a ptKey, and two strings");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtSetOfferBookMode expects a ptKey, and two strings");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	cyMisc::EnableOfferBookMode(*key, ageFilename, ageInstanceName);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetShareSpawnPoint, args, "Params: spawnPoint\nThis sets the desired spawn point for the receiver to link to")
{
	char* spawnPoint;
	if (!PyArg_ParseTuple(args, "s", &spawnPoint))
	{
		PyErr_SetString(PyExc_TypeError, "PtSetShareSpawnPoint expects a string");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::SetShareSpawnPoint(spawnPoint);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetShareAgeInstanceGuid, args, "Params: instanceGuid\nThis sets the desired age instance guid for the receiver to link to")
{
	char* guidStr;
	if (!PyArg_ParseTuple(args, "s", &guidStr))
	{
		PyErr_SetString(PyExc_TypeError, "PtSetShareAgeInstanceGuid expects a string");
		PYTHON_RETURN_ERROR;
	}
	Uuid guid;
	if (!GuidFromString(guidStr, &guid))
	{
		PyErr_SetString(PyExc_TypeError, "PtSetShareAgeInstanceGuid string parameter is not a guid string");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::SetShareAgeInstanceGuid(guid);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtNotifyOffererLinkAccepted, args, "Params: offerer\nTell the offerer that we accepted the link offer")
{
	unsigned long offerer;
	if (!PyArg_ParseTuple(args, "l", &offerer))
	{
		PyErr_SetString(PyExc_TypeError, "PtNotifyOffererLinkAccepted expects an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::NotifyOffererPublicLinkAccepted(offerer);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtNotifyOffererLinkRejected, args, "Params: offerer\nTell the offerer that we rejected the link offer")
{
	unsigned long offerer;
	if (!PyArg_ParseTuple(args, "l", &offerer))
	{
		PyErr_SetString(PyExc_TypeError, "PtNotifyOffererLinkRejected expects an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::NotifyOffererPublicLinkRejected(offerer);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtNotifyOffererLinkCompleted, args, "Params: offerer\nTell the offerer that we completed the link")
{
	unsigned long offerer;
	if (!PyArg_ParseTuple(args, "l", &offerer))
	{
		PyErr_SetString(PyExc_TypeError, "PtNotifyOffererLinkCompleted expects an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::NotifyOffererPublicLinkCompleted(offerer);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtClearOfferBookMode, cyMisc::DisableOfferBookMode, "Cancel the offer book interface")

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetLocalClientID, "Returns our local client ID number")
{
	return PyInt_FromLong(cyMisc::GetLocalClientID());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtIsCCRAway, "Returns current status of CCR dept")
{
	PYTHON_RETURN_BOOL(cyMisc::IsCCRAwayStatus());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtAmCCR, "Returns true if local player is a CCR")
{
	PYTHON_RETURN_BOOL(cyMisc::AmCCR());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtToggleAvatarClickability, args, "Params: on\nTurns on and off our avatar's clickability")
{
	char on;
	if (!PyArg_ParseTuple(args, "b", &on))
	{
		PyErr_SetString(PyExc_TypeError, "PtToggleAvatarClickability expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::ToggleAvatarClickability(on != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtTransferParticlesToObject, args, "Params: objFrom, objTo, num\nTransfers num particles from objFrom to objTo")
{
	PyObject* objFrom = NULL;
	PyObject* objTo = NULL;
	int num;
	if (!PyArg_ParseTuple(args, "OOi", &objFrom, &objTo, &num))
	{
		PyErr_SetString(PyExc_TypeError, "PtTransferParticlesToObject expects two ptKeys and an int");
		PYTHON_RETURN_ERROR;
	}
	if ((!pyKey::Check(objFrom)) || (!pyKey::Check(objTo)))
	{
		PyErr_SetString(PyExc_TypeError, "PtTransferParticlesToObject expects two ptKeys and an int");
		PYTHON_RETURN_ERROR;
	}
	pyKey* from = pyKey::ConvertFrom(objFrom);
	pyKey* to = pyKey::ConvertFrom(objTo);
	cyMisc::TransferParticlesToKey(*from, *to, num);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetParticleDissentPoint, args, "Params: x, y, z, particlesys\nSets the dissent point of the particlesys to x,y,z")
{
	float x,y,z;
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "fffO", &x, &y, &z, &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtSetParticleDissentPoint expects three floats and a ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtSetParticleDissentPoint expects three floats and a ptKey");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	cyMisc::SetParticleDissentPoint(x, y, z, *key);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtGetControlEvents, args, "Params: on, key\nRegisters or unregisters for control event messages")
{
	char on;
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "bO", &on, &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtGetControlEvents expects a boolean and a ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtGetControlEvents expects a boolean and a ptKey");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	cyMisc::RegisterForControlEventMessages(on != 0, *key);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetLanguage, "Returns the current language as a PtLanguage enum")
{
	return PyInt_FromLong(cyMisc::GetLanguage());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtUsingUnicode, "Returns true if the current language is a unicode language (like Japanese)")
{
	PYTHON_RETURN_BOOL(cyMisc::UsingUnicode());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtFakeLinkAvatarToObject, args, "Params: avatar,object\nPseudo-links avatar to object within the same age\n")
{
	PyObject* avatarObj = NULL;
	PyObject* objectObj = NULL;
	if (!PyArg_ParseTuple(args, "OO", &avatarObj, &objectObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtFakeLinkAvatarToObject expects two ptKeys");
		PYTHON_RETURN_ERROR;
	}
	if ((!pyKey::Check(avatarObj)) || (!pyKey::Check(objectObj)))
	{
		PyErr_SetString(PyExc_TypeError, "PtFakeLinkAvatarToObject expects two ptKeys");
		PYTHON_RETURN_ERROR;
	}
	pyKey* avatar = pyKey::ConvertFrom(avatarObj);
	pyKey* object = pyKey::ConvertFrom(objectObj);
	cyMisc::FakeLinkToObject(*avatar, *object);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtWearDefaultClothingType, args, "Params: key,type\nForces the avatar to wear the default clothing of the specified type")
{
	PyObject* keyObj = NULL;
	unsigned long type;
	if (!PyArg_ParseTuple(args, "Ol", &keyObj, &type))
	{
		PyErr_SetString(PyExc_TypeError, "PtWearDefaultClothingType expects a ptKey and an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtWearDefaultClothingType expects a ptKey and an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	cyMisc::WearDefaultClothingType(*key, type);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtFileExists, args, "Params: filename\nReturns true if the specified file exists")
{
	PyObject* filenameObj;
	if (!PyArg_ParseTuple(args, "O", &filenameObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtFileExists expects a string");
		PYTHON_RETURN_ERROR;
	}

	if (PyUnicode_Check(filenameObj))
	{
		int strLen = PyUnicode_GetSize(filenameObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)filenameObj, text, strLen);
		text[strLen] = L'\0';
		bool retVal = cyMisc::FileExists(text);
		delete [] text;
		PYTHON_RETURN_BOOL(retVal);
	}
	else if (PyString_Check(filenameObj))
	{
		// we'll allow this, just in case something goes weird
		char* text = PyString_AsString(filenameObj);
		wchar_t* wText = hsStringToWString(text);
		bool retVal = cyMisc::FileExists(wText);
		delete [] wText;
		PYTHON_RETURN_BOOL(retVal);
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "PtFileExists expects a string");
		PYTHON_RETURN_ERROR;
	}
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtCreateDir, args, "Params: directory\nCreates the directory and all parent folders. Returns false on failure")
{
	PyObject* directoryObj;
	if (!PyArg_ParseTuple(args, "O", &directoryObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtCreateDir expects a string");
		PYTHON_RETURN_ERROR;
	}

	if (PyUnicode_Check(directoryObj))
	{
		int strLen = PyUnicode_GetSize(directoryObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)directoryObj, text, strLen);
		text[strLen] = L'\0';
		bool retVal = cyMisc::CreateDir(text);
		delete [] text;
		PYTHON_RETURN_BOOL(retVal);
	}
	else if (PyString_Check(directoryObj))
	{
		// we'll allow this, just in case something goes weird
		char* text = PyString_AsString(directoryObj);
		wchar_t* wText = hsStringToWString(text);
		bool retVal = cyMisc::CreateDir(wText);
		delete [] wText;
		PYTHON_RETURN_BOOL(retVal);
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "PtCreateDir expects a string");
		PYTHON_RETURN_ERROR;
	}
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetUserPath, "Returns the unicode path to the client's root user directory. Do NOT convert to a standard string.")
{
	std::wstring val = cyMisc::GetUserPath();
	return PyUnicode_FromWideChar(val.c_str(), val.length());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetInitPath, "Returns the unicode path to the client's init directory. Do NOT convert to a standard string.")
{
	std::wstring val = cyMisc::GetInitPath();
	return PyUnicode_FromWideChar(val.c_str(), val.length());
}

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaMethods - the python method definitions
//

void cyMisc::AddPlasmaMethods3(std::vector<PyMethodDef> &methods)
{
	PYTHON_GLOBAL_METHOD(methods, PtSendPetitionToCCR);
	PYTHON_GLOBAL_METHOD(methods, PtSendChatToCCR);

	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetPythonLoggingLevel);
	PYTHON_GLOBAL_METHOD(methods, PtSetPythonLoggingLevel);

	PYTHON_GLOBAL_METHOD(methods, PtConsole);
	PYTHON_GLOBAL_METHOD(methods, PtConsoleNet);

#if 1
	// TEMP
	PYTHON_GLOBAL_METHOD(methods, PtPrintToScreen);
#endif

	PYTHON_GLOBAL_METHOD(methods, PtAtTimeCallback);
	PYTHON_GLOBAL_METHOD(methods, PtClearTimerCallbacks);
	
	PYTHON_GLOBAL_METHOD(methods, PtFindSceneobject);
	PYTHON_GLOBAL_METHOD(methods, PtFindActivator);
	PYTHON_BASIC_GLOBAL_METHOD(methods, PtClearCameraStack);
	PYTHON_GLOBAL_METHOD(methods, PtWasLocallyNotified);

	PYTHON_GLOBAL_METHOD(methods, PtAttachObject);
	PYTHON_GLOBAL_METHOD(methods, PtDetachObject);
	
	//PYTHON_GLOBAL_METHOD(methods, PtLinkToAge);
	
	PYTHON_GLOBAL_METHOD(methods, PtDirtySynchState);
	PYTHON_GLOBAL_METHOD(methods, PtDirtySynchClients);

	PYTHON_GLOBAL_METHOD(methods, PtEnableControlKeyEvents);
	PYTHON_GLOBAL_METHOD(methods, PtDisableControlKeyEvents);

	PYTHON_BASIC_GLOBAL_METHOD(methods, PtEnableAvatarCursorFade);
	PYTHON_BASIC_GLOBAL_METHOD(methods, PtDisableAvatarCursorFade);
	PYTHON_GLOBAL_METHOD(methods, PtFadeLocalAvatar);

	PYTHON_GLOBAL_METHOD(methods, PtSetOfferBookMode);
	PYTHON_GLOBAL_METHOD(methods, PtSetShareSpawnPoint);
	PYTHON_GLOBAL_METHOD(methods, PtSetShareAgeInstanceGuid);
	PYTHON_GLOBAL_METHOD(methods, PtNotifyOffererLinkAccepted);
	PYTHON_GLOBAL_METHOD(methods, PtNotifyOffererLinkRejected);
	PYTHON_GLOBAL_METHOD(methods, PtNotifyOffererLinkCompleted);
	PYTHON_BASIC_GLOBAL_METHOD(methods, PtClearOfferBookMode);

	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetLocalClientID);
	
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtIsCCRAway);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtAmCCR);

	PYTHON_GLOBAL_METHOD(methods, PtToggleAvatarClickability);
	
	PYTHON_GLOBAL_METHOD(methods, PtTransferParticlesToObject);
	PYTHON_GLOBAL_METHOD(methods, PtSetParticleDissentPoint);

	PYTHON_GLOBAL_METHOD(methods, PtGetControlEvents);
	
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetLanguage);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtUsingUnicode);
	
	PYTHON_GLOBAL_METHOD(methods, PtFakeLinkAvatarToObject);
	
	PYTHON_GLOBAL_METHOD(methods, PtWearDefaultClothingType);

	PYTHON_GLOBAL_METHOD(methods, PtFileExists);
	PYTHON_GLOBAL_METHOD(methods, PtCreateDir);

	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetUserPath);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetInitPath);
}