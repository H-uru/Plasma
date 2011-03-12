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
#include "pyTTTMsg.h"
#include "../../pyEnum.h"

#include <python.h>

///////////////////////////////////////////////////////////////////////////////
//
// Base TTT msg class
//

PYTHON_CLASS_DEFINITION(ptTTTMsg, pyTTTMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptTTTMsg, pyTTTMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptTTTMsg)

PYTHON_NO_INIT_DEFINITION(ptTTTMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptTTTMsg, getTTTMsgType)
{
	return PyInt_FromLong(self->fThis->GetTTTMsgType());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptTTTMsg, upcastToFinalTTTMsg)
{
	return self->fThis->UpcastToFinalTTTMsg();
}

PYTHON_START_METHODS_TABLE(ptTTTMsg)
	PYTHON_METHOD_NOARGS(ptTTTMsg, getTTTMsgType, "Returns the type of the TTT message (see PtTTTMsgTypes)"),
	PYTHON_METHOD_NOARGS(ptTTTMsg, upcastToFinalTTTMsg, "Returns this message as the TTT msg it is"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptTTTMsg, pyGameCliMsg, "Base class for TicTacToe game messages");
PYTHON_EXPOSE_TYPE_DEFINITION(ptTTTMsg, pyTTTMsg);

// required functions for PyObject interoperability
PyObject* pyTTTMsg::New(pfGameCliMsg* msg)
{
	ptTTTMsg *newObj = (ptTTTMsg*)ptTTTMsg_type.tp_new(&ptTTTMsg_type, NULL, NULL);
	if (msg && (msg->gameCli->GetGameTypeId() == kGameTypeId_TicTacToe))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptTTTMsg, pyTTTMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptTTTMsg, pyTTTMsg)

// Module and method definitions
void pyTTTMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptTTTMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

void pyTTTMsg::AddPlasmaConstantsClasses(PyObject* m)
{
	PYTHON_ENUM_START(PtTTTMsgTypes);
	PYTHON_ENUM_ELEMENT(PtTTTMsgTypes, kTTTGameStarted, kSrv2Cli_TTT_GameStarted);
	PYTHON_ENUM_ELEMENT(PtTTTMsgTypes, kTTTGameOver, kSrv2Cli_TTT_GameOver);
	PYTHON_ENUM_ELEMENT(PtTTTMsgTypes, kTTTMoveMade, kSrv2Cli_TTT_MoveMade);
	PYTHON_ENUM_END(m, PtTTTMsgTypes);
}

///////////////////////////////////////////////////////////////////////////////
//
// Game client message subclasses
//

PYTHON_CLASS_DEFINITION(ptTTTGameStartedMsg, pyTTTGameStartedMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptTTTGameStartedMsg, pyTTTGameStartedMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptTTTGameStartedMsg)

PYTHON_NO_INIT_DEFINITION(ptTTTGameStartedMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptTTTGameStartedMsg, yourTurn)
{
	PYTHON_RETURN_BOOL(self->fThis->YourTurn());
}

PYTHON_START_METHODS_TABLE(ptTTTGameStartedMsg)
	PYTHON_METHOD_NOARGS(ptTTTGameStartedMsg, yourTurn, "Returns true if you are the first player (and therefore it's your turn)"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptTTTGameStartedMsg, pyTTTMsg, "TicTacToe message received when the game is started");

// required functions for PyObject interoperability
PyObject* pyTTTGameStartedMsg::New(pfGameCliMsg* msg)
{
	ptTTTGameStartedMsg *newObj = (ptTTTGameStartedMsg*)ptTTTGameStartedMsg_type.tp_new(&ptTTTGameStartedMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_TTT_GameStarted))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptTTTGameStartedMsg, pyTTTGameStartedMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptTTTGameStartedMsg, pyTTTGameStartedMsg)

// Module and method definitions
void pyTTTGameStartedMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptTTTGameStartedMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////
PYTHON_CLASS_DEFINITION(ptTTTGameOverMsg, pyTTTGameOverMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptTTTGameOverMsg, pyTTTGameOverMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptTTTGameOverMsg)

PYTHON_NO_INIT_DEFINITION(ptTTTGameOverMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptTTTGameOverMsg, result)
{
	return PyInt_FromLong(self->fThis->Result());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptTTTGameOverMsg, winnerID)
{
	return PyLong_FromUnsignedLong(self->fThis->WinnerID());
}

PYTHON_START_METHODS_TABLE(ptTTTGameOverMsg)
	PYTHON_METHOD_NOARGS(ptTTTGameOverMsg, result, "Returns the result of the game (see PtTTTGameResult)"),
	PYTHON_METHOD_NOARGS(ptTTTGameOverMsg, winnerID, "Returns the winner's ID"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptTTTGameOverMsg, pyTTTMsg, "TicTacToe message received when the game is over");

// required functions for PyObject interoperability
PyObject* pyTTTGameOverMsg::New(pfGameCliMsg* msg)
{
	ptTTTGameOverMsg *newObj = (ptTTTGameOverMsg*)ptTTTGameOverMsg_type.tp_new(&ptTTTGameOverMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_TTT_GameOver))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptTTTGameOverMsg, pyTTTGameOverMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptTTTGameOverMsg, pyTTTGameOverMsg)

// Module and method definitions
void pyTTTGameOverMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptTTTGameOverMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////
PYTHON_CLASS_DEFINITION(ptTTTMoveMadeMsg, pyTTTMoveMadeMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptTTTMoveMadeMsg, pyTTTMoveMadeMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptTTTMoveMadeMsg)

PYTHON_NO_INIT_DEFINITION(ptTTTMoveMadeMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptTTTMoveMadeMsg, playerID)
{
	return PyLong_FromUnsignedLong(self->fThis->PlayerID());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptTTTMoveMadeMsg, row)
{
	return PyInt_FromLong(self->fThis->Row());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptTTTMoveMadeMsg, col)
{
	return PyInt_FromLong(self->fThis->Col());
}

PYTHON_START_METHODS_TABLE(ptTTTMoveMadeMsg)
	PYTHON_METHOD_NOARGS(ptTTTMoveMadeMsg, playerID, "Returns the the ID of the player that just moved"),
	PYTHON_METHOD_NOARGS(ptTTTMoveMadeMsg, row, "Returns the row index of the move (1..3)"),
	PYTHON_METHOD_NOARGS(ptTTTMoveMadeMsg, col, "Returns the col index of the move (1..3)"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptTTTMoveMadeMsg, pyTTTMsg, "TicTacToe message received when someone makes a move");

// required functions for PyObject interoperability
PyObject* pyTTTMoveMadeMsg::New(pfGameCliMsg* msg)
{
	ptTTTMoveMadeMsg *newObj = (ptTTTMoveMadeMsg*)ptTTTMoveMadeMsg_type.tp_new(&ptTTTMoveMadeMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_TTT_MoveMade))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptTTTMoveMadeMsg, pyTTTMoveMadeMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptTTTMoveMadeMsg, pyTTTMoveMadeMsg)

// Module and method definitions
void pyTTTMoveMadeMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptTTTMoveMadeMsg);
	PYTHON_CLASS_IMPORT_END(m);
}