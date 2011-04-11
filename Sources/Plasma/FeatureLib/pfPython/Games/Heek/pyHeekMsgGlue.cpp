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
#include "pyHeekMsg.h"
#include "../../pyEnum.h"

#include <python.h>

///////////////////////////////////////////////////////////////////////////////
//
// Base Heek msg class
//

PYTHON_CLASS_DEFINITION(ptHeekMsg, pyHeekMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptHeekMsg, pyHeekMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptHeekMsg)

PYTHON_NO_INIT_DEFINITION(ptHeekMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptHeekMsg, getHeekMsgType)
{
	return PyInt_FromLong(self->fThis->GetHeekMsgType());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptHeekMsg, upcastToFinalHeekMsg)
{
	return self->fThis->UpcastToFinalHeekMsg();
}

PYTHON_START_METHODS_TABLE(ptHeekMsg)
	PYTHON_METHOD_NOARGS(ptHeekMsg, getHeekMsgType, "Returns the type of the Heek message (see PtHeekMsgTypes)"),
	PYTHON_METHOD_NOARGS(ptHeekMsg, upcastToFinalHeekMsg, "Returns this message as the Heek message it is"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptHeekMsg, pyGameCliMsg, "Base class for Heek game messages");
PYTHON_EXPOSE_TYPE_DEFINITION(ptHeekMsg, pyHeekMsg);

// required functions for PyObject interoperability
PyObject* pyHeekMsg::New(pfGameCliMsg* msg)
{
	ptHeekMsg *newObj = (ptHeekMsg*)ptHeekMsg_type.tp_new(&ptHeekMsg_type, NULL, NULL);
	if (msg && (msg->gameCli->GetGameTypeId() == kGameTypeId_Heek))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptHeekMsg, pyHeekMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptHeekMsg, pyHeekMsg)

// Module and method definitions
void pyHeekMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptHeekMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

void pyHeekMsg::AddPlasmaConstantsClasses(PyObject* m)
{
	PYTHON_ENUM_START(PtHeekMsgTypes);
	PYTHON_ENUM_ELEMENT(PtHeekMsgTypes, kHeekPlayGame, kSrv2Cli_Heek_PlayGame);
	PYTHON_ENUM_ELEMENT(PtHeekMsgTypes, kHeekGoodbye, kSrv2Cli_Heek_Goodbye);
	PYTHON_ENUM_ELEMENT(PtHeekMsgTypes, kHeekWelcome, kSrv2Cli_Heek_Welcome);
	PYTHON_ENUM_ELEMENT(PtHeekMsgTypes, kHeekDrop, kSrv2Cli_Heek_Drop);
	PYTHON_ENUM_ELEMENT(PtHeekMsgTypes, kHeekSetup, kSrv2Cli_Heek_Setup);
	PYTHON_ENUM_ELEMENT(PtHeekMsgTypes, kHeekLightState, kSrv2Cli_Heek_LightState);
	PYTHON_ENUM_ELEMENT(PtHeekMsgTypes, kHeekInterfaceState, kSrv2Cli_Heek_InterfaceState);
	PYTHON_ENUM_ELEMENT(PtHeekMsgTypes, kHeekCountdownState, kSrv2Cli_Heek_CountdownState);
	PYTHON_ENUM_ELEMENT(PtHeekMsgTypes, kHeekWinLose, kSrv2Cli_Heek_WinLose);
	PYTHON_ENUM_ELEMENT(PtHeekMsgTypes, kHeekGameWin, kSrv2Cli_Heek_GameWin);
	PYTHON_ENUM_ELEMENT(PtHeekMsgTypes, kHeekPointUpdate, kSrv2Cli_Heek_PointUpdate);
	PYTHON_ENUM_END(m, PtHeekMsgTypes);
}

///////////////////////////////////////////////////////////////////////////////
//
// Game client message subclasses
//

PYTHON_CLASS_DEFINITION(ptHeekPlayGameMsg, pyHeekPlayGameMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptHeekPlayGameMsg, pyHeekPlayGameMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptHeekPlayGameMsg)

PYTHON_NO_INIT_DEFINITION(ptHeekPlayGameMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptHeekPlayGameMsg, isPlaying)
{
	PYTHON_RETURN_BOOL(self->fThis->IsPlaying());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptHeekPlayGameMsg, isSinglePlayer)
{
	PYTHON_RETURN_BOOL(self->fThis->IsSinglePlayer());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptHeekPlayGameMsg, enableButtons)
{
	PYTHON_RETURN_BOOL(self->fThis->EnableButtons())
}

