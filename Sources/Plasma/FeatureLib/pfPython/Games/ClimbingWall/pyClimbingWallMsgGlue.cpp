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
#include "pyClimbingWallMsg.h"
#include "../../pyEnum.h"

#include <python.h>

///////////////////////////////////////////////////////////////////////////////
//
// Base climbing wall msg class
//

PYTHON_CLASS_DEFINITION(ptClimbingWallMsg, pyClimbingWallMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptClimbingWallMsg, pyClimbingWallMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptClimbingWallMsg)

PYTHON_NO_INIT_DEFINITION(ptClimbingWallMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptClimbingWallMsg, getClimbingWallMsgType)
{
	return PyInt_FromLong(self->fThis->GetClimbingWallMsgType());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptClimbingWallMsg, upcastToFinalClimbingWallMsg)
{
	return self->fThis->UpcastToFinalClimbingWallMsg();
}

PYTHON_START_METHODS_TABLE(ptClimbingWallMsg)
	PYTHON_METHOD_NOARGS(ptClimbingWallMsg, getClimbingWallMsgType, "Returns the type of the ClimbingWall message (see PtClimbingWallMsgTypes)"),
	PYTHON_METHOD_NOARGS(ptClimbingWallMsg, upcastToFinalClimbingWallMsg, "Returns this message as the ClimbingWall msg it is"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptClimbingWallMsg, pyGameCliMsg, "Base class for ClimbingWall game messages");
PYTHON_EXPOSE_TYPE_DEFINITION(ptClimbingWallMsg, pyClimbingWallMsg);

// required functions for PyObject interoperability
PyObject* pyClimbingWallMsg::New(pfGameCliMsg* msg)
{
	ptClimbingWallMsg *newObj = (ptClimbingWallMsg*)ptClimbingWallMsg_type.tp_new(&ptClimbingWallMsg_type, NULL, NULL);
	if (msg && (msg->gameCli->GetGameTypeId() == kGameTypeId_ClimbingWall))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptClimbingWallMsg, pyClimbingWallMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptClimbingWallMsg, pyClimbingWallMsg)

// Module and method definitions
void pyClimbingWallMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptClimbingWallMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

void pyClimbingWallMsg::AddPlasmaConstantsClasses(PyObject* m)
{
	PYTHON_ENUM_START(PtClimbingWallMsgTypes);
	PYTHON_ENUM_ELEMENT(PtClimbingWallMsgTypes, kClimbingWallNumBlockersChanged, kSrv2Cli_ClimbingWall_NumBlockersChanged);
	PYTHON_ENUM_ELEMENT(PtClimbingWallMsgTypes, kClimbingWallReadyMsg, kSrv2Cli_ClimbingWall_Ready);
	PYTHON_ENUM_ELEMENT(PtClimbingWallMsgTypes, kClimbingWallBlockersChanged, kSrv2Cli_ClimbingWall_BlockersChanged);
	PYTHON_ENUM_ELEMENT(PtClimbingWallMsgTypes, kClimbingWallPlayerEntered, kSrv2Cli_ClimbingWall_PlayerEntered);
	PYTHON_ENUM_ELEMENT(PtClimbingWallMsgTypes, kClimbingWallSuitMachineLocked, kSrv2Cli_ClimbingWall_SuitMachineLocked);
	PYTHON_ENUM_ELEMENT(PtClimbingWallMsgTypes, kClimbingWallGameOver, kSrv2Cli_ClimbingWall_GameOver);
	PYTHON_ENUM_END(m, PtClimbingWallMsgTypes);
}

///////////////////////////////////////////////////////////////////////////////
//
// Game client message subclasses
//

PYTHON_CLASS_DEFINITION(ptClimbingWallNumBlockersChangedMsg, pyClimbingWallNumBlockersChangedMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptClimbingWallNumBlockersChangedMsg, pyClimbingWallNumBlockersChangedMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptClimbingWallNumBlockersChangedMsg)

PYTHON_NO_INIT_DEFINITION(ptClimbingWallNumBlockersChangedMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptClimbingWallNumBlockersChangedMsg, newBlockerCount)
{
	return PyInt_FromLong((long)self->fThis->NewBlockerCount());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptClimbingWallNumBlockersChangedMsg, localOnly)
{
	PYTHON_RETURN_BOOL(self->fThis->LocalOnly());
}

