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
#ifndef plZlibCompress_h
#define plZlibCompress_h

#include "plCompress.h"

class hsStream;

class plZlibCompress : public plCompress
{
protected:
	hsBool ICopyBuffers(UInt8** bufIn, UInt32* bufLenIn, char* bufOut, UInt32 bufLenOut, int offset, bool ok );
public:
	hsBool Uncompress(UInt8* bufOut, UInt32* bufLenOut, const UInt8* bufIn, UInt32 bufLenIn);
	hsBool Compress(UInt8* bufOut, UInt32* bufLenOut, const UInt8* bufIn, UInt32 bufLenIn);

	// in place versions
	hsBool Uncompress(UInt8** bufIn, UInt32* bufLenIn, UInt32 maxBufLenOut, int offset=0);
	hsBool Compress(UInt8** bufIn, UInt32* bufLenIn, int offset=0);

	// .gz versions
	static hsBool	UncompressFile( const char *compressedPath, const char *destPath );
	static hsBool	CompressFile( const char *uncompressedPath, const char *destPath );

	// file <-> stream
	static hsBool	UncompressToStream( const char * filename, hsStream * s );
	static hsBool	CompressToFile( hsStream * s, const char * filename );
};

#endif	// plZlibCompress_h
