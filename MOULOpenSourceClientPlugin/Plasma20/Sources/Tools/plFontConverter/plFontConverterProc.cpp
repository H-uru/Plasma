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
#include "hsTypes.h"
#include <windows.h>
#include "res/resource.h"
#include <shlwapi.h>
#include <shlobj.h>
#include <commctrl.h>
#include <time.h>

#include "hsStream.h"
#include "hsResMgr.h"
#include "plFontFreeType.h"
#include "../plGImage/plFont.h"
#include "../plGImage/plMipmap.h"
#include "../pnKeyedObject/plUoid.h"
#include "../pnKeyedObject/plKeyImp.h"


extern HINSTANCE	gInstance;

// My global font that i'm working on
plFont	*gFont = nil;

// Preview bitmap
HDC		gPreviewHDC = nil;
HBITMAP	gPreviewBitmap = nil;

void	IMakeFontGoAway( void )
{
	if( gFont != nil )
	{
		plKeyImp *imp = (plKeyImp *)(gFont->GetKey());
		if( imp != nil )
			imp->SetObjectPtr( nil );
		gFont = nil;
	}
}

void	IMakeNewFont( void )
{
	IMakeFontGoAway();
	gFont = new plFont();
}

BOOL CALLBACK AboutDialogProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	if( msg == WM_COMMAND )
		EndDialog( hWnd, 0 );
	return 0;
}

bool	PromptForFile( HWND parent, const char *prompt, const char *filter, char *fileName, int fileNameMax, bool save )
{
	OPENFILENAME	openInfo;


	memset( &openInfo, 0, sizeof( OPENFILENAME ) );
	openInfo.hInstance = gInstance;
	openInfo.hwndOwner = parent;
	openInfo.lStructSize = sizeof( OPENFILENAME );
	openInfo.lpstrFile = fileName;
	openInfo.nMaxFile = fileNameMax;
	openInfo.lpstrFilter = filter;
	openInfo.lpstrTitle = prompt;
	openInfo.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	if( !save )
		openInfo.Flags |= OFN_READONLY;

	if( save )
		return GetSaveFileName( &openInfo ) ? true : false;

	return GetOpenFileName( &openInfo ) ? true : false;
}

