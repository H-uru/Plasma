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
#include "pyBlueSpiralMsg.h"
#include "../../pyEnum.h"

#include <python.h>

///////////////////////////////////////////////////////////////////////////////
//
// Base TTT msg class
//

PYTHON_CLASS_DEFINITION(ptBlueSpiralMsg, pyBlueSpiralMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptBlueSpiralMsg, pyBlueSpiralMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptBlueSpiralMsg)

PYTHON_NO_INIT_DEFINITION(ptBlueSpiralMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptBlueSpiralMsg, getBlueSpiralMsgType)
{
	return PyInt_FromLong(self->fThis->GetBlueSpiralMsgType());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptBlueSpiralMsg, upcastToFinalBlueSpiralMsg)
{
	return self->fThis->UpcastToFinalBlueSpiralMsg();
}

PYTHON_START_METHODS_TABLE(ptBlueSpiralMsg)
	PYTHON_METHOD_NOARGS(ptBlueSpiralMsg, getBlueSpiralMsgType, "Returns the type of the BlueSpiral message (see PtBlueSpiralMsgTypes)"),
	PYTHON_METHOD_NOARGS(ptBlueSpiralMsg, upcastToFinalBlueSpiralMsg, "Returns this message as the BlueSpiral message it really is"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptBlueSpiralMsg, pyGameCliMsg, "Base class for BlueSpiral game messages");
PYTHON_EXPOSE_TYPE_DEFINITION(ptBlueSpiralMsg, pyBlueSpiralMsg);

// required functions for PyObject interoperability
PyObject* pyBlueSpiralMsg::New(pfGameCliMsg* msg)
{
	ptBlueSpiralMsg *newObj = (ptBlueSpiralMsg*)ptBlueSpiralMsg_type.tp_new(&ptBlueSpiralMsg_type, NULL, NULL);
	if (msg && (msg->gameCli->GetGameTypeId() == kGameTypeId_BlueSpiral))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptBlueSpiralMsg, pyBlueSpiralMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptBlueSpiralMsg, pyBlueSpiralMsg)

// Module and method definitions
void pyBlueSpiralMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptBlueSpiralMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

void pyBlueSpiralMsg::AddPlasmaConstantsClasses(PyObject* m)
{
	PYTHON_ENUM_START(PtBlueSpiralMsgTypes);
	PYTHON_ENUM_ELEMENT(PtBlueSpiralMsgTypes, kBlueSpiralClothOrder, kSrv2Cli_BlueSpiral_ClothOrder);
	PYTHON_ENUM_ELEMENT(PtBlueSpiralMsgTypes, kBlueSpiralSuccessfulHit, kSrv2Cli_BlueSpiral_SuccessfulHit);
	PYTHON_ENUM_ELEMENT(PtBlueSpiralMsgTypes, kBlueSpiralGameWon, kSrv2Cli_BlueSpiral_GameWon);
	PYTHON_ENUM_ELEMENT(PtBlueSpiralMsgTypes, kBlueSpiralGameOver, kSrv2Cli_BlueSpiral_GameOver);
	PYTHON_ENUM_ELEMENT(PtBlueSpiralMsgTypes, kBlueSpiralGameStarted, kSrv2Cli_BlueSpiral_GameStarted);
	PYTHON_ENUM_END(m, PtBlueSpiralMsgTypes);
}

///////////////////////////////////////////////////////////////////////////////
//
// Game client message subclasses
//

PYTHON_CLASS_DEFINITION(ptBlueSpiralClothOrderMsg, pyBlueSpiralClothOrderMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptBlueSpiralClothOrderMsg, pyBlueSpiralClothOrderMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptBlueSpiralClothOrderMsg)

PYTHON_NO_INIT_DEFINITION(ptBlueSpiralClothOrderMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptBlueSpiralClothOrderMsg, order)
{
	std::vector<int> order = self->fThis->Order();
	PyObject* retVal = PyList_New(order.size());
	for (unsigned i = 0; i < order.size(); ++i)
		PyList_SetItem(retVal, i, PyInt_FromLong(order[i]));
	return retVal;
}

PYTHON_START_METHODS_TABLE(ptBlueSpiralClothOrderMsg)
	PYTHON_METHOD_NOARGS(ptBlueSpiralClothOrderMsg, order, "Returns a list of numbers indicating the correct order to hit the clothes in"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptBlueSpiralClothOrderMsg, pyBlueSpiralMsg, "BlueSpiral message received when the game is started and the cloth order is set");

// required functions for PyObject interoperability
PyObject* pyBlueSpiralClothOrderMsg::New(pfGameCliMsg* msg)
{
	ptBlueSpiralClothOrderMsg *newObj = (ptBlueSpiralClothOrderMsg*)ptBlueSpiralClothOrderMsg_type.tp_new(&ptBlueSpiralClothOrderMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_BlueSpiral_ClothOrder))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptBlueSpiralClothOrderMsg, pyBlueSpiralClothOrderMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptBlueSpiralClothOrderMsg, pyBlueSpiralClothOrderMsg)

// Module and method definitions
void pyBlueSpiralClothOrderMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptBlueSpiralClothOrderMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////
PYTHON_CLASS_DEFINITION(ptBlueSpiralSuccessfulHitMsg, pyBlueSpiralSuccessfulHitMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptBlueSpiralSuccessfulHitMsg, pyBlueSpiralSuccessfulHitMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptBlueSpiralSuccessfulHitMsg)

