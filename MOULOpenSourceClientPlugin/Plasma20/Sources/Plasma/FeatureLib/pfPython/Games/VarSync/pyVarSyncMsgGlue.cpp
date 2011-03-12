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
#include "pyVarSyncMsg.h"
#include "../../pyEnum.h"

#include <python.h>

///////////////////////////////////////////////////////////////////////////////
//
// Base VarSync msg class
//

PYTHON_CLASS_DEFINITION(ptVarSyncMsg, pyVarSyncMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptVarSyncMsg, pyVarSyncMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVarSyncMsg)

PYTHON_NO_INIT_DEFINITION(ptVarSyncMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptVarSyncMsg, getVarSyncMsgType)
{
	return PyInt_FromLong(self->fThis->GetVarSyncMsgType());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVarSyncMsg, upcastToFinalVarSyncMsg)
{
	return self->fThis->UpcastToFinalVarSyncMsg();
}

PYTHON_START_METHODS_TABLE(ptVarSyncMsg)
	PYTHON_METHOD_NOARGS(ptVarSyncMsg, getVarSyncMsgType, "Returns the type of the VarSync message (see PtVarSyncMsgTypes)"),
	PYTHON_METHOD_NOARGS(ptVarSyncMsg, upcastToFinalVarSyncMsg, "Returns this message as the VarSync msg it is"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptVarSyncMsg, pyGameCliMsg, "Base class for VarSync game messages");
PYTHON_EXPOSE_TYPE_DEFINITION(ptVarSyncMsg, pyVarSyncMsg);

// required functions for PyObject interoperability
PyObject* pyVarSyncMsg::New(pfGameCliMsg* msg)
{
	ptVarSyncMsg *newObj = (ptVarSyncMsg*)ptVarSyncMsg_type.tp_new(&ptVarSyncMsg_type, NULL, NULL);
	if (msg && (msg->gameCli->GetGameTypeId() == kGameTypeId_VarSync))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptVarSyncMsg, pyVarSyncMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptVarSyncMsg, pyVarSyncMsg)

// Module and method definitions
void pyVarSyncMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptVarSyncMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

void pyVarSyncMsg::AddPlasmaConstantsClasses(PyObject* m)
{
	PYTHON_ENUM_START(PtVarSyncMsgTypes);
	PYTHON_ENUM_ELEMENT(PtVarSyncMsgTypes, kVarSyncStringVarChanged, kSrv2Cli_VarSync_StringVarChanged);
	PYTHON_ENUM_ELEMENT(PtVarSyncMsgTypes, kVarSyncNumericVarChanged, kSrv2Cli_VarSync_NumericVarChanged);
	PYTHON_ENUM_ELEMENT(PtVarSyncMsgTypes, kVarSyncAllVarsSent, kSrv2Cli_VarSync_AllVarsSent);
	PYTHON_ENUM_ELEMENT(PtVarSyncMsgTypes, kVarSyncStringVarCreated, kSrv2Cli_VarSync_StringVarCreated);
	PYTHON_ENUM_ELEMENT(PtVarSyncMsgTypes, kVarSyncNumericVarCreated, kSrv2Cli_VarSync_NumericVarCreated);
	PYTHON_ENUM_END(m, PtVarSyncMsgTypes);
}

///////////////////////////////////////////////////////////////////////////////
//
// Game client message subclasses
//

PYTHON_CLASS_DEFINITION(ptVarSyncStringVarChangedMsg, pyVarSyncStringVarChangedMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptVarSyncStringVarChangedMsg, pyVarSyncStringVarChangedMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVarSyncStringVarChangedMsg)

PYTHON_NO_INIT_DEFINITION(ptVarSyncStringVarChangedMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptVarSyncStringVarChangedMsg, id)
{
	return PyLong_FromUnsignedLong(self->fThis->ID());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVarSyncStringVarChangedMsg, value)
{
	std::wstring retVal = self->fThis->Value();
	return PyUnicode_FromWideChar(retVal.c_str(), retVal.size());
}

PYTHON_START_METHODS_TABLE(ptVarSyncStringVarChangedMsg)
	PYTHON_METHOD_NOARGS(ptVarSyncStringVarChangedMsg, id, "Returns the id of the var that changed"),
	PYTHON_METHOD_NOARGS(ptVarSyncStringVarChangedMsg, value, "Returns the variable's new value"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptVarSyncStringVarChangedMsg, pyVarSyncMsg, "VarSync message received when a string variable's value changes");

