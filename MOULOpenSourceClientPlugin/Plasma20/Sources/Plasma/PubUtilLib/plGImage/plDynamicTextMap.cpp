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
//	plDynamicTextMap Class Functions										 //
//	Derived bitmap class representing a single mipmap.						 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	6.7.2001 mcn - Created.													 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "plDynamicTextMap.h"

#include "hsStream.h"
#include "hsExceptions.h"
#include "hsUtils.h"
#include "hsMatrix44.h"
#include "../plPipeline/hsGDeviceRef.h"
#include "../plMessage/plDynamicTextMsg.h"
#include "../pnKeyedObject/plKey.h"
#include "plProfile.h"
#include "../plStatusLog/plStatusLog.h"
#include "plFont.h"
#include "plFontCache.h"
#include "../plResMgr/plLocalization.h"


plProfile_CreateMemCounter("DynaTextMem", "PipeC", DynaTextMem);
plProfile_CreateCounterNoReset("DynaTexts", "PipeC", DynaTexts);
plProfile_Extern(MemMipmaps);


//// Constructor & Destructor /////////////////////////////////////////////////

plDynamicTextMap::plDynamicTextMap() : plMipmap()
{
	fVisWidth = fVisHeight = 0;
	fHasAlpha = false;
	fJustify = kLeftJustify;
	fInitBuffer = nil;
	fFontFace = nil;
	fFontSize = 0;
	fFontFlags = 0;
	fFontAntiAliasRGB = false;
	fFontColor.Set( 0, 0, 0, 1 );
	fFontBlockRGB = false;
	fHasCreateBeenCalled = false;

}

plDynamicTextMap::~plDynamicTextMap()
{
	Reset();
}

plDynamicTextMap::plDynamicTextMap( UInt32 width, UInt32 height, hsBool hasAlpha, UInt32 extraWidth, UInt32 extraHeight ) : plMipmap()
{
	fInitBuffer = nil;
	fFontFace = nil;
	Create( width, height, hasAlpha, extraWidth, extraHeight );
}

//// SetNoCreate //////////////////////////////////////////////////////////////
//	For export time, we want to set up the config to write to disk, but we
//	don't want to actually be creating OS surfaces. So we call this function
//	instead, which does just that. It basically does all the setup work that
//	Create() does, or enough for us to write out later.

void	plDynamicTextMap::SetNoCreate( UInt32 width, UInt32 height, hsBool hasAlpha )
{
	// OK, so it really isn't that much work...
	fVisWidth = (UInt16)width;
	fVisHeight = (UInt16)height;
	fHasAlpha = hasAlpha;
	fImage = nil;		// So we know we haven't actually done anything yet
	delete [] fInitBuffer;
	fInitBuffer = nil;
}

//// Create ///////////////////////////////////////////////////////////////////

void	plDynamicTextMap::Create( UInt32 width, UInt32 height, hsBool hasAlpha, UInt32 extraWidth, UInt32 extraHeight )
{
	SetConfig( hasAlpha ? kARGB32Config : kRGB32Config );


	fVisWidth = (UInt16)width;
	fVisHeight = (UInt16)height;
	fHasAlpha = hasAlpha;

	for( fWidth = 1; fWidth < width + extraWidth; fWidth <<= 1 );
	for( fHeight = 1; fHeight < height + extraHeight; fHeight <<= 1 );

	// instead of allocating the fImage here, we'll wait for the first draw operation to be called (in IIsValid)
	fHasCreateBeenCalled = true;

	fRowBytes = fWidth << 2;
	fNumLevels = 1;
	fFlags |= plMipmap::kDontThrowAwayImage;
	fCompressionType = plMipmap::kUncompressed;
	fUncompressedInfo.fType = plMipmap::UncompressedInfo::kRGB8888;

	// Destroy the old texture ref, if we have one. This should force the 
	// pipeline to recreate one more suitable for our use
	SetDeviceRef( nil );

	// Some init color
	SetFont( "Arial", 12 );
	hsColorRGBA	color;
	color.Set( 0,0,1,1);
	SetTextColor( color );

	SetCurrLevel( 0 );
	plProfile_Inc(DynaTexts);

}

//// Reset ////////////////////////////////////////////////////////////////////

