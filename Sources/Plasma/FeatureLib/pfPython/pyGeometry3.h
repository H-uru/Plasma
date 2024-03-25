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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

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

#include "pyGlueDefinitions.h"

class pyPoint3
{
protected:
    pyPoint3() {}
    pyPoint3(float x, float y, float z) : fPoint(x,y,z) {}
    pyPoint3(const hsPoint3& pt) : fPoint(pt) {}

public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptPoint3);
    PYTHON_CLASS_NEW_DEFINITION;
    static PyObject *New(const hsPoint3 &obj);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyPoint3 object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyPoint3); // converts a PyObject to a pyPoint3 (throws error if not correct type)
    
    static void AddPlasmaClasses(PyObject *m);

    hsPoint3        fPoint;

    // python get attributes helpers
    float    getX() const { return fPoint.fX; }
    float    getY() const { return fPoint.fY; }
    float    getZ() const { return fPoint.fZ; }

    // python set attributes helpers
    void    setX(float x) { fPoint.fX = x; }
    void    setY(float y) { fPoint.fY = y; }
    void    setZ(float z) { fPoint.fZ = z; }

    // methods to manipulate point3's
    void Zero() { fPoint.fX=0; fPoint.fY=0; fPoint.fZ=0; }
    PyObject* Copy() const { return pyPoint3::New(fPoint); }
    float Distance(const pyPoint3& other) const { return hsVector3(&fPoint, &other.fPoint).Magnitude(); }
    float DistanceSquared(const pyPoint3& other) const { return hsVector3(&fPoint, &other.fPoint).MagnitudeSquared(); }
};


class pyVector3
{
protected:
    pyVector3() {}
    pyVector3(float x, float y, float z) : fVector(x,y,z) {}
    pyVector3(const hsVector3& v) : fVector(v) {}

public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptVector3);
    PYTHON_CLASS_NEW_DEFINITION;
    static PyObject *New(const hsVector3 &obj);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyVector3 object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyVector3); // converts a PyObject to a pyVector3 (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);

    hsVector3       fVector;

    // python get attributes helpers
    float    getX() const { return fVector.fX; }
    float    getY() const { return fVector.fY; }
    float    getZ() const { return fVector.fZ; }

    // python set attributes helpers
    void    setX(float x) { fVector.fX = x; }
    void    setY(float y) { fVector.fY = y; }
    void    setZ(float z) { fVector.fZ = z; }

    // operator methods
    PyObject*   operator+(const pyVector3& b) const { return pyVector3::New(fVector + b.fVector); }
    PyObject*   operator-(const pyVector3& b) const { return pyVector3::New(fVector - b.fVector); }

    // methods to manipulate vectors
    void    Normalize() { fVector.Normalize(); }
    float Dot(const pyVector3& other) { return fVector * other.fVector;}
    PyObject* Cross(const pyVector3& other) const { return pyVector3::New(fVector % other.fVector); }
    float Magnitude() const { return fVector.Magnitude(); }
    float MagnitudeSquared() const { return fVector.MagnitudeSquared(); }
    void Zero() { fVector.fX=0; fVector.fY=0; fVector.fZ=0; }
    PyObject* Scale(float scale) const { return pyVector3::New(fVector * scale); }
    PyObject* Add(const pyVector3& other) const { return pyVector3::New(fVector + other.fVector); }
    PyObject* Subtract(const pyVector3& other) const { return pyVector3::New(fVector - other.fVector); }
    PyObject* Copy() const { return pyVector3::New(fVector); }
};


#endif // _pyGeometry3_h_