PYTHON_START_METHODS_TABLE(ptClimbingWallNumBlockersChangedMsg)
	PYTHON_METHOD_NOARGS(ptClimbingWallNumBlockersChangedMsg, newBlockerCount, "Returns the number of blockers this game is current running with"),
	PYTHON_METHOD_NOARGS(ptClimbingWallNumBlockersChangedMsg, localOnly, "Returns true if we are only supposed to adjust our stuff locally, and not net-prop it"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptClimbingWallNumBlockersChangedMsg, pyClimbingWallMsg, "ClimbingWall message received when the number of blockers is changed");

// required functions for PyObject interoperability
PyObject* pyClimbingWallNumBlockersChangedMsg::New(pfGameCliMsg* msg)
{
	ptClimbingWallNumBlockersChangedMsg *newObj = (ptClimbingWallNumBlockersChangedMsg*)ptClimbingWallNumBlockersChangedMsg_type.tp_new(&ptClimbingWallNumBlockersChangedMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_ClimbingWall_NumBlockersChanged))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptClimbingWallNumBlockersChangedMsg, pyClimbingWallNumBlockersChangedMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptClimbingWallNumBlockersChangedMsg, pyClimbingWallNumBlockersChangedMsg)

// Module and method definitions
void pyClimbingWallNumBlockersChangedMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptClimbingWallNumBlockersChangedMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////

PYTHON_CLASS_DEFINITION(ptClimbingWallReadyMsg, pyClimbingWallReadyMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptClimbingWallReadyMsg, pyClimbingWallReadyMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptClimbingWallReadyMsg)

PYTHON_NO_INIT_DEFINITION(ptClimbingWallReadyMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptClimbingWallReadyMsg, readyType)
{
	return PyInt_FromLong((long)self->fThis->ReadyType());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptClimbingWallReadyMsg, team1Ready)
{
	PYTHON_RETURN_BOOL(self->fThis->Team1Ready());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptClimbingWallReadyMsg, team2Ready)
{
	PYTHON_RETURN_BOOL(self->fThis->Team2Ready());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptClimbingWallReadyMsg, localOnly)
{
	PYTHON_RETURN_BOOL(self->fThis->LocalOnly());
}

PYTHON_START_METHODS_TABLE(ptClimbingWallReadyMsg)
	PYTHON_METHOD_NOARGS(ptClimbingWallReadyMsg, readyType, "The type of ready message this represents (see PtClimbingWallReadyTypes)"),
	PYTHON_METHOD_NOARGS(ptClimbingWallReadyMsg, team1Ready, "Whether team 1 is ready or not"),
	PYTHON_METHOD_NOARGS(ptClimbingWallReadyMsg, team2Ready, "Whether team 2 is ready or not"),
	PYTHON_METHOD_NOARGS(ptClimbingWallReadyMsg, localOnly, "Returns true if we are only supposed to adjust our stuff locally, and not net-prop it"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptClimbingWallReadyMsg, pyClimbingWallMsg, "ClimbingWall message received when the ready state of the teams is changed");

// required functions for PyObject interoperability
PyObject* pyClimbingWallReadyMsg::New(pfGameCliMsg* msg)
{
	ptClimbingWallReadyMsg *newObj = (ptClimbingWallReadyMsg*)ptClimbingWallReadyMsg_type.tp_new(&ptClimbingWallReadyMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_ClimbingWall_Ready))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptClimbingWallReadyMsg, pyClimbingWallReadyMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptClimbingWallReadyMsg, pyClimbingWallReadyMsg)

// Module and method definitions
void pyClimbingWallReadyMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptClimbingWallReadyMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////

PYTHON_CLASS_DEFINITION(ptClimbingWallBlockersChangedMsg, pyClimbingWallBlockersChangedMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptClimbingWallBlockersChangedMsg, pyClimbingWallBlockersChangedMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptClimbingWallBlockersChangedMsg)

PYTHON_NO_INIT_DEFINITION(ptClimbingWallBlockersChangedMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptClimbingWallBlockersChangedMsg, teamNumber)
{
	return PyInt_FromLong((long)self->fThis->TeamNumber());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptClimbingWallBlockersChangedMsg, blockersSet)
{
	std::vector<int> blockers = self->fThis->BlockersSet();
	PyObject* retVal = PyList_New(blockers.size());
	for (unsigned i = 0; i < blockers.size(); ++i)
		PyList_SetItem(retVal, i, PyInt_FromLong(blockers[i])); // steals the ref
	return retVal;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptClimbingWallBlockersChangedMsg, localOnly)
{
	PYTHON_RETURN_BOOL(self->fThis->LocalOnly());
}

PYTHON_START_METHODS_TABLE(ptClimbingWallBlockersChangedMsg)
	PYTHON_METHOD_NOARGS(ptClimbingWallBlockersChangedMsg, teamNumber, "The team that this message is for"),
	PYTHON_METHOD_NOARGS(ptClimbingWallBlockersChangedMsg, blockersSet, "Returns an array of blocker indicies denoting which blockers are set"),
	PYTHON_METHOD_NOARGS(ptClimbingWallBlockersChangedMsg, localOnly, "Returns true if we are only supposed to adjust our stuff locally, and not net-prop it"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptClimbingWallBlockersChangedMsg, pyClimbingWallMsg, "ClimbingWall message received when the blocker state changes");

// required functions for PyObject interoperability
PyObject* pyClimbingWallBlockersChangedMsg::New(pfGameCliMsg* msg)
{
	ptClimbingWallBlockersChangedMsg *newObj = (ptClimbingWallBlockersChangedMsg*)ptClimbingWallBlockersChangedMsg_type.tp_new(&ptClimbingWallBlockersChangedMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_ClimbingWall_BlockersChanged))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptClimbingWallBlockersChangedMsg, pyClimbingWallBlockersChangedMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptClimbingWallBlockersChangedMsg, pyClimbingWallBlockersChangedMsg)

// Module and method definitions
void pyClimbingWallBlockersChangedMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptClimbingWallBlockersChangedMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////

PYTHON_CLASS_DEFINITION(ptClimbingWallPlayerEnteredMsg, pyClimbingWallPlayerEnteredMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptClimbingWallPlayerEnteredMsg, pyClimbingWallPlayerEnteredMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptClimbingWallPlayerEnteredMsg)

PYTHON_NO_INIT_DEFINITION(ptClimbingWallPlayerEnteredMsg)

PYTHON_START_METHODS_TABLE(ptClimbingWallPlayerEnteredMsg)
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptClimbingWallPlayerEnteredMsg, pyClimbingWallMsg, "ClimbingWall message received when you successfully enter the suit machine");

// required functions for PyObject interoperability
PyObject* pyClimbingWallPlayerEnteredMsg::New(pfGameCliMsg* msg)
{
	ptClimbingWallPlayerEnteredMsg *newObj = (ptClimbingWallPlayerEnteredMsg*)ptClimbingWallPlayerEnteredMsg_type.tp_new(&ptClimbingWallPlayerEnteredMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_ClimbingWall_PlayerEntered))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptClimbingWallPlayerEnteredMsg, pyClimbingWallPlayerEnteredMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptClimbingWallPlayerEnteredMsg, pyClimbingWallPlayerEnteredMsg)

// Module and method definitions
void pyClimbingWallPlayerEnteredMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptClimbingWallPlayerEnteredMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////

PYTHON_CLASS_DEFINITION(ptClimbingWallSuitMachineLockedMsg, pyClimbingWallSuitMachineLockedMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptClimbingWallSuitMachineLockedMsg, pyClimbingWallSuitMachineLockedMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptClimbingWallSuitMachineLockedMsg)

PYTHON_NO_INIT_DEFINITION(ptClimbingWallSuitMachineLockedMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptClimbingWallSuitMachineLockedMsg, team1MachineLocked)
{
	PYTHON_RETURN_BOOL(self->fThis->Team1MachineLocked());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptClimbingWallSuitMachineLockedMsg, team2MachineLocked)
{
	PYTHON_RETURN_BOOL(self->fThis->Team2MachineLocked());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptClimbingWallSuitMachineLockedMsg, localOnly)
{
	PYTHON_RETURN_BOOL(self->fThis->LocalOnly());
}

