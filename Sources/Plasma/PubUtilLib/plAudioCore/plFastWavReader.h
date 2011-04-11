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
//////////////////////////////////////////////////////////////////////////////

#ifndef _plFastWavReader_h
#define _plFastWavReader_h

#include "plAudioFileReader.h"


//// Class Definition ////////////////////////////////////////////////////////

class plRIFFChunk;

class plFastWAV : public plAudioFileReader
{
public:
	plFastWAV( const char *path, plAudioCore::ChannelSelect whichChan = plAudioCore::kAll );
	virtual ~plFastWAV();

	virtual plWAVHeader	&GetHeader( void );

	virtual void	Open();
	virtual void	Close( void );

	virtual UInt32	GetDataSize( void ) { return fDataSize / fChannelAdjust; }
	virtual float	GetLengthInSecs( void );

	virtual hsBool	SetPosition( UInt32 numBytes );
	virtual hsBool	Read( UInt32 numBytes, void *buffer );
	virtual UInt32	NumBytesLeft( void );

	virtual hsBool	IsValid( void ) { return ( fFileHandle != nil ) ? true : false; }

protected:
	enum
	{
		kPCMFormatTag = 1
	};

	char			fFilename[ 512 ];
	FILE *			fFileHandle;
	plWAVHeader		fHeader, fFakeHeader;
	UInt32			fDataStartPos, fCurrDataPos, fDataSize;
	UInt32			fChunkStart;
	plAudioCore::ChannelSelect	fWhichChannel;
	UInt32						fChannelAdjust, fChannelOffset;

	void	IError( const char *msg );
	bool	ISeekToChunk( const char *type, plRIFFChunk *c );
};

#endif //_plFastWavReader_h
