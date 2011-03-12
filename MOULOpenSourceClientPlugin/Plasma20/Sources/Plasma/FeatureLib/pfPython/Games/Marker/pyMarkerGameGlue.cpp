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
#include "pyMarkerGame.h"

#include <python.h>
#include "../../pyEnum.h"

///////////////////////////////////////////////////////////////////////////////
//
// Base TTT game client class
//

PYTHON_CLASS_DEFINITION(ptMarkerGame, pyMarkerGame);

PYTHON_DEFAULT_NEW_DEFINITION(ptMarkerGame, pyMarkerGame)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptMarkerGame)

PYTHON_NO_INIT_DEFINITION(ptMarkerGame)

PYTHON_GLOBAL_METHOD_DEFINITION(PtIsMarkerGame, args, "Params: typeID\nReturns true if the specifed typeID (guid as a string) is a Marker game")
{
	PyObject* textObj;
	if (!PyArg_ParseTuple(args, "O", &textObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtIsMarkerGame expects a unicode string");
		PYTHON_RETURN_ERROR;
	}
	if (PyUnicode_Check(textObj))
	{
		int strLen = PyUnicode_GetSize(textObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)textObj, text, strLen);
		text[strLen] = L'\0';
		bool retVal = pyMarkerGame::IsMarkerGame(text);
		delete [] text;
		PYTHON_RETURN_BOOL(retVal);
	}
	else if (PyString_Check(textObj))
	{
		// we'll allow this, just in case something goes weird
		char* text = PyString_AsString(textObj);
		wchar_t* wText = hsStringToWString(text);
		bool retVal = pyMarkerGame::IsMarkerGame(wText);
		delete [] wText;
		PYTHON_RETURN_BOOL(retVal);
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "PtIsMarkerGame expects a unicode string");
		PYTHON_RETURN_ERROR;
	}
}

PYTHON_GLOBAL_METHOD_DEFINITION_WKEY(PtCreateMarkerGame, args, keywords, "Params: callbackKey, gameType, gameName = \"\", timeLimit = 0, templateId = \"\"\n"
	"Creates a new Marker game with the specified callback key, game type (from PtMarkerGameTypes), time limit (in ms), and template id (guid string)")
{
	char *kwlist[] = {"callbackKey", "gameType", "gameName", "timeLimit", "templateId", NULL};
	PyObject* callbackObj = NULL;
	unsigned int gameType = 0;
	PyObject* gameNameObj = NULL;
	unsigned long timeLimit = 0;
	PyObject* templateIdObj = NULL;
	if (!PyArg_ParseTupleAndKeywords(args, keywords, "OI|OkO", kwlist, &callbackObj, &gameType, &gameNameObj, &timeLimit, &templateIdObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtCreateMarkerGame expects a ptKey, unsigned int, and optionally a string, an unsigned long, and another string");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(callbackObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtCreateMarkerGame expects a ptKey, unsigned int, and optionally a string, an unsigned long, and another string");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(callbackObj);
	std::wstring name = L"";
	std::wstring templateId = L"";
	if (gameNameObj != NULL)
	{
		if (PyUnicode_Check(gameNameObj))
		{
			int strLen = PyUnicode_GetSize(gameNameObj);
			wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
			PyUnicode_AsWideChar((PyUnicodeObject*)gameNameObj, text, strLen);
			text[strLen] = L'\0';
			name = text;
			delete [] text;
		}
		else if (PyString_Check(gameNameObj))
		{
			// we'll allow this, just in case something goes weird
			char* text = PyString_AsString(gameNameObj);
			wchar_t* wText = hsStringToWString(text);
			name = wText;
			delete [] wText;
		}
		else
		{
			PyErr_SetString(PyExc_TypeError, "PtCreateMarkerGame expects a ptKey, unsigned int, and optionally a string, an unsigned long, and another string");
			PYTHON_RETURN_ERROR;
		}
	}
	if (templateIdObj != NULL)
	{
		if (PyUnicode_Check(templateIdObj))
		{
			int strLen = PyUnicode_GetSize(templateIdObj);
			wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
			PyUnicode_AsWideChar((PyUnicodeObject*)templateIdObj, text, strLen);
			text[strLen] = L'\0';
			templateId = text;
			delete [] text;
		}
		else if (PyString_Check(templateIdObj))
		{
			// we'll allow this, just in case something goes weird
			char* text = PyString_AsString(templateIdObj);
			wchar_t* wText = hsStringToWString(text);
			templateId = wText;
			delete [] wText;
		}
		else
		{
			PyErr_SetString(PyExc_TypeError, "PtCreateMarkerGame expects a ptKey, unsigned int, and optionally a string, an unsigned long, and another string");
			PYTHON_RETURN_ERROR;
		}
	}
	pyMarkerGame::CreateMarkerGame(*key, gameType, name, timeLimit, templateId);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptMarkerGame, startGame, StartGame)
PYTHON_BASIC_METHOD_DEFINITION(ptMarkerGame, pauseGame, PauseGame)
PYTHON_BASIC_METHOD_DEFINITION(ptMarkerGame, resetGame, ResetGame)

PYTHON_METHOD_DEFINITION(ptMarkerGame, changeGameName, args)
{
	PyObject* textObj;
	if (!PyArg_ParseTuple(args, "O", &textObj))
	{
		PyErr_SetString(PyExc_TypeError, "changeGameName expects a unicode string");
		PYTHON_RETURN_ERROR;
	}
	if (PyUnicode_Check(textObj))
	{
		int strLen = PyUnicode_GetSize(textObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)textObj, text, strLen);
		text[strLen] = L'\0';
		self->fThis->ChangeGameName(text);
		delete [] text;
		PYTHON_RETURN_NONE;
	}
	else if (PyString_Check(textObj))
	{
		// we'll allow this, just in case something goes weird
		char* text = PyString_AsString(textObj);
		wchar_t* wText = hsStringToWString(text);
		self->fThis->ChangeGameName(wText);
		delete [] wText;
		PYTHON_RETURN_NONE;
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "changeGameName expects a unicode string");
		PYTHON_RETURN_ERROR;
	}
}

