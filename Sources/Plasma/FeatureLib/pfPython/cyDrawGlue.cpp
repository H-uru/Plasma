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
#include "cyDraw.h"
#include "pyKey.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptDraw, cyDraw);

PYTHON_DEFAULT_NEW_DEFINITION(ptDraw, cyDraw)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptDraw)

PYTHON_NO_INIT_DEFINITION(ptDraw)

PYTHON_METHOD_DEFINITION(ptDraw, netForce, args)
{
	char forceFlag;
	if (!PyArg_ParseTuple(args, "b", &forceFlag))
	{
		PyErr_SetString(PyExc_TypeError, "netForce requires a boolean argument");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetNetForce(forceFlag != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptDraw, enable, args)
{
	char state = 1;
	if (!PyArg_ParseTuple(args, "|b", &state))
	{
		PyErr_SetString(PyExc_TypeError, "enable expects an optional boolean argument");
		PYTHON_RETURN_ERROR;
	}

	self->fThis->EnableT(state != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptDraw, disable, Disable)

PYTHON_START_METHODS_TABLE(ptDraw)
	PYTHON_METHOD(ptDraw, netForce, "Params: forceFlag\nSpecify whether this object needs to use messages that are forced to the network\n"
				"- This is to be used if your Python program is running on only one client\n"
				"Such as a game master, only running on the client that owns a particular object"),
	PYTHON_METHOD(ptDraw, enable, "Params: state=1\nSets the draw enable for the sceneobject attached"),
	PYTHON_BASIC_METHOD(ptDraw, disable, "Disables the draw on the sceneobject attached\n"
				"In other words, makes it invisible"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptDraw, "Plasma Draw class");

// required functions for PyObject interoperability
PyObject *cyDraw::New(PyObject *sender, PyObject *recvr)
{
	ptDraw *newObj = (ptDraw*)ptDraw_type.tp_new(&ptDraw_type, NULL, NULL);
	if (sender != NULL)
	{
		pyKey *senderKey = pyKey::ConvertFrom(sender);
		newObj->fThis->SetSender(senderKey->getKey());
	}
	if (recvr != NULL)
	{
		pyKey *recvrKey = pyKey::ConvertFrom(recvr);
		newObj->fThis->AddRecvr(recvrKey->getKey());
	}
	newObj->fThis->fNetForce = false;

	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptDraw, cyDraw)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptDraw, cyDraw)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void cyDraw::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptDraw);
	PYTHON_CLASS_IMPORT_END(m);
}