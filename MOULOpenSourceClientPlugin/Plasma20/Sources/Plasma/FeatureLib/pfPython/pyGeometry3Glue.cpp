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
#include "pyGeometry3.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptPoint3, pyPoint3);

PYTHON_DEFAULT_NEW_DEFINITION(ptPoint3, pyPoint3)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptPoint3)

PYTHON_INIT_DEFINITION(ptPoint3, args, keywords)
{
	float x = 0.0f, y = 0.0f, z = 0.0f;
	if (!PyArg_ParseTuple(args, "|fff", &x, &y, &z))
	{
		PyErr_SetString(PyExc_TypeError, "init optionally expects three floats");
		PYTHON_RETURN_INIT_ERROR;
	}

	self->fThis->fPoint.fX = x;
	self->fThis->fPoint.fY = y;
	self->fThis->fPoint.fZ = z;

	PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptPoint3, getX)
{
	return PyFloat_FromDouble((double)self->fThis->getX());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptPoint3, getY)
{
	return PyFloat_FromDouble((double)self->fThis->getY());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptPoint3, getZ)
{
	return PyFloat_FromDouble((double)self->fThis->getZ());
}

PYTHON_METHOD_DEFINITION(ptPoint3, setX, args)
{
	float x;
	if (!PyArg_ParseTuple(args, "f", &x))
	{
		PyErr_SetString(PyExc_TypeError, "setX expects a float");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->setX(x);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptPoint3, setY, args)
{
	float y;
	if (!PyArg_ParseTuple(args, "f", &y))
	{
		PyErr_SetString(PyExc_TypeError, "setY expects a float");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->setY(y);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptPoint3, setZ, args)
{
	float z;
	if (!PyArg_ParseTuple(args, "f", &z))
	{
		PyErr_SetString(PyExc_TypeError, "setZ expects a float");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->setZ(z);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptPoint3, zero, Zero)

PYTHON_METHOD_DEFINITION_NOARGS(ptPoint3, copy)
{
	return self->fThis->Copy();
}

PYTHON_METHOD_DEFINITION(ptPoint3, distance, args)
{
	PyObject *otherObject = NULL;
	if (!PyArg_ParseTuple(args, "O", &otherObject))
	{
		PyErr_SetString(PyExc_TypeError, "distance expects a ptPoint3");
		PYTHON_RETURN_ERROR;
	}
	if (!pyPoint3::Check(otherObject))
	{
		PyErr_SetString(PyExc_TypeError, "distance expects a ptPoint3");
		PYTHON_RETURN_ERROR;
	}

	pyPoint3 *other = pyPoint3::ConvertFrom(otherObject);
	return PyFloat_FromDouble((double)self->fThis->Distance(*other));
}

PYTHON_METHOD_DEFINITION(ptPoint3, distanceSq, args)
{
	PyObject *otherObject = NULL;
	if (!PyArg_ParseTuple(args, "O", &otherObject))
	{
		PyErr_SetString(PyExc_TypeError, "distanceSq expects a ptPoint3");
		PYTHON_RETURN_ERROR;
	}
	if (!pyPoint3::Check(otherObject))
	{
		PyErr_SetString(PyExc_TypeError, "distanceSq expects a ptPoint3");
		PYTHON_RETURN_ERROR;
	}

	pyPoint3 *other = pyPoint3::ConvertFrom(otherObject);
	return PyFloat_FromDouble((double)self->fThis->DistanceSquared(*other));
}

PYTHON_START_METHODS_TABLE(ptPoint3)
	PYTHON_METHOD_NOARGS(ptPoint3, getX, "Returns the 'x' component of the point"),
	PYTHON_METHOD_NOARGS(ptPoint3, getY, "Returns the 'y' component of the point"),
	PYTHON_METHOD_NOARGS(ptPoint3, getZ, "Returns the 'z' component of the point"),
	PYTHON_METHOD(ptPoint3, setX, "Params: x\nSets the 'x' component of the point"),
	PYTHON_METHOD(ptPoint3, setY, "Params: y\nSets the 'y' component of the point"),
	PYTHON_METHOD(ptPoint3, setZ, "Params: z\nSets the 'z' component of the point"),
	PYTHON_BASIC_METHOD(ptPoint3, zero, "Sets the 'x','y' and the 'z' component to zero"),
	PYTHON_METHOD_NOARGS(ptPoint3, copy, "Returns a copy of the point in another ptPoint3 object"),
	PYTHON_METHOD(ptPoint3, distance, "Params: other\nComputes the distance from this point to 'other' point"),
	PYTHON_METHOD(ptPoint3, distanceSq, "Params: other\nComputes the distance squared from this point to 'other' point\n"
				"- this function is faster than distance(other)"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptPoint3, "Params: x=0, y=0, z=0\nPlasma Point class");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptPoint3, pyPoint3)

PyObject *pyPoint3::New(const hsPoint3 &obj)
{
	ptPoint3 *newObj = (ptPoint3*)ptPoint3_type.tp_new(&ptPoint3_type, NULL, NULL);
	newObj->fThis->fPoint.Set(&obj);
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptPoint3, pyPoint3)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptPoint3, pyPoint3)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyPoint3::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptPoint3);
	PYTHON_CLASS_IMPORT_END(m);
}

