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
#include "HeadSpin.h"
#include "max.h"
#include "resource.h"
#include "hsTemplates.h"
#include "plComponent.h"
#include "plComponentReg.h"
#include "plMiscComponents.h"
#include "plAnimComponent.h"
#include "plNotetrackAnim.h"

#include "plGUIComponents.h"

#include "plAudioComponents.h"

#include "../MaxMain/plPlasmaRefMsgs.h"

#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../pnSceneObject/plDrawInterface.h"
#include "../plDrawable/plDrawableSpans.h"
#include "../plDrawable/plGeometrySpan.h"
#include "../plSurface/plLayerInterface.h"
#include "../plSurface/plLayer.h"
#include "../plSurface/hsGMaterial.h"
#include "../plGImage/plMipmap.h"
#include "../plGImage/plDynamicTextMap.h"

#include "../plMessage/plLayRefMsg.h"
#include "../plMessage/plMatRefMsg.h"

#include "../MaxMain/plPluginResManager.h"


#include "plgDispatch.h"
#include "../pnMessage/plObjRefMsg.h"
#include "../pnMessage/plIntRefMsg.h"
#include "../pnMessage/plNodeRefMsg.h"


#include "../plScene/plSceneNode.h"
#include "../MaxConvert/hsConverterUtils.h"
#include "../MaxConvert/hsControlConverter.h"
#include "../MaxConvert/hsMaterialConverter.h"
#include "../MaxConvert/plLayerConverter.h"
#include "../plInterp/plController.h"
#include "../plInterp/plAnimEaseTypes.h"
#include "../MaxMain/plMaxNode.h"
#include "../pnKeyedObject/plKey.h"

// GUIDialog component.
#include "../plScene/plPostEffectMod.h"
#include "../pfGameGUIMgr/pfGameGUIMgr.h"
#include "../pfGameGUIMgr/pfGUIDialogMod.h"
#include "../pfGameGUIMgr/pfGUIControlMod.h"
#include "../pfGameGUIMgr/pfGUIControlHandlers.h"
#include "../pfGameGUIMgr/pfGUIButtonMod.h"
#include "../pfGameGUIMgr/pfGUIDraggableMod.h"
#include "../pfGameGUIMgr/pfGUIListBoxMod.h"
#include "../pfGameGUIMgr/pfGUITextBoxMod.h"
#include "../pfGameGUIMgr/pfGUIEditBoxMod.h"
#include "../pfGameGUIMgr/pfGUIUpDownPairMod.h"
#include "../pfGameGUIMgr/pfGUIKnobCtrl.h"
#include "../pfGameGUIMgr/pfGUITagDefs.h"
#include "../pfGameGUIMgr/pfGUIDragBarCtrl.h"
#include "../pfGameGUIMgr/pfGUICheckBoxCtrl.h"
#include "../pfGameGUIMgr/pfGUIRadioGroupCtrl.h"
#include "../pfGameGUIMgr/pfGUIDynDisplayCtrl.h"
#include "../pfGameGUIMgr/pfGUIMultiLineEditCtrl.h"
#include "../pfGameGUIMgr/pfGUIProgressCtrl.h"
#include "../pfGameGUIMgr/pfGUIClickMapCtrl.h"
#include "../pfGameGUIMgr/pfGUIPopUpMenu.h"

// Location Related
#include "../plAgeDescription/plAgeDescription.h"
#include "../MaxMain/plMaxCFGFile.h"
#include "../MaxMain/plAgeDescInterface.h"
#include "../plFile/hsFiles.h"

#include "../MaxConvert/plConvert.h"
#include "../MaxPlasmaMtls/Layers/plDynamicTextLayer.h"
#include "../MaxPlasmaMtls/Layers/plLayerTexBitmapPB.h"

#include "../MaxMain/plMaxAccelerators.h"

#include "plPickMaterialMap.h"

#include "../plInterp/plController.h"
#include "../plAvatar/plMatrixChannel.h"

#include "../MaxPlasmaMtls/Layers/plLayerTex.h"

#include "pfGUISkinComp.h"

#include "../plResMgr/plLocalization.h"

#include "plPickLocalizationDlg.h"

#include <vector>
#include <string>


void DummyCodeIncludeFuncGUI() {}



/////////////////////////////////////////////////////////////////////////////////////////////////
//// Helper Classes /////////////////////////////////////////////////////////////////////////////

//// Hit Callback for GUI Controls //////////////////////////////////////////////////////////////

class plGUICtrlHitCallback : public HitByNameDlgCallback
{
protected:
	INode*			fOwner;
	IParamBlock2*	fPB;
	ParamID			fNodeListID;
	BOOL			fRestrict;
	hsTArray<Class_ID>		fRestrictedIDs;
	TCHAR			fTitle[ 128 ];
	BOOL			fSingle;

public:
	plGUICtrlHitCallback( INode* owner, IParamBlock2 *pb, ParamID nodeListID, TCHAR *title = nil,
							BOOL restricted = FALSE, Class_ID rID = GUI_BUTTON_CLASSID, BOOL single = TRUE )
		: fOwner( owner ), fPB( pb ), fNodeListID( nodeListID ), fRestrict( restricted ), fSingle( single )

	{
		fRestrictedIDs.Append( rID );
		strcpy( fTitle, title );
	}

	plGUICtrlHitCallback( INode* owner, IParamBlock2 *pb, ParamID nodeListID, TCHAR *title, 
							hsTArray<Class_ID> &rID )
		: fOwner( owner ), fPB( pb ), fNodeListID( nodeListID ), fRestrict( true ), fSingle(TRUE)

	{
		for( int i = 0; i < rID.GetCount(); i++ )
			fRestrictedIDs.Append( rID[ i ] );

		strcpy( fTitle, title );
	}

	virtual TCHAR *dialogTitle() { return fTitle; }
	virtual TCHAR *buttonText() { return "OK"; }

	virtual int filter(INode *node)
	{
		if( node == fOwner )
			return FALSE;

		plComponentBase *comp = ((plMaxNodeBase*)node)->ConvertToComponent();

		// If this is an activator type component
		if( comp )
		{
			if( ( fRestrict && fRestrictedIDs.Find( comp->ClassID() ) != fRestrictedIDs.kMissingIndex )
				|| ( !fRestrict && plGUIControlBase::GetGUIComp( comp ) != nil ) )
			{

				// And this wouldn't create a cyclical reference (Max doesn't like those)
				if (comp->TestForLoop(FOREVER, fPB) == REF_FAIL)
					return FALSE;

				return TRUE;
			}
		}
		else if( fRestrict && fRestrictedIDs.Find( node->ClassID() ) != fRestrictedIDs.kMissingIndex )
		{
			return TRUE;
		}

		return FALSE;
	}

	virtual void proc(INodeTab &nodeTab)
	{
		if ( nodeTab.Count() > 0 )
		{
			if( fSingle )
				fPB->SetValue( fNodeListID, TimeValue(0), nodeTab[0] );
			else
				fPB->Append( fNodeListID, nodeTab.Count(), &nodeTab[0] );
		}
	}

	virtual BOOL showHiddenAndFrozen() { return TRUE; }
	virtual BOOL singleSelect() { return TRUE; }
};

//// Single GUI Control Dialog Proc /////////////////////////////////////////////////////////////

class plGUISingleCtrlDlgProc : public ParamMap2UserDlgProc
{
protected:
	ParamID			fNodeID;
	int				fDlgItem;
	TCHAR			fTitle[ 128 ];
	hsTArray<Class_ID>	fClassesToSelect;

	ParamMap2UserDlgProc	*fProcChain;

public:

	int		GetHandledDlgItem( void ) const { return fDlgItem; }

	static const Class_ID		kEndClassList; 

	plGUISingleCtrlDlgProc( ParamID nodeID, int dlgItem, TCHAR *title, Class_ID *restrict, ParamMap2UserDlgProc *parentProc = nil )
	{
		fNodeID = nodeID;
		fDlgItem = dlgItem;
		for( int i = 0; restrict[ i ] != kEndClassList; i++ )
			fClassesToSelect.Append( restrict[ i ] );
//		fClassToSelect = restrict;
		strcpy( fTitle, title );
		fProcChain = parentProc;
	}

	BOOL DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
	{
		switch ( msg )
		{
			case WM_INITDIALOG:
				{
					IParamBlock2 *pb = map->GetParamBlock();

					INode *node = pb->GetINode( fNodeID );
					TSTR newName( node ? node->GetName() : "Pick" );
					::SetWindowText( ::GetDlgItem( hWnd, fDlgItem ), newName );
				}
				break;

			case WM_COMMAND:
				if( ( HIWORD( wParam ) == BN_CLICKED ) )
				{
					if( LOWORD( wParam ) == fDlgItem )
					{
						IParamBlock2 *pb = map->GetParamBlock();
						plGUICtrlHitCallback hitCB( (INode *)pb->GetOwner(), pb, fNodeID, fTitle, fClassesToSelect );
						GetCOREInterface()->DoHitByNameDialog( &hitCB );

						INode* node = pb->GetINode( fNodeID );
						TSTR newName( node ? node->GetName() : "Pick" );
						::SetWindowText( ::GetDlgItem(hWnd, fDlgItem ), newName );
						map->Invalidate( fNodeID );
						::InvalidateRect( hWnd, NULL, TRUE );
						return true;
					}
				}
				break;
		}

		if( fProcChain )
			fProcChain->DlgProc( t, map, hWnd, msg, wParam, lParam );

		return false;
	}

	void DeleteThis() {}
};

const Class_ID		plGUISingleCtrlDlgProc::kEndClassList = Class_ID(); 

Class_ID	sSkinClassesToSelect[] = { GUI_SKIN_CLASSID, plGUISingleCtrlDlgProc::kEndClassList };


//// Multiple GUI Control Dialog Proc ///////////////////////////////////////////////////////////

class plGUIMultipleCtrlDlgProc : public ParamMap2UserDlgProc
{
protected:
	hsTArray<plGUISingleCtrlDlgProc *>	fSingleProcs;
	hsTArray<ParamMap2UserDlgProc *>	fProcs;

public:

	plGUIMultipleCtrlDlgProc( plGUISingleCtrlDlgProc **singleProcs, ParamMap2UserDlgProc **procs=nil )
	{
		for( int i = 0; singleProcs[ i ] != nil; i++ )
			fSingleProcs.Append( singleProcs[ i ] );
		if ( procs )
		{
			for( int i = 0; procs[ i ] != nil; i++ )
				fProcs.Append( procs[ i ] );
		}
	}

	BOOL DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
	{
		int		i;


		switch ( msg )
		{
			case WM_INITDIALOG:
				for( i = 0; i < fSingleProcs.GetCount(); i++ )
					fSingleProcs[ i ]->DlgProc( t, map, hWnd, msg, wParam, lParam );

				for( i = 0; i < fProcs.GetCount(); i++ )
					fProcs[ i ]->DlgProc( t, map, hWnd, msg, wParam, lParam );

				return true;

			case WM_COMMAND:
				for( i = 0; i < fSingleProcs.GetCount(); i++ )
				{
					if( fSingleProcs[ i ]->GetHandledDlgItem() == LOWORD( wParam ) )
					{
						fSingleProcs[ i ]->DlgProc( t, map, hWnd, msg, wParam, lParam );
						break;
					}
				}
				// and now do the procs that want more control
				for( i = 0; i < fProcs.GetCount(); i++ )
					fProcs[ i ]->DlgProc( t, map, hWnd, msg, wParam, lParam );

				return true;
		}

		return false;
	}

	void DeleteThis() {}
};


/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUITag Component /////////////////////////////////////////////////////////////////////
//
//	GUI element that can be dragged anywhere in the 2D viewing plane.
//
/////////////////////////////////////////////////////////////////////////////////////////////////


class plGUITagProc : public ParamMap2UserDlgProc
{
protected:

	void	ILoadTags( HWND hWnd, IParamBlock2 *pb );

public:

	void DeleteThis() {}

	BOOL DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
};

static plGUITagProc gGUITagProc;

// Class that accesses the paramblock below.
class plGUITagComponent : public plComponent
{
public:
	plGUITagComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);

	hsBool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

	enum
	{
		kRefCurrIDSel = 64		// So we can share it among other components
	};

	static UInt32 GetTagIDOnNode( plMaxNode *node );
};

//Max desc stuff necessary below.
#define kGUITagClassID		Class_ID(0x77276e84, 0x24f360c5)
CLASS_DESC(plGUITagComponent, gGUITagDesc, "GUI ID Tag",  "GUITag", COMP_TYPE_GUI, kGUITagClassID )

ParamBlockDesc2 gGUITagBk
(	// KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUITag"), 0, &gGUITagDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_GUITAG, IDS_COMP_GUITAG, 0, 0, &gGUITagProc,

		plGUITagComponent::kRefCurrIDSel,	_T("currSel"),		TYPE_INT, 		0, 0,
		end,

	end
);

void	plGUITagProc::ILoadTags( HWND hWnd, IParamBlock2 *pb )
{
	int		idx, idx2 = 0;
	char	str[] = "(none)";


	SendMessage( hWnd, CB_RESETCONTENT, 0, 0 );
	idx2 = idx = SendMessage( hWnd, CB_ADDSTRING, 0, (LPARAM)str );
	SendMessage( hWnd, CB_SETITEMDATA, (WPARAM)idx, (LPARAM)0 );

	for( UInt32 i = 0; i < pfGameGUIMgr::GetNumTags(); i++ )
	{
		pfGUITag *tag = pfGameGUIMgr::GetTag( i );
		idx = SendMessage( hWnd, CB_ADDSTRING, 0, (LPARAM)tag->fName );
		SendMessage( hWnd, CB_SETITEMDATA, (WPARAM)idx, (LPARAM)tag->fID );

		if( tag->fID == pb->GetInt( plGUITagComponent::kRefCurrIDSel ) )
			idx2 = idx;
	}

	if( idx2 == 0 && pb->GetInt( plGUITagComponent::kRefCurrIDSel ) != 0 )
	{
		char	str[ 32 ];
		sprintf( str, "%d", pb->GetInt( plGUITagComponent::kRefCurrIDSel ) );
		SendMessage( hWnd, WM_SETTEXT, 0, (LPARAM)str );
	}
	else
		SendMessage( hWnd, CB_SETCURSEL, idx2, 0 );
}

// Callback enum proc for below
BOOL CALLBACK	GetEditCtrlEnumProc( HWND hWnd, LPARAM lParam )
{
	char	className[ 128 ];


	// ICK
	GetClassName( hWnd, className, sizeof( className ) - 1 );
	if( stricmp( className, "EDIT" ) == 0 )
	{
		HWND	*ptr = (HWND *)lParam;
		*ptr = hWnd;
		return FALSE;
	}
	return TRUE;
}

// Small proc that, given the handle of a combo box, returns the handle of the edit window for it
static HWND		GetEditCtrlFromComboBox( HWND combo )
{
	HWND	toReturn;

	EnumChildWindows( combo, GetEditCtrlEnumProc, (LPARAM)&toReturn );
	return toReturn;
}

// Small proc to only allow numbers in an edit box
static WNDPROC		sOriginalProc = nil;
LRESULT CALLBACK	SubclassedEditProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		case WM_CHAR:
			if( !isdigit( (TCHAR)wParam ) )
				return 0;
			break;

		case WM_GETDLGCODE:
			return DLGC_WANTALLKEYS;

		case WM_KEYDOWN:
			if( wParam == VK_RETURN )
			{
				// Do the same thing as when we lose focus--check our int value and make
				// sure it's big enough (don't worry about setting the paramBlock value,
				// that'll happen when the control loses focus)

				char	str[ 32 ];
				GetWindowText( hWnd, str, sizeof( str ) - 1 );
				int id = atoi( str );

				if( id < pfGameGUIMgr::GetHighestTag() + 1 )
				{
					id = pfGameGUIMgr::GetHighestTag() + 1;
					sprintf( str, "%d", id );
					SetWindowText( hWnd, str );
				}
				SendMessage( hWnd, EM_SETSEL, 0, (LPARAM)-1 );
				return 0;
			}
			break;
	}

	return CallWindowProc( sOriginalProc, hWnd, msg, wParam, lParam );
}

BOOL plGUITagProc::DlgProc( TimeValue t, IParamMap2 *pmap, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	HWND	edit;
	BOOL	dummy1;


	switch( msg )
	{
		case WM_INITDIALOG:
			ILoadTags( GetDlgItem( hWnd, IDC_GUI_TAGCOMBO ), pmap->GetParamBlock() );

			// Set the edit control of the combo box to only accept number characters
			edit = GetEditCtrlFromComboBox( GetDlgItem( hWnd, IDC_GUI_TAGCOMBO ) );
			SetWindowLong( edit, GWL_STYLE, GetWindowLong( edit, GWL_STYLE ) | ES_WANTRETURN );
			sOriginalProc = (WNDPROC)SetWindowLong( edit, GWL_WNDPROC, (DWORD)SubclassedEditProc );
            
			return true;

		case WM_DESTROY:
			SetWindowLong( GetDlgItem( hWnd, IDC_GUI_TAGCOMBO ), GWL_WNDPROC, (DWORD)sOriginalProc );
			break;

		case WM_COMMAND:
			if( LOWORD( wParam ) == IDC_GUI_TAGCOMBO )
			{
				if( HIWORD( wParam ) == CBN_SELCHANGE )
				{
					int idx = SendDlgItemMessage( hWnd, IDC_GUI_TAGCOMBO, CB_GETCURSEL, 0, 0 );
					if( idx == CB_ERR )
					{
						// Must be a custom one
						int id = GetDlgItemInt( hWnd, IDC_GUI_TAGCOMBO, &dummy1, false );
						pmap->GetParamBlock()->SetValue( plGUITagComponent::kRefCurrIDSel, 0, id );
					}
					else
					{
						pmap->GetParamBlock()->SetValue( plGUITagComponent::kRefCurrIDSel, 0, 
									SendDlgItemMessage( hWnd, IDC_GUI_TAGCOMBO, CB_GETITEMDATA, idx, 0 ) );
					}
				}
				else if( HIWORD( wParam ) == CBN_KILLFOCUS )
				{
					plMaxAccelerators::Enable();

					// Make sure the number inside is valid
					if( SendDlgItemMessage( hWnd, IDC_GUI_TAGCOMBO, CB_GETCURSEL, 0, 0 ) == CB_ERR )
					{
						int id = GetDlgItemInt( hWnd, IDC_GUI_TAGCOMBO, &dummy1, false );
						if( id < pfGameGUIMgr::GetHighestTag() + 1 )
						{
							id = pfGameGUIMgr::GetHighestTag() + 1;
							SetDlgItemInt( hWnd, IDC_GUI_TAGCOMBO, id, false );
						}

						pmap->GetParamBlock()->SetValue( plGUITagComponent::kRefCurrIDSel, 0, id );
					}
				}
				else if( HIWORD( wParam ) == CBN_SETFOCUS )
				{
					plMaxAccelerators::Disable();
				}
			}
			break;
	}
	return false;
}

plGUITagComponent::plGUITagComponent()
{
	fClassDesc = &gGUITagDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plGUITagComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	return true;
}

hsBool plGUITagComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{

	return true;
}

hsBool plGUITagComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	return true;
}

UInt32 plGUITagComponent::GetTagIDOnNode( plMaxNode *node )
{
	UInt32	i;


	for( i = 0; i < node->NumAttachedComponents( false ); i++ )
	{
		plComponentBase *comp = node->GetAttachedComponent( i, false );
		if( comp->ClassID() == kGUITagClassID )
		{
			plGUITagComponent *tag = (plGUITagComponent *)comp;
			return tag->GetParamBlockByID( plComponent::kBlkComp )->GetInt( kRefCurrIDSel );
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIColorScheme Component /////////////////////////////////////////////////////////////
//
//	Defines the color scheme for a single control or an entire dialog
//
/////////////////////////////////////////////////////////////////////////////////////////////////


class plGUIColorSchemeProc : public ParamMap2UserDlgProc
{
protected:

	void	ILoadFonts( HWND hWnd, IParamBlock2 *pb );

	static int CALLBACK IMyFontEnumProc( const ENUMLOGFONTEX *logFontData, const NEWTEXTMETRICEX *physFontData,
													unsigned long fontType, LPARAM lParam );

public:

	void DeleteThis() {}

	virtual void	Update( TimeValue t, Interval &valid, IParamMap2 *map );

	BOOL DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

};

static plGUIColorSchemeProc gGUIColorSchemeProc;

// Class that accesses the paramblock below.
class plGUIColorSchemeComp : public plComponent
{
public:
	plGUIColorSchemeComp();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);

	hsBool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

	enum
	{
		kRefForeColor = 128,		// So we can share it among other components
		kRefBackColor,
		kRefSelForeColor,
		kRefSelBackColor,
		kRefUseAlphas,
		kRefFontFace,
		kRefFontSize,
		kRefFontBold,
		kRefFontItalic,

		kRefForeAlpha,
		kRefBackAlpha,
		kRefSelForeAlpha,
		kRefSelBackAlpha,

		kRefFontShadowed
	};

	static void		ConvertScheme( IParamBlock2 *pb, pfGUIColorScheme *destScheme, plErrorMsg *pErrMsg );
};

//Max desc stuff necessary below.
CLASS_DESC(plGUIColorSchemeComp, gGUIColorSchemeDesc, "GUI Color Scheme",  "GUIColorScheme", COMP_TYPE_GUI, GUI_COLORSCHEME_CLASSID )

static ParamBlockDesc2	gGUIColorSchemeBk
(
	/// Main def
	 plComponent::kBlkComp, _T("GUIColorScheme"), 0, &gGUIColorSchemeDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, plComponent::kRefComp,

	1, 
	plGUIDialogComponent::kSchemeRollout, IDD_COMP_GUISCHEME, IDS_COMP_GUISCHEME, 0, 0, &gGUIColorSchemeProc,	

	plGUIColorSchemeComp::kRefForeColor,	_T("foreColor"),		TYPE_RGBA, 		0, 0,
		p_ui, plGUIDialogComponent::kSchemeRollout, TYPE_COLORSWATCH, IDC_GUI_FGCOLOR,
		p_default, Color( 1.f, 1.f, 1.f ),
		end,

	plGUIColorSchemeComp::kRefBackColor,	_T("backColor"),		TYPE_RGBA, 		0, 0,
		p_ui, plGUIDialogComponent::kSchemeRollout, TYPE_COLORSWATCH, IDC_GUI_BGCOLOR,
		p_default, Color( 0.f, 0.f, 0.f ),
		end,

	plGUIColorSchemeComp::kRefSelForeColor,	_T("selForeColor"),		TYPE_RGBA, 		0, 0,
		p_ui, plGUIDialogComponent::kSchemeRollout, TYPE_COLORSWATCH, IDC_GUI_SFGCOLOR,
		p_default, Color( 1.f, 1.f, 1.f ),
		end,

	plGUIColorSchemeComp::kRefSelBackColor,	_T("selBackColor"),		TYPE_RGBA, 		0, 0,
		p_ui, plGUIDialogComponent::kSchemeRollout, TYPE_COLORSWATCH, IDC_GUI_SBGCOLOR,
		p_default, Color( 0.f, 0.f, 1.f ),
		end,


	plGUIColorSchemeComp::kRefForeAlpha,	_T("foreAlpha"),		TYPE_FLOAT, 		0, 0,
		p_ui, plGUIDialogComponent::kSchemeRollout, TYPE_SLIDER, EDITTYPE_FLOAT, IDC_GUI_FGAEDIT, IDC_GUI_FGALPHA, 4,
		p_range, 0.f, 1.f,
		p_default, 1.f,
		end,

	plGUIColorSchemeComp::kRefBackAlpha,	_T("backAlpha"),		TYPE_FLOAT, 		0, 0,
		p_ui, plGUIDialogComponent::kSchemeRollout, TYPE_SLIDER, EDITTYPE_FLOAT, IDC_GUI_BGAEDIT, IDC_GUI_BGALPHA, 4,
		p_range, 0.f, 1.f,
		p_default, 1.f,
		end,

	plGUIColorSchemeComp::kRefSelForeAlpha,	_T("selForeAlpha"),		TYPE_FLOAT, 		0, 0,
		p_ui, plGUIDialogComponent::kSchemeRollout, TYPE_SLIDER, EDITTYPE_FLOAT, IDC_GUI_SFGAEDIT, IDC_GUI_SFGALPHA, 4,
		p_range, 0.f, 1.f,
		p_default, 1.f,
		end,

	plGUIColorSchemeComp::kRefSelBackAlpha,	_T("selBackAlpha"),		TYPE_FLOAT, 		0, 0,
		p_ui, plGUIDialogComponent::kSchemeRollout, TYPE_SLIDER, EDITTYPE_FLOAT, IDC_GUI_SBGAEDIT, IDC_GUI_SBGALPHA, 4,
		p_range, 0.f, 1.f,
		p_default, 1.f,
		end,


	plGUIColorSchemeComp::kRefUseAlphas, _T("useAlphas"),		TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui, plGUIDialogComponent::kSchemeRollout, TYPE_SINGLECHEKBOX, IDC_GUI_USEALPHAS,
		p_enable_ctrls, 4, plGUIColorSchemeComp::kRefForeAlpha, plGUIColorSchemeComp::kRefBackAlpha, 
							plGUIColorSchemeComp::kRefSelForeAlpha, plGUIColorSchemeComp::kRefSelBackAlpha,
		end,		


	plGUIColorSchemeComp::kRefFontFace, _T("fontFace"),		TYPE_STRING, 		0, 0,
		p_default,	_T( "Times New Roman" ),
		end,		

	plGUIColorSchemeComp::kRefFontSize,	_T("fontSize"),		TYPE_INT, 		0, 0,
		p_ui,	plGUIDialogComponent::kSchemeRollout, TYPE_SPINNER, EDITTYPE_POS_INT, IDC_GUI_FONTSIZE, IDC_GUI_FONTSIZE_SPIN, SPIN_AUTOSCALE,
		p_default, 10,
		end,

	plGUIColorSchemeComp::kRefFontBold, _T("fontBold"),		TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui, plGUIDialogComponent::kSchemeRollout, TYPE_SINGLECHEKBOX, IDC_GUI_FONTBOLD,
		end,		

	plGUIColorSchemeComp::kRefFontItalic, _T("fontItalic"),		TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui, plGUIDialogComponent::kSchemeRollout, TYPE_SINGLECHEKBOX, IDC_GUI_FONTITALIC,
		end,		

	plGUIColorSchemeComp::kRefFontShadowed, _T("fontShadowed"),		TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui, plGUIDialogComponent::kSchemeRollout, TYPE_SINGLECHEKBOX, IDC_GUI_FONTSHADOWED,
		end,		

	end
);

int CALLBACK plGUIColorSchemeProc::IMyFontEnumProc( const ENUMLOGFONTEX *logFontData, const NEWTEXTMETRICEX *physFontData,
													unsigned long fontType, LPARAM lParam )
{
	HWND	combo = (HWND)lParam;


	if( SendMessage( combo, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)logFontData->elfLogFont.lfFaceName ) == CB_ERR )
		SendMessage( combo, CB_ADDSTRING, 0, (LPARAM)logFontData->elfLogFont.lfFaceName );

	return -1;
}

void	plGUIColorSchemeProc::ILoadFonts( HWND hWnd, IParamBlock2 *pb )
{
	LOGFONT	logFont;


	logFont.lfCharSet = DEFAULT_CHARSET;
	strcpy( logFont.lfFaceName, "" );
	logFont.lfPitchAndFamily = 0;

	SendMessage( hWnd, CB_RESETCONTENT, 0, 0 );

	HDC hDC = GetDC( nil );
	EnumFontFamiliesEx( hDC, &logFont, (FONTENUMPROC)IMyFontEnumProc, (LPARAM)hWnd, 0 );
	ReleaseDC( nil, hDC );

	SendMessage( hWnd, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)pb->GetStr( plGUIColorSchemeComp::kRefFontFace ) );
}

