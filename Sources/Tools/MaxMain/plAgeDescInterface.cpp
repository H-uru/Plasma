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
#include "plAgeDescInterface.h"
#include "max.h"
#include "resource.h"
#include "../plFile/hsFiles.h"
#include "../plAgeDescription/plAgeDescription.h"
#include "plMaxCFGFile.h"
#include "hsStream.h"
#include "hsUtils.h"
#include "../../AssetMan/PublicInterface/MaxAssInterface.h"
#include "plMaxAccelerators.h"

#include <string>
using std::string;

extern HINSTANCE hInstance;

//// Tree Data Wrapper Class //////////////////////////////////////////////////

class plAgeFile
{
protected:
	void IGetAgeName(const char* path)
	{
		char name[_MAX_FNAME];
		_splitpath(path, nil, nil, name, nil);
		fAgeName = name;
	}

public:	
	jvUniqueId fAssetID;
	string	fPath;
	string	fAgeName;

	enum Types
	{
		kAssetFile,
		kLocalFile
	};
	Types fType;

	plAgeFile(Types type) : fType(type), fPath(nil) { }
	plAgeFile(Types type, const char *path) : fType(type)
	{
		fPath = path;
		IGetAgeName(path);
	}
	plAgeFile(Types type, const char *path, jvUniqueId& id) : fType(type), fAssetID(id)
	{
		fPath = path;
		IGetAgeName(path);
	}
};

//// Static Tree Helpers //////////////////////////////////////////////////////

static HTREEITEM	SAddTreeItem( HWND hTree, HTREEITEM hParent, const char *label, int userData );
static int	SGetTreeData( HWND tree, HTREEITEM item );

static void		RemovePageItem( HWND listBox, int item )
{
	plAgePage *page = (plAgePage *)ListBox_GetItemData( listBox, item );
	delete page;
	ListBox_DeleteString( listBox, item );
}

//// Dummy Dialog Proc ////////////////////////////////////////////////////////

BOOL CALLBACK DumbDialogProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		case WM_COMMAND:
			EndDialog( hDlg, LOWORD( wParam ) );
			return TRUE;
	}
	return FALSE;
}


//// Constructor/Destructor ///////////////////////////////////////////////////

plAgeDescInterface::plAgeDescInterface() : fhDlg(nil), fDirty(false), fSpin(nil), fCurAge(-1)
{
	fCurrAgeCheckedOut = false;
	fBoldFont = nil;
	fAssetManIface = nil;

	// Make sure the date/time picker controls we need are initialized
	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_DATE_CLASSES;
	InitCommonControlsEx(&icc);
}

plAgeDescInterface::~plAgeDescInterface()
{
	IClearAgeFiles(fAgeFiles);

	DeleteObject( fBoldFont );
	DeleteObject( fHiliteBrush );
	fBoldFont = nil;

	delete fAssetManIface;
	fAssetManIface = nil;
}

plAgeDescInterface& plAgeDescInterface::Instance()
{
	static plAgeDescInterface theInstance;
	return theInstance;
}

void plAgeDescInterface::Open()
{
	if (!fhDlg)
	{
		CreateDialog(hInstance,
					MAKEINTRESOURCE(IDD_AGE_DESC),
					GetCOREInterface()->GetMAXHWnd(),
					ForwardDlgProc);

		GetCOREInterface()->RegisterDlgWnd(fhDlg);
		ShowWindow(fhDlg, SW_SHOW);
	}
}

BOOL CALLBACK plAgeDescInterface::ForwardDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return Instance().DlgProc(hDlg, msg, wParam, lParam);
}

BOOL plAgeDescInterface::DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		fhDlg = hDlg;

		if( fAssetManIface == nil )
			fAssetManIface = TRACKED_NEW MaxAssBranchAccess();

		// Make our bold font by getting the normal font and bolding
		if( fBoldFont == nil )
		{
			HFONT origFont = (HFONT)SendMessage( hDlg, WM_GETFONT, 0, 0 );
			LOGFONT	origInfo, newInfo;
			GetObject( origFont, sizeof( origInfo ), &origInfo );
			memcpy( &newInfo, &origInfo, sizeof( newInfo ) );
			newInfo.lfWeight = FW_BOLD;

			fBoldFont = CreateFontIndirect( &newInfo );
		}

		if( fHiliteBrush == nil )
			fHiliteBrush = CreateSolidBrush( RGB( 255, 0, 0 ) );

		IInitControls();
		IFillAgeTree();
		return TRUE;

	case WM_DESTROY:
		delete fAssetManIface;
		fAssetManIface = nil;
		return TRUE;

	// Day length spinner changed
	case CC_SPINNER_CHANGE:
		if (LOWORD(wParam) == IDC_DAYLEN_SPINNER ||
			LOWORD(wParam) == IDC_CAP_SPINNER ||
			LOWORD(wParam) == IDC_SEQPREFIX_SPIN )
		{
			fDirty = true;
			return TRUE;
		}
		break;

	case WM_CLOSE:
		::SendMessage( fhDlg, WM_COMMAND, IDOK, 0 );
		return true;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			if( IMakeSureCheckedIn() )
			{
				IUpdateCurAge();
				DestroyWindow(fhDlg);
				fhDlg = nil;
				fDirty = false;
				fCurAge = -1;
				fSpin = nil;
			}
			return TRUE;

