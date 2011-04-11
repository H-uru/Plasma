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
#include "pyMarkerMsg.h"
#include "../../pyEnum.h"

#include <python.h>

///////////////////////////////////////////////////////////////////////////////
//
// Base Marker msg class
//

PYTHON_CLASS_DEFINITION(ptMarkerMsg, pyMarkerMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptMarkerMsg, pyMarkerMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptMarkerMsg)

PYTHON_NO_INIT_DEFINITION(ptMarkerMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptMarkerMsg, getMarkerMsgType)
{
	return PyInt_FromLong(self->fThis->GetMarkerMsgType());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptMarkerMsg, upcastToFinalMarkerMsg)
{
	return self->fThis->UpcastToFinalMarkerMsg();
}

PYTHON_START_METHODS_TABLE(ptMarkerMsg)
	PYTHON_METHOD_NOARGS(ptMarkerMsg, getMarkerMsgType, "Returns the type of the Marker message (see PtMarkerMsgTypes)"),
	PYTHON_METHOD_NOARGS(ptMarkerMsg, upcastToFinalMarkerMsg, "Returns this message as the Marker message it is"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptMarkerMsg, pyGameCliMsg, "Base class for Marker game messages");
PYTHON_EXPOSE_TYPE_DEFINITION(ptMarkerMsg, pyMarkerMsg);

// required functions for PyObject interoperability
PyObject* pyMarkerMsg::New(pfGameCliMsg* msg)
{
	ptMarkerMsg *newObj = (ptMarkerMsg*)ptMarkerMsg_type.tp_new(&ptMarkerMsg_type, NULL, NULL);
	if (msg && (msg->gameCli->GetGameTypeId() == kGameTypeId_Marker))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptMarkerMsg, pyMarkerMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptMarkerMsg, pyMarkerMsg)

// Module and method definitions
void pyMarkerMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptMarkerMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

void pyMarkerMsg::AddPlasmaConstantsClasses(PyObject* m)
{
	PYTHON_ENUM_START(PtMarkerMsgTypes);
	PYTHON_ENUM_ELEMENT(PtMarkerMsgTypes, kMarkerTemplateCreated, kSrv2Cli_Marker_TemplateCreated);
	PYTHON_ENUM_ELEMENT(PtMarkerMsgTypes, kMarkerTeamAssigned, kSrv2Cli_Marker_TeamAssigned);
	PYTHON_ENUM_ELEMENT(PtMarkerMsgTypes, kMarkerGameType, kSrv2Cli_Marker_GameType);
	PYTHON_ENUM_ELEMENT(PtMarkerMsgTypes, kMarkerGameStarted, kSrv2Cli_Marker_GameStarted);
	PYTHON_ENUM_ELEMENT(PtMarkerMsgTypes, kMarkerGamePaused, kSrv2Cli_Marker_GamePaused);
	PYTHON_ENUM_ELEMENT(PtMarkerMsgTypes, kMarkerGameReset, kSrv2Cli_Marker_GameReset);
	PYTHON_ENUM_ELEMENT(PtMarkerMsgTypes, kMarkerGameOver, kSrv2Cli_Marker_GameOver);
	PYTHON_ENUM_ELEMENT(PtMarkerMsgTypes, kMarkerGameNameChanged, kSrv2Cli_Marker_GameNameChanged);
	PYTHON_ENUM_ELEMENT(PtMarkerMsgTypes, kMarkerTimeLimitChanged, kSrv2Cli_Marker_TimeLimitChanged);
	PYTHON_ENUM_ELEMENT(PtMarkerMsgTypes, kMarkerGameDeleted, kSrv2Cli_Marker_GameDeleted);
	PYTHON_ENUM_ELEMENT(PtMarkerMsgTypes, kMarkerMarkerAdded, kSrv2Cli_Marker_MarkerAdded);
	PYTHON_ENUM_ELEMENT(PtMarkerMsgTypes, kMarkerMarkerDeleted, kSrv2Cli_Marker_MarkerDeleted);
	PYTHON_ENUM_ELEMENT(PtMarkerMsgTypes, kMarkerMarkerNameChanged, kSrv2Cli_Marker_MarkerNameChanged);
	PYTHON_ENUM_ELEMENT(PtMarkerMsgTypes, kMarkerMarkerCaptured, kSrv2Cli_Marker_MarkerCaptured);
	PYTHON_ENUM_END(m, PtMarkerMsgTypes);
}