#define MAXTOCOLORREF( max ) RGB( max.r * 255.f, max.g * 255.f, max.b * 255.f )

void	plGUIColorSchemeProc::Update( TimeValue t, Interval &valid, IParamMap2 *pmap )
{
}

BOOL plGUIColorSchemeProc::DlgProc( TimeValue t, IParamMap2 *pmap, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	char			str[ 256 ];
	HWND			placeCtrl;
	PAINTSTRUCT		paintInfo;
	RECT			previewRect, r;
	HBRUSH			bgPattBrush = nil;
	Color			fgColor, bgColor, selFgColor, selBgColor;
	Color			hatchColor = Color( 0.4f, 0.4f, 0.4f ), blendedColor, whiteColor = Color( 0.7f, 0.7f, 0.7f );
	Color			blackColor = Color( 0, 0, 0 ), blendedColor2;
	float			fgAlpha, bgAlpha, selFgAlpha, selBgAlpha;
	char			previewString[] = "Preview";
	HFONT			font;


	switch( msg )
	{
		case WM_INITDIALOG:
			ILoadFonts( GetDlgItem( hWnd, IDC_GUI_FONTFACE ), pmap->GetParamBlock() );           
			return true;

		case WM_DESTROY:
			break;

		case WM_COMMAND:
			if( LOWORD( wParam ) == IDC_GUI_FONTFACE )
			{
				if( HIWORD( wParam ) == CBN_SELCHANGE )
				{
					int idx = SendDlgItemMessage( hWnd, IDC_GUI_FONTFACE, CB_GETCURSEL, 0, 0 );

					SendDlgItemMessage( hWnd, IDC_GUI_FONTFACE, CB_GETLBTEXT, idx, (LPARAM)str );

					pmap->GetParamBlock()->SetValue( plGUIColorSchemeComp::kRefFontFace, 0, str );
				}
			}
			break;

		case CC_COLOR_CHANGE:
		case CC_COLOR_DROP:
			placeCtrl = ::GetDlgItem( hWnd, IDC_GUI_SCHEMEPREV );
			::GetClientRect( placeCtrl, &previewRect );
			::MapWindowPoints( placeCtrl, hWnd, (POINT *)&previewRect, 2 );
			::InvalidateRect( hWnd, &previewRect, FALSE );
			break;

		case WM_PAINT:

			fgColor = pmap->GetParamBlock()->GetColor( plGUIColorSchemeComp::kRefForeColor );
			bgColor = pmap->GetParamBlock()->GetColor( plGUIColorSchemeComp::kRefBackColor );
			selFgColor = pmap->GetParamBlock()->GetColor( plGUIColorSchemeComp::kRefSelForeColor );
			selBgColor = pmap->GetParamBlock()->GetColor( plGUIColorSchemeComp::kRefSelBackColor );

			fgAlpha = pmap->GetParamBlock()->GetFloat( plGUIColorSchemeComp::kRefForeAlpha );
			bgAlpha = pmap->GetParamBlock()->GetFloat( plGUIColorSchemeComp::kRefBackAlpha );
			selFgAlpha = pmap->GetParamBlock()->GetFloat( plGUIColorSchemeComp::kRefSelForeAlpha );
			selBgAlpha = pmap->GetParamBlock()->GetFloat( plGUIColorSchemeComp::kRefSelBackAlpha );
			if( pmap->GetParamBlock()->GetInt( plGUIColorSchemeComp::kRefUseAlphas ) == 0 )
				fgAlpha = bgAlpha = selFgAlpha = selBgAlpha = 1.f;

			placeCtrl = ::GetDlgItem( hWnd, IDC_GUI_SCHEMEPREV );
			::GetClientRect( placeCtrl, &previewRect );
			::MapWindowPoints( placeCtrl, hWnd, (POINT *)&previewRect, 2 );

			::BeginPaint( hWnd, &paintInfo );
			::SetBkMode( paintInfo.hdc, TRANSPARENT );

			int weight = pmap->GetParamBlock()->GetInt( plGUIColorSchemeComp::kRefFontBold ) ? FW_BOLD : FW_NORMAL;
			bool italic = pmap->GetParamBlock()->GetInt( plGUIColorSchemeComp::kRefFontItalic ) ? true : false;
			int nHeight = -MulDiv( pmap->GetParamBlock()->GetInt( plGUIColorSchemeComp::kRefFontSize ), GetDeviceCaps( paintInfo.hdc, LOGPIXELSY ), 72 );
			const char *face = pmap->GetParamBlock()->GetStr( plGUIColorSchemeComp::kRefFontFace );

			font = ::CreateFont( nHeight, 0, 0, 0, weight, italic, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH, face );
			SelectObject( paintInfo.hdc, font );

			// Left side
			r = previewRect;
			r.right = ( r.right + r.left ) >> 1;

			blendedColor = bgColor * bgAlpha + ( whiteColor * ( 1.f - bgAlpha ) );

			// doesn't like the Color to DWORD operator, so duplicating it here
			#define ColorToDWORD(color) RGB(FLto255(color.r),FLto255(color.g),FLto255(color.b))
			::SetBkColor( paintInfo.hdc, ColorToDWORD(blendedColor) );

			blendedColor = bgColor * bgAlpha + ( hatchColor * ( 1.f - bgAlpha ) );
			bgPattBrush = CreateHatchBrush( HS_DIAGCROSS, MAXTOCOLORREF( blendedColor ) );

			::FillRect( paintInfo.hdc, &r, bgPattBrush );
			if( pmap->GetParamBlock()->GetInt( plGUIColorSchemeComp::kRefFontShadowed ) )
			{
				blendedColor2 = blackColor * fgAlpha + ( blendedColor * ( 1.f - fgAlpha ) );
				::SetTextColor( paintInfo.hdc, MAXTOCOLORREF( blendedColor2 ) );
				::OffsetRect( &r, 1, 1 );
				::DrawText( paintInfo.hdc, previewString, strlen( previewString ), &r, DT_CENTER | DT_VCENTER );
				::OffsetRect( &r, -1, -1 );
			}

			blendedColor = fgColor * fgAlpha + ( blendedColor * ( 1.f - fgAlpha ) );
			::SetTextColor( paintInfo.hdc, MAXTOCOLORREF( blendedColor ) );
			::DrawText( paintInfo.hdc, previewString, strlen( previewString ), &r, DT_CENTER | DT_VCENTER );
			
			::DeleteObject( bgPattBrush );

			// Right side
			r.left = r.right;
			r.right = previewRect.right;

			blendedColor = selBgColor * selBgAlpha + ( whiteColor * ( 1.f - selBgAlpha ) );
			::SetBkColor( paintInfo.hdc, ColorToDWORD(blendedColor) );
			blendedColor = selBgColor * selBgAlpha + ( hatchColor * ( 1.f - selBgAlpha ) );
			bgPattBrush = CreateHatchBrush( HS_DIAGCROSS, MAXTOCOLORREF( blendedColor ) );

			::FillRect( paintInfo.hdc, &r, bgPattBrush );
			if( pmap->GetParamBlock()->GetInt( plGUIColorSchemeComp::kRefFontShadowed ) )
			{
				blendedColor2 = blackColor * selFgAlpha + ( blendedColor * ( 1.f - selFgAlpha ) );
				::SetTextColor( paintInfo.hdc, MAXTOCOLORREF( blendedColor2 ) );
				::OffsetRect( &r, 1, 1 );
				::DrawText( paintInfo.hdc, previewString, strlen( previewString ), &r, DT_CENTER | DT_VCENTER );
				::OffsetRect( &r, -1, -1 );
			}
			blendedColor = selFgColor * selFgAlpha + ( blendedColor * ( 1.f - selFgAlpha ) );
			::SetTextColor( paintInfo.hdc, MAXTOCOLORREF( blendedColor ) );
			::DrawText( paintInfo.hdc, previewString, strlen( previewString ), &r, DT_CENTER | DT_VCENTER );
			
			::DeleteObject( bgPattBrush );

			::DeleteObject( font );

			::EndPaint( hWnd, &paintInfo );

			return true;
	}
	return false;
}

plGUIColorSchemeComp::plGUIColorSchemeComp()
{
	fClassDesc = &gGUIColorSchemeDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plGUIColorSchemeComp::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	return true;
}

hsBool plGUIColorSchemeComp::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{

	return true;
}

hsBool plGUIColorSchemeComp::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	pfGUIControlMod *ctrl = plGUIControlBase::GrabControlFromObject( node );
	if( ctrl != nil )
	{
		pfGUIColorScheme *cs = TRACKED_NEW pfGUIColorScheme;
		ConvertScheme( fCompPB, cs, pErrMsg );
		ctrl->SetColorScheme( cs );
	}
	else
	{
		pErrMsg->Set( true, "GUI Color Scheme Error", "You have applied a GUI color scheme to an object (%s) without a GUI control. This scheme will be ignored.", node->GetName()).Show();
		pErrMsg->Set( false );
		return false;	
	}

	return true;
}

void	SMaxRGBAToPlasmaRGBA( Color maxRGB, hsColorRGBA &plasmaRGBA )
{
	plasmaRGBA.Set( maxRGB.r, maxRGB.g, maxRGB.b, 1.f );
}

void	plGUIColorSchemeComp::ConvertScheme( IParamBlock2 *pb, pfGUIColorScheme *destScheme, plErrorMsg *pErrMsg )
{
	SMaxRGBAToPlasmaRGBA( pb->GetColor( kRefForeColor ), destScheme->fForeColor );
	SMaxRGBAToPlasmaRGBA( pb->GetColor( kRefBackColor ), destScheme->fBackColor );
	SMaxRGBAToPlasmaRGBA( pb->GetColor( kRefSelForeColor ), destScheme->fSelForeColor );
	SMaxRGBAToPlasmaRGBA( pb->GetColor( kRefSelBackColor ), destScheme->fSelBackColor );

	destScheme->fForeColor.a = pb->GetFloat( kRefForeAlpha );
	destScheme->fBackColor.a = pb->GetFloat( kRefBackAlpha );
	destScheme->fSelForeColor.a = pb->GetFloat( kRefSelForeAlpha );
	destScheme->fSelBackColor.a = pb->GetFloat( kRefSelBackAlpha );

	destScheme->fTransparent = pb->GetInt( kRefUseAlphas ) ? true : false;
	
	destScheme->SetFontFace( pb->GetStr( kRefFontFace ) );
	destScheme->fFontSize = pb->GetInt( kRefFontSize );
	destScheme->fFontFlags = 0;
	if( pb->GetInt( kRefFontBold ) )
		destScheme->fFontFlags |= pfGUIColorScheme::kFontBold;
	if( pb->GetInt( kRefFontItalic ) )
		destScheme->fFontFlags |= pfGUIColorScheme::kFontItalic;
	if( pb->GetInt( kRefFontShadowed ) )
		destScheme->fFontFlags |= pfGUIColorScheme::kFontShadowed;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIProxy Rollout /////////////////////////////////////////////////////////////////////////
//
//	Defines a proxy object to be used when calculating mouse-down locations and dynamic text
//	sizing. 
//
/////////////////////////////////////////////////////////////////////////////////////////////////

enum plProxyRefs
{
	kRefProxyNode = 196,
	kRefHideProxy,
	kRefBetterHitTests
};

//// DialogProc /////////////////////////////////////////////////////////////////////////////////

class plGUIProxyDlgProc : public ParamMap2UserDlgProc
{
public:

	plGUIProxyDlgProc()
	{
	}

	void DeleteThis() {}

	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		IParamBlock2 *pblock = map->GetParamBlock();

		switch( msg )
		{
			case WM_COMMAND:
//				if( LOWORD( wParam ) == IDC_GUI_CLEAR )
//				{
//					pblock->Reset( (ParamID)kRefProxyNode );
//					return true;
//				}
				break;
		}	
		return false;
	}

};	

static plGUIProxyDlgProc	sGUIProxyDlgProc;

//// ParamBlock /////////////////////////////////////////////////////////////////////////////////
//	Note: we can't make this a real ParamBlock and do P_INCLUDE_PARAMS because, in Discreet's 
//	amazing method of doing things, we can't INCLUDE more than one ParamBlock in any other PB.
//	So either we chain them together here (and thus make them dependent on one another, which
//	is lame) or we just make the whole damned thing a #define, which is all P_INCLUDE_PARAMS
//	really does anyway.

#define sGUIProxyParamHeader		plGUIControlBase::kRollProxy, IDD_COMP_GUIPROXY, IDS_COMP_GUIPROXY, 0, APPENDROLL_CLOSED, &sGUIProxyDlgProc
//static ParamBlockDesc2	sSndEAXPropsParamTemplate
//(
	/// Main def
//	plComponent::kBlkComp + 1, _T("sndEAXProps"), 0, nil, P_AUTO_UI + P_MULTIMAP + P_AUTO_CONSTRUCT, plComponent::kRefComp, 

//	1, 
//	kSndEAXParams, IDD_COMP_EAXBUFFER, IDS_COMP_EAXBUFFER, 0, 0, nil,	

#define sGUIProxyParamTemplate \
																											\
	kRefBetterHitTests, _T("guiBetterHitTests"), TYPE_BOOL, 0, 0,														\
		p_ui, plGUIControlBase::kRollProxy, TYPE_SINGLECHEKBOX, IDC_GUI_BETTERHIT,							\
		p_default, false,																					\
		end																								

//	, end
//);


/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIDialog Component //////////////////////////////////////////////////////////////////////
//
//	Defines a dialog box (i.e. a collection of controls) to be defined with the GUI manager at
//	runtime. Acts a lot like a CamView component, but it additionally handles a few other things.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

class plGUIDialogProc : public ParamMap2UserDlgProc
{
protected:

	void	ILoadPages( HWND hWnd, IParamBlock2 *pb );

public:

	void DeleteThis() {}

	BOOL DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
};
static plGUIDialogProc gGUIDialogProc;

//Max desc stuff necessary below.
CLASS_DESC(plGUIDialogComponent, gGUIDialogDesc, "GUI Dialog",  "GUIDialog", COMP_TYPE_GUI, GUI_DIALOG_COMP_CLASS_ID )

ParamBlockDesc2 gGUIDialogBk
(	// KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUIDialog"), 0, &gGUIDialogDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

	3, 
	plGUIDialogComponent::kMainRollout,		IDD_COMP_GUIDIALOG, IDS_COMP_GUIDIALOG, 0, 0, &gGUIDialogProc,
	plGUIDialogComponent::kTagIDRollout,	IDD_COMP_GUITAG,	IDS_COMP_GUITAG,	0, 0, &gGUITagProc,
	plGUIDialogComponent::kSchemeRollout,	IDD_COMP_GUISCHEME, IDS_COMP_GUISCHEME, 0, 0, &gGUIColorSchemeProc,	

	&gGUIColorSchemeBk,

		plGUIDialogComponent::kRefDialogName,	_T("DialogName"),		TYPE_STRING, 		0, 0,
//			p_ui, plGUIDialogComponent::kMainRollout, TYPE_EDITBOX, IDC_GUIDLG_NAME,
			end,

		plGUIDialogComponent::kRefAgeName,	_T("ageName"),		TYPE_STRING, 		0, 0,
			p_default, _T( "GUI" ),
			end,
			
		plGUIDialogComponent::kRefIsModal, _T("isModal"),		TYPE_BOOL, 		0, 0,
			p_default,	FALSE,
			p_ui, plGUIDialogComponent::kMainRollout, TYPE_SINGLECHEKBOX, IDC_COMP_GUI_MODAL,
			end,		

		plGUIDialogComponent::kRefVersion,	_T("version"),		TYPE_INT, 		0, 0,
			p_ui,	plGUIDialogComponent::kMainRollout, TYPE_SPINNER, EDITTYPE_POS_INT, IDC_GUI_VERSION, IDC_GUI_VERSION_SPIN, SPIN_AUTOSCALE,
			p_default, 0,
			end,

		plGUITagComponent::kRefCurrIDSel,	_T("currSel"),		TYPE_INT, 		0, 0,
			end,

	end
);

plGUIDialogComponent::plGUIDialogComponent( hsBool dontInit )
{
	if( !dontInit )
	{
		fClassDesc = &gGUIDialogDesc;
		fClassDesc->MakeAutoParamBlocks(this);
	}
	fDialogMod = nil;
	fProcReceiver = nil;
}

pfGUIDialogMod	*plGUIDialogComponent::IMakeDialog( void )
{
	return TRACKED_NEW pfGUIDialogMod();
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plGUIDialogComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	TimeValue timeVal( 0 );
	Object* obj = node->EvalWorldState( timeVal ).obj;

	fDialogMod = nil;

	if( obj->CanConvertToType( Class_ID( LOOKAT_CAM_CLASS_ID, 0 ) ) ||
		obj->CanConvertToType( Class_ID( SIMPLE_CAM_CLASS_ID, 0 ) ) )
	{
		// We're applied to a camera. Do our camera stuff
		node->SetForceLocal( true );
	}
	else
	{
		// We're applied to a normal object.
		node->SetNoSpanReSort(true);
		node->SetNoSpanSort(true);
	}

	/// Either way, we mangle our own location component. None of this user-defined-location stuff.

	char *dialogName = fCompPB->GetStr( kRefDialogName );
	if( dialogName == nil || *dialogName == 0 )
	{
		pErrMsg->Set(true, "GUI Dialog Component Error", "No dialog name specified on GUI Dialog component (object: %s)", node->GetName()).Show();
		return false;	
	}

	char *ageName = fCompPB->GetStr(kRefAgeName);
	Int32 seqNum = plPageInfoUtils::GetSeqNumFromAgeDesc( ageName, dialogName );
	Int32 newNum = plPluginResManager::ResMgr()->VerifySeqNumber( seqNum, ageName, dialogName );
	if( newNum != seqNum )
	{
		if( !fSeqNumValidated )
		{
			plLocation pageLoc = plPluginResManager::ResMgr()->FindLocation( ageName, dialogName );
			Int32 pageSeqNum = pageLoc.GetSequenceNumber();
			char errMsg[ 512 ];
			sprintf( errMsg, "The sequence number stored by the resource manager (0x%X) for page %s, District, %s does not match\n"
							"the sequence number stored in the .age file (0x%X). Forcing it to use the one in the .age file", 
								pageSeqNum, ageName, dialogName, seqNum );
			pErrMsg->Set( true, "PageInfo Convert Error", errMsg ).Show(); 
			pErrMsg->Set( false );
			fSeqNumValidated = true;
		}
		// force the component to use the sequence number in the .age file
		//seqNum = newNum;
	}

	plKey roomKey = plPluginResManager::ResMgr()->NameToLoc( fCompPB->GetStr( kRefAgeName ), fCompPB->GetStr( kRefDialogName ), seqNum );
	if( !roomKey )
	{
		pErrMsg->Set( true, "GUI Dialog Component Error", "GUI Dialog Component %s has a Missing Location.  Nuke the files in the dat directory and re-export.",((INode*)node)->GetName()).Show(); 
		return false;
	}

	node->SetRoomKey( roomKey );

	// Also, we make sure this node will never be fogged (affects material convert)
	node->SetIsGUI( true );

	return true;
}

hsBool plGUIDialogComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	TimeValue timeVal(0);

	Object* obj = node->EvalWorldState(timeVal).obj;

	GenCamera* cam = nil;
	if( obj->CanConvertToType(Class_ID(LOOKAT_CAM_CLASS_ID, 0)) )
		cam = (GenCamera *) obj->ConvertToType(timeVal, Class_ID(LOOKAT_CAM_CLASS_ID, 0));
	else 
	if( obj->CanConvertToType(Class_ID(SIMPLE_CAM_CLASS_ID, 0)) ) 
		cam = (GenCamera *) obj->ConvertToType(timeVal, Class_ID(SIMPLE_CAM_CLASS_ID, 0));

	if( !cam )
	{
		// Not applied to a camera, so applied to a normal object. Since this is valid (we also act
		// as a location component), just return
		return true;
	}

	plPostEffectMod* mod = TRACKED_NEW plPostEffectMod;

	float hither = cam->GetEnvRange(timeVal, ENV_NEAR_RANGE); 
	if( hither < 0.5f )
		hither = 0.5f;
	float yon = cam->GetEnvRange(timeVal, ENV_FAR_RANGE);
	mod->SetHither(hither);
	mod->SetYon(yon);

	// radians
	float fov = cam->GetFOV(timeVal);
	// convert
	int FOVType = cam->GetFOVType();
	hsScalar fovX, fovY;
	switch(FOVType)
	{
	case 0: // FOV_W
		{
			fovX = fov;
			fovY = fovX *3.f / 4.f;
		}
		break;
	case 1: // FOV_H
		{
			fovY = fov;
			fovX = fovY * 4.f / 3.f;
		}
		break;
	}
	fovX *= 180.f / hsScalarPI;
	fovY *= 180.f / hsScalarPI;
	mod->SetFovX(fovX);
	mod->SetFovY(fovY);

	// Should already be created from SetupProperties...
	// Note: can't just grab the node's room key, 'cause we might not be on the right node!
	plKey sceneNodeKey = plPluginResManager::ResMgr()->NameToLoc( fCompPB->GetStr( kRefAgeName ), 
														fCompPB->GetStr( kRefDialogName ), (UInt32)-1 );
	mod->SetNodeKey( sceneNodeKey );

