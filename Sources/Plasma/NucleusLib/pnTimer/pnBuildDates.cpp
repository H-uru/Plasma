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
//	pnBuildDates.cpp														//
//																			//
//	Thanks to there not being a pre-build step in MSVC, we have to do this	//
//	kluge instead. Basically, as a post-build step, we run a custom app		//
//	that inserts the date and time as string resources into the client EXE.	//
//	Then, on startup, the static class here gets inited and we load the		//
//	string resources into our char strings.									//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsWindows.h"
#include "../pnTimer/pnBuildDates.h"

char	pnBuildDates::fBuildDate[ 128 ] = __DATE__;
char	pnBuildDates::fBuildTime[ 128 ] = __TIME__;

// This forces our constructor to run on app load
static	pnBuildDates	staticObjToForceResLoad;

#if HS_BUILD_FOR_WIN32
HINSTANCE	sModuleHandle = GetModuleHandle( NULL );
#endif

pnBuildDates::pnBuildDates()
{
	// Note; these are int konstants so we don't have to worry about sharing symbols
	// across apps (in this case, our custom app that updates these strings on post-build)
	IGetString( 1000, fBuildDate, sizeof( fBuildDate ) - 1 );
	IGetString( 1001, fBuildTime, sizeof( fBuildTime ) - 1 );
}

void	pnBuildDates::IGetString( int resID, char *destBuffer, int size )
{
#if HS_BUILD_FOR_WIN32
	HRSRC rsrc = ::FindResource( sModuleHandle, MAKEINTRESOURCE( resID ), RT_RCDATA );

	if( rsrc != NULL )
	{
		HGLOBAL handle = ::LoadResource( sModuleHandle, rsrc );

		if( handle != NULL )
		{
			char *str = (char *)::LockResource( handle );
			strncpy( destBuffer, str, size );
			UnlockResource( handle );
		}
	}
#endif
}


