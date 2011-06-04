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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
///////////////////////////////////////////////////////////////////////////////
//																			 //
//	plTextGenerator Class Functions											 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	12.13.2001 mcn - Created.  												 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#include "hsWindows.h"
#include "hsTypes.h"
#include "hsMatrix44.h"
#include "../pnKeyedObject/hsKeyedObject.h"
#include "plTextGenerator.h"
#include "../plGImage/plMipmap.h"
#include "../plPipeline/hsGDeviceRef.h"
#include "../pnMessage/plRefMsg.h"

#include "plgDispatch.h"
#include "hsResMgr.h"

// Because tempKeys haven't been fixed yet (mf says to blame Eric Ellis), reffing
// objects when we have a tempKey (or they have a tempKey) just don't work. In
// fact, it will do nasty things like crashing on shutdown. Until then, we simply
// won't do the refs. Note that this is BAD, but given the only time we currently
// use these objects are very limited, controlled cases that *should* be okay
// for now, we should be reasonably safe. For now.
//#define MCN_DO_REFS

//// Constructor & Destructor /////////////////////////////////////////////////

plTextGenerator::plTextGenerator()
{
	fHost = nil;
}

plTextGenerator::plTextGenerator( plMipmap *host, UInt16 width, UInt16 height )
{
	fHost = nil;
	Attach( host, width, height );
}

plTextGenerator::~plTextGenerator()
{
	// This also won't work until tempKeys work, since the mipmap will be gone by 
	// this time, in which case, calling Detach() crashes
#ifdef MCN_DO_REFS
	Detach();
#endif
}

//// Attach ///////////////////////////////////////////////////////////////////
//	Grab onto a plMipmap, suck the texture out of it and replace it with our
//	own.		

void	plTextGenerator::Attach( plMipmap *host, UInt16 width, UInt16 height )
{
	UInt16		textWidth, textHeight;


	hsAssert( fHost == nil, "Attempting to attach an already attached plTextGenerator" );

	fHost = host;

	/// Suck the old texture data out
	fHost->Reset();

	/// Make some new

	// Note that we need POW-2 textures, so we go for the next one up that will
	// fit what we need
	for( textWidth = 1; textWidth < width; textWidth <<= 1 );
	for( textHeight = 1; textHeight < height; textHeight <<= 1 );

	fWidth = width;
	fHeight = height;
	fHost->fImage = (void *)IAllocateOSSurface( textWidth, textHeight );
	fHost->SetConfig( plMipmap::kARGB32Config );
	fHost->fWidth = textWidth;
	fHost->fHeight = textHeight;
	fHost->fPixelSize = 32;
	fHost->fRowBytes = textWidth * 4;
	fHost->fNumLevels = 1;
	fHost->fFlags |= plMipmap::kUserOwnsBitmap | plMipmap::kDontThrowAwayImage;
	fHost->fCompressionType = plMipmap::kUncompressed;
	fHost->fUncompressedInfo.fType = plMipmap::UncompressedInfo::kRGB8888;
	fHost->IBuildLevelSizes();
	fHost->fTotalSize = fHost->GetLevelSize( 0 );

	// Destroy the old texture ref, since it's probably completely nutsoid at this point.
	// This should force the pipeline to recreate one more suitable for our use
	fHost->SetDeviceRef( nil );

	// Some init color
	hsColorRGBA	color;
	color.Set( 0.f, 0.f, 0.f, 1.f );
	ClearToColor( color );
	FlushToHost();

#ifdef MCN_DO_REFS
	/// Of course, brilliantly enough, if we did an attach on the constructor, we don't have a key
	/// yet, so we better give ourselves one before we can call AddViaNotify()
	if( GetKey() == nil )
	{
		char	str[ 256 ];
		sprintf( str, "plTextGen:%s", fHost->GetKeyName() );
		hsgResMgr::ResMgr()->NewKey( str, this, plLocation::kGlobalFixedLoc );
	}

	/// Send ourselves a passive ref of the mipmap, so we get notified if and when it goes away
	hsgResMgr::ResMgr()->AddViaNotify( fHost->GetKey(), TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnCreate, 0, 0 ), plRefFlags::kActiveRef );
