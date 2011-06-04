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
#ifndef hsMMIOStream_inc
#define hsMMIOStream_inc

#include "hsWindows.h"
#include "hsStream.h"

#if HS_BUILD_FOR_WIN32
#include <mmsystem.h>
#endif

class hsMMIOStream: public hsStream
{	
	HMMIO	fHmfr;
public:

	virtual hsBool	Open(const char *, const char *)	{ hsAssert(0, "hsMMIOStream::Open  NotImplemented"); return false; }
	virtual hsBool	Close()				{ hsAssert(0, "hsMMIOStream::Close  NotImplemented"); return false; }

	virtual hsBool	AtEnd();
	virtual UInt32	Read(UInt32 byteCount, void* buffer);
	virtual UInt32	Write(UInt32 byteCount, const void* buffer);
	virtual void	Skip(UInt32 deltaByteCount);
	virtual void	Rewind();
	virtual void	FastFwd();
    virtual void    Truncate();

	HMMIO	GetHandle() { return fHmfr; }
	void	SetHandle(HMMIO handle) { fHmfr = handle; }
};

#endif	// hsMMIOStream_inc