PYTHON_START_METHODS_TABLE(ptClimbingWallSuitMachineLockedMsg)
	PYTHON_METHOD_NOARGS(ptClimbingWallSuitMachineLockedMsg, team1MachineLocked, "Whether team 1's suit machine is locked or not"),
	PYTHON_METHOD_NOARGS(ptClimbingWallSuitMachineLockedMsg, team2MachineLocked, "Whether team 2's suit machine is locked or not"),
	PYTHON_METHOD_NOARGS(ptClimbingWallSuitMachineLockedMsg, localOnly, "Returns true if we are only supposed to adjust our stuff locally, and not net-prop it"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptClimbingWallSuitMachineLockedMsg, pyClimbingWallMsg, "ClimbingWall message received when the locked state of the suit machines is changed");

// required functions for PyObject interoperability
PyObject* pyClimbingWallSuitMachineLockedMsg::New(pfGameCliMsg* msg)
{
	ptClimbingWallSuitMachineLockedMsg *newObj = (ptClimbingWallSuitMachineLockedMsg*)ptClimbingWallSuitMachineLockedMsg_type.tp_new(&ptClimbingWallSuitMachineLockedMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_ClimbingWall_SuitMachineLocked))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptClimbingWallSuitMachineLockedMsg, pyClimbingWallSuitMachineLockedMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptClimbingWallSuitMachineLockedMsg, pyClimbingWallSuitMachineLockedMsg)

// Module and method definitions
void pyClimbingWallSuitMachineLockedMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptClimbingWallSuitMachineLockedMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////

PYTHON_CLASS_DEFINITION(ptClimbingWallGameOverMsg, pyClimbingWallGameOverMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptClimbingWallGameOverMsg, pyClimbingWallGameOverMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptClimbingWallGameOverMsg)

PYTHON_NO_INIT_DEFINITION(ptClimbingWallGameOverMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptClimbingWallGameOverMsg, teamWon)
{
	return PyInt_FromLong((long)self->fThis->TeamWon());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptClimbingWallGameOverMsg, team1Blockers)
{
	std::vector<int> blockers = self->fThis->Team1Blockers();
	PyObject* retVal = PyList_New(blockers.size());
	for (unsigned i = 0; i < blockers.size(); ++i)
		PyList_SetItem(retVal, i, PyInt_FromLong(blockers[i])); // steals the ref
	return retVal;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptClimbingWallGameOverMsg, team2Blockers)
{
	std::vector<int> blockers = self->fThis->Team2Blockers();
	PyObject* retVal = PyList_New(blockers.size());
	for (unsigned i = 0; i < blockers.size(); ++i)
		PyList_SetItem(retVal, i, PyInt_FromLong(blockers[i])); // steals the ref
	return retVal;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptClimbingWallGameOverMsg, localOnly)
{
	PYTHON_RETURN_BOOL(self->fThis->LocalOnly());
}

PYTHON_START_METHODS_TABLE(ptClimbingWallGameOverMsg)
	PYTHON_METHOD_NOARGS(ptClimbingWallGameOverMsg, teamWon, "The team that won the game"),
	PYTHON_METHOD_NOARGS(ptClimbingWallGameOverMsg, team1Blockers, "Returns an array of blocker indicies denoting which blockers team 1 set"),
	PYTHON_METHOD_NOARGS(ptClimbingWallGameOverMsg, team2Blockers, "Returns an array of blocker indicies denoting which blockers team 2 set"),
	PYTHON_METHOD_NOARGS(ptClimbingWallGameOverMsg, localOnly, "Returns true if we are only supposed to adjust our stuff locally, and not net-prop it"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptClimbingWallGameOverMsg, pyClimbingWallMsg, "ClimbingWall message received when the game is over");

// required functions for PyObject interoperability
PyObject* pyClimbingWallGameOverMsg::New(pfGameCliMsg* msg)
{
	ptClimbingWallGameOverMsg *newObj = (ptClimbingWallGameOverMsg*)ptClimbingWallGameOverMsg_type.tp_new(&ptClimbingWallGameOverMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_ClimbingWall_GameOver))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptClimbingWallGameOverMsg, pyClimbingWallGameOverMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptClimbingWallGameOverMsg, pyClimbingWallGameOverMsg)

// Module and method definitions
void pyClimbingWallGameOverMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptClimbingWallGameOverMsg);
	PYTHON_CLASS_IMPORT_END(m);
}