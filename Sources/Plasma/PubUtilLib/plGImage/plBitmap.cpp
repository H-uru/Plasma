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
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  plBitmap Class Functions                                                 //
//  Base bitmap class for all the types of bitmaps (mipmaps, cubic envmaps,  //
//  etc.                                                                     //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  6.7.2001 mcn - Created.                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "plBitmap.h"

#include "hsResMgr.h"
#include "hsStream.h"
#include "hsGDeviceRef.h"
#include "pnKeyedObject/plKey.h"


//// Static Members ///////////////////////////////////////////////////////////

uint8_t   plBitmap::fGlobalNumLevelsToChop = 0;


//// Constructor & Destructor /////////////////////////////////////////////////

plBitmap::plBitmap()
{
    fPixelSize = 0;
    fSpace = kNoSpace;
    fFlags = 0;
    fCompressionType = kUncompressed;
    fUncompressedInfo.fType = UncompressedInfo::kRGB8888;
    fDeviceRef = nullptr;
    fLowModifiedTime = fHighModifiedTime = 0;
}

plBitmap::~plBitmap()
{
    if (fDeviceRef != nullptr)
        hsRefCnt_SafeUnRef( fDeviceRef );
}

bool plBitmap::IsSameModifiedTime(uint32_t lowTime, uint32_t highTime)
{
    return (fLowModifiedTime == lowTime && fHighModifiedTime == highTime);
}

void plBitmap::SetModifiedTime(uint32_t lowTime, uint32_t highTime)
{
    fLowModifiedTime = lowTime;
    fHighModifiedTime = highTime;
}

//// Read /////////////////////////////////////////////////////////////////////

static uint8_t    sBitmapVersion = 2;

uint32_t  plBitmap::Read( hsStream *s )
{
    uint8_t   version = s->ReadByte();
    uint32_t  read = 6;


    hsAssert( version == sBitmapVersion, "Invalid bitamp version on Read()" );

    fPixelSize = s->ReadByte();
    fSpace = s->ReadByte();
    fFlags = s->ReadLE16();
    fCompressionType = s->ReadByte();

    if( fCompressionType == kUncompressed || fCompressionType == kJPEGCompression ||
        fCompressionType == kPNGCompression )
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

    fLowModifiedTime = s->ReadLE32();
    fHighModifiedTime = s->ReadLE32();

    return read;
}

//// Write ////////////////////////////////////////////////////////////////////

uint32_t  plBitmap::Write( hsStream *s )
{
    uint32_t  written = 6;


    s->WriteByte( sBitmapVersion );

    s->WriteByte( fPixelSize );
    s->WriteByte( fSpace );
    s->WriteLE16( fFlags );
    s->WriteByte( fCompressionType );

    if( fCompressionType == kUncompressed || fCompressionType == kJPEGCompression ||
        fCompressionType == kPNGCompression )
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

    s->WriteLE32(fLowModifiedTime);
    s->WriteLE32(fHighModifiedTime);

    return written;
}

//// SetDeviceRef /////////////////////////////////////////////////////////////

void    plBitmap::SetDeviceRef( hsGDeviceRef *const devRef )
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
