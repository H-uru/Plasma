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
#ifndef plListBox_h_inc
#define plListBox_h_inc


class plListBox : public plControl
{
public:
	DECLARE_WINDOWSUBCLASS(plListBox,plControl)

	plDelegate DoubleClickDelegate;
	plDelegate SelectionChangeDelegate;
	plDelegate SelectionCancelDelegate;
	plDelegate SetFocusDelegate;
	plDelegate KillFocusDelegate;

	plListBox()
	{}
	plListBox( plWindow * inOwner, int inId=0, WNDPROC inSuperProc=nil )
	: plControl( inOwner, inId, inSuperProc?inSuperProc:_SuperProc )
	{
		CHECK(fOwnerWindow);
	}

	void OpenWindow( bool visible, bool integral, bool multiSel, bool ownerDrawVariable )
	{
		PerformCreateWindowEx
		(
			WS_EX_CLIENTEDGE,
            nil,
            WS_CHILD | WS_BORDER | WS_VSCROLL | WS_CLIPCHILDREN | LBS_NOTIFY | (visible?WS_VISIBLE:0) | (integral?0:LBS_NOINTEGRALHEIGHT) | (multiSel?(LBS_EXTENDEDSEL|LBS_MULTIPLESEL):0) | (ownerDrawVariable?LBS_OWNERDRAWVARIABLE:0),
            0, 0,
			0, 0,
            *fOwnerWindow,
            (HMENU)fControlID,
            plWndCtrls::Instance()
		);
		SendMessage( *this, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(0,0) );
	}

	bool InterceptControlCommand( unsigned int message, unsigned int wParam, LONG lParam )
	{
		if     ( HIWORD(wParam)==LBN_DBLCLK   ) {DoubleClickDelegate();     return 1;}
		else if( HIWORD(wParam)==LBN_SELCHANGE) {SelectionChangeDelegate(); return 1;}
		else if( HIWORD(wParam)==LBN_SELCANCEL) {SelectionCancelDelegate(); return 1;}
		else if( HIWORD(wParam)==LBN_SETFOCUS)  {SetFocusDelegate();        return 1;}
		else if( HIWORD(wParam)==LBN_KILLFOCUS) {KillFocusDelegate();       return 1;}
		else return 0;
	}

	std::wstring GetString( int index ) const
	{
		int length = SendMessage(*this,LB_GETTEXTLEN,index,0);
		if (length == LB_ERR)
			return L"";
		std::wstring ch;
		ch.resize(length);
		length = SendMessage( *this, LB_GETTEXT, index, (LPARAM)ch.data() );
		ch.resize(length);
		return ch;
	}
	void * GetItemData( int index )
	{
		return (void*)SendMessage( *this, LB_GETITEMDATA, index, 0 );
	}
	void SetItemData( int index, void * value )
	{
		SendMessage( *this, LB_SETITEMDATA, index, (LPARAM)value );
	}
	int GetCurrent() const
	{
		return SendMessage( *this, LB_GETCARETINDEX, 0, 0 );
	}
	void SetCurrent( int index, bool bScrollIntoView )
	{
		SendMessage( *this, LB_SETCURSEL, index, 0 );
		SendMessage( *this, LB_SETCARETINDEX, index, bScrollIntoView );
	}
	int GetTop()
	{
		return SendMessage( *this, LB_GETTOPINDEX, 0, 0 );
	}
	void SetTop( int index )
	{
		SendMessage( *this, LB_SETTOPINDEX, index, 0 );
	}
	void DeleteString( int index )
	{
		SendMessage( *this, LB_DELETESTRING, index, 0 );
	}
	int GetCount()
	{
		return SendMessage( *this, LB_GETCOUNT, 0, 0 );
	}
	int GetItemHeight( int index )
	{
		return SendMessage( *this, LB_GETITEMHEIGHT, index, 0 );
	}
	int ItemFromPoint( const plPoint & P )
	{
		DWORD result=SendMessage( *this, LB_ITEMFROMPOINT, 0, MAKELPARAM(P.X,P.Y) );
		return HIWORD(result) ? -1 : LOWORD(result);
	}
	plRect GetItemRect( int index )
	{
		RECT R; R.left=R.right=R.top=R.bottom=0;
		SendMessage( *this, LB_GETITEMRECT, index, (LPARAM)&R );
		return R;
	}
	void Empty()
	{
		SendMessage( *this, LB_RESETCONTENT, 0, 0 );
	}
	bool GetSelected( int index )
	{
		return SendMessage( *this, LB_GETSEL, index, 0 )?true:false;
	}

