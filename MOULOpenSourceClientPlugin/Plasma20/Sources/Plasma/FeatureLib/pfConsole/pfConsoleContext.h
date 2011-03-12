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
//	pfConsoleContext Header													//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfConsoleContext_h
#define _pfConsoleContext_h

#include "hsTypes.h"
#include "pfConsoleCmd.h"


//// Class Definition ////////////////////////////////////////////////////////

class pfConsoleContext
{
	protected:

		hsBool	fAddWhenNotFound;		// Controls whether we add variables on Set() calls if they're not found

		char	*fName;

		hsTArray<char *>			fVarNames;
		hsTArray<pfConsoleCmdParam>	fVarValues;

		void	IAddVar( const char *name, const pfConsoleCmdParam &value );

		static pfConsoleContext	fRootContext;

	public:

		pfConsoleContext( const char *name );
		virtual ~pfConsoleContext();

		void	Clear( void );

		UInt32				GetNumVars( void ) const;
		const char			*GetVarName( UInt32 idx ) const;
		pfConsoleCmdParam	&GetVarValue( UInt32 idx ) const;

		Int32	FindVar( const char *name ) const;
		void	RemoveVar( UInt32 idx );

		void	AddVar( const char *name, const pfConsoleCmdParam &value );
		void	AddVar( const char *name, int value );
		void	AddVar( const char *name, float value );
		void	AddVar( const char *name, const char *value );
		void	AddVar( const char *name, char value );
		void	AddVar( const char *name, bool value );

		hsBool	SetVar( UInt32 idx, const pfConsoleCmdParam &value );

		hsBool	SetVar( const char *name, const pfConsoleCmdParam &value );
		hsBool	SetVar( const char *name, int value );
		hsBool	SetVar( const char *name, float value );
		hsBool	SetVar( const char *name, const char *value );
		hsBool	SetVar( const char *name, char value );
		hsBool	SetVar( const char *name, bool value );

		// Decide whether Sets() on nonexistant variables will fail or add a new variable
		void	SetAddWhenNotFound( hsBool f ) { fAddWhenNotFound = f; }
		hsBool	GetAddWhenNotFound( void ) const { return fAddWhenNotFound; }

		static pfConsoleContext	&GetRootContext( void );
};


#endif //_pfConsoleContext_h

