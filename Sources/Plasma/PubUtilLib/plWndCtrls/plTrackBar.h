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
#ifndef plTrackBar_h_inc
#define plTrackBar_h_inc


class plTrackBar : public plControl
{
public:
	DECLARE_WINDOWSUBCLASS(plTrackBar,plControl)

	plDelegate fThumbTrackDelegate;
	plDelegate fThumbPositionDelegate;

	plTrackBar()
	{}
	plTrackBar( plWindow * inOwner, int inId=0, WNDPROC inSuperProc=nil )
	: plControl( inOwner, inId, inSuperProc?inSuperProc:_SuperProc )
	{}

	void OpenWindow( bool Visible )
	{
		PerformCreateWindowEx
		(
			WS_EX_CLIENTEDGE,
            nil,
            WS_CHILD | TBS_HORZ | TBS_AUTOTICKS | TBS_BOTTOM | (Visible?WS_VISIBLE:0),
            0, 0,
			0, 0,
            *fOwnerWindow,
            (HMENU)fControlID,
            plWndCtrls::Instance()
		);
	}

	bool InterceptControlCommand( unsigned int Message, unsigned int wParam, LONG lParam )
	{
		if     ( Message==WM_HSCROLL && LOWORD(wParam)==TB_THUMBTRACK ) {fThumbTrackDelegate();    return 1;}
		else if( Message==WM_HSCROLL                                  ) {fThumbPositionDelegate(); return 1;}
		else return 0;
	}

	void SetTicFreq( int TicFreq )
	{
		SendMessage( *this, TBM_SETTICFREQ, TicFreq, 0 );
	}
	void SetRange( int Min, int Max )
	{
		SendMessage( *this, TBM_SETRANGE, 1, MAKELONG(Min,Max) );
	}
	void SetPos( int Pos )
	{
		SendMessage( *this, TBM_SETPOS, 1, Pos );
	}
	int GetRangeMin()
	{
		return SendMessage(*this, TBM_GETRANGEMIN, 0, 0);
	}
	int GetRangeMax()
	{
		return SendMessage(*this, TBM_GETRANGEMAX, 0, 0);
	}
	int GetPos() const
	{
		return SendMessage( *this, TBM_GETPOS, 0, 0 );
	}
	std::string IGetValue() const
	{
		char tmp[20];
		sprintf(tmp,"%d",GetPos());
		return tmp;
	}
	void ISetValue(const char * value)
	{
		SetPos(atoi(value));
	}
};

#endif // plTrackBar_h_inc
