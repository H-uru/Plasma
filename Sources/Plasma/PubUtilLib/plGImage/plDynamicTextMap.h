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
//  plDynamicTextMap Class Header                                            //
//  I derived from plMipmap not because I inherit a lot of the functionality,//
//  but because this acts very very much like a mipmap with one mip level.   //
//  The only difference is that the actual data gets generated on the fly,   //
//  instead of converted at export time. However, to the outside world,      //
//  we're just a plain old mipmap, and I'd like to keep it that way.         //
//                                                                           //
//  Note that we are also of a fixed format--ARGB8888. Keeps things nice and //
//  simple that way.                                                         //
//                                                                           //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  1.14.2002 mcn - Created.                                                 //
//  10.28.2002 mcn - Changing the arrangement a bit. Now we have a single    //
//                   writable "creation" surface that actually generates     //
//                   data, which then copies out on a flush to the actual    //
//                   mipmap data. This is slower because it requires two     //
//                   copies (second one when we prepare to write to a new    //
//                   surface, unless kDiscard is specified as a flush option)//
//                   but it lets us allocate only one OS surface, which saves//
//                   us on Win98/ME where we're very limited in terms of     //
//                   available OS surfaces.                                  //
//                   To facilitate this, we create a new abstract class to   //
//                   encapsulate the actual GDI functionality of Windows.    //
//                   This way, we have two options when creating DTMaps:     //
//                   allocate an OS writer per surface, which lets us avoid  //
//                   the copy problem mentioned above, or allocate one to    //
//                   share. This will also let us optimize by switching to   //
//                   non-shared writers once we write a non-OS-reliant       //
//                   writer/text renderer.                                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plDynamicTextMap_h
#define _plDynamicTextMap_h

#include "plMipmap.h"
#include "hsColorRGBA.h"

struct hsMatrix44;

//// Class Definition /////////////////////////////////////////////////////////

class plFont;
class plDynamicTextMap : public plMipmap
{
    protected:

        uint32_t      fVisWidth, fVisHeight;

        uint32_t  Read(hsStream *s) override;
        uint32_t  Write(hsStream *s) override;

    public:
        //// Public Flags ////

        enum Justify
        {
            kLeftJustify = 0,
            kCenter,
            kRightJustify
        };

        //// Public Data /////
        
        
        //// Public Members ////


        plDynamicTextMap();
        plDynamicTextMap( uint32_t width, uint32_t height, bool hasAlpha = false, uint32_t extraWidth = 0, uint32_t extraHeight = 0, bool premultipliedAlpha = false );
        virtual ~plDynamicTextMap();

        CLASSNAME_REGISTER( plDynamicTextMap );
        GETINTERFACE_ANY( plDynamicTextMap, plMipmap );


        void            Create( uint32_t width, uint32_t height, bool hasAlpha, uint32_t extraWidth = 0, uint32_t extraHeight = 0, bool premultipliedAlpha = false );
        void            SetNoCreate( uint32_t width, uint32_t height, bool hasAlpha );

        void    Reset() override;

        void    Read(hsStream *s, hsResMgr *mgr) override { hsKeyedObject::Read(s, mgr); this->Read(s); }
        void    Write(hsStream *s, hsResMgr *mgr) override { hsKeyedObject::Write(s, mgr); this->Write(s); }

        uint8_t   GetNumLevels() const override { return 1; }

        void        Colorize() override { }
        plMipmap    *Clone() const override;
        void        CopyFrom(const plMipmap *source) override;


        /// Operations to perform on the text block

        bool    IsValid() { return IIsValid(); }

        // allow the user of the DynaTextMap that they are done with the image... for now
        // ... the fImage will be re-created on the next operation that requires the image
        void    PurgeImage();

        void    ClearToColor( hsColorRGBA &color );

        enum FontFlags
        {
            kFontBold       = 0x01,
            kFontItalic     = 0x02,
            kFontShadowed   = 0x04
        };

        void    SetFont( const ST::string &face, uint16_t size, uint8_t fontFlags = 0, bool antiAliasRGB = true );
        void    SetLineSpacing( int16_t spacing );
        void    SetTextColor( hsColorRGBA &color, bool blockRGB = false );
        void    SetJustify( Justify j );

