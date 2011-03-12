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
#include "hsMMIOStream.h"

//////////////////////////////////////////////////////////////////////////////////////

#if HS_BUILD_FOR_WIN32

#include "hsExceptions.h"

UInt32 hsMMIOStream::Read(UInt32 bytes,  void* buffer)
{
	fBytesRead += bytes;
    fPosition += bytes;
	int numItems = ::mmioRead(fHmfr, (char*)buffer, bytes);
	if ((unsigned)numItems < bytes) 
	{
		if (numItems>=0 && ::mmioSeek(fHmfr,0,SEEK_CUR)==::mmioSeek(fHmfr,0,SEEK_END)) {
			// EOF ocurred
			char str[128];
			sprintf(str, "Hit EOF on MMIO Read, only read %d out of requested %d bytes\n", numItems, bytes);
			hsDebugMessage(str, 0);
		}
		else 
		{
			hsDebugMessage("Error on MMIO Read",0);
		}
	}
	return numItems;
}

hsBool  hsMMIOStream::AtEnd()
{
	return (::mmioSeek(fHmfr,0,SEEK_CUR)==::mmioSeek(fHmfr,0,SEEK_END));
}

UInt32 hsMMIOStream::Write(UInt32 bytes, const void* buffer)
{
    fPosition += bytes;
	return ::mmioWrite(fHmfr,(const char*)buffer,bytes);
}

void hsMMIOStream::Skip(UInt32 delta)
{
	fBytesRead += delta;
    fPosition += delta;
	(void)::mmioSeek(fHmfr, delta, SEEK_CUR);
}

void hsMMIOStream::Rewind()
{
	fBytesRead = 0;
    fPosition = 0;
	(void)::mmioSeek(fHmfr, 0, SEEK_SET);
}

void hsMMIOStream::FastFwd()
{
	fBytesRead = fPosition = ::mmioSeek(fHmfr, 0, SEEK_END);
}

void hsMMIOStream::Truncate()
{
	hsThrow("Truncate unimplemented by subclass of stream");
}
#endif
