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
#include "hsWindows.h"
#include "plResTreeView.h"

#include "../plResMgr/plResManager.h"
#include "../plResMgr/plRegistryHelpers.h"
#include "../plResMgr/plRegistryNode.h"
#include "../plResMgr/plPageInfo.h"
#include "../pnKeyedObject/plUoid.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnKeyedObject/plKeyImp.h"
#include "../pnFactory/plFactory.h"

#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include "res\resource.h"


extern HINSTANCE		gInstance;
HWND	plResTreeView::fInfoDlg = nil;
bool	plResTreeView::fFilter = false;

static char			gSearchString[ 512 ];
static HTREEITEM	fFoundItem = nil;

extern void	ViewPatchDetails( plKey &patchKey );


BOOL CALLBACK	FindDialogProc( HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		case WM_INITDIALOG:
			return true;

		case WM_COMMAND:
			if( LOWORD( wParam ) == IDOK )
			{
				GetDlgItemText( dlg, IDC_SEARCHSTRING, gSearchString, sizeof( gSearchString ) );
				fFoundItem = nil;
				EndDialog( dlg, IDOK );
			}
			else if( LOWORD( wParam ) == IDCANCEL )
				EndDialog( dlg, IDCANCEL );

			return -1;
	}

	return 0;
}


struct plKeyInfo
{
	plKey				fKey;
	plRegistryPageNode	*fPage;

	plKeyInfo( plKey k, plRegistryPageNode *p ) : fKey( k ), fPage( p ) {}
};

// How's this for functionality?
class plResDlgLoader : public plRegistryPageIterator, public plRegistryKeyIterator
{
	protected:

		HWND		fTree;
		HTREEITEM	fCurrItem, fCurrTypeItem;
		UInt16		fCurrType;
		bool		fFilter;

		plRegistryPageNode	*fCurrPage;


		HTREEITEM AddLeaf(HWND hTree, HTREEITEM hParent, const char *text, plKeyInfo *info )
		{
			TVITEM tvi = {0};
			tvi.mask       = TVIF_TEXT | TVIF_PARAM;
			tvi.pszText    = text ? (char*)text : "<NULL>";
			tvi.cchTextMax = text ? strlen(text) : 7;
			tvi.lParam     = (LPARAM)info;

			TVINSERTSTRUCT tvins = {0};
			tvins.item         = tvi;
			tvins.hParent      = hParent;
			tvins.hInsertAfter = TVI_SORT;

			return TreeView_InsertItem(hTree, &tvins);
		}

	public:

		plResDlgLoader( HWND hTree, bool filter )
		{
			fFilter = filter;
			fTree = hTree;
			((plResManager *)hsgResMgr::ResMgr())->IterateAllPages( this );
		}

		virtual hsBool	EatPage( plRegistryPageNode *page )
		{
			char	str[ 512 ];


			fCurrPage = page;
			const plPageInfo &info = page->GetPageInfo();
			sprintf( str, "%s->%s->%s", info.GetAge(), info.GetPage() );
			fCurrItem = AddLeaf( fTree, NULL, str, new plKeyInfo( nil, fCurrPage ) );

			fCurrType = (UInt16)-1;
			page->LoadKeys();
			page->IterateKeys( this );
			return true;
		}

		virtual hsBool	EatKey( const plKey& key )
		{
			if( fCurrType != key->GetUoid().GetClassType() )
			{
				fCurrType = key->GetUoid().GetClassType();
				const char *className = plFactory::GetNameOfClass( fCurrType );
				fCurrTypeItem = AddLeaf( fTree, fCurrItem, className != nil ? className : "<unknown>", nil );
			}

			if( !fFilter )
				AddLeaf( fTree, fCurrTypeItem, key->GetUoid().GetObjectName(), new plKeyInfo( key, fCurrPage ) );
			return true;
		}
};

void	plResTreeView::FillTreeViewFromRegistry( HWND hWnd )
{
	plResDlgLoader loader( hWnd, fFilter );
}

