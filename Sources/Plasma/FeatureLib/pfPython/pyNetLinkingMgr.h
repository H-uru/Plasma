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
#ifndef pyNetLinkingMgr_h_inc
#define pyNetLinkingMgr_h_inc

#include "hsTypes.h"
#include "hsStlUtils.h"

#include <python.h>
#include "pyGlueHelpers.h"

//////////////////////////////////////////////////////////////////////
//
// pyNetLinkingMgr   - a wrapper class to provide interface to the plNetLinkingMgr
//
//////////////////////////////////////////////////////////////////////

class pyAgeInfoStruct;
class pyAgeLinkStruct;
class pyAgeLinkStructRef;

class pyNetLinkingMgr
{
#ifndef BUILDING_PYPLASMA
protected:
	pyNetLinkingMgr() {}

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptNetLinkingMgr);
	PYTHON_CLASS_NEW_DEFINITION;
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyNetLinkingMgr object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyNetLinkingMgr); // converts a PyObject to a pyNetLinkingMgr (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);
#else
public:
#endif // BUILDING_PYPLASMA
	static void AddPlasmaConstantsClasses(PyObject *m);

#ifndef BUILDING_PYPLASMA
	// enable/disable linking
	hsBool IsEnabled( void ) const;
	void SetEnabled( hsBool b );

	// Link to a public instance. PLS will load balance.
	void LinkToAge( pyAgeLinkStruct & link, const char* linkAnim );
	// Link to my Personal Age
	void LinkToMyPersonalAge();
	// link to my personal age with the YeehsaBook
	void LinkToMyPersonalAgeWithYeeshaBook();
	// Link to my Neighborhood Age
	void LinkToMyNeighborhoodAge();
	// Link player to my current age
	void LinkPlayerHere( UInt32 playerID );
	// Link player to specified age
	void LinkPlayerToAge( pyAgeLinkStruct & link, UInt32 playerID );
	// Link to player's current age
	void LinkToPlayersAge( UInt32 playerID );

	PyObject* GetCurrAgeLink(); // returns pyAgeLinkStructRef
	PyObject* GetPrevAgeLink(); // returns pyAgeLinkStructRef
#endif // BUILDING_PYPLASMA
};

#endif // pyNetLinkingMgr_h_inc
