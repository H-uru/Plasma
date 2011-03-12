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
#include "pyBlueSpiralGame.h"

#include <python.h>
#include "../../pyEnum.h"

///////////////////////////////////////////////////////////////////////////////
//
// Base BlueSpiral game client class
//

PYTHON_CLASS_DEFINITION(ptBlueSpiralGame, pyBlueSpiralGame);

PYTHON_DEFAULT_NEW_DEFINITION(ptBlueSpiralGame, pyBlueSpiralGame)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptBlueSpiralGame)

PYTHON_NO_INIT_DEFINITION(ptBlueSpiralGame)

PYTHON_GLOBAL_METHOD_DEFINITION(PtIsBlueSpiralGame, args, "Params: typeID\nReturns true if the specifed typeID (guid as a string) is a BlueSpiral game")
{
	PyObject* textObj;
	if (!PyArg_ParseTuple(args, "O", &textObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtIsBlueSpiralGame expects a unicode string");
		PYTHON_RETURN_ERROR;
	}
	if (PyUnicode_Check(textObj))
	{
		int strLen = PyUnicode_GetSize(textObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)textObj, text, strLen);
		text[strLen] = L'\0';
		bool retVal = pyBlueSpiralGame::IsBlueSpiralGame(text);
		delete [] text;
		PYTHON_RETURN_BOOL(retVal);
	}
	else if (PyString_Check(textObj))
	{
		// we'll allow this, just in case something goes weird
		char* text = PyString_AsString(textObj);
		wchar_t* wText = hsStringToWString(text);
		bool retVal = pyBlueSpiralGame::IsBlueSpiralGame(wText);
		delete [] wText;
		PYTHON_RETURN_BOOL(retVal);
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "PtIsBlueSpiralGame expects a unicode string");
		PYTHON_RETURN_ERROR;
	}
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtJoinCommonBlueSpiralGame, args, "Params: callbackKey, gameID\nJoins a common BlueSpiral game with the specified ID. If one doesn't exist, it creates it")
{
	PyObject* callbackObj = NULL;
	int gameID = 0;
	if (!PyArg_ParseTuple(args, "Oi", &callbackObj, &gameID))
	{
		PyErr_SetString(PyExc_TypeError, "PtJoinCommonBlueSpiralGame expects a ptKey and an integer");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(callbackObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtJoinCommonBlueSpiralGame expects a ptKey and an integer");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(callbackObj);
	pyBlueSpiralGame::JoinCommonBlueSpiralGame(*key, gameID);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptBlueSpiralGame, startGame, StartGame)

PYTHON_METHOD_DEFINITION(ptBlueSpiralGame, hitCloth, args)
{
	int clothNum = 0;
	if (!PyArg_ParseTuple(args, "i", &clothNum))
	{
		PyErr_SetString(PyExc_TypeError, "hitCloth expects one integer");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->HitCloth(clothNum);
	PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptBlueSpiralGame)
	PYTHON_BASIC_METHOD(ptBlueSpiralGame, startGame, "Starts a new game"),
	PYTHON_METHOD(ptBlueSpiralGame, hitCloth, "Params: clothNum\nTells the server you hit the specified cloth"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptBlueSpiralGame, pyGameCli, "Game client for the BlueSpiral game");

// required functions for PyObject interoperability
PyObject* pyBlueSpiralGame::New(pfGameCli* client)
{
	ptBlueSpiralGame *newObj = (ptBlueSpiralGame*)ptBlueSpiralGame_type.tp_new(&ptBlueSpiralGame_type, NULL, NULL);
	if (client && (client->GetGameTypeId() == kGameTypeId_BlueSpiral))
		newObj->fThis->gameClient = client;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptBlueSpiralGame, pyBlueSpiralGame)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptBlueSpiralGame, pyBlueSpiralGame)

// Module and method definitions
void pyBlueSpiralGame::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptBlueSpiralGame);
	PYTHON_CLASS_IMPORT_END(m);
}

void pyBlueSpiralGame::AddPlasmaMethods(std::vector<PyMethodDef>& methods)
{
	PYTHON_GLOBAL_METHOD(methods, PtIsBlueSpiralGame);
	PYTHON_GLOBAL_METHOD(methods, PtJoinCommonBlueSpiralGame);
}