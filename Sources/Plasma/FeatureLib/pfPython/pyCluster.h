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
#ifndef pyCluster_h
#define pyCluster_h

#include "pyKey.h"

#include <python.h>
#include "pyGlueHelpers.h"

//////////////////////////////////////////////////////////////////////
//
// pyCluster   - a wrapper class to provide interface to cluster objects
//
//////////////////////////////////////////////////////////////////////

class pyCluster
{
private:
	plKey fClusterKey;

protected:
	pyCluster(): fClusterKey(nil) {} // for python glue only, do NOT call
	pyCluster(plKey key);
	pyCluster(pyKey& key);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptCluster);
	static PyObject *New(plKey key);
	static PyObject *New(pyKey& key);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyCluster object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyCluster); // converts a PyObject to a pyCluster (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	void SetKey(pyKey& key) {fClusterKey = key.getKey();} // for python glue only, do NOT call

	void SetVisible(bool visible);
};


#endif // pyCluster_h