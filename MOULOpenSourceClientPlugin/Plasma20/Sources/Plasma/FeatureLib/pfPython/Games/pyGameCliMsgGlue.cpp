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
#include "pyGameCliMsg.h"
#include "../pyEnum.h"

#include <python.h>

///////////////////////////////////////////////////////////////////////////////
//
// Base game client msg class
//

PYTHON_CLASS_DEFINITION(ptGameCliMsg, pyGameCliMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptGameCliMsg, pyGameCliMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGameCliMsg)

PYTHON_NO_INIT_DEFINITION(ptGameCliMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptGameCliMsg, getType)
{
	return PyInt_FromLong(self->fThis->GetType());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameCliMsg, getGameCli)
{
	return self->fThis->GetGameCli();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameCliMsg, upcastToFinalGameCliMsg)
{
	return self->fThis->UpcastToFinalGameCliMsg();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameCliMsg, upcastToGameMsg)
{
	return self->fThis->UpcastToGameMsg();
}

PYTHON_START_METHODS_TABLE(ptGameCliMsg)
	PYTHON_METHOD_NOARGS(ptGameCliMsg, getType, "Returns the type of the message (see PtGameCliMsgTypes)"),
	PYTHON_METHOD_NOARGS(ptGameCliMsg, getGameCli, "Returns the game client associated with this message"),
	PYTHON_METHOD_NOARGS(ptGameCliMsg, upcastToFinalGameCliMsg, "Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"),
	PYTHON_METHOD_NOARGS(ptGameCliMsg, upcastToGameMsg, "Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptGameCliMsg, "Message from the game server from a game");
PYTHON_EXPOSE_TYPE_DEFINITION(ptGameCliMsg, pyGameCliMsg);

// required functions for PyObject interoperability
PyObject* pyGameCliMsg::New(pfGameCliMsg* msg)
{
	ptGameCliMsg *newObj = (ptGameCliMsg*)ptGameCliMsg_type.tp_new(&ptGameCliMsg_type, NULL, NULL);
	newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGameCliMsg, pyGameCliMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGameCliMsg, pyGameCliMsg)

// Module and method definitions
void pyGameCliMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptGameCliMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

void pyGameCliMsg::AddPlasmaConstantsClasses(PyObject* m)
{
	PYTHON_ENUM_START(PtGameCliMsgTypes);
	PYTHON_ENUM_ELEMENT(PtGameCliMsgTypes, kGameCliPlayerJoinedMsg, kSrv2Cli_Game_PlayerJoined);
	PYTHON_ENUM_ELEMENT(PtGameCliMsgTypes, kGameCliPlayerLeftMsg, kSrv2Cli_Game_PlayerLeft);
	PYTHON_ENUM_ELEMENT(PtGameCliMsgTypes, kGameCliInviteFailedMsg, kSrv2Cli_Game_InviteFailed);
	PYTHON_ENUM_ELEMENT(PtGameCliMsgTypes, kGameCliOwnerChangeMsg, kSrv2Cli_Game_OwnerChange);
	PYTHON_ENUM_ELEMENT(PtGameCliMsgTypes, kGameCliTTTMsg, kPyGameCliTTTMsg);
	PYTHON_ENUM_ELEMENT(PtGameCliMsgTypes, kGameCliHeekMsg, kPyGameCliHeekMsg);
	PYTHON_ENUM_ELEMENT(PtGameCliMsgTypes, kGameCliMarkerMsg, kPyGameCliMarkerMsg);
	PYTHON_ENUM_ELEMENT(PtGameCliMsgTypes, kGameCliBlueSpiralMsg, kPyGameCliBlueSpiralMsg);
	PYTHON_ENUM_ELEMENT(PtGameCliMsgTypes, kGameCliClimbingWallMsg, kPyGameCliClimbingWallMsg);
	PYTHON_ENUM_ELEMENT(PtGameCliMsgTypes, kGameCliVarSyncMsg, kPyGameCliVarSyncMsg);
	PYTHON_ENUM_END(m, PtGameCliMsgTypes);
}

///////////////////////////////////////////////////////////////////////////////
//
// Game client message subclasses
//

PYTHON_CLASS_DEFINITION(ptGameCliPlayerJoinedMsg, pyGameCliPlayerJoinedMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptGameCliPlayerJoinedMsg, pyGameCliPlayerJoinedMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGameCliPlayerJoinedMsg)

PYTHON_NO_INIT_DEFINITION(ptGameCliPlayerJoinedMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptGameCliPlayerJoinedMsg, playerID)
{
	return PyLong_FromUnsignedLong(self->fThis->PlayerID());
}

PYTHON_START_METHODS_TABLE(ptGameCliPlayerJoinedMsg)
	PYTHON_METHOD_NOARGS(ptGameCliPlayerJoinedMsg, playerID, "Returns the player's ID number"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptGameCliPlayerJoinedMsg, pyGameCliMsg, "Game client message when a player joined message is received");

// required functions for PyObject interoperability
PyObject* pyGameCliPlayerJoinedMsg::New(pfGameCliMsg* msg)
{
	ptGameCliPlayerJoinedMsg *newObj = (ptGameCliPlayerJoinedMsg*)ptGameCliPlayerJoinedMsg_type.tp_new(&ptGameCliPlayerJoinedMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_Game_PlayerJoined))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGameCliPlayerJoinedMsg, pyGameCliPlayerJoinedMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGameCliPlayerJoinedMsg, pyGameCliPlayerJoinedMsg)

