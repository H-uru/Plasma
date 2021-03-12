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
//  plDynamicTextMap Class Functions                                         //
//  Derived bitmap class representing a single mipmap.                       //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  6.7.2001 mcn - Created.                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "plDynamicTextMap.h"

#include "hsStream.h"
#include "hsExceptions.h"

#include "hsMatrix44.h"
#include "hsGDeviceRef.h"
#include "plMessage/plDynamicTextMsg.h"
#include "pnKeyedObject/plKey.h"
#include "plProfile.h"
#include "plFont.h"
#include "plFontCache.h"
#include "plResMgr/plLocalization.h"


plProfile_CreateMemCounter("DynaTextMem", "PipeC", DynaTextMem);
plProfile_CreateCounterNoReset("DynaTexts", "PipeC", DynaTexts);
plProfile_Extern(MemMipmaps);


//// Constructor & Destructor /////////////////////////////////////////////////

plDynamicTextMap::plDynamicTextMap()
    : fVisWidth(0), fVisHeight(0), fHasAlpha(false), fPremultipliedAlpha(false), fJustify(kLeftJustify),
      fInitBuffer(), fFontSize(), fFontFlags(), fShadowed(), fLineSpacing(),
      fCurrFont(), fFontAntiAliasRGB(), fFontBlockRGB(), fHasCreateBeenCalled()
{
    fFontColor.Set(0, 0, 0, 1);
}

plDynamicTextMap::~plDynamicTextMap()
{
    Reset();
}

plDynamicTextMap::plDynamicTextMap( uint32_t width, uint32_t height, bool hasAlpha, uint32_t extraWidth, uint32_t extraHeight, bool premultipliedAlpha )
    : fInitBuffer(nullptr)
{
    Create( width, height, hasAlpha, extraWidth, extraHeight, premultipliedAlpha );
}

//// SetNoCreate //////////////////////////////////////////////////////////////
//  For export time, we want to set up the config to write to disk, but we
//  don't want to actually be creating OS surfaces. So we call this function
//  instead, which does just that. It basically does all the setup work that
//  Create() does, or enough for us to write out later.

void    plDynamicTextMap::SetNoCreate( uint32_t width, uint32_t height, bool hasAlpha )
{
    // OK, so it really isn't that much work...
    fVisWidth = (uint16_t)width;
    fVisHeight = (uint16_t)height;
    fHasAlpha = hasAlpha;
    fImage = nullptr;       // So we know we haven't actually done anything yet
    delete [] fInitBuffer;
    fInitBuffer = nullptr;
}

//// Create ///////////////////////////////////////////////////////////////////

void    plDynamicTextMap::Create( uint32_t width, uint32_t height, bool hasAlpha, uint32_t extraWidth, uint32_t extraHeight, bool premultipliedAlpha )
{
    SetConfig( hasAlpha ? kARGB32Config : kRGB32Config );


    fVisWidth = width;
    fVisHeight = height;
    fHasAlpha = hasAlpha;
    fPremultipliedAlpha = premultipliedAlpha;

    for( fWidth = 1; fWidth < width + extraWidth; fWidth <<= 1 );
    for( fHeight = 1; fHeight < height + extraHeight; fHeight <<= 1 );

    // instead of allocating the fImage here, we'll wait for the first draw operation to be called (in IIsValid)
    fHasCreateBeenCalled = true;

    fRowBytes = fWidth << 2;
    fNumLevels = 1;
    fFlags |= plMipmap::kDontThrowAwayImage | plMipmap::kAutoGenMipmap;
    fCompressionType = plMipmap::kUncompressed;
    fUncompressedInfo.fType = plMipmap::UncompressedInfo::kRGB8888;

    // Destroy the old texture ref, if we have one. This should force the 
    // pipeline to recreate one more suitable for our use
    SetDeviceRef(nullptr);

    // Some init color
    SetFont( "Arial", 12 );
    hsColorRGBA color;
    color.Set( 0,0,1,1);
    SetTextColor( color );

    SetCurrLevel( 0 );
    plProfile_Inc(DynaTexts);

}

//// Reset ////////////////////////////////////////////////////////////////////

