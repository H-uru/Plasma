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

#include "HeadSpin.h"
#include "hsStream.h"
#include "hsExceptions.h"

#include "plPNG.h"
#include "plGImage/plMipmap.h"

#include <png.h>
#define PNGSIGSIZE 8

//  Custom functions to read and write data from or to an hsStream
//  used by libPNG's respective functions
void pngReadDelegate(png_structp png_ptr, png_bytep png_data, png_size_t length)
{
    hsStream* inStream = (hsStream*)png_get_io_ptr(png_ptr);
    inStream->Read(length, (uint8_t*)png_data);
}

void pngWriteDelegate(png_structp png_ptr, png_bytep png_data, png_size_t length)
{
    hsStream* outStream = (hsStream*)png_get_io_ptr(png_ptr);
    outStream->Write(length, (uint8_t*)png_data);
}

//// Singleton Instance ///////////////////////////////////////////////////////

plPNG& plPNG::Instance(void)
{
    static plPNG theInstance;
    return theInstance;
}

//// IRead ////////////////////////////////////////////////////////////////////
//  Given an open hsStream, reads the PNG data off of the
//  stream and decodes it into a new plMipmap. The mipmap's buffer ends up
//  being a packed RGBA buffer.
//  Returns a pointer to the new mipmap if successful, NULL otherwise.

plMipmap* plPNG::IRead(hsStream* inStream)
{
    plMipmap* newMipmap = NULL;
    png_structp png_ptr;
    png_infop info_ptr;
    png_infop end_info;

    try {
        //  Check PNG Signature
        png_byte sig[PNGSIGSIZE];
        inStream->Read8Bytes((char*) sig);

        if (!png_sig_cmp(sig, 0, PNGSIGSIZE)) {
            //  Allocate required structs
            png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

            if (!png_ptr) {
                throw false;
            }

            info_ptr = png_create_info_struct(png_ptr);

            if (!info_ptr) {
                png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
                throw false;
            }

            end_info = png_create_info_struct(png_ptr);

            if (!end_info) {
                png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
                throw false;
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

                    if (bitdepth < 8) {
                        png_set_expand_gray_1_2_4_to_8(png_ptr);
                    }

                    bitdepth = 8;
                    break;
            }

            //  Convert transparency (if needed) to a full alpha channel
            if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
                png_set_tRNS_to_alpha(png_ptr);
                channels += 1;
            } else if (channels == 3) {
                // Add an opaque alpha channel if still none exists
                png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
                channels = 4;
            }

            // Invert color byte-order as used by plMipmap for DirectX
            png_set_bgr(png_ptr);
            /// Construct a new mipmap to hold everything
            newMipmap = new plMipmap(imgWidth, imgHeight, plMipmap::kARGB32Config, 1, plMipmap::kUncompressed);
            char* destp = (char*)newMipmap->GetImage();
            png_bytep* row_ptrs = new png_bytep[imgHeight];
            const unsigned int stride = imgWidth * bitdepth * channels / 8;

            //  Assign row pointers to the appropriate locations in the newly-created Mipmap
            for (size_t i = 0; i < imgHeight; i++) {
                row_ptrs[i] = (png_bytep)destp + (i * stride);
            }

            png_read_image(png_ptr, row_ptrs);
            png_read_end(png_ptr, end_info);
            //  Clean up allocated structs
            png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
            delete [] row_ptrs;
        }
    } catch (...) {
        delete newMipmap;
        newMipmap = nullptr;
    }

    return newMipmap;
}

plMipmap* plPNG::ReadFromFile(const plFileName& fileName)
{
    hsUNIXStream in;

    if (!in.Open(fileName, "rb")) {
        return nil;
    }

    plMipmap* ret = IRead(&in);
    in.Close();
    return ret;
}

bool plPNG::IWrite(plMipmap* source, hsStream* outStream)
{
    bool result = true;

    try {
        //  Allocate required structs
        png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

        if (!png_ptr) {
            throw false;
        }

        png_infop info_ptr = png_create_info_struct(png_ptr);

        if (!info_ptr) {
            png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
            throw false;
        }

        //  Assign delegate function for writing to hsStream
        png_set_write_fn(png_ptr, (png_voidp)outStream, pngWriteDelegate, NULL);
        uint8_t psize = source->GetPixelSize();
        png_set_IHDR(png_ptr, info_ptr, source->GetWidth(), source->GetHeight(), 8, PNG_COLOR_TYPE_RGB_ALPHA,
                     PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
        // Invert color byte-order as used by plMipmap for DirectX
        png_set_bgr(png_ptr);
        // Write out the image metadata
        png_write_info(png_ptr, info_ptr);
        char* srcp = (char*)source->GetImage();
        png_bytep* row_ptrs = new png_bytep[source->GetHeight()];
        const unsigned int stride = source->GetWidth() * source->GetPixelSize() / 8;

        //  Assign row pointers to the appropriate locations in the newly-created Mipmap
        for (size_t i = 0; i < source->GetHeight(); i++) {
            row_ptrs[i] = (png_bytep)srcp + (i * stride);
        }

        png_write_image(png_ptr, row_ptrs);
        png_write_end(png_ptr, info_ptr);
        //  Clean up allocated structs
        png_destroy_write_struct(&png_ptr, &info_ptr);
        delete [] row_ptrs;
    } catch (...) {
        result = false;
    }

    return result;
}

bool plPNG::WriteToFile(const plFileName& fileName, plMipmap* sourceData)
{
    hsUNIXStream out;

    if (!out.Open(fileName, "wb")) {
        return false;
    }

    bool ret = IWrite(sourceData, &out);
    out.Close();
    return ret;
}
