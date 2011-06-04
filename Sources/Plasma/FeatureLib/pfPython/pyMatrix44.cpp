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

pyMatrix44::pyMatrix44() { fMatrix.Reset(); }
pyMatrix44::pyMatrix44(hsMatrix44 other)
{	// copy the other matrix to this one
	int i,j;
	for ( i=0;i<4;i++)
		for ( j=0;j<4;j++)
			fMatrix.fMap[i][j] = other.fMap[i][j];
	fMatrix.fFlags = other.fFlags;
}

PyObject* pyMatrix44::GetInverse(PyObject* inverse)
{
	pyMatrix44 *obj = pyMatrix44::ConvertFrom(inverse);
	fMatrix.GetInverse(&(obj->fMatrix));
	Py_INCREF(inverse); // incref it because we need to return a new ref
	return inverse;
}

PyObject* pyMatrix44::GetTranspose(PyObject* transpose)
{
	pyMatrix44 *obj = pyMatrix44::ConvertFrom(transpose);
	fMatrix.GetTranspose(&(obj->fMatrix));
	Py_INCREF(transpose); // incref it because we need to return a new ref
	return transpose;
}

PyObject* pyMatrix44::GetAdjoint(PyObject* adjoint)
{
	pyMatrix44 *obj = pyMatrix44::ConvertFrom(adjoint);
	fMatrix.GetAdjoint(&(obj->fMatrix));
	Py_INCREF(adjoint); // incref it because we need to return a new ref
	return adjoint;
}

PyObject* pyMatrix44::GetTranslate(PyObject* pt)
{
	pyVector3 *obj = pyVector3::ConvertFrom(pt);
	fMatrix.GetTranslate(&(obj->fVector));
	Py_INCREF(pt); // incref it because we need to return a new ref
	return pt;
}

std::vector< std::vector<hsScalar> > pyMatrix44::GetData()
{
	std::vector<hsScalar> row0, row1, row2, row3;
	row0.push_back(fMatrix.fMap[0][0]); row0.push_back(fMatrix.fMap[0][1]); row0.push_back(fMatrix.fMap[0][2]); row0.push_back(fMatrix.fMap[0][3]);
	row1.push_back(fMatrix.fMap[1][0]); row1.push_back(fMatrix.fMap[1][1]); row1.push_back(fMatrix.fMap[1][2]); row1.push_back(fMatrix.fMap[1][3]);
	row2.push_back(fMatrix.fMap[2][0]); row2.push_back(fMatrix.fMap[2][1]); row2.push_back(fMatrix.fMap[2][2]); row2.push_back(fMatrix.fMap[2][3]);
	row3.push_back(fMatrix.fMap[3][0]); row3.push_back(fMatrix.fMap[3][1]); row3.push_back(fMatrix.fMap[3][2]); row3.push_back(fMatrix.fMap[3][3]);

	std::vector< std::vector<hsScalar> > pyMat;
	pyMat.push_back(row0);
	pyMat.push_back(row1);
	pyMat.push_back(row2);
	pyMat.push_back(row3);

	return pyMat;
}

void pyMatrix44::SetData(const std::vector< std::vector<hsScalar> > & mat)
{
	// make sure they are passing us the correct size
	if ( mat.size() == 4 )
	{
		int i,j;
		for ( i=0;i<3;i++)
		{
			std::vector<hsScalar> pyrow = mat[i];
			if ( pyrow.size() == 4 )
			{
				for ( j=0;j<3;j++)
					fMatrix.fMap[i][j] = pyrow[j];
			}
			else  // not enough ... throw error
			{
				char errmsg[256];
				sprintf(errmsg, "Wrong number of elements in row of matrix");
				PyErr_SetString(PyExc_TypeError, errmsg);
				return;
			}
		}
	}
	else
	{
		char errmsg[256];
		sprintf(errmsg, "Wrong number of rows in the matrix");
		PyErr_SetString(PyExc_TypeError, errmsg);
	}
}
