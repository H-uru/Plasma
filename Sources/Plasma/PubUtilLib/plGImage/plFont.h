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
//	plFont Class Header														 //
//	Seems like we've come full circle again. This is our generic Plasma		 //
//	bitmap font class/format. Quick list of things it supports, or needs to: //
//		- Antialiasing, either in the font def or at rendertime				 //
//		- Doublebyte character sets											 //
//		- Platform independence, of course									 //
//		- Render to reasonably arbitrary mipmap								 //
//		- Character-level kerning, both before and after, as well as		 //
//		  negative kerning (for ligatures)									 //
//																			 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	5.4.2003 mcn - Created.													 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plFont_h
#define _plFont_h

#include "hsTypes.h"
#include "hsColorRGBA.h"
#include "hsTemplates.h"
#include "pcSmallRect.h"

#include "../pnKeyedObject/hsKeyedObject.h"

#include <wchar.h>

class plBDFConvertCallback
{
	public:
		virtual void	NumChars( UInt16 chars ) {}
		virtual void	CharDone( void ) {}
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
			kRenderItalic		= 0x00000001,
			kRenderBold			= 0x00000002,
			kRenderScaleAA		= 0x00000004,
			kRenderClip			= 0x00000008,
			kRenderWrap			= 0x00000010,

			kRenderJustXLeft	= 0,
			kRenderJustXRight	= 0x00000020,	// X right justify
			kRenderJustXCenter	= 0x00000040,	// X center justify (left default)
			kRenderJustXForceLeft=0x00000080,	// Force left (no kerning)
			kRenderJustXMask	= 0x000000e0,

			kRenderJustYBaseline = 0,
			kRenderJustYTop		= 0x00000100,	// Y top justify
			kRenderJustYCenter	= 0x00000200,	// Y center justify
			kRenderJustYBottom	= 0x00000400,	// Y bottom justify (baseline is default)
			kRenderJustYMask	= 0x00000700,

