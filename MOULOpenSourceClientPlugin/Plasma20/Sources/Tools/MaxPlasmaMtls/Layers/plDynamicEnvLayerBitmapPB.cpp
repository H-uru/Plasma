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
//	plDynamicEnvLayer ParamBlock Functions									 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	8.22.2001 mcn - Created.												 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//// PickAnchorNode ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class PickAnchorNode : public PickObjectProc
{
	public:				
		plDynamicEnvLayer	*fLayer;
		HWND				fHWnd;
		int	bleah;

		PickAnchorNode() { fLayer = NULL; }

		BOOL	Pick( INode *node )
		{
			const char *dbgNodeName = node->GetName();

			if( fLayer )
				fLayer->GetParamBlockByID( plDynamicEnvLayer::kBlkBitmap )->SetValue( plDynamicEnvLayer::kBmpAnchorNode, TimeValue( 0 ), node );

			return TRUE;
		}

		void	EnterMode()		{ ISetButton( TRUE ); }
		void	ExitMode()		{ ISetButton( FALSE ); }

		BOOL	Filter( INode *node )
		{
			Object	*obj = node->EvalWorldState( 0 ).obj;
			if( obj != NULL )
			{
				if( obj->CanConvertToType( triObjectClassID ) || 
					obj->ClassID() == Class_ID( DUMMY_CLASS_ID, 0 ) )
					return TRUE;
			}
			return FALSE; 
		}

	protected:

		void	ISetButton( BOOL checkIt )
		{
			ICustButton		*iBut = GetICustButton( GetDlgItem( fHWnd, IDC_ANCHOR_NODE ) );
			if( iBut )
			{
				iBut->SetCheck( checkIt );
				if( fLayer )
				{
					if( fLayer->GetParamBlockByID( plDynamicEnvLayer::kBlkBitmap )->GetINode( plDynamicEnvLayer::kBmpAnchorNode ) == NULL )
						iBut->SetText( _T( "<self>" ) );
				}
			}
			ReleaseICustButton( iBut );
		}
};


///////////////////////////////////////////////////////////////////////////////
//// ParamBlock Dialog Proc ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class DELBitmapDlgProc : public ParamMap2UserDlgProc
{
	PickAnchorNode	fPickAnchorCallback;

	/// Called to update the controls of the dialog
	virtual void	Update( TimeValue t, Interval &valid, IParamMap2 *map )
	{
		IParamBlock2	*pblock;
		int				i;


		ParamMap2UserDlgProc::Update( t, valid, map );

		pblock = map->GetParamBlock();

		i = pblock->GetInt( plDynamicEnvLayer::kBmpTextureSize, t );
		pblock->SetValue( plDynamicEnvLayer::kBmpLastTextureSize, t, i );

		if( pblock->GetINode( plDynamicEnvLayer::kBmpAnchorNode ) == NULL )
		{
			ICustButton		*bmSelectBtn = GetICustButton( GetDlgItem( pblock->GetMap()->GetHWnd(), IDC_ANCHOR_NODE ) );
			bmSelectBtn->SetText( _T( "<self>" ) );
			ReleaseICustButton( bmSelectBtn );
		}
	}

	/// Clamp texture sizes to a power of 2
	void	IClampTexSizeSpinner( TimeValue t, IParamMap2 *map )
	{
		IParamBlock2 *pblock = map->GetParamBlock();

		int		lastVal = pblock->GetInt( plDynamicEnvLayer::kBmpLastTextureSize, t );
		int		tempVal, newVal = pblock->GetInt( plDynamicEnvLayer::kBmpTextureSize, t );

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

		pblock->SetValue( plDynamicEnvLayer::kBmpTextureSize, t, newVal );
		pblock->SetValue( plDynamicEnvLayer::kBmpLastTextureSize, t, newVal );
	}

	/// Main message proc
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
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
					IClampTexSizeSpinner( t, map );

				else if( LOWORD( wParam ) == IDC_ANCHOR_NODE )
				{
					plDynamicEnvLayer *layer = (plDynamicEnvLayer *)map->GetParamBlock()->GetOwner();

					layer->fIMtlParams->EndPickMode();
					fPickAnchorCallback.fHWnd = hWnd;
					fPickAnchorCallback.fLayer = layer;

					layer->fIMtlParams->SetPickMode( &fPickAnchorCallback );
					break;
				}
				break;
		}

		return FALSE;
	}

	void DeleteThis() {};
};

static DELBitmapDlgProc gDELBitmapDlgProc;

class BleahPBAccessor : public PBAccessor
{
public:
	void Set(PB2Value& val, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
		plDynamicEnvLayer* layer = (plDynamicEnvLayer *)owner;
		IParamBlock2 *pb = layer->GetParamBlockByID( plDynamicEnvLayer::kBlkBitmap );

		switch (id)
		{
			case plDynamicEnvLayer::kBmpAnchorNode:
				INode	*newNode = (INode *)val.r;
				if( newNode == NULL )
				{
					// Instead of displaying "none", display "<self>", since that's what nil means
					// for us
					ICustButton		*bmSelectBtn = GetICustButton( GetDlgItem( pb->GetMap()->GetHWnd(), IDC_ANCHOR_NODE ) );
					bmSelectBtn->SetText( _T( "<self>" ) );
					ReleaseICustButton( bmSelectBtn );
				}
				break;
		}
	}
	void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid)
	{
	}
};

static BleahPBAccessor gBleahPBAccessor;

///////////////////////////////////////////////////////////////////////////////
//// ParamBlock Definition ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static ParamBlockDesc2 gBitmapParamBlk
(
	plDynamicEnvLayer::kBlkBitmap, _T("bitmap"),  0, GetDynamicEnvLayerDesc(),
	P_AUTO_CONSTRUCT + P_AUTO_UI, plDynamicEnvLayer::kRefBitmap,

	IDD_DYNAMIC_ENVMAP_LAYER, IDS_DYNAMIC_ENVMAP_LAYER_TEX, 0, 0, &gDELBitmapDlgProc,

	// General parameters
	plDynamicEnvLayer::kBmpTextureSize,	_T("textureSize"),	TYPE_INT,		0, 0,
		p_ui,			TYPE_SPINNER, EDITTYPE_INT, IDC_TEXSIZE_EDIT, IDC_TEXSIZE_SPIN, SPIN_AUTOSCALE,
		p_range,		4, 512,
		p_default,		64,
		end,
	plDynamicEnvLayer::kBmpAnchorNode, _T("anchorNode"), TYPE_INODE, 0, 0,
		p_ui,			TYPE_PICKNODEBUTTON, IDC_ANCHOR_NODE,
		p_prompt,		IDS_SELECT_ANCHOR,
		p_accessor,		&gBleahPBAccessor,
		end,
	plDynamicEnvLayer::kBmpLastTextureSize,	_T("lastTextureSize"),	TYPE_INT,		0, 0,
		end,

	plDynamicEnvLayer::kBmpRefract,	_T("refract"),TYPE_BOOL,		0, 0,
		p_ui,			TYPE_SINGLECHEKBOX, IDC_REFRACT,
		end,

	end
);

