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
//  plFont Class Header                                                      //
//  Seems like we've come full circle again. This is our generic Plasma      //
//  bitmap font class/format. Quick list of things it supports, or needs to: //
//      - Antialiasing, either in the font def or at rendertime              //
//      - Doublebyte character sets                                          //
//      - Platform independence, of course                                   //
//      - Render to reasonably arbitrary mipmap                              //
//      - Character-level kerning, both before and after, as well as         //
//        negative kerning (for ligatures)                                   //
//                                                                           //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  5.4.2003 mcn - Created.                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plFont_h
#define _plFont_h

#include "HeadSpin.h"
#include "hsColorRGBA.h"
#include "pcSmallRect.h"

#include <vector>

#include "pnKeyedObject/hsKeyedObject.h"

class plBDFConvertCallback
{
    public:
        virtual void    NumChars( uint16_t chars ) {}
        virtual void    CharDone() {}
};

//// Class Definition /////////////////////////////////////////////////////////

class plMipmap;
class plBDFHeaderParser;
class plBDFPropertiesParser;
class plBDFCharsParser;
class plFont : public hsKeyedObject
{
    public:

        enum RenderFlags
        {
            kRenderItalic       = 0x00000001,
            kRenderBold         = 0x00000002,
            kRenderScaleAA      = 0x00000004,
            kRenderClip         = 0x00000008,
            kRenderWrap         = 0x00000010,

            kRenderJustXLeft    = 0,
            kRenderJustXRight   = 0x00000020,   // X right justify
            kRenderJustXCenter  = 0x00000040,   // X center justify (left default)
            kRenderJustXForceLeft=0x00000080,   // Force left (no kerning)
            kRenderJustXMask    = 0x000000e0,

            kRenderJustYBaseline = 0,
            kRenderJustYTop     = 0x00000100,   // Y top justify
            kRenderJustYCenter  = 0x00000200,   // Y center justify
            kRenderJustYBottom  = 0x00000400,   // Y bottom justify (baseline is default)
            kRenderJustYMask    = 0x00000700,

            kRenderIntoAlpha    = 0x00000800,   // This option causes grayscale (AA) fonts to 
                                                // render into the alpha channel instead of the color
                                                // channel, so that the resulting pixels touched always
                                                // have the renderColor and the alpha = to the font pixel.
                                                // By default, we use the font pixel as an alpha blending
                                                // value between the renderColor and the destColor and
                                                // leave the alpha as-is
                                                // This flag has no effect on monochrome fonts
            kRenderAlphaPremultiplied = 0x00001000, // Destination has color values premultiplied by alpha
            kRenderShadow             = 0x00002000, // Render text shadows
        };

        enum Flags
        {
            kFlagBold       = 0x00000001,
            kFlagItalic     = 0x00000002
        };

    protected:

        friend class plBDFHeaderParser;
        friend class plBDFPropertiesParser;
        friend class plBDFCharsParser;

        // Font face and size. This is just used for IDing purposes, not for rendering
        ST::string  fFace;
        uint8_t     fSize;
        uint32_t    fFlags;

        // Size of the whole font bitmap. Fonts are stored vertically, one
        // character at a time, so fWidth is really the max width of any one
        // character bitmap, with of course the proper padding
        uint32_t  fWidth, fHeight;

        // Bpp of our font bitmap. We're grayscale, remember...
        uint8_t   fBPP;

        // Bitmap data!
        uint8_t   *fBMapData;

        // Our character class, for per-char info
        class plCharacter
        {
            public:
                uint32_t  fBitmapOff;     // Offset in the font bitmap in bytes 
                                        // to the first uint8_t of the character
                uint32_t  fHeight;        // Height in pixels of this character
                int32_t   fBaseline;      // Number of pixels down from the top of 
                                        // the char bitmap to the baseline.
                
                float    fLeftKern;  // Kerning values for this char, in pixels
                float    fRightKern; // Note that the right kern is relative to 
                                        // the right side of the bitmap area, which
                                        // is the width of the font bitmap, so
                                        // basically each character is the same width,
                                        // just kerned back!
                                        // (left kerning currently unsupported, just 
                                        // in here in case we need to eventually)

                plCharacter();
                void    Read(hsStream *s);
                void    Write(hsStream *s) const;
        };

        // First character we encode--everything below this we don't render
        uint16_t  fFirstChar;

        // Our characters, stored in a vector for easy construction
        std::vector<plCharacter> fCharacters;

        // Max character bitmap height and max ascent for any character
        uint32_t  fMaxCharHeight;
        int32_t   fFontAscent, fFontDescent;

        typedef void (plFont::*CharRenderFunc)( const plCharacter &c );

        // Render info
        class plRenderInfo
        {
            public:
                int16_t       fFirstLineIndent;
                int16_t       fX, fY, fNumCols, fFarthestX, fLastX, fLastY;
                int16_t       fMaxWidth, fMaxHeight, fMaxAscent, fMaxDescent;
                int16_t       fLineSpacing;
                uint8_t       *fDestPtr;
                uint32_t      fDestStride;
                uint8_t       fDestBPP;
                uint32_t      fColor;
                plMipmap    *fMipmap;
                uint32_t      fFlags;
                pcSmallRect fClipRect;
                float    fFloatWidth;

                const wchar_t   *fVolatileStringPtr;    // Just so we know where we clipped

