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
//  plCubicEnvironmap Class Functions                                        //
//  Derived bitmap class representing a collection of mipmaps to be used for //
//  cubic environment mapping.                                               //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  6.7.2001 mcn - Created.                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "plCubicEnvironmap.h"
#include "plMipmap.h"


//// Constructor & Destructor /////////////////////////////////////////////////

plCubicEnvironmap::plCubicEnvironmap()
{
    int     i;

    for( i = 0; i < 6; i++ )
        fFaces[ i ] = new plMipmap;

    fInitialized = false;
}

plCubicEnvironmap::~plCubicEnvironmap()
{
    int     i;

    for( i = 0; i < 6; i++ )
        delete fFaces[ i ];
}

//// GetTotalSize /////////////////////////////////////////////////////////////
//  Get the total size in bytes

uint32_t  plCubicEnvironmap::GetTotalSize() const
{
    uint32_t  size, i;


    for( size = 0, i = 0; i < 6; i++ )
    {
        hsAssert(fFaces[i] != nullptr, "Nil face in GetTotalSize()");
        size += fFaces[ i ]->GetTotalSize();
    }

    return size;
}

//// Read /////////////////////////////////////////////////////////////////////

uint32_t  plCubicEnvironmap::Read( hsStream *s )
{
    uint32_t  i, tr = plBitmap::Read( s );


    for( i = 0; i < 6; i++ )
        tr += fFaces[ i ]->Read( s );

    fInitialized = true;

    return tr;
}

//// Write ////////////////////////////////////////////////////////////////////

uint32_t  plCubicEnvironmap::Write( hsStream *s )
{
    uint32_t  i, tw = plBitmap::Write( s );


    for( i = 0; i < 6; i++ )
        tw += fFaces[ i ]->Write( s );

    return tw;
}

//// CopyToFace ///////////////////////////////////////////////////////////////
//  Export-only: Copy the mipmap given into a face

void    plCubicEnvironmap::CopyToFace( plMipmap *mip, uint8_t face )
{
    hsAssert( face < 6, "Invalid face index in CopyToFace()" );
    hsAssert(fFaces[face] != nullptr, "nil face in CopyToFace()");
    hsAssert(mip != nullptr, "nil source in CopyToFace()");


    if( !fInitialized )
    {
        // Make sure our stuff matches
        fCompressionType = mip->fCompressionType;
        if( fCompressionType != kDirectXCompression )
            fUncompressedInfo.fType = mip->fUncompressedInfo.fType;
        else
        {
            fDirectXInfo.fBlockSize = mip->fDirectXInfo.fBlockSize;
            fDirectXInfo.fCompressionType = mip->fDirectXInfo.fCompressionType;
        }

        fPixelSize = mip->GetPixelSize();
        fSpace = kDirectSpace;
        fFlags = mip->GetFlags();

        fInitialized = true;
    }
    else
    {
        // Check to make sure their stuff matches
        if( IsCompressed() != mip->IsCompressed() )
        {
            hsAssert( false, "Compression types do not match in CopyToFace()" );
            return;
        }

        if( !IsCompressed() )
        {
            if( fUncompressedInfo.fType != mip->fUncompressedInfo.fType )
            {
                hsAssert( false, "Compression formats do not match in CopyToFace()" );
                return;
            }
        }
        else
        {
            if( fDirectXInfo.fBlockSize != mip->fDirectXInfo.fBlockSize ||
                fDirectXInfo.fCompressionType != mip->fDirectXInfo.fCompressionType )
            {
                hsAssert( false, "Compression formats do not match in CopyToFace()" );
                return;
            }
        }

        if( fPixelSize != mip->GetPixelSize() )
        {
            hsAssert( false, "Bitdepths do not match in CopyToFace()" );
            return;
        }

        if( fFlags != mip->GetFlags() )
        {
            hsAssert( false, "Flags do not match in CopyToFace()" );
        }
    }

    // Copy the mipmap data
    fFaces[ face ]->CopyFrom( mip );
}

