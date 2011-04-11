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
#ifndef plCompress_h
#define plCompress_h

#include "hsTypes.h"

//
// Abstract base class for compression code
//
class plCompress
{
public:
	plCompress() {}
	virtual ~plCompress() {}

	// return true if successful
	virtual hsBool Uncompress(UInt8* bufOut, UInt32* bufLenOut, const UInt8* bufIn, UInt32 bufLenIn) = 0;
	virtual hsBool Compress(UInt8* bufOut, UInt32* bufLenOut, const UInt8* bufIn, UInt32 bufLenIn) = 0;

	// in place versions
	virtual hsBool Uncompress(UInt8** bufIn, UInt32* bufLenIn, UInt32 maxBufLenOut, int offset=0) = 0;
	virtual hsBool Compress(UInt8** bufIn, UInt32* bufLenIn, int offset=0) = 0;
};


#endif	// plCompress_h