void	plDynamicTextMap::Reset( void )
{
	IDestroyOSSurface();

	plMipmap::Reset();

	// they need to call create again to undo the affects of call Reset()
	fHasCreateBeenCalled = false;

	delete [] fInitBuffer;
	fInitBuffer = nil;

	delete [] fFontFace;
	fFontFace = nil;

	// Destroy the old texture ref, since we're no longer using it
	SetDeviceRef( nil );
}

///////////////////////////////////////////////////////////////////////////////
//// OS-Specific Functions ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

hsBool		plDynamicTextMap::IIsValid( void )
{
	if( GetImage() == nil && fHasCreateBeenCalled )
	{
		// we are going to allocate the fImage at this point... when someone is looking for it
		fImage = (void *)IAllocateOSSurface( (UInt16)fWidth, (UInt16)fHeight );
		hsColorRGBA	color;
		if( fInitBuffer != nil )
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
		SetDeviceRef( nil );
		plProfile_NewMem(MemMipmaps, fTotalSize);
		plProfile_NewMem(DynaTextMem, fTotalSize);
#ifdef MEMORY_LEAK_TRACER
		IAddToMemRecord( this, plRecord::kViaCreate );
#endif
	}

	if( GetImage() == nil )
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
	SetDeviceRef( nil );
}

//// IAllocateOSSurface ///////////////////////////////////////////////////////
//	OS-specific. Allocates a rectangular bitmap of the given dimensions that
//	the OS can draw text into. Returns a pointer to the pixels.

UInt32* plDynamicTextMap::IAllocateOSSurface( UInt16 width, UInt16 height )
{
	UInt32* pixels = TRACKED_NEW UInt32[ width * height ];
	return pixels;
}

//// IDestroyOSSurface ////////////////////////////////////////////////////////
//	Opposite of allocate. DUH!

void	plDynamicTextMap::IDestroyOSSurface( void )
{
#ifdef MEMORY_LEAK_TRACER
	if( fImage != nil )
		IRemoveFromMemRecord( (UInt8 *)fImage );
#endif

	delete [] fImage;
	fImage = nil;

	plProfile_Dec(DynaTexts);
	plProfile_DelMem(DynaTextMem, fTotalSize);
	plProfile_DelMem(MemMipmaps, fTotalSize);
}

///////////////////////////////////////////////////////////////////////////////
//// Virtual Functions ////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// Read /////////////////////////////////////////////////////////////////////

UInt32  plDynamicTextMap::Read( hsStream *s )
{
	UInt32 totalRead = plBitmap::Read( s );

	// The funny thing is that we don't read anything like a mipmap; we just
	// keep the width and height and call Create() after we read those in

	fVisWidth = (UInt16)(s->ReadSwap32());
	fVisHeight = (UInt16)(s->ReadSwap32());
	fHasAlpha = s->ReadBool();
	totalRead += 2 * 4;

	UInt32 initSize = s->ReadSwap32();
	totalRead += 4;
	if( initSize > 0 )
	{
		fInitBuffer = TRACKED_NEW UInt32[ initSize ];

		s->ReadSwap32( initSize, fInitBuffer );
		totalRead += initSize * 4;
	}
	else
		fInitBuffer = nil;

	Create( fVisWidth, fVisHeight, fHasAlpha );
	
	delete [] fInitBuffer;
	fInitBuffer = nil;

	return totalRead;
}

//// Write ////////////////////////////////////////////////////////////////////

UInt32 	plDynamicTextMap::Write( hsStream *s )
{
	UInt32 totalWritten = plBitmap::Write( s );

	s->WriteSwap32( fVisWidth );
	s->WriteSwap32( fVisHeight );
	s->WriteBool( fHasAlpha );

	s->WriteSwap32( fInitBuffer != nil ? fVisWidth * fVisHeight * sizeof( UInt32 ) : 0 );
	if( fInitBuffer != nil )
	{
		s->WriteSwap32( fVisWidth * fVisHeight, fInitBuffer );
	}

	return totalWritten;
}