//	node->AddModifier(mod);
	// Note: we do NOT add this to the sceneObject, we don't want it actually associated with
	// a sceneObject. Instead, we just grab the LocalToWorld() off the sceneObject, since that's 
	// all we want
	hsMatrix44 l2w = node->GetLocalToWorld44();
	hsMatrix44 w2l = node->GetWorldToLocal44();
	mod->SetWorldToCamera( w2l, l2w );

	// Add it to the sceneNode as a generic interface, so it gets loaded with the sceneNode
	plLocation nodeLoc = sceneNodeKey->GetUoid().GetLocation();

	plKey modKey = hsgResMgr::ResMgr()->NewKey( fCompPB->GetStr( kRefDialogName ), mod, nodeLoc );
	hsgResMgr::ResMgr()->AddViaNotify( modKey, TRACKED_NEW plNodeRefMsg( sceneNodeKey, plRefMsg::kOnCreate, -1, plNodeRefMsg::kGeneric ), plRefFlags::kActiveRef );

	// Also add our dialog mod to the scene node in the same way
	hsgResMgr::ResMgr()->AddViaNotify( fDialogMod->GetKey(), TRACKED_NEW plNodeRefMsg( sceneNodeKey, plRefMsg::kOnCreate, -1, plNodeRefMsg::kGeneric ), plRefFlags::kActiveRef );

	/// Already created our mod, just gotta fill it out
	fDialogMod->SetRenderMod( mod );
	fDialogMod->SetName( fCompPB->GetStr( kRefDialogName ) );
	if( fCompPB->GetInt( kRefIsModal ) )
		fDialogMod->SetFlag( pfGUIDialogMod::kModal );
	fDialogMod->SetProcReceiver(fProcReceiver);
	fDialogMod->SetVersion( fCompPB->GetInt( kRefVersion ) );

	plGUIColorSchemeComp::ConvertScheme( fCompPB, fDialogMod->GetColorScheme(), pErrMsg );

	return true;
}

hsBool plGUIDialogComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	TimeValue timeVal(0);
	Object* obj = node->EvalWorldState(timeVal).obj;

	if( obj->CanConvertToType(Class_ID(LOOKAT_CAM_CLASS_ID, 0))
		|| obj->CanConvertToType(Class_ID(SIMPLE_CAM_CLASS_ID, 0)) )
	{
		// Don't do this. -mf
//		IMakeEveryoneOpaque(node);

		// Make a blank dialog modifier, which will be filled in on convert. Do 
		// this as a separate step so the dialog controls can query and get the dialog
		// mod to store a ref to

		fDialogMod = IMakeDialog();

		// Note: can't just grab the node's room key, 'cause we might not be on the right node!
		plKey sceneNodeKey = plPluginResManager::ResMgr()->NameToLoc( fCompPB->GetStr( kRefAgeName ),
															fCompPB->GetStr( kRefDialogName ), (UInt32)-1 );

		plLocation nodeLoc = sceneNodeKey->GetUoid().GetLocation();
		plKey dlgKey = hsgResMgr::ResMgr()->NewKey( fCompPB->GetStr( kRefDialogName ), fDialogMod, nodeLoc );

		fDialogMod->SetSceneNodeKey( sceneNodeKey );

		// See if there's a tag to be had
		UInt32 id = fCompPB->GetInt( plGUITagComponent::kRefCurrIDSel );
		if( id > 0 )
			fDialogMod->SetTagID( id );

		fProcReceiver = nil;
	}
	else
	{
	}

	return true;
}

void plGUIDialogComponent::IMakeEveryoneOpaque(plMaxNode* node)
{
	plMaxNode* root = (plMaxNode *)node->GetInterface()->GetRootNode();

	int i;
	for( i = 0; i < root->NumberOfChildren(); i++ )
		IMakeEveryoneOpaqueRecur((plMaxNode*)(root->GetChildNode(i)));

}

void plGUIDialogComponent::IMakeEveryoneOpaqueRecur(plMaxNode* node)
{
	if( node->CanConvert() )
	{
		node->SetNoSpanReSort(true);
		node->SetNoSpanSort(true);

		int i;
		for( i = 0; i < node->NumberOfChildren(); i++ )
		{
			IMakeEveryoneOpaqueRecur((plMaxNode *)(node->GetChildNode(i)));
		}
	}
}

plKey	plGUIDialogComponent::GetModifierKey( void )
{
	if( fDialogMod != nil )
		return fDialogMod->GetKey();

	return nil;
}

bool	plGUIDialogComponent::SetNotifyReceiver( plKey key )
{
	if( fProcReceiver != nil )
		return false;

	fProcReceiver = key;
	return true;
}

pfGUIDialogMod	*plGUIDialogComponent::GetNodeDialog( plMaxNode *childNode )
{
	UInt32 i, numComp = childNode->NumAttachedComponents( false );
	for( i = 0; i < numComp; i++ )
	{
		plComponentBase *comp = childNode->GetAttachedComponent( i );
		if( comp->ClassID() == GUI_DIALOG_COMP_CLASS_ID )
			return ( (plGUIDialogComponent *)comp )->GetModifier();
	}

	return nil;
}

void	plGUIDialogProc::ILoadPages( HWND hWnd, IParamBlock2 *pb )
{
	plAgeDescription	*aged = plPageInfoUtils::GetAgeDesc( pb->GetStr( plGUIDialogComponent::kRefAgeName ) );

	if( aged == nil )
		return;

	plAgePage	*page;
	char	*selPageName = pb->GetStr( plGUIDialogComponent::kRefDialogName );
	aged->SeekFirstPage();
	ComboBox_ResetContent( hWnd );

	while( ( page = aged->GetNextPage() ) != nil )
	{
		int idx = ComboBox_AddString( hWnd, page->GetName() );
		if( selPageName && stricmp( page->GetName(), selPageName ) == 0 )
			ComboBox_SetCurSel( hWnd, idx );
	}

	delete aged;
}

BOOL plGUIDialogProc::DlgProc( TimeValue t, IParamMap2 *pmap, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		case WM_INITDIALOG:
			// Load the age combo box
			{
				int					i, idx, selIdx = 0;
				HWND				ageCombo = GetDlgItem( hWnd, IDC_GUIDLG_AGE );
				hsTArray<char *>	ageList;

				plAgeDescInterface::BuildAgeFileList( ageList );
				ComboBox_ResetContent( ageCombo );
				for( i = 0; i < ageList.GetCount(); i++ )
				{
					char ageName[ _MAX_FNAME ];
					_splitpath( ageList[ i ], nil, nil, ageName, nil );

					idx = ComboBox_AddString( ageCombo, ageName );
					if( stricmp( ageName, pmap->GetParamBlock()->GetStr( plGUIDialogComponent::kRefAgeName ) ) == 0 )
					{
						selIdx = idx;
					}
				}
				ComboBox_SetCurSel( ageCombo, selIdx );
			}

			ILoadPages( GetDlgItem( hWnd, IDC_GUIDLG_NAME ), pmap->GetParamBlock() );           
			return true;

		case WM_DESTROY:
			break;

		case WM_COMMAND:
			if( HIWORD( wParam ) == CBN_SELCHANGE )
			{
				if( LOWORD( wParam ) == IDC_GUIDLG_NAME )
				{
					int idx = SendDlgItemMessage( hWnd, IDC_GUIDLG_NAME, CB_GETCURSEL, 0, 0 );
					if( idx != CB_ERR )
					{
						char	name[ 256 ];
						ComboBox_GetLBText( GetDlgItem( hWnd, IDC_GUIDLG_NAME ), idx, name );
						pmap->GetParamBlock()->SetValue( plGUIDialogComponent::kRefDialogName, 0, name );
					}
				}
				else if( LOWORD( wParam ) == IDC_GUIDLG_AGE )
				{
					int idx = SendDlgItemMessage( hWnd, IDC_GUIDLG_AGE, CB_GETCURSEL, 0, 0 );
					if( idx != CB_ERR )
					{
						char	name[ 256 ];
						ComboBox_GetLBText( GetDlgItem( hWnd, IDC_GUIDLG_AGE ), idx, name );
						pmap->GetParamBlock()->SetValue( plGUIDialogComponent::kRefAgeName, 0, name );
					}

					ILoadPages( GetDlgItem( hWnd, IDC_GUIDLG_NAME ), pmap->GetParamBlock() );           
				}
			}
			break;
	}
	return false;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIControl Component Base Class //////////////////////////////////////////////////////////
//
//	Defines a base class for all GUI control components.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

void	plGUIControlBase::CollectNonDrawables( INodeTab &nonDrawables )
{
/*	if( ICanHaveProxy() )
	{
		bool hideProxy = fCompPB->GetInt( (ParamID)kRefHideProxy ) ? true : false;
		if( hideProxy )
		{
			INode	*node = fCompPB->GetINode( (ParamID)kRefProxyNode );
			if( node != nil )
				nonDrawables.Append( 1, &node );
		}
	}
*/
}

pfGUIDialogMod	*plGUIControlBase::IGetDialogMod( plMaxNode *node )
{
	UInt32		i;


	for( i = 0; i < node->NumAttachedComponents( false ); i++ )
	{
		plComponentBase *comp = node->GetAttachedComponent( i, false );
		if( comp->ClassID() == GUI_DIALOG_COMP_CLASS_ID )
		{
			// Found it!
			pfGUIDialogMod	*dlgMod = ((plGUIDialogComponent *)comp)->GetModifier();
			return dlgMod;
		}
	}

	return nil;
}

hsBool plGUIControlBase::SetupProperties( plMaxNode *pNode, plErrorMsg *pErrMsg )
{
	if( INeedsDynamicText() )
	{
		// If we're going to be using a dynamic text layer, we need to make sure the material
		// is unique for every node we're applied to
		pNode->SetForceMaterialCopy( true );
	}

	return true;
}

hsBool plGUIControlBase::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	// Create a new control
	fControl = IGetNewControl();

	// Add it as a modifier to this node
	node->AddModifier( fControl, IGetUniqueName(node) );

	// Look for any tag IDs
	UInt32 id = plGUITagComponent::GetTagIDOnNode( node );
	if( id > 0 )
		fControl->SetTagID( id );

	// Now add it to our list of converted nodes
	UInt32 i = fTargetNodes.Find( node );
	if( i == fTargetNodes.kMissingIndex )
	{
		fTargetNodes.Append( node );
		fTargetControls.Append( fControl );
	}
	else
	{
		fTargetControls[ i ] = fControl;
	}

	return true;
}

hsBool plGUIControlBase::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	// Error check--make sure we're in the same room as our parent (can get confusing with the wrong
	// parent-child relationships)
	if( !node->GetParentNode()->IsRootNode() )
	{
		plMaxNode *parent = (plMaxNode *)node->GetParentNode();
		if( parent->GetRoomKey() != node->GetRoomKey() )
		{
			pErrMsg->Set( true, "GUI Control Component Error", "The object %s is assigned to a different GUI dialog than its parent. Make sure both this object and its parent belong to the same GUI dialog (this control will be ignored).", node->GetName() ).Show(); 
			pErrMsg->Set( false );
			return false;
		}
	}

	pfGUIDialogMod *dialog = IGetDialogMod( node );
	if( dialog == nil )
	{
		pErrMsg->Set( true, "GUI Control Component Error", "The object %s has a GUI control applied but not a GUI Dialog Component. Apply a GUI Dialog Component to this object.", node->GetName() ).Show(); 
		pErrMsg->Set( false );
		return false;
	}

	// Grab fControl from the modifier list on the node, since fControl isn't valid
	// between PreConvert() and Convert() (it might get called multiple times, once per node applied)
	UInt32 i = fTargetNodes.Find( node );
	if( i == fTargetNodes.kMissingIndex )
	{
		pErrMsg->Set( true, "GUI Control Component Error", "The object %s somehow skipped the GUI control Pre-convert stage. Inform a programmer immediately and seek shelter.", node->GetName() ).Show(); 
		pErrMsg->Set( false );
		return false;
	}
	
	fControl = fTargetControls[ i ];

	dialog->AddControlOnExport( fControl );

	if( IHasProcRollout() )
	{
		// Also common for all controls: process the Procedure rollout--i.e. what kind of control proc do we get?
		switch( fCompPB->GetInt( kRefChoice ) )
		{
			case 0:
				// Console command
				fControl->SetHandler( TRACKED_NEW pfGUIConsoleCmdProc( fCompPB->GetStr( kRefConsoleCmd ) ) );
				break;

			case 1:
				// Inherit from parent dialog - this is a runtime flag, so we don't bother actually setting
				// a handler here, except to ensure it's nil
				fControl->SetHandler( nil );
				fControl->SetFlag( pfGUIControlMod::kInheritProcFromDlg );
				break;

			case 2:
				fControl->SetHandler( TRACKED_NEW pfGUICloseDlgProc() );
				break;

			case 3:
				// Do nothing. Just set a nil proc, but do NOT inherit from the dialog
				fControl->SetHandler( nil );
				fControl->ClearFlag( pfGUIControlMod::kInheritProcFromDlg );
				break;
		}
	}

	if( INeedsDynamicText() )
	{
		// We're a control that dynamically creates text, so look for the first dynamic layer 
		// (and hopefully the ONLY one) and store it on the control
		Mtl *maxMaterial = hsMaterialConverter::Instance().GetBaseMtl( node );
		hsTArray<plExportMaterialData> *mtlArray = hsMaterialConverter::Instance().CreateMaterialArray( maxMaterial, node, 0 );
		
		UInt32 i, j;
		plDynamicTextMap *dynText = nil;
		plLayerInterface *layerIFace = nil;

		for( i = 0; i < mtlArray->GetCount() && dynText == nil; i++ )
		{
			hsGMaterial	*plasmaMat = (*mtlArray)[ 0 ].fMaterial;

			for( j = 0; j < plasmaMat->GetNumLayers(); j++ )
			{
				layerIFace = plasmaMat->GetLayer( j );
				dynText = plDynamicTextMap::ConvertNoRef( layerIFace->GetTexture() );
				if( dynText != nil )
					break;
			}
		}

		if( dynText == nil )
		{
			pErrMsg->Set( true, "GUI Component Error", "The object %s needs a Plasma Dynamic Text Layer in its material. "
				"This control will not function properly until you apply one.", node->GetName() ).Show();
			pErrMsg->Set( false );
		}
		else
			fControl->SetDynTextMap( layerIFace, dynText );

		delete mtlArray;
	}

	if( ICanHaveProxy() )
	{
		// No proxy objects just yet, just options for better hit testing
		if( fCompPB->GetInt( kRefBetterHitTests ) )
			fControl->SetFlag( pfGUIControlMod::kBetterHitTesting );
	}

	return true;
}

pfGUIControlMod	*plGUIControlBase::GrabControlFromObject( INode *node )
{
	UInt32	i;
	plMaxNodeBase	*maxNode = (plMaxNodeBase *)node;


	for( i = 0; i < maxNode->NumAttachedComponents( false ); i++ )
	{
		plComponentBase *comp = maxNode->GetAttachedComponent( i, false );
		pfGUIControlMod	*ctrl = ConvertCompToControl( comp, maxNode );
		if( ctrl != nil )
			return ctrl;
	}

	return nil;	
}

// Given an INode, gives you a pointer to the GUI component if it actually is one, nil otherwise
plGUIControlBase	*plGUIControlBase::GetGUIComp( INode *node )
{
	if( node == nil )
		return nil;

	return GetGUIComp( ( ( plMaxNodeBase *)node )->ConvertToComponent() );
}

plGUIControlBase	*plGUIControlBase::GetGUIComp( plComponentBase *comp )
{
	if( comp == nil )
		return nil;

	if( comp->ClassID() == GUI_UPDOWNPAIR_CLASSID ||
		comp->ClassID() == GUI_BUTTON_CLASSID ||
		comp->ClassID() == GUI_DRAGGABLE_CLASSID ||
		comp->ClassID() == GUI_LISTBOX_CLASSID ||
		comp->ClassID() == GUI_TEXTBOX_CLASSID ||
		comp->ClassID() == GUI_EDITBOX_CLASSID ||
		comp->ClassID() == GUI_KNOBCTRL_CLASSID ||
		comp->ClassID() == GUI_DRAGBAR_CLASSID ||
		comp->ClassID() == GUI_CHECKBOX_CLASSID ||
		comp->ClassID() == GUI_RADIOGROUP_CLASSID ||
		comp->ClassID() == GUI_DYNDISPLAY_CLASSID ||
		comp->ClassID() == GUI_MULTILINE_CLASSID ||
		comp->ClassID() == GUI_PROGRESS_CLASSID ||
		comp->ClassID() == GUI_CLICKMAP_CLASSID )
	{
		return (plGUIControlBase *)comp;
	}

	return nil;
}

pfGUIControlMod *plGUIControlBase::GrabControlMod( INode *node, INode *sceneObjectNode )
{
	if( node == nil )
		return nil;

	plComponentBase *comp = ( ( plMaxNodeBase *)node )->ConvertToComponent();
	return ConvertCompToControl( comp, sceneObjectNode );
}

pfGUIControlMod *plGUIControlBase::ConvertCompToControl( plComponentBase *comp, INode *sceneObjectNode )
{
	plGUIControlBase	*base = GetGUIComp( comp );
	if( base != nil )
	{
		if( sceneObjectNode == nil )
		{
			// Not good, but if you select a component like this, it better only be applied to one object,
			// hence will only have one fTargetControl
			if( base->fTargetControls.GetCount() > 0 )
				return base->fTargetControls[ 0 ];
		}
		else
		{
			UInt32 i = base->fTargetNodes.Find( (plMaxNode *)sceneObjectNode );
			if( i == base->fTargetNodes.kMissingIndex )
				return nil;

			return base->fTargetControls[ i ];
		}
	}

	return nil;
}

const char	*plGUIControlBase::ISetSoundIndex( ParamID checkBoxID, ParamID sndCompID, UInt8 guiCtrlEvent, plMaxNode *maxNode )
{
	if( fCompPB->GetInt( checkBoxID ) )
	{
		plMaxNode *sndNode = (plMaxNode *)fCompPB->GetReferenceTarget( sndCompID );
		if( sndNode != nil )
		{
			plComponentBase *comp = sndNode->ConvertToComponent();
			if( comp != nil )
			{
				int idx = plAudioComp::GetSoundModIdx( comp, maxNode );
				if( idx != -1 )
				{
					fControl->SetSoundIndex( guiCtrlEvent, idx );
					return nil;
				}
				else
					return "The selected sound component could not be found on GUI control %s. Make sure you have a sound component on the same object selected.";
			}
			else
				return "The selected sound node on GUI control %s could not be converted to a component. Make sure you have a sound component selected.";
		}
		else
			return "The GUI control %s has a sound event enabled but no sound component selected. Make sure you have a sound component on the same object selected.";
	}

	return nil;
}


//// ParamBlock for Control Proc Rollout ////////////////////////////////////////////////////////

static ParamBlockDesc2	sGUIControlProcParamTemplate
(
	/// Main def
	plGUIControlBase::kBlkProc, _T("GUIControlProc"), 0, nil, P_AUTO_UI + P_MULTIMAP + P_AUTO_CONSTRUCT, plComponent::kRefComp, 

	1, 
	plGUIControlBase::kRollProc, IDD_COMP_GUIPROCROLLOUT, IDS_COMP_GUIPROCROLLOUT, 0, 0, nil,	

	plGUIControlBase::kRefChoice,	_T("which"), TYPE_INT, 		0, 0,
		p_ui,	plGUIControlBase::kRollProc, TYPE_RADIO, 4, IDC_GUI_CONRADIO, IDC_GUI_INHERITRADIO, IDC_GUI_CLOSERADIO, IDC_GUI_NILRADIO,
		p_default, 1,
		end,

	plGUIControlBase::kRefConsoleCmd,	_T("ConsoleCmd"),		TYPE_STRING, 		0, 0,
		p_ui,	plGUIControlBase::kRollProc, TYPE_EDITBOX, IDC_GUI_CONCMD,
		end,

	end
);


/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIButton Component //////////////////////////////////////////////////////////////////////
//
//	Defines a dialog button to be defined with the GUI manager at runtime. Belongs to exactly 
//	one dialog, defined by parent-child relationship, also at runtime.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

class plGUIButtonProc : public ParamMap2UserDlgProc
{
public:

	void DeleteThis() {}

	BOOL DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
};


static plGUIButtonProc gGUIButtonProc;


// Class that accesses the paramblock below.
class plGUIButtonComponent : public plGUIControlBase
{
protected:

	virtual pfGUIControlMod	*IGetNewControl( void ) { return TRACKED_NEW pfGUIButtonMod; }
	virtual bool			ICanHaveProxy( void ) { return true; }

public:
	plGUIButtonComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);

	hsBool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

	enum
	{
		kRefConCmdRadio,
		kRefPythonRadio,
		kRefConsoleCmd,
		kRefAnimate,
		kRefAnimation,
		kRefMouseOverAnimate,
		kRefMouseOverAnimation,
		kRefMouseDownSound,
		kRefMouseDownSoundComp,
		kRefMouseUpSound,
		kRefMouseUpSoundComp,
		kRefMouseOverSound,
		kRefMouseOverSoundComp,
		kRefMouseOffSound,
		kRefMouseOffSoundComp,
		kRefAnimationNode,
		kRefAnimationNodeType,
		kRefMouseOverAnimationNode,
		kRefMouseOverAnimationNodeType,
		kRefDraggableChild,
		kRefUseDraggableChild,
		kRefNotifyType
	};
};

