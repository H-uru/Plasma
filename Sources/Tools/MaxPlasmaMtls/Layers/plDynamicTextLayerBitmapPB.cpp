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
#include "hsTypes.h"
#include "plDynamicTextLayer.h"

#include "iparamb2.h"
#include "iparamm2.h"
#include "stdmat.h"

#include "plBMSampler.h"
#include "../MaxMain/plPlasmaRefMsgs.h"


///////////////////////////////////////////////////////////////////////////////
//// ParamBlock Dialog Proc ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class DTLBitmapDlgProc : public ParamMap2UserDlgProc
{
	/// Called to update the controls of the dialog
	virtual void	Update( TimeValue t, Interval &valid, IParamMap2 *map )
	{
		IParamBlock2	*pblock;
		BitmapInfo		bi;
		ICustButton		*bmSelectBtn;


		ParamMap2UserDlgProc::Update( t, valid, map );

		pblock = map->GetParamBlock();

		plDynamicTextLayer *layer = (plDynamicTextLayer *)map->GetParamBlock()->GetOwner();

		bmSelectBtn = GetICustButton( GetDlgItem( map->GetHWnd(), IDC_INITIMAGE ) );
		PBBitmap *pbbm = pblock->GetBitmap( plDynamicTextLayer::kBmpInitBitmap );
		if( pbbm )
			bmSelectBtn->SetText( (TCHAR *)pbbm->bi.Filename() );
		else
			bmSelectBtn->SetText( _T( "None" ) );
	    ReleaseICustButton( bmSelectBtn );
	}

	/// Clamp texture sizes to a power of 2
	void	IClampTexSizeSpinner( TimeValue t, IParamMap2 *map )
	{
	}

	/// Main message proc
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{

		switch (msg)
		{
			case WM_INITDIALOG:
			 break;

		/// Note: the following *could* be done in the accessor, except that you end up in an
		/// infinite loop updating the values. Not good. 
		case CC_SPINNER_CHANGE:	
			
			if( LOWORD( wParam ) == IDC_EXPORTWIDTH_SPINNER )
				IClampTexSizeSpinner( t, map, true );

			else if( LOWORD( wParam ) == IDC_EXPORTHEIGHT_SPINNER )
				IClampTexSizeSpinner( t, map, false );

			break;

		case WM_COMMAND:

			if( HIWORD( wParam ) == EN_CHANGE && LOWORD( wParam ) == IDC_EXPORTWIDTH )
				IClampTexSizeSpinner( t, map, true );

			else if( HIWORD( wParam ) == EN_CHANGE && LOWORD( wParam ) == IDC_EXPORTHEIGHT )
				IClampTexSizeSpinner( t, map, false );

			else if( HIWORD( wParam ) == BN_CLICKED && LOWORD( wParam ) == IDC_INITIMAGE_RELOAD )
			{
				// TEMP
				IParamBlock2 *pblock = map->GetParamBlock();
				PBBitmap *pbbm = pblock->GetBitmap( plDynamicTextLayer::kBmpInitBitmap );
				if( pbbm )
				{
					plDynamicTextLayer *layer = (plDynamicTextLayer *)map->GetParamBlock()->GetOwner();
					layer->RefreshBitmaps();
					layer->IChanged();
				}
				return TRUE;
			}

			else if( LOWORD( wParam ) == IDC_INITIMAGE )
			{
				plPlasmaMAXLayer *layer = (plPlasmaMAXLayer *)map->GetParamBlock()->GetOwner();
				if( layer == nil )
					return FALSE;
				BOOL selectedNewBitmap = layer->HandleBitmapSelection();		
				if( selectedNewBitmap )
				{
					IParamBlock2 *pblock = map->GetParamBlock();
					
					ICustButton *bmSelectBtn = GetICustButton( GetDlgItem( hWnd, IDC_INITIMAGE ) );
					PBBitmap *pbbm = layer->GetPBBitmap();
					
					bmSelectBtn->SetText( pbbm != nil ? (TCHAR *)pbbm->bi.Filename() : "");
					
					ReleaseICustButton( bmSelectBtn );							
				}

				return TRUE;
			}
			break;
		}

		return FALSE;
	}
	void DeleteThis() {};

protected:
	/// Clamp texture sizes to a power of 2
	void	IClampTexSizeSpinner( TimeValue t, IParamMap2 *map, bool clampWidth )
	{
		IParamBlock2 *pblock = map->GetParamBlock();
		ParamID		clampNew, clampOld;
		ParamID		otherNew, otherOld;

		if( clampWidth )
		{
			clampNew = plDynamicTextLayer::kBmpExportWidth; clampOld = plDynamicTextLayer::kBmpExportLastWidth;
			otherNew = plDynamicTextLayer::kBmpExportHeight; otherOld = plDynamicTextLayer::kBmpExportLastHeight;
		}
		else
		{
			clampNew = plDynamicTextLayer::kBmpExportHeight; clampOld = plDynamicTextLayer::kBmpExportLastHeight;
			otherNew = plDynamicTextLayer::kBmpExportWidth; otherOld = plDynamicTextLayer::kBmpExportLastWidth;
		}

		int		lastVal = pblock->GetInt( clampOld, t );
		int		tempVal, newVal = pblock->GetInt( clampNew, t );

		if( newVal < lastVal )
		{
			lastVal = newVal;
			for( tempVal = 1; tempVal <= newVal; tempVal <<= 1 );
			newVal = tempVal >> 1;
		}
		else
		{
			lastVal = newVal;
			for( tempVal = 1; tempVal < newVal; tempVal <<= 1 );
			newVal = tempVal;
		}

		pblock->SetValue( clampNew, t, newVal );
		pblock->SetValue( clampOld, t, newVal );
	}

	int		IFloorPow2( int value )
	{
		int		v;


		for( v = 1; v <= value; v <<= 1 );
		return v >> 1;
	}
};