//		case IDC_AGE_LIST:
//			if (HIWORD(wParam) == LBN_SELCHANGE)
//			{
//				IUpdateCurAge();
//				return TRUE;
//			}
//			break;
		
		case IDC_AGE_CHECKOUT:
			ICheckOutCurrentAge();
			return TRUE;

		case IDC_AGE_CHECKIN:
			ICheckInCurrentAge();
			return TRUE;

		case IDC_AGE_UNDOCHECKOUT:
			IUndoCheckOutCurrentAge();
			return TRUE;

		case IDC_AGE_NEW:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				INewAge();
				return TRUE;
			}
			break;

		case IDC_PAGE_NEW:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				INewPage();
				return TRUE;
			}
			break;

		case IDC_PAGE_DEL:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				HWND hPage = GetDlgItem(hDlg, IDC_PAGE_LIST);
				int sel = ListBox_GetCurSel(hPage);
				if (sel != LB_ERR)
				{
					RemovePageItem( hPage, sel );
					fDirty = true;
				}
				return TRUE;
			}
			break;

		case IDC_PAGE_LIST:
			if( HIWORD( wParam ) == LBN_SELCHANGE )
			{
				// Sel change
				HWND list = GetDlgItem( hDlg, IDC_PAGE_LIST );
				int sel = ListBox_GetCurSel( list );
				if( sel != LB_ERR )
				{
					IEnablePageControls(true);

					plAgePage *page = (plAgePage *)ListBox_GetItemData( list, sel );
					CheckDlgButton( hDlg, IDC_ADM_DONTLOAD, ( page->GetFlags() & plAgePage::kPreventAutoLoad ) ? TRUE : FALSE );
					CheckDlgButton( hDlg, IDC_ADM_LOADSDL, ( page->GetFlags() & plAgePage::kLoadIfSDLPresent ) ? TRUE : FALSE );
					CheckDlgButton( hDlg, IDC_ADM_LOCAL_ONLY, ( page->GetFlags() & plAgePage::kIsLocalOnly ) ? TRUE : FALSE );
					CheckDlgButton( hDlg, IDC_ADM_VOLATILE, ( page->GetFlags() & plAgePage::kIsVolatile ) ? TRUE : FALSE );
				}
				else
					IEnablePageControls(false);
			}
			break;

		case IDC_ADM_DONTLOAD:
		case IDC_ADM_LOADSDL:
		case IDC_ADM_LOCAL_ONLY:
		case IDC_ADM_VOLATILE:
			ICheckedPageFlag(LOWORD(wParam));
			break;

		case IDC_EDITREG:
			// Ask the user to make sure they really want to do this
			if( GetAsyncKeyState( VK_SHIFT ) & (~1) )
			{
				if( MessageBox( hDlg, "Are you sure you wish to reassign the sequence prefix for this age?", "WARNING", MB_YESNO | MB_ICONEXCLAMATION ) == IDYES )
				{
					Int32 prefix = (Int32)IGetNextFreeSequencePrefix( IsDlgButtonChecked( hDlg, IDC_RSVDCHECK ) );
					fSeqPrefixSpin->SetValue( ( prefix >= 0 ) ? prefix : -prefix, false );
					fDirty = true;
				}
			}
			else
			{
				if( MessageBox( hDlg, "Editing the registry data for an age can be extremely dangerous and "
									"can cause instabilities and crashes, particularly with multiplayer. "
									"Are you sure you want to do this?", "WARNING", MB_YESNO | MB_ICONEXCLAMATION ) == IDYES )
				{
					// Enable the controls
					EnableWindow( GetDlgItem( hDlg, IDC_RSVDCHECK ), TRUE );
					EnableWindow( GetDlgItem( hDlg, IDC_SEQPREFIX_EDIT ), TRUE );
					EnableWindow( GetDlgItem( hDlg, IDC_SEQPREFIX_SPIN ), TRUE );
				}
			}
			return TRUE;

		case IDC_RSVDCHECK:
			fDirty = true;
			return FALSE;	// Still process as normal

		case IDC_BRANCHCOMBO:
			if( HIWORD( wParam ) == CBN_SELCHANGE )
			{
				int idx = SendDlgItemMessage( hDlg, IDC_BRANCHCOMBO, CB_GETCURSEL, 0, 0 );
				if( idx != CB_ERR )
				{
					int id = SendDlgItemMessage( hDlg, IDC_BRANCHCOMBO, CB_GETITEMDATA, idx, 0 );

					fAssetManIface->SetCurrBranch( id );
					IFillAgeTree();
				}
			}
			return TRUE;
		}
		break;

	case WM_PAINT:
		PAINTSTRUCT	paintInfo;

		BeginPaint( hDlg, &paintInfo );

		if( fCurrAgeCheckedOut )
		{
			RECT	r;
			HWND	dummy = GetDlgItem( hDlg, IDC_AGEDESC );
			GetClientRect( dummy, &r );
			MapWindowPoints( dummy, hDlg, (POINT *)&r, 2 );

			for( int i = 0; i < 3; i++ )
			{
				InflateRect( &r, -1, -1 );
				FrameRect( paintInfo.hdc, &r, fHiliteBrush );
			}
		}

		EndPaint( hDlg, &paintInfo );
		return TRUE;

	case WM_NOTIFY:
		{
			NMHDR *hdr = (NMHDR*)lParam;

			// Message from the start date/time controls
			if (hdr->idFrom == IDC_DATE || hdr->idFrom == IDC_TIME)
			{
				if (hdr->code == NM_KILLFOCUS)
				{
					plMaxAccelerators::Enable();
					return TRUE;
				}
				else if (hdr->code == NM_SETFOCUS)
				{
					plMaxAccelerators::Disable();
					return TRUE;
				}
				// Time or date changed, set dirty
				else if (hdr->code == DTN_DATETIMECHANGE)
				{
					fDirty = true;
					return TRUE;
				}	
			}
			else if( hdr->idFrom == IDC_AGE_LIST )
			{
				if( hdr->code == NM_CUSTOMDRAW )
				{
					// Custom draw notifications for our treeView control
					LPNMTVCUSTOMDRAW	treeNotify = (LPNMTVCUSTOMDRAW)lParam;

					if( treeNotify->nmcd.dwDrawStage == CDDS_PREPAINT )
					{
						// Sent at the start of redraw, lets us request more specific notifys
						SetWindowLong( hDlg, DWL_MSGRESULT, CDRF_NOTIFYITEMDRAW );
						return TRUE;
					}
					else if( treeNotify->nmcd.dwDrawStage == CDDS_ITEMPREPAINT )
					{
						// Prepaint on an item. We get to change item font and color here
						int idx = SGetTreeData( hdr->hwndFrom, (HTREEITEM)treeNotify->nmcd.dwItemSpec );
/*						if( item == nil || item->fType != plAgeFile::kBranch )
						{
							// Default drawing, with default font and such
							SetWindowLong( hDlg, DWL_MSGRESULT, CDRF_DODEFAULT );
							return TRUE;
						}		
*/
						// Color change (only if the item isn't selected)
						if( (HTREEITEM)treeNotify->nmcd.dwItemSpec != TreeView_GetSelection( hdr->hwndFrom ) )
						{
							treeNotify->clrText = GetColorManager()->GetColor( kText );
							treeNotify->clrTextBk = GetColorManager()->GetColor( kWindow );
						}

						if (idx == -1)
						{
							// Set a bold font for the branch headers
							if( fBoldFont != nil )
								SelectObject( treeNotify->nmcd.hdc, fBoldFont );
						}

						// Let Windows know we changed the font
						SetWindowLong( hDlg, DWL_MSGRESULT, CDRF_NEWFONT );
						return TRUE;
					}
					else
						// Let the default handle it
						return FALSE;
				}
				else if( hdr->code == TVN_SELCHANGING )
				{
					SetWindowLong( hDlg, DWL_MSGRESULT, !IMakeSureCheckedIn() );
					return TRUE;
				}
				else if( hdr->code == TVN_SELCHANGED )
				{
					// Update the viewing age
					IUpdateCurAge();
					return TRUE;
				}
			}
		}
		break;
	}

	return FALSE;
}

