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
//////////////////////////////////////////////////////////////////////////////
//
//	plWinRegistryTools
//	Utility class for doing various usefull things in Win32
//	Written by Mathew Burrack
//	4.23.2002
//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "plWinRegistryTools.h"
#include "hsWindows.h"


//////////////////////////////////////////////////////////////////////////////
//// Static Utility Functions ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// ISetRegKey //////////////////////////////////////////////////////////////
//	Sets the given registry key to the given string value. If valueName = nil,
//	sets the (default) value

static hsBool	ISetRegKey( const char *keyName, const char *value, const char *valueName = nil )
{
	HKEY	regKey;
	DWORD	result;


	// Create the key (just opens if it already exists)
	if( ::RegCreateKeyEx( HKEY_CLASSES_ROOT, keyName, 0, nil, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
							nil, &regKey, &result ) != ERROR_SUCCESS )
	{
		hsStatusMessageF( "Warning: Registry database open failed for key '%s'.\n", keyName );
		return false;
	}

	// Assign the "default" subkey value
	LONG lResult = ::RegSetValueEx( regKey, valueName, 0, REG_SZ, (const BYTE *)value, ( lstrlen( value ) + 1 ) * sizeof( TCHAR ) );
	
	if( ::RegCloseKey( regKey ) == ERROR_SUCCESS && lResult == ERROR_SUCCESS )
		return true;

	hsStatusMessageF( "Warning: Registry database update failed for key '%s'.\n", keyName );
	return false;
}

//////////////////////////////////////////////////////////////////////////////
//// Public Utility Functions ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// AssociateFileType ///////////////////////////////////////////////////////
//	Associates a given file type in the Win32 registry with the given 
//	application. Also assigns a default icon if iconIndex != -1
//
//	To do this, we create a set of keys in the registry under CLASSES_ROOT that
//	looks like this:
//		fileTypeID	(value = fileTypeName)
//			|
//			|--- DefaultIcon (value = path,index) [omit this one if you don't 
//			|									   want a default icon]
//			|
//			|--- shell
//				   |
//				   |--- open
//					      |
//						  |--- command (value = command line)
//

hsBool	plWinRegistryTools::AssociateFileType( const char *fileTypeID, const char *fileTypeName, const char *appPath, int iconIndex )
{
	char		keyName[ 512 ], keyValue[ 512 ];


	// Root key
	if( !ISetRegKey( fileTypeID, fileTypeName ) )
		return false;

	// DefaultIcon key, if we want one
	if( iconIndex != -1 )
	{
		sprintf( keyName, "%s\\DefaultIcon", fileTypeID );
		sprintf( keyValue, "%s,%d", appPath, iconIndex );
		if( !ISetRegKey( keyName, keyValue ) )
			return false;
	}

	// shell/open/command key
	sprintf( keyName, "%s\\shell\\open\\command", fileTypeID );
	sprintf( keyValue, "\"%s\" \"%%1\"", appPath );
	if( !ISetRegKey( keyName, keyValue ) )
		return false;

	// Success!
	return true;
}

//// AssociateFileExtension //////////////////////////////////////////////////
//	Assigns a given file extension to a previously registered Win32 file type 
//	(using the above function)
//
//	We do this by creating a key entry under CLASSES_ROOT of the following 
//	structure:
//
//			fileExtension (value = fileTypeID)
//
//	where fileExtension includes the leading . and fileTypeID is the same
//	typeID registered with the above function

hsBool	plWinRegistryTools::AssociateFileExtension( const char *fileExtension, const char *fileTypeID )
{
	return ISetRegKey( fileExtension, fileTypeID );	
}

//// GetCurrentFileExtensionAssociation //////////////////////////////////////
//	Obtains the current fileTypeID associated with the given file extension,
//	or a null string if it isn't yet associated.

hsBool	plWinRegistryTools::GetCurrentFileExtensionAssociation( const char *extension, char *buffer, int bufferLen )
{
	long	dataLen;


	buffer[ 0 ] = 0;
	dataLen = bufferLen;

	LONG retVal = ::RegQueryValue( HKEY_CLASSES_ROOT, extension, buffer, &dataLen );
	if( retVal != ERROR_SUCCESS )
	{
		char msg[ 512 ];
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, 0, retVal, 0, msg, sizeof( msg ), nil );
		hsStatusMessageF( "Error querying registry key '%s' : %s\n", extension, msg );
		return false;
	}

	return true;
}