BOOL plGUIButtonProc::DlgProc( TimeValue t, IParamMap2 *pmap, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{

	switch( msg )
	{
		case WM_INITDIALOG:
			SendMessage( GetDlgItem( hWnd, IDC_COMBO_BUTTON_NOTIFYTYPE ), CB_RESETCONTENT, 0, 0 );
			SendMessage( GetDlgItem( hWnd, IDC_COMBO_BUTTON_NOTIFYTYPE ), CB_ADDSTRING, 0, (LPARAM)"Button Up" );
			SendMessage( GetDlgItem( hWnd, IDC_COMBO_BUTTON_NOTIFYTYPE ), CB_ADDSTRING, 0, (LPARAM)"Button Down" );
			SendMessage( GetDlgItem( hWnd, IDC_COMBO_BUTTON_NOTIFYTYPE ), CB_ADDSTRING, 0, (LPARAM)"Button Down and Up" );
			SendMessage( GetDlgItem( hWnd, IDC_COMBO_BUTTON_NOTIFYTYPE ), CB_SETCURSEL, pmap->GetParamBlock()->GetInt( plGUIButtonComponent::kRefNotifyType ), 0 );
			return true;

		case WM_DESTROY:
			break;

		case WM_COMMAND:
			if( LOWORD( wParam ) == IDC_COMBO_BUTTON_NOTIFYTYPE )
			{
				if( HIWORD( wParam ) == CBN_SELCHANGE )
				{
					int idx = SendDlgItemMessage( hWnd, IDC_COMBO_BUTTON_NOTIFYTYPE, CB_GETCURSEL, 0, 0 );
					pmap->GetParamBlock()->SetValue( plGUIButtonComponent::kRefNotifyType, 0, idx );
				}
			}
			break;
	}
	return false;
}

class plGUIButtonAccessor : public PBAccessor
{
public:
	void Set( PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t )
	{
		if( id == plGUIButtonComponent::kRefAnimation ||
			id == plGUIButtonComponent::kRefMouseOverAnimation ||
			id == plGUIButtonComponent::kRefMouseDownSoundComp ||
			id == plGUIButtonComponent::kRefMouseUpSoundComp ||
			id == plGUIButtonComponent::kRefMouseOverSoundComp ||
			id == plGUIButtonComponent::kRefMouseOffSoundComp ||
			id == plGUIButtonComponent::kRefAnimationNode ||
			id == plGUIButtonComponent::kRefMouseOverAnimationNode )
		{
			plGUIButtonComponent *comp = (plGUIButtonComponent *)owner;
			comp->NotifyDependents( FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED );
		}
	}
};

Class_ID	sBtnClassesToSelect[] = { ANIM_COMP_CID, ANIM_GROUP_COMP_CID, plGUISingleCtrlDlgProc::kEndClassList };
Class_ID	sBtnSndClassesToSelect[] = { GUI_SOUND_COMPONENT_ID, plGUISingleCtrlDlgProc::kEndClassList };

Class_ID	sBtnDragClassesToSelect[] = { GUI_DRAGGABLE_CLASSID, plGUISingleCtrlDlgProc::kEndClassList };

static plGUIButtonAccessor sGUIButtonAccessor;

static plGUISingleCtrlDlgProc sGUIButtonSndAProc( plGUIButtonComponent::kRefMouseDownSoundComp, IDC_GUI_MDOWNSNDCOMP,
											"Select the sound to play when the mouse clicks this button", sBtnSndClassesToSelect );
static plGUISingleCtrlDlgProc sGUIButtonSndBProc( plGUIButtonComponent::kRefMouseUpSoundComp, IDC_GUI_MUPSNDCOMP,
											"Select the sound to play when the mouse lets up on this button", sBtnSndClassesToSelect );
static plGUISingleCtrlDlgProc sGUIButtonSndCProc( plGUIButtonComponent::kRefMouseOverSoundComp, IDC_GUI_MOVERSNDCOMP,
											"Select the sound to play when the mouse moves over this button", sBtnSndClassesToSelect );
static plGUISingleCtrlDlgProc sGUIButtonSndDProc( plGUIButtonComponent::kRefMouseOffSoundComp, IDC_GUI_MOFFSNDCOMP,
											"Select the sound to play when the mouse moves off of this button", sBtnSndClassesToSelect );

static plGUISingleCtrlDlgProc sGUIButtonDragChildProc( plGUIButtonComponent::kRefDraggableChild, IDC_GUI_DRAGCHILD,
											"Select the draggable to use when the mouse is dragged off of this button", sBtnDragClassesToSelect );

static plGUISingleCtrlDlgProc	*sGUIButtonSubProcs[] = { &sGUIButtonSndAProc, &sGUIButtonSndBProc, 
														  &sGUIButtonSndCProc, &sGUIButtonSndDProc, 
														  &sGUIButtonDragChildProc, nil };
static ParamMap2UserDlgProc	*sGUIButtonSubSubProcs[] = { &gGUIButtonProc, nil };

static plGUIMultipleCtrlDlgProc	sGUIButtonSels( sGUIButtonSubProcs, sGUIButtonSubSubProcs );

static plPlasmaAnimSelectDlgProc	sGUIButtonAnimA( plGUIButtonComponent::kRefAnimation, IDC_GUI_COMPSELBTN, 
													plGUIButtonComponent::kRefAnimationNode, plGUIButtonComponent::kRefAnimationNodeType, IDC_GUI_ANIMNODESEL, 
													"Select the animation to play when this button is clicked", &sGUIButtonSels );
static plPlasmaAnimSelectDlgProc	sGUIButtonProc( plGUIButtonComponent::kRefMouseOverAnimation, IDC_GUI_COMPSELBTN2, 
													plGUIButtonComponent::kRefMouseOverAnimationNode, plGUIButtonComponent::kRefMouseOverAnimationNodeType, IDC_GUI_ANIMNODESEL2, 
													"Select the animation to play when the mouse moves over this button", &sGUIButtonAnimA );


#define GUI_SOUND_REF( comp, evt, allCapsEvt )		\
	 	comp##::kRefMouse##evt##Sound,	_T( "mouse##evt##Sound" ), TYPE_BOOL, 0, 0,					\
			p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_M##allCapsEvt##SND,		\
			p_default, FALSE,																		\
			p_enable_ctrls, 1, comp##::kRefMouse##evt##SoundComp,									\
			end,																					\
		comp##::kRefMouse##evt##SoundComp, _T("mouse##evt##SoundComp"),	TYPE_INODE,		0, 0,		\
			p_accessor, &sGUIButtonAccessor,														\
			end

//Max desc stuff necessary below.
CLASS_DESC(plGUIButtonComponent, gGUIButtonDesc, "GUI Button",  "GUIButton", COMP_TYPE_GUI, GUI_BUTTON_CLASSID )

ParamBlockDesc2 gGUIButtonBk
(	// KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUIButton"), 0, &gGUIButtonDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_INCLUDE_PARAMS + P_MULTIMAP, plComponent::kRefComp,

	3,
	plGUIControlBase::kRollMain, IDD_COMP_GUIBUTTON, IDS_COMP_GUIBUTTON, 0, 0, &sGUIButtonProc,
	plGUIControlBase::kRollProc, IDD_COMP_GUIPROCROLLOUT, IDS_COMP_GUIPROCROLLOUT, 0, 0, nil,	
	sGUIProxyParamHeader,

	&sGUIControlProcParamTemplate,

		plGUIButtonComponent::kRefAnimate,	_T( "animate" ), TYPE_BOOL, 0, 0,
			p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_ANIMATE,
			p_default, FALSE,
			p_enable_ctrls, 1, plGUIButtonComponent::kRefAnimation,
			end,

		plGUIButtonComponent::kRefAnimation, _T("animation"),	TYPE_INODE,		0, 0,
			p_prompt, IDS_COMP_GUI_SELECTANIM,
			p_accessor, &sGUIButtonAccessor,
			end,

		plGUIButtonComponent::kRefMouseOverAnimate,	_T( "mouseOverAnimate" ), TYPE_BOOL, 0, 0,
			p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_MOUSEOVERANIM,
			p_default, FALSE,
			p_enable_ctrls, 1, plGUIButtonComponent::kRefMouseOverAnimation,
			end,

		plGUIButtonComponent::kRefMouseOverAnimation, _T("mouseOverAnimation"),	TYPE_INODE,		0, 0,
			p_prompt, IDS_COMP_GUI_SELECTMOUSEOVERANIM,
			p_accessor, &sGUIButtonAccessor,
			end,

		GUI_SOUND_REF( plGUIButtonComponent, Down, DOWN ),
		GUI_SOUND_REF( plGUIButtonComponent, Up, UP ),
		GUI_SOUND_REF( plGUIButtonComponent, Over, OVER ),
		GUI_SOUND_REF( plGUIButtonComponent, Off, OFF ),

	sGUIProxyParamTemplate,

		plGUIButtonComponent::kRefAnimationNode, _T("animationNode"),	TYPE_INODE,		0, 0,
			p_accessor, &sGUIButtonAccessor,
			end,

		plGUIButtonComponent::kRefAnimationNodeType, _T("animationNodeType"),	TYPE_INT,		0, 0,
			p_default, plAnimObjInterface::kUseOwnerNode,
			end,

		plGUIButtonComponent::kRefMouseOverAnimationNode, _T("moAnimationNode"),	TYPE_INODE,		0, 0,
			p_accessor, &sGUIButtonAccessor,
			end,

		plGUIButtonComponent::kRefMouseOverAnimationNodeType, _T("moAnimationNodeType"),	TYPE_INT,		0, 0,
			p_default, plAnimObjInterface::kUseOwnerNode,
			end,

		plGUIButtonComponent::kRefUseDraggableChild,	_T( "useDragChild" ), TYPE_BOOL, 0, 0,
			p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_USEDRAGCHILD,
			p_default, FALSE,
			p_enable_ctrls, 1, plGUIButtonComponent::kRefDraggableChild,
			end,

		plGUIButtonComponent::kRefDraggableChild,	_T( "dragChild" ), TYPE_INODE, 0, 0,
			p_accessor, &sGUIButtonAccessor,
			end,

		plGUIButtonComponent::kRefNotifyType,	_T("notifyType"),		TYPE_INT, 		0, 0,
			p_default, pfGUIButtonMod::kNotifyOnUp,
			end,
	end
);

plGUIButtonComponent::plGUIButtonComponent()
{
	fClassDesc = &gGUIButtonDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plGUIButtonComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	if( fCompPB->GetInt( kRefAnimate ) )
	{
		plAnimObjInterface *iface = plAnimComponentBase::GetAnimInterface( fCompPB->GetINode( kRefAnimation ) );
		if( iface != nil && iface->MightRequireSeparateMaterial() )
		{
			INode *restrict = ( fCompPB->GetInt( kRefAnimationNodeType ) == plAnimObjInterface::kUseParamBlockNode ) 
									? fCompPB->GetINode( kRefAnimationNode )
									: (INode *)node;

			if( restrict != nil )
			{
				node->SetForceMaterialCopy( true );
			}
		}
	}

	if( fCompPB->GetInt( kRefMouseOverAnimate ) )
	{
		plAnimObjInterface *iface = plAnimComponentBase::GetAnimInterface( fCompPB->GetINode( kRefMouseOverAnimation ) );
		if( iface != nil && iface->MightRequireSeparateMaterial() )
		{
			INode *restrict = ( fCompPB->GetInt( kRefMouseOverAnimationNodeType ) == plAnimObjInterface::kUseParamBlockNode ) 
									? fCompPB->GetINode( kRefMouseOverAnimationNode )
									: (INode *)node;

			if( restrict != nil )
			{
				node->SetForceMaterialCopy( true );
			}
		}
	}

	return plGUIControlBase::SetupProperties( node, pErrMsg );
}

hsBool plGUIButtonComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	return plGUIControlBase::PreConvert( node, pErrMsg );
}

hsBool plGUIButtonComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	if( !plGUIControlBase::Convert( node, pErrMsg ) )
		return false;

	pfGUIButtonMod *button = (pfGUIButtonMod *)fControl;

	// set the notify type
	button->SetNotifyType(fCompPB->GetInt( kRefNotifyType ));

	if( fCompPB->GetInt( kRefAnimate ) )
	{
		plAnimObjInterface *iface = plAnimComponentBase::GetAnimInterface( fCompPB->GetINode( kRefAnimation ) );
		if( iface != nil )
		{
			INode *restrict = ( fCompPB->GetInt( kRefAnimationNodeType ) == plAnimObjInterface::kUseParamBlockNode ) 
									? fCompPB->GetINode( kRefAnimationNode )
									: (INode *)node;


			hsTArray<plKey> keys;
			if( iface->GetKeyList( restrict, keys ) && keys.GetCount() > 0 )
				button->SetAnimationKeys( keys, iface->GetIfaceSegmentName( false ) );
		}
	}

	if( fCompPB->GetInt( kRefMouseOverAnimate ) )
	{
		plAnimObjInterface *iface = plAnimComponentBase::GetAnimInterface( fCompPB->GetINode( kRefMouseOverAnimation ) );
		if( iface != nil )
		{
			INode *restrict = ( fCompPB->GetInt( kRefMouseOverAnimationNodeType ) == plAnimObjInterface::kUseParamBlockNode ) 
									? fCompPB->GetINode( kRefMouseOverAnimationNode )
									: (INode *)node;


			hsTArray<plKey> keys;
			if( iface->GetKeyList( restrict, keys ) && keys.GetCount() > 0 )
				button->SetMouseOverAnimKeys( keys, iface->GetIfaceSegmentName( false ) );
		}
	}

	// Do sound stuff
	const char *errMsg1 = ISetSoundIndex( kRefMouseDownSound, kRefMouseDownSoundComp, pfGUIButtonMod::kMouseDown, node );
	const char *errMsg2 = ISetSoundIndex( kRefMouseUpSound, kRefMouseUpSoundComp, pfGUIButtonMod::kMouseUp, node );
	const char *errMsg3 = ISetSoundIndex( kRefMouseOverSound, kRefMouseOverSoundComp, pfGUIButtonMod::kMouseOver, node );
	const char *errMsg4 = ISetSoundIndex( kRefMouseOffSound, kRefMouseOffSoundComp, pfGUIButtonMod::kMouseOff, node );

	const char *errMsg = ( errMsg1 != nil ) ? errMsg1 : ( errMsg2 != nil ) ? errMsg2 : ( errMsg3 != nil ) ? errMsg3 : errMsg4;
	if( errMsg != nil )
	{
		pErrMsg->Set( true, "GUI Sound Event Error", errMsg, node->GetName() ).Show();
		pErrMsg->Set( false );
	}

	if( fCompPB->GetInt( kRefUseDraggableChild ) )
	{
		pfGUIDraggableMod *dragChild = pfGUIDraggableMod::ConvertNoRef( GrabControlMod( fCompPB->GetINode( kRefDraggableChild ) ) );
		if( dragChild != nil )
		{
			hsgResMgr::ResMgr()->AddViaNotify( dragChild->GetKey(), 
								new plGenRefMsg( button->GetKey(), plRefMsg::kOnCreate, -1, pfGUIButtonMod::kRefDraggable ), plRefFlags::kActiveRef );
		}
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUICheckBox Component //////////////////////////////////////////////////////////////////////
//
//	Defines a dialog button to be defined with the GUI manager at runtime. Belongs to exactly 
//	one dialog, defined by parent-child relationship, also at runtime.
//
/////////////////////////////////////////////////////////////////////////////////////////////////


// Class that accesses the paramblock below.
class plGUICheckBoxComponent : public plGUIControlBase
{
protected:

	virtual pfGUIControlMod	*IGetNewControl( void ) { return TRACKED_NEW pfGUICheckBoxCtrl; }
	virtual bool			ICanHaveProxy( void ) { return true; }

public:
	plGUICheckBoxComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);

	hsBool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

	enum
	{
		kRefConCmdRadio,
		kRefPythonRadio,
		kRefConsoleCmd,
		kRefAnimate,
		kRefAnimation,
		kRefAnimationNode,
		kRefAnimationNodeType,
		kRefMouseDownSound,
		kRefMouseDownSoundComp,
		kRefMouseUpSound,
		kRefMouseUpSoundComp,
		kRefMouseOverSound,
		kRefMouseOverSoundComp,
		kRefMouseOffSound,
		kRefMouseOffSoundComp,
	};
};

class plGUICheckBoxAccessor : public PBAccessor
{
public:
	void Set( PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t )
	{
		if( id == plGUICheckBoxComponent::kRefAnimation ||
			id == plGUICheckBoxComponent::kRefMouseDownSoundComp ||
			id == plGUICheckBoxComponent::kRefMouseUpSoundComp ||
			id == plGUICheckBoxComponent::kRefMouseOverSoundComp ||
			id == plGUICheckBoxComponent::kRefMouseOffSoundComp )
		{
			plGUICheckBoxComponent *comp = (plGUICheckBoxComponent *)owner;
			comp->NotifyDependents( FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED );
		}
	}
};
static plGUICheckBoxAccessor	sGUICheckBoxAccessor;

static plGUISingleCtrlDlgProc sGUICheckSndAProc( plGUICheckBoxComponent::kRefMouseDownSoundComp, IDC_GUI_MDOWNSNDCOMP,
											"Select the sound to play when the mouse clicks this button", sBtnSndClassesToSelect );
static plGUISingleCtrlDlgProc sGUICheckSndBProc( plGUICheckBoxComponent::kRefMouseUpSoundComp, IDC_GUI_MUPSNDCOMP,
											"Select the sound to play when the mouse lets up on this button", sBtnSndClassesToSelect );
static plGUISingleCtrlDlgProc sGUICheckSndCProc( plGUICheckBoxComponent::kRefMouseOverSoundComp, IDC_GUI_MOVERSNDCOMP,
											"Select the sound to play when the mouse moves over this button", sBtnSndClassesToSelect );
static plGUISingleCtrlDlgProc sGUICheckSndDProc( plGUICheckBoxComponent::kRefMouseOffSoundComp, IDC_GUI_MOFFSNDCOMP,
											"Select the sound to play when the mouse moves off of this button", sBtnSndClassesToSelect );

static plGUISingleCtrlDlgProc	*sGUICheckSubProcs[] = { &sGUICheckSndAProc, &sGUICheckSndBProc, 
														  &sGUICheckSndCProc, &sGUICheckSndDProc, nil };

static plGUIMultipleCtrlDlgProc	sGUICheckSels( sGUICheckSubProcs );

static plPlasmaAnimSelectDlgProc	sGUICheckBoxProc( plGUICheckBoxComponent::kRefAnimation, IDC_GUI_COMPSELBTN, 
													plGUICheckBoxComponent::kRefAnimationNode, plGUICheckBoxComponent::kRefAnimationNodeType, IDC_GUI_ANIMNODESEL, 
													"Select the animation to play when this check box is clicked", &sGUICheckSels );

//Max desc stuff necessary below.
CLASS_DESC(plGUICheckBoxComponent, gGUICheckBoxDesc, "GUI CheckBox",  "GUICheckBox", COMP_TYPE_GUI, GUI_CHECKBOX_CLASSID )

ParamBlockDesc2 gGUICheckBoxBk
(	// KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUICheckBox"), 0, &gGUICheckBoxDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

	3,
	plGUIControlBase::kRollMain, IDD_COMP_GUIBUTTON, IDS_COMP_GUICHECKBOX, 0, 0, &sGUICheckBoxProc,
	plGUIControlBase::kRollProc, IDD_COMP_GUIPROCROLLOUT, IDS_COMP_GUIPROCROLLOUT, 0, 0, nil,	
	
	sGUIProxyParamHeader,

	&sGUIControlProcParamTemplate,

		plGUICheckBoxComponent::kRefAnimate,	_T( "animate" ), TYPE_BOOL, 0, 0,
			p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_ANIMATE,
			p_default, FALSE,
			p_enable_ctrls, 1, plGUIButtonComponent::kRefAnimation,
			end,

		plGUICheckBoxComponent::kRefAnimation, _T("animation"),	TYPE_INODE,		0, 0,
			p_prompt, IDS_COMP_GUI_SELECTANIM,
			p_accessor, &sGUICheckBoxAccessor,
			end,

	sGUIProxyParamTemplate,

		plGUICheckBoxComponent::kRefAnimationNode, _T("animationNode"),	TYPE_INODE,		0, 0,
			p_accessor, &sGUIButtonAccessor,
			end,

		plGUICheckBoxComponent::kRefAnimationNodeType, _T("animationNodeType"),	TYPE_INT,		0, 0,
			p_default, plAnimObjInterface::kUseOwnerNode,
			end,

		GUI_SOUND_REF( plGUICheckBoxComponent, Down, DOWN ),
		GUI_SOUND_REF( plGUICheckBoxComponent, Up, UP ),
		GUI_SOUND_REF( plGUICheckBoxComponent, Over, OVER ),
		GUI_SOUND_REF( plGUICheckBoxComponent, Off, OFF ),

	end
);

plGUICheckBoxComponent::plGUICheckBoxComponent()
{
	fClassDesc = &gGUICheckBoxDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plGUICheckBoxComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	if( fCompPB->GetInt( kRefAnimate ) )
	{
		plAnimObjInterface *iface = plAnimComponentBase::GetAnimInterface( fCompPB->GetINode( kRefAnimation ) );
		if( iface != nil && iface->MightRequireSeparateMaterial() )
		{
			INode *restrict = ( fCompPB->GetInt( kRefAnimationNodeType ) == plAnimObjInterface::kUseParamBlockNode ) 
									? fCompPB->GetINode( kRefAnimationNode )
									: (INode *)node;

			if( restrict != nil )
			{
				node->SetForceMaterialCopy( true );
			}
		}
	}

	return plGUIControlBase::SetupProperties( node, pErrMsg );
}

hsBool plGUICheckBoxComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	return plGUIControlBase::PreConvert( node, pErrMsg );
}

hsBool plGUICheckBoxComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	if( !plGUIControlBase::Convert( node, pErrMsg ) )
		return false;

	pfGUICheckBoxCtrl *button = (pfGUICheckBoxCtrl *)fControl;
	
	if( fCompPB->GetInt( kRefAnimate ) )
	{
		plAnimObjInterface *iface = plAnimComponentBase::GetAnimInterface( fCompPB->GetINode( kRefAnimation ) );
		if( iface != nil )
		{
			INode *restrict = ( fCompPB->GetInt( kRefAnimationNodeType ) == plAnimObjInterface::kUseParamBlockNode ) 
									? fCompPB->GetINode( kRefAnimationNode )
									: (INode *)node;


			hsTArray<plKey> keys;
			if( iface->GetKeyList( restrict, keys ) && keys.GetCount() > 0 )
				button->SetAnimationKeys( keys, iface->GetIfaceSegmentName( false ) );
		}
	}

	// Do sound stuff
	const char *errMsg1 = ISetSoundIndex( kRefMouseDownSound, kRefMouseDownSoundComp, pfGUICheckBoxCtrl::kMouseDown, node );
	const char *errMsg2 = ISetSoundIndex( kRefMouseUpSound, kRefMouseUpSoundComp, pfGUICheckBoxCtrl::kMouseUp, node );
	const char *errMsg3 = ISetSoundIndex( kRefMouseOverSound, kRefMouseOverSoundComp, pfGUICheckBoxCtrl::kMouseOver, node );
	const char *errMsg4 = ISetSoundIndex( kRefMouseOffSound, kRefMouseOffSoundComp, pfGUICheckBoxCtrl::kMouseOff, node );

	const char *errMsg = ( errMsg1 != nil ) ? errMsg1 : ( errMsg2 != nil ) ? errMsg2 : ( errMsg3 != nil ) ? errMsg3 : errMsg4;
	if( errMsg != nil )
	{
		pErrMsg->Set( true, "GUI Sound Event Error", errMsg, node->GetName() ).Show();
		pErrMsg->Set( false );
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIDraggable Component ///////////////////////////////////////////////////////////////////
//
//	GUI element that can be dragged anywhere in the 2D viewing plane.
//
/////////////////////////////////////////////////////////////////////////////////////////////////


// Class that accesses the paramblock below.
class plGUIDraggableComponent : public plGUIControlBase
{
protected:

	virtual pfGUIControlMod	*IGetNewControl( void ) { return TRACKED_NEW pfGUIDraggableMod; }
	virtual bool			ICanHaveProxy( void ) { return true; }

public:
	plGUIDraggableComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);

	hsBool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

	enum
	{
		kRefReportDragging,
		kRefHideCursor,
		kRefAlwaysSnap
	};
};

//Max desc stuff necessary below.
CLASS_DESC(plGUIDraggableComponent, gGUIDraggableDesc, "GUI Draggable",  "GUIDraggable", COMP_TYPE_GUI, GUI_DRAGGABLE_CLASSID )

ParamBlockDesc2 gGUIDraggableBk
(	// KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUIDraggable"), 0, &gGUIDraggableDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

	3,
	plGUIControlBase::kRollMain, IDD_COMP_GUIDRAGGABLE, IDS_COMP_GUIDRAGGABLE, 0, 0, NULL,
	plGUIControlBase::kRollProc, IDD_COMP_GUIPROCROLLOUT, IDS_COMP_GUIPROCROLLOUT, 0, 0, nil,	
	
	sGUIProxyParamHeader,
	
	&sGUIControlProcParamTemplate,

	sGUIProxyParamTemplate,

	plGUIDraggableComponent::kRefReportDragging, _T("reportWhileDragging"), TYPE_BOOL, 0, 0,
		p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_REPORTDRAG,
		p_default, FALSE,
		end,

	plGUIDraggableComponent::kRefHideCursor, _T("hideCursorWhileDragging"), TYPE_BOOL, 0, 0,
		p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_HIDECURSOR,
		p_default, FALSE,
		end,

	plGUIDraggableComponent::kRefAlwaysSnap, _T("alwaysSnapBackToStart"), TYPE_BOOL, 0, 0,
		p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_SNAPSTART,
		p_default, FALSE,
		end,

	end
);

plGUIDraggableComponent::plGUIDraggableComponent()
{
	fClassDesc = &gGUIDraggableDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plGUIDraggableComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	node->SetForceLocal( true );
	return plGUIControlBase::SetupProperties( node, pErrMsg );
}

