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
#ifndef plEdit_h_inc
#define plEdit_h_inc


class plEdit : public plControl
{
private:
	bool fRestrictToAlphaNum;
public:
	DECLARE_WINDOWSUBCLASS(plEdit,plControl)

	plDelegate	fChangeDelegate;
	plDelegate	fKillFocusDelegate;

	plEdit()
	{fRestrictToAlphaNum = false;}
	plEdit( plWindow * inOwner, int inId=0, WNDPROC inSuperProc=nil, bool restrict=false )
	: plControl( inOwner, inId, inSuperProc?inSuperProc:_SuperProc ), fRestrictToAlphaNum(restrict)
	{}

	void OpenWindow( bool visible, bool multiline, bool readOnly )
	{
		PerformCreateWindowEx
		(
			WS_EX_CLIENTEDGE,
            nil,
            WS_CHILD | (visible?WS_VISIBLE:0) | ES_LEFT | (multiline?(ES_MULTILINE|WS_VSCROLL|WS_HSCROLL):0) | ES_AUTOVSCROLL | ES_AUTOHSCROLL | (readOnly?ES_READONLY:0),
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
		if (HIWORD(wParam)==EN_CHANGE)
		{
			fChangeDelegate();
			return 1;
		}
		else if(HIWORD(wParam)==EN_KILLFOCUS)
		{
			fKillFocusDelegate();
			return 1;
		}
		else
			return 0;
	}
	
	virtual LONG OnChar( char ch )
	{
		if (fRestrictToAlphaNum)
		{
			// we only accept 0-9, a-z, A-Z, or backspace
			if ((ch < '0' || ch > '9') && (ch < 'a' || ch > 'z') && (ch < 'A' || ch >'Z') && !(ch == VK_BACK))
			{
				MessageBeep(-1); // alert the user
				return FALSE; // and make sure the default handler doesn't get it
			}
		}
		return TRUE;
	}

	bool GetReadOnly()
	{
		CHECK(Handle());
		return (GetWindowLong( *this, GWL_STYLE )&ES_READONLY)!=0;
	}
	void SetReadOnly( bool readOnly )
	{
		CHECK(Handle());
		SendMessage( *this, EM_SETREADONLY, readOnly, 0 );
	}
	int GetLineCount()
	{
		CHECK(Handle());
		return SendMessage( *this, EM_GETLINECOUNT, 0, 0 );
	}
	int GetLineIndex( int line )
	{
		CHECK(Handle());
		return SendMessage( *this, EM_LINEINDEX, line, 0 );
	}
	void GetSelection( int& start, int& end )
	{
		CHECK(Handle());
		SendMessage( *this, EM_GETSEL, (WPARAM)&start, (LPARAM)&end );
	}
	void SetSelection( int start, int end )
	{
		CHECK(Handle());
		SendMessage( *this, EM_SETSEL, start, end );
	}
	void SetSelectedText( const wchar_t * text )
	{
		CHECK(Handle());
		SendMessage( *this, EM_REPLACESEL, 1, (LPARAM)text );
	}
	void SetLimitText( int maxChars )
	{
		CHECK(Handle());
		SendMessage( *this, EM_SETLIMITTEXT, maxChars, 0 );
	}
	bool GetModify()
	{
		return SendMessage( *this, EM_GETMODIFY, 0, 0 )!=0;
	}
	void SetModify( bool modified )
	{
		SendMessage( *this, EM_SETMODIFY, modified, 0 );
	}
	void ScrollCaret()
	{
		SendMessage( *this, EM_SCROLLCARET, 0, 0 );
	}
};


#endif // plEdit_h_inc