void    plDynamicTextMap::Reset()
{
    IDestroyOSSurface();

    plMipmap::Reset();

    // they need to call create again to undo the affects of call Reset()
    fHasCreateBeenCalled = false;

    delete [] fInitBuffer;
    fInitBuffer = nullptr;

    fFontFace = ST::string();

    // Destroy the old texture ref, since we're no longer using it
    SetDeviceRef(nullptr);
}

///////////////////////////////////////////////////////////////////////////////
//// OS-Specific Functions ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool        plDynamicTextMap::IIsValid()
{
    if (GetImage() == nullptr && fHasCreateBeenCalled)
    {
        // we are going to allocate the fImage at this point... when someone is looking for it
        fImage = (void *)IAllocateOSSurface( (uint16_t)fWidth, (uint16_t)fHeight );
        hsColorRGBA color;
        if (fInitBuffer != nullptr)
        {
            IClearFromBuffer( fInitBuffer );
        }
        else
        {
            color.Set( 0.f, 0.f, 0.f, 1.f );
            ClearToColor( color );
            FlushToHost();
        }
        IBuildLevelSizes();
        fTotalSize = GetLevelSize( 0 );
        SetCurrLevel( 0 );
        // Destroy the old texture ref, if we have one. This should force the 
        // pipeline to recreate one more suitable for our use
        SetDeviceRef(nullptr);
        plProfile_NewMem(MemMipmaps, fTotalSize);
        plProfile_NewMem(DynaTextMem, fTotalSize);
#ifdef MEMORY_LEAK_TRACER
        IAddToMemRecord( this, plRecord::kViaCreate );
#endif
    }

    if (GetImage() == nullptr)
        return false;

    return true;//fWriter->IsValid();
}

// allow the user of the DynaTextMap that they are done with the image... for now
// ... the fImage will be re-created on the next operation that requires the image
void plDynamicTextMap::PurgeImage()
{
    IDestroyOSSurface();
    fTotalSize = 0;
    SetCurrLevel( 0 );
    // Destroy the old texture ref, if we have one. This should force the 
    // pipeline to recreate one more suitable for our use
    SetDeviceRef(nullptr);
}

//// IAllocateOSSurface ///////////////////////////////////////////////////////
//  OS-specific. Allocates a rectangular bitmap of the given dimensions that
//  the OS can draw text into. Returns a pointer to the pixels.

uint32_t* plDynamicTextMap::IAllocateOSSurface( uint16_t width, uint16_t height )
{
    uint32_t* pixels = new uint32_t[ width * height ];
    return pixels;
}

//// IDestroyOSSurface ////////////////////////////////////////////////////////
//  Opposite of allocate. DUH!

void    plDynamicTextMap::IDestroyOSSurface()
{
#ifdef MEMORY_LEAK_TRACER
    if (fImage != nullptr)
        IRemoveFromMemRecord( (uint8_t *)fImage );
#endif

    delete[] (uint32_t*)fImage;
    fImage = nullptr;

    plProfile_Dec(DynaTexts);
    plProfile_DelMem(DynaTextMem, fTotalSize);
    plProfile_DelMem(MemMipmaps, fTotalSize);
}

///////////////////////////////////////////////////////////////////////////////
//// Virtual Functions ////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// Read /////////////////////////////////////////////////////////////////////

uint32_t  plDynamicTextMap::Read( hsStream *s )
{
    uint32_t totalRead = plBitmap::Read( s );

    // The funny thing is that we don't read anything like a mipmap; we just
    // keep the width and height and call Create() after we read those in

    fVisWidth = s->ReadLE32();
    fVisHeight = s->ReadLE32();
    fHasAlpha = s->ReadBool();
    totalRead += 2 * 4;

    uint32_t initSize = s->ReadLE32();
    totalRead += 4;
    if( initSize > 0 )
    {
        fInitBuffer = new uint32_t[ initSize ];

        s->ReadLE32( initSize, fInitBuffer );
        totalRead += initSize * 4;
    }
    else
        fInitBuffer = nullptr;

    Create( fVisWidth, fVisHeight, fHasAlpha );
    
    delete [] fInitBuffer;
    fInitBuffer = nullptr;

    return totalRead;
}

