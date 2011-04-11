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
#ifndef _pfGUISkinComp_h
#define _pfGUISkinComp_h


#include "plComponent.h"
#include "../MaxPlasmaMtls/Layers/plLayerTex.h"
#include "../pfGameGUIMgr/pfGUIPopUpMenu.h"

/// skin component class
class plGUISkinComp : public plComponent
{
public:
	plGUISkinComp();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);

	hsBool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

	hsBool DeInit(plMaxNode *node, plErrorMsg *pErrMsg);

	plLayerTex	*GetSkinBitmap( void );

	virtual UInt32	GetNumMtls( void ) const;
	virtual Texmap	*GetMtl( UInt32 idx );

	enum
	{
		kRefBitmap = 256,			// So we can share it among other components
		kRefUpLeftCorner		= 257,
		kRefTopSpan				= 257 + 4,
		kRefUpRightCorner		= 257 + 8,
		kRefRightSpan			= 257 + 12,
		kRefLowerRightCorner	= 257 + 16,
		kRefBottomSpan			= 257 + 20,
		kRefLowerLeftCorner		= 257 + 24,
		kRefLeftSpan			= 257 + 28,
		kRefMiddleFill			= 257 + 32,
		kRefSelectedFill		= 257 + 36,
		kRefSubMenuArrow		= 257 + 40,
		kRefSelectedSubMenuArrow = 257 + 44,
		kRefTreeButtonClosed	= 257 + 48,
		kRefTreeButtonOpen		= 257 + 52,
		kRefItemMargin			= 400,
		kRefBorderMargin
	};

	pfGUISkin	*GetConvertedSkin( void ) const { return fConvertedSkin; }
	plKey		GetConvertedSkinKey( void ) const;

	// Given an INode, gives you a pointer to the GUI component if it actually is one, nil otherwise
	static plGUISkinComp		*GetGUIComp( INode *node );
	
protected:

	pfGUISkin	*fConvertedSkin;
};

/// skin editor proc class
class pfGUISkinEditProc
{
	protected:
		static pfGUISkinEditProc	*fInstance;

		static int		fZoom;

		plGUISkinComp	*fComp;
		RECT			fPreviewRect, fCurrElemRect;
		HDC				fDblDC, fImageDC;
		HBITMAP			fDblBitmap, fImageBitmap;
		int				fDblWidth, fDblHeight;
		int				fXOffset, fYOffset;
		int				fCurrPBRefSet;
		HWND			fHWnd;
		HPEN			fDefPen, fOtherPen;

		bool			fDragging;
		UInt8			fDragType;
		int				fDragTimer;
		int				fDragOffsetX, fDragOffsetY;
		HCURSOR			fOrigCursor;

		pfGUISkin::pfSRect	fBackups[ pfGUISkin::kNumElements ];


		void	IRefreshDblBuffer( void );
		void	IRefreshImageBuffer( void );
		void	IInitDblBuffer( void );
		void	IKillDblBuffer( void );

		void	ISetScrollRanges( void );

		enum
		{
			kRangeSlop = 4
		};

		bool	IPointWithinRange( int x, int y, int ptX, int ptY );
		bool	IPointWithinVertRange( int x, int y, int ptX, int ptY1, int ptY2 );
		bool	IPointWithinHorzRange( int x, int y, int ptX1, int ptX2, int ptY );

		enum DragTypeFlags
		{
			kLeft	= 0x01,
			kTop	= 0x02,
			kRight	= 0x04,
			kBottom	= 0x08,
			kDragAll = kLeft | kTop | kRight | kBottom
		};

		UInt8	IGetDragTypeFlags( int x, int y );

		void	IJustDrawOneRect( int whichElement, IParamBlock2 *pb, HDC hDC, HPEN whichPen, int refToIgnore );

	public:

		pfGUISkinEditProc( plGUISkinComp *comp );
		~pfGUISkinEditProc();

		static INT_PTR CALLBACK	DlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );

		INT_PTR CALLBACK	DialogProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
};


#endif // _pfGUISkinComp_h