	int AddString( const wchar_t * C )
	{
		return SendMessage( *this, LB_ADDSTRING, 0, (LPARAM)C );
	}
	void AddStrings( std::vector<std::wstring> & strings )
	{
		for (int i=0; i<strings.size(); i++)
			SendMessage( *this, LB_ADDSTRING, 0, (LPARAM)strings[i].c_str() );
	}
	void InsertString( int index, const wchar_t * C )
	{
		SendMessage( *this, LB_INSERTSTRING, index, (LPARAM)C );
	}
	int FindString( const wchar_t * C )
	{
		return SendMessage( *this, LB_FINDSTRING, -1, (LPARAM)C );
	}
	int FindStringExact( const wchar_t * C )
	{
		return SendMessage( *this, LB_FINDSTRINGEXACT, -1, (LPARAM)C );
	}
	int FindStringChecked( const wchar_t * C )
	{
		int result = SendMessage( *this, LB_FINDSTRING, -1, (LPARAM)C );
		CHECK(result!=LB_ERR);
		return result;
	}
	int FindStringExactChecked( const wchar_t * C )
	{
		int result = SendMessage( *this, LB_FINDSTRINGEXACT, -1, (LPARAM)C );
		CHECK(result!=LB_ERR);
		return result;
	}
	void InsertStringAfter( const wchar_t * existing, const wchar_t * str )
	{
		InsertString( FindStringChecked(existing)+1, str );
	}

	int AddItem( const void * C )
	{
		return SendMessage( *this, LB_ADDSTRING, 0, (LPARAM)C );
	}
	void InsertItem( int index, const void * C )
	{
		SendMessage( *this, LB_INSERTSTRING, index, (LPARAM)C );
	}
	int FindItem( const void * C, int fromIdx=-1 )
	{
		return SendMessage( *this, LB_FINDSTRING, fromIdx, (LPARAM)C );
	}
	int FindItemExact( const void * C, int fromIdx=-1  )
	{
		return SendMessage( *this, LB_FINDSTRINGEXACT, fromIdx, (LPARAM)C );
	}
	int FindItemChecked( const void * C, int fromIdx=-1  )
	{
		int result = SendMessage( *this, LB_FINDSTRING, fromIdx, (LPARAM)C );
		CHECK(result!=LB_ERR);
		return result;
	}
	int FindItemExactChecked( const void * C, int fromIdx=-1  )
	{
		int result = SendMessage( *this, LB_FINDSTRINGEXACT, fromIdx, (LPARAM)C );
		CHECK(result!=LB_ERR);
		return result;
	}
	void InsertItemAfter( const void * existing, const void * str )
	{
		InsertItem( FindItemChecked(existing)+1, str );
	}
	std::string IGetValue() const
	{
		std::string retVal = "";
		std::wstring tempString = L"";
		int index = GetCurrent();
		if (index>=0)
			tempString = GetString(index);
		else
			tempString = GetText();
		
		char *temp = hsWStringToString(tempString.c_str());
		retVal = temp;
		delete [] temp;
		return retVal;
	}
	void ISetValue(const char * value)
	{
		wchar_t *temp = hsStringToWString(value);
		SetCurrent(FindString(temp), true);
		delete [] temp;
	}
};

#endif // plListBox_h_inc
