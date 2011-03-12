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
//	plBufferedFileReader - Reads in a given file into a RAM buffer, then	//
//						   "reads" from that buffer as requested. Useless	//
//						   for normal sounds, but perfect for streaming		//
//						   from RAM.										//
//																			//
//// Notes ///////////////////////////////////////////////////////////////////
//																			//
//	11.1.2002 - Created by mcn.												//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hsTypes.h"
#include "plBufferedFileReader.h"
//#include "plProfile.h"


//plProfile_CreateMemCounter( "BufferedRAM", "Sound", SndBufferedMem );

//// Constructor/Destructor //////////////////////////////////////////////////

plBufferedFileReader::plBufferedFileReader( const char *path, plAudioCore::ChannelSelect whichChan )
{
	// Init some stuff
	fBufferSize = 0;
	fBuffer = nil;
	fCursor = 0;

	hsAssert( path != nil, "Invalid path specified in plBufferedFileReader" );

	// Ask plAudioFileReader for another reader to get this file
	// Note: have this reader do the chanSelect for us
	plAudioFileReader *reader = plAudioFileReader::CreateReader( path, whichChan );
	if( reader == nil || !reader->IsValid() )
	{
		delete reader;
		IError( "Unable to open file to read in to RAM buffer" );
		return;
	}

	fHeader = reader->GetHeader();

	fBufferSize = reader->GetDataSize();
	fBuffer = TRACKED_NEW UInt8[ fBufferSize ];
	//plProfile_NewMem( SndBufferedMem, fBufferSize );
	if( fBuffer == nil )
	{
		delete reader;
		IError( "Unable to allocate RAM buffer" );
		return;
	}

	if( !reader->Read( fBufferSize, fBuffer ) )
	{
		delete reader;
		IError( "Unable to read file into RAM buffer" );
		return;
	}

	// All done!
	delete reader;
}

plBufferedFileReader::~plBufferedFileReader()
{
	Close();
}

void	plBufferedFileReader::Close( void )
{
	//plProfile_DelMem( SndBufferedMem, fBufferSize );

	DEL(fBuffer);;
	fBuffer = nil;
	fBufferSize = 0;
	fCursor = 0;
}

void	plBufferedFileReader::IError( const char *msg )
{
	hsAssert( false, msg );
	Close();
}

plWAVHeader	&plBufferedFileReader::GetHeader( void )
{
	hsAssert( IsValid(), "GetHeader() called on an invalid RAM buffer" );

	return fHeader;
}

float	plBufferedFileReader::GetLengthInSecs( void )
{
	hsAssert( IsValid(), "GetLengthInSecs() called on an invalid RAM buffer" );

	return (float)fBufferSize / (float)fHeader.fAvgBytesPerSec;
}

hsBool	plBufferedFileReader::SetPosition( UInt32 numBytes )
{
	hsAssert( IsValid(), "SetPosition() called on an invalid RAM buffer" );

	if( numBytes > fBufferSize )
	{
		hsAssert( false, "Invalid position in SetPosition()" );
		return false;
	}

	fCursor = numBytes;
	return true;
}

hsBool	plBufferedFileReader::Read( UInt32 numBytes, void *buffer )
{
	hsAssert( IsValid(), "Read() called on an invalid RAM buffer" );


	hsBool valid = true;

	if( fCursor + numBytes > fBufferSize )
	{
		numBytes = fBufferSize - fCursor;
		valid = false;
	}

	memcpy( buffer, fBuffer + fCursor, numBytes );
	fCursor += numBytes;

	return valid;
}

UInt32	plBufferedFileReader::NumBytesLeft( void )
{
	hsAssert( IsValid(), "NumBytesLeft() called on an invalid RAM buffer" );

	return fBufferSize - fCursor;
}