HTREEITEM	IGetNextTreeItem( HWND tree, HTREEITEM item )
{
	// First try child items of this one
	HTREEITEM next = TreeView_GetChild( tree, item );
	if( next == nil )
		// If no child items, try next sibling
		next = TreeView_GetNextSibling( tree, item );
	if( next == nil )
	{
		// If no siblings, go up to the parent and keep searching until we find a parent with a sibling
		next = item;
		while( true )
		{
			next = TreeView_GetParent( tree, next );
			if( next == nil )
			{
				// No parent; not found, so stop
				break;
			}
			else if( TreeView_GetNextSibling( tree, next ) != nil )
			{
				next = TreeView_GetNextSibling( tree, next );
				break;
			}
		}
	}

	return next;
}

void	plResTreeView::FindNextObject( HWND tree )
{
	if( fFoundItem == nil )
		FindObject( tree );
	else
	{
		fFoundItem = IGetNextTreeItem( tree, fFoundItem );
		IFindNextObject( tree );
	}
}

void	plResTreeView::FindObject( HWND tree )
{
	if( DialogBox( gInstance, MAKEINTRESOURCE( IDD_FINDOBJ ), tree, FindDialogProc ) == IDOK )
	{
		fFoundItem = TreeView_GetRoot( tree );
		IFindNextObject( tree );
	}
}

void	plResTreeView::IFindNextObject( HWND tree )
{
	while( fFoundItem != nil )
	{
		// Get the item
		TVITEM	itemInfo;
		itemInfo.mask = TVIF_PARAM | TVIF_HANDLE;
		itemInfo.hItem = fFoundItem;
		TreeView_GetItem( tree, &itemInfo );
		plKeyInfo *keyInfo = (plKeyInfo *)itemInfo.lParam;
		if( keyInfo != nil && keyInfo->fKey != nil )
		{
			if( StrStrI( keyInfo->fKey->GetUoid().GetObjectName(), gSearchString ) != nil )
			{
				/// FOUND
				TreeView_SelectItem( tree, fFoundItem );
				return;
			}
		}

		// Keep searching. First try child items of this one
		fFoundItem = IGetNextTreeItem( tree, fFoundItem );
	}

	MessageBox( tree, "No objects found", "Find Object", MB_OK );
}

void	IDeleteRecurse( HWND tree, HTREEITEM item )
{
	while( item != nil )
	{
		HTREEITEM	child = TreeView_GetChild( tree, item );
		if( child != nil )
			IDeleteRecurse( tree, child );

		TVITEM	itemInfo;
		itemInfo.mask = TVIF_PARAM | TVIF_HANDLE;
		itemInfo.hItem = item;
		TreeView_GetItem( tree, &itemInfo );
		plKeyInfo *keyInfo = (plKeyInfo *)itemInfo.lParam;
		if( keyInfo != nil )
		{
			delete keyInfo;
			itemInfo.lParam = 0;
			TreeView_SetItem( tree, &itemInfo );
		}

		item = TreeView_GetNextSibling( tree, item );
	}
}

void	plResTreeView::ClearTreeView( HWND hWnd )
{
	HTREEITEM	root = TreeView_GetRoot( hWnd );
	if( root != nil )
		IDeleteRecurse( hWnd, root );

	TreeView_DeleteAllItems( hWnd );
}

void	plResTreeView::SelectionDblClicked( HWND treeCtrl )
{
	HTREEITEM	sel = TreeView_GetSelection( treeCtrl );
	if( sel != nil )
	{
		TVITEM	item;

		item.mask = TVIF_PARAM | TVIF_HANDLE;
		item.hItem = sel;
		if( !TreeView_GetItem( treeCtrl, &item ) )
			return;
	}
}

void	plResTreeView::FilterLoadables( bool filter, HWND treeCtrl )
{
	fFilter = filter;
	ClearTreeView( treeCtrl );
	FillTreeViewFromRegistry( treeCtrl );
}

BOOL CALLBACK	plResTreeView::InfoDlgProc( HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		case WM_INITDIALOG:
			fInfoDlg = dlg;
			break;

		case WM_COMMAND:
			return SendMessage( GetParent( dlg ), msg, wParam, lParam );			
	}

	return 0;
}

