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
#include "pyGameMgrMsg.h"
#include "../pyEnum.h"

#include <python.h>

///////////////////////////////////////////////////////////////////////////////
//
// Base game manager msg class
//

PYTHON_CLASS_DEFINITION(ptGameMgrMsg, pyGameMgrMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptGameMgrMsg, pyGameMgrMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGameMgrMsg)

PYTHON_NO_INIT_DEFINITION(ptGameMgrMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptGameMgrMsg, getType)
{
	return PyInt_FromLong(self->fThis->GetType());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameMgrMsg, upcastToInviteReceivedMsg)
{
	return self->fThis->UpcastToInviteReceivedMsg();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameMgrMsg, upcastToInviteRevokedMsg)
{
	return self->fThis->UpcastToInviteRevokedMsg();
}

PYTHON_START_METHODS_TABLE(ptGameMgrMsg)
	PYTHON_METHOD_NOARGS(ptGameMgrMsg, getType, "Returns the type of the message (see PtGameMgrMsgTypes)"),
	PYTHON_METHOD_NOARGS(ptGameMgrMsg, upcastToInviteReceivedMsg, "Returns this message as a ptGameMgrInviteReceivedMsg"),
	PYTHON_METHOD_NOARGS(ptGameMgrMsg, upcastToInviteRevokedMsg, "Returns this message as a ptGameMgrInviteRevokedMsg"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptGameMgrMsg, "Message from the game manager");
PYTHON_EXPOSE_TYPE_DEFINITION(ptGameMgrMsg, pyGameMgrMsg);

// required functions for PyObject interoperability
PyObject* pyGameMgrMsg::New(pfGameMgrMsg* msg)
{
	ptGameMgrMsg *newObj = (ptGameMgrMsg*)ptGameMgrMsg_type.tp_new(&ptGameMgrMsg_type, NULL, NULL);
	newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGameMgrMsg, pyGameMgrMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGameMgrMsg, pyGameMgrMsg)

// Module and method definitions
void pyGameMgrMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptGameMgrMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

void pyGameMgrMsg::AddPlasmaConstantsClasses(PyObject* m)
{
	PYTHON_ENUM_START(PtGameMgrMsgTypes);
	PYTHON_ENUM_ELEMENT(PtGameMgrMsgTypes, kGameMgrInviteReceivedMsg, kSrv2Cli_GameMgr_InviteReceived);
	PYTHON_ENUM_ELEMENT(PtGameMgrMsgTypes, kGameMgrInviteRevokedMsg, kSrv2Cli_GameMgr_InviteRevoked);
	PYTHON_ENUM_END(m, PtGameMgrMsgTypes);
}

///////////////////////////////////////////////////////////////////////////////
//
// Game manager message subclasses
//

PYTHON_CLASS_DEFINITION(ptGameMgrInviteReceivedMsg, pyGameMgrInviteReceivedMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptGameMgrInviteReceivedMsg, pyGameMgrInviteReceivedMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGameMgrInviteReceivedMsg)

PYTHON_NO_INIT_DEFINITION(ptGameMgrInviteReceivedMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptGameMgrInviteReceivedMsg, inviterID)
{
	return PyLong_FromUnsignedLong(self->fThis->InviterID());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameMgrInviteReceivedMsg, gameTypeID)
{
	std::wstring retVal = self->fThis->GameTypeID();
	return PyUnicode_FromWideChar(retVal.c_str(), retVal.length());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameMgrInviteReceivedMsg, newGameID)
{
	return PyLong_FromUnsignedLong(self->fThis->NewGameID());
}

PYTHON_START_METHODS_TABLE(ptGameMgrInviteReceivedMsg)
	PYTHON_METHOD_NOARGS(ptGameMgrInviteReceivedMsg, inviterID, "Returns the inviter's ID number"),
	PYTHON_METHOD_NOARGS(ptGameMgrInviteReceivedMsg, gameTypeID, "Returns the game type ID (as a guid string)"),
	PYTHON_METHOD_NOARGS(ptGameMgrInviteReceivedMsg, newGameID, "Returns the new game's ID number"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptGameMgrInviteReceivedMsg, pyGameMgrMsg, "Game manager message when an invite is received");

// required functions for PyObject interoperability
PyObject* pyGameMgrInviteReceivedMsg::New(pfGameMgrMsg* msg)
{
	ptGameMgrInviteReceivedMsg *newObj = (ptGameMgrInviteReceivedMsg*)ptGameMgrInviteReceivedMsg_type.tp_new(&ptGameMgrInviteReceivedMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_GameMgr_InviteReceived))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGameMgrInviteReceivedMsg, pyGameMgrInviteReceivedMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGameMgrInviteReceivedMsg, pyGameMgrInviteReceivedMsg)

// Module and method definitions
void pyGameMgrInviteReceivedMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptGameMgrInviteReceivedMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////
PYTHON_CLASS_DEFINITION(ptGameMgrInviteRevokedMsg, pyGameMgrInviteRevokedMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptGameMgrInviteRevokedMsg, pyGameMgrInviteRevokedMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGameMgrInviteRevokedMsg)

PYTHON_NO_INIT_DEFINITION(ptGameMgrInviteRevokedMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptGameMgrInviteRevokedMsg, inviterID)
{
	return PyLong_FromUnsignedLong(self->fThis->InviterID());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameMgrInviteRevokedMsg, gameTypeID)
{
	std::wstring retVal = self->fThis->GameTypeID();
	return PyUnicode_FromWideChar(retVal.c_str(), retVal.length());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameMgrInviteRevokedMsg, newGameID)
{
	return PyLong_FromUnsignedLong(self->fThis->NewGameID());
}

PYTHON_START_METHODS_TABLE(ptGameMgrInviteRevokedMsg)
	PYTHON_METHOD_NOARGS(ptGameMgrInviteRevokedMsg, inviterID, "Returns the inviter's ID number"),
	PYTHON_METHOD_NOARGS(ptGameMgrInviteRevokedMsg, gameTypeID, "Returns the game type ID (as a guid string)"),
	PYTHON_METHOD_NOARGS(ptGameMgrInviteRevokedMsg, newGameID, "Returns the new game's ID number"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptGameMgrInviteRevokedMsg, pyGameMgrMsg, "Game manager message when an invite is received");

// required functions for PyObject interoperability
PyObject* pyGameMgrInviteRevokedMsg::New(pfGameMgrMsg* msg)
{
	ptGameMgrInviteRevokedMsg *newObj = (ptGameMgrInviteRevokedMsg*)ptGameMgrInviteRevokedMsg_type.tp_new(&ptGameMgrInviteRevokedMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_GameMgr_InviteRevoked))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGameMgrInviteRevokedMsg, pyGameMgrInviteRevokedMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGameMgrInviteRevokedMsg, pyGameMgrInviteRevokedMsg)

// Module and method definitions
void pyGameMgrInviteRevokedMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptGameMgrInviteRevokedMsg);
	PYTHON_CLASS_IMPORT_END(m);
}