///////////////////////////////////////////////////////////////////////////////
//
// Game client message subclasses
//

PYTHON_CLASS_DEFINITION(ptMarkerTemplateCreatedMsg, pyMarkerTemplateCreatedMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptMarkerTemplateCreatedMsg, pyMarkerTemplateCreatedMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptMarkerTemplateCreatedMsg)

PYTHON_NO_INIT_DEFINITION(ptMarkerTemplateCreatedMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptMarkerTemplateCreatedMsg, templateID)
{
	std::wstring retVal = self->fThis->TemplateID();
	return PyUnicode_FromWideChar(retVal.c_str(), retVal.length());
}

PYTHON_START_METHODS_TABLE(ptMarkerTemplateCreatedMsg)
	PYTHON_METHOD_NOARGS(ptMarkerTemplateCreatedMsg, templateID, "Returns the ID number of the template that was created"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptMarkerTemplateCreatedMsg, pyMarkerMsg, "Marker message received when a quest game template is created");

// required functions for PyObject interoperability
PyObject* pyMarkerTemplateCreatedMsg::New(pfGameCliMsg* msg)
{
	ptMarkerTemplateCreatedMsg *newObj = (ptMarkerTemplateCreatedMsg*)ptMarkerTemplateCreatedMsg_type.tp_new(&ptMarkerTemplateCreatedMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_Marker_TemplateCreated))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptMarkerTemplateCreatedMsg, pyMarkerTemplateCreatedMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptMarkerTemplateCreatedMsg, pyMarkerTemplateCreatedMsg)

// Module and method definitions
void pyMarkerTemplateCreatedMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptMarkerTemplateCreatedMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////

PYTHON_CLASS_DEFINITION(ptMarkerTeamAssignedMsg, pyMarkerTeamAssignedMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptMarkerTeamAssignedMsg, pyMarkerTeamAssignedMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptMarkerTeamAssignedMsg)

PYTHON_NO_INIT_DEFINITION(ptMarkerTeamAssignedMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptMarkerTeamAssignedMsg, teamNumber)
{
	return PyInt_FromLong(self->fThis->TeamNumber());
}

PYTHON_START_METHODS_TABLE(ptMarkerTeamAssignedMsg)
	PYTHON_METHOD_NOARGS(ptMarkerTeamAssignedMsg, teamNumber, "Returns the number of the team you were assigned to"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptMarkerTeamAssignedMsg, pyMarkerMsg, "Marker message received when you are assigned a team number");

// required functions for PyObject interoperability
PyObject* pyMarkerTeamAssignedMsg::New(pfGameCliMsg* msg)
{
	ptMarkerTeamAssignedMsg *newObj = (ptMarkerTeamAssignedMsg*)ptMarkerTeamAssignedMsg_type.tp_new(&ptMarkerTeamAssignedMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_Marker_TeamAssigned))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptMarkerTeamAssignedMsg, pyMarkerTeamAssignedMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptMarkerTeamAssignedMsg, pyMarkerTeamAssignedMsg)

// Module and method definitions
void pyMarkerTeamAssignedMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptMarkerTeamAssignedMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////

PYTHON_CLASS_DEFINITION(ptMarkerGameTypeMsg, pyMarkerGameTypeMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptMarkerGameTypeMsg, pyMarkerGameTypeMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptMarkerGameTypeMsg)

PYTHON_NO_INIT_DEFINITION(ptMarkerGameTypeMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptMarkerGameTypeMsg, gameType)
{
	return PyInt_FromLong(self->fThis->GameType());
}

PYTHON_START_METHODS_TABLE(ptMarkerGameTypeMsg)
	PYTHON_METHOD_NOARGS(ptMarkerGameTypeMsg, gameType, "Returns the type of the game you just joined"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptMarkerGameTypeMsg, pyMarkerMsg, "Marker message received when you are assigned a team number");

// required functions for PyObject interoperability
PyObject* pyMarkerGameTypeMsg::New(pfGameCliMsg* msg)
{
	ptMarkerGameTypeMsg *newObj = (ptMarkerGameTypeMsg*)ptMarkerGameTypeMsg_type.tp_new(&ptMarkerGameTypeMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_Marker_GameType))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptMarkerGameTypeMsg, pyMarkerGameTypeMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptMarkerGameTypeMsg, pyMarkerGameTypeMsg)

