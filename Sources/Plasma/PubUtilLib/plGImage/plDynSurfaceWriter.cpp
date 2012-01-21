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

#include "HeadSpin.h"

#include "plDynSurfaceWriter.h"

#include "plDynamicTextMap.h"
#include "hsExceptions.h"

#include "hsMatrix44.h"
#include "plMessage/plDynamicTextMsg.h"
#include "pnKeyedObject/plKey.h"
#include "plProfile.h"
#include "plStatusLog/plStatusLog.h"
#include "plWinFontCache.h"


//// plWinSurface Helper Functions ////////////////////////////////////////////

#if HS_BUILD_FOR_WIN32

static uint32_t       sNumDCsAllocated;
static uint32_t       sNumBitmapsAllocated;

plDynSurfaceWriter::plWinSurface::plWinSurface()
{
    fDC = nil;
    fBitmap = nil;
    fFont = nil;
    fBits = nil;
    fTextColor = RGB( 255, 255, 255 );
    fWidth = fHeight = 0;

    fSaveNum = 0;
    fFontFace = nil;
    fFontSize = 0;
    fFontFlags = 0;
    fFontAntiAliasRGB = false;
    fFontBlockedRGB = false;
}

plDynSurfaceWriter::plWinSurface::~plWinSurface()
{
    Release();
}

void    plDynSurfaceWriter::plWinSurface::Allocate( uint16_t w, uint16_t h )
{
    int         i;
    BITMAPINFO  *bmi;


    Release();

    fWidth = w;
    fHeight = h;

    /// Initialize a bitmap info struct to describe our surface
    if( IBitsPerPixel() == 8 )
        bmi = (BITMAPINFO *)( new uint8_t[ sizeof( BITMAPINFOHEADER ) + sizeof( RGBQUAD ) * 256 ] );
    else
        bmi = new BITMAPINFO;

    memset( &bmi->bmiHeader, 0, sizeof( BITMAPINFOHEADER ) );
    bmi->bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
    bmi->bmiHeader.biWidth = (int)fWidth;
    bmi->bmiHeader.biHeight = -(int)fHeight;
    bmi->bmiHeader.biPlanes = 1;
    bmi->bmiHeader.biCompression = BI_RGB;
    bmi->bmiHeader.biBitCount = IBitsPerPixel();
    if( IBitsPerPixel() == 8 )
    {
        // Set up map for grayscale bitmap
        for( i = 0; i < 256; i++ )
        {
            bmi->bmiColors[ i ].rgbRed = i;
            bmi->bmiColors[ i ].rgbGreen = i;
            bmi->bmiColors[ i ].rgbBlue = i;
            bmi->bmiColors[ i ].rgbReserved = i;
        }
    }
    
    /// Create a screen-compatible DC
    fDC = CreateCompatibleDC( nil );
    if( fDC == nil )
    {
        char msg[ 256 ];
        FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nil, GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), msg, sizeof( msg ), nil );
        char *ret = strrchr( msg, '\n' );
        if( ret != nil )
            *ret = 0;

        plStatusLog::AddLineS( "pipeline.log", 0xffff0000, "Unable to allocate DC for dynamic text map (%s, %d DCs allocated already)", msg, sNumDCsAllocated );
        if (IBitsPerPixel() == 8 )
            delete [] bmi;
        else
            delete bmi;
        return;
    }
    sNumDCsAllocated++;

    /// Create a bitmap using the DC and the bitmapInfo struct we filled out
    fBitmap = CreateDIBSection( fDC, bmi, DIB_RGB_COLORS, (void **)&fBits, nil, 0 );
    if( fBitmap == nil )
    {
        char msg[ 256 ];
        FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nil, GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), msg, sizeof( msg ), nil );
        char *ret = strrchr( msg, '\n' );
        if( ret != nil )
            *ret = 0;

        plStatusLog::AddLineS( "pipeline.log", 0xffff0000, "Unable to allocate RGB DIB section for dynamic text map (%s, %d bitmaps allocated already)", msg, sNumBitmapsAllocated );
        if (IBitsPerPixel() == 8 )
            delete [] bmi;
        else
            delete bmi;
        return;
    }
    sNumBitmapsAllocated++;

    /// Set up some basic props
    SetMapMode( fDC, MM_TEXT );
    SetBkMode( fDC, TRANSPARENT );
    SetTextAlign( fDC, TA_TOP | TA_LEFT );

    fSaveNum = SaveDC( fDC );
    
    SelectObject( fDC, fBitmap );

    if (IBitsPerPixel() == 8 )
        delete [] bmi;
    else
        delete bmi;
}

