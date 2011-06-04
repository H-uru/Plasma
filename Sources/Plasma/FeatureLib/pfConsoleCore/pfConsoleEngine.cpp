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
//	pfConsoleEngine Functions												//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "pfConsoleEngine.h"
#include "pfConsoleCmd.h"
#include "pfConsoleContext.h"

#include "../plFile/plEncryptedStream.h"


const Int32		pfConsoleEngine::fMaxNumParams = 16;
const char		pfConsoleEngine::fTokenSeparators[] = " =\r\n\t,";
const char		pfConsoleEngine::fTokenGrpSeps[] = " =\r\n._\t,";



//// Constructor & Destructor ////////////////////////////////////////////////

pfConsoleEngine::pfConsoleEngine()
{
}

pfConsoleEngine::~pfConsoleEngine()
{
}

//// PrintCmdHelp ////////////////////////////////////////////////////////////

hsBool	pfConsoleEngine::PrintCmdHelp( char *name, void (*PrintFn)( const char * ) )
{
	pfConsoleCmd		*cmd;
	pfConsoleCmdGroup	*group, *subGrp;
	char				*ptr;
	static char			string[ 512 ];
	static char			tempString[ 512 ];
	UInt32				i;

	
	/// Scan for subgroups. This can be an empty loop
	group = pfConsoleCmdGroup::GetBaseGroup();
	ptr = strtok( name, fTokenGrpSeps );
	while( ptr != nil )
	{
		// Take this token and check to see if it's a group
		if( ( subGrp = group->FindSubGroupNoCase( ptr ) ) != nil )
			group = subGrp;
		else
			break;

		ptr = strtok( nil, fTokenGrpSeps );
	}

	if( ptr == nil )
	{
		if( group == nil )
		{
			ISetErrorMsg( "Invalid command syntax" );
			return false;
		}

		// Print help for this group
		if( group == pfConsoleCmdGroup::GetBaseGroup() )
			strcpy( string, "Base commands and groups:" );
		else
			sprintf( string, "Group %s:", group->GetName() );
		PrintFn( string );
		PrintFn( "  Subgroups:" );
		for( subGrp = group->GetFirstSubGroup(); subGrp != nil; subGrp = subGrp->GetNext() )
		{
			sprintf( string, "    %s", subGrp->GetName() );
			PrintFn( string );
		}
		PrintFn( "  Commands:" );
		for( cmd = group->GetFirstCommand(); cmd != nil; cmd = cmd->GetNext() )
		{
			for( ptr = cmd->GetHelp(), i = 0; ptr[ i ] != 0 && ptr[ i ] != '\n'; i++ )
				tempString[ i ] = ptr[ i ];
			tempString[ i ] = 0;

			sprintf( string, "    %s: %s", cmd->GetName(), tempString );
			PrintFn( string );
		}

		return true;
	}

	/// OK, so what we found wasn't a group. Which means we need a command...
	cmd = group->FindCommandNoCase( ptr );
	if( cmd == nil )
	{
		ISetErrorMsg( "Invalid syntax: command not found" );
		return false;
	}

	/// That's it!
	sprintf( string, "\nHelp for the command %s:", cmd->GetName() );
	PrintFn( string );
	sprintf( string, "\\i%s", cmd->GetHelp() );
	PrintFn( string );
	sprintf( string, "\\iUsage: %s", cmd->GetSignature() );
	PrintFn( string );

	return true;
}

//// GetCmdSignature /////////////////////////////////////////////////////////

const char	*pfConsoleEngine::GetCmdSignature( char *name )
{
	pfConsoleCmd		*cmd;
	pfConsoleCmdGroup	*group, *subGrp;
	char				*ptr;
	static char			string[ 512 ];

	
	/// Scan for subgroups. This can be an empty loop
	group = pfConsoleCmdGroup::GetBaseGroup();
	ptr = strtok( name, fTokenGrpSeps );
	while( ptr != nil )
	{
		// Take this token and check to see if it's a group
		if( ( subGrp = group->FindSubGroupNoCase( ptr ) ) != nil )
			group = subGrp;
		else
			break;

		ptr = strtok( nil, fTokenGrpSeps );
	}

	if( ptr == nil )
	{
		ISetErrorMsg( "Invalid command syntax" );
		return nil;
	}

	/// OK, so what we found wasn't a group. Which means we need a command...
	cmd = group->FindCommandNoCase( ptr );
	if( cmd == nil )
	{
		ISetErrorMsg( "Invalid syntax: command not found" );
		return nil;
	}

	/// That's it!
	return (char *)cmd->GetSignature();
}

