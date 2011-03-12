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
//	plDynSurfaceWriter Class Header											 //
//	Abstract class wrapping around Windows GDI functionality for writing to	 //
//	a generic RGBA surface. Allows us to create one writer per DTMap or a	 //
//	single shared writer to conserve OS resources on 98/ME.					 //
//																			 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	10.28.2002 mcn - Created.												 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plDynSurfaceWriter_h
#define _plDynSurfaceWriter_h

#include "hsColorRGBA.h"
#include "hsWindows.h"				// EVIL

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
			kSupportAlpha	= 0x00000001,
			
			kFontBold		= 0x00000002,
			kFontItalic		= 0x00000004,
			kFontShadowed	= 0x00000008,
			kFontMask		= 0x0000000e,

			kDiscardOnFlush	= 0x00000010
		};

		//// Public Data /////
		
		
		//// Public Members ////


		plDynSurfaceWriter();
		plDynSurfaceWriter( plDynamicTextMap *target, UInt32 flags = 0 );
		virtual ~plDynSurfaceWriter();


		/// Operations to perform on the text block
		
		void	ClearToColor( hsColorRGBA &color );
		void	SetFont( const char *face, UInt16 size, UInt8 fontFlags = 0, hsBool antiAliasRGB = true );
		void	SetTextColor( hsColorRGBA &color, hsBool blockRGB = false );
		void	SetJustify( Justify j );

		void	DrawString( UInt16 x, UInt16 y, const char *text );
		void	DrawClippedString( Int16 x, Int16 y, const char *text, UInt16 width, UInt16 height );
		void	DrawClippedString( Int16 x, Int16 y, const char *text, UInt16 clipX, UInt16 clipY, UInt16 width, UInt16 height );
		void	DrawWrappedString( UInt16 x, UInt16 y, const char *text, UInt16 width, UInt16 height );
		UInt16	CalcStringWidth( const char *text, UInt16 *height = nil );
		void	CalcWrappedStringSize( const char *text, UInt16 *width, UInt16 *height );
		void	FillRect( UInt16 x, UInt16 y, UInt16 width, UInt16 height, hsColorRGBA &color );
		void	FrameRect( UInt16 x, UInt16 y, UInt16 width, UInt16 height, hsColorRGBA &color );

//		void	DrawImage( UInt16 x, UInt16 y, plMipmap *image, hsBool respectAlpha = false );
//		void	DrawClippedImage( UInt16 x, UInt16 y, plMipmap *image, UInt16 srcClipX, UInt16 srcClipY, 
//								UInt16 srcClipWidth, UInt16 srcClipHeight, hsBool respectAlpha = false );

		// Copy the raw data from the given buffer.
//		void	SetBitsFromBuffer( UInt32 *clearBuffer, UInt16 width, UInt16 height )

		/// Target switching operations

		// Flushes all ops to the target.
		void	FlushToTarget( void );

		// Switches targets. Will flush to old target before switching. Also, if kDiscard isn't specified, will copy contents of new target to working surface
		void	SwitchTarget( plDynamicTextMap *target );		// Will force a flush

		// Clears and resets everything. Does NOT flush.
		void	Reset( void );

		hsBool	IsValid( void ) const;

		static hsBool	CanHandleLotsOfThem( void );

	protected:

		//// Protected Members ////

		void		IInit( void );
		void		IEnsureSurfaceUpdated( void );
		void		IRefreshOSJustify( void );
		void		ISetTextColor( hsColorRGBA &color, hsBool blockRGB );

		void		ISetFont( const char *face, UInt16 size, UInt8 fontFlags = 0, hsBool antiAliasRGB = true );

		plDynamicTextMap	*fCurrTarget;
		UInt32				fFlags;
		Justify				fJustify;
		hsBool				fFlushed;

		char		*fFontFace;
		UInt16		fFontSize;
		UInt8		fFontFlags;
		hsBool		fFontAntiAliasRGB;
		hsBool		fFontBlockedRGB;

		static hsBool		fForceSharedSurfaces;
		static hsBool		fOSDetected;
		static hsBool		fOSCanShareSurfaces;

#if HS_BUILD_FOR_WIN32
		class plWinSurface
		{
			protected:
				void		*fBits;

				virtual UInt8	IBitsPerPixel( void ) const = 0;

			public:
				HDC			fDC;
				HBITMAP		fBitmap;
				HFONT		fFont;
				COLORREF	fTextColor;
				int			fSaveNum;

				UInt16		fWidth, fHeight;

				char		*fFontFace;
				UInt16		fFontSize;
				UInt8		fFontFlags;
				hsBool		fFontAntiAliasRGB, fFontBlockedRGB;

				plWinSurface();
				~plWinSurface();

				void	Allocate( UInt16 w, UInt16 h );
				void	Release( void );

				hsBool	WillFit( UInt16 w, UInt16 h );
				hsBool	FontMatches( const char *face, UInt16 size, UInt8 flags, hsBool aaRGB );
				void	SetFont( const char *face, UInt16 size, UInt8 flags, hsBool aaRGB );
		};

		class plWinRGBSurface : public plWinSurface
		{
				virtual UInt8	IBitsPerPixel( void ) const { return 32; }
			public:
				UInt32	*GetBits( void ) const { return (UInt32 *)fBits; }
		};

		class plWinAlphaSurface : public plWinSurface
		{
				virtual UInt8	IBitsPerPixel( void ) const { return 8; }
			public:
				UInt8	*GetBits( void ) const { return (UInt8 *)fBits; }
		};

		plWinRGBSurface		fRGBSurface;
		plWinAlphaSurface	fAlphaSurface;
#endif
};


#endif // _plDynSurfaceWriter_h
