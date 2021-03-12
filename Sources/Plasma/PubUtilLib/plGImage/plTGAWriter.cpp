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
//  plTGAWriter Class Functions                                              //
//  Simple utility class for writing a plMipmap out as a TGA file            //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  8.15.2001 mcn - Created.                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "plTGAWriter.h"
#include "hsColorRGBA.h"
#include "plMipmap.h"
#include "hsStream.h"


//// Class Statics ////////////////////////////////////////////////////////////

plTGAWriter plTGAWriter::fInstance;


//// WriteMipmap //////////////////////////////////////////////////////////////

void    plTGAWriter::WriteMipmap( const char *fileName, plMipmap *mipmap )
{
    hsUNIXStream    stream;
    int             x, y;
    hsRGBAColor32   pixel;


    stream.Open( fileName, "wb" );

    /// Write header
    stream.WriteByte(uint8_t(0));  // Size of ID field
    stream.WriteByte(uint8_t(0));  // Map type
    stream.WriteByte(uint8_t(2));  // Type 2 image - Unmapped RGB

    stream.WriteByte(uint8_t(0));  // Color map spec
    stream.WriteByte(uint8_t(0));  // Color map spec
    stream.WriteByte(uint8_t(0));  // Color map spec
    stream.WriteByte(uint8_t(0));  // Color map spec
    stream.WriteByte(uint8_t(0));  // Color map spec

    stream.WriteLE16(uint16_t(0));    // xOrigin
    stream.WriteLE16(uint16_t(0));    // yOrigin

    stream.WriteLE16( (uint16_t)mipmap->GetWidth() );
    stream.WriteLE16( (uint16_t)mipmap->GetHeight() );

    stream.WriteByte(uint8_t(24));
    stream.WriteByte(uint8_t(0));

    /// Write image data (gotta do inversed, stupid TGAs...)
    for( y = mipmap->GetHeight() - 1; y >= 0; y-- )
    {
        for( x = 0; x < mipmap->GetWidth(); x++ )
        {
            pixel = *( (hsRGBAColor32 *)mipmap->GetAddr32( x, y ) );
            stream.WriteByte( pixel.b );
            stream.WriteByte( pixel.g );
            stream.WriteByte( pixel.r );
        }
    }

    /// All done!
    stream.Close();
}

