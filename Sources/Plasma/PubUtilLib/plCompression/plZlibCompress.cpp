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
#include "plZlibCompress.h"
#include "zlib.h"
#include "hsStream.h"

#include <memory>

bool plZlibCompress::Uncompress(uint8_t* bufOut, uint32_t* bufLenOut, const uint8_t* bufIn, uint32_t bufLenIn)
{
    unsigned long buflen_out = *bufLenOut;
    bool result = (uncompress(bufOut, &buflen_out, bufIn, bufLenIn) == Z_OK);
    *bufLenOut = buflen_out;
    return result;
}

bool plZlibCompress::Compress(uint8_t* bufOut, uint32_t* bufLenOut, const uint8_t* bufIn, uint32_t bufLenIn)
{
    // according to compress doc, the bufOut buffer should be at least .1% larger than source buffer, plus 12 bytes.
    hsAssert(*bufLenOut>=(int)(bufLenIn*1.1+12), "bufOut compress buffer is not large enough");
    unsigned long buflen_out = *bufLenOut;
    bool result = (compress(bufOut, &buflen_out, bufIn, bufLenIn) == Z_OK);
    *bufLenOut = buflen_out;
    return result;
}

//
// copy bufOut to bufIn, set bufLenIn=bufLenOut
//
bool plZlibCompress::ICopyBuffers(uint8_t** bufIn, uint32_t* bufLenIn, char* bufOut, uint32_t bufLenOut, int offset, bool ok)
{
    if (ok)
    {
        *bufLenIn = bufLenOut+offset;
        uint8_t* newBuf = new uint8_t[*bufLenIn];   // alloc new buffer
        memmove(newBuf, *bufIn, offset);            // copy offset (uncompressed) part
        delete [] *bufIn;                           // delete original buffer

        memmove(newBuf+offset, bufOut, bufLenOut);  // copy compressed part
        delete [] bufOut;
        *bufIn = newBuf;
        return true;
    }
    delete [] bufOut;
    return false;
}

//
// In place version
// offset is how much to skip over when compressing
//
bool plZlibCompress::Compress(uint8_t** bufIn, uint32_t* bufLenIn, int offset)
{
    uint32_t adjBufLenIn = *bufLenIn - offset;
    uint8_t* adjBufIn = *bufIn + offset;

    // according to compress doc, the bufOut buffer should be at least .1% larger than source buffer, plus 12 bytes.
    uint32_t bufLenOut = (int)(adjBufLenIn*1.1+12);
    char* bufOut = new char[bufLenOut];
    
    bool ok=(Compress((uint8_t*)bufOut, &bufLenOut, (uint8_t*)adjBufIn, adjBufLenIn) && 
        bufLenOut < adjBufLenIn);
    return ICopyBuffers(bufIn, bufLenIn, bufOut, bufLenOut, offset, ok);
}

//
// In place version
//
bool plZlibCompress::Uncompress(uint8_t** bufIn, uint32_t* bufLenIn, uint32_t bufLenOut, int offset)
{
    uint32_t adjBufLenIn = *bufLenIn - offset;
    uint8_t* adjBufIn = *bufIn + offset;

    char* bufOut = new char[bufLenOut];
    
    bool ok=Uncompress((uint8_t*)bufOut, &bufLenOut, (uint8_t*)adjBufIn, adjBufLenIn) ? true : false;
    return ICopyBuffers(bufIn, bufLenIn, bufOut, bufLenOut, offset, ok);
}

//// .gz File Versions ///////////////////////////////////////////////////////

constexpr size_t kGzBufferSize = 64 * 1024; // 64 KiB
static_assert(UINT32_MAX >= kGzBufferSize, "GZ Buffer size should be 32-bits");

#if 1

bool  plZlibCompress::UncompressFile( const char *compressedPath, const char *destPath )
{
    gzFile  inFile;
    FILE    *outFile;
    bool    worked = false;
    int     length, err;

    auto buffer = std::make_unique<uint8_t[]>(kGzBufferSize);

    outFile = fopen( destPath, "wb" );
    if (outFile != nullptr)
    {
        inFile = gzopen( compressedPath, "rb" );
        if (inFile != nullptr)
        {
            for( ;; )
            {
                length = gzread( inFile, buffer.get(), kGzBufferSize );
                if( length < 0 )
                {
                    gzerror( inFile, &err );
                    break;
                }
                if( length == 0 )
                {
                    worked = true;
                    break;
                }
                if( fwrite( buffer.get(), 1, (unsigned int)length, outFile ) != length )
                    break;
            }
            if( gzclose( inFile ) != Z_OK )
                worked = false;
        }
        fclose( outFile );
    }

    return worked;
}


bool  plZlibCompress::CompressFile( const char *uncompressedPath, const char *destPath )
{
    FILE    *inFile;
    gzFile  outFile;
    bool    worked = false;
    int     err;

    auto buffer = std::make_unique<uint8_t[]>(kGzBufferSize);


    inFile = fopen( uncompressedPath, "rb" );
    if (inFile != nullptr)
    {
        outFile = gzopen( destPath, "wb" );
        if (outFile != nullptr)
        {
            for( ;; )
            {
                size_t length = fread( buffer.get(), 1, kGzBufferSize, inFile );
                if( ferror( inFile ) )
                    break;

                if( length == 0 )
                {
                    worked = true;
                    break;
                }
                if( gzwrite( outFile, buffer.get(), (unsigned int)length ) != length )
                {
                    gzerror( outFile, &err );
                    break;
                }
            }
            if( gzclose( outFile ) != Z_OK )
                worked = false;
        }
        fclose( inFile );
    }

    return worked;
}


//// file <-> stream ///////////////////////////////////////////////////////

bool  plZlibCompress::UncompressToStream( const char * filename, hsStream * s )
{
    gzFile  inFile;
    bool    worked = false;
    int     length, err;

    auto buffer = std::make_unique<uint8_t[]>(kGzBufferSize);

    inFile = gzopen( filename, "rb" );
    if (inFile != nullptr)
    {
        for( ;; )
        {
            length = gzread( inFile, buffer.get(), kGzBufferSize );
            if( length < 0 )
            {
                gzerror( inFile, &err );
                break;
            }
            if( length == 0 )
            {
                worked = true;
                break;
            }
            s->Write( length, buffer.get() );
        }
        if( gzclose( inFile ) != Z_OK )
            worked = false;
    }

    return worked;
}


bool  plZlibCompress::CompressToFile( hsStream * s, const char * filename )
{
    gzFile  outFile;
    bool    worked = false;
    int     err;

    auto buffer = std::make_unique<uint8_t[]>(kGzBufferSize);

    outFile = gzopen( filename, "wb" );
    if (outFile != nullptr)
    {
        for( ;; )
        {
            uint32_t avail = s->GetEOF() - s->GetPosition();
            uint32_t n = ( avail > kGzBufferSize ) ? kGzBufferSize : avail;

            if( n == 0 )
            {
                worked = true;
                break;
            }

            uint32_t length = s->Read( n, buffer.get() );

            if( length == 0 )
            {
                worked = true;
                break;
            }

            if( gzwrite( outFile, buffer.get(), length ) != length )
            {
                gzerror( outFile, &err );
                break;
            }
        }
        if( gzclose( outFile ) != Z_OK )
            worked = false;
    }

    return worked;
}


#endif