void    plDynSurfaceWriter::plWinSurface::Release( void )
{
    if( fBitmap != nil )
        sNumBitmapsAllocated--;
    if( fDC != nil )
        sNumDCsAllocated--;

    if( fSaveNum != 0 )
        RestoreDC( fDC, fSaveNum );
    fSaveNum = 0;

    DeleteObject( fBitmap );
    DeleteDC( fDC );

    fDC = nil;
    fBitmap = nil;
    fFont = nil;
    fBits = nil;
    fWidth = fHeight = 0;

    delete [] fFontFace;
    fFontFace = nil;
    fFontSize = 0;
    fFontFlags = 0;
    fFontAntiAliasRGB = false;
    fFontBlockedRGB = false;
}

hsBool  plDynSurfaceWriter::plWinSurface::WillFit( uint16_t w, uint16_t h )
{
    if( fWidth >= w && fHeight >= h )
        return true;
    return false;
}

static int      SafeStrCmp( const char *str1, const char *str2 )
{
    if( str1 == nil && str2 == nil )
        return -1;
    if( str1 != nil && str2 != nil )
        return strcmp( str1, str2 );
    return -1;
}

hsBool  plDynSurfaceWriter::plWinSurface::FontMatches( const char *face, uint16_t size, uint8_t flags, hsBool aaRGB )
{
    if( SafeStrCmp( face, fFontFace ) == 0 && fFontSize == size && 
        fFontFlags == flags && fFontAntiAliasRGB == aaRGB )
        return true;
    
    return false;
}

void    plDynSurfaceWriter::plWinSurface::SetFont( const char *face, uint16_t size, uint8_t flags, hsBool aaRGB )
{
    delete [] fFontFace;
    fFontFace = ( face != nil ) ? hsStrcpy( face ) : nil;
    fFontSize = size;
    fFontFlags = flags;
    fFontAntiAliasRGB = aaRGB;

    bool hadAFont = false;
    if( fFont != nil )
    {
        hadAFont = true;
        plWinFontCache::GetInstance().FreeFont( fFont );    
        fFont = nil;
    }

    if( face == nil )
        return;

    bool    bold = ( fFontFlags & plDynSurfaceWriter::kFontBold ) ? true : false;
    bool    italic = ( fFontFlags & plDynSurfaceWriter::kFontItalic ) ? true : false;

    int nHeight = -MulDiv( size, GetDeviceCaps( fDC, LOGPIXELSY ), 72 );
    fFont = plWinFontCache::GetInstance().GetMeAFont( face, nHeight, bold ? FW_BOLD : FW_NORMAL, italic, 
                                                        fFontAntiAliasRGB ? ANTIALIASED_QUALITY : DEFAULT_QUALITY );
    if( fFont == nil && fFontAntiAliasRGB )
    {
        static bool warnedCantAntiAlias = false;

        // Creation of font failed; could be that we can't do anti-aliasing? Try not doing it...
        if( !warnedCantAntiAlias )
        {
            plStatusLog::AddLineS( "pipeline.log", "WARNING: Cannot allocate anti-aliased font. Falling back to non-anti-aliased. This will be the only warning" );
            warnedCantAntiAlias = true;
        }

        fFont = plWinFontCache::GetInstance().GetMeAFont( face, nHeight, bold ? FW_BOLD : FW_NORMAL, italic, 
                                                            fFontAntiAliasRGB ? ANTIALIASED_QUALITY : DEFAULT_QUALITY );
    }

    if( fFont == nil )
    {
        hsAssert( false, "Cannot create Windows font for plDynSurfaceWriter" );
        plStatusLog::AddLineS( "pipeline.log", "ERROR: Cannot allocate font for RGB surface! (face: %s, size: %d %s %s)", face, nHeight, bold ? "bold" : "", italic ? "italic" : "" );

        delete [] fFontFace;
        fFontFace = nil;
        fFontSize = 0;
        return;
    }

    if( SelectObject( fDC, fFont ) == nil && hadAFont )
    {
        char msg[ 256 ];
        FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nil, GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), msg, sizeof( msg ), nil );
        char *ret = strrchr( msg, '\n' );
        if( ret != nil )
            *ret = 0;

        plStatusLog::AddLineS( "pipeline.log", 0xffff0000, "SelectObject() FAILED (%s)", msg );
    }

}

#endif // BUILD_FOR_WIN32

//// StupidStatic /////////////////////////////////////////////////////////////

hsBool  plDynSurfaceWriter::fForceSharedSurfaces = false;
hsBool  plDynSurfaceWriter::fOSDetected = false;
hsBool  plDynSurfaceWriter::fOSCanShareSurfaces = false;

