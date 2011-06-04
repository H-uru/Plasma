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
//	pfConsoleCmd Functions													//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "pfConsoleCmd.h"
#include "hsUtils.h"


//////////////////////////////////////////////////////////////////////////////
//// pfConsoleCmdGroup Stuff /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

pfConsoleCmdGroup	*pfConsoleCmdGroup::fBaseCmdGroup = nil;
UInt32				pfConsoleCmdGroup::fBaseCmdGroupRef = 0;


//// Constructor & Destructor ////////////////////////////////////////////////

pfConsoleCmdGroup::pfConsoleCmdGroup( char *name, char *parent )
{
	Dummy();
	DummyJunior();
	DummyNet();
	DummyAvatar();
	DummyCCR();

	fNext = nil;
	fPrevPtr = nil;
	fCommands = nil;
	fSubGroups = nil;
	
	if( name == nil )
	{
		/// Create base
		hsStrncpy( fName, "base", sizeof( fName ) );
		fParentGroup = nil;
	}
	else
	{
		pfConsoleCmdGroup	*group = GetBaseGroup();

		if( parent != nil && parent[ 0 ] != 0 )
		{
			group = group->FindSubGroupRecurse( parent );
			hsAssert( group != nil, "Trying to register group under nonexistant group!" );
		}

		hsStrncpy( fName, name, sizeof( fName ) );
		group->AddSubGroup( this );
		fParentGroup = group;
	}

}

pfConsoleCmdGroup::~pfConsoleCmdGroup()
{
	if( this != fBaseCmdGroup )
	{
		Unlink();

		DecBaseCmdGroupRef();
	}
}

//// GetBaseGroup ////////////////////////////////////////////////////////////

pfConsoleCmdGroup	*pfConsoleCmdGroup::GetBaseGroup( void )
{
	if( fBaseCmdGroup == nil )
	{
		/// Initialize base group
		fBaseCmdGroup = TRACKED_NEW pfConsoleCmdGroup( nil, nil );
	}

	return fBaseCmdGroup;
}

//// DecBaseCmdGroupRef //////////////////////////////////////////////////////

void	pfConsoleCmdGroup::DecBaseCmdGroupRef( void )
{
	fBaseCmdGroupRef--;
	if( fBaseCmdGroupRef == 0 )
	{
		delete fBaseCmdGroup;
		fBaseCmdGroup = nil;
	}
}

//// Add Functions ///////////////////////////////////////////////////////////

void	pfConsoleCmdGroup::AddCommand( pfConsoleCmd *cmd )
{
	cmd->Link( &fCommands );
	fBaseCmdGroupRef++;
}

void	pfConsoleCmdGroup::AddSubGroup( pfConsoleCmdGroup *group )
{
	group->Link( &fSubGroups );
	fBaseCmdGroupRef++;
}

//// FindCommand /////////////////////////////////////////////////////////////
//	No longer recursive.

pfConsoleCmd	*pfConsoleCmdGroup::FindCommand( char *name )
{
	pfConsoleCmd	*cmd;


	hsAssert( name != nil, "nil name passed to FindCommand()" );

	/// Only search locally
	for( cmd = fCommands; cmd != nil; cmd = cmd->GetNext() )
	{
		if( strcmp( cmd->GetName(), name ) == 0 )
			return cmd;
	}

	return nil;
}

//// FindNestedPartialCommand ////////////////////////////////////////////////
//	Okay. YAFF. This one searches through the group and its children looking
//	for a partial command based on the string given. The counter determines
//	how many matches it skips before returning a match. (That way you can
//	cycle through matches by sending 1 + the last counter every time).

pfConsoleCmd	*pfConsoleCmdGroup::FindNestedPartialCommand( char *name, UInt32 *counter )
{
	pfConsoleCmd		*cmd;
	pfConsoleCmdGroup	*group;


	hsAssert( name != nil, "nil name passed to FindNestedPartialCommand()" );
	hsAssert( counter != nil, "nil counter passed to FindNestedPartialCommand()" );

	// Try us
	for( cmd = fCommands; cmd != nil; cmd = cmd->GetNext() )
	{
		if( _strnicmp( cmd->GetName(), name, strlen( name ) ) == 0 )
		{
			if( *counter == 0 )
				return cmd;

			(*counter)--;
		}
	}

	// Try children
	for( group = fSubGroups; group != nil; group = group->GetNext() )
	{
		cmd = group->FindNestedPartialCommand( name, counter );
		if( cmd != nil )
			return cmd;
	}

	return nil;
}

