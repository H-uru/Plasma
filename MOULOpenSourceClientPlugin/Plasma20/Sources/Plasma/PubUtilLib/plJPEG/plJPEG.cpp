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
//	plJPEG - JPEG Codec Wrapper for Plasma									 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	2.1.2002 mcn - Created.													 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "plJPEG.h"
#include "hsStream.h"
#include "hsExceptions.h"
#include "hsUtils.h"
#include "../plGImage/plMipmap.h"

#ifndef HS_BUILD_FOR_WIN32
#error Currently the JPEG libraries don't build for anything but Win32. If you're building this on a non-Win32 platform....WHY??
#endif

#include "../../../../../StaticSDKs/Win32/IJL/include/ijl.h"

//// Local Statics ////////////////////////////////////////////////////////////
//	Done this way so we don't have to declare them in the .h file and pull in
//	the platform-specific library

static IJLERR	sLastErrCode = IJL_OK;


//// Instance /////////////////////////////////////////////////////////////////

plJPEG	&plJPEG::Instance( void )
{
	static plJPEG	theInstance;
	return theInstance;
}

//// GetLastError /////////////////////////////////////////////////////////////

const char	*plJPEG::GetLastError( void )
{
	return ijlErrorStr( sLastErrCode );
}

//// IRead ////////////////////////////////////////////////////////////////////
//	Given an open hsStream (or a filename), reads the JPEG data off of the 
//	stream and decodes it into a new plMipmap. The mipmap's buffer ends up 
//	being a packed RGBx buffer, where x is 8 bits of unused alpha (go figure 
//	that JPEG images can't store alpha, or even if they can, IJL certainly 
//	doesn't know about it).
//	Returns a pointer to the new mipmap if successful, nil otherwise.
//	Note: more or less lifted straight out of the IJL documentation, with
//	some changes to fit Plasma coding style and formats.

plMipmap	*plJPEG::IRead( hsStream *inStream )
{
	MemPushDisableTracking();

	plMipmap	*newMipmap = nil;
	UInt8		*jpegSourceBuffer = nil;
	UInt32		jpegSourceSize;
	
	JPEG_CORE_PROPERTIES	jcProps;


	sLastErrCode = IJL_OK;

	try
	{
		/// Init the IJL library
		sLastErrCode = ijlInit( &jcProps );
		if( sLastErrCode != IJL_OK )
			throw( false );

		/// Read in the JPEG header
		if ( inStream->GetEOF() == 0 )
			throw( false );

		/// Wonderful limitation of mixing our streams with IJL--it wants either a filename
		/// or a memory buffer. Since we can't give it the former, we have to read the entire
		/// JPEG stream into a separate buffer before we can decode it. Which means we ALSO
		/// have to write/read a length of said buffer. Such is life, I guess...
		jpegSourceSize = inStream->ReadSwap32();
		jpegSourceBuffer = TRACKED_NEW UInt8[ jpegSourceSize ];
		if( jpegSourceBuffer == nil )
		{
			sLastErrCode = IJL_MEMORY_ERROR;
			throw( false );
		}

		inStream->Read( jpegSourceSize, jpegSourceBuffer );

		jcProps.JPGFile = nil;
		jcProps.JPGBytes = jpegSourceBuffer;
		jcProps.JPGSizeBytes = jpegSourceSize;

		sLastErrCode = ijlRead( &jcProps, IJL_JBUFF_READPARAMS );
		if( sLastErrCode != IJL_OK )
			throw( false );

		/// So we got lots of data to play with now. First, set the JPEG color
		/// space to read in from/as...
		/// From the IJL code:
		// "Set the JPG color space ... this will always be
		// somewhat of an educated guess at best because JPEG
		// is "color blind" (i.e., nothing in the bit stream
		// tells you what color space the data was encoded from).
		// However, in this example we assume that we are
		// reading JFIF files which means that 3 channel images
		// are in the YCbCr color space and 1 channel images are
		// in the Y color space."

		switch( jcProps.JPGChannels )
		{
			case 1:
				jcProps.JPGColor = IJL_G;
				jcProps.DIBColor = IJL_RGBA_FPX;
				jcProps.DIBChannels = 4;		// We ALWAYS try to decode to 4 channels
				break;

			case 3:
				jcProps.JPGColor = IJL_YCBCR;
				jcProps.DIBColor = IJL_RGBA_FPX;
				jcProps.DIBChannels = 4;
				break;

			default:
				// We should probably assert here, since we're pretty sure we WON'T get ARGB. <sigh>
				hsAssert( false, "Unknown JPEG stream format in ReadFromStream()" );
				jcProps.JPGColor = IJL_OTHER;
				jcProps.DIBColor = IJL_OTHER;
				jcProps.DIBChannels = jcProps.JPGChannels;
				break;
		}
		
		/// Construct a new mipmap to hold everything
		newMipmap = TRACKED_NEW plMipmap( jcProps.JPGWidth, jcProps.JPGHeight, plMipmap::kRGB32Config, 1, plMipmap::kJPEGCompression );
		if( newMipmap == nil || newMipmap->GetImage() == nil )
		{
			sLastErrCode = IJL_MEMORY_ERROR;
			throw( false );
		}

		/// Set up to read in to that buffer we now have
		jcProps.DIBWidth = newMipmap->GetWidth();
		jcProps.DIBHeight = newMipmap->GetHeight();
		jcProps.DIBPadBytes = 0;
		jcProps.DIBBytes = (UInt8 *)newMipmap->GetImage();

		sLastErrCode = ijlRead( &jcProps, IJL_JBUFF_READWHOLEIMAGE );
		if( sLastErrCode != IJL_OK )
			throw( false );

		// Sometimes life just sucks
		ISwapRGBAComponents( (UInt32 *)newMipmap->GetImage(), newMipmap->GetWidth() * newMipmap->GetHeight() );
	}
	catch( ... )
	{
		delete newMipmap;
		newMipmap = nil;
	}

	// Cleanup
	delete [] jpegSourceBuffer;

	// Clean up the IJL Library
	ijlFree( &jcProps );
	MemPopDisableTracking();

	// All done!
	return newMipmap;
}

