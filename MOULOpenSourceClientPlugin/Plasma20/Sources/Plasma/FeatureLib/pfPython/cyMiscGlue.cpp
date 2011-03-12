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
#include "pyPlayer.h"

#include "hsUtils.h"

#include <python.h>

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetAgeName, "DEPRECIATED - use ptDniInfoSource instead")
{
	return PyString_FromString(cyMisc::GetAgeName());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetAgeInfo, "Returns ptAgeInfoStruct of the current Age")
{
	return cyMisc::GetAgeInfo();
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetAgeTime, "DEPRECIATED - use ptDniInfoSource instead")
{
	return PyLong_FromUnsignedLong(cyMisc::GetAgeTime());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetPrevAgeName, "Returns filename of previous age visited")
{
	return PyString_FromString(cyMisc::GetPrevAgeName());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetPrevAgeInfo, "Returns ptAgeInfoStruct of previous age visited")
{
	return cyMisc::GetPrevAgeInfo();
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetDniTime, "Returns current D'Ni time")
{
	return PyLong_FromUnsignedLong(cyMisc::GetDniTime());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetServerTime, "Returns the current time on the server (which is GMT)")
{
	return PyLong_FromUnsignedLong(cyMisc::GetServerTime());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtGMTtoDniTime, args, "Params: gtime\nConverts GMT time (passed in) to D'Ni time")
{
	unsigned long gtime;
	if (!PyArg_ParseTuple(args, "l", &gtime))
	{
		PyErr_SetString(PyExc_TypeError, "PtGMTtoDniTime expects a long");
		PYTHON_RETURN_ERROR;
	}
	return PyLong_FromUnsignedLong(cyMisc::ConvertGMTtoDni(gtime));
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtGetClientName, args, "Params: avatarKey=None\nThis will return the name of the client that is owned by the avatar\n"
			"- avatarKey is the ptKey of the avatar to get the client name of.\n"
			"If avatarKey is omitted then the local avatar is used")
{
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "|O", &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtGetClientName expects an optional ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (keyObj != NULL)
	{
		if (!pyKey::Check(keyObj))
		{
			PyErr_SetString(PyExc_TypeError, "PtGetClientName expects a ptKey");
			PYTHON_RETURN_ERROR;
		}
		pyKey* key = pyKey::ConvertFrom(keyObj);
		return PyString_FromString(cyMisc::GetClientName(*key));
	}
	else
		return PyString_FromString(cyMisc::GetLocalClientName());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetLocalAvatar, "This will return a ptSceneobject of the local avatar\n"
			"- if there is no local avatar a NameError exception will happen.")
{
	return cyMisc::GetLocalAvatar();
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetLocalPlayer, "Returns a ptPlayer object of the local player")
{
	return cyMisc::GetLocalPlayer();
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetPlayerList, "Returns a list of ptPlayer objects of all the remote players")
{
	std::vector<PyObject*> playerList = cyMisc::GetPlayerList();
	PyObject* retVal = PyList_New(playerList.size());
	for (int i = 0; i < playerList.size(); i++)
		PyList_SetItem(retVal, i, playerList[i]); // steals the ref
	return retVal;
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetPlayerListDistanceSorted, "Returns a list of ptPlayers, sorted by distance")
{
	std::vector<PyObject*> playerList = cyMisc::GetPlayerListDistanceSorted();
	PyObject* retVal = PyList_New(playerList.size());
	for (int i = 0; i < playerList.size(); i++)
		PyList_SetItem(retVal, i, playerList[i]); // steals the ref
	return retVal;
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtMaxListenListSize, "Returns the maximum listen number of players")
{
	return PyLong_FromUnsignedLong(cyMisc::GetMaxListenListSize());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtMaxListenDistSq, "Returns the maximum distance (squared) of the listen range")
{
	return PyFloat_FromDouble(cyMisc::GetMaxListenDistSq());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtGetAvatarKeyFromClientID, args, "Params: clientID\nFrom an integer that is the clientID, find the avatar and return its ptKey")
{
	int clientID;
	if (!PyArg_ParseTuple(args, "i", &clientID))
	{
		PyErr_SetString(PyExc_TypeError, "PtGetAvatarKeyFromClientID expects an integer");
		PYTHON_RETURN_ERROR;
	}
	return cyMisc::GetAvatarKeyFromClientID(clientID);
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtGetClientIDFromAvatarKey, args, "Params: avatarKey\nFrom a ptKey that points at an avatar, return the players clientID (integer)")
{
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtGetClientIDFromAvatarKey expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtGetClientIDFromAvatarKey expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	pyKey *key = pyKey::ConvertFrom(keyObj);
	return PyInt_FromLong(cyMisc::GetClientIDFromAvatarKey(*key));
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetNumRemotePlayers, "Returns the number of remote players in this Age with you.")
{
	return PyInt_FromLong(cyMisc::GetNumRemotePlayers());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtValidateKey, args, "Params: key\nReturns true(1) if 'key' is valid and loaded,\n"
			"otherwise returns false(0)")
{
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtValidateKey expects an object");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PYTHON_RETURN_BOOL(false);
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	PYTHON_RETURN_BOOL(cyMisc::ValidateKey(*key));
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSendRTChat, args, "Params: fromPlayer,toPlayerList,message,flags\nSends a realtime chat message to the list of ptPlayers\n"
			"If toPlayerList is an empty list, it is a broadcast message")
{
	PyObject* fromPlayerObj = NULL;
	PyObject* toPlayerListObj = NULL;
	char* message = NULL;
	unsigned long msgFlags;
	if (!PyArg_ParseTuple(args, "OOsl", &fromPlayerObj, &toPlayerListObj, &message, &msgFlags))
	{
		PyErr_SetString(PyExc_TypeError, "PtSendRTChat expects a ptPlayer, a list of ptPlayers, a string, and a long");
		PYTHON_RETURN_ERROR;
	}
	if (!pyPlayer::Check(fromPlayerObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtSendRTChat expects a ptPlayer, a list of ptPlayers, a string, and a long");
		PYTHON_RETURN_ERROR;
	}

	pyPlayer* fromPlayer = pyPlayer::ConvertFrom(fromPlayerObj);

	std::vector<pyPlayer*> toPlayerList;
	if (PyList_Check(toPlayerListObj))
	{
		int listSize = PyList_Size(toPlayerListObj);
		for (int i = 0; i < listSize; i++)
		{
			PyObject* listItem = PyList_GetItem(toPlayerListObj, i);
			if (!pyPlayer::Check(listItem))
			{
				PyErr_SetString(PyExc_TypeError, "PtSendRTChat expects a ptPlayer, a list of ptPlayers, a string, and a long");
				PYTHON_RETURN_ERROR;
			}
			toPlayerList.push_back(pyPlayer::ConvertFrom(listItem));
		}
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "PtSendRTChat expects a ptPlayer, a list of ptPlayers, a string, and a long");
		PYTHON_RETURN_ERROR;
	}

	return PyLong_FromUnsignedLong(cyMisc::SendRTChat(*fromPlayer, toPlayerList, message, msgFlags));
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSendKIMessage, args, "Params: command,value\nSends a command message to the KI frontend.\n"
			"See PlasmaKITypes.py for list of commands")
{
	unsigned long command;
	PyObject* val;
	if (!PyArg_ParseTuple(args, "lO", &command, &val))
	{
		PyErr_SetString(PyExc_TypeError, "PtSendKIMessage expects a long and either a float or a string");
		PYTHON_RETURN_ERROR;
	}
	if (PyString_Check(val))
	{
		char* strValue = PyString_AsString(val);
		wchar_t* temp = hsStringToWString(strValue);
		cyMisc::SendKIMessageS(command, temp);
		delete [] temp;
	}
	else if (PyUnicode_Check(val))
	{
		int len = PyUnicode_GetSize(val);
		wchar_t* buffer = TRACKED_NEW wchar_t[len + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)val, buffer, len);
		buffer[len] = L'\0';
		cyMisc::SendKIMessageS(command, buffer);
		delete [] buffer;
	}
	else if (PyFloat_Check(val))
	{
		float floatValue = (float)PyFloat_AsDouble(val);
		cyMisc::SendKIMessage(command, floatValue);
	}
	else if (PyInt_Check(val))
	{
		// accepting an int if people get lazy
		float floatValue = (float)PyInt_AsLong(val);
		cyMisc::SendKIMessage(command, floatValue);
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "PtSendKIMessage expects a long and either a float or a string");
		PYTHON_RETURN_ERROR;
	}
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSendKIMessageInt, args, "Params: command,value\nSame as PtSendKIMessage except the value is guaranteed to be a UInt32\n"
			"(for things like player IDs)")
{
	unsigned long command;
	long val;
	if (!PyArg_ParseTuple(args, "ll", &command, &val))
	{
		PyErr_SetString(PyExc_TypeError, "PtSendKIMessageInt expects two longs");
		PYTHON_RETURN_ERROR;
	}
	cyMisc::SendKIMessageI(command, val);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtLoadAvatarModel, args, "Params: modelName, spawnPoint, userStr = \"\"\nLoads an avatar model at the given spawn point. Assigns the user specified string to it.")
{
	char* modelName;
	PyObject* keyObj = NULL;
	PyObject* userStrObj = NULL;
	if (!PyArg_ParseTuple(args, "sO|O", &modelName, &keyObj, &userStrObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtLoadAvatarModel expects a string, a ptKey, and an optional string");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtLoadAvatarModel expects a string, a ptKey, and an optional string");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);

	std::string userStr = "";
	// convert name from a string or unicode string
	if (userStrObj)
	{
		if (PyUnicode_Check(userStrObj))
		{
			int len = PyUnicode_GetSize(userStrObj);
			wchar_t* buffer = TRACKED_NEW wchar_t[len + 1];
			PyUnicode_AsWideChar((PyUnicodeObject*)userStrObj, buffer, len);
			buffer[len] = L'\0';
			char* temp = hsWStringToString(buffer);
			delete [] buffer;
			userStr = temp;
			delete [] temp;
		}
		else if (PyString_Check(userStrObj))
			userStr = PyString_AsString(userStrObj);
		else
		{
			PyErr_SetString(PyExc_TypeError, "PtLoadAvatarModel expects a string, a ptKey, and an optional string");
			PYTHON_RETURN_ERROR;
		}
	}

	return cyMisc::LoadAvatarModel(modelName, *key, userStr.c_str());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtUnLoadAvatarModel, args, "Params: avatarKey\nUnloads the specified avatar model")
{
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtUnLoadAvatarModel expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtUnLoadAvatarModel expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	cyMisc::UnLoadAvatarModel(*key);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtForceCursorHidden, cyMisc::ForceCursorHidden, "Forces the cursor to hide, overriding everything.\n"
			"Only call if other methods won't work. The only way to show the cursor after this call is PtForceMouseShown()")
PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(PtForceCursorShown, cyMisc::ForceCursorShown, "Forces the cursor to show, overriding everything.\n"
			"Only call if other methods won't work. This is the only way to show the cursor after a call to PtForceMouseHidden()")

PYTHON_GLOBAL_METHOD_DEFINITION(PtGetLocalizedString, args, "Params: name, arguments=None\nReturns the localized string specified by name "
			"(format is Age.Set.Name) and substitutes the arguments in the list of strings passed in as arguments.")
{
	PyObject* nameObj = NULL;
	PyObject* argObj = NULL;
	if (!PyArg_ParseTuple(args, "O|O", &nameObj, &argObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtGetLocalizedString expects a unicode string and a list of unicode strings");
		PYTHON_RETURN_ERROR;
	}
	std::wstring name;
	std::vector<std::wstring> argList;

	// convert name from a string or unicode string
	if (PyUnicode_Check(nameObj))
	{
		int len = PyUnicode_GetSize(nameObj);
		wchar_t* buffer = TRACKED_NEW wchar_t[len + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)nameObj, buffer, len);
		buffer[len] = L'\0';
		name = buffer;
		delete [] buffer;
	}
	else if (PyString_Check(nameObj))
	{
		char* temp = PyString_AsString(nameObj);
		wchar_t* wTemp = hsStringToWString(temp);
		name = wTemp;
		delete [] wTemp;
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "PtGetLocalizedString expects a unicode string and a list of unicode strings");
		PYTHON_RETURN_ERROR;
	}

	if (argObj != NULL) // NULL is valid... but won't fill the args vector
	{
		// convert args from a list of strings or unicode strings
		if (!PyList_Check(argObj))
		{
			PyErr_SetString(PyExc_TypeError, "PtGetLocalizedString expects a unicode string and a list of unicode strings");
			PYTHON_RETURN_ERROR;
		}

		int len = PyList_Size(argObj);
		for (int curItem = 0; curItem < len; curItem++)
		{
			PyObject* item = PyList_GetItem(argObj, curItem);
			std::wstring arg = L"INVALID ARG";
			if (item == Py_None) // none is allowed, but treated as a blank string
				arg = L"";
			else if (PyUnicode_Check(item))
			{
				int strLen = PyUnicode_GetSize(item);
				wchar_t* buffer = TRACKED_NEW wchar_t[strLen + 1];
				PyUnicode_AsWideChar((PyUnicodeObject*)item, buffer, strLen);
				buffer[strLen] = L'\0';
				arg = buffer;
				delete [] buffer;
			}
			else if (PyString_Check(item))
			{
				char* temp = PyString_AsString(item);
				wchar_t* wTemp = hsStringToWString(temp);
				arg = wTemp;
				delete [] wTemp;
			}
			// everything else won't throw an error, but will show up as INVALID ARG in the string
			argList.push_back(arg);
		}
	}

	std::wstring retVal = cyMisc::GetLocalizedString(name, argList);
	return PyUnicode_FromWideChar(retVal.c_str(), retVal.length());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtDumpLogs, args, "Params: folder\nDumps all current log files to the specified folder (a sub-folder to the log folder)")
{
	PyObject* folderObj = NULL;
	if (!PyArg_ParseTuple(args, "O|O", &folderObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtDumpLogs expects a unicode or standard string");
		PYTHON_RETURN_ERROR;
	}

	// convert folder from a string or unicode string
	if (PyUnicode_Check(folderObj))
	{
		int len = PyUnicode_GetSize(folderObj);
		wchar_t* buffer = TRACKED_NEW wchar_t[len + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)folderObj, buffer, len);
		buffer[len] = L'\0';
		bool retVal = cyMisc::DumpLogs(buffer);
		delete [] buffer;
		PYTHON_RETURN_BOOL(retVal);
	}
	else if (PyString_Check(folderObj))
	{
		char* temp = PyString_AsString(folderObj);
		wchar_t* wTemp = hsStringToWString(temp);
		bool retVal = cyMisc::DumpLogs(wTemp);
		delete [] wTemp;
		PYTHON_RETURN_BOOL(retVal);
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "PtDumpLogs expects a unicode or standard string");
		PYTHON_RETURN_ERROR;
	}
}

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaMethods - the python method definitions
//

