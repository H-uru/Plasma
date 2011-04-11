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
//	plTGAWriter Class Functions												 //
//	Simple utility class for writing a plMipmap out as a TGA file			 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	8.15.2001 mcn - Created.												 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "plTGAWriter.h"
#include "plMipmap.h"
#include "hsStream.h"


//// Class Statics ////////////////////////////////////////////////////////////

plTGAWriter	plTGAWriter::fInstance;


//// WriteMipmap //////////////////////////////////////////////////////////////

void	plTGAWriter::WriteMipmap( const char *fileName, plMipmap *mipmap )
{
	hsUNIXStream	stream;
	int				x, y;
	hsRGBAColor32	pixel;


	stream.Open( fileName, "wb" );

	/// Write header
	stream.WriteByte( 0 );	// Size of ID field
	stream.WriteByte( 0 );	// Map type
	stream.WriteByte( 2 );	// Type 2 image - Unmapped RGB

	stream.WriteByte( 0 );	// Color map spec
	stream.WriteByte( 0 );	// Color map spec
	stream.WriteByte( 0 );	// Color map spec
	stream.WriteByte( 0 );	// Color map spec
	stream.WriteByte( 0 );	// Color map spec

	stream.WriteSwap16( 0 );	// xOrigin
	stream.WriteSwap16( 0 );	// yOrigin

	stream.WriteSwap16( (UInt16)mipmap->GetWidth() );
	stream.WriteSwap16( (UInt16)mipmap->GetHeight() );

	stream.WriteByte( 24 );
	stream.WriteByte( 0 );

	/// Write image data (gotta do inversed, stupid TGAs...)
	for( y = mipmap->GetHeight() - 1; y >= 0; y-- )
	{
		for( x = 0; x < mipmap->GetWidth(); x++ )
		{
			pixel = *( (hsRGBAColor32 *)mipmap->GetAddr32( x, y ) );
			stream.WriteByte( pixel.b );
			stream.WriteByte( pixel.g );
			stream.WriteByte( pixel.r );
		}
	}

	/// All done!
	stream.Close();
}