// required functions for PyObject interoperability
PyObject* pyVarSyncStringVarChangedMsg::New(pfGameCliMsg* msg)
{
	ptVarSyncStringVarChangedMsg *newObj = (ptVarSyncStringVarChangedMsg*)ptVarSyncStringVarChangedMsg_type.tp_new(&ptVarSyncStringVarChangedMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_VarSync_StringVarChanged))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptVarSyncStringVarChangedMsg, pyVarSyncStringVarChangedMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptVarSyncStringVarChangedMsg, pyVarSyncStringVarChangedMsg)

// Module and method definitions
void pyVarSyncStringVarChangedMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptVarSyncStringVarChangedMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////
PYTHON_CLASS_DEFINITION(ptVarSyncNumericVarChangedMsg, pyVarSyncNumericVarChangedMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptVarSyncNumericVarChangedMsg, pyVarSyncNumericVarChangedMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVarSyncNumericVarChangedMsg)

PYTHON_NO_INIT_DEFINITION(ptVarSyncNumericVarChangedMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptVarSyncNumericVarChangedMsg, id)
{
	return PyLong_FromUnsignedLong(self->fThis->ID());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVarSyncNumericVarChangedMsg, value)
{
	return PyLong_FromDouble(self->fThis->Value());
}

PYTHON_START_METHODS_TABLE(ptVarSyncNumericVarChangedMsg)
	PYTHON_METHOD_NOARGS(ptVarSyncNumericVarChangedMsg, id, "Returns the id of the var that changed"),
	PYTHON_METHOD_NOARGS(ptVarSyncNumericVarChangedMsg, value, "Returns the variable's new value"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptVarSyncNumericVarChangedMsg, pyVarSyncMsg, "VarSync message received when a numeric variable's value changes");

// required functions for PyObject interoperability
PyObject* pyVarSyncNumericVarChangedMsg::New(pfGameCliMsg* msg)
{
	ptVarSyncNumericVarChangedMsg *newObj = (ptVarSyncNumericVarChangedMsg*)ptVarSyncNumericVarChangedMsg_type.tp_new(&ptVarSyncNumericVarChangedMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_VarSync_NumericVarChanged))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptVarSyncNumericVarChangedMsg, pyVarSyncNumericVarChangedMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptVarSyncNumericVarChangedMsg, pyVarSyncNumericVarChangedMsg)

// Module and method definitions
void pyVarSyncNumericVarChangedMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptVarSyncNumericVarChangedMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////
PYTHON_CLASS_DEFINITION(ptVarSyncAllVarsSentMsg, pyVarSyncAllVarsSentMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptVarSyncAllVarsSentMsg, pyVarSyncAllVarsSentMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVarSyncAllVarsSentMsg)

PYTHON_NO_INIT_DEFINITION(ptVarSyncAllVarsSentMsg)

PYTHON_START_METHODS_TABLE(ptVarSyncAllVarsSentMsg)
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptVarSyncAllVarsSentMsg, pyVarSyncMsg, "VarSync message received after the last var is sent to you when you join the game, or request a list of vars");

// required functions for PyObject interoperability
PyObject* pyVarSyncAllVarsSentMsg::New(pfGameCliMsg* msg)
{
	ptVarSyncAllVarsSentMsg *newObj = (ptVarSyncAllVarsSentMsg*)ptVarSyncAllVarsSentMsg_type.tp_new(&ptVarSyncAllVarsSentMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_VarSync_AllVarsSent))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptVarSyncAllVarsSentMsg, pyVarSyncAllVarsSentMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptVarSyncAllVarsSentMsg, pyVarSyncAllVarsSentMsg)

// Module and method definitions
void pyVarSyncAllVarsSentMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptVarSyncAllVarsSentMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////
PYTHON_CLASS_DEFINITION(ptVarSyncStringVarCreatedMsg, pyVarSyncStringVarCreatedMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptVarSyncStringVarCreatedMsg, pyVarSyncStringVarCreatedMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVarSyncStringVarCreatedMsg)

PYTHON_NO_INIT_DEFINITION(ptVarSyncStringVarCreatedMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptVarSyncStringVarCreatedMsg, name)
{
	std::wstring retVal = self->fThis->Name();
	return PyUnicode_FromWideChar(retVal.c_str(), retVal.size());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVarSyncStringVarCreatedMsg, id)
{
	return PyLong_FromUnsignedLong(self->fThis->ID());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVarSyncStringVarCreatedMsg, value)
{
	std::wstring retVal = self->fThis->Value();
	return PyUnicode_FromWideChar(retVal.c_str(), retVal.size());
}

PYTHON_START_METHODS_TABLE(ptVarSyncStringVarCreatedMsg)
	PYTHON_METHOD_NOARGS(ptVarSyncStringVarCreatedMsg, name, "Returns the name of the var that was created"),
	PYTHON_METHOD_NOARGS(ptVarSyncStringVarCreatedMsg, id, "Returns the id that was assigned to this variable"),
	PYTHON_METHOD_NOARGS(ptVarSyncStringVarCreatedMsg, value, "Returns the variable's new value"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptVarSyncStringVarCreatedMsg, pyVarSyncMsg, "VarSync message received when a string variable is created and assigned an id");

// required functions for PyObject interoperability
PyObject* pyVarSyncStringVarCreatedMsg::New(pfGameCliMsg* msg)
{
	ptVarSyncStringVarCreatedMsg *newObj = (ptVarSyncStringVarCreatedMsg*)ptVarSyncStringVarCreatedMsg_type.tp_new(&ptVarSyncStringVarCreatedMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_VarSync_StringVarCreated))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptVarSyncStringVarCreatedMsg, pyVarSyncStringVarCreatedMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptVarSyncStringVarCreatedMsg, pyVarSyncStringVarCreatedMsg)

// Module and method definitions
void pyVarSyncStringVarCreatedMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptVarSyncStringVarCreatedMsg);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////
PYTHON_CLASS_DEFINITION(ptVarSyncNumericVarCreatedMsg, pyVarSyncNumericVarCreatedMsg);

PYTHON_DEFAULT_NEW_DEFINITION(ptVarSyncNumericVarCreatedMsg, pyVarSyncNumericVarCreatedMsg)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVarSyncNumericVarCreatedMsg)

PYTHON_NO_INIT_DEFINITION(ptVarSyncNumericVarCreatedMsg)

PYTHON_METHOD_DEFINITION_NOARGS(ptVarSyncNumericVarCreatedMsg, name)
{
	std::wstring retVal = self->fThis->Name();
	return PyUnicode_FromWideChar(retVal.c_str(), retVal.size());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVarSyncNumericVarCreatedMsg, id)
{
	return PyLong_FromUnsignedLong(self->fThis->ID());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVarSyncNumericVarCreatedMsg, value)
{
	return PyLong_FromDouble(self->fThis->Value());
}

PYTHON_START_METHODS_TABLE(ptVarSyncNumericVarCreatedMsg)
	PYTHON_METHOD_NOARGS(ptVarSyncNumericVarCreatedMsg, name, "Returns the name of the var that was created"),
	PYTHON_METHOD_NOARGS(ptVarSyncNumericVarCreatedMsg, id, "Returns the id assigned to this variable"),
	PYTHON_METHOD_NOARGS(ptVarSyncNumericVarCreatedMsg, value, "Returns the variable's new value"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptVarSyncNumericVarCreatedMsg, pyVarSyncMsg, "VarSync message received when a numeric variable is created and assigned an id");

// required functions for PyObject interoperability
PyObject* pyVarSyncNumericVarCreatedMsg::New(pfGameCliMsg* msg)
{
	ptVarSyncNumericVarCreatedMsg *newObj = (ptVarSyncNumericVarCreatedMsg*)ptVarSyncNumericVarCreatedMsg_type.tp_new(&ptVarSyncNumericVarCreatedMsg_type, NULL, NULL);
	if (msg && (msg->netMsg->messageId == kSrv2Cli_VarSync_NumericVarCreated))
		newObj->fThis->message = msg;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptVarSyncNumericVarCreatedMsg, pyVarSyncNumericVarCreatedMsg)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptVarSyncNumericVarCreatedMsg, pyVarSyncNumericVarCreatedMsg)

// Module and method definitions
void pyVarSyncNumericVarCreatedMsg::AddPlasmaClasses(PyObject* m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptVarSyncNumericVarCreatedMsg);
	PYTHON_CLASS_IMPORT_END(m);
}