void cyMisc::AddPlasmaMethods(std::vector<PyMethodDef> &methods)
{
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetAgeName);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetAgeInfo);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetAgeTime);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetPrevAgeName);	
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetPrevAgeInfo);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetDniTime);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetServerTime);
	PYTHON_GLOBAL_METHOD(methods, PtGMTtoDniTime);
	
	PYTHON_GLOBAL_METHOD(methods, PtGetClientName);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetLocalAvatar);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetLocalPlayer);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetPlayerList);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetPlayerListDistanceSorted);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtMaxListenListSize);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtMaxListenDistSq);
	PYTHON_GLOBAL_METHOD(methods, PtGetAvatarKeyFromClientID);
	PYTHON_GLOBAL_METHOD(methods, PtGetClientIDFromAvatarKey);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetNumRemotePlayers);

	PYTHON_GLOBAL_METHOD(methods, PtValidateKey);

	PYTHON_GLOBAL_METHOD(methods, PtSendRTChat);
	PYTHON_GLOBAL_METHOD(methods, PtSendKIMessage);
	PYTHON_GLOBAL_METHOD(methods, PtSendKIMessageInt);
	
	PYTHON_GLOBAL_METHOD(methods, PtLoadAvatarModel);
	PYTHON_GLOBAL_METHOD(methods, PtUnLoadAvatarModel);

	PYTHON_BASIC_GLOBAL_METHOD(methods, PtForceCursorHidden);
	PYTHON_BASIC_GLOBAL_METHOD(methods, PtForceCursorShown);

	PYTHON_GLOBAL_METHOD(methods, PtGetLocalizedString);

	PYTHON_GLOBAL_METHOD(methods, PtDumpLogs);

	AddPlasmaMethods2(methods);
	AddPlasmaMethods3(methods);
	AddPlasmaMethods4(methods);
}