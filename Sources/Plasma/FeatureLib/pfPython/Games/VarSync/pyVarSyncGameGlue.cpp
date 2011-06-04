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
#include "pyVarSyncGame.h"

#include <python.h>
#include "../../pyEnum.h"

///////////////////////////////////////////////////////////////////////////////
//
// Base VarSync game client class
//

PYTHON_CLASS_DEFINITION(ptVarSyncGame, pyVarSyncGame);

PYTHON_DEFAULT_NEW_DEFINITION(ptVarSyncGame, pyVarSyncGame)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVarSyncGame)

PYTHON_NO_INIT_DEFINITION(ptVarSyncGame)

PYTHON_GLOBAL_METHOD_DEFINITION(PtIsVarSyncGame, args, "Params: typeID\nReturns true if the specifed typeID (guid as a string) is a VarSync game")
{
	PyObject* textObj;
	if (!PyArg_ParseTuple(args, "O", &textObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtIsVarSyncGame expects a unicode string");
		PYTHON_RETURN_ERROR;
	}
	if (PyUnicode_Check(textObj))
	{
		int strLen = PyUnicode_GetSize(textObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)textObj, text, strLen);
		text[strLen] = L'\0';
		bool retVal = pyVarSyncGame::IsVarSyncGame(text);
		delete [] text;
		PYTHON_RETURN_BOOL(retVal);
	}
	else if (PyString_Check(textObj))
	{
		// we'll allow this, just in case something goes weird
		char* text = PyString_AsString(textObj);
		wchar_t* wText = hsStringToWString(text);
		bool retVal = pyVarSyncGame::IsVarSyncGame(wText);
		delete [] wText;
		PYTHON_RETURN_BOOL(retVal);
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "PtIsVarSyncGame expects a unicode string");
		PYTHON_RETURN_ERROR;
	}
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtJoinCommonVarSyncGame, args, "Params: callbackKey\nJoins the common VarSync game. If one doesn't exist, it creates it")
{
	PyObject* callbackObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &callbackObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtJoinCommonVarSyncGame expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(callbackObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtJoinCommonVarSyncGame expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(callbackObj);
	pyVarSyncGame::JoinCommonVarSyncGame(*key);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVarSyncGame, setStringVar, args)
{
	unsigned long id;
	PyObject* valueObj = NULL;
	if (!PyArg_ParseTuple(args, "kO", &id, &valueObj))
	{
		PyErr_SetString(PyExc_TypeError, "setStringVar expects an unsigned long and a string");
		PYTHON_RETURN_ERROR;
	}

	if (PyUnicode_Check(valueObj))
	{
		int strLen = PyUnicode_GetSize(valueObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)valueObj, text, strLen);
		text[strLen] = L'\0';
		self->fThis->SetStringVar(id, text);
		delete [] text;
		PYTHON_RETURN_NONE;
	}
	else if (PyString_Check(valueObj))
	{
		// we'll allow this, just in case something goes weird
		char* text = PyString_AsString(valueObj);
		wchar_t* wText = hsStringToWString(text);
		self->fThis->SetStringVar(id, wText);
		delete [] wText;
		PYTHON_RETURN_NONE;
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "setStringVar expects an unsigned long and a string");
		PYTHON_RETURN_ERROR;
	}
}

PYTHON_METHOD_DEFINITION(ptVarSyncGame, setNumericVar, args)
{
	unsigned long id;
	PyObject* valueObj = NULL;
	if (!PyArg_ParseTuple(args, "kO", &id, &valueObj))
	{
		PyErr_SetString(PyExc_TypeError, "setNumericVar expects an unsigned long and a number");
		PYTHON_RETURN_ERROR;
	}

	double val = 0;
	if (PyFloat_Check(valueObj))
		val = PyFloat_AsDouble(valueObj);
	else if (PyInt_Check(valueObj))
		val = (double)PyInt_AsLong(valueObj);
	else if (PyLong_Check(valueObj))
		val = PyLong_AsDouble(valueObj);
	else
	{
		PyErr_SetString(PyExc_TypeError, "setNumericVar expects an unsigned long and a number");
		PYTHON_RETURN_ERROR;
	}

	self->fThis->SetNumericVar(id, val);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptVarSyncGame, requestAllVars, RequestAllVars)

