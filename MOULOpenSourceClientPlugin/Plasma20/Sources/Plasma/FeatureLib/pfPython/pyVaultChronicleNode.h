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
#ifndef _pyVaultChronicleNode_h_
#define _pyVaultChronicleNode_h_

//////////////////////////////////////////////////////////////////////
//
// pyVaultChronicleNode   - a wrapper class to provide interface to the RelVaultNode
//
//////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "hsStlUtils.h"

#include <python.h>
#include "pyGlueHelpers.h"

#include "pyVaultNode.h"

struct RelVaultNode;


class pyVaultChronicleNode : public pyVaultNode
{
	char *	ansiName;
	char *	ansiValue;
	
protected:
	// should only be created from C++ side
	pyVaultChronicleNode(RelVaultNode* nfsNode);

	//create from the Python side
	pyVaultChronicleNode(int n=0);

public:
	~pyVaultChronicleNode ();
	
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptVaultChronicleNode);
	static PyObject *New(RelVaultNode* nfsNode);
	static PyObject *New(int n=0);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyVaultChronicleNode object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyVaultChronicleNode); // converts a PyObject to a pyVaultChronicleNode (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

//==================================================================
// class RelVaultNode : public plVaultNode
//
	void Chronicle_SetName( const char * text );
	const char * Chronicle_GetName( void );
	void Chronicle_SetValue( const char * text );
	const char * Chronicle_GetValue( void );
	void Chronicle_SetType( UInt32 type );
	UInt32 Chronicle_GetType( void );
};

#endif // _pyVaultChronicleNode_h_