void	IUpdateInfo( HWND hDlg )
{
	const int TEST_STRING_SIZE = 512;
	static wchar_t testString[ TEST_STRING_SIZE ] = L"The quick brown fox jumped over the lazy dog! ABCabc012345;,.";

	if( gFont == nil )
	{
		SetDlgItemText( hDlg, IDC_FACE, "" );
		SetDlgItemText( hDlg, IDC_FSIZE, "" );
		SetDlgItemText( hDlg, IDC_STARTG, "" );
		SetDlgItemText( hDlg, IDC_GCOUNT, "" );

		SetDlgItemText( hDlg, IDC_WIDTH, "" );
		SetDlgItemText( hDlg, IDC_HEIGHT, "" );
		SetDlgItemText( hDlg, IDC_BPP, "" );

		CheckDlgButton( hDlg, IDC_BOLD, false );
		CheckDlgButton( hDlg, IDC_ITALIC, false );
		return;
	}

	SetDlgItemText( hDlg, IDC_FACE, gFont->GetFace() );
	SetDlgItemInt( hDlg, IDC_FSIZE, gFont->GetSize(), false );
	SetDlgItemInt( hDlg, IDC_STARTG, gFont->GetFirstChar(), false );
	SetDlgItemInt( hDlg, IDC_GCOUNT, gFont->GetNumChars(), false );

	SetDlgItemInt( hDlg, IDC_WIDTH, gFont->GetBitmapWidth(), false );
	SetDlgItemInt( hDlg, IDC_HEIGHT, gFont->GetBitmapHeight(), false );
	SetDlgItemInt( hDlg, IDC_BPP, gFont->GetBitmapBPP(), false );

	CheckDlgButton( hDlg, IDC_BOLD, gFont->IsFlagSet( plFont::kFlagBold ) );
	CheckDlgButton( hDlg, IDC_ITALIC, gFont->IsFlagSet( plFont::kFlagItalic ) );

	if( gPreviewHDC != nil )
	{
		DeleteObject( gPreviewHDC );
		DeleteObject( gPreviewBitmap );
		gPreviewHDC = nil;
		gPreviewBitmap = nil;
	}

	// Get the size of our preview
	RECT r;
	GetClientRect( GetDlgItem( hDlg, IDC_PREVIEW ), &r );
	MapWindowPoints( GetDlgItem( hDlg, IDC_PREVIEW ), hDlg, (POINT *)&r, 2 );

	InvalidateRect( hDlg, &r, false );

	if( gFont->GetNumChars() == 0 )
		return;

	// Our preview bitmap
	HDC deskDC = GetDC( nil );
	gPreviewHDC = CreateCompatibleDC( deskDC );
	gPreviewBitmap = CreateCompatibleBitmap( deskDC, r.right - r.left, r.bottom - r.top );
	SelectObject( gPreviewHDC, gPreviewBitmap );
	ReleaseDC( nil, deskDC );

	::GetDlgItemTextW( hDlg, IDC_PREVTEXT, testString, TEST_STRING_SIZE );

	// Create us a mipmap to render onto, render onto it, then copy that to our DC
	plMipmap *mip = new plMipmap( r.right - r.left, r.bottom - r.top, plMipmap::kARGB32Config, 1 );
	memset( mip->GetImage(), 0xff, mip->GetWidth() * mip->GetHeight() * 4 );

	gFont->SetRenderColor( 0xff000000 );
	gFont->SetRenderFlag( plFont::kRenderClip, true );
	gFont->SetRenderClipRect( 0, 0, (Int16)(r.right - r.left), (Int16)(r.bottom - r.top) );
	UInt16 w, h, a, lastX, lastY;
	UInt32 firstCC;
	gFont->CalcStringExtents( testString, w, h, a, firstCC, lastX, lastY );

	int cY = ( ( ( r.bottom - r.top ) - h ) >> 1 ) + a;

	if( cY < 0 )
		cY = 0;
	else if( cY > r.bottom - r.top - 1 )
		cY = r.bottom - r.top - 1;

	memset( mip->GetAddr32( 8, cY ), 0xc0, ( r.right - r.left - 8 ) * 4 );

	gFont->RenderString( mip, 8, cY, testString );

	int x, y;
	for( y = 0; y < r.bottom - r.top; y++ )
	{
		for( x = 0; x < r.right - r.left; x++ )
		{
			UInt32 color = *mip->GetAddr32( x, y );
			hsColorRGBA   rgba;
			rgba.FromARGB32( color );

			if( color != 0xffffffff && color != 0xff000000 )
			{
				int q = 0;
			}
			SetPixel( gPreviewHDC, x, y, RGB( rgba.r * 255.f, rgba.g * 255.f, rgba.b * 255.f) );
		}
	}

	delete mip;
}

class plSetKeyObj : public hsKeyedObject
{
	public:
		void	SetMyKey( const plKey &key )
		{
			SetKey( key );
		}
};

class plMyBDFCallback : public plBDFConvertCallback
{
	protected:
		HWND	fDlg;
		clock_t	fLastTime;
		UInt16	fPoint;

		void	IPumpMessageQueue( void )
		{
			MSG	msg;
			while( PeekMessage( &msg, fDlg, 0, 0, PM_REMOVE ) )
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
		}

	public:
		plMyBDFCallback( HWND dlg ) : fDlg( dlg ) {}

		virtual void	NumChars( UInt16 chars )
		{
			::SendDlgItemMessage( fDlg, IDC_PROGRESS, PBM_SETRANGE, 0, MAKELPARAM( 0, chars ) );
			IPumpMessageQueue();
			fLastTime = clock();
			fPoint = 0;
		}

