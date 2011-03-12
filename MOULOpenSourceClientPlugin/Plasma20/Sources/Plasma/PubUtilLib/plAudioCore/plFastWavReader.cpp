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
//	plFastWavReader - Quick, dirty, and highly optimized class for reading	//
//					  in the samples of a WAV file when you're in a hurry.	//
//					  ONLY WORKS WITH PCM (i.e. uncompressed) DATA			//
//																			//
//// Notes ///////////////////////////////////////////////////////////////////
//																			//
//	11.5.2001 - Created by mcn.												//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hsTypes.h"
#include "plFastWavReader.h"


//// Local Helpers ///////////////////////////////////////////////////////////

class plRIFFChunk
{
	public:
		char	fID[ 4 ];
		UInt32	fSize;

		void	Read( FILE *fp )
		{
			fread( fID, 1, 4, fp );
			fread( &fSize, sizeof( UInt32 ), 1, fp );
		}

		bool	IsA( const char *type )
		{
			return ( memcmp( fID, type, 4 ) == 0 ) ? true : false;
		}
};

class plRIFFHeader
{
	protected:
		plRIFFChunk		fChunk;
		bool			fValid;
		char			fFormat[ 4 ];

	public:
		plRIFFHeader( FILE *fp )
		{
			fValid = false;
			fChunk.Read( fp );
			if( fChunk.IsA( "RIFF" ) )
			{
				if( fread( &fFormat, 1, 4, fp ) == 4 )
				{
					fValid = true;
				}
			}
		}

		bool	IsValid( void )
		{
			return fValid;
		}

		bool	IsA( const char *type )
		{
			return ( memcmp( fFormat, type, 4 ) == 0 ) ? true : false;
		}
};

//// Constructor/Destructor //////////////////////////////////////////////////

plFastWAV::plFastWAV( const char *path, plAudioCore::ChannelSelect whichChan ) : fFileHandle( nil )
{
	hsAssert( path != nil, "Invalid path specified in plFastWAV reader" );

	strncpy( fFilename, path, sizeof( fFilename ) );
	fWhichChannel = whichChan;

	fFileHandle = fopen( path, "rb" );
	if( fFileHandle != nil )
	{
		/// Read in our header and calc our start position
		plRIFFHeader	riffHdr( fFileHandle );

		if( !riffHdr.IsValid() )
		{
			IError( "Invalid RIFF file header in plFastWAV" );
			return;
		}

		if( !riffHdr.IsA( "WAVE" ) )
		{
			IError( "Invalid RIFF file type in plFastWAV" );
			return;
		}

		fChunkStart = ftell( fFileHandle );

		// Seek and read the "fmt " header
		plRIFFChunk		chunk;
		if( !ISeekToChunk( "fmt ", &chunk ) )
		{
			IError( "Unable to find fmt chunk in WAV file" );
			return;
		}

		if( fread( &fHeader, 1, sizeof( plWAVHeader ), fFileHandle ) != sizeof( plWAVHeader ) )
		{
			IError( "Invalid WAV file header in plFastWAV" );
			return;
		}

		// Check format
		if( fHeader.fFormatTag != kPCMFormatTag )
		{
			IError( "Invalid format in plFastWAV" );
			return;
		}

		// Seek to and get the position of the data chunk
		if( !ISeekToChunk( "data", &chunk ) )
		{
			IError( "Unable to find data chunk in WAV file" );
			return;
		}
		fDataStartPos = ftell( fFileHandle );
		fDataSize = chunk.fSize;

		// HACKY FIX FOR BAD WAV FILES
		fDataSize -= ( fDataSize & ( fHeader.fBlockAlign - 1 ) );

		if( fWhichChannel != plAudioCore::kAll )
		{
			fChannelAdjust = 2;
			fChannelOffset = ( fWhichChannel == plAudioCore::kLeft ) ? 0 : 1;
		}
		else
		{
			fChannelAdjust = 1;
			fChannelOffset = 0;
		}

		fFakeHeader = fHeader;
		fFakeHeader.fAvgBytesPerSec /= fChannelAdjust;
		fFakeHeader.fNumChannels /= (UInt16)fChannelAdjust;
		fFakeHeader.fBlockAlign /= (UInt16)fChannelAdjust;

		SetPosition( 0 );
//		fCurrDataPos = 0;
	}
}

plFastWAV::~plFastWAV()
{
	if( fFileHandle != nil )
		fclose( fFileHandle );
}