//// FindSubGroup ////////////////////////////////////////////////////////////

pfConsoleCmdGroup	*pfConsoleCmdGroup::FindSubGroup( char *name )
{
	pfConsoleCmdGroup	*group;


	hsAssert( name != nil, "nil name passed to FindSubGroup()" );

	/// Only search locally
	for( group = fSubGroups; group != nil; group = group->GetNext() )
	{
		if( strcmp( group->GetName(), name ) == 0 )
			return group;
	}

	return nil;
}

//// FindSubGroupRecurse /////////////////////////////////////////////////////
//	Resurces through a string, finding the final subgroup that the string
//	represents. Parses with spaces, _ or . as the separators. Copies string.

pfConsoleCmdGroup	*pfConsoleCmdGroup::FindSubGroupRecurse( const char *name )
{
	char				*ptr, *string;
	pfConsoleCmdGroup	*group;
	static char			seps[] = " ._";


	string = TRACKED_NEW char[ strlen( name ) + 1 ];
	hsAssert( string != nil, "Cannot allocate string in FindSubGroupRecurse()" );
	strcpy( string, name );

	/// Scan for subgroups
	group = pfConsoleCmdGroup::GetBaseGroup();
	ptr = strtok( string, seps );
	while( ptr != nil )
	{
		// Take this token and check to see if it's a group
		group = group->FindSubGroup( ptr );
		hsAssert( group != nil, "Invalid group name to FindSubGroupRecurse()" );

		ptr = strtok( nil, seps );
	}

	delete [] string;
	return group;
}

//// FindCommandNoCase ///////////////////////////////////////////////////////
//	Case-insensitive version of FindCommand.

pfConsoleCmd	*pfConsoleCmdGroup::FindCommandNoCase( char *name, UInt8 flags, pfConsoleCmd *start )
{
	pfConsoleCmd	*cmd;


	hsAssert( name != nil, "nil name passed to FindCommandNoCase()" );

	/// Only search locally
	if( start == nil )
		start = fCommands;
	else
		start = start->GetNext();

	if( flags & kFindPartial )
	{
		for( cmd = start; cmd != nil; cmd = cmd->GetNext() )
		{
			if( _strnicmp( cmd->GetName(), name, strlen( name ) ) == 0 )
				return cmd;
		}
	}
	else
	{
		for( cmd = start; cmd != nil; cmd = cmd->GetNext() )
		{
			if( stricmp( cmd->GetName(), name ) == 0 )
				return cmd;
		}
	}

	return nil;
}

//// FindSubGroupNoCase //////////////////////////////////////////////////////

pfConsoleCmdGroup	*pfConsoleCmdGroup::FindSubGroupNoCase( char *name, UInt8 flags, pfConsoleCmdGroup *start )
{
	pfConsoleCmdGroup	*group;


	hsAssert( name != nil, "nil name passed to FindSubGroupNoCase()" );

	/// Only search locally
	if( start == nil )
		start = fSubGroups;
	else
		start = start->GetNext();

	if( flags & kFindPartial )
	{
		for( group = start; group != nil; group = group->GetNext() )
		{
			if( _strnicmp( group->GetName(), name, strlen( name ) ) == 0 )
				return group;
		}
	}
	else
	{
		for( group = start; group != nil; group = group->GetNext() )
		{
			if( stricmp( group->GetName(), name ) == 0 )
				return group;
		}
	}

	return nil;
}

//// Link & Unlink ///////////////////////////////////////////////////////////

void	pfConsoleCmdGroup::Link( pfConsoleCmdGroup **prevPtr )
{
	hsAssert( fNext == nil && fPrevPtr == nil, "Trying to link console group that's already linked!" );

	fNext = *prevPtr;
	if( *prevPtr )
		(*prevPtr)->fPrevPtr = &fNext;
	fPrevPtr = prevPtr;
	*fPrevPtr = this;
}

void	pfConsoleCmdGroup::Unlink( void )
{
	hsAssert( fNext != nil || fPrevPtr != nil, "Trying to unlink console group that isn't linked!" );

	if( fNext )
		fNext->fPrevPtr = fPrevPtr;
	*fPrevPtr = fNext;
}