		virtual void	CharDone( void )
		{
			fPoint++;
			if( clock() - fLastTime > CLOCKS_PER_SEC / 16 )
			{
				::SendDlgItemMessage( fDlg, IDC_PROGRESS, PBM_SETPOS, fPoint, 0 );
				IPumpMessageQueue();
				fLastTime = clock();
			}
		}
};

BOOL CALLBACK ProgressWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	return 0;
}
	
void	IImportFNT( HWND hWnd, const char *path )
{
	IMakeNewFont();
	if( !gFont->LoadFromFNT( path ) )
		MessageBox( hWnd, "Failure converting FNT file", "ERROR", MB_OK | MB_ICONEXCLAMATION );
	IUpdateInfo( hWnd );
}

void	IImportBDF( HWND hWnd, const char *path )
{
	IMakeNewFont();
	HWND dialog = CreateDialog( gInstance, MAKEINTRESOURCE( IDD_PROGRESS ), hWnd, ProgressWndProc );
	ShowWindow( dialog, SW_SHOW );
	EnableWindow( hWnd, false );
	plMyBDFCallback	callback( dialog );

	if( !gFont->LoadFromBDF( path, &callback ) )
		MessageBox( hWnd, "Failure converting BDF file", "ERROR", MB_OK | MB_ICONEXCLAMATION );

	DestroyWindow( dialog );
	EnableWindow( hWnd, true );

	IUpdateInfo( hWnd );
}

void	IOpenP2F( HWND hWnd, const char *path )
{
	IMakeNewFont();
	if( !gFont->LoadFromP2FFile( path ) )
		MessageBox( hWnd, "Failure opening P2F file", "ERROR", MB_OK | MB_ICONEXCLAMATION );

	IUpdateInfo( hWnd );
}

struct ResRecord
{
	HRSRC	fHandle;
	char	fName[ 512 ];

	ResRecord() { fHandle = nil; fName[ 0 ] = 0; }
	ResRecord( HRSRC h, const char *n ) { fHandle = h; strncpy( fName, n, sizeof( fName ) ); }
};

BOOL CALLBACK	ResEnumProc( HMODULE module, LPCTSTR type, LPTSTR name, LONG_PTR lParam )
{
	HRSRC res = FindResource( module, name, type );
	if( res != nil )
	{
		hsTArray<ResRecord *>	*array = (hsTArray<ResRecord *> *)lParam;
		array->Append( new ResRecord( res, name ) );
	}

	return true;
}

BOOL CALLBACK ResListWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		case WM_INITDIALOG:
			{
				SendDlgItemMessage( hWnd, IDC_RESLIST, LB_RESETCONTENT, 0, 0 );
				hsTArray<ResRecord *> *array = (hsTArray<ResRecord *> *)lParam;
				for( UInt32 i = 0; i < array->GetCount(); i++ )
				{
					ResRecord *rec = array->Get( i );
					int idx = SendDlgItemMessage( hWnd, IDC_RESLIST, LB_ADDSTRING, 0, (LPARAM)rec->fName );
					SendDlgItemMessage( hWnd, IDC_RESLIST, LB_SETITEMDATA, idx, (LPARAM)rec );
				}
			}
			return 0;

		case WM_COMMAND:
			if( wParam == IDCANCEL )
				EndDialog( hWnd, nil );
			else
			{
				int idx = SendDlgItemMessage( hWnd, IDC_RESLIST, LB_GETCURSEL, 0, 0 );
				if( idx == LB_ERR )
					EndDialog( hWnd, nil );
				else
				{
					ResRecord *rec = (ResRecord *)SendDlgItemMessage( hWnd, IDC_RESLIST, LB_GETITEMDATA, idx, 0 );
					EndDialog( hWnd, (int)rec );
				}
			}
			return true;
	}

	return 0;
}