//// Dummy Local Function ////////////////////////////////////////////////////

void	DummyPrintFn( const char *line )
{
}

//// ExecuteFile /////////////////////////////////////////////////////////////

hsBool	pfConsoleEngine::ExecuteFile( const char *fileName )
{
	wchar* wFilename = hsStringToWString(fileName);
	hsBool ret = ExecuteFile(wFilename);
	delete [] wFilename;
	return ret;
}

hsBool	pfConsoleEngine::ExecuteFile( const wchar *fileName )
{
	char			string[ 512 ];
	int				line;

	hsStream* stream = plEncryptedStream::OpenEncryptedFile(fileName);

	if( !stream )
	{
		ISetErrorMsg( "Cannot open given file" );
//		return false;
		/// THIS IS BAD: because of the asserts we throw after this if we return false, a missing
		/// file will throw an assert. This is all well and good except for the age-specific .fni files,
		/// which aren't required to be there and rely on this functionality to test whether the file is
		/// present. Once age-specific .fni's are gone, reinstate the return false here. -mcn
		return true;
	}

	for( line = 1; stream->ReadLn( string, sizeof( string ) ); line++ )
	{
		strncpy( fLastErrorLine, string, sizeof( fLastErrorLine ) );

		if( !RunCommand( string, DummyPrintFn ) )
		{
			sprintf( string, "Error in console file %s, command line %d: %s", fileName, line, fErrorMsg );
			ISetErrorMsg( string );
			stream->Close();
			delete stream;
			return false;
		}
	}
	stream->Close();
	delete stream;
	fLastErrorLine[ 0 ] = 0;

	return true;
}

//// RunCommand //////////////////////////////////////////////////////////////
//	Updated 2.14.2001 mcn to support spaces, _ or . as group separators. This
//	requires tokenizing the entire line and searching the tokens one by one,
//	parsing them first as groups, then commands and then params.