#endif
	/// All done!
}

//// IAllocateOSSurface ///////////////////////////////////////////////////////
//	OS-specific. Allocates a rectangular bitmap of the given dimensions that
//	the OS can draw text into. Returns a pointer to the pixels.

UInt32		*plTextGenerator::IAllocateOSSurface( UInt16 width, UInt16 height )
{
#if HS_BUILD_FOR_WIN32

	BITMAPINFO	bmi;


	// Create a new DC and bitmap that we can draw characters to
	memset( &bmi.bmiHeader, 0, sizeof( BITMAPINFOHEADER ) );
	bmi.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = -(int)height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biBitCount = 32;
	
	fWinRGBDC = CreateCompatibleDC( nil );
	fWinRGBBitmap = CreateDIBSection( fWinRGBDC, &bmi, DIB_RGB_COLORS, (void **)&fWinRGBBits, nil, 0 );
	SetMapMode( fWinRGBDC, MM_TEXT );
	SetBkMode( fWinRGBDC, TRANSPARENT );
	SetTextAlign( fWinRGBDC, TA_TOP | TA_LEFT );

	SelectObject( fWinRGBDC, fWinRGBBitmap );

	// Now create a second DC/bitmap combo, this one for writing alpha values to
	memset( &bmi.bmiHeader, 0, sizeof( BITMAPINFOHEADER ) );
	bmi.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = -(int)height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biBitCount = 8;
	
	fWinAlphaDC = CreateCompatibleDC( nil );
	fWinAlphaBitmap = CreateDIBSection( fWinAlphaDC, &bmi, DIB_RGB_COLORS, (void **)&fWinAlphaBits, nil, 0 );
	SetMapMode( fWinAlphaDC, MM_TEXT );
	SetBkMode( fWinAlphaDC, TRANSPARENT );
	SetTextAlign( fWinAlphaDC, TA_TOP | TA_LEFT );

	SelectObject( fWinAlphaDC, fWinAlphaBitmap );

	return (UInt32 *)fWinRGBBits;
#endif
}

//// Detach ///////////////////////////////////////////////////////////////////
//	Release the mipmap unto itself.

void	plTextGenerator::Detach( void )
{
	if( fHost == nil )
		return;
//	hsAssert( fHost != nil, "Attempting to detach unattached host" );

	SetFont( nil, 0 );
	IDestroyOSSurface();

	fHost->Reset();
	fHost->fFlags &= ~( plMipmap::kUserOwnsBitmap | plMipmap::kDontThrowAwayImage );

	// Destroy the old texture ref, since we're no longer using it
	fHost->SetDeviceRef( nil );

	plMipmap	*oldHost = fHost;

	fHost = nil;

#ifdef MCN_DO_REFS
	// Now send ourselves a unref msg, just in case we were called directly (if this was done by 
	// message, we'll get called a few times, but that's ok, we're set up to handle that, and it
	// won't happen 'cept on destruction so the speed penalty shouldn't be a problem)
	GetKey()->Release( oldHost->GetKey() );
#endif
}

//// IDestroyOSSurface ////////////////////////////////////////////////////////
//	Opposite of allocate. DUH!

void	plTextGenerator::IDestroyOSSurface( void )
{
#if HS_BUILD_FOR_WIN32

	fHost->fImage = nil;	// DeleteObject() will get rid of it for us
	DeleteObject( fWinRGBBitmap );
	DeleteDC( fWinRGBDC );

	DeleteObject( fWinAlphaBitmap );
	DeleteDC( fWinAlphaDC );
	
#endif
}

//// ClearToColor /////////////////////////////////////////////////////////////

void	plTextGenerator::ClearToColor( hsColorRGBA &color )
{
	int		i;
	UInt32	*data = (UInt32 *)fHost->fImage;
	UInt32	hexColor = color.ToARGB32();

#if HS_BUILD_FOR_WIN32
	GdiFlush();
#endif

	for( i = 0; i < fHost->fWidth * fHost->fHeight; i++ )
		data[ i ] = hexColor;

	// Fill our alpha bitmap as well, since we use that too
#if HS_BUILD_FOR_WIN32
	memset( fWinAlphaBits, (UInt8)( color.a * 255.f ), fHost->fWidth * fHost->fHeight );
#endif
}