void plAgeDescInterface::ICheckedPageFlag(int ctrlID)
{
	HWND hList = GetDlgItem(fhDlg, IDC_PAGE_LIST);
	int sel = ListBox_GetCurSel(hList);
	if (sel == LB_ERR)
		return;

	plAgePage* page = (plAgePage*)ListBox_GetItemData(hList, sel);
	bool isChecked = (IsDlgButtonChecked(fhDlg, ctrlID) == BST_CHECKED);

	if (ctrlID == IDC_ADM_DONTLOAD)
	{
		if (isChecked)
		{
			page->SetFlags(plAgePage::kPreventAutoLoad, true);

			// Doesn't make sense to have loadWithSDL checked too
			CheckDlgButton(fhDlg, IDC_ADM_LOADSDL, FALSE);
			page->SetFlags(plAgePage::kLoadIfSDLPresent, false);
		}
		else
			page->SetFlags(plAgePage::kPreventAutoLoad, false);
	}
	else if (ctrlID == IDC_ADM_LOADSDL)
	{
		if (isChecked)
		{
			page->SetFlags(plAgePage::kLoadIfSDLPresent, true);

			// Doesn't make sense to have dontLoad checked too
			CheckDlgButton(fhDlg, IDC_ADM_DONTLOAD, FALSE);
			page->SetFlags(plAgePage::kPreventAutoLoad, false);
		}
		else
			page->SetFlags(plAgePage::kLoadIfSDLPresent, false);
	}
	else if (ctrlID == IDC_ADM_LOCAL_ONLY)
	{
		page->SetFlags(plAgePage::kIsLocalOnly, isChecked);
	}
	else if (ctrlID == IDC_ADM_VOLATILE)
	{
		page->SetFlags(plAgePage::kIsVolatile, isChecked);
	}
}

void	plAgeDescInterface::ICheckOutCurrentAge( void )
{
	hsAssert( !fCurrAgeCheckedOut, "Trying to re-check out an age!" );

	plAgeFile *currAge = IGetCurrentAge();
	if( currAge == nil )
	{
		hsAssert( false, "How are you checking out an age if none is selected?" );
		return;
	}

	if( currAge->fType != plAgeFile::kAssetFile )
		return;

	// Check it out from AssetMan
	bool checkOutSuccess = (*fAssetManIface)->CheckOutAsset( currAge->fAssetID, fCheckedOutPath, sizeof( fCheckedOutPath ) );
	if( !checkOutSuccess )
	{
		hsMessageBox( "Unable to check out age file from AssetMan. Most likely somebody already has it checked out.", "Error", hsMessageBoxNormal );
		return;
	}
	fCurrAgeCheckedOut = true;

	// Make sure we loaded the latest version
	ILoadAge( fCheckedOutPath, true );

	IInvalidateCheckOutIndicator();
	IEnableControls( true );
}

