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
#include "plLayerTex.h"
#include "plLayerTexBitmapPB.h"
#include "plDetailCurveCtrl.h"

#if 1
class BMTexPBAccessor;
extern BMTexPBAccessor bmtex_accessor;

class BitmapDlgProc; 
extern BitmapDlgProc gBitmapDlgProc;

static ParamBlockDesc2 gBitmapParamBlk
(
	plLayerTex::kBlkBitmap, _T("bitmap"),  0, GetLayerTexDesc(),//NULL,
	P_AUTO_CONSTRUCT + P_AUTO_UI, plLayerTex::kRefBitmap,

	IDD_LAYER_TEX, IDS_LAYER_TEX, 0, 0, &gBitmapDlgProc,

	// Bitmap
	kBmpUseBitmap,		_T("useBitmap"),	TYPE_BOOL,		0, 0,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_USE_BITMAP,
		end,
	kBmpBitmap,			_T("bitmap"),		TYPE_BITMAP,	P_SHORT_LABELS, 0,
		p_accessor,		&bmtex_accessor,
		end,

	// Crop/Place
	kBmpApply,			_T("apply"),	TYPE_BOOL,		0, 0,
		p_default,		FALSE,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_BM_CLIP,
		end,
	kBmpCropPlace,		_T("cropPlace"), TYPE_INT,		0, 0,
		p_default,		0,
		p_range,		0,	1,
		p_ui,			TYPE_RADIO, 2,  IDC_BM_CROP,IDC_BM_PLACE,
		end,
	kBmpClipU,			_T("clipU"),	TYPE_FLOAT,		P_ANIMATABLE, IDS_BITMAP_CLIPU,
		p_default,		0.0,
		p_range,		0.0, 1.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_CLIP_X, IDC_CLIP_XSPIN, 0.001f,
		p_accessor,		&bmtex_accessor,
		end,
	kBmpClipV,			_T("clipV"),	TYPE_FLOAT,		P_ANIMATABLE, IDS_BITMAP_CLIPV,
		p_default,		0.0,
		p_range,		0.0, 1.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_CLIP_Y, IDC_CLIP_YSPIN, 0.001f,
		p_accessor,		&bmtex_accessor,
		end,
	kBmpClipW,			_T("clipW"),	TYPE_FLOAT,		P_ANIMATABLE, IDS_BITMAP_CLIPW,
		p_default,		1.0,
		p_range,		0.0, 1.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_CLIP_W, IDC_CLIP_WSPIN, 0.001f,
		p_accessor,		&bmtex_accessor,
		end,
	kBmpClipH,			_T("clipH"),	TYPE_FLOAT,		P_ANIMATABLE, IDS_BITMAP_CLIPH,
		p_default,		1.0,
		p_range,		0.0, 1.0,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_CLIP_H, IDC_CLIP_HSPIN, 0.001f,
		p_accessor,		&bmtex_accessor,
		end,

	// Texture Color/Alpha
	kBmpDiscardColor,	_T("discardColor"),	TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_BLEND_NO_COLOR,
		end,
	kBmpInvertColor,	_T("invertColor"),	TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_BLEND_INV_COLOR,
		end,
	kBmpDiscardAlpha,	_T("discardAlpha"),	TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_DISCARD_ALPHA,
		end,
	kBmpInvertAlpha,	_T("invertAlpha"),	TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_BLEND_INV_ALPHA,
		end,

	// Texture Quality
	kBmpNonCompressed,	_T("nonCompressed"),TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_FORCE_NONCOMPRESSED,
		end,
	kBmpScaling,		_T("scaling"),		TYPE_INT,		0, 0,
		p_ui,			TYPE_RADIO, 3, IDC_SCALE_ALL, IDC_SCALE_HALF, IDC_SCALE_NONE,
		end,

	// Max Only
	kBmpMonoOutput,		_T("monoOutput"),	TYPE_INT,		0, 0,
		p_ui,			TYPE_RADIO, 2, IDC_HSMAX_LAYER_RGBOUT, IDC_HSMAX_LAYER_ALPHAOUT,
		end,
	kBmpRGBOutput,		_T("rgbOutput"),	TYPE_INT,		0, 0,
		p_ui,			TYPE_RADIO, 2, IDC_HSMAX_LAYER_RGBOUT2, IDC_HSMAX_LAYER_ALPHAOUT2,
		end,

	// Mipmap
	kBmpNoFilter,		_T("noFilter"),	TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_NO_FILTERING,
		end,
	kBmpMipBlur,		_T("mipBlur"),		TYPE_FLOAT,		0, 0,
		p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_MIPBLUR_EDIT, IDC_MIPBLUR_SPIN, 0.4,
		p_range,		0.01f, 100.0f,
		p_default,		1.0,
		end,
	kBmpMipBias,		_T("mipBias"),		TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_USE_MIPBIAS,
		p_enable_ctrls,	1, kBmpMipBiasAmt,
		end,
	kBmpMipBiasAmt,		_T("mipBiasAmt"),	TYPE_FLOAT,		0, 0,
		p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_MIPBIAS_EDIT, IDC_MIPBIAS_SPIN, 0.7,
		p_range,		-100.0, 100.0,
		p_default,		1.0,
		end,

	// Detail
	kBmpUseDetail,		_T("useDetail"),	TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_USE_DETAIL,
		p_default,		FALSE,
		p_enable_ctrls,	4, kBmpDetailStartSize, kBmpDetailStopSize, kBmpDetailStartOpac, kBmpDetailStopOpac,
		p_accessor,		&bmtex_accessor,
		end,

	kBmpDetailStartSize,_T("dropOffStart"),	TYPE_INT,	0, 0,
		p_ui,			TYPE_SPINNER, EDITTYPE_INT, IDC_DETAIL_START_SIZE_EDIT, IDC_DETAIL_START_SIZE_SPIN, 0.4,
		p_range,		0, 100,
		p_default,		0,
		p_accessor,		&bmtex_accessor,
		end,
	kBmpDetailStopSize,	_T("dropOffStop"),	TYPE_INT,	0, 0,
		p_ui,			TYPE_SPINNER, EDITTYPE_INT, IDC_DETAIL_STOP_SIZE_EDIT, IDC_DETAIL_STOP_SIZE_SPIN, 0.4,
		p_range,		0, 100,
		p_default,		100,
		p_accessor,		&bmtex_accessor,
		end,
	kBmpDetailStartOpac,	_T("detailMax"),	TYPE_INT,	0, 0,
		p_ui,			TYPE_SPINNER, EDITTYPE_INT, IDC_DETAIL_START_OPAC_EDIT, IDC_DETAIL_START_OPAC_SPIN, 0.4,
		p_range,		0, 100,
		p_default,		100,
		p_accessor,		&bmtex_accessor,
		end,
	kBmpDetailStopOpac,	_T("detailMin"),	TYPE_INT,	0, 0,
		p_ui,			TYPE_SPINNER, EDITTYPE_INT, IDC_DETAIL_STOP_OPAC_EDIT, IDC_DETAIL_STOP_OPAC_SPIN, 0.4,
		p_range,		0, 100,
		p_default,		0,
		p_accessor,		&bmtex_accessor,
		end,

	kBmpExportWidth,	_T("exportWidth"),	TYPE_INT,	0, 0,
		p_ui,			TYPE_SPINNER, EDITTYPE_INT, IDC_EXPORTWIDTH, IDC_EXPORTWIDTH_SPINNER, SPIN_AUTOSCALE,
		p_range,		4, 2048,
		p_default,		512,
		end,
	kBmpExportHeight,	_T("exportHeight"),	TYPE_INT,	0, 0,
		p_ui,			TYPE_SPINNER, EDITTYPE_INT, IDC_EXPORTHEIGHT, IDC_EXPORTHEIGHT_SPINNER, SPIN_AUTOSCALE,
		p_range,		4, 2048,
		p_default,		512,
		end,
	kBmpExportLastWidth,	_T("lastExportWidth"),	TYPE_INT,		0, 0,
		end,
	kBmpExportLastHeight,	_T("lastExportHeight"),	TYPE_INT,		0, 0,
		end,

	// Keep a sysmem copy at runtime (for image examination/manipulation).
	kBmpNoDiscard,		_T("noDiscard"),	TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_NO_DISCARD,
		p_default,		FALSE,
		end,

	end
);
ParamBlockDesc2 *GetBitmapBlk() { return &gBitmapParamBlk; }

