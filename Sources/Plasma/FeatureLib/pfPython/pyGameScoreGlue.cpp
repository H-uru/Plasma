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
#include "pyGameScore.h"

#include "../pfGameScoreMgr/pfGameScoreMgr.h"

// glue functions
PYTHON_CLASS_DEFINITION(ptGameScore, pyGameScore);

PYTHON_DEFAULT_NEW_DEFINITION(ptGameScore, pyGameScore)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGameScore)

PYTHON_NO_INIT_DEFINITION(ptGameScore)

PYTHON_METHOD_DEFINITION_NOARGS(ptGameScore, getScoreID)
{
	return PyInt_FromLong(self->fThis->GetScoreID());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameScore, getCreatedTime)
{
	return PyLong_FromUnsignedLong(self->fThis->GetCreatedTime());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameScore, getOwnerID)
{
	return PyInt_FromLong(self->fThis->GetOwnerID());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameScore, getValue)
{
	return PyInt_FromLong(self->fThis->GetValue());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameScore, getGameType)
{
	return PyInt_FromLong(self->fThis->GetGameType());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameScore, getGameName)
{
	return PyString_FromString(self->fThis->GetGameName());
}

PYTHON_METHOD_DEFINITION(ptGameScore, addPoints, args)
{
	int numPoints = 0;
	if (!PyArg_ParseTuple(args, "i", &numPoints))
	{
		PyErr_SetString(PyExc_TypeError, "addPoints expects an int");
		PYTHON_RETURN_ERROR;
	}

	PYTHON_RETURN_BOOL(self->fThis->AddPoints(numPoints));
}

PYTHON_METHOD_DEFINITION(ptGameScore, transferPoints, args)
{
	unsigned destination = 0;
	int numPoints = 0;
	if (!PyArg_ParseTuple(args, "Ii", &destination, &numPoints))
	{
		PyErr_SetString(PyExc_TypeError, "transferPoints expects an unsigned int and an int");
		PYTHON_RETURN_ERROR;
	}

	PYTHON_RETURN_BOOL(self->fThis->TransferPoints(destination, numPoints));
}

PYTHON_METHOD_DEFINITION(ptGameScore, setPoints, args)
{
	int numPoints = 0;
	if (!PyArg_ParseTuple(args, "i", &numPoints))
	{
		PyErr_SetString(PyExc_TypeError, "setPoints expects an int");
		PYTHON_RETURN_ERROR;
	}

	PYTHON_RETURN_BOOL(self->fThis->SetPoints(numPoints));
}

PYTHON_START_METHODS_TABLE(ptGameScore)
	PYTHON_METHOD_NOARGS(ptGameScore, getScoreID, "Returns the score id."),
	PYTHON_METHOD_NOARGS(ptGameScore, getOwnerID, "Returns a the score owner id."),
	PYTHON_METHOD_NOARGS(ptGameScore, getCreatedTime, "Returns a the score creation time."),
	PYTHON_METHOD_NOARGS(ptGameScore, getValue, "Returns a the score owner value."),
	PYTHON_METHOD_NOARGS(ptGameScore, getGameType, "Returns a the score game type."),
	PYTHON_METHOD_NOARGS(ptGameScore, getGameName, "Returns a the score game name."),
	PYTHON_METHOD(ptGameScore, addPoints, "Params: numPoints\nAdds points to the score"),
	PYTHON_METHOD(ptGameScore, transferPoints, "Params: dest, numPoints\nTransfers points from one score to another"),
	PYTHON_METHOD(ptGameScore, setPoints, "Params: numPoints\nSets the number of points in the score\nDon't use to add/remove points, use only to reset values!"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptGameScore, "Game score manager");

// required functions for PyObject interoperability
PyObject* pyGameScore::New(pfGameScore* score)
{
	ptGameScore* newObj = (ptGameScore*)ptGameScore_type.tp_new(&ptGameScore_type, NULL, NULL);
	if (newObj->fThis->fScore)
		newObj->fThis->fScore->DecRef();
	newObj->fThis->fScore = score;
	if (newObj->fThis->fScore)
		newObj->fThis->fScore->IncRef();
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGameScore, pyGameScore)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGameScore, pyGameScore)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyGameScore::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptGameScore);
	PYTHON_CLASS_IMPORT_END(m);
}
