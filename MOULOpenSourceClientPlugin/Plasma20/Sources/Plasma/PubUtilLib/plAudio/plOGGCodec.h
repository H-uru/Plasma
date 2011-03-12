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
//	plOGGCodec - Plasma codec support for the OGG/Vorbis file format.		//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plOGGCodec_h
#define _plOGGCodec_h

#include "../plAudioCore/plAudioFileReader.h"


//// Class Definition ////////////////////////////////////////////////////////

struct OggVorbis_File;

class plOGGCodec : public plAudioFileReader
{
public:

	plOGGCodec( const char *path, plAudioCore::ChannelSelect whichChan = plAudioCore::kAll );
	virtual ~plOGGCodec();

	enum DecodeFormat
	{
		k8bitUnsigned,
		k16bitSigned
	};

	enum DecodeFlags
	{
		kFastSeeking = 0x01
	};
	
	virtual plWAVHeader	&GetHeader( void );

	virtual void	Close( void );

	virtual UInt32	GetDataSize( void ) { return fDataSize / fChannelAdjust; }
	virtual float	GetLengthInSecs( void );

	virtual hsBool	SetPosition( UInt32 numBytes );
	virtual hsBool	Read( UInt32 numBytes, void *buffer );
	virtual UInt32	NumBytesLeft( void );

	virtual hsBool	IsValid( void ) { return ( fOggFile != nil ) ? true : false; }

	static void		SetDecodeFormat( DecodeFormat f ) { fDecodeFormat = f; }
	static void		SetDecodeFlag( UInt8 flag, hsBool on ) { if( on ) fDecodeFlags |= flag; else fDecodeFlags &= ~flag; }
	static UInt8	GetDecodeFlags( void ) { return fDecodeFlags; }
	void			ResetWaveHeaderRef() { fCurHeaderPos = 0; }
	void			BuildActualWaveHeader();
	bool			ReadFromHeader(int numBytes, void *data); // read from Actual wave header

protected:

	enum
	{
		kPCMFormatTag = 1
	};

	char			fFilename[ 512 ];
	FILE			*fFileHandle;
	OggVorbis_File	*fOggFile;

	plWAVHeader		fHeader, fFakeHeader;
	UInt32			fDataStartPos, fCurrDataPos, fDataSize;

	plAudioCore::ChannelSelect	fWhichChannel;
	UInt32						fChannelAdjust, fChannelOffset;

	static DecodeFormat	fDecodeFormat;
	static UInt8		fDecodeFlags;
	UInt8 *				fHeadBuf;
	int					fCurHeaderPos;

	void	IError( const char *msg );
	void	IOpen( const char *path, plAudioCore::ChannelSelect whichChan = plAudioCore::kAll );
};

#endif //_plOGGCodec_h
