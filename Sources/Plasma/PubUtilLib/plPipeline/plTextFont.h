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
//	plTextFont Class Header		    								 		 //
//	Generic 3D text font handler       										 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	2.19.2001 mcn - Created.												 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plTextFont_h
#define _plTextFont_h

#include "hsTemplates.h"
#include "hsGeometry3.h"

//// plTextFont Class Definition //////////////////////////////////////////////

class plPipeline;

class plTextFont
{
	protected:

		struct plDXCharInfo
		{
			UInt16		fW, fH;
			hsPoint3	fUVs[ 2 ];
		};

		struct plFontVertex
		{
			hsPoint3	fPoint;
			UInt32		fColor;
			hsPoint3	fUV;

			plFontVertex& operator=(const int zero)
			{
				fPoint.Set(0,0,0);
				fColor = 0;
				fUV.Set(0,0,0);

				return *this;
			}
		};

		UInt32	fMaxNumIndices;
		UInt32	fTextureWidth, fTextureHeight;

		char	fFace[ 128 ];
		UInt16	fSize;
		hsBool	fInitialized;
		UInt16	fFontHeight;
		
		plPipeline	*fPipe;

		plTextFont		*fNext;
		plTextFont		**fPrevPtr;

		plDXCharInfo	fCharInfo[ 128 ];


		virtual void	IInitObjects( void );
		virtual void	ICreateTexture( UInt16 *data ) = 0;
		virtual void	IInitStateBlocks( void ) = 0;
		virtual void	IDrawPrimitive( UInt32 count, plFontVertex *array ) = 0;
		virtual void	IDrawLines( UInt32 count, plFontVertex *array ) = 0;
		
		UInt16	*IInitFontTexture( void );

		void	IUnlink( void )
		{
			hsAssert( fPrevPtr, "Font not in list" );
			if( fNext )
				fNext->fPrevPtr = fPrevPtr;
			*fPrevPtr = fNext;

			fNext = nil;
			fPrevPtr = nil;
		}

	public:

		plTextFont( plPipeline *pipe );
		virtual ~plTextFont();

		void	Create( char *face, UInt16 size );
		void	DrawString( const char *string, int x, int y, UInt32 hexColor, UInt8 style, UInt32 rightEdge = 0 );
		void	DrawRect( int left, int top, int right, int bottom, UInt32 hexColor );
		void	Draw3DBorder( int left, int top, int right, int bottom, UInt32 hexColor1, UInt32 hexColor2 );
		UInt32	CalcStringWidth( const char *string );
		UInt32	GetFontSize( void ) { return fSize; }

		UInt16	GetFontHeight() { return fFontHeight; }

		virtual void	DestroyObjects( void ) = 0;
		virtual void	SaveStates( void ) = 0;
		virtual void	RestoreStates( void ) = 0;
		virtual void	FlushDraws( void ) = 0;

		void	Link( plTextFont **back )
		{
			fNext = *back;
			if( *back )
				(*back)->fPrevPtr = &fNext;
			fPrevPtr = back;
			*back = this;
		}

};


#endif // _plTextFont_h