void	plAgeDescInterface::ICheckInCurrentAge( void )
{
	hsAssert( fCurrAgeCheckedOut, "Trying to check in an age when none is checked out!" );

	plAgeFile *currAge = IGetCurrentAge();
	if( currAge == nil )
	{
		hsAssert( false, "How are you checking in an age if none is selected?" );
		return;
	}

	// Save the sucker
	ISaveCurAge( fCheckedOutPath );

	// Check the age file back in to AssetMan
	(*fAssetManIface)->CheckInAsset( currAge->fAssetID, fCheckedOutPath, "" );
	fCurrAgeCheckedOut = false;

	IInvalidateCheckOutIndicator();
	IEnableControls( true );
}

void	plAgeDescInterface::IUndoCheckOutCurrentAge( void )
{
	hsAssert( fCurrAgeCheckedOut, "Trying to undo check out an age when none is checked out!" );

	plAgeFile *currAge = IGetCurrentAge();
	if( currAge == nil )
	{
		hsAssert( false, "How are you undoing a checkout if no age is selected?" );
		return;
	}

	// Check the age file back in to AssetMan
	(*fAssetManIface)->UndoCheckOutAsset(currAge->fAssetID);
	fCurrAgeCheckedOut = false;

	// Reload the non-saved version
	ILoadAge( fCheckedOutPath );

	IInvalidateCheckOutIndicator();
	IEnableControls( true );
}

void	plAgeDescInterface::IInvalidateCheckOutIndicator( void )
{
	RECT	r;
	HWND	dummy = GetDlgItem( fhDlg, IDC_AGEDESC );
	GetClientRect( dummy, &r );
	MapWindowPoints( dummy, fhDlg, (POINT *)&r, 2 );

	RedrawWindow( fhDlg, &r, nil, RDW_INVALIDATE | RDW_ERASE );
}

hsBool	plAgeDescInterface::IMakeSureCheckedIn( void )
{
	int result;
	plAgeFile* currAge = IGetCurrentAge();
	if (!currAge)
		return true;

	if ( currAge->fType == plAgeFile::kAssetFile && fCurrAgeCheckedOut )
	{
		// We're switching away from an age we have checked out--ask what they want to do
		result = DialogBox( hInstance, MAKEINTRESOURCE( IDD_AGE_CHECKIN ),
								GetCOREInterface()->GetMAXHWnd(), DumbDialogProc );
		if( result == IDCANCEL )
		{
			// Got cancelled
			return false;
		}
		else if( result == IDYES )
		{
			ICheckInCurrentAge();
		}
		else
		{
			IUndoCheckOutCurrentAge();
		}
	}
	else if( currAge->fType == plAgeFile::kLocalFile && fDirty )
	{
		// Ask if we want to save changes
		result = DialogBox( hInstance, MAKEINTRESOURCE( IDD_AGE_SAVEYESNO ),
								GetCOREInterface()->GetMAXHWnd(), DumbDialogProc );
		if( result == IDCANCEL )
		{
			// Got cancelled
			return false;
		}
		else if( result == IDYES )
		{
			ISaveCurAge( currAge->fPath.c_str() );
		}
		else
		{
			// Reload the non-saved version
			ILoadAge( currAge->fPath.c_str() );
		}
		IEnableControls( true );
	}
	return true;
}

void	plAgeDescInterface::IUpdateCurAge( void )
{
	// Get the current age selection
	plAgeFile	*currAge = IGetCurrentAge();

	if (currAge == nil)
	{
		ISetControlDefaults();
		IEnableControls( false );
		return;
	}

	IEnableControls( true );

	if( currAge->fType == plAgeFile::kAssetFile )
	{
		// Gotta get the latest version from assetMan before loading
		char				localFilename[ MAX_PATH ];
		if( !(*fAssetManIface)->GetLatestVersionFile( currAge->fAssetID, localFilename, sizeof( localFilename ) ) )
		{
			hsMessageBox( "Unable to get latest version of age asset from AssetMan. Things are about to get very funky...", "ERROR", hsMessageBoxNormal );
		}
		else
		{
			ILoadAge( localFilename );
		}
	}
	else
		// Load the local age, also check its sequence #s
		ILoadAge( currAge->fPath.c_str(), true );
}

static const int kDefaultCapacity = 10;

void plAgeDescInterface::IInitControls()
{
	// Fill the branch combo box
	SendDlgItemMessage( fhDlg, IDC_BRANCHCOMBO, CB_RESETCONTENT, 0, 0 );
	const jvTypeArray &branches = (*fAssetManIface)->GetBranches();
	int i, curr = 0;
	for( i = 0; i < branches.Size(); i++ )
	{
		int idx = SendDlgItemMessage( fhDlg, IDC_BRANCHCOMBO, CB_ADDSTRING, 0, (LPARAM)(const char *)( branches[ i ].Name ) );
		SendDlgItemMessage( fhDlg, IDC_BRANCHCOMBO, CB_SETITEMDATA, idx, (LPARAM)branches[ i ].Id );

		if( branches[ i ].Id == fAssetManIface->GetCurrBranch() )
			curr = i;
	}
	SendDlgItemMessage( fhDlg, IDC_BRANCHCOMBO, CB_SETCURSEL, curr, 0 );


	fSpin = SetupFloatSpinner(fhDlg, IDC_DAYLEN_SPINNER, IDC_DAYLEN_EDIT, 1.f, 100.f, 24.f);
	fCapSpin = SetupIntSpinner(fhDlg, IDC_CAP_SPINNER, IDC_CAP_EDIT, 1, 250, kDefaultCapacity);
	fSeqPrefixSpin = SetupIntSpinner(fhDlg, IDC_SEQPREFIX_SPIN, IDC_SEQPREFIX_EDIT, 1, 102400, 1);

	SendDlgItemMessage( fhDlg, IDC_AGELIST_STATIC, WM_SETFONT, (WPARAM)fBoldFont, MAKELPARAM( TRUE, 0 ) );
	SendDlgItemMessage( fhDlg, IDC_AGEDESC, WM_SETFONT, (WPARAM)fBoldFont, MAKELPARAM( TRUE, 0 ) );

	ISetControlDefaults();
	IEnableControls(false);
}

