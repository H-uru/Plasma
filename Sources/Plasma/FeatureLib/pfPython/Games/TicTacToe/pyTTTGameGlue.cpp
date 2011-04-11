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
#include "pyTTTGame.h"

#include <python.h>
#include "../../pyEnum.h"

///////////////////////////////////////////////////////////////////////////////
//
// Base TTT game client class
//

PYTHON_CLASS_DEFINITION(ptTTTGame, pyTTTGame);

PYTHON_DEFAULT_NEW_DEFINITION(ptTTTGame, pyTTTGame)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptTTTGame)

PYTHON_NO_INIT_DEFINITION(ptTTTGame)

PYTHON_GLOBAL_METHOD_DEFINITION(PtIsTTTGame, args, "Params: typeID\nReturns true if the specifed typeID (guid as a string) is a TicTacToe game")
{
	PyObject* textObj;
	if (!PyArg_ParseTuple(args, "O", &textObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtIsTTTGame expects a unicode string");
		PYTHON_RETURN_ERROR;
	}
	if (PyUnicode_Check(textObj))
	{
		int strLen = PyUnicode_GetSize(textObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)textObj, text, strLen);
		text[strLen] = L'\0';
		bool retVal = pyTTTGame::IsTTTGame(text);
		delete [] text;
		PYTHON_RETURN_BOOL(retVal);
	}
	else if (PyString_Check(textObj))
	{
		// we'll allow this, just in case something goes weird
		char* text = PyString_AsString(textObj);
		wchar_t* wText = hsStringToWString(text);
		bool retVal = pyTTTGame::IsTTTGame(wText);
		delete [] wText;
		PYTHON_RETURN_BOOL(retVal);
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "PtIsTTTGame expects a unicode string");
		PYTHON_RETURN_ERROR;
	}
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtCreateTTTGame, args, "Params: callbackKey, numPlayers\nCreates a new TicTacToe game with the specified callback key and number of players (1 or 2)")
{
	PyObject* callbackObj = NULL;
	int numPlayers = 0;
	if (!PyArg_ParseTuple(args, "Oi", &callbackObj, &numPlayers))
	{
		PyErr_SetString(PyExc_TypeError, "PtCreateTTTGame expects a ptKey and an integer");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(callbackObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtCreateTTTGame expects a ptKey and an integer");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(callbackObj);
	pyTTTGame::CreateTTTGame(*key, numPlayers);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtJoinCommonTTTGame, args, "Params: callbackKey, gameID, numPlayers\nJoins a common TicTacToe game with the specified ID. If one doesn't exist, it creates it with the specified number of players")
{
	PyObject* callbackObj = NULL;
	int gameID = 0, numPlayers = 0;
	if (!PyArg_ParseTuple(args, "Oii", &callbackObj, &gameID, &numPlayers))
	{
		PyErr_SetString(PyExc_TypeError, "PtJoinCommonTTTGame expects a ptKey and two integers");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(callbackObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtJoinCommonTTTGame expects a ptKey and two integers");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(callbackObj);
	pyTTTGame::JoinCommonTTTGame(*key, gameID, numPlayers);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptTTTGame, makeMove, args)
{
	int row = 0, col = 0;
	if (!PyArg_ParseTuple(args, "ii", &row, &col))
	{
		PyErr_SetString(PyExc_TypeError, "makeMove expects two integers");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->MakeMove(row, col);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptTTTGame, showBoard, ShowBoard)

PYTHON_START_METHODS_TABLE(ptTTTGame)
	PYTHON_METHOD(ptTTTGame, makeMove, "Params: row, col\nMakes a move in the specified spot"),
	PYTHON_BASIC_METHOD(ptTTTGame, showBoard, "Prints the current board layout to the console"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptTTTGame, pyGameCli, "Game client for the TicTacToe game");

// required functions for PyObject interoperability
PyObject* pyTTTGame::New(pfGameCli* client)
{
	ptTTTGame *newObj = (ptTTTGame*)ptTTTGame_type.tp_new(&ptTTTGame_type, NULL, NULL);
	if (client && (client->GetGameTypeId() == kGameTypeId_TicTacToe))
		newObj->fThis->gameClient = client;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptTTTGame, pyTTTGame)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptTTTGame, pyTTTGame)

// Module and method definitions
void pyTTTGame::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptTTTGame);
	PYTHON_CLASS_IMPORT_END(m);
}

void pyTTTGame::AddPlasmaConstantsClasses(PyObject* m)
{
	PYTHON_ENUM_START(PtTTTGameResult);
	PYTHON_ENUM_ELEMENT(PtTTTGameResult, kTTTGameResultWinner, kTTTGameResultWinner);
	PYTHON_ENUM_ELEMENT(PtTTTGameResult, kTTTGameResultTied, kTTTGameResultTied);
	PYTHON_ENUM_ELEMENT(PtTTTGameResult, kTTTGameResultGave, kTTTGameResultGave);
	PYTHON_ENUM_ELEMENT(PtTTTGameResult, kTTTGameResultError, kTTTGameResultError);
	PYTHON_ENUM_END(m, PtTTTGameResult);
}

void pyTTTGame::AddPlasmaMethods(std::vector<PyMethodDef>& methods)
{
	PYTHON_GLOBAL_METHOD(methods, PtIsTTTGame);
	PYTHON_GLOBAL_METHOD(methods, PtCreateTTTGame);
	PYTHON_GLOBAL_METHOD(methods, PtJoinCommonTTTGame);
}