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
#ifndef _pyVaultMarkerGameNode_h_
#define _pyVaultMarkerGameNode_h_

//////////////////////////////////////////////////////////////////////
//
// pyVaultMarkerGameNode   - a wrapper class to provide interface to the RelVaultNode
//
//////////////////////////////////////////////////////////////////////

#include "hsTypes.h"

#include <python.h>
#include "pyGlueHelpers.h"
#include "pyVaultNode.h"

#include "../pnNetBase/pnNetBase.h"

struct RelVaultNode;

class pyVaultMarkerGameNode : public pyVaultNode
{
private:
	mutable char fGameName[kMaxVaultNodeStringLength];
	mutable char fGameGuid[64];

protected:
	// should only be created from C++ side
	pyVaultMarkerGameNode(RelVaultNode* vaultNode);

	//create from the Python side
	pyVaultMarkerGameNode(int n=0);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptVaultMarkerGameNode);
	static PyObject *New(RelVaultNode* vaultNode);
	static PyObject *New(int n=0);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyVaultMarkerGameNode object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyVaultMarkerGameNode); // converts a PyObject to a pyVaultMarkerGameNode (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

//==================================================================
// class RelVaultNode : public plVaultNode
//
	const char *	GetGameName () const;
	void			SetGameName (const char v[]);

	const char *	GetGameGuid () const;
	void			SetGameGuid (const char v[]);
};

#endif // _pyVaultMarkerGameNode_h_
