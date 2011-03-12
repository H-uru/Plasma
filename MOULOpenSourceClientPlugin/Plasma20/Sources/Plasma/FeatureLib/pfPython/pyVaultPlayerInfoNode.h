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
#ifndef _pyVaultPlayerInfoNode_h_
#define _pyVaultPlayerInfoNode_h_

//////////////////////////////////////////////////////////////////////
//
// pyVaultPlayerInfoNode   - a wrapper class to provide interface to the plVaultPlayerInfoNode
//
//////////////////////////////////////////////////////////////////////

#include "pyVaultNode.h"
#include <python.h>
#include "pyGlueHelpers.h"

class pyVaultPlayerInfoNode : public pyVaultNode
{
	mutable char *		ansiPlayerName;
	mutable char *		ansiAgeInstName;
	mutable char		ansiAgeInstUuid[256];

protected:
	// should only be created from C++ side
	pyVaultPlayerInfoNode(RelVaultNode * node);

	//create from the Python side
	pyVaultPlayerInfoNode();

public:
	~pyVaultPlayerInfoNode ();

	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptVaultPlayerInfoNode);
	PYTHON_CLASS_NEW_DEFINITION;
	static PyObject *New(RelVaultNode * node);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyVaultPlayerInfoNode object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyVaultPlayerInfoNode); // converts a PyObject to a pyVaultPlayerInfoNode (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

//==================================================================
// class plVaultPlayerInfoNode : public plVaultNode
//
	void	Player_SetPlayerID( UInt32 plyrid );
	UInt32	Player_GetPlayerID( void );
	void	Player_SetPlayerName( const char * name );
	const char * Player_GetPlayerName( void );

	// age the player is currently in, if any.
	void	Player_SetAgeInstanceName( const char * agename );
	const char * Player_GetAgeInstanceName( void );
	void	Player_SetAgeGuid( const char * guidtext);
	const char * Player_GetAgeGuid( void );
	// online status
	void	Player_SetOnline( bool b );
	hsBool	Player_IsOnline( void );

	int		Player_GetCCRLevel( void );
};

#endif // _pyVaultPlayerInfoNode_h_