hsBool plGUIDraggableComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	return plGUIControlBase::PreConvert( node, pErrMsg );
}

hsBool plGUIDraggableComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	if( !plGUIControlBase::Convert( node, pErrMsg ) )
		return false;

	pfGUIDraggableMod *ctrl = (pfGUIDraggableMod *)fControl;
	
	if( fCompPB->GetInt( kRefReportDragging ) )
		ctrl->SetFlag( pfGUIDraggableMod::kReportDragging );

	if( fCompPB->GetInt( kRefHideCursor ) )
		ctrl->SetFlag( pfGUIDraggableMod::kHideCursorWhileDragging );

	if( fCompPB->GetInt( kRefAlwaysSnap ) )
		ctrl->SetFlag( pfGUIDraggableMod::kAlwaysSnapBackToStart );

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIKnobCtrl Component ///////////////////////////////////////////////////////////////////
//
//	GUI element that can be dragged anywhere in the 2D viewing plane.
//
/////////////////////////////////////////////////////////////////////////////////////////////////


// Class that accesses the paramblock below.
class plGUIKnobCtrlComponent : public plGUIControlBase
{
protected:

	virtual pfGUIControlMod	*IGetNewControl( void ) { return TRACKED_NEW pfGUIKnobCtrl; }
	virtual bool			ICanHaveProxy( void ) { return true; }

	hsBool	IGrabAnimationRange( plMaxNode *node, plErrorMsg *pErrMsg, hsMatrix44 &startL2W, hsMatrix44 &endL2W );

public:
	plGUIKnobCtrlComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);

	hsBool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

	enum
	{
		kRefMinValue,
		kRefMaxValue,
		kRefStep,
		kReverseValues,
		kRefOrientation,
		kRefMouseMapping,
		kRefTriggerOnMouseUp,
		kRefAnimation,
		kRefAnimationNode,
		kRefAnimationNodeType
	};
};

static plPlasmaAnimSelectDlgProc	sGUIKnobCtrlProc( plGUIKnobCtrlComponent::kRefAnimation, IDC_GUI_COMPSELBTN, 
													plGUIKnobCtrlComponent::kRefAnimationNode, plGUIKnobCtrlComponent::kRefAnimationNodeType, IDC_GUI_ANIMNODESEL, 
													"Select the animation to use when displaying this knob control", nil );

//Max desc stuff necessary below.
CLASS_DESC(plGUIKnobCtrlComponent, gGUIKnobCtrlDesc, "GUI Knob Control",  "GUIKnobCtrl", COMP_TYPE_GUI, GUI_KNOBCTRL_CLASSID )

ParamBlockDesc2 gGUIKnobCtrlBk
(	// KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUIKnobCtrl"), 0, &gGUIKnobCtrlDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

	3,
	plGUIControlBase::kRollMain, IDD_COMP_GUIKNOB, IDS_COMP_GUIKNOB, 0, 0, &sGUIKnobCtrlProc,
	plGUIControlBase::kRollProc, IDD_COMP_GUIPROCROLLOUT, IDS_COMP_GUIPROCROLLOUT, 0, 0, nil,	
	
	sGUIProxyParamHeader,
	
	&sGUIControlProcParamTemplate,
	
	plGUIKnobCtrlComponent::kRefMinValue, _T("minValue"),	TYPE_FLOAT,	0, 0,	
		p_default, 0.0f,
		p_ui, plGUIControlBase::kRollMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT, IDC_GUI_LOWER, IDC_GUI_LOWER_SPIN, SPIN_AUTOSCALE,
		end,

	plGUIKnobCtrlComponent::kRefMaxValue, _T("maxValue"),	TYPE_FLOAT,	0, 0,	
		p_default, 10.0f,
		p_range, -10000.f, 10000.f,			// WHY do we even need to specify this?
		p_ui, plGUIControlBase::kRollMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT, IDC_GUI_UPPER, IDC_GUI_UPPER_SPIN, SPIN_AUTOSCALE,
		end,

	plGUIKnobCtrlComponent::kRefStep, _T("step"),	TYPE_FLOAT,	0, 0,	
		p_default, 1.0f,
		p_ui, plGUIControlBase::kRollMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT, IDC_GUI_STEP, IDC_GUI_STEP_SPIN, SPIN_AUTOSCALE,
		end,

	plGUIKnobCtrlComponent::kReverseValues, _T("reverseValues"), TYPE_BOOL, 0, 0,
		p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_REVERSE,
		p_default, FALSE,
		end,

	plGUIKnobCtrlComponent::kRefOrientation,	_T("orientation"), TYPE_INT, 		0, 0,
		p_ui,	plGUIControlBase::kRollMain, TYPE_RADIO, 2, IDC_ORIENTATION_RADIO, IDC_ORIENTATION_RADIO2,
		p_default, 0,
		end,
	
	plGUIKnobCtrlComponent::kRefMouseMapping, _T("mouseMapping"), TYPE_INT, 0, 0,
		p_ui, plGUIControlBase::kRollMain, TYPE_RADIO, 3, IDC_GUI_MOUSEMAPREL, IDC_GUI_MOUSEMAPANIM, IDC_GUI_MOUSEMAPSCRN,
		p_default, 0,
		end,

	plGUIKnobCtrlComponent::kRefTriggerOnMouseUp, _T("triggerOnMouseUp"), TYPE_BOOL, 0, 0,
		p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_TRIGGERONUP,
		p_default, FALSE,
		end,
		
	sGUIProxyParamTemplate,

	plGUIKnobCtrlComponent::kRefAnimation, _T("animation"),	TYPE_INODE,		0, 0,
		p_prompt, IDS_COMP_GUI_SELECTANIM,
		end,

	plGUIKnobCtrlComponent::kRefAnimationNode, _T("animationNode"),	TYPE_INODE,		0, 0,
		end,

	plGUIKnobCtrlComponent::kRefAnimationNodeType, _T("animationNodeType"),	TYPE_INT,		0, 0,
		p_default, plAnimObjInterface::kUseOwnerNode,
		end,

	end
);

plGUIKnobCtrlComponent::plGUIKnobCtrlComponent()
{
	fClassDesc = &gGUIKnobCtrlDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool	plGUIKnobCtrlComponent::IGrabAnimationRange( plMaxNode *node, plErrorMsg *pErrMsg, hsMatrix44 &startL2W, hsMatrix44 &endL2W )
{
	hsBool	result = false;


	// Get the affine parts and the TM Controller
	plSceneObject *obj = node->GetSceneObject();
	hsAffineParts * parts = TRACKED_NEW hsAffineParts;
	plController* tmc = hsControlConverter::Instance().ConvertTMAnim(obj, node, parts);

	if (tmc)
	{
		plMatrixControllerChannel *channel = TRACKED_NEW plMatrixControllerChannel(tmc, parts);

		hsScalar length = tmc->GetLength();

		startL2W = channel->Value( 0.f );
		endL2W = channel->Value( length );

		delete channel;
		result = true;
	}

	delete parts;	// We copy this over, so no need to keep it around
	return result;
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plGUIKnobCtrlComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	node->SetForceLocal( true );

	plAnimObjInterface *iface = plAnimComponentBase::GetAnimInterface( fCompPB->GetINode( kRefAnimation ) );
	if( iface != nil && iface->MightRequireSeparateMaterial() )
	{
		INode *restrict = ( fCompPB->GetInt( kRefAnimationNodeType ) == plAnimObjInterface::kUseParamBlockNode ) 
								? fCompPB->GetINode( kRefAnimationNode )
								: (INode *)node;

		if( restrict != nil )
		{
			node->SetForceMaterialCopy( true );
		}
	}

	return plGUIControlBase::SetupProperties( node, pErrMsg );
}

hsBool plGUIKnobCtrlComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	return plGUIControlBase::PreConvert( node, pErrMsg );
}

// For hackery below (see warning below)
#include "../plAvatar/plAGMasterMod.h"

hsBool plGUIKnobCtrlComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	if( !plGUIControlBase::Convert( node, pErrMsg ) )
		return false;

	pfGUIKnobCtrl *ctrl = (pfGUIKnobCtrl *)fControl;
	
	ctrl->SetRange( fCompPB->GetFloat( kRefMinValue ), fCompPB->GetFloat( kRefMaxValue ) );
	ctrl->SetStep( fCompPB->GetFloat( kRefStep ) );

	if( fCompPB->GetInt( kReverseValues ) )
		ctrl->SetFlag( pfGUIKnobCtrl::kReverseValues );

	// Get the animation to use
	plAnimObjInterface *iface = plAnimComponentBase::GetAnimInterface( fCompPB->GetINode( kRefAnimation ) );
	if( iface != nil )
	{
		INode *restrict = ( fCompPB->GetInt( kRefAnimationNodeType ) == plAnimObjInterface::kUseParamBlockNode ) 
								? fCompPB->GetINode( kRefAnimationNode )
								: (INode *)node;


		hsTArray<plKey> keys;
		if( iface->GetKeyList( restrict, keys ) && keys.GetCount() > 0 )
			ctrl->SetAnimationKeys( keys, iface->GetIfaceSegmentName( false ) );
	}
	else
	{
		// HACKERY WARNING: Old knobs assumed the animation was just the same one applied to our node,
		// so to avoid breaking old formats, if we can't grab an animObjInterface, we just grab the key
		// of the master mod of our node, like we would've before
		plAGMasterMod	*master = node->GetAGMasterMod();
		hsTArray<plKey> keys;
		keys.Append( master->GetKey() );
		ctrl->SetAnimationKeys( keys, ENTIRE_ANIMATION_NAME );
	}

	if( fCompPB->GetInt( kRefOrientation ) == 1 )
		ctrl->SetFlag( pfGUIKnobCtrl::kLeftRightOrientation );

	hsMatrix44 startL2W, endL2W;
	switch( fCompPB->GetInt( kRefMouseMapping ) )
	{
		case 0:		// Default, normal (old) relative behavior
			break;
		case 1:		// Map to the range of animation positions
			if( !IGrabAnimationRange( node, pErrMsg, startL2W, endL2W ) )
			{
				pErrMsg->Set( true, "Unable to grab animation range for the GUI Knob Control %s. The Map-To-Screen-Range feature will be disabled.", node->GetName() ).Show();
				pErrMsg->Set( false );
			}
			else
			{
				hsPoint3 startPos = startL2W.GetTranslate();
				hsPoint3 endPos = endL2W.GetTranslate();

				ctrl->SetScreenRange( startPos, endPos );
				ctrl->SetFlag( pfGUIKnobCtrl::kMapToAnimationRange );
			}
			break;
		case 2:		// Map to a range on the screen
			ctrl->SetFlag( pfGUIKnobCtrl::kMapToScreenRange );
			break;
	}

	if( fCompPB->GetInt( kRefTriggerOnMouseUp ) )
		ctrl->SetFlag( pfGUIKnobCtrl::kTriggerOnlyOnMouseUp );

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIListBox Component /////////////////////////////////////////////////////////////////////
//
//	GUI element that can be dragged anywhere in the 2D viewing plane.
//
/////////////////////////////////////////////////////////////////////////////////////////////////


// Class that accesses the paramblock below.
class plGUIListBoxComponent : public plGUIControlBase
{
protected:

	virtual pfGUIControlMod	*IGetNewControl( void ) { return TRACKED_NEW pfGUIListBoxMod; }
	virtual bool			INeedsDynamicText( void ) { return true; }

public:
	plGUIListBoxComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);

	hsBool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

	enum
	{
		kRefUseScroll,
		kRefScrollCtrl,
		kRefSingleSelect,
		kRefXparentBgnd,
		kRefDragDropSource,
		kRefDisableKeys,
		kRefAllow2DElementGrid,
		kRefScrollLeftToRight,
		kRefScaleWithRes,
		kRefPassClicksThrough,
		kRefEnableTreeBehavior,
		kRefSkin,
		kRefHandsOffMultiSelect
	};
};

// When one of our parameters that is a ref changes, send out the component ref
// changed message.  Normally, messages from component refs are ignored since
// they pass along all the messages of the ref, which generates a lot of false
// converts.
class plGUIListBoxAccessor : public PBAccessor
{
public:
	void Set( PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t )
	{
		if( id == plGUIListBoxComponent::kRefScrollCtrl )
		{
			plGUIListBoxComponent *comp = (plGUIListBoxComponent *)owner;
			comp->NotifyDependents( FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED );
		}
	}
};

Class_ID	sScrollingClassesToSelect[] = { GUI_UPDOWNPAIR_CLASSID, GUI_KNOBCTRL_CLASSID, plGUISingleCtrlDlgProc::kEndClassList };

static plGUIListBoxAccessor sGUIListBoxAccessor;
static plGUISingleCtrlDlgProc sGUIListBoxProc( plGUIListBoxComponent::kRefScrollCtrl, IDC_GUI_COMPSELBTN,
											"Select the control to use for scrolling this list box", sScrollingClassesToSelect );

static plGUISingleCtrlDlgProc sGUILBSkinSelectProc( plGUIListBoxComponent::kRefSkin, IDC_GUI_SKIN,
											"Select the skin to use for this list box", sSkinClassesToSelect,
											&sGUIListBoxProc );

//Max desc stuff necessary below.
CLASS_DESC(plGUIListBoxComponent, gGUIListBoxDesc, "GUI List Box",  "GUIListBox", COMP_TYPE_GUI, GUI_LISTBOX_CLASSID )

ParamBlockDesc2 gGUIListBoxBk
(	// KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUIListBox"), 0, &gGUIListBoxDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

	2,
	plGUIControlBase::kRollMain, IDD_COMP_GUILISTBOX, IDS_COMP_GUILISTBOX, 0, 0, &sGUILBSkinSelectProc,
	plGUIControlBase::kRollProc, IDD_COMP_GUIPROCROLLOUT, IDS_COMP_GUIPROCROLLOUT, 0, 0, nil,	
	
	&sGUIControlProcParamTemplate,	

	plGUIListBoxComponent::kRefUseScroll,	_T( "enableScrolling" ), TYPE_BOOL, 0, 0,
		p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_SCROLLCTRL,
		p_default, FALSE,
		p_enable_ctrls, 1, plGUIListBoxComponent::kRefScrollCtrl,
		end,

	plGUIListBoxComponent::kRefScrollCtrl, _T("scrollControl"),	TYPE_INODE,		0, 0,
		p_prompt, IDS_COMP_GUI_SELECTSCROLL,
		p_accessor, &sGUIListBoxAccessor,
		end,

	plGUIListBoxComponent::kRefSingleSelect,	_T( "singleSelect" ), TYPE_BOOL, 0, 0,
		p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_SINGLESEL,
		p_default, FALSE,
		end,

	plGUIListBoxComponent::kRefXparentBgnd,	_T( "xparentBgnd" ), TYPE_BOOL, 0, 0,
		p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_XPARENT,
		p_default, FALSE,
		end,
	
	plGUIListBoxComponent::kRefDragDropSource,	_T( "dragDropCapable" ), TYPE_BOOL, 0, 0,
		p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_DRAGDROPSRC,
		p_default, FALSE,
		end,

	plGUIListBoxComponent::kRefDisableKeys,	_T( "disableKeys" ), TYPE_BOOL, 0, 0,
		p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_DISABLEKEYS,
		p_default, FALSE,
		end,
	
	plGUIListBoxComponent::kRefAllow2DElementGrid,	_T( "allow2DElementGrid" ), TYPE_BOOL, 0, 0,
		p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_ALLOWMULTIROW,
		p_default, FALSE,
		end,
	
	plGUIListBoxComponent::kRefScrollLeftToRight,	_T( "scrollLeftToRight" ), TYPE_BOOL, 0, 0,
		p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_SCROLLL2R,
		p_default, FALSE,
		end,

	plGUIListBoxComponent::kRefScaleWithRes,	_T( "scaleWithRes" ), TYPE_BOOL, 0, 0,
		p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_SCALERES,
		p_default, FALSE,
		end,
				
	plGUIListBoxComponent::kRefPassClicksThrough,	_T( "passClicksThru" ), TYPE_BOOL, 0, 0,
		p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_PASSTHRU,
		p_default, FALSE,
		end,
		
	plGUIListBoxComponent::kRefEnableTreeBehavior,	_T( "makeLikeATree" ), TYPE_BOOL, 0, 0,
		p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_ENABLETREE,
		p_default, FALSE,
		end,
				
	plGUIListBoxComponent::kRefSkin, _T("skin"),	TYPE_INODE,		0, 0,
		end,

	plGUIListBoxComponent::kRefHandsOffMultiSelect,	_T( "handsOffMultiSelect" ), TYPE_BOOL, 0, 0,
		p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_HANDSOFF,
		p_default, FALSE,
		end,
		
	end
);

plGUIListBoxComponent::plGUIListBoxComponent()
{
	fClassDesc = &gGUIListBoxDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plGUIListBoxComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	return plGUIControlBase::SetupProperties( node, pErrMsg );
}

hsBool plGUIListBoxComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	return plGUIControlBase::PreConvert( node, pErrMsg );
}

hsBool plGUIListBoxComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	if( !plGUIControlBase::Convert( node, pErrMsg ) )
		return false;

	pfGUIListBoxMod *ctrl = (pfGUIListBoxMod *)fControl;
	
	if( fCompPB->GetInt( kRefUseScroll ) )
	{
		// Get the scrolling control to use
		pfGUIValueCtrl *scroll = pfGUIValueCtrl::ConvertNoRef( GrabControlMod( fCompPB->GetINode( kRefScrollCtrl ) ) );
		if( scroll != nil )
		{
			hsgResMgr::ResMgr()->AddViaNotify( scroll->GetKey(), TRACKED_NEW plGenRefMsg( ctrl->GetKey(), 
										plRefMsg::kOnCreate, -1, pfGUIListBoxMod::kRefScrollCtrl ), plRefFlags::kActiveRef );
		}
	}

	if( fCompPB->GetInt( kRefSingleSelect ) )
		ctrl->SetSingleSelect( true );

	if( fCompPB->GetInt( kRefXparentBgnd ) )
		ctrl->SetFlag( pfGUIListBoxMod::kXparentBgnd );

	if( fCompPB->GetInt( kRefDragDropSource ) )
		ctrl->SetFlag( pfGUIListBoxMod::kDragAndDropCapable );

	if( fCompPB->GetInt( kRefDisableKeys ) )
		ctrl->SetFlag( pfGUIListBoxMod::kDisableKeyActions );

	if( fCompPB->GetInt( kRefAllow2DElementGrid ) )
		ctrl->SetFlag( pfGUIListBoxMod::kAllowMultipleElementsPerRow );

	if( fCompPB->GetInt( kRefScrollLeftToRight ) )
		ctrl->SetFlag( pfGUIListBoxMod::kScrollLeftToRight );
	
	if( fCompPB->GetInt( kRefScaleWithRes ) )
		ctrl->SetFlag( pfGUIControlMod::kScaleTextWithResolution );

	if( fCompPB->GetInt( kRefPassClicksThrough ) )
		ctrl->SetFlag( pfGUIListBoxMod::kAllowMousePassThrough );

	if( fCompPB->GetInt( kRefEnableTreeBehavior ) )
		ctrl->SetFlag( pfGUIListBoxMod::kGrowLeavesAndProcessOxygen );

	if( fCompPB->GetInt( kRefHandsOffMultiSelect ) )
		ctrl->SetFlag( pfGUIListBoxMod::kHandsOffMultiSelect );
	
	INode *sNode = fCompPB->GetINode( kRefSkin );
	if( sNode != nil )
	{
		plComponentBase *comp = ( (plMaxNode *)sNode )->ConvertToComponent();
		if( comp != nil )
		{
			Class_ID nodeID = comp->ClassID();
			hsAssert( nodeID == GUI_SKIN_CLASSID, "Bad node param in GUIMenu::Convert()" );

			plGUISkinComp *skin = (plGUISkinComp *)comp;
			hsgResMgr::ResMgr()->AddViaNotify( skin->GetConvertedSkin()->GetKey(), TRACKED_NEW plGenRefMsg( ctrl->GetKey(), plRefMsg::kOnCreate, -1, pfGUIControlMod::kRefSkin ), plRefFlags::kActiveRef );
		}
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUITextBox Component /////////////////////////////////////////////////////////////////////
//
//	GUI element that displays a block of wrapped text.
//
/////////////////////////////////////////////////////////////////////////////////////////////////


// Class that accesses the paramblock below.
class plGUITextBoxComponent : public plGUIControlBase
{
protected:

	virtual pfGUIControlMod	*IGetNewControl( void ) { return TRACKED_NEW pfGUITextBoxMod; }
	virtual bool			INeedsDynamicText( void ) { return true; }

public:
	plGUITextBoxComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);

	hsBool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

	enum
	{
		kRefInitText,
		kRefFontSize,
		kRefXparentBgnd,
		kRefJustify,
		kRefScaleWithRes,
		kRefUseLocalization,
		kRefLocalizationPath
	};
};

class plGUITextBoxProc : public ParamMap2UserDlgProc
{
private:
	std::vector<std::string> fTranslations;
	int fCurLanguage;
	void ISetTranslation(int lang, std::string text)
	{
		while (lang >= fTranslations.size())
			fTranslations.push_back("");
		fTranslations[lang] = text;
	}
protected:

public:

	void DeleteThis() {}

