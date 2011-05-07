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


plMipmap *plPNG::IRead( hsStream *inStream )
{
    plMipmap *newMipmap = NULL;
    png_structp png_ptr;
    png_infop info_ptr;
    png_infop end_info;

    try
    {
        //  Check PNG Signature
        png_byte sig[8];
        inStream->Read8Bytes((char *) sig);
        if (!png_sig_cmp(sig, 0, 8))
        {
            inStream->Rewind();

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



            //  Clean up allocated structs
            png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
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

hsBool  plPNG::IWrite( plMipmap *source, hsStream *outStream )
{
    hsBool result = true;

    try
    {
    }
    catch( ... )
    {
        result = false;
    }

    return result;
}

hsBool  plPNG::WriteToFile( const char *fileName, plMipmap *sourceData )
{
    wchar* wFilename = hsStringToWString(fileName);
    hsBool retVal = WriteToFile(wFilename, sourceData);
    delete [] wFilename;
    return retVal;
}

hsBool  plPNG::WriteToFile( const wchar *fileName, plMipmap *sourceData )
{
    hsUNIXStream out;
    if (!out.Open(fileName, L"wb"))
        return false;
    hsBool ret = IWrite(sourceData, &out);
    out.Close();
    return ret;
}