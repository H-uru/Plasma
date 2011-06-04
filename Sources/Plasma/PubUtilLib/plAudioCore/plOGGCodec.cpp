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
//// Notes ///////////////////////////////////////////////////////////////////
//																			//
//	2.7.2003 - Created by mcn. If only life were really this simple...		//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <math.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include "hsTypes.h"
#include "plOGGCodec.h"

#include "hsTimer.h"
#include "../pnNetCommon/plNetApp.h"

plOGGCodec::DecodeFormat	plOGGCodec::fDecodeFormat = plOGGCodec::k16bitSigned;
UInt8						plOGGCodec::fDecodeFlags = 0;

//// Constructor/Destructor //////////////////////////////////////////////////

plOGGCodec::plOGGCodec( const char *path, plAudioCore::ChannelSelect whichChan ) : fFileHandle( nil )
{
	fOggFile = nil;
	IOpen( path, whichChan );
	fCurHeaderPos = 0;
	fHeadBuf = nil;
}

void	plOGGCodec::BuildActualWaveHeader()
{
	// Build an actual WAVE header for this ogg
	int fmtSize = 16;
	short fmt = 1;
	int factsize = 4;
	int factdata = 0;
	int size = fDataSize+48; // size of data with header except for first four bytes

	fHeadBuf = (UInt8 *) ALLOC(56);
	memcpy(fHeadBuf, "RIFF", 4);
	memcpy(fHeadBuf+4, &size, 4);
	memcpy(fHeadBuf+8, "WAVE", 4);
	memcpy(fHeadBuf+12, "fmt ", 4);
	memcpy(fHeadBuf+16, &fmtSize, 4);
	memcpy(fHeadBuf+20, &fmt, 2); /* format */
	memcpy(fHeadBuf+22, &fHeader.fNumChannels, 2);
	memcpy(fHeadBuf+24, &fHeader.fNumSamplesPerSec, 4);
	memcpy(fHeadBuf+28, &fHeader.fAvgBytesPerSec, 4);
	memcpy(fHeadBuf+32, &fHeader.fBlockAlign, 4);
	memcpy(fHeadBuf+34, &fHeader.fBitsPerSample, 2);
	memcpy(fHeadBuf+36, "fact", 4);
	memcpy(fHeadBuf+40, &factsize, 4);
	memcpy(fHeadBuf+44, &factdata, 4);
	memcpy(fHeadBuf+48, "data", 4);
	memcpy(fHeadBuf+52, &fDataSize, 4);
}

bool plOGGCodec::ReadFromHeader(int numBytes, void *data)
{
	if(fCurHeaderPos < 56)
	{
		memcpy(data, fHeadBuf+fCurHeaderPos, numBytes);
		fCurHeaderPos += numBytes;
		return true;
	}
	return false;
}

void	plOGGCodec::IOpen( const char *path, plAudioCore::ChannelSelect whichChan )
{
	hsAssert( path != nil, "Invalid path specified in plOGGCodec reader" );

	// plNetClientApp::StaticDebugMsg("Ogg Open %s, t=%f, start", path, hsTimer::GetSeconds());

	strncpy( fFilename, path, sizeof( fFilename ) );
	fWhichChannel = whichChan;

	/// Open the file as a plain binary stream
	fFileHandle = fopen( path, "rb" );
	if( fFileHandle != nil )
	{
		/// Create the OGG data struct
		fOggFile = TRACKED_NEW OggVorbis_File;

		/// Open the OGG decompressor
		if( ov_open( fFileHandle, fOggFile, NULL, 0 ) < 0 )
		{
			IError( "Unable to open OGG source file" );
			return;
		}

		/// Construct some header info from the ogg info
	    vorbis_info *vInfo = ov_info( fOggFile, -1 );

		fHeader.fFormatTag = 1;
		fHeader.fNumChannels = vInfo->channels;
		fHeader.fNumSamplesPerSec = vInfo->rate;
	
		// Funny thing about the bits per sample: we get to CHOOSE. Go figure! 
		fHeader.fBitsPerSample = ( fDecodeFormat == k8bitUnsigned ) ? 8 : 16;

		// Why WAV files hold this info when it can be calculated is beyond me...
		fHeader.fBlockAlign = ( fHeader.fBitsPerSample * fHeader.fNumChannels ) >> 3;
		fHeader.fAvgBytesPerSec = fHeader.fNumSamplesPerSec * fHeader.fBlockAlign;

		
		/// The size in bytes of our PCM data stream
		/// Note: OGG sometimes seems to be off by 1 sample, which causes our reads to suck
		/// because we end up waiting for 1 more sample than we actually have. So, on the
		/// assumption that OGG is just slightly wrong sometimes, we just subtract 1 sample
		/// from what it tells us. As Brice put it, who's going to miss 1/40,000'ths of a second?
		fDataSize = (UInt32)(( ov_pcm_total( fOggFile, -1 ) - 1 ) * fHeader.fBlockAlign);

		/// Channel select
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
		

		/// Construct our fake header for channel adjustment
		fFakeHeader = fHeader;
		fFakeHeader.fAvgBytesPerSec /= fChannelAdjust;
		fFakeHeader.fNumChannels /= (UInt16)fChannelAdjust;
		fFakeHeader.fBlockAlign /= (UInt16)fChannelAdjust;

		SetPosition( 0 );
	}
//	plNetClientApp::StaticDebugMsg("Ogg Open %s, t=%f, end", path, hsTimer::GetSeconds());
}