void plAgeDescInterface::ISetControlDefaults()
{
	HWND hDate = GetDlgItem(fhDlg, IDC_DATE);
	HWND hTime = GetDlgItem(fhDlg, IDC_TIME);

	SYSTEMTIME st = {0};
	st.wDay = 1;
	st.wMonth = 1;
	st.wYear = 2000;
	DateTime_SetSystemtime(hDate, GDT_VALID, &st);
	DateTime_SetSystemtime(hTime, GDT_VALID, &st);

	fSpin->SetValue(24.f, FALSE);
	fCapSpin->SetValue(kDefaultCapacity, FALSE);

	CheckDlgButton( fhDlg, IDC_ADM_DONTLOAD, FALSE );

	int i;
	HWND ctrl = GetDlgItem( fhDlg, IDC_PAGE_LIST );
	for( i = SendMessage( ctrl, LB_GETCOUNT, 0, 0 ) - 1; i >= 0; i-- )
		RemovePageItem( ctrl, i );

	SetDlgItemText( fhDlg, IDC_AGEDESC, "Age Description" );
}

void plAgeDescInterface::IEnableControls(bool enable)
{
	bool checkedOut = true;

	plAgeFile *currAge = IGetCurrentAge();
	if( currAge != nil && currAge->fType == plAgeFile::kAssetFile )
		checkedOut = fCurrAgeCheckedOut && enable;
	else if( currAge != nil && currAge->fType == plAgeFile::kLocalFile )
		checkedOut = enable;
	else
		checkedOut = false;

	EnableWindow(GetDlgItem(fhDlg, IDC_DATE), checkedOut );
	EnableWindow(GetDlgItem(fhDlg, IDC_TIME), checkedOut );
	EnableWindow(GetDlgItem(fhDlg, IDC_PAGE_LIST), checkedOut );
	EnableWindow(GetDlgItem(fhDlg, IDC_PAGE_NEW), checkedOut );
	EnableWindow(GetDlgItem(fhDlg, IDC_PAGE_DEL), checkedOut );
	EnableWindow(GetDlgItem(fhDlg, IDC_EDITREG), checkedOut );

	if (!enable || !checkedOut)
		IEnablePageControls(false);

	fSpin->Enable(checkedOut );
	fCapSpin->Enable(checkedOut );

	if( currAge != nil && currAge->fType == plAgeFile::kAssetFile )
	{
		EnableWindow( GetDlgItem( fhDlg, IDC_AGE_CHECKIN ), checkedOut );
		EnableWindow( GetDlgItem( fhDlg, IDC_AGE_UNDOCHECKOUT ), checkedOut );
		EnableWindow( GetDlgItem( fhDlg, IDC_AGE_CHECKOUT ), !checkedOut );
	}
	else
	{
		EnableWindow( GetDlgItem( fhDlg, IDC_AGE_CHECKIN ), false );
		EnableWindow( GetDlgItem( fhDlg, IDC_AGE_UNDOCHECKOUT ), false );
		EnableWindow( GetDlgItem( fhDlg, IDC_AGE_CHECKOUT ), false );
	}
}

void plAgeDescInterface::IEnablePageControls(bool enable)
{
	EnableWindow(GetDlgItem(fhDlg, IDC_ADM_DONTLOAD), enable);
	EnableWindow(GetDlgItem(fhDlg, IDC_ADM_LOADSDL), enable);
	EnableWindow(GetDlgItem(fhDlg, IDC_ADM_LOCAL_ONLY), enable);
	EnableWindow(GetDlgItem(fhDlg, IDC_ADM_VOLATILE), enable);
}

bool plAgeDescInterface::IGetLocalAgePath(char *path)
{
	// Get the path to the description folder
	const char *plasmaPath = plMaxConfig::GetClientPath();
	if (!plasmaPath)
		return false;

	strcpy(path, plasmaPath);
	strcat(path, plAgeDescription::kAgeDescPath);

	// Make sure the desc folder exists
	CreateDirectory(path, NULL);

	return true;
}

int plAgeDescInterface::IFindAge(const char* ageName, vector<plAgeFile*>& ageFiles)
{
	for (int i = 0; i < ageFiles.size(); i++)
		if (ageFiles[i]->fAgeName == ageName)
			return i;

	return -1;
}

void plAgeDescInterface::IGetAgeFiles(vector<plAgeFile*>& ageFiles)
{
	IClearAgeFiles(ageFiles);

	char agePath[MAX_PATH];

	// Make list of "local" ages. This might contain copies of those in AssetMan, so we make the
	// list first and take out the ones that are in AssetMan
	char localPath[MAX_PATH];
	if (IGetLocalAgePath(localPath))
	{
		hsFolderIterator ageFolder(localPath);
		while (ageFolder.NextFileSuffix(".age"))
		{
			ageFolder.GetPathAndName(agePath);

			plAgeFile* age = TRACKED_NEW plAgeFile(plAgeFile::kLocalFile, agePath);
			ageFiles.push_back(age);
		}
	}

	// Add AssetMan ages, if available (since we're static, go thru the main MaxAss interface)
	MaxAssInterface *assetMan = GetMaxAssInterface();
	if( assetMan!= nil )
	{
		hsTArray<jvUniqueId> doneAssets;

		jvArray<jvUniqueId>* assets = assetMan->GetAssetsByType(MaxAssInterface::kTypeAge);
		for (int i = 0; i < assets->Size(); i++)
		{
			if( doneAssets.Find( (*assets)[ i ] ) == doneAssets.kMissingIndex )
			{
				if (assetMan->GetLatestVersionFile((*assets)[i], agePath, sizeof(agePath)))
				{
					plAgeFile* age = TRACKED_NEW plAgeFile(plAgeFile::kAssetFile, agePath, (*assets)[i]);

					int existing = IFindAge(age->fAgeName.c_str(), ageFiles);
					// Remove it from our "local" list if there, since it's a duplicate
					if (existing != -1)
					{
						delete ageFiles[existing];
						ageFiles[existing] = age;
					}
					else
						ageFiles.push_back(age);

					doneAssets.Append( (*assets)[ i ] );
				}
			}
		}
		assets->DeleteSelf();
	}
}

