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
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  plDebugText and plDebugTextManager Headers                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _plDebugText_h
#define _plDebugText_h

#include <string_theory/string>
#include <utility>
#include <vector>

#include "HeadSpin.h"
#include "hsColorRGBA.h"


//// plDebugText Class Definition ////////////////////////////////////////////

class plPipeline;
class plDebugTextManager;

class plDebugText
{
    private:

        static plDebugText  fInstance;

        plDebugText() 
        { 
            fManager = nullptr;
#ifdef PLASMA_EXTERNAL_RELEASE
            SetFont(ST_LITERAL("Trebuchet MS Bold"), 8);
#else
            SetFont(ST_LITERAL("Courier New"), 8);
#endif
            SetEnable( true );
            fLockEnable = false;
            fDrawOnTopMode = false;
        }

    protected:

        plDebugTextManager  *fManager;

        ST::string fFontFace;
        uint16_t          fFontSize;
        bool            fEnabled, fLockEnable, fDrawOnTopMode;

    public:

        enum Styles
        {
            kStyleItalic = 0x01,
            kStyleBold = 0x02
        };

        ~plDebugText() { }

        static plDebugText  &Instance() { return fInstance; }

        uint32_t CalcStringWidth(const char *string);
        uint32_t CalcStringWidth_TEMP(const ST::string &string) { return CalcStringWidth(string.c_str()); }

        void DrawString(uint16_t x, uint16_t y, const char *string, uint32_t hexColor, uint8_t style = 0);

        void DrawString_TEMP(uint16_t x, uint16_t y, const ST::string &string, uint32_t hexColor, uint8_t style = 0)
        {
            DrawString(x, y, string.c_str(), hexColor, style);
        }

        void DrawString(uint16_t x, uint16_t y, const ST::string &string, hsColorRGBA &color, uint8_t style = 0)
        {
            uint32_t  hex;
            uint8_t   r, g, b, a;


            r = (uint8_t)( color.r * 255.0 );
            g = (uint8_t)( color.g * 255.0 );
            b = (uint8_t)( color.b * 255.0 );
            a = (uint8_t)( color.a * 255.0 );
            hex = ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | ( b );

            DrawString_TEMP(x, y, string, hex, style);
        }

        void DrawString(uint16_t x, uint16_t y, const ST::string& string)
        {
            DrawString_TEMP(x, y, string, 0xffffffff, 0);
        }

        void DrawString(uint16_t x, uint16_t y, const ST::string &string, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255, uint8_t style = 0)
        {
            DrawString_TEMP(x, y, string, (uint32_t)( ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | ( b ) ), style);
        }

        void    SetDrawOnTopMode( bool enable ) { fDrawOnTopMode = enable; }

        /// TEMPORARY FUNCTION (until we can find a better way to do this, one way or the other)
        void    DrawRect( uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, uint32_t hexColor );

        /// TEMPORARY FUNCTION (until we can find a better way to do this, one way or the other)
        void    DrawRect( uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255 )
        {
            DrawRect( left, top, right, bottom, (uint32_t)( ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | ( b ) ) );
        }

        /// EVEN MORE TEMPORARY FUNCTION (until we can find a better way to do this, one way or the other)
        void    Draw3DBorder( uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, uint32_t hexColor1, uint32_t hexColor2 );

        void    SetManager( plDebugTextManager *m ) { fManager = m; }

        void SetFont(ST::string face, uint16_t size) { fFontFace = std::move(face); fFontSize = size; }
        ST::string GetFontFace() { return fFontFace; }
        uint16_t        GetFontSize() { return fFontSize; }
        uint16_t        GetFontHeight();

        void            SetEnable( bool on ) { fEnabled = on; }
        void            DisablePermanently() { fEnabled = false; fLockEnable = true; }
        bool            IsEnabled() { return fEnabled; }

        void            GetScreenSize( uint32_t *width, uint32_t *height );
};

//// plDebugTextManager Class Definition /////////////////////////////////////

class plTextFont;

class   plDebugTextManager
{
    protected:

        struct plDebugTextNode
        {
            char    fText[ 256 ];
            uint32_t  fColor, fDarkColor;
            uint16_t  fX, fY, fRight, fBottom;    // Last 2 are for rects only
            uint8_t   fStyle;                     // 0xff means rectangle, 0xfe means 3d border

            plDebugTextNode() { fText[ 0 ] = 0; fColor = 0; fX = fY = 0; fStyle = 0; }
            plDebugTextNode( const char *s, uint32_t c, uint16_t x, uint16_t y, uint8_t style ); 
            plDebugTextNode( uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, uint32_t c ); 
            plDebugTextNode( uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, uint32_t c1, uint32_t c2 );
            ~plDebugTextNode() { }
        };

        std::vector<plDebugTextNode> fList;
        std::vector<plDebugTextNode> fDrawOnTopList;

        plTextFont                  *fFont;
        uint32_t                      fSWidth, fSHeight;

    public:

        plDebugTextManager() { plDebugText::Instance().SetManager(this); fFont = nullptr; }
        ~plDebugTextManager();

        void    AddString( uint16_t x, uint16_t y, const char *s, uint32_t hexColor, uint8_t style, bool drawOnTop = false );
        uint32_t  CalcStringWidth( const char *string );

        /// TEMPORARY FUNCTION (until we can find a better way to do this, one way or the other)
        void    DrawRect( uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, uint32_t hexColor, bool drawOnTop = false );

        /// EVEN MORE TEMPORARY FUNCTION (until we can find a better way to do this, one way or the other)
        void    Draw3DBorder( uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, uint32_t hexColor1, uint32_t hexColor2, bool drawOnTop = false );

        void    DrawToDevice( plPipeline *pipe );

        void    GetScreenSize( uint32_t *width, uint32_t *height );

        uint16_t  GetFontHeight();
};


#endif //_plDebugText_h

