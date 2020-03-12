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
//  pcSmallRect - A tiny int16_t-based 2D rectangle class                      //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// Note: I'm introducing the concept here that new-style coreLib files/classes
// are named with the "pc" prefix, just as featureLib uses "pf" and nucleusLib
// uses "pn". I have no clue if this is kosher or good, but I'm doing it anyway.
// Feel free to change if desired

#ifndef _pcSmallRect_h
#define _pcSmallRect_h

#include "HeadSpin.h"

class hsStream;
class pcSmallRect
{
    public:
        int16_t   fX, fY, fWidth, fHeight;

        pcSmallRect() { Empty(); }
        pcSmallRect( int16_t x, int16_t y, int16_t w, int16_t h ) { Set( x, y, w, h ); }
        
        void    Set( int16_t x, int16_t y, int16_t w, int16_t h ) { fX = x; fY = y; fWidth = w; fHeight = h; }
        void    Empty() { fX = fY = fWidth = fHeight = 0; }

        int16_t   GetRight() const { return fX + fWidth; }
        int16_t   GetBottom() const { return fY + fHeight; }

        void    Read( hsStream *s );
        void    Write( hsStream *s );

        bool  Contains( int16_t x, int16_t y ) const
        {
            return (x >= fX) && (x <= fX + fWidth) && (y >= fY) && (y <= fY + fHeight);
        }
};


#endif // _pcSmallRect_h
