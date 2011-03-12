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
#include "plPythonSDLModifier.h"
#include "pyKey.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptSDL, pySDLModifier);

PYTHON_DEFAULT_NEW_DEFINITION(ptSDL, pySDLModifier)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptSDL)

PYTHON_INIT_DEFINITION(ptSDL, args, keywords)
{
	PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION(ptSDL, setIndex, args)
{
	char* key;
	int idx;
	PyObject* value = NULL;
	if (!PyArg_ParseTuple(args, "siO", &key, &idx, &value))
	{
		PyErr_SetString(PyExc_TypeError, "setIndex expects a string, int, and an object");
		PYTHON_RETURN_ERROR;
	}
	pySDLModifier::SetItemIdx(*(self->fThis), key, idx, value);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptSDL, setIndexNow, args)
{
	char* key;
	int idx;
	PyObject* value = NULL;
	if (!PyArg_ParseTuple(args, "siO", &key, &idx, &value))
	{
		PyErr_SetString(PyExc_TypeError, "setIndexNow expects a string, int, and an object");
		PYTHON_RETURN_ERROR;
	}
	pySDLModifier::SetItemIdxImmediate(*(self->fThis), key, idx, value);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptSDL, setDefault, args)
{
	char* key;
	PyObject* value = NULL;
	if (!PyArg_ParseTuple(args, "sO", &key, &value))
	{
		PyErr_SetString(PyExc_TypeError, "setDefault expects a string and a tuple");
		PYTHON_RETURN_ERROR;
	}
	if (!PyTuple_Check(value))
	{
		PyErr_SetString(PyExc_TypeError, "setDefault expects a string and a tuple");
		PYTHON_RETURN_ERROR;
	}
	pySDLModifier::SetDefault(*(self->fThis), key, value);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptSDL, sendToClients, args)
{
	char* key;
	if (!PyArg_ParseTuple(args, "s", &key))
	{
		PyErr_SetString(PyExc_TypeError, "sendToClients expects a string");
		PYTHON_RETURN_ERROR;
	}
	pySDLModifier::SendToClients(*(self->fThis), key);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptSDL, setNotify, args)
{
	PyObject* selfKeyObj;
	char* key;
	float tolerance;
	if (!PyArg_ParseTuple(args, "Osf", &selfKeyObj, &key, &tolerance))
	{
		PyErr_SetString(PyExc_TypeError, "setNotify expects a ptKey, string, and float");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(selfKeyObj))
	{
		PyErr_SetString(PyExc_TypeError, "setNotify expects a ptKey, string, and float");
		PYTHON_RETURN_ERROR;
	}
	pyKey* selfKey = pyKey::ConvertFrom(selfKeyObj);
	pySDLModifier::SetNotify(*(self->fThis), *selfKey, key, tolerance);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptSDL, setFlags, args)
{
	char* key;
	char sendImmediate, skipOwnershipCheck;
	if (!PyArg_ParseTuple(args, "sbb", &key, &sendImmediate, &skipOwnershipCheck))
	{
		PyErr_SetString(PyExc_TypeError, "setFlags expects a string and two booleans");
		PYTHON_RETURN_ERROR;
	}
	pySDLModifier::SetFlags(*(self->fThis), key, sendImmediate != 0, skipOwnershipCheck != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptSDL, setTagString, args)
{
	char* key;
	char* tag;
	if (!PyArg_ParseTuple(args, "ss", &key, &tag))
	{
		PyErr_SetString(PyExc_TypeError, "setTagString expects two strings");
		PYTHON_RETURN_ERROR;
	}
	pySDLModifier::SetTagString(*(self->fThis), key, tag);
	PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptSDL)
	PYTHON_METHOD(ptSDL, setIndex, "Params: key,idx,value\nSets the value at a specific index in the tuple,\n"
				"so you don't have to pass the whole thing in"),
	PYTHON_METHOD(ptSDL, setIndexNow, "Params: key,idx,value\nSame as setIndex but sends immediately"),
	PYTHON_METHOD(ptSDL, setDefault, "Params: key,value\nLike setitem, but doesn't broadcast over the net.\n"
				"Only use for setting defaults that everyone will\n"
				"already know (from reading it off disk)"),
	PYTHON_METHOD(ptSDL, sendToClients, "Params: key\nSets it so changes to this key are sent to the\n"
				"server AND the clients. (Normally it just goes\n"
				"to the server.)"),
	PYTHON_METHOD(ptSDL, setNotify, "Params: selfkey,key,tolerance\nSets the OnSDLNotify to be called when 'key'\n"
				"SDL variable changes by 'tolerance' (if number)"),
	PYTHON_METHOD(ptSDL, setFlags, "Params: name,sendImmediate,skipOwnershipCheck\nSets the flags for a variable in this SDL"),
	PYTHON_METHOD(ptSDL, setTagString, "Params: name,tag\nSets the tag string for a variable"),
PYTHON_END_METHODS_TABLE;

PyObject* ptSDL_subscript(ptSDL* self, PyObject* key)
{
	if (!PyString_Check(key))
	{
		PyErr_SetString(PyExc_TypeError, "SDL indexes must be strings");
		PYTHON_RETURN_ERROR;
	}
	char *keyStr = PyString_AsString(key);
	return pySDLModifier::GetItem(*(self->fThis), keyStr);
}

int ptSDL_ass_subscript(ptSDL* self, PyObject* key, PyObject* value)
{
	if (value == NULL) // remove, which isn't supported
	{
		PyErr_SetString(PyExc_RuntimeError, "Cannot remove sdl records");
		return -1; // error return
	}
	if (!PyString_Check(key))
	{
		PyErr_SetString(PyExc_TypeError, "SDL indexes must be strings");
		return -1; // error return
	}
	if (!PyTuple_Check(value))
	{
		PyErr_SetString(PyExc_TypeError, "SDL values must be tuples");
		return -1; // error return
	}
	char* keyStr = PyString_AsString(key);
	pySDLModifier::SetItem(*(self->fThis), keyStr, value);
	return 0; // success return
}

PYTHON_START_AS_MAPPING_TABLE(ptSDL)
	0,									/* mp_length */
	(binaryfunc)ptSDL_subscript,		/* mp_subscript */
	(objobjargproc)ptSDL_ass_subscript,	/* mp_ass_subscript */
PYTHON_END_AS_MAPPING_TABLE;

#define ptSDL_COMPARE		PYTHON_NO_COMPARE
#define ptSDL_AS_NUMBER		PYTHON_NO_AS_NUMBER
#define ptSDL_AS_SEQUENCE	PYTHON_NO_AS_SEQUENCE
#define ptSDL_AS_MAPPING	PYTHON_DEFAULT_AS_MAPPING(ptSDL)
#define ptSDL_STR			PYTHON_NO_STR
#define ptSDL_RICH_COMPARE	PYTHON_NO_RICH_COMPARE
#define ptSDL_GETSET		PYTHON_NO_GETSET
#define ptSDL_BASE			PYTHON_NO_BASE
PLASMA_CUSTOM_TYPE(ptSDL, "SDL accessor");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptSDL, pySDLModifier)

PyObject *pySDLModifier::New(plPythonSDLModifier *sdlMod)
{
	ptSDL *newObj = (ptSDL*)ptSDL_type.tp_new(&ptSDL_type, NULL, NULL);
	newObj->fThis->fRecord = sdlMod;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptSDL, pySDLModifier)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptSDL, pySDLModifier)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pySDLModifier::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptSDL);
	PYTHON_CLASS_IMPORT_END(m);
}

PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(PtGetAgeSDL, "Returns the global ptSDL for the current Age")
{
	return pySDLModifier::GetAgeSDL();
}

void pySDLModifier::AddPlasmaMethods(std::vector<PyMethodDef> &methods)
{
	PYTHON_GLOBAL_METHOD_NOARGS(methods, PtGetAgeSDL);
}