hsBool  plDynSurfaceWriter::CanHandleLotsOfThem( void )
{
    if( fOSDetected )
        return fOSCanShareSurfaces;

    fOSDetected = true;

#if HS_BUILD_FOR_WIN32
    OSVERSIONINFO   versionInfo;
    memset( &versionInfo, 0, sizeof( versionInfo ) );
    versionInfo.dwOSVersionInfoSize = sizeof( versionInfo );

    if( GetVersionEx( &versionInfo ) )
    {
        plStatusLog::AddLineS( "pipeline.log", "OS version detection results:" );
        plStatusLog::AddLineS( "pipeline.log", "   Version: %d.%d", versionInfo.dwMajorVersion, versionInfo.dwMinorVersion );
        plStatusLog::AddLineS( "pipeline.log", "   Build #: %d", versionInfo.dwBuildNumber );
        plStatusLog::AddLineS( "pipeline.log", "   Platform ID: %d", versionInfo.dwPlatformId );

        if( versionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT )
        {
            if( fForceSharedSurfaces )
            {
                plStatusLog::AddLineS( "pipeline.log", "Detected NT-based platform, but sharing surfaces due to override" );
                fOSCanShareSurfaces = false;
            }
            else
            {
                plStatusLog::AddLineS( "pipeline.log", "Detected NT-based platform, allowing separate surfaces" );
                fOSCanShareSurfaces = true;
            }
        }
        else
        {
            plStatusLog::AddLineS( "pipeline.log", "Detected non-NT-based platform: sharing surfaces" );
            fOSCanShareSurfaces = false;
        }
    }
    else
    {
        plStatusLog::AddLineS( "pipeline.log", "OS version detection failed" );
        fOSCanShareSurfaces = false;
    }

#endif
    return fOSCanShareSurfaces;
}

//// Constructor & Destructor /////////////////////////////////////////////////

plDynSurfaceWriter::plDynSurfaceWriter()
{
    IInit();
}

plDynSurfaceWriter::~plDynSurfaceWriter()
{
    Reset();
}

plDynSurfaceWriter::plDynSurfaceWriter( plDynamicTextMap *target, uint32_t flags )
{
    IInit();
    fFlags = flags;
    SwitchTarget( target );
}

void    plDynSurfaceWriter::IInit( void )
{
    fCurrTarget = 0;
    fJustify = kLeftJustify;
    fFlags = 0;
    fFlushed = true;
    fFontFace = nil;
    fFontSize = 0;
    fFontBlockedRGB = false;
}

//// Reset ////////////////////////////////////////////////////////////////////

void    plDynSurfaceWriter::Reset( void )
{
#if HS_BUILD_FOR_WIN32
    fRGBSurface.Release();
    fAlphaSurface.Release();
#endif
    fCurrTarget = nil;
    fFlushed = true;

    delete [] fFontFace;
    fFontFace = nil;
    fFontSize = 0;
}


///////////////////////////////////////////////////////////////////////////////
//// Target Switching /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// FlushToTarget ////////////////////////////////////////////////////////////
//  Flushes all ops to the target.

void    plDynSurfaceWriter::FlushToTarget( void )
{
    int     x, y;


    if( fCurrTarget != nil && !fFlushed )
    {
#if HS_BUILD_FOR_WIN32
        // Flush the GDI so we can grab the bits
        GdiFlush();

        uint32_t *destBits = (uint32_t *)fCurrTarget->GetImage();

        // Are we merging in the alpha bits?
        if( fFlags & kSupportAlpha )
        {
            // Yup, munge 'em
            uint32_t  *srcRGBBits = fRGBSurface.GetBits();
            uint8_t   *srcAlphaBits = fAlphaSurface.GetBits();
            uint32_t  destWidth = fCurrTarget->GetWidth();

            for( y = 0; y < fCurrTarget->GetHeight(); y++ )
            {
                for( x = 0; x < destWidth; x++ )
                    destBits[ x ] = ( srcRGBBits[ x ] & 0x00ffffff ) | ( (uint32_t)srcAlphaBits[ x ] << 24 );

                destBits += destWidth;
                srcRGBBits += fRGBSurface.fWidth;
                srcAlphaBits += fAlphaSurface.fWidth;
            }
        }
        else
        {
            // Nope, just a 24-bit copy and set alphas to ff
            uint32_t  *srcBits = fRGBSurface.GetBits();
            uint32_t  destWidth = fCurrTarget->GetWidth();

            for( y = 0; y < fCurrTarget->GetHeight(); y++ )
            {
                memcpy( destBits, srcBits, destWidth * sizeof( uint32_t ) );

                // Fill in 0xff
                for( x = 0; x < destWidth; x++ )
                    destBits[ x ] |= 0xff000000;

                destBits += destWidth;
                srcBits += fRGBSurface.fWidth;
            }
        }
#endif
    }
    fFlushed = true;
}

