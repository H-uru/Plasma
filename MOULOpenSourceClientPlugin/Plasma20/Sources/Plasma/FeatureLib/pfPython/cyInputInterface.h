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
#ifndef cyInputInterface_h
#define cyInputInterface_h

/////////////////////////////////////////////////////////////////////////////
//
// NAME: cyInputInterface
//
// PURPOSE: Class wrapper to map InputInterface functions to plasma2 message
//

#include <python.h>
#include "pyGlueHelpers.h"

class plInputInterface;

class cyInputInterface
{
protected:
	plInputInterface* fTelescopeInterface;

	cyInputInterface();
public:
	~cyInputInterface();

	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptInputInterface);
	PYTHON_CLASS_NEW_DEFINITION;
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a cyInputInterface object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(cyInputInterface); // converts a PyObject to a cyInputInterface (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	// setters

	// we should add a ::Push_X_Interface function for any special type
	// of interface we might want to set... for now there's just the telescope...
	void PushTelescopeInterface();
	void PopTelescope();


	/////////////////////////////////////////////////////////////////////////////
};


#endif  // cyInputInterface_h