// Module and method definitions
void pyMarkerGameTypeMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptMarkerGameTypeMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////

PYTHON_CLASS_DEFINITION(ptMarkerGameStartedMsg, pyMarkerGameStartedMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptMarkerGameStartedMsg, pyMarkerGameStartedMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptMarkerGameStartedMsg)

PYTHON_NO_INIT_DEFINITION(ptMarkerGameStartedMsg)

PYTHON_START_METHODS_TABLE(ptMarkerGameStartedMsg)
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptMarkerGameStartedMsg, pyMarkerMsg, "Marker message received when the game is started by the owner");

// required functions for PyObject interoperability
PyObject* pyMarkerGameStartedMsg::New(pfGameCliMsg* msg)
{
	ptMarkerGameStartedMsg *newObj = (ptMarkerGameStartedMsg*)ptMarkerGameStartedMsg_type.tp_new(&ptMarkerGameStartedMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_Marker_GameStarted))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptMarkerGameStartedMsg, pyMarkerGameStartedMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptMarkerGameStartedMsg, pyMarkerGameStartedMsg)

// Module and method definitions
void pyMarkerGameStartedMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptMarkerGameStartedMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////

PYTHON_CLASS_DEFINITION(ptMarkerGamePausedMsg, pyMarkerGamePausedMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptMarkerGamePausedMsg, pyMarkerGamePausedMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptMarkerGamePausedMsg)

PYTHON_NO_INIT_DEFINITION(ptMarkerGamePausedMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptMarkerGamePausedMsg, timeLeft)
{
	return PyLong_FromUnsignedLong(self->fThis->TimeLeft());
}

PYTHON_START_METHODS_TABLE(ptMarkerGamePausedMsg)
	PYTHON_METHOD_NOARGS(ptMarkerGamePausedMsg, timeLeft, "Returns the amount of time left on the server clock"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptMarkerGamePausedMsg, pyMarkerMsg, "Marker message received when the game is paused by the owner");

// required functions for PyObject interoperability
PyObject* pyMarkerGamePausedMsg::New(pfGameCliMsg* msg)
{
	ptMarkerGamePausedMsg *newObj = (ptMarkerGamePausedMsg*)ptMarkerGamePausedMsg_type.tp_new(&ptMarkerGamePausedMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_Marker_GamePaused))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptMarkerGamePausedMsg, pyMarkerGamePausedMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptMarkerGamePausedMsg, pyMarkerGamePausedMsg)

// Module and method definitions
void pyMarkerGamePausedMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptMarkerGamePausedMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////

PYTHON_CLASS_DEFINITION(ptMarkerGameResetMsg, pyMarkerGameResetMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptMarkerGameResetMsg, pyMarkerGameResetMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptMarkerGameResetMsg)

PYTHON_NO_INIT_DEFINITION(ptMarkerGameResetMsg)

PYTHON_START_METHODS_TABLE(ptMarkerGameResetMsg)
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptMarkerGameResetMsg, pyMarkerMsg, "Marker message received when the game is reset by the owner");

// required functions for PyObject interoperability
PyObject* pyMarkerGameResetMsg::New(pfGameCliMsg* msg)
{
	ptMarkerGameResetMsg *newObj = (ptMarkerGameResetMsg*)ptMarkerGameResetMsg_type.tp_new(&ptMarkerGameResetMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_Marker_GameReset))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptMarkerGameResetMsg, pyMarkerGameResetMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptMarkerGameResetMsg, pyMarkerGameResetMsg)

// Module and method definitions
void pyMarkerGameResetMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptMarkerGameResetMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////

PYTHON_CLASS_DEFINITION(ptMarkerGameOverMsg, pyMarkerGameOverMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptMarkerGameOverMsg, pyMarkerGameOverMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptMarkerGameOverMsg)

PYTHON_NO_INIT_DEFINITION(ptMarkerGameOverMsg)

PYTHON_START_METHODS_TABLE(ptMarkerGameOverMsg)
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptMarkerGameOverMsg, pyMarkerMsg, "Marker message received when the server determines the game is over (usually via timeout)");

