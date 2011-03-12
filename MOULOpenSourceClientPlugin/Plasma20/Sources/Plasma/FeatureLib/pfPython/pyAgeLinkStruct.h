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
#ifndef pyAgeLinkStruct_h_inc
#define pyAgeLinkStruct_h_inc

#include "hsTypes.h"
#include "hsStlUtils.h"
#include "../plNetCommon/plNetServerSessionInfo.h"
#include "pyAgeInfoStruct.h"

#include <python.h>
#include "pyGlueHelpers.h"

//////////////////////////////////////////////////////////////////////
//
// pyAgeLinkStruct   - a wrapper class to provide interface to the plAgeLinkStruct
//
//////////////////////////////////////////////////////////////////////

class pyVaultAgeLinkNode;
class pySpawnPointInfo;
class pySpawnPointInfoRef;
class pyAgeLinkStructRef;

class pyAgeLinkStruct
{
private:
	plAgeLinkStruct fAgeLink;

protected:
	pyAgeLinkStruct();
	pyAgeLinkStruct( plAgeLinkStruct * link );

public:
	~pyAgeLinkStruct();

	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptAgeLinkStruct);
	PYTHON_CLASS_NEW_DEFINITION;
	static PyObject *New(plAgeLinkStruct* link);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyAgeLinkStruct object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyAgeLinkStruct); // converts a PyObject to a pyAgeLinkStruct (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	bool operator==(const pyAgeLinkStruct &other) const;
	bool operator!=(const pyAgeLinkStruct &other) const { return !(other==*this); }
	plAgeLinkStruct * GetAgeLink() { return &fAgeLink; }
	const plAgeLinkStruct * GetAgeLink() const { return &fAgeLink; }
	PyObject * GetAgeInfo(); // returns pyAgeInfoStructRef
	void	SetAgeInfo( pyAgeInfoStruct & info );
	const char* GetParentAgeFilename();
	void	SetParentAgeFilename( const char* parentname );
	void	CopyFrom( const pyAgeLinkStruct & other );
	void	CopyFromRef( const pyAgeLinkStructRef & other );
	void	SetLinkingRules( int v );
	int		GetLinkingRules() const;
	void	SetSpawnPoint( pySpawnPointInfo & v );
	void	SetSpawnPointRef( pySpawnPointInfoRef & v );
	PyObject * GetSpawnPoint(); // returns pySpawnPointInfoRef
};


class pyAgeLinkStructRef
{
private:
	static plAgeLinkStruct fDefaultLinkStruct; // created so a default constructor could be made for python, do NOT use
	plAgeLinkStruct & fAgeLink;

protected:
	pyAgeLinkStructRef(): fAgeLink(fDefaultLinkStruct) {} // only used by python glue, do NOT call directly
	pyAgeLinkStructRef( plAgeLinkStruct & link ):fAgeLink(link) {}

public:
	~pyAgeLinkStructRef(){}

	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptAgeLinkStructRef);
	static PyObject *New(plAgeLinkStruct& link);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyAgeLinkStructRef object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyAgeLinkStructRef); // converts a PyObject to a pyAgeLinkStructRef (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	plAgeLinkStruct * GetAgeLink() { return &fAgeLink; }
	const plAgeLinkStruct * GetAgeLink() const { return &fAgeLink; }
	PyObject * GetAgeInfo(); // returns pyAgeInfoStructRef
	void	SetAgeInfo( pyAgeInfoStruct & info );
	void	CopyFrom( const pyAgeLinkStruct & other );
	void	CopyFromRef( const pyAgeLinkStructRef & other );
	void	SetLinkingRules( int v );
	int		GetLinkingRules() const;
	void	SetSpawnPoint( pySpawnPointInfo & v );
	void	SetSpawnPointRef( pySpawnPointInfoRef & v );
	PyObject * GetSpawnPoint(); // returns pySpawnPointInfoRef
};


#endif // pyAgeLinkStruct_h_inc