int  pfConsoleCmdGroup::IterateCommands(pfConsoleCmdIterator* t, int depth)
{
	pfConsoleCmd *cmd;

	cmd = this->GetFirstCommand();
	while(cmd)
	{
		t->ProcessCmd(cmd,depth);
		cmd = cmd->GetNext();
	}

	pfConsoleCmdGroup *grp;

	grp = this->GetFirstSubGroup();
	while(grp)
	{
		if(t->ProcessGroup(grp, depth))
			grp->IterateCommands(t, depth+1);
		grp = grp->GetNext();
	}
	return 0;
}



//////////////////////////////////////////////////////////////////////////////
//// pfConsoleCmd Functions //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

char	pfConsoleCmd::fSigTypes[ kNumTypes ][ 8 ] = { "int", "float", "bool", "string", "char", "void", "..." };

pfConsoleCmd::pfConsoleCmd( char *group, char *name, char *paramList, char *help, 
							pfConsoleCmdPtr func, hsBool localOnly )
{
	fNext = nil;
	fPrevPtr = nil;

	fFunction = func;
	fLocalOnly = localOnly;
	
	hsStrncpy( fName, name, sizeof( fName ) );
	fHelpString = help;

	ICreateSignature( paramList );
	Register( group, name );
}

pfConsoleCmd::~pfConsoleCmd()
{
	int		i;


	for( i = 0; i < fSigLabels.GetCount(); i++ )
	{
		if( fSigLabels[ i ] != nil )
			delete [] fSigLabels[ i ];
	}
	Unregister();
	
	fSignature.Reset();
	fSigLabels.Reset();
}

//// ICreateSignature ////////////////////////////////////////////////////////
//	Creates the signature and sig labels based on the given string.

void	pfConsoleCmd::ICreateSignature( char *paramList )
{
	static char	seps[] = " :-";

	char	params[ 256 ];
	char	*ptr, *nextPtr, *tok, *tok2;
	int		i;


	/// Simple check
	if( paramList == nil )
	{
		fSignature.Push( kAny );
		fSigLabels.Push( (char *)nil );
		return;
	}

	/// So we can do stuff to it
	hsAssert( strlen( paramList ) < sizeof( params ), "Make the (#*$& params string larger!" );
	hsStrcpy( params, paramList );

	fSignature.Empty();
	fSigLabels.Empty();

	/// Loop through all the types given in the list
	ptr = params;
	do
	{
		/// Find break
		nextPtr = strchr( ptr, ',' );
		if( nextPtr != nil )
		{
			*nextPtr = 0;
			nextPtr++;
		}

		/// Do this param
		tok = strtok( ptr, seps );
		if( tok == nil && ptr == params )
			break;

		hsAssert( tok != nil, "Bad parameter list for console command!" );
		tok2 = strtok( nil, seps );

		if( tok2 != nil )
		{
			// Type and label: assume label second
			fSigLabels.Push( hsStrcpy( tok2 ) );			
		}
		else
			fSigLabels.Push( (char *)nil );

		// Find type
		for( i = 0; i < kNumTypes; i++ )
		{
			if( strcmp( fSigTypes[ i ], tok ) == 0 )
			{
				fSignature.Push( (UInt8)i );
				break;
			}
		}

		hsAssert( i < kNumTypes, "Bad parameter type in console command parameter list!" );

	} while( ( ptr = nextPtr ) != nil );
}

//// Register ////////////////////////////////////////////////////////////////
//	Finds the group this command should be in and registers it with that
//	group.

void	pfConsoleCmd::Register( char *group, char *name )
{
	pfConsoleCmdGroup	*g;


	if( group == nil || group[ 0 ] == 0 )
	{
		g = pfConsoleCmdGroup::GetBaseGroup();
		g->AddCommand( this );
	}
	else
	{
		g = pfConsoleCmdGroup::FindSubGroupRecurse( group );
		hsAssert( g != nil, "Trying to register command under nonexistant group!" );
		g->AddCommand( this );
	}

	fParentGroup = g;
}

//// Unregister //////////////////////////////////////////////////////////////

void	pfConsoleCmd::Unregister( void )
{
	Unlink();
	pfConsoleCmdGroup::DecBaseCmdGroupRef();
}

//// Execute /////////////////////////////////////////////////////////////////
//	Run da thing!

void	pfConsoleCmd::Execute( Int32 numParams, pfConsoleCmdParam *params, void (*PrintFn)( const char * ) )
{
	fFunction( numParams, params, PrintFn );
}