// required functions for PyObject interoperability
PyObject* pyMarkerGameOverMsg::New(pfGameCliMsg* msg)
{
	ptMarkerGameOverMsg *newObj = (ptMarkerGameOverMsg*)ptMarkerGameOverMsg_type.tp_new(&ptMarkerGameOverMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_Marker_GameOver))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptMarkerGameOverMsg, pyMarkerGameOverMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptMarkerGameOverMsg, pyMarkerGameOverMsg)

// Module and method definitions
void pyMarkerGameOverMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptMarkerGameOverMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////

PYTHON_CLASS_DEFINITION(ptMarkerGameNameChangedMsg, pyMarkerGameNameChangedMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptMarkerGameNameChangedMsg, pyMarkerGameNameChangedMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptMarkerGameNameChangedMsg)

PYTHON_NO_INIT_DEFINITION(ptMarkerGameNameChangedMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptMarkerGameNameChangedMsg, name)
{
	std::wstring retVal = self->fThis->Name();
	return PyUnicode_FromWideChar(retVal.c_str(), retVal.length());
}

PYTHON_START_METHODS_TABLE(ptMarkerGameNameChangedMsg)
	PYTHON_METHOD_NOARGS(ptMarkerGameNameChangedMsg, name, "Returns the new game name"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptMarkerGameNameChangedMsg, pyMarkerMsg, "Marker message received when the game name is changed");

// required functions for PyObject interoperability
PyObject* pyMarkerGameNameChangedMsg::New(pfGameCliMsg* msg)
{
	ptMarkerGameNameChangedMsg *newObj = (ptMarkerGameNameChangedMsg*)ptMarkerGameNameChangedMsg_type.tp_new(&ptMarkerGameNameChangedMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_Marker_GameNameChanged))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptMarkerGameNameChangedMsg, pyMarkerGameNameChangedMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptMarkerGameNameChangedMsg, pyMarkerGameNameChangedMsg)

// Module and method definitions
void pyMarkerGameNameChangedMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptMarkerGameNameChangedMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////

PYTHON_CLASS_DEFINITION(ptMarkerTimeLimitChangedMsg, pyMarkerTimeLimitChangedMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptMarkerTimeLimitChangedMsg, pyMarkerTimeLimitChangedMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptMarkerTimeLimitChangedMsg)

PYTHON_NO_INIT_DEFINITION(ptMarkerTimeLimitChangedMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptMarkerTimeLimitChangedMsg, timeLimit)
{
	return PyLong_FromUnsignedLong(self->fThis->TimeLimit());
}

PYTHON_START_METHODS_TABLE(ptMarkerTimeLimitChangedMsg)
	PYTHON_METHOD_NOARGS(ptMarkerTimeLimitChangedMsg, timeLimit, "Returns the new time limit (in ms)"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptMarkerTimeLimitChangedMsg, pyMarkerMsg, "Marker message received when the game name is changed");

// required functions for PyObject interoperability
PyObject* pyMarkerTimeLimitChangedMsg::New(pfGameCliMsg* msg)
{
	ptMarkerTimeLimitChangedMsg *newObj = (ptMarkerTimeLimitChangedMsg*)ptMarkerTimeLimitChangedMsg_type.tp_new(&ptMarkerTimeLimitChangedMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_Marker_TimeLimitChanged))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptMarkerTimeLimitChangedMsg, pyMarkerTimeLimitChangedMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptMarkerTimeLimitChangedMsg, pyMarkerTimeLimitChangedMsg)

// Module and method definitions
void pyMarkerTimeLimitChangedMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptMarkerTimeLimitChangedMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////

PYTHON_CLASS_DEFINITION(ptMarkerGameDeletedMsg, pyMarkerGameDeletedMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptMarkerGameDeletedMsg, pyMarkerGameDeletedMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptMarkerGameDeletedMsg)

PYTHON_NO_INIT_DEFINITION(ptMarkerGameDeletedMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptMarkerGameDeletedMsg, failed)
{
	PYTHON_RETURN_BOOL(self->fThis->Failed());
}

PYTHON_START_METHODS_TABLE(ptMarkerGameDeletedMsg)
	PYTHON_METHOD_NOARGS(ptMarkerGameDeletedMsg, failed, "Returns whether the delete succeeded or not"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptMarkerGameDeletedMsg, pyMarkerMsg, "Marker message received when the game is deleted");