void	plResTreeView::VerifyCurrentPage( HWND treeCtrl )
{
	HTREEITEM	sel = TreeView_GetSelection( treeCtrl );
	if( sel != nil )
	{
		TVITEM	item;

		item.mask = TVIF_PARAM | TVIF_HANDLE;
		item.hItem = sel;
		if( !TreeView_GetItem( treeCtrl, &item ) )
			return;

		plKeyInfo *info = (plKeyInfo *)item.lParam;
		if( info != nil )
		{
			if( info->fPage != nil )
			{
				// TODO: FIXME
				/*
				/// HACK. Live with it
				class plHackResManager : public plResManager
				{
					public:
						plRegistry *GetRegistry( void ) { return IGetRegistry(); }
				};

				plRegistry *registry = ((plHackResManager *)hsgResMgr::ResMgr())->GetRegistry();

				plRegistry::plPageCond result = registry->VerifyOnePage( info->fPage );

				char msg[ 512 ];
				if( result == plRegistry::kOK || result == plRegistry::kTooNew )
					strcpy( msg, "Page verifies OK" );
				else if( result == plRegistry::kChecksumInvalid )
					strcpy( msg, "Checksums for page are invalid" );
				else if( result == plRegistry::kOutOfDate )
					strcpy( msg, "Page is older than the current data version" );

				hsMessageBox( msg, "Verification Results", hsMessageBoxNormal );
				*/
			}
		}
	}
}