			kRenderIntoAlpha	= 0x00000800,	// This option causes grayscale (AA) fonts to 
												// render into the alpha channel instead of the color
												// channel, so that the resulting pixels touched always
												// have the renderColor and the alpha = to the font pixel.
												// By default, we use the font pixel as an alpha blending
												// value between the renderColor and the destColor and
												// leave the alpha as-is
												// This flag has no effect on monochrome fonts
		};

		enum Flags
		{
			kFlagBold		= 0x00000001,
			kFlagItalic		= 0x00000002
		};

	protected:

		friend class plBDFHeaderParser;
		friend class plBDFPropertiesParser;
		friend class plBDFCharsParser;

		// Font face and size. This is just used for IDing purposes, not for rendering
		char	fFace[ 256 ];
		UInt8	fSize;
		UInt32	fFlags;

		// Size of the whole font bitmap. Fonts are stored vertically, one
		// character at a time, so fWidth is really the max width of any one
		// character bitmap, with of course the proper padding
		UInt32	fWidth, fHeight;

		// Bpp of our font bitmap. We're grayscale, remember...
		UInt8	fBPP;

		// Bitmap data!
		UInt8	*fBMapData;

		// Our character class, for per-char info
		class plCharacter
		{
			public:
				UInt32	fBitmapOff;		// Offset in the font bitmap in bytes 
										// to the first byte of the character
				UInt32	fHeight;		// Height in pixels of this character
				Int32	fBaseline;		// Number of pixels down from the top of 
										// the char bitmap to the baseline.
				
				hsScalar	fLeftKern;	// Kerning values for this char, in pixels
				hsScalar	fRightKern;	// Note that the right kern is relative to 
										// the right side of the bitmap area, which
										// is the width of the font bitmap, so
										// basically each character is the same width,
										// just kerned back!
										// (left kerning currently unsupported, just 
										// in here in case we need to eventually)

				plCharacter& operator=(const int zero) 
				{ 
					fBitmapOff=0;
					fHeight = 0;
					fBaseline = 0;
					fLeftKern = 0;
					fRightKern = 0;

					return *this;
				}

				plCharacter();
				void	Read( hsStream *s );
				void	Write( hsStream *s );
		};

		// First character we encode--everything below this we don't render
		UInt16	fFirstChar;

		// Our characters, stored in an hsTArray for easy construction
		hsTArray<plCharacter>	fCharacters;

		// Max character bitmap height and max ascent for any character
		UInt32	fMaxCharHeight;
		Int32	fFontAscent, fFontDescent;

		typedef void (plFont::*CharRenderFunc)( const plCharacter &c );

		// Render info
		class plRenderInfo
		{
			public:
				Int16		fFirstLineIndent;
				Int16		fX, fY, fNumCols, fFarthestX, fLastX, fLastY;
				Int16		fMaxWidth, fMaxHeight, fMaxAscent, fMaxDescent;
				Int16		fLineSpacing;
				UInt8		*fDestPtr;
				UInt32		fDestStride;
				UInt8		fDestBPP;
				UInt32		fColor;
				plMipmap	*fMipmap;
				UInt32		fFlags;
				pcSmallRect	fClipRect;
				hsScalar	fFloatWidth;

				const wchar_t	*fVolatileStringPtr;	// Just so we know where we clipped

				CharRenderFunc	fRenderFunc;
		};

		plRenderInfo	fRenderInfo;

		void	IClear( bool onConstruct = false );
		void	ICalcFontAscent( void );

		UInt8	*IGetFreeCharData( UInt32 &newOffset );

		void	IRenderLoop( const wchar_t *string, Int32 maxCount );
		void	IRenderString( plMipmap *mip, UInt16 x, UInt16 y, const wchar_t *string, hsBool justCalc );

		// Various render functions
		void	IRenderChar1To32( const plCharacter &c );
		void	IRenderChar1To32AA( const plCharacter &c );
		void	IRenderChar8To32( const plCharacter &c );
		void	IRenderChar8To32Alpha( const plCharacter &c );
		void	IRenderChar8To32FullAlpha( const plCharacter &c );
		void	IRenderCharNull( const plCharacter &c );

	public:

		plFont();
		virtual ~plFont();

		CLASSNAME_REGISTER( plFont );
		GETINTERFACE_ANY( plFont, hsKeyedObject );

		virtual void	Read( hsStream *s, hsResMgr *mgr );
		virtual void	Write( hsStream *s, hsResMgr *mgr );

		const char	*GetFace( void ) const { return fFace; }
		UInt8		GetSize( void ) const { return fSize; }
		UInt16		GetFirstChar( void ) const { return fFirstChar; }
		UInt16		GetNumChars( void ) const { return fCharacters.GetCount(); }
		UInt32		GetFlags( void ) const { return fFlags; }
		hsScalar	GetDescent( void ) const { return (hsScalar)fFontDescent; }
		hsScalar	GetAscent( void ) const { return (hsScalar)fFontAscent; }

		UInt32		GetBitmapWidth( void ) const { return fWidth; }
		UInt32		GetBitmapHeight( void ) const { return fHeight; }
		UInt8		GetBitmapBPP( void ) const { return fBPP; }

		void	SetFace( const char *face );
		void	SetSize( UInt8 size );
		void	SetFlags( UInt32 flags ) { fFlags = flags; }
		void	SetFlag( UInt32 flag, hsBool on ) { if( on ) fFlags |= flag; else fFlags &= ~flag; }
		hsBool	IsFlagSet( UInt32 flag ) { if( fFlags & flag ) return true; return false; }

		void	SetRenderColor( UInt32 color );
		void	SetRenderFlag( UInt32 flag, hsBool on ) { if( on ) fRenderInfo.fFlags |= flag; else fRenderInfo.fFlags &= ~flag; }
		hsBool	IsRenderFlagSet( UInt32 flag ) { return ( fRenderInfo.fFlags & flag ) ? true : false; }
		void	SetRenderClipRect( Int16 x, Int16 y, Int16 width, Int16 height );
		void	SetRenderXJustify( UInt32 j ) { fRenderInfo.fFlags &= ~kRenderJustXMask; fRenderInfo.fFlags |= j; }
		void	SetRenderYJustify( UInt32 j ) { fRenderInfo.fFlags &= ~kRenderJustYMask; fRenderInfo.fFlags |= j; }
		void	SetRenderFirstLineIndent( Int16 indent ) { fRenderInfo.fFirstLineIndent = indent; }
		void	SetRenderLineSpacing( Int16 spacing ) { fRenderInfo.fLineSpacing = spacing; }

		// Sets flags too
		void	SetRenderClipping( Int16 x, Int16 y, Int16 width, Int16 height );
		void	SetRenderWrapping( Int16 x, Int16 y, Int16 width, Int16 height );

		void	RenderString( plMipmap *mip, UInt16 x, UInt16 y, const char *string, UInt16 *lastX = nil, UInt16 *lastY = nil );
		void	RenderString( plMipmap *mip, UInt16 x, UInt16 y, const wchar_t *string, UInt16 *lastX = nil, UInt16 *lastY = nil );

		UInt16	CalcStringWidth( const char *string );
		UInt16	CalcStringWidth( const wchar_t *string );
		void	CalcStringExtents( const char *string, UInt16 &width, UInt16 &height, UInt16 &ascent, UInt32 &firstClippedChar, UInt16 &lastX, UInt16 &lastY );
		void	CalcStringExtents( const wchar_t *string, UInt16 &width, UInt16 &height, UInt16 &ascent, UInt32 &firstClippedChar, UInt16 &lastX, UInt16 &lastY );

		hsBool	LoadFromFNT( const char *path );
		hsBool	LoadFromFNTStream( hsStream *stream );

		hsBool	LoadFromBDF( const char *path, plBDFConvertCallback *callback );
		hsBool	LoadFromBDFStream( hsStream *stream, plBDFConvertCallback *callback );

		hsBool	LoadFromP2FFile( const char *path );

		hsBool	ReadRaw( hsStream *stream );
		hsBool	WriteRaw( hsStream *stream );
};

#endif // _plFont_h