void plAgeDescInterface::IClearAgeFiles(vector<plAgeFile*>& ageFiles)
{
	for (int i = 0; i < ageFiles.size(); i++)
		delete ageFiles[i];
	ageFiles.clear();
}

void plAgeDescInterface::BuildAgeFileList( hsTArray<char *> &ageList )
{
	vector<plAgeFile*> tempAgeFiles;
	IGetAgeFiles(tempAgeFiles);

	for (int i = 0; i < tempAgeFiles.size(); i++)
	{
		ageList.Push(hsStrcpy(tempAgeFiles[i]->fPath.c_str()));
		delete tempAgeFiles[ i ];
	}
}

//// IFillAgeTree /////////////////////////////////////////////////////////////
//	Refreshes/inits the tree view of all ages we have to work with. If
//	specified, will also get the latest version of the .age files from assetMan.
void plAgeDescInterface::IFillAgeTree( void )
{
	HWND ageTree = GetDlgItem( fhDlg, IDC_AGE_LIST );

	// Clear the tree first and add our two root headers
	TreeView_DeleteAllItems(ageTree);

	if( fAssetManIface != nil )
		fAssetManBranch = SAddTreeItem(ageTree, nil, "AssetMan Ages", -1);
	else
		fAssetManBranch = nil;
	fLocalBranch = SAddTreeItem(ageTree, nil, "Local Ages", -1);

	IGetAgeFiles(fAgeFiles);

	// Add the ages to the tree
	for (int i = 0; i < fAgeFiles.size(); i++)
	{
		SAddTreeItem(ageTree,
					(fAgeFiles[i]->fType == plAgeFile::kAssetFile) ? fAssetManBranch : fLocalBranch,
					fAgeFiles[i]->fAgeName.c_str(),
					i);
	}

	// Select the first age to view
	IUpdateCurAge();
}

BOOL CALLBACK NewAgeDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static char *name = nil;

	switch (msg)
	{
	case WM_INITDIALOG:
		name = (char*)lParam;
		SetWindowText(hDlg, name);
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDOK)
		{
			if (GetDlgItemText(hDlg, IDC_AGE_NAME, name, _MAX_FNAME) > 0)
				EndDialog(hDlg, 1);
			else
				EndDialog(hDlg, 0);
			return TRUE;
		}
		else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, 0);
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CALLBACK NewSeqNumberProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static char msg1[] = "This age currently does not have a sequence number assigned to it. All ages "
						 "must have a unique sequence number for multiplayer to work. Unassigned ages "
						 "will get a temporary random number at export time, but will result in undefined "
						 "(but probably bad) behavior during multiplayer games. In general, you should "
						 "always assign a sequence number to an age unless you have a very specific and "
						 "good reason not to.";
	static char msg2[] = "The ADManager can find and assign a new, unique sequence number to this age for "
						 "you. You can choose to assign a normal or a global/reserved number. Normal ages "
						 "are ones for gameplay (that you can link to, walk around in, etc.), while "
						 "global/reserved ages are typically for storing data (such as avatars, GUI dialogs, etc.)";
	char	msg3[ 512 ];


	switch (msg)
	{
	case WM_INITDIALOG:
		SetDlgItemText( hDlg, IDC_INFOMSG, msg1 );
		SetDlgItemText( hDlg, IDC_ADMMSG, msg2 );
		sprintf( msg3, "Age: %s", (char *)lParam );
		SetDlgItemText( hDlg, IDC_AGEMSG, msg3 );
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED )
		{
			EndDialog( hDlg, LOWORD( wParam ) );
			return TRUE;
		}
	}

	return FALSE;
}

void plAgeDescInterface::INewAge()
{  
	VARIANT assetId;
	VariantInit(&assetId);

	bool makeAsset = true;
	if( hsMessageBox( "Do you wish to store your new age in AssetMan?", "Make source-controlled?", hsMessageBoxYesNo ) == hsMBoxNo )
		makeAsset = false;

	char	newAssetFilename[ MAX_PATH ];
	if (!fAssetManIface)
		makeAsset = false;

	if( !IGetLocalAgePath( newAssetFilename ) )
		return;

	char name[_MAX_FNAME];
	strcpy(name, "New Age Name");

	// Get the name of the new age from the user
	int ret = DialogBoxParam(hInstance,
			   MAKEINTRESOURCE(IDD_AGE_NAME),
			   GetCOREInterface()->GetMAXHWnd(),
			   NewAgeDlgProc,
			   (LPARAM)name);
	if (ret != 1)
		return;

	strcat(newAssetFilename, name);
	strcat(newAssetFilename, ".age");

	if( !makeAsset )
		fForceSeqNumLocal = true;
	ISetControlDefaults();
	ISaveCurAge(newAssetFilename, true);	// Check sequence # while we're at it
	fForceSeqNumLocal = false;

	if( makeAsset )
		(*fAssetManIface)->AddNewAsset(newAssetFilename);

	// Refresh the tree now
	IFillAgeTree();
}

