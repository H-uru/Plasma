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
//	pfConsoleEngine Header													//
//																			//
//// Description /////////////////////////////////////////////////////////////
//																			//
//	The engine is where parsing and execution of console commands takes		//
//	place. It takes place independently of the interface, so that the		//
//	execution can happen from an INI file, from a screen-based console or	//
//	even a GUI interface.													//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfConsoleEngine_h
#define _pfConsoleEngine_h

#include "hsTypes.h"
#include "hsUtils.h"


//// pfConsoleEngine Class Definition ////////////////////////////////////////

class pfConsoleCmdParam;
class pfConsoleCmdGroup;
class pfConsoleEngine
{
	private:

		static const Int32		fMaxNumParams;
		static const char		fTokenSeparators[];
		static const char		fTokenGrpSeps[];

		hsBool	IConvertToParam( UInt8 type, char *string, pfConsoleCmdParam *param );

		char	fErrorMsg[ 128 ];
		char	fLastErrorLine[ 512 ];

		void	ISetErrorMsg( char *msg ) { hsStrncpy( fErrorMsg, msg, sizeof( fErrorMsg ) ); }

		// Recursive function to build a string of the groups a command is in
		void		IBuildCmdNameRecurse( pfConsoleCmdGroup *group, char *string );

	public:

		pfConsoleEngine();
		~pfConsoleEngine();

		// Gets the signature for the command given (NO groups!)
		const char	*GetCmdSignature( char *name );

		// Prints out the help for a given command (or group)
		hsBool	PrintCmdHelp( char *name, void (*PrintFn)( const char * ) );

		// Breaks the given line into a command and parameters and runs the command
		hsBool	RunCommand( char *line, void (*PrintFn)( const char * ) );

		// Executes the given file as a sequence of console commands
		hsBool	ExecuteFile( const char *fileName );
		hsBool	ExecuteFile( const wchar *fileName );

		// Get the last reported error
		const char	*GetErrorMsg( void ) { return fErrorMsg; }

		// Get the line for which the last reported error was for
		const char	*GetLastErrorLine( void ) { return fLastErrorLine; }

		// Does command completion on a partially-complete console line
		hsBool		FindPartialCmd( char *line, hsBool findAgain = false, hsBool perserveParams = false );

		// Does command completion without restrictions to any group, skipping the number of matches given
		hsBool		FindNestedPartialCmd( char *line, UInt32 numToSkip, hsBool perserveParams = false );
};


#endif //_pfConsoleEngine_h
