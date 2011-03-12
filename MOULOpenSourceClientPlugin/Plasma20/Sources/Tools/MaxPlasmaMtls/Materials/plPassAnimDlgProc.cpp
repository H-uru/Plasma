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
//	plPassAnimDlgProc - Base Animation Dlg Proc for plPassMtlBase			//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "plPassAnimDlgProc.h"

#include "plPassBaseParamIDs.h"
#include "../MaxComponent/plNotetrackAnim.h"
#include "resource.h"
#include "iparamm2.h"

#include "plAnimStealthNode.h"

#include "../MaxComponent/plMaxAnimUtils.h"
#include "../MaxComponent/plAnimComponent.h"
#include "../MaxExport/plErrorMsg.h"

const char *kPassNameNone = ENTIRE_ANIMATION_NAME;

#include "plAnimStealthNode.h"

using namespace plPassBaseParamIDs;

plPassAnimDlgProc::plPassAnimDlgProc()
{
	fCurrParamMap = nil;
	fInitingNames = false;
}

plPassAnimDlgProc::~plPassAnimDlgProc()
{
	if( fCurrParamMap != nil )
	{
		plPassMtlBase *mtl = (plPassMtlBase *)( fCurrParamMap->GetParamBlock()->GetOwner() );
		mtl->RegisterChangeCallback( this );
	}
}

plPassAnimDlgProc	&plPassAnimDlgProc::Get( void )
{
	static plPassAnimDlgProc	instance;
	return instance;
}

void plPassAnimDlgProc::Update(TimeValue t, Interval& valid, IParamMap2* pmap)
{
/*	plAnimStealthNode *testStealth = (plAnimStealthNode *)pmap->GetParamBlock()->GetINode( (ParamID)kPBAnimTESTING );
	if( testStealth != nil )
	{
		IParamBlock2 *pb = testStealth->GetParamBlockByID( plAnimStealthNode::kBlockPB );
		if( pb && pb->GetMap() && pb->GetMap()->GetUserDlgProc() )
			pb->GetMap()->GetUserDlgProc()->Update( t, valid, pmap );
	}
*/

	HWND hWnd = pmap->GetHWnd();
	IParamBlock2 *pb = pmap->GetParamBlock();
	plAnimComponentProc::SetBoxToAgeGlobal(GetDlgItem(hWnd, IDC_MTL_GLOBAL_NAME), pb->GetStr(ParamID(kPBAnimGlobalName))); 	
}

BOOL plPassAnimDlgProc::DlgProc(TimeValue t, IParamMap2 *pMap, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if( fCurrParamMap != pMap )
	{
		if( fCurrParamMap != nil )
		{
			plPassMtlBase *mtl = (plPassMtlBase *)( fCurrParamMap->GetParamBlock()->GetOwner() );
			mtl->UnregisterChangeCallback( this );
		}

		fCurrParamMap = pMap;

		if( fCurrParamMap != nil )
		{
			plPassMtlBase *mtl = (plPassMtlBase *)( fCurrParamMap->GetParamBlock()->GetOwner() );
			mtl->RegisterChangeCallback( this );
		}
	}

	IParamBlock2 *pb = pMap->GetParamBlock();
	plPassMtlBase *mtl = (plPassMtlBase*)pb->GetOwner();
	HWND gWnd = GetDlgItem(hWnd, IDC_MTL_GLOBAL_NAME);	
	char buff[512];

	switch (msg)
	{
		case WM_DESTROY:
			if( fCurrParamMap != nil )
			{
				plPassMtlBase *mtl = (plPassMtlBase *)( fCurrParamMap->GetParamBlock()->GetOwner() );
				mtl->RegisterChangeCallback( this );
				fCurrParamMap = nil;
			}
			break;

		case WM_INITDIALOG:
			{
				fhWnd = hWnd;
				fCurrStealth = nil;
				IInitControls(mtl, pb);

				plAnimComponentProc::FillAgeGlobalComboBox(gWnd, pb->GetStr(ParamID(kPBAnimGlobalName)));							
				plAnimComponentProc::SetBoxToAgeGlobal(gWnd, pb->GetStr(ParamID(kPBAnimGlobalName))); 	
				IEnableGlobal(hWnd, pb->GetInt( (ParamID)kPBAnimUseGlobal ) );		

				bool stopPoints = false;
				if( DoesHaveStopPoints( pb->GetOwner() ) )
				{
					stopPoints = true;
					break;
				}
				
				IEnableEaseStopPoints( pMap, stopPoints );
			}
			return TRUE;

		case WM_COMMAND:
			// Anim name selection changed
			if (LOWORD(wParam) == IDC_NAMES && HIWORD(wParam) == CBN_SELCHANGE)
			{
				IUpdateSegmentSel( pMap );
				return TRUE;
			}
			// Refresh clicked
			else if (LOWORD(wParam) == IDC_REFRESH_ANIMS && HIWORD(wParam) == BN_CLICKED)
			{
				IInitControls(mtl, pb);
				return TRUE;
			}
			else if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_MTL_GLOBAL_NAME)
			{
				ComboBox_GetLBText(gWnd, ComboBox_GetCurSel(gWnd), buff);
				pb->SetValue(ParamID(kPBAnimGlobalName), 0, _T(buff));
			}			
			else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_MTL_USE_GLOBAL)
			{
				IEnableGlobal(hWnd, pb->GetInt( (ParamID)kPBAnimUseGlobal ) );
			}

			break;
	}

	return FALSE;
}

