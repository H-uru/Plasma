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

#include "hsTypes.h"
#include "hsStream.h"
#include "hsExceptions.h"
#include "hsUtils.h"
#include "plPNG.h"
#include "plGImage/plMipmap.h"

#include <png.h>
#define PNGSIGSIZE 8

//  Custom functions to read and write data from or to an hsStream
//  used by libPNG's respective functions
void pngReadDelegate(png_structp png_ptr, png_bytep png_data, png_size_t length)
{
    hsStream *inStream = (hsStream *)png_get_io_ptr(png_ptr);
    inStream->Read(length, (UInt8 *)png_data);
}

void pngWriteDelegate(png_structp png_ptr, png_bytep png_data, png_size_t length)
{
}

void pngFlushDelegate(png_structp png_ptr)
{
}

//// Singleton Instance ///////////////////////////////////////////////////////

plPNG &plPNG::Instance( void )
{
    static plPNG theInstance;
    return theInstance;
}

//// IRead ////////////////////////////////////////////////////////////////////
//  Given an open hsStream, reads the PNG data off of the
//  stream and decodes it into a new plMipmap. The mipmap's buffer ends up
//  being a packed RGBA buffer.
//  Returns a pointer to the new mipmap if successful, NULL otherwise.

plMipmap* plPNG::IRead( hsStream *inStream )
{
    plMipmap *newMipmap = NULL;
    png_structp png_ptr;
    png_infop info_ptr;
    png_infop end_info;

    try
    {
        //  Check PNG Signature
        png_byte sig[PNGSIGSIZE];
        inStream->Read8Bytes((char *) sig);
        if (!png_sig_cmp(sig, 0, PNGSIGSIZE))
        {
            //  Allocate required structs
            png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
            if (!png_ptr)
               throw( false );

            info_ptr = png_create_info_struct(png_ptr);
            if (!info_ptr)
            {
               png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
               throw( false );
            }

            end_info = png_create_info_struct(png_ptr);
            if (!end_info)
            {
               png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
               throw( false );
            }

            //  Assign delegate function for reading from hsStream
            png_set_read_fn(png_ptr, (png_voidp)inStream, pngReadDelegate);

            //  Get PNG Header information
            png_set_sig_bytes(png_ptr, PNGSIGSIZE);
            png_read_info(png_ptr, info_ptr);
            png_uint_32 imgWidth =  png_get_image_width(png_ptr, info_ptr);
            png_uint_32 imgHeight = png_get_image_height(png_ptr, info_ptr);
            png_uint_32 bitdepth   = png_get_bit_depth(png_ptr, info_ptr);
            png_uint_32 channels   = png_get_channels(png_ptr, info_ptr);
            png_uint_32 color_type = png_get_color_type(png_ptr, info_ptr);

            //  Convert images to RGB color space
            switch (color_type) {
                case PNG_COLOR_TYPE_PALETTE:
                    png_set_palette_to_rgb(png_ptr);
                    channels = 3;
                    break;
                case PNG_COLOR_TYPE_GRAY:
                    if (bitdepth < 8)
                        png_set_expand_gray_1_2_4_to_8(png_ptr);
                    bitdepth = 8;
                break;
            }

            //  Convert transparency (if needed) to a full alpha channel
            if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
            {
                png_set_tRNS_to_alpha(png_ptr);
                channels+=1;
            }

            png_set_bgr(png_ptr);

            /// Construct a new mipmap to hold everything
            newMipmap = TRACKED_NEW plMipmap(imgWidth, imgHeight, plMipmap::kARGB32Config, 1, plMipmap::kUncompressed);

            char *destp = (char *)newMipmap->GetImage();
            png_bytep *row_ptrs = new png_bytep[imgHeight];
            const unsigned int stride = imgWidth * bitdepth * channels / 8;

            //  Assign row pointers to the appropriate locations in the newly-created Mipmap
            for (size_t i = 0; i < imgHeight; i++)
            {
                row_ptrs[i] = (png_bytep)destp + (i * stride);
            }
            png_read_image(png_ptr, row_ptrs);

            //  Clean up allocated structs
            png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
            delete [] row_ptrs;
        }
    }
    catch( ... )
    {
        if (newMipmap != NULL)
        {
            delete newMipmap;
            newMipmap = NULL;
        }
    }

    return newMipmap;
}

plMipmap* plPNG::ReadFromFile( const char *fileName )
{
    wchar* wFilename = hsStringToWString(fileName);
    plMipmap* retVal = ReadFromFile(wFilename);
    delete [] wFilename;
    return retVal;
}

plMipmap* plPNG::ReadFromFile( const wchar *fileName )
{
    hsUNIXStream in;
    if (!in.Open(fileName, L"rb"))
        return false;
    plMipmap* ret = IRead(&in);
    in.Close();
    return ret;
}

hsBool plPNG::IWrite( plMipmap *source, hsStream *outStream )
{
    hsBool result = true;

    try
    {
        //png_set_write_fn(png_ptr, (png_voidp)&outStream, pngWriteDelegate, pngFlushDelegate);
    }
    catch( ... )
    {
        result = false;
    }

    return result;
}

hsBool plPNG::WriteToFile( const char *fileName, plMipmap *sourceData )
{
    wchar* wFilename = hsStringToWString(fileName);
    hsBool retVal = WriteToFile(wFilename, sourceData);
    delete [] wFilename;
    return retVal;
}

hsBool plPNG::WriteToFile( const wchar *fileName, plMipmap *sourceData )
{
    hsUNIXStream out;
    if (!out.Open(fileName, L"wb"))
        return false;
    hsBool ret = IWrite(sourceData, &out);
    out.Close();
    return ret;
}
