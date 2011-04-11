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
#ifndef _pyVaultNodeRef_h_
#define _pyVaultNodeRef_h_

//////////////////////////////////////////////////////////////////////
//
// pyVaultNodeRef   - a wrapper class to provide interface to the plVaultNodeRef
//
//////////////////////////////////////////////////////////////////////

#include "hsTypes.h"

#include <python.h>
#include "pyGlueHelpers.h"

struct RelVaultNode;

class pyVaultNodeRef
{
	RelVaultNode *	fParent;
	RelVaultNode *	fChild;

protected:
	// should only be created from C++ side
	pyVaultNodeRef(RelVaultNode * parent, RelVaultNode * child);
	pyVaultNodeRef(int =0 );

public:
	~pyVaultNodeRef();
	
	RelVaultNode *	GetParentNode () const { return fParent; }
	RelVaultNode *	GetChildNode () const { return fChild; }

	// required functions for PyObject interoperability
	PYTHON_EXPOSE_TYPE; // so we can subclass
	PYTHON_CLASS_NEW_FRIEND(ptVaultNodeRef);
	static PyObject *New(RelVaultNode * parent, RelVaultNode * child);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyVaultNodeRef object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyVaultNodeRef); // converts a PyObject to a pyVaultNodeRef (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	PyObject * GetParent ();
	PyObject * GetChild ();
	PyObject * GetSaver ();	// returns pyVaultPlayerInfoNode
	bool BeenSeen ();
	void SetSeen (bool v);
	
	unsigned GetParentID ();
	unsigned GetChildID ();
	unsigned GetSaverID ();
};

#endif // _pyVaultNodeRef_h_