//// Write ////////////////////////////////////////////////////////////////////

uint32_t  plDynamicTextMap::Write( hsStream *s )
{
    uint32_t totalWritten = plBitmap::Write( s );

    s->WriteLE32( fVisWidth );
    s->WriteLE32( fVisHeight );
    s->WriteBool( fHasAlpha );

    s->WriteLE32(fInitBuffer != nullptr ? uint32_t(fVisWidth * fVisHeight * sizeof(uint32_t)) : 0U);
    if (fInitBuffer != nullptr)
    {
        s->WriteLE32( fVisWidth * fVisHeight, fInitBuffer );
    }

    return totalWritten;
}

///////////////////////////////////////////////////////////////////////////////
//// Some More Functions //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// SetInitBuffer ////////////////////////////////////////////////////////////
//  Sets an initial buffer, which is written to disk and read in to use for
//  initializing the color buffer upon creation. If not specified, we init to
//  black. ASSUMES the buffer is of dimensions fVisWidth x fVisHeight.

void    plDynamicTextMap::SetInitBuffer( uint32_t *buffer )
{
    delete [] fInitBuffer;
    if (buffer == nullptr)
    {
        fInitBuffer = nullptr;
        return;
    }

    fInitBuffer = new uint32_t[ fVisWidth * fVisHeight ];
    memcpy( fInitBuffer, buffer, fVisWidth * fVisHeight * sizeof( uint32_t ) );
}

//// CopyFrom /////////////////////////////////////////////////////////////////

void plDynamicTextMap::CopyFrom(const plMipmap *source)
{
    hsAssert( false, "Copying plDynamicTextMaps is not supported." );
}

//// Clone ////////////////////////////////////////////////////////////////////

