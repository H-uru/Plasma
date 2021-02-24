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
//  plTextGenerator Class Header                                             //
//  Helper utility class that "attaches" to a mipmap and fills that mipmap   //
//  with text.
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  12.13.2001 mcn - Created.                                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plTextGenerator_h
#define _plTextGenerator_h

// Dammit
#include "hsWindows.h"

#include "hsColorRGBA.h"
#include "hsGeometry3.h"

#include "pnKeyedObject/hsKeyedObject.h"

//// plTextGenerator Class Definition //////////////////////////////////////////////

class plMipmap;
struct hsMatrix44;

class plTextGenerator : public hsKeyedObject
{
    protected:
    
        plMipmap    *fHost;
        uint16_t      fWidth, fHeight;

#if HS_BUILD_FOR_WIN32
        HDC         fWinRGBDC;
        HBITMAP     fWinRGBBitmap;
        HFONT       fWinFont;
        uint32_t      *fWinRGBBits;

        HFONT       fWinAlphaFont;
        HDC         fWinAlphaDC;
        HBITMAP     fWinAlphaBitmap;
        uint8_t       *fWinAlphaBits;
#endif

        uint32_t      *IAllocateOSSurface( uint16_t width, uint16_t height );
        void        IDestroyOSSurface();

    public:

        plTextGenerator();
        plTextGenerator( plMipmap *host, uint16_t width, uint16_t height );
        virtual ~plTextGenerator();

        void    Attach( plMipmap *host, uint16_t width, uint16_t height );
        void    Detach();

        /// Operations to perform on the text block
        
        void    ClearToColor( hsColorRGBA &color );

        void    SetFont( const char *face, uint16_t size, bool antiAliasRGB = true );
        void    SetTextColor( hsColorRGBA &color, bool blockRGB = false );

        void        DrawString( uint16_t x, uint16_t y, const char *text );
        void        DrawString( uint16_t x, uint16_t y, const wchar_t *text );
        void        DrawClippedString( int16_t x, int16_t y, const char *text, uint16_t width, uint16_t height );
        void        DrawClippedString( int16_t x, int16_t y, const wchar_t *text, uint16_t width, uint16_t height );
        void        DrawClippedString( int16_t x, int16_t y, const char *text, uint16_t clipX, uint16_t clipY, uint16_t width, uint16_t height );
        void        DrawClippedString( int16_t x, int16_t y, const wchar_t *text, uint16_t clipX, uint16_t clipY, uint16_t width, uint16_t height );
        void        DrawWrappedString( uint16_t x, uint16_t y, const char *text, uint16_t width, uint16_t height );
        void        DrawWrappedString( uint16_t x, uint16_t y, const wchar_t *text, uint16_t width, uint16_t height );
        uint16_t    CalcStringWidth(const char *text, uint16_t *height = nullptr);
        uint16_t    CalcStringWidth(const wchar_t *text, uint16_t *height = nullptr);
        void        CalcWrappedStringSize( const char *text, uint16_t *width, uint16_t *height );
        void        CalcWrappedStringSize( const wchar_t *text, uint16_t *width, uint16_t *height );
        void        FillRect( uint16_t x, uint16_t y, uint16_t width, uint16_t height, hsColorRGBA &color );
        void        FrameRect( uint16_t x, uint16_t y, uint16_t width, uint16_t height, hsColorRGBA &color );

        void    FlushToHost();

        uint16_t  GetTextWidth();
        uint16_t  GetTextHeight();

        uint16_t  GetWidth() { return fWidth; }
        uint16_t  GetHeight() { return fHeight; }

        // Since the textGen can actually create a texture bigger than you were expecting,
        // you want to be able to apply a layer texture transform that will compensate. This
        // function will give you that transform. Just feed it into plLayer->SetTransform().

        hsMatrix44  GetLayerTransform();


        bool MsgReceive(plMessage *msg) override;
};


#endif // _plTextGenerator_h

