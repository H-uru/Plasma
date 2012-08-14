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
//  plDynSurfaceWriter Class Header                                          //
//  Abstract class wrapping around Windows GDI functionality for writing to  //
//  a generic RGBA surface. Allows us to create one writer per DTMap or a    //
//  single shared writer to conserve OS resources on 98/ME.                  //
//                                                                           //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  10.28.2002 mcn - Created.                                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plDynSurfaceWriter_h
#define _plDynSurfaceWriter_h

#include "hsColorRGBA.h"
              // EVIL

struct hsMatrix44;

class plDynamicTextMap;

//// Class Definition /////////////////////////////////////////////////////////

class plDynSurfaceWriter
{
    public:

        //// Public Flags ////
        enum Justify
        {
            kLeftJustify = 0,
            kCenter,
            kRightJustify
        };

        enum Flags
        {
            kSupportAlpha   = 0x00000001,
            
            kFontBold       = 0x00000002,
            kFontItalic     = 0x00000004,
            kFontShadowed   = 0x00000008,
            kFontMask       = 0x0000000e,

            kDiscardOnFlush = 0x00000010
        };

        //// Public Data /////
        
        
        //// Public Members ////


        plDynSurfaceWriter();
        plDynSurfaceWriter( plDynamicTextMap *target, uint32_t flags = 0 );
        virtual ~plDynSurfaceWriter();


        /// Operations to perform on the text block
        
        void    ClearToColor( hsColorRGBA &color );
        void    SetFont( const char *face, uint16_t size, uint8_t fontFlags = 0, bool antiAliasRGB = true );
        void    SetTextColor( hsColorRGBA &color, bool blockRGB = false );
        void    SetJustify( Justify j );

        void    DrawString( uint16_t x, uint16_t y, const char *text );
        void    DrawClippedString( int16_t x, int16_t y, const char *text, uint16_t width, uint16_t height );
        void    DrawClippedString( int16_t x, int16_t y, const char *text, uint16_t clipX, uint16_t clipY, uint16_t width, uint16_t height );
        void    DrawWrappedString( uint16_t x, uint16_t y, const char *text, uint16_t width, uint16_t height );
        uint16_t  CalcStringWidth( const char *text, uint16_t *height = nil );
        void    CalcWrappedStringSize( const char *text, uint16_t *width, uint16_t *height );
        void    FillRect( uint16_t x, uint16_t y, uint16_t width, uint16_t height, hsColorRGBA &color );
        void    FrameRect( uint16_t x, uint16_t y, uint16_t width, uint16_t height, hsColorRGBA &color );

//      void    DrawImage( uint16_t x, uint16_t y, plMipmap *image, bool respectAlpha = false );
//      void    DrawClippedImage( uint16_t x, uint16_t y, plMipmap *image, uint16_t srcClipX, uint16_t srcClipY, 
//                              uint16_t srcClipWidth, uint16_t srcClipHeight, bool respectAlpha = false );

        // Copy the raw data from the given buffer.
//      void    SetBitsFromBuffer( uint32_t *clearBuffer, uint16_t width, uint16_t height )

        /// Target switching operations

        // Flushes all ops to the target.
        void    FlushToTarget( void );

        // Switches targets. Will flush to old target before switching. Also, if kDiscard isn't specified, will copy contents of new target to working surface
        void    SwitchTarget( plDynamicTextMap *target );       // Will force a flush

        // Clears and resets everything. Does NOT flush.
        void    Reset( void );

        bool    IsValid( void ) const;

        static bool     CanHandleLotsOfThem( void );

    protected:

        //// Protected Members ////

        void        IInit( void );
        void        IEnsureSurfaceUpdated( void );
        void        IRefreshOSJustify( void );
        void        ISetTextColor( hsColorRGBA &color, bool blockRGB );

        void        ISetFont( const char *face, uint16_t size, uint8_t fontFlags = 0, bool antiAliasRGB = true );

        plDynamicTextMap    *fCurrTarget;
        uint32_t              fFlags;
        Justify             fJustify;
        bool                fFlushed;

        char        *fFontFace;
        uint16_t      fFontSize;
        uint8_t       fFontFlags;
        bool        fFontAntiAliasRGB;
        bool        fFontBlockedRGB;

        static bool         fForceSharedSurfaces;
        static bool         fOSDetected;
        static bool         fOSCanShareSurfaces;

#if HS_BUILD_FOR_WIN32
        class plWinSurface
        {
            protected:
                void        *fBits;

                virtual uint8_t   IBitsPerPixel( void ) const = 0;

            public:
                HDC         fDC;
                HBITMAP     fBitmap;
                HFONT       fFont;
                COLORREF    fTextColor;
                int         fSaveNum;

                uint16_t      fWidth, fHeight;

                char        *fFontFace;
                uint16_t      fFontSize;
                uint8_t       fFontFlags;
                bool        fFontAntiAliasRGB, fFontBlockedRGB;

                plWinSurface();
                ~plWinSurface();

                void    Allocate( uint16_t w, uint16_t h );
                void    Release( void );

                bool    WillFit( uint16_t w, uint16_t h );
                bool    FontMatches( const char *face, uint16_t size, uint8_t flags, bool aaRGB );
                void    SetFont( const char *face, uint16_t size, uint8_t flags, bool aaRGB );
        };

        class plWinRGBSurface : public plWinSurface
        {
                virtual uint8_t   IBitsPerPixel( void ) const { return 32; }
            public:
                uint32_t  *GetBits( void ) const { return (uint32_t *)fBits; }
        };

        class plWinAlphaSurface : public plWinSurface
        {
                virtual uint8_t   IBitsPerPixel( void ) const { return 8; }
            public:
                uint8_t   *GetBits( void ) const { return (uint8_t *)fBits; }
        };

        plWinRGBSurface     fRGBSurface;
        plWinAlphaSurface   fAlphaSurface;
#endif
};


#endif // _plDynSurfaceWriter_h
