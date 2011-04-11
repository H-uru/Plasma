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
#include "hsTemplates.h"
#include <windows.h>
#include "afxres.h"
#include "res/resource.h"
#include <commctrl.h>
#include <shlwapi.h>
#include <shlobj.h>

#include "plResTreeView.h"
#include "../plResMgr/plResManager.h"
#include "../plResMgr/plResMgrSettings.h"
#include "plWinRegistryTools.h"
#include "../plFile/hsFiles.h"

#define IDC_REGTREEVIEW		1000

extern HINSTANCE gInstance;
extern char		*gCommandLine;

HWND	gTreeView;
HWND	gInfoDlg;

class plWaitCursor
{
		HCURSOR	fOrig;
	public:
		plWaitCursor()
		{
			fOrig = ::SetCursor( ::LoadCursor( nil, IDC_WAIT ) );
		}

		~plWaitCursor()
		{
			::SetCursor( fOrig );
		}
};

void	SetWindowTitle( HWND hWnd, char *path )
{
	char	fun[ MAX_PATH + 50 ];


	sprintf( fun, "plResBrowser%s%s", path != nil ? " - " : "", path != nil ? path : "" );
	SetWindowText( hWnd, fun );
}

BOOL CALLBACK AboutDialogProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	if( msg == WM_COMMAND )
		EndDialog( hWnd, 0 );
	return 0;
}

LRESULT CALLBACK HandleCommand( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
	OPENFILENAME	openInfo;
	char			fileName[ MAX_PATH ];
	char			path[ MAX_PATH ];

	static bool		filter = true;


	switch( LOWORD( wParam ) )
	{
		case ID_FILE_EXIT:
			PostQuitMessage( 0 );
			break;

		case ID_FILE_OPENDIRECTORY:

			BROWSEINFO		bInfo;
			LPITEMIDLIST	itemList;
			LPMALLOC		shMalloc;


			memset( &bInfo, 0, sizeof( bInfo ) );
			bInfo.hwndOwner = hWnd;
			bInfo.pidlRoot = NULL;
			bInfo.pszDisplayName = path;
			bInfo.lpszTitle = "Select a Plasma 2 Data Directory:";
			bInfo.ulFlags = BIF_EDITBOX;

			itemList = SHBrowseForFolder( &bInfo );
			if( itemList != NULL )
			{
				plWaitCursor	myWaitCursor;

				SHGetPathFromIDList( itemList, path );
				SHGetMalloc( &shMalloc );
				shMalloc->Free( itemList );
				shMalloc->Release();

				hsgResMgr::Reset();
				plResTreeView::ClearTreeView( gTreeView );

				// Load that source
				plResManager *mgr = (plResManager *)hsgResMgr::ResMgr();

				hsFolderIterator pathIterator(path);
				while (pathIterator.NextFileSuffix(".prp"))
				{
					char fileName[kFolderIterator_MaxPath];
					pathIterator.GetPathAndName(fileName);
					mgr->AddSinglePage(fileName);
				}
	
				plResTreeView::FillTreeViewFromRegistry( gTreeView );

				SetWindowTitle( hWnd, path );
			}

			break;

		case ID_FILE_OPEN:
			fileName[ 0 ] = 0;

			memset( &openInfo, 0, sizeof( OPENFILENAME ) );
			openInfo.hInstance = gInstance;
			openInfo.hwndOwner = hWnd;
			openInfo.lStructSize = sizeof( OPENFILENAME );
			openInfo.lpstrFile = fileName;
			openInfo.nMaxFile = sizeof( fileName );
			openInfo.lpstrFilter = "Plasma 2 Pack Files\0*.prp\0All Files\0*.*\0";
			openInfo.lpstrTitle = "Choose a file to browse:";
			openInfo.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

			if( GetOpenFileName( &openInfo ) )
			{
				plWaitCursor	myWaitCursor;
	
				hsgResMgr::Reset();
				plResTreeView::ClearTreeView( gTreeView );

				// Load that source
				plResManager *mgr = (plResManager *)hsgResMgr::ResMgr();
				mgr->AddSinglePage(fileName);
				plResTreeView::FillTreeViewFromRegistry( gTreeView );

				SetWindowTitle( hWnd, fileName );
			}

			break;

		case ID_FILE_ABOUT:
			DialogBox( gInstance, MAKEINTRESOURCE( IDD_ABOUT ), hWnd, AboutDialogProc );
			break;

		case ID_FILE_FINDOBJECT:
			plResTreeView::FindObject( gTreeView );
			break;

		case ID_FILE_FINDNEXT:
			plResTreeView::FindNextObject( gTreeView );
			break;

		case ID_FILE_VERIFYPAGE:
			plResTreeView::VerifyCurrentPage( gTreeView );
			break;

		case IDC_SHOWASHEX:
			plResTreeView::UpdateInfoDlg( gTreeView );
			break;

		case ID_FILE_ONLYLOAD:
			filter = !filter;
			plResTreeView::FilterLoadables( filter, gTreeView );
			{
				HMENU menu = ::GetMenu( hWnd );
				menu = ::GetSubMenu( menu, 0 );
				::CheckMenuItem( menu, ID_FILE_ONLYLOAD, MF_BYCOMMAND | ( filter ? MF_CHECKED : MF_UNCHECKED ) );
			}
			break;

		case ID_FILE_SAVESELECTED:
			plResTreeView::SaveSelectedObject(gTreeView);
			break;

		default:
			return DefWindowProc( hWnd, WM_COMMAND, wParam, lParam );
	}

	return 0;
}

