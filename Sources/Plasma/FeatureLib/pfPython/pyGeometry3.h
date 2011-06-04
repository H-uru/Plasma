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
#ifndef _pyGeometry3_h_
#define _pyGeometry3_h_

//////////////////////////////////////////////////////////////////////
//
// pyGeometry3   - the wrapper class for hsPoint3 and hsVector3
//
//////////////////////////////////////////////////////////////////////

#include "hsGeometry3.h"

#include <python.h>
#include "pyGlueHelpers.h"

class pyPoint3
{
protected:
	pyPoint3() : fPoint(0,0,0) {}
	pyPoint3(hsScalar x, hsScalar y, hsScalar z) : fPoint(x,y,z) {}
	pyPoint3(hsPoint3 pt) : fPoint(pt.fX,pt.fY,pt.fZ) {}

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptPoint3);
	PYTHON_CLASS_NEW_DEFINITION;
	static PyObject *New(const hsPoint3 &obj);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyPoint3 object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyPoint3); // converts a PyObject to a pyPoint3 (throws error if not correct type)
	
	static void AddPlasmaClasses(PyObject *m);

	hsPoint3		fPoint;

	// python get attributes helpers
	hsScalar	getX() { return fPoint.fX; }
	hsScalar	getY() { return fPoint.fY; }
	hsScalar	getZ() { return fPoint.fZ; }

	// python set attributes helpers
	void	setX(hsScalar x) { fPoint.fX = x; }
	void	setY(hsScalar y) { fPoint.fY = y; }
	void	setZ(hsScalar z) { fPoint.fZ = z; }

	// methods to manipulate point3's
	void Zero() { fPoint.fX=0; fPoint.fY=0; fPoint.fZ=0; }
	PyObject* Copy() { return pyPoint3::New(fPoint); }
	hsScalar Distance(pyPoint3 other) { return hsVector3(&fPoint,&other.fPoint).Magnitude(); }
	hsScalar DistanceSquared(pyPoint3 other) { return hsVector3(&fPoint,&other.fPoint).MagnitudeSquared(); }
};


class pyVector3
{
protected:
	pyVector3() : fVector(0,0,0) {}
	pyVector3(hsScalar x, hsScalar y, hsScalar z) : fVector(x,y,z) {}
	pyVector3(hsVector3 v) : fVector(v.fX,v.fY,v.fZ) {}

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptVector3);
	PYTHON_CLASS_NEW_DEFINITION;
	static PyObject *New(const hsVector3 &obj);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyVector3 object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyVector3); // converts a PyObject to a pyVector3 (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	hsVector3		fVector;

	// python get attributes helpers
	hsScalar	getX() { return fVector.fX; }
	hsScalar	getY() { return fVector.fY; }
	hsScalar	getZ() { return fVector.fZ; }

	// python set attributes helpers
	void	setX(hsScalar x) { fVector.fX = x; }
	void	setY(hsScalar y) { fVector.fY = y; }
	void	setZ(hsScalar z) { fVector.fZ = z; }

	// operator methods
	PyObject*	operator+(const pyVector3& b) const { return pyVector3::New(fVector + b.fVector); }
	PyObject*	operator-(const pyVector3& b) const { return pyVector3::New(fVector - b.fVector); }

	// methods to manipulate vectors
	void	Normalize() { fVector.Normalize(); }
	hsScalar Dot(pyVector3 other) { return fVector*other.fVector;}
	PyObject* Cross(pyVector3 other) {return pyVector3::New(fVector%other.fVector); }
	hsScalar Magnitude() { return fVector.Magnitude(); }
	hsScalar MagnitudeSquared() { return fVector.MagnitudeSquared(); }
	void Zero() { fVector.fX=0; fVector.fY=0; fVector.fZ=0; }
	PyObject* Scale(hsScalar scale) { return pyVector3::New(fVector * scale); }
	PyObject* Add(pyVector3& other) { return pyVector3::New(fVector + other.fVector); }
	PyObject* Subtract(pyVector3& other) { return pyVector3::New(fVector - other.fVector); }
	PyObject* Copy() { return pyVector3::New(fVector); }
};


#endif // _pyGeometry3_h_
