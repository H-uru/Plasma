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
#include "pyVNodeMgr.h"
#include "../FeatureLib/pfPython/pyVaultNode.h"
#include "../pyNetClientComm/pyNetClientComm.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptVNodeMgr, pyVNodeMgr);

PYTHON_DEFAULT_NEW_DEFINITION(ptVNodeMgr, pyVNodeMgr)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVNodeMgr)

PYTHON_NO_INIT_DEFINITION(ptVNodeMgr)

PYTHON_METHOD_DEFINITION(ptVNodeMgr, update, args)
{
	double secs;
	if (!PyArg_ParseTuple(args, "d", &secs))
	{
		PyErr_SetString(PyExc_TypeError, "update expects a double");
		PYTHON_RETURN_ERROR;
	}
	return PyInt_FromLong(self->fThis->Update(secs));
}

PYTHON_BASIC_METHOD_DEFINITION(ptVNodeMgr, startup, Startup)
PYTHON_BASIC_METHOD_DEFINITION(ptVNodeMgr, shutdown, Shutdown)

PYTHON_METHOD_DEFINITION_NOARGS(ptVNodeMgr, isConnected)
{
	PYTHON_RETURN_BOOL(self->fThis->IsConnected());
}

PYTHON_METHOD_DEFINITION(ptVNodeMgr, disconnect, args)
{
	PyObject* cb = NULL;
	unsigned long context = 0;
	if (!PyArg_ParseTuple(args, "|Ol", &cb, &context))
	{
		PyErr_SetString(PyExc_TypeError, "disconnect expects an optional object, and an optional unsigned long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Disconnect(cb, context);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVNodeMgr, connect, args)
{
	int childFetchLevel = -1;
	PyObject* cb = NULL;
	unsigned long context = 0;
	if (!PyArg_ParseTuple(args, "|iOl", &childFetchLevel, &cb, &context))
	{
		PyErr_SetString(PyExc_TypeError, "connect expects an optional int, an optional object, and an optional unsigned long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Connect(childFetchLevel, cb, context);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVNodeMgr, fetchNode, args)
{
	unsigned long nodeID;
	int childFetchLevel = -1;
	PyObject* cb = NULL;
	unsigned long context = 0;
	if (!PyArg_ParseTuple(args, "l|iOl", &nodeID, &childFetchLevel, &cb, &context))
	{
		PyErr_SetString(PyExc_TypeError, "fetchNode expects an unsigned long, an optional int, an optional object, and an optional unsigned long");
		PYTHON_RETURN_ERROR;
	}
	PYTHON_RETURN_BOOL(self->fThis->FetchNode(nodeID, childFetchLevel, cb, context));
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVNodeMgr, getRootNode)
{
	return self->fThis->GetRootNode();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVNodeMgr, getClientID)
{
	return PyLong_FromUnsignedLong(self->fThis->GetClientID());
}

PYTHON_METHOD_DEFINITION(ptVNodeMgr, getNode, args)
{
	unsigned long nodeID;
	if (!PyArg_ParseTuple(args, "l", &nodeID))
	{
		PyErr_SetString(PyExc_TypeError, "getNode expects an unsigned long");
		PYTHON_RETURN_ERROR;
	}
	return self->fThis->GetNode(nodeID);
}

PYTHON_METHOD_DEFINITION(ptVNodeMgr, findNode, args)
{
	PyObject* templateObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &templateObj))
	{
		PyErr_SetString(PyExc_TypeError, "findNode expects a ptVaultNode");
		PYTHON_RETURN_ERROR;
	}
	if (!pyVaultNode::Check(templateObj))
	{
		PyErr_SetString(PyExc_TypeError, "findNode expects a ptVaultNode");
		PYTHON_RETURN_ERROR;
	}
	pyVaultNode* templateNode = pyVaultNode::ConvertFrom(templateObj);
	return self->fThis->FindNode(templateNode);
}

PYTHON_METHOD_DEFINITION(ptVNodeMgr, enableCallbacks, args)
{
	char enable;
	if (!PyArg_ParseTuple(args, "b", &enable))
	{
		PyErr_SetString(PyExc_TypeError, "enableCallbacks expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	PYTHON_RETURN_BOOL(self->fThis->EnableCallbacks(enable != 0));
}

PYTHON_METHOD_DEFINITION(ptVNodeMgr, addCallback, args)
{
	PyObject* cb = NULL;
	if (!PyArg_ParseTuple(args, "O", &cb))
	{
		PyErr_SetString(PyExc_TypeError, "addCallback expects an object");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->AddCallback(cb);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVNodeMgr, removeCallback, args)
{
	PyObject* cb = NULL;
	if (!PyArg_ParseTuple(args, "O", &cb))
	{
		PyErr_SetString(PyExc_TypeError, "removeCallback expects an object");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->RemoveCallback(cb);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVNodeMgr, createNode, args)
{
	int nodeType;
	bool persistent;
	if (!PyArg_ParseTuple(args, "ib", &nodeType, &persistent))
	{
		PyErr_SetString(PyExc_TypeError, "createNode expects an int and a boolean");
		PYTHON_RETURN_ERROR;
	}
	return self->fThis->CreateNode(nodeType, persistent != 0);
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVNodeMgr, getNetClient)
{
	return self->fThis->GetNetClient();
}

PYTHON_BASIC_METHOD_DEFINITION(ptVNodeMgr, dump, Dump)

PYTHON_START_METHODS_TABLE(ptVNodeMgr)
	PYTHON_METHOD(ptVNodeMgr, update, "Params: secs\nUNKNOWN"),
	PYTHON_BASIC_METHOD(ptVNodeMgr, startup, "UNKNOWN"),
	PYTHON_BASIC_METHOD(ptVNodeMgr, shutdown, "UNKNOWN"),
	PYTHON_METHOD_NOARGS(ptVNodeMgr, isConnected, "Are we connected to the vault?"),
	PYTHON_METHOD(ptVNodeMgr, disconnect, "Params: callback=None,cbContext=0\nDisconnects us from the vault"),
	PYTHON_METHOD(ptVNodeMgr, connect, "Params: childFetchLevel=-1,callback=None,cbContext=0\nConnects us ti the vault"),
	PYTHON_METHOD(ptVNodeMgr, fetchNode, "Params: nodeID,childFetchLevel=-1,callback=None,cbContext=0\nFetchs the specified node"),
	PYTHON_METHOD_NOARGS(ptVNodeMgr, getRootNode, "Returns the root node"),
	PYTHON_METHOD_NOARGS(ptVNodeMgr, getClientID, "Returns the client ID number"),
	PYTHON_METHOD(ptVNodeMgr, getNode, "Params: nodeID\nReturns the specified node"),
	PYTHON_METHOD(ptVNodeMgr, findNode, "Params: templateNode\nLocates a node matching the template"),
	PYTHON_METHOD(ptVNodeMgr, enableCallbacks, "Params: enable\nEnable/disable callbacks"),
	PYTHON_METHOD(ptVNodeMgr, addCallback, "Params: callback\nUNKNOWN"),
	PYTHON_METHOD(ptVNodeMgr, removeCallback, "Params: callback\nUNKNOWN"),
	PYTHON_METHOD(ptVNodeMgr, createNode, "Params: nodeType,persistent\nCreates a new node"),
	PYTHON_METHOD(ptVNodeMgr, getNetClient, "Returns our internal ptNetClientComm"),
	PYTHON_BASIC_METHOD(ptVNodeMgr, dump, "Dump contents to our log"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptVNodeMgr, "UNKNOWN");
PYTHON_EXPOSE_TYPE_DEFINITION(ptVNodeMgr, pyVNodeMgr);

// required functions for PyObject interoperability
PyObject *pyVNodeMgr::New(PyObject* thaComm)
{
	ptVNodeMgr *newObj = (ptVNodeMgr*)ptVNodeMgr_type.tp_new(&ptVNodeMgr_type, NULL, NULL);
	if (!pyNetClientComm::Check(thaComm))
	{
		Py_DECREF(newObj);
		return NULL; // bad parameter
	}
	newObj->fThis->setMyComm(thaComm);
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptVNodeMgr, pyVNodeMgr)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptVNodeMgr, pyVNodeMgr)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyVNodeMgr::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptVNodeMgr);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////

// glue functions
PYTHON_CLASS_DEFINITION(ptPlayerVNodeMgr, pyPlayerVNodeMgr);

PYTHON_DEFAULT_NEW_DEFINITION(ptPlayerVNodeMgr, pyPlayerVNodeMgr)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptPlayerVNodeMgr)

PYTHON_INIT_DEFINITION(ptPlayerVNodeMgr, args, keywords)
{
	PyObject* netClientComm = NULL;
	if (!PyArg_ParseTuple(args, "O", &netClientComm))
	{
		PyErr_SetString(PyExc_TypeError, "__init__ expects a ptNetClientComm");
		PYTHON_RETURN_INIT_ERROR;
	}
	if (!pyNetClientComm::Check(netClientComm))
	{
		PyErr_SetString(PyExc_TypeError, "__init__ expects a ptNetClientComm");
		PYTHON_RETURN_INIT_ERROR;
	}
	self->fThis->setMyComm(netClientComm);
	PYTHON_RETURN_INIT_OK;
}

PYTHON_START_METHODS_TABLE(ptPlayerVNodeMgr)
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptPlayerVNodeMgr, pyVNodeMgr, "UNKNOWN");

// required functions for PyObject interoperability
PyObject *pyPlayerVNodeMgr::New(PyObject* thaComm)
{
	ptPlayerVNodeMgr *newObj = (ptPlayerVNodeMgr*)ptPlayerVNodeMgr_type.tp_new(&ptPlayerVNodeMgr_type, NULL, NULL);
	if (!pyNetClientComm::Check(thaComm))
	{
		Py_DECREF(newObj);
		return NULL; // bad parameter
	}
	newObj->fThis->setMyComm(thaComm);
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptPlayerVNodeMgr, pyPlayerVNodeMgr)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptPlayerVNodeMgr, pyPlayerVNodeMgr)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyPlayerVNodeMgr::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptPlayerVNodeMgr);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////

// glue functions
PYTHON_CLASS_DEFINITION(ptAgeVNodeMgr, pyAgeVNodeMgr);

PYTHON_DEFAULT_NEW_DEFINITION(ptAgeVNodeMgr, pyAgeVNodeMgr)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptAgeVNodeMgr)

PYTHON_INIT_DEFINITION(ptAgeVNodeMgr, args, keywords)
{
	PyObject* netClientComm = NULL;
	if (!PyArg_ParseTuple(args, "O", &netClientComm))
	{
		PyErr_SetString(PyExc_TypeError, "__init__ expects a ptNetClientComm");
		PYTHON_RETURN_INIT_ERROR;
	}
	if (!pyNetClientComm::Check(netClientComm))
	{
		PyErr_SetString(PyExc_TypeError, "__init__ expects a ptNetClientComm");
		PYTHON_RETURN_INIT_ERROR;
	}
	self->fThis->setMyComm(netClientComm);
	PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION(ptAgeVNodeMgr, setAgeInfo, args)
{
	char* filename;
	char* guid;
	if (!PyArg_ParseTuple(args, "ss", &filename, &guid))
	{
		PyErr_SetString(PyExc_TypeError, "setAgeInfo expects two strings");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetAgeInfo(filename, guid);
	PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptAgeVNodeMgr)
	PYTHON_METHOD(ptAgeVNodeMgr, setAgeInfo, "Params: filename,guid\nUNKNOWN"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptAgeVNodeMgr, pyVNodeMgr, "UNKNOWN");

// required functions for PyObject interoperability
PyObject *pyAgeVNodeMgr::New(PyObject* thaComm)
{
	ptAgeVNodeMgr *newObj = (ptAgeVNodeMgr*)ptAgeVNodeMgr_type.tp_new(&ptAgeVNodeMgr_type, NULL, NULL);
	if (!pyNetClientComm::Check(thaComm))
	{
		Py_DECREF(newObj);
		return NULL; // bad parameter
	}
	newObj->fThis->setMyComm(thaComm);
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptAgeVNodeMgr, pyAgeVNodeMgr)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptAgeVNodeMgr, pyAgeVNodeMgr)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyAgeVNodeMgr::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptAgeVNodeMgr);
	PYTHON_CLASS_IMPORT_END(m);
}

