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
//	plDetailCurveCtrl Class Functions										 //
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

#include "hsTypes.h"
#include "plDetailCurveCtrl.h"
#include "resource.h"


//// Static Stuff /////////////////////////////////////////////////////////////

int			plDetailCurveCtrl::fClassRefCnt = 0;
HINSTANCE	plDetailCurveCtrl::fInstance = NULL;
HBITMAP		plDetailCurveCtrl::fBgndImage = NULL;
HFONT		plDetailCurveCtrl::fFont = NULL;

#ifdef MCN_TWO_GRAPH_MODE
HBITMAP		plDetailCurveCtrl::fBgndImage2 = NULL;
bool		plDetailCurveCtrl::fXAsMipmapLevel = false;
#endif

const char	gCtrlClassName[] = "DetailCurveClass";

#define kHiResStep	0.01f


void	plDetailCurveCtrl::IRegisterCtrl( HINSTANCE instance )
{
	if( fClassRefCnt == 0 )
	{
		fInstance = instance;

		WNDCLASSEX	clInfo;

		memset( &clInfo, 0, sizeof( clInfo ) );
		clInfo.cbSize = sizeof( clInfo );
		clInfo.style = CS_OWNDC | CS_NOCLOSE;
		clInfo.lpfnWndProc = (WNDPROC)IWndProc;
		clInfo.cbClsExtra = 0;
		clInfo.cbWndExtra = 0;
		clInfo.hInstance = fInstance;
		clInfo.hIcon = NULL;
		clInfo.hCursor = LoadCursor( NULL, IDC_CROSS );
		clInfo.hbrBackground = NULL;
		clInfo.lpszMenuName = NULL;
		clInfo.lpszClassName = gCtrlClassName;
		clInfo.hIconSm = NULL;

		RegisterClassEx( &clInfo );

		fBgndImage = (HBITMAP)LoadImage( fInstance, MAKEINTRESOURCE( IDB_DETAILBGND ), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR );
#ifdef MCN_TWO_GRAPH_MODE
		fBgndImage2 = (HBITMAP)LoadImage( fInstance, MAKEINTRESOURCE( IDB_DETAILBGND2 ), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR );
#endif

		HDC hDC = GetDC( NULL );
		fFont = CreateFont( -MulDiv( 8, GetDeviceCaps( hDC, LOGPIXELSY ), 72 ), 0, 0, 0, 0, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 
							CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial" );
		ReleaseDC( NULL, hDC );
	}

	fClassRefCnt++;
}

void	plDetailCurveCtrl::IUnregisterCtrl( void )
{
	fClassRefCnt--;
	if( fClassRefCnt == 0 )
	{
		UnregisterClass( gCtrlClassName, fInstance );
		if( fFont != NULL )
			DeleteObject( fFont );
	}
}

//// Constructor & Destructor /////////////////////////////////////////////////

plDetailCurveCtrl::plDetailCurveCtrl( HWND parentWnd, WPARAM id, RECT *clientRect, HINSTANCE instance )
{
	// Class init
	if( instance == NULL )
		instance = (HINSTANCE)GetWindowLong( parentWnd, GWL_HINSTANCE );
	IRegisterCtrl( instance );

	// Per-object init
	fDblDC = NULL;
	fDblBitmap = NULL;
	fStartPercent = 0;
	fStartOpac = 0;
	fEndPercent = 1.f;
	fEndOpac = 1.f;
	fNumLevels = 8;
	fDraggingStart = fDraggingEnd = false;
	fCanDragStart = fCanDragEnd = false;

	// Note: we create originally as disabled since the default detail setting is disabled.
	// The MAX Update stuff should change this if necessary after we're created
	fHWnd = ::CreateWindowEx( WS_EX_CLIENTEDGE, gCtrlClassName, "Detail Curve", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_DISABLED,
							clientRect->left, clientRect->top, clientRect->right - clientRect->left,
							clientRect->bottom - clientRect->top,
							parentWnd, (HMENU)id, instance, 0 );
	if( fHWnd == NULL )
		return;

	SetWindowLong( fHWnd, GWL_USERDATA, (LONG)this );
}

plDetailCurveCtrl::~plDetailCurveCtrl()
{
	if( fDblDC != NULL )
	{
		SelectObject( fDblDC, (HBITMAP)NULL );
		DeleteObject( fDblBitmap );
		DeleteDC( fDblDC );
	}

	if( fWhiteBrush != NULL )
		DeleteObject( fWhiteBrush );
	if( fBluePen != NULL )
		DeleteObject( fBluePen );
	if( fLiteBluePen != NULL )
		DeleteObject( fLiteBluePen );
	if( fBlueBrush != NULL )
		DeleteObject( fBlueBrush );

//	DestroyWindow( fHWnd );
	IUnregisterCtrl();
}

//// IInitDblBuffer ///////////////////////////////////////////////////////////
//	Note: For some strange reason, grabbing the HDC of the window doesn't do
//	any good, 'cause it's black-and-white (right, THAT makes sense). Grabbing
//	the desktop DC works, however.

void	plDetailCurveCtrl::IInitDblBuffer( void )
{
	if( fDblDC == NULL )
	{
		int		width, height;
		RECT	r;
		HDC		desk = GetDC( NULL );

		GetClientRect( fHWnd, &r );
		width = r.right - r.left;
		height = r.bottom - r.top;

		fDblDC = CreateCompatibleDC( desk );
		fDblBitmap = CreateCompatibleBitmap( desk/*fDblDC*/, width, height );
		SelectObject( fDblDC, fDblBitmap );
		ReleaseDC( NULL, desk );

		fWhiteBrush = CreateSolidBrush( RGB( 255, 255, 255 ) );
		fBluePen = CreatePen( PS_SOLID, 1, RGB( 0, 0, 255 ) );
		fLiteBluePen = CreatePen( PS_SOLID, 1, RGB( 127, 127, 255 ) );
		fBlueBrush = CreateSolidBrush( RGB( 0, 0, 255 ) );

		IRefreshDblBuffer();
	}
}

//// IRefreshDblBuffer ////////////////////////////////////////////////////////

void	plDetailCurveCtrl::IRefreshDblBuffer( void )
{
	HDC			hBgndDC;
	RECT		clientRect, r;
	SIZE		bgndSize;
	int			width, height, x, y;
	HPEN		oldPen;
	BITMAPINFO	bmpInfo;
	POINT		pt1, pt2;


	IInitDblBuffer();

	GetClientRect( fHWnd, &clientRect );
	width = clientRect.right - clientRect.left;
	height = clientRect.bottom - clientRect.top;

	if( fDblBitmap != NULL )
	{
		FillRect( fDblDC, &clientRect, fWhiteBrush );

		if( fBgndImage != NULL )
		{

			// Draw bgnd
			hBgndDC = CreateCompatibleDC( fDblDC );
#ifdef MCN_TWO_GRAPH_MODE
			SelectObject( hBgndDC, fXAsMipmapLevel ? fBgndImage2 : fBgndImage );
#else
			SelectObject( hBgndDC, fBgndImage );
#endif

			bmpInfo.bmiHeader.biSize = sizeof( bmpInfo.bmiHeader );
			bmpInfo.bmiHeader.biBitCount = 0;
			GetDIBits( hBgndDC, fBgndImage, 0, 0, NULL, &bmpInfo, DIB_RGB_COLORS );
			bgndSize.cx = bmpInfo.bmiHeader.biWidth;
			bgndSize.cy = bmpInfo.bmiHeader.biHeight;
			x = ( width - bgndSize.cx ) >> 1;
			y = ( height - bgndSize.cy ) >> 1;

			BitBlt( fDblDC, x, y, bgndSize.cx, bgndSize.cy, hBgndDC, 0, 0, SRCCOPY );
			SelectObject( hBgndDC, (HBITMAP)NULL );
			DeleteDC( hBgndDC );

			/// Draw graph
			if( IsWindowEnabled( fHWnd ) )
			{
				bgndSize.cx -= 8;
				oldPen = (HPEN)SelectObject( fDblDC, fLiteBluePen );

				/// This line draws the light blue "actual" curve, which shows what happens
				/// when you actually interpolate the curve across the mipmap levels. It's
				/// more accurate in that it shows the actual values computed for each level,
				/// but less accurate because it doesn't take into account mipmap sampling
				/// and such. Given the latter, we leave it out (for now) to avoid confusing
				/// the artists.
//				IDrawCurve( fDblDC, true, x, y, &bgndSize );

				SelectObject( fDblDC, fBluePen );
				IDrawCurve( fDblDC, false, x, y, &bgndSize );

				SelectObject( fDblDC, oldPen );

				if( fStartPercent == 0.07f && fStartOpac == 0.23f && fEndPercent == 0.19f && fEndOpac == 0.79f )
				{
					const char	str[] = "\x48\x61\x70\x70\x79\x20\x62\x64\x61\x79\x20\x74\x6f\x20\x6d\x63\x6e\x21";
					SetBkMode( fDblDC, TRANSPARENT );
					SetTextColor( fDblDC, RGB( 0, 0, 255 ) );
					SelectObject( fDblDC, fFont );
					TextOut( fDblDC, x, y + bgndSize.cy - 10, str, strlen( str ) );
				}

				// Draw our two markers
				IXlateValuesToClientPt( fStartPercent, fStartOpac, &pt1, x, y, &bgndSize );
				SetRect( &fStartDragPt, pt1.x - 4, pt1.y - 4, pt1.x + 4, pt1.y + 4 );
				r = fStartDragPt;
				if( !fCanDragStart )
					InflateRect( &r, -2, -2 );
				FillRect( fDblDC, &r, fBlueBrush );

				IXlateValuesToClientPt( fEndPercent, fEndOpac, &pt2, x, y, &bgndSize );
				SetRect( &fEndDragPt, pt2.x - 4, pt2.y - 4, pt2.x + 4, pt2.y + 4 );
				r = fEndDragPt;
				if( !fCanDragEnd )
					InflateRect( &r, -2, -2 );
				FillRect( fDblDC, &r, fBlueBrush );
			}
		}
	}
}

//// IDrawCurve ///////////////////////////////////////////////////////////////
//	Draw the damned curve.

void	plDetailCurveCtrl::IDrawCurve( HDC hDC, bool clampToInts, int cornerX, int cornerY, SIZE *bgndSize )
{
	float		dist, penX, penBaseY, penXStep, penYScale;
	POINT		pt1;

	
	// Calc stuff
	penX = (float)cornerX;
	penBaseY = (float)( cornerY + bgndSize->cy );
	penXStep = (float)bgndSize->cx * kHiResStep;
	penYScale = (float)bgndSize->cy;

	// Draw curve
	pt1.x = (int)penX;
	pt1.y = (int)( penBaseY - penYScale * fStartOpac );

	float	artificialBias = 1.f / (float)fNumLevels;	//	So we never get a howFar less than 0
	float	artificialMaxDist = 1.f - artificialBias;

	for( dist = 0.f; dist <= 1.f; dist += kHiResStep )
	{
		float opac = IXlateDistToValue( dist, clampToInts );

		if( dist == 0.f )
			MoveToEx( hDC, (int)penX, (int)( penBaseY - penYScale * opac ), NULL );
		else
			LineTo( hDC, (int)penX, (int)( penBaseY - penYScale * opac ) );

		penX += penXStep;
	}
}

//// IXlateDistToValue ////////////////////////////////////////////////////////
//	I.E. from distance across graph to distance up on graph (percentage-wise)

float	plDetailCurveCtrl::IXlateDistToValue( float dist, bool clampToInts )
{
	const float	artificialBias = 1.f / (float)fNumLevels;	//	So we never get a howFar less than 0
	const float	artificialMaxDist = 1.f - artificialBias;
	float		howFar, opac;


	howFar = IXlateDistToX( dist, clampToInts );

	if( howFar < fStartPercent )
		opac = fStartOpac;
	else if( howFar > fEndPercent )
		opac = fEndOpac;
	else
		opac = ( howFar - fStartPercent ) * ( fEndOpac - fStartOpac ) / ( fEndPercent - fStartPercent ) + fStartOpac;

	return opac;
}

//// IXlateDistToX ////////////////////////////////////////////////////////////
//	I.E. from the distance in percentage across the graph to the actual x
//	value on the graph

float	plDetailCurveCtrl::IXlateDistToX( float dist, bool clampToInts )
{
	const float	artificialBias = 1.f / (float)fNumLevels;	//	So we never get a howFar less than 0
	const float	artificialMaxDist = 1.f - artificialBias;
	float		howFar;


#ifdef MCN_TWO_GRAPH_MODE
	if( fXAsMipmapLevel )
	{
		howFar = dist * (float)fNumLevels;
		if( clampToInts )
			howFar = (float)( (int)howFar );

		howFar /= (float)fNumLevels;
		return howFar;
	}
#endif

	if( dist == 0.f )
		howFar = 0.f;
	else
	{
		howFar = 1.f - ( ( 1.f - dist ) * artificialMaxDist );
		howFar = ( (float)fNumLevels - 1.f / howFar );
		if( howFar < 0.f )
			howFar = 0.f;
		else if( howFar > (float)fNumLevels - 1.f )
			howFar = (float)fNumLevels - 1.f;

		if( clampToInts )
			howFar = (float)( (int)howFar );

		howFar /= (float)fNumLevels - 1.f;
	}

	return howFar;
}

//// IXlateXToDist ////////////////////////////////////////////////////////////
//	I.E. from the actual x value of the graph to the actual distance in
//	percentage across the graph.

float	plDetailCurveCtrl::IXlateXToDist( float howFar )
{
	const float	artificialBias = 1.f / (float)fNumLevels;	//	So we never get a howFar less than 0
	const float	artificialMaxDist = 1.f - artificialBias;
	float		dist;


#ifdef MCN_TWO_GRAPH_MODE
	if( fXAsMipmapLevel )
	{
		return howFar;
	}
#endif

	if( howFar == 0.f )
		dist = 0.f;
	else
	{
		howFar *= (float)fNumLevels - 1.f;
		howFar = 1.f / ( (float)fNumLevels - howFar );
		howFar = ( ( howFar - 1.f ) / artificialMaxDist ) + 1.f;
	}

	return howFar;
}

//// IXlateValuesToClientPt ///////////////////////////////////////////////////
//	I.E. from graph x,y values to client coordinates

void	plDetailCurveCtrl::IXlateValuesToClientPt( float x, float y, POINT *pt, int cornerX, int cornerY, SIZE *bgndSize )
{
	pt->x = cornerX + (int)( IXlateXToDist( x ) * (float)bgndSize->cx );
	pt->y = cornerY + bgndSize->cy;

	pt->y -= (int)( (float)bgndSize->cy * y );
}

//// IMapMouseToValues ////////////////////////////////////////////////////////
//	Map mouse x,y coordinates in clientspace to graph values. If the last param
//	is true, maps to the start point, else maps to the end point

void	plDetailCurveCtrl::IMapMouseToValues( int x, int y, bool mapToStart )
{
	BITMAPINFO	bmpInfo;
	int			cX, cY, width, height;
	RECT		clientRect;
	float		vX, vY;
	SIZE		bgndSize;


	if( fBgndImage == NULL || fDblDC == NULL || !IsWindowEnabled( fHWnd ) )
		return;

	GetClientRect( fHWnd, &clientRect );
	width = clientRect.right - clientRect.left;
	height = clientRect.bottom - clientRect.top;

	bmpInfo.bmiHeader.biSize = sizeof( bmpInfo.bmiHeader );
	bmpInfo.bmiHeader.biBitCount = 0;
	GetDIBits( fDblDC, fBgndImage, 0, 0, NULL, &bmpInfo, DIB_RGB_COLORS );
	bgndSize.cx = bmpInfo.bmiHeader.biWidth;
	bgndSize.cy = bmpInfo.bmiHeader.biHeight;
	cX = ( width - bgndSize.cx ) >> 1;
	cY = ( height - bgndSize.cy ) >> 1;

	bgndSize.cx -= 8;

	// Xlate to graph space and clamp
	x -= cX;
	y = bgndSize.cy - ( y - cY );
	if( x < 0 )
		x = 0;
	else if( x > bgndSize.cx )
		x = bgndSize.cx;
	if( y < 0 )
		y = 0;
	else if( y > bgndSize.cy )
		y = bgndSize.cy;

	vX = IXlateDistToX( (float)x / (float)bgndSize.cx, false );
	vY = (float)y / (float)bgndSize.cy;

	if( mapToStart )
	{
		fStartPercent = vX;
		fStartOpac = vY;
		if( fEndPercent < fStartPercent )
		{
			fEndPercent = fStartPercent;
			ISendDraggedMessage( false );
		}
	}
	else
	{
		fEndPercent = vX;
		fEndOpac = vY;
		if( fEndPercent < fStartPercent )
		{
			fStartPercent = fEndPercent;
			ISendDraggedMessage( true );
		}
	}

	IRefreshDblBuffer();
	InvalidateRect( fHWnd, NULL, false );
	RedrawWindow( fHWnd, NULL, NULL, RDW_UPDATENOW );

	ISendDraggedMessage( mapToStart );
}

//// ISendDraggedMessage //////////////////////////////////////////////////////

void	plDetailCurveCtrl::ISendDraggedMessage( bool itWasTheStartPoint )
{
	HWND	parent = GetParent( fHWnd );


	if( parent == NULL )
		return;

	SendMessage( parent, PL_DC_POINT_DRAGGED, itWasTheStartPoint ? PL_DC_START_POINT : PL_DC_END_POINT,
					(LPARAM)this );
}

//// SetStart/EndPoint ////////////////////////////////////////////////////////

void	plDetailCurveCtrl::SetStartPoint( float percentLevel, float opacity )
{
	fStartPercent = percentLevel;
	fStartOpac = opacity;
	IRefreshDblBuffer();
	InvalidateRect( fHWnd, NULL, false );
}

void	plDetailCurveCtrl::SetEndPoint( float percentLevel, float opacity )
{
	fEndPercent = percentLevel;
	fEndOpac = opacity;
	IRefreshDblBuffer();
	InvalidateRect( fHWnd, NULL, false );
}

void	plDetailCurveCtrl::SetNumLevels( int numLevels )
{
	fNumLevels = numLevels;
	IRefreshDblBuffer();
	InvalidateRect( fHWnd, NULL, false );
}

//// IWndProc /////////////////////////////////////////////////////////////////

LRESULT CALLBACK	plDetailCurveCtrl::IWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	HDC				hDC;
	RECT			clientRect;
	int				width, height;
	PAINTSTRUCT		pInfo;
	POINT			pt;


	plDetailCurveCtrl	*ctrl = (plDetailCurveCtrl *)GetWindowLong( hWnd, GWL_USERDATA );
	GetClientRect( hWnd, &clientRect );
	width = clientRect.right - clientRect.left;
	height = clientRect.bottom - clientRect.top;

	switch( msg )
	{
		case WM_CREATE:
			return 0;

		case WM_ENABLE:
			if( ctrl != NULL )
				ctrl->IRefreshDblBuffer();
			return 0;

		case WM_PAINT:
			BeginPaint( hWnd, &pInfo );
			hDC = (HDC)pInfo.hdc;

			if( ctrl != NULL )
			{
				if( ctrl->fDblDC == NULL )
					ctrl->IInitDblBuffer();

				BitBlt( hDC, 0, 0, width, height, ctrl->fDblDC, 0, 0, SRCCOPY );
			}

			EndPaint( hWnd, &pInfo );
			return 0;

		case WM_ERASEBKGND:
			return TRUE;

		case WM_LBUTTONDOWN:
			if( ctrl != NULL && !ctrl->fDraggingStart && !ctrl->fDraggingEnd )
			{
				pt.x = LOWORD( lParam );
				pt.y = HIWORD( lParam );
				if( PtInRect( &ctrl->fStartDragPt, pt ) )
				{
					if( !ctrl->fCanDragStart && !ctrl->fCanDragEnd )
						SetCapture( hWnd );
					ctrl->fDraggingStart = true;
				}
				else if( PtInRect( &ctrl->fEndDragPt, pt ) )
				{
					if( !ctrl->fCanDragStart && !ctrl->fCanDragEnd )
						SetCapture( hWnd );
					ctrl->fDraggingEnd = true;
				}
			}
			return 0;

		case WM_MOUSEMOVE:
			if( ctrl != NULL )
			{
				pt.x = LOWORD( lParam );
				pt.y = HIWORD( lParam );

				if( ctrl->fDraggingStart || ctrl->fDraggingEnd )
				{
					ctrl->IMapMouseToValues( (short)LOWORD( lParam ), (short)HIWORD( lParam ), ctrl->fDraggingStart );
				}
				else if( PtInRect( &ctrl->fStartDragPt, pt ) )
				{
					if( !ctrl->fCanDragStart )
					{
						ctrl->fCanDragStart = true;
						ctrl->fCanDragEnd = false;
						SetCapture( hWnd );
						ctrl->IRefreshDblBuffer();
						InvalidateRect( hWnd, NULL, false );
					}
				}
				else if( PtInRect( &ctrl->fEndDragPt, pt ) )
				{
					if( !ctrl->fCanDragEnd )
					{
						ctrl->fCanDragEnd = true;
						ctrl->fCanDragStart = false;
						SetCapture( hWnd );
						ctrl->IRefreshDblBuffer();
						InvalidateRect( hWnd, NULL, false );
					}
				}
				else if( ctrl->fCanDragStart || ctrl->fCanDragEnd )
				{
					ctrl->fCanDragStart = false;
					ctrl->fCanDragEnd = false;
					ReleaseCapture();
					ctrl->IRefreshDblBuffer();
					InvalidateRect( hWnd, NULL, false );
				}
			}
			return 0;

		case WM_LBUTTONUP:
			if( ctrl != NULL && ( ctrl->fDraggingStart || ctrl->fDraggingEnd ) )
			{
				if( !ctrl->fCanDragStart && !ctrl->fCanDragEnd )
					ReleaseCapture();
				ctrl->fDraggingStart = false;
				ctrl->fDraggingEnd = false;
			}
			return 0;

#ifdef MCN_TWO_GRAPH_MODE
		case WM_RBUTTONDOWN:
			fXAsMipmapLevel = !fXAsMipmapLevel;
			ctrl->IRefreshDblBuffer();
			InvalidateRect( hWnd, NULL, false );
			return 0;
#endif

		case WM_DESTROY:
			delete ctrl;
			SetWindowLong( hWnd, GWL_USERDATA, 0 );
			return 0;

		default:
			return DefWindowProc( hWnd, msg, wParam, lParam );
	}
}