void	IImportFON( HWND hWnd, const char *path )
{
	// FON files are really just resource modules
	IMakeNewFont();
	HMODULE file = LoadLibraryEx( path, nil, LOAD_LIBRARY_AS_DATAFILE | DONT_RESOLVE_DLL_REFERENCES );
	if( file == nil )
	{
		char msg[ 512 ], msg2[ 1024 ];

		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
						nil, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)msg, sizeof( msg ), nil );

		sprintf( msg2, "Failure importing FON file: can't open as resource library (%s)", msg );
		MessageBox( hWnd, msg2, "Error", MB_OK | MB_ICONEXCLAMATION );
	}
	else
	{
		hsTArray<ResRecord *>	resList;
		
		if( EnumResourceNames( file, "Font", ResEnumProc, (LPARAM)&resList ) )
		{
			// Put up a list of the resources so the user can choose which one
			ResRecord *res = (ResRecord *)DialogBoxParam( gInstance, MAKEINTRESOURCE( IDD_FONCHOOSER ), hWnd, 
															ResListWndProc, (LPARAM)&resList );
			if( res != nil )
			{
				// Load the resource into a ram stream
				hsRAMStream	stream;

				HGLOBAL glob = LoadResource( file, res->fHandle );
				if( glob != nil )
				{
					void *data = LockResource( glob );
					if( data != nil )
					{
						stream.Write( SizeofResource( file, res->fHandle ), data );
						stream.Rewind();

						if( !gFont->LoadFromFNTStream( &stream ) )
							MessageBox( hWnd, "Failure importing FON file: can't parse resource as FNT",
												"Error", MB_OK | MB_ICONEXCLAMATION );

					}
				}
			}
		}
		else
		{
			char msg[ 512 ], msg2[ 1024 ];

			FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
							nil, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)msg, sizeof( msg ), nil );

			sprintf( msg2, "Failure importing FON file: can't enumerate resources (%s)", msg );
			MessageBox( hWnd, msg2, "Error", MB_OK | MB_ICONEXCLAMATION );
		}

		UInt32 i;
		for( i = 0; i < resList.GetCount(); i++ )
			delete resList[ i ];
		resList.Reset();

		FreeLibrary( file );
	}

	IUpdateInfo( hWnd );
}

BOOL CALLBACK FreeTypeDlgProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	static plFontFreeType::Options	*info;

	switch( message )
	{
		case WM_INITDIALOG:
			info = (plFontFreeType::Options *)lParam;
			SetDlgItemInt( hWnd, IDC_PSIZE, info->fSize, false );
			SetDlgItemInt( hWnd, IDC_RES, info->fScreenRes, false );
			SetDlgItemInt( hWnd, IDC_MAXCHAR, info->fMaxCharLimit, false );
			CheckRadioButton( hWnd, IDC_BITDEPTH, IDC_BITDEPTH2, info->fBitDepth == 1 ? IDC_BITDEPTH : IDC_BITDEPTH2 );
			return 0;

		case WM_COMMAND:
			if( wParam == IDOK || wParam == IDCANCEL || wParam == IDC_BATCH )
			{
				info->fSize = GetDlgItemInt( hWnd, IDC_PSIZE, nil, false );
				info->fScreenRes = GetDlgItemInt( hWnd, IDC_RES, nil, false );
				info->fMaxCharLimit = GetDlgItemInt( hWnd, IDC_MAXCHAR, nil, false );
				
				if( IsDlgButtonChecked( hWnd, IDC_BITDEPTH ) )
					info->fBitDepth = 1;
				else
					info->fBitDepth = 8;

				EndDialog( hWnd, wParam );
			}
			return 1;
	}
	return 0;
}

void	IBatchFreeType( HWND hWnd, const char *path );

