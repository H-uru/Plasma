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
#include "pyNetClientComm.h"
#include "../FeatureLib/pfPython/pyEnum.h"
#include "../FeatureLib/pfPython/pyNetServerSessionInfo.h"
#include "../FeatureLib/pfPython/pyAgeLinkStruct.h"

#include "../plNetCommon/plCreatePlayerFlags.h"
#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptNetClientComm, pyNetClientComm);

PYTHON_DEFAULT_NEW_DEFINITION(ptNetClientComm, pyNetClientComm)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptNetClientComm)

PYTHON_INIT_DEFINITION(ptNetClientComm, args, keywords)
{
	PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION(ptNetClientComm, initObj, args)
{
	char threaded = 1;
	int logLevel = 0;
	if (!PyArg_ParseTuple(args, "bi", &threaded, &logLevel))
	{
		PyErr_SetString(PyExc_TypeError, "initObj expects a boolean and an int");
		PYTHON_RETURN_ERROR;
	}
	return PyInt_FromLong(self->fThis->Init(threaded != 0, logLevel));
}

PYTHON_METHOD_DEFINITION(ptNetClientComm, fini, args)
{
	float flushMsgsSecs = 0;
	if (!PyArg_ParseTuple(args, "f", &flushMsgsSecs))
	{
		PyErr_SetString(PyExc_TypeError, "fini expects a float");
		PYTHON_RETURN_ERROR;
	}
	return PyInt_FromLong(self->fThis->Fini(flushMsgsSecs));
}

PYTHON_METHOD_DEFINITION_NOARGS(ptNetClientComm, update)
{
	return PyInt_FromLong(self->fThis->Update());
}

PYTHON_METHOD_DEFINITION(ptNetClientComm, setActiveServer, args)
{
	PyObject* arg1;
	int port = 0;
	if (!PyArg_ParseTuple(args, "O|i", &arg1, &port))
	{
		PyErr_SetString(PyExc_TypeError, "setActiveServer expects a string and an int, or a ptNetServerSessionInfo");
		PYTHON_RETURN_ERROR;
	}
	if (pyNetServerSessionInfo::Check(arg1))
	{
		pyNetServerSessionInfo* info = pyNetServerSessionInfo::ConvertFrom(arg1);
		return PyInt_FromLong(self->fThis->SetActiveServer(info));
	}
	else if (PyString_Check(arg1))
	{
		char* addr = PyString_AsString(arg1);
		return PyInt_FromLong(self->fThis->SetActiveServer2(addr, port));
	}
	PyErr_SetString(PyExc_TypeError, "setActiveServer expects a string and an int, or a ptNetServerSessionInfo");
	PYTHON_RETURN_ERROR;
}

PYTHON_METHOD_DEFINITION(ptNetClientComm, setAuthInfo, args)
{
	char* account;
	char* password;
	if (!PyArg_ParseTuple(args, "ss", &account, &password))
	{
		PyErr_SetString(PyExc_TypeError, "setAuthInfo expects two strings");
		PYTHON_RETURN_ERROR;
	}
	return PyInt_FromLong(self->fThis->SetAuthInfo(account, password));
}

PYTHON_METHOD_DEFINITION(ptNetClientComm, authenticate, args)
{
	double maxAuthSecs;
	PyObject* cb = NULL;
	unsigned long context = 0;
	if (!PyArg_ParseTuple(args, "d|Ol", &maxAuthSecs, &cb, &context))
	{
		PyErr_SetString(PyExc_TypeError, "authenticate expects a double, an optional object and an optional unsigned long");
		PYTHON_RETURN_ERROR;
	}
	return PyInt_FromLong(self->fThis->NetAuthenticate(maxAuthSecs, cb, context));
}

PYTHON_METHOD_DEFINITION(ptNetClientComm, getPlayerList, args)
{
	PyObject* cb = NULL;
	unsigned long context = 0;
	if (!PyArg_ParseTuple(args, "|Ol", &cb, &context))
	{
		PyErr_SetString(PyExc_TypeError, "getPlayerList expects an optional object and an optional unsigned long");
		PYTHON_RETURN_ERROR;
	}
	return PyInt_FromLong(self->fThis->NetGetPlayerList(cb, context));
}

PYTHON_METHOD_DEFINITION(ptNetClientComm, setActivePlayer, args)
{
	unsigned long playerID;
	char* playerName;
	PyObject* cb = NULL;
	unsigned long context = 0;
	if (!PyArg_ParseTuple(args, "ls|Ol", &playerID, &playerName, &cb, &context))
	{
		PyErr_SetString(PyExc_TypeError, "setActivePlayer expects a double, a string, an optional object and an optional unsigned long");
		PYTHON_RETURN_ERROR;
	}
	return PyInt_FromLong(self->fThis->NetSetActivePlayer(playerID, playerName, cb, context));
}

PYTHON_METHOD_DEFINITION(ptNetClientComm, createPlayer, args)
{
	char* playerName;
	char* avatarShape;
	unsigned long createFlags;
	PyObject* cb = NULL;
	unsigned long context = 0;
	if (!PyArg_ParseTuple(args, "ssl|Ol", &playerName, &avatarShape, &createFlags, &cb, &context))
	{
		PyErr_SetString(PyExc_TypeError, "createPlayer expects two strings, a double, an optional object and an optional unsigned long");
		PYTHON_RETURN_ERROR;
	}
	return PyInt_FromLong(self->fThis->NetCreatePlayer(playerName, avatarShape, createFlags, cb, context));
}

PYTHON_METHOD_DEFINITION(ptNetClientComm, findAge, args)
{
	PyObject* linkObj = NULL;
	PyObject* cb = NULL;
	unsigned long context = 0;
	if (!PyArg_ParseTuple(args, "O|Ol", &linkObj, &cb, &context))
	{
		PyErr_SetString(PyExc_TypeError, "findAge expects a ptAgeLinkStruct, an optional object and an optional unsigned long");
		PYTHON_RETURN_ERROR;
	}
	if (!pyAgeLinkStruct::Check(linkObj))
	{
		PyErr_SetString(PyExc_TypeError, "findAge expects a ptAgeLinkStruct, an optional object and an optional unsigned long");
		PYTHON_RETURN_ERROR;
	}
	pyAgeLinkStruct* link = pyAgeLinkStruct::ConvertFrom(linkObj);
	return PyInt_FromLong(self->fThis->NetFindAge(link, cb, context));
}

PYTHON_METHOD_DEFINITION(ptNetClientComm, joinAge, args)
{
	PyObject* cb = NULL;
	unsigned long context = 0;
	if (!PyArg_ParseTuple(args, "|Ol", &cb, &context))
	{
		PyErr_SetString(PyExc_TypeError, "joinAge expects an optional object and an optional unsigned long");
		PYTHON_RETURN_ERROR;
	}
	return PyInt_FromLong(self->fThis->NetJoinAge(cb, context));
}

PYTHON_METHOD_DEFINITION(ptNetClientComm, leave, args)
{
	unsigned char reason;
	PyObject* cb = NULL;
	unsigned long context = 0;
	if (!PyArg_ParseTuple(args, "b|Ol", &reason, &cb, &context))
	{
		PyErr_SetString(PyExc_TypeError, "leave expects an unsigned 8-bit int, an optional object and an optional unsigned long");
		PYTHON_RETURN_ERROR;
	}
	return PyInt_FromLong(self->fThis->NetLeave(reason, cb, context));
}

PYTHON_METHOD_DEFINITION(ptNetClientComm, ping, args)
{
	int serverType;
	int timeoutSecs = 0;
	PyObject* cb = NULL;
	unsigned long context = 0;
	if (!PyArg_ParseTuple(args, "i|iOl", &serverType, &timeoutSecs, &cb, &context))
	{
		PyErr_SetString(PyExc_TypeError, "ping expects an int, and optional int, an optional object and an optional unsigned long");
		PYTHON_RETURN_ERROR;
	}
	return PyInt_FromLong(self->fThis->NetPing(serverType, timeoutSecs, cb, context));
}

PYTHON_METHOD_DEFINITION(ptNetClientComm, setTimeout, args)
{
	float timeoutSecs;
	PyObject* cb = NULL;
	unsigned long context = 0;
	if (!PyArg_ParseTuple(args, "f|Ol", &timeoutSecs, &cb, &context))
	{
		PyErr_SetString(PyExc_TypeError, "setTimeout expects a float, an optional object and an optional unsigned long");
		PYTHON_RETURN_ERROR;
	}
	return PyInt_FromLong(self->fThis->NetSetTimeout(timeoutSecs, cb, context));
}

PYTHON_METHOD_DEFINITION(ptNetClientComm, setLog, args)
{
	char* name;
	unsigned long flags = 0;
	if (!PyArg_ParseTuple(args, "s|l", &name, &flags))
	{
		PyErr_SetString(PyExc_TypeError, "setLog expects a string and an optional unsigned long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetLogByName(name, flags);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptNetClientComm, getLog)
{
	return self->fThis->GetLog();
}

PYTHON_METHOD_DEFINITION(ptNetClientComm, setLogLevel, args)
{
	int level;
	if (!PyArg_ParseTuple(args, "i", &level))
	{
		PyErr_SetString(PyExc_TypeError, "setLogLevel expects an integer");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetLogLevel(level);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptNetClientComm, setServerSilenceTime, args)
{
	float secs;
	if (!PyArg_ParseTuple(args, "f", &secs))
	{
		PyErr_SetString(PyExc_TypeError, "setServerSilenceTime expects a float");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetServerSilenceTime(secs);
	PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptNetClientComm)
	PYTHON_METHOD(ptNetClientComm, initObj, "Params: threaded=1,logLevel=0\nInitialize this object"),
	PYTHON_METHOD(ptNetClientComm, fini, "Params: flushMsgsSecs=0\nFinalize this object"),
	PYTHON_METHOD_NOARGS(ptNetClientComm, update, "Update this object"),
	PYTHON_METHOD(ptNetClientComm, setActiveServer, "Params: addr,port\nAlso accepts a ptNetServerSessionInfo instead of address and port"),
	PYTHON_METHOD(ptNetClientComm, setAuthInfo, "Params: accountName,password\nSets the authentication info"),
	PYTHON_METHOD(ptNetClientComm, authenticate, "Params: maxAuthSecs,callback=None,cbContext=0\nAuthenticate with the server"),
	PYTHON_METHOD(ptNetClientComm, getPlayerList, "Params: callback=None,cbContext=0\nGets a list of players and uses the callback"),
	PYTHON_METHOD(ptNetClientComm, setActivePlayer, "Params: playerID,playerName,callback=None,cbContext=0\nSets the current active player"),
	PYTHON_METHOD(ptNetClientComm, createPlayer, "Params: playerName,avatarShape,createFlags,callback=None,cbContext=0\nCreates a new player"),
	PYTHON_METHOD(ptNetClientComm, findAge, "Params: ageLink,callback=None,cbContext=0\nFinds an age based on a ptAgeLinkStruct"),
	PYTHON_METHOD(ptNetClientComm, joinAge, "Params: callback=None,cbContext=0\nUNKNOWN"),
	PYTHON_METHOD(ptNetClientComm, leave, "Params: reason,callback=None,cbContext=0\nLeaves the lobby"),
	PYTHON_METHOD(ptNetClientComm, ping, "Params: serverType,timeoutSecs=0,callback=None,cbContext=0\nPings a server"),
	PYTHON_METHOD(ptNetClientComm, setTimeout, "Params: timeoutSecs,callback=None,cbContext=0\nSets the timeout duration"),
	PYTHON_METHOD(ptNetClientComm, setLog, "Params: name,flags=0\nUNKNOWN"),
	PYTHON_METHOD_NOARGS(ptNetClientComm, getLog, "UNKNOWN"),
	PYTHON_METHOD(ptNetClientComm, setLogLevel, "Params: level\nSets the logging level"),
	PYTHON_METHOD(ptNetClientComm, setServerSilenceTime, "Params: secs\nUNKNOWN"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptNetClientComm, "UNKNOWN");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptNetClientComm, pyNetClientComm)

PYTHON_CLASS_CHECK_IMPL(ptNetClientComm, pyNetClientComm)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptNetClientComm, pyNetClientComm)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyNetClientComm::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptNetClientComm);
	PYTHON_CLASS_IMPORT_END(m);

	PYTHON_ENUM_START(PtCreatePlayerFlags);
	PYTHON_ENUM_ELEMENT(PtCreatePlayerFlags, kDefaultFlags, plCreatePlayerFlags::kDefaultFlags);
	PYTHON_ENUM_ELEMENT(PtCreatePlayerFlags, kNoNeighborhood, plCreatePlayerFlags::kNoNeighborhood);
	PYTHON_ENUM_ELEMENT(PtCreatePlayerFlags, kNoCityLink, plCreatePlayerFlags::kNoCityLink);
	PYTHON_ENUM_END(m, PtCreatePlayerFlags);
}