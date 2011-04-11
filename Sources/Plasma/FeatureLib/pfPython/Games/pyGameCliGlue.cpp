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
#include "pyGameCli.h"
#include "../pyEnum.h"

#include <python.h>

///////////////////////////////////////////////////////////////////////////////
//
// Base game client class
//

PYTHON_CLASS_DEFINITION(ptGameCli, pyGameCli);

PYTHON_DEFAULT_NEW_DEFINITION(ptGameCli, pyGameCli)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGameCli)

PYTHON_NO_INIT_DEFINITION(ptGameCli)

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetGameIDs, "Returns a list of game IDs that the player is currently joined to")
{
	std::vector<unsigned> ids = pyGameCli::GetGameIDs();
	PyObject* retVal = PyList_New(ids.size());
	for (unsigned i = 0; i < ids.size(); ++i)
		PyList_SetItem(retVal, i, PyInt_FromLong(ids[i]));
	return retVal;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtGetGameCli, args, "Params: gameID\nReturns a ptGameCli associated with the specified id")
{
	int gameID = 0;
	if (!PyArg_ParseTuple(args, "i", &gameID))
	{
		PyErr_SetString(PyExc_TypeError, "PtGetGameCli expects an integer");
		PYTHON_RETURN_ERROR;
	}
	return pyGameCli::GetGameCli(gameID);
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtGetGameNameByTypeID, args, "Params: guid\nReturns the name of the game represented by guid passed in as a string")
{
	PyObject* textObj;
	if (!PyArg_ParseTuple(args, "O", &textObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtGetGameNameByTypeID expects a unicode string");
		PYTHON_RETURN_ERROR;
	}
	if (PyUnicode_Check(textObj))
	{
		int strLen = PyUnicode_GetSize(textObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)textObj, text, strLen);
		text[strLen] = L'\0';
		std::wstring retVal = pyGameCli::GetGameNameByTypeID(text);
		delete [] text;
		return PyUnicode_FromWideChar(retVal.c_str(), retVal.length());
	}
	else if (PyString_Check(textObj))
	{
		// we'll allow this, just in case something goes weird
		char* text = PyString_AsString(textObj);
		wchar_t* wText = hsStringToWString(text);
		std::wstring retVal = pyGameCli::GetGameNameByTypeID(wText);
		delete [] wText;
		return PyUnicode_FromWideChar(retVal.c_str(), retVal.length());
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "PtGetGameNameByTypeID expects a unicode string");
		PYTHON_RETURN_ERROR;
	}
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtJoinGame, args, "Params: callbackKey, gameID\nSends a join request to the specified game. Messages are sent to the callback key")
{
	PyObject* callbackObj = NULL;
	int gameID = 0;
	if (!PyArg_ParseTuple(args, "Oi", &callbackObj, &gameID))
	{
		PyErr_SetString(PyExc_TypeError, "PtJoinGame expects a ptKey and an integer");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(callbackObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtJoinGame expects a ptKey and an integer");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(callbackObj);
	pyGameCli::JoinGame(*key, gameID);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameCli, gameID)
{
	return PyInt_FromLong(self->fThis->GameID());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameCli, gameTypeID)
{
	std::wstring retVal = self->fThis->GameTypeID();
	return PyUnicode_FromWideChar(retVal.c_str(), retVal.length());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameCli, name)
{
	std::wstring retVal = self->fThis->Name();
	return PyUnicode_FromWideChar(retVal.c_str(), retVal.length());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameCli, playerCount)
{
	return PyInt_FromLong(self->fThis->PlayerCount());
}

PYTHON_METHOD_DEFINITION(ptGameCli, invitePlayer, args)
{
	int playerID = 0;
	if (!PyArg_ParseTuple(args, "i", &playerID))
	{
		PyErr_SetString(PyExc_TypeError, "invitePlayer expects an unsigned int");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->InvitePlayer(playerID);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGameCli, uninvitePlayer, args)
{
	int playerID = 0;
	if (!PyArg_ParseTuple(args, "i", &playerID))
	{
		PyErr_SetString(PyExc_TypeError, "uninvitePlayer expects an unsigned int");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->UninvitePlayer(playerID);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptGameCli, leaveGame, LeaveGame)

PYTHON_METHOD_DEFINITION_NOARGS(ptGameCli, upcastToTTTGame)
{
	return self->fThis->UpcastToTTTGame();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameCli, upcastToHeekGame)
{
	return self->fThis->UpcastToHeekGame();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameCli, upcastToMarkerGame)
{
	return self->fThis->UpcastToMarkerGame();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameCli, upcastToBlueSpiralGame)
{
	return self->fThis->UpcastToBlueSpiralGame();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameCli, upcastToClimbingWallGame)
{
	return self->fThis->UpcastToClimbingWallGame();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameCli, upcastToVarSyncGame)
{
	return self->fThis->UpcastToVarSyncGame();
}

PYTHON_START_METHODS_TABLE(ptGameCli)
	PYTHON_METHOD_NOARGS(ptGameCli, gameID, "Returns the ID number for this game"),
	PYTHON_METHOD_NOARGS(ptGameCli, gameTypeID, "Returns the game type ID for this game (as a guid string)"),
	PYTHON_METHOD_NOARGS(ptGameCli, name, "Returns the name of the game"),
	PYTHON_METHOD_NOARGS(ptGameCli, playerCount, "Returns the current number of players"),
	PYTHON_METHOD(ptGameCli, invitePlayer, "Params: playerID\nInvites the specified player to join the game"),
	PYTHON_METHOD(ptGameCli, uninvitePlayer, "Params: playerID\nRevokes the invitation for the specified player"),
	PYTHON_BASIC_METHOD(ptGameCli, leaveGame, "Leaves this game"),
	PYTHON_METHOD_NOARGS(ptGameCli, upcastToTTTGame, "Returns this game client as a ptTTTGame"),
	PYTHON_METHOD_NOARGS(ptGameCli, upcastToHeekGame, "Returns this game client as a ptHeekGame"),
	PYTHON_METHOD_NOARGS(ptGameCli, upcastToMarkerGame, "Returns this game client as a ptMarkerGame"),
	PYTHON_METHOD_NOARGS(ptGameCli, upcastToBlueSpiralGame, "Returns this game client as a ptBlueSpiralGame"),
	PYTHON_METHOD_NOARGS(ptGameCli, upcastToClimbingWallGame, "Returns this game client as a ptClimbingWallGame"),
	PYTHON_METHOD_NOARGS(ptGameCli, upcastToVarSyncGame, "Returns this game client as a ptVarSyncGame"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptGameCli, "Base class for all game client interfaces");
PYTHON_EXPOSE_TYPE_DEFINITION(ptGameCli, pyGameCli);

// required functions for PyObject interoperability
PyObject* pyGameCli::New(pfGameCli* client)
{
	ptGameCli *newObj = (ptGameCli*)ptGameCli_type.tp_new(&ptGameCli_type, NULL, NULL);
	newObj->fThis->gameClient = client;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGameCli, pyGameCli)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGameCli, pyGameCli)

// Module and method definitions
void pyGameCli::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptGameCli);
	PYTHON_CLASS_IMPORT_END(m);
}

void pyGameCli::AddPlasmaMethods(std::vector<PyMethodDef>& methods)
{
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetGameIDs);
	PYTHON_GLOBAL_METHOD(methods, PtGetGameCli);
	PYTHON_GLOBAL_METHOD(methods, PtGetGameNameByTypeID);
	PYTHON_GLOBAL_METHOD(methods, PtJoinGame);
}