hsBool	pfConsoleEngine::RunCommand( char *line, void (*PrintFn)( const char * ) )
{
	pfConsoleCmd		*cmd;
	pfConsoleCmdGroup	*group, *subGrp;
	Int32				numParams, i, numQuotedParams = 0;
	pfConsoleCmdParam	paramArray[ fMaxNumParams + 1 ];
	char				*ptr;
	hsBool				valid = true;


	hsAssert( line != nil, "Bad parameter to RunCommand()" );

	/// Loop #1: Scan for subgroups. This can be an empty loop
	group = pfConsoleCmdGroup::GetBaseGroup();
	ptr = strtok( line, fTokenGrpSeps );
	while( ptr != nil )
	{
		// Take this token and check to see if it's a group
		if( ( subGrp = group->FindSubGroupNoCase( ptr ) ) != nil )
			group = subGrp;
		else
			break;

		ptr = strtok( nil, fTokenGrpSeps );
	}

	if( ptr == nil )
	{
		ISetErrorMsg( "Invalid command syntax" );
		return false;
	}

	/// OK, so what we found wasn't a group. Which means we need a command next
	cmd = group->FindCommandNoCase( ptr );
	if( cmd == nil )
	{
		ISetErrorMsg( "Invalid syntax: command not found" );
		return false;
	}

	/// We have the group, we have the command from that group. So continue
	/// tokenizing (with the new separators now, mind you) and turn them into
	/// params

	for( numParams = numQuotedParams = 0; numParams < fMaxNumParams 
						&& ( ptr = strtok( nil, fTokenSeparators ) ) != nil
						&& valid; numParams++ )
	{
		if( ptr[ 0 ] == '\'' || ptr[ 0 ] == '"' )
		{
			// String parameter--keep getting tokens until we hit the other end

			// Note: since params take pointers to strings, we have to have unique temp strings
			// for each quoted param we parse. So we have a static array here to a) do so, b)
			// avoid having to delete them afterwards, and thus c) reduce overhead.
			static char			tempStrings[ fMaxNumParams ][ 512 ];

			char	*tempStr = tempStrings[ numQuotedParams++ ], toSearch[ 2 ] = "'";
			
			toSearch[ 0 ] = ptr[ 0 ];

			if( strlen( ptr ) >= sizeof( tempStrings[ 0 ] ) )	// They're all the same, after all...
			{
				ISetErrorMsg( "Invalid syntax: quoted parameter too long" );
				return false;
			}

			if( strlen( ptr ) > 1 && ptr[ strlen( ptr ) - 1 ] == toSearch[ 0 ] )
			{
				// Single word string
				strcpy( tempStr, ptr + 1 );
				tempStr[ strlen( tempStr ) - 1 ] = 0;
			}
			else
			{
				// Multiple word string
				sprintf( tempStr, "%s ", ptr + 1 ); // Not perfect, but close
				ptr = strtok( nil, toSearch );
				if( ptr == nil )
				{
					ISetErrorMsg( "Invalid syntax: unterminated quoted parameter" );
					return false;
				}

				if( strlen( ptr ) + strlen( tempStr ) >= sizeof( tempStrings[ 0 ] ) )	// They're all the same, after all...
				{
					ISetErrorMsg( "Invalid syntax: quoted parameter too long" );
					return false;
				}
				strcat( tempStr, ptr );
			}

			valid = IConvertToParam( cmd->GetSigEntry( (UInt8)numParams ), tempStr, &paramArray[ numParams ] );
		}
		else
		{
			// Normal parameter

			// Special case for context variables--if we're specifying one, we want to just grab
			// the value of it and return that instead
			valid = false;
			if( ptr[ 0 ] == '$' )
			{
				pfConsoleContext	&context = pfConsoleContext::GetRootContext();

				// Potential variable, see if we can find it
				Int32 idx = context.FindVar( ptr + 1 );
				if( idx == -1 )
				{
					ISetErrorMsg( "Invalid console variable name" );
				}
				else
				{
					// Just copy. Note that this will copy string pointers, but so long as the variable in
					// question doesn't change, we'll be OK...
					paramArray[ numParams ] = context.GetVarValue( idx );
					valid = true;
				}
			}

			if( !valid )
				valid = IConvertToParam( cmd->GetSigEntry( (UInt8)numParams ), ptr, &paramArray[ numParams ] );
		}
	}
	for( i = numParams; i < fMaxNumParams + 1; i++ )
		paramArray[ i ].SetNone();

	if( !valid || ( cmd->GetSigEntry( (UInt8)numParams ) != pfConsoleCmd::kAny && 
					cmd->GetSigEntry( (UInt8)numParams ) != pfConsoleCmd::kNone ) )
	{
		// Print help string and return
		static char		string[ 512 ];

		ISetErrorMsg( "" );	// Printed on next line
		PrintFn( "Invalid parameters to command" );
		sprintf( string, "Usage: %s", cmd->GetSignature() );
		PrintFn( string );
		return false;
	}

	/// Execute it and return
	cmd->Execute( numParams, paramArray, PrintFn );
	return true;
}

//// IConvertToParam /////////////////////////////////////////////////////////
//	Converts a null-terminated string representing a parameter to a 
//	pfConsoleCmdParam argument.

hsBool	pfConsoleEngine::IConvertToParam( UInt8 type, char *string, pfConsoleCmdParam *param )
{
	char	*c, expChars[] = "dDeE+-.";
	hsBool	hasDecimal = false, hasLetters = false;


	if( type == pfConsoleCmd::kNone )
		return false;

	for( c = string; *c != 0; c++ )
	{
		if( !isdigit( *c ) )
		{
			if( c == string && ( *c == '-' || *c == '+' ) )
			{
				// Do nothing--perfectly legal to have these at the beginning of an int
			}
			else if( strchr( expChars, *c ) != nil )
				hasDecimal = true;
			else
				hasLetters = true;
		}
	}

	if( type == pfConsoleCmd::kAny )
	{
		/// Want "any"
		param->SetAny( string );
	}
	else if( type == pfConsoleCmd::kString )
	{
		/// Want just a string
		param->SetString( string );
	}
	else if( type == pfConsoleCmd::kFloat )
	{
		if( hasLetters )
			return false;

		param->SetFloat( (float)atof( string ) );
	}
	else if( type == pfConsoleCmd::kInt )
	{
		if( hasLetters || hasDecimal )
			return false;

		param->SetInt( atoi( string ) );
	}
	else if( type == pfConsoleCmd::kBool )
	{
		if( stricmp( string, "true" ) == 0 || stricmp( string, "t" ) == 0 )
			param->SetBool( true );
		else if( stricmp( string, "false" ) == 0 || stricmp( string, "f" ) == 0 )
			param->SetBool( false );
		else if( atoi( string ) == 0 )
			param->SetBool( false );
		else
			param->SetBool( true );
	}

	return true;
}