//// SwitchTarget /////////////////////////////////////////////////////////////
//  Switches targets. Will flush to old target before switching. Also, if 
//  kDiscard isn't specified, will copy contents of new target to working 
//  surface.

void    plDynSurfaceWriter::SwitchTarget( plDynamicTextMap *target )
{
    if( target == fCurrTarget )
        return;

    if( !fFlushed )
        FlushToTarget();

    fCurrTarget = target;
    fFlushed = true;        // Will force a copy next IEnsureSurfaceUpdated()

    // Make sure our surfaces fit
    bool hadToAllocate = false;
#if HS_BUILD_FOR_WIN32
    if( target != nil )
    {
        if( !fRGBSurface.WillFit( (uint16_t)(target->GetWidth()), (uint16_t)(target->GetHeight()) ) )
        {
            fRGBSurface.Allocate( (uint16_t)(target->GetWidth()), (uint16_t)(target->GetHeight()) );
            hadToAllocate = true;
        }

        if( fFlags & kSupportAlpha )
        {
            if( !fAlphaSurface.WillFit( (uint16_t)(target->GetWidth()), (uint16_t)(target->GetHeight()) ) ) 
            {
                fAlphaSurface.Allocate( (uint16_t)(target->GetWidth()), (uint16_t)(target->GetHeight()) );
                hadToAllocate = true;
            }
        }
    }
    else
    {
        fRGBSurface.Release();
        fAlphaSurface.Release();
        hadToAllocate = true;
    }
#endif

    if( hadToAllocate )
    {
        delete [] fFontFace;
        fFontFace = nil;
        fFontSize = 0;
        fFontFlags = 0;
    }
}

//// IEnsureSurfaceUpdated ////////////////////////////////////////////////////
//  Makes sure our surfaces are ready to write to.

void    plDynSurfaceWriter::IEnsureSurfaceUpdated( void )
{
    uint32_t  x, y;


    // If we're flushed, then we haven't drawn since the last flush,
    // which means we want to copy from our target before we start drawing.
    // If we've already drawn, then we won't be flushed and we don't want
    // to be copying over what we've already drawn
    if( fCurrTarget != nil && fFlushed )
    {
        uint32_t *srcBits = (uint32_t *)fCurrTarget->GetImage();

#if HS_BUILD_FOR_WIN32
        // Are we merging in the alpha bits?
        if( fFlags & kSupportAlpha )
        {
            // Yup, de-munge 'em
            uint32_t  *destRGBBits = fRGBSurface.GetBits();
            uint8_t   *destAlphaBits = fAlphaSurface.GetBits();
            uint32_t  srcWidth = fCurrTarget->GetWidth();

            for( y = 0; y < fCurrTarget->GetHeight(); y++ )
            {
                for( x = 0; x < srcWidth; x++ )
                {
                    destRGBBits[ x ] = srcBits[ x ];    // Windows GDI probably doesn't care about the alpha bits here. Hopefully...
                    destAlphaBits[ x ] = (uint8_t)(srcBits[ x ] >> 24);
                }

                srcBits += srcWidth;
                destRGBBits += fRGBSurface.fWidth;
                destAlphaBits += fAlphaSurface.fWidth;
            }
        }
        else
        {
            // Nope, just do a straight memcopy
            uint32_t  *destBits = fRGBSurface.GetBits();
            uint32_t  srcWidth = fCurrTarget->GetWidth();

            for( y = 0; y < fCurrTarget->GetHeight(); y++ )
            {
                memcpy( destBits, srcBits, srcWidth * sizeof( uint32_t ) );

                srcBits += srcWidth;
                destBits += fRGBSurface.fWidth;
            }
        }       
#endif

        // ALSO, we need to re-update our settings, since different targets
        // can have different fonts or justifications
        ISetFont( fCurrTarget->GetFontFace(), fCurrTarget->GetFontSize(), 0/*fCurrTarget->GetWriterFontFlags()*/, fCurrTarget->GetFontAARGB() );
        SetJustify( (Justify)fCurrTarget->GetFontJustify() );
        hsColorRGBA col = fCurrTarget->GetFontColor();
        ISetTextColor( col, fCurrTarget->GetFontBlockRGB() );

        fFlushed = false;
    }
}

