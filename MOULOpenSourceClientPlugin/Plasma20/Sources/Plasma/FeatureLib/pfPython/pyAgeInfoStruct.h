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
#ifndef pyAgeInfoStruct_h_inc
#define pyAgeInfoStruct_h_inc

#include "hsTypes.h"
#include "hsStlUtils.h"
#include "../plNetCommon/plNetServerSessionInfo.h"

#include <python.h>
#include "pyGlueHelpers.h"

//////////////////////////////////////////////////////////////////////
//
// pyAgeInfoStruct   - a wrapper class to provide interface to the plAgeInfoStruct
//
//////////////////////////////////////////////////////////////////////

class pyVaultAgeInfoNode;
class pyAgeInfoStructRef;


class pyAgeInfoStruct
{
private:
	plAgeInfoStruct fAgeInfo;
	mutable std::string fAgeInstanceGuidStr;	// for getting Age Instance GUID
	mutable std::string	fDisplayName;			// used by GetDisplayName()

protected:
	pyAgeInfoStruct();
	pyAgeInfoStruct(plAgeInfoStruct * info);

public:
	~pyAgeInfoStruct();

	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptAgeInfoStruct);
	PYTHON_CLASS_NEW_DEFINITION;
	static PyObject *New(plAgeInfoStruct *info);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyAgeInfoStruct object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyAgeInfoStruct); // converts a PyObject to a pyAgeInfoStruct (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	bool operator==(const pyAgeInfoStruct &other) const;
	bool operator!=(const pyAgeInfoStruct &other) const { return !(other==*this); }
	plAgeInfoStruct * GetAgeInfo() { return &fAgeInfo; }
	const plAgeInfoStruct * GetAgeInfo() const { return &fAgeInfo; }
	static void PythonModDef();
	void	CopyFrom( const pyAgeInfoStruct & other );
	void	CopyFromRef( const pyAgeInfoStructRef & other );
	const char * GetAgeFilename() const;
	void	SetAgeFilename( const char * v );
	const char * GetAgeInstanceName() const;
	void	SetAgeInstanceName( const char * v );
	const char * GetAgeUserDefinedName() const;
	void	SetAgeUserDefinedName( const char * v );
	const char * GetAgeDescription() const;
	void	SetAgeDescription( const char * v );
	const char * GetAgeInstanceGuid() const;
	void	SetAgeInstanceGuid( const char * guid );
	Int32	GetAgeSequenceNumber() const;
	void	SetAgeSequenceNumber( Int32 v );
	Int32	GetAgeLanguage() const;
	void	SetAgeLanguage( Int32 v );
	const char * GetDisplayName() const;
};

class pyAgeInfoStructRef
{
private:
	static plAgeInfoStruct fDefaultAgeInfo; // created so a default constructor could be made for python. Do NOT use

	plAgeInfoStruct & fAgeInfo;
	mutable std::string fAgeInstanceGuidStr;	// for getting Age Instance GUID
	mutable std::string	fDisplayName;			// used by GetDisplayName()

protected:
	pyAgeInfoStructRef(): fAgeInfo( fDefaultAgeInfo ) {} // only here for the python glue... do NOT call directly
	pyAgeInfoStructRef(plAgeInfoStruct & info): fAgeInfo( info ){}

public:
	~pyAgeInfoStructRef() {}

	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptAgeInfoStructRef);
	static PyObject *New(plAgeInfoStruct &info);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyAgeInfoStructRef object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyAgeInfoStructRef); // converts a PyObject to a pyAgeInfoStructRef (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	plAgeInfoStruct * GetAgeInfo() { return &fAgeInfo; }
	const plAgeInfoStruct * GetAgeInfo() const { return &fAgeInfo; }
	void	CopyFrom( const pyAgeInfoStruct & other );
	void	CopyFromRef( const pyAgeInfoStructRef & other );
	const char * GetAgeFilename() const;
	void	SetAgeFilename( const char * v );
	const char * GetAgeInstanceName() const;
	void	SetAgeInstanceName( const char * v );
	const char * GetAgeUserDefinedName() const;
	void	SetAgeUserDefinedName( const char * v );
	const char * GetAgeInstanceGuid() const;
	void	SetAgeInstanceGuid( const char * guid );
	Int32	GetAgeSequenceNumber() const;
	void	SetAgeSequenceNumber( Int32 v );
	const char * GetDisplayName() const;
};

#endif // pyAgeInfoStruct_h_inc