void	SizeControls( HWND parent )
{
	RECT	clientRect, infoRect;


	GetClientRect( parent, &clientRect );
	GetClientRect( gInfoDlg, &infoRect );

	SetWindowPos( gTreeView, NULL, 0, 0, clientRect.right - infoRect.right - 4, clientRect.bottom, 0 );

	OffsetRect( &infoRect, clientRect.right - infoRect.right, ( clientRect.bottom >> 1 ) - ( infoRect.bottom >> 1 ) );
	SetWindowPos( gInfoDlg, NULL, infoRect.left, infoRect.top, 0, 0, SWP_NOSIZE );
}

void	InitWindowControls( HWND hWnd )
{
	RECT	clientRect;


	GetClientRect( hWnd, &clientRect );

	gTreeView = CreateWindowEx( WS_EX_CLIENTEDGE, WC_TREEVIEW, "Tree View", WS_VISIBLE | WS_CHILD | WS_BORDER |
							TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_SHOWSELALWAYS,
							0, 0, 0, 0,
							hWnd, (HMENU)IDC_REGTREEVIEW, gInstance, NULL );


	gInfoDlg = CreateDialog( gInstance, MAKEINTRESOURCE( IDD_INFODLG ), hWnd, plResTreeView::InfoDlgProc );

	SizeControls( hWnd );
}

static bool	sFileTypesRegistered = false;