hsBool  plDynSurfaceWriter::IsValid( void ) const
{
    if( fCurrTarget == nil )
        return false;

#if HS_BUILD_FOR_WIN32
    if( fRGBSurface.fDC == nil || fRGBSurface.fBitmap == nil )
        return false;

    if( ( fFlags & kSupportAlpha ) && ( fAlphaSurface.fDC == nil || fAlphaSurface.fBitmap == nil ) )
        return false;
#endif

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//// Rendering Functions //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// SetBitsFromBuffer ////////////////////////////////////////////////////////

/*
void    plDynSurfaceWriter::SetBitsFromBuffer( uint32_t *clearBuffer, uint16_t width, uint16_t height )
{
    int         x, y;
    uint32_t      *data = (uint32_t *)fImage, *srcData = clearBuffer;
    uint8_t       *destAlpha = nil;


    if( !IsValid() )
        return;

#if HS_BUILD_FOR_WIN32
    GdiFlush();
#endif

    // Clear *all* to zero
    memset( data, 0, fWidth * fHeight * sizeof( uint32_t ) );
    if( fHasAlpha )
    {
#if HS_BUILD_FOR_WIN32
        memset( fWinAlphaBits, 0, fWidth * fHeight );
        destAlpha = fWinAlphaBits;
#endif
    }

    // Buffer is of size fVisWidth x fVisHeight, so we need a bit of work to do this right
    for( y = 0; y < fVisHeight; y++ )
    {
        for( x = 0; x < fVisWidth; x++ )
        {
            data[ x ] = srcData[ x ];
        }

        if( destAlpha != nil )
        {
            for( x = 0; x < fVisWidth; x++ )
                destAlpha[ x ] = srcData[ x ] >> 24;

            destAlpha += fWidth;
        }

        data += fWidth;
        srcData += fVisWidth;
    }
}
*/

//// ClearToColor /////////////////////////////////////////////////////////////

void    plDynSurfaceWriter::ClearToColor( hsColorRGBA &color )
{
    if( !IsValid() )
        return;

    IEnsureSurfaceUpdated();

#if HS_BUILD_FOR_WIN32

    uint32_t      i, hexColor = color.ToARGB32();


    // Flush the GDI first, so it doesn't decide to overwrite us later
    GdiFlush();

    uint32_t *rgbBits = fRGBSurface.GetBits();
    for( i = 0; i < fRGBSurface.fWidth * fRGBSurface.fHeight; i++ )
        rgbBits[ i ] = hexColor;

    if( fFlags & kSupportAlpha )
    {
        uint8_t *alphaBits = fAlphaSurface.GetBits(), alpha = (uint8_t)(hexColor >> 24);

        for( i = 0; i < fAlphaSurface.fWidth * fAlphaSurface.fHeight; i++ )
            alphaBits[ i ] = alpha;
    }
#endif
}

//// SetFont //////////////////////////////////////////////////////////////////
//  OS-specific. Load the given font for drawing the text with.

void    plDynSurfaceWriter::SetFont( const char *face, uint16_t size, uint8_t fontFlags, hsBool antiAliasRGB )
{
    if( !IsValid() )
        return;

    IEnsureSurfaceUpdated();

    ISetFont( face, size, fontFlags, antiAliasRGB );
}

//// ISetFont /////////////////////////////////////////////////////////////////

void    plDynSurfaceWriter::ISetFont( const char *face, uint16_t size, uint8_t fontFlags, hsBool antiAliasRGB )
{
    fFlags = ( fFlags & ~kFontShadowed ) | ( fontFlags & kFontShadowed );

#if HS_BUILD_FOR_WIN32
    if( !fRGBSurface.FontMatches( face, size, fontFlags, antiAliasRGB ) )
        fRGBSurface.SetFont( face, size, fontFlags, antiAliasRGB );

    if( fFlags & kSupportAlpha )
    {
        if( !fAlphaSurface.FontMatches( face, size, fontFlags, !antiAliasRGB ) )
            fAlphaSurface.SetFont( face, size, fontFlags, !antiAliasRGB );
    }
#endif
}

//// SetTextColor /////////////////////////////////////////////////////////////
//  blockRGB basically forces the RGB channel to write in blocks instead of
//  actual characters. This isn't useful unless you're relying on the alpha
//  channel to do the text (opaque text and transparent background), in which
//  case you want plenty of block color in your RGB channel because it'll get
//  alpha-ed out by the alpha channel.

void    plDynSurfaceWriter::SetTextColor( hsColorRGBA &color, hsBool blockRGB )
{
    if( !IsValid() )
        return;

    IEnsureSurfaceUpdated();
    ISetTextColor( color, blockRGB );
}

//// IRefreshTextColor ////////////////////////////////////////////////////////

void    plDynSurfaceWriter::ISetTextColor( hsColorRGBA &color, hsBool blockRGB )
{
#if HS_BUILD_FOR_WIN32

    int r = (int)(color.r * 255.f);
    int g = (int)(color.g * 255.f);
    int b = (int)(color.b * 255.f);

    fRGBSurface.fTextColor = RGB( r, g, b );
    if( fFlags & kSupportAlpha )
    {
        int a = (int)(color.a * 255.f);
        fAlphaSurface.fTextColor = RGB( a, a, a );
    }

    fFontBlockedRGB = blockRGB;

    if( fFontBlockedRGB && !( fFlags & kFontShadowed ) )
    {
        ::SetBkColor( fRGBSurface.fDC, fRGBSurface.fTextColor );
        ::SetBkMode( fRGBSurface.fDC, OPAQUE );
    }
    else
        ::SetBkMode( fRGBSurface.fDC, TRANSPARENT );

    ::SetTextColor( fRGBSurface.fDC, fRGBSurface.fTextColor );
    
    if( fFlags & kSupportAlpha )
        ::SetTextColor( fAlphaSurface.fDC, fAlphaSurface.fTextColor );
#endif
}

//// SetJustify ///////////////////////////////////////////////////////////////

void    plDynSurfaceWriter::SetJustify( Justify j )
{
    fJustify = j;
}

//// IRefreshOSJustify ////////////////////////////////////////////////////////

void    plDynSurfaceWriter::IRefreshOSJustify( void )
{
    if( !IsValid() )
        return;

#if HS_BUILD_FOR_WIN32
    uint32_t justMode;
    switch( fJustify )
    {
        case kLeftJustify:  justMode = TA_LEFT; break;
        case kCenter:       justMode = TA_CENTER; break;
        case kRightJustify: justMode = TA_RIGHT; break;
    }
    ::SetTextAlign( fRGBSurface.fDC, justMode );
    if( fFlags & kSupportAlpha )
        ::SetTextAlign( fAlphaSurface.fDC, justMode );
#endif
}

//// DrawString ///////////////////////////////////////////////////////////////

void    plDynSurfaceWriter::DrawString( uint16_t x, uint16_t y, const char *text )
{
    if( !IsValid() )
        return;

    IEnsureSurfaceUpdated();

    IRefreshOSJustify();

#if HS_BUILD_FOR_WIN32
    if( fFlags & kFontShadowed )
    {
        ::SetTextColor( fRGBSurface.fDC, RGB( 0, 0, 0 ) );
        ::TextOut( fRGBSurface.fDC, x + 1, y + 1, text, strlen( text ) );

        ::SetTextColor( fRGBSurface.fDC, fRGBSurface.fTextColor );
        ::TextOut( fRGBSurface.fDC, x, y, text, strlen( text ) );

        if( fFlags & kSupportAlpha )
        {
            ::TextOut( fAlphaSurface.fDC, x + 1, y + 1, text, strlen( text ) );
            ::TextOut( fAlphaSurface.fDC, x, y, text, strlen( text ) );
        }
    }
    else
    {
        ::TextOut( fRGBSurface.fDC, x, y, text, strlen( text ) );
        if( fFlags & kSupportAlpha )
            ::TextOut( fAlphaSurface.fDC, x, y, text, strlen( text ) );
    }

#endif
}

//// DrawClippedString ////////////////////////////////////////////////////////

void    plDynSurfaceWriter::DrawClippedString( int16_t x, int16_t y, const char *text, uint16_t width, uint16_t height )
{
    if( !IsValid() )
        return;

    IEnsureSurfaceUpdated();

    IRefreshOSJustify();

#if HS_BUILD_FOR_WIN32
    
    RECT    r;
    ::SetRect( &r, x, y, x + width, y + height );

    if( fJustify == kRightJustify )
        x += width - 1;
    else if( fJustify == kCenter )
        x += width >> 1;

    if( fFlags & kFontShadowed )
    {
        ::SetTextColor( fRGBSurface.fDC, RGB( 0, 0, 0 ) );

        ::OffsetRect( &r, 1, 1 );
        ::ExtTextOut( fRGBSurface.fDC, x + 1, y + 1, ETO_CLIPPED, &r, text, strlen( text ), nil );
        if( fFlags & kSupportAlpha )
            ::ExtTextOut( fAlphaSurface.fDC, x + 1, y + 1, ETO_CLIPPED, &r, text, strlen( text ), nil );
        ::OffsetRect( &r, -1, -1 );

        ::SetTextColor( fRGBSurface.fDC, fRGBSurface.fTextColor );
    }

    ::ExtTextOut( fRGBSurface.fDC, x, y, ETO_CLIPPED, &r, text, strlen( text ), nil );
    if( fFlags & kSupportAlpha )
        ::ExtTextOut( fAlphaSurface.fDC, x, y, ETO_CLIPPED, &r, text, strlen( text ), nil );
    
#endif
}

//// DrawClippedString ////////////////////////////////////////////////////////

void    plDynSurfaceWriter::DrawClippedString( int16_t x, int16_t y, const char *text, uint16_t clipX, uint16_t clipY, uint16_t width, uint16_t height )
{
    if( !IsValid() )
        return;

    IEnsureSurfaceUpdated();

    IRefreshOSJustify();

#if HS_BUILD_FOR_WIN32
    
    RECT    r;
    ::SetRect( &r, clipX, clipY, clipX + width, clipY + height );

    if( fFlags & kFontShadowed )
    {
        ::SetTextColor( fRGBSurface.fDC, RGB( 0, 0, 0 ) );

        ::OffsetRect( &r, 1, 1 );
        ::ExtTextOut( fRGBSurface.fDC, x + 1, y + 1, ETO_CLIPPED, &r, text, strlen( text ), nil );
        if( fFlags & kSupportAlpha )
            ::ExtTextOut( fAlphaSurface.fDC, x + 1, y + 1, ETO_CLIPPED, &r, text, strlen( text ), nil );
        ::OffsetRect( &r, -1, -1 );

        ::SetTextColor( fRGBSurface.fDC, fRGBSurface.fTextColor );
    }

    ::ExtTextOut( fRGBSurface.fDC, x, y, ETO_CLIPPED, &r, text, strlen( text ), nil );
    if( fFlags & kSupportAlpha )
        ::ExtTextOut( fAlphaSurface.fDC, x, y, ETO_CLIPPED, &r, text, strlen( text ), nil );

#endif
}

//// DrawWrappedString ////////////////////////////////////////////////////////

void    plDynSurfaceWriter::DrawWrappedString( uint16_t x, uint16_t y, const char *text, uint16_t width, uint16_t height )
{
    if( !IsValid() )
        return;

    IEnsureSurfaceUpdated();

#if HS_BUILD_FOR_WIN32
    
    RECT    r;
    ::SetRect( &r, x, y, x + width, y + height );

    UINT    format = DT_TOP | DT_NOPREFIX | DT_WORDBREAK;
    switch( fJustify )
    {
        case kLeftJustify:  format |= DT_LEFT; break;
        case kCenter:       format |= DT_CENTER; break;
        case kRightJustify: format |= DT_RIGHT; break;
    }

    if( fFlags & kFontShadowed )
    {
        ::SetTextColor( fRGBSurface.fDC, RGB( 0, 0, 0 ) );

        ::OffsetRect( &r, 1, 1 );
        ::DrawText( fRGBSurface.fDC, text, strlen( text ), &r, format );
        if( fFlags & kSupportAlpha )
            ::DrawText( fAlphaSurface.fDC, text, strlen( text ), &r, format );
        ::OffsetRect( &r, -1, -1 );

        ::SetTextColor( fRGBSurface.fDC, fRGBSurface.fTextColor );
    }

    ::DrawText( fRGBSurface.fDC, text, strlen( text ), &r, format );
    if( fFlags & kSupportAlpha )
        ::DrawText( fAlphaSurface.fDC, text, strlen( text ), &r, format );

#endif
}

//// CalcStringWidth //////////////////////////////////////////////////////////

uint16_t      plDynSurfaceWriter::CalcStringWidth( const char *text, uint16_t *height )
{
    if( !IsValid() )
        return 0;

    IEnsureSurfaceUpdated();

#if HS_BUILD_FOR_WIN32

    SIZE size;
    ::GetTextExtentPoint32( fRGBSurface.fDC, text, strlen( text ), &size );

    if( height != nil )
        *height = (uint16_t)size.cy;

    return (uint16_t)size.cx;
#endif
}

//// CalcWrappedStringSize ////////////////////////////////////////////////////

void    plDynSurfaceWriter::CalcWrappedStringSize( const char *text, uint16_t *width, uint16_t *height )
{
    if( !IsValid() )
        return;

    IEnsureSurfaceUpdated();

#if HS_BUILD_FOR_WIN32

    RECT    r;
    ::SetRect( &r, 0, 0, *width, 0 );

    UINT    format = DT_TOP | DT_NOPREFIX | DT_WORDBREAK | DT_CALCRECT;
    switch( fJustify )
    {
        case kLeftJustify:  format |= DT_LEFT; break;
        case kCenter:       format |= DT_CENTER; break;
        case kRightJustify: format |= DT_RIGHT; break;
    }

    ::DrawText( fRGBSurface.fDC, text, strlen( text ), &r, format );

    *width = (uint16_t)(r.right);
    if( height != nil )
        *height = (uint16_t)r.bottom;
#endif
}

//// FillRect /////////////////////////////////////////////////////////////////

void    plDynSurfaceWriter::FillRect( uint16_t x, uint16_t y, uint16_t width, uint16_t height, hsColorRGBA &color )
{
    if( !IsValid() )
        return;

    IEnsureSurfaceUpdated();

#if HS_BUILD_FOR_WIN32

    RECT    rc;
    ::SetRect( &rc, x, y, x + width, y + height );

    int r = (int)(color.r * 255.f);
    int g = (int)(color.g * 255.f);
    int b = (int)(color.b * 255.f);
    int a = (int)(color.a * 255.f);

    HBRUSH brush = ::CreateSolidBrush( RGB( r, g, b ) );
    ::FillRect( fRGBSurface.fDC, &rc, brush );
    ::DeleteObject( brush );

    if( fFlags & kSupportAlpha )
    {
        brush = ::CreateSolidBrush( RGB( a, a, a ) );
        ::FillRect( fAlphaSurface.fDC, &rc, brush );
        ::DeleteObject( brush );
    }

#endif
}

//// FrameRect ////////////////////////////////////////////////////////////////

void    plDynSurfaceWriter::FrameRect( uint16_t x, uint16_t y, uint16_t width, uint16_t height, hsColorRGBA &color )
{
    if( !IsValid() )
        return;

    IEnsureSurfaceUpdated();

#if HS_BUILD_FOR_WIN32

    RECT    rc;
    ::SetRect( &rc, x, y, x + width, y + height );

    int r = (int)(color.r * 255.f);
    int g = (int)(color.g * 255.f);
    int b = (int)(color.b * 255.f);
    int a = (int)(color.a * 255.f);

    HBRUSH brush = ::CreateSolidBrush( RGB( r, g, b ) );
    ::FrameRect( fRGBSurface.fDC, &rc, brush );
    ::DeleteObject( brush );

    if( fFlags & kSupportAlpha )
    {
        brush = ::CreateSolidBrush( RGB( a, a, a ) );
        ::FrameRect( fAlphaSurface.fDC, &rc, brush );
        ::DeleteObject( brush );
    }

#endif
}

/*
//// DrawImage ////////////////////////////////////////////////////////////////

void    plDynSurfaceWriter::DrawImage( uint16_t x, uint16_t y, plMipmap *image, hsBool respectAlpha )
{
    if( !IsValid() )
        return;

#if HS_BUILD_FOR_WIN32
    // Flush the GDI first, to make sure it's done
    GdiFlush();
#endif

    plMipmap::CompositeOptions  opts( respectAlpha ? 0 : plMipmap::kForceOpaque );
    if( !fHasAlpha )
        opts.fFlags = plMipmap::kCopySrcAlpha;  // Don't care, this is fastest

    Composite( image, x, y, &opts );

    /// HACK for now, since the alpha in the mipmap gets copied straight into the
    /// 32-bit color buffer, but our separate hacked alpha buffer hasn't been updated
    if( fHasAlpha && !respectAlpha )
    {
        HBRUSH brush = ::CreateSolidBrush( RGB( 255, 255, 255 ) );
        RECT rc;
        ::SetRect( &rc, x, y, x + image->GetWidth(), y + image->GetHeight() );
        ::FillRect( fWinAlphaDC, &rc, brush );
        ::DeleteObject( brush );
    }
}

//// DrawClippedImage /////////////////////////////////////////////////////////

void    plDynSurfaceWriter::DrawClippedImage( uint16_t x, uint16_t y, plMipmap *image, 
                                            uint16_t srcClipX, uint16_t srcClipY, 
                                            uint16_t srcClipWidth, uint16_t srcClipHeight, 
                                            hsBool respectAlpha )
{
    if( !IsValid() )
        return;

#if HS_BUILD_FOR_WIN32
    // Flush the GDI first, to make sure it's done
    GdiFlush();
#endif

    plMipmap::CompositeOptions  opts( respectAlpha ? 0 : plMipmap::kForceOpaque );
    if( !fHasAlpha )
        opts.fFlags = plMipmap::kCopySrcAlpha;  // Don't care, this is fastest

    opts.fSrcClipX = srcClipX;
    opts.fSrcClipY = srcClipY;
    opts.fSrcClipWidth = srcClipWidth;
    opts.fSrcClipHeight = srcClipHeight;
    Composite( image, x, y, &opts );

    /// HACK for now, since the alpha in the mipmap gets copied straight into the
    /// 32-bit color buffer, but our separate hacked alpha buffer hasn't been updated
    if( fHasAlpha && !respectAlpha )
    {
        HBRUSH brush = ::CreateSolidBrush( RGB( 255, 255, 255 ) );
        RECT rc;
        ::SetRect( &rc, x, y, x + ( srcClipWidth > 0 ? srcClipWidth : image->GetWidth() ),
                            y + ( srcClipHeight > 0 ? srcClipHeight : image->GetHeight() ) );
        ::FillRect( fWinAlphaDC, &rc, brush );
        ::DeleteObject( brush );
    }
}
*/