//// SetFont //////////////////////////////////////////////////////////////////
//	OS-specific. Load the given font for drawing the text with.

void	plTextGenerator::SetFont( const char *face, UInt16 size, hsBool antiAliasRGB )
{
#if HS_BUILD_FOR_WIN32
	if( fWinFont != nil )
	{
		DeleteObject( fWinFont );
		fWinFont = nil;
	}
	if( fWinAlphaFont != nil )
	{
		DeleteObject( fWinAlphaFont );
		fWinAlphaFont = nil;
	}

	if( face != nil )
	{
		int nHeight = -MulDiv( size, GetDeviceCaps( fWinRGBDC, LOGPIXELSY ), 72 );
		fWinFont = CreateFont( nHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
								CLIP_DEFAULT_PRECIS, antiAliasRGB ? ANTIALIASED_QUALITY : DEFAULT_QUALITY, VARIABLE_PITCH, face );
		hsAssert( fWinFont != nil, "Cannot create Windows font for plTextGenerator" );

		// The font for the alpha channel is identical except that it's antialiased, whereas the RGB version isn't.
		fWinAlphaFont = CreateFont( nHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
								CLIP_DEFAULT_PRECIS, (!antiAliasRGB) ? ANTIALIASED_QUALITY : DEFAULT_QUALITY, VARIABLE_PITCH, face );
		hsAssert( fWinAlphaFont != nil, "Cannot create Windows font for plTextGenerator" );

		SelectObject( fWinRGBDC, fWinFont );
		SelectObject( fWinAlphaDC, fWinAlphaFont );
	}
#endif
}

//// SetTextColor /////////////////////////////////////////////////////////////
//	blockRGB basically forces the RGB channel to write in blocks instead of
//	actual characters. This isn't useful unless you're relying on the alpha
//	channel to do the text (opaque text and transparent background), in which
//	case you want plenty of block color in your RGB channel because it'll get
//	alpha-ed out by the alpha channel.

void	plTextGenerator::SetTextColor( hsColorRGBA &color, hsBool blockRGB )
{
#if HS_BUILD_FOR_WIN32
	int r = (int)(color.r * 255.f);
	int g = (int)(color.g * 255.f);
	int b = (int)(color.b * 255.f);
	int a = (int)(color.a * 255.f);

	if( blockRGB )
	{
		::SetBkColor( fWinRGBDC, RGB( r, g, b ) );
		::SetBkMode( fWinRGBDC, OPAQUE );
	}
	else
		::SetBkMode( fWinRGBDC, TRANSPARENT );

	::SetTextColor( fWinRGBDC, RGB( r, g, b ) );
	::SetTextColor( fWinAlphaDC, RGB( a, a, a ) );
#endif
}

//// DrawString ///////////////////////////////////////////////////////////////

void	plTextGenerator::DrawString( UInt16 x, UInt16 y, const char *text )
{
	wchar_t *wText = hsStringToWString(text);
	DrawString(x,y,wText);
	delete [] wText;
}

void	plTextGenerator::DrawString( UInt16 x, UInt16 y, const wchar_t *text )
{
#if HS_BUILD_FOR_WIN32
	
	::TextOutW( fWinRGBDC, x, y, text, wcslen( text ) );
	::TextOutW( fWinAlphaDC, x, y, text, wcslen( text ) );

#endif
}

//// DrawClippedString ////////////////////////////////////////////////////////

void	plTextGenerator::DrawClippedString( Int16 x, Int16 y, const char *text, UInt16 width, UInt16 height )
{
	wchar_t *wText = hsStringToWString(text);
	DrawClippedString(x,y,wText,width,height);
	delete [] wText;
}

void	plTextGenerator::DrawClippedString( Int16 x, Int16 y, const wchar_t *text, UInt16 width, UInt16 height )
{
#if HS_BUILD_FOR_WIN32
	
	RECT	r;
	::SetRect( &r, x, y, x + width, y + height );

	::ExtTextOutW( fWinRGBDC, x, y, ETO_CLIPPED, &r, text, wcslen( text ), nil );
	::ExtTextOutW( fWinAlphaDC, x, y, ETO_CLIPPED, &r, text, wcslen( text ), nil );

#endif
}