        void    DrawString( uint16_t x, uint16_t y, const wchar_t *text );
        void    DrawClippedString( int16_t x, int16_t y, const ST::string &text, uint16_t width, uint16_t height );
        void    DrawClippedString( int16_t x, int16_t y, const wchar_t *text, uint16_t width, uint16_t height );
        void    DrawClippedString( int16_t x, int16_t y, const ST::string &text, uint16_t clipX, uint16_t clipY, uint16_t width, uint16_t height );
        void    DrawClippedString( int16_t x, int16_t y, const wchar_t *text, uint16_t clipX, uint16_t clipY, uint16_t width, uint16_t height );
        void    DrawWrappedString(uint16_t x, uint16_t y, const ST::string &text, uint16_t width, uint16_t height, uint16_t *lastX = nullptr, uint16_t *lastY = nullptr);
        void    DrawWrappedString(uint16_t x, uint16_t y, const wchar_t *text, uint16_t width, uint16_t height, uint16_t *lastX = nullptr, uint16_t *lastY = nullptr);
        uint16_t  CalcStringWidth(const ST::string &text, uint16_t *height = nullptr);
        uint16_t  CalcStringWidth(const wchar_t *text, uint16_t *height = nullptr);
        void    CalcWrappedStringSize(const ST::string &text, uint16_t *width, uint16_t *height, uint32_t *firstClippedChar = nullptr, uint16_t *maxAscent = nullptr, uint16_t *lastX = nullptr, uint16_t *lastY = nullptr);
        void    CalcWrappedStringSize(const wchar_t *text, uint16_t *width, uint16_t *height, uint32_t *firstClippedChar = nullptr, uint16_t *maxAscent = nullptr, uint16_t *lastX = nullptr, uint16_t *lastY = nullptr);
        void    FillRect( uint16_t x, uint16_t y, uint16_t width, uint16_t height, hsColorRGBA &color );
        void    FrameRect( uint16_t x, uint16_t y, uint16_t width, uint16_t height, hsColorRGBA &color );
        void    SetFirstLineIndent( int16_t indent );

        enum DrawMethods
        {
            kImgNoAlpha,        // Just copy color data, force alpha to full if present
            kImgBlend,          // Blend color onto dest using src alpha, keep dest alpha
            kImgSprite          // Copy color data and alphas
        };
        void    DrawImage( uint16_t x, uint16_t y, plMipmap *image, DrawMethods method = kImgNoAlpha );
        void    DrawClippedImage( uint16_t x, uint16_t y, plMipmap *image, uint16_t srcClipX, uint16_t srcClipY, 
                                uint16_t srcClipWidth, uint16_t srcClipHeight, DrawMethods method = kImgNoAlpha );

        void    FlushToHost();

        bool    MsgReceive(plMessage *msg) override;

        uint32_t  GetVisibleWidth() const { return fVisWidth; }
        uint32_t  GetVisibleHeight() const { return fVisHeight; }

        // Since the dynamic text can actually create a texture bigger than you were expecting,
        // you want to be able to apply a layer texture transform that will compensate. This
        // function will give you that transform. Just feed it into plLayer->SetTransform().

        hsMatrix44  GetLayerTransform();

        void    SetInitBuffer( uint32_t *buffer );

        // Gets for font values
        Justify     GetFontJustify() const { return fJustify; }
        ST::string  GetFontFace() const { return fFontFace; }
        uint16_t    GetFontSize() const { return fFontSize; }
        bool        GetFontAARGB() const { return fFontAntiAliasRGB; }
        hsColorRGBA GetFontColor() const { return fFontColor; }
        bool        GetFontBlockRGB() const { return fFontBlockRGB; }
        int16_t       GetLineSpacing() const { return fLineSpacing; }

        plFont      *GetCurrFont() const { return fCurrFont; }

        virtual void    Swap( plDynamicTextMap *other );

    protected:

        //// Protected Members ////

        bool        IIsValid();
        void        IClearFromBuffer( uint32_t *clearBuffer );

        uint32_t      *IAllocateOSSurface( uint16_t width, uint16_t height );
        void        IDestroyOSSurface();

        void        IPropagateFlags();

        bool        fHasAlpha, fPremultipliedAlpha, fShadowed;

        Justify     fJustify;
        ST::string  fFontFace;
        uint16_t    fFontSize;
        uint8_t     fFontFlags;
        bool        fFontAntiAliasRGB;
        hsColorRGBA fFontColor;
        bool        fFontBlockRGB;
        int16_t       fLineSpacing;

        plFont      *fCurrFont;

        uint32_t      *fInitBuffer;
        
        bool        fHasCreateBeenCalled;
};


#endif // _plDynamicTextMap_h
