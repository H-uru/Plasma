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

#ifdef IJL_SDK_AVAILABLE
#ifndef HS_BUILD_FOR_WIN32
#error Currently the JPEG libraries don't build for anything but Win32. If you're building this on a non-Win32 platform....WHY??
#endif

#include "../../../../../StaticSDKs/Win32/IJL/include/ijl.h"
#else
#include "jpeglib.h"
#include "jerror.h"
#endif

//// Local Statics ////////////////////////////////////////////////////////////
//	Done this way so we don't have to declare them in the .h file and pull in
//	the platform-specific library

#ifdef IJL_SDK_AVAILABLE
static IJLERR	sLastErrCode = IJL_OK;
#else
// It is not thread-safe to use a static char buffer, but it wasn't really
// thread-safe to use a static int either. The char buffer is slightly
// worse since you could get mangled strings instead of just a stale error.
static char jpegmsg[JMSG_LENGTH_MAX];

// jpeglib error handlers
static void plJPEG_error_exit( j_common_ptr cinfo )
{
	(*cinfo->err->format_message) ( cinfo, jpegmsg );
	throw ( false );
}
static void plJPEG_emit_message( j_common_ptr cinfo, int msg_level )
{
	// log NOTHING
}
static void clear_jpegmsg()
{
	// "Success" is what IJL produced for no error
	strcpy( jpegmsg, "Success" );
}
#endif


//// Instance /////////////////////////////////////////////////////////////////

plJPEG	&plJPEG::Instance( void )
{
#ifndef IJL_SDK_AVAILABLE
	clear_jpegmsg();
#endif
	static plJPEG	theInstance;
	return theInstance;
}

//// GetLastError /////////////////////////////////////////////////////////////