// Module and method definitions
void pyGameCliPlayerJoinedMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptGameCliPlayerJoinedMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////
PYTHON_CLASS_DEFINITION(ptGameCliPlayerLeftMsg, pyGameCliPlayerLeftMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptGameCliPlayerLeftMsg, pyGameCliPlayerLeftMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGameCliPlayerLeftMsg)

PYTHON_NO_INIT_DEFINITION(ptGameCliPlayerLeftMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptGameCliPlayerLeftMsg, playerID)
{
	return PyLong_FromUnsignedLong(self->fThis->PlayerID());
}

PYTHON_START_METHODS_TABLE(ptGameCliPlayerLeftMsg)
	PYTHON_METHOD_NOARGS(ptGameCliPlayerLeftMsg, playerID, "Returns the player's ID number"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptGameCliPlayerLeftMsg, pyGameCliMsg, "Game client message when a player left message is received");

// required functions for PyObject interoperability
PyObject* pyGameCliPlayerLeftMsg::New(pfGameCliMsg* msg)
{
	ptGameCliPlayerLeftMsg *newObj = (ptGameCliPlayerLeftMsg*)ptGameCliPlayerLeftMsg_type.tp_new(&ptGameCliPlayerLeftMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_Game_PlayerLeft))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGameCliPlayerLeftMsg, pyGameCliPlayerLeftMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGameCliPlayerLeftMsg, pyGameCliPlayerLeftMsg)

// Module and method definitions
void pyGameCliPlayerLeftMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptGameCliPlayerLeftMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////
PYTHON_CLASS_DEFINITION(ptGameCliInviteFailedMsg, pyGameCliInviteFailedMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptGameCliInviteFailedMsg, pyGameCliInviteFailedMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGameCliInviteFailedMsg)

PYTHON_NO_INIT_DEFINITION(ptGameCliInviteFailedMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptGameCliInviteFailedMsg, inviteeID)
{
	return PyLong_FromUnsignedLong(self->fThis->InviteeID());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameCliInviteFailedMsg, operationID)
{
	return PyLong_FromUnsignedLong(self->fThis->OperationID());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameCliInviteFailedMsg, error)
{
	return PyLong_FromLong(self->fThis->Error());
}