plOGGCodec::~plOGGCodec()
{
	Close();
}

void	plOGGCodec::Close( void )
{
	// plNetClientApp::StaticDebugMsg("Ogg Close, t=%f, start", hsTimer::GetSeconds());
	FREE(fHeadBuf);
	fHeadBuf = nil;
	if( fOggFile != nil )
	{
		ov_clear( fOggFile );
		DEL(fOggFile);
		fOggFile = nil;
	}

	if( fFileHandle != nil )
	{
		fclose( fFileHandle );
		fFileHandle = nil;
	}
	// plNetClientApp::StaticDebugMsg("Ogg Close, t=%f, end", hsTimer::GetSeconds());
}

void	plOGGCodec::IError( const char *msg )
{
	hsAssert( false, msg );
	Close();
}

plWAVHeader	&plOGGCodec::GetHeader( void )
{
	hsAssert( IsValid(), "GetHeader() called on an invalid OGG file" );

	return fFakeHeader;
}

float	plOGGCodec::GetLengthInSecs( void )
{
	hsAssert( IsValid(), "GetLengthInSecs() called on an invalid OGG file" );

	// Just query ogg directly...starting to see how cool ogg is yet?
	return (float)ov_time_total( fOggFile, -1 );
}

hsBool	plOGGCodec::SetPosition( UInt32 numBytes )
{
	hsAssert( IsValid(), "GetHeader() called on an invalid OGG file" );
	

	if( !ov_seekable( fOggFile ) )
	{
		hsAssert( false, "Trying to set position on an unseekable OGG stream!" );
		return false;
	}

	// The numBytes position is in uncompressed space and should be sample-aligned anyway,
	// so this should be just fine here.
	ogg_int64_t newSample = ( numBytes / (fFakeHeader.fBlockAlign * fChannelAdjust) );

	// Now please note how freaking easy it is here to do accurate or fast seeking...
	// Also note that if we're doing our channel extraction, we MUST do it the accurate way
	if( ( fDecodeFlags & kFastSeeking ) && fChannelAdjust == 1 )
	{
		if( ov_pcm_seek_page( fOggFile, newSample ) != 0 )
		{
			IError( "Unable to seek OGG stream" );
			return false;
		}
	}
	else
	{
		if( ov_pcm_seek( fOggFile, newSample ) != 0 )
		{
			IError( "Unable to seek OGG stream" );
			return false;
		}
	}
	return true;
}

hsBool	plOGGCodec::Read( UInt32 numBytes, void *buffer )
{
	hsAssert( IsValid(), "GetHeader() called on an invalid OGG file" );
//	plNetClientApp::StaticDebugMsg("Ogg Read, t=%f, start", hsTimer::GetSeconds());

	int	bytesPerSample = ( fDecodeFormat == k16bitSigned ) ? 2 : 1;
	int isSigned = ( fDecodeFormat == k16bitSigned ) ? 1 : 0;
	int currSection;
	
	if( fWhichChannel == plAudioCore::kAll )
	{
		// Easy, just a straight read
		char	*uBuffer = (char *)buffer;

		while( numBytes > 0 )
		{
			// Supposedly we should pay attention to currSection in case of bitrate changes,
			// but hopefully we'll never have those....

			long bytesRead = ov_read( fOggFile, uBuffer, numBytes, 0, bytesPerSample, isSigned, &currSection );
			
			// Since our job is so simple, do some extra error checking
			if( bytesRead == OV_HOLE )
			{
				IError( "Unable to read from OGG file: missing data" );
				return false;
			}
			else if( bytesRead == OV_EBADLINK )
			{
				IError( "Unable to read from OGG file: corrupt link" );
				return false;
			}
			else if( bytesRead == 0 )
			{
				IError( "Unable to finish reading from OGG file: end of stream" );
				return false;
			}
			else if( bytesRead < 0 )
			{
				IError( "Unable to read from OGG file: unknown error" );
				return false;
			}

			numBytes -= bytesRead;
			uBuffer += bytesRead;
		}
	}
	else
	{
		/// Read in 4k chunks and extract
		static char		trashBuffer[ 4096 ];

		long	toRead, i, thisRead, sampleSize = fFakeHeader.fBlockAlign;

		for( ; numBytes > 0; )
		{
			/// Read 4k worth of samples
			toRead = ( sizeof( trashBuffer ) < numBytes * fChannelAdjust ) ? sizeof( trashBuffer ) : numBytes * fChannelAdjust;


			thisRead = ov_read( fOggFile, (char *)trashBuffer, toRead, 0, bytesPerSample, isSigned, &currSection );
			if( thisRead < 0 )
				return false;

			/// Copy every other sample out
			int sampleOffset = (fChannelOffset == 1) ? sampleSize : 0;
			for (i = 0; i < thisRead; i += sampleSize * 2)
			{
				memcpy(buffer, &trashBuffer[i + sampleOffset], sampleSize);
				buffer = (void*)((UInt8*)buffer + sampleSize);

				numBytes -= sampleSize;
			}
		}
	}

//	plNetClientApp::StaticDebugMsg("Ogg Read, t=%f, end", hsTimer::GetSeconds());
	return true;
}

UInt32	plOGGCodec::NumBytesLeft( void )
{
	if(!IsValid())
	{
		hsAssert( false, "GetHeader() called on an invalid OGG file" );
		return 0;
	}

	return (UInt32)(( fDataSize - ( ov_pcm_tell( fOggFile ) * fHeader.fBlockAlign ) ) / fChannelAdjust);
}
