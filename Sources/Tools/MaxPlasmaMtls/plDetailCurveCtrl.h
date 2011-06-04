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
//	plDetailCurveCtrl Class Header											 //
//	Custom Win32 Control class for drawing the detail map opacity curve so	 //
//	the artists can figure out what the hell is going on.					 //
//	Cyan, Inc.																 //
//																			 //
//	To use:																	 //
//		1. Create a new plDetailCurveCtrl, giving it a parent window and a	 //
//		   client rect.														 //
//		2. Set the start and end percentages, along with the start and end	 //
//		   opacities.														 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	10.1.2001 mcn - Created.												 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plDetailCurveCtrl_h
#define _plDetailCurveCtrl_h

#include "Max.h"
#include "resource.h"


#define GET_DETAIL_CURVE_CTRL( dlg, id ) (GetDlgItem( dlg, id ) ? (plDetailCurveCtrl *)GetWindowLong( GetDlgItem( dlg, id ), GWL_USERDATA ) : NULL )

// Message to parent to let it know a point got dragged. lParam = pointer to control, wParam = 1 if start point, 0 if end point
#define PL_DC_POINT_DRAGGED	WM_USER + 50
#define PL_DC_START_POINT	1
#define PL_DC_END_POINT		0

/// The following #define was for back when I had two graphs, one was with mipmap level
/// as the X axis, the other was as it is now (distance). Uncomment this define to
/// re-enable the two-graph mode (right-click switches)
//#define MCN_TWO_GRAPH_MODE

//// Class Definition /////////////////////////////////////////////////////////

class plDetailCurveCtrl
{
	protected:

		HWND	fHWnd;

		HDC		fDblDC;
		HBITMAP	fDblBitmap;
		HBRUSH	fWhiteBrush, fBlueBrush;
		HPEN	fBluePen, fLiteBluePen;
		
		RECT	fStartDragPt, fEndDragPt;

		bool	fDraggingStart, fDraggingEnd;
		bool	fCanDragStart, fCanDragEnd;

		int		fNumLevels;
		float	fStartPercent, fEndPercent;
		float	fStartOpac, fEndOpac;

		void	IInitDblBuffer( void );
		void	IRefreshDblBuffer( void );
		void	IDrawCurve( HDC hDC, bool clampToInts, int cornerX, int cornerY, SIZE *bgndSize );

		float	IXlateDistToValue( float dist, bool clampToInts );
		float	IXlateDistToX( float dist, bool clampToInts );
		float	IXlateXToDist( float howFar );
		void	IXlateValuesToClientPt( float x, float y, POINT *pt, int cornerX, int cornerY, SIZE *bgndSize );
		void	IMapMouseToValues( int x, int y, bool mapToStart );

		void	ISendDraggedMessage( bool itWasTheStartPoint );

		static HINSTANCE	fInstance;
		static int			fClassRefCnt;
		static HBITMAP		fBgndImage;
		static HFONT		fFont;

#ifdef MCN_TWO_GRAPH_MODE
		static HBITMAP		fBgndImage2;
		static bool			fXAsMipmapLevel;
#endif

		static void	IRegisterCtrl( HINSTANCE instance );
		static void	IUnregisterCtrl( void );

		static LRESULT CALLBACK	IWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );


	public:

		plDetailCurveCtrl( HWND parentWnd, WPARAM id, RECT *clientRect, HINSTANCE instance = NULL );
		~plDetailCurveCtrl();

		void	SetStartPoint( float percentLevel, float opacity );
		void	SetEndPoint( float percentLevel, float opacity );
		void	SetNumLevels( int numLevels );

		void	GetStartPoint( float &percent, float &opacity ) { percent = fStartPercent; opacity = fStartOpac; }
		void	GetEndPoint( float &percent, float &opacity ) { percent = fEndPercent; opacity = fEndOpac; }

};

#endif // _plDetailCurveCtrl_h