class BMCropper;

class BMTexPBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& val, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
      plLayerTex* layer = (plLayerTex*)owner;

      if(layer == NULL) return;
      
      IParamBlock2 *pb = layer->GetParamBlockByID(plLayerTex::kBlkBitmap);

		switch (id)
		{
			case kBmpBitmap:
				if (pb->GetMap())
					pb->GetMap()->Invalidate(kBmpBitmap);

				// Update the bitmap saved by the layer
				//layer->SetBitmap(&val.bm->bi, tabIndex);
				break;

/*
			case kBmpFilename:
				bmt->SetMapName(val.s);
				break;

			case kBmpFiltering:
				bmt->filterType = val.i;
				if (bmt->thebm)	
					bmt->thebm->SetFilter(bmFilterType(val.i));
				break;	
*/
			case kBmpClipU:
			{
				float u = val.f;
				float w = pb->GetFloat(kBmpClipW, t);
				if (u + w > 1.0f)
				{
					pb->SetValue(kBmpClipW, t, 1.0f-u);
					if (pb->GetMap())
						pb->GetMap()->Invalidate(kBmpClipW);
				}
				break;
			}
			case kBmpClipW:
			{
				float w = val.f;
				float u = pb->GetFloat(kBmpClipU, t);
				if (u + w > 1.0f)
				{
					pb->SetValue(kBmpClipU, t, 1.0f-w);
					if (pb->GetMap())
						pb->GetMap()->Invalidate(kBmpClipU);
				}
				break;
			}
			case kBmpClipV:
			{
				float v = val.f;
				float h = pb->GetFloat(kBmpClipH, t);
				if (v + h > 1.0f)
				{
					pb->SetValue(kBmpClipH, t, 1.0f-v);
					if (pb->GetMap())
						pb->GetMap()->Invalidate(kBmpClipH);
				}
				break;
			}
			case kBmpClipH:
			{
				float h = val.f;
				float v = pb->GetFloat(kBmpClipV, t);
				if (v + h > 1.0f)
				{
					pb->SetValue(kBmpClipV, t, 1.0f-h);
					if (pb->GetMap())
						pb->GetMap()->Invalidate(kBmpClipV);
				}
				break;
			}

			case kBmpDetailStartSize:
			case kBmpDetailStopSize:
			case kBmpDetailStartOpac:
			case kBmpDetailStopOpac:
				if( pb != NULL )
				{
					if( IIsProcSettingDetailValues( pb ) )
						break;	// Ignore, since we're the ones setting 'em

					HWND dlg = pb->GetMap()->GetHWnd();
					plDetailCurveCtrl *ctrl = GET_DETAIL_CURVE_CTRL( dlg, IDC_DETAIL_CURVE_CTRL );
					if( ctrl != NULL )
					{
						if( id == kBmpDetailStartSize || id == kBmpDetailStartOpac )
							ctrl->SetStartPoint( (float)pb->GetInt( kBmpDetailStartSize, t ) / 100.f,
												 (float)pb->GetInt( kBmpDetailStartOpac, t ) / 100.f );
						else
							ctrl->SetEndPoint(   (float)pb->GetInt( kBmpDetailStopSize, t ) / 100.f,
												 (float)pb->GetInt( kBmpDetailStopOpac, t ) / 100.f );
					}

					// Make sure start is less than end
					if( id == kBmpDetailStartSize )
					{
						int	end = pb->GetInt( kBmpDetailStopSize, t );
						if( val.i > end )
							pb->SetValue( kBmpDetailStopSize, t, val.i );
					}
					else if( id == kBmpDetailStopSize )
					{
						int start = pb->GetInt( kBmpDetailStartSize, t );
						if( val.i < start )
							pb->SetValue( kBmpDetailStartSize, t, val.i );
					}

				}
				break;

			case kBmpUseDetail:
				if( pb != NULL )
				{
					HWND dlg = pb->GetMap()->GetHWnd();
					EnableWindow( GetDlgItem( dlg, IDC_DETAIL_CURVE_CTRL ), (BOOL)val.i );
				}
				break;
		}
	}
	void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid)
	{
	}

	// Gotta love hacks....
	bool	IIsProcSettingDetailValues( IParamBlock2 *pb );

};
static BMTexPBAccessor bmtex_accessor;