// glue functions
PYTHON_CLASS_DEFINITION(ptVector3, pyVector3);

PYTHON_DEFAULT_NEW_DEFINITION(ptVector3, pyVector3)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVector3)

PYTHON_INIT_DEFINITION(ptVector3, args, keywords)
{
	float x = 0.0f, y = 0.0f, z = 0.0f;
	if (!PyArg_ParseTuple(args, "|fff", &x, &y, &z))
	{
		PyErr_SetString(PyExc_TypeError, "init optionally expects three floats");
		PYTHON_RETURN_INIT_ERROR;
	}

	self->fThis->fVector.fX = x;
	self->fThis->fVector.fY = y;
	self->fThis->fVector.fZ = z;

	PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVector3, getX)
{
	return PyFloat_FromDouble((double)self->fThis->getX());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVector3, getY)
{
	return PyFloat_FromDouble((double)self->fThis->getY());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVector3, getZ)
{
	return PyFloat_FromDouble((double)self->fThis->getZ());
}

PYTHON_METHOD_DEFINITION(ptVector3, setX, args)
{
	float x;
	if (!PyArg_ParseTuple(args, "f", &x))
	{
		PyErr_SetString(PyExc_TypeError, "setX expects a float");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->setX(x);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVector3, setY, args)
{
	float y;
	if (!PyArg_ParseTuple(args, "f", &y))
	{
		PyErr_SetString(PyExc_TypeError, "setY expects a float");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->setY(y);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVector3, setZ, args)
{
	float z;
	if (!PyArg_ParseTuple(args, "f", &z))
	{
		PyErr_SetString(PyExc_TypeError, "setZ expects a float");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->setZ(z);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptVector3, zero, Zero)

PYTHON_METHOD_DEFINITION_NOARGS(ptVector3, copy)
{
	return self->fThis->Copy();
}

PYTHON_METHOD_DEFINITION(ptVector3, scale, args)
{
	float scale;
	if (!PyArg_ParseTuple(args, "f", &scale))
	{
		PyErr_SetString(PyExc_TypeError, "scale expects a float");
		PYTHON_RETURN_ERROR;
	}
	return self->fThis->Scale(scale);
}

PYTHON_METHOD_DEFINITION(ptVector3, add, args)
{
	PyObject *otherObject;
	if (!PyArg_ParseTuple(args, "O", &otherObject))
	{
		PyErr_SetString(PyExc_TypeError, "add expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	if (!pyVector3::Check(otherObject))
	{
		PyErr_SetString(PyExc_TypeError, "add expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	pyVector3 *other = pyVector3::ConvertFrom(otherObject);
	return self->fThis->Add(*other);
}

PYTHON_METHOD_DEFINITION(ptVector3, subtract, args)
{
	PyObject *otherObject;
	if (!PyArg_ParseTuple(args, "O", &otherObject))
	{
		PyErr_SetString(PyExc_TypeError, "subtract expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	if (!pyVector3::Check(otherObject))
	{
		PyErr_SetString(PyExc_TypeError, "subtract expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	pyVector3 *other = pyVector3::ConvertFrom(otherObject);
	return self->fThis->Subtract(*other);
}

PYTHON_BASIC_METHOD_DEFINITION(ptVector3, normalize, Normalize)

PYTHON_METHOD_DEFINITION(ptVector3, dotProduct, args)
{
	PyObject *otherObject;
	if (!PyArg_ParseTuple(args, "O", &otherObject))
	{
		PyErr_SetString(PyExc_TypeError, "dotProduct expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	if (!pyVector3::Check(otherObject))
	{
		PyErr_SetString(PyExc_TypeError, "dotProduct expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	pyVector3 *other = pyVector3::ConvertFrom(otherObject);
	return PyFloat_FromDouble((double)self->fThis->Dot(*other));
}

PYTHON_METHOD_DEFINITION(ptVector3, crossProduct, args)
{
	PyObject *otherObject;
	if (!PyArg_ParseTuple(args, "O", &otherObject))
	{
		PyErr_SetString(PyExc_TypeError, "crossProduct expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	if (!pyVector3::Check(otherObject))
	{
		PyErr_SetString(PyExc_TypeError, "crossProduct expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	pyVector3 *other = pyVector3::ConvertFrom(otherObject);
	return self->fThis->Cross(*other);
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVector3, length)
{
	return PyFloat_FromDouble((double)self->fThis->Magnitude());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVector3, lengthSq)
{
	return PyFloat_FromDouble((double)self->fThis->MagnitudeSquared());
}

PYTHON_START_METHODS_TABLE(ptVector3)
	PYTHON_METHOD_NOARGS(ptVector3, getX, "Returns the 'x' component of the vector"),
	PYTHON_METHOD_NOARGS(ptVector3, getY, "Returns the 'y' component of the vector"),
	PYTHON_METHOD_NOARGS(ptVector3, getZ, "Returns the 'z' component of the vector"),
	PYTHON_METHOD(ptVector3, setX, "Params: x\nSets the 'x' component of the vector"),
	PYTHON_METHOD(ptVector3, setY, "Params: y\nSets the 'y' component of the vector"),
	PYTHON_METHOD(ptVector3, setZ, "Params: z\nSets the 'z' component of the vector"),
	PYTHON_BASIC_METHOD(ptVector3, zero, "Zeros the vector's components"),
	PYTHON_METHOD_NOARGS(ptVector3, copy, "Copies the vector into another one (which it returns)"),
	PYTHON_METHOD(ptVector3, scale, "Params: scale\nScale the vector by scale"),
	PYTHON_METHOD(ptVector3, add, "Params: other\nAdds other to the current vector"),
	PYTHON_METHOD(ptVector3, subtract, "Params: other\nSubtracts other from the current vector"),
	PYTHON_BASIC_METHOD(ptVector3, normalize, "Normalizes the vector to length 1"),
	PYTHON_METHOD(ptVector3, dotProduct, "Params: other\nFinds the dot product between other and this vector"),
	PYTHON_METHOD(ptVector3, crossProduct, "Params: other\nFinds the cross product between other and this vector"),
	PYTHON_METHOD_NOARGS(ptVector3, length, "Returns the length of the vector"),
	PYTHON_METHOD_NOARGS(ptVector3, lengthSq, "Returns the length of the vector, squared\n"
				"- this function is faster then length(other)"),
PYTHON_END_METHODS_TABLE;

PyObject *ptVector3_sub(PyObject *v, PyObject *w)
{
	if (pyVector3::Check(v))
	{
		pyVector3 *me = pyVector3::ConvertFrom(v);
		if (pyVector3::Check(w))
		{
			pyVector3 *other = pyVector3::ConvertFrom(w);
			return (*me) - (*other);
		}
	}
	PyErr_SetString(PyExc_NotImplementedError, "can only subtract a ptVector3 from a ptVector3");
	PYTHON_RETURN_NOT_IMPLEMENTED;
}

PyObject *ptVector3_add(PyObject *v, PyObject *w)
{
	if (pyVector3::Check(v))
	{
		pyVector3 *me = pyVector3::ConvertFrom(v);
		if (pyVector3::Check(w))
		{
			pyVector3 *other = pyVector3::ConvertFrom(w);
			return (*me) + (*other);
		}
	}
	PyErr_SetString(PyExc_NotImplementedError, "can only subtract a ptVector3 from a ptVector3");
	PYTHON_RETURN_NOT_IMPLEMENTED;
}

PYTHON_START_AS_NUMBER_TABLE(ptVector3)
	(binaryfunc)ptVector3_add,	/*nb_add*/
	(binaryfunc)ptVector3_sub,	/*nb_subtract*/
	0,							/*nb_multiply*/
	0							/*nb_divide*/
	/* the rest can be null */
PYTHON_END_AS_NUMBER_TABLE;

// Type structure definition
#define ptVector3_COMPARE		PYTHON_NO_COMPARE
#define ptVector3_AS_NUMBER		PYTHON_DEFAULT_AS_NUMBER(ptVector3)
#define ptVector3_AS_SEQUENCE	PYTHON_NO_AS_SEQUENCE
#define ptVector3_AS_MAPPING	PYTHON_NO_AS_MAPPING
#define ptVector3_STR			PYTHON_NO_STR
#define ptVector3_RICH_COMPARE	PYTHON_NO_RICH_COMPARE
#define ptVector3_GETSET		PYTHON_NO_GETSET
#define ptVector3_BASE			PYTHON_NO_BASE
PLASMA_CUSTOM_TYPE(ptVector3, "Params: x=0, y=0, z=0\nPlasma Point class");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptVector3, pyVector3)

PyObject *pyVector3::New(const hsVector3 &obj)
{
	ptVector3 *newObj = (ptVector3*)ptVector3_type.tp_new(&ptVector3_type, NULL, NULL);
	newObj->fThis->fVector.Set(&obj);
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptVector3, pyVector3)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptVector3, pyVector3)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyVector3::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptVector3);
	PYTHON_CLASS_IMPORT_END(m);
}
