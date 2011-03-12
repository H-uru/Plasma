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
#ifndef plControl_h_inc
#define plControl_h_inc


class plControl : public plWindow
{
public:
	WNDPROC WindowDefWndProc;

	plControl()
	{}
	plControl( plWindow * ownerWindow, int inId, WNDPROC inSuperProc )
	: plWindow( ownerWindow )
	{
		CHECK(fOwnerWindow);
		WindowDefWndProc = inSuperProc;
		fControlID = inId ? inId : ownerWindow->fTopControlID++;
		fOwnerWindow->fControls.push_back( this );
	}
	~plControl()
	{
		CHECK(fOwnerWindow);
		std::vector<plControl*>::iterator it = std::find(fOwnerWindow->fControls.begin(),fOwnerWindow->fControls.end(),this);
		if (it!=fOwnerWindow->fControls.end())
			fOwnerWindow->fControls.erase(it);
	}

	int CallDefaultProc( unsigned int message, unsigned int wParam, LONG lParam )
	{
		return CallWindowProc( WindowDefWndProc, Handle(), message, wParam, lParam );
	}
	static WNDPROC RegisterWindowClass( const wchar_t * name, const wchar_t * winBaseClass )
	{
#ifdef UNICODE
		WNDPROC superProc=nil;
		WNDCLASSEX cls;
		HSMemory::ClearMemory( &cls, sizeof(cls) );
		cls.cbSize        = sizeof(cls);
		CHECK( GetClassInfoEx( nil, winBaseClass, &cls ) );
		superProc         = cls.lpfnWndProc;
		cls.lpfnWndProc   = plWindow::StaticWndProc;
		cls.lpszClassName = name;
		cls.hInstance     = plWndCtrls::Instance();
		CHECK(cls.lpszMenuName==nil);
		CHECK(RegisterClassEx( &cls ));
#else
		char* cWinBaseClass = hsWStringToString(winBaseClass);
		char* cName = hsWStringToString(name);

		WNDPROC superProc=nil;
		WNDCLASSEX cls;
		HSMemory::ClearMemory( &cls, sizeof(cls) );
		cls.cbSize        = sizeof(cls);
		CHECK( GetClassInfoEx( nil, cWinBaseClass, &cls ) );
		superProc         = cls.lpfnWndProc;
		cls.lpfnWndProc   = plWindow::StaticWndProc;
		cls.lpszClassName = cName;
		cls.hInstance     = plWndCtrls::Instance();
		CHECK(cls.lpszMenuName==nil);
		CHECK(RegisterClassEx( &cls ));

		delete [] cName;
		delete [] cWinBaseClass;
#endif
		return superProc;
	}
};


#endif //  plControl_h_inc