//// Link & Unlink ///////////////////////////////////////////////////////////

void	pfConsoleCmd::Link( pfConsoleCmd **prevPtr )
{
	hsAssert( fNext == nil && fPrevPtr == nil, "Trying to link console command that's already linked!" );

	fNext = *prevPtr;
	if( *prevPtr )
		(*prevPtr)->fPrevPtr = &fNext;
	fPrevPtr = prevPtr;
	*fPrevPtr = this;
}

void	pfConsoleCmd::Unlink( void )
{
	hsAssert( fNext != nil || fPrevPtr != nil, "Trying to unlink console command that isn't linked!" );

	if( fNext )
		fNext->fPrevPtr = fPrevPtr;
	*fPrevPtr = fNext;
}

//// GetSigEntry /////////////////////////////////////////////////////////////

UInt8	pfConsoleCmd::GetSigEntry( UInt8 i )
{
	if( fSignature.GetCount() == 0 )
		return kNone;

	if( i < fSignature.GetCount() )
	{
		if( fSignature[ i ] == kEtc )
			return kAny;

		return fSignature[ i ];
	}

	if( fSignature[ fSignature.GetCount() - 1 ] == kEtc )
		return kAny;

	return kNone;
}

//// GetSignature ////////////////////////////////////////////////////////////
//	Gets the signature of the command as a string. Format is:
//		name [ type param [, type param ... ] ]
//	WARNING: uses a static buffer, so don't rely on the contents if you call
//	it more than once! (You shouldn't need to, though)

const char	*pfConsoleCmd::GetSignature( void )
{
	static	char	string[ 256 ];
	
	int		i;
	char	pStr[ 128 ];


	strcpy( string, fName );
	for( i = 0; i < fSignature.GetCount(); i++ )
	{
		if( fSigLabels[ i ] == nil )
			sprintf( pStr, "%s", fSigTypes[ fSignature[ i ] ] );
		else
			sprintf( pStr, "%s %s", fSigTypes[ fSignature[ i ] ], fSigLabels[ i ] );

		hsAssert( strlen( string ) + strlen( pStr ) + 2 < sizeof( string ), "Not enough room for signature string" );
		strcat( string, ( i > 0 ) ? ", " : " " );

		strcat( string, pStr );
	}

	return string;
}


//////////////////////////////////////////////////////////////////////////////
//// pfConsoleCmdParam Functions /////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// Conversion Functions ////////////////////////////////////////////////////

const int &	pfConsoleCmdParam::IToInt( void ) const
{
	hsAssert( fType == kInt || fType == kAny, "Trying to use a non-int parameter as an int!" );

	static int	i;
	if( fType == kAny )
	{
		hsAssert( fValue.s != nil, "Weird parameter during conversion" );
		i = atoi( fValue.s );
		return i;
	}
	
	return fValue.i;
}

const float &	pfConsoleCmdParam::IToFloat( void ) const
{
	hsAssert( fType == kFloat || fType == kAny, "Trying to use a non-float parameter as a float!" );

	static float f;
	if( fType == kAny )
	{
		hsAssert( fValue.s != nil, "Weird parameter during conversion" );
		f = (float)atof( fValue.s );
		return f;
	}
	
	return fValue.f;
}

const bool &	pfConsoleCmdParam::IToBool( void ) const
{
	hsAssert( fType == kBool || fType == kAny, "Trying to use a non-bool parameter as a bool!" );

	static bool	b;
	if( fType == kAny )
	{
		hsAssert( fValue.s != nil, "Weird parameter during conversion" );
		if( atoi( fValue.s ) > 0 || stricmp( fValue.s, "true" ) == 0 )
			b = true;
		else
			b = false;

		return b;
	}
	
	return fValue.b;
}

const pfConsoleCmdParam::CharPtr &	pfConsoleCmdParam::IToString( void ) const
{
	hsAssert( fType == kString || fType == kAny, "Trying to use a non-string parameter as a string!" );

	return fValue.s;
}

const char &	pfConsoleCmdParam::IToChar( void ) const
{
	hsAssert( fType == kChar || fType == kAny, "Trying to use a non-char parameter as a char!" );

	static char		c;
	if( fType == kAny )
	{
		hsAssert( fValue.s != nil, "Weird parameter during conversion" );
		c = fValue.s[ 0 ];
		return c;
	}
	
	return fValue.c;
}

