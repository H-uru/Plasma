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
#ifndef plWindow_h_inc
#define plWindow_h_inc

class plControl;

#include <strstream>


class plWindow
: public plClass	// plClass _must_ be the first base class in list.
, public plConfigValueBase
{
public:
	HWND					fhWnd;
	WORD					fControlID;
	WORD					fTopControlID;
	bool					fDestroyed;
	bool					fMdiChild;
	plWindow *				fOwnerWindow;
	std::vector<plControl*>	fControls;
	HWND & Handle() { return fhWnd;}
	const HWND & Handle() const { return fhWnd;}
	bool					fEdited;

	static int _ModalCount;
	static std::vector<plWindow*> _Windows;
	static std::vector<plWindow*> _DeleteWindows;
	static LONG APIENTRY StaticWndProc(HWND hWnd, unsigned int message, unsigned int wParam, LONG lParam )
	{
		// look for this hwnd in window list
		int i;
		for(i=0; i<_Windows.size(); i++ )
			if( _Windows[i]->Handle()==hWnd )
				break;
		if (i==_Windows.size())
		{
//			hsStatusMessage("hwnd not found in _Windows.\n");
		}

		// if window not found and this is WM_NCCREATE or WM_INITDIALOG msg...
		if( i==_Windows.size() && (message==WM_NCCREATE || message==WM_INITDIALOG || message==WM_ACTIVATE) )
		{
			// get the plWindow object
			plWindow * WindowCreate
			=	message!=WM_NCCREATE
			?	(plWindow*)lParam
			:	(GetWindowLong(hWnd,GWL_EXSTYLE) & WS_EX_MDICHILD)
			?	(plWindow*)((MDICREATESTRUCT*)((CREATESTRUCT*)lParam)->lpCreateParams)->lParam
			:	(plWindow*)((CREATESTRUCT*)lParam)->lpCreateParams;
			CHECK(WindowCreate);
			CHECK(!WindowCreate->Handle());

			// set the hwnd for this plWindow
			WindowCreate->Handle() = hWnd;
			// look for this plWindow in window list
			for( i=0; i<_Windows.size(); i++ )
				if( _Windows[i]==WindowCreate )
					break;
			if (i==_Windows.size())
			{
				hsStatusMessage("plWindow not found in _Windows.\n");
			}
			CHECK(i<_Windows.size());
		}
		// if window not found and message is not WM_NCCREATE or WM_INITDIALOG msg...
		if( i==_Windows.size() )
		{
			// Gets through before WM_NCCREATE: WM_GETMINMAXINFO
			return DefWindowProc( hWnd, message, wParam, lParam );
		}
		else
		{
			// call the plWindow class's message handler
			return _Windows[i]->WndProc( message, wParam, lParam );			
		}
	}
	static WNDPROC RegisterWindowClass( const wchar_t * name, DWORD style, int iconId=0 )
	{
#ifdef UNICODE
		WNDCLASSEX cls;
		HSMemory::ClearMemory( &cls, sizeof(cls) );
		cls.cbSize			= sizeof(cls);
		cls.style			= style;
		cls.lpfnWndProc		= StaticWndProc;
		cls.hInstance		= plWndCtrls::Instance();
		cls.hIcon			= LoadIcon(plWndCtrls::Instance(),MAKEINTRESOURCE(iconId));
		cls.lpszClassName	= name;
		cls.hIconSm			= LoadIcon(plWndCtrls::Instance(),MAKEINTRESOURCE(iconId));
		CHECK(RegisterClassEx( &cls ));
#else
		char *cName = hsWStringToString(name);
		WNDCLASSEX cls;
		HSMemory::ClearMemory( &cls, sizeof(cls) );
		cls.cbSize			= sizeof(cls);
		cls.style			= style;
		cls.lpfnWndProc		= StaticWndProc;
		cls.hInstance		= plWndCtrls::Instance();
		cls.hIcon			= LoadIcon(plWndCtrls::Instance(),MAKEINTRESOURCE(iconId));
		cls.lpszClassName	= cName;
		cls.hIconSm			= LoadIcon(plWndCtrls::Instance(),MAKEINTRESOURCE(iconId));
		CHECK(RegisterClassEx( &cls ));
		delete [] cName;
#endif
		return nil;
	}

	plWindow( plWindow * ownerWindow=nil )
	:	fhWnd			(nil)
	,	fControlID		(0)
	,	fTopControlID	(FIRST_AUTO_CONTROL)
	,	fDestroyed		(0)
	,   fMdiChild		(0)
	,	fOwnerWindow	(ownerWindow)
	,	fEdited			(false)
	{
		fReadEvaluate = plEvaluate(this,(TEvaluate)HasConfigName);
		fWriteEvaluate = plEvaluate(this,(TEvaluate)HasConfigName);
	}
	virtual ~plWindow()
	{
		MaybeDestroy();
		std::vector<plWindow*>::iterator it = std::find(_DeleteWindows.begin(),_DeleteWindows.end(),this);
		if (it!=_DeleteWindows.end())
			_DeleteWindows.erase(it);
	}

	plRect GetClientRect() const
	{
		RECT R;
		::GetClientRect( Handle(), &R );
		return plRect( R );
	}
	plRect GetUpdateRect(bool erase) const
	{
		RECT R;
		::GetUpdateRect(*this,&R,erase);
		return plRect(R);
	}
	void MoveWindow( plRect R, bool bRepaint )
	{
		::MoveWindow( Handle(), R.Min.X, R.Min.Y, R.Width(), R.Height(), bRepaint );
	}
	plRect GetWindowRect() const
	{
		RECT R;
		::GetWindowRect( Handle(), &R );
		return fOwnerWindow ? fOwnerWindow->ScreenToClient(R) : plRect(R);
	}
	plPoint ClientToScreen( const plPoint& inP )
	{
		POINT P;
		P.x = inP.X;
		P.y = inP.Y;
		::ClientToScreen( Handle(), &P );
		return plPoint( P.x, P.y );
	}
	plPoint ScreenToClient( const plPoint& inP )
	{
		POINT P;
		P.x = inP.X;
		P.y = inP.Y;
		::ScreenToClient( Handle(), &P );
		return plPoint( P.x, P.y );
	}
	plRect ClientToScreen( const plRect& inR )
	{
		return plRect( ClientToScreen(inR.Min), ClientToScreen(inR.Max) );
	}
	plRect ScreenToClient( const plRect& inR )
	{
		return plRect( ScreenToClient(inR.Min), ScreenToClient(inR.Max) );
	}
	plPoint GetCursorPos()
	{
		plPoint mouse;
		::GetCursorPos( mouse );
		return ScreenToClient( mouse );
	}
	void Show( bool show = true )
	{
		ShowWindow( Handle(), show ? SW_SHOW : SW_HIDE );
	}
	void ShowHow( int how )
	{
		ShowWindow( Handle(), how );
	}
	void Hide()
	{
		Show(false);
	}
	bool IsVisible()
	{
		return ::IsWindowVisible(*this)?true:false;
	}

	virtual void DoDestroy()
	{
		if( Handle() )
			DestroyWindow( *this );
		std::vector<plWindow*>::iterator it = std::find(_Windows.begin(),_Windows.end(),this);
		if (it!=_Windows.end())
			_Windows.erase(it);
	}
	virtual void GetWindowClassName( wchar_t * result )=0;
	virtual LONG WndProc( unsigned int message, unsigned int wParam, LONG lParam )
	{
		try
		{
			if( message==WM_DESTROY )
			{
				OnDestroy();
			}
			else if( message==WM_DRAWITEM )
			{
				DRAWITEMSTRUCT * Info = (DRAWITEMSTRUCT*)lParam;
				for( int i=0; i<fControls.size(); i++ )
					if( ((plWindow*)fControls[i])->Handle()==Info->hwndItem )
						{((plWindow*)fControls[i])->OnDrawItem(Info); break;}
				return 1;
			}
			else if( message==WM_MEASUREITEM )
			{
				MEASUREITEMSTRUCT * Info = (MEASUREITEMSTRUCT*)lParam;
				for( int i=0; i<fControls.size(); i++ )
					if( ((plWindow*)fControls[i])->fControlID==Info->CtlID )
						{((plWindow*)fControls[i])->OnMeasureItem(Info); break;}
				return 1;
			}
			else if ( message==WM_NOTIFY )
			{
				OnNotify((int)wParam,(LPNMHDR)lParam);
			}
			else if( message==WM_CLOSE )
			{
				OnClose();
			}
			else if( message==WM_CHAR )
			{
				if (!OnChar( wParam )) // give the control a chance to filter input
					return FALSE;
			}
			else if( message==WM_KEYDOWN )
			{
				OnKeyDown( wParam );
			}
			else if( message==WM_KEYUP )
			{
				OnKeyUp( wParam );
			}
			else if( message==WM_PAINT )
			{
				OnPaint();
			}
			else if( message==WM_CREATE )
			{
				OnCreate();
			}
			else if( message==WM_TIMER )
			{
				OnTimer( (int)wParam );
			}
			else if( message==WM_INITDIALOG )
			{
				OnInitDialog();
			}
			else if( message==WM_SETFOCUS )
			{
				OnSetFocus( (HWND)wParam );
			}
			else if( message==WM_ACTIVATE )
			{
				OnActivate( LOWORD(wParam)!=0 );
			}
			else if( message==WM_KILLFOCUS )
			{
				OnKillFocus( (HWND)wParam );
			}
			else if( message==WM_SIZE )
			{
				OnSize( wParam, LOWORD(lParam), HIWORD(lParam) );
			}
			else if( message==WM_PASTE )
			{
				OnPaste();
			}
			else if( message==WM_SHOWWINDOW )
			{
				OnShowWindow( wParam?true:false );
			}
			else if( message==WM_COPYDATA )
			{
				OnCopyData( (HWND)wParam, (COPYDATASTRUCT*)lParam );
			}
			else if( message==WM_CAPTURECHANGED )
			{
				OnReleaseCapture();
			}
			else if( message==WM_MDIACTIVATE )
			{
				OnMdiActivate( (HWND)lParam==Handle());
			}
			else if( message==WM_MOUSEMOVE )
			{
				OnMouseMove( wParam, plPoint(LOWORD(lParam), HIWORD(lParam)) );
			}
			else if( message==WM_LBUTTONDOWN )
			{
				OnLeftButtonDown();
			}
			else if( message==WM_RBUTTONDOWN )
			{
				OnRightButtonDown();
			}
			else if( message==WM_LBUTTONUP )
			{
				OnLeftButtonUp();
			}
			else if( message==WM_RBUTTONUP )
			{
				OnRightButtonUp();
			}
			else if( message==WM_CUT )
			{
				OnCut();
			}
			else if( message==WM_COPY )
			{
				OnCopy();
			}
			else if( message==WM_UNDO )
			{
				OnUndo();
			}
			else if( message==WM_SETCURSOR )
			{
				if( OnSetCursor() )
					return 1;
			}
			else if( message==WM_COMMAND || message==WM_HSCROLL || message==WM_VSCROLL )
			{
				for( int i=0; i<fControls.size(); i++ )
					if
					(	(HWND)lParam==((plWindow*)fControls[i])->Handle()
					&&	((plWindow*)fControls[i])->InterceptControlCommand(message,wParam,lParam) )
						return 1;
				OnCommand( wParam );
			}
			else if ( message==WM_SYSCOMMAND )
			{
				OnSysCommand( wParam );
			}
			return CallDefaultProc( message, wParam, lParam );
		}
		catch( const char * e )
		{
			hsStatusMessage( e );
			hsStatusMessage("\n");
			return 0;
		}
	}
	virtual int CallDefaultProc( unsigned int message, unsigned int wParam, LONG lParam )
	{
		if( fMdiChild )
			return DefMDIChildProc( Handle(), message, wParam, lParam );
		else
			return DefWindowProc( Handle(), message, wParam, lParam );
	}
	virtual bool InterceptControlCommand( unsigned int message, unsigned int wParam, LONG lParam )
	{
		return 0;
	}
	virtual std::wstring GetText() const
	{
		CHECK(Handle());
		int length = GetLength();
		std::wstring result;
		if (length==0)
			return result;
		result.resize(length);
		SendMessage( *this, WM_GETTEXT, length+1, (LPARAM)result.data() );
		return result;
	}
	virtual void SetText( const wchar_t * text )
	{
		CHECK(Handle());
		SendMessage( *this, WM_SETTEXT, 0, (LPARAM)text );
	}
	virtual void SetTextF( const wchar_t * fmt, ... )
	{
		va_list args;
		va_start( args, fmt );
		SetTextV( fmt, args );
		va_end( args );
	}
	virtual void SetTextV( const wchar_t * fmt, va_list args )
	{
		std::wstring s;
		xtl::formatv( s, fmt, args );
		SetText( s.c_str() );
	}
	virtual void SetEnabled(bool enabled)
	{
		CHECK(Handle());
		EnableWindow(*this,enabled);
	}
	virtual bool IsEnabled()
	{
		return IsWindowEnabled(*this)?true:false;
	}
	virtual int GetLength() const
	{
		CHECK(Handle());
		return SendMessage( *this, WM_GETTEXTLENGTH, 0, 0 );
	}
	virtual void SetFocus()
	{
		::SetFocus(*this);
	}
	virtual void Activate()
	{
		::SetActiveWindow(*this);
	}

	// plWindow notifications.
	virtual void OnNotify( int idCtrl, LPNMHDR pnmh )
	{}
	virtual void OnCopyData( HWND hWndSender, COPYDATASTRUCT * CD )
	{}
	virtual void OnSetFocus( HWND hWndLosingFocus )
	{}
	virtual void OnKillFocus( HWND hWndGainingFocus )
	{}
	virtual void OnSize( DWORD flags, int newX, int newY )
	{}
	virtual void OnCommand( int command )
	{}
	virtual void OnSysCommand( int command )
	{}
	virtual void OnActivate( bool active )
	{}
	virtual LONG OnChar( char ch ) // Return TRUE if you want to let the default handler grab it, FALSE if you don't
	{return TRUE;}
	virtual void OnKeyDown( UInt16 ch )
	{}
	virtual void OnKeyUp( UInt16 ch )
	{}
	virtual void OnCut()
	{}
	virtual void OnCopy()
	{}
	virtual void OnPaste()
	{}
	virtual void OnShowWindow( bool bShow )
	{}
	virtual void OnUndo()
	{}
	virtual void OnPaint()
	{}
	virtual void OnCreate()
	{}
	virtual void OnDrawItem( DRAWITEMSTRUCT * info )
	{}
	virtual void OnMeasureItem( MEASUREITEMSTRUCT * info )
	{}
	virtual void OnInitDialog()
	{}
	virtual void OnMouseEnter()
	{}
	virtual void OnMouseLeave()
	{}
	virtual void OnMouseHover()
	{}
	virtual void OnTimer( int timer )
	{}
	virtual void OnReleaseCapture()
	{}
	virtual void OnMdiActivate( bool active )
	{}
	virtual void OnMouseMove( DWORD flags, plPoint location )
	{}
	virtual void OnLeftButtonDown()
	{}
	virtual void OnRightButtonDown()
	{}
	virtual void OnLeftButtonUp()
	{}
	virtual void OnRightButtonUp()
	{}
	virtual int OnSetCursor()
	{
		return 0;
	}
	virtual void OnClose()
	{
		DestroyWindow( *this );
	}
	virtual void OnDestroy()
	{
		CHECK(Handle());
		std::vector<plWindow*>::iterator it = std::find(_Windows.begin(),_Windows.end(),this);
		if (it!=_Windows.end())
			_Windows.erase(it);
		Handle() = nil;
	}

	// plWindow functions.
	void MaybeDestroy()
	{
		if( !fDestroyed )
		{
			fDestroyed=1;
			DoDestroy();
		}
	}
	void _CloseWindow()
	{
		CHECK(Handle());
		DestroyWindow( *this );
	}
	operator HWND() const
	{
		return Handle();
	}
	void SetFont( HFONT hFont )
	{
		SendMessage( *this, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(0,0) );
	}
	void SetSmallIcon( HICON hIcon )
	{
		SendMessage( *this, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
	}
	void SetBigIcon( HICON hIcon )
	{
		SendMessage( *this, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	}
	void PerformCreateWindowEx(
		DWORD dwExStyle,
		LPCTSTR lpWindowName,
		DWORD dwStyle,
		int x, int y,
		int nWidth, int nHeight,
		HWND hWndParent,
		HMENU hMenu,
		HINSTANCE hInstance)
	{
		CHECK(Handle()==nil);
		_Windows.push_back( this );

		wchar_t className[256];
		GetWindowClassName( className );
#ifdef UNICODE
		HWND hWndCreated = CreateWindowEx(
			dwExStyle,
			className,
			lpWindowName,
			dwStyle,x,y,
			nWidth,nHeight,
			hWndParent,hMenu,
			plWndCtrls::Instance(),
			this);
#else
		char *cClassName = hsWStringToString(className);
		HWND hWndCreated = CreateWindowEx(
			dwExStyle,
			cClassName,
			lpWindowName,
			dwStyle,x,y,
			nWidth,nHeight,
			hWndParent,hMenu,
			plWndCtrls::Instance(),
			this);
		delete [] cClassName;
#endif

/*
#define SAFE(s)		((s)?s:"null")

		std::strstream str;
		str
			<< "vars: " << std::endl
			<< dwExStyle << std::endl
			<< SAFE(className) << std::endl
			<< SAFE(lpWindowName) << std::endl
			<< dwStyle << std::endl
			<< x << std::endl
			<< y << std::endl
			<< nWidth << std::endl
			<< nHeight << std::endl
			<< hWndParent << std::endl
			<< hMenu  << std::endl
			<< plWndCtrls::Instance() << std::endl
			<< '\0';
		MessageBox(nil,str.str(),"",MB_OK);
		str.freeze(false);
*/

		if( !hWndCreated )
			hsStatusMessage( "CreateWindowEx failed" );
		CHECK(hWndCreated);
		CHECK(hWndCreated==Handle());
	}
	void SetRedraw( bool redraw )
	{
		SendMessage( *this, WM_SETREDRAW, redraw, 0 );
	}

	// plConfigValueBase
	std::string IGetValue() const
	{
		std::string sText = "";
		char *temp = hsWStringToString(GetText().c_str());
		sText = temp;
		delete [] temp;
		return sText;
	}
	void ISetValue(const char * value)
	{
		wchar_t *wValue = hsStringToWString(value);
		SetText(wValue);
		delete [] wValue;
	}
	virtual void SetEdited(bool value)
	{
		fEdited = value;
	}
	virtual bool Edited() const
	{
		return fEdited;
	}
};





#endif plWindow_h_inc