bool	plFastWAV::ISeekToChunk( const char *type, plRIFFChunk *c )
{
	plRIFFChunk		chunk;


	// Start from chunk start and search through all the, well, chunks :)
	fseek( fFileHandle, fChunkStart, SEEK_SET );
	while( !feof( fFileHandle ) )
	{
		chunk.Read( fFileHandle );
		if( chunk.IsA( type ) )
		{
			*c = chunk;
			return true;
		}

		// Seek past this one
		fseek( fFileHandle, chunk.fSize, SEEK_CUR );
	}

	return false;
}

void plFastWAV::Open()
{
	if(fFileHandle)
		return;
	
	fFileHandle = fopen( fFilename, "rb" );
	if(!fFileHandle) 
		return;

	fCurrDataPos = 0;

	fseek( fFileHandle, fDataStartPos, SEEK_SET );
}

void	plFastWAV::Close( void )
{
	if( fFileHandle != nil )
	{
		fclose( fFileHandle );
		fFileHandle = nil;
	}
}

void	plFastWAV::IError( const char *msg )
{
	hsAssert( false, msg );
	Close();
}

plWAVHeader	&plFastWAV::GetHeader( void )
{
	hsAssert( IsValid(), "GetHeader() called on an invalid WAV file" );

	return fFakeHeader;
}

float	plFastWAV::GetLengthInSecs( void )
{
	hsAssert( IsValid(), "GetLengthInSecs() called on an invalid WAV file" );

	return (float)( fDataSize / fChannelAdjust ) / (float)fHeader.fAvgBytesPerSec;
}

hsBool	plFastWAV::SetPosition( UInt32 numBytes )
{
	hsAssert( IsValid(), "GetHeader() called on an invalid WAV file" );


	fCurrDataPos = numBytes * fChannelAdjust + ( fChannelOffset * fHeader.fBlockAlign / fChannelAdjust );
	
	hsAssert( fCurrDataPos <= fDataSize, "Invalid new position while seeking WAV file" );

	return ( fseek( fFileHandle, fDataStartPos + fCurrDataPos, SEEK_SET ) == 0 ) ? true : false;
}

hsBool	plFastWAV::Read( UInt32 numBytes, void *buffer )
{
	hsAssert( IsValid(), "GetHeader() called on an invalid WAV file" );


	if( fWhichChannel != plAudioCore::kAll )
	{
		size_t	numRead, sampleSize = fHeader.fBlockAlign / fChannelAdjust;
		static UInt8	trashBuffer[ 32 ];

		UInt32 numBytesFull = numBytes;
		if( fCurrDataPos + ( numBytes * fChannelAdjust ) > fDataSize )
			numBytesFull -= sampleSize;

		for( numRead = 0; numRead < numBytesFull; numRead += sampleSize )
		{
			size_t thisRead = fread( buffer, 1, sampleSize, fFileHandle );
			if( thisRead != sampleSize )
				return false;

//			fseek( fFileHandle, sampleSize * ( fChannelAdjust - 1 ), SEEK_CUR );
			thisRead = fread( trashBuffer, 1, sampleSize, fFileHandle );
			if( thisRead != sampleSize )
				return false;

			buffer = (void *)( (UInt8 *)buffer + sampleSize );
			fCurrDataPos += sampleSize * fChannelAdjust;
		}

		if( numRead < numBytes )
		{
			if( numBytes - numRead > sampleSize )
			{
				hsAssert( false, "Invalid logic in plFastWAV::Read()" );
				return false;
			}

	
			// Must not have enough room left, so no more skipping
			size_t thisRead = fread( buffer, 1, sampleSize, fFileHandle );

			if( thisRead != sampleSize )
				return false;

			buffer = (void *)( (UInt8 *)buffer + sampleSize );
			fCurrDataPos += sampleSize;
		}

		hsAssert( fCurrDataPos <= fDataSize, "Invalid new position while reading WAV file" );
	}
	else
	{	
		size_t numRead = fread( buffer, 1, numBytes, fFileHandle );
		
		fCurrDataPos += numRead;
		hsAssert( fCurrDataPos <= fDataSize, "Invalid new position while reading WAV file" );

		if( numRead < numBytes )
			return false;
	}

	return true;
}

UInt32	plFastWAV::NumBytesLeft( void )
{
	hsAssert( IsValid(), "GetHeader() called on an invalid WAV file" );
	hsAssert( fCurrDataPos <= fDataSize, "Invalid current position while reading WAV file" );

	return ( fDataSize - fCurrDataPos ) / fChannelAdjust;
}
