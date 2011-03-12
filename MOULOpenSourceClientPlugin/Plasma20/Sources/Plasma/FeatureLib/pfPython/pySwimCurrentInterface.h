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
#ifndef pySwimCurrentInterface_h
#define pySwimCurrentInterface_h

#include "pyKey.h"

#include <python.h>
#include "pyGlueHelpers.h"

class pySwimCurrentInterface
{
private:
	plKey fSwimCurrentKey;

protected:
	pySwimCurrentInterface(): fSwimCurrentKey(nil) {} // for python glue only, do NOT call
	pySwimCurrentInterface(plKey key);
	pySwimCurrentInterface(pyKey& key);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptSwimCurrentInterface);
	static PyObject *New(plKey key);
	static PyObject *New(pyKey& key);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pySwimCurrentInterface object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pySwimCurrentInterface); // converts a PyObject to a pySwimCurrentInterface (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	void setKey(pyKey& key) {fSwimCurrentKey = key.getKey();} // for python glue only, do NOT call

	hsScalar getNearDist();
	void setNearDist(hsScalar val);

	hsScalar getFarDist();
	void setFarDist(hsScalar val);

	hsScalar getNearVel();
	void setNearVel(hsScalar val);

	hsScalar getFarVel();
	void setFarVel(hsScalar val);

	hsScalar getRotation();
	void setRotation(hsScalar val);

	void enable();
	void disable();
};

#endif