///////////////////////////////////////////////////////////////////////////////
//// Some More Functions //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// SetInitBuffer ////////////////////////////////////////////////////////////
//	Sets an initial buffer, which is written to disk and read in to use for
//	initializing the color buffer upon creation. If not specified, we init to
//	black. ASSUMES the buffer is of dimensions fVisWidth x fVisHeight.

void	plDynamicTextMap::SetInitBuffer( UInt32 *buffer )
{
	delete [] fInitBuffer;
	if( buffer == nil )
	{
		fInitBuffer = nil;
		return;
	}

	fInitBuffer = TRACKED_NEW UInt32[ fVisWidth * fVisHeight ];
	memcpy( fInitBuffer, buffer, fVisWidth * fVisHeight * sizeof( UInt32 ) );
}

//// CopyFrom /////////////////////////////////////////////////////////////////

void	plDynamicTextMap::CopyFrom( plMipmap *source )
{
	hsAssert( false, "Copying plDynamicTextMaps is not supported." );
}

//// Clone ////////////////////////////////////////////////////////////////////

plMipmap	*plDynamicTextMap::Clone( void )
{
	static bool alreadyWarned = false;

	if( !alreadyWarned )
	{
		hsAssert( false, "Cloning plDynamicTextMaps is not supported." );
		alreadyWarned = true;
	}

	return nil;
}

///////////////////////////////////////////////////////////////////////////////
//// Rendering Functions //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// IClearFromBuffer /////////////////////////////////////////////////////////

void	plDynamicTextMap::IClearFromBuffer( UInt32 *clearBuffer )
{
	int			y;
	UInt32		*data = (UInt32 *)fImage, *srcData = clearBuffer;
	UInt8		*destAlpha = nil;


	if( !IIsValid() )
		return;

	// Clear *all* to zero
	memset( data, 0, fWidth * fHeight * sizeof( UInt32 ) );

	// Buffer is of size fVisWidth x fVisHeight, so we need a bit of work to do this right
	for( y = 0; y < fVisHeight; y++ )
	{
		memcpy( data, srcData, fVisWidth * sizeof( UInt32 ) );
		data += fWidth;
		srcData += fVisWidth;
	}
}

//// ClearToColor /////////////////////////////////////////////////////////////

void	plDynamicTextMap::ClearToColor( hsColorRGBA &color )
{
	if( !IIsValid() )
		return;

	UInt32		i, hex = color.ToARGB32();
	UInt32		*data = (UInt32 *)fImage;

	// Buffer is of size fVisWidth x fVisHeight, so we need a bit of work to do this right
	for( i = 0; i < fHeight * fWidth; i++ )
		data[ i ] = hex;
}

//// SetJustify ///////////////////////////////////////////////////////////////

void	plDynamicTextMap::SetJustify( Justify j )
{
// ===> Don't need to validate creation
//	if( !IIsValid() )
//		return;

	fJustify = j; 
	switch( fJustify )
	{
		case kLeftJustify:	fCurrFont->SetRenderXJustify( plFont::kRenderJustXForceLeft ); break;
		case kCenter:		fCurrFont->SetRenderXJustify( plFont::kRenderJustXCenter ); break;
		case kRightJustify:	fCurrFont->SetRenderXJustify( plFont::kRenderJustXRight ); break;
	}
}

//// SetFont //////////////////////////////////////////////////////////////////

void	plDynamicTextMap::SetFont( const char *face, UInt16 size, UInt8 fontFlags, hsBool antiAliasRGB )
{
// ===> Don't need to validate creation
//	if( !IIsValid() )
//		return;

	delete [] fFontFace;
	if (plLocalization::UsingUnicode())
	{
		// unicode has a bunch of chars that most fonts don't have, so we override the font choice with one
		// that will work with the desired language
		hsStatusMessageF("We are using a unicode language, overriding font choice of %s", face ? face : "nil");
		fFontFace = hsStrcpy( "Unicode" );
	}
	else
		fFontFace = ( face != nil ) ? hsStrcpy( face ) : nil;
	fFontSize = size;
	fFontFlags = fontFlags;
	fFontAntiAliasRGB = antiAliasRGB;

	fCurrFont = plFontCache::GetInstance().GetFont( fFontFace, (UInt8)fFontSize,
						( ( fFontFlags & kFontBold ) ? plFont::kFlagBold : 0 ) | 
						( ( fFontFlags & kFontItalic ) ? plFont::kFlagItalic : 0 ) );
	if ( fCurrFont == nil )
	{
		if (!fCurrFont)
			hsStatusMessageF("Font missing - %s. Using Arial", fFontFace ? fFontFace : "nil");

		if ( fFontFace )
			delete [] fFontFace;
		fFontFace = hsStrcpy( "Arial" );
		// lets try again with Arial
		fCurrFont = plFontCache::GetInstance().GetFont( fFontFace, (UInt8)fFontSize,
							( ( fFontFlags & kFontBold ) ? plFont::kFlagBold : 0 ) | 
							( ( fFontFlags & kFontItalic ) ? plFont::kFlagItalic : 0 ) );
	}

	// This will be nil if we're just running the page optimizer.
	if (fCurrFont)
	{
		fCurrFont->SetRenderYJustify( plFont::kRenderJustYTop );
		SetJustify( fJustify );
	}
}

