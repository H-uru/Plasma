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

#include "pyGlueHelpers.h"
#include "pyGeometry3.h"
#include "pyMatrix44.h"

PyObject* pyMatrix44::operator*(const pyVector3& p) const
{
    return pyVector3::New(fMatrix * p.fVector);
}

PyObject* pyMatrix44::operator*(const pyPoint3& p) const
{
    return pyPoint3::New(fMatrix * p.fPoint);
}

void pyMatrix44::Translate(const pyVector3& v)
{
    fMatrix.Translate(&v.fVector);
}

void pyMatrix44::Scale(const pyVector3& v)
{
    fMatrix.Scale(&v.fVector);
}

void pyMatrix44::MakeTranslateMat(const pyVector3& trans)
{
    fMatrix.MakeTranslateMat(&trans.fVector);
}

void pyMatrix44::MakeScaleMat(const pyVector3& scale)
{
    fMatrix.MakeScaleMat(&scale.fVector);
}

void pyMatrix44::Make(const pyPoint3& from, const pyPoint3& at, const pyVector3& up)
{
    fMatrix.Make(&from.fPoint, &at.fPoint, &up.fVector);
}

void pyMatrix44::MakeUpPreserving(const pyPoint3& from, const pyPoint3& at, const pyVector3& up)
{
    fMatrix.MakeUpPreserving(&from.fPoint, &at.fPoint, &up.fVector);
}

PyObject* pyMatrix44::GetInverse(PyObject* inverse) const
{
    pyMatrix44 *obj = pyMatrix44::ConvertFrom(inverse);
    fMatrix.GetInverse(&(obj->fMatrix));
    Py_INCREF(inverse); // incref it because we need to return a new ref
    return inverse;
}

PyObject* pyMatrix44::GetTranspose(PyObject* transpose) const
{
    pyMatrix44 *obj = pyMatrix44::ConvertFrom(transpose);
    fMatrix.GetTranspose(&(obj->fMatrix));
    Py_INCREF(transpose); // incref it because we need to return a new ref
    return transpose;
}

PyObject* pyMatrix44::GetAdjoint(PyObject* adjoint) const
{
    pyMatrix44 *obj = pyMatrix44::ConvertFrom(adjoint);
    fMatrix.GetAdjoint(&(obj->fMatrix));
    Py_INCREF(adjoint); // incref it because we need to return a new ref
    return adjoint;
}

PyObject* pyMatrix44::GetTranslate(PyObject* pt) const
{
    pyVector3 *obj = pyVector3::ConvertFrom(pt);
    fMatrix.GetTranslate(&(obj->fVector));
    Py_INCREF(pt); // incref it because we need to return a new ref
    return pt;
}

PyObject* pyMatrix44::GetViewAxis() const
{
    return pyVector3::New(fMatrix.GetAxis(hsMatrix44::kView));
}

PyObject* pyMatrix44::GetUpAxis() const
{
    return pyVector3::New(fMatrix.GetAxis(hsMatrix44::kUp));
}

PyObject* pyMatrix44::GetRightAxis() const
{
    return pyVector3::New(fMatrix.GetAxis(hsMatrix44::kRight));
}

mat44_t pyMatrix44::GetData() const
{
    mat44_t res;
    res[0] = fMatrix.fMap[0][0];  res[1] = fMatrix.fMap[0][1];  res[2] = fMatrix.fMap[0][2];  res[3] = fMatrix.fMap[0][3];
    res[4] = fMatrix.fMap[1][0];  res[5] = fMatrix.fMap[1][1];  res[6] = fMatrix.fMap[1][2];  res[7] = fMatrix.fMap[1][3];
    res[8] = fMatrix.fMap[2][0];  res[9] = fMatrix.fMap[2][1];  res[10] = fMatrix.fMap[2][2]; res[11] = fMatrix.fMap[2][3];
    res[12] = fMatrix.fMap[3][0]; res[13] = fMatrix.fMap[3][1]; res[14] = fMatrix.fMap[3][2]; res[15] = fMatrix.fMap[3][3];

    return res;
}

void pyMatrix44::SetData(const float mat[])
{
    fMatrix.fMap[0][0] = mat[0];
    fMatrix.fMap[0][1] = mat[1];
    fMatrix.fMap[0][2] = mat[2];
    fMatrix.fMap[0][3] = mat[3];

    fMatrix.fMap[1][0] = mat[4];
    fMatrix.fMap[1][1] = mat[5];
    fMatrix.fMap[1][2] = mat[6];
    fMatrix.fMap[1][3] = mat[7];

    fMatrix.fMap[2][0] = mat[8];
    fMatrix.fMap[2][1] = mat[9];
    fMatrix.fMap[2][2] = mat[10];
    fMatrix.fMap[2][3] = mat[11];

    fMatrix.fMap[3][0] = mat[12];
    fMatrix.fMap[3][1] = mat[13];
    fMatrix.fMap[3][2] = mat[14];
    fMatrix.fMap[3][3] = mat[15];
    fMatrix.NotIdentity();
}