	BOOL DlgProc( TimeValue t, IParamMap2 *pmap, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
	{
		int i;
		switch( msg )
		{
			case WM_INITDIALOG:
				// make sure there is a string to get
				if ( pmap->GetParamBlock()->GetStr( plGUITextBoxComponent::kRefInitText ) )
				{
					fTranslations = plLocalization::StringToLocal(pmap->GetParamBlock()->GetStr( plGUITextBoxComponent::kRefInitText ) );
					SetDlgItemText( hWnd, IDC_GUI_INITTEXT, fTranslations[0].c_str() );
				}
				else
					// if there is no text, then there is nothing to translate
					SetDlgItemText( hWnd, IDC_GUI_INITTEXT, pmap->GetParamBlock()->GetStr( plGUITextBoxComponent::kRefInitText ) );
				SendMessage( GetDlgItem( hWnd, IDC_GUI_LANGUAGE ), CB_RESETCONTENT, 0, 0 );
				for (i=0; i<plLocalization::kNumLanguages; i++)
					SendMessage( GetDlgItem( hWnd, IDC_GUI_LANGUAGE ), CB_ADDSTRING, 0, (LPARAM)plLocalization::GetLanguageName((plLocalization::Language)i) );
				SendMessage( GetDlgItem( hWnd, IDC_GUI_LANGUAGE ), CB_SETCURSEL, 0, 0 );
				fCurLanguage = 0;

				SetDlgItemText( hWnd, IDC_GUI_LOCALIZATION_PATH, pmap->GetParamBlock()->GetStr( plGUITextBoxComponent::kRefLocalizationPath ) );

				if ( pmap->GetParamBlock()->GetInt( plGUITextBoxComponent::kRefUseLocalization ) != 0 )
				{
					// disable standard text, enable loc path
					EnableWindow( GetDlgItem( hWnd, IDC_GUI_INITTEXT ), false );
					EnableWindow( GetDlgItem( hWnd, IDC_GUI_LANGUAGE ), false );
					EnableWindow( GetDlgItem( hWnd, IDC_GUI_LOCALIZATION_PATH ), true );
					EnableWindow( GetDlgItem( hWnd, IDC_GUI_SELECT_LOC_PATH ), true );
					CheckDlgButton( hWnd, IDC_GUI_USE_LOCALIZATION, BST_CHECKED );
				}
				else
				{
					// enable standard text, disable loc path
					EnableWindow( GetDlgItem( hWnd, IDC_GUI_INITTEXT ), true );
					EnableWindow( GetDlgItem( hWnd, IDC_GUI_LANGUAGE ), true );
					EnableWindow( GetDlgItem( hWnd, IDC_GUI_LOCALIZATION_PATH ), false );
					EnableWindow( GetDlgItem( hWnd, IDC_GUI_SELECT_LOC_PATH ), false );
					CheckDlgButton( hWnd, IDC_GUI_USE_LOCALIZATION, BST_UNCHECKED );
				}
				return true;

			case WM_DESTROY:
				break;

			case WM_COMMAND:
				if( LOWORD( wParam ) == IDC_GUI_INITTEXT )
				{
					if( HIWORD( wParam ) == EN_CHANGE )
					{
						int strLen = SendDlgItemMessage( hWnd, IDC_GUI_INITTEXT, WM_GETTEXTLENGTH, 0, 0 );
						if( strLen > 0 )
						{
							char *str = TRACKED_NEW char[ strLen + 1 ];
							GetDlgItemText( hWnd, IDC_GUI_INITTEXT, str, strLen + 1 );
							str[ strLen ] = 0;
							ISetTranslation(fCurLanguage,str);
							delete [] str;

							std::string translation = plLocalization::LocalToString(fTranslations);
							str = TRACKED_NEW char[ translation.length() + 1 ];
							strcpy(str,translation.c_str());
							str[translation.length()] = 0;
				
							pmap->GetParamBlock()->SetValue( plGUITextBoxComponent::kRefInitText, 0, str );
							delete [] str;
						}
					}
					else if( HIWORD( wParam ) == EN_KILLFOCUS )
					{
						plMaxAccelerators::Enable();
					}
					else if( HIWORD( wParam ) == EN_SETFOCUS )
					{
						plMaxAccelerators::Disable();
					}
				}
				else if( LOWORD( wParam ) == IDC_GUI_LOCALIZATION_PATH )
				{
					if( HIWORD( wParam ) == EN_CHANGE )
					{
						int strLen = SendDlgItemMessage( hWnd, IDC_GUI_LOCALIZATION_PATH, WM_GETTEXTLENGTH, 0, 0 );
						if( strLen > 0 )
						{
							char *str = TRACKED_NEW char[ strLen + 1 ];
							GetDlgItemText( hWnd, IDC_GUI_LOCALIZATION_PATH, str, strLen + 1 );
							str[ strLen ] = 0;
							pmap->GetParamBlock()->SetValue( plGUITextBoxComponent::kRefLocalizationPath, 0, str );
							delete [] str;
						}
					}
					else if( HIWORD( wParam ) == EN_KILLFOCUS )
					{
						plMaxAccelerators::Enable();
					}
					else if( HIWORD( wParam ) == EN_SETFOCUS )
					{
						plMaxAccelerators::Disable();
					}
				}
				else if( LOWORD( wParam ) == IDC_GUI_LANGUAGE )
				{
					if( HIWORD( wParam ) == CBN_SELCHANGE )
					{
						int idx = SendDlgItemMessage( hWnd, IDC_GUI_LANGUAGE, CB_GETCURSEL, 0, 0 );
						if (idx >= fTranslations.size())
							SetDlgItemText( hWnd, IDC_GUI_INITTEXT, "" );
						else
							SetDlgItemText( hWnd, IDC_GUI_INITTEXT, fTranslations[idx].c_str() );
						fCurLanguage = idx;
					}
				}
				else if( LOWORD( wParam ) == IDC_GUI_SELECT_LOC_PATH )
				{
					char value[512];
					GetDlgItemText( hWnd, IDC_GUI_LOCALIZATION_PATH, value, 512 );
					plPickLocalizationDlg dlg( value );
					if( dlg.DoPick() )
					{
						pmap->GetParamBlock()->SetValue( plGUITextBoxComponent::kRefLocalizationPath, 0, (char*)dlg.GetValue() );
						SetDlgItemText( hWnd, IDC_GUI_LOCALIZATION_PATH, (char*)dlg.GetValue() );
					}
				}
				else if( LOWORD( wParam ) == IDC_GUI_USE_LOCALIZATION )
				{
					// enable/disable the appropriate values
					bool useLoc = ( IsDlgButtonChecked( hWnd, IDC_GUI_USE_LOCALIZATION ) == BST_CHECKED );
					pmap->GetParamBlock()->SetValue( plGUITextBoxComponent::kRefUseLocalization, 0, useLoc ? 1 : 0 );

					if ( useLoc )
					{
						// disable standard text, enable loc path
						EnableWindow( GetDlgItem( hWnd, IDC_GUI_INITTEXT ), false );
						EnableWindow( GetDlgItem( hWnd, IDC_GUI_LANGUAGE ), false );
						EnableWindow( GetDlgItem( hWnd, IDC_GUI_LOCALIZATION_PATH ), true );
						EnableWindow( GetDlgItem( hWnd, IDC_GUI_SELECT_LOC_PATH ), true );
					}
					else
					{
						// enable standard text, disable loc path
						EnableWindow( GetDlgItem( hWnd, IDC_GUI_INITTEXT ), true );
						EnableWindow( GetDlgItem( hWnd, IDC_GUI_LANGUAGE ), true );
						EnableWindow( GetDlgItem( hWnd, IDC_GUI_LOCALIZATION_PATH ), false );
						EnableWindow( GetDlgItem( hWnd, IDC_GUI_SELECT_LOC_PATH ), false );
					}
				}
				break;
		}
		return false;
	}
};
static plGUITextBoxProc gGUITextBoxProc;

//Max desc stuff necessary below.
CLASS_DESC(plGUITextBoxComponent, gGUITextBoxDesc, "GUI Text Box",  "GUITextBox", COMP_TYPE_GUI, GUI_TEXTBOX_CLASSID )

ParamBlockDesc2 gGUITextBoxBk
(	// KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUITextBox"), 0, &gGUITextBoxDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

	2,
	plGUIControlBase::kRollMain, IDD_COMP_GUITEXTBOX, IDS_COMP_GUITEXTBOX, 0, 0, &gGUITextBoxProc,
	plGUIControlBase::kRollProc, IDD_COMP_GUIPROCROLLOUT, IDS_COMP_GUIPROCROLLOUT, 0, 0, nil,	
	
	&sGUIControlProcParamTemplate,	

		plGUITextBoxComponent::kRefInitText,	_T("InitText"),		TYPE_STRING, 		0, 0,
//			p_ui,	plGUIControlBase::kRollMain, TYPE_EDITBOX, IDC_GUI_INITTEXT,
			end,

		plGUITextBoxComponent::kRefXparentBgnd,	_T( "xparentBgnd" ), TYPE_BOOL, 0, 0,
			p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_XPARENT,
			p_default, FALSE,
			end,
	
		plGUITextBoxComponent::kRefJustify,	_T("justify"), TYPE_INT, 		0, 0,
			p_ui,	plGUIControlBase::kRollMain, TYPE_RADIO, 3, IDC_JUSTIFYRADIO, IDC_JUSTRADIO2, IDC_JUSTRADIO3, 
			p_default, 0,
			end,

		plGUITextBoxComponent::kRefScaleWithRes,	_T( "scaleWithRes" ), TYPE_BOOL, 0, 0,
			p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_SCALERES,
			p_default, FALSE,
			end,

		plGUITextBoxComponent::kRefUseLocalization,	_T( "useLocalization" ), TYPE_BOOL, 0, 0,
			p_default, FALSE,
			end,

		plGUITextBoxComponent::kRefLocalizationPath,_T( "localizationPath" ),TYPE_STRING, 0, 0,
			end,
				
	end
);

plGUITextBoxComponent::plGUITextBoxComponent()
{
	fClassDesc = &gGUITextBoxDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plGUITextBoxComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	return plGUIControlBase::SetupProperties( node, pErrMsg );
}

hsBool plGUITextBoxComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	return plGUIControlBase::PreConvert( node, pErrMsg );
}

hsBool plGUITextBoxComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	if( !plGUIControlBase::Convert( node, pErrMsg ) )
		return false;

	pfGUITextBoxMod *ctrl = (pfGUITextBoxMod *)fControl;

	ctrl->SetText( fCompPB->GetStr( kRefInitText ) );

	if( fCompPB->GetInt( kRefXparentBgnd ) )
		ctrl->SetFlag( pfGUITextBoxMod::kXparentBgnd );

	int just = fCompPB->GetInt( kRefJustify );
	if( just == 1 )
		ctrl->SetFlag( pfGUITextBoxMod::kCenterJustify );
	else if( just == 2 )
		ctrl->SetFlag( pfGUITextBoxMod::kRightJustify );

	if( fCompPB->GetInt( kRefScaleWithRes ) )
		ctrl->SetFlag( pfGUIControlMod::kScaleTextWithResolution );

	ctrl->SetUseLocalizationPath( fCompPB->GetInt( kRefUseLocalization ) != 0 );
	ctrl->SetLocalizationPath( fCompPB->GetStr( kRefLocalizationPath ) );

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIEditBox Component /////////////////////////////////////////////////////////////////////
//
//	GUI element that can be dragged anywhere in the 2D viewing plane.
//
/////////////////////////////////////////////////////////////////////////////////////////////////


// Class that accesses the paramblock below.
class plGUIEditBoxComponent : public plGUIControlBase
{
protected:

	virtual pfGUIControlMod	*IGetNewControl( void ) { return TRACKED_NEW pfGUIEditBoxMod; }
	virtual bool			INeedsDynamicText( void ) { return true; }

public:
	plGUIEditBoxComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);

	hsBool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

	enum
	{
		kRefXparentBgnd,
		kRefScaleWithRes
	};
};

//Max desc stuff necessary below.
CLASS_DESC(plGUIEditBoxComponent, gGUIEditBoxDesc, "GUI Edit Box",  "GUIEditBox", COMP_TYPE_GUI, GUI_EDITBOX_CLASSID )

ParamBlockDesc2 gGUIEditBoxBk
(	// KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUIEditBox"), 0, &gGUIEditBoxDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

	2,
	plGUIControlBase::kRollMain, IDD_COMP_GUIEDITBOX, IDS_COMP_GUIEDITBOX, 0, 0, NULL,
	plGUIControlBase::kRollProc, IDD_COMP_GUIPROCROLLOUT, IDS_COMP_GUIPROCROLLOUT, 0, 0, nil,	
	
	&sGUIControlProcParamTemplate,	

	plGUIEditBoxComponent::kRefXparentBgnd,	_T( "xparentBgnd" ), TYPE_BOOL, 0, 0,
		p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_XPARENT,
		p_default, FALSE,
		end,
	
	plGUIEditBoxComponent::kRefScaleWithRes,	_T( "scaleWithRes" ), TYPE_BOOL, 0, 0,
		p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_SCALERES,
		p_default, FALSE,
		end,
			
	end
);

plGUIEditBoxComponent::plGUIEditBoxComponent()
{
	fClassDesc = &gGUIEditBoxDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plGUIEditBoxComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	return plGUIControlBase::SetupProperties( node, pErrMsg );
}

hsBool plGUIEditBoxComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	return plGUIControlBase::PreConvert( node, pErrMsg );
}

hsBool plGUIEditBoxComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	if( !plGUIControlBase::Convert( node, pErrMsg ) )
		return false;

	pfGUIEditBoxMod *ctrl = (pfGUIEditBoxMod *)fControl;
	
	if( fCompPB->GetInt( kRefXparentBgnd ) )
		ctrl->SetFlag( pfGUIEditBoxMod::kXparentBgnd );

	if( fCompPB->GetInt( kRefScaleWithRes ) )
		ctrl->SetFlag( pfGUIControlMod::kScaleTextWithResolution );

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIUpDownPair Component //////////////////////////////////////////////////////////////////
//
//	GUI grouping element that uses two buttons to alter a value up and down
//
/////////////////////////////////////////////////////////////////////////////////////////////////


// Class that accesses the paramblock below.
class plGUIUpDownPairComponent : public plGUIControlBase
{
protected:

	virtual pfGUIControlMod	*IGetNewControl( void ) { return TRACKED_NEW pfGUIUpDownPairMod; }

public:
	plGUIUpDownPairComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);

	hsBool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

	enum
	{
		kRefMinValue,
		kRefMaxValue,
		kRefStep,
		kRefUpControl,
		kRefDownControl
	};
};

//// Dialog proc ////////////////////////////////////////////////////////////////////////////////

class plGUIUDPairDlgProc : public ParamMap2UserDlgProc
{
protected:
	ParamID			fUpNodeID, fDownNodeID;
	int				fUpDlgItem, fDownDlgItem;
	TCHAR			fTitle[ 128 ];

public:
	plGUIUDPairDlgProc( ParamID upNodeID, int upDlgItem, ParamID downNodeID, int downDlgItem, TCHAR *title )
	{
		fUpNodeID = upNodeID;
		fDownNodeID = downNodeID;
		fUpDlgItem = upDlgItem;
		fDownDlgItem = downDlgItem;
		strcpy( fTitle, title );
	}

	BOOL DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
	{
		switch ( msg )
		{
			case WM_INITDIALOG:
				{
					IParamBlock2 *pb = map->GetParamBlock();

					INode *node = pb->GetINode( fUpNodeID );
					TSTR newName( node ? node->GetName() : "Pick" );
					::SetWindowText( ::GetDlgItem( hWnd, fUpDlgItem ), newName );

					node = pb->GetINode( fDownNodeID );
					TSTR newName2( node ? node->GetName() : "Pick" );
					::SetWindowText( ::GetDlgItem( hWnd, fDownDlgItem ), newName2 );
				}
				return true;

			case WM_COMMAND:
				if( ( HIWORD( wParam ) == BN_CLICKED ) )
				{
					if( LOWORD( wParam ) == fUpDlgItem )
					{
						IParamBlock2 *pb = map->GetParamBlock();
						plGUICtrlHitCallback hitCB( (INode *)pb->GetOwner(), pb, fUpNodeID, fTitle, true, GUI_BUTTON_CLASSID );
						GetCOREInterface()->DoHitByNameDialog( &hitCB );

						INode* node = pb->GetINode( fUpNodeID );
						TSTR newName( node ? node->GetName() : "Pick" );
						::SetWindowText( ::GetDlgItem(hWnd, fUpDlgItem ), newName );
						map->Invalidate( fUpNodeID );
						::InvalidateRect( hWnd, NULL, TRUE );
					}
					else if( LOWORD( wParam ) == fDownDlgItem )
					{
						IParamBlock2 *pb = map->GetParamBlock();
						plGUICtrlHitCallback hitCB( (INode *)pb->GetOwner(), pb, fDownNodeID, fTitle, true, GUI_BUTTON_CLASSID );
						GetCOREInterface()->DoHitByNameDialog( &hitCB );

						INode* node = pb->GetINode( fDownNodeID );
						TSTR newName( node ? node->GetName() : "Pick" );
						::SetWindowText( ::GetDlgItem(hWnd, fDownDlgItem ), newName );
						map->Invalidate( fDownDlgItem );
						::InvalidateRect( hWnd, NULL, TRUE );
					}
				}
				return true;
		}

		return false;
	}

	void DeleteThis() {}
};

// When one of our parameters that is a ref changes, send out the component ref
// changed message.  Normally, messages from component refs are ignored since
// they pass along all the messages of the ref, which generates a lot of false
// converts.
class plGUIUDAccessor : public PBAccessor
{
public:
	void Set( PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t )
	{
		if( id == plGUIUpDownPairComponent::kRefUpControl
			|| id == plGUIUpDownPairComponent::kRefDownControl )
		{
			plGUIUpDownPairComponent *comp = (plGUIUpDownPairComponent *)owner;
			comp->NotifyDependents( FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED );
		}
	}
};

static plGUIUDAccessor sGUIUDAccessor;
static plGUIUDPairDlgProc sGUIUDPairDlgProc( plGUIUpDownPairComponent::kRefUpControl, IDC_GUI_COMPSELBTN,
											plGUIUpDownPairComponent::kRefDownControl, IDC_GUI_COMPSELBTN2,
											"Select the control to use in this pair" );

//Max desc stuff necessary below.
CLASS_DESC(plGUIUpDownPairComponent, gGUIUDPairDesc, "GUI Up/Down Pair",  "GUIUDPair", COMP_TYPE_GUI, GUI_UPDOWNPAIR_CLASSID )

ParamBlockDesc2 gGUIUDPairBk
(	// KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUIUDPair"), 0, &gGUIUDPairDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

	2,
	plGUIControlBase::kRollMain, IDD_COMP_GUIUDSCROLL, IDS_COMP_GUIUDSCROLL, 0, 0, &sGUIUDPairDlgProc,
	plGUIControlBase::kRollProc, IDD_COMP_GUIPROCROLLOUT, IDS_COMP_GUIPROCROLLOUT, 0, 0, nil,	
	
	&sGUIControlProcParamTemplate,	

	plGUIUpDownPairComponent::kRefUpControl, _T("upControl"),	TYPE_INODE,		0, 0,
		p_prompt, IDS_COMP_GUI_SELECTUDCTRL,
		p_accessor, &sGUIUDAccessor,
		end,

	plGUIUpDownPairComponent::kRefDownControl, _T("downControl"),	TYPE_INODE,		0, 0,
		p_prompt, IDS_COMP_GUI_SELECTUDCTRL,
		p_accessor, &sGUIUDAccessor,
		end,

	plGUIUpDownPairComponent::kRefMinValue, _T("minValue"),	TYPE_FLOAT,	0, 0,	
		p_default, 0.0f,
		p_ui, plGUIControlBase::kRollMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT, IDC_GUI_LOWER, IDC_GUI_LOWER_SPIN, SPIN_AUTOSCALE,
		end,

	plGUIUpDownPairComponent::kRefMaxValue, _T("maxValue"),	TYPE_FLOAT,	0, 0,	
		p_default, 10.0f,
		p_ui, plGUIControlBase::kRollMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT, IDC_GUI_UPPER, IDC_GUI_UPPER_SPIN, SPIN_AUTOSCALE,
		end,

	plGUIUpDownPairComponent::kRefStep, _T("step"),	TYPE_FLOAT,	0, 0,	
		p_default, 1.0f,
		p_ui, plGUIControlBase::kRollMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT, IDC_GUI_STEP, IDC_GUI_STEP_SPIN, SPIN_AUTOSCALE,
		end,

	end
);

plGUIUpDownPairComponent::plGUIUpDownPairComponent()
{
	fClassDesc = &gGUIUDPairDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plGUIUpDownPairComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	return plGUIControlBase::SetupProperties( node, pErrMsg );
}

hsBool plGUIUpDownPairComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	return plGUIControlBase::PreConvert( node, pErrMsg );
}

hsBool plGUIUpDownPairComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	if( !plGUIControlBase::Convert( node, pErrMsg ) )
		return false;

	pfGUIUpDownPairMod *ctrl = (pfGUIUpDownPairMod *)fControl;

	// Get the child controls
	pfGUIButtonMod *up = pfGUIButtonMod::ConvertNoRef( GrabControlMod( fCompPB->GetINode( kRefUpControl ) ) );
	pfGUIButtonMod *down = pfGUIButtonMod::ConvertNoRef( GrabControlMod( fCompPB->GetINode( kRefDownControl ) ) );

	ctrl->SetControls( up, down );

	ctrl->SetRange( fCompPB->GetFloat( kRefMinValue ), fCompPB->GetFloat( kRefMaxValue ) );
	ctrl->SetStep( fCompPB->GetFloat( kRefStep ) );

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIDragBar Component ///////////////////////////////////////////////////////////////////
//
//	GUI element that can be dragged anywhere in the 2D viewing plane.
//
/////////////////////////////////////////////////////////////////////////////////////////////////


// Class that accesses the paramblock below.
class plGUIDragBarComponent : public plGUIControlBase
{
protected:

	virtual pfGUIControlMod	*IGetNewControl( void ) { return TRACKED_NEW pfGUIDragBarCtrl; }
	virtual bool			ICanHaveProxy( void ) { return true; }

public:
	plGUIDragBarComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);

	hsBool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

	enum
	{
	};
};

//Max desc stuff necessary below.
CLASS_DESC(plGUIDragBarComponent, gGUIDragBarDesc, "GUI Dialog Drag Bar",  "GUIDragBar", COMP_TYPE_GUI, GUI_DRAGBAR_CLASSID )

ParamBlockDesc2 gGUIDragBarBk
(	// KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUIDragBar"), 0, &gGUIDragBarDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

	3,
	plGUIControlBase::kRollMain, IDD_COMP_GUIDRAGBAR, IDS_COMP_GUIDRAGBAR, 0, 0, NULL,
	plGUIControlBase::kRollProc, IDD_COMP_GUIPROCROLLOUT, IDS_COMP_GUIPROCROLLOUT, 0, 0, nil,	
	
	sGUIProxyParamHeader,
	
	&sGUIControlProcParamTemplate,	

	sGUIProxyParamTemplate,

	end
);

plGUIDragBarComponent::plGUIDragBarComponent()
{
	fClassDesc = &gGUIDragBarDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plGUIDragBarComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	node->SetForceLocal( true );
	return plGUIControlBase::SetupProperties( node, pErrMsg );
}

hsBool plGUIDragBarComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	return plGUIControlBase::PreConvert( node, pErrMsg );
}

hsBool plGUIDragBarComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	if( !plGUIControlBase::Convert( node, pErrMsg ) )
		return false;

	pfGUIDragBarCtrl *ctrl = (pfGUIDragBarCtrl *)fControl;
	
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIRadioGroup Component //////////////////////////////////////////////////////////////////
//
//	GUI grouping element that ensures that only one of a group of check boxes is checked at any
//	one time, and takes on the value of whichever one is currently checked, or -1 if none.
//
/////////////////////////////////////////////////////////////////////////////////////////////////


// Class that accesses the paramblock below.
class plGUIRadioGroupAccessor;
class plGUIRadioGroupComponent : public plGUIControlBase
{
	friend class plGUIRadioGroupAccessor;

protected:

	virtual pfGUIControlMod	*IGetNewControl( void ) { return TRACKED_NEW pfGUIRadioGroupCtrl; }

public:
	plGUIRadioGroupComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);

	hsBool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

	enum
	{
		kRefCheckBoxes,
		kRefDefaultSel,
		kRefAllowNoSel
	};
};

//// Dialog proc ////////////////////////////////////////////////////////////////////////////////

class plGUIRadioGroupProc : public ParamMap2UserDlgProc
{
protected:

public:
	plGUIRadioGroupProc()
	{
	}

	void	SetSpinnerRange( IParamMap2 *pMap )
	{
		if( pMap == nil )
			return;

		HWND	hWnd = pMap->GetHWnd();
		if( hWnd == nil )
			return;

		ISpinnerControl *spin = GetISpinner( GetDlgItem( hWnd, IDC_GUI_DEFSEL_SPIN ) );

		int minValue = pMap->GetParamBlock()->GetInt( plGUIRadioGroupComponent::kRefAllowNoSel ) ? -1 : 0;
		int maxValue = pMap->GetParamBlock()->Count( plGUIRadioGroupComponent::kRefCheckBoxes );

		spin->SetLimits( minValue, maxValue );

		ReleaseISpinner( spin );
	}

	BOOL DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
	{
		switch ( msg )
		{
			case WM_INITDIALOG:
				{
					SetSpinnerRange( map );
				}
				return true;

			case WM_COMMAND:
				if( ( HIWORD( wParam ) == BN_CLICKED ) )
				{
					if( LOWORD( wParam ) == IDC_GUI_ADDCHECK )
					{
						IParamBlock2 *pb = map->GetParamBlock();

						plGUICtrlHitCallback hitCB( (INode *)pb->GetOwner(), pb, plGUIRadioGroupComponent::kRefCheckBoxes, 
									"Select a check box to add to this radio group", true, GUI_CHECKBOX_CLASSID, false );

						GetCOREInterface()->DoHitByNameDialog( &hitCB );

						map->Invalidate( plGUIRadioGroupComponent::kRefCheckBoxes );
					}
				}
				return true;
		}

		return false;
	}

	void DeleteThis() {}
};
static plGUIRadioGroupProc sGUIRadioGroupProc;

//Max desc stuff necessary below.
CLASS_DESC(plGUIRadioGroupComponent, gGUIRadioGroupDesc, "GUI Radio Group",  "GUIRadioGroup", COMP_TYPE_GUI, GUI_RADIOGROUP_CLASSID )