PYTHON_NO_INIT_DEFINITION(ptBlueSpiralSuccessfulHitMsg)

PYTHON_START_METHODS_TABLE(ptBlueSpiralSuccessfulHitMsg)
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptBlueSpiralSuccessfulHitMsg, pyBlueSpiralMsg, "BlueSpiral message received when a cloth is hit in the correct order");

// required functions for PyObject interoperability
PyObject* pyBlueSpiralSuccessfulHitMsg::New(pfGameCliMsg* msg)
{
	ptBlueSpiralSuccessfulHitMsg *newObj = (ptBlueSpiralSuccessfulHitMsg*)ptBlueSpiralSuccessfulHitMsg_type.tp_new(&ptBlueSpiralSuccessfulHitMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_BlueSpiral_SuccessfulHit))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptBlueSpiralSuccessfulHitMsg, pyBlueSpiralSuccessfulHitMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptBlueSpiralSuccessfulHitMsg, pyBlueSpiralSuccessfulHitMsg)

// Module and method definitions
void pyBlueSpiralSuccessfulHitMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptBlueSpiralSuccessfulHitMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////
PYTHON_CLASS_DEFINITION(ptBlueSpiralGameWonMsg, pyBlueSpiralGameWonMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptBlueSpiralGameWonMsg, pyBlueSpiralGameWonMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptBlueSpiralGameWonMsg)

PYTHON_NO_INIT_DEFINITION(ptBlueSpiralGameWonMsg)

PYTHON_START_METHODS_TABLE(ptBlueSpiralGameWonMsg)
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptBlueSpiralGameWonMsg, pyBlueSpiralMsg, "BlueSpiral message received when the last cloth is successfully hit");

// required functions for PyObject interoperability
PyObject* pyBlueSpiralGameWonMsg::New(pfGameCliMsg* msg)
{
	ptBlueSpiralGameWonMsg *newObj = (ptBlueSpiralGameWonMsg*)ptBlueSpiralGameWonMsg_type.tp_new(&ptBlueSpiralGameWonMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_BlueSpiral_GameWon))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptBlueSpiralGameWonMsg, pyBlueSpiralGameWonMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptBlueSpiralGameWonMsg, pyBlueSpiralGameWonMsg)

// Module and method definitions
void pyBlueSpiralGameWonMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptBlueSpiralGameWonMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////
PYTHON_CLASS_DEFINITION(ptBlueSpiralGameOverMsg, pyBlueSpiralGameOverMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptBlueSpiralGameOverMsg, pyBlueSpiralGameOverMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptBlueSpiralGameOverMsg)

PYTHON_NO_INIT_DEFINITION(ptBlueSpiralGameOverMsg)

PYTHON_START_METHODS_TABLE(ptBlueSpiralGameOverMsg)
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptBlueSpiralGameOverMsg, pyBlueSpiralMsg, "BlueSpiral message received when the timer runs out, someone hits the wrong cloth, or the game is restarted (before a game start msg in that last case)");

// required functions for PyObject interoperability
PyObject* pyBlueSpiralGameOverMsg::New(pfGameCliMsg* msg)
{
	ptBlueSpiralGameOverMsg *newObj = (ptBlueSpiralGameOverMsg*)ptBlueSpiralGameOverMsg_type.tp_new(&ptBlueSpiralGameOverMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_BlueSpiral_GameOver))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptBlueSpiralGameOverMsg, pyBlueSpiralGameOverMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptBlueSpiralGameOverMsg, pyBlueSpiralGameOverMsg)

// Module and method definitions
void pyBlueSpiralGameOverMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptBlueSpiralGameOverMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////
PYTHON_CLASS_DEFINITION(ptBlueSpiralGameStartedMsg, pyBlueSpiralGameStartedMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptBlueSpiralGameStartedMsg, pyBlueSpiralGameStartedMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptBlueSpiralGameStartedMsg)

PYTHON_NO_INIT_DEFINITION(ptBlueSpiralGameStartedMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptBlueSpiralGameStartedMsg, startSpin)
{
	PYTHON_RETURN_BOOL(self->fThis->StartSpin());
}

PYTHON_START_METHODS_TABLE(ptBlueSpiralGameStartedMsg)
	PYTHON_METHOD_NOARGS(ptBlueSpiralGameStartedMsg, startSpin, "Returns true if you are supposed to start spinning the door thingy"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptBlueSpiralGameStartedMsg, pyBlueSpiralMsg, "BlueSpiral message received when someone starts the game (or when you join a game that is running)");

// required functions for PyObject interoperability
PyObject* pyBlueSpiralGameStartedMsg::New(pfGameCliMsg* msg)
{
	ptBlueSpiralGameStartedMsg *newObj = (ptBlueSpiralGameStartedMsg*)ptBlueSpiralGameStartedMsg_type.tp_new(&ptBlueSpiralGameStartedMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_BlueSpiral_GameStarted))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptBlueSpiralGameStartedMsg, pyBlueSpiralGameStartedMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptBlueSpiralGameStartedMsg, pyBlueSpiralGameStartedMsg)

// Module and method definitions
void pyBlueSpiralGameStartedMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptBlueSpiralGameStartedMsg);
	PYTHON_CLASS_IMPORT_END(m);
}