///////////////////////////////////////////////////////////////////////////////

// glue functions
PYTHON_CLASS_DEFINITION(ptAdminVNodeMgr, pyAdminVNodeMgr);

PYTHON_DEFAULT_NEW_DEFINITION(ptAdminVNodeMgr, pyAdminVNodeMgr)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptAdminVNodeMgr)

PYTHON_INIT_DEFINITION(ptAdminVNodeMgr, args, keywords)
{
	PyObject* netClientComm = NULL;
	if (!PyArg_ParseTuple(args, "O", &netClientComm))
	{
		PyErr_SetString(PyExc_TypeError, "__init__ expects a ptNetClientComm");
		PYTHON_RETURN_INIT_ERROR;
	}
	if (!pyNetClientComm::Check(netClientComm))
	{
		PyErr_SetString(PyExc_TypeError, "__init__ expects a ptNetClientComm");
		PYTHON_RETURN_INIT_ERROR;
	}
	self->fThis->setMyComm(netClientComm);
	PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION(ptAdminVNodeMgr, setWantGlobalSDL, args)
{
	bool flag;
	if (!PyArg_ParseTuple(args, "b", &flag))
	{
		PyErr_SetString(PyExc_TypeError, "setWantGlobalSDL expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetWantGlobalSDL(flag != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAdminVNodeMgr, setWantAllPlayers, args)
{
	bool flag;
	if (!PyArg_ParseTuple(args, "b", &flag))
	{
		PyErr_SetString(PyExc_TypeError, "setWantAllPlayers expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetWantAllPlayers(flag != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAdminVNodeMgr, getGlobalInbox)
{
	return self->fThis->GetGlobalInbox();
}

PYTHON_START_METHODS_TABLE(ptAdminVNodeMgr)
	PYTHON_METHOD(ptAdminVNodeMgr, setWantGlobalSDL, "Params: flag\nUNKNOWN"),
	PYTHON_METHOD(ptAdminVNodeMgr, setWantAllPlayers, "Params: flag\nUNKNOWN"),
	PYTHON_METHOD_NOARGS(ptAdminVNodeMgr, getGlobalInbox, "UNKNOWN"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptAdminVNodeMgr, pyVNodeMgr, "UNKNOWN");

// required functions for PyObject interoperability
PyObject *pyAdminVNodeMgr::New(PyObject* thaComm)
{
	ptAdminVNodeMgr *newObj = (ptAdminVNodeMgr*)ptAdminVNodeMgr_type.tp_new(&ptAdminVNodeMgr_type, NULL, NULL);
	if (!pyNetClientComm::Check(thaComm))
	{
		Py_DECREF(newObj);
		return NULL; // bad parameter
	}
	newObj->fThis->setMyComm(thaComm);
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptAdminVNodeMgr, pyAdminVNodeMgr)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptAdminVNodeMgr, pyAdminVNodeMgr)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyAdminVNodeMgr::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptAdminVNodeMgr);
	PYTHON_CLASS_IMPORT_END(m);
}