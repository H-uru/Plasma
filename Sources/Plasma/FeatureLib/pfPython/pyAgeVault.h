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
#ifndef _pyAgeVault_h_
#define _pyAgeVault_h_

#ifdef BUILDING_PYPLASMA
# error "pyAgeVault is not compatible with pyPlasma.pyd. Use BUILDING_PYPLASMA macro to ifdef out unwanted headers."
#endif

//////////////////////////////////////////////////////////////////////
//
// pyAgeVault   - a wrapper class to provide interface to the plVaultAgeNode
//
//////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "hsStlUtils.h"

#include <python.h>
#include "pyGlueHelpers.h"

class pyVaultNode;
class pyVaultFolderNode;
class pyVaultPlayerInfoListNode;
class pyVaultAgeLinkNode;
class pyVaultAgeInfoNode;
class pyAgeInfoStruct;
class pyVaultChronicleNode;
class pySDLStateDataRecord;
class pyVaultTextNoteNode;

class pyAgeVault
{
private:
	mutable char fAgeGuid[MAX_PATH];	// for getting Age GUID

protected:
	pyAgeVault();

public:
	~pyAgeVault();

	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptAgeVault);
	PYTHON_CLASS_NEW_DEFINITION;
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyAgeVault object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyAgeVault); // converts a PyObject to a pyAgeVault (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	const char*		GetAgeGuid( void );

	PyObject *		GetAgeSDL() const; // returns pySDLStateDataRecord
	void			UpdateAgeSDL( pySDLStateDataRecord & pyrec );

	PyObject*		GetAgeInfo(); // returns pyVaultAgeInfoNode
	PyObject*		GetAgeDevicesFolder( void ); // returns pyVaultFolderNode
	PyObject*		GetSubAgesFolder( void ); // returns pyVaultFolderNode
	PyObject*		GetChronicleFolder( void ); // returns pyVaultFolderNode
	// Age chronicle (not the player chronicle!)
	PyObject*		FindChronicleEntry( const char * entryName ); // returns pyVaultChronicleNode
	void AddChronicleEntry( const char * name, UInt32 type, const char * value );
	// Players who have published to devices in this age
	PyObject*		GetPeopleIKnowAboutFolder( void ); // returns pyVaultPlayerInfoListNode
	// PERSONAL AGE SPECIFIC
	PyObject*		GetBookshelfFolder ( void ); // returns pyVaultFolderNode
	// NEXUS SPECIFIC
	PyObject*		GetPublicAgesFolder( void ); // returns pyVaultFolderNode
	PyObject*		GetSubAgeLink( const pyAgeInfoStruct & info ); // returns pyVaultAgeLinkNode
	// AGE DEVICES. AKA IMAGERS, WHATEVER.
	// Add a new device.
	void AddDevice( const char * deviceName, PyObject * cb=nil, UInt32 cbContext=0 );
	// Remove a device.
	void RemoveDevice( const char * deviceName );
	// True if device exists in age.
	bool HasDevice( const char * deviceName );
	// Get the device node by name.
	PyObject * GetDevice( const char * deviceName ); // returns pyVaultTextNoteNode
	// Sets the inbox associated with a device.
	void SetDeviceInbox( const char * deviceName, const char * inboxName, PyObject * cb=nil, UInt32 cbContext=0 );
	// Get the inbox associated with a device.
	PyObject * GetDeviceInbox( const char * deviceName ); // returns pyVaultFolderNode
	// find matching node
	PyObject* FindNode( pyVaultNode* templateNode ) const; // returns pyVaultNode
};

#endif // _pyAgeVault_h_