void	plDynamicTextMap::SetFont( const wchar_t *face, UInt16 size, UInt8 fontFlags , hsBool antiAliasRGB )
{
	char *sFace = hsWStringToString(face);
	SetFont(sFace,size,fontFlags,antiAliasRGB);
	delete [] sFace;
}

//// SetLineSpacing ///////////////////////////////////////////////////////////

void	plDynamicTextMap::SetLineSpacing( Int16 spacing )
{
// ===> Don't need to validate creation
//	if( !IIsValid() )
//		return;

	fLineSpacing = spacing;
	fCurrFont->SetRenderLineSpacing(spacing);
}

//// SetTextColor /////////////////////////////////////////////////////////////

void	plDynamicTextMap::SetTextColor( hsColorRGBA &color, hsBool blockRGB )
{
// ===> Don't need to validate creation
//	if( !IIsValid() )
//		return;

	fFontColor = color;
	fFontBlockRGB = blockRGB;

	if (fCurrFont)
		fCurrFont->SetRenderColor( fFontColor.ToARGB32() );
}

//// DrawString ///////////////////////////////////////////////////////////////

void	plDynamicTextMap::DrawString( UInt16 x, UInt16 y, const char *text )
{
	wchar_t *wText = hsStringToWString(text);
	DrawString(x,y,wText);
	delete [] wText;
}

void	plDynamicTextMap::DrawString( UInt16 x, UInt16 y, const wchar_t *text )
{
	if( !IIsValid() )
		return;

	SetJustify( fJustify );
	fCurrFont->SetRenderFlag( plFont::kRenderWrap | plFont::kRenderClip, false );
	fCurrFont->SetRenderColor( fFontColor.ToARGB32() );
	fCurrFont->SetRenderFlag( plFont::kRenderIntoAlpha, fFontBlockRGB );
	fCurrFont->RenderString( this, x, y, text );
}

//// DrawClippedString ////////////////////////////////////////////////////////

void	plDynamicTextMap::DrawClippedString( Int16 x, Int16 y, const char *text, UInt16 width, UInt16 height )
{
	wchar_t *wText = hsStringToWString(text);
	DrawClippedString(x,y,wText,width,height);
	delete [] wText;
}

void	plDynamicTextMap::DrawClippedString( Int16 x, Int16 y, const wchar_t *text, UInt16 width, UInt16 height )
{
	if( !IIsValid() )
		return;

	SetJustify( fJustify );
	fCurrFont->SetRenderClipping( x, y, width, height );
	fCurrFont->SetRenderColor( fFontColor.ToARGB32() );
	fCurrFont->SetRenderFlag( plFont::kRenderIntoAlpha, fFontBlockRGB );
	fCurrFont->RenderString( this, x, y, text );
}

//// DrawClippedString ////////////////////////////////////////////////////////

void	plDynamicTextMap::DrawClippedString( Int16 x, Int16 y, const char *text, UInt16 clipX, UInt16 clipY, UInt16 width, UInt16 height )
{
	wchar_t *wText = hsStringToWString(text);
	DrawClippedString(x,y,wText,clipX,clipY,width,height);
	delete [] wText;
}