//// DrawClippedString ////////////////////////////////////////////////////////

void	plTextGenerator::DrawClippedString( Int16 x, Int16 y, const char *text, UInt16 clipX, UInt16 clipY, UInt16 width, UInt16 height )
{
	wchar_t *wText = hsStringToWString(text);
	DrawClippedString(x,y,wText,clipX,clipY,width,height);
	delete [] wText;
}

void	plTextGenerator::DrawClippedString( Int16 x, Int16 y, const wchar_t *text, UInt16 clipX, UInt16 clipY, UInt16 width, UInt16 height )
{
#if HS_BUILD_FOR_WIN32
	
	RECT	r;
	::SetRect( &r, clipX, clipY, clipX + width, clipY + height );

	::ExtTextOutW( fWinRGBDC, x, y, ETO_CLIPPED, &r, text, wcslen( text ), nil );
	::ExtTextOutW( fWinAlphaDC, x, y, ETO_CLIPPED, &r, text, wcslen( text ), nil );

#endif
}

//// DrawWrappedString ////////////////////////////////////////////////////////

void	plTextGenerator::DrawWrappedString( UInt16 x, UInt16 y, const char *text, UInt16 width, UInt16 height )
{
	wchar_t *wText = hsStringToWString(text);
	DrawWrappedString(x,y,wText,width,height);
	delete [] wText;
}

void	plTextGenerator::DrawWrappedString( UInt16 x, UInt16 y, const wchar_t *text, UInt16 width, UInt16 height )
{
#if HS_BUILD_FOR_WIN32
	
	RECT	r;
	::SetRect( &r, x, y, x + width, y + height );

//	HBRUSH brush = ::CreateSolidBrush( RGB( 255, 255, 255 ) );
//	::FillRect( fWinRGBDC, &r, brush );
//	::DeleteObject( brush );
	::DrawTextW( fWinRGBDC, text, wcslen( text ), &r, 
				DT_TOP | DT_LEFT | DT_NOPREFIX | DT_WORDBREAK );
	::DrawTextW( fWinAlphaDC, text, wcslen( text ), &r, 
				DT_TOP | DT_LEFT | DT_NOPREFIX | DT_WORDBREAK );

#endif
}

//// CalcStringWidth //////////////////////////////////////////////////////////

UInt16		plTextGenerator::CalcStringWidth( const char *text, UInt16 *height )
{
	wchar_t *wText = hsStringToWString(text);
	UInt16 retVal = CalcStringWidth(wText,height);
	delete [] wText;
	return retVal;
}

UInt16		plTextGenerator::CalcStringWidth( const wchar_t *text, UInt16 *height )
{
#if HS_BUILD_FOR_WIN32

	SIZE size;
	::GetTextExtentPoint32W( fWinRGBDC, text, wcslen( text ), &size );

	if( height != nil )
		*height = (UInt16)size.cy;

	return (UInt16)size.cx;
#endif
}

//// CalcWrappedStringSize ////////////////////////////////////////////////////

void	plTextGenerator::CalcWrappedStringSize( const char *text, UInt16 *width, UInt16 *height )
{
	wchar_t *wText = hsStringToWString(text);
	CalcWrappedStringSize(wText,width,height);
	delete [] wText;
}

void	plTextGenerator::CalcWrappedStringSize( const wchar_t *text, UInt16 *width, UInt16 *height )
{
#if HS_BUILD_FOR_WIN32

	RECT	r;
	::SetRect( &r, 0, 0, *width, 0 );

	::DrawTextW( fWinRGBDC, text, wcslen( text ), &r, DT_TOP | DT_LEFT | DT_NOPREFIX | DT_WORDBREAK | DT_CALCRECT );

	*width = (UInt16)(r.right);
	if( height != nil )
		*height = (UInt16)r.bottom;
#endif
}

//// FillRect /////////////////////////////////////////////////////////////////

