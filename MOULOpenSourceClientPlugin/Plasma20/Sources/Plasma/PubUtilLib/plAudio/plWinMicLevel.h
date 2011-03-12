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
#ifndef _plWinMicLevel_h
#define _plWinMicLevel_h

//////////////////////////////////////////////////////////////////////////////
//																			//
//	plWinMicLevel - Annoying class to deal with the annoying problem of		//
//					setting the microphone recording volume in Windows.		//
//					Yeah, you'd THINK there'd be some easier way...			//
//																			//
//// Notes ///////////////////////////////////////////////////////////////////
//																			//
//	5.8.2001 - Created by mcn.												//
//																			//
//////////////////////////////////////////////////////////////////////////////


//// Class Definition ////////////////////////////////////////////////////////

class plWinMicLevel
{
public:

	~plWinMicLevel();
	// Gets the microphone volume, range 0-1, -1 if error
	static hsScalar	GetLevel( void );

	// Sets the microphone volume, range 0-1
	static void		SetLevel( hsScalar level );

	// Returns whether we can set the level
	static hsBool	CanSetLevel( void );

protected:
	plWinMicLevel();	// Protected constructor for IGetInstance. Just to init some stuff
	static plWinMicLevel	&IGetInstance( void );
	void	IShutdown( void );
};

#endif // _plWinMicLevel_h