PYTHON_START_METHODS_TABLE(ptHeekPlayGameMsg)
	PYTHON_METHOD_NOARGS(ptHeekPlayGameMsg, isPlaying, "Returns true if the server accepted the play game request"),
	PYTHON_METHOD_NOARGS(ptHeekPlayGameMsg, isSinglePlayer, "Returns true if you are the only player at the table"),
	PYTHON_METHOD_NOARGS(ptHeekPlayGameMsg, enableButtons, "Returns true if we should enable the buttons at the place we sat down"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptHeekPlayGameMsg, pyHeekMsg, "Heek message received when the server processes your play game request");

// required functions for PyObject interoperability
PyObject* pyHeekPlayGameMsg::New(pfGameCliMsg* msg)
{
	ptHeekPlayGameMsg *newObj = (ptHeekPlayGameMsg*)ptHeekPlayGameMsg_type.tp_new(&ptHeekPlayGameMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_Heek_PlayGame))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptHeekPlayGameMsg, pyHeekPlayGameMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptHeekPlayGameMsg, pyHeekPlayGameMsg)

// Module and method definitions
void pyHeekPlayGameMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptHeekPlayGameMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////
PYTHON_CLASS_DEFINITION(ptHeekGoodbyeMsg, pyHeekGoodbyeMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptHeekGoodbyeMsg, pyHeekGoodbyeMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptHeekGoodbyeMsg)

PYTHON_NO_INIT_DEFINITION(ptHeekGoodbyeMsg)

PYTHON_START_METHODS_TABLE(ptHeekGoodbyeMsg)
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptHeekGoodbyeMsg, pyHeekMsg, "Heek message received when the server processes leave request");

// required functions for PyObject interoperability
PyObject* pyHeekGoodbyeMsg::New(pfGameCliMsg* msg)
{
	ptHeekGoodbyeMsg *newObj = (ptHeekGoodbyeMsg*)ptHeekGoodbyeMsg_type.tp_new(&ptHeekGoodbyeMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_Heek_Goodbye))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptHeekGoodbyeMsg, pyHeekGoodbyeMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptHeekGoodbyeMsg, pyHeekGoodbyeMsg)

// Module and method definitions
void pyHeekGoodbyeMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptHeekGoodbyeMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////
PYTHON_CLASS_DEFINITION(ptHeekWelcomeMsg, pyHeekWelcomeMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptHeekWelcomeMsg, pyHeekWelcomeMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptHeekWelcomeMsg)

PYTHON_NO_INIT_DEFINITION(ptHeekWelcomeMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptHeekWelcomeMsg, points)
{
	return PyLong_FromUnsignedLong(self->fThis->Points());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptHeekWelcomeMsg, rank)
{
	return PyLong_FromUnsignedLong(self->fThis->Rank());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptHeekWelcomeMsg, name)
{
	std::wstring retVal = self->fThis->Name();
	return PyUnicode_FromWideChar(retVal.c_str(), retVal.length());
}

PYTHON_START_METHODS_TABLE(ptHeekWelcomeMsg)
	PYTHON_METHOD_NOARGS(ptHeekWelcomeMsg, points, "Returns the new player's points"),
	PYTHON_METHOD_NOARGS(ptHeekWelcomeMsg, rank, "Returns the new player's rank"),
	PYTHON_METHOD_NOARGS(ptHeekWelcomeMsg, name, "Returns the new player's name"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptHeekWelcomeMsg, pyHeekMsg, "Heek message received when a new player sits down");

// required functions for PyObject interoperability
PyObject* pyHeekWelcomeMsg::New(pfGameCliMsg* msg)
{
	ptHeekWelcomeMsg *newObj = (ptHeekWelcomeMsg*)ptHeekWelcomeMsg_type.tp_new(&ptHeekWelcomeMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_Heek_Welcome))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptHeekWelcomeMsg, pyHeekWelcomeMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptHeekWelcomeMsg, pyHeekWelcomeMsg)

// Module and method definitions
void pyHeekWelcomeMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptHeekWelcomeMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////
PYTHON_CLASS_DEFINITION(ptHeekDropMsg, pyHeekDropMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptHeekDropMsg, pyHeekDropMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptHeekDropMsg)

