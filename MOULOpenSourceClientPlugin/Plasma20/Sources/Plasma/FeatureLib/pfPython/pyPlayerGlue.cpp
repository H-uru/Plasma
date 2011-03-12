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
#include "pyPlayer.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptPlayer, pyPlayer);

PYTHON_DEFAULT_NEW_DEFINITION(ptPlayer, pyPlayer)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptPlayer)

PYTHON_INIT_DEFINITION(ptPlayer, args, keywords)
{
	// we have two sets of arguments we can use, hence the generic PyObject* pointers
	// argument set 1: pyKey, string, UInt32, hsScalar
	// argument set 2: string, UInt32
	PyObject* firstObj = NULL; // can be a pyKey or a string
	PyObject* secondObj = NULL; // can be a string or a UInt32
	PyObject* thirdObj = NULL; // UInt32
	PyObject* fourthObj = NULL; // hsScalar
	if (!PyArg_ParseTuple(args, "OO|OO", &firstObj, &secondObj, &thirdObj, &fourthObj))
	{
		PyErr_SetString(PyExc_TypeError, "__init__ expects one of two argument lists: (ptKey, string, unsigned long, float) or (string, unsigned long)");
		PYTHON_RETURN_INIT_ERROR;
	}
	if (pyKey::Check(firstObj)) // arg set 1
	{
		// make sure the remaining objects fit the arg list
		if ((!thirdObj) || (!fourthObj))
		{
			// missing arguments
			PyErr_SetString(PyExc_TypeError, "__init__ expects one of two argument lists: (ptKey, string, unsigned long, float) or (string, unsigned long)");
			PYTHON_RETURN_INIT_ERROR;
		}
		if ((!PyString_Check(secondObj)) || (!PyLong_Check(thirdObj)) || (!PyFloat_Check(fourthObj)))
		{
			// incorrect types
			PyErr_SetString(PyExc_TypeError, "__init__ expects one of two argument lists: (ptKey, string, unsigned long, float) or (string, unsigned long)");
			PYTHON_RETURN_INIT_ERROR;
		}
		// all args are correct, convert and init
		pyKey* key = pyKey::ConvertFrom(firstObj);
		char* name = PyString_AsString(secondObj);
		unsigned long pid = PyLong_AsUnsignedLong(thirdObj);
		float distsq = (float)PyFloat_AsDouble(fourthObj);
		self->fThis->Init(key->getKey(), name, pid, distsq);
		PYTHON_RETURN_INIT_OK;
	}
	else if (PyString_Check(firstObj)) // arg set 2
	{
		// make sure there are only two args
		if (thirdObj || fourthObj)
		{
			// too many arguments
			PyErr_SetString(PyExc_TypeError, "__init__ expects one of two argument lists: (ptKey, string, unsigned long, float) or (string, unsigned long)");
			PYTHON_RETURN_INIT_ERROR;
		}
		char* name = PyString_AsString(firstObj);
		unsigned long pid = 0;
		if (PyLong_Check(secondObj))
			pid = PyLong_AsUnsignedLong(secondObj);
		else if (PyInt_Check(secondObj))
			pid = (unsigned long)PyInt_AsLong(secondObj);
		else
		{
			// incorrect type
			PyErr_SetString(PyExc_TypeError, "__init__ expects one of two argument lists: (ptKey, string, unsigned long, float) or (string, unsigned long)");
			PYTHON_RETURN_INIT_ERROR;
		}
		self->fThis->Init(nil, name, pid, -1);
		PYTHON_RETURN_INIT_OK;
	}
	// some other args came in
	PyErr_SetString(PyExc_TypeError, "__init__ expects one of two argument lists: (ptKey, string, unsigned long, float) or (string, unsigned long)");
	PYTHON_RETURN_INIT_ERROR;
}