plMipmap *plDynamicTextMap::Clone() const
{
    static bool alreadyWarned = false;

    if( !alreadyWarned )
    {
        hsAssert( false, "Cloning plDynamicTextMaps is not supported." );
        alreadyWarned = true;
    }

    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//// Rendering Functions //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// IClearFromBuffer /////////////////////////////////////////////////////////

void    plDynamicTextMap::IClearFromBuffer( uint32_t *clearBuffer )
{
    int         y;
    uint32_t      *data = (uint32_t *)fImage, *srcData = clearBuffer;


    if( !IIsValid() )
        return;

    // Clear *all* to zero
    memset( data, 0, fWidth * fHeight * sizeof( uint32_t ) );

    // Buffer is of size fVisWidth x fVisHeight, so we need a bit of work to do this right
    for( y = 0; y < fVisHeight; y++ )
    {
        memcpy( data, srcData, fVisWidth * sizeof( uint32_t ) );
        data += fWidth;
        srcData += fVisWidth;
    }
}

//// IPropagateFlags //////////////////////////////////////////////////////////

void    plDynamicTextMap::IPropagateFlags()
{
    SetJustify(fJustify);
    fCurrFont->SetRenderFlag(plFont::kRenderShadow, fFontFlags & kFontShadowed);
    fCurrFont->SetRenderFlag(plFont::kRenderIntoAlpha, fFontBlockRGB);
    fCurrFont->SetRenderFlag(plFont::kRenderAlphaPremultiplied, fPremultipliedAlpha);
    fCurrFont->SetRenderColor(fFontColor.ToARGB32());
}

//// ClearToColor /////////////////////////////////////////////////////////////

void    plDynamicTextMap::ClearToColor( hsColorRGBA &color )
{
    if( !IIsValid() )
        return;

    uint32_t      i, hex = fPremultipliedAlpha ? color.ToARGB32Premultiplied() : color.ToARGB32();
    uint32_t      *data = (uint32_t *)fImage;

    // Buffer is of size fVisWidth x fVisHeight, so we need a bit of work to do this right
    for( i = 0; i < fHeight * fWidth; i++ )
        data[ i ] = hex;
}

//// SetJustify ///////////////////////////////////////////////////////////////

void    plDynamicTextMap::SetJustify( Justify j )
{
// ===> Don't need to validate creation
//  if( !IIsValid() )
//      return;

    fJustify = j; 
    switch( fJustify )
    {
        case kLeftJustify:  fCurrFont->SetRenderXJustify( plFont::kRenderJustXForceLeft ); break;
        case kCenter:       fCurrFont->SetRenderXJustify( plFont::kRenderJustXCenter ); break;
        case kRightJustify: fCurrFont->SetRenderXJustify( plFont::kRenderJustXRight ); break;
    }
}

//// SetFont //////////////////////////////////////////////////////////////////

void    plDynamicTextMap::SetFont( const ST::string &face, uint16_t size, uint8_t fontFlags, bool antiAliasRGB )
{
// ===> Don't need to validate creation
//  if( !IIsValid() )
//      return;

    if (plLocalization::UsingUnicode())
    {
        // unicode has a bunch of chars that most fonts don't have, so we override the font choice with one
        // that will work with the desired language
        hsStatusMessageF("We are using a unicode language, overriding font choice of %s", face.c_str("nil"));
        fFontFace = "Unicode";
    }
    else
        fFontFace = face;
    fFontSize = size;
    fFontFlags = fontFlags;
    fFontAntiAliasRGB = antiAliasRGB;

    fCurrFont = plFontCache::GetInstance().GetFont( fFontFace, (uint8_t)fFontSize,
                        ( ( fFontFlags & kFontBold ) ? plFont::kFlagBold : 0 ) | 
                        ( ( fFontFlags & kFontItalic ) ? plFont::kFlagItalic : 0 ) );
    if (fCurrFont == nullptr)
    {
        if (!fCurrFont)
            hsStatusMessageF("Font missing - %s. Using Arial", fFontFace.c_str("nil"));

        fFontFace = "Arial";
        // lets try again with Arial
        fCurrFont = plFontCache::GetInstance().GetFont( fFontFace, (uint8_t)fFontSize,
                            ( ( fFontFlags & kFontBold ) ? plFont::kFlagBold : 0 ) | 
                            ( ( fFontFlags & kFontItalic ) ? plFont::kFlagItalic : 0 ) );
    }

    // This will be nil if we're just running the page optimizer.
    if (fCurrFont)
    {
        if (fFontFlags & kFontShadowed)
            fCurrFont->SetRenderFlag(plFont::kRenderShadow, true);
        fCurrFont->SetRenderYJustify( plFont::kRenderJustYTop );
        SetJustify( fJustify );
    }
}

//// SetLineSpacing ///////////////////////////////////////////////////////////

void    plDynamicTextMap::SetLineSpacing( int16_t spacing )
{
// ===> Don't need to validate creation
//  if( !IIsValid() )
//      return;

    fLineSpacing = spacing;
    fCurrFont->SetRenderLineSpacing(spacing);
}

//// SetTextColor /////////////////////////////////////////////////////////////

void    plDynamicTextMap::SetTextColor( hsColorRGBA &color, bool blockRGB )
{
// ===> Don't need to validate creation
//  if( !IIsValid() )
//      return;

    fFontColor = color;
    fFontBlockRGB = blockRGB;

    if (fCurrFont)
        fCurrFont->SetRenderColor( fFontColor.ToARGB32() );
}

//// DrawString ///////////////////////////////////////////////////////////////

void    plDynamicTextMap::DrawString( uint16_t x, uint16_t y, const char *text )
{
    wchar_t *wText = hsStringToWString(text);
    DrawString(x,y,wText);
    delete [] wText;
}

void    plDynamicTextMap::DrawString( uint16_t x, uint16_t y, const wchar_t *text )
{
    if( !IIsValid() )
        return;

    IPropagateFlags();
    fCurrFont->SetRenderFlag( plFont::kRenderWrap | plFont::kRenderClip, false );
	fCurrFont->SetRenderClipRect( 0, 0, fVisWidth, fVisHeight );
    fCurrFont->RenderString( this, x, y, text );
}

//// DrawClippedString ////////////////////////////////////////////////////////

void    plDynamicTextMap::DrawClippedString( int16_t x, int16_t y, const ST::string &text, uint16_t width, uint16_t height )
{
    // TEMP
    DrawClippedString(x, y, text.to_wchar().data(), width, height);
}

void    plDynamicTextMap::DrawClippedString( int16_t x, int16_t y, const wchar_t *text, uint16_t width, uint16_t height )
{
    if( !IIsValid() )
        return;

    IPropagateFlags();
    fCurrFont->SetRenderClipping( x, y, width, height );
    fCurrFont->RenderString( this, x, y, text );
}

//// DrawClippedString ////////////////////////////////////////////////////////

void    plDynamicTextMap::DrawClippedString( int16_t x, int16_t y, const ST::string &text, uint16_t clipX, uint16_t clipY, uint16_t width, uint16_t height )
{
    // TEMP
    DrawClippedString(x, y, text.to_wchar().data(), width, height);
}

void    plDynamicTextMap::DrawClippedString( int16_t x, int16_t y, const wchar_t *text, uint16_t clipX, uint16_t clipY, uint16_t width, uint16_t height )
{
    if( !IIsValid() )
        return;

    IPropagateFlags();
    fCurrFont->SetRenderClipping( clipX, clipY, width, height );
    fCurrFont->RenderString( this, x, y, text );
}

//// DrawWrappedString ////////////////////////////////////////////////////////

void    plDynamicTextMap::DrawWrappedString( uint16_t x, uint16_t y, const ST::string &text, uint16_t width, uint16_t height, uint16_t *lastX, uint16_t *lastY )
{
    // TEMP
    DrawWrappedString(x, y, text.to_wchar().data(), width, height, lastX, lastY);
}

void    plDynamicTextMap::DrawWrappedString( uint16_t x, uint16_t y, const wchar_t *text, uint16_t width, uint16_t height, uint16_t *lastX, uint16_t *lastY )
{
    if( !IIsValid() )
        return;

    IPropagateFlags();
    fCurrFont->SetRenderWrapping( x, y, width, height );
    fCurrFont->RenderString( this, x, y, text, lastX, lastY );
}

//// CalcStringWidth //////////////////////////////////////////////////////////

uint16_t      plDynamicTextMap::CalcStringWidth( const ST::string &text, uint16_t *height )
{
    // TEMP
    return CalcStringWidth(text.to_wchar().data(), height);
}

uint16_t      plDynamicTextMap::CalcStringWidth( const wchar_t *text, uint16_t *height )
{
// ===> Don't need to validate creation
//  if( !IIsValid() )
//      return 0;

    SetJustify( fJustify );
    uint16_t w, h, a, lastX, lastY;
    uint32_t firstClipped;
    fCurrFont->SetRenderFlag( plFont::kRenderClip | plFont::kRenderWrap, false );
    fCurrFont->CalcStringExtents( text, w, h, a, firstClipped, lastX, lastY );
    if (height != nullptr)
        *height = h;
    return w;
}

//// SetFirstLineIndent ///////////////////////////////////////////////////////

void    plDynamicTextMap::SetFirstLineIndent( int16_t indent )
{
// ===> Don't need to validate creation
//  if( !IIsValid() )
//      return;

    fCurrFont->SetRenderFirstLineIndent( indent );
}

//// CalcWrappedStringSize ////////////////////////////////////////////////////

void    plDynamicTextMap::CalcWrappedStringSize( const ST::string &text, uint16_t *width, uint16_t *height, uint32_t *firstClippedChar, uint16_t *maxAscent, uint16_t *lastX, uint16_t *lastY )
{
    // TEMP
    CalcWrappedStringSize(text.to_wchar().data(), width, height, firstClippedChar, maxAscent, lastX, lastY);
}

void    plDynamicTextMap::CalcWrappedStringSize( const wchar_t *text, uint16_t *width, uint16_t *height, uint32_t *firstClippedChar, uint16_t *maxAscent, uint16_t *lastX, uint16_t *lastY )
{
// ===> Don't need to validate creation
//  if( !IIsValid() )
//      return;

    SetJustify( fJustify );
    uint16_t w, h, a, lX, lY;
    uint32_t firstClipped;
    fCurrFont->SetRenderWrapping( 0, 0, *width, *height );
    fCurrFont->CalcStringExtents( text, w, h, a, firstClipped, lX, lY );
    *width = w;
    *height = h;
    if (firstClippedChar != nullptr)
        *firstClippedChar = firstClipped;
    if (maxAscent != nullptr)
        *maxAscent = a;
    if (lastX != nullptr)
        *lastX = lX;
    if (lastY != nullptr)
        *lastY = lY;
}

//// FillRect /////////////////////////////////////////////////////////////////

void    plDynamicTextMap::FillRect( uint16_t x, uint16_t y, uint16_t width, uint16_t height, hsColorRGBA &color )
{
    if( !IIsValid() )
        return;

    if( x + width > fWidth )
        width = (uint16_t)(fWidth - x);

    // Gee, how hard can it REALLY be?
    uint32_t i, hex = fPremultipliedAlpha ? color.ToARGB32Premultiplied() : color.ToARGB32();
    height += y;
    if( height > fHeight )
        height = (uint16_t)fHeight;
    for( ; y < height; y++ )
    {
        uint32_t *destPtr = GetAddr32( x, y );
        for( i = 0; i < width; i++ )
            destPtr[ i ] = hex;
    }
}

//// FrameRect ////////////////////////////////////////////////////////////////

void    plDynamicTextMap::FrameRect( uint16_t x, uint16_t y, uint16_t width, uint16_t height, hsColorRGBA &color )
{
    if( !IIsValid() )
        return;

    if( x + width > fWidth )
        width = (uint16_t)(fWidth - x);
    if( y + height > fHeight )
        height = (uint16_t)(fHeight - y);

    // Shouldn't be much harder
    uint32_t i, hex = fPremultipliedAlpha ? color.ToARGB32Premultiplied() : color.ToARGB32();
    uint32_t *dest1, *dest2;

    dest1 = GetAddr32( x, y );
    dest2 = GetAddr32( x, y + height - 1 );
    for( i = 0; i < width; i++ )
        dest1[ i ] = dest2[ i ] = hex;

    for( i = 0; i < height; i++ )
    {
        dest1[ 0 ] = dest1[ width - 1 ] = hex;
        dest1 += fWidth;
    }
}

//// DrawImage ////////////////////////////////////////////////////////////////

void    plDynamicTextMap::DrawImage( uint16_t x, uint16_t y, plMipmap *image, DrawMethods method )
{
    if( !IIsValid() )
        return;

    plMipmap::CompositeOptions  opts;
    if( method == kImgNoAlpha )
    {
        if( fHasAlpha )
            opts.fFlags = plMipmap::kForceOpaque;
        else
            opts.fFlags = plMipmap::kCopySrcAlpha; // Don't care, this is fastest
    }
    else if( method == kImgBlend )
        opts.fFlags = 0;        // Default opts
    else if( method == kImgSprite )
        opts.fFlags = plMipmap::kCopySrcAlpha;

    if( fPremultipliedAlpha )
        opts.fFlags |= plMipmap::kDestPremultiplied;

    Composite( image, x, y, &opts );

    /// HACK for now, since the alpha in the mipmap gets copied straight into the
    /// 32-bit color buffer, but our separate hacked alpha buffer hasn't been updated
/*  if( fHasAlpha && !respectAlpha )
    {
        HBRUSH brush = ::CreateSolidBrush( RGB( 255, 255, 255 ) );
        RECT rc;
        ::SetRect( &rc, x, y, x + image->GetWidth(), y + image->GetHeight() );
        ::FillRect( fWinAlphaDC, &rc, brush );
        ::DeleteObject( brush );
    }
*/
}

//// DrawClippedImage /////////////////////////////////////////////////////////

void    plDynamicTextMap::DrawClippedImage( uint16_t x, uint16_t y, plMipmap *image, 
                                            uint16_t srcClipX, uint16_t srcClipY, 
                                            uint16_t srcClipWidth, uint16_t srcClipHeight, 
                                            DrawMethods method )
{
    if( !IIsValid() )
        return;

    plMipmap::CompositeOptions  opts;
    if( method == kImgNoAlpha )
    {
        if( fHasAlpha )
            opts.fFlags = plMipmap::kForceOpaque;
        else
            opts.fFlags = plMipmap::kCopySrcAlpha; // Don't care, this is fastest
    }
    else if( method == kImgBlend )
        opts.fFlags = 0;        // Default opts
    else if( method == kImgSprite )
        opts.fFlags = plMipmap::kCopySrcAlpha;

    if( fPremultipliedAlpha )
        opts.fFlags |= plMipmap::kDestPremultiplied;

    opts.fSrcClipX = srcClipX;
    opts.fSrcClipY = srcClipY;
    opts.fSrcClipWidth = srcClipWidth;
    opts.fSrcClipHeight = srcClipHeight;
    Composite( image, x, y, &opts );

    /// HACK for now, since the alpha in the mipmap gets copied straight into the
    /// 32-bit color buffer, but our separate hacked alpha buffer hasn't been updated
/*  if( fHasAlpha && !respectAlpha )
    {
        HBRUSH brush = ::CreateSolidBrush( RGB( 255, 255, 255 ) );
        RECT rc;
        ::SetRect( &rc, x, y, x + ( srcClipWidth > 0 ? srcClipWidth : image->GetWidth() ),
                            y + ( srcClipHeight > 0 ? srcClipHeight : image->GetHeight() ) );
        ::FillRect( fWinAlphaDC, &rc, brush );
        ::DeleteObject( brush );
    }
*/
}

//// FlushToHost //////////////////////////////////////////////////////////////

void    plDynamicTextMap::FlushToHost()
{
    if( !IIsValid() )
        return;

    // Dirty the mipmap's deviceRef, if there is one
    if (GetDeviceRef() != nullptr)
        GetDeviceRef()->SetDirty( true );
}

//// GetLayerTransform ////////////////////////////////////////////////////////
//  Since the textGen can actually create a texture bigger than you were expecting,
//  you want to be able to apply a layer texture transform that will compensate. This
//  function will give you that transform. Just feed it into plLayer->SetTransform().

hsMatrix44  plDynamicTextMap::GetLayerTransform()
{
    hsMatrix44  xform;
    hsVector3   scale((float)GetVisibleWidth() / (float)GetWidth(),
                      (float)GetVisibleHeight() / (float)GetHeight(), 1.f);

    xform.MakeScaleMat( &scale );
    return xform;
}

//// MsgReceive ///////////////////////////////////////////////////////////////

bool    plDynamicTextMap::MsgReceive( plMessage *msg )
{
    plDynamicTextMsg    *textMsg = plDynamicTextMsg::ConvertNoRef( msg );
    if (textMsg != nullptr)
    {
        if( textMsg->fCmd & plDynamicTextMsg::kClear )
            ClearToColor( textMsg->fClearColor );

        if( textMsg->fCmd & plDynamicTextMsg::kSetTextColor )
            SetTextColor( textMsg->fColor, textMsg->fBlockRGB );

        if( (textMsg->fCmd & plDynamicTextMsg::kSetFont ) && !textMsg->fString.empty())
            SetFont( textMsg->fString, textMsg->fX, (uint8_t)(textMsg->fFlags) );
        
        if( textMsg->fCmd & plDynamicTextMsg::kSetLineSpacing )
            SetLineSpacing( textMsg->fLineSpacing );

        if( textMsg->fCmd & plDynamicTextMsg::kSetJustify )
            SetJustify( (Justify)textMsg->fFlags );

        if( textMsg->fCmd & plDynamicTextMsg::kFillRect )
            FillRect( textMsg->fLeft, textMsg->fTop, textMsg->fRight - textMsg->fLeft + 1,
                        textMsg->fBottom - textMsg->fTop + 1, textMsg->fColor );

        if( textMsg->fCmd & plDynamicTextMsg::kFrameRect )
            FrameRect( textMsg->fLeft, textMsg->fTop, textMsg->fRight - textMsg->fLeft + 1,
                        textMsg->fBottom - textMsg->fTop + 1, textMsg->fColor );

        if( (textMsg->fCmd & plDynamicTextMsg::kDrawString ) && !textMsg->fString.empty())
            DrawString( textMsg->fX, textMsg->fY, textMsg->fString.to_wchar().data() );

        if( (textMsg->fCmd & plDynamicTextMsg::kDrawClippedString ) && !textMsg->fString.empty())
            DrawClippedString( textMsg->fX, textMsg->fY, textMsg->fString.to_wchar().data(),
                                textMsg->fLeft, textMsg->fTop, textMsg->fRight - textMsg->fLeft + 1,
                                textMsg->fBottom - textMsg->fTop + 1 );

        if( (textMsg->fCmd & plDynamicTextMsg::kDrawWrappedString ) && !textMsg->fString.empty())
            DrawWrappedString( textMsg->fX, textMsg->fY, textMsg->fString.to_wchar().data(), textMsg->fRight, textMsg->fBottom );

        if( textMsg->fCmd & plDynamicTextMsg::kDrawImage )
        {
            plMipmap *mip = plMipmap::ConvertNoRef(textMsg->fImageKey ? textMsg->fImageKey->ObjectIsLoaded() : nullptr);
            if (mip != nullptr)
                DrawImage( textMsg->fX, textMsg->fY, mip, textMsg->fFlags ? kImgBlend : kImgNoAlpha );
        }

        if( textMsg->fCmd & plDynamicTextMsg::kDrawClippedImage )
        {
            plMipmap *mip = plMipmap::ConvertNoRef(textMsg->fImageKey ? textMsg->fImageKey->ObjectIsLoaded() : nullptr);
            if (mip != nullptr)
                DrawClippedImage( textMsg->fX, textMsg->fY, mip, textMsg->fLeft, textMsg->fTop, 
                                textMsg->fRight, textMsg->fBottom, textMsg->fFlags ? kImgBlend : kImgNoAlpha );
        }

        if( textMsg->fCmd & plDynamicTextMsg::kFlush )
            FlushToHost();

        if( textMsg->fCmd & plDynamicTextMsg::kPurgeImage )
            PurgeImage();

        return true;
    }

    return plMipmap::MsgReceive( msg );
}

//// Swap /////////////////////////////////////////////////////////////////////
//  Swapping is an evil little trick. It's also something that should probably
//  be exposed at the mipmap level eventually, but there's no need for it yet.
//  Basically, it lets you take the contents of two DTMaps and swap them, as
//  if you had swapped pointers, but you didn't. This is so you can, well, swap
//  DTMaps without swapping pointers! (Like, say, you don't have access to them)

#define SWAP_ME( Type, a, b ) { Type temp; temp = a; a = b; b = temp; }

void    plDynamicTextMap::Swap( plDynamicTextMap *other )
{
    // We only do this if the two are the same size, color depth, etc
    if( other->GetWidth() != GetWidth() || other->GetHeight() != GetHeight() ||
        other->GetPixelSize() != GetPixelSize() )
        return;

    // Swap image pointers
    void *ptr = other->fImage;
    other->fImage = fImage;
    fImage = ptr;

    // Invalidate both device refs (don't risk swapping THOSE)
    if (GetDeviceRef() != nullptr)
        GetDeviceRef()->SetDirty( true );
    if (other->GetDeviceRef() != nullptr)
        other->GetDeviceRef()->SetDirty( true );

    // Swap DTMap info
    SWAP_ME( bool, fHasAlpha, other->fHasAlpha );
    SWAP_ME( bool, fPremultipliedAlpha, other->fPremultipliedAlpha );
    SWAP_ME( bool, fShadowed, other->fShadowed );

    SWAP_ME( Justify, fJustify, other->fJustify );
    SWAP_ME( ST::string, fFontFace, other->fFontFace );
    SWAP_ME( uint16_t, fFontSize, other->fFontSize );
    SWAP_ME( uint8_t, fFontFlags, other->fFontFlags );
    SWAP_ME( bool, fFontAntiAliasRGB, other->fFontAntiAliasRGB );
    SWAP_ME( hsColorRGBA, fFontColor, other->fFontColor );
    SWAP_ME( bool, fFontBlockRGB, other->fFontBlockRGB );

    SWAP_ME( bool, fFontBlockRGB, other->fFontBlockRGB );
    SWAP_ME( bool, fFontBlockRGB, other->fFontBlockRGB );
    SWAP_ME( bool, fFontBlockRGB, other->fFontBlockRGB );

    SWAP_ME( plFont *, fCurrFont, other->fCurrFont );
    SWAP_ME( uint32_t *, fInitBuffer, other->fInitBuffer );
}
