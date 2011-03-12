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
#include "pyMatrix44.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptMatrix44, pyMatrix44);

PYTHON_DEFAULT_NEW_DEFINITION(ptMatrix44, pyMatrix44)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptMatrix44)

PYTHON_INIT_DEFINITION(ptMatrix44, args, keywords)
{
	PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptMatrix44, copy)
{
	return self->fThis->Copy();
}

PYTHON_METHOD_DEFINITION(ptMatrix44, translate, args)
{
	PyObject *vectorObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &vectorObj))
	{
		PyErr_SetString(PyExc_TypeError, "translate expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	if (!pyVector3::Check(vectorObj))
	{
		PyErr_SetString(PyExc_TypeError, "translate expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}

	pyVector3 *vec = pyVector3::ConvertFrom(vectorObj);
	self->fThis->Translate(*vec);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptMatrix44, scale, args)
{
	PyObject *vectorObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &vectorObj))
	{
		PyErr_SetString(PyExc_TypeError, "scale expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	if (!pyVector3::Check(vectorObj))
	{
		PyErr_SetString(PyExc_TypeError, "scale expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}

	pyVector3 *vec = pyVector3::ConvertFrom(vectorObj);
	self->fThis->Scale(*vec);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptMatrix44, rotate, args)
{
	int axis;
	float radians;
	if (!PyArg_ParseTuple(args, "if", &axis, &radians))
	{
		PyErr_SetString(PyExc_TypeError, "rotate expects an integer and a float");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Rotate(axis, radians);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptMatrix44, reset, Reset)

PYTHON_METHOD_DEFINITION(ptMatrix44, makeTranslateMat, args)
{
	PyObject *vectorObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &vectorObj))
	{
		PyErr_SetString(PyExc_TypeError, "makeTranslateMat expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	if (!pyVector3::Check(vectorObj))
	{
		PyErr_SetString(PyExc_TypeError, "makeTranslateMat expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}

	pyVector3 *vec = pyVector3::ConvertFrom(vectorObj);
	self->fThis->MakeTranslateMat(*vec);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptMatrix44, makeScaleMat, args)
{
	PyObject *vectorObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &vectorObj))
	{
		PyErr_SetString(PyExc_TypeError, "makeScaleMat expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	if (!pyVector3::Check(vectorObj))
	{
		PyErr_SetString(PyExc_TypeError, "makeScaleMat expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}

	pyVector3 *vec = pyVector3::ConvertFrom(vectorObj);
	self->fThis->MakeScaleMat(*vec);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptMatrix44, makeRotateMat, args)
{
	int axis;
	float radians;
	if (!PyArg_ParseTuple(args, "if", &axis, &radians))
	{
		PyErr_SetString(PyExc_TypeError, "makeRotateMat expects an integer and a float");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->MakeRotateMat(axis, radians);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptMatrix44, make, args)
{
	PyObject *fromPtObj = NULL;
	PyObject *atPtObj = NULL;
	PyObject *upVecObj = NULL;
	if (!PyArg_ParseTuple(args, "OOO", &fromPtObj, &atPtObj, &upVecObj))
	{
		PyErr_SetString(PyExc_TypeError, "make expects two ptPoint3 objects and a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	if ((!pyPoint3::Check(fromPtObj))||(!pyPoint3::Check(atPtObj))||(!pyVector3::Check(upVecObj)))
	{
		PyErr_SetString(PyExc_TypeError, "make expects two ptPoint3 objects and a ptVector3");
		PYTHON_RETURN_ERROR;
	}

	pyPoint3 *fromPt = pyPoint3::ConvertFrom(fromPtObj);
	pyPoint3 *atPt = pyPoint3::ConvertFrom(atPtObj);
	pyVector3 *upVec = pyVector3::ConvertFrom(upVecObj);
	self->fThis->Make(*fromPt, *atPt, *upVec);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptMatrix44, makeUpPreserving, args)
{
	PyObject *fromPtObj = NULL;
	PyObject *atPtObj = NULL;
	PyObject *upVecObj = NULL;
	if (!PyArg_ParseTuple(args, "OOO", &fromPtObj, &atPtObj, &upVecObj))
	{
		PyErr_SetString(PyExc_TypeError, "makeUpPreserving expects two ptPoint3 objects and a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	if ((!pyPoint3::Check(fromPtObj))||(!pyPoint3::Check(atPtObj))||(!pyVector3::Check(upVecObj)))
	{
		PyErr_SetString(PyExc_TypeError, "makeUpPreserving expects two ptPoint3 objects and a ptVector3");
		PYTHON_RETURN_ERROR;
	}

	pyPoint3 *fromPt = pyPoint3::ConvertFrom(fromPtObj);
	pyPoint3 *atPt = pyPoint3::ConvertFrom(atPtObj);
	pyVector3 *upVec = pyVector3::ConvertFrom(upVecObj);
	self->fThis->MakeUpPreserving(*fromPt, *atPt, *upVec);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptMatrix44, getParity)
{
	PYTHON_RETURN_BOOL(self->fThis->GetParity());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptMatrix44, getDeterminant)
{
	return PyFloat_FromDouble((double)self->fThis->GetDeterminant());
}

PYTHON_METHOD_DEFINITION(ptMatrix44, getInverse, args)
{
	PyObject *inverseObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &inverseObj))
	{
		PyErr_SetString(PyExc_TypeError, "getInverse expects a ptMatrix44");
		PYTHON_RETURN_ERROR;
	}
	if (!pyMatrix44::Check(inverseObj))
	{
		PyErr_SetString(PyExc_TypeError, "getInverse expects a ptMatrix44");
		PYTHON_RETURN_ERROR;
	}
	return self->fThis->GetInverse(inverseObj);
}

PYTHON_METHOD_DEFINITION(ptMatrix44, getTranspose, args)
{
	PyObject *transposeObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &transposeObj))
	{
		PyErr_SetString(PyExc_TypeError, "getTranspose expects a ptMatrix44");
		PYTHON_RETURN_ERROR;
	}
	if (!pyMatrix44::Check(transposeObj))
	{
		PyErr_SetString(PyExc_TypeError, "getTranspose expects a ptMatrix44");
		PYTHON_RETURN_ERROR;
	}
	return self->fThis->GetTranspose(transposeObj);
}

PYTHON_METHOD_DEFINITION(ptMatrix44, getAdjoint, args)
{
	PyObject *adjointObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &adjointObj))
	{
		PyErr_SetString(PyExc_TypeError, "getAdjoint expects a ptMatrix44");
		PYTHON_RETURN_ERROR;
	}
	if (!pyMatrix44::Check(adjointObj))
	{
		PyErr_SetString(PyExc_TypeError, "getAdjoint expects a ptMatrix44");
		PYTHON_RETURN_ERROR;
	}
	return self->fThis->GetAdjoint(adjointObj);
}

PYTHON_METHOD_DEFINITION(ptMatrix44, getTranslate, args)
{
	PyObject *translateObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &translateObj))
	{
		PyErr_SetString(PyExc_TypeError, "translateObj expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	if (!pyVector3::Check(translateObj))
	{
		PyErr_SetString(PyExc_TypeError, "translateObj expects a ptVector3");
		PYTHON_RETURN_ERROR;
	}
	return self->fThis->GetTranslate(translateObj);
}

PYTHON_METHOD_DEFINITION_NOARGS(ptMatrix44, view)
{
	return self->fThis->GetViewAxis();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptMatrix44, up)
{
	return self->fThis->GetUpAxis();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptMatrix44, right)
{
	return self->fThis->GetRightAxis();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptMatrix44, getData)
{
	std::vector< std::vector<hsScalar> > mat = self->fThis->GetData();
	PyObject *retVal = PyTuple_New(4);
	for (int curRow = 0; curRow < mat.size(); curRow++)
	{
		PyObject *row = PyTuple_New(4);
		for (int curElement = 0; curElement < mat[curRow].size(); curElement++)
		{
			PyObject *item = PyInt_FromLong((long)mat[curRow][curElement]);
			PyTuple_SetItem(row, curElement, item); // steals the ref
		}
		PyTuple_SetItem(retVal, curRow, row); // steals the ref
	}
	return retVal;
}

PYTHON_METHOD_DEFINITION(ptMatrix44, setData, args)
{
	PyObject *matTuple = NULL;
	if (!PyArg_ParseTuple(args, "O", &matTuple))
	{
		PyErr_SetString(PyExc_TypeError, "setData expects a 4x4 tuple of floats");
		PYTHON_RETURN_ERROR;
	}
	if (!PyTuple_Check(matTuple))
	{
		PyErr_SetString(PyExc_TypeError, "setData expects a 4x4 tuple of floats");
		PYTHON_RETURN_ERROR;
	}

	std::vector< std::vector<hsScalar> > mat;
	int numRows = PyTuple_Size(matTuple);
	for (int curRow = 0; curRow < numRows; curRow++)
	{
		std::vector<hsScalar> vecRow;
		PyObject *row = PyTuple_GetItem(matTuple, curRow); // borrowed ref
		if (!PyTuple_Check(row))
		{
			PyErr_SetString(PyExc_TypeError, "setData expects a 4x4 tuple of floats");
			PYTHON_RETURN_ERROR;
		}
		int numElements = PyTuple_Size(row);
		for (int curElement = 0; curElement < numElements; curElement++)
		{
			PyObject *item = PyTuple_GetItem(row, curElement); // borrowed ref
			if (!PyFloat_Check(item))
			{
				PyErr_SetString(PyExc_TypeError, "setData expects a 4x4 tuple of floats");
				PYTHON_RETURN_ERROR;
			}
			vecRow.push_back((float)PyFloat_AsDouble(item));
		}
		mat.push_back(vecRow);
	}

	self->fThis->SetData(mat);
	if (PyErr_Occurred())
		PYTHON_RETURN_ERROR;
	PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptMatrix44)
	PYTHON_METHOD_NOARGS(ptMatrix44, copy, "Copies the matrix and returns the copy"),
	PYTHON_METHOD(ptMatrix44, translate, "Params: vector\nTranslates the matrix by the vector"),
	PYTHON_METHOD(ptMatrix44, scale, "Params: scale\nScales the matrix by the vector"),
	PYTHON_METHOD(ptMatrix44, rotate, "Params: axis,radians\nRotates the matrix by radians around the axis"),
	PYTHON_BASIC_METHOD(ptMatrix44, reset, "Reset the matrix to identity"),
	PYTHON_METHOD(ptMatrix44, makeTranslateMat, "Params: trans\nMakes the matrix a translation matrix"),
	PYTHON_METHOD(ptMatrix44, makeScaleMat, "Params: scale\nMakes the matrix a scaling matrix"),
	PYTHON_METHOD(ptMatrix44, makeRotateMat, "Params: axis,radians\nMakes the matrix a rotation matrix"),
	PYTHON_METHOD(ptMatrix44, make, "Params: fromPt, atPt, upVec\nCreates the matrix from from and at points, and the up vector"),
	PYTHON_METHOD(ptMatrix44, makeUpPreserving, "Params: fromPt, atPt, upVec\nCreates the matrix from from and at points, and the up vector (perserving the up vector)"),
	PYTHON_METHOD_NOARGS(ptMatrix44, getParity, "Get the parity of the matrix"),
	PYTHON_METHOD_NOARGS(ptMatrix44, getDeterminant, "Get the matrix's determinant"),
	PYTHON_METHOD(ptMatrix44, getInverse, "Params: inverseMat\nReturns the inverse of the matrix"),
	PYTHON_METHOD(ptMatrix44, getTranspose, "Params: transposeMat\nReturns the transpose of the matrix"),
	PYTHON_METHOD(ptMatrix44, getAdjoint, "Params: adjointMat\nReturns the adjoint of the matrix"),
	PYTHON_METHOD(ptMatrix44, getTranslate, "Params: vector\nReturns the translate vector of the matrix (and sets vector to it as well)"),
	PYTHON_METHOD_NOARGS(ptMatrix44, view, "Returns the view vector of the matrix"),
	PYTHON_METHOD_NOARGS(ptMatrix44, up, "Returns the up vector of the matrix"),
	PYTHON_METHOD_NOARGS(ptMatrix44, right, "Returns the right vector of the matrix"),
	PYTHON_METHOD_NOARGS(ptMatrix44, getData, "Returns the matrix in tuple form"),
	PYTHON_METHOD(ptMatrix44, setData, "Params: mat\nSets the matrix using tuples"),
PYTHON_END_METHODS_TABLE;

PyObject *ptMatrix44_mul(PyObject *v, PyObject *w)
{
	if (pyMatrix44::Check(v))
	{
		pyMatrix44 *us = pyMatrix44::ConvertFrom(v);
		if (pyMatrix44::Check(w))
		{
			pyMatrix44 *them = pyMatrix44::ConvertFrom(w);
			return (*us) * (*them);
		}
		else if (pyVector3::Check(w))
		{
			pyVector3 *them = pyVector3::ConvertFrom(w);
			return (*us) * (*them);
		}
		else if (pyPoint3::Check(w))
		{
			pyPoint3 *them = pyPoint3::ConvertFrom(w);
			return (*us) * (*them);
		}
	}
	PyErr_SetString(PyExc_NotImplementedError, "can only multiply a ptMatrix44 by a ptVector3, or ptPoint3");
	PYTHON_RETURN_NOT_IMPLEMENTED;
}

// we support some of the number methods
PYTHON_START_AS_NUMBER_TABLE(ptMatrix44)
	0,							/*nb_add*/
	0,							/*nb_subtract*/
	(binaryfunc)ptMatrix44_mul,	/*nb_multiply*/
	0							/*nb_divide*/
	/* the rest can be null */
PYTHON_END_AS_NUMBER_TABLE;

// Type structure definition
#define ptMatrix44_COMPARE			PYTHON_NO_COMPARE
#define ptMatrix44_AS_NUMBER		PYTHON_DEFAULT_AS_NUMBER(ptMatrix44)
#define ptMatrix44_AS_SEQUENCE		PYTHON_NO_AS_SEQUENCE
#define ptMatrix44_AS_MAPPING		PYTHON_NO_AS_MAPPING
#define ptMatrix44_STR				PYTHON_NO_STR
#define ptMatrix44_RICH_COMPARE		PYTHON_NO_RICH_COMPARE
#define ptMatrix44_GETSET			PYTHON_NO_GETSET
#define ptMatrix44_BASE				PYTHON_NO_BASE
PLASMA_CUSTOM_TYPE(ptMatrix44, "Plasma Matrix44 class");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptMatrix44, pyMatrix44)

PyObject *pyMatrix44::New(const hsMatrix44 &obj)
{
	ptMatrix44 *newObj = (ptMatrix44*)ptMatrix44_type.tp_new(&ptMatrix44_type, NULL, NULL);
	newObj->fThis->fMatrix = obj;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptMatrix44, pyMatrix44)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptMatrix44, pyMatrix44)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyMatrix44::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptMatrix44);
	PYTHON_CLASS_IMPORT_END(m);
}
