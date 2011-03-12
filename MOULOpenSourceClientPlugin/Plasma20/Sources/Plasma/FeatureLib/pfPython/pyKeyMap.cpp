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
//////////////////////////////////////////////////////////////////////
//
// pyKeyMap   - a wrapper class all the key mapping functions
//
//////////////////////////////////////////////////////////////////////

#include "pyKeyMap.h"


#include "../plInputCore/plInputInterfaceMgr.h"
#include "../pnInputCore/plKeyMap.h"

// conversion functions
const char* pyKeyMap::ConvertVKeyToChar( UInt32 vk, UInt32 flags )
{
	char *key = plKeyMap::ConvertVKeyToChar( vk );
	static char shortKey[ 2 ];
	if( key == nil )
	{
		if( isalnum( vk ) )
		{
			shortKey[ 0 ] = (char)vk;
			shortKey[ 1 ] = 0;
			key = shortKey;
		}
		else
			return "(unmapped)";
	}

	static char newKey[ 16 ];
	strcpy( newKey, key );
	if( flags & kShift )
		strcat( newKey, "_S" );
	if( flags & kCtrl )
		strcat( newKey, "_C" );

	return newKey;
}

UInt32 pyKeyMap::ConvertCharToVKey( const char* charVKey )
{
	char	str[ 16 ];
	int		i;

	if( strcmp( charVKey, "(unmapped" ) == 0 )
		return KEY_UNMAPPED;

	strcpy( str, charVKey );

	// Get rid of modififers
	for( i = 0; str[ i ] != 0 && str[ i ] != '_'; i++ );
	str[ i ] = 0;

	// Convert raw key and return
	return plKeyMap::ConvertCharToVKey( str );
}

UInt32 pyKeyMap::ConvertCharToFlags( const char *charVKey )
{
	char	str[ 16 ];
	strcpy( str, charVKey );

	// Find modifiers to set flags with
	UInt32 keyFlags = 0;
	if( strstr( str, "_S" ) || strstr( str, "_s" ) )
		keyFlags |= plKeyCombo::kShift;
	if( strstr( str, "_C" ) || strstr( str, "_c" ) )
		keyFlags |= plKeyCombo::kCtrl;

	return keyFlags;
}


UInt32 pyKeyMap::ConvertCharToControlCode(const char* charCode)
{
	ControlEventCode code = plInputMap::ConvertCharToControlCode(charCode);
	return (UInt32)code;
}

const char* pyKeyMap::ConvertControlCodeToString( UInt32 code )
{
	return plInputMap::ConvertControlCodeToString((ControlEventCode)code);
}


// bind a key to an action
void pyKeyMap::BindKey( const char* keyStr1, const char* keyStr2, const char* act)
{
	
	ControlEventCode code = plKeyMap::ConvertCharToControlCode( act );
	if( code == END_CONTROLS )
	{
		// error.... traceback?
		return;
	}

	if( plInputInterfaceMgr::GetInstance() != nil )
	{
		plKeyCombo key1 = IBindKeyToVKey( keyStr1 );
		if (keyStr2)
		{
			plKeyCombo key2 = IBindKeyToVKey( keyStr2 );
			plInputInterfaceMgr::GetInstance()->BindAction( key1, key2, code );
		}
		else
			plInputInterfaceMgr::GetInstance()->BindAction( key1, code );
	}
}

// bind a key to an action
void pyKeyMap::BindKeyToConsoleCommand( const char* keyStr1, const char* command)
{
	
	if( command && plInputInterfaceMgr::GetInstance() != nil )
	{
		plKeyCombo key1;
		if (keyStr1)
			key1 = IBindKeyToVKey( keyStr1 );
		else
			key1 = IBindKeyToVKey( "(unmapped)" );
		plInputInterfaceMgr::GetInstance()->BindConsoleCmd( key1, command, plKeyMap::kFirstAlways);
	}
}

plKeyCombo pyKeyMap::IBindKeyToVKey( const char *keyStr )
{
	char	str[ 16 ];
	int		i;

	plKeyCombo	combo;


	strcpy( str, keyStr );

	// Find modifiers to set flags with
	combo.fFlags = 0;
	if( strstr( str, "_S" ) || strstr( str, "_s" ) )
		combo.fFlags |= plKeyCombo::kShift;
	if( strstr( str, "_C" ) || strstr( str, "_c" ) )
		combo.fFlags |= plKeyCombo::kCtrl;
	
	// Get rid of modififers
	for( i = 0; str[ i ] != 0 && str[ i ] != '_'; i++ );
	str[ i ] = 0;

	// Convert raw key
	combo.fKey = plKeyMap::ConvertCharToVKey( str );
	if( combo.fKey == KEY_UNMAPPED )
		combo = plKeyCombo::kUnmapped;

	// And return!
	return combo;
}

UInt32 pyKeyMap::GetBindingKey1(UInt32 code)
{
	if( plInputInterfaceMgr::GetInstance() != nil )
	{
		const plKeyBinding* keymap = plInputInterfaceMgr::GetInstance()->FindBinding((ControlEventCode)code);
		if ( keymap )
		{
			plKeyCombo key = keymap->GetKey1();
			return (UInt32)(key.fKey);
		}
	}
	return 0;
}

UInt32 pyKeyMap::GetBindingFlags1(UInt32 code)
{
	if( plInputInterfaceMgr::GetInstance() != nil )
	{
		const plKeyBinding* keymap = plInputInterfaceMgr::GetInstance()->FindBinding((ControlEventCode)code);
		if ( keymap )
		{
			plKeyCombo key = keymap->GetKey1();
			return (UInt32)(key.fFlags);
		}
	}
	return 0;
}

UInt32 pyKeyMap::GetBindingKey2(UInt32 code)
{
	if( plInputInterfaceMgr::GetInstance() != nil )
	{
		const plKeyBinding* keymap = plInputInterfaceMgr::GetInstance()->FindBinding((ControlEventCode)code);
		if ( keymap )
		{
			plKeyCombo key = keymap->GetKey2();
			return (UInt32)(key.fKey);
		}
	}
	return 0;
}

UInt32 pyKeyMap::GetBindingFlags2(UInt32 code)
{
	if( plInputInterfaceMgr::GetInstance() != nil )
	{
		const plKeyBinding* keymap = plInputInterfaceMgr::GetInstance()->FindBinding((ControlEventCode)code);
		if ( keymap )
		{
			plKeyCombo key = keymap->GetKey2();
			return (UInt32)(key.fFlags);
		}
	}
	return 0;
}

UInt32 pyKeyMap::GetBindingKeyConsole(const char* command)
{
	if( plInputInterfaceMgr::GetInstance() != nil )
	{
		const plKeyBinding* keymap = plInputInterfaceMgr::GetInstance()->FindBindingByConsoleCmd(command);
		if ( keymap )
		{
			plKeyCombo key = keymap->GetKey1();
			return (UInt32)(key.fKey);
		}
	}
	return 0;
}

UInt32 pyKeyMap::GetBindingFlagsConsole(const char* command)
{
	if( plInputInterfaceMgr::GetInstance() != nil )
	{
		const plKeyBinding* keymap = plInputInterfaceMgr::GetInstance()->FindBindingByConsoleCmd(command);
		if ( keymap )
		{
			plKeyCombo key = keymap->GetKey1();
			return (UInt32)(key.fFlags);
		}
	}
	return 0;
}

void pyKeyMap::WriteKeyMap()
{
	if( plInputInterfaceMgr::GetInstance() != nil )
		plInputInterfaceMgr::GetInstance()->WriteKeyMap();
}