// required functions for PyObject interoperability
PyObject* pyMarkerGameDeletedMsg::New(pfGameCliMsg* msg)
{
	ptMarkerGameDeletedMsg *newObj = (ptMarkerGameDeletedMsg*)ptMarkerGameDeletedMsg_type.tp_new(&ptMarkerGameDeletedMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_Marker_GameDeleted))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptMarkerGameDeletedMsg, pyMarkerGameDeletedMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptMarkerGameDeletedMsg, pyMarkerGameDeletedMsg)

// Module and method definitions
void pyMarkerGameDeletedMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptMarkerGameDeletedMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////

PYTHON_CLASS_DEFINITION(ptMarkerMarkerAddedMsg, pyMarkerMarkerAddedMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptMarkerMarkerAddedMsg, pyMarkerMarkerAddedMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptMarkerMarkerAddedMsg)

PYTHON_NO_INIT_DEFINITION(ptMarkerMarkerAddedMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptMarkerMarkerAddedMsg, x)
{
	return PyFloat_FromDouble(self->fThis->X());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptMarkerMarkerAddedMsg, y)
{
	return PyFloat_FromDouble(self->fThis->Y());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptMarkerMarkerAddedMsg, z)
{
	return PyFloat_FromDouble(self->fThis->Z());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptMarkerMarkerAddedMsg, markerId)
{
	return PyLong_FromUnsignedLong(self->fThis->MarkerId());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptMarkerMarkerAddedMsg, name)
{
	std::wstring retVal = self->fThis->Name();
	return PyUnicode_FromWideChar(retVal.c_str(), retVal.length());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptMarkerMarkerAddedMsg, age)
{
	std::wstring retVal = self->fThis->Age();
	return PyUnicode_FromWideChar(retVal.c_str(), retVal.length());
}

PYTHON_START_METHODS_TABLE(ptMarkerMarkerAddedMsg)
	PYTHON_METHOD_NOARGS(ptMarkerMarkerAddedMsg, x, "Returns x coord of the marker"),
	PYTHON_METHOD_NOARGS(ptMarkerMarkerAddedMsg, y, "Returns y coord of the marker"),
	PYTHON_METHOD_NOARGS(ptMarkerMarkerAddedMsg, z, "Returns z coord of the marker"),
	PYTHON_METHOD_NOARGS(ptMarkerMarkerAddedMsg, markerId, "Returns the id number of the marker"),
	PYTHON_METHOD_NOARGS(ptMarkerMarkerAddedMsg, name, "Returns the name of the marker"),
	PYTHON_METHOD_NOARGS(ptMarkerMarkerAddedMsg, age, "Returns the age the marker was created in"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptMarkerMarkerAddedMsg, pyMarkerMsg, "Marker message received when a marker is added to the game");

// required functions for PyObject interoperability
PyObject* pyMarkerMarkerAddedMsg::New(pfGameCliMsg* msg)
{
	ptMarkerMarkerAddedMsg *newObj = (ptMarkerMarkerAddedMsg*)ptMarkerMarkerAddedMsg_type.tp_new(&ptMarkerMarkerAddedMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_Marker_MarkerAdded))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptMarkerMarkerAddedMsg, pyMarkerMarkerAddedMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptMarkerMarkerAddedMsg, pyMarkerMarkerAddedMsg)

// Module and method definitions
void pyMarkerMarkerAddedMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptMarkerMarkerAddedMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////

PYTHON_CLASS_DEFINITION(ptMarkerMarkerDeletedMsg, pyMarkerMarkerDeletedMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptMarkerMarkerDeletedMsg, pyMarkerMarkerDeletedMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptMarkerMarkerDeletedMsg)

PYTHON_NO_INIT_DEFINITION(ptMarkerMarkerDeletedMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptMarkerMarkerDeletedMsg, markerId)
{
	return PyLong_FromUnsignedLong(self->fThis->MarkerId());
}

PYTHON_START_METHODS_TABLE(ptMarkerMarkerDeletedMsg)
	PYTHON_METHOD_NOARGS(ptMarkerMarkerDeletedMsg, markerId, "Returns id of the marker that was deleted"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptMarkerMarkerDeletedMsg, pyMarkerMsg, "Marker message received when a marker is deleted");

