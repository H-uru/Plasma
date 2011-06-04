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
#ifndef plProgressBar_h_inc
#define plProgressBar_h_inc

class plProgressBar : public plControl
{
public:
	DECLARE_WINDOWSUBCLASS(plProgressBar,plControl)

	int fPercent;
	int fMax;

	plProgressBar()
	{}
	plProgressBar( plWindow * inOwner, int inId=0, WNDPROC inSuperProc=nil )
	: plControl( inOwner, inId, inSuperProc?inSuperProc:_SuperProc )
	, fPercent( 0 )
	, fMax( 100 )
	{}

	void OpenWindow( bool Visible )
	{
		PerformCreateWindowEx
		(
			WS_EX_CLIENTEDGE,
            nil,
            WS_CHILD | (Visible?WS_VISIBLE:0),
            0, 0,
			0, 0,
            *fOwnerWindow,
            (HMENU)fControlID,
            plWndCtrls::Instance()
		);
		SendMessage( *this, PBM_SETRANGE, 0, 100 );
	}

	void SetMax(int inMax)
	{
		fMax = inMax;
	}
	void SetProgress( int inCurrent )
	{
		int inPercent = (int)((float(inCurrent)/float(Max(fMax,1)))*100.f);
		if( inPercent!=fPercent )
			SendMessage( *this, PBM_SETPOS, inPercent, 0 );
		fPercent = inPercent;
	}
	int GetProgress() const
	{
		int value = SendMessage( *this, PBM_GETPOS, 0, 0 );
		return value;
	}
	std::string IGetValue() const
	{
		char tmp[20];
		sprintf(tmp,"%d",GetProgress());
		return tmp;
	}
	void ISetValue(const char * value)
	{
		SetProgress(atoi(value));
	}
};


#endif // plProgressBar_h_inc
