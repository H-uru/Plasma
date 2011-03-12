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
///////////////////////////////////////////////////////////////////////////////
//																			 //
//	plBitmap Class Functions												 //
//	Base bitmap class for all the types of bitmaps (mipmaps, cubic envmaps,  //
//	etc.																	 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	6.7.2001 mcn - Created.													 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "plBitmap.h"

#include "hsResMgr.h"
#include "hsStream.h"
#include "../pnKeyedObject/plKey.h"
#include "../plPipeline/hsGDeviceRef.h"


//// Static Members ///////////////////////////////////////////////////////////

UInt8	plBitmap::fGlobalNumLevelsToChop = 0;


//// Constructor & Destructor /////////////////////////////////////////////////

plBitmap::plBitmap()
{
	fPixelSize = 0;
	fSpace = kNoSpace;
	fFlags = 0;
	fCompressionType = kUncompressed;
	fUncompressedInfo.fType = UncompressedInfo::kRGB8888;
	fDeviceRef = nil;
	fLowModifiedTime = fHighModifiedTime = 0;
}

plBitmap::~plBitmap()
{
	if( fDeviceRef != nil )
		hsRefCnt_SafeUnRef( fDeviceRef );
}

bool plBitmap::IsSameModifiedTime(UInt32 lowTime, UInt32 highTime)
{
	return (fLowModifiedTime == lowTime && fHighModifiedTime == highTime);
}

void plBitmap::SetModifiedTime(UInt32 lowTime, UInt32 highTime)
{
	fLowModifiedTime = lowTime;
	fHighModifiedTime = highTime;
}

//// Read /////////////////////////////////////////////////////////////////////

static UInt8	sBitmapVersion = 2;

UInt32 	plBitmap::Read( hsStream *s )
{
	UInt8	version = s->ReadByte();
	UInt32	read = 6;


	hsAssert( version == sBitmapVersion, "Invalid bitamp version on Read()" );

	fPixelSize = s->ReadByte();
	fSpace = s->ReadByte();
	fFlags = s->ReadSwap16();
	fCompressionType = s->ReadByte();

	if(( fCompressionType == kUncompressed )||( fCompressionType == kJPEGCompression ))
	{
		fUncompressedInfo.fType = s->ReadByte();
		read++;
	}
	else
	{
		fDirectXInfo.fBlockSize = s->ReadByte();
		fDirectXInfo.fCompressionType = s->ReadByte();
		read += 2;
	}

	fLowModifiedTime = s->ReadSwap32();
	fHighModifiedTime = s->ReadSwap32();

	return read;
}

//// Write ////////////////////////////////////////////////////////////////////

UInt32 	plBitmap::Write( hsStream *s )
{
	UInt32	written = 6;


	s->WriteByte( sBitmapVersion );

	s->WriteByte( fPixelSize );
	s->WriteByte( fSpace );
	s->WriteSwap16( fFlags );
	s->WriteByte( fCompressionType );

	if(( fCompressionType == kUncompressed )||(fCompressionType == kJPEGCompression ))
	{
		s->WriteByte( fUncompressedInfo.fType );
		written++;
	}
	else
	{
		s->WriteByte( fDirectXInfo.fBlockSize );
		s->WriteByte( fDirectXInfo.fCompressionType );
		written += 2;
	}

	s->WriteSwap32(fLowModifiedTime);
	s->WriteSwap32(fHighModifiedTime);

	return written;
}

//// SetDeviceRef /////////////////////////////////////////////////////////////

void	plBitmap::SetDeviceRef( hsGDeviceRef *const devRef )
{
	if( fDeviceRef == devRef )
		return;

	hsRefCnt_SafeAssign( fDeviceRef, devRef );
}

void plBitmap::MakeDirty()
{
	if( fDeviceRef )
		fDeviceRef->SetDirty(true);
}
