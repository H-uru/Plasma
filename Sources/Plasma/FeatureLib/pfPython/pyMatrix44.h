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
#ifndef pyMatrix44_h_inc
#define pyMatrix44_h_inc

#include "hsStlUtils.h"

#include "hsMatrix44.h"
#include "pyGeometry3.h"

#include <python.h>
#include "pyGlueHelpers.h"


class pyMatrix44
{
protected:
	pyMatrix44();
	pyMatrix44(hsMatrix44 other);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptMatrix44);
	PYTHON_CLASS_NEW_DEFINITION;
	static PyObject *New(const hsMatrix44 &obj);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyMatrix44 object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyMatrix44); // converts a PyObject to a pyMatrix44 (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	hsMatrix44		fMatrix;

	// operator methods
	PyObject* operator*(const pyMatrix44& b) const { return pyMatrix44::New(fMatrix * b.fMatrix); } // returns pyMatrix44
	PyObject* operator*(const pyVector3& p) const { return pyVector3::New(fMatrix * p.fVector); } // returns pyVector3
	PyObject* operator*(const pyPoint3& p) const { return pyPoint3::New(fMatrix * p.fPoint); } // returns pyPoint3

	// other methods
	PyObject* Copy() { return pyMatrix44::New(fMatrix); } // returns pyMatrix44
	void Translate(pyVector3 v) { fMatrix.Translate(&v.fVector); }
	void Scale(pyVector3 v) { fMatrix.Scale(&v.fVector); }
	void Rotate(int axis, hsScalar radians) { fMatrix.Rotate(axis,radians); }
	void Reset() { fMatrix.Reset(); }
	void MakeTranslateMat(pyVector3 trans) { fMatrix.MakeTranslateMat(&trans.fVector); }
	void MakeScaleMat(pyVector3 scale) { fMatrix.MakeScaleMat(&scale.fVector); }
	void MakeRotateMat(int axis, hsScalar radians) { fMatrix.MakeRotateMat(axis,radians); }
	void Make(pyPoint3 from,pyPoint3 at,pyVector3 up) { fMatrix.Make(&from.fPoint,&at.fPoint,&up.fVector); }
	void MakeUpPreserving(pyPoint3 from,pyPoint3 at,pyVector3 up) { fMatrix.MakeUpPreserving(&from.fPoint,&at.fPoint,&up.fVector); }
	hsBool GetParity() { return fMatrix.GetParity(); }
	hsScalar GetDeterminant() { return fMatrix.GetDeterminant(); }
	PyObject* GetInverse(PyObject* inverse); // returns (and accepts) pyMatrix44
	PyObject* GetTranspose(PyObject* inverse); // returns (and accepts) pyMatrix44
	PyObject* GetAdjoint(PyObject* adjoint); // returns (and accepts) pyMatrix44
	PyObject* GetTranslate(PyObject* pt); // returns (and accepts) pyVector3
	PyObject* GetViewAxis() { return pyVector3::New(fMatrix.GetAxis(hsMatrix44::kView)); } // returns pyVector3
	PyObject* GetUpAxis() { return pyVector3::New(fMatrix.GetAxis(hsMatrix44::kUp)); } // returns pyVector3
	PyObject* GetRightAxis() { return pyVector3::New(fMatrix.GetAxis(hsMatrix44::kRight)); } // returns pyVector3

	std::vector< std::vector<hsScalar> > GetData();
	void SetData(const std::vector< std::vector<hsScalar> > & mat);
};


#endif // pyMatrix44_h_inc