// required functions for PyObject interoperability
PyObject* pyMarkerMarkerDeletedMsg::New(pfGameCliMsg* msg)
{
	ptMarkerMarkerDeletedMsg *newObj = (ptMarkerMarkerDeletedMsg*)ptMarkerMarkerDeletedMsg_type.tp_new(&ptMarkerMarkerDeletedMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_Marker_MarkerDeleted))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptMarkerMarkerDeletedMsg, pyMarkerMarkerDeletedMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptMarkerMarkerDeletedMsg, pyMarkerMarkerDeletedMsg)

// Module and method definitions
void pyMarkerMarkerDeletedMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptMarkerMarkerDeletedMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////

PYTHON_CLASS_DEFINITION(ptMarkerMarkerNameChangedMsg, pyMarkerMarkerNameChangedMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptMarkerMarkerNameChangedMsg, pyMarkerMarkerNameChangedMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptMarkerMarkerNameChangedMsg)

PYTHON_NO_INIT_DEFINITION(ptMarkerMarkerNameChangedMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptMarkerMarkerNameChangedMsg, markerId)
{
	return PyLong_FromUnsignedLong(self->fThis->MarkerId());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptMarkerMarkerNameChangedMsg, name)
{
	std::wstring retVal = self->fThis->Name();
	return PyUnicode_FromWideChar(retVal.c_str(), retVal.length());
}

PYTHON_START_METHODS_TABLE(ptMarkerMarkerNameChangedMsg)
	PYTHON_METHOD_NOARGS(ptMarkerMarkerNameChangedMsg, markerId, "Returns id of the marker who's name was changed"),
	PYTHON_METHOD_NOARGS(ptMarkerMarkerNameChangedMsg, name, "Returns the new name"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptMarkerMarkerNameChangedMsg, pyMarkerMsg, "Marker message received when the name of a marker is changed");

// required functions for PyObject interoperability
PyObject* pyMarkerMarkerNameChangedMsg::New(pfGameCliMsg* msg)
{
	ptMarkerMarkerNameChangedMsg *newObj = (ptMarkerMarkerNameChangedMsg*)ptMarkerMarkerNameChangedMsg_type.tp_new(&ptMarkerMarkerNameChangedMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_Marker_MarkerNameChanged))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptMarkerMarkerNameChangedMsg, pyMarkerMarkerNameChangedMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptMarkerMarkerNameChangedMsg, pyMarkerMarkerNameChangedMsg)

// Module and method definitions
void pyMarkerMarkerNameChangedMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptMarkerMarkerNameChangedMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////

PYTHON_CLASS_DEFINITION(ptMarkerMarkerCapturedMsg, pyMarkerMarkerCapturedMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptMarkerMarkerCapturedMsg, pyMarkerMarkerCapturedMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptMarkerMarkerCapturedMsg)

PYTHON_NO_INIT_DEFINITION(ptMarkerMarkerCapturedMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptMarkerMarkerCapturedMsg, markerId)
{
	return PyLong_FromUnsignedLong(self->fThis->MarkerId());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptMarkerMarkerCapturedMsg, team)
{
	return PyInt_FromLong((long)self->fThis->Team());
}

PYTHON_START_METHODS_TABLE(ptMarkerMarkerCapturedMsg)
	PYTHON_METHOD_NOARGS(ptMarkerMarkerCapturedMsg, markerId, "Returns id of the marker which was captured"),
	PYTHON_METHOD_NOARGS(ptMarkerMarkerCapturedMsg, team, "Returns the team number of the team that captured it (0 for no team, or a quest game)"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptMarkerMarkerCapturedMsg, pyMarkerMsg, "Marker message received when a marker is captured");

// required functions for PyObject interoperability
PyObject* pyMarkerMarkerCapturedMsg::New(pfGameCliMsg* msg)
{
	ptMarkerMarkerCapturedMsg *newObj = (ptMarkerMarkerCapturedMsg*)ptMarkerMarkerCapturedMsg_type.tp_new(&ptMarkerMarkerNameChangedMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_Marker_MarkerCaptured))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptMarkerMarkerCapturedMsg, pyMarkerMarkerCapturedMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptMarkerMarkerCapturedMsg, pyMarkerMarkerCapturedMsg)

// Module and method definitions
void pyMarkerMarkerCapturedMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptMarkerMarkerCapturedMsg);
	PYTHON_CLASS_IMPORT_END(m);
}