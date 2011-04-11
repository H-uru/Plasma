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
//	plTextGenerator Class Header		    						 		 //
//	Helper utility class that "attaches" to a mipmap and fills that mipmap	 //
//	with text.
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	12.13.2001 mcn - Created.												 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plTextGenerator_h
#define _plTextGenerator_h

#include "hsTemplates.h"
#include "hsGeometry3.h"
#include "hsColorRGBA.h"

//// plTextGenerator Class Definition //////////////////////////////////////////////

class plMipmap;
struct hsMatrix44;

class plTextGenerator : public hsKeyedObject
{
	protected:
	
		plMipmap	*fHost;
		UInt16		fWidth, fHeight;

#if HS_BUILD_FOR_WIN32
		HDC			fWinRGBDC;
		HBITMAP		fWinRGBBitmap;
		HFONT		fWinFont;
		UInt32		*fWinRGBBits;

		HFONT		fWinAlphaFont;
		HDC			fWinAlphaDC;
		HBITMAP		fWinAlphaBitmap;
		UInt8		*fWinAlphaBits;
#endif

		UInt32		*IAllocateOSSurface( UInt16 width, UInt16 height );
		void		IDestroyOSSurface( void );

	public:

		plTextGenerator();
		plTextGenerator( plMipmap *host, UInt16 width, UInt16 height );
		virtual ~plTextGenerator();

		void	Attach( plMipmap *host, UInt16 width, UInt16 height );
		void	Detach( void );

		/// Operations to perform on the text block
		
		void	ClearToColor( hsColorRGBA &color );

		void	SetFont( const char *face, UInt16 size, hsBool antiAliasRGB = true );
		void	SetTextColor( hsColorRGBA &color, hsBool blockRGB = false );

		void		DrawString( UInt16 x, UInt16 y, const char *text );
		void		DrawString( UInt16 x, UInt16 y, const wchar_t *text );
		void		DrawClippedString( Int16 x, Int16 y, const char *text, UInt16 width, UInt16 height );
		void		DrawClippedString( Int16 x, Int16 y, const wchar_t *text, UInt16 width, UInt16 height );
		void		DrawClippedString( Int16 x, Int16 y, const char *text, UInt16 clipX, UInt16 clipY, UInt16 width, UInt16 height );
		void		DrawClippedString( Int16 x, Int16 y, const wchar_t *text, UInt16 clipX, UInt16 clipY, UInt16 width, UInt16 height );
		void		DrawWrappedString( UInt16 x, UInt16 y, const char *text, UInt16 width, UInt16 height );
		void		DrawWrappedString( UInt16 x, UInt16 y, const wchar_t *text, UInt16 width, UInt16 height );
		UInt16		CalcStringWidth( const char *text, UInt16 *height = nil );
		UInt16		CalcStringWidth( const wchar_t *text, UInt16 *height = nil );
		void		CalcWrappedStringSize( const char *text, UInt16 *width, UInt16 *height );
		void		CalcWrappedStringSize( const wchar_t *text, UInt16 *width, UInt16 *height );
		void		FillRect( UInt16 x, UInt16 y, UInt16 width, UInt16 height, hsColorRGBA &color );
		void		FrameRect( UInt16 x, UInt16 y, UInt16 width, UInt16 height, hsColorRGBA &color );

		void	FlushToHost( void );

		UInt16	GetTextWidth( void );
		UInt16	GetTextHeight( void );

		UInt16	GetWidth( void ) { return fWidth; }
		UInt16	GetHeight( void ) { return fHeight; }

		// Since the textGen can actually create a texture bigger than you were expecting,
		// you want to be able to apply a layer texture transform that will compensate. This
		// function will give you that transform. Just feed it into plLayer->SetTransform().

		hsMatrix44	GetLayerTransform( void );


		virtual hsBool MsgReceive( plMessage *msg );
};


#endif // _plTextGenerator_h