void	RegisterFileTypes( HWND mainWnd )
{
	if( sFileTypesRegistered )
		return;

	// Make sure our file types are created
	char	path[ MAX_PATH ];

	if( ::GetModuleFileName( nil, path, sizeof( path ) ) == 0 )
		return;

	//plWinRegistryTools::AssociateFileType( "PlasmaIdxFile", "Plasma 2 Index File", path, 1 );
	//plWinRegistryTools::AssociateFileType( "PlasmaDatFile", "Plasma 2 Data File", path, 2 );
	//plWinRegistryTools::AssociateFileType( "PlasmaPatchFile", "Plasma 2 Patch File", path, 3 );
	plWinRegistryTools::AssociateFileType( "PlasmaPackFile", "Plasma 2 Packfile", path, 4 );

	// Check our file extensions
	char	prpAssoc[ 512 ];
	hsBool	needToRegister = true;
	if( plWinRegistryTools::GetCurrentFileExtensionAssociation( ".prp", prpAssoc, sizeof( prpAssoc ) ) )
	{
		if( strcmp( prpAssoc, "PlasmaPackFile" ) == 0 )
			needToRegister = false;
	}

	if( needToRegister )
	{
		if( MessageBox( nil, "The Plasma 2 packed data file extension .prp is not currently associated with "
					"plResBrowser. Would you like to associate it now?", "plResBrowser File Type Association", 
					MB_YESNO | MB_ICONQUESTION) == IDYES )
		{
			// Associate 'em
			plWinRegistryTools::AssociateFileExtension( ".prp", "PlasmaPackFile" );
		}
	}

	sFileTypesRegistered = true;
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{	
    switch( message ) 
	{		
		case WM_CREATE:
			InitCommonControls();
			InitWindowControls( hWnd );
			RegisterFileTypes( hWnd );
			plResMgrSettings::Get().SetLoadPagesOnInit(false);

			{
				plResTreeView::FilterLoadables( true, gTreeView );
				HMENU menu = ::GetMenu( hWnd );
				menu = ::GetSubMenu( menu, 0 );
				::CheckMenuItem( menu, ID_FILE_ONLYLOAD, MF_BYCOMMAND | MF_CHECKED );
			}

			if( gCommandLine != nil )
			{
				plWaitCursor	myWaitCursor;
	
				char path[ MAX_PATH ];
				if( gCommandLine[ 0 ] == '"' )
				{
					strcpy( path, gCommandLine + 1 );
					char *c = strchr( path, '"' );
					if( c != nil )
						*c = 0;
				}
				else
					strcpy( path, gCommandLine );

				if( stricmp( PathFindExtension( path ), ".prp" ) == 0 )
				{
					hsgResMgr::Reset();
					plResTreeView::ClearTreeView( gTreeView );
					plResManager *mgr = (plResManager *)hsgResMgr::ResMgr();
					mgr->AddSinglePage(path);
					plResTreeView::FillTreeViewFromRegistry( gTreeView );

					SetWindowTitle( hWnd, path );
				}
			}
			break;

        case WM_CLOSE:
			DestroyWindow( hWnd );
			break;
        case WM_DESTROY:
			plResTreeView::ClearTreeView( gTreeView );
            PostQuitMessage(0);
			break;

		case WM_SIZING:
		case WM_SIZE:
			SizeControls( hWnd );
			break;

		case WM_NOTIFY:
			if( wParam == IDC_REGTREEVIEW )
			{
				NMHDR	*hdr = (NMHDR *)lParam;
				if( hdr->code == TVN_SELCHANGED )
				{
					plResTreeView::UpdateInfoDlg( gTreeView );
					//NMTREEVIEW	*tv = (NMTREEVIEW *)hdr;

				}
				else if( hdr->code == NM_DBLCLK )
				{
					plResTreeView::SelectionDblClicked( gTreeView );
				}
			}
			break;

		case WM_DROPFILES:
			{
				int		i, j, fileCount = DragQueryFile( (HDROP)wParam, -1, nil, nil );
				char	path[ MAX_PATH ];

				plWaitCursor	myWaitCursor;

				hsgResMgr::Reset();
				plResTreeView::ClearTreeView( gTreeView );
				plResManager *mgr = (plResManager *)hsgResMgr::ResMgr();

				if( fileCount == 1 && DragQueryFile( (HDROP)wParam, 0, path, sizeof( path ) ) > 0 &&
					( (char *)PathFindExtension( path ) )[ 0 ] == 0 )
				{
					// Must be a directory
					hsFolderIterator pathIterator(path);
					while (pathIterator.NextFileSuffix(".prp"))
					{
						char fileName[kFolderIterator_MaxPath];
						pathIterator.GetPathAndName(fileName);
						mgr->AddSinglePage(fileName);
					}
				}
				else
				{
					hsTArray<char *> filesAdded;

					filesAdded.Reset();
					for( i = 0; i < fileCount; i++ )
					{
						if( DragQueryFile( (HDROP)wParam, i, path, sizeof( path ) ) > 0 )
						{
							// Check for duplicates
							for( j = 0; j < filesAdded.GetCount(); j++ )
							{
								if( stricmp( filesAdded[ j ], path ) == 0 )
									break;
							}
							if( j < filesAdded.GetCount() )
								continue;

							if( stricmp( PathFindExtension( path ), ".prp" ) == 0 )
							{
								mgr->AddSinglePage(path);
								filesAdded.Append( hsStrcpy( path ) );
							}
						}
					}

					for( j = 0; j < filesAdded.GetCount(); j++ )
						delete [] filesAdded[ j ];
				}
				plResTreeView::FillTreeViewFromRegistry( gTreeView );

				PathRemoveFileSpec( path );
				SetWindowTitle( hWnd, path );
			}
			break;

		case WM_COMMAND:
			return HandleCommand( hWnd, wParam, lParam );
    }
    
    return DefWindowProc( hWnd, message, wParam, lParam );
}
 
