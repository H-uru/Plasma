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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  plJPEG - JPEG Codec Wrapper for Plasma                                   //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  2.1.2002 mcn - Created.                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "plJPEG.h"
#include "hsStream.h"
#include "hsExceptions.h"

#include "plMipmap.h"

#include <jpeglib.h>
#include <jerror.h>

//// Local Statics ////////////////////////////////////////////////////////////
//  Done this way so we don't have to declare them in the .h file and pull in
//  the platform-specific library

// It is not thread-safe to use a static char buffer, but it wasn't really
// thread-safe to use a static int either. The char buffer is slightly
// worse since you could get mangled strings instead of just a stale error.
static char jpegmsg[JMSG_LENGTH_MAX];

// jpeglib error handlers
static void plJPEG_error_exit( j_common_ptr cinfo )
{
    (*cinfo->err->format_message) ( cinfo, jpegmsg );
    throw false;
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


//// Instance /////////////////////////////////////////////////////////////////

plJPEG  &plJPEG::Instance()
{
    clear_jpegmsg();

    static plJPEG   theInstance;
    return theInstance;
}

//// GetLastError /////////////////////////////////////////////////////////////

const char  *plJPEG::GetLastError()
{
    return jpegmsg;
}

//// IRead ////////////////////////////////////////////////////////////////////
//  Given an open hsStream (or a filename), reads the JPEG data off of the 
//  stream and decodes it into a new plMipmap. The mipmap's buffer ends up 
//  being a packed RGBx buffer, where x is 8 bits of unused alpha (go figure 
//  that JPEG images can't store alpha, or even if they can, IJL certainly 
//  doesn't know about it).
//  Returns a pointer to the new mipmap if successful, nullptr otherwise.
//  Note: more or less lifted straight out of the IJL documentation, with
//  some changes to fit Plasma coding style and formats.

plMipmap    *plJPEG::IRead( hsStream *inStream )
{
    plMipmap    *newMipmap = nullptr;
    uint8_t       *jpegSourceBuffer = nullptr;
    uint32_t      jpegSourceSize;
    
    struct jpeg_decompress_struct   cinfo;
    struct jpeg_error_mgr       jerr;


    clear_jpegmsg();
    cinfo.err = jpeg_std_error( &jerr );
    jerr.error_exit = plJPEG_error_exit;
    jerr.emit_message = plJPEG_emit_message;

    try
    {
        jpeg_create_decompress( &cinfo );

        /// Read in the JPEG header
        if ( inStream->GetEOF() == 0 )
            throw false;

        /// Wonderful limitation of mixing our streams with IJL--it wants either a filename
        /// or a memory buffer. Since we can't give it the former, we have to read the entire
        /// JPEG stream into a separate buffer before we can decode it. Which means we ALSO
        /// have to write/read a length of said buffer. Such is life, I guess...
        jpegSourceSize = inStream->ReadLE32();
        jpegSourceBuffer = new uint8_t[ jpegSourceSize ];

        inStream->Read( jpegSourceSize, jpegSourceBuffer );
        jpeg_mem_src( &cinfo, jpegSourceBuffer, jpegSourceSize );
        (void) jpeg_read_header( &cinfo, TRUE );

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

        switch( cinfo.jpeg_color_space )
        {
            case JCS_GRAYSCALE:
            case JCS_YCbCr:
                cinfo.out_color_space = JCS_RGB;
                break;

            default:
                // We should probably assert here, since we're pretty sure we WON'T get ARGB. <sigh>
                hsAssert( false, "Unknown JPEG stream format in ReadFromStream()" );
                cinfo.out_color_space = JCS_UNKNOWN;
                break;
        }

        (void) jpeg_start_decompress( &cinfo );

        /// Construct a new mipmap to hold everything
        newMipmap = new plMipmap( cinfo.output_width, cinfo.output_height, plMipmap::kRGB32Config, 1, plMipmap::kJPEGCompression );

        /// Set up to read in to that buffer we now have
        JSAMPROW jbuffer;
        int row_stride = cinfo.output_width * cinfo.output_components;
        int out_stride = cinfo.output_width * 4;  // Decompress to RGBA
        jbuffer = new JSAMPLE[row_stride];

        uint8_t *destp = (uint8_t *)newMipmap->GetImage();
        while( cinfo.output_scanline < cinfo.output_height )
        {
            (void) jpeg_read_scanlines( &cinfo, &jbuffer, 1 );
            (void) memset( destp, 0xFF, out_stride );
            
            for( size_t pixel = 0; pixel < cinfo.output_width; ++pixel )
            {
                (void) memcpy( destp + (pixel * 4),
                               jbuffer + (pixel * cinfo.output_components),
                               cinfo.out_color_components );
            }

            destp += out_stride;
        }

        (void) jpeg_finish_decompress(&cinfo);
        delete [] jbuffer;

        // Sometimes life just sucks
        ISwapRGBAComponents( (uint32_t *)newMipmap->GetImage(), newMipmap->GetWidth() * newMipmap->GetHeight() );
    }
    catch (...)
    {
        delete newMipmap;
        newMipmap = nullptr;
    }

    // Cleanup
    delete [] jpegSourceBuffer;

    // Clean up the JPEG Library
    jpeg_destroy_decompress( &cinfo );

    // All done!
    return newMipmap;
}

plMipmap*   plJPEG::ReadFromFile( const plFileName &fileName )
{
    // we use a stream because the IJL can't handle unicode
    hsRAMStream tempstream;
    hsUNIXStream in;
    if (!in.Open(fileName, "rb"))
        return nullptr;

    // The stream reader for JPEGs expects a 32-bit size at the start,
    // so insert that into the stream before passing it on
    in.FastFwd();
    uint32_t fsize = in.GetPosition();
    uint8_t *tempbuffer = new uint8_t[fsize];
    in.Rewind();
    in.Read(fsize, tempbuffer);
    tempstream.WriteLE32(fsize);
    tempstream.Write(fsize, tempbuffer);
    delete [] tempbuffer;
    tempstream.Rewind();

    plMipmap* ret = IRead(&tempstream);
    in.Close();
    return ret;
}

//// IWrite ///////////////////////////////////////////////////////////////////
//  Oh, figure it out yourself. :P

bool    plJPEG::IWrite( plMipmap *source, hsStream *outStream )
{
    bool    result = true, swapped = false;
    uint8_t   *jpgBuffer = nullptr;
    uint32_t  jpgBufferSize = 0;

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr       jerr;


    clear_jpegmsg();
    cinfo.err = jpeg_std_error( &jerr );
    jerr.error_exit = plJPEG_error_exit;
    jerr.emit_message = plJPEG_emit_message;

    try
    {
        jpeg_create_compress( &cinfo );

        // Create a buffer to hold the data
        jpgBufferSize = source->GetWidth() * source->GetHeight() * 3;
        jpgBuffer = new uint8_t[ jpgBufferSize ];

        uint8_t *bufferAddr = jpgBuffer;
        unsigned long bufferSize = jpgBufferSize;
        jpeg_mem_dest( &cinfo, &bufferAddr, &bufferSize );

        cinfo.image_width = source->GetWidth();
        cinfo.image_height = source->GetHeight();
        cinfo.input_components = 3;
        cinfo.in_color_space = JCS_RGB;

        jpeg_set_defaults( &cinfo );

#if JPEG_LIB_VERSION >= 70
        cinfo.jpeg_width = source->GetWidth(); // default
        cinfo.jpeg_height = source->GetHeight(); // default
#endif
        cinfo.jpeg_color_space = JCS_YCbCr; // default
        // not sure how to set 4:1:1 but supposedly it's the default
        jpeg_set_quality( &cinfo, fWriteQuality, TRUE );

        jpeg_start_compress( &cinfo, TRUE );

        // Sometimes life just sucks
        ISwapRGBAComponents( (uint32_t *)source->GetImage(), source->GetWidth() * source->GetHeight() );
        swapped = true;

        // Write it!
        JSAMPROW jbuffer;
        int in_stride = cinfo.image_width * 4;  // Input is always RGBA
        int row_stride = cinfo.image_width * cinfo.input_components;
        jbuffer = new JSAMPLE[row_stride];

        uint8_t *srcp = (uint8_t *)source->GetImage();
        while( cinfo.next_scanline < cinfo.image_height )
        {
            for( size_t pixel = 0; pixel < cinfo.image_width; ++pixel )
            {
                (void) memcpy( jbuffer + (pixel * cinfo.input_components),
                               srcp + (pixel * 4), cinfo.input_components );
            }

            (void) jpeg_write_scanlines( &cinfo, &jbuffer, 1 );
            srcp += in_stride;
        }

        jpeg_finish_compress( &cinfo );
        delete [] jbuffer;

        // jpeglib changes bufferSize and bufferAddr
        outStream->WriteLE32((uint32_t)bufferSize);
        outStream->Write( bufferSize, bufferAddr );
    }
    catch (...)
    {
        result = false;
    }

    // Cleanup
    if ( jpgBuffer )
        delete [] jpgBuffer;
    jpeg_destroy_compress( &cinfo );

    if( swapped )
        ISwapRGBAComponents( (uint32_t *)source->GetImage(), source->GetWidth() * source->GetHeight() );

    return result;
}

bool    plJPEG::WriteToFile( const plFileName &fileName, plMipmap *sourceData )
{
    // we use a stream because the IJL can't handle unicode
    hsRAMStream tempstream;
    hsUNIXStream out;
    if (!out.Open(fileName, "wb"))
        return false;
    bool ret = IWrite(sourceData, &tempstream);
    if (ret)
    {
        // The stream writer for JPEGs prepends a 32-bit size,
        // so remove that from the stream before saving to a file
        tempstream.Rewind();
        uint32_t fsize = tempstream.ReadLE32();
        uint8_t *tempbuffer = new uint8_t[fsize];
        tempstream.Read(fsize, tempbuffer);
        out.Write(fsize, tempbuffer);

        delete [] tempbuffer;
    }
    out.Close();
    return ret;
}

//// ISwapRGBAComponents //////////////////////////////////////////////////////

void    plJPEG::ISwapRGBAComponents( uint32_t *data, uint32_t count )
{
    while( count-- )
    {
        *data = ( ( ( *data ) & 0xff00ff00 )       ) |
                ( ( ( *data ) & 0x00ff0000 ) >> 16 ) |
//              ( ( ( *data ) & 0x0000ff00 ) << 8 ) |
                ( ( ( *data ) & 0x000000ff ) << 16 );
        data++;
    }
}

