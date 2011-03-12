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
//////////////////////////////////////////////////////////////////////////////
//																			//
//	plDebugText and plDebugTextManager Headers								//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plDebugText_h
#define _plDebugText_h

#include "hsTypes.h"
#include "hsColorRGBA.h"
#include "hsTemplates.h"
#include "hsUtils.h"

//// plDebugText Class Definition ////////////////////////////////////////////

class plPipeline;
class plDebugTextManager;

class plDebugText
{
	private:

		static plDebugText	fInstance;

		plDebugText() 
		{ 
			fManager = nil;
#ifdef PLASMA_EXTERNAL_RELEASE
			SetFont( "Trebuchet MS Bold", 8 );
#else
			SetFont( "Courier New", 8 );
#endif
			SetEnable( true );
			fLockEnable = false;
			fDrawOnTopMode = false;
		}

	protected:

		plDebugTextManager	*fManager;

		char			fFontFace[ 128 ];
		UInt16			fFontSize;
		hsBool			fEnabled, fLockEnable, fDrawOnTopMode;

	public:

		enum Styles
		{
			kStyleItalic = 0x01,
			kStyleBold = 0x02
		};

		~plDebugText() { ; }
		
		static plDebugText	&Instance( void ) { return fInstance; }

		UInt32	CalcStringWidth( const char *string );

		void	DrawString( UInt16 x, UInt16 y, const char *string, UInt32 hexColor, UInt8 style = 0 );

		void	DrawString( UInt16 x, UInt16 y, const char *string, hsColorRGBA &color, UInt8 style = 0 )
		{
			UInt32	hex;
			UInt8	r, g, b, a;


			r = (UInt8)( color.r * 255.0 );
			g = (UInt8)( color.g * 255.0 );
			b = (UInt8)( color.b * 255.0 );
			a = (UInt8)( color.a * 255.0 );
			hex = ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | ( b );

			DrawString( x, y, string, hex, style );
		}

		void	DrawString( UInt16 x, UInt16 y, const char *string, UInt8 r = 255, UInt8 g = 255, UInt8 b = 255, UInt8 a = 255, UInt8 style = 0 )
		{
			DrawString( x, y, string, (UInt32)( ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | ( b ) ), style );
		}

		void	SetDrawOnTopMode( hsBool enable ) { fDrawOnTopMode = enable; }

		/// TEMPORARY FUNCTION (until we can find a better way to do this, one way or the other)
		void	DrawRect( UInt16 left, UInt16 top, UInt16 right, UInt16 bottom, UInt32 hexColor );

		/// TEMPORARY FUNCTION (until we can find a better way to do this, one way or the other)
		void	DrawRect( UInt16 left, UInt16 top, UInt16 right, UInt16 bottom, UInt8 r, UInt8 g, UInt8 b, UInt8 a = 255 )
		{
			DrawRect( left, top, right, bottom, (UInt32)( ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | ( b ) ) );
		}

		/// EVEN MORE TEMPORARY FUNCTION (until we can find a better way to do this, one way or the other)
		void	Draw3DBorder( UInt16 left, UInt16 top, UInt16 right, UInt16 bottom, UInt32 hexColor1, UInt32 hexColor2 );

		void	SetManager( plDebugTextManager *m ) { fManager = m; }

		void			SetFont( char *face, UInt16 size ) { hsStrncpy( fFontFace, face, sizeof( fFontFace ) ); fFontSize = size; }
		const char		*GetFontFace( void ) { return fFontFace; }
		const UInt16	GetFontSize( void ) { return fFontSize; }
		UInt16			GetFontHeight();
		
		void			SetEnable( hsBool on ) { fEnabled = on; }
		void			DisablePermanently( void ) { fEnabled = false; fLockEnable = true; }
		const hsBool	IsEnabled( void ) { return fEnabled; }

		void			GetScreenSize( UInt32 *width, UInt32 *height );
};

//// plDebugTextManager Class Definition /////////////////////////////////////

class plTextFont;

class	plDebugTextManager
{
	protected:

		struct plDebugTextNode
		{
			char	fText[ 256 ];
			UInt32	fColor, fDarkColor;
			UInt16	fX, fY, fRight, fBottom;	// Last 2 are for rects only
			UInt8	fStyle;						// 0xff means rectangle, 0xfe means 3d border

			plDebugTextNode() { fText[ 0 ] = 0; fColor = 0; fX = fY = 0; fStyle = 0; }
			plDebugTextNode( const char *s, UInt32 c, UInt16 x, UInt16 y, UInt8 style ); 
			plDebugTextNode( UInt16 left, UInt16 top, UInt16 right, UInt16 bottom, UInt32 c ); 
			plDebugTextNode( UInt16 left, UInt16 top, UInt16 right, UInt16 bottom, UInt32 c1, UInt32 c2 );
			~plDebugTextNode() {;}
		};

		hsTArray<plDebugTextNode>	fList;
		hsTArray<plDebugTextNode>	fDrawOnTopList;

		plTextFont					*fFont;
		UInt32						fSWidth, fSHeight;

	public:

		plDebugTextManager() { plDebugText::Instance().SetManager( this ); fFont = nil; }
		~plDebugTextManager();

		void	AddString( UInt16 x, UInt16 y, const char *s, UInt32 hexColor, UInt8 style, hsBool drawOnTop = false );
		UInt32	CalcStringWidth( const char *string );

		/// TEMPORARY FUNCTION (until we can find a better way to do this, one way or the other)
		void	DrawRect( UInt16 left, UInt16 top, UInt16 right, UInt16 bottom, UInt32 hexColor, hsBool drawOnTop = false );

		/// EVEN MORE TEMPORARY FUNCTION (until we can find a better way to do this, one way or the other)
		void	Draw3DBorder( UInt16 left, UInt16 top, UInt16 right, UInt16 bottom, UInt32 hexColor1, UInt32 hexColor2, hsBool drawOnTop = false );

		void	DrawToDevice( plPipeline *pipe );

		void	GetScreenSize( UInt32 *width, UInt32 *height );

		UInt16	GetFontHeight();
};


#endif //_plDebugText_h