PYTHON_METHOD_DEFINITION(ptVarSyncGame, createStringVar, args)
{
	PyObject* varNameObj = NULL;
	PyObject* valueObj = NULL;
	if (!PyArg_ParseTuple(args, "OO", &varNameObj, &valueObj))
	{
		PyErr_SetString(PyExc_TypeError, "createStringVar expects two strings");
		PYTHON_RETURN_ERROR;
	}

	std::wstring varName = L"";
	if (PyUnicode_Check(varNameObj))
	{
		int strLen = PyUnicode_GetSize(varNameObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)varNameObj, text, strLen);
		text[strLen] = L'\0';
		varName = text;
		delete [] text;
	}
	else if (PyString_Check(varNameObj))
	{
		// we'll allow this, just in case something goes weird
		char* text = PyString_AsString(varNameObj);
		wchar_t* wText = hsStringToWString(text);
		varName = wText;
		delete [] wText;
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "createStringVar expects two strings");
		PYTHON_RETURN_ERROR;
	}

	std::wstring val = L"";
	if (PyUnicode_Check(valueObj))
	{
		int strLen = PyUnicode_GetSize(valueObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)valueObj, text, strLen);
		text[strLen] = L'\0';
		val = text;
		delete [] text;
	}
	else if (PyString_Check(valueObj))
	{
		// we'll allow this, just in case something goes weird
		char* text = PyString_AsString(valueObj);
		wchar_t* wText = hsStringToWString(text);
		val = wText;
		delete [] wText;
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "createStringVar expects two strings");
		PYTHON_RETURN_ERROR;
	}

	self->fThis->CreateStringVar(varName, val);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVarSyncGame, createNumericVar, args)
{
	PyObject* varNameObj = NULL;
	PyObject* valueObj = NULL;
	if (!PyArg_ParseTuple(args, "OO", &varNameObj, &valueObj))
	{
		PyErr_SetString(PyExc_TypeError, "createNumericVar expects a string and a number");
		PYTHON_RETURN_ERROR;
	}

	std::wstring varName = L"";
	if (PyUnicode_Check(varNameObj))
	{
		int strLen = PyUnicode_GetSize(varNameObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)varNameObj, text, strLen);
		text[strLen] = L'\0';
		varName = text;
		delete [] text;
	}
	else if (PyString_Check(varNameObj))
	{
		// we'll allow this, just in case something goes weird
		char* text = PyString_AsString(varNameObj);
		wchar_t* wText = hsStringToWString(text);
		varName = wText;
		delete [] wText;
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "createNumericVar expects a string and a number");
		PYTHON_RETURN_ERROR;
	}

	double val = 0;
	if (PyFloat_Check(valueObj))
		val = PyFloat_AsDouble(valueObj);
	else if (PyInt_Check(valueObj))
		val = (double)PyInt_AsLong(valueObj);
	else if (PyLong_Check(valueObj))
		val = PyLong_AsDouble(valueObj);
	else
	{
		PyErr_SetString(PyExc_TypeError, "createNumericVar expects a string and a number");
		PYTHON_RETURN_ERROR;
	}

	self->fThis->CreateNumericVar(varName, val);
	PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptVarSyncGame)
	PYTHON_METHOD(ptVarSyncGame, setStringVar, "Params: varID, value\nAttempts to set a string variable to the specified string (clipped to 255 chars)"),
	PYTHON_METHOD(ptVarSyncGame, setNumericVar, "Params: varID, value\nAttempts to set a numeric variable to the specified number (clipped to double)"),
	PYTHON_BASIC_METHOD(ptVarSyncGame, requestAllVars, "Requests all the vars the server knows about"),
	PYTHON_METHOD(ptVarSyncGame, createStringVar, "Params: varName, value\nAttempts to create a new string variable and set it to the specified string (clipped to 255 chars)"),
	PYTHON_METHOD(ptVarSyncGame, createNumericVar, "Params: varName, value\nAttempts to create a new numeric variable and set it to the specified number (clipped to double)"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptVarSyncGame, pyGameCli, "Game client for the VarSync game");

// required functions for PyObject interoperability
PyObject* pyVarSyncGame::New(pfGameCli* client)
{
	ptVarSyncGame *newObj = (ptVarSyncGame*)ptVarSyncGame_type.tp_new(&ptVarSyncGame_type, NULL, NULL);
	if (client && (client->GetGameTypeId() == kGameTypeId_VarSync))
		newObj->fThis->gameClient = client;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptVarSyncGame, pyVarSyncGame)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptVarSyncGame, pyVarSyncGame)

// Module and method definitions
void pyVarSyncGame::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptVarSyncGame);
	PYTHON_CLASS_IMPORT_END(m);
}

void pyVarSyncGame::AddPlasmaMethods(std::vector<PyMethodDef>& methods)
{
	PYTHON_GLOBAL_METHOD(methods, PtIsVarSyncGame);
	PYTHON_GLOBAL_METHOD(methods, PtJoinCommonVarSyncGame);
}