//// FindPartialCmd //////////////////////////////////////////////////////////
//	Given a string which is the beginning of a console command, modifies the
//	string to represent the best match of command (or group) for that string.
//	WARNING: modifies the string passed to it.

hsBool	pfConsoleEngine::FindPartialCmd( char *line, hsBool findAgain, hsBool preserveParams )
{
	pfConsoleCmd		*cmd = nil;
	pfConsoleCmdGroup	*group, *subGrp;
	hsBool				foundMore = false;

	static char					*ptr = nil, *insertLoc = nil;
	static pfConsoleCmd			*lastCmd = nil;
	static pfConsoleCmdGroup	*lastGroup = nil, *lastParentGroup = nil;
	static char					newStr[ 256 ];


	/// Repeat search
	if( !findAgain )
	{
		lastCmd = nil;
		lastGroup = nil;
	}

	/// New search
	if( strlen( line ) > sizeof( newStr ) )
		return false;

	newStr[ 0 ] = 0;
	insertLoc = newStr;

	/// Loop #1: Scan for subgroups. This can be an empty loop
	lastParentGroup = group = pfConsoleCmdGroup::GetBaseGroup();
	ptr = strtok( line, fTokenGrpSeps );
	while( ptr != nil )
	{
		// Take this token and check to see if it's a group
		if( ( subGrp = group->FindSubGroupNoCase( ptr, 0/*pfConsoleCmdGroup::kFindPartial*/
			, /*lastGroup*/nil ) ) != nil )
		{
			lastParentGroup = group;
			group = subGrp;
			strcat( newStr, group->GetName() );
			insertLoc += strlen( group->GetName() );
		}
		else
			break;

		ptr = strtok( nil, fTokenGrpSeps );
		strcat( newStr, "." );
		insertLoc++;
	}

	if( ptr != nil )
	{
		// Still got at least one token left. Try to match to either
		// a partial group or a partial command
		if( ( subGrp = group->FindSubGroupNoCase( ptr, pfConsoleCmdGroup::kFindPartial, lastGroup ) ) != nil )
		{
			lastParentGroup = group;
			lastGroup = group = subGrp;
			strcat( newStr, group->GetName() );
			strcat( newStr, "." );
		}
		else 
		{
			cmd = group->FindCommandNoCase( ptr, pfConsoleCmdGroup::kFindPartial, lastCmd );
			if( cmd == nil )
				return false;

			strcat( newStr, cmd->GetName() );
			strcat( newStr, " " );
			lastCmd = cmd;
		}
	}

	if( preserveParams )
	{
		/// Preserve the rest of the string after the matched command
		ptr = strtok( nil, "\0" );
		if( ptr != nil )
			strcat( newStr, ptr );
	}

	// Copy back!
	strcpy( line, newStr );

	return true;
}

//// FindNestedPartialCmd ////////////////////////////////////////////////////
//	Same as FindPartialCmd, only starts from the global group and searches
//	everything. The string passed should only be a partial command sans
//	groups. numToSkip specifies how many matches to skip before returning one
//	(so if numToSkip = 1, then this will return the second match found).

hsBool	pfConsoleEngine::FindNestedPartialCmd( char *line, UInt32 numToSkip, hsBool preserveParams )
{
	pfConsoleCmd		*cmd;

	
	/// Somewhat easier than FindPartialCmd...
	cmd = pfConsoleCmdGroup::GetBaseGroup()->FindNestedPartialCommand( line, &numToSkip );
	if( cmd == nil )
		return false;

	/// Recurse back up and get the group hierarchy
	line[ 0 ] = 0;
	IBuildCmdNameRecurse( cmd->GetParent(), line );
	strcat( line, cmd->GetName() );
	strcat( line, " " );

	if( preserveParams )
	{
		/// Preserve the rest of the string after the matched command
	}

	return true;
}

//// IBuildCmdNameRecurse ////////////////////////////////////////////////////

void	pfConsoleEngine::IBuildCmdNameRecurse( pfConsoleCmdGroup *group, char *string )
{
	if( group == nil || group == pfConsoleCmdGroup::GetBaseGroup() )
		return;


	IBuildCmdNameRecurse( group->GetParent(), string );

	strcat( string, group->GetName() );
	strcat( string, "." );
}