void plAgeDescInterface::INewPage()
{
	char name[256];
	strcpy(name, "New Page Name");

	// Get the name of the new age from the user
	int ret = DialogBoxParam(hInstance,
							MAKEINTRESOURCE(IDD_AGE_NAME),
							GetCOREInterface()->GetMAXHWnd(),
							NewAgeDlgProc,
							(LPARAM)name);
	if (ret != 1)
		return;

	HWND hPages = GetDlgItem(fhDlg, IDC_PAGE_LIST);

	// Make sure this page doesn't already exist
	int count = ListBox_GetCount(hPages);
	for (int i = 0; i < count; i++)
	{
		char pageName[256];
		ListBox_GetText(hPages, i, pageName);
		if (!strcmp(pageName, name))
			return;
	}

	// Add the new page and select it
	int idx = ListBox_AddString(hPages, name);

	// Choose a new sequence suffix for it
	plAgePage *newPage = TRACKED_NEW plAgePage( name, IGetFreePageSeqSuffix( hPages ), 0 );
	ListBox_SetItemData( hPages, idx, (LPARAM)newPage );
   
	fDirty = true;
}

UInt32	plAgeDescInterface::IGetFreePageSeqSuffix( HWND pageCombo )
{
	int		i, count = ListBox_GetCount( pageCombo );
	UInt32	searchSeq = 1;

	do
	{
		for( i = 0; i < count; i++ )
		{
			plAgePage *page = (plAgePage *)ListBox_GetItemData( pageCombo, i );
			if( page != nil && page->GetSeqSuffix() == searchSeq )
			{
				searchSeq++;
				break;
			}
		}

	} while( i < count );

	return searchSeq;
}

void plAgeDescInterface::ISaveCurAge( const char *path, hsBool checkSeqNum )
{
	hsUNIXStream s;
	if( !s.Open( path, "wt" ) )
	{
		hsMessageBox("Unable to open the Age Description file for writing. Updates not saved.", "Error", hsMessageBoxNormal);
		return;
	}

	plAgeDescription aged;
	aged.SetAgeNameFromPath( path );

	// set the date and time
	HWND hDate = GetDlgItem(fhDlg, IDC_DATE);
	SYSTEMTIME dst = {0};
	DateTime_GetSystemtime(hDate, &dst);
	HWND hTime = GetDlgItem(fhDlg, IDC_TIME);
	SYSTEMTIME tst = {0};
	DateTime_GetSystemtime(hTime, &tst);
	aged.SetStart(dst.wYear,dst.wMonth,dst.wDay,tst.wHour,tst.wMinute,tst.wSecond);
	aged.SetDayLength(fSpin->GetFVal());
	aged.SetMaxCapacity(fCapSpin->GetIVal());
	if( checkSeqNum )
	{
		ICheckSequenceNumber( aged );
	}
	else if( IsDlgButtonChecked( fhDlg, IDC_RSVDCHECK ) )
	{
		// Store reserved sequence prefix
		aged.SetSequencePrefix( -fSeqPrefixSpin->GetIVal() );
	}
	else
	{
		aged.SetSequencePrefix( fSeqPrefixSpin->GetIVal() );
	}

	// gather pages
	HWND hPages = GetDlgItem(fhDlg, IDC_PAGE_LIST);
	int count = ListBox_GetCount(hPages);
	if (count != LB_ERR)
	{
		for (int i = 0; i < count; i++)
		{
			char pageName[256];
			ListBox_GetText(hPages, i, pageName);
			plAgePage *page = (plAgePage *)ListBox_GetItemData( hPages, i );
			aged.AppendPage( pageName, page->GetSeqSuffix(), page->GetFlags() );
		}
	}

	// write it all out
	aged.Write(&s);
	s.Close();
}

//// ICheckSequenceNumber /////////////////////////////////////////////////////
//	Checks to make sure the sequence prefix is valid. If not, asks to assign
//	a good one.

void	plAgeDescInterface::ICheckSequenceNumber( plAgeDescription &aged )
{
	if( aged.GetSequencePrefix() == 0 )	// Default of uninitialized
	{
   		// Ask about the sequence #
		int ret = DialogBoxParam( hInstance, MAKEINTRESOURCE( IDD_AGE_SEQNUM ),
									GetCOREInterface()->GetMAXHWnd(),
										NewSeqNumberProc, (LPARAM)aged.GetAgeName() );
		if( ret == IDYES )
		{
			aged.SetSequencePrefix( IGetNextFreeSequencePrefix( false ) );
			fDirty = true;
		}
		else if( ret == IDNO )
		{
			aged.SetSequencePrefix( IGetNextFreeSequencePrefix( true ) );
			fDirty = true;
		}
	}
}

