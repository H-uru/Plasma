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
//																			//
//	pfConsoleContext														//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "pfConsoleContext.h"


//// Static Root Context /////////////////////////////////////////////////////

pfConsoleContext	pfConsoleContext::fRootContext( "global" );

pfConsoleContext	&pfConsoleContext::GetRootContext( void )
{
	return fRootContext;
}

//// Constructor/Destructor //////////////////////////////////////////////////

pfConsoleContext::pfConsoleContext( const char *name )
{
	fName = ( name != nil ) ? hsStrcpy( name ) : nil;
	fAddWhenNotFound = true;
}

pfConsoleContext::~pfConsoleContext()
{
	Clear();
	delete [] fName;
}

//// Clear ///////////////////////////////////////////////////////////////////

void	pfConsoleContext::Clear( void )
{
	Int32		idx;


	for( idx = fVarValues.GetCount() - 1; idx >= 0; idx-- )
		RemoveVar( idx );
}

//// Getters /////////////////////////////////////////////////////////////////

UInt32	pfConsoleContext::GetNumVars( void ) const
{
	hsAssert( fVarValues.GetCount() == fVarNames.GetCount(), "Mismatch in console var context arrays" );
	return fVarValues.GetCount();
}

const char			*pfConsoleContext::GetVarName( UInt32 idx ) const
{
	hsAssert( fVarValues.GetCount() == fVarNames.GetCount(), "Mismatch in console var context arrays" );

	if( idx >= fVarNames.GetCount() )
	{
		hsAssert( false, "GetVarName() index out of range for console context" );
		return nil;
	}

	return fVarNames[ idx ];
}

pfConsoleCmdParam	&pfConsoleContext::GetVarValue( UInt32 idx ) const
{
	hsAssert( fVarValues.GetCount() == fVarNames.GetCount(), "Mismatch in console var context arrays" );
	hsAssert( idx < fVarValues.GetCount(), "GetVarValue() index out of range for console context" );

	return fVarValues[ idx ];
}


//// FindVar /////////////////////////////////////////////////////////////////

Int32	pfConsoleContext::FindVar( const char *name ) const
{
	UInt32		idx;


	hsAssert( fVarValues.GetCount() == fVarNames.GetCount(), "Mismatch in console var context arrays" );

	for( idx = 0; idx < fVarNames.GetCount(); idx++ )
	{
		if( stricmp( name, fVarNames[ idx ] ) == 0 )
		{
			return (Int32)idx;
		}
	}

	return -1;
}

//// RemoveVar ///////////////////////////////////////////////////////////////

void	pfConsoleContext::RemoveVar( UInt32 idx )
{
	hsAssert( fVarValues.GetCount() == fVarNames.GetCount(), "Mismatch in console var context arrays" );

	if( idx >= fVarValues.GetCount() )
	{
		hsAssert( false, "RemoveVar() index out of range for console context" );
		return;
	}

	delete [] fVarNames[ idx ];
	if( fVarValues[ idx ].GetType() == pfConsoleCmdParam::kString )
		// Necessary because the params won't delete the data themselves
		delete [] ( (char *)fVarValues[ idx ] );

	fVarNames.Remove( idx );
	fVarValues.Remove( idx );
}

//// AddVar Variants /////////////////////////////////////////////////////////

void	pfConsoleContext::IAddVar( const char *name, const pfConsoleCmdParam &value )
{
	fVarNames.Append( hsStrcpy( name ) );
	fVarValues.Append( value );
	
	// Remember, params won't know any better, since by default they don't own a copy of their string
	UInt32 idx = fVarValues.GetCount() - 1;
	if( fVarValues[ idx ].GetType() == pfConsoleCmdParam::kString )
		fVarValues[ idx ].SetString( hsStrcpy( fVarValues[ idx ] ) );
}

void	pfConsoleContext::AddVar( const char *name, const pfConsoleCmdParam &value )
{
	Int32 idx = FindVar( name );
	if( idx != -1 )
	{
		hsAssert( false, "AddVar() failed because variable already in console context" );
		return;
	}

	IAddVar( name, value );
}

void	pfConsoleContext::AddVar( const char *name, int value )
{
	pfConsoleCmdParam	param;
	param.SetInt( value );
	AddVar( name, param );
}

void	pfConsoleContext::AddVar( const char *name, float value )
{
	pfConsoleCmdParam	param;
	param.SetFloat( value );
	AddVar( name, param );
}

void	pfConsoleContext::AddVar( const char *name, const char *value )
{
	pfConsoleCmdParam	param;
	param.SetString( (char *)value );	// It's ok, we'll be copying it soon 'nuf
	AddVar( name, param );
}

void	pfConsoleContext::AddVar( const char *name, char value )
{
	pfConsoleCmdParam	param;
	param.SetChar( value );
	AddVar( name, param );
}

void	pfConsoleContext::AddVar( const char *name, bool value )
{
	pfConsoleCmdParam	param;
	param.SetBool( value );
	AddVar( name, param );
}

//// SetVar Variants /////////////////////////////////////////////////////////

hsBool	pfConsoleContext::SetVar( UInt32 idx, const pfConsoleCmdParam &value )
{
	hsAssert( fVarValues.GetCount() == fVarNames.GetCount(), "Mismatch in console var context arrays" );
	if( idx >= fVarValues.GetCount() )
	{
		hsAssert( false, "SetVar() index out of range for console context" );
		return false;
	}

	if( fVarValues[ idx ].GetType() == pfConsoleCmdParam::kString )
	{
		// Remember, params won't know any better, since by default they don't own a copy of their string
		delete [] ( (char *)fVarValues[ idx ] );
	}

	fVarValues[ idx ] = value;
	if( fVarValues[ idx ].GetType() == pfConsoleCmdParam::kString )
		fVarValues[ idx ].SetString( hsStrcpy( fVarValues[ idx ] ) );

	return true;
}

hsBool	pfConsoleContext::SetVar( const char *name, const pfConsoleCmdParam &value )
{
	Int32 idx = FindVar( name );
	if( idx == -1 )
	{
		if( fAddWhenNotFound )
		{
			// Don't panic, just add
			IAddVar( name, value );
			return true;
		}
		return false;
	}

	return SetVar( idx, value );
}

hsBool	pfConsoleContext::SetVar( const char *name, int value )
{
	pfConsoleCmdParam	param;
	param.SetInt( value );
	return SetVar( name, param );
}

hsBool	pfConsoleContext::SetVar( const char *name, float value )
{
	pfConsoleCmdParam	param;
	param.SetFloat( value );
	return SetVar( name, param );
}

hsBool	pfConsoleContext::SetVar( const char *name, const char *value )
{
	pfConsoleCmdParam	param;
	param.SetString( (char *)value );	// Don't worry, we'll be copying it soon 'nuf
	return SetVar( name, param );
}

hsBool	pfConsoleContext::SetVar( const char *name, char value )
{
	pfConsoleCmdParam	param;
	param.SetChar( value );
	return SetVar( name, param );
}

hsBool	pfConsoleContext::SetVar( const char *name, bool value )
{
	pfConsoleCmdParam	param;
	param.SetBool( value );
	return SetVar( name, param );
}