const char	*plJPEG::GetLastError( void )
{
#ifdef IJL_SDK_AVAILABLE
	return ijlErrorStr( sLastErrCode );
#else
	return jpegmsg;
#endif
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
	
#ifdef IJL_SDK_AVAILABLE
	JPEG_CORE_PROPERTIES	jcProps;
#else
	struct jpeg_decompress_struct	cinfo;
	struct jpeg_error_mgr		jerr;
#endif


#ifdef IJL_SDK_AVAILABLE
	sLastErrCode = IJL_OK;
#else
	clear_jpegmsg();
	cinfo.err = jpeg_std_error( &jerr );
	jerr.error_exit = plJPEG_error_exit;
	jerr.emit_message = plJPEG_emit_message;
#endif

	try
	{
#ifdef IJL_SDK_AVAILABLE
		/// Init the IJL library
		sLastErrCode = ijlInit( &jcProps );
		if( sLastErrCode != IJL_OK )
			throw( false );
#else
		jpeg_create_decompress( &cinfo );
#endif

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
#ifdef IJL_SDK_AVAILABLE
			sLastErrCode = IJL_MEMORY_ERROR;
			throw( false );
#else
			// waah.
			ERREXIT1( &cinfo, JERR_OUT_OF_MEMORY, 0 );
#endif
		}

		inStream->Read( jpegSourceSize, jpegSourceBuffer );

#ifdef IJL_SDK_AVAILABLE
		jcProps.JPGFile = nil;
		jcProps.JPGBytes = jpegSourceBuffer;
		jcProps.JPGSizeBytes = jpegSourceSize;
#else
		jpeg_mem_src( &cinfo, jpegSourceBuffer, jpegSourceSize );
#endif

#ifdef IJL_SDK_AVAILABLE
		sLastErrCode = ijlRead( &jcProps, IJL_JBUFF_READPARAMS );
		if( sLastErrCode != IJL_OK )
			throw( false );
#else
		(void) jpeg_read_header( &cinfo, TRUE );
#endif

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

#ifdef IJL_SDK_AVAILABLE
		switch( jcProps.JPGChannels )
#else
		switch( cinfo.jpeg_color_space )
#endif
		{
#ifdef IJL_SDK_AVAILABLE
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
#else
			case JCS_GRAYSCALE:
			case JCS_YCbCr:
				cinfo.out_color_space = JCS_RGBA;
				break;
#endif

			default:
				// We should probably assert here, since we're pretty sure we WON'T get ARGB. <sigh>
				hsAssert( false, "Unknown JPEG stream format in ReadFromStream()" );
#ifdef IJL_SDK_AVAILABLE
				jcProps.JPGColor = IJL_OTHER;
				jcProps.DIBColor = IJL_OTHER;
				jcProps.DIBChannels = jcProps.JPGChannels;
#else
				cinfo.out_color_space = JCS_UNKNOWN;
#endif
				break;
		}
		
#ifndef IJL_SDK_AVAILABLE
		(void) jpeg_start_decompress( &cinfo );
#endif

		/// Construct a new mipmap to hold everything
#ifdef IJL_SDK_AVAILABLE
		newMipmap = TRACKED_NEW plMipmap( jcProps.JPGWidth, jcProps.JPGHeight, plMipmap::kRGB32Config, 1, plMipmap::kJPEGCompression );
#else
		newMipmap = TRACKED_NEW plMipmap( cinfo.output_width, cinfo.output_height, plMipmap::kRGB32Config, 1, plMipmap::kJPEGCompression );
#endif
		if( newMipmap == nil || newMipmap->GetImage() == nil )
		{
#ifdef IJL_SDK_AVAILABLE
			sLastErrCode = IJL_MEMORY_ERROR;
			throw( false );
#else
			ERREXIT1( &cinfo, JERR_OUT_OF_MEMORY, 0 );
#endif
		}

		/// Set up to read in to that buffer we now have
#ifdef IJL_SDK_AVAILABLE
		jcProps.DIBWidth = newMipmap->GetWidth();
		jcProps.DIBHeight = newMipmap->GetHeight();
		jcProps.DIBPadBytes = 0;
		jcProps.DIBBytes = (UInt8 *)newMipmap->GetImage();

		sLastErrCode = ijlRead( &jcProps, IJL_JBUFF_READWHOLEIMAGE );
		if( sLastErrCode != IJL_OK )
			throw( false );
#else
		while( cinfo.output_scanline < cinfo.output_height )
		{
			UInt8 *startp = newMipmap->GetAddr8( 0, cinfo.output_scanline );
			(void) jpeg_read_scanlines( &cinfo, &startp, 1 );
		}
#endif

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
#ifdef IJL_SDK_AVAILABLE
	ijlFree( &jcProps );
#else
	jpeg_destroy_decompress( &cinfo );
#endif
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

#ifdef IJL_SDK_AVAILABLE
	JPEG_CORE_PROPERTIES	jcProps;
#else
	struct jpeg_compress_struct	cinfo;
	struct jpeg_error_mgr		jerr;
#endif


#ifndef IJL_SDK_AVAILABLE
	clear_jpegmsg();
	cinfo.err = jpeg_std_error( &jerr );
	jerr.error_exit = plJPEG_error_exit;
	jerr.emit_message = plJPEG_emit_message;
#endif

	try
	{
#ifdef IJL_SDK_AVAILABLE
		// Init the IJL Library
		sLastErrCode = ijlInit( &jcProps );
		if( sLastErrCode != IJL_OK )
			throw( false );
#else
		jpeg_create_compress( &cinfo );
#endif

		// Create a buffer to hold the data
		jpgBufferSize = source->GetWidth() * source->GetHeight() * 3;
		jpgBuffer = TRACKED_NEW UInt8[ jpgBufferSize ];
		if( jpgBuffer == nil )
		{
#ifdef IJL_SDK_AVAILABLE
			sLastErrCode = IJL_MEMORY_ERROR;
			throw( false );
#else
			ERREXIT1( &cinfo, JERR_OUT_OF_MEMORY, 0 );
#endif
		}

#ifdef IJL_SDK_AVAILABLE
		jcProps.JPGFile = nil;
		jcProps.JPGBytes = jpgBuffer;
		jcProps.JPGSizeBytes = jpgBufferSize;
#else
		UInt8 *bufferAddr = jpgBuffer;
		UInt32 bufferSize = jpgBufferSize;
		jpeg_mem_dest( &cinfo, &bufferAddr, &bufferSize );
#endif

#ifdef IJL_SDK_AVAILABLE
		jcProps.DIBWidth = source->GetWidth();
		jcProps.DIBHeight = source->GetHeight();
		jcProps.DIBBytes = (UInt8 *)source->GetImage();
		jcProps.DIBPadBytes = 0;
		jcProps.DIBChannels = 4;
		jcProps.DIBColor = IJL_RGB;
#else
		cinfo.image_width = source->GetWidth();
		cinfo.image_height = source->GetHeight();
		cinfo.input_components = 4;
		cinfo.in_color_space = JCS_RGBA;

		jpeg_set_defaults( &cinfo );
#endif

#ifdef IJL_SDK_AVAILABLE
		jcProps.JPGWidth = source->GetWidth();
		jcProps.JPGHeight = source->GetHeight();
		jcProps.JPGChannels = 3;
		jcProps.JPGColor = IJL_YCBCR;
		jcProps.JPGSubsampling = IJL_411;	// 4:1:1 subsampling
		jcProps.jquality = fWriteQuality;
#else
		cinfo.jpeg_width = source->GetWidth(); // default
		cinfo.jpeg_width = source->GetHeight(); // default
		cinfo.jpeg_color_space = JCS_YCbCr; // default
		// not sure how to set 4:1:1 but supposedly it's the default
		jpeg_set_quality( &cinfo, fWriteQuality, TRUE );

		jpeg_start_compress( &cinfo, TRUE );
#endif

		// Sometimes life just sucks
		ISwapRGBAComponents( (UInt32 *)source->GetImage(), source->GetWidth() * source->GetHeight() );
		swapped = true;

		// Write it!
#ifdef IJL_SDK_AVAILABLE
		sLastErrCode = ijlWrite( &jcProps, IJL_JBUFF_WRITEWHOLEIMAGE );
		if( sLastErrCode != IJL_OK )
			throw( false );
#else
		while( cinfo.next_scanline < cinfo.image_height )
		{
			UInt8 *startp = source->GetAddr8( 0, cinfo.next_scanline );
			(void) jpeg_write_scanlines( &cinfo, &startp, 1 );
		}
		jpeg_finish_compress( &cinfo );
#endif
		
#ifdef IJL_SDK_AVAILABLE
		// On output, the IJL fills our image buffer with the JPEG stream, plus changes JPGSizeBytes to
		// the length of the stream
		outStream->WriteSwap32( jcProps.JPGSizeBytes );
		outStream->Write( jcProps.JPGSizeBytes, jcProps.JPGBytes );
#else
		// jpeglib similarly changes bufferSize and bufferAddr
		outStream->WriteSwap32( bufferSize );
		outStream->Write( bufferSize, bufferAddr );
#endif
	}
	catch( ... )
	{
		result = false;
	}

	// Cleanup
	if ( jpgBuffer )
		delete [] jpgBuffer;
#ifdef IJL_SDK_AVAILABLE
	ijlFree( &jcProps );
#else
	jpeg_destroy_compress( &cinfo );
#endif

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

