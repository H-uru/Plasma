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
#include "plStaticEnvLayer.h"



///////////////////////////////////////////////////////////////////////////////
//// Bitmap Accessor //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class SELBMTexPBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& val, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
		if (!owner)
			return;

		plStaticEnvLayer* layer = (plStaticEnvLayer*)owner;
		IParamBlock2 *pb = layer->GetParamBlockByID(plStaticEnvLayer::kBlkBitmap);

		switch (id)
		{
			case plStaticEnvLayer::kBmpFrontBitmap:
			case plStaticEnvLayer::kBmpBackBitmap:
			case plStaticEnvLayer::kBmpLeftBitmap:
			case plStaticEnvLayer::kBmpRightBitmap:
			case plStaticEnvLayer::kBmpTopBitmap:
			case plStaticEnvLayer::kBmpBottomBitmap:
				// Set up the enums so these would match...
				if (pb->GetMap())
					pb->GetMap()->Invalidate( id );
				// Update the bitmap saved by the layer
				//layer->SetBitmap( id, &val.bm->bi );
				break;

			case plStaticEnvLayer::kBmpBaseFilename:
				if( pb->GetMap() )
				{
					pb->GetMap()->Enable( plStaticEnvLayer::kBmpGenerateFaces, ( val.s == NULL || val.s[ 0 ] == 0 ) ? FALSE : TRUE );

					ICustButton		*bmSelectBtn = GetICustButton( GetDlgItem( pb->GetMap()->GetHWnd(), IDC_GENERATE_FACES ) );
					bmSelectBtn->SetText( _T( "Generate From Node" ) );
					ReleaseICustButton( bmSelectBtn );
				}
				break;
		}
	}
	void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid)
	{
	}
};
static SELBMTexPBAccessor bmtex_accessor;


///////////////////////////////////////////////////////////////////////////////
//// PickControlNode //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class PickControlNode : public PickObjectProc
{
	public:				
		plStaticEnvLayer	*fLayer;
		HWND				fHWnd;

		PickControlNode() { fLayer = NULL; }

		BOOL	Pick( INode *node )
		{
			if( node && fLayer )
				fLayer->RenderCubicMap( node );
			return TRUE;
		}

		void	EnterMode()
		{
			ICustButton		*iBut = GetICustButton( GetDlgItem( fHWnd, IDC_GENERATE_FACES ) );
			if( iBut )
			{
				iBut->SetCheck( TRUE );
				iBut->SetText( _T( "Generate From Node" ) );
			}
			ReleaseICustButton( iBut );
		}

		void	ExitMode()
		{
			ICustButton		*iBut = GetICustButton( GetDlgItem( fHWnd, IDC_GENERATE_FACES ) );
			if( iBut )
			{
				iBut->SetCheck( FALSE );
				iBut->SetText( _T( "Generate From Node" ) );
			}
			ReleaseICustButton( iBut );
		}

		BOOL	Filter( INode *node ) { return TRUE; }
};


