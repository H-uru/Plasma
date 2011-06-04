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
#include "pyClimbingWallGame.h"

#include <python.h>
#include "../../pyEnum.h"

///////////////////////////////////////////////////////////////////////////////
//
// Base climbing wall game client class
//

PYTHON_CLASS_DEFINITION(ptClimbingWallGame, pyClimbingWallGame);

PYTHON_DEFAULT_NEW_DEFINITION(ptClimbingWallGame, pyClimbingWallGame)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptClimbingWallGame)

PYTHON_NO_INIT_DEFINITION(ptClimbingWallGame)

PYTHON_GLOBAL_METHOD_DEFINITION(PtIsClimbingWallGame, args, "Params: typeID\nReturns true if the specifed typeID (guid as a string) is a ClimbingWall game")
{
	PyObject* textObj;
	if (!PyArg_ParseTuple(args, "O", &textObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtIsClimbingWallGame expects a unicode string");
		PYTHON_RETURN_ERROR;
	}
	if (PyUnicode_Check(textObj))
	{
		int strLen = PyUnicode_GetSize(textObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)textObj, text, strLen);
		text[strLen] = L'\0';
		bool retVal = pyClimbingWallGame::IsClimbingWallGame(text);
		delete [] text;
		PYTHON_RETURN_BOOL(retVal);
	}
	else if (PyString_Check(textObj))
	{
		// we'll allow this, just in case something goes weird
		char* text = PyString_AsString(textObj);
		wchar_t* wText = hsStringToWString(text);
		bool retVal = pyClimbingWallGame::IsClimbingWallGame(wText);
		delete [] wText;
		PYTHON_RETURN_BOOL(retVal);
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "PtIsClimbingWallGame expects a unicode string");
		PYTHON_RETURN_ERROR;
	}
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtJoinCommonClimbingWallGame, args, "Params: callbackKey, gameID\nJoins a common ClimbingWall game with the specified ID. If one doesn't exist, it creates it")
{
	PyObject* callbackObj = NULL;
	int gameID = 0;
	if (!PyArg_ParseTuple(args, "Oii", &callbackObj, &gameID))
	{
		PyErr_SetString(PyExc_TypeError, "PtJoinCommonClimbingWallGame expects a ptKey and an integer");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(callbackObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtJoinCommonClimbingWallGame expects a ptKey and an integer");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(callbackObj);
	pyClimbingWallGame::JoinCommonClimbingWallGame(*key, gameID);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptClimbingWallGame, changeNumBlockers, args)
{
	int amountToAdjust;
	if (!PyArg_ParseTuple(args, "i", &amountToAdjust))
	{
		PyErr_SetString(PyExc_TypeError, "changeNumBlockers expects an integer");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->ChangeNumBlockers(amountToAdjust);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptClimbingWallGame, ready, args)
{
	int readyType, teamNumber;
	if (!PyArg_ParseTuple(args, "ii", &readyType, &teamNumber))
	{
		PyErr_SetString(PyExc_TypeError, "ready expects two integers");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Ready((unsigned)readyType, (unsigned)teamNumber);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptClimbingWallGame, changeBlocker, args)
{
	int teamNumber, blockerNumber;
	char added;
	if (!PyArg_ParseTuple(args, "iib", &teamNumber, &blockerNumber, &added))
	{
		PyErr_SetString(PyExc_TypeError, "changeBlocker expects two integers and a boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->ChangeBlocker(teamNumber, blockerNumber, added != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptClimbingWallGame, reset, Reset)

PYTHON_METHOD_DEFINITION(ptClimbingWallGame, playerEntered, args)
{
	int teamNumber;
	if (!PyArg_ParseTuple(args, "i", &teamNumber))
	{
		PyErr_SetString(PyExc_TypeError, "playerEntered expects an integer");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->PlayerEntered(teamNumber);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptClimbingWallGame, finishedGame, FinishedGame)

PYTHON_BASIC_METHOD_DEFINITION(ptClimbingWallGame, panic, Panic)

PYTHON_START_METHODS_TABLE(ptClimbingWallGame)
	PYTHON_METHOD(ptClimbingWallGame, changeNumBlockers, "Params: amountToAdjust\nAdjusts the number of blockers we are playing with"),
	PYTHON_METHOD(ptClimbingWallGame, ready, "Params: readyType, teamNumber\nMarks the specified team as ready for the specified type (See PtClimbingWallReadyTypes)"),
	PYTHON_METHOD(ptClimbingWallGame, changeBlocker, "Params: teamNumber, blockerNumber, added\nChanges the specified marker's state for the specified team"),
	PYTHON_BASIC_METHOD(ptClimbingWallGame, reset, "Attempts to reset the game's control panel"),
	PYTHON_METHOD(ptClimbingWallGame, playerEntered, "Params: teamNumber\nTells the server that you are trying to play the game for the specified team"),
	PYTHON_BASIC_METHOD(ptClimbingWallGame, finishedGame, "Tells the server you reached the top of the wall"),
	PYTHON_BASIC_METHOD(ptClimbingWallGame, panic, "Tells the server you are panicking and want your blockers reset"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptClimbingWallGame, pyGameCli, "Game client for the ClimbingWall game");

// required functions for PyObject interoperability
PyObject* pyClimbingWallGame::New(pfGameCli* client)
{
	ptClimbingWallGame *newObj = (ptClimbingWallGame*)ptClimbingWallGame_type.tp_new(&ptClimbingWallGame_type, NULL, NULL);
	if (client && (client->GetGameTypeId() == kGameTypeId_ClimbingWall))
		newObj->fThis->gameClient = client;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptClimbingWallGame, pyClimbingWallGame)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptClimbingWallGame, pyClimbingWallGame)

// Module and method definitions
void pyClimbingWallGame::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptClimbingWallGame);
	PYTHON_CLASS_IMPORT_END(m);
}

void pyClimbingWallGame::AddPlasmaMethods(std::vector<PyMethodDef>& methods)
{
	PYTHON_GLOBAL_METHOD(methods, PtIsClimbingWallGame);
	PYTHON_GLOBAL_METHOD(methods, PtJoinCommonClimbingWallGame);
}

void pyClimbingWallGame::AddPlasmaConstantsClasses(PyObject* m)
{
	PYTHON_ENUM_START(PtClimbingWallReadyTypes);
	PYTHON_ENUM_ELEMENT(PtClimbingWallReadyTypes, kClimbingWallReadyNumBlockers, kClimbingWallReadyNumBlockers);
	PYTHON_ENUM_ELEMENT(PtClimbingWallReadyTypes, kClimbingWallReadyBlockers, kClimbingWallReadyBlockers);
	PYTHON_ENUM_END(m, PtClimbingWallReadyTypes);
}