PYTHON_NO_INIT_DEFINITION(ptHeekDropMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptHeekDropMsg, position)
{
	return PyLong_FromUnsignedLong(self->fThis->Position());
}

PYTHON_START_METHODS_TABLE(ptHeekDropMsg)
	PYTHON_METHOD_NOARGS(ptHeekDropMsg, position, "Returns player position to cleanup and dump"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptHeekDropMsg, pyHeekMsg, "Heek message received when another player's position needs to be reset/modified");

// required functions for PyObject interoperability
PyObject* pyHeekDropMsg::New(pfGameCliMsg* msg)
{
	ptHeekDropMsg *newObj = (ptHeekDropMsg*)ptHeekDropMsg_type.tp_new(&ptHeekDropMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_Heek_Drop))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptHeekDropMsg, pyHeekDropMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptHeekDropMsg, pyHeekDropMsg)

// Module and method definitions
void pyHeekDropMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptHeekDropMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////
PYTHON_CLASS_DEFINITION(ptHeekSetupMsg, pyHeekSetupMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptHeekSetupMsg, pyHeekSetupMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptHeekSetupMsg)

PYTHON_NO_INIT_DEFINITION(ptHeekSetupMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptHeekSetupMsg, position)
{
	return PyInt_FromLong(self->fThis->Position());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptHeekSetupMsg, buttonState)
{
	PYTHON_RETURN_BOOL(self->fThis->ButtonState());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptHeekSetupMsg, lightOn)
{
	std::vector<bool> lights = self->fThis->LightOn();
	PyObject* retVal = PyList_New(lights.size());
	for (unsigned i = 0; i < lights.size(); ++i)
		PyList_SetItem(retVal, i, PyInt_FromLong(lights[i] ? 1 : 0));
	return retVal;
}

PYTHON_START_METHODS_TABLE(ptHeekSetupMsg)
	PYTHON_METHOD_NOARGS(ptHeekSetupMsg, position, "Returns the position this message is for"),
	PYTHON_METHOD_NOARGS(ptHeekSetupMsg, buttonState, "Returns whether the buttons are enabled or not"),
	PYTHON_METHOD_NOARGS(ptHeekSetupMsg, lightOn, "Returns a list of bools representing lights on or off"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptHeekSetupMsg, pyHeekMsg, "Heek message for setting up each position's state");

// required functions for PyObject interoperability
PyObject* pyHeekSetupMsg::New(pfGameCliMsg* msg)
{
	ptHeekSetupMsg *newObj = (ptHeekSetupMsg*)ptHeekSetupMsg_type.tp_new(&ptHeekSetupMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_Heek_Setup))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptHeekSetupMsg, pyHeekSetupMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptHeekSetupMsg, pyHeekSetupMsg)

// Module and method definitions
void pyHeekSetupMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptHeekSetupMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////
PYTHON_CLASS_DEFINITION(ptHeekLightStateMsg, pyHeekLightStateMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptHeekLightStateMsg, pyHeekLightStateMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptHeekLightStateMsg)

PYTHON_NO_INIT_DEFINITION(ptHeekLightStateMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptHeekLightStateMsg, lightNum)
{
	return PyInt_FromLong(self->fThis->LightNum());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptHeekLightStateMsg, state)
{
	return PyInt_FromLong(self->fThis->State());
}

PYTHON_START_METHODS_TABLE(ptHeekLightStateMsg)
	PYTHON_METHOD_NOARGS(ptHeekLightStateMsg, lightNum, "Returns the index of the light this refers to"),
	PYTHON_METHOD_NOARGS(ptHeekLightStateMsg, state, "Returns state the light should be switched to (see PtHeekLightStates)"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptHeekLightStateMsg, pyHeekMsg, "Heek message received when one of your local lights needs to change state");

// required functions for PyObject interoperability
PyObject* pyHeekLightStateMsg::New(pfGameCliMsg* msg)
{
	ptHeekLightStateMsg *newObj = (ptHeekLightStateMsg*)ptHeekLightStateMsg_type.tp_new(&ptHeekLightStateMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_Heek_LightState))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptHeekLightStateMsg, pyHeekLightStateMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptHeekLightStateMsg, pyHeekLightStateMsg)

// Module and method definitions
void pyHeekLightStateMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptHeekLightStateMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