//=========================================================================================
// BMCropper
//=========================================================================================
class BMCropper : public CropCallback
{
	IParamBlock2 *fPBlock;

public:
	BMCropper(IParamBlock2* pblock) : fPBlock(pblock) {}

	float GetInitU() { return fPBlock->GetFloat(kBmpClipU); }
	float GetInitV() { return fPBlock->GetFloat(kBmpClipV); }
	float GetInitW() { return fPBlock->GetFloat(kBmpClipW); }
	float GetInitH() { return fPBlock->GetFloat(kBmpClipH); }
	BOOL GetInitMode() { return fPBlock->GetInt(kBmpCropPlace); }
	void SetValues(float u, float v, float w, float h, BOOL md);
	void OnClose();
};

void BMCropper::SetValues(float u, float v, float w, float h, BOOL md) 
{
	TimeValue t = GetCOREInterface()->GetTime();

	if (u != fPBlock->GetFloat(kBmpClipU, t))
	{
		fPBlock->SetValue(kBmpClipU, t, u);
		fPBlock->GetMap()->Invalidate(kBmpClipU);
	}

	if (v != fPBlock->GetFloat(kBmpClipV, t))
	{
		fPBlock->SetValue(kBmpClipV, t, v);
		fPBlock->GetMap()->Invalidate(kBmpClipV);
	}

	if (w != fPBlock->GetFloat(kBmpClipW, t))
	{
		fPBlock->SetValue(kBmpClipW, t, w);
		fPBlock->GetMap()->Invalidate(kBmpClipW);
	}

	if (h != fPBlock->GetFloat(kBmpClipH, t))
	{
		fPBlock->SetValue(kBmpClipH, t, h);
		fPBlock->GetMap()->Invalidate(kBmpClipH);
	}

	if (md != fPBlock->GetInt(kBmpCropPlace))
	{
		fPBlock->SetValue(kBmpCropPlace, t, md);
		fPBlock->GetMap()->Invalidate(kBmpCropPlace);
	}
}

