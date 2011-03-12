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
#include "pyHeekGame.h"

#include <python.h>
#include "../../pyEnum.h"

///////////////////////////////////////////////////////////////////////////////
//
// Base Heek game client class
//

PYTHON_CLASS_DEFINITION(ptHeekGame, pyHeekGame);

PYTHON_DEFAULT_NEW_DEFINITION(ptHeekGame, pyHeekGame)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptHeekGame)

PYTHON_NO_INIT_DEFINITION(ptHeekGame)

PYTHON_GLOBAL_METHOD_DEFINITION(PtIsHeekGame, args, "Params: typeID\nReturns true if the specifed typeID (guid as a string) is a Heek game")
{
	PyObject* textObj;
	if (!PyArg_ParseTuple(args, "O", &textObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtIsHeekGame expects a unicode string");
		PYTHON_RETURN_ERROR;
	}
	if (PyUnicode_Check(textObj))
	{
		int strLen = PyUnicode_GetSize(textObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)textObj, text, strLen);
		text[strLen] = L'\0';
		bool retVal = pyHeekGame::IsHeekGame(text);
		delete [] text;
		PYTHON_RETURN_BOOL(retVal);
	}
	else if (PyString_Check(textObj))
	{
		// we'll allow this, just in case something goes weird
		char* text = PyString_AsString(textObj);
		wchar_t* wText = hsStringToWString(text);
		bool retVal = pyHeekGame::IsHeekGame(wText);
		delete [] wText;
		PYTHON_RETURN_BOOL(retVal);
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "PtIsHeekGame expects a unicode string");
		PYTHON_RETURN_ERROR;
	}
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtJoinCommonHeekGame, args, "Params: callbackKey, gameID\nJoins a common Heek game with the specified ID. If one doesn't exist, it creates it")
{
	PyObject* callbackObj = NULL;
	int gameID = 0;
	if (!PyArg_ParseTuple(args, "Oi", &callbackObj, &gameID))
	{
		PyErr_SetString(PyExc_TypeError, "PtJoinCommonHeekGame expects a ptKey and an integer");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(callbackObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtJoinCommonHeekGame expects a ptKey and an integer");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(callbackObj);
	pyHeekGame::JoinCommonHeekGame(*key, gameID);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptHeekGame, playGame, args)
{
	int position = 0;
	long points = 0;
	PyObject* textObj = NULL;
	if (!PyArg_ParseTuple(args,"ilO", &position, &points, &textObj))
	{
		PyErr_SetString(PyExc_TypeError, "playGame expects an int, a long, and a unicode string");
		PYTHON_RETURN_ERROR;
	}
	if (PyUnicode_Check(textObj))
	{
		int strLen = PyUnicode_GetSize(textObj);
		wchar_t* temp = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)textObj, temp, strLen);
		temp[strLen] = L'\0';
		self->fThis->PlayGame(position, points, temp);
		delete [] temp;
		PYTHON_RETURN_NONE;
	}
	else if (PyString_Check(textObj))
	{
		// we'll allow this, just in case something goes weird
		char* temp = PyString_AsString(textObj);
		wchar_t* wTemp = hsStringToWString(temp);
		self->fThis->PlayGame(position, points, wTemp);
		delete [] wTemp;
		PYTHON_RETURN_NONE;
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "playGame expects an int, a long, and a unicode string");
		PYTHON_RETURN_ERROR;
	}
}

PYTHON_BASIC_METHOD_DEFINITION(ptHeekGame, leaveGame, LeaveGame)

PYTHON_METHOD_DEFINITION(ptHeekGame, choose, args)
{
	int choice = 0;
	if (!PyArg_ParseTuple(args, "i", &choice))
	{
		PyErr_SetString(PyExc_TypeError, "choose expects an int");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Choose(choice);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptHeekGame, sequenceFinished, args)
{
	int seq = 0;
	if (!PyArg_ParseTuple(args, "i", &seq))
	{
		PyErr_SetString(PyExc_TypeError, "sequenceFinished expects an int");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SequenceFinished(seq);
	PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptHeekGame)
	PYTHON_METHOD(ptHeekGame, playGame, "Params: position, points, name\nRequests to start playing the game in the specified position"),
	PYTHON_BASIC_METHOD(ptHeekGame, leaveGame, "Leaves this game (puts us into \"observer\" mode"),
	PYTHON_METHOD(ptHeekGame, choose, "Params: choice\nMakes the specified move (see PtHeekGameChoice)"),
	PYTHON_METHOD(ptHeekGame, sequenceFinished, "Params: sequence\nTells the server that the specified animation finished (see PtHeekGameSeq)"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptHeekGame, pyGameCli, "Game client for the Heek game");

// required functions for PyObject interoperability
PyObject* pyHeekGame::New(pfGameCli* client)
{
	ptHeekGame *newObj = (ptHeekGame*)ptHeekGame_type.tp_new(&ptHeekGame_type, NULL, NULL);
	if (client && (client->GetGameTypeId() == kGameTypeId_Heek))
		newObj->fThis->gameClient = client;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptHeekGame, pyHeekGame)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptHeekGame, pyHeekGame)

// Module and method definitions
void pyHeekGame::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptHeekGame);
	PYTHON_CLASS_IMPORT_END(m);
}

void pyHeekGame::AddPlasmaConstantsClasses(PyObject* m)
{
	PYTHON_ENUM_START(PtHeekGameChoice);
	PYTHON_ENUM_ELEMENT(PtHeekGameChoice, kHeekGameChoiceRock, kHeekRock);
	PYTHON_ENUM_ELEMENT(PtHeekGameChoice, kHeekGameChoicePaper, kHeekPaper);
	PYTHON_ENUM_ELEMENT(PtHeekGameChoice, kHeekGameChoiceScissors, kHeekScissors);
	PYTHON_ENUM_END(m, PtHeekGameChoice);

	PYTHON_ENUM_START(PtHeekGameSeq);
	PYTHON_ENUM_ELEMENT(PtHeekGameSeq, kHeekGameSeqCountdown, kHeekCountdownSeq);
	PYTHON_ENUM_ELEMENT(PtHeekGameSeq, kHeekGameSeqChoiceAnim, kHeekChoiceAnimSeq);
	PYTHON_ENUM_ELEMENT(PtHeekGameSeq, kHeekGameSeqGameWinAnim, kHeekGameWinAnimSeq);
	PYTHON_ENUM_END(m, PtHeekGameSeq);
}

void pyHeekGame::AddPlasmaMethods(std::vector<PyMethodDef>& methods)
{
	PYTHON_GLOBAL_METHOD(methods, PtIsHeekGame);
	PYTHON_GLOBAL_METHOD(methods, PtJoinCommonHeekGame);
}