void pyHeekLightStateMsg::AddPlasmaConstantsClasses(PyObject* m)
{
	PYTHON_ENUM_START(PtHeekLightStates);
	PYTHON_ENUM_ELEMENT(PtHeekLightStates, kHeekLightOn, kHeekLightOn);
	PYTHON_ENUM_ELEMENT(PtHeekLightStates, kHeekLightOff, kHeekLightOff);
	PYTHON_ENUM_ELEMENT(PtHeekLightStates, kHeekLightFlash, kHeekLightFlash);
	PYTHON_ENUM_END(m, PtHeekLightStates);
}

///////////////////////////////////////////////////////////////////////////////
PYTHON_CLASS_DEFINITION(ptHeekInterfaceStateMsg, pyHeekInterfaceStateMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptHeekInterfaceStateMsg, pyHeekInterfaceStateMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptHeekInterfaceStateMsg)

PYTHON_NO_INIT_DEFINITION(ptHeekInterfaceStateMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptHeekInterfaceStateMsg, buttonsEnabled)
{
	PYTHON_RETURN_BOOL(self->fThis->ButtonsEnabled());
}

PYTHON_START_METHODS_TABLE(ptHeekInterfaceStateMsg)
	PYTHON_METHOD_NOARGS(ptHeekInterfaceStateMsg, buttonsEnabled, "Returns whether your buttons should be enabled"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptHeekInterfaceStateMsg, pyHeekMsg, "Heek message received when your interface buttons need to enable or disable");

// required functions for PyObject interoperability
PyObject* pyHeekInterfaceStateMsg::New(pfGameCliMsg* msg)
{
	ptHeekInterfaceStateMsg *newObj = (ptHeekInterfaceStateMsg*)ptHeekInterfaceStateMsg_type.tp_new(&ptHeekInterfaceStateMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_Heek_InterfaceState))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptHeekInterfaceStateMsg, pyHeekInterfaceStateMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptHeekInterfaceStateMsg, pyHeekInterfaceStateMsg)

// Module and method definitions
void pyHeekInterfaceStateMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptHeekInterfaceStateMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////
PYTHON_CLASS_DEFINITION(ptHeekCountdownStateMsg, pyHeekCountdownStateMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptHeekCountdownStateMsg, pyHeekCountdownStateMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptHeekCountdownStateMsg)

PYTHON_NO_INIT_DEFINITION(ptHeekCountdownStateMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptHeekCountdownStateMsg, state)
{
	return PyInt_FromLong(self->fThis->State());
}

PYTHON_START_METHODS_TABLE(ptHeekCountdownStateMsg)
	PYTHON_METHOD_NOARGS(ptHeekCountdownStateMsg, state, "Returns state the countdown should be switched to (see PtHeekCountdownStates)"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptHeekCountdownStateMsg, pyHeekMsg, "Heek message received by game admin when the countdown state needs to change");

// required functions for PyObject interoperability
PyObject* pyHeekCountdownStateMsg::New(pfGameCliMsg* msg)
{
	ptHeekCountdownStateMsg *newObj = (ptHeekCountdownStateMsg*)ptHeekCountdownStateMsg_type.tp_new(&ptHeekCountdownStateMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_Heek_CountdownState))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptHeekCountdownStateMsg, pyHeekCountdownStateMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptHeekCountdownStateMsg, pyHeekCountdownStateMsg)

// Module and method definitions
void pyHeekCountdownStateMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptHeekCountdownStateMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

void pyHeekCountdownStateMsg::AddPlasmaConstantsClasses(PyObject* m)
{
	PYTHON_ENUM_START(PtHeekCountdownStates);
	PYTHON_ENUM_ELEMENT(PtHeekCountdownStates, kHeekCountdownStart, kHeekCountdownStart);
	PYTHON_ENUM_ELEMENT(PtHeekCountdownStates, kHeekCountdownStop, kHeekCountdownStop);
	PYTHON_ENUM_ELEMENT(PtHeekCountdownStates, kHeekCountdownIdle, kHeekCountdownIdle);
	PYTHON_ENUM_END(m, PtHeekCountdownStates);
}

///////////////////////////////////////////////////////////////////////////////
PYTHON_CLASS_DEFINITION(ptHeekWinLoseMsg, pyHeekWinLoseMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptHeekWinLoseMsg, pyHeekWinLoseMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptHeekWinLoseMsg)