void BMCropper::OnClose()
{
	delete this;
}

class BitmapDlgProc : public ParamMap2UserDlgProc
{
	friend class BMTexPBAccessor;
	
	
	PBBitmap	*fLastBMap;
	bool		fSettingDetailValues;
	
	
	/// Called to update the controls of the dialog
	/// Note: we're bad that we use a static here, but 
	virtual void	Update( TimeValue t, Interval &valid, IParamMap2 *map )
	{
		ICustButton		*bmSelectBtn;
		IParamBlock2	*pblock;
		int				width, height;
		
		
		ParamMap2UserDlgProc::Update( t, valid, map );
		
		if( fSettingDetailValues )
		{
			// We're getting an update just because we changed detail values, so we
			// know we don't have to do anything ourselves
			return;
		}
		
		pblock = map->GetParamBlock();
		
		// Update texture map button
		bmSelectBtn = GetICustButton( GetDlgItem( map->GetHWnd(), IDC_LAYER_NAME ) );
		PBBitmap *pbbm = pblock->GetBitmap( kBmpBitmap, t );
		if( pbbm )
		{
			if( pbbm != fLastBMap )
			{
				bmSelectBtn->SetText( (TCHAR *)pbbm->bi.Filename() );
				
				// Init values for clamping spinners to powers of 2
				width = IFloorPow2( pbbm->bi.Width() );
				map->SetRange( kBmpExportWidth, 4.f, (float)width );
				
				height = IFloorPow2( pbbm->bi.Height() );
				map->SetRange( kBmpExportHeight, 4.f, (float)height );
				
				IClampTexSizeSpinner( t, map, true );
				ISetDetailCurveNumLevels( map, t );
			}
		}
		else if( pbbm != fLastBMap )
			bmSelectBtn->SetText( _T( "None" ) );
		
		fLastBMap = pbbm;
		
		ReleaseICustButton( bmSelectBtn );
		
		// Update detail curve control
		HWND dlg = map->GetHWnd();
		
		plDetailCurveCtrl *ctrl = GET_DETAIL_CURVE_CTRL( dlg, IDC_DETAIL_CURVE_CTRL );
		if( ctrl == NULL )
		{
			// The control hasn't been created, so create it already!
			HWND				basis;
			RECT				r;
			
			// Create the detail map control
			basis = GetDlgItem( dlg, IDC_DETAIL_SAMPLE );
			GetClientRect( basis, &r );
			MapWindowPoints( basis, dlg, (POINT *)&r, 2 );
			
			ctrl = TRACKED_NEW plDetailCurveCtrl( dlg, IDC_DETAIL_CURVE_CTRL, &r );
		}
		
		EnableWindow( GetDlgItem( dlg, IDC_DETAIL_CURVE_CTRL ), (BOOL)pblock->GetInt( kBmpUseDetail, t ) );
		
		if( ctrl != NULL )
		{
			ctrl->SetStartPoint( (float)pblock->GetInt( kBmpDetailStartSize, t ) / 100.f,
				(float)pblock->GetInt( kBmpDetailStartOpac, t ) / 100.f );
			ctrl->SetEndPoint(   (float)pblock->GetInt( kBmpDetailStopSize, t ) / 100.f,
				(float)pblock->GetInt( kBmpDetailStopOpac, t ) / 100.f );
		}
		
	}
	
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		static ICustButton* bmSelectBtn;
		