///////////////////////////////////////////////////////////////////////////////
//// ParamBlock Dialog Proc ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class SELBitmapDlgProc : public ParamMap2UserDlgProc
{
	PickControlNode	fPickCallback;

public:
	IMtlParams	*fMtlParams;

	/// Called to update the controls of the dialog
	virtual void	Update( TimeValue t, Interval &valid, IParamMap2 *map )
	{
		ICustButton		*bmSelectBtn;
		IParamBlock2	*pblock;
		int				i;
		long			buttons[ 6 ] = { IDC_FRONT_NAME, IDC_BACK_NAME, IDC_LEFT_NAME, IDC_RIGHT_NAME, IDC_TOP_NAME, IDC_BOTTOM_NAME };
		BitmapInfo		bi;


		ParamMap2UserDlgProc::Update( t, valid, map );

		pblock = map->GetParamBlock();
		for( i = plStaticEnvLayer::kBmpFrontBitmap; i <= plStaticEnvLayer::kBmpBottomBitmap; i++ )
		{
			bmSelectBtn = GetICustButton( GetDlgItem( map->GetHWnd(), buttons[ i ] ) );
			PBBitmap *pbbm = pblock->GetBitmap( i, t );
			if( pbbm )
				bmSelectBtn->SetText( (TCHAR *)pbbm->bi.Filename() );
			else
				bmSelectBtn->SetText( _T( "None" ) );
		    ReleaseICustButton( bmSelectBtn );
		}

		plStaticEnvLayer *layer = (plStaticEnvLayer *)map->GetParamBlock()->GetOwner();
		bi.SetName( layer->GetBaseFilename( t ) );
		SetDlgItemText( map->GetHWnd(), IDC_BASE_FILENAME, bi.Filename() );
		map->Enable( plStaticEnvLayer::kBmpGenerateFaces, ( bi.Name() == NULL || bi.Name()[ 0 ] == 0 ) ? FALSE : TRUE );

		bmSelectBtn = GetICustButton( GetDlgItem( map->GetHWnd(), IDC_GENERATE_FACES ) );
		bmSelectBtn->SetText( _T( "Generate From Node" ) );
		ReleaseICustButton( bmSelectBtn );

		i = pblock->GetInt( plStaticEnvLayer::kBmpTextureSize, t );
		pblock->SetValue( plStaticEnvLayer::kBmpLastTextureSize, t, i );
	}

	/// Clamp texture sizes to a power of 2
	void	IClampTexSizeSpinner( TimeValue t, IParamMap2 *map )
	{
		IParamBlock2 *pblock = map->GetParamBlock();

		int		lastVal = pblock->GetInt( plStaticEnvLayer::kBmpLastTextureSize, t );
		int		tempVal, newVal = pblock->GetInt( plStaticEnvLayer::kBmpTextureSize, t );

		if( newVal < lastVal )
		{
			lastVal = newVal;
			for( tempVal = 1; tempVal < newVal; tempVal <<= 1 );
			newVal = tempVal >> 1;
		}
		else
		{
			lastVal = newVal;
			for( tempVal = 1; tempVal < newVal; tempVal <<= 1 );
			newVal = tempVal;
		}

		pblock->SetValue( plStaticEnvLayer::kBmpTextureSize, t, newVal );
		pblock->SetValue( plStaticEnvLayer::kBmpLastTextureSize, t, newVal );
	}

	/// Main message proc
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
      static ICustButton* bmSelectBtn;
	  long	buttons[ 6 ] = { IDC_FRONT_NAME, IDC_BACK_NAME, IDC_LEFT_NAME, IDC_RIGHT_NAME, IDC_TOP_NAME, IDC_BOTTOM_NAME };

		switch (msg)
		{
			case WM_INITDIALOG:
			 break;

		case CC_SPINNER_CHANGE:		 
			if( LOWORD( wParam ) == IDC_TEXSIZE_SPIN )
				IClampTexSizeSpinner( t, map );
			break;

		case WM_COMMAND:
			if( HIWORD( wParam ) == EN_CHANGE && LOWORD( wParam ) == IDC_TEXSIZE_EDIT )
			{
				IClampTexSizeSpinner( t, map );
			}
			else if( HIWORD( wParam ) == BN_CLICKED && LOWORD( wParam ) == IDC_LAYER_RELOAD )
			{
				plStaticEnvLayer *layer = (plStaticEnvLayer*)map->GetParamBlock()->GetOwner();
				layer->RefreshBitmaps();
				layer->IChanged();

				return TRUE;
			}
			else if( LOWORD( wParam ) == IDC_FRONT_NAME )
				return IDoLayerClicked( LOWORD( wParam ), plStaticEnvLayer::kFrontFace, map, t, hWnd );

			else if( LOWORD( wParam ) == IDC_BACK_NAME )
				return IDoLayerClicked( LOWORD( wParam ), plStaticEnvLayer::kBackFace, map, t, hWnd );

			else if( LOWORD( wParam ) == IDC_LEFT_NAME )
				return IDoLayerClicked( LOWORD( wParam ), plStaticEnvLayer::kLeftFace, map, t, hWnd );

			else if( LOWORD( wParam ) == IDC_RIGHT_NAME )
				return IDoLayerClicked( LOWORD( wParam ), plStaticEnvLayer::kRightFace, map, t, hWnd );

			else if( LOWORD( wParam ) == IDC_TOP_NAME )
				return IDoLayerClicked( LOWORD( wParam ), plStaticEnvLayer::kTopFace, map, t, hWnd );

			else if( LOWORD( wParam ) == IDC_BOTTOM_NAME )
				return IDoLayerClicked( LOWORD( wParam ), plStaticEnvLayer::kBottomFace, map, t, hWnd );

			else if( LOWORD( wParam ) == IDC_LAYER_LOAD_GEN )
				return IDoLoadGenerated( map, t, hWnd );

			else if( LOWORD( wParam ) == IDC_GENERATE_FACES )
			{
				plStaticEnvLayer *layer = (plStaticEnvLayer*)map->GetParamBlock()->GetOwner();

				fMtlParams->EndPickMode();
				fPickCallback.fHWnd = hWnd;
				fPickCallback.fLayer = layer;

				fMtlParams->SetPickMode( &fPickCallback );
				break;
			}

			else if( LOWORD( wParam ) == IDC_BASE_FILENAME )
				return IDoSelectBaseFilename( map, t, hWnd );

			break;
		}

		return FALSE;
	}
	void DeleteThis() {};

	BOOL	IDoSelectBaseFilename( IParamMap2 *map, TimeValue t, HWND hWnd )
	{
		BitmapInfo		bi;


		plStaticEnvLayer *layer = (plStaticEnvLayer*)map->GetParamBlock()->GetOwner();

		/// Select one file
		bi.SetName( layer->GetBaseFilename( t ) );
		if( !TheManager->SelectFileOutput( &bi, GetCOREInterface()->GetMAXHWnd(), _T( "Choose the base filename for the rendered faces" ) ) )
			return FALSE;

		/// Just store the name and set the button label as such, too
		SetDlgItemText( hWnd, IDC_BASE_FILENAME, bi.Filename() );
		layer->SetBaseFilename( bi.Name(), t );		
		return TRUE;
	}

	BOOL	IDoLoadGenerated( IParamMap2 *map, TimeValue t, HWND hWnd )
	{
		BitmapInfo		bi;
		int				i;
		TCHAR			filename[ MAX_PATH ];
		TCHAR			*modPoint, faces[ 6 ][ 4 ] = { "_FR", "_BK", "_LF", "_RT", "_UP", "_DN" };
		
		
		/// Select one file
		PBBitmap *pbbm = map->GetParamBlock()->GetBitmap( plStaticEnvLayer::kBmpFrontBitmap, t );
		if( pbbm != NULL )
			bi.SetName( pbbm->bi.Name() );
		if( !TheManager->SelectFileInput( &bi, GetCOREInterface()->GetMAXHWnd(), _T( "Select one of the generated face bitmaps" ) ) )
			return FALSE;

		/// Copy the name over and get our mod point
		strcpy( filename, bi.Filename() );
		modPoint = strstr( filename, "_UP" );
		if( modPoint == NULL )
			modPoint = strstr( filename, "_DN" );
		if( modPoint == NULL )
			modPoint = strstr( filename, "_LF" );
		if( modPoint == NULL )
			modPoint = strstr( filename, "_RT" );
		if( modPoint == NULL )
			modPoint = strstr( filename, "_FR" );
		if( modPoint == NULL )
			modPoint = strstr( filename, "_BK" );

		/// Load each face
		for( i = 0; i < 6; i++ )
		{
			memcpy( modPoint, faces[ i ], sizeof( TCHAR ) * 3 );
			if( !ILoadFace( i, filename, map, t, hWnd ) )
				return FALSE;
		}

		return TRUE;
	}

	BOOL	ILoadFace( int whichFace, const TCHAR *fileName, IParamMap2 *map, TimeValue t, HWND hWnd )
	{
		long	buttons[ 6 ] = { IDC_FRONT_NAME, IDC_BACK_NAME, IDC_LEFT_NAME, IDC_RIGHT_NAME, IDC_TOP_NAME, IDC_BOTTOM_NAME };

		IParamBlock2		*pblock = map->GetParamBlock();
		plStaticEnvLayer	*layer = (plStaticEnvLayer*)map->GetParamBlock()->GetOwner();
		ICustButton			*bmSelectBtn;
		BitmapInfo			bi;


		if( TheManager->GetImageInfo( &bi, fileName ) != BMMRES_SUCCESS )
			return FALSE;

		layer->SetBitmap( &bi, whichFace - plStaticEnvLayer::kFrontFace );

		PBBitmap *pbbm = layer->GetPBBitmap( whichFace - plStaticEnvLayer::kFrontFace );
		bmSelectBtn = GetICustButton( GetDlgItem( hWnd, buttons[ whichFace ] ) );
		bmSelectBtn->SetText((TCHAR*)pbbm->bi.Filename());
		ReleaseICustButton( bmSelectBtn );

		return TRUE;
	}

	BOOL	IDoLayerClicked( int whichBtn, int whichFace, IParamMap2 *map, TimeValue t, HWND hWnd )
	{
		plPlasmaMAXLayer *layer = (plPlasmaMAXLayer *)map->GetParamBlock()->GetOwner();
		if (layer == nil)
			return FALSE;

		BOOL selectedNewBitmap = layer->HandleBitmapSelection( whichFace - plStaticEnvLayer::kFrontFace );
		if(selectedNewBitmap)
		{
			ICustButton* bmSelectBtn;

			PBBitmap *pbbm = layer->GetPBBitmap( whichFace - plStaticEnvLayer::kFrontFace );
			bmSelectBtn = GetICustButton( GetDlgItem( hWnd, whichBtn ) );
			bmSelectBtn->SetText(pbbm != nil ? (TCHAR*)pbbm->bi.Filename() : nil);
			ReleaseICustButton(bmSelectBtn);

			return TRUE;
		}
		else
		{
		   return FALSE;
		}
	}
};
static SELBitmapDlgProc gSELBitmapDlgProc;

