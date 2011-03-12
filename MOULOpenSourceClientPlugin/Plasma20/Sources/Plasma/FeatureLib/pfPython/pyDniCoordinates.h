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
#ifndef _pyDniCoordinates_h_
#define _pyDniCoordinates_h_

//////////////////////////////////////////////////////////////////////
//
// pyDniCoordinates   - the wrapper class for D'ni coordinates system
//
//////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "hsGeometry3.h"

#include <python.h>
#include "pyGlueHelpers.h"

class plDniCoordinateInfo;

class pyDniCoordinates
{
private:
	plDniCoordinateInfo* fCoords;

protected:
	pyDniCoordinates();
	pyDniCoordinates(plDniCoordinateInfo* coord);

public:
	~pyDniCoordinates();

	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptDniCoordinates);
	PYTHON_CLASS_NEW_DEFINITION;
	static PyObject *New(plDniCoordinateInfo* coord);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyDniCoordinates object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyDniCoordinates); // converts a PyObject to a pyDniCoordinates (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	// python get attributes helpers
	int			GetHSpans() const;
	int			GetVSpans() const;
	int			GetTorans() const;
	void		UpdateCoordinates();
	void		FromPoint(const hsPoint3& pt);
};


#endif // _pyDniCoordinates_h_
