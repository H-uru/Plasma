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
#ifndef _pyVaultTextNoteNode_h_
#define _pyVaultTextNoteNode_h_

//////////////////////////////////////////////////////////////////////
//
// pyVaultTextNoteNode   - a wrapper class to provide interface to the RelVaultNode
//
//////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "hsStlUtils.h"
#include <python.h>
#include "pyGlueHelpers.h"

#include "pyVaultNode.h"

struct RelVaultNode;
class pyVaultAgeLinkNode;


class pyVaultTextNoteNode : public pyVaultNode
{
protected:
	// should only be created from C++ side
	pyVaultTextNoteNode(RelVaultNode* nfsNode);

	//create from the Python side
	pyVaultTextNoteNode();

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptVaultTextNoteNode);
	PYTHON_CLASS_NEW_DEFINITION;
	static PyObject *New(RelVaultNode* nfsNode);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyVaultTextNoteNode object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyVaultTextNoteNode); // converts a PyObject to a pyVaultTextNoteNode (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);


//==================================================================
// class RelVaultNode : public plVaultNode
//
	void Note_SetTitle( const char * text );
	void Note_SetTitleW( const wchar_t * text );
	std::string Note_GetTitle( void );
	std::wstring Note_GetTitleW( void );
	void Note_SetText( const char * text );
	void Note_SetTextW( const wchar_t * text );
	std::string Note_GetText( void );
	std::wstring Note_GetTextW( void );
	void Note_SetType( Int32 type );
	Int32 Note_GetType( void );

	void Note_SetSubType( Int32 type );
	Int32 Note_GetSubType( void );

	PyObject * GetDeviceInbox() const; // returns pyVaultFolderNode
	void SetDeviceInbox( const char * devName, PyObject * cb=nil, UInt32 cbContext=0 );
};

#endif // _pyVaultTextNoteNode_h_
