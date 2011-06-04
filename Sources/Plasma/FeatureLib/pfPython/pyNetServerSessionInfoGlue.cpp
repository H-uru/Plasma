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
#include "pyNetServerSessionInfo.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptNetServerSessionInfo, pyNetServerSessionInfo);

PYTHON_DEFAULT_NEW_DEFINITION(ptNetServerSessionInfo, pyNetServerSessionInfo)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptNetServerSessionInfo)

PYTHON_INIT_DEFINITION(ptNetServerSessionInfo, args, keywords)
{
	PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION(ptNetServerSessionInfo, setServerName, args)
{
	char* name;
	if (!PyArg_ParseTuple(args, "s", &name))
	{
		PyErr_SetString(PyExc_TypeError, "setServerName expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetServerName(name);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptNetServerSessionInfo, setServerType, args)
{
	unsigned char serverType;
	if (!PyArg_ParseTuple(args, "b", &serverType))
	{
		PyErr_SetString(PyExc_TypeError, "setServerType expects an unsigned 8-bit int");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetServerType(serverType);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptNetServerSessionInfo, setServerAddr, args)
{
	char* addr;
	if (!PyArg_ParseTuple(args, "s", &addr))
	{
		PyErr_SetString(PyExc_TypeError, "setServerAddr expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetServerAddr(addr);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptNetServerSessionInfo, setServerPort, args)
{
	unsigned short port;
	if (!PyArg_ParseTuple(args, "h", &port))
	{
		PyErr_SetString(PyExc_TypeError, "setServerPort expects a unsigned short");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetServerPort(port);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptNetServerSessionInfo, setServerGuid, args)
{
	char* guid;
	if (!PyArg_ParseTuple(args, "s", &guid))
	{
		PyErr_SetString(PyExc_TypeError, "setServerGuid expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetServerGuid(guid);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptNetServerSessionInfo, hasServerName)
{
	PYTHON_RETURN_BOOL(self->fThis->HasServerName());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptNetServerSessionInfo, hasServerType)
{
	PYTHON_RETURN_BOOL(self->fThis->HasServerType());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptNetServerSessionInfo, hasServerAddr)
{
	PYTHON_RETURN_BOOL(self->fThis->HasServerAddr());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptNetServerSessionInfo, hasServerPort)
{
	PYTHON_RETURN_BOOL(self->fThis->HasServerPort());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptNetServerSessionInfo, hasServerGuid)
{
	PYTHON_RETURN_BOOL(self->fThis->HasServerGuid());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptNetServerSessionInfo, getServerName)
{
	return PyString_FromString(self->fThis->GetServerName());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptNetServerSessionInfo, getServerType)
{
	return PyInt_FromLong(self->fThis->GetServerType());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptNetServerSessionInfo, getServerAddr)
{
	return PyString_FromString(self->fThis->GetServerAddr());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptNetServerSessionInfo, getServerPort)
{
	return PyInt_FromLong(self->fThis->GetServerPort());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptNetServerSessionInfo, getServerGuid)
{
	return PyString_FromString(self->fThis->GetServerGuid());
}

PYTHON_START_METHODS_TABLE(ptNetServerSessionInfo)
	PYTHON_METHOD(ptNetServerSessionInfo, setServerName, "Params: name\nUNKNOWN"),
	PYTHON_METHOD(ptNetServerSessionInfo, setServerType, "Params: type\nUNKNOWN"),
	PYTHON_METHOD(ptNetServerSessionInfo, setServerAddr, "Params: addr\nUNKNOWN"),
	PYTHON_METHOD(ptNetServerSessionInfo, setServerPort, "Params: port\nUNKNOWN"),
	PYTHON_METHOD(ptNetServerSessionInfo, setServerGuid, "Params: guid\nUNKNOWN"),
	PYTHON_METHOD_NOARGS(ptNetServerSessionInfo, hasServerName, "UNKNOWN"),
	PYTHON_METHOD_NOARGS(ptNetServerSessionInfo, hasServerType, "UNKNOWN"),
	PYTHON_METHOD_NOARGS(ptNetServerSessionInfo, hasServerAddr, "UNKNOWN"),
	PYTHON_METHOD_NOARGS(ptNetServerSessionInfo, hasServerPort, "UNKNOWN"),
	PYTHON_METHOD_NOARGS(ptNetServerSessionInfo, hasServerGuid, "UNKNOWN"),
	PYTHON_METHOD_NOARGS(ptNetServerSessionInfo, getServerName, "UNKNOWN"),
	PYTHON_METHOD_NOARGS(ptNetServerSessionInfo, getServerType, "UNKNOWN"),
	PYTHON_METHOD_NOARGS(ptNetServerSessionInfo, getServerAddr, "UNKNOWN"),
	PYTHON_METHOD_NOARGS(ptNetServerSessionInfo, getServerPort, "UNKNOWN"),
	PYTHON_METHOD_NOARGS(ptNetServerSessionInfo, getServerGuid, "UNKNOWN"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptNetServerSessionInfo, "Basic server session info class");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptNetServerSessionInfo, pyNetServerSessionInfo)

PyObject *pyNetServerSessionInfo::New(const plNetServerSessionInfo &info)
{
	ptNetServerSessionInfo *newObj = (ptNetServerSessionInfo*)ptNetServerSessionInfo_type.tp_new(&ptNetServerSessionInfo_type, NULL, NULL);
	newObj->fThis->fInfo = info;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptNetServerSessionInfo, pyNetServerSessionInfo)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptNetServerSessionInfo, pyNetServerSessionInfo)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyNetServerSessionInfo::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptNetServerSessionInfo);
	PYTHON_CLASS_IMPORT_END(m);
}

// glue functions
PYTHON_CLASS_DEFINITION(ptNetServerSessionInfoRef, pyNetServerSessionInfoRef);

PYTHON_DEFAULT_NEW_DEFINITION(ptNetServerSessionInfoRef, pyNetServerSessionInfoRef)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptNetServerSessionInfoRef)

PYTHON_NO_INIT_DEFINITION(ptNetServerSessionInfoRef)

PYTHON_METHOD_DEFINITION(ptNetServerSessionInfoRef, setServerName, args)
{
	char* name;
	if (!PyArg_ParseTuple(args, "s", &name))
	{
		PyErr_SetString(PyExc_TypeError, "setServerName expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetServerName(name);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptNetServerSessionInfoRef, setServerType, args)
{
	unsigned char serverType;
	if (!PyArg_ParseTuple(args, "b", &serverType))
	{
		PyErr_SetString(PyExc_TypeError, "setServerType expects an unsigned 8-bit int");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetServerType(serverType);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptNetServerSessionInfoRef, setServerAddr, args)
{
	char* addr;
	if (!PyArg_ParseTuple(args, "s", &addr))
	{
		PyErr_SetString(PyExc_TypeError, "setServerAddr expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetServerAddr(addr);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptNetServerSessionInfoRef, setServerPort, args)
{
	unsigned short port;
	if (!PyArg_ParseTuple(args, "h", &port))
	{
		PyErr_SetString(PyExc_TypeError, "setServerPort expects a unsigned short");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetServerPort(port);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptNetServerSessionInfoRef, setServerGuid, args)
{
	char* guid;
	if (!PyArg_ParseTuple(args, "s", &guid))
	{
		PyErr_SetString(PyExc_TypeError, "setServerGuid expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetServerGuid(guid);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptNetServerSessionInfoRef, hasServerName)
{
	PYTHON_RETURN_BOOL(self->fThis->HasServerName());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptNetServerSessionInfoRef, hasServerType)
{
	PYTHON_RETURN_BOOL(self->fThis->HasServerType());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptNetServerSessionInfoRef, hasServerAddr)
{
	PYTHON_RETURN_BOOL(self->fThis->HasServerAddr());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptNetServerSessionInfoRef, hasServerPort)
{
	PYTHON_RETURN_BOOL(self->fThis->HasServerPort());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptNetServerSessionInfoRef, hasServerGuid)
{
	PYTHON_RETURN_BOOL(self->fThis->HasServerGuid());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptNetServerSessionInfoRef, getServerName)
{
	return PyString_FromString(self->fThis->GetServerName());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptNetServerSessionInfoRef, getServerType)
{
	return PyInt_FromLong(self->fThis->GetServerType());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptNetServerSessionInfoRef, getServerAddr)
{
	return PyString_FromString(self->fThis->GetServerAddr());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptNetServerSessionInfoRef, getServerPort)
{
	return PyInt_FromLong(self->fThis->GetServerPort());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptNetServerSessionInfoRef, getServerGuid)
{
	return PyString_FromString(self->fThis->GetServerGuid());
}

PYTHON_START_METHODS_TABLE(ptNetServerSessionInfoRef)
	PYTHON_METHOD(ptNetServerSessionInfoRef, setServerName, "Params: name\nUNKNOWN"),
	PYTHON_METHOD(ptNetServerSessionInfoRef, setServerType, "Params: type\nUNKNOWN"),
	PYTHON_METHOD(ptNetServerSessionInfoRef, setServerAddr, "Params: addr\nUNKNOWN"),
	PYTHON_METHOD(ptNetServerSessionInfoRef, setServerPort, "Params: port\nUNKNOWN"),
	PYTHON_METHOD(ptNetServerSessionInfoRef, setServerGuid, "Params: guid\nUNKNOWN"),
	PYTHON_METHOD_NOARGS(ptNetServerSessionInfoRef, hasServerName, "UNKNOWN"),
	PYTHON_METHOD_NOARGS(ptNetServerSessionInfoRef, hasServerType, "UNKNOWN"),
	PYTHON_METHOD_NOARGS(ptNetServerSessionInfoRef, hasServerAddr, "UNKNOWN"),
	PYTHON_METHOD_NOARGS(ptNetServerSessionInfoRef, hasServerPort, "UNKNOWN"),
	PYTHON_METHOD_NOARGS(ptNetServerSessionInfoRef, hasServerGuid, "UNKNOWN"),
	PYTHON_METHOD_NOARGS(ptNetServerSessionInfoRef, getServerName, "UNKNOWN"),
	PYTHON_METHOD_NOARGS(ptNetServerSessionInfoRef, getServerType, "UNKNOWN"),
	PYTHON_METHOD_NOARGS(ptNetServerSessionInfoRef, getServerAddr, "UNKNOWN"),
	PYTHON_METHOD_NOARGS(ptNetServerSessionInfoRef, getServerPort, "UNKNOWN"),
	PYTHON_METHOD_NOARGS(ptNetServerSessionInfoRef, getServerGuid, "UNKNOWN"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptNetServerSessionInfoRef, "Basic server session info class");

// required functions for PyObject interoperability
PyObject *pyNetServerSessionInfoRef::New(plNetServerSessionInfo &info)
{
	ptNetServerSessionInfoRef *newObj = (ptNetServerSessionInfoRef*)ptNetServerSessionInfoRef_type.tp_new(&ptNetServerSessionInfoRef_type, NULL, NULL);
	newObj->fThis->fInfo = info;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptNetServerSessionInfoRef, pyNetServerSessionInfoRef)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptNetServerSessionInfoRef, pyNetServerSessionInfoRef)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyNetServerSessionInfoRef::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptNetServerSessionInfoRef);
	PYTHON_CLASS_IMPORT_END(m);
}