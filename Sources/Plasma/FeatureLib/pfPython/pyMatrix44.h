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
#ifndef pyMatrix44_h_inc
#define pyMatrix44_h_inc

#include <array>
#include "hsMatrix44.h"
#include "pyGlueDefinitions.h"

class pyPoint3;
class pyVector3;

typedef std::array<float, 4*4> mat44_t;

class pyMatrix44
{
protected:
    pyMatrix44() { fMatrix.Reset(); }
    pyMatrix44(const hsMatrix44& other) : fMatrix(other) { }

public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptMatrix44);
    PYTHON_CLASS_NEW_DEFINITION;
    static PyObject *New(const hsMatrix44 &obj);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyMatrix44 object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyMatrix44); // converts a PyObject to a pyMatrix44 (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);

    hsMatrix44      fMatrix;

    // operator methods
    PyObject* operator*(const pyMatrix44& b) const { return pyMatrix44::New(fMatrix * b.fMatrix); } // returns pyMatrix44
    PyObject* operator*(const pyVector3& p) const;
    PyObject* operator*(const pyPoint3& p) const;

    // other methods
    PyObject* Copy() const { return pyMatrix44::New(fMatrix); } // returns pyMatrix44
    void Translate(const pyVector3& v);
    void Scale(const pyVector3& v);
    void Rotate(int axis, float radians) { fMatrix.Rotate(axis, radians); }
    void Reset() { fMatrix.Reset(); }
    void MakeTranslateMat(const pyVector3& trans);
    void MakeScaleMat(const pyVector3& scale);
    void MakeRotateMat(int axis, float radians) { fMatrix.MakeRotateMat(axis, radians); }
    void Make(const pyPoint3& from, const pyPoint3& at, const pyVector3& up);
    void MakeUpPreserving(const pyPoint3& from, const pyPoint3& at, const pyVector3& up);
    bool GetParity() const { return fMatrix.GetParity(); }
    float GetDeterminant() const { return fMatrix.GetDeterminant(); }
    PyObject* GetInverse(PyObject* inverse) const; // returns (and accepts) pyMatrix44
    PyObject* GetTranspose(PyObject* inverse) const; // returns (and accepts) pyMatrix44
    PyObject* GetAdjoint(PyObject* adjoint) const; // returns (and accepts) pyMatrix44
    PyObject* GetTranslate(PyObject* pt) const; // returns (and accepts) pyVector3
    PyObject* GetViewAxis() const; // returns pyVector3
    PyObject* GetUpAxis() const; // returns pyVector3
    PyObject* GetRightAxis() const; // returns pyVector3

    /** Returns a copy of the 4x4 matrix data */
    mat44_t GetData() const;
    void SetData(const float mat[]);
};


#endif // pyMatrix44_h_inc
