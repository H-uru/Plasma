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
#include "plZlibCompress.h"
#include "zlib.h"
#include "hsMemory.h"
#include "hsStream.h"

hsBool plZlibCompress::Uncompress(UInt8* bufOut, UInt32* bufLenOut, const UInt8* bufIn, UInt32 bufLenIn)
{
	return (uncompress(bufOut, bufLenOut, bufIn, bufLenIn) == Z_OK);
}

hsBool plZlibCompress::Compress(UInt8* bufOut, UInt32* bufLenOut, const UInt8* bufIn, UInt32 bufLenIn)
{
	// according to compress doc, the bufOut buffer should be at least .1% larger than source buffer, plus 12 bytes.
	hsAssert(*bufLenOut>=(int)(bufLenIn*1.1+12), "bufOut compress buffer is not large enough");
	return (compress(bufOut, bufLenOut, bufIn, bufLenIn) == Z_OK);
}

//
// copy bufOut to bufIn, set bufLenIn=bufLenOut
//
hsBool plZlibCompress::ICopyBuffers(UInt8** bufIn, UInt32* bufLenIn, char* bufOut, UInt32 bufLenOut, int offset, bool ok)
{
	if (ok)
	{
		*bufLenIn = bufLenOut+offset;
		UInt8* newBuf = TRACKED_NEW UInt8[*bufLenIn];			// alloc new buffer
		HSMemory::BlockMove(*bufIn, newBuf, offset);	// copy offset (uncompressed) part
		delete [] *bufIn;								// delete original buffer

		HSMemory::BlockMove(bufOut, newBuf+offset, bufLenOut);	// copy compressed part
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
hsBool plZlibCompress::Compress(UInt8** bufIn, UInt32* bufLenIn, int offset)
{
	UInt32 adjBufLenIn = *bufLenIn - offset;
	UInt8* adjBufIn = *bufIn + offset;

	// according to compress doc, the bufOut buffer should be at least .1% larger than source buffer, plus 12 bytes.
	UInt32 bufLenOut = (int)(adjBufLenIn*1.1+12);
	char* bufOut = TRACKED_NEW char[bufLenOut];
	
	bool ok=(Compress((UInt8*)bufOut, &bufLenOut, (UInt8*)adjBufIn, adjBufLenIn) && 
		bufLenOut < adjBufLenIn);
	return ICopyBuffers(bufIn, bufLenIn, bufOut, bufLenOut, offset, ok);
}

//
// In place version
//
hsBool plZlibCompress::Uncompress(UInt8** bufIn, UInt32* bufLenIn, UInt32 bufLenOut, int offset)
{
	UInt32 adjBufLenIn = *bufLenIn - offset;
	UInt8* adjBufIn = *bufIn + offset;

	char* bufOut = TRACKED_NEW char[bufLenOut];
	
	bool ok=Uncompress((UInt8*)bufOut, &bufLenOut, (UInt8*)adjBufIn, adjBufLenIn) ? true : false;
	return ICopyBuffers(bufIn, bufLenIn, bufOut, bufLenOut, offset, ok);
}

//// .gz File Versions ///////////////////////////////////////////////////////

#define kGzBufferSize	64 * 1024
#if 1

hsBool	plZlibCompress::UncompressFile( const char *compressedPath, const char *destPath )
{
	gzFile	inFile;
	FILE	*outFile;
	hsBool	worked = false;
	int		length, err;

	UInt8	buffer[ kGzBufferSize ];


	outFile = fopen( destPath, "wb" );
	if( outFile != nil )
	{
		inFile = gzopen( compressedPath, "rb" );
		if( inFile != nil )
		{
			for( ;; )
			{
				length = gzread( inFile, buffer, sizeof( buffer ) );
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
				if( fwrite( buffer, 1, length, outFile ) != length )
					break;
			}
			if( gzclose( inFile ) != Z_OK )
				worked = false;
		}
		fclose( outFile );
	}

	return worked;
}


hsBool	plZlibCompress::CompressFile( const char *uncompressedPath, const char *destPath )
{
	FILE	*inFile;
	gzFile	outFile;
	hsBool	worked = false;
	int		length, err;

	UInt8	buffer[ kGzBufferSize ];


	inFile = fopen( uncompressedPath, "rb" );
	if( inFile != nil )
	{
		outFile = gzopen( destPath, "wb" );
		if( outFile != nil )
		{
			for( ;; )
			{
				length = fread( buffer, 1, sizeof( buffer ), inFile );
				if( ferror( inFile ) )
					break;

				if( length == 0 )
				{
					worked = true;
					break;
				}
				if( gzwrite( outFile, buffer, (unsigned)length ) != length )
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

hsBool	plZlibCompress::UncompressToStream( const char * filename, hsStream * s )
{
	gzFile	inFile;
	hsBool	worked = false;
	int		length, err;

	UInt8	buffer[ kGzBufferSize ];


	inFile = gzopen( filename, "rb" );
	if( inFile != nil )
	{
		for( ;; )
		{
			length = gzread( inFile, buffer, sizeof( buffer ) );
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
			s->Write( length, buffer );
		}
		if( gzclose( inFile ) != Z_OK )
			worked = false;
	}

	return worked;
}


hsBool	plZlibCompress::CompressToFile( hsStream * s, const char * filename )
{
	gzFile	outFile;
	hsBool	worked = false;
	int		length, err;

	UInt8	buffer[ kGzBufferSize ];


	outFile = gzopen( filename, "wb" );
	if( outFile != nil )
	{
		for( ;; )
		{
			int avail = s->GetEOF()-s->GetPosition();
			int n = ( avail>sizeof( buffer ) ) ? sizeof( buffer ) : avail;

			if( n == 0 )
			{
				worked = true;
				break;
			}

			length = s->Read( n, buffer );

			if( length == 0 )
			{
				worked = true;
				break;
			}

			if( gzwrite( outFile, buffer, (unsigned)length ) != length )
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
