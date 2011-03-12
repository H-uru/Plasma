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
#ifndef pyDniInfoSource_h_inc
#define pyDniInfoSource_h_inc

#include "hsTypes.h"
#include "hsStlUtils.h"


#include <python.h>
#include "pyGlueHelpers.h"

class pyDniCoordinates;

class pyDniInfoSource
{
private:
	mutable char * fAgeName;
	mutable char fAgeGuid[MAX_PATH];

protected:
	pyDniInfoSource();

public:
	~pyDniInfoSource();

	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptDniInfoSource);
	PYTHON_CLASS_NEW_DEFINITION;
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyDniInfoSource object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyDniInfoSource); // converts a PyObject to a pyDniInfoSource (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	// current coords of the player in current age as a pyDniCoordinates
	PyObject* GetAgeCoords( void ); // returns pyDniCoordinates
	// current time in current age (tbd)
	UInt32			GetAgeTime( void ) const;
	// name of current age
	const char *	GetAgeName( void ) const;
	// unique identifier for this age instance
	const char *	GetAgeGuid( void ) const;
};


#endif // pyDniInfoSource_h_inc