static DTLBitmapDlgProc gDTLBitmapDlgProc;

///////////////////////////////////////////////////////////////////////////////
//// Bitmap Accessor //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class DTLPBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& val, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
		if( !owner )
			return;

		plDynamicTextLayer	*layer = (plDynamicTextLayer *)owner;

		IParamBlock2 *pb = owner->GetParamBlockByID( plDynamicTextLayer::kBlkBitmap );
		switch( id )
		{
			case plDynamicTextLayer::kBmpInitBitmap:
			case plDynamicTextLayer::kBmpUseInitImage:
				if (pb->GetMap())
					pb->GetMap()->Invalidate( id );

				layer->IChanged();

				break;
		}
	}
	void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid)
	{
	}
};
static DTLPBAccessor gDTLPBAccessor;

///////////////////////////////////////////////////////////////////////////////
//// ParamBlock Definition ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static ParamBlockDesc2 gBitmapParamBlk
(
	plDynamicTextLayer::kBlkBitmap, _T("bitmap"),  0, GetDynamicTextLayerDesc(),//NULL,
	P_AUTO_CONSTRUCT + P_AUTO_UI, plDynamicTextLayer::kRefBitmap,

	IDD_DYN_TEXT_LAYER, IDS_DYN_TEXT_LAYER_PROPS, 0, 0, &gDTLBitmapDlgProc,

	// Texture Color/Alpha
	plDynamicTextLayer::kBmpDiscardColor,	_T("discardColor"),	TYPE_BOOL,		0, 0,
//		p_ui,			TYPE_SINGLECHEKBOX, IDC_BLEND_NO_COLOR,
		end,
	plDynamicTextLayer::kBmpInvertColor,	_T("invertColor"),	TYPE_BOOL,		0, 0,
//		p_ui,			TYPE_SINGLECHEKBOX, IDC_BLEND_INV_COLOR,
		end,
	plDynamicTextLayer::kBmpDiscardAlpha,	_T("discardAlpha"),	TYPE_BOOL,		0, 0,
//		p_ui,			TYPE_SINGLECHEKBOX, IDC_DISCARD_ALPHA,
		end,
	plDynamicTextLayer::kBmpInvertAlpha,	_T("invertAlpha"),	TYPE_BOOL,		0, 0,
//		p_ui,			TYPE_SINGLECHEKBOX, IDC_BLEND_INV_ALPHA,
		end,
	
	// Texture size
	plDynamicTextLayer::kBmpExportWidth,	_T("exportWidth"),	TYPE_INT,	0, 0,
		p_ui,			TYPE_SPINNER, EDITTYPE_INT, IDC_EXPORTWIDTH, IDC_EXPORTWIDTH_SPINNER, SPIN_AUTOSCALE,
		p_range,		4, 2048,
		p_default,		512,
		end,
	plDynamicTextLayer::kBmpExportHeight,	_T("exportHeight"),	TYPE_INT,	0, 0,
		p_ui,			TYPE_SPINNER, EDITTYPE_INT, IDC_EXPORTHEIGHT, IDC_EXPORTHEIGHT_SPINNER, SPIN_AUTOSCALE,
		p_range,		4, 2048,
		p_default,		512,
		end,
	plDynamicTextLayer::kBmpExportLastWidth,	_T("lastExportWidth"),	TYPE_INT,		0, 0,
		end,
	plDynamicTextLayer::kBmpExportLastHeight,	_T("lastExportHeight"),	TYPE_INT,		0, 0,
		end,

	plDynamicTextLayer::kBmpIncludeAlphaChannel,	_T("includeAlphaChannel"),	TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_DYNTEXT_ALPHA,
		p_default, FALSE,
		end,
		
	// Initial image
	plDynamicTextLayer::kBmpUseInitImage,	_T("useInitImage"),	TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_USEINITIMAGE,
		p_enable_ctrls, 1, plDynamicTextLayer::kBmpInitBitmap, 
		p_accessor,		&gDTLPBAccessor,
		end,
	plDynamicTextLayer::kBmpInitBitmap,	_T("initBitmap"),		TYPE_BITMAP,	P_SHORT_LABELS, 0,
		p_accessor,		&gDTLPBAccessor,
		end,

/*		plGUIButtonComponent::kRefAnimate,	_T( "animate" ), TYPE_BOOL, 0, 0,
			p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_ANIMATE,
			p_default, FALSE,
			p_enable_ctrls, 1, plGUIButtonComponent::kRefAnimation,
			end,

	// Static text settings
	plDynamicTextLayer::kBmpMakeStatic,	_T("makeStatic"),	TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_DYNTEXT_MAKESTATIC,
		p_default, FALSE,
		p_enable_ctrls, 5, kBmpFontSize, kBmpLeftMargin, kBmpTopMargin, kBmpRightMargin, kBmpBottomMargin,
		end,
		
	plDynamicTextLayer::kBmpText,
	plDynamicTextLayer::kBmpFontFace,
	plDynamicTextLayer::kBmpFontSize,
	plDynamicTextLayer::kBmpLeftMargin,
	plDynamicTextLayer::kBmpTopMargin,
	plDynamicTextLayer::kBmpRightMargin,
	plDynamicTextLayer::kBmpBottomMargin,
*/
	end
);

