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
//////////////////////////////////////////////////////////////////////////////

#ifndef _plBufferedFileReader_h
#define _plBufferedFileReader_h

#include "../plAudioCore/plAudioFileReader.h"


//// Class Definition ////////////////////////////////////////////////////////

class plBufferedFileReader : public plAudioFileReader
{
public:
	plBufferedFileReader( const char *path, plAudioCore::ChannelSelect whichChan = plAudioCore::kAll );
	virtual ~plBufferedFileReader();

	virtual plWAVHeader	&GetHeader( void );
	virtual void	Close( void );
	virtual UInt32	GetDataSize( void ) { return fBufferSize; }
	virtual float	GetLengthInSecs( void );
	virtual hsBool	SetPosition( UInt32 numBytes );
	virtual hsBool	Read( UInt32 numBytes, void *buffer );
	virtual UInt32	NumBytesLeft( void );
	virtual hsBool	IsValid( void ) { return ( fBuffer != nil ) ? true : false; }

protected:
	UInt32			fBufferSize;
	UInt8			*fBuffer;
	plWAVHeader		fHeader;
	UInt32			fCursor;
	void			IError( const char *msg );
};

#endif //_plBufferedFileReader_h