void	plPassAnimDlgProc::SegmentListChanged( void )
{
	if( fCurrParamMap != nil )
		ILoadNames( fCurrParamMap->GetParamBlock() );
}

void	plPassAnimDlgProc::IUpdateSegmentSel( IParamMap2 *thisMap, hsBool clear )
{
	plAnimStealthNode *newStealth;
	HWND hAnims = GetDlgItem( fhWnd, IDC_NAMES );
	
	// Get current selection
	if( clear )
		newStealth = nil;
	else
	{
		int sel = SendDlgItemMessage( fhWnd, IDC_NAMES, CB_GETCURSEL, 0, 0 );
		if( sel == CB_ERR )
		{
			// Somehow we don't have a selection...fine, just destroy
			newStealth = nil;
		}
		else
		{
			newStealth = (plAnimStealthNode *)SendDlgItemMessage( fhWnd, IDC_NAMES, CB_GETITEMDATA, sel, 0 );
		}
	}

	// Did we really not change?
	if( newStealth == fCurrStealth )
		return;

	if( fCurrStealth != nil && newStealth != nil )
	{
		fCurrStealth->SwitchDlg( newStealth );
	}
	else
	{
		// Destroy the old
		if( fCurrStealth != nil )
			fCurrStealth->ReleaseDlg();

		// Show the new
		if( newStealth != nil )
			IExposeStealthNode( newStealth, thisMap );
	}

	// And update!
	fCurrStealth = newStealth;
}


void	plPassAnimDlgProc::IExposeStealthNode( HelperObject *node, IParamMap2 *thisMap )
{
	if( node->ClassID() != ANIMSTEALTH_CLASSID )
		return;

	// Get our stealth pointer
	plAnimStealthNode *stealth = (plAnimStealthNode *)node;

	// Create the paramMap-based dialog for us
	IParamBlock2 *pb = thisMap->GetParamBlock();
	plPassMtlBase *mtl = (plPassMtlBase *)pb->GetOwner();

	if( !stealth->CreateAndEmbedDlg( thisMap, mtl->fIMtlParams, GetDlgItem( fhWnd, IDC_PLACEHOLDER ) ) )
	{
	}
}


void plPassAnimDlgProc::SetThing(ReferenceTarget *m)
{
	plPassMtlBase *mtl = (plPassMtlBase*)m;
	IInitControls(mtl, mtl->fAnimPB );
}

void plPassAnimDlgProc::IInitControls(Animatable *anim, IParamBlock2 *pb)
{
	ILoadNames( pb );
	IEnableGlobal( fhWnd, pb->GetInt( ParamID( kPBAnimUseGlobal ) ) );		
}

void plPassAnimDlgProc::ILoadNames(IParamBlock2 *pb )
{
	// The following is to prevent IGetNumStealths() from re-calling us
	if( fInitingNames )
		return;
	fInitingNames = true;

	HWND hAnims = GetDlgItem(fhWnd, IDC_NAMES);
	SendMessage(hAnims, CB_RESETCONTENT, 0, 0);

	plPassMtlBase *mtl = (plPassMtlBase *)pb->GetOwner();

	// Loop through our stealth nodes and add them all to the combo,
	// since that's what we're selecting...erm, yeah
	int i, count = mtl->IGetNumStealths( true );
	for( i = 0; i < count; i++ )
	{
		plAnimStealthNode *stealth = mtl->IGetStealth( i, false );
		if( stealth != nil )
		{
			int idx = SendMessage( hAnims, CB_ADDSTRING, 0, (LPARAM)stealth->GetSegmentName() );
			SendMessage( hAnims, CB_SETITEMDATA, idx, (LPARAM)stealth );
		}
	}

	SendMessage( hAnims, CB_SETCURSEL, 0, 0 );
	IUpdateSegmentSel( pb->GetMap() );

	fInitingNames = false;
}

void plPassAnimDlgProc::IEnableGlobal(HWND hWnd, hsBool enable)
{
	Edit_Enable(GetDlgItem(hWnd, IDC_MTL_GLOBAL_NAME), enable);
	ComboBox_Enable(GetDlgItem(hWnd, IDC_NAMES), !enable);

	HWND stealthWnd = ( fCurrStealth != nil ) ? fCurrStealth->GetWinDlg() : nil;
	if( stealthWnd != nil )
		EnableWindow( stealthWnd, !enable );
}

void	plPassAnimDlgProc::IEnableEaseStopPoints( IParamMap2 *pm, bool enable )
{
	pm->Enable( (ParamID)kPBAnimEaseInMin, enable );
	pm->Enable( (ParamID)kPBAnimEaseInMax, enable );
	pm->Enable( (ParamID)kPBAnimEaseOutMin, enable );
	pm->Enable( (ParamID)kPBAnimEaseOutMax, enable );
}