void	plDynamicTextMap::DrawClippedString( Int16 x, Int16 y, const wchar_t *text, UInt16 clipX, UInt16 clipY, UInt16 width, UInt16 height )
{
	if( !IIsValid() )
		return;

	SetJustify( fJustify );
	fCurrFont->SetRenderClipping( clipX, clipY, width, height );
	fCurrFont->SetRenderColor( fFontColor.ToARGB32() );
	fCurrFont->RenderString( this, x, y, text );
}

//// DrawWrappedString ////////////////////////////////////////////////////////

void	plDynamicTextMap::DrawWrappedString( UInt16 x, UInt16 y, const char *text, UInt16 width, UInt16 height, UInt16 *lastX, UInt16 *lastY )
{
	wchar_t *wText = hsStringToWString(text);
	DrawWrappedString(x,y,wText,width,height,lastX,lastY);
	delete [] wText;
}

void	plDynamicTextMap::DrawWrappedString( UInt16 x, UInt16 y, const wchar_t *text, UInt16 width, UInt16 height, UInt16 *lastX, UInt16 *lastY )
{
	if( !IIsValid() )
		return;

	SetJustify( fJustify );
	fCurrFont->SetRenderWrapping( x, y, width, height );
	fCurrFont->SetRenderColor( fFontColor.ToARGB32() );
	fCurrFont->SetRenderFlag( plFont::kRenderIntoAlpha, fFontBlockRGB );
	fCurrFont->RenderString( this, x, y, text, lastX, lastY );
}

//// CalcStringWidth //////////////////////////////////////////////////////////

UInt16		plDynamicTextMap::CalcStringWidth( const char *text, UInt16 *height )
{
	wchar_t *wText = hsStringToWString(text);
	UInt16 w = CalcStringWidth(wText,height);
	delete [] wText;
	return w;
}

UInt16		plDynamicTextMap::CalcStringWidth( const wchar_t *text, UInt16 *height )
{
// ===> Don't need to validate creation
//	if( !IIsValid() )
//		return 0;

	SetJustify( fJustify );
	UInt16 w, h, a, lastX, lastY;
	UInt32 firstClipped;
	fCurrFont->SetRenderFlag( plFont::kRenderClip | plFont::kRenderWrap, false );
	fCurrFont->CalcStringExtents( text, w, h, a, firstClipped, lastX, lastY );
	if( height != nil )
		*height = h;
	return w;
}

//// SetFirstLineIndent ///////////////////////////////////////////////////////

void	plDynamicTextMap::SetFirstLineIndent( Int16 indent )
{
// ===> Don't need to validate creation
//	if( !IIsValid() )
//		return;

	fCurrFont->SetRenderFirstLineIndent( indent );
}

//// CalcWrappedStringSize ////////////////////////////////////////////////////

void	plDynamicTextMap::CalcWrappedStringSize( const char *text, UInt16 *width, UInt16 *height, UInt32 *firstClippedChar, UInt16 *maxAscent, UInt16 *lastX, UInt16 *lastY )
{
	wchar_t *wText = hsStringToWString(text);
	CalcWrappedStringSize(wText,width,height,firstClippedChar,maxAscent,lastX,lastY);
	delete [] wText;
}

void	plDynamicTextMap::CalcWrappedStringSize( const wchar_t *text, UInt16 *width, UInt16 *height, UInt32 *firstClippedChar, UInt16 *maxAscent, UInt16 *lastX, UInt16 *lastY )
{
// ===> Don't need to validate creation
//	if( !IIsValid() )
//		return;

	SetJustify( fJustify );
	UInt16 w, h, a, lX, lY;
	UInt32 firstClipped;
	fCurrFont->SetRenderWrapping( 0, 0, *width, *height );
	fCurrFont->CalcStringExtents( text, w, h, a, firstClipped, lX, lY );
	*width = w;
	*height = h;
	if( firstClippedChar != nil )
		*firstClippedChar = firstClipped;
	if( maxAscent != nil )
		*maxAscent = a;
	if( lastX != nil )
		*lastX = lX;
	if( lastY != nil )
		*lastY = lY;
}

//// FillRect /////////////////////////////////////////////////////////////////