void	plResTreeView::UpdateInfoDlg( HWND treeCtrl )
{
	hsBool showAsHex = (hsBool)IsDlgButtonChecked( fInfoDlg, IDC_SHOWASHEX );

	SetDlgItemText( fInfoDlg, IDC_NAME, "" );
	SetDlgItemText( fInfoDlg, IDC_CLASS, "" );
	SetDlgItemText( fInfoDlg, IDC_LENGTH, "" );
	SetDlgItemText( fInfoDlg, IDC_STARTPOS, "" );
	EnableWindow( GetDlgItem( fInfoDlg, ID_FILE_VERIFYPAGE ), FALSE );

	HTREEITEM	sel = TreeView_GetSelection( treeCtrl );
	if( sel != nil )
	{
		TVITEM	item;

		item.mask = TVIF_PARAM | TVIF_HANDLE;
		item.hItem = sel;
		if( !TreeView_GetItem( treeCtrl, &item ) )
			return;

		plKeyInfo *info = (plKeyInfo *)item.lParam;
		if( info != nil )
		{
			if( info->fPage != nil )
			{
				const plPageInfo	&pageInfo = info->fPage->GetPageInfo();
				char				tempStr[ 32 ];

				SetDlgItemText( fInfoDlg, IDC_AGE, pageInfo.GetAge() );
				SetDlgItemText( fInfoDlg, IDC_PAGE, pageInfo.GetPage() );

				SetDlgItemText( fInfoDlg, IDC_LOCATION, pageInfo.GetLocation().StringIze( tempStr ) );

				CheckDlgButton(fInfoDlg, IDC_RESERVED,   (pageInfo.GetLocation().GetFlags() & plLocation::kReserved) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(fInfoDlg, IDC_BUILTIN,    (pageInfo.GetLocation().GetFlags() & plLocation::kBuiltIn) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(fInfoDlg, IDC_VOLATILE,	 (pageInfo.GetLocation().GetFlags() & plLocation::kVolatile) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(fInfoDlg, IDC_LOCAL_ONLY, (pageInfo.GetLocation().GetFlags() & plLocation::kLocalOnly) ? BST_CHECKED : BST_UNCHECKED);

				sprintf( tempStr, "%d", pageInfo.GetMajorVersion());
				SetDlgItemText( fInfoDlg, IDC_DATAVERSION, tempStr );

				SetDlgItemText( fInfoDlg, IDC_CHECKSUMTYPE, "Basic (file size)" );
				EnableWindow( GetDlgItem( fInfoDlg, ID_FILE_VERIFYPAGE ), TRUE );
			}

			if( info->fKey != nil )
			{
				char	str[ 128 ];


				SetDlgItemText( fInfoDlg, IDC_NAME, info->fKey->GetUoid().GetObjectName() );

				const char *name = plFactory::GetNameOfClass( info->fKey->GetUoid().GetClassType() );
				sprintf( str, "%s (%d)", name != nil ? name : "<unknown>", info->fKey->GetUoid().GetClassType() );
				SetDlgItemText( fInfoDlg, IDC_CLASS, str );

				plKeyImp *imp = (plKeyImp *)info->fKey;
				EnableWindow( GetDlgItem( fInfoDlg, IDC_STARTPOS_LABEL ), true );
				EnableWindow( GetDlgItem( fInfoDlg, IDC_SIZE_LABEL ), true );

				if( showAsHex )
					sprintf( str, "0x%X", imp->GetStartPos() );
				else
					sprintf( str, "%d", imp->GetStartPos() );
				SetDlgItemText( fInfoDlg, IDC_STARTPOS, str );

				if( imp->GetDataLen() < 1024 )
					sprintf( str, "%d bytes", imp->GetDataLen() );
				else if( imp->GetDataLen() < 1024 * 1024 )
					sprintf( str, "%4.2f kB", imp->GetDataLen() / 1024.f );
				else
					sprintf( str, "%4.2f MB", imp->GetDataLen() / 1024.f / 1024.f );

				SetDlgItemText( fInfoDlg, IDC_LENGTH, str );
			}
		}
	}
}

#include "hsStream.h"
#include <commdlg.h>

void plResTreeView::SaveSelectedObject(HWND treeCtrl)
{
	// TODO: FIXME
	/*
	plKey itemKey = nil;

	HTREEITEM sel = TreeView_GetSelection(treeCtrl);
	if (sel != nil)
	{
		TVITEM	item;
		item.mask = TVIF_PARAM | TVIF_HANDLE;
		item.hItem = sel;
		if (TreeView_GetItem(treeCtrl, &item))
		{
			plKeyInfo *info = (plKeyInfo*)item.lParam;
			if (info != nil)
				itemKey = info->fKey;
		}
	}

	if (!itemKey)
		return;

	char fileName[MAX_PATH];
	sprintf(fileName, "%s.bin", itemKey->GetName());

	OPENFILENAME openInfo;
	memset( &openInfo, 0, sizeof( OPENFILENAME ) );
//	openInfo.hInstance = gInstance;
//	openInfo.hwndOwner = hWnd;
	openInfo.lStructSize = sizeof( OPENFILENAME );
	openInfo.lpstrFile = fileName;
	openInfo.nMaxFile = sizeof( fileName );
	openInfo.lpstrFilter = "Binary Files\0*.bin\0All Files\0*.*\0";
//	openInfo.lpstrTitle = "Choose a pack index file to browse:";
//	openInfo.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

	if (GetSaveFileName(&openInfo))
	{
		plKeyImp* keyImp = (plKeyImp*)itemKey;

		if (keyImp->GetDataLen() <= 0)
			return;

		plResManager* resMgr = (plResManager*)hsgResMgr::ResMgr();
		const plPageInfo& pageInfo = resMgr->FindPage(keyImp->GetUoid().GetLocation())->GetPageInfo();

		plRegistryDataStream *stream = registry->OpenPageDataStream( keyImp->GetUoid().GetLocation(), false );
		if( stream == nil )
			return;

		hsStream	*dataStream = stream->GetStream();
		UInt8		*buffer = TRACKED_NEW UInt8[ keyImp->GetDataLen() ];
		if( buffer != nil )
		{
			dataStream->SetPosition( keyImp->GetStartPos() );
			dataStream->Read( keyImp->GetDataLen(), buffer );
		}
		delete stream;

		if( buffer == nil )
			return;

		hsUNIXStream outStream;
		outStream.Open(fileName, "wb");
		outStream.Write(keyImp->GetDataLen(), buffer);
		outStream.Close();

		delete [] buffer;
	}
	*/
}