                CharRenderFunc  fRenderFunc;
        };

        plRenderInfo    fRenderInfo;

        void    IClear( bool onConstruct = false );
        void    ICalcFontAscent();

        uint8_t   *IGetFreeCharData( uint32_t &newOffset );

        const plCharacter& IGetCharacter(wchar_t c) const;
        void    IRenderLoop( const wchar_t *string, int32_t maxCount );
        void    IRenderString( plMipmap *mip, uint16_t x, uint16_t y, const wchar_t *string, bool justCalc );

        // Various render functions
        void    IRenderChar1To32( const plCharacter &c );
        void    IRenderChar1To32AA( const plCharacter &c );
        void    IRenderChar8To32( const plCharacter &c );
        void    IRenderChar8To32Alpha( const plCharacter &c );
        void    IRenderChar8To32FullAlpha( const plCharacter &c );
        void    IRenderChar8To32AlphaPremultiplied( const plCharacter &c );
        void    IRenderChar8To32AlphaPremShadow( const plCharacter &c );
        void    IRenderCharNull( const plCharacter &c );

        uint32_t IGetCharPixel( const plCharacter &c, int32_t x, int32_t y )
        {
            // only for 8-bit characters
            return (x < 0 || y < 0 || (uint32_t)x >= fWidth || (uint32_t)y >= c.fHeight) ? 0 : *(fBMapData + c.fBitmapOff + y*fWidth + x);
        }

    public:

        plFont();
        virtual ~plFont();

        CLASSNAME_REGISTER( plFont );
        GETINTERFACE_ANY( plFont, hsKeyedObject );

        void    Read(hsStream *s, hsResMgr *mgr) override;
        void    Write(hsStream *s, hsResMgr *mgr) override;

        ST::string  GetFace() const { return fFace; }
        uint8_t     GetSize() const { return fSize; }
        uint16_t    GetFirstChar() const { return fFirstChar; }
        size_t      GetNumChars() const { return fCharacters.size(); }
        uint32_t    GetFlags() const { return fFlags; }
        float       GetDescent() const { return (float)fFontDescent; }
        float       GetAscent() const { return (float)fFontAscent; }

        uint32_t      GetBitmapWidth() const { return fWidth; }
        uint32_t      GetBitmapHeight() const { return fHeight; }
        uint8_t       GetBitmapBPP() const { return fBPP; }

        void    SetFace( const ST::string &face ) { fFace = face; }
        void    SetSize( uint8_t size ) { fSize = size; }
        void    SetFlags( uint32_t flags ) { fFlags = flags; }
        void    SetFlag( uint32_t flag, bool on ) { if( on ) fFlags |= flag; else fFlags &= ~flag; }
        bool    IsFlagSet( uint32_t flag ) { if( fFlags & flag ) return true; return false; }

        void    SetRenderColor( uint32_t color );
        void    SetRenderFlag( uint32_t flag, bool on ) { if( on ) fRenderInfo.fFlags |= flag; else fRenderInfo.fFlags &= ~flag; }
        bool    IsRenderFlagSet( uint32_t flag ) { return ( fRenderInfo.fFlags & flag ) ? true : false; }
        void    SetRenderClipRect( int16_t x, int16_t y, int16_t width, int16_t height );
        void    SetRenderXJustify( uint32_t j ) { fRenderInfo.fFlags &= ~kRenderJustXMask; fRenderInfo.fFlags |= j; }
        void    SetRenderYJustify( uint32_t j ) { fRenderInfo.fFlags &= ~kRenderJustYMask; fRenderInfo.fFlags |= j; }
        void    SetRenderFirstLineIndent( int16_t indent ) { fRenderInfo.fFirstLineIndent = indent; }
        void    SetRenderLineSpacing( int16_t spacing ) { fRenderInfo.fLineSpacing = spacing; }

        // Sets flags too
        void    SetRenderClipping( int16_t x, int16_t y, int16_t width, int16_t height );
        void    SetRenderWrapping( int16_t x, int16_t y, int16_t width, int16_t height );

        void    RenderString(plMipmap *mip, uint16_t x, uint16_t y, const ST::string &string, uint16_t *lastX = nullptr, uint16_t *lastY = nullptr);
        void    RenderString(plMipmap *mip, uint16_t x, uint16_t y, const wchar_t *string, uint16_t *lastX = nullptr, uint16_t *lastY = nullptr);

        uint16_t  CalcStringWidth( const ST::string &string );
        uint16_t  CalcStringWidth( const wchar_t *string );
        void    CalcStringExtents( const ST::string &string, uint16_t &width, uint16_t &height, uint16_t &ascent, uint16_t &lastX, uint16_t &lastY );
        void    CalcStringExtents( const wchar_t *string, uint16_t &width, uint16_t &height, uint16_t &ascent, uint32_t &firstClippedChar, uint16_t &lastX, uint16_t &lastY );

        bool    LoadFromFNT( const plFileName &path );
        bool    LoadFromFNTStream( hsStream *stream );

        bool    LoadFromBDF( const plFileName &path, plBDFConvertCallback *callback );
        bool    LoadFromBDFStream( hsStream *stream, plBDFConvertCallback *callback );

        bool    LoadFromP2FFile( const plFileName &path );

        bool    ReadRaw( hsStream *stream );
        bool    WriteRaw( hsStream *stream );
};

#endif // _plFont_h
