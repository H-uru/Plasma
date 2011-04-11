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
#ifndef _pyVaultFolderNode_h_
#define _pyVaultFolderNode_h_

//////////////////////////////////////////////////////////////////////
//
// pyVaultFolderNode   - a wrapper class to provide interface to the plVaultNode
//
//////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "hsStlUtils.h"

#include <python.h>
#include "pyGlueHelpers.h"

#include "pyVaultNode.h"

struct RelVaultNode;

	
class pyVaultFolderNode : public pyVaultNode
{
protected:
	// should only be created from C++ side
	pyVaultFolderNode(RelVaultNode* nfsNode);

	//create from the Python side
	pyVaultFolderNode(int n=0);

public:
	~pyVaultFolderNode();

	// required functions for PyObject interoperability
	PYTHON_EXPOSE_TYPE; // so we can subclass
	PYTHON_CLASS_NEW_FRIEND(ptVaultFolderNode);
	static PyObject *New(RelVaultNode* nfsNode);
	static PyObject *New(int n=0);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyVaultFolderNode object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyVaultFolderNode); // converts a PyObject to a pyVaultFolderNode (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	virtual void	Folder_SetType( int type );
	virtual int		Folder_GetType( void );
	void	Folder_SetName( std::string name );
	void	Folder_SetNameW( std::wstring name );
	std::string Folder_GetName( void );
	std::wstring Folder_GetNameW( void );



};

#endif // _pyVaultFolderNode_h_