PYTHON_METHOD_DEFINITION(ptMarkerGame, changeTimeLimit, args)
{
	unsigned long timeLimit;
	if (!PyArg_ParseTuple(args, "k", &timeLimit))
	{
		PyErr_SetString(PyExc_TypeError, "changeTimeLimit expects an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->ChangeTimeLimit(timeLimit);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptMarkerGame, deleteGame, DeleteGame)

PYTHON_METHOD_DEFINITION_WKEY(ptMarkerGame, addMarker, args, keywords)
{
	char *kwlist[] = {"x", "y", "z", "name", "age", NULL};
	double x, y, z;
	PyObject* nameObj = NULL;
	PyObject* ageObj = NULL;
	if (!PyArg_ParseTupleAndKeywords(args, keywords, "ddd|OO", kwlist, &x, &y, &z, &nameObj, &ageObj))
	{
		PyErr_SetString(PyExc_TypeError, "addMarker expects three doubles, and optionally two strings");
		PYTHON_RETURN_ERROR;
	}
	std::wstring name = L"";
	std::wstring age = L"";
	if (nameObj != NULL)
	{
		if (PyUnicode_Check(nameObj))
		{
			int strLen = PyUnicode_GetSize(nameObj);
			wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
			PyUnicode_AsWideChar((PyUnicodeObject*)nameObj, text, strLen);
			text[strLen] = L'\0';
			name = text;
			delete [] text;
		}
		else if (PyString_Check(nameObj))
		{
			// we'll allow this, just in case something goes weird
			char* text = PyString_AsString(nameObj);
			wchar_t* wText = hsStringToWString(text);
			name = wText;
			delete [] wText;
		}
		else
		{
			PyErr_SetString(PyExc_TypeError, "addMarker expects three doubles, and optionally two strings");
			PYTHON_RETURN_ERROR;
		}
	}
	if (ageObj != NULL)
	{
		if (PyUnicode_Check(ageObj))
		{
			int strLen = PyUnicode_GetSize(ageObj);
			wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
			PyUnicode_AsWideChar((PyUnicodeObject*)ageObj, text, strLen);
			text[strLen] = L'\0';
			age = text;
			delete [] text;
		}
		else if (PyString_Check(ageObj))
		{
			// we'll allow this, just in case something goes weird
			char* text = PyString_AsString(ageObj);
			wchar_t* wText = hsStringToWString(text);
			age = wText;
			delete [] wText;
		}
		else
		{
			PyErr_SetString(PyExc_TypeError, "addMarker expects three doubles, and optionally two strings");
			PYTHON_RETURN_ERROR;
		}
	}
	self->fThis->AddMarker(x, y, z, name, age);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptMarkerGame, deleteMarker, args)
{
	unsigned long markerId;
	if (!PyArg_ParseTuple(args, "k", &markerId))
	{
		PyErr_SetString(PyExc_TypeError, "deleteMarker expects an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->DeleteMarker(markerId);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptMarkerGame, changeMarkerName, args)
{
	unsigned long markerId;
	PyObject* nameObj = NULL;
	if (!PyArg_ParseTuple(args, "kO", &markerId, &nameObj))
	{
		PyErr_SetString(PyExc_TypeError, "changeMarkerName expects an unsigned long and a string");
		PYTHON_RETURN_ERROR;
	}
	if (PyUnicode_Check(nameObj))
	{
		int strLen = PyUnicode_GetSize(nameObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)nameObj, text, strLen);
		text[strLen] = L'\0';
		self->fThis->ChangeMarkerName(markerId, text);
		delete [] text;
		PYTHON_RETURN_NONE;
	}
	else if (PyString_Check(nameObj))
	{
		// we'll allow this, just in case something goes weird
		char* text = PyString_AsString(nameObj);
		wchar_t* wText = hsStringToWString(text);
		self->fThis->ChangeMarkerName(markerId, wText);
		delete [] wText;
		PYTHON_RETURN_NONE;
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "changeMarkerName expects an unsigned long and a string");
		PYTHON_RETURN_ERROR;
	}
}

PYTHON_METHOD_DEFINITION(ptMarkerGame, captureMarker, args)
{
	unsigned long markerId;
	if (!PyArg_ParseTuple(args, "k", &markerId))
	{
		PyErr_SetString(PyExc_TypeError, "captureMarker expects an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->CaptureMarker(markerId);
	PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptMarkerGame)
	PYTHON_BASIC_METHOD(ptMarkerGame, startGame, "Starts the game. Won't work on MP games if you're not the owner/creator"),
	PYTHON_BASIC_METHOD(ptMarkerGame, pauseGame, "Pauses the game. Won't work on MP games if you're not the owner/creator"),
	PYTHON_BASIC_METHOD(ptMarkerGame, resetGame, "Resets the game. Won't work on MP games if you're not the owner/creator"),
	PYTHON_METHOD(ptMarkerGame, changeGameName, "Params: newName\nChanges the name of the game. Won't work if you're not the game owner/creator"),
	PYTHON_METHOD(ptMarkerGame, changeTimeLimit, "Params: newTimeLimit\nChanges the time limit on the game (in ms). Won't work if you're not the game owner/creator, or if it's a quest game"),
	PYTHON_BASIC_METHOD(ptMarkerGame, deleteGame, "Tells the server to delete the game. Won't work if you're not the game owner/creator"),
	PYTHON_METHOD_WKEY(ptMarkerGame, addMarker, "Params: x, y, z, name = \"\", age = \"\"\nAdds a marker to the game. Age is ignored in a non-quest game. Won't work if you're not the owner/creator"),
	PYTHON_METHOD(ptMarkerGame, deleteMarker, "Params: markerId\nDeletes the specified marker from the game. Won't work if you're not the game owner/creator"),
	PYTHON_METHOD(ptMarkerGame, changeMarkerName, "Params: markerId, newName\nChanges the name of the specified marker. Won't work if you're not the game owner/creator"),
	PYTHON_METHOD(ptMarkerGame, captureMarker, "Params: markerId\nCaptures the specified marker"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptMarkerGame, pyGameCli, "Game client for the Marker game");

// required functions for PyObject interoperability
PyObject* pyMarkerGame::New(pfGameCli* client)
{
	ptMarkerGame *newObj = (ptMarkerGame*)ptMarkerGame_type.tp_new(&ptMarkerGame_type, NULL, NULL);
	if (client && (client->GetGameTypeId() == kGameTypeId_Marker))
		newObj->fThis->gameClient = client;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptMarkerGame, pyMarkerGame)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptMarkerGame, pyMarkerGame)

// Module and method definitions
void pyMarkerGame::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptMarkerGame);
	PYTHON_CLASS_IMPORT_END(m);
}

void pyMarkerGame::AddPlasmaConstantsClasses(PyObject* m)
{
	PYTHON_ENUM_START(PtMarkerGameTypes);
	PYTHON_ENUM_ELEMENT(PtMarkerGameTypes, kMarkerGameQuest, kMarkerGameQuest);
	PYTHON_ENUM_ELEMENT(PtMarkerGameTypes, kMarkerGameCGZ, kMarkerGameCGZ);
	PYTHON_ENUM_ELEMENT(PtMarkerGameTypes, kMarkerGameCapture, kMarkerGameCapture);
	PYTHON_ENUM_ELEMENT(PtMarkerGameTypes, kMarkerGameCaptureAndHold, kMarkerGameCaptureAndHold);
	PYTHON_ENUM_END(m, PtMarkerGameTypes);
}

void pyMarkerGame::AddPlasmaMethods(std::vector<PyMethodDef>& methods)
{
	PYTHON_GLOBAL_METHOD(methods, PtIsMarkerGame);
	PYTHON_GLOBAL_METHOD_WKEY(methods, PtCreateMarkerGame);
}