void	plTextGenerator::FillRect( UInt16 x, UInt16 y, UInt16 width, UInt16 height, hsColorRGBA &color )
{
#if HS_BUILD_FOR_WIN32

	RECT	rc;
	::SetRect( &rc, x, y, x + width, y + height );

	int r = (int)(color.r * 255.f);
	int g = (int)(color.g * 255.f);
	int b = (int)(color.b * 255.f);
	int a = (int)(color.a * 255.f);

	HBRUSH brush = ::CreateSolidBrush( RGB( r, g, b ) );
	::FillRect( fWinRGBDC, &rc, brush );
	::DeleteObject( brush );

	brush = ::CreateSolidBrush( RGB( a, a, a ) );
	::FillRect( fWinAlphaDC, &rc, brush );
	::DeleteObject( brush );
#endif
}

//// FrameRect ////////////////////////////////////////////////////////////////

void	plTextGenerator::FrameRect( UInt16 x, UInt16 y, UInt16 width, UInt16 height, hsColorRGBA &color )
{
#if HS_BUILD_FOR_WIN32

	RECT	rc;
	::SetRect( &rc, x, y, x + width, y + height );

	int r = (int)(color.r * 255.f);
	int g = (int)(color.g * 255.f);
	int b = (int)(color.b * 255.f);
	int a = (int)(color.a * 255.f);

	HBRUSH brush = ::CreateSolidBrush( RGB( r, g, b ) );
	::FrameRect( fWinRGBDC, &rc, brush );
	::DeleteObject( brush );

	brush = ::CreateSolidBrush( RGB( a, a, a ) );
	::FrameRect( fWinAlphaDC, &rc, brush );
	::DeleteObject( brush );
#endif
}

//// FlushToHost //////////////////////////////////////////////////////////////

void	plTextGenerator::FlushToHost( void )
{
#if HS_BUILD_FOR_WIN32
	// Flush the GDI first, to make sure it's done
	GdiFlush();

	// Now copy our alpha channel over. I hate the GDI
	UInt32		i = fHost->fWidth * fHost->fHeight;
	UInt32		*dest = fWinRGBBits;
	UInt8		*src = fWinAlphaBits;

/*	while( i-- )
	{
		*dest &= 0x00ffffff;
		*dest |= ( *src ) << 24;
//		*dest |= ( *dest << 16 ) & 0xff000000;
		dest++;
		src++;
	}
*/
	do
	{
		i--;
		dest[ i ] &= 0x00ffffff;
		dest[ i ] |= src[ i ] << 24;
	} while( i );
#endif

	// Dirty the mipmap's deviceRef, if there is one
	if( fHost->GetDeviceRef() != nil )
		fHost->GetDeviceRef()->SetDirty( true );
}

//// GetTextWidth/Height //////////////////////////////////////////////////////

UInt16	plTextGenerator::GetTextWidth( void )
{
	return ( fHost != nil ) ? (UInt16)(fHost->fWidth) : 0;
}

UInt16	plTextGenerator::GetTextHeight( void )
{
	return ( fHost != nil ) ? (UInt16)(fHost->fHeight) : 0;
}

//// GetLayerTransform ////////////////////////////////////////////////////////
//	Since the textGen can actually create a texture bigger than you were expecting,
//	you want to be able to apply a layer texture transform that will compensate. This
//	function will give you that transform. Just feed it into plLayer->SetTransform().

hsMatrix44	plTextGenerator::GetLayerTransform( void )
{
	hsMatrix44	xform;
	hsVector3	scale;

	scale.Set( (float)GetWidth() / (float)GetTextWidth(), 
			   (float)GetHeight() / (float)GetTextHeight(), 1.f );

	xform.MakeScaleMat( &scale );
	return xform;
}

//// MsgReceive ///////////////////////////////////////////////////////////////

hsBool	plTextGenerator::MsgReceive( plMessage *msg )
{
#ifdef MCN_DO_REFS
	plGenRefMsg	*refMsg = plGenRefMsg::ConvertNoRef( msg );
	if( refMsg != nil )
	{
		if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest ) )
		{
			// Don't do anything--already did an attach
		}
		else if( refMsg->GetContext() & ( plRefMsg::kOnDestroy | plRefMsg::kOnRemove ) )
		{
			Detach();
		}
		return true;
	}
#endif

	return hsKeyedObject::MsgReceive( msg );
}