PYTHON_START_METHODS_TABLE(ptGameCliInviteFailedMsg)
	PYTHON_METHOD_NOARGS(ptGameCliInviteFailedMsg, inviteeID, "Returns the invitee's ID number"),
	PYTHON_METHOD_NOARGS(ptGameCliInviteFailedMsg, operationID, "Returns the operation's ID number"),
	PYTHON_METHOD_NOARGS(ptGameCliInviteFailedMsg, error, "Returns the error value (See PtGameCliInviteErrors)"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptGameCliInviteFailedMsg, pyGameCliMsg, "Game client message when an invite failed message is received");

// required functions for PyObject interoperability
PyObject* pyGameCliInviteFailedMsg::New(pfGameCliMsg* msg)
{
	ptGameCliInviteFailedMsg *newObj = (ptGameCliInviteFailedMsg*)ptGameCliInviteFailedMsg_type.tp_new(&ptGameCliInviteFailedMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_Game_InviteFailed))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGameCliInviteFailedMsg, pyGameCliInviteFailedMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGameCliInviteFailedMsg, pyGameCliInviteFailedMsg)

// Module and method definitions
void pyGameCliInviteFailedMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptGameCliInviteFailedMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

void pyGameCliInviteFailedMsg::AddPlasmaConstantsClasses(PyObject* m)
{
	PYTHON_ENUM_START(PtGameCliInviteErrors);
	PYTHON_ENUM_ELEMENT(PtGameCliInviteErrors, kGameInviteSuccess, kGameInviteSuccess);
	PYTHON_ENUM_ELEMENT(PtGameCliInviteErrors, kGameInviteErrNotOwner, kGameInviteErrNotOwner);
	PYTHON_ENUM_ELEMENT(PtGameCliInviteErrors, kGameInviteErrAlreadyInvited, kGameInviteErrAlreadyInvited);
	PYTHON_ENUM_ELEMENT(PtGameCliInviteErrors, kGameInviteErrAlreadyJoined, kGameInviteErrAlreadyJoined);
	PYTHON_ENUM_ELEMENT(PtGameCliInviteErrors, kGameInviteErrGameStarted, kGameInviteErrGameStarted);
	PYTHON_ENUM_ELEMENT(PtGameCliInviteErrors, kGameInviteErrGameOver, kGameInviteErrGameOver);
	PYTHON_ENUM_ELEMENT(PtGameCliInviteErrors, kGameInviteErrGameFull, kGameInviteErrGameFull);
	PYTHON_ENUM_ELEMENT(PtGameCliInviteErrors, kGameInviteErrNoJoin, kGameInviteErrNoJoin);
	PYTHON_ENUM_END(m, PtGameCliInviteErrors);
}

///////////////////////////////////////////////////////////////////////////////
PYTHON_CLASS_DEFINITION(ptGameCliOwnerChangeMsg, pyGameCliOwnerChangeMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptGameCliOwnerChangeMsg, pyGameCliOwnerChangeMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGameCliOwnerChangeMsg)

PYTHON_NO_INIT_DEFINITION(ptGameCliOwnerChangeMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptGameCliOwnerChangeMsg, ownerID)
{
	return PyLong_FromUnsignedLong(self->fThis->OwnerID());
}

PYTHON_START_METHODS_TABLE(ptGameCliOwnerChangeMsg)
	PYTHON_METHOD_NOARGS(ptGameCliOwnerChangeMsg, ownerID, "Returns the owner's ID number"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptGameCliOwnerChangeMsg, pyGameCliMsg, "Game client message when a owner change message is received");

// required functions for PyObject interoperability
PyObject* pyGameCliOwnerChangeMsg::New(pfGameCliMsg* msg)
{
	ptGameCliOwnerChangeMsg *newObj = (ptGameCliOwnerChangeMsg*)ptGameCliOwnerChangeMsg_type.tp_new(&ptGameCliOwnerChangeMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_Game_OwnerChange))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGameCliOwnerChangeMsg, pyGameCliOwnerChangeMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGameCliOwnerChangeMsg, pyGameCliOwnerChangeMsg)

// Module and method definitions
void pyGameCliOwnerChangeMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptGameCliOwnerChangeMsg);
	PYTHON_CLASS_IMPORT_END(m);
}