void	IImportFreeType( HWND hWnd, const char *path )
{
	static plFontFreeType::Options info;

	int ret = DialogBoxParam( gInstance, MAKEINTRESOURCE( IDD_FREETYPE ), hWnd, FreeTypeDlgProc, (LPARAM)&info );
	if( ret == IDCANCEL )
		return;
	else if( ret == IDC_BATCH )
	{
		IBatchFreeType( hWnd, path );
		return;
	}

	IMakeNewFont();
	HWND dialog = CreateDialog( gInstance, MAKEINTRESOURCE( IDD_PROGRESS ), hWnd, ProgressWndProc );
	ShowWindow( dialog, SW_SHOW );
	EnableWindow( hWnd, false );
	plMyBDFCallback	callback( dialog );

	plFontFreeType *ft2Convert = (plFontFreeType *)gFont;
	if( !ft2Convert->ImportFreeType( path, &info, &callback ) )
		MessageBox( hWnd, "Failure converting TrueType file", "ERROR", MB_OK | MB_ICONEXCLAMATION );

	DestroyWindow( dialog );
	EnableWindow( hWnd, true );

	IUpdateInfo( hWnd );
}

static UInt8	sNumSizes = 0;
static UInt8	sSizeArray[ 256 ];
static char		sFontName[ 256 ]; // desired font name for FreeType conversions

BOOL CALLBACK FreeTypeBatchDlgProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	static plFontFreeType::Options	*info;

	switch( message )
	{
		case WM_INITDIALOG:
			info = (plFontFreeType::Options *)lParam;
			SetDlgItemText( hWnd, IDC_PSIZE, "12" );
			SetDlgItemText( hWnd, IDC_FONTNAME, "Untitled" );
			SetDlgItemInt( hWnd, IDC_RES, info->fScreenRes, false );
			SetDlgItemInt( hWnd, IDC_MAXCHAR, info->fMaxCharLimit, false );
			CheckRadioButton( hWnd, IDC_BITDEPTH, IDC_BITDEPTH2, info->fBitDepth == 1 ? IDC_BITDEPTH : IDC_BITDEPTH2 );
			return 0;

		case WM_COMMAND:
			if( wParam == IDOK || wParam == IDCANCEL )
			{
				sNumSizes = 0;

				char *c, *lastC, str[ 1024 ];
				GetDlgItemText( hWnd, IDC_PSIZE, str, sizeof( str ) );
				lastC = str;
				while( ( c = strchr( lastC, ',' ) ) != nil && sNumSizes < 255 )
				{
					*c = 0;
					sSizeArray[ sNumSizes++ ] = atoi( lastC );
					lastC = c + 1;			
				}

				sSizeArray[ sNumSizes++ ] = atoi( lastC );

				info->fScreenRes = GetDlgItemInt( hWnd, IDC_RES, nil, false );
				info->fMaxCharLimit = GetDlgItemInt( hWnd, IDC_MAXCHAR, nil, false );
				
				if( IsDlgButtonChecked( hWnd, IDC_BITDEPTH ) )
					info->fBitDepth = 1;
				else
					info->fBitDepth = 8;
				
				GetDlgItemText( hWnd, IDC_FONTNAME, sFontName, sizeof(sFontName) );

				EndDialog( hWnd, wParam );
			}
			return 1;
	}
	return 0;
}

