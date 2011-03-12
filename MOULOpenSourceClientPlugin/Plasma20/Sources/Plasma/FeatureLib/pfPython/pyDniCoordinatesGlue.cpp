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
#include "pyDniCoordinates.h"
#include "pyGeometry3.h"
#include "../plVault/plDniCoordinateInfo.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptDniCoordinates, pyDniCoordinates);

PYTHON_DEFAULT_NEW_DEFINITION(ptDniCoordinates, pyDniCoordinates)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptDniCoordinates)

PYTHON_INIT_DEFINITION(ptDniCoordinates, args, keywords)
{
	PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptDniCoordinates, getHSpans)
{
	return PyInt_FromLong(self->fThis->GetHSpans());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptDniCoordinates, getVSpans)
{
	return PyInt_FromLong(self->fThis->GetVSpans());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptDniCoordinates, getTorans)
{
	return PyInt_FromLong(self->fThis->GetTorans());
}

PYTHON_BASIC_METHOD_DEFINITION(ptDniCoordinates, update, UpdateCoordinates)

PYTHON_METHOD_DEFINITION(ptDniCoordinates, fromPoint, args)
{
	PyObject* pointObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &pointObj))
	{
		PyErr_SetString(PyExc_TypeError, "fromPoint expects a ptPoint3");
		PYTHON_RETURN_ERROR;
	}
	if (pyPoint3::Check(pointObj))
	{
		pyPoint3* pos = pyPoint3::ConvertFrom(pointObj);
		self->fThis->FromPoint(pos->fPoint);
		PYTHON_RETURN_NONE;
	}
	PyErr_SetString(PyExc_TypeError, "fromPoint expects a ptPoint3");
	PYTHON_RETURN_ERROR;
}

PYTHON_START_METHODS_TABLE(ptDniCoordinates)
	PYTHON_METHOD_NOARGS(ptDniCoordinates, getHSpans, "Returns the HSpans component of the coordinate"),
	PYTHON_METHOD_NOARGS(ptDniCoordinates, getVSpans, "Returns the VSpans component of the coordinate"),
	PYTHON_METHOD_NOARGS(ptDniCoordinates, getTorans, "Returns the Torans component of the coordinate"),
	PYTHON_BASIC_METHOD(ptDniCoordinates, update, "Update these coordinates with the players current position"),
	PYTHON_METHOD(ptDniCoordinates, fromPoint, "Params: pt\nUpdate these coordinates with the specified ptPoint3"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptDniCoordinates, "Constructor for a D'Ni coordinate");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptDniCoordinates, pyDniCoordinates)

PyObject *pyDniCoordinates::New(plDniCoordinateInfo* coord)
{
	ptDniCoordinates *newObj = (ptDniCoordinates*)ptDniCoordinates_type.tp_new(&ptDniCoordinates_type, NULL, NULL);
	if (coord) {
		newObj->fThis->fCoords->SetTorans(coord->GetTorans());
		newObj->fThis->fCoords->SetHSpans(coord->GetHSpans());
		newObj->fThis->fCoords->SetVSpans(coord->GetVSpans());
	}
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptDniCoordinates, pyDniCoordinates)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptDniCoordinates, pyDniCoordinates)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyDniCoordinates::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptDniCoordinates);
	PYTHON_CLASS_IMPORT_END(m);
}