PYTHON_NO_INIT_DEFINITION(ptHeekWinLoseMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptHeekWinLoseMsg, win)
{
	PYTHON_RETURN_BOOL(self->fThis->Win());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptHeekWinLoseMsg, choice)
{
	return PyInt_FromLong(self->fThis->Choice());
}

PYTHON_START_METHODS_TABLE(ptHeekWinLoseMsg)
	PYTHON_METHOD_NOARGS(ptHeekWinLoseMsg, win, "Returns true if you won"),
	PYTHON_METHOD_NOARGS(ptHeekWinLoseMsg, choice, "Returns the choice that won or lost (see PtHeekGameChoice)"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptHeekWinLoseMsg, pyHeekMsg, "Heek message received when the round is over and you won or lost");

// required functions for PyObject interoperability
PyObject* pyHeekWinLoseMsg::New(pfGameCliMsg* msg)
{
	ptHeekWinLoseMsg *newObj = (ptHeekWinLoseMsg*)ptHeekWinLoseMsg_type.tp_new(&ptHeekWinLoseMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_Heek_WinLose))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptHeekWinLoseMsg, pyHeekWinLoseMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptHeekWinLoseMsg, pyHeekWinLoseMsg)

// Module and method definitions
void pyHeekWinLoseMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptHeekWinLoseMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////
PYTHON_CLASS_DEFINITION(ptHeekGameWinMsg, pyHeekGameWinMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptHeekGameWinMsg, pyHeekGameWinMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptHeekGameWinMsg)

PYTHON_NO_INIT_DEFINITION(ptHeekGameWinMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptHeekGameWinMsg, choice)
{
	return PyInt_FromLong(self->fThis->Choice());
}

PYTHON_START_METHODS_TABLE(ptHeekGameWinMsg)
	PYTHON_METHOD_NOARGS(ptHeekGameWinMsg, choice, "Returns the choice that won (see PtHeekGameChoice)"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptHeekGameWinMsg, pyHeekMsg, "Heek message received by game admin when a game is won");

// required functions for PyObject interoperability
PyObject* pyHeekGameWinMsg::New(pfGameCliMsg* msg)
{
	ptHeekGameWinMsg *newObj = (ptHeekGameWinMsg*)ptHeekGameWinMsg_type.tp_new(&ptHeekGameWinMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_Heek_GameWin))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptHeekGameWinMsg, pyHeekGameWinMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptHeekGameWinMsg, pyHeekGameWinMsg)

// Module and method definitions
void pyHeekGameWinMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptHeekGameWinMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////
PYTHON_CLASS_DEFINITION(ptHeekPointUpdateMsg, pyHeekPointUpdateMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptHeekPointUpdateMsg, pyHeekPointUpdateMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptHeekPointUpdateMsg)

PYTHON_NO_INIT_DEFINITION(ptHeekPointUpdateMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptHeekPointUpdateMsg, displayUpdate)
{
	PYTHON_RETURN_BOOL(self->fThis->DisplayUpdate());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptHeekPointUpdateMsg, points)
{
	return PyLong_FromUnsignedLong(self->fThis->Points());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptHeekPointUpdateMsg, rank)
{
	return PyLong_FromUnsignedLong(self->fThis->Rank());
}

PYTHON_START_METHODS_TABLE(ptHeekPointUpdateMsg)
	PYTHON_METHOD_NOARGS(ptHeekPointUpdateMsg, displayUpdate, "Returns whether you should display a message to the user"),
	PYTHON_METHOD_NOARGS(ptHeekPointUpdateMsg, points, "Returns your new amount of points"),
	PYTHON_METHOD_NOARGS(ptHeekPointUpdateMsg, rank, "Returns your new rank"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptHeekPointUpdateMsg, pyHeekMsg, "Heek message received when the number of points you have needs to be changed");

// required functions for PyObject interoperability
PyObject* pyHeekPointUpdateMsg::New(pfGameCliMsg* msg)
{
	ptHeekPointUpdateMsg *newObj = (ptHeekPointUpdateMsg*)ptHeekPointUpdateMsg_type.tp_new(&ptHeekPointUpdateMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_Heek_PointUpdate))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptHeekPointUpdateMsg, pyHeekPointUpdateMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptHeekPointUpdateMsg, pyHeekPointUpdateMsg)

// Module and method definitions
void pyHeekPointUpdateMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptHeekPointUpdateMsg);
	PYTHON_CLASS_IMPORT_END(m);
}