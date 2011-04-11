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
//	plEncryptLogLine Header													//
//																			//
//// Description /////////////////////////////////////////////////////////////
//																			//
//	Broken into a separate file for easy include in utility apps			//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "plEncryptLogLine.h"

#include <string.h>

void	plStatusEncrypt::Encrypt( UInt8 *line, UInt8 hint )
{
	// Current encryption scheme: rotate all characters right by 2 bits,
	// then rotate the whole damn line by 3 bits to the right
	UInt32 i, len = strlen( (char *)line );
	UInt8 newHi, hiBits = ( ( line[ len - 1 ] ) << ( 5 - 2 ) ) & 0xe0;

	for( i = 0; i < len; i++ )
	{
		// So each character will be the src char rotated right 2 bits, then shifted
		// right 3 bits, or'ed with the last high bits, and the 3 discarded bits
		// become the new high bits
		// Too bad C doesn't have a bit-rotate op

		line[ i ] = ( line[ i ] << 6 ) | ( line[ i ] >> 2 );
		newHi = line[ i ] << 5;
		line[ i ] = ( line[ i ] >> 3 ) | hiBits;
		line[ i ] ^= hint;	// Should wrap around

		hiBits = newHi;
	}
}

void	plStatusEncrypt::Decrypt( UInt8 *line, Int32 len, UInt8 hint )
{
	// Da reverse, of course!
	Int32 i;
	UInt8 lastChar = 0, newLo, loBits = ( line[ 0 ] ^ hint ) >> 5;

	for( i = len - 1; i >= 0; i-- )
	{
		lastChar = line[ i ];
		line[ i ] ^= hint;
		newLo = line[ i ] >> 5;
		line[ i ] = ( line[ i ] << 3 ) | loBits;
		line[ i ] = ( line[ i ] >> 6 ) | ( line[ i ] << 2 );

		loBits = newLo;
	}
}