void plAgeDescInterface::ILoadAge( const char *path, hsBool checkSeqNum )
{
	ISetControlDefaults();
	
	fDirty = false;

	// create and read the age desc
	plAgeDescription aged( path );

	// Get the name of the age
	char ageName[_MAX_FNAME];
	_splitpath( path, nil, nil, ageName, nil );

	// Check the sequence prefix #
	if( checkSeqNum )
		ICheckSequenceNumber( aged );

	char str[ _MAX_FNAME + 30 ];
	sprintf( str, "Description for %s", ageName );
	SetDlgItemText( fhDlg, IDC_AGEDESC, str );

	// Set up the Dlgs
	SYSTEMTIME st;

	HWND hTime = GetDlgItem(fhDlg, IDC_TIME);
	memset(&st,0, sizeof(st));
	st.wYear = 2000;
	st.wMonth = 1;
	st.wDay = 1;
	st.wHour = aged.GetStartHour();
	st.wMinute = aged.GetStartMinute();
	st.wSecond = aged.GetStartSecond();
	DateTime_SetSystemtime(hTime, GDT_VALID, &st);

	
	HWND hDate = GetDlgItem(fhDlg, IDC_DATE);
	memset(&st,0, sizeof(st));
	st.wMonth = aged.GetStartMonth();
	st.wDay = aged.GetStartDay();
	st.wYear = aged.GetStartYear();
	DateTime_SetSystemtime(hDate, GDT_VALID, &st);

	
	fSpin->SetValue(aged.GetDayLength(), FALSE);

	int maxCap = aged.GetMaxCapacity();
	if (maxCap == -1)
	{
		maxCap = kDefaultCapacity;
		fDirty = true;
	}
	fCapSpin->SetValue(maxCap, FALSE);

	Int32 seqPrefix = aged.GetSequencePrefix();
	if( seqPrefix < 0 )
	{
		// Reserved prefix
		fSeqPrefixSpin->SetValue( (int)( -seqPrefix ), FALSE );
		CheckDlgButton( fhDlg, IDC_RSVDCHECK, BST_CHECKED );
	}
	else
	{
		fSeqPrefixSpin->SetValue( (int)seqPrefix, FALSE );
		CheckDlgButton( fhDlg, IDC_RSVDCHECK, BST_UNCHECKED );
	}

	// Disable the registry controls for now
	EnableWindow( GetDlgItem( fhDlg, IDC_RSVDCHECK ), false );
	EnableWindow( GetDlgItem( fhDlg, IDC_SEQPREFIX_EDIT ), false );
	EnableWindow( GetDlgItem( fhDlg, IDC_SEQPREFIX_SPIN ), false );

	aged.SeekFirstPage();
	plAgePage *page;

	HWND hPage = GetDlgItem(fhDlg, IDC_PAGE_LIST);
	while( ( page = aged.GetNextPage() ) != nil )
	{
		int idx = ListBox_AddString( hPage, page->GetName() );
		ListBox_SetItemData( hPage, idx, (LPARAM)new plAgePage( *page ) );
	}
}

UInt32	plAgeDescInterface::IGetNextFreeSequencePrefix( hsBool getReservedPrefix )
{
	Int32				searchSeq = getReservedPrefix ? -1 : 1;
	hsTArray<char *>	ageList;
	int					i;

	hsTArray<plAgeDescription>	ages;


	if( fForceSeqNumLocal )
		searchSeq = getReservedPrefix ? -1024 : 1024;

	IGetAgeFiles(fAgeFiles);

	ages.SetCount( fAgeFiles.size() );
	for( i = 0; i < fAgeFiles.size(); i++ )
	{
		hsUNIXStream stream;
		if( stream.Open( fAgeFiles[ i ]->fPath.c_str(), "rt" ) )
		{
			ages[ i ].Read( &stream );
			stream.Close();
		}
	}

	do
	{
		if( getReservedPrefix )
		{
			for( i = 0; i < ages.GetCount(); i++ )
			{
				if( ages[ i ].GetSequencePrefix() == searchSeq )
				{
					searchSeq--;
					break;
				}
			}
		}
		else
		{
			for( i = 0; i < ages.GetCount(); i++ )
			{
				if( ages[ i ].GetSequencePrefix() == searchSeq )
				{
					searchSeq++;
					break;
				}
			}
		}
	} while( i < ages.GetCount() );

	return searchSeq;
}

plAgeFile* plAgeDescInterface::IGetCurrentAge( void )
{
	HWND	ageTree = GetDlgItem( fhDlg, IDC_AGE_LIST );
	fCurrAgeItem = TreeView_GetSelection( ageTree );
	if( fCurrAgeItem == nil )
		return nil;

	int idx = SGetTreeData( ageTree, fCurrAgeItem );
	if (idx == -1)
		return nil;

	return fAgeFiles[idx];
}

//// SAddTreeItem /////////////////////////////////////////////////////////////
//	Static helper function for adding an item to a treeView

static HTREEITEM SAddTreeItem( HWND hTree, HTREEITEM hParent, const char *label, int userData )
{
	TVITEM tvi = {0};
	tvi.mask       = TVIF_TEXT | TVIF_PARAM;
	tvi.pszText    = (char *)label;
	tvi.cchTextMax = strlen( label );  
	tvi.lParam     = (LPARAM)userData;
	if( userData == -1 )
	{
		tvi.mask |= TVIF_STATE;
		tvi.state = tvi.stateMask = TVIS_BOLD | TVIS_EXPANDED;
	}

	TVINSERTSTRUCT tvins = {0};
	tvins.item         = tvi;
	tvins.hParent      = hParent;
	tvins.hInsertAfter = TVI_LAST;

	return TreeView_InsertItem( hTree, &tvins );
}

//// SGetTreeData /////////////////////////////////////////////////////////////
//	Gets the tree data for the given item

int SGetTreeData( HWND tree, HTREEITEM item )
{
	TVITEM	itemInfo;
	itemInfo.mask = TVIF_PARAM | TVIF_HANDLE;
	itemInfo.hItem = item;
	TreeView_GetItem( tree, &itemInfo );
	return itemInfo.lParam;
}