void	plDynamicTextMap::FillRect( UInt16 x, UInt16 y, UInt16 width, UInt16 height, hsColorRGBA &color )
{
	if( !IIsValid() )
		return;

	if( x + width > fWidth )
		width = (UInt16)(fWidth - x);

	// Gee, how hard can it REALLY be?
	UInt32 i, hex = color.ToARGB32();
	height += y;
	if( height > fHeight )
		height = (UInt16)fHeight;
	for( ; y < height; y++ )
	{
		UInt32 *destPtr = GetAddr32( x, y );
		for( i = 0; i < width; i++ )
			destPtr[ i ] = hex;
	}
}

//// FrameRect ////////////////////////////////////////////////////////////////

void	plDynamicTextMap::FrameRect( UInt16 x, UInt16 y, UInt16 width, UInt16 height, hsColorRGBA &color )
{
	if( !IIsValid() )
		return;

	if( x + width > fWidth )
		width = (UInt16)(fWidth - x);
	if( y + height > fHeight )
		height = (UInt16)(fHeight - y);

	// Shouldn't be much harder
	UInt32 i, hex = color.ToARGB32();
	UInt32 *dest1, *dest2;

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

void	plDynamicTextMap::DrawImage( UInt16 x, UInt16 y, plMipmap *image, DrawMethods method )
{
	if( !IIsValid() )
		return;

	plMipmap::CompositeOptions	opts;
	if( method == kImgNoAlpha )
	{
		if( fHasAlpha )
			opts.fFlags = plMipmap::kForceOpaque;
		else
			opts.fFlags = plMipmap::kCopySrcAlpha; // Don't care, this is fastest
	}
	else if( method == kImgBlend )
		opts.fFlags = 0;		// Default opts
	else if( method == kImgSprite )
		opts.fFlags = plMipmap::kCopySrcAlpha;

	Composite( image, x, y, &opts );

	/// HACK for now, since the alpha in the mipmap gets copied straight into the
	/// 32-bit color buffer, but our separate hacked alpha buffer hasn't been updated
/*	if( fHasAlpha && !respectAlpha )
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

void	plDynamicTextMap::DrawClippedImage( UInt16 x, UInt16 y, plMipmap *image, 
											UInt16 srcClipX, UInt16 srcClipY, 
											UInt16 srcClipWidth, UInt16 srcClipHeight, 
											DrawMethods method )
{
	if( !IIsValid() )
		return;

	plMipmap::CompositeOptions	opts;
	if( method == kImgNoAlpha )
	{
		if( fHasAlpha )
			opts.fFlags = plMipmap::kForceOpaque;
		else
			opts.fFlags = plMipmap::kCopySrcAlpha; // Don't care, this is fastest
	}
	else if( method == kImgBlend )
		opts.fFlags = 0;		// Default opts
	else if( method == kImgSprite )
		opts.fFlags = plMipmap::kCopySrcAlpha;

	opts.fSrcClipX = srcClipX;
	opts.fSrcClipY = srcClipY;
	opts.fSrcClipWidth = srcClipWidth;
	opts.fSrcClipHeight = srcClipHeight;
	Composite( image, x, y, &opts );

	/// HACK for now, since the alpha in the mipmap gets copied straight into the
	/// 32-bit color buffer, but our separate hacked alpha buffer hasn't been updated
/*	if( fHasAlpha && !respectAlpha )
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

void	plDynamicTextMap::FlushToHost( void )
{
	if( !IIsValid() )
		return;

	// Dirty the mipmap's deviceRef, if there is one
	if( GetDeviceRef() != nil )
		GetDeviceRef()->SetDirty( true );
}

//// GetLayerTransform ////////////////////////////////////////////////////////
//	Since the textGen can actually create a texture bigger than you were expecting,
//	you want to be able to apply a layer texture transform that will compensate. This
//	function will give you that transform. Just feed it into plLayer->SetTransform().

hsMatrix44	plDynamicTextMap::GetLayerTransform( void )
{
	hsMatrix44	xform;
	hsVector3	scale;

	scale.Set( (float)GetVisibleWidth() / (float)GetWidth(), 
			   (float)GetVisibleHeight() / (float)GetHeight(), 1.f );

	xform.MakeScaleMat( &scale );
	return xform;
}

//// MsgReceive ///////////////////////////////////////////////////////////////

hsBool	plDynamicTextMap::MsgReceive( plMessage *msg )
{
	plDynamicTextMsg	*textMsg = plDynamicTextMsg::ConvertNoRef( msg );
	if( textMsg != nil )
	{
		if( textMsg->fCmd & plDynamicTextMsg::kClear )
			ClearToColor( textMsg->fClearColor );

		if( textMsg->fCmd & plDynamicTextMsg::kSetTextColor )
			SetTextColor( textMsg->fColor, textMsg->fBlockRGB );

		if( (textMsg->fCmd & plDynamicTextMsg::kSetFont ) && textMsg->fString)
			SetFont( textMsg->fString, textMsg->fX, (UInt8)(textMsg->fFlags) );
		
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

		if( (textMsg->fCmd & plDynamicTextMsg::kDrawString ) && textMsg->fString)
			DrawString( textMsg->fX, textMsg->fY, textMsg->fString );

		if( (textMsg->fCmd & plDynamicTextMsg::kDrawClippedString ) && textMsg->fString)
			DrawClippedString( textMsg->fX, textMsg->fY, textMsg->fString,
								textMsg->fLeft, textMsg->fTop, textMsg->fRight - textMsg->fLeft + 1,
								textMsg->fBottom - textMsg->fTop + 1 );

		if( (textMsg->fCmd & plDynamicTextMsg::kDrawWrappedString ) && textMsg->fString)
			DrawWrappedString( textMsg->fX, textMsg->fY, textMsg->fString, textMsg->fRight, textMsg->fBottom );

		if( textMsg->fCmd & plDynamicTextMsg::kDrawImage )
		{
			plMipmap *mip = plMipmap::ConvertNoRef( textMsg->fImageKey ? textMsg->fImageKey->ObjectIsLoaded() : nil);
			if( mip != nil )
				DrawImage( textMsg->fX, textMsg->fY, mip, textMsg->fFlags ? kImgBlend : kImgNoAlpha );
		}

		if( textMsg->fCmd & plDynamicTextMsg::kDrawClippedImage )
		{
			plMipmap *mip = plMipmap::ConvertNoRef( textMsg->fImageKey ? textMsg->fImageKey->ObjectIsLoaded() : nil);
			if( mip != nil )
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
//	Swapping is an evil little trick. It's also something that should probably
//	be exposed at the mipmap level eventually, but there's no need for it yet.
//	Basically, it lets you take the contents of two DTMaps and swap them, as
//	if you had swapped pointers, but you didn't. This is so you can, well, swap
//	DTMaps without swapping pointers! (Like, say, you don't have access to them)

#define SWAP_ME( Type, a, b ) { Type temp; temp = a; a = b; b = temp; }

void	plDynamicTextMap::Swap( plDynamicTextMap *other )
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
	if( GetDeviceRef() != nil )
		GetDeviceRef()->SetDirty( true );
	if( other->GetDeviceRef() != nil )
		other->GetDeviceRef()->SetDirty( true );

	// Swap DTMap info
	SWAP_ME( hsBool, fHasAlpha, other->fHasAlpha );
	SWAP_ME( hsBool, fShadowed, other->fShadowed );

	SWAP_ME( Justify, fJustify, other->fJustify );
	SWAP_ME( char *, fFontFace, other->fFontFace );
	SWAP_ME( UInt16, fFontSize, other->fFontSize );
	SWAP_ME( UInt8, fFontFlags, other->fFontFlags );
	SWAP_ME( hsBool, fFontAntiAliasRGB, other->fFontAntiAliasRGB );
	SWAP_ME( hsColorRGBA, fFontColor, other->fFontColor );
	SWAP_ME( hsBool, fFontBlockRGB, other->fFontBlockRGB );

	SWAP_ME( hsBool, fFontBlockRGB, other->fFontBlockRGB );
	SWAP_ME( hsBool, fFontBlockRGB, other->fFontBlockRGB );
	SWAP_ME( hsBool, fFontBlockRGB, other->fFontBlockRGB );

	SWAP_ME( plFont *, fCurrFont, other->fCurrFont );
	SWAP_ME( UInt32 *, fInitBuffer, other->fInitBuffer );
}