void	IBatchFreeType( HWND hWnd, const char *path )
{
	static plFontFreeType::Options info;

	if( DialogBoxParam( gInstance, MAKEINTRESOURCE( IDD_FREETYPEBATCH ), hWnd, FreeTypeBatchDlgProc, (LPARAM)&info ) == IDCANCEL )
		return;

	BROWSEINFO		bInfo;
	LPITEMIDLIST	itemList;
	LPMALLOC		shMalloc;
	static char		destPath[ MAX_PATH ] = "";

	memset( &bInfo, 0, sizeof( bInfo ) );
	bInfo.hwndOwner = hWnd;
	bInfo.pidlRoot = NULL;
	bInfo.pszDisplayName = destPath;
	bInfo.lpszTitle = "Select a path to write the P2F fonts to:";
	bInfo.ulFlags = BIF_EDITBOX;

	itemList = SHBrowseForFolder( &bInfo );
	if( itemList != NULL )
	{
		SHGetPathFromIDList( itemList, destPath );
		SHGetMalloc( &shMalloc );
		shMalloc->Free( itemList );
		shMalloc->Release();
	}
	else
		return;

	HWND dialog = CreateDialog( gInstance, MAKEINTRESOURCE( IDD_PROGRESS ), hWnd, ProgressWndProc );
	ShowWindow( dialog, SW_SHOW );
	EnableWindow( hWnd, false );
	plMyBDFCallback	callback( dialog );

	callback.NumChars( sNumSizes );
	UInt8 i;
	for( i = 0; i < sNumSizes; i++ )
	{
		IMakeNewFont();
		plFontFreeType *ft2Convert = (plFontFreeType *)gFont;

		info.fSize = sSizeArray[ i ];
		if( !ft2Convert->ImportFreeType( path, &info, nil ) )
		{
			MessageBox( hWnd, "Failure converting TrueType file", "ERROR", MB_OK | MB_ICONEXCLAMATION );
			continue;
		}

		gFont->SetFace(sFontName);
		char fileName[ MAX_PATH ];
		sprintf( fileName, "%s\\%s-%d.p2f", destPath, gFont->GetFace(), gFont->GetSize() );
		hsUNIXStream stream;
		if( !stream.Open( fileName, "wb" ) )
			MessageBox( hWnd, "Can't open file for writing", "Error", MB_OK | MB_ICONEXCLAMATION );
		else
		{
			gFont->WriteRaw( &stream );
			stream.Close();
		}

		callback.CharDone();
	}

	DestroyWindow( dialog );
	EnableWindow( hWnd, true );
	IUpdateInfo( hWnd );
}