		switch (msg)
		{
		case WM_INITDIALOG:
			fLastBMap = NULL;
			fSettingDetailValues = false;
			break;
			
			/// Note: the following *could* be done in the accessor, except that you end up in an
			/// infinite loop updating the values. Not good. 
		case CC_SPINNER_CHANGE:	
			
			if( LOWORD( wParam ) == IDC_EXPORTWIDTH_SPINNER )
				IClampTexSizeSpinner( t, map, true );
			
			else if( LOWORD( wParam ) == IDC_EXPORTHEIGHT_SPINNER )
				IClampTexSizeSpinner( t, map, false );
			
			break;
			
			// Message from the detail curve that a point got dragged
		case PL_DC_POINT_DRAGGED:
			{
				plDetailCurveCtrl	*ctrl = (plDetailCurveCtrl *)lParam;
				IParamBlock2		*pblock = map->GetParamBlock();
				float				x, y;
				
				
				fSettingDetailValues = true;
				
				if( wParam == PL_DC_START_POINT )
				{
					ctrl->GetStartPoint( x, y );
					pblock->SetValue( kBmpDetailStartSize, t, (int)( x * 100.f ) );
					pblock->SetValue( kBmpDetailStartOpac, t, (int)( y * 100.f ) );
				}
				else
				{
					ctrl->GetEndPoint( x, y );
					pblock->SetValue( kBmpDetailStopSize, t, (int)( x * 100.f ) );
					pblock->SetValue( kBmpDetailStopOpac, t, (int)( y * 100.f ) );
				}
				
				map->UpdateUI( t );
				fSettingDetailValues = false;
			}
			return 0;
			
		case WM_COMMAND:
			if( HIWORD( wParam ) == EN_CHANGE && LOWORD( wParam ) == IDC_EXPORTWIDTH )
				IClampTexSizeSpinner( t, map, true );
			
			else if( HIWORD( wParam ) == EN_CHANGE && LOWORD( wParam ) == IDC_EXPORTHEIGHT )
				IClampTexSizeSpinner( t, map, false );
			
			else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_BM_CROP_IMAGE)
			{
				IParamBlock2 *pblock = map->GetParamBlock();
				PBBitmap *pbbm = pblock->GetBitmap(kBmpBitmap, t);
				if (pbbm)
				{
					if (!pbbm->bm)
						pbbm->bm = TheManager->Load(&pbbm->bi);
					
					BMCropper *cropper = TRACKED_NEW BMCropper(pblock);
					
					pbbm->bm->Display("Specify Cropping/Placement", BMM_CN, FALSE, TRUE, cropper);
				}
				//				bm->DeleteThis();
				return TRUE;
			}
			else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_LAYER_RELOAD)
			{
				// TEMP
				IParamBlock2 *pblock = map->GetParamBlock();
				PBBitmap *pbbm = pblock->GetBitmap(kBmpBitmap, t);
				if (pbbm)
				{
					plLayerTex *layer = (plLayerTex*)map->GetParamBlock()->GetOwner();

					layer->RefreshBitmaps();
					
					layer->fMtlParams->MtlChanged();
					layer->IChanged();
				}
				return TRUE;
			}
			else if (LOWORD(wParam) == IDC_LAYER_NAME)
			{
				plPlasmaMAXLayer *layer = (plPlasmaMAXLayer *)map->GetParamBlock()->GetOwner();
				if (layer == nil)
					return FALSE;
				BOOL selectedNewBitmap = layer->HandleBitmapSelection();
				
				if(selectedNewBitmap)
				{
					IParamBlock2 *pblock = map->GetParamBlock();
					//plLayerTex *layer = (plLayerTex*)map->GetParamBlock()->GetOwner();
					
					//layer->SetBitmap(&bi);
					//layer->IChanged();
					//BitmapInfo *bi = &layer->GetPBBitmap()->bi;
					
					bmSelectBtn = GetICustButton(GetDlgItem(hWnd,IDC_LAYER_NAME));
					PBBitmap *pbbm = layer->GetPBBitmap();
					bmSelectBtn->SetText(pbbm != nil ? (TCHAR*)pbbm->bi.Filename() : "");
					ReleaseICustButton(bmSelectBtn);
					
					if (pbbm != nil)
					{
						// Init values for clamping spinners to powers of 2
						int width = IFloorPow2( pbbm->bi.Width() );
						map->SetRange( kBmpExportWidth, 4.f, (float)width );
						
						int height = IFloorPow2( pbbm->bi.Height() );
						map->SetRange( kBmpExportHeight, 4.f, (float)height );
						
						if( width > 512 )
						{
							height = (int)( 512.f * (float)( (float)height / (float)width ) );
							width = 512;
						}
						else if( height > 512 )
						{
							width = (int)( 512.f * (float)( (float)width / (float)height ) );
							height = 512;
						}
						pblock->SetValue( kBmpExportWidth, t, width );
						pblock->SetValue( kBmpExportLastWidth, t, width );
						pblock->SetValue( kBmpExportHeight, t, height );
						pblock->SetValue( kBmpExportLastHeight, t, height );
						
						IClampTexSizeSpinner( t, map, true );
					}	
					return TRUE;
				}
				else
				{
					return FALSE;
				}
			}
			break;
		}
		
		return FALSE;
	}
	void DeleteThis() {};
	
	void	ISetDetailCurveNumLevels( IParamMap2 *map, TimeValue t )
	{
		/// Set the level count on the detail control
		plDetailCurveCtrl *ctrl = GET_DETAIL_CURVE_CTRL( map->GetHWnd(), IDC_DETAIL_CURVE_CTRL );
		if( ctrl != NULL )
		{
			IParamBlock2 *pblock = map->GetParamBlock();
			int w = pblock->GetInt( kBmpExportWidth, t );
			int h = pblock->GetInt( kBmpExportHeight, t );
			int numLevels = 0;
			while( w > 1 && h > 1 )
			{
				w >>= 1;
				h >>= 1;
				numLevels++;
			}
			ctrl->SetNumLevels( numLevels );
		}
	}
	
	/// Clamp texture sizes to a power of 2
	void	IClampTexSizeSpinner( TimeValue t, IParamMap2 *map, bool clampWidth )
	{
		IParamBlock2 *pblock = map->GetParamBlock();
		ParamID		clampNew, clampOld;
		ParamID		otherNew, otherOld;
		
		if( clampWidth )
		{
			clampNew = kBmpExportWidth; clampOld = kBmpExportLastWidth;
			otherNew = kBmpExportHeight; otherOld = kBmpExportLastHeight;
		}
		else
		{
			clampNew = kBmpExportHeight; clampOld = kBmpExportLastHeight;
			otherNew = kBmpExportWidth; otherOld = kBmpExportLastWidth;
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
		
		// And clamp aspect ratio
        PBBitmap		*pbbm = pblock->GetBitmap( kBmpBitmap, t );
		
		if( pbbm != NULL )
		{
			int	realWidth = pbbm->bi.Width();
			int realHeight = pbbm->bi.Height();
			
			float aspect;
			if( clampWidth )			
				aspect = (float)realHeight / (float)realWidth;
			else
				aspect = (float)realWidth / (float)realHeight;
			
			int	value = newVal;
			value *= aspect;
			
			if( value < 4 )
			{
				// Can't be below 4!
				value = 4;
				pblock->SetValue( otherNew, t, value );
				pblock->SetValue( otherOld, t, value );
				value = value / aspect;
				pblock->SetValue( clampNew, t, value );
				pblock->SetValue( clampOld, t, value );
			}
			else
			{
				pblock->SetValue( otherNew, t, value );
				pblock->SetValue( otherOld, t, value );
			}
		}
		
		ISetDetailCurveNumLevels( map, t );
	}
	
	int		IFloorPow2( int value )
	{
		int		v;
		
		
		for( v = 1; v <= value; v <<= 1 );
		return v >> 1;
	}
	
};

static BitmapDlgProc gBitmapDlgProc;


// Gotta love hacks....
bool	BMTexPBAccessor::IIsProcSettingDetailValues( IParamBlock2 *pb )
{
	BitmapDlgProc *proc = (BitmapDlgProc *)pb->GetMap()->GetUserDlgProc();
	if( proc != NULL )
		return proc->fSettingDetailValues;

	return false;
}

#endif