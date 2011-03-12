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
///////////////////////////////////////////////////////////////////////////////
//																			 //
//	pcSmallRect - A tiny Int16-based 2D rectangle class						 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

// Note: I'm introducing the concept here that new-style coreLib files/classes
// are named with the "pc" prefix, just as featureLib uses "pf" and nucleusLib
// uses "pn". I have no clue if this is kosher or good, but I'm doing it anyway.
// Feel free to change if desired

#ifndef _pcSmallRect_h
#define _pcSmallRect_h

#include "hsTypes.h"

class hsStream;
class pcSmallRect
{
	public:
		Int16	fX, fY, fWidth, fHeight;

		pcSmallRect() { Empty(); }
		pcSmallRect( Int16 x, Int16 y, Int16 w, Int16 h ) { Set( x, y, w, h ); }
		
		void	Set( Int16 x, Int16 y, Int16 w, Int16 h ) { fX = x; fY = y; fWidth = w; fHeight = h; }
		void	Empty( void ) { fX = fY = fWidth = fHeight = 0; }

		Int16	GetRight( void ) const { return fX + fWidth; }
		Int16	GetBottom( void ) const { return fY + fHeight; }

		void	Read( hsStream *s );
		void	Write( hsStream *s );

		hsBool	Contains( Int16 x, Int16 y ) { if( x >= fX && x <= fX + fWidth && y >= fY && y <= fY + fHeight ) return true; return false; }
};


#endif // _pcSmallRect_h