BOOL CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	char		fileName[ MAX_PATH ];
	PAINTSTRUCT	paintInfo;
	HDC			myDC;
	RECT		r;


	switch( message )
	{
		case WM_PAINT:
			myDC = BeginPaint( hWnd, &paintInfo );

			GetClientRect( GetDlgItem( hWnd, IDC_PREVIEW ), &r );
			MapWindowPoints( GetDlgItem( hWnd, IDC_PREVIEW ), hWnd, (POINT *)&r, 2 );

			if( gPreviewHDC != nil )
				BitBlt( myDC, r.left, r.top, r.right - r.left, r.bottom - r.top, gPreviewHDC, 0, 0, SRCCOPY );
			else
				FillRect( myDC, &r, GetSysColorBrush( COLOR_3DFACE ) );

			DrawEdge( myDC, &r, EDGE_SUNKEN, BF_RECT );

			EndPaint( hWnd, &paintInfo );
			return 0;

		case WM_INITDIALOG:
			SendMessage( hWnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon( gInstance, MAKEINTRESOURCE( IDI_APPICON ) ) );
			SetDlgItemTextW( hWnd, IDC_PREVTEXT, L"The quick brown fox jumped over the lazy dog! ABCabc012345;,." );
			return 0;

		case WM_COMMAND:
			if( wParam == ID_FILE_ABOUT )
			{
				DialogBox( gInstance, MAKEINTRESOURCE( IDD_ABOUT ), hWnd, AboutDialogProc );
			}
			else if( wParam == ID_FILE_EXIT )
				PostQuitMessage( 0 );
			else if( wParam == ID_FILE_FNT )
			{
				fileName[ 0 ] = 0;
				if( PromptForFile( hWnd, "Choose a FNT file to convert", "Windows FNT files\0*.fnt\0All files\0*.*\0", fileName, sizeof( fileName ), false ) )
					IImportFNT( hWnd, fileName );
			}
			else if( wParam == ID_FILE_P2F )
			{
				fileName[ 0 ] = 0;
				if( PromptForFile( hWnd, "Choose a P2F file to open", "Plasma 2 font files\0*.p2f\0All files\0*.*\0", fileName, sizeof( fileName ), false ) )
					IOpenP2F( hWnd, fileName );
			}
			else if( wParam == ID_FILE_FON )
			{
				fileName[ 0 ] = 0;
				if( PromptForFile( hWnd, "Choose a FON file to convert", "Windows FON files\0*.fon\0All files\0*.*\0", fileName, sizeof( fileName ), false ) )
					IImportFON( hWnd, fileName );
			}
			else if( wParam == ID_FILE_TRUETYPE )
			{
				fileName[ 0 ] = 0;
				if( PromptForFile( hWnd, "Choose a TrueType font to convert", "TrueType files\0*.ttf\0TrueType Collections\0*.ttc\0All files\0*.*\0", fileName, sizeof( fileName ), false ) )
					IBatchFreeType( hWnd, fileName );
			}
			else if( wParam == ID_FILE_EXPORT )
			{
				// Grab updated values for the font
				GetDlgItemText( hWnd, IDC_FACE, fileName, sizeof( fileName ) );
				gFont->SetFace( fileName );
				gFont->SetSize( GetDlgItemInt( hWnd, IDC_FSIZE, nil, false ) );
				gFont->SetFlag( plFont::kFlagBold, IsDlgButtonChecked( hWnd, IDC_BOLD ) == BST_CHECKED );
				gFont->SetFlag( plFont::kFlagItalic, IsDlgButtonChecked( hWnd, IDC_ITALIC ) == BST_CHECKED );

				// Write out
				sprintf( fileName, "%s-%d.p2f", gFont->GetFace(), gFont->GetSize() );
				if( PromptForFile( hWnd, "Specify a file to export to", "Plasma 2 font files\0*.p2f\0", fileName, sizeof( fileName ), true ) )
				{
					hsUNIXStream stream;
					if( !stream.Open( fileName, "wb" ) )
						MessageBox( hWnd, "Can't open file for writing", "Error", MB_OK | MB_ICONEXCLAMATION );
					else
					{
/*						sprintf( fileName, "%s-%d", gFont->GetFace(), gFont->GetSize() );

						if( gFont->GetKey() == nil )
							hsgResMgr::ResMgr()->NewKey( fileName, gFont, plLocation::kGlobalFixedLoc );

*/						
						gFont->WriteRaw( &stream );
						stream.Close();
					}
				}
			}
			else if( LOWORD( wParam ) == IDC_PREVTEXT && HIWORD( wParam ) == EN_CHANGE )
			{
				IUpdateInfo( hWnd );
			}
			return true;

		case WM_CLOSE:
			PostQuitMessage( 0 );
			return true;

		case WM_DROPFILES:
			{
				int		i, fileCount = DragQueryFile( (HDROP)wParam, -1, nil, nil );
				char	path[ MAX_PATH ];


				for( i = 0; i < fileCount; i++ )
				{
					if( DragQueryFile( (HDROP)wParam, i, path, sizeof( path ) ) > 0 )
					{
						char *ext = PathFindExtension( path );
						if( stricmp( ext, ".fnt" ) == 0 )
							IImportFNT( hWnd, path );
						else if( stricmp( ext, ".bdf" ) == 0 )
							IImportBDF( hWnd, path );
						else if( stricmp( ext, ".fon" ) == 0 )
							IImportFON( hWnd, path );
						else if( stricmp( ext, ".exe" ) == 0 )
							IImportFON( hWnd, path );
						else if(( stricmp( ext, ".ttf" ) == 0 ) || ( stricmp( ext, ".ttc" ) == 0 ))
							IImportFreeType( hWnd, path );
						else if( stricmp( ext, ".p2f" ) == 0 )
							IOpenP2F( hWnd, path );
						else
							// Try using our freeType converter
							IImportFreeType( hWnd, path );
					}
				}

				IUpdateInfo( hWnd );
			}
			break;

	}
	return 0;//DefWindowProc( hWnd, message, wParam, lParam );
}