PYTHON_RICH_COMPARE_DEFINITION(ptPlayer, obj1, obj2, compareType)
{
	if ((obj1 == Py_None) || (obj2 == Py_None) || !pyPlayer::Check(obj1) || !pyPlayer::Check(obj2))
	{
		// if they aren't the same type, they don't match, obviously (we also never equal none)
		if (compareType == Py_EQ)
			PYTHON_RCOMPARE_FALSE;
		else if (compareType == Py_NE)
			PYTHON_RCOMPARE_TRUE;
		else
		{
			PyErr_SetString(PyExc_NotImplementedError, "invalid comparison for a ptPlayer object");
			PYTHON_RCOMPARE_ERROR;
		}
	}
	pyPlayer *player1 = pyPlayer::ConvertFrom(obj1);
	pyPlayer *player2 = pyPlayer::ConvertFrom(obj2);
	if (compareType == Py_EQ)
	{
		if ((*player1) == (*player2))
			PYTHON_RCOMPARE_TRUE;
		PYTHON_RCOMPARE_FALSE;
	}
	else if (compareType == Py_NE)
	{
		if ((*player1) != (*player2))
			PYTHON_RCOMPARE_TRUE;
		PYTHON_RCOMPARE_FALSE;
	}
	PyErr_SetString(PyExc_NotImplementedError, "invalid comparison for a ptPlayer object");
	PYTHON_RCOMPARE_ERROR;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptPlayer, getPlayerName)
{
	return PyString_FromString(self->fThis->GetPlayerName());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptPlayer, getPlayerID)
{
	return PyLong_FromUnsignedLong(self->fThis->GetPlayerID());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptPlayer, getDistanceSq)
{
	return PyFloat_FromDouble(self->fThis->GetDistSq());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptPlayer, isCCR)
{
	PYTHON_RETURN_BOOL(self->fThis->IsCCR());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptPlayer, isServer)
{
	PYTHON_RETURN_BOOL(self->fThis->IsServer());
}

PYTHON_START_METHODS_TABLE(ptPlayer)
	PYTHON_METHOD_NOARGS(ptPlayer, getPlayerName, "Returns the name of the player"),
	PYTHON_METHOD_NOARGS(ptPlayer, getPlayerID, "Returns the unique player ID"),
	PYTHON_METHOD_NOARGS(ptPlayer, getDistanceSq, "Returns the distance to remote player from local player"),
	PYTHON_METHOD_NOARGS(ptPlayer, isCCR, "Is this player a CCR?"),
	PYTHON_METHOD_NOARGS(ptPlayer, isServer, "Is this player a server?"),
PYTHON_END_METHODS_TABLE;

// type structure definition
#define ptPlayer_COMPARE		PYTHON_NO_COMPARE
#define ptPlayer_AS_NUMBER		PYTHON_NO_AS_NUMBER
#define ptPlayer_AS_SEQUENCE	PYTHON_NO_AS_SEQUENCE
#define ptPlayer_AS_MAPPING		PYTHON_NO_AS_MAPPING
#define ptPlayer_STR			PYTHON_NO_STR
#define ptPlayer_RICH_COMPARE	PYTHON_DEFAULT_RICH_COMPARE(ptPlayer)
#define ptPlayer_GETSET			PYTHON_NO_GETSET
#define ptPlayer_BASE			PYTHON_NO_BASE
PLASMA_CUSTOM_TYPE(ptPlayer, "Params: avkey,name,playerID,distanceSq\nAnd optionally __init__(name,playerID)");

// required functions for PyObject interoperability
PyObject *pyPlayer::New(pyKey& avKey, const char* pname, UInt32 pid, hsScalar distsq)
{
	ptPlayer *newObj = (ptPlayer*)ptPlayer_type.tp_new(&ptPlayer_type, NULL, NULL);
	newObj->fThis->Init(avKey.getKey(), pname, pid, distsq);
	return (PyObject*)newObj;
}

PyObject *pyPlayer::New(plKey avKey, const char* pname, UInt32 pid, hsScalar distsq)
{
	ptPlayer *newObj = (ptPlayer*)ptPlayer_type.tp_new(&ptPlayer_type, NULL, NULL);
	newObj->fThis->Init(avKey, pname, pid, distsq);
	return (PyObject*)newObj;
}

PyObject *pyPlayer::New(const char* pname, UInt32 pid)
{
	ptPlayer *newObj = (ptPlayer*)ptPlayer_type.tp_new(&ptPlayer_type, NULL, NULL);
	newObj->fThis->Init(nil, pname, pid, -1);
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptPlayer, pyPlayer)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptPlayer, pyPlayer)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyPlayer::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptPlayer);
	PYTHON_CLASS_IMPORT_END(m);
}