plMipmap*	plJPEG::ReadFromFile( const char *fileName )
{
	wchar* wFilename = hsStringToWString(fileName);
	plMipmap* retVal = ReadFromFile(wFilename);
	delete [] wFilename;
	return retVal;
}

plMipmap*	plJPEG::ReadFromFile( const wchar *fileName )
{
	// we use a stream because the IJL can't handle unicode
	hsUNIXStream out;
	if (!out.Open(fileName, L"rb"))
		return false;
	plMipmap* ret = IRead(&out);
	out.Close();
	return ret;
}

//// IWrite ///////////////////////////////////////////////////////////////////
//	Oh, figure it out yourself. :P

hsBool	plJPEG::IWrite( plMipmap *source, hsStream *outStream )
{
	hsBool	result = true, swapped = false;
	UInt8	*jpgBuffer = nil;
	UInt32	jpgBufferSize = 0;

	JPEG_CORE_PROPERTIES	jcProps;


	try
	{
		// Init the IJL Library
		sLastErrCode = ijlInit( &jcProps );
		if( sLastErrCode != IJL_OK )
			throw( false );

		// Create a buffer to hold the data
		jpgBufferSize = source->GetWidth() * source->GetHeight() * 3;
		jpgBuffer = TRACKED_NEW UInt8[ jpgBufferSize ];
		if( jpgBuffer == nil )
		{
			sLastErrCode = IJL_MEMORY_ERROR;
			throw( false );
		}

		jcProps.JPGFile = nil;
		jcProps.JPGBytes = jpgBuffer;
		jcProps.JPGSizeBytes = jpgBufferSize;

		jcProps.DIBWidth = source->GetWidth();
		jcProps.DIBHeight = source->GetHeight();
		jcProps.DIBBytes = (UInt8 *)source->GetImage();
		jcProps.DIBPadBytes = 0;
		jcProps.DIBChannels = 4;
		jcProps.DIBColor = IJL_RGB;

		jcProps.JPGWidth = source->GetWidth();
		jcProps.JPGHeight = source->GetHeight();
		jcProps.JPGChannels = 3;
		jcProps.JPGColor = IJL_YCBCR;
		jcProps.JPGSubsampling = IJL_411;	// 4:1:1 subsampling
		jcProps.jquality = fWriteQuality;

		// Sometimes life just sucks
		ISwapRGBAComponents( (UInt32 *)source->GetImage(), source->GetWidth() * source->GetHeight() );
		swapped = true;

		// Write it!
		sLastErrCode = ijlWrite( &jcProps, IJL_JBUFF_WRITEWHOLEIMAGE );
		if( sLastErrCode != IJL_OK )
			throw( false );
		
		// On output, the IJL fills our image buffer with the JPEG stream, plus changes JPGSizeBytes to
		// the length of the stream
		outStream->WriteSwap32( jcProps.JPGSizeBytes );
		outStream->Write( jcProps.JPGSizeBytes, jcProps.JPGBytes );
	}
	catch( ... )
	{
		result = false;
	}

	// Cleanup
	if ( jpgBuffer )
		delete [] jpgBuffer;
	ijlFree( &jcProps );

	if( swapped )
		ISwapRGBAComponents( (UInt32 *)source->GetImage(), source->GetWidth() * source->GetHeight() );

	return result;
}

hsBool	plJPEG::WriteToFile( const char *fileName, plMipmap *sourceData )
{
	wchar* wFilename = hsStringToWString(fileName);
	hsBool retVal = WriteToFile(wFilename, sourceData);
	delete [] wFilename;
	return retVal;
}

hsBool	plJPEG::WriteToFile( const wchar *fileName, plMipmap *sourceData )
{
	// we use a stream because the IJL can't handle unicode
	hsUNIXStream out;
	if (!out.Open(fileName, L"wb"))
		return false;
	hsBool ret = IWrite(sourceData, &out);
	out.Close();
	return ret;
}

//// ISwapRGBAComponents //////////////////////////////////////////////////////

void	plJPEG::ISwapRGBAComponents( UInt32 *data, UInt32 count )
{
	while( count-- )
	{
		*data = ( ( ( *data ) & 0xff00ff00 )       ) |
				( ( ( *data ) & 0x00ff0000 ) >> 16 ) |
//				( ( ( *data ) & 0x0000ff00 ) << 8 ) |
				( ( ( *data ) & 0x000000ff ) << 16 );
		data++;
	}
}

