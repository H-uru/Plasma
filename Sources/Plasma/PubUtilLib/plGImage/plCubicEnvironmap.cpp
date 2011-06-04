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
//	plCubicEnvironmap Class Functions										 //
//	Derived bitmap class representing a collection of mipmaps to be used for //
//	cubic environment mapping.												 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	6.7.2001 mcn - Created.													 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "plCubicEnvironmap.h"
#include "plMipmap.h"


//// Constructor & Destructor /////////////////////////////////////////////////

plCubicEnvironmap::plCubicEnvironmap()
{
	int		i;

	for( i = 0; i < 6; i++ )
		fFaces[ i ] = TRACKED_NEW plMipmap;

	fInitialized = false;
}

plCubicEnvironmap::~plCubicEnvironmap()
{
	int		i;

	for( i = 0; i < 6; i++ )
		delete fFaces[ i ];
}

//// GetTotalSize /////////////////////////////////////////////////////////////
//	Get the total size in bytes

UInt32	plCubicEnvironmap::GetTotalSize( void ) const
{
	UInt32	size, i;


	for( size = 0, i = 0; i < 6; i++ )
	{
		hsAssert( fFaces[ i ] != nil, "Nil face in GetTotalSize()" );
		size += fFaces[ i ]->GetTotalSize();
	}

	return size;
}

//// Read /////////////////////////////////////////////////////////////////////

UInt32	plCubicEnvironmap::Read( hsStream *s )
{
	UInt32	i, tr = plBitmap::Read( s );


	for( i = 0; i < 6; i++ )
		tr += fFaces[ i ]->Read( s );

	fInitialized = true;

	return tr;
}

//// Write ////////////////////////////////////////////////////////////////////

UInt32	plCubicEnvironmap::Write( hsStream *s )
{
	UInt32	i, tw = plBitmap::Write( s );


	for( i = 0; i < 6; i++ )
		tw += fFaces[ i ]->Write( s );

	return tw;
}

//// CopyToFace ///////////////////////////////////////////////////////////////
//	Export-only: Copy the mipmap given into a face

void	plCubicEnvironmap::CopyToFace( plMipmap *mip, UInt8 face )
{
	hsAssert( face < 6, "Invalid face index in CopyToFace()" );
	hsAssert( fFaces[ face ] != nil, "nil face in CopyToFace()" );
	hsAssert( mip != nil, "nil source in CopyToFace()" );


	if( !fInitialized )
	{
		// Make sure our stuff matches
		fCompressionType = mip->fCompressionType;
		if( fCompressionType != kDirectXCompression )
			fUncompressedInfo.fType = mip->fUncompressedInfo.fType;
		else
		{
			fDirectXInfo.fBlockSize = mip->fDirectXInfo.fBlockSize;
			fDirectXInfo.fCompressionType = mip->fDirectXInfo.fCompressionType;
		}

		fPixelSize = mip->GetPixelSize();
		fSpace = kDirectSpace;
		fFlags = mip->GetFlags();

		fInitialized = true;
	}
	else
	{
		// Check to make sure their stuff matches
		if( IsCompressed() != mip->IsCompressed() )
		{
			hsAssert( false, "Compression types do not match in CopyToFace()" );
			return;
		}

		if( !IsCompressed() )
		{
			if( fUncompressedInfo.fType != mip->fUncompressedInfo.fType )
			{
				hsAssert( false, "Compression formats do not match in CopyToFace()" );
				return;
			}
		}
		else
		{
			if( fDirectXInfo.fBlockSize != mip->fDirectXInfo.fBlockSize ||
				fDirectXInfo.fCompressionType != mip->fDirectXInfo.fCompressionType )
			{
				hsAssert( false, "Compression formats do not match in CopyToFace()" );
				return;
			}
		}

		if( fPixelSize != mip->GetPixelSize() )
		{
			hsAssert( false, "Bitdepths do not match in CopyToFace()" );
			return;
		}

		if( fFlags != mip->GetFlags() )
		{
			hsAssert( false, "Flags do not match in CopyToFace()" );
		}
	}

	// Copy the mipmap data
	fFaces[ face ]->CopyFrom( mip );
}

