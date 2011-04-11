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
#ifndef _pyVaultAgeInfoNode_h_
#define _pyVaultAgeInfoNode_h_

//////////////////////////////////////////////////////////////////////
//
// pyVaultAgeInfoNode   - a wrapper class to provide interface to the RelVaultNode
//
//////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "hsStlUtils.h"

#include <python.h>
#include "pyGlueHelpers.h"

#include "pyVaultNode.h"

struct RelVaultNode;
class pyVaultSDLNode;
class pyAgeInfoStruct;


class pyVaultAgeInfoNode : public pyVaultNode
{
private:
	mutable char fAgeInstGuid[64];
	mutable std::string fAgeFilename;
	mutable std::string fAgeInstName;
	mutable std::string fAgeUserName;
	mutable std::string fAgeDispName;
	mutable std::string fAgeDescription;

protected:
	// should only be created from C++ side
	pyVaultAgeInfoNode(RelVaultNode* vaultNode);

	//create from the Python side
	pyVaultAgeInfoNode(int n=0);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptVaultAgeInfoNode);
	static PyObject *New(RelVaultNode* vaultNode);
	static PyObject *New(int n=0);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyVaultAgeInfoNode object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyVaultAgeInfoNode); // converts a PyObject to a pyVaultAgeInfoNode (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

//==================================================================
// class RelVaultNode : public plVaultNode
//
	PyObject *	GetCanVisitFolder() const; // returns pyVaultPlayerInfoListNode
	PyObject * GetAgeOwnersFolder() const; // returns pyVaultPlayerInfoListNode
	PyObject* GetChildAgesFolder( void ); // returns pyVaultFolderNode
	PyObject *	GetAgeSDL() const; // returns pyVaultSDLNode
	PyObject * GetCzar() const; // returns pyVaultPlayerInfoNode

	PyObject * GetParentAgeLink () const;	// returns pyVaultAgeLinkNode, or None if not a child age.

	const char * GetAgeFilename() const;
	void	SetAgeFilename( const char * v );

	const char * GetAgeInstanceName() const;
	void	SetAgeInstanceName( const char * v );

	const char * GetAgeUserDefinedName() const;
	void	SetAgeUserDefinedName( const char * v );

	const char * GetAgeInstanceGuid() const;
	void	SetAgeInstanceGuid( const char * guid );

	const char * GetAgeDescription() const;
	void	SetAgeDescription( const char * v );

	Int32	GetSequenceNumber() const;
	void	SetSequenceNumber( Int32 v );
	
	Int32	GetAgeLanguage() const;
	void	SetAgeLanguage( Int32 v );

	UInt32	GetAgeID() const;
	void	SetAgeID( UInt32 v );

	UInt32	GetCzarID() const;

	bool	IsPublic() const;

	const char * GetDisplayName() const;

	PyObject * AsAgeInfoStruct() const; // returns pyAgeInfoStruct
};

#endif // _pyVaultAgeInfoNode_h_