// When one of our parameters that is a ref changes, send out the component ref
// changed message.  Normally, messages from component refs are ignored since
// they pass along all the messages of the ref, which generates a lot of false
// converts.
class plGUIRadioGroupAccessor : public PBAccessor
{
public:
	void Set( PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t )
	{
		plGUIRadioGroupComponent *comp = (plGUIRadioGroupComponent *)owner;
		IParamBlock2	*pBlock = comp->fCompPB;

		if( id == plGUIRadioGroupComponent::kRefCheckBoxes )
		{
			comp->NotifyDependents( FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED );
			sGUIRadioGroupProc.SetSpinnerRange( pBlock->GetMap( plGUIControlBase::kRollMain ) );
		}
		else if( id == plGUIRadioGroupComponent::kRefAllowNoSel )
			sGUIRadioGroupProc.SetSpinnerRange( pBlock->GetMap( plGUIControlBase::kRollMain ) );
	}

	void	TabChanged( tab_changes changeCode, Tab<PB2Value> *tab, ReferenceMaker *owner, ParamID id, int tabIndex, int count )
	{
		plGUIRadioGroupComponent *comp = (plGUIRadioGroupComponent *)owner;
		IParamBlock2	*pBlock = comp->fCompPB;

		if( id == plGUIRadioGroupComponent::kRefCheckBoxes )
		{
			comp->NotifyDependents( FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED );
			sGUIRadioGroupProc.SetSpinnerRange( pBlock->GetMap( plGUIControlBase::kRollMain ) );
		}
	}

};

static plGUIRadioGroupAccessor sGUIRadioGroupAccessor;

ParamBlockDesc2 gGUIRadioGroupBk
(	// KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUIRadioGroup"), 0, &gGUIRadioGroupDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

	2,
	plGUIControlBase::kRollMain, IDD_COMP_GUIRADIOGROUP, IDS_COMP_GUIRADIOGROUP, 0, 0, &sGUIRadioGroupProc,
	plGUIControlBase::kRollProc, IDD_COMP_GUIPROCROLLOUT, IDS_COMP_GUIPROCROLLOUT, 0, 0, nil,	
	
	&sGUIControlProcParamTemplate,	

	plGUIRadioGroupComponent::kRefCheckBoxes,	_T("checkBoxes"),	TYPE_INODE_TAB, 0,		0, 0,
		p_ui,			plGUIControlBase::kRollMain, TYPE_NODELISTBOX, IDC_GUI_CHECKLIST, 0, 0, IDC_GUI_DELCHECK,
		p_accessor,		&sGUIRadioGroupAccessor,
		end,

	plGUIRadioGroupComponent::kRefDefaultSel, _T("defaultSelection"),	TYPE_INT,	0, 0,	
		p_default, 0,
		p_range, 0, 0,
		p_ui, plGUIControlBase::kRollMain, TYPE_SPINNER,	EDITTYPE_INT, IDC_GUI_DEFSEL, IDC_GUI_DEFSEL_SPIN, SPIN_AUTOSCALE,
		end,

	plGUIRadioGroupComponent::kRefAllowNoSel,	_T( "allowNoSel" ), TYPE_BOOL, 0, 0,
		p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_ALLOWNONE,
		p_default, FALSE,
		p_accessor, &sGUIRadioGroupAccessor,
		end,

	end
);

plGUIRadioGroupComponent::plGUIRadioGroupComponent()
{
	fClassDesc = &gGUIRadioGroupDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plGUIRadioGroupComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	return plGUIControlBase::SetupProperties( node, pErrMsg );
}


hsBool plGUIRadioGroupComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	return plGUIControlBase::PreConvert( node, pErrMsg );
}

hsBool plGUIRadioGroupComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	if( !plGUIControlBase::Convert( node, pErrMsg ) )
		return false;

	pfGUIRadioGroupCtrl *ctrl = (pfGUIRadioGroupCtrl *)fControl;

	int	i;
	ctrl->ClearControlList();
	for( i = 0; i < fCompPB->Count( kRefCheckBoxes ); i++ )
	{
		pfGUICheckBoxCtrl *cb = pfGUICheckBoxCtrl::ConvertNoRef( GrabControlMod( fCompPB->GetINode( kRefCheckBoxes, 0, i ) ) );
		if( cb != nil )
			ctrl->AddControl( cb );
	}

	if( fCompPB->GetInt( kRefAllowNoSel ) )
		ctrl->SetFlag( pfGUIRadioGroupCtrl::kAllowNoSelection );

	ctrl->SetDefaultValue( fCompPB->GetInt( kRefDefaultSel ) );

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIDynDisplay Component //////////////////////////////////////////////////////////////////
//
//	GUI element that can be dragged anywhere in the 2D viewing plane.
//
/////////////////////////////////////////////////////////////////////////////////////////////////


// Class that accesses the paramblock below.
class plGUIDynDisplayComponent : public plGUIControlBase
{
protected:

	virtual pfGUIControlMod	*IGetNewControl( void ) { return TRACKED_NEW pfGUIDynDisplayCtrl; }
	virtual bool			IHasProcRollout( void ) { return false; }

public:
	plGUIDynDisplayComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);

	hsBool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

	enum
	{
		kRefDynLayer
	};
};

//// Dialog proc ////////////////////////////////////////////////////////////////////////////////

class plGUIDynDisplayProc : public ParamMap2UserDlgProc
{
protected:

public:
	plGUIDynDisplayProc()
	{
	}

	BOOL DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
	{
		switch ( msg )
		{
			case WM_INITDIALOG:
				{
					IParamBlock2 *pb = map->GetParamBlock();

					Texmap *tmap = pb->GetTexmap( plGUIDynDisplayComponent::kRefDynLayer );
					if( tmap != nil )
						SetDlgItemText( hWnd, IDC_GUI_PICKMAT, (const char *)tmap->GetName() );
					else
						SetDlgItemText( hWnd, IDC_GUI_PICKMAT, "Pick" );
				}
				return true;

			case WM_COMMAND:
				if( ( HIWORD( wParam ) == BN_CLICKED ) )
				{
					if( LOWORD( wParam ) == IDC_GUI_PICKMAT )
					{
						IParamBlock2 *pb = map->GetParamBlock();

						if( plPickMaterialMap::PickTexmap( pb, plGUIDynDisplayComponent::kRefDynLayer ) )
						{
							Texmap *tmap = pb->GetTexmap( plGUIDynDisplayComponent::kRefDynLayer );
							if( tmap != nil )
								SetDlgItemText( hWnd, IDC_GUI_PICKMAT, (const char *)tmap->GetName() );
							else
								SetDlgItemText( hWnd, IDC_GUI_PICKMAT, "Pick" );

							map->Invalidate( plGUIDynDisplayComponent::kRefDynLayer );
						}
					}
				}
				return true;
		}

		return false;
	}

	void DeleteThis() {}
};
static plGUIDynDisplayProc	sGUIDynDisplayProc;

//Max desc stuff necessary below.
CLASS_DESC(plGUIDynDisplayComponent, gGUIDynDisplayDesc, "GUI Dynamic Display",  "GUIDynDisplay", COMP_TYPE_GUI, GUI_DYNDISPLAY_CLASSID )

ParamBlockDesc2 gGUIDynDisplayBk
(	// KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUIDynDisplay"), 0, &gGUIDynDisplayDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, plComponent::kRefComp,

	1,
	plGUIControlBase::kRollMain, IDD_COMP_GUIDYNDISPLAY, IDS_COMP_GUIDYNDISPLAY, 0, 0, &sGUIDynDisplayProc,

	plGUIDynDisplayComponent::kRefDynLayer,	_T("dynLayer"),	TYPE_TEXMAP, 0, 0,
//		p_ui, plGUIControlBase::kRollMain, TYPE_TEXMAPBUTTON, IDC_GUI_COMPSELBTN,
		end,

	end
);

plGUIDynDisplayComponent::plGUIDynDisplayComponent()
{
	fClassDesc = &gGUIDynDisplayDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plGUIDynDisplayComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	return plGUIControlBase::SetupProperties( node, pErrMsg );
}

hsBool plGUIDynDisplayComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	return plGUIControlBase::PreConvert( node, pErrMsg );
}

hsBool plGUIDynDisplayComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	if( !plGUIControlBase::Convert( node, pErrMsg ) )
		return false;

	pfGUIDynDisplayCtrl *ctrl = (pfGUIDynDisplayCtrl *)fControl;
	
	Texmap *tmap = fCompPB->GetTexmap( plGUIDynDisplayComponent::kRefDynLayer );
	plPlasmaMAXLayer *pLayer = plPlasmaMAXLayer::GetPlasmaMAXLayer( tmap );
	if( pLayer == nil /*|| pLayer->ClassID() != DYN_TEXT_LAYER_CLASS_ID */ )
	{

		pErrMsg->Set(true, "GUI Control Component Error", "The texmap selected for the Dynamic Display Control on object \"%s\" is not a Plasma Dynamic Text Layer. Please fix.", node->GetName() ).Show();
		return false;	
	}
	
	const hsTArray<hsMaterialConverter::DoneMaterialData> &materials = hsMaterialConverter::Instance().DoneMaterials();

	UInt32 i,count = pLayer->GetNumConversionTargets();
	for( i = 0; i < count; i++ )
	{
		plLayerInterface *layIface = pLayer->GetConversionTarget( i );
		
		ctrl->AddLayer( layIface );

		plDynamicTextMap *map = plDynamicTextMap::ConvertNoRef( layIface->GetTexture() );
		if( map != nil )
			ctrl->AddMap( map );

		UInt32 mat;
		bool found = false;
		for (mat=0; mat<materials.GetCount(); mat++)
		{
			hsGMaterial *curMaterial = materials[mat].fHsMaterial;
			UInt32 lay;
			for (lay=0; lay<curMaterial->GetNumLayers(); lay++)
			{
				if (layIface->BottomOfStack() == curMaterial->GetLayer(lay))
				{
					ctrl->AddMaterial(curMaterial);
					found = true;
					break;
				}
			}
			if (found)
				break;
		}
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIMultiLineEdit Component ///////////////////////////////////////////////////////////////
//
//	GUI element that can be dragged anywhere in the 2D viewing plane.
//
/////////////////////////////////////////////////////////////////////////////////////////////////


// Class that accesses the paramblock below.
class plGUIMultiLineEditComp : public plGUIControlBase
{
protected:

	virtual pfGUIControlMod	*IGetNewControl( void ) { return TRACKED_NEW pfGUIMultiLineEditCtrl; }
	virtual bool			INeedsDynamicText( void ) { return true; }

public:
	plGUIMultiLineEditComp();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);

	hsBool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

	enum
	{
		kRefXparentBgnd,
		kRefScaleWithRes,
		kRefUseScroll,
		kRefScrollCtrl,
	};
};

static plGUISingleCtrlDlgProc sGUIMultiLineProc( plGUIMultiLineEditComp::kRefScrollCtrl, IDC_GUI_COMPSELBTN,
											"Select the control to use for scrolling this multi-line edit box", sScrollingClassesToSelect );

//Max desc stuff necessary below.
CLASS_DESC(plGUIMultiLineEditComp, gGUIMultiLineEditDesc, "GUI Multi-Line Edit Box",  "GUIMultiLineEdit", COMP_TYPE_GUI, GUI_MULTILINE_CLASSID )

ParamBlockDesc2 gGUIMultiLineEditBoxBk
(	// KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUIMultiLineEdit"), 0, &gGUIMultiLineEditDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

	2,
	plGUIControlBase::kRollMain, IDD_COMP_GUIMULTILINE, IDS_COMP_GUIMULTILINE, 0, 0, &sGUIMultiLineProc,
	plGUIControlBase::kRollProc, IDD_COMP_GUIPROCROLLOUT, IDS_COMP_GUIPROCROLLOUT, 0, 0, nil,	
	
	&sGUIControlProcParamTemplate,	

	plGUIMultiLineEditComp::kRefXparentBgnd,	_T( "xparentBgnd" ), TYPE_BOOL, 0, 0,
		p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_XPARENT,
		p_default, FALSE,
		end,
	
	plGUIMultiLineEditComp::kRefScaleWithRes,	_T( "scaleWithRes" ), TYPE_BOOL, 0, 0,
		p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_SCALERES,
		p_default, FALSE,
		end,
			
	plGUIMultiLineEditComp::kRefUseScroll,	_T( "enableScrolling" ), TYPE_BOOL, 0, 0,
		p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_SCROLLCTRL,
		p_default, FALSE,
		p_enable_ctrls, 1, plGUIMultiLineEditComp::kRefScrollCtrl,
		end,

	plGUIMultiLineEditComp::kRefScrollCtrl, _T("scrollControl"),	TYPE_INODE,		0, 0,
		p_prompt, IDS_COMP_GUI_SELECTSCROLL,
		p_accessor, &sGUIListBoxAccessor,
		end,

	end
);

plGUIMultiLineEditComp::plGUIMultiLineEditComp()
{
	fClassDesc = &gGUIMultiLineEditDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plGUIMultiLineEditComp::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	return plGUIControlBase::SetupProperties( node, pErrMsg );
}

hsBool plGUIMultiLineEditComp::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	return plGUIControlBase::PreConvert( node, pErrMsg );
}

hsBool plGUIMultiLineEditComp::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	if( !plGUIControlBase::Convert( node, pErrMsg ) )
		return false;

	pfGUIMultiLineEditCtrl *ctrl = (pfGUIMultiLineEditCtrl *)fControl;
	
	if( fCompPB->GetInt( kRefXparentBgnd ) )
		ctrl->SetFlag( pfGUIControlMod::kXparentBgnd );

	if( fCompPB->GetInt( kRefScaleWithRes ) )
		ctrl->SetFlag( pfGUIControlMod::kScaleTextWithResolution );

	if( fCompPB->GetInt( kRefUseScroll ) )
	{
		// Get the scrolling control to use
		pfGUIValueCtrl *scroll = pfGUIValueCtrl::ConvertNoRef( GrabControlMod( fCompPB->GetINode( kRefScrollCtrl ) ) );
		if( scroll != nil )
		{
			hsgResMgr::ResMgr()->AddViaNotify( scroll->GetKey(), TRACKED_NEW plGenRefMsg( ctrl->GetKey(), 
										plRefMsg::kOnCreate, -1, pfGUIMultiLineEditCtrl::kRefScrollCtrl ), plRefFlags::kActiveRef );
		}
	}

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIProgressCtrl Component ///////////////////////////////////////////////////////////////////
//
//	GUI element that can be dragged anywhere in the 2D viewing plane.
//
/////////////////////////////////////////////////////////////////////////////////////////////////


// Class that accesses the paramblock below.
class plGUIProgressCtrlComponent : public plGUIControlBase
{
protected:

	virtual pfGUIControlMod	*IGetNewControl( void ) { return TRACKED_NEW pfGUIProgressCtrl; }
	virtual bool			ICanHaveProxy( void ) { return false; }

public:
	plGUIProgressCtrlComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);

	hsBool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

	enum
	{
		kRefMinValue,
		kRefMaxValue,
		kRefStep,
		kReverseValues,
		kRefOrientation,
		kRefMouseMapping,
		kRefTriggerOnMouseUp,
		kRefAnimation,
		kRefAnimationNode,
		kRefAnimationNodeType,
		kRefAnimateSound,
		kRefAnimateSoundComp
	};
};

class plGUIProgressCtrlAccessor : public PBAccessor
{
public:
	void Set( PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t )
	{
		if( id == plGUIProgressCtrlComponent::kRefAnimateSoundComp )
		{
			plGUIProgressCtrlComponent *comp = (plGUIProgressCtrlComponent *)owner;
			comp->NotifyDependents( FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED );
		}
	}
};

static plGUIProgressCtrlAccessor	sGUIProgressCtrlAccessor;

class plGUISoundDlgProc : public ParamMap2UserDlgProc
{
protected:
	ParamID			fSoundID;
	int				fSoundItem;
	TCHAR			fTitle[ 128 ];
	
public:
	plGUISoundDlgProc( ParamID soundID, int soundItem, TCHAR *title )
	{
		fSoundID = soundID;
		fSoundItem = soundItem;
		strcpy( fTitle, title );
	}
	
	BOOL DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
	{
		switch ( msg )
		{
		case WM_INITDIALOG:
			{
				IParamBlock2 *pb = map->GetParamBlock();
				
				INode *node = pb->GetINode( fSoundID );
				TSTR newName( node ? node->GetName() : "Pick" );
				::SetWindowText( ::GetDlgItem( hWnd, fSoundItem ), newName );
			}
			return true;
			
		case WM_COMMAND:
			if( ( HIWORD( wParam ) == BN_CLICKED ) )
			{
				if( LOWORD( wParam ) == fSoundItem )
				{
					IParamBlock2 *pb = map->GetParamBlock();
					plGUICtrlHitCallback hitCB( (INode *)pb->GetOwner(), pb, fSoundID, fTitle, true, GUI_SOUND_COMPONENT_ID );
					GetCOREInterface()->DoHitByNameDialog( &hitCB );
					
					INode* node = pb->GetINode( fSoundID );
					TSTR newName( node ? node->GetName() : "Pick" );
					::SetWindowText( ::GetDlgItem(hWnd, fSoundItem ), newName );
					map->Invalidate( fSoundID );
					::InvalidateRect( hWnd, NULL, TRUE );
					return true;
				}
			}
			break;
		}
		
		return false;
	}
	
	void DeleteThis() {}
};

static plGUISoundDlgProc			sGUIProgressCtrlSndProc( plGUIProgressCtrlComponent::kRefAnimateSoundComp, IDC_GUI_ANIMSNDCOMP,
											"Select the sound to play when this control animates" );

static plPlasmaAnimSelectDlgProc	sGUIProgressCtrlProc( plGUIProgressCtrlComponent::kRefAnimation, IDC_GUI_COMPSELBTN, 
													plGUIProgressCtrlComponent::kRefAnimationNode, plGUIProgressCtrlComponent::kRefAnimationNodeType, IDC_GUI_ANIMNODESEL, 
													"Select the animation to use when displaying this knob control", &sGUIProgressCtrlSndProc );

//Max desc stuff necessary below.
CLASS_DESC(plGUIProgressCtrlComponent, gGUIProgressCtrlDesc, "GUI Progress Control",  "GUIProgressCtrl", COMP_TYPE_GUI, GUI_PROGRESS_CLASSID )

ParamBlockDesc2 gGUIProgressCtrlBk
(	// KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUIProgressCtrl"), 0, &gGUIProgressCtrlDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

	2,
	plGUIControlBase::kRollMain, IDD_COMP_GUIPROGRESS, IDS_COMP_GUIPROGRESS, 0, 0, &sGUIProgressCtrlProc,
	plGUIControlBase::kRollProc, IDD_COMP_GUIPROCROLLOUT, IDS_COMP_GUIPROCROLLOUT, 0, 0, nil,	
	
	&sGUIControlProcParamTemplate,
	
	plGUIProgressCtrlComponent::kRefMinValue, _T("minValue"),	TYPE_FLOAT,	0, 0,	
		p_default, 0.0f,
		p_ui, plGUIControlBase::kRollMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT, IDC_GUI_LOWER, IDC_GUI_LOWER_SPIN, SPIN_AUTOSCALE,
		end,

	plGUIProgressCtrlComponent::kRefMaxValue, _T("maxValue"),	TYPE_FLOAT,	0, 0,	
		p_default, 10.0f,
		p_range, -10000.f, 10000.f,			// WHY do we even need to specify this?
		p_ui, plGUIControlBase::kRollMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT, IDC_GUI_UPPER, IDC_GUI_UPPER_SPIN, SPIN_AUTOSCALE,
		end,

	plGUIProgressCtrlComponent::kRefStep, _T("step"),	TYPE_FLOAT,	0, 0,	
		p_default, 1.0f,
		p_ui, plGUIControlBase::kRollMain, TYPE_SPINNER,	EDITTYPE_POS_FLOAT, IDC_GUI_STEP, IDC_GUI_STEP_SPIN, SPIN_AUTOSCALE,
		end,

	plGUIProgressCtrlComponent::kReverseValues, _T("reverseValues"), TYPE_BOOL, 0, 0,
		p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_REVERSE,
		p_default, FALSE,
		end,

	plGUIProgressCtrlComponent::kRefAnimation, _T("animation"),	TYPE_INODE,		0, 0,
		p_prompt, IDS_COMP_GUI_SELECTANIM,
		end,

	plGUIProgressCtrlComponent::kRefAnimationNode, _T("animationNode"),	TYPE_INODE,		0, 0,
		end,

	plGUIProgressCtrlComponent::kRefAnimationNodeType, _T("animationNodeType"),	TYPE_INT,		0, 0,
		p_default, plAnimObjInterface::kUseOwnerNode,
		end,

	plGUIProgressCtrlComponent::kRefAnimateSound,	_T( "animateSound" ), TYPE_BOOL, 0, 0,
		p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_ANIMSND,
		p_default, FALSE,
		p_enable_ctrls, 1, plGUIProgressCtrlComponent::kRefAnimateSoundComp,
		end,
		
	plGUIProgressCtrlComponent::kRefAnimateSoundComp, _T("animateSoundComp"),	TYPE_INODE,		0, 0,
		p_accessor, &sGUIProgressCtrlAccessor,
		end,

	end
);

plGUIProgressCtrlComponent::plGUIProgressCtrlComponent()
{
	fClassDesc = &gGUIProgressCtrlDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plGUIProgressCtrlComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	node->SetForceLocal( true );

	plAnimObjInterface *iface = plAnimComponentBase::GetAnimInterface( fCompPB->GetINode( kRefAnimation ) );
	if( iface != nil && iface->MightRequireSeparateMaterial() )
	{
		INode *restrict = ( fCompPB->GetInt( kRefAnimationNodeType ) == plAnimObjInterface::kUseParamBlockNode ) 
								? fCompPB->GetINode( kRefAnimationNode )
								: (INode *)node;

		if( restrict != nil )
		{
			node->SetForceMaterialCopy( true );
		}
	}

	return plGUIControlBase::SetupProperties( node, pErrMsg );
}

hsBool plGUIProgressCtrlComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	return plGUIControlBase::PreConvert( node, pErrMsg );
}

// For hackery below (see warning below)
#include "../plAvatar/plAGMasterMod.h"

hsBool plGUIProgressCtrlComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	if( !plGUIControlBase::Convert( node, pErrMsg ) )
		return false;

	pfGUIProgressCtrl *ctrl = (pfGUIProgressCtrl *)fControl;
	
	ctrl->SetRange( fCompPB->GetFloat( kRefMinValue ), fCompPB->GetFloat( kRefMaxValue ) );
	ctrl->SetStep( fCompPB->GetFloat( kRefStep ) );

	if( fCompPB->GetInt( kReverseValues ) )
		ctrl->SetFlag( pfGUIProgressCtrl::kReverseValues );

	// Get the animation to use
	plAnimObjInterface *iface = plAnimComponentBase::GetAnimInterface( fCompPB->GetINode( kRefAnimation ) );
	if( iface != nil )
	{
		INode *restrict = ( fCompPB->GetInt( kRefAnimationNodeType ) == plAnimObjInterface::kUseParamBlockNode ) 
								? fCompPB->GetINode( kRefAnimationNode )
								: (INode *)node;


		hsTArray<plKey> keys;
		if( iface->GetKeyList( restrict, keys ) && keys.GetCount() > 0 )
			ctrl->SetAnimationKeys( keys, iface->GetIfaceSegmentName( false ) );
	}
	else
	{
		// HACKERY WARNING: Old knobs assumed the animation was just the same one applied to our node,
		// so to avoid breaking old formats, if we can't grab an animObjInterface, we just grab the key
		// of the master mod of our node, like we would've before
		plAGMasterMod	*master = node->GetAGMasterMod();
		hsTArray<plKey> keys;
		keys.Append( master->GetKey() );
		ctrl->SetAnimationKeys( keys, ENTIRE_ANIMATION_NAME );
	}

	const char *errMsg = ISetSoundIndex( kRefAnimateSound, kRefAnimateSoundComp, pfGUIProgressCtrl::kAnimateSound, node );
	if( errMsg != nil )
	{
		pErrMsg->Set( true, "GUI Sound Event Error", errMsg, node->GetName() ).Show();
		pErrMsg->Set( false );
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIClickMap Component ///////////////////////////////////////////////////////////////////
//
//	GUI element that just keeps track of where on its surface (from 0-1) that it was clicked.
//
/////////////////////////////////////////////////////////////////////////////////////////////////


// Class that accesses the paramblock below.
class plGUIClickMapComponent : public plGUIControlBase
{
protected:

	virtual pfGUIControlMod	*IGetNewControl( void ) { return TRACKED_NEW pfGUIClickMapCtrl; }
	virtual bool			ICanHaveProxy( void ) { return false; }

public:
	plGUIClickMapComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);

	hsBool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

	enum
	{
		kRefReportDragging
	};
};

