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
#include "pyDniInfoSource.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptDniInfoSource, pyDniInfoSource);

PYTHON_DEFAULT_NEW_DEFINITION(ptDniInfoSource, pyDniInfoSource)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptDniInfoSource)

PYTHON_INIT_DEFINITION(ptDniInfoSource, args, keywords)
{
	PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptDniInfoSource, getAgeCoords)
{
	return self->fThis->GetAgeCoords();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptDniInfoSource, getAgeTime)
{
	return PyLong_FromUnsignedLong(self->fThis->GetAgeTime());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptDniInfoSource, getAgeName)
{
	return PyString_FromString(self->fThis->GetAgeName());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptDniInfoSource, getAgeGuid)
{
	return PyString_FromString(self->fThis->GetAgeGuid());
}

PYTHON_START_METHODS_TABLE(ptDniInfoSource)
	PYTHON_METHOD_NOARGS(ptDniInfoSource, getAgeCoords, "Current coords of the player in current age as a ptDniCoordinates"),
	PYTHON_METHOD_NOARGS(ptDniInfoSource, getAgeTime, "Current time in current age (tbd)"),
	PYTHON_METHOD_NOARGS(ptDniInfoSource, getAgeName, "Name of current age"),
	PYTHON_METHOD_NOARGS(ptDniInfoSource, getAgeGuid, "Unique identifier for this age instance"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptDniInfoSource, "DO NOT USE");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptDniInfoSource, pyDniInfoSource)

PYTHON_CLASS_CHECK_IMPL(ptDniInfoSource, pyDniInfoSource)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptDniInfoSource, pyDniInfoSource)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyDniInfoSource::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptDniInfoSource);
	PYTHON_CLASS_IMPORT_END(m);
}