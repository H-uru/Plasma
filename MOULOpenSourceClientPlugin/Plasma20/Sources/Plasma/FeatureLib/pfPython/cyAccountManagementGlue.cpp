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
#include "cyAccountManagement.h"

#include "pyGlueHelpers.h"
#include "pyEnum.h"

#include "../plMessage/plAccountUpdateMsg.h"

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtIsSubscriptionActive, "Returns true if the current player is a paying subscriber")
{
	PYTHON_RETURN_BOOL(cyAccountManagement::IsSubscriptionActive());
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetAccountPlayerList, "Returns list of players associated with the current account")
{
	return cyAccountManagement::GetPlayerList();
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetAccountName, "Returns the account name for the current account")
{
	std::wstring name = cyAccountManagement::GetAccountName();
	return PyUnicode_FromWideChar(name.c_str(), name.length());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtCreatePlayer, args, "Params: playerName, avatarShape, invitation\nCreates a new player")
{
	char* playerName;
	char* avatarShape;
	char* invitation;
	if (!PyArg_ParseTuple(args, "ssz", &playerName, &avatarShape, &invitation))
	{
		PyErr_SetString(PyExc_TypeError, "PtCreatePlayer expects three strings");
		PYTHON_RETURN_ERROR;
	}

	cyAccountManagement::CreatePlayer(playerName, avatarShape, invitation);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtCreatePlayerW, args, "Params: playerName, avatarShape, invitation\nUnicode version of PtCreatePlayer")
{
	PyObject* playerNameObj;
	PyObject* avatarShapeObj;
	PyObject* invitationObj;
	if (!PyArg_ParseTuple(args, "OOO", &playerNameObj, &avatarShapeObj, &invitationObj))
	{
		PyErr_SetString(PyExc_TypeError, "PtCreatePlayerW expects three unicode strings");
		PYTHON_RETURN_ERROR;
	}

	std::wstring playerName, avatarShape, invitation;

	if (PyUnicode_Check(playerNameObj))
	{
		int strLen = PyUnicode_GetSize(playerNameObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)playerNameObj, text, strLen);
		text[strLen] = L'\0';
		playerName = text;
		delete [] text;
	}
	else if (PyString_Check(playerNameObj))
	{
		// we'll allow this, just in case something goes weird
		char* text = PyString_AsString(playerNameObj);
		wchar_t* temp = hsStringToWString(text);
		playerName = temp;
		delete [] temp;
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "PtCreatePlayerW expects three unicode strings");
		PYTHON_RETURN_ERROR;
	}

	if (PyUnicode_Check(avatarShapeObj))
	{
		int strLen = PyUnicode_GetSize(avatarShapeObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)avatarShapeObj, text, strLen);
		text[strLen] = L'\0';
		avatarShape = text;
		delete [] text;
	}
	else if (PyString_Check(avatarShapeObj))
	{
		// we'll allow this, just in case something goes weird
		char* text = PyString_AsString(avatarShapeObj);
		wchar_t* temp = hsStringToWString(text);
		avatarShape = temp;
		delete [] temp;
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "PtCreatePlayerW expects three unicode strings");
		PYTHON_RETURN_ERROR;
	}

	if (PyUnicode_Check(invitationObj))
	{
		int strLen = PyUnicode_GetSize(invitationObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)invitationObj, text, strLen);
		text[strLen] = L'\0';
		invitation = text;
		delete [] text;
	}
	else if (PyString_Check(invitationObj))
	{
		// we'll allow this, just in case something goes weird
		char* text = PyString_AsString(invitationObj);
		wchar_t* temp = hsStringToWString(text);
		invitation = temp;
		delete [] temp;
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "PtCreatePlayerW expects three unicode strings");
		PYTHON_RETURN_ERROR;
	}

	cyAccountManagement::CreatePlayerW(playerName.c_str(), avatarShape.c_str(), invitation.c_str());
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtDeletePlayer, args, "Params: playerInt\nDeletes a player associated with the current account")
{
	unsigned playerInt = 0;
	if (!PyArg_ParseTuple(args, "I", &playerInt))
	{
		PyErr_SetString(PyExc_TypeError, "PtDeletePlayer expects a unsigned int");
		PYTHON_RETURN_ERROR;
	}

	cyAccountManagement::DeletePlayer(playerInt);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtSetActivePlayer, args, "Params: playerInt\nSets the active player associated with the current account")
{
	unsigned playerInt = 0;
	if (!PyArg_ParseTuple(args, "I", &playerInt))
	{
		PyErr_SetString(PyExc_TypeError, "PtSetActivePlayer expects a unsigned int");
		PYTHON_RETURN_ERROR;
	}

	cyAccountManagement::SetActivePlayer(playerInt);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtIsActivePlayerSet, "Returns whether or not an active player is set")
{
	PYTHON_RETURN_BOOL(cyAccountManagement::IsActivePlayerSet());
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtUpgradeVisitorToExplorer, args, "Params: playerInt\nUpgrades the player to explorer status")
{
	unsigned playerInt = 0;
	if (!PyArg_ParseTuple(args, "I", &playerInt))
	{
		PyErr_SetString(PyExc_TypeError, "PtUpgradeVisitorToExplorer expects a unsigned int");
		PYTHON_RETURN_ERROR;
	}

	cyAccountManagement::UpgradeVisitorToExplorer(playerInt);
	PYTHON_RETURN_NONE;
}

PYTHON_GLOBAL_METHOD_DEFINITION(PtChangePassword, args, "Params: password\nChanges the current account's password")
{
	char* password = nil;
	if (!PyArg_ParseTuple(args, "s", &password))
	{
		PyErr_SetString(PyExc_TypeError, "PtChangePassword expects a string");
		PYTHON_RETURN_ERROR;
	}

	cyAccountManagement::ChangePassword(password);
	PYTHON_RETURN_NONE;
}

void cyAccountManagement::AddPlasmaMethods(std::vector<PyMethodDef> &methods)
{
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtIsSubscriptionActive);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetAccountPlayerList);
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetAccountName);
	PYTHON_GLOBAL_METHOD(methods, PtCreatePlayer);
	PYTHON_GLOBAL_METHOD(methods, PtCreatePlayerW);
	PYTHON_GLOBAL_METHOD(methods, PtDeletePlayer);
	PYTHON_GLOBAL_METHOD(methods, PtSetActivePlayer);
	PYTHON_GLOBAL_METHOD(methods, PtIsActivePlayerSet);
	PYTHON_GLOBAL_METHOD(methods, PtUpgradeVisitorToExplorer);
	PYTHON_GLOBAL_METHOD(methods, PtChangePassword);
}

void cyAccountManagement::AddPlasmaConstantsClasses(PyObject *m)
{
	PYTHON_ENUM_START(PtAccountUpdateType);
	PYTHON_ENUM_ELEMENT(PtAccountUpdateType, kCreatePlayer,		plAccountUpdateMsg::kCreatePlayer);
	PYTHON_ENUM_ELEMENT(PtAccountUpdateType, kDeletePlayer,		plAccountUpdateMsg::kDeletePlayer);
	PYTHON_ENUM_ELEMENT(PtAccountUpdateType, kUpgradePlayer,	plAccountUpdateMsg::kUpgradePlayer);
	PYTHON_ENUM_ELEMENT(PtAccountUpdateType, kActivePlayer,		plAccountUpdateMsg::kActivePlayer);
	PYTHON_ENUM_ELEMENT(PtAccountUpdateType, kChangePassword,	plAccountUpdateMsg::kChangePassword);
	PYTHON_ENUM_END(m, PtAccountUpdateType);
}
