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
#ifndef plComboBox_h_inc
#define plComboBox_h_inc

class plComboBox : public plControl
{
public:
	DECLARE_WINDOWSUBCLASS(plComboBox,plControl)

	plDelegate fDoubleClickDelegate;
	plDelegate fDropDownDelegate;
	plDelegate fCloseComboDelegate;
	plDelegate fEditChangeDelegate;
	plDelegate fEditUpdateDelegate;
	plDelegate fSetFocusDelegate;
	plDelegate fKillFocusDelegate;
	plDelegate fSelectionChangeDelegate;
	plDelegate fSelectionEndOkDelegate;
	plDelegate fSelectionEndCancelDelegate;
	plDelegate fEnterKeyPressedDelegate;
 
	plComboBox()
	{}
	plComboBox( plWindow * inOwner, int inId=0, WNDPROC inSuperProc=nil )
	: plControl( inOwner, inId, inSuperProc?inSuperProc:_SuperProc )
	{}

	void OpenWindow( bool visible )
	{
		PerformCreateWindowEx
		(
			0,
            nil,
            WS_CHILD | WS_VSCROLL | CBS_DROPDOWNLIST | (visible?WS_VISIBLE:0),
            0, 0,
			64, 384,
            *fOwnerWindow,
            (HMENU)fControlID,
            plWndCtrls::Instance()
		);
		SendMessage( *this, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(0,0) );
	}
	LONG WndProc( unsigned int message, unsigned int wParam, LONG lParam )
	{
		switch (message)
		{
		case WM_GETDLGCODE:
			return DLGC_WANTALLKEYS;
			
		case WM_CHAR:
			//Process this message to avoid message beeps.
			if ((wParam == VK_RETURN) || (wParam == VK_TAB))
				return 0;
			else
				return plControl::WndProc( message, wParam, lParam );
			
		default:
			return plControl::WndProc( message, wParam, lParam );
		}
	}

	void OnKeyDown( UInt16 ch )
	{
		if (ch==VK_RETURN)
			fEnterKeyPressedDelegate();
		else if (ch==VK_TAB)
			PostMessage (*fOwnerWindow, WM_NEXTDLGCTL, 0, 0L);
	}

	bool InterceptControlCommand( unsigned int message, unsigned int wParam, LONG lParam )
	{
		if     ( HIWORD(wParam)==CBN_DBLCLK         ) {fDoubleClickDelegate();        return 1;}
		else if( HIWORD(wParam)==CBN_DROPDOWN       ) {fDropDownDelegate();           return 1;}
		else if( HIWORD(wParam)==CBN_CLOSEUP        ) {fCloseComboDelegate();         return 1;}
		else if( HIWORD(wParam)==CBN_EDITCHANGE     ) {fEditChangeDelegate();         return 1;}
		else if( HIWORD(wParam)==CBN_EDITUPDATE     ) {fEditUpdateDelegate();         return 1;}
		else if( HIWORD(wParam)==CBN_SETFOCUS       ) {fSetFocusDelegate();           return 1;}
		else if( HIWORD(wParam)==CBN_KILLFOCUS      ) {fKillFocusDelegate();          return 1;}
		else if( HIWORD(wParam)==CBN_SELCHANGE      ) {fSelectionChangeDelegate();    return 1;}
		else if( HIWORD(wParam)==CBN_SELENDOK       ) {fSelectionEndOkDelegate();     return 1;}
		else if( HIWORD(wParam)==CBN_SELENDCANCEL   ) {fSelectionEndCancelDelegate(); return 1;}
		else return 0;
	}

	virtual int InsertString( int index, const wchar_t * str )
	{
		return SendMessage( *this, CB_INSERTSTRING, index, (LPARAM)str );
	}
	virtual void InsertStringAndData( int index, const wchar_t * str, void * item )
	{
		int i = SendMessage( *this, CB_INSERTSTRING, index, (LPARAM)str );
		SetItemData(i,item);
	}
	virtual int AddString( const wchar_t * str )
	{
		return SendMessage( *this, CB_ADDSTRING, 0, (LPARAM)str );
	}
	virtual void AddStringAndData( const wchar_t * str, void * item )
	{
		int index = SendMessage( *this, CB_ADDSTRING, 0, (LPARAM)str );
		SetItemData(index,item);
	}
	virtual void AddStrings( const std::vector<std::wstring> & strings )
	{
		for (int i=0; i<strings.size(); i++)
			SendMessage( *this, CB_ADDSTRING, 0, (LPARAM)strings[i].c_str() );
	}
	virtual void AddStrings( const std::vector<std::string> & strings )
	{
		for (int i=0; i<strings.size(); i++)
		{
			wchar_t* temp = hsStringToWString(strings[i].c_str());
			SendMessage( *this, CB_ADDSTRING, 0, (LPARAM)temp );
			delete [] temp;
		}
	}
	virtual void SetItemData(int index, void * item)
	{
		SendMessage( *this, CB_SETITEMDATA, index, (LPARAM)item );
	}
	virtual void * GetItemData(int index)
	{
		return (void*)SendMessage( *this, CB_GETITEMDATA, index, 0 );
	}
	virtual std::wstring GetString( int index ) const
	{
		int length = SendMessage( *this, CB_GETLBTEXTLEN, index, 0 );
		if( length==CB_ERR || length == 0 )
		return L"";
		std::wstring text;
		text.resize(length);
		SendMessage( *this, CB_GETLBTEXT, index, (LPARAM)text.data() );
		return text;
	}
	virtual void DeleteString(int index)
	{
		SendMessage(*this,CB_DELETESTRING, index, 0);
	}
	virtual int GetCount()
	{
		return SendMessage( *this, CB_GETCOUNT, 0, 0 );
	}
	virtual void SetCurrent( int index )
	{
		SendMessage( *this, CB_SETCURSEL, index, 0 );
	}
	virtual int GetCurrent() const
	{
		return SendMessage( *this, CB_GETCURSEL, 0, 0 );
	}
	virtual int FindStringExact( const wchar_t * string, int fromIdx = -1 )
	{
		int index = SendMessage( *this, CB_FINDSTRINGEXACT, fromIdx, (LPARAM)string );
		return index!=CB_ERR ? index : -1;
	}
	virtual int FindString( const wchar_t * string, int fromIdx = -1 )
	{
		int index = SendMessage( *this, CB_FINDSTRING, fromIdx, (LPARAM)string );
		return index!=CB_ERR ? index : -1;
	}
	void Empty()
	{
		SendMessage( *this, CB_RESETCONTENT, 0, 0 );
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
		SetCurrent(FindString(temp));
		delete [] temp;
	}
};


#endif // plComboBox_h_inc