//Max desc stuff necessary below.
CLASS_DESC(plGUIClickMapComponent, gGUIClickMapDesc, "GUI Clickable Map",  "GUIClickMap", COMP_TYPE_GUI, GUI_CLICKMAP_CLASSID )

ParamBlockDesc2 gGUIClickMapBk
(	// KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUIClickMap"), 0, &gGUIClickMapDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

	2,
	plGUIControlBase::kRollMain, IDD_COMP_GUICLICKMAP, IDS_COMP_GUICLICKMAP, 0, 0, NULL,
	plGUIControlBase::kRollProc, IDD_COMP_GUIPROCROLLOUT, IDS_COMP_GUIPROCROLLOUT, 0, 0, nil,	
	
	&sGUIControlProcParamTemplate,

	plGUIClickMapComponent::kRefReportDragging, _T("reportWhileDragging"), TYPE_BOOL, 0, 0,
		p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_REPORTDRAG,
		p_default, FALSE,
		end,

	end
);

plGUIClickMapComponent::plGUIClickMapComponent()
{
	fClassDesc = &gGUIClickMapDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plGUIClickMapComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	node->SetForceLocal( true );
	return plGUIControlBase::SetupProperties( node, pErrMsg );
}

hsBool plGUIClickMapComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	return plGUIControlBase::PreConvert( node, pErrMsg );
}

hsBool plGUIClickMapComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	if( !plGUIControlBase::Convert( node, pErrMsg ) )
		return false;

	pfGUIClickMapCtrl *ctrl = (pfGUIClickMapCtrl *)fControl;
	
	if( fCompPB->GetInt( kRefReportDragging ) )
		ctrl->SetFlag( pfGUIClickMapCtrl::kReportDragging );

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUISkin Component ////////////////////////////////////////////////////////////////////
//
//	Defines a skin to use when rendering certain GUI controls (just menus for now)
//
/////////////////////////////////////////////////////////////////////////////////////////////////


class pfGUISkinProc : public ParamMap2UserDlgProc
{
protected:

public:

	void DeleteThis() {}

//	virtual void	Update( TimeValue t, Interval &valid, IParamMap2 *map );

	BOOL DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
};

static pfGUISkinProc gGUISkinProc;

// Component defined in pfGUISkinProc.h

#define kDeclSkinRectValues( ref ) (plGUISkinComp::##ref + 0), _T("f##ref##.left"), TYPE_INT, 0, 0, p_default, 0, end, \
									(plGUISkinComp::##ref + 1), _T("f##ref##.top"), TYPE_INT, 0, 0, p_default, 0, end, \
									(plGUISkinComp::##ref + 2), _T("f##ref##.width"), TYPE_INT, 0, 0, p_default, 8, end, \
									(plGUISkinComp::##ref + 3), _T("f##ref##.height"), TYPE_INT, 0, 0, p_default, 8, end

#define kSetSkinRectValues( pb, ref, l, t, w, h ) { pb->SetValue( ref + 0, 0, (int) l ); \
													pb->SetValue( ref + 1, 0, (int) t ); \
													pb->SetValue( ref + 2, 0, (int) r ); \
													pb->SetValue( ref + 3, 0, (int) b ); }


//Max desc stuff necessary below.
CLASS_DESC(plGUISkinComp, gGUISkinDesc, "GUI Skin",  "GUISkin", COMP_TYPE_GUI, GUI_SKIN_CLASSID )

static ParamBlockDesc2	gGUISkinBk
(
	/// Main def
	 plComponent::kBlkComp, _T("GUISkin"), 0, &gGUISkinDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_GUISKIN, IDS_COMP_GUISKIN, 0, 0, &gGUISkinProc,	

	plGUISkinComp::kRefBitmap,	_T("bitmap"),		TYPE_TEXMAP, 		0, 0,
		end,

	kDeclSkinRectValues( kRefUpLeftCorner ),
	kDeclSkinRectValues( kRefTopSpan ),
	kDeclSkinRectValues( kRefUpRightCorner ),
	kDeclSkinRectValues( kRefRightSpan ),
	kDeclSkinRectValues( kRefLowerRightCorner ),
	kDeclSkinRectValues( kRefBottomSpan ),
	kDeclSkinRectValues( kRefLowerLeftCorner ),
	kDeclSkinRectValues( kRefLeftSpan ),
	kDeclSkinRectValues( kRefMiddleFill ),
	kDeclSkinRectValues( kRefSelectedFill ),
	kDeclSkinRectValues( kRefSubMenuArrow ),
	kDeclSkinRectValues( kRefSelectedSubMenuArrow ),
	kDeclSkinRectValues( kRefTreeButtonClosed ),
	kDeclSkinRectValues( kRefTreeButtonOpen ),

	plGUISkinComp::kRefItemMargin,	_T("itemMargin"),		TYPE_INT, 		0, 0,
		p_ui,	TYPE_SPINNER, EDITTYPE_POS_INT, IDC_GUI_IMARGIN, IDC_GUI_IMARGIN_SPIN, SPIN_AUTOSCALE,
		p_default, 1,
		end,

	plGUISkinComp::kRefBorderMargin,	_T("borderMargin"),		TYPE_INT, 		0, 0,
		p_ui,	TYPE_SPINNER, EDITTYPE_POS_INT, IDC_GUI_BMARGIN, IDC_GUI_BMARGIN_SPIN, SPIN_AUTOSCALE,
		p_default, 4,
		end,

	end
);

// Editor proc
extern HINSTANCE hInstance;

BOOL pfGUISkinProc::DlgProc( TimeValue t, IParamMap2 *pmap, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	IParamBlock2		*pb = pmap->GetParamBlock();
	plGUISkinComp		*comp = (plGUISkinComp *)pb->GetOwner();
	PBBitmap			*bitmap;
	plLayerTex			*layer = comp->GetSkinBitmap();
	ICustButton			*bmSelectBtn;

	switch( msg )
	{
		case WM_INITDIALOG:
			// Set projection map bitmap name
			bmSelectBtn = GetICustButton( GetDlgItem( hWnd, IDC_GUI_SKINBMAP ) );
			if( bmSelectBtn != nil )
			{
				bitmap = ( layer == nil ) ? nil : layer->GetPBBitmap();
				if( bitmap != nil )
					bmSelectBtn->SetText( (TCHAR *)bitmap->bi.Filename() );
				else
					bmSelectBtn->SetText( _T( "<none>" ) );
				ReleaseICustButton( bmSelectBtn );
			}

			return true;

		case WM_DESTROY:
			break;

		case WM_COMMAND:
			if( LOWORD( wParam ) == IDC_GUI_EDITELEM )
			{
				bitmap = ( layer == nil ) ? nil : layer->GetPBBitmap();
				if( bitmap != nil )
				{
					pfGUISkinEditProc	proc( comp );
					DialogBox( hInstance, MAKEINTRESOURCE( IDD_COMP_SKINEDIT ), GetCOREInterface()->GetMAXHWnd(), proc.DlgProc );
				}
			}

			else if( LOWORD( wParam ) == IDC_GUI_SKINBMAP )
			{
				BOOL selectedNewBitmap = layer->HandleBitmapSelection();
				if( selectedNewBitmap )
				{
					bmSelectBtn = GetICustButton( GetDlgItem( hWnd, IDC_GUI_SKINBMAP ) );
					bitmap = layer->GetPBBitmap();
					bmSelectBtn->SetText( bitmap != nil ? (TCHAR *)bitmap->bi.Filename() : "");
					ReleaseICustButton( bmSelectBtn );
				}
				return false;
			}
			break;

	}
	return false;
}

plKey	plGUISkinComp::GetConvertedSkinKey( void ) const
{
	if( fConvertedSkin != nil )
		return fConvertedSkin->GetKey();

	return nil;
}

UInt32	plGUISkinComp::GetNumMtls( void ) const
{
	return 1;
}

Texmap	*plGUISkinComp::GetMtl( UInt32 idx )
{
	return (Texmap *)GetSkinBitmap();
}

//// GetSkinBitmap ///////////////////////////////////////////////////////////

plLayerTex	*plGUISkinComp::GetSkinBitmap( void )
{  
	// If we don't have one, create one
	plLayerTex	*layer = (plLayerTex *)fCompPB->GetTexmap( kRefBitmap, 0 );
	if( layer == nil || layer->ClassID() != LAYER_TEX_CLASS_ID )
	{
		layer = TRACKED_NEW plLayerTex;

		fCompPB->SetValue( kRefBitmap, 0, (Texmap *)layer );
	}
	if( layer )
	{
		IParamBlock2* bitmapPB = layer->GetParamBlockByID( plLayerTex::kBlkBitmap );
		if( bitmapPB->GetInt(kBmpScaling) != kScalingNone )
			bitmapPB->SetValue(kBmpScaling, TimeValue(0), kScalingNone);
	}


	return layer;
}
	
plGUISkinComp::plGUISkinComp()
{
	fClassDesc = &gGUISkinDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plGUISkinComp::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	fConvertedSkin = nil;
	return true;
}

hsBool plGUISkinComp::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	// Create and assign key here, so other components can grab the key later
	if( fConvertedSkin != nil )
		return true;		// Only convert once, since we don't care what node we're on

	Texmap *texture = fCompPB->GetTexmap( kRefBitmap );
	if( texture == nil || texture->ClassID() != LAYER_TEX_CLASS_ID || ( (plLayerTex *)texture )->GetPBBitmap() == nil )
	{
		pErrMsg->Set( true, "GUI Skin Convert Error", 
							"The GUI skin component %s doesn't have a mipmap associated with it. This skin will not "
							"be exported.", GetINode()->GetName() ).CheckAndAsk();
		pErrMsg->Set( false );
		return true;
	}

	fConvertedSkin = TRACKED_NEW pfGUISkin();
	hsgResMgr::ResMgr()->NewKey( IGetUniqueName(node), fConvertedSkin, node->GetLocation() );

	return true;
}

hsBool plGUISkinComp::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	// Actually do the work of converting all the skin data
	if( fConvertedSkin == nil )
		return true;		// Eh?


	fConvertedSkin->SetMargins( fCompPB->GetInt( kRefItemMargin ), fCompPB->GetInt( kRefBorderMargin ) );

	UInt32	i;
	for( i = 0; i < pfGUISkin::kNumElements; i++ )
	{
		ParamID		id = ( i * 4 ) + kRefUpLeftCorner;

		fConvertedSkin->SetElement( i, fCompPB->GetInt( id + 0 ), fCompPB->GetInt( id + 1 ), 
									fCompPB->GetInt( id + 2 ), fCompPB->GetInt( id + 3 ) );
	}

	plLayerTex *layer= (plLayerTex *)fCompPB->GetTexmap( kRefBitmap );
	if( layer != nil )
	{
		PBBitmap *texture = layer->GetPBBitmap();
		if( texture != nil )
		{
			plBitmap *bMap = plLayerConverter::Instance().CreateSimpleTexture( texture->bi.Name(), fConvertedSkin->GetKey()->GetUoid().GetLocation(), 0, plMipmap::kForceNonCompressed | plMipmap::kAlphaChannelFlag | plMipmap::kNoMaxSize );
			if( bMap != nil && plMipmap::ConvertNoRef( bMap ) != nil )
			{
				hsgResMgr::ResMgr()->AddViaNotify( bMap->GetKey(), TRACKED_NEW plGenRefMsg( fConvertedSkin->GetKey(), 
										plRefMsg::kOnCreate, -1, pfGUISkin::kRefMipmap ), plRefFlags::kActiveRef );
			}
		}
	}

	return true;
}

hsBool plGUISkinComp::DeInit(plMaxNode *node, plErrorMsg *pErrMsg)
{
	fConvertedSkin = nil;
	return true; 
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIPopUpMenu Component ///////////////////////////////////////////////////////////////////
//
//	Defines a pop-up menu, with an auto-anchor to the sceneObject it's attached to
//
/////////////////////////////////////////////////////////////////////////////////////////////////


/*class plGUIMenuProc : public ParamMap2UserDlgProc
{
protected:

	void	ILoadPages( HWND hWnd, IParamBlock2 *pb );

public:

	void DeleteThis() {}

	BOOL DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
};
static plGUIMenuProc gGUIMenuProc;
*/

static plGUISingleCtrlDlgProc sGUISkinSelectProc( plGUIMenuComponent::kRefSkin, IDC_GUI_SKIN,
											"Select the skin to use for this pop-up menu", sSkinClassesToSelect,
											&gGUIDialogProc );

//Max desc stuff necessary below.
CLASS_DESC(plGUIMenuComponent, gGUIMenuDesc, "GUI Menu",  "GUIMenu", COMP_TYPE_GUI, GUI_MENUANCHOR_CLASSID )

ParamBlockDesc2 gGUIMenuBk
(	// KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUIMenu"), 0, &gGUIMenuDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

	3, 
	plGUIMenuComponent::kMainRollout,	IDD_COMP_GUIMENUANCHOR, IDS_COMP_GUIMENUANCHOR, 0, 0, &sGUISkinSelectProc,
	plGUIMenuComponent::kTagIDRollout,	IDD_COMP_GUITAG,	IDS_COMP_GUITAG,	0, 0, &gGUITagProc,
	plGUIMenuComponent::kSchemeRollout,	IDD_COMP_GUISCHEME, IDS_COMP_GUISCHEME, 0, 0, &gGUIColorSchemeProc,	

	&gGUIColorSchemeBk,

		plGUIMenuComponent::kRefDialogName,	_T("MenuName"),		TYPE_STRING, 		0, 0,
//			p_ui, plGUIMenuComponent::kMainRollout, TYPE_EDITBOX, IDC_GUIDLG_NAME,
			end,

		plGUIMenuComponent::kRefAgeName,	_T("ageName"),		TYPE_STRING, 		0, 0,
			p_default, _T( "GUI" ),
			end,
			
		plGUIMenuComponent::kRefVersion,	_T("version"),		TYPE_INT, 		0, 0,
			p_ui,	plGUIMenuComponent::kMainRollout, TYPE_SPINNER, EDITTYPE_POS_INT, IDC_GUI_VERSION, IDC_GUI_VERSION_SPIN, SPIN_AUTOSCALE,
			p_default, 0,
			end,

		plGUITagComponent::kRefCurrIDSel,	_T("currSel"),		TYPE_INT, 		0, 0,
			end,

		plGUIMenuComponent::kRefSkin, _T("skin"),	TYPE_INODE,		0, 0,
			end,

		plGUIMenuComponent::kRefNeverClose, _T("neverClose"),		TYPE_BOOL, 		0, 0,
			p_default,	FALSE,
			p_ui, plGUIMenuComponent::kMainRollout, TYPE_SINGLECHEKBOX, IDC_GUI_NEVERCLOSE,
			end,		

		plGUIMenuComponent::kRefModalOutside, _T("modalOutside"),		TYPE_BOOL, 		0, 0,
			p_default,	FALSE,
			p_ui, plGUIMenuComponent::kMainRollout, TYPE_SINGLECHEKBOX, IDC_GUI_MODALOUTSIDE,
			end,		

		plGUIMenuComponent::kRefOpenOnHover, _T("openSubsOnHover"),		TYPE_BOOL, 		0, 0,
			p_default,	FALSE,
			p_ui, plGUIMenuComponent::kMainRollout, TYPE_SINGLECHEKBOX, IDC_GUI_HOVER,
			end,		

		plGUIMenuComponent::kRefAlignment, _T("alignment"), TYPE_INT, 0, 0,
			p_default, 3,
			p_ui, plGUIMenuComponent::kMainRollout, TYPE_RADIO, 4, IDC_ALIGNRADIO1, IDC_ALIGNRADIO2, IDC_ALIGNRADIO3, IDC_ALIGNRADIO4,
			end,

		plGUIMenuComponent::kRefScaleWithScreenRes, _T("maintainSizeAcrossRes"),		TYPE_BOOL, 		0, 0,
			p_default,	FALSE,
			p_ui, plGUIMenuComponent::kMainRollout, TYPE_SINGLECHEKBOX, IDC_GUI_SCALERES,
			end,		

	end
);

plGUIMenuComponent::plGUIMenuComponent() : plGUIDialogComponent( true )
{
	fClassDesc = &gGUIMenuDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

pfGUIDialogMod	*plGUIMenuComponent::IMakeDialog( void )
{
	return TRACKED_NEW pfGUIPopUpMenu();
}

plKey	plGUIMenuComponent::GetConvertedMenuKey( void ) const
{
	if( fConvertedMenu == nil )
		return nil;

	return fConvertedMenu->GetKey();
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plGUIMenuComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
//	return plGUIDialogComponent::SetupProperties( node, pErrMsg );
	fConvertedMenu = nil;
	return true;
}

hsBool plGUIMenuComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	pfGUIPopUpMenu *menu = fConvertedMenu;

//	hsBool	b = plGUIDialogComponent::Convert( node, pErrMsg );
//	if( b )
	{
//		pfGUIPopUpMenu *menu = pfGUIPopUpMenu::ConvertNoRef( fDialogMod );
//		hsAssert( menu != nil, "Somehow got a bad poitner in GUIMenu::Convert()" );

		INode *sNode = fCompPB->GetINode( kRefSkin );
		if( sNode != nil )
		{
			plComponentBase *comp = ( (plMaxNode *)sNode )->ConvertToComponent();
			if( comp != nil )
			{
				Class_ID nodeID = comp->ClassID();
				hsAssert( nodeID == GUI_SKIN_CLASSID, "Bad node param in GUIMenu::Convert()" );

				plGUISkinComp *skin = (plGUISkinComp *)comp;
				menu->SetSkin( skin->GetConvertedSkin() );
			}
		}

		if( fCompPB->GetInt( kRefNeverClose ) )
			menu->SetFlag( pfGUIPopUpMenu::kStayOpenAfterClick );

		if( fCompPB->GetInt( kRefModalOutside ) )
			menu->SetFlag( pfGUIPopUpMenu::kModalOutsideMenus );

		if( fCompPB->GetInt( kRefOpenOnHover ) )
			menu->SetFlag( pfGUIPopUpMenu::kOpenSubMenusOnHover );

		if( fCompPB->GetInt( kRefScaleWithScreenRes ) )
			menu->SetFlag( pfGUIPopUpMenu::kScaleWithResolution );

		switch( fCompPB->GetInt( kRefAlignment ) )
		{
			case 0: menu->SetAlignment( pfGUIPopUpMenu::kAlignUpLeft ); break;
			case 1: menu->SetAlignment( pfGUIPopUpMenu::kAlignUpRight ); break;
			case 2: menu->SetAlignment( pfGUIPopUpMenu::kAlignDownLeft ); break;
			case 3: menu->SetAlignment( pfGUIPopUpMenu::kAlignDownRight ); break;
		}
	}

	// Note: we use the owning dialog of our anchor object as the context, i.e. who translates
	// our point at runtime into screen coordinates
	menu->SetOriginAnchor( node->GetSceneObject(), plGUIDialogComponent::GetNodeDialog( node ) );

	const plLocation &loc = menu->GetKey()->GetUoid().GetLocation();

	// Create the rendermod
	plPostEffectMod	*renderMod = TRACKED_NEW plPostEffectMod;
	hsgResMgr::ResMgr()->NewKey( IGetUniqueName(node), renderMod, loc );

	renderMod->SetHither( 0.5f );
	renderMod->SetYon( 200.f );
	renderMod->SetNodeKey( fConvertedNode );

	float scrnWidth = 20.f;

	// fovX should be such that scrnWidth is the projected width at z=100
	float fovX = atan( scrnWidth / ( 2.f * 100.f ) ) * 2.f;
	float fovY = fovX;// * 3.f / 4.f;

	renderMod->SetFovX( fovX * 180.f / hsScalarPI );
	renderMod->SetFovY( fovY * 180.f / hsScalarPI );


	hsgResMgr::ResMgr()->AddViaNotify( renderMod->GetKey(), TRACKED_NEW plNodeRefMsg( fConvertedNode, plRefMsg::kOnCreate, -1, plNodeRefMsg::kGeneric ), plRefFlags::kActiveRef );
	hsgResMgr::ResMgr()->AddViaNotify( fConvertedNode, TRACKED_NEW plGenRefMsg( renderMod->GetKey(), plRefMsg::kOnCreate, 0, plPostEffectMod::kNodeRef ), plRefFlags::kPassiveRef );		

	menu->SetRenderMod( renderMod );
	menu->SetName( fCompPB->GetStr( kRefDialogName ) );

	// Create the dummy scene object to hold the menu
	plSceneObject	*newObj = TRACKED_NEW plSceneObject;
	hsgResMgr::ResMgr()->NewKey( IGetUniqueName(node), newObj, loc );

	// *#&$(*@&#$ need a coordIface...
	plCoordinateInterface *newCI = TRACKED_NEW plCoordinateInterface;
	hsgResMgr::ResMgr()->NewKey( IGetUniqueName(node), newCI, loc );


	hsgResMgr::ResMgr()->AddViaNotify( menu->GetKey(), TRACKED_NEW plObjRefMsg( newObj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier ), plRefFlags::kActiveRef );		

	hsgResMgr::ResMgr()->AddViaNotify( newCI->GetKey(), TRACKED_NEW plObjRefMsg( newObj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kInterface ), plRefFlags::kActiveRef );		
	hsgResMgr::ResMgr()->AddViaNotify( renderMod->GetKey(), TRACKED_NEW plObjRefMsg( newObj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier ), plRefFlags::kActiveRef );		

	newObj->SetSceneNode( fConvertedNode );
	menu->SetSceneNodeKey( fConvertedNode );

	{
		hsMatrix44 l2w, w2l;
		l2w.Reset();
		l2w.GetInverse( &w2l );
		newObj->SetTransform( l2w, w2l );
	}

	// Should be done now...
	return true;
}

hsBool plGUIMenuComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
	// Create and assign key here, so other components can grab the key later
	if( fConvertedMenu != nil )
		return true;		// Only convert once, since we don't care what node we're on

	/// Create an entirely new sceneNode for us
	Int32 seqNum = plPageInfoUtils::GetSeqNumFromAgeDesc( fCompPB->GetStr( kRefAgeName ), fCompPB->GetStr( kRefDialogName ) );
	Int32 newNum = plPluginResManager::ResMgr()->VerifySeqNumber( seqNum, fCompPB->GetStr( kRefAgeName ), fCompPB->GetStr( kRefDialogName ) );
	if( newNum != seqNum )
	{
		if( !fSeqNumValidated )
		{
			char errMsg[ 512 ];
			sprintf( errMsg, "GUI Menu Component %s has an invalid location sequence number (0x%X). Temporarily using a valid one (0x%X).", 
								node->GetName(), seqNum, newNum );
			pErrMsg->Set( true, "PageInfo Convert Error", errMsg ).Show(); 
			pErrMsg->Set( false );
			fSeqNumValidated = true;
		}
		seqNum = newNum;
	}

	fConvertedNode = plPluginResManager::ResMgr()->NameToLoc( fCompPB->GetStr( kRefAgeName ), fCompPB->GetStr( kRefDialogName ), seqNum );
	if( !fConvertedNode )
	{
		pErrMsg->Set( true, "GUI Menu Component Error", "GUI MenuComponent %s has a Missing Location.  Nuke the files in the dat directory and re-export.",((INode*)node)->GetName()).Show(); 
		return false;
	}

	fConvertedMenu = TRACKED_NEW pfGUIPopUpMenu();
	hsgResMgr::ResMgr()->NewKey( IGetUniqueName(node), fConvertedMenu, fConvertedNode->GetUoid().GetLocation() );

	return true;

//	return plGUIDialogComponent::PreConvert( node, pErrMsg );
}

hsBool plGUIMenuComponent::DeInit(plMaxNode *node, plErrorMsg *pErrMsg)
{
	fConvertedMenu = nil;
	fConvertedNode = nil;
	return true;
}