///////////////////////////////////////////////////////////////////////////////
//// ParamBlock Definition ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static ParamBlockDesc2 gBitmapParamBlk
(
	plStaticEnvLayer::kBlkBitmap, _T("bitmap"),  0, GetStaticEnvLayerDesc(),//NULL,
	P_AUTO_CONSTRUCT + P_AUTO_UI, plStaticEnvLayer::kRefBitmap,

	IDD_STATIC_ENVMAP_LAYER, IDS_STATIC_ENVMAP_LAYER_TEX, 0, 0, &gSELBitmapDlgProc,

	// Bitmaps
	plStaticEnvLayer::kBmpFrontBitmap,	_T("frontBitmap"),		TYPE_BITMAP,	P_SHORT_LABELS, 0,
		p_accessor,		&bmtex_accessor,
		end,
	plStaticEnvLayer::kBmpBackBitmap,		_T("backBitmap"),		TYPE_BITMAP,	P_SHORT_LABELS, 0,
		p_accessor,		&bmtex_accessor,
		end,
	plStaticEnvLayer::kBmpLeftBitmap,		_T("leftBitmap"),		TYPE_BITMAP,	P_SHORT_LABELS, 0,
		p_accessor,		&bmtex_accessor,
		end,
	plStaticEnvLayer::kBmpRightBitmap,	_T("rightBitmap"),		TYPE_BITMAP,	P_SHORT_LABELS, 0,
		p_accessor,		&bmtex_accessor,
		end,
	plStaticEnvLayer::kBmpTopBitmap,		_T("topBitmap"),		TYPE_BITMAP,	P_SHORT_LABELS, 0,
		p_accessor,		&bmtex_accessor,
		end,
	plStaticEnvLayer::kBmpBottomBitmap,	_T("bottomBitmap"),		TYPE_BITMAP,	P_SHORT_LABELS, 0,
		p_accessor,		&bmtex_accessor,
		end,

	// Texture Color/Alpha
	plStaticEnvLayer::kBmpDiscardColor,	_T("discardColor"),	TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_BLEND_NO_COLOR,
		end,
	plStaticEnvLayer::kBmpInvertColor,	_T("invertColor"),	TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_BLEND_INV_COLOR,
		end,
	plStaticEnvLayer::kBmpDiscardAlpha,	_T("discardAlpha"),	TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_DISCARD_ALPHA,
		end,
	plStaticEnvLayer::kBmpInvertAlpha,	_T("invertAlpha"),	TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_BLEND_INV_ALPHA,
		end,

	// Texture Quality
	plStaticEnvLayer::kBmpNonCompressed,	_T("nonCompressed"),TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_FORCE_NONCOMPRESSED,
		end,
	plStaticEnvLayer::kBmpScaling,		_T("scaling"),		TYPE_INT,		0, 0,
		p_ui,			TYPE_RADIO, 3, IDC_SCALE_ALL, IDC_SCALE_HALF, IDC_SCALE_NONE,
		end,

	// Max Only
	plStaticEnvLayer::kBmpMonoOutput,		_T("monoOutput"),	TYPE_INT,		0, 0,
		p_ui,			TYPE_RADIO, 2, IDC_HSMAX_LAYER_RGBOUT, IDC_HSMAX_LAYER_ALPHAOUT,
		end,
	plStaticEnvLayer::kBmpRGBOutput,		_T("rgbOutput"),	TYPE_INT,		0, 0,
		p_ui,			TYPE_RADIO, 2, IDC_HSMAX_LAYER_RGBOUT2, IDC_HSMAX_LAYER_ALPHAOUT2,
		end,

	// Detail
	plStaticEnvLayer::kBmpUseDetail,		_T("useDetail"),	TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_USE_DETAIL,
		p_default,		FALSE,
		p_enable_ctrls,	4, plStaticEnvLayer::kBmpDetailStartSize, plStaticEnvLayer::kBmpDetailStopSize, 
							plStaticEnvLayer::kBmpDetailStartOpac, plStaticEnvLayer::kBmpDetailStopOpac,
		end,

	plStaticEnvLayer::kBmpDetailStartSize,_T("dropOffStart"),	TYPE_INT,	0, 0,
		p_ui,			TYPE_SPINNER, EDITTYPE_INT, IDC_DETAIL_START_SIZE_EDIT, IDC_DETAIL_START_SIZE_SPIN, 0.4,
		p_range,		0, 100,
		p_default,		0,
		end,
	plStaticEnvLayer::kBmpDetailStopSize,	_T("dropOffStop"),	TYPE_INT,	0, 0,
		p_ui,			TYPE_SPINNER, EDITTYPE_INT, IDC_DETAIL_STOP_SIZE_EDIT, IDC_DETAIL_STOP_SIZE_SPIN, 0.4,
		p_range,		0, 100,
		p_default,		100,
		end,
	plStaticEnvLayer::kBmpDetailStartOpac,	_T("detailMax"),	TYPE_INT,	0, 0,
		p_ui,			TYPE_SPINNER, EDITTYPE_INT, IDC_DETAIL_START_OPAC_EDIT, IDC_DETAIL_START_OPAC_SPIN, 0.4,
		p_range,		0, 100,
		p_default,		8,
		end,
	plStaticEnvLayer::kBmpDetailStopOpac,	_T("detailMin"),	TYPE_INT,	0, 0,
		p_ui,			TYPE_SPINNER, EDITTYPE_INT, IDC_DETAIL_STOP_OPAC_EDIT, IDC_DETAIL_STOP_OPAC_SPIN, 0.4,
		p_range,		0, 100,
		p_default,		0,
		end,

	// Face generation
	plStaticEnvLayer::kBmpTextureSize,	_T("textureSize"),	TYPE_INT,		0, 0,
		p_ui,			TYPE_SPINNER, EDITTYPE_INT, IDC_TEXSIZE_EDIT, IDC_TEXSIZE_SPIN, SPIN_AUTOSCALE,
		p_range,		4, 512,
		p_default,		64,
		end,
	plStaticEnvLayer::kBmpBaseFilename, _T("baseFilename"), TYPE_FILENAME, 0, 0,
		p_default,		_T( "" ),
		p_accessor,		&bmtex_accessor,
		end,
	plStaticEnvLayer::kBmpGenerateFaces, _T("genFaces"), TYPE_INODE, 0, 0,
		p_ui,			TYPE_PICKNODEBUTTON, IDC_GENERATE_FACES,
		p_prompt,		IDS_SELECT_NODE,
		end,
	plStaticEnvLayer::kBmpUseMAXAtmosphere,		_T("useMAXAtmos"),	TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_USEMAXFOG,
		p_default,		FALSE,
		p_enable_ctrls,	1, plStaticEnvLayer::kBmpFarDistance,
		end,
	plStaticEnvLayer::kBmpFarDistance,	_T("farDistance"),	TYPE_FLOAT,		0, 0,
		p_ui,			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_FARDIST_EDIT, IDC_FARDIST_SPIN, SPIN_AUTOSCALE,
		p_range,		0.f, 9999999.f,
		p_default,		500.f,
		end,
	plStaticEnvLayer::kBmpLastTextureSize,	_T("lastTextureSize"),	TYPE_INT,		0, 0,
		end,
	
	plStaticEnvLayer::kBmpRefract,	_T("refract"),TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